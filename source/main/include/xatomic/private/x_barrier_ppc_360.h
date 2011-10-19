/**
 * @file xatomic\private\x_barrier_ppc_360.h
 * PPC (64 bit) specific barriers for Xbox 360.
 * @warning do not include directly. @see xatomic\x_barrier.h
 */

namespace xcore
{
	// We're using inline function here instead of #defines to avoid name space clashes.
	namespace barrier
	{
		// Memory barriers
		force_inline void barrier::comp()		{  }

		force_inline void barrier::memr()		{ __lwsync(); }
		force_inline void barrier::memw()		{ __lwsync(); }
		force_inline void barrier::memrw()		{ __sync(); }
	}
}