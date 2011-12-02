#include "xbase\x_target.h"
#include "xbase\x_allocator.h"
#include "xatomic\x_llstack.h"
#include "xatomic\x_atomic.h"

namespace xcore
{
	namespace atomic
	{
		void			xllstack_free(xllstack_t *stack) 
		{
			if (stack != NULL)
			{
				xllstack_clear(stack);
				stack->mAllocator->deallocate(stack);
			}
		}

		bool			xllstack_is_empty (xllstack_t *stack) 
		{
			return(stack->mHead.mPtr == NULL);
		}

		void			xllstack_clear (xllstack_t *stack) 
		{
			while (xllstack_pop(stack) != NULL)
			{
			}
		}

		void*			xllstack_peek (xllstack_t *stack)
		{
			xllstack::pointer ptr = stack->mHead;
			return ptr.mPtr->mValue;
		}

		void*			xllstack_pop (xllstack_t *stack)
		{
			xllstack::pointer head, tail, next, tmp;
			void* outValue = NULL;
			do
			{
				head       = stack->mHead;
				next       = head.mPtr->mNext;
				if (next.mPtr == NULL)
					return NULL;

				tmp.mPtr   = next.mPtr;
				tmp.mCount = head.mCount + 1;
				outValue   = head.mPtr->mValue;
			} while (cas_u64((u64 volatile*)&stack->mHead, *((u64*)&head), *((u64*)&tmp)) == false);

			stack->mFunctors.mFDealloc(*stack, head.mPtr);
			return outValue;
		}

		bool			xllstack_push (xllstack_t *stack, void* data) 
		{
			xllstack::pointer head, next, tmp;

			xllstack::node *newNode = stack->mFunctors.mFAlloc(*stack, data);
			if (newNode == NULL)
				return false;

			do
			{
				head = stack->mHead;
				next = head.mPtr->mNext;
				newNode->mNext = next;
				tmp.mPtr   = newNode;
				tmp.mCount = head.mCount + 1;
			} while (cas_u64((u64 volatile*)&stack->mHead, *((u64*)&head), *((u64*)&tmp)) == false);

			return(true);
		}



		struct xllstack_heap_provider : public xllstack_t
		{
		public:
			struct xllstacknode : public xllstack::node
			{
				XCORE_CLASS_PLACEMENT_NEW_DELETE
			};

			static xllstack::node*	alloc(xllstack_t& stack, void* item)
			{
				void * mem = stack.mAllocator->allocate(sizeof(xllstacknode), 4);
				xllstacknode* node = new (mem) xllstacknode();
				node->mValue = item;
				return node;
			}

			static void				dealloc(xllstack_t& stack, xllstack::node* node)
			{
				stack.mAllocator->deallocate(node);
			}

			static xllstack::node*	to_node(xllstack_t& stack, void* item)
			{
				// This provider does not have the ability to translate an item to a node
				return NULL;
			}

			static void*			to_item(xllstack_t& stack, xllstack::node* node)
			{
				xllstacknode* item_node = reinterpret_cast<xllstacknode*>(node);
				return item_node->mValue;
			}

			XCORE_CLASS_PLACEMENT_NEW_DELETE

			xllstack_heap_provider(x_iallocator* allocator)
			{
				mAllocator = allocator;
				mHead      = xllstack::pointer();
				mHead.mPtr = &mGuard;

				mFunctors.mFAlloc = xllstack_heap_provider::alloc;
				mFunctors.mFDealloc = xllstack_heap_provider::dealloc;
				mFunctors.mFItemToNode = xllstack_heap_provider::to_node;
				mFunctors.mFNodeToItem = xllstack_heap_provider::to_item;
			}
		};

		xllstack_t*		xllstack_heap(x_iallocator* heap)
		{
			void* mem = heap->allocate(sizeof(xllstack_heap_provider), 4);
			xllstack_heap_provider* provider = new (mem) xllstack_heap_provider(heap);
			return provider;
		}



		struct xllstack_embedded_provider : public xllstack_t
		{
		public:
			u32						mMemberOffset;

			static xllstack::node*	alloc(xllstack_t& stack, void* item)
			{
				xllstack::node* node = (xllstack::node*)((u32)item + ((xllstack_embedded_provider&)stack).mMemberOffset);
				node->mValue = item;
				return node;
			}

			static void				dealloc(xllstack_t& stack, xllstack::node* node)
			{
				node->mValue = NULL;
			}

			static xllstack::node*	to_node(xllstack_t& stack, void* item)
			{
				xllstack::node* node = (xllstack::node*)((u32)item + ((xllstack_embedded_provider&)stack).mMemberOffset);
				return node;
			}

			static void*			to_item(xllstack_t& stack, xllstack::node* node)
			{
				void* item = node->mValue;
				return item;
			}

			XCORE_CLASS_PLACEMENT_NEW_DELETE

			xllstack_embedded_provider(x_iallocator* allocator, u32 member_offset)
				: mMemberOffset(member_offset)
			{
				mAllocator = allocator;
				mHead      = xllstack::pointer();
				mHead.mPtr = &mGuard;

				mFunctors.mFAlloc = xllstack_embedded_provider::alloc;
				mFunctors.mFDealloc = xllstack_embedded_provider::dealloc;
				mFunctors.mFItemToNode = xllstack_embedded_provider::to_node;
				mFunctors.mFNodeToItem = xllstack_embedded_provider::to_item;
			}
		};

		xllstack_t*		xllstack_member(x_iallocator* heap, u32 member_offset)
		{
			void* mem = heap->allocate(sizeof(xllstack_embedded_provider), 4);
			xllstack_embedded_provider* provider = new (mem) xllstack_embedded_provider(heap, member_offset);
			return provider;
		}

	} // namespace atomic
} // namespace xcore
