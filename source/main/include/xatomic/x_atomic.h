#ifndef __XMULTICORE_ATOMIC_H__
#define __XMULTICORE_ATOMIC_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

namespace xcore
{
	namespace atomic
	{
		//-------------------------------------------------------------------------------------
		// atomic library init/exit
		//-------------------------------------------------------------------------------------
		extern void		x_Init();
		extern void		x_Exit();

		//-------------------------------------------------------------------------------------
		// atomic integers, forward declare
		//-------------------------------------------------------------------------------------
		class atom_s32;
		class atom_u32;
		class atom_s64;
		class atom_u64;

		//-------------------------------------------------------------------------------------
		// atomic read32/write32, read64/write64, and cas32/cas64 functions
		//-------------------------------------------------------------------------------------
		static s32		read_s32(s32 volatile* p);
		static void		write_s32(s32 volatile* p, s32 v);
		static bool		cas_s32(s32 volatile* mem, s32 old, s32 n);
		static bool		cas_s32(s32 volatile* mem, s16 ol, s16 oh, s16 nl, s16 nh);

		static u32		read_u32(u32 volatile* p);
		static void		write_u32(u32 volatile* p, u32 v);
		static bool		cas_u32(u32 volatile* mem, u32 old, u32 n);
		static bool		cas_u32(u32 volatile* mem, u16 ol, u16 oh, u16 nl, u16 nh);

		static s64		read_s64(s64 volatile* p);
		static void		write_s64(s64 volatile* p, s64 v);
		static bool		cas_s64(s64 volatile* mem, s64 old, s64 n);
		static bool		cas_s64(s64 volatile* mem, s32 ol, s32 oh, s32 nl, s32 nh);

		static u64		read_u64(u64 volatile* p);
		static void		write_u64(u64 volatile* p, u64 v);
		static bool		cas_u64(u64 volatile* mem, u64 old, u64 n);
		static bool		cas_u64(u64 volatile* mem, u32 ol, u32 oh, u32 nl, u32 nh);


		//-------------------------------------------------------------------------------------
		// atomic integer public base
		//-------------------------------------------------------------------------------------
		template<class T>
		class atom_int_type
		{
		public:
			typedef			volatile T			vo_int;

		protected:
			vo_int			_data;

		public:
			T				get() const;
			void			set(T v);

			T				swap(T i);

			void			incr();
			void			decr();

			bool			test_decr();
			bool			decr_test();

			void			add(T i);
			void			sub(T i);

			void			bit_or(T i);
			void			bit_xor(T i);
			void			bit_and(T i);

			void			bit_set(u32 n);
			void			bit_clr(u32 n);
			void			bit_chg(u32 n);

			bool			bit_test_set(u32 n);
			bool			bit_test_clr(u32 n);
			bool			bit_test_chg(u32 n);

							atom_int_type();
							atom_int_type(const atom_int_type& i);
							atom_int_type(T i);
		};
	}
}

#if defined(TARGET_PC)
	#include "xatomic\private\x_atomic_x86_win32.h"
#elif defined(TARGET_360)
	#include "xatomic\private\x_atomic_ppc_360.h"
#elif defined(TARGET_PS3)
	#include "xatomic\private\x_atomic_ppc_ps3.h"
#elif defined(TARGET_WII)
	#include "xatomic\private\x_atomic_ppc_wii.h"
#elif defined(TARGET_3DS)
	#include "xatomic\private\x_atomic_arm_3ds.h"
#else
	#error Unsupported CPU
#endif



#endif // __XMULTICORE_ATOMIC_H__
