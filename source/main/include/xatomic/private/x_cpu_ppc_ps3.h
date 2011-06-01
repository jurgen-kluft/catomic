/**
 * @file xatomic\private\x_cpu_ppe_ps3.h
 * PS3 specific part of the CPU library.
 * @see xatomic\x_cpu.h
 */

#include <cell/rtc.h>

namespace xcore
{
	namespace cpu
	{
		static inline u64 __tsc(void)
		{
			CellRtcTick tick;
			cellRtcGetCurrentTickUtc(&tick);
			return (u64)tick.tick;
		}
	}
}