#ifndef __XMULTICORE_LF_QUEUE_H__
#define __XMULTICORE_LF_QUEUE_H__
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
		struct xqueue_data;
		struct xqueue_node
		{
			void*		mData[4];
		};

		struct xqueue
		{
			x_iallocator*		mAllocator;
			void*				mQueueData;

			typedef bool		(*is_empty_func)(void* queue_data);
			typedef void		(*clear_func)(void* queue_data);

			typedef bool		(*push_func	)(void* queue_data, void* item);
			typedef void*		(*peek_func	)(void* queue_data);
			typedef void*		(*pop_func	)(void* queue_data);

			is_empty_func		mFIsEmpty;
			clear_func			mFClear;

			push_func			mFPush;
			peek_func			mFPeek;
			pop_func			mFPop;
		};

		///< Requires the allocator to be thread safe, can be pool allocator since we
		///< will only allocate nodes from this allocator.
		xqueue*			xqueue_heap			(x_iallocator* heap, x_iallocator* nodes);
		
		///< Does own nodes and detects double push/pop, fully thread safe
		xqueue*			xqueue_member		(x_iallocator* heap, u32 member_offset);
		
		///< Fully thread safe, will copy the item
		xqueue*			xqueue_pool			(x_iallocator* heap, u32 item_size, u32 pool_size);

		void			xqueue_free			(xqueue *queue);

		bool			xqueue_is_empty		(xqueue *queue);
		void			xqueue_clear		(xqueue *queue);

		bool			xqueue_enqueue		(xqueue *queue, void* item);
		void*			xqueue_peek			(xqueue *queue);
		void*			xqueue_dequeue		(xqueue *queue);
	};
};

#endif	///< __XMULTICORE_LF_QUEUE_H__
