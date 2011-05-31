#ifndef __XMULTICORE_BITFIELD_BASE_H__
#define __XMULTICORE_BITFIELD_BASE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xatomic\x_atomic.h"
#include "xatomic\x_compiler.h"

namespace xcore
{
	namespace atomic
	{
		#if defined(TARGET_PC)
			#define X_WORDSIZE	64
		#elif defined(TARGET_PS3)
			#define X_WORDSIZE	64
		#elif defined(TARGET_360)
			#define X_WORDSIZE	64
		#elif defined(TARGET_WII)
			#define X_WORDSIZE	64
		#elif defined(TARGET_3DS)
			#define X_WORDSIZE	64
		#else
			#error Unsupported CPU
		#endif

		/**
		* Internal bitfield functions
		*/
		class __bitfield
		{
		public:
		#if X_WORDSIZE == 64
			enum
			{
				SHIFT   = 6,
				MASK    = 63,
			};
			typedef		s64		value_type;
		#else
			enum
			{
				SHIFT   = 5,
				MASK    = 31,
			};
			typedef		s32		value_type;
		#endif

			enum
			{
				NCHUNKS = (64 + MASK) >> SHIFT
			};
		private:
			// Platform specific code provides:
			// 	void set(u32 n, u64 *addr);
			// 	void clear(u32 n, u64 *addr);
			// 	bool test(u32 n, u64 *addr) const;
			// 	bool atac(u32 n, volatile u64 *addr);
			// 	bool tac(u32 n, u64 *addr);
			// 	bool atas(u32 n, volatile u64 *addr);
			// 	bool tas(u32 n, u64 *addr);
			// 	u64 ffs(u64 d);
			// 	u64 ffz(u64 d);

			#if defined(TARGET_PC)
				#include "xatomic\private\x_bitfield_x86_win32.h"
			#elif defined(TARGET_PS3)
				#include "xatomic\private\x_bitfield_ppc_ps3.h"
			#elif defined(TARGET_360)
				#include "xatomic\private\x_bitfield_ppc_360.h"
			#elif defined(TARGET_WII)
				#include "xatomic\private\x_bitfield_ppc_wii.h"
			#else
				#error Unsupported CPU
			#endif

		public:

			// Set a bit
			static inline void set(u32 n, value_type *addr)							{ __set(n, addr); }
			// Clear a bit
			static inline void clear(u32 n, value_type *addr)						{ __clear(n, addr); }
			// Test a bit
			static inline bool test(u32 n, value_type *addr)						{ return __test(n, addr); }
			// Test and set
			static inline bool tas(u32 n, value_type *addr)							{ return __tas(n, addr); }
			// Test and clear
			static inline bool tac(u32 n, value_type *addr)							{ return __tac(n, addr); }
			// Atomic test and set
			static inline bool atas(u32 n, volatile value_type *addr)				{ return __atas(n, addr); }
			// Atomic test and clear
			static inline bool atac(u32 n, volatile value_type *addr)				{ return __atac(n, addr); }
			// Find first non-zero bit
			static inline value_type ffs(value_type d)								{ return __ffs(d); }
			// Find first zero bit
			static inline value_type ffz(value_type d)								{ return __ffz(d); }

			/*
			* Generic version of the popular parallel bit counting 
			* algorithm described in "Hacker's Delight" book.
			*/
			#define __W_SHIFT(c)   (0x1ULL << (c))
			#define __W_MASK(c)    (~0UL / (__W_SHIFT(__W_SHIFT(c)) + 1UL))
			#define __W_COUNT(x,c) ((x) & __W_MASK(c)) + (((x) >> (__W_SHIFT(c))) & __W_MASK(c))

			static u32 weight(u8 n)
			{
				n = __W_COUNT(n, 0);
				n = __W_COUNT(n, 1);
				n = __W_COUNT(n, 2);
				return n;
			}
			static u32 weight(s8 n)	{ return weight((u8)n); }

			static u32 weight(u16 n)
			{
				n = __W_COUNT(n, 0);
				n = __W_COUNT(n, 1);
				n = __W_COUNT(n, 2);
				n = __W_COUNT(n, 3);
				return n;
			}
			static u32 weight(s16 n)	{ return weight((u16)n); }

			static u32 weight(u32 n)
			{
				n = __W_COUNT(n, 0);
				n = __W_COUNT(n, 1);
				n = __W_COUNT(n, 2);
				n = __W_COUNT(n, 3);
				n = __W_COUNT(n, 4);
				return n;
			}
			static u32 weight(s32 n)	{ return weight((u32)n); }

			static u32 weight(u64 n)
			{
				n = __W_COUNT(n, 0);
				n = __W_COUNT(n, 1);
				n = __W_COUNT(n, 2);
				n = __W_COUNT(n, 3);
				n = __W_COUNT(n, 4);
				n = __W_COUNT(n, 5);
				return (u32)n;
			}
			static u32 weight(s64 n)	{ return weight((u64)n); }

			/**
			* Count non-zero bits in a bitfield.
			* @return number of non-zero bits
			*/
			static u32 count(const value_type *chunk, u32 nchunks);

			/**
			* Find first non-zero bit
			* @return index of the non-zero bit or -1 if not found.
			*/
			static s32 ffs(const value_type *chunk, u32 nchunks);

			/**
			* Find first non-zero bit starting at offset
			* @param offset bit offset to start with
			* @return index of the non-zero bit or -1 if not found.
			*/
			static s32 ffso(const value_type *chunk, u32 nchunks, u32 offset);

			/**
			* Find first zero bit
			* @return index of the non-zero bit or -1 if not found.
			*/
			static s32 ffz(const value_type *chunk, u32 nchunks);

			/**
			* Find first zero bit starting at offset
			* @param offset bit offset to start with
			* @return index of the non-zero bit or -1 if not found.
			*/
			static s32 ffzo(const value_type *chunk, u32 nchunks, u32 offset);

			/**
			* Copy bitfield into this bitfield.
			* @param b bitfield to copy from
			*/ 
			static void copy(value_type *to, const value_type *from, u32 nchunks);

			/**
			* Bitwise OR.
			* @param b bitfield to OR with
			*/ 
			static void bor(value_type *to, const value_type *from, u32 nchunks);

			/**
			* Bitwise XOR.
			* @param b bitfield to OR with
			*/ 
			static void bxor(value_type *to, const value_type *from, u32 nchunks);

			/**
			* Bitwise AND.
			* @param b bitfield to AND with
			*/ 
			static void band(value_type *to, const value_type *from, u32 nchunks);

			/**
			* Copy an inverted bitfield into this bitfield.
			* @param b bitfield to copy
			*/ 
			static void invert(value_type *to, const value_type *from, u32 nchunks);

			/**
			* Zero out entire bitfield.
			*/ 
			static void zero(value_type *chunk, u32 nchunks);

			/**
			* Fill entire bitfield with ones.
			*/ 
			static void fill(value_type *chunk, u32 nchunks);
		}; // class bitfield
	} // namespace atomic
} // namespace xcore


#endif // __XMULTICORE_BITFIELD_BASE_H__
