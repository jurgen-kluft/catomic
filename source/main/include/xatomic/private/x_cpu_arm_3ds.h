/**
 * @file xatomic\private\x_cpu_arm_3ds.h
 * 3DS specific part of the CPU library.
 * @see xatomic\x_cpu.h
 */
#include <nn/os/os_Tick.h>

namespace xcore
{
	namespace cpu
	{
		static inline u64 __tsc(void)
		{
			u64 ticks = (u64)nnosTickGetSystemCurrent();
			return ticks;
		}
	}
}
