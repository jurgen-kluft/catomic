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
		class aint32_t;
		class auint32_t;
		class aint64_t;
		class auint64_t;


		typedef		aint32_t				int32;
		typedef		auint32_t				uint32;

		typedef		aint64_t				int64;
		typedef		auint64_t				uint64;


		//-------------------------------------------------------------------------------------
		// atomic integer public base
		//-------------------------------------------------------------------------------------
		template<class T, class U>
		class integer_base
		{
		public:
			typedef			volatile T			vo_int;

		protected:
			vo_int			_data;

		public:
			static T		read(vo_int* p);
			static void		write(vo_int* p, T v);
			static bool		cas(vo_int* mem, T old, T n);
			static bool		cas(vo_int* mem, U ol, U oh, U nl, U nh);

			T				get() const;
			void			set(T v);

			T				swap(T i);

			void			incr();

			bool			testAndDecr();
			bool			decrAndTest();
			void			decr();

			void			add(T i);
			void			sub(T i);

			inline			integer_base()													{ set(0); }
			inline			integer_base(const integer_base& i)								{ set(i.get()); }
			inline			integer_base(T i)												{ set(i); }
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
#else
	#error Unsupported CPU
#endif

namespace xcore
{
	namespace atomic
	{

		//-------------------------------------------------------------------------------------
		// atomic integer base function implementations
		//-------------------------------------------------------------------------------------

		template<class T, class U>
		inline bool		integer_base<T,U>::cas(vo_int* mem, U ol, U oh, U nl, U nh)
		{
			T old = oh; old = old << (sizeof(T)*4); old = old | ol;
			T n   = nh; n   = n   << (sizeof(T)*4); n   = n   | nl;
			return integer_base<T,U>::cas(mem, old, n);
		}	

		template<class T, class U>
		inline T		integer_base<T,U>::get() const
		{
			return read((vo_int*)&_data); 
		}

		template<class T, class U>
		inline void		integer_base<T,U>::set(T v)
		{
			write(&_data, v); 
		}

		// Swap and return old value
		template<class T, class U>
		inline T		integer_base<T,U>::swap(T i)
		{
			// Automatically locks when doing this op with a memory operand.
			register T old;
			do
			{
				old = read((vo_int*)&_data);
			} while (cas(&_data, old, i) == false);
			return old;
		}

		// Increment
		template<class T, class U>
		inline void		integer_base<T,U>::incr()
		{
			register T old;
			do
			{
				old = read((vo_int*)&_data);
			} while (cas(&_data, old, old + 1) == false);
		}

		// Test for zero and decrement if non-zero
		template<class T, class U>
		inline bool		integer_base<T,U>::testAndDecr()
		{
			register T old;
			do
			{
				old = read((vo_int*)&_data);
				if (old == 0)
					return false;
			} while (cas(&_data, old, old - 1) == false);
			return true;
		}

		// Decrement and test for non zero
		template<class T, class U>
		inline bool		integer_base<T,U>::decrAndTest()
		{
			register T old;
			do
			{
				old = read((vo_int*)&_data);
			} while (cas(&_data, old, old - 1) == false);
			return (old-1) != 0;
		}

		// Decrement, return true if non-zero
		template<class T, class U>
		inline void		integer_base<T,U>::decr()
		{
			register T old;
			do
			{
				old = read((vo_int*)&_data);
			} while (cas(&_data, old, old - 1) == false);
		}

		// Add
		template<class T, class U>
		inline void		integer_base<T,U>::add(T i)
		{
			register T old;
			do
			{
				old = read((vo_int*)&_data);
			} while (cas(&_data, old, old + i) == false);
		}

		// Subtract
		template<class T, class U>
		inline void		integer_base<T,U>::sub(T i)
		{
			register T old;
			do
			{
				old = read((vo_int*)&_data);
			} while (cas(&_data, old, old - i) == false);
		}

	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_ATOMIC_H__
