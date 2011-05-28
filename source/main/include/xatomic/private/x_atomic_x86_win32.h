/**
 * @file xmulticore\private\atomic-x86.h
 * X86 (32 and 64 bit) specific implementation of the atomic operations.
 * @warning Do not include this header file directly. Include "xmulticore\atomic.h" instead.
 */

// Windows includes first
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMB
#define NOKANJI
#include <windows.h>

namespace xcore
{
	namespace atomic
	{
		// 32 and 64 bit interlocked compare and exchange functions for a 32 bit cpu
		namespace cpu_x86_32
		{
			// Disable the no return value warning, because the assembly language
			// routines load the appropriate registers directly
			#pragma warning(disable:4035)

			inline static u32 sInterlockedCompareExchange(volatile u32 *dest, u32 exchange, u32 comperand)
			{
				// value returned in eax
				__asm 
				{
					mov eax,comperand
					mov ecx,exchange
					mov edx,dest
					lock cmpxchg [edx],ecx
				}
			}

			inline static bool sInterlockedSetIfEqual(volatile u32 *dest, u32 exchange, u32 comperand)
			{
				// value returned in eax
				__asm 
				{
					mov eax,comperand
					mov ecx,exchange
					mov edx,dest
					lock cmpxchg [edx],ecx
					mov eax,0;
					setz al;
				}
			}

			inline static u64 sInterlockedCompareExchange64(volatile u64 *dest, u64 exchange, u64 comperand) 
			{
				// value returned in eax::edx
				__asm 
				{
					lea esi,comperand;
					lea edi,exchange;

					mov eax,[esi];
					mov edx,4[esi];
					mov ebx,[edi];
					mov ecx,4[edi];
					mov esi,dest;
					// lock CMPXCHG8B [esi] is equivalent to the following except that it's atomic:
					// ZeroFlag = (edx:eax == *esi);
					// if (ZeroFlag) *esi = ecx:ebx;
					// else edx:eax = *esi;
					lock CMPXCHG8B [esi];
				}
			}

			// Most common use of InterlockedCompareExchange
			// It's more efficient to use the z flag than to do another compare
			inline static bool sInterlockedSetIfEqual64(volatile u64 *dest, u64 exchange, u64 comperand) 
			{
				//value returned in eax
				__asm 
				{
					lea esi,comperand;
					lea edi,exchange;

					mov eax,[esi];
					mov edx,4[esi];
					mov ebx,[edi];
					mov ecx,4[edi];
					mov esi,dest;
					// lock CMPXCHG8B [esi] is equivalent to the following except that it's atomic:
					// ZeroFlag = (edx:eax == *esi);
					// if (ZeroFlag) *esi = ecx:ebx;
					// else edx:eax = *esi;
					lock CMPXCHG8B [esi];			
					mov eax,0;
					setz al;
				}
			}
			#pragma warning(default:4035)
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
			s32 r = cpu_x86_32::sInterlockedCompareExchange((u32 volatile*)mem, (u32)n, (u32)old);
			return r == old;
		}

		static inline bool cas32(volatile u32 *mem, u32 old, u32 n)
		{
			u32 r = cpu_x86_32::sInterlockedCompareExchange(mem, n, old);
			return r == old;
		}

		/**
		 * 64-bit atomic CAS (Compare And Swap).
		 * Compare mem with old if equal update it with new.
		 * @param mem pointer to memory that needs to be updated
		 * @param old old value
		 * @param n new value
		 * @return non zero if update was successful
		 */
		static inline bool cas64(volatile s64 *mem, s64 old, s64 n)
		{
			s64 r = cpu_x86_32::sInterlockedCompareExchange64((u64 volatile*)mem, n, old);
			return r == old;
		}

		static inline bool cas64(volatile s64 *mem, s32 oh, s32 ol, s32 nh, s32 nl)
		{
			s64 old = oh; old = old << 32; old = old | ol;
			s64 n   = nh; n   = n   << 32; n   = n   | nl;
			return cas64(mem, old, n);
		}

		static inline bool cas64(volatile u64 *mem, u64 old, u64 n)
		{
			u64 r = cpu_x86_32::sInterlockedCompareExchange64((u64 volatile*)mem, (s64)n, (s64)old);
			return r == old;
		}

		static inline bool cas64(volatile u64 *mem, u32 oh, u32 ol, u32 nh, u32 nl)
		{
			u64 old = oh; old = old << 32; old = old | ol;
			u64 n   = nh; n   = n   << 32; n   = n   | nl;
			return cas64((volatile u64*)mem, old, n);
		}

		static inline s64 read64(s64 volatile* p)
		{
			return cpu_x86_32::sInterlockedCompareExchange64((volatile u64*)p, 0, 0);
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

		// Test for zero and decrement if non-zero
		inline bool int32::testAndDecr()
		{
			register s32 old;
			do
			{
				old = _data;
				if (old == 0)
					return false;
			} while (cas32(&_data, old, old - 1) == false);
			return true;
		}

		// Decrement and test for non zero
		inline bool int32::decrAndTest()
		{
			register s32 old;
			do
			{
				old = _data;
			} while (cas32(&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		inline void	int32::decr()
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
				old = read64(&_data);
			} while (cas64(&_data, i, old) == false);
			return old;
		}

		// Increment
		inline void int64::incr()
		{
			s64 old;
			do
			{
				old = read64(&_data);
			} while (cas64((volatile LONGLONG*)&_data, old, old + 1) == false);
		}

		// Decrement, return true if non-zero
		inline bool int64::testAndDecr()
		{
			s64 old;
			do
			{
				old = read64(&_data);
				if (old == 0)
					return false;
			} while (cas64((volatile LONGLONG*)&_data, old, old - 1) == false);
			return true;
		}

		// Decrement, return true if non-zero
		inline bool int64::decrAndTest()
		{
			s64 old;
			do
			{
				old = read64(&_data);
			} while (cas64((volatile LONGLONG*)&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		inline void	int64::decr()
		{
			s64 old;
			do
			{
				old = read64(&_data);
			} while (cas64((volatile LONGLONG*)&_data, old, old - 1) == false);
		}

		// Add
		inline void int64::add(s64 i)
		{
			s64 old;
			do
			{
				old = read64(&_data);
			} while (cas64((volatile LONGLONG*)&_data, old, old + i) == false);
		}

		// Subtract
		inline void int64::sub(s64 i)
		{
			s64 old;
			do
			{
				old = read64(&_data);
			} while (cas64((volatile LONGLONG*)&_data, old, old - i) == false);
		}
	}
}
