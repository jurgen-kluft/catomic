/**
 * @file xatomic\private\x_barrier_x86_win64.h
 * X86 (32 and 64 bit) specific barriers.
 * @warning do not include directly. @see xatomic\x_barrier.h
 */
#include <intrin.h>
namespace xcore
{
	/**
	 * We're using inline function here instead of #defines to avoid name space clashes.
	 */
	namespace barrier
	{
		/**
		 * Memory barriers
		 * This version requires SSE capable CPU.
		 */
		force_inline void barrier::comp()		{  }

		force_inline void barrier::memr()		{ _ReadBarrier(); }
		force_inline void barrier::memw()		{ _WriteBarrier(); }
		force_inline void barrier::memrw()		{ _ReadWriteBarrier(); }
	}
}