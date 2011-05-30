/**
 * @file xatomic\private\x_cpu_ppc_360.h
 * Xbox360 specific part of the CPU library.
 * @see xatomic\x_cpu.h
 */
#include <Xtl.h>

namespace xcore
{
	namespace cpu
	{
		static inline u64 __tsc(void)
		{
			LARGE_INTEGER ticks;
			QueryPerformanceCounter( &ticks );
			return (u64)ticks.QuadPart;
		}
	}
}