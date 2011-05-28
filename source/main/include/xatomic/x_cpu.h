#ifndef __XMULTICORE_CPU_H__
#define __XMULTICORE_CPU_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xmulticore\x_compiler.h"

/**
* CPU library. Provides things like reading cpu cycle counter (tsc), 
* converting tsc to microseconds, etc.
*/
namespace xcore
{
	namespace xcpu
	{
		// Functions that start with double underscore are processor specific

		// Cpu frequency in kilo-hertz
		extern u64 _khz;

		// Overhead of various functions
		extern struct overhead
		{
			u64 nanosleep;
		} _overhead;

		extern bool calibrate();

		// Capabilities of the current process
		extern u64 _caps;
		enum
		{
			CAPS_IRQ_LOCK = 0,
		};

		static inline bool capable(u32 cap)
		{
			return _caps & cap;
		}

		#if (defined(TARGET_PC))
			#include "xmulticore\private\x_cpu_x86_win32.h"
		#else
			#error Unsupported CPU
		#endif

		/** 
		* Get current value of the CPU cycle counter.
		* Non serializing version. i.e. may complete sooner than previous
		* instructions.
		* Example:
		*    @code
		*    u64 tsc = cpu::tsc();
		*    @endcode
		*/
		static inline u64 tsc(void)
		{
			return __tsc();
		}

		/**
		* Sleep for @a nsec nanoseconds.
		* This is a busy sleep not a regular OS nanosleep().
		* @param usec Number of nanoseconds to sleep for
		*/
		void nanosleep(u64 nsec);

		/**
		* Sleep for @a usec microseconds.
		* This is a busy sleep not a regular OS usleep().
		* @param usec Number of microseconds to sleep for
		*/
		static inline void usleep(u64 usec)
		{
			nanosleep(usec * 1000);
		}

		// Scaled math for converting tsc to nsec and usec.
		// Floating point version
		//     nsec = tsc * (1000000.0 / khz)
		// scaled version
		//     nsec = tsc * (1000000 * SCALE / khz) / SCALE
		// If SCALE is a power of two, last divisions can be replaced with a shift
		//     nsec = tsc * ((1000000 << SCALE_SHIFT) / khz) >> SCALE_SHIFT
		//
		// Scale factors are computed by cpu::calibrate()
		extern struct scale
		{
			u64 tsc2usec;
			u64 tsc2nsec;
			u64 nsec2tsc;
			u64 usec2tsc;
		} _scale;

		// These shifts provide good precision (almost match floating point)
		// but require full 64bits
		enum 
		{
			TSC2NSEC_SCALE_SHIFT = 20,
			TSC2USEC_SCALE_SHIFT = 30,
			NSEC2TSC_SCALE_SHIFT = 30,
			USEC2TSC_SCALE_SHIFT = 20,
		};

		/**
		* Convert tsc to nanoseconds.
		* @param tsc number of cycles
		*/
		static inline u64 tsc2nsec(u64 tsc)
		{
			return (tsc * _scale.tsc2nsec) >> TSC2NSEC_SCALE_SHIFT;
		}

		/**
		* Convert tsc to microseconds.
		* @param tsc number of cycles
		*/
		static inline u64 tsc2usec(u64 tsc)
		{
			return (tsc * _scale.tsc2usec) >> TSC2USEC_SCALE_SHIFT;
		}

		/**
		* Convert nanoseconds to tsc.
		* @param nsec nanoseconds
		* @warning due to rounding errors it's better to always convert
		* one way. In other words not to mix tsc2nsec and nsec2tsc.
		*/
		static inline u64 nsec2tsc(u64 nsec)
		{
			return (nsec * _scale.nsec2tsc) >> NSEC2TSC_SCALE_SHIFT;
		}

		/**
		* Convert microseconds to CPU tsc.
		* @param usec microseconds
		* @warning due to rounding errors it's better to always convert
		* one way. In other words to not mix tsc2usec and usec2tsc.
		*/
		static inline u64 usec2tsc(u64 usec)
		{
			return (usec * _scale.usec2tsc) >> USEC2TSC_SCALE_SHIFT;
		}

		/**
		* Get difference in usec from the previous measurement.
		* @param t number of cycles
		*/
		static inline u64 usec_elapsed(u64 t)
		{
			return tsc2usec(tsc() - t);
		}

		/**
		* Get difference in nsec from the previous measurement.
		* @param t number of cycles
		*/
		static inline u64 nsec_elapsed(u64 t)
		{
			return tsc2nsec(tsc() - t);
		}

	} // namespace cpu
} // namespace xcore

#endif // __XMULTICORE_CPU_H__
