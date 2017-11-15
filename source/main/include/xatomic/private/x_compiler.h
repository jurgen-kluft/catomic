#ifndef __XMULTICORE_COMPILER_H__
#define __XMULTICORE_COMPILER_H__
#include "xbase/x_target.h"
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
		#define force_inline	f_inline
	#endif
#elif defined(COMPILER_X360)
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
#elif defined(COMPILER_SN_PS3)
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
#elif defined(COMPILER_SN_PSP)
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
#elif defined(COMPILER_MW_WII) || defined(COMPILER_3DS)
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



#endif // __XMULTICORE_COMPILER_H__

