#ifndef __XMULTICORE_RING_H__
#define __XMULTICORE_RING_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase/x_allocator.h"
#include "xbase/x_integer.h"

#include "xatomic/private/x_allocator.h"
#include "xatomic/private/x_compiler.h"
#include "xatomic/x_barrier.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Ring buffer. Simple and efficient lock free circular fifo.
		* Thread safety is guaranteed only for the 'single writer -> single reader' case.
		* Ring size is restricted to the powers of two. One item is always reserved for 
		* internal operation.
		*/
		template <typename T = u32>
		class ring
		{
		public:
			struct node
			{
				inline	node() {}
				T		item;
				XCORE_CLASS_PLACEMENT_NEW_DELETE
			};

		protected:
			typedef volatile u32 vo_u32; 

			x_iallocator*	_allocator;

			node*			_items;
			u32				_size;

			// R/W access by the reader
			// R/O access by the writer
			vo_u32			_popi;
			T*				_pop_transaction;

			// R/W access by the writer
			// R/O access by the reader
			vo_u32			_pushi;
			T*				_push_transaction;

		public:
			XCORE_CLASS_NEW_DELETE(sGetAllocator, 4)

			/**
			* Construct an invalid ring, use init() to initialize a valid ring.
			*/
			ring()
			{
				_allocator = NULL;
				_items = NULL;
				_size = 0;
				_popi = 0;
				_pop_transaction = NULL;
				_pushi = 0;
				_push_transaction = NULL;
			}

			~ring()
			{
				clear();
			}


			/**
			* Allocate the ring of specified size
			*/
			bool		init(x_iallocator* allocator, u32 size)
			{
				_allocator = allocator;
				_size = size;
				_items = allocate_array<node>(_allocator, _size + 1);
				if (_items == NULL)
				{
					_size = 0;
					return false;
				}
				reset();
				return true;
			}


			/**
			* Initialize the ring of specified size with a give item buffer
			* Size is the size given by the user - 1, since we need one item
			* for internal use
			*/
			bool		init(node* items, u32 size)
			{
				_allocator = NULL;
				_size = size - 1;
				_items = items;
				if (_items == NULL)
				{
					_size = 0;
					return false;
				}
				reset();
				return true;
			}

			/**
			* Clear the ring, deallocate all data
			*/
			void		clear()
			{
				if (_allocator != NULL)
					deallocate_array(_allocator, _items, _size);

				_allocator = NULL;
				_items = NULL;
				_size = 0;
				_popi = 0;
				_pop_transaction = NULL;
				_pushi = 0;
				_push_transaction = NULL;
			}

			/** 
			* Reset the ring.
			* @warning Not thread safe.
			*/
			void		reset(void)
			{ 
				_popi = 0;
				_pop_transaction = NULL;
				_pushi = 0;
				_push_transaction = NULL;
			}

			/**
			* Get ring size.
			* @return max number of ring items
			*/
			u32			max_size() const									{ return _size; }

			/**
			* Get number of items in the ring
			* @return number of items in the ring
			*/
			u32			count() const										{ return (_pushi - _popi); }
			u32			size()   const										{ return (_pushi - _popi); }

			/**
			* Get available room.
			* @return number that can be ring
			*/
			u32			room() const										{ return _size - (_pushi - _popi); }

			/**
			* Check if ring is empty.
			* @return true if ring is empty, false otherwise.
			*/
			bool		empty() const										{ return _popi == _pushi; }

			// -------- Writer interface ---------
			/**
			* Begin push transaction. Grabs tail item. 
			* @return pointer to the item or null of ring is full
			*/
			T*			push_begin()
			{
				ASSERT(_push_transaction == NULL);

				u32 h = _popi;
				u32 t0 = _pushi;
				u32 t1 = (t0 + 1) % _size;

				if (t1 == h)
					return 0;

				_push_transaction = &_items[t0].item;
				return _push_transaction;
			}

			/**
			* Cancel push transaction. 
			*/
			void		push_cancel(T* p)
			{
				ASSERT(_push_transaction != NULL);
				ASSERT(_push_transaction == p);
				_push_transaction = NULL;
			}

			/**
			 * Commit push transaction. Puts item obtained with push_begin() into the ring tail.
			 * @param p obtained by calling push_begin
			 */
			void		push_commit(T* p)
			{
				ASSERT(_push_transaction != NULL);

				u32 t0 = _pushi;
				ASSERT(_push_transaction == &_items[t0].item);

				// Barrier is needed to make sure that item is updated
				// before it's made available to the reader.
				barrier::memw();

				_pushi = (t0 + 1) % _size;
				_push_transaction = NULL;
			}

			/**
			* Push an item into the ring tail.
			* One shot push.
			* @param data data to push into the ring buffer
			* @return false if the ring is full, true otherwise
			*/
			bool		push(T const &data)
			{
				ASSERT(_push_transaction == NULL);

				u32 h = _popi;
				u32 t0 = _pushi;
				u32 t1 = (t0 + 1) % _size;

				if (t1 == h)
					return false;

				_items[t0].item = data;

				// Barrier is needed to make sure that item is updated 
				// before it's made available to the reader
				barrier::memw();

				_pushi = t1;
				return true;
			}

			// -------- Reader interface ---------


			/**
			 * Begin pop transaction. 
			 * Get the pointer to the next item from the head of the ring without removing it. 
			 * @return pointer to the item
			 */
			T*			pop_begin()
			{
				u32 h = _popi;
				if (h == _pushi)
					return 0;

				_pop_transaction = &_items[h].item;
				return _pop_transaction;
			}

			/**
			* Cancel pop transaction. 
			*/
			void		pop_cancel(T* p)
			{
				ASSERT(_pop_transaction != NULL);
				ASSERT(_pop_transaction == p);
				_pop_transaction = NULL;
			}

			/**
			 * Commit pop transaction.
			 * @param n number of records consumed
			 */
			void		pop_commit(T* p)
			{
				ASSERT(_pop_transaction != NULL);

				// Barrier is needed to make sure that we finished reading items 
				// before moving the head
				barrier::comp();

				u32 h = _popi;
				_popi = (h + 1) % _size;

				ASSERT(_push_transaction == &_items[h].item);
				_pop_transaction = NULL;
			}

			/**
			* Pop the next item from the head of the ring. 
			* One shot pop.
			* @param data pointer to the data 
			* @return false if the ring is empty, true otherwise
			*/
			bool		pop(T &data)
			{
				ASSERT(_pop_transaction == NULL);

				u32 t = _pushi;
				u32 h = _popi;
				if (h == t)
					return false;

				data = _items[h].item;

				// Barrier is needed to make sure that we finished
				// reading the item before moving the head.
				barrier::comp();
				_popi = (h + 1) % _size;
				return true;
			}

			/**
			* Get the next item from the head of the ring without removing it.
			* @param data pointer to the data 
			* @return false if the ring is empty, true otherwise
			*/
			bool		peek(T &data) const
			{
				u32 h = _popi;
				if (h == _pushi)
					return false;
				*data = _items[h];
				return true;
			}

			/**
			* Return True if ring is valid
			*/
			bool		valid() const
			{
				return _items!=NULL && _size>0 && _pushi<_size && _popi<_size;
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
				u32			_popi;
				u32			_pushi;
				ring*		_ring;

			public:
				/**
				* Allocate empty iterator.
				* User is supposed to initialize it with 'iterate(&ring)' before using.
				*/
				iterator() 
					: _popi(0)
					, _pushi(0)
					, _ring(0)													{}

				/**
				* Allocate and initialize iterator.
				* @param r pointer to the ring object to iterate over
				*/
				iterator(ring<T> *r) 
					: _popi(r->_popi)
					, _pushi(r->_pushi)
					, _ring(r)													{}

				~iterator()														{}

				/**
				* Get the pointer to the next item.
				* @param data pointer to the data
				* @return false if there are no more items, true otherwise
				*/
				bool		next(T **data)
				{
					if (_popi == _pushi)
						return false;      
					*data = &_ring->_items[_popi];
					_popi = (_popi + 1) % _ring->_size;
					return true;
				}

				/**
				* Get next item.
				* @param data pointer to the data
				* @return false if there are no more items, true otherwise
				*/
				bool		next(T *data)
				{
					if (_popi == _pushi)
						return false;      
					*data = _ring->_items[_popi];
					_popi = (_popi + 1) % _ring->_size;
					return true;
				}

				/**
				* Reset the iterator to iterate over a new ring.
				* @param q pointer to the ring
				*/
				void		iterate(ring<T> *r)
				{
					_ring = r;
					_popi = r->_popi;
					_pushi = r->_pushi;
				}
			};
		};
	}
} // namespace xcore

#endif // __XMULTICORE_RING_H__
