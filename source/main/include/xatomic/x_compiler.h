#ifndef __XMULTICORE_COMPILER_H__
#define __XMULTICORE_COMPILER_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#if defined(COMPILER_MSVC)
	// Branch likely hood links
	#ifndef likely
		#define likely(x)    x
	#endif

	#ifndef unlikely
		#define unlikely(x)  x
	#endif

	// Forced inlining 
	#ifndef force_inline
		#define force_inline	__forceinline
	#endif
#endif

#endif // __XMULTICORE_COMPILER_H__

