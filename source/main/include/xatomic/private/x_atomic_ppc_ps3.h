/**
 * @file xmulticore\private\x_atomic_ppc_ps3.h
 * PS3 (32 and 64 bit) specific implementation of the atomic operations.
 * @warning Do not include this header file directly. Include "xmulticore\x_atomic.h" instead.
 */
#include <cell/atomic.h>

namespace xcore
{
	namespace atomic
	{
		namespace cpu_ppc_ps3
		{
			inline static u32 sInterlockedCompareExchange(volatile u32 *dest, u32 exchange, u32 comperand)
			{
				uint32_t r = cellAtomicCompareAndSwap32((uint32_t volatile*)dest, (uint32_t)exchange, (uint32_t)comperand);
				return r;
			}

			inline static bool sInterlockedSetIfEqual(volatile u32 *dest, u32 exchange, u32 comperand)
			{
				uint32_t r = cellAtomicCompareAndSwap32((uint32_t volatile*)dest, (uint32_t)exchange, (uint32_t)comperand) == comperand;
				return r == old;
			}

			inline static u64 sInterlockedCompareExchange64(volatile u64 *dest, u64 exchange, u64 comperand) 
			{
				uint64_t r = cellAtomicCompareAndSwap64((uint64_t volatile*)dest, (uint64_t)exchange, (uint64_t)comperand);
				return r;
			}

			// Most common use of InterlockedCompareExchange
			// It's more efficient to use the z flag than to do another compare
			inline static bool sInterlockedSetIfEqual64(volatile u64 *dest, u64 exchange, u64 comperand) 
			{
				uint64_t r = cellAtomicCompareAndSwap64((uint64_t volatile*)dest, (uint64_t)exchange, (uint64_t)comperand);
				return r == comperand;
			}
		}

		/**
		 * 32-bit atomic CAS (Compare And Swap).
		 * Compare mem with old if equal update it with _new.
		 * @param mem pointer to memory that needs to be updated
		 * @param old old value
		 * @param n new value
		 * @return non zero if update was successful
		 */
		static inline bool cas32(volatile s32 *mem, s32 old, s32 n)
		{
			return cpu_ppc_ps3::sInterlockedSetIfEqual(mem, n, old);
		}

		/**
		 * 64-bit atomic CAS (Compare And Swap).
		 * Compare mem with old (low + high) if equal update it with new (low + high).
		 * @param mem pointer to memory that needs to be updated
		 * @param ol old value (low)
		 * @param ol old value (high)
		 * @param nl new value (low)
		 * @param nh new value (high)
		 * @return non zero if update was successful
		 */
		static inline bool cas64(volatile s64 *mem, s64 old, s64 n)
		{
			return cpu_ppc_ps3::sInterlockedSetIfEqual64(mem, n, old);
		}

		static inline s64 read64(s64 volatile* p)
		{
			return cpu_ppc_ps3::sInterlockedCompareExchange64((volatile u64*)p, 0, 0);
		}

		// Swap and return old value
		inline s32 int32::swap(s32 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, i) == false);
			return old;
		}

		// Increment
		inline void int32::inc()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old + 1) == false);
			return old != 0;
		}

		// Decrement, return true if non-zero
		inline bool int32::decAndTest()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old - 1) == false);
			return old != 0;
		}

		// Decrement, return true if non-zero
		inline void	int32::dec()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old - 1) == false);
		}

		// Add
		inline void int32::add(s32 i)
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old + i) == false);
		}

		// Subtract
		inline void int32::sub(s32 i)
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old - i) == false);
		}



		// Swap and return old value
		inline s64 int64::swap(s64 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register s64 old;
			do
			{
				old = read64(_data);
			} while (cas64(&_data, old, i) == false);
			return old;
		}

		// Increment
		inline void int64::inc()
		{
			register s64 old;
			do
			{
				old = read64(_data);
			} while (cas64(&_data, old, old + 1) == false);
		}

		// Decrement, return true if non-zero
		inline bool int64::decAndTest()
		{
			register s64 old;
			do
			{
				old = read64(_data);
			} while (cas64(&_data, old, old - 1) == false);
			return old != 0;
		}

		// Decrement, return true if non-zero
		inline void	int64::dec()
		{
			register s64 old;
			do
			{
				old = read64(_data);
			} while (cas64(&_data, old, old - 1) == false);
		}

		// Add
		inline void int64::add(s64 i)
		{
			register s64 old;
			do
			{
				old = read64(_data);
			} while (cas64(&_data, old, old + i) == false);
		}

		// Subtract
		inline void int64::sub(s64 i)
		{
			register s64 old;
			do
			{
				old = read64(_data);
			} while (cas64(&_data, old, old - i) == false);
		}
	}
}
