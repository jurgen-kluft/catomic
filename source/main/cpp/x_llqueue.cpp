#include "xbase\x_target.h"
#include "xbase\x_allocator.h"
#include "xatomic\x_llqueue.h"
#include "xatomic\x_atomic.h"

namespace xcore
{
	namespace atomic
	{
		// Linked list structures
		namespace ll
		{
			struct node;
			struct pointer
			{
								pointer() : mCount(0), mPtr(NULL) { }
								pointer(pointer const& o) : mCount(o.mCount), mPtr(o.mPtr) { }

				u32				mCount;
				node*			mPtr;
			};

			struct node
			{
								node() : mValue(NULL) { }
								node(node const& o) : mValue(o.mValue), mNext(o.mNext) { }
				XCORE_CLASS_PLACEMENT_NEW_DELETE
				void*			mValue;
				pointer			mNext;
			};

			// Linked list queue data
			struct data
			{
				x_iallocator*	mAllocator;
				ll::node		mGuard;
				ll::pointer		mHead;
				ll::pointer		mTail;
			};
		};



		void			xqueue_free(xqueue *queue) 
		{
			if (queue != NULL)
			{
				xqueue_clear(queue);
				queue->mAllocator->deallocate(queue);
			}
		}

		bool			xqueue_is_empty (xqueue *queue) 
		{
			return queue->mFIsEmpty(queue->mQueueData);
		}

		void			xqueue_clear (xqueue *queue) 
		{
			queue->mFClear(queue->mQueueData);
		}

		bool			xqueue_enqueue (xqueue *queue, void* item)
		{
			return queue->mFPush(queue->mQueueData, item);
		}

		void*			xqueue_peek (xqueue *queue)
		{
			return queue->mFPeek(queue->mQueueData);
		}

		void*			xqueue_dequeue (xqueue *queue)
		{
			return queue->mFPop(queue->mQueueData);
		}

		static bool		ll_enqueue (ll::data *queue, ll::node* new_node) 
		{
			ll::pointer tail, next, tmp;

			while (1)
			{
				tail = queue->mTail;
				next = tail.mPtr->mNext;

				// Are tail and next consistent ?
				if (tail.mCount == queue->mTail.mCount && tail.mPtr == queue->mTail.mPtr)
				{
					// Is tail pointing to the last node ?
					if (next.mPtr == NULL) 
					{
						// Try to link new node at the end of the linked list
						tmp.mPtr = new_node;
						tmp.mCount = next.mCount + 1;
						if (cas_u64((u64 volatile*)&tail.mPtr->mNext, *((u64*)&next), *((u64*)&tmp)) == true)
						{
							break;
						}
					}
					else	// Tail was not pointing to the last node
					{
						tmp.mPtr = next.mPtr;
						tmp.mCount = tail.mCount + 1;
						// Try to swing tail to the next node
						cas_u64((u64 volatile*)&queue->mTail, *((u64*)&tail), *((u64*)&tmp));
					}
				}
			}
			tmp.mPtr = new_node;
			tmp.mCount = tail.mCount + 1;
			cas_u64((u64 volatile*)&queue->mTail, *((u64*)&tail), *((u64*)&tmp));

			return(true);
		}

		static void*		ll_peek(ll::data* queue)
		{
			ll::pointer head = queue->mHead;
			ll::pointer next = head.mPtr->mNext;
			if (next.mPtr != NULL)
				return next.mPtr->mValue;
			else
				return NULL;
		}

		static ll::node*	ll_dequeue (ll::data *queue)
		{
			ll::pointer head, tail, next, tmp;
			void* outValue = NULL;
			while (true)
			{
				head = queue->mHead;
				tail = queue->mTail;
				next = head.mPtr->mNext;

				if (head.mCount == queue->mHead.mCount && head.mPtr == queue->mHead.mPtr)
				{
					// Is tail falling behind ?
					if (head.mPtr == tail.mPtr)
					{
						// Is the list empty ?
						if (next.mPtr == NULL)
							return NULL;

						tmp.mPtr   = next.mPtr;
						tmp.mCount = head.mCount + 1;
						// Tail is falling behind, try to advance it
						cas_u64((u64 volatile*)&queue->mTail, *((u64*)&tail), *((u64*)&tmp));
					}
					else	// No need to deal with tail
					{
						// Read value before CAS otherwise another deque might try to free the next node
						outValue   = next.mPtr->mValue;

						tmp.mPtr   = next.mPtr;
						tmp.mCount = head.mCount + 1;
						// Try to swing head to the next node
						if (cas_u64((u64 volatile*)&queue->mHead, *((u64*)&head), *((u64*)&tmp)) == true)
							break;
					}
				}
			}
			head.mPtr->mValue = outValue;
			return head.mPtr;
		}

		
		struct xqueue_heap_provider : public xqueue
		{
		public:
			struct llnode : public ll::node
			{
				XCORE_CLASS_PLACEMENT_NEW_DELETE
			};

			static bool			is_empty(void* queue_data)
			{
				void* item = ll_peek((ll::data*)queue_data);
				return item!=NULL;
			}

			static void			clear(void* queue_data)
			{
				while (ll_dequeue((ll::data*)queue_data)!=NULL)
				{

				}
			}

			static bool			push(void* queue_data, void* item)
			{
				// Allocate a new node
				ll::data* ll_data = (ll::data*)queue_data;
				void* new_node_mem = ll_data->mAllocator->allocate(sizeof(ll::node), 4);
				ll::node* new_node = new (new_node_mem) ll::node();
				new_node->mValue = item;
				return ll_enqueue(ll_data, new_node);
			}

			static void*		peek(void* queue_data)
			{
				return ll_peek((ll::data*)queue_data);
			}

			static void*		pop(void* queue_data)
			{
				return ll_dequeue((ll::data*)queue_data);
			}

			XCORE_CLASS_PLACEMENT_NEW_DELETE

			xqueue_heap_provider(x_iallocator* allocator)
			{
				mAllocator = allocator;

				mData.mAllocator = allocator;
				mData.mGuard.mValue = (void*)0xDEADFEED;
				mData.mHead      = ll::pointer();
				mData.mTail      = ll::pointer();
				mData.mHead.mPtr = &mData.mGuard;
				mData.mTail.mPtr = &mData.mGuard;

				mFIsEmpty = is_empty;
				mFClear   = clear;
				mFPush    = push;
				mFPeek    = peek;
				mFPop     = pop;
			}
		private:

			ll::data			mData;
		};

		xqueue*		xqueue_heap(x_iallocator* heap)
		{
			void* mem = heap->allocate(sizeof(xqueue_heap_provider), 4);
			xqueue_heap_provider* provider = new (mem) xqueue_heap_provider(heap);
			return provider;
		}



		struct xqueue_embedded_provider : public xqueue
		{
		public:
			static bool			is_empty(void* queue_data)
			{
				void* item = ll_peek((ll::data*)queue_data);
				return item!=NULL;
			}

			static void			clear(void* queue_data)
			{
				while (ll_dequeue((ll::data*)queue_data)!=NULL)
				{

				}
			}

			static bool			push(void* queue_data, void* item)
			{
				// Allocate a new node
				ll::node* new_node = NULL;
				new_node->mValue = item;
				return ll_enqueue((ll::data*)queue_data, new_node);
			}

			static void*		peek(void* queue_data)
			{
				return ll_peek((ll::data*)queue_data);
			}

			static void*		pop(void* queue_data)
			{
				return ll_dequeue((ll::data*)queue_data);
			}

			XCORE_CLASS_PLACEMENT_NEW_DELETE

			xqueue_embedded_provider(x_iallocator* allocator, u32 member_offset)
				: mMemberOffset(member_offset)
			{
				mAllocator = allocator;

				mData.mAllocator = allocator;
				mData.mGuard.mValue = (void*)0xDEADFEED;
				mData.mHead      = ll::pointer();
				mData.mTail      = ll::pointer();
				mData.mHead.mPtr = &mData.mGuard;
				mData.mTail.mPtr = &mData.mGuard;

				mFIsEmpty = is_empty;
				mFClear   = clear;
				mFPush    = push;
				mFPeek    = peek;
				mFPop     = pop;
			}

		private:
			u32					mMemberOffset;
			ll::data			mData;
		};

		xqueue*		xqueue_member(x_iallocator* heap, u32 member_offset)
		{
			void* mem = heap->allocate(sizeof(xqueue_embedded_provider), 4);
			xqueue_embedded_provider* provider = new (mem) xqueue_embedded_provider(heap, member_offset);
			return provider;
		}

	} // namespace atomic
} // namespace xcore
