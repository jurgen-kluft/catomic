#ifndef __XMULTICORE_FIFO_H__
#define __XMULTICORE_FIFO_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xatomic\x_compiler.h"
#include "xatomic\x_atomic.h"
#include "xatomic\x_barrier.h"

namespace xcore
{
	namespace atomic
	{
		/*
		* FIFO implementation is based on the
		* "Lock-Free Techniques for Concurrent Access to Shared Objects"
		* paper by Dominique Fober, Yann Orlarey and Stephane Letz.
		* Published on September 30 2003.
		*/

		/**
		* Multi-reader, multi-writer lock-free FIFO.
		* Both push() and pop() are O(1) and have fairly low overhead.
		* Algorithm relies on the atomic CAS64 (64bit compare and swap) 
		* to guaranty atomicity and thread safety.
		* Singly linked list of 32bit indices (instead of pointers) is used to 
		* keep track of the pushed elements.
		* @see mfifo
		*/
		class fifo
		{
		protected:
			union state 
			{
				volatile u64 next_salt64;
				struct
				{
#ifdef X_LITTLE_ENDIAN
					volatile u32 next;		// low u32, little endian will swap into the low word of a u64
					volatile u32 salt;		// high u32
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

			enum
			{
				UNUSED = ~0U,
				LAST   = UNUSED - 1,
			};

			state		_head;
			state		_tail;
			link*		_chain;
			u32			_size;

		public:
			/**
			* Create empty lifo. It can be initialized lated by calling init().
			*/
						fifo() : _chain(NULL), _size(0)							{ }

			/**
			* Create lifo.
			* @param size number of elements
			*/
						fifo(u32 size)											{ init(size); }

			/**
			* Destroy lifo.
			*/
						~fifo();

			/**
			* Complete initialization.
			*/
			bool		init(u32 size);

			/**
			* Size of the fifo.
			* @return maximum number of elements
			*/
			u32			size() const											{ return _size; }

			/**
			* Number of unused elements.
			* @return approximate number of unused elements
			*/
			u32			room() const
			{
				// head.salt is incremented for each pop()
				// tail.salt is incremented for each push()
				u32 used = (_tail.next_salt32.salt - _head.next_salt32.salt);
				if (used > _size)
					return _size;
				return _size - used;
			}

			/**
			* Check if fifo is empty
			* @return true if empty
			*/
			bool		empty() const
			{
				u32 h = _head.next_salt32.next;
				u32 t = _tail.next_salt32.next;
				u32 n = _chain[h].next;
				return (t == h && n == LAST);
			}

			/**
			* Reset fifo state.
			* @warning not thread safe
			* @param d index of the dummy node
			*/
			void		reset(u32 d = 0);

			/**
			* Fill fifo with a sequence of elements [0 - size];
			* @warning not thread safe
			*/
			void		fill();

			/**
			* Push an element into the fifo.
			* @param[in] i index of the element
			* @return false if push failed, true otherwise
			*/
			bool		push(u32 i);

			/**
			* Pop the top element out of the fifo.
			* @param[out] i index of the returned element
			* @param[out] r index of the element that can be reused
			* @return false if the FIFO is empty, true otherwise
			*/
			bool		pop(u32 &i, u32 &r);

			/**
			* Inline version of the @see push().
			* Most people should use regular version.
			*/
			bool		ipush(u32 i);

			/**
			* Inline version of the @see pop().
			* Most people should use regular version.
			*/
			bool		ipop(u32 &i, u32 &r);
		};

		inline bool fifo::ipush(u32 i)
		{
			u32 n;
			state t;

			if (i > _size)
				return false;

			// Double push trap. Thread safe because the caller still
			// owns the element.
			if (_chain[i].next != UNUSED)
				return false;

			_chain[i].next = LAST;
			barrier::memw();

			// Loop until push is successful
			while (1)
			{
				t.next_salt64 = _tail.next_salt64;
				n             = _chain[t.next_salt32.next].next;
				if (n == LAST) 
				{
					// Try to link this element to the tail element
					if (cas_u32((xcore::u32 volatile*)&_chain[t.next_salt32.next].next, LAST, i))
						break;
				} 
				else
				{
					// Tail element was not the last, try to fix it up.
					// This is done to take care of the races were another 
					// writer got preempted before completing the push.
					cas_u64(&_tail.next_salt64, t.next_salt32.next, t.next_salt32.salt, n, t.next_salt32.salt + 1);
				}
			}

			// Complete the push.
			// Try to point tail to this element.
			cas_u64(&_tail.next_salt64, t.next_salt32.next, t.next_salt32.salt, i, t.next_salt32.salt + 1);

			return true;
		}

		inline bool fifo::ipop(u32 &i, u32 &r)
		{
			u32 n;
			state h, t;

			// Loop until pop is successful or the fifo is empty.
			while (1) 
			{
				h.next_salt64 = _head.next_salt64;
				t.next_salt64 = _tail.next_salt64;
				n             = _chain[h.next_salt32.next].next;

				if (t.next_salt32.next == h.next_salt32.next)
				{
					if (n == LAST) 
					{
						// Empty
						return false;
					}

					// Tail is pointing to the head in the non empty fifo,
					// try to fix it up.
					cas_u64(&_tail.next_salt64, t.next_salt32.next, t.next_salt32.salt, n, t.next_salt32.salt + 1);
					continue;
				}

				if (n != LAST) 
				{
					// Head points to the usable element.
					// Try to re point it to the next element
					if (cas_u64(&_head.next_salt64, h.next_salt32.next, h.next_salt32.salt, n, h.next_salt32.salt + 1))
						break;
				}
			}

			// Pop succeeded
			// Old head can be reused.
			r = h.next_salt32.next;
			i = n;

			// Set the next pointer so that push() could check for 
			// double push. Thread safe here because caller now owns
			// the element.
			_chain[r].next = UNUSED;

			return true;
		}
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_FIFO_H__
