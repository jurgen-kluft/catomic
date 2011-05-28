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
		/** 
		* Atomic 32 bit integer.
		*/
		class int32
		{
		private:
			typedef			volatile s32		vo_s32;

			vo_s32			_data;

		public:
			/**
			* Constructor.
			*/ 	
							int32(s32 val = 0) : _data(val)							{ }
							int32(const int32& other) : _data(other._data)			{ }

			/**
			* Get value of atomic integer. 
			* @return integer value
			*/
			inline s32		get() const												{ return _data; }

			/**
			* Set value of atomic integer. 
			*/
			inline void		set(s32 v) 												{ _data = v; }

			/**
			* Get and set value.
			* @param i desired value
			* @return integer value of the counter
			*/
			s32				swap(s32 i);

			/**
			* Increment atomic counter
			*/
			void			incr();

			/**
			* Test for zero and decrement if not zero.
			* @return returns true if result is not zero. 
			*/
			bool			testAndDecr();

			/**
			* Decrement and test for non zero.
			* @return returns true if result is not zero. 
			*/
			bool			decrAndTest();

			/**
			* Decrement atomic integer.
			* This is a bit cheaper than 'dec()' because it does not
			* check the result.
			*/
			void			decr();

			/**
			* Add to atomic integer.
			* @param i the amount to be added
			*/ 
			void			add(s32 i);

			/**
			* Subtract from atomic integer.
			* @param i the amount to subtract
			*/
			void			sub(s32 i);
		};

		/** 
		* Atomic 64 bit integer.
		*/
		class int64
		{
		private:
			typedef			volatile s64		vo_s64;

			vo_s64			_data;

		public:
			/**
			* Constructor.
			*/ 	
							int64(s64 val = 0) : _data(val)							{ }
							int64(const int64& other) : _data(other._data)			{ }

			/**
			* Get value of atomic integer. 
			* @return integer value
			*/
			inline s64		get() const												{ return _data; }

			/**
			* Set value of atomic integer. 
			*/
			inline void		set(s64 v)												{ _data = v; }

			/**
			* Get and set value.
			* @param i desired value
			* @return integer value of the counter
			*/
			s64				swap(s64 i);

			/**
			* Increment atomic counter
			*/
			void			incr();

			/**
			* Test for zero and decrement if not zero.
			* @return returns true if result is not zero. 
			*/
			bool			testAndDecr();

			/**
			* Decrement and test for non zero.
			* @return returns true if result is not zero. 
			*/
			bool			decrAndTest();

			/**
			* Decrement atomic integer.
			* This is a bit cheaper than 'dec()' because it does not
			* check the result.
			*/
			void			decr();

			/**
			* Add to atomic integer.
			* @param i the amount to be added
			*/ 
			void			add(s64 i);

			/**
			* Subtract from atomic integer.
			* @param i the amount to subtract
			*/
			void			sub(s64 i);

			void			bor(s64 i);
			void			band(s64 i);
			void			bxor(s64 i);
		};


	} // namespace atomic


} // namespace xcore


#if defined(TARGET_PC)
	#include "xmulticore\private\x_atomic_x86_win32.h"
#elif defined(TARGET_360)
	#include "xmulticore\private\x_atomic_ppc_360.h"
#elif defined(TARGET_PS3)
	#include "xmulticore\private\x_atomic_ppc_ps3.h"
#elif defined(TARGET_WII)
	#include "xmulticore\private\x_atomic_ppc_wii.h"
#else
	#error Unsupported CPU
#endif


#endif // __XMULTICORE_ATOMIC_H__
