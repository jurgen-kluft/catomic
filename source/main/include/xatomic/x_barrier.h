#ifndef __XMULTICORE_BARRIER_H__
#define __XMULTICORE_BARRIER_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xatomic\x_compiler.h"

namespace xcore
{
	// We're using inline function here instead of #defines to avoid name space clashes.
	namespace barrier
	{
		void		comp();
		void		memr();
		void		memw();
		void		memrw();
	} // namespace barrier

	#if defined(TARGET_PC)
		#include "xatomic\private\x_barrier_x86_win32.h"
	#elif defined(TARGET_PS3)
		#include "xatomic\private\x_barrier_ppc_ps3.h"
	#else
		#error Unsupported CPU
	#endif

} // namespace xcore

#endif // __XMULTICORE_BARRIER_H__
