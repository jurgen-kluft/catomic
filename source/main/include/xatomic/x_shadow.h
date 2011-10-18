#ifndef __XMULTICORE_SHADOW_H__
#define __XMULTICORE_SHADOW_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_allocator.h"

#include "xatomic\private\x_allocator.h"
#include "xatomic\x_barrier.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Generic method for sharing data object in a lock-less fashion.
		* This technique is based on shadow buffers to handle the case when 
		* writer is preempted in the middle of the update.
		* Thread safe for multi-reader, single-writer case.
		*/
		template <class T>
		class shadow 
		{
		private:
			typedef volatile u32 vo_u32;

			vo_u32		_wcount;
			T			_val[2];

		public:
			XCORE_CLASS_NEW_DELETE(sGetAllocator, 4)

			shadow() : _wcount(0)
			{
				_val[0] = 0;
				_val[1] = 0;
			}

			shadow(const T& v)
			{
				_val[0] = v;
				_val[1] = v;
				_wcount = 0;
			}

			shadow(const shadow<T> &s)
			{
				s.read(_val[0]);
				s.read(_val[1]);
				_wcount = 0;
			}

			shadow<T>&	operator=(const shadow<T> &s)
			{
				T v;
				s.read(v);
				write(v);
				return *this;
			}

			shadow<T>&	operator=(const T& v)
			{
				write(v);
				return *this;
			}

			/**
			* Read current value.
			* Ensures consistent results with respect to updates.
			* Reader's interface.
			* @param[out] v pointer read the value into.
			*/
			void		read(T *v) const
			{
				u32 c0, c1;

				// Spin until the read is consistent
				// Barrier ensures that the copy is complete when we read
				// the counter second time.
				do 
				{
					c0 = _wcount;
					*v = _val[c0 & 1];
					barrier::memw();
					c1 = _wcount;
				} while (c1 != c0);
			}

			void		read(T &v) const											{ read(&v); }

			/**
			* Update current value.
			* Writer's interface.
			* @param[in] v pointer to the value to update to 
			*/
			void		write(const T *v)
			{
				u32 c0 = _wcount;
				// Barrier ensures that the copy is complete when we increment
				// the counter.
				_val[!(c0 & 1)] = *v;
				barrier::memw();
				_wcount++;
			}

			void		write(const T &v)													{ write(&v); }
		};
	}
} // namespace xcore

#endif // __XMULTICORE_SHADOW_H__
