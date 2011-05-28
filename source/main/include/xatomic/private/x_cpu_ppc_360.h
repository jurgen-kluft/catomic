/**
 * @file xmulticore\private\x_cpu_ppc_360.h
 * Xbox360 specific part of the CPU library.
 * @see xmulticore\x_cpu.h
 */


static inline u64 __tsc(void)
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter( &ticks );
	return (u64)ticks.QuadPart;
}
