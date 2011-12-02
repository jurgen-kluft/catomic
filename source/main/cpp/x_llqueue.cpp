#include "xbase\x_target.h"
#include "xbase\x_allocator.h"
#include "xatomic\x_llqueue.h"
#include "xatomic\x_atomic.h"

namespace xcore
{
	namespace atomic
	{
		void		xllqueue_free(xllqueue_t *queue) 
		{
			if (queue != NULL)
			{
				xllqueue_clear(queue);
				queue->mAllocator->deallocate(queue);
			}
		}

		bool			xllqueue_is_empty (xllqueue_t *queue) 
		{
			return(queue->mHead.mPtr == NULL);
		}

		void			xllqueue_clear (xllqueue_t *queue) 
		{
			while (xllqueue_dequeue(queue) != NULL)
			{
			}
		}

		void*		xllqueue_peek (xllqueue_t *queue)
		{
			xllqueue::pointer ptr = queue->mHead;
			return ptr.mPtr->mValue;
		}

		void*		xllqueue_dequeue (xllqueue_t *queue)
		{
			xllqueue::pointer head, tail, next, tmp;
			void* outValue = NULL;
			while (true)
			{
				head = queue->mHead;
				tail = queue->mTail;
				next = head.mPtr->mNext;

				if (head.mCount == queue->mHead.mCount && head.mPtr == queue->mHead.mPtr)
				{
					if (head.mPtr == tail.mPtr)
					{
						if (next.mPtr == NULL)
							return NULL;

						tmp.mPtr   = next.mPtr;
						tmp.mCount = head.mCount + 1;
						cas_u64((u64 volatile*)&queue->mTail, *((u64*)&tail), *((u64*)&tmp));
					}
					else
					{
						outValue   = next.mPtr->mValue;

						tmp.mPtr   = next.mPtr;
						tmp.mCount = head.mCount + 1;

						if (cas_u64((u64 volatile*)&queue->mHead, *((u64*)&head), *((u64*)&tmp)) == true)
							break;
					}
				}
			}
			queue->mFunctors.mFDealloc(*queue, head.mPtr);
			return outValue;
		}

		bool			xllqueue_enqueue (xllqueue_t *queue, void* data) 
		{
			xllqueue::pointer tail, next, tmp;

			xllqueue::node *newNode = queue->mFunctors.mFAlloc(*queue, data);
			if (newNode == NULL)
				return false;

			while (1)
			{
				tail = queue->mTail;
				next = tail.mPtr->mNext;

				if (tail.mCount == queue->mTail.mCount && tail.mPtr == queue->mTail.mPtr)
				{
					if (next.mPtr == NULL) 
					{
						tmp.mPtr = newNode;
						tmp.mCount = next.mCount + 1;
						if (cas_u64((u64 volatile*)&tail.mPtr->mNext, *((u64*)&next), *((u64*)&tmp)) == true)
						{
							break;
						}
					}
					else
					{
						tmp.mPtr = next.mPtr;
						tmp.mCount = tail.mCount + 1;
						cas_u64((u64 volatile*)&queue->mTail, *((u64*)&tail), *((u64*)&tmp));
					}
				}
			}
			tmp.mPtr = newNode;
			tmp.mCount = tail.mCount + 1;
			cas_u64((u64 volatile*)&queue->mTail, *((u64*)&tail), *((u64*)&tmp));

			return(true);
		}



		struct xllqueue_heap_provider : public xllqueue_t
		{
		public:
			struct xllqueuenode : public xllqueue::node
			{
				XCORE_CLASS_PLACEMENT_NEW_DELETE
			};

			static xllqueue::node*	alloc(xllqueue_t& queue, void* item)
			{
				void * mem = queue.mAllocator->allocate(sizeof(xllqueuenode), 4);
				xllqueuenode* node = new (mem) xllqueuenode();
				node->mValue = item;
				return node;
			}

			static void				dealloc(xllqueue_t& queue, xllqueue::node* node)
			{
				queue.mAllocator->deallocate(node);
			}

			static xllqueue::node*	to_node(xllqueue_t& queue, void* item)
			{
				// This provider does not have the ability to translate an item to a node
				return NULL;
			}

			static void*			to_item(xllqueue_t& queue, xllqueue::node* node)
			{
				xllqueuenode* item_node = reinterpret_cast<xllqueuenode*>(node);
				return item_node->mValue;
			}

			XCORE_CLASS_PLACEMENT_NEW_DELETE

			xllqueue_heap_provider(x_iallocator* allocator)
			{
				mAllocator = allocator;
				mHead      = xllqueue::pointer();
				mTail      = xllqueue::pointer();

				mHead.mPtr = &mGuard;
				mTail.mPtr = &mGuard;

				mFunctors.mFAlloc = xllqueue_heap_provider::alloc;
				mFunctors.mFDealloc = xllqueue_heap_provider::dealloc;
				mFunctors.mFItemToNode = xllqueue_heap_provider::to_node;
				mFunctors.mFNodeToItem = xllqueue_heap_provider::to_item;
			}
		};

		xllqueue_t*		xllqueue_heap(x_iallocator* heap)
		{
			void* mem = heap->allocate(sizeof(xllqueue_heap_provider), 4);
			xllqueue_heap_provider* provider = new (mem) xllqueue_heap_provider(heap);
			return provider;
		}



		struct xllqueue_embedded_provider : public xllqueue_t
		{
		public:
			u32						mMemberOffset;

			static xllqueue::node*	alloc(xllqueue_t& queue, void* item)
			{
				xllqueue::node* node = (xllqueue::node*)((u32)item + ((xllqueue_embedded_provider&)queue).mMemberOffset);
				node->mValue = item;
				return node;
			}

			static void				dealloc(xllqueue_t& queue, xllqueue::node* node)
			{
				node->mValue = NULL;
			}

			static xllqueue::node*	to_node(xllqueue_t& queue, void* item)
			{
				xllqueue::node* node = (xllqueue::node*)((u32)item + ((xllqueue_embedded_provider&)queue).mMemberOffset);
				return node;
			}

			static void*			to_item(xllqueue_t& queue, xllqueue::node* node)
			{
				void* item = node->mValue;
				return item;
			}

			XCORE_CLASS_PLACEMENT_NEW_DELETE

			xllqueue_embedded_provider(x_iallocator* allocator, u32 member_offset)
				: mMemberOffset(member_offset)
			{
				mAllocator = allocator;
				mHead      = xllqueue::pointer();
				mTail      = xllqueue::pointer();

				mHead.mPtr = &mGuard;
				mTail.mPtr = &mGuard;

				mFunctors.mFAlloc = xllqueue_embedded_provider::alloc;
				mFunctors.mFDealloc = xllqueue_embedded_provider::dealloc;
				mFunctors.mFItemToNode = xllqueue_embedded_provider::to_node;
				mFunctors.mFNodeToItem = xllqueue_embedded_provider::to_item;
			}
		};

		xllqueue_t*		xllqueue_member(x_iallocator* heap, u32 member_offset)
		{
			void* mem = heap->allocate(sizeof(xllqueue_embedded_provider), 4);
			xllqueue_embedded_provider* provider = new (mem) xllqueue_embedded_provider(heap, member_offset);
			return provider;
		}

	} // namespace atomic
} // namespace xcore
