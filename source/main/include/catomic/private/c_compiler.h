#ifndef __CMULTICORE_COMPILER_H__
#define __CMULTICORE_COMPILER_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#if defined(COMPILER_WINDOWS_MSVC)
	// Branch likely hood links
	#ifndef likely
		#define likely(x)    x
	#endif

	#ifndef unlikely
		#define unlikely(x)  x
	#endif

	// Forced inlining 
	#ifndef force_inline
		#define force_inline	f_inline
	#endif
#else
	#error Unsupported CPU
#endif



#endif // __CMULTICORE_COMPILER_H__

