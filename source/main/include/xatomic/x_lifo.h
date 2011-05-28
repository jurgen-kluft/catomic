#ifndef __XMULTICORE_LIFO_H__
#define __XMULTICORE_LIFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xmulticore\x_compiler.h"
#include "xmulticore\x_atomic.h"
#include "xmulticore\x_barrier.h"

namespace xcore
{
	namespace atomic
	{
		/*
		* LIFO implementation is based on the
		* "Lock-Free Techniques for Concurrent Access to Shared Objects"
		* paper by Dominique Fober, Yann Orlarey and Stephane Letz.
		* Published on September 30 2003.
		*/

		/**
		* Multi-reader, multi-writer lock-free LIFO.
		* Both push() and pop() are O(1) and have fairly low overhead.
		* Algorithm relies on the atomic CAS64 (64bit compare and swap) 
		* to guaranty atomicity and thread safety.
		* Singly linked list of 32bit indices (instead of pointers) is used to 
		* keep track of the pushed elements.
		* @see mfifo
		*/
		class lifo 
		{
		protected:
			union state
			{
				volatile u64 next_salt64;
				struct
				{
#ifdef X_BIG_ENDIAN
					volatile u32 next;
					volatile u32 salt;
#else
					volatile u32 salt;
					volatile u32 next;
#endif
				} next_salt32;
			};

			struct link 
			{
				volatile u32 next;
			};

			state		_head;
			link*		_chain;
			u32			_size;

		public:
			/**
			* Create empty lifo. It can be initialized later by calling init().
			*/
						lifo() : _chain(NULL), _size(0)							{ }

			/**
			* Create lifo.
			* @param size number of elements
			*/
						lifo(u32 size)											{ init(size); }

			/**
			* Destructor
			*/
						~lifo();

			/**
			* Complete initialization.
			*/
			bool		init(u32 size);

			/**
			* Reset lifo state.
			* @warning Not thread safe
			*/
			void		reset();

			/**
			* Fill lifo with a sequence of elements from 0 - size.
			* @warning Not thread safe
			*/
			void		fill();

			/**
			* Size of the lifo.
			* @return maximum number of elements
			*/
			u32			size() const											{ return _size; }

			/**
			* Number of unused elements.
			* @return approximate number of unused elements
			* @warning this method is slow and inefficient
			*/
			u32			room() const
			{
				// head.salt is incremented before each push and pop,
				// hence we cannot use it just like we do in the fifo.
				u32 i, n = 0;
				for (i=0; i<_size; i++)
					n += (_chain[i].next == ~0U);
				return n;
			}

			/**
			* Check if lifo is empty
			* @return true if empty
			*/
			bool		empty() const
			{
				return _head.next_salt32.next == _size;
			}

			/**
			* Push an element into the lifo.
			* @param[in] i index of the element
			*/
			bool		push(u32 i);

			/**
			* Pop the top element out of the lifo.
			* @param[out] index index of the returned element
			* @return false if the FIFO is empty, true otherwise
			*/
			bool		pop(u32 &i);

			/**
			* Inline version of @see push().
			* Most cases should use regular version.
			*/
			bool		ipush(u32 i);

			/**
			* Inline version of @see pop().
			* Most cases should use regular version.
			*/
			bool		ipop(u32 &i);

			// Emulates fifo::pop() interface.
			// Used in the unit-test
			bool		pop(u32 &i, u32 &r)
			{
				bool b = pop(i); r = i;
				return b;
			}
		};

		inline bool lifo::ipush(u32 i)
		{
			state h;

			if (i >= _size)
				return false;

			// Double push trap. Thread safe because the caller still
			// owns the element.
			if (_chain[i].next != ~0U)
				return false;

			// Spin until push is successful 
			do
			{
				h.next_salt64 = _head.next_salt64;

				_chain[i].next = h.next_salt32.next;
				// We need write barrier here to make sure that _chain[i].next
				// is visible on all CPUs before it's linked in.
				barrier::memw();
			} while (!cas64(&_head.next_salt64, h.next_salt32.next, h.next_salt32.salt, i, h.next_salt32.salt + 1));

			return true;
		}

		inline bool lifo::ipop(u32 &i)
		{
			u32 n;
			state h;

			// Spin until pop is successful
			do
			{
				h.next_salt64 = _head.next_salt64;

				// Empty ?
				if (h.next_salt32.next == _size)
					return false;

				n = _chain[h.next_salt32.next].next;
			} while (!cas64(&_head.next_salt64, h.next_salt32.next, h.next_salt32.salt, n, h.next_salt32.salt + 1));

			i = h.next_salt32.next;

			// Clear 'next' index so that push() could check for
			// double push. Thread safe here because caller now owns
			// the element.
			_chain[h.next_salt32.next].next = ~0U;

			return true;
		}
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_LIFO_H__
