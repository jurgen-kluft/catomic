/**
 * @file xatomic\private\atomic-x86.h
 * X86 (32 and 64 bit) specific implementation of the atomic operations.
 * @warning Do not include this header file directly. Include "xatomic\atomic.h" instead.
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

			inline static u32 sRead(volatile u32 *src)
			{
				return sInterlockedCompareExchange(src, 0, 0);
			}

			inline static void sWrite(volatile u32 *src, u32 v)
			{
				*src = v;
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

			inline static u64 sRead64(volatile u64 *src)
			{
				return sInterlockedCompareExchange64(src, 0, 0);
			}

			inline static void sWrite64(volatile u64 *src, u64 v)
			{
				*src = v;
			}

			#pragma warning(default:4035)
		}

		namespace cpu_interlocked = cpu_x86_32;

		//-------------------------------------------------------------------------------------
		// 32 bit signed integer
		//-------------------------------------------------------------------------------------
		class atom_s32 : public atom_int_type<s32>
		{
		public:
			inline			atom_s32() : atom_int_type<s32>(0)						{ }
			inline			atom_s32(s32 i) : atom_int_type<s32>(i)					{ }
		};

		static inline s32	read_s32(s32 volatile* p)
		{
			return *p;
		}

		static inline void	write_s32(s32 volatile* p, s32 v)
		{
			*p = v;
		}

		static inline bool	cas_s32(s32 volatile* mem, s32 old, s32 n)
		{
			s32 r = (s32)cpu_interlocked::sInterlockedCompareExchange((u32 volatile*)mem, (u32)n, (u32)old);
			return r == old;
		}

		static inline bool	cas_s32(volatile s32* mem, s16 ol, s16 oh, s16 nl, s16 nh)
		{
			u32 old = oh; old = old << 16; old = old | ol;
			u32 n = nh; n = n << 16; n = n | nl;
			s32 r = (s32)cpu_interlocked::sInterlockedCompareExchange((u32 volatile*)mem, (u32)n, (u32)old);
			return r == old;
		}

		//-------------------------------------------------------------------------------------
		// atomic integer base function implementations
		//-------------------------------------------------------------------------------------

		template <>
		inline s32		atom_int_type<s32>::get() const
		{
			return read_s32((s32 volatile*)&_data); 
		}

		template <>
		inline void		atom_int_type<s32>::set(s32 v)
		{
			write_s32(&_data, v); 
		}

		// Swap and return old value
		template <>
		inline s32		atom_int_type<s32>::swap(s32 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, i) == false);
			return old;
		}

		// Increment
		template <>
		inline void		atom_int_type<s32>::incr()
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old + 1) == false);
		}

		// Test for zero and decrement if non-zero
		template <>
		inline bool		atom_int_type<s32>::test_decr()
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
				if (old == 0)
					return false;
			} while (cas_s32(&_data, old, old - 1) == false);
			return true;
		}

		// Decrement and test for non zero
		template <>
		inline bool		atom_int_type<s32>::decr_test()
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		template <>
		inline void		atom_int_type<s32>::decr()
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old - 1) == false);
		}

		// Add
		template <>
		inline void		atom_int_type<s32>::add(s32 i)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old + i) == false);
		}

		// Subtract
		template <>
		inline void		atom_int_type<s32>::sub(s32 i)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old - i) == false);
		}

		template <>
		inline void		atom_int_type<s32>::bit_or(s32 i)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old | i) == false);
		}

		template <>
		inline void		atom_int_type<s32>::bit_xor(s32 i)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old ^ i) == false);
		}

		template <>
		inline void		atom_int_type<s32>::bit_and(s32 i)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old & i) == false);
		}

		template <>
		inline void		atom_int_type<s32>::bit_set(u32 n)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old | (1<<n)) == false);
		}

		template <>
		inline void		atom_int_type<s32>::bit_clr(u32 n)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old ^ (1<<n)) == false);
		}

		template <>
		inline void		atom_int_type<s32>::bit_chg(u32 n)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old ^ (1<<n)) == false);
		}

		template <>
		inline bool		atom_int_type<s32>::bit_test_set(u32 n)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old | (1<<n)) == false);
			return (old & (1<<n)) != 0;
		}

		template <>
		inline bool		atom_int_type<s32>::bit_test_clr(u32 n)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old & ~(1<<n)) == false);
			return (old & (1<<n)) != 0;
		}

		template <>
		inline bool		atom_int_type<s32>::bit_test_chg(u32 n)
		{
			register s32 old;
			do
			{
				old = read_s32((s32 volatile*)&_data);
			} while (cas_s32(&_data, old, old ^ (1<<n)) == false);
			return (old & (1<<n)) != 0;
		}

		//-------------------------------------------------------------------------------------
		// 32 bit unsigned integer
		//-------------------------------------------------------------------------------------
		class atom_u32 : public atom_int_type<u32>
		{
		public:
			inline			atom_u32() : atom_int_type<u32>(0)						{ }
			inline			atom_u32(u32 i) : atom_int_type<u32>(i)					{ }
		};

		static inline u32	read_u32(u32 volatile* p)
		{
			return *p;
		}

		static inline void	write_u32(u32 volatile* p, u32 v)
		{
			*p = v;
		}

		static inline bool	cas_u32(u32 volatile* mem, u32 old, u32 n)
		{
			u32 r = cpu_interlocked::sInterlockedCompareExchange(mem, n, old);
			return r == old;
		}

		static inline bool	cas_u32(volatile u32* mem, u16 ol, u16 oh, u16 nl, u16 nh)
		{
			u32 old = oh; old = old << 16; old = old | ol;
			u32 n = nh; n = n << 16; n = n | nl;
			u32 r = (u32)cpu_interlocked::sInterlockedCompareExchange((u32 volatile*)mem, (u32)n, (u32)old);
			return r == old;
		}

		template <>
		inline u32		atom_int_type<u32>::get() const
		{
			return read_u32((u32 volatile*)&_data); 
		}

		template <>
		inline void		atom_int_type<u32>::set(u32 v)
		{
			write_u32(&_data, v); 
		}

		// Swap and return old value
		template <>
		inline u32		atom_int_type<u32>::swap(u32 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, i) == false);
			return old;
		}

		// Increment
		template <>
		inline void		atom_int_type<u32>::incr()
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old + 1) == false);
		}

		// Test for zero and decrement if non-zero
		template <>
		inline bool		atom_int_type<u32>::test_decr()
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
				if (old == 0)
					return false;
			} while (cas_u32(&_data, old, old - 1) == false);
			return true;
		}

		// Decrement and test for non zero
		template <>
		inline bool		atom_int_type<u32>::decr_test()
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		template <>
		inline void		atom_int_type<u32>::decr()
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old - 1) == false);
		}

		// Add
		template <>
		inline void		atom_int_type<u32>::add(u32 i)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old + i) == false);
		}

		// Subtract
		template <>
		inline void		atom_int_type<u32>::sub(u32 i)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old - i) == false);
		}

		template <>
		inline void		atom_int_type<u32>::bit_or(u32 i)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old | i) == false);
		}

		template <>
		inline void		atom_int_type<u32>::bit_xor(u32 i)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old ^ i) == false);
		}

		template <>
		inline void		atom_int_type<u32>::bit_and(u32 i)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old & i) == false);
		}

		template <>
		inline void		atom_int_type<u32>::bit_set(u32 n)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old | (1<<n)) == false);
		}

		template <>
		inline void		atom_int_type<u32>::bit_clr(u32 n)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old ^ (1<<n)) == false);
		}

		template <>
		inline void		atom_int_type<u32>::bit_chg(u32 n)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old ^ (1<<n)) == false);
		}

		template <>
		inline bool		atom_int_type<u32>::bit_test_set(u32 n)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old | (1<<n)) == false);
			return (old & (1<<n)) != 0;
		}

		template <>
		inline bool		atom_int_type<u32>::bit_test_clr(u32 n)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old & ~(1<<n)) == false);
			return (old & (1<<n)) != 0;
		}

		template <>
		inline bool		atom_int_type<u32>::bit_test_chg(u32 n)
		{
			register u32 old;
			do
			{
				old = read_u32((u32 volatile*)&_data);
			} while (cas_u32(&_data, old, old ^ (1<<n)) == false);
			return (old & (1<<n)) != 0;
		}

		//-------------------------------------------------------------------------------------
		// 64 bit signed integer
		//-------------------------------------------------------------------------------------
		class atom_s64 : public atom_int_type<s64>
		{
		public:
			inline			atom_s64() : atom_int_type<s64>(0)						{ }
			inline			atom_s64(s64 i) : atom_int_type<s64>(i)					{ }
		};

		static inline s64	read_s64(s64 volatile* p)
		{
			return cpu_interlocked::sInterlockedCompareExchange64((volatile u64*)p, 0, 0);
		}

		static inline void	write_s64(s64 volatile* p, s64 v)
		{
			*p = v;
		}

		static inline bool	cas_s64(s64 volatile* mem, s64 old, s64 n)
		{
			s64 r = (s64)cpu_interlocked::sInterlockedCompareExchange64((u64 volatile*)mem, (u64)n, (u64)old);
			return r == old;
		}

		static inline bool	cas_s64(volatile s64* mem, s32 ol, s32 oh, s32 nl, s32 nh)
		{
			u64 old = oh; old = old << 32; old = old | ol;
			u64 n = nh; n = n << 32; n = n | nl;
			s64 r = (s64)cpu_interlocked::sInterlockedCompareExchange64((u64 volatile*)mem, (u64)n, (u64)old);
			return r == old;
		}

		template <>
		inline s64		atom_int_type<s64>::get() const
		{
			return read_s64((s64 volatile*)&_data); 
		}

		template <>
		inline void		atom_int_type<s64>::set(s64 v)
		{
			write_s64(&_data, v); 
		}

		// Swap and return old value
		template <>
		inline s64		atom_int_type<s64>::swap(s64 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, i) == false);
			return old;
		}

		// Increment
		template <>
		inline void		atom_int_type<s64>::incr()
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old + 1) == false);
		}

		// Test for zero and decrement if non-zero
		template <>
		inline bool		atom_int_type<s64>::test_decr()
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
				if (old == 0)
					return false;
			} while (cas_s64(&_data, old, old - 1) == false);
			return true;
		}

		// Decrement and test for non zero
		template <>
		inline bool		atom_int_type<s64>::decr_test()
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		template <>
		inline void		atom_int_type<s64>::decr()
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old - 1) == false);
		}

		// Add
		template <>
		inline void		atom_int_type<s64>::add(s64 i)
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old + i) == false);
		}

		// Subtract
		template <>
		inline void		atom_int_type<s64>::sub(s64 i)
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old - i) == false);
		}

		template <>
		inline void		atom_int_type<s64>::bit_or(s64 i)
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old | i) == false);
		}

		template <>
		inline void		atom_int_type<s64>::bit_xor(s64 i)
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old ^ i) == false);
		}

		template <>
		inline void		atom_int_type<s64>::bit_and(s64 i)
		{
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old & i) == false);
		}

		template <>
		inline void		atom_int_type<s64>::bit_set(u32 n)
		{
			s64 i = (1<<n);
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old | i) == false);
		}

		template <>
		inline void		atom_int_type<s64>::bit_clr(u32 n)
		{
			s64 i = (1<<n);
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old ^ i) == false);
		}

		template <>
		inline void		atom_int_type<s64>::bit_chg(u32 n)
		{
			s64 i = (1<<n);
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old ^ i) == false);
		}

		template <>
		inline bool		atom_int_type<s64>::bit_test_set(u32 n)
		{
			s64 i = (1<<n);
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old | i) == false);
			return (old & i) != 0;
		}

		template <>
		inline bool		atom_int_type<s64>::bit_test_clr(u32 n)
		{
			s64 i = (1<<n);
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old & ~i) == false);
			return (old & i) != 0;
		}

		template <>
		inline bool		atom_int_type<s64>::bit_test_chg(u32 n)
		{
			s64 i = (1<<n);
			register s64 old;
			do
			{
				old = read_s64((s64 volatile*)&_data);
			} while (cas_s64(&_data, old, old ^ i) == false);
			return (old & i) != 0;
		}

		//-------------------------------------------------------------------------------------
		// 64 bit unsigned integer
		//-------------------------------------------------------------------------------------
		class atom_u64 : public atom_int_type<u64>
		{
		public:
			inline			atom_u64() : atom_int_type<u64>(0)							{ }
			inline			atom_u64(u64 i) : atom_int_type<u64>(i)						{ }
		};

		static inline u64	read_u64(volatile u64* p)
		{
			return cpu_interlocked::sInterlockedCompareExchange64((volatile u64*)p, 0, 0);
		}

		static inline void	write_u64(volatile u64* p, u64 v)
		{
			*p = v;
		}

		static inline bool	cas_u64(volatile u64* mem, u64 old, u64 n)
		{
			u64 r = (u64)cpu_interlocked::sInterlockedCompareExchange64((u64 volatile*)mem, (u64)n, (u64)old);
			return r == old;
		}

		static inline bool	cas_u64(volatile u64* mem, u32 ol, u32 oh, u32 nl, u32 nh)
		{
			u64 old = oh; old = old << 32; old = old | ol;
			u64 n = nh; n = n << 32; n = n | nl;
			u64 r = (u64)cpu_interlocked::sInterlockedCompareExchange64((u64 volatile*)mem, (u64)n, (u64)old);
			return r == old;
		}

		template <>
		inline u64		atom_int_type<u64>::get() const
		{
			return read_u64((u64 volatile*)&_data); 
		}

		template <>
		inline void		atom_int_type<u64>::set(u64 v)
		{
			write_u64(&_data, v); 
		}

		// Swap and return old value
		template <>
		inline u64		atom_int_type<u64>::swap(u64 i)
		{
			// Automatically locks when doing this op with a memory operand.
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, i) == false);
			return old;
		}

		// Increment
		template <>
		inline void		atom_int_type<u64>::incr()
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old + 1) == false);
		}

		// Test for zero and decrement if non-zero
		template <>
		inline bool		atom_int_type<u64>::test_decr()
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
				if (old == 0)
					return false;
			} while (cas_u64(&_data, old, old - 1) == false);
			return true;
		}

		// Decrement and test for non zero
		template <>
		inline bool		atom_int_type<u64>::decr_test()
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		template <>
		inline void		atom_int_type<u64>::decr()
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old - 1) == false);
		}

		// Add
		template <>
		inline void		atom_int_type<u64>::add(u64 i)
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old + i) == false);
		}

		// Subtract
		template <>
		inline void		atom_int_type<u64>::sub(u64 i)
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old - i) == false);
		}

		template <>
		inline void		atom_int_type<u64>::bit_or(u64 i)
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old | i) == false);
		}

		template <>
		inline void		atom_int_type<u64>::bit_xor(u64 i)
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old ^ i) == false);
		}

		template <>
		inline void		atom_int_type<u64>::bit_and(u64 i)
		{
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old & i) == false);
		}

		template <>
		inline void		atom_int_type<u64>::bit_set(u32 n)
		{
			u64 i = (1<<n);
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old | i) == false);
		}

		template <>
		inline void		atom_int_type<u64>::bit_clr(u32 n)
		{
			u64 i = (1<<n);
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old ^ i) == false);
		}

		template <>
		inline void		atom_int_type<u64>::bit_chg(u32 n)
		{
			u64 i = (1<<n);
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old ^ i) == false);
		}

		template <>
		inline bool		atom_int_type<u64>::bit_test_set(u32 n)
		{
			u64 i = (1<<n);
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old | i) == false);
			return (old & i) != 0;
		}

		template <>
		inline bool		atom_int_type<u64>::bit_test_clr(u32 n)
		{
			u64 i = (1<<n);
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old & ~i) == false);
			return (old & i) != 0;
		}

		template <>
		inline bool		atom_int_type<u64>::bit_test_chg(u32 n)
		{
			u64 i = (1<<n);
			register u64 old;
			do
			{
				old = read_u64((u64 volatile*)&_data);
			} while (cas_u64(&_data, old, old ^ i) == false);
			return (old & i) != 0;
		}

	}
}
