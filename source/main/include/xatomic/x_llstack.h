#ifndef __XMULTICORE_LL_STACK_H__
#define __XMULTICORE_LL_STACK_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase/x_debug.h"
#include "xbase/x_allocator.h"

namespace xcore
{
	namespace atomic
	{
		namespace xllstack
		{
			struct node;
			struct pointer
			{
							pointer() : mCount(0), mPtr(NULL) { }
							pointer(pointer const& o) : mCount(o.mCount), mPtr(o.mPtr) { }

				u32			mCount;
				node*		mPtr;
			};

			struct node
			{
							node() : mValue(NULL) { }
							node(node const& o) : mValue(o.mValue), mNext(o.mNext) { }
				void*		mValue;
				pointer		mNext;
			};
		};

		struct xllstack_t;

		namespace xllstack_f
		{
			typedef xllstack::node*			(*alloc)  (xllstack_t& stack, void* item);
			typedef void					(*dealloc)(xllstack_t& stack, xllstack::node* node);

			typedef xllstack::node*			(*to_node)(xllstack_t& stack, void* item);
			typedef void*					(*to_item)(xllstack_t& stack, xllstack::node* node);
		}

		struct xllstack_functors
		{
			xllstack_f::alloc			mFAlloc;
			xllstack_f::dealloc			mFDealloc;
			xllstack_f::to_node			mFItemToNode;
			xllstack_f::to_item			mFNodeToItem;
		};

		struct xllstack_t
		{
			x_iallocator*				mAllocator;
			xllstack::node				mGuard;
			xllstack::pointer			mHead;
			xllstack_functors			mFunctors;
		};

		xllstack_t*		xllstack_heap		(x_iallocator* heap);
		xllstack_t*		xllstack_member		(x_iallocator* heap, u32 member_offset);

		void			xllstack_free		(xllstack_t *stack);

		bool			xllstack_is_empty	(xllstack_t *stack);
		void			xllstack_clear		(xllstack_t *stack);

		bool			xllstack_push		(xllstack_t *stack, void* data);
		void*			xllstack_pop		(xllstack_t *stack);
		void*			xllstack_peek		(xllstack_t *stack);

	};
};

#endif	///< __XMULTICORE_LL_STACK_H__
