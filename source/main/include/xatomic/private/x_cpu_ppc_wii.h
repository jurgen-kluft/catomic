/**
 * @file xmulticore\private\x_cpu_ppc_wii.h
 * WII specific part of the CPU library.
 * @see xmulticore\x_cpu.h
 */


static inline u64 __tsc(void)
{
	OSTick ticks = OSGetTick();
	return (u64)ticks;
}
