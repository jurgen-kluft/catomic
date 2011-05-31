#ifndef __XMULTICORE_CPUINFO_H__
#define __XMULTICORE_CPUINFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"


namespace xcore
{
	namespace cpu
	{
		/** 
		* Get current value of the CPU cycle counter.
		* Non serializing version. i.e. may complete sooner than previous
		* instructions.
		* Example:
		*    @code
		*    u64 tsc = cpu::tsc();
		*    @endcode
		*/
		static u64	__tsc();
		static inline u64 tsc(void)
		{
			return __tsc();
		}

		// Scaled math for converting tsc to nsec and usec.
		// Floating point version
		//     nsec = tsc * (1000000.0 / khz)
		// scaled version
		//     nsec = tsc * (1000000 * SCALE / khz) / SCALE
		// If SCALE is a power of two, last divisions can be replaced with a shift
		//     nsec = tsc * ((1000000 << SCALE_SHIFT) / khz) >> SCALE_SHIFT
		struct xtimescale
		{
			u64 tsc2usec;
			u64 tsc2nsec;
			u64 nsec2tsc;
			u64 usec2tsc;
		};
		extern xtimescale	 _scale;

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

		class xspeed
		{
		public:
							xspeed();

			void			calculate();

			u64				getCPUSpeedInkHz() const;
			u64				getCPUSpeedInMHz() const;
		};

		class xinfo
		{
		public:
							xinfo();

			s32				getPhysicalProcessors () const;
			s32				getLogicalProcessorsPerPhysical () const;
			u64				getProcessorClockFrequency () const;

		private:
			// TODO: These should be removed from the public interface
			const char*		getVendorString () const;
			const char*		getVendorID () const;
			const char*		getTypeID () const;
			const char*		getFamilyID () const;
			const char*		getModelID () const;
			const char*		getSteppingCode () const;
			const char*		getExtendedProcessorName () const;
			const char*		getProcessorSerialNumber () const;
			s32				getProcessorAPICID () const;
			s32				getProcessorCacheXSize (u32) const;
			bool			doesCPUSupportFeature (u32) const;
		};
	}
}

#if (defined(TARGET_PC))
	#include "xatomic\private\x_cpu_x86_win32.h"
#elif (defined(TARGET_360))
	#include "xatomic\private\x_cpu_ppc_360.h"
#elif (defined(TARGET_PS3))
	#include "xatomic\private\x_cpu_ppc_ps3.h"
#elif (defined(TARGET_WII))
	#include "xatomic\private\x_cpu_ppc_wii.h"
#elif (defined(TARGET_3DS))
	#include "xatomic\private\x_cpu_arm_3ds.h"
#else
	#error Unsupported CPU
#endif

#endif // __XMULTICORE_CPUINFO_H__
