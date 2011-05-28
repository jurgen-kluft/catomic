/**
 * @file xmulticore\private\x_atomic_ppc_360.h
 * Xbox360 PPC specific implementation of the atomic operations.
 * @warning Do not include this header file directly. Include "xmulticore\x_atomic.h" instead.
 */

#include <Xtl.h>

namespace xcore
{
	namespace atomic
	{
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
			s32 r = ::InterlockedCompareExchange((u32 volatile*)mem, (u32)n, (u32)old);
			return r == old;
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
			s64 r = ::InterlockedCompareExchange64((LONGLONG volatile*)mem, n, old);
			return r == old;
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
		inline void int32::incr()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old + 1) == false);
		}

		// Decrement, return true if non-zero
		inline bool int32::testAndDecr()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old + 1) == false);
			return old != 0;
		}

		// Decrement, return true if non-zero
		inline void	int32::decr()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old + 1) == false);
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

		static inline s64 ao_read64(s64 volatile* p)
		{
			return ::InterlockedCompareExchange64((volatile LONGLONG*)p, 0, 0);
		}

		// Swap and return old value
		inline s64 int64::swap(s64 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register s64 old;
			do
			{
				old = ao_read64(&_data);
			} while (cas64(&_data, old, i) == false);
			return old;
		}

		// Increment
		inline void int64::incr()
		{
			::InterlockedIncrement64((volatile LONGLONG*)&_data);
		}

		// Decrement, return true if non-zero
		inline bool int64::testAndDecr()
		{
			s64 old;
			do
			{
				old = ao_read64(&_data);
				if (old == 0)
					return false;
			} while (::InterlockedCompareExchange64((volatile LONGLONG*)&_data, old - 1, old) != old);

			return old != 0;
		}

		// Decrement, return true if non-zero
		inline void	int64::decr()
		{
			::InterlockedDecrement64((volatile LONGLONG*)&_data);
		}

		// Add
		inline void int64::add(s64 i)
		{
			s64 old;
			do
			{
				old = ao_read64(&_data);
			} while (::InterlockedCompareExchange64((volatile LONGLONG*)&_data, old + i, old) != old);
		}

		// Subtract
		inline void int64::sub(s64 i)
		{
			s64 old;
			do
			{
				old = ao_read64(&_data);
			} while (::InterlockedCompareExchange64((volatile LONGLONG*)&_data, old - i, old) != old);
		}
	}
}
