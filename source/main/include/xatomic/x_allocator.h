#ifndef __XMCORE_ALLOCATION_H__
#define __XMCORE_ALLOCATION_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_allocator.h"

//==============================================================================
// xcore namespace
//==============================================================================
namespace xcore
{
	namespace atomic
	{
		/// The allocator given to xcore needs to be thread-safe
		extern void		set_heap_allocator(xcore::x_iallocator* allocator);
	}

	//==============================================================================
	// END xcore namespace
	//==============================================================================
};

//==============================================================================
// END
//==============================================================================
#endif    /// __XMCORE_ALLOCATION_H__
