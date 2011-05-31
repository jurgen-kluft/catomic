/**
 * @file xatomic\private\x_barrier_x86_win32.h
 * X86 (32 and 64 bit) specific barriers.
 * @warning do not include directly. @see xatomic\x_barrier.h
 */

namespace xcore
{
	// We're using inline function here instead of #defines to avoid name space clashes.
	namespace barrier
	{
		// Memory barriers
		// This version requires SSE capable CPU.
		force_inline void barrier::comp()		{ __asm { }; }

		force_inline void barrier::memr()		{ __asm { __asm lfence }; }
		force_inline void barrier::memw()		{ __asm { __asm sfence }; }
		force_inline void barrier::memrw()		{ __asm { __asm mfence }; }
	}
}