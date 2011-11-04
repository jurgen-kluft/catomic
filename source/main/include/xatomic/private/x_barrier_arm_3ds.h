/**
 * @file xatomic\private\x_barrier_ppc_wii.h
 * PPC (32 bit) specific barriers for WII.
 * @warning do not include directly. @see xatomic\x_barrier.h
 */
#include <nn/os.h>

namespace xcore
{
	/**
	 * We're using inline function here instead of #defines to avoid name space clashes.
	 */
	namespace barrier
	{
		/**
		 * Memory barriers
		 */
		inline void barrier::comp()		{ nn::os::ARM::InstructionMemoryBarrier(); }

		inline void barrier::memr()		{ nn::os::ARM::DataSynchronizationBarrier(); }
		inline void barrier::memw()		{ nn::os::ARM::DataSynchronizationBarrier(); }
		inline void barrier::memrw()	{ nn::os::ARM::DataSynchronizationBarrier(); }
	}
}
