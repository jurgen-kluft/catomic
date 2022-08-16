#ifndef __XMULTICORE_LIFO_H__
#define __XMULTICORE_LIFO_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "cbase/c_debug.h"
#include "cbase/c_allocator.h"

#include "catomic/private/c_allocator.h"
#include "catomic/private/c_compiler.h"
#include "catomic/c_atomic.h"
#include "catomic/c_barrier.h"

namespace xcore
{
	class alloc_t;

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
		* Maximum number of elements < 65535
		* @see stack, fifo
		*/
		class lifo 
		{
		public:
			struct link_t
			{
				volatile u32	next;
			};

			struct link : public link_t
			{
				XCORE_CLASS_PLACEMENT_NEW_DELETE
			};

		protected:
			typedef		volatile u32	vu32;


			union state
			{
				volatile u64 next_salt64;
				struct
				{
#ifdef X_LITTLE_ENDIAN
					vu32	next;
					vu32	salt;
#else
					vu32	salt;
					vu32	next;
#endif
				} next_salt32;
			};

			enum
			{
				UNUSED = 0xffffffff,
				LAST   = 0xfffffffe,
			};

			state			_head;
			link*			_chain;
			u32				_max_size;
			alloc_t*	_allocator;

			inline u32	increase_push(u32 salt)
			{
				u32 const cnt = ((salt & 0x0000ffff) + 0x00000001) & 0x0000ffff;
				return (salt & 0xffff0000) | cnt;
			}

			inline u32	increase_pop(u32 salt)
			{
				u32 const cnt = ((salt & 0xffff0000) + 0x00010000) & 0xffff0000;
				return (salt & 0x0000ffff) | cnt;
			}

		public:
			XCORE_CLASS_NEW_DELETE(sGetAllocator, 4)

			/**
			* Create empty lifo. It can be initialized later by calling init().
			*/
						lifo() 
							: _chain(NULL)
							, _max_size(0)
							, _allocator(NULL)									{ }

			/**
			* Destructor
			*/
						~lifo();

			/**
			* Complete initialization.
			*/
			bool		init(alloc_t* allocator, u32 size);

			/**
			* Complete initialization.
			*/
			bool		init(link* chain, u32 size);

			/**
			* Clear the lifo, deallocate all memory
			*/
			void		clear();

			/**
			* Validate lifo.
			* @return True if lifo is initialized
			*/
			bool		valid() const
			{
				return _chain!=NULL && _max_size>0;
			}

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
			u32			max_size() const
			{
				return _max_size; 
			}

			/**
			* Number of unused elements.
			* @return approximate number of unused elements
			* @warning this method is slow and inefficient
			*/
			u32			room() const
			{
				u32 const s = _head.next_salt32.salt;
				u32 const h = (s & 0x0000ffff);
				u32 const t = (s & 0xffff0000) >> 16;

				u32 used = (t >= h) ? (t - h) : (h - t);
				if (used > _max_size)
					used = _max_size;
				return _max_size - used;
			}

			/**
			* Number of used elements.
			* @return approximate number of unused elements
			* @warning this method is slow and inefficient
			*/
			u32			size() const
			{
				u32 const s = _head.next_salt32.salt;
				u32 const h = (s & 0x0000ffff);
				u32 const t = (s & 0xffff0000) >> 16;

				u32 used = (t >= h) ? (t - h) : (h - t);
				if (used > _max_size)
					used = _max_size;
				return used;
			}

			/**
			* Check if lifo is empty
			* @return true if empty
			*/
			bool		empty() const
			{
				return size() == 0;
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
				bool b = pop(i);
				r = i;
				return b;
			}
		};

		inline bool lifo::ipush(u32 i)
		{
			state h;

			if (i >= _max_size)
				return false;

			// Double push trap.
			// Thread safe because the caller still owns the element.
			if (_chain[i].next != UNUSED)
				return false;

			// Spin until push is successful 
			do
			{
				h.next_salt64 = _head.next_salt64;

				_chain[i].next = h.next_salt32.next;
				// We need write barrier here to make sure that _chain[i].next
				// is visible on all CPUs before it's linked in.
				barrier::memw();
			} while (!cas_u64(&_head.next_salt64, h.next_salt32.next, h.next_salt32.salt, i, increase_push(h.next_salt32.salt)));

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
				if (h.next_salt32.next == _max_size)
					return false;

				n = _chain[h.next_salt32.next].next;
			} while (!cas_u64(&_head.next_salt64, h.next_salt32.next, h.next_salt32.salt, n, increase_pop(h.next_salt32.salt)));

			i = h.next_salt32.next;

			// Clear 'next' index so that push() can check for double push. 
			// Thread safe here because caller now owns the element.
			ASSERT(i < _max_size);
			_chain[i].next = UNUSED;

			return true;
		}
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_LIFO_H__
