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

		//-------------------------------------------------------------------------------------
		// 32 bit signed integer
		//-------------------------------------------------------------------------------------
		class aint32_t : public integer_base<s32, s16>
		{
		public:
			inline			aint32_t() : integer_base<s32,s16>(0)						{ }
			inline			aint32_t(s32 i) : integer_base<s32,s16>(i)					{ }
		};

		template<>
		inline s32			integer_base<s32,s16>::read(vo_int* p)
		{
			return *p;
		}

		template<>
		inline void			integer_base<s32,s16>::write(vo_int* p, s32 v)
		{
			*p = v;
		}

		template<>
		inline bool			integer_base<s32,s16>::cas(vo_int* mem, s32 old, s32 n)
		{
			s32 r = (s32)cpu_x86_32::sInterlockedCompareExchange((u32 volatile*)mem, (u32)n, (u32)old);
			return r == old;
		}


		//-------------------------------------------------------------------------------------
		// 32 bit unsigned integer
		//-------------------------------------------------------------------------------------
		class auint32_t : public integer_base<u32,u16>
		{
		public:
			inline			auint32_t() : integer_base<u32,u16>(0)						{ }
			inline			auint32_t(u32 i) : integer_base<u32,u16>(i)					{ }
		};

		template<>
		inline u32			integer_base<u32,u16>::read(vo_int* p)
		{
			return *p;
		}

		template<>
		inline void			integer_base<u32,u16>::write(vo_int* p, u32 v)
		{
			*p = v;
		}

		template<>
		inline bool			integer_base<u32,u16>::cas(vo_int* mem, u32 old, u32 n)
		{
			u32 r = cpu_x86_32::sInterlockedCompareExchange(mem, n, old);
			return r == old;
		}


		//-------------------------------------------------------------------------------------
		// 64 bit signed integer
		//-------------------------------------------------------------------------------------
		class aint64_t : public integer_base<s64,s32>
		{
		public:
			inline			aint64_t() : integer_base<s64,s32>(0)						{ }
			inline			aint64_t(s64 i) : integer_base<s64,s32>(i)					{ }
		};

		template<>
		inline s64			integer_base<s64,s32>::read(vo_int* p)
		{
			return cpu_x86_32::sInterlockedCompareExchange64((volatile u64*)p, 0, 0);
		}

		template<>
		inline void			integer_base<s64,s32>::write(vo_int* p, s64 v)
		{
			*p = v;
		}

		template<>
		inline bool			integer_base<s64,s32>::cas(vo_int* mem, s64 old, s64 n)
		{
			s64 r = (s64)cpu_x86_32::sInterlockedCompareExchange64((u64 volatile*)mem, (u64)n, (u64)old);
			return r == old;
		}


		//-------------------------------------------------------------------------------------
		// 64 bit unsigned integer
		//-------------------------------------------------------------------------------------
		class auint64_t : public integer_base<u64,u32>
		{
		public:
			inline			auint64_t() : integer_base<u64,u32>(0)						{ }
			inline			auint64_t(u64 i) : integer_base<u64,u32>(i)					{ }
		};

		template<>
		inline u64			integer_base<u64,u32>::read(vo_int* p)
		{
			return cpu_x86_32::sInterlockedCompareExchange64((volatile u64*)p, 0, 0);
		}

		template<>
		inline void			integer_base<u64,u32>::write(vo_int* p, u64 v)
		{
			*p = v;
		}

		template<>
		inline bool			integer_base<u64,u32>::cas(vo_int* mem, u64 old, u64 n)
		{
			u64 r = (u64)cpu_x86_32::sInterlockedCompareExchange64((u64 volatile*)mem, (u64)n, (u64)old);
			return r == old;
		}


	}
}
