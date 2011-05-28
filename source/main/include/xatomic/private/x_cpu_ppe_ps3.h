/**
 * @file xmulticore\private\x_cpu_ppe_ps3.h
 * PS3 specific part of the CPU library.
 * @see xmulticore\x_cpu.h
 */

// mfence ensures that tsc reads are properly serialized
// On Intel chips it's actually enough to just do lfence but
// that would require some conditional logic. 


static inline u64 __tsc(void)
{
	CellRtcTick tick;
	cellRtcGetCurrentTick(&tick);
	return (u64)tick.tick;
}
