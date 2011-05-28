#ifndef __XMULTICORE_RING_H__
#define __XMULTICORE_RING_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xatomic\private\x_allocator.h"
#include "xatomic\x_barrier.h"
#include "xatomic\x_compiler.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Ring buffer. Simple and efficient lock free circular fifo.
		* Thread safety is guarantied only for the 'single writer -> single reader' case.
		* Ring size is restricted to the powers of two. One item is always reserved for 
		* internal operation.
		*/
		template <typename T = u32>
		class ring
		{
		protected:
			typedef volatile u32 vo_u32; 

			T*				_item;
			u32				_size;

			// R/W access by the reader
			// R/O access by the writer
			vo_u32			_head;

			// R/W access by the writer
			// R/O access by the reader
			vo_u32			_tail;

			// Round up to the power of two	
			static u32		po2(u32 size)
			{
				u32 i;
				for (i=0; (1U << i) < size; i++) {}
				return 1U << i;
			}

			ring() {};

		public:
			/**
			* Allocate the ring of specified size
			*/
			ring(u32 size)
			{
				_size = po2(size);

				//_item = new T[_size]();
				_item = allocate_array<T>(_size);
				
				if (_item == NULL)
				{
					_size = 0;
					return;
				}
				_size--;
				reset();
			}

			~ring()
			{
				deallocate_array(_item, _size);
			}

			/** 
			* Reset the ring.
			* @warning Not thread safe.
			*/
			void		reset(void)
			{ 
				_head = _tail = 0;
			}

			/**
			* Get ring size.
			* @return max number of ring items
			*/
			u32			size() const										{ return _size + 1; }

			/**
			* Get number of items in the ring
			* @return number of items in the ring
			*/
			u32			count() const										{ return (_tail - _head) & _size; }
			u32			len()   const										{ return (_tail - _head) & _size; }

			/**
			* Get available room.
			* @return number that can be ring
			*/
			u32			room() const										{ return (_head - _tail - 1) & _size; }

			/**
			* Check if ring is empty.
			* @return true if ring is empty, false otherwise.
			*/
			bool		empty() const										{ return _head == _tail; }

			// -------- Writer interface ---------
			/**
			* Begin push transaction. Grabs tail item. Writer's interface.
			* @return pointer to the item or null of ring is full
			*/
			T*			push_begin()
			{
				u32 h = (_head - 1) & _size;
				u32 t = _tail;

				if (t == h)
					return 0;

				return &_item[t];
			}

			/**
			 * Commit push transaction. Puts item obtained with push_begin() into the ring tail.
			 * Writer's interface.
			 * @param n number of records pushed
			 */
			void		push_commit(u32 n = 1)
			{
				u32 t = _tail;

				// Barrier is needed to make sure that item is updated
				// before it's made available to the reader.
				barrier::memw();

				_tail = (t + n) & _size;
			}

			/**
			* Push an item into the ring tail. Writer's interface.
			* One shot push.
			* @param data data to push into the ring buffer
			* @return false if the ring is full, true otherwise
			*/
			bool		push(T *data)
			{
				u32 h = (_head - 1) & _size;
				u32 t = _tail;

				if (t == h)
					return false;

				_item[t] = *data;

				// Barrier is needed to make sure that item is updated 
				// before it's made available to the reader
				barrier::memw();

				_tail = (t + 1) & _size;
				return true;
			}

			bool		push(T data)										{ return push(&data); }

			// -------- Reader interface ---------


			/**
			 * Begin pop transaction. 
			 * Get the pointer to the next item from the head of the ring without removing it. 
			 * Reader's interface.
			 * @return pointer to the item
			 */
			T*			pop_begin()
			{
				u32 h = _head;
				if (h == _tail)
					return 0;

				return &_item[h];
			}

			/**
			 * Commit pop transaction.
			 * Reader's interface.
			 * @param n number of records consumed
			 */
			void		pop_commit(u32 n = 1)
			{
				// Barrier is needed to make sure that we finished reading items 
				// before moving the head
				barrier::comp();

				u32 h = _head;
				_head = (h + n) & _size;
			}

			/**
			* Pop the next item from the head of the ring. 
			* One shot pop.
			* Reader's interface.
			* @param data pointer to the data 
			* @return false if the ring is empty, true otherwise
			*/
			bool		pop(T *data)
			{
				u32 t = _tail;
				u32 h = _head;
				if (h == t)
					return false;

				*data = _item[h];

				// Barrier is needed to make sure that we finished
				// reading the item before moving the head.
				barrier::comp();
				_head = (h + 1) & _size;
				return true;
			}

			/**
			* Get the next item from the head of the ring without removing it.
			* Reader's interface.
			* @param data pointer to the data 
			* @return false if the ring is empty, true otherwise
			*/
			bool		peek(T *data) const
			{
				u32 h = _head;
				if (h == _tail)
					return false;
				*data = _item[h];
				return true;
			}

			friend class iterator;

			/**
			* Simple ring iterator.
			* Iterates over ring items from head to tail.
			* @warning Can be used safely only by the reader.
			*/
			class iterator
			{
			private:
				u32			_head;
				u32			_tail;
				ring*		_ring;

			public:
				/**
				* Allocate empty iterator.
				* User is supposed to initialize it with 'iterate(&ring)' before using.
				*/
				iterator() 
					: _head(0)
					, _tail(0)
					, _ring(0)													{}

				/**
				* Allocate and initialize iterator.
				* @param r pointer to the ring object to iterate over
				*/
				iterator(ring<T> *r) 
					: _head(r->_head)
					, _tail(r->_tail)
					, _ring(r)													{}

				~iterator()														{}

				/**
				* Get the pointer to the next item.
				* @param data pointer to the data
				* @return false if there are no more items, true otherwise
				*/
				bool		next(T **data)
				{
					if (_head == _tail)
						return false;      
					*data = &_ring->_item[_head];
					_head = (_head + 1) & _ring->_size;
					return true;
				}

				/**
				* Get next item.
				* @param data pointer to the data
				* @return false if there are no more items, true otherwise
				*/
				bool		next(T *data)
				{
					if (_head == _tail)
						return false;      
					*data = _ring->_item[_head];
					_head = (_head + 1) & _ring->_size;
					return true;
				}

				/**
				* Reset the iterator to iterate over a new ring.
				* @param q pointer to the ring
				*/
				void		iterate(ring<T> *r)
				{
					_ring = r;
					_head = r->_head;
					_tail = r->_tail;
				}
			};
		};
	}
} // namespace xcore

#endif // __XMULTICORE_RING_H__
