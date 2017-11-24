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
		struct xllstack_t;

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
