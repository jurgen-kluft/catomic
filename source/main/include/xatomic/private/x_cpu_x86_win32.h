/**
 * @file xmulticore\private\x_cpu_x86_win32.h
 * X86 (32 and 64 bit) specific parts of the CPU library.
 * @see xmulticore\x_cpu.h
 */

static inline u64 __tsc(void)
{
   u32 a, d;
   _asm 
   {
		push eax
		push edx

		rdtsc
		mov a, eax
		mov d, edx

		pop edx
		pop eax
   }
   return ((u64) a) | (((u64) d) << 32);
}
