/**
 * @file xmulticore\private\x_atomic_ppc_wii.h
 * Wii PPC 32 bit specific implementation of the atomic operations.
 * @warning Do not include this header file directly. Include "xmulticore\x_atomic.h" instead.
 */
#include <revolution/os.h>

namespace xcore
{
	namespace atomic
	{
		namespace cpu_ppc_wii
		{
			inline static u32 sInterlockedCompareExchange(volatile u32 *dest, u32 exchange, u32 comperand)
			{
				s32 wasEnabled = OSDisableInterrupts();
				u32 old = *dest;
				if (*dest == comperand)
					*dest = exchange;
				OSRestoreInterrupts(wasEnabled);
				return old;
			}

			inline static bool sInterlockedSetIfEqual(volatile u32 *dest, u32 exchange, u32 comperand)
			{
				s32 wasEnabled = OSDisableInterrupts();
				if (*dest == comperand)
				{
					*dest = exchange;
					OSRestoreInterrupts(wasEnabled);
					return true;
				}
				OSRestoreInterrupts(wasEnabled);
				return false;
			}

			inline static u64 sInterlockedCompareExchange64(volatile u64 *dest, u64 exchange, u64 comperand) 
			{
				s32 wasEnabled = OSDisableInterrupts();
				u64 old = *dest;
				if (*dest == comperand)
					*dest = exchange;
				OSRestoreInterrupts(wasEnabled);
				return old;
			}

			// Most common use of InterlockedCompareExchange
			// It's more efficient to use the z flag than to do another compare
			inline static bool sInterlockedSetIfEqual64(volatile u64 *dest, u64 exchange, u64 comperand) 
			{
				s32 wasEnabled = OSDisableInterrupts();
				if (*dest == comperand)
				{
					*dest = exchange;
					OSRestoreInterrupts(wasEnabled);
					return true;
				}
				OSRestoreInterrupts(wasEnabled);
				return false;
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
			return cpu_ppc_wii::sInterlockedSetIfEqual((volatile u32*)mem, old, n);
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
			return cpu_ppc_wii::sInterlockedSetIfEqual64((volatile u64*)mem, old, n);
		}

		static inline s64 read64(s64 volatile* p)
		{
			s32 wasEnabled = OSDisableInterrupts();
			s64 r = *p;
			OSRestoreInterrupts(wasEnabled);
			return r;
		}

		// Swap and return old value
		inline s32 int32::swap(s32 i)
		{
			s32 wasEnabled = OSDisableInterrupts();
			s32 old = _data;
			_data = i;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}

		// Increment
		inline void int32::incr()
		{
			s32 wasEnabled = OSDisableInterrupts();
			_data++;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}

		// Decrement, return true if non-zero
		inline bool int32::testAndDecr()
		{
			s32 wasEnabled = OSDisableInterrupts();
			s32 old = _data;
			if (old == 0)
				return false;
			--_data;
			OSRestoreInterrupts(wasEnabled);
			return old!=0;
		}

		// Decrement, return true if non-zero
		inline void	int32::decr()
		{
			s32 wasEnabled = OSDisableInterrupts();
			--_data;
			OSRestoreInterrupts(wasEnabled);
		}

		// Add
		inline void int32::add(s32 i)
		{
			s32 wasEnabled = OSDisableInterrupts();
			_data += i;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}

		// Subtract
		inline void int32::sub(s32 i)
		{
			s32 wasEnabled = OSDisableInterrupts();
			_data -= i;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}





		// Swap and return old value
		inline s64 int64::swap(s64 i)
		{
			s64 wasEnabled = OSDisableInterrupts();
			s64 old = _data;
			_data = i;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}

		// Increment
		inline void int64::incr()
		{
			s64 wasEnabled = OSDisableInterrupts();
			_data++;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}

		// Decrement, return true if non-zero
		inline bool int64::testAndDecr()
		{
			s64 wasEnabled = OSDisableInterrupts();
			s64 old = _data;
			if (old == 0)
				return false;
			--_data;
			OSRestoreInterrupts(wasEnabled);
			return old!=0;
		}

		// Decrement, return true if non-zero
		inline void	int64::decr()
		{
			s64 wasEnabled = OSDisableInterrupts();
			--_data;
			OSRestoreInterrupts(wasEnabled);
		}

		// Add
		inline void int64::add(s64 i)
		{
			s64 wasEnabled = OSDisableInterrupts();
			_data += i;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}

		// Subtract
		inline void int64::sub(s64 i)
		{
			s64 wasEnabled = OSDisableInterrupts();
			_data -= i;
			OSRestoreInterrupts(wasEnabled);
			return old;
		}
	}
}
