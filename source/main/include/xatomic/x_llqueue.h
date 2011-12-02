#ifndef __XMULTICORE_LL_QUEUE_H__
#define __XMULTICORE_LL_QUEUE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_debug.h"
#include "xbase\x_allocator.h"

namespace xcore
{
	namespace atomic
	{
		namespace xllqueue
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

		struct xllqueue_t;

		namespace xllqueue_f
		{
			typedef xllqueue::node*			(*alloc)  (xllqueue_t& queue, void* item);
			typedef void					(*dealloc)(xllqueue_t& queue, xllqueue::node* node);

			typedef xllqueue::node*			(*to_node)(xllqueue_t& queue, void* item);
			typedef void*					(*to_item)(xllqueue_t& queue, xllqueue::node* node);
		}

		struct xllqueue_functors
		{
			xllqueue_f::alloc			mFAlloc;
			xllqueue_f::dealloc			mFDealloc;
			xllqueue_f::to_node			mFItemToNode;
			xllqueue_f::to_item			mFNodeToItem;
		};

		struct xllqueue_t
		{
			x_iallocator*				mAllocator;
			xllqueue::node				mGuard;
			xllqueue::pointer			mHead;
			xllqueue::pointer			mTail;
			xllqueue_functors			mFunctors;
		};

		xllqueue_t*		xllqueue_heap		(x_iallocator* heap);
		xllqueue_t*		xllqueue_member		(x_iallocator* heap, u32 member_offset);

		void			xllqueue_free		(xllqueue_t *queue);

		bool			xllqueue_is_empty	(xllqueue_t *queue);
		void			xllqueue_clear		(xllqueue_t *queue);

		bool			xllqueue_enqueue	(xllqueue_t *queue, void* data);
		void*			xllqueue_dequeue	(xllqueue_t *queue);

		void*			xllqueue_peek		(xllqueue_t *queue);

		inline bool		xllqueue_push		(xllqueue_t *queue, void* data)				{ return xllqueue_enqueue(queue, data); }
		inline void*	xllqueue_pop		(xllqueue_t *queue)							{ return xllqueue_dequeue(queue); }

	};
};

#endif	///< __XMULTICORE_LL_QUEUE_H__
