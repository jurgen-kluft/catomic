#ifndef __XMULTICORE_BARRIER_H__
#define __XMULTICORE_BARRIER_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "catomic/private/c_compiler.h"

#if defined(TARGET_360)
#include <PPCIntrinsics.h>
#endif

namespace xcore
{
	/**
	 * We're using inline function here instead of #defines to avoid name space clashes.
	 */
	namespace barrier
	{
		void		comp();
		void		memr();
		void		memw();
		void		memrw();
	} // namespace barrier
}


#if defined(TARGET_PC)
	#if defined(TARGET_32BIT)
		#include "catomic/private/c_barrier_x86_win32.h"
	#else
		#include "catomic/private/c_barrier_x86_win64.h"
	#endif
#elif defined(TARGET_360)
	#include "catomic/private/c_barrier_ppc_360.h"
#elif defined(TARGET_PS3)
	#include "catomic/private/c_barrier_ppc_ps3.h"
#elif defined(TARGET_WII)
	#include "catomic/private/c_barrier_ppc_wii.h"
#elif defined(TARGET_3DS)
	#include "catomic/private/c_barrier_arm_3ds.h"
#else
	#error Unsupported CPU
#endif

#endif // __XMULTICORE_BARRIER_H__