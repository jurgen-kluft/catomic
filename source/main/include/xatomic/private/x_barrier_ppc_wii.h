/**
 * @file xatomic\private\x_barrier_ppc_wii.h
 * PPC (32 bit) specific barriers for WII.
 * @warning do not include directly. @see xatomic\x_barrier.h
 */

namespace xcore
{
	// We're using inline function here instead of #defines to avoid name space clashes.
	namespace barrier
	{
		// Memory barriers
		inline void barrier::comp()		{  }

		inline void barrier::memr()		{ }
		inline void barrier::memw()		{ }
		inline void barrier::memrw()	{ }
	}
}