#ifndef __XMULTICORE_QUEUE_H__
#define __XMULTICORE_QUEUE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xatomic\private\x_allocator.h"
#include "xatomic\private\x_compiler.h"
#include "xatomic\x_fifo.h"
#include "xatomic\x_mempool.h"
#include "xatomic\x_atomic.h"

namespace xcore
{
	namespace atomic
	{
		//#define X_ATOMIC_QUEUE_REF_CNT

		/**
		* Multi-reader, multi-writer lock-free queue.
		*/
		template <typename T>
		class queue
		{
		private:
			mempool			_pool;
			fifo			_fifo;
#ifdef X_ATOMIC_QUEUE_REF_CNT
			atom_s32*		_ref;
#endif
			/**
			* Put an item back into the pool.
			* Item must have been obtained via get() or pop().
			* @param[in] i item index
			*/
			void			release(u32 i)
			{
#ifdef X_ATOMIC_QUEUE_REF_CNT
				if (!_ref[i].dec())
#endif
					_pool.put(i);
			}

		public:
			/**
			* Constructor.
			*/
			queue() 
			{
			}

			/**
			* Init.
			* Allocates memory pool and fifo.
			* Use 'size() != 0' to check whether 
			* allocations where successful or not.
			* @param size number of items for the queue
			*/
			bool		init(u32 size);

			/**
			* Init.
			* Memory pool, lifo, fifo and type buffer are supplied by the user.
			*/
			bool		init(fifo::link* fifo_chain, lifo::link* lifo_chain, u32 queue_lifo_fifo_size, xbyte *mempool_buf, u32 mempool_buf_size, u32 mempool_buf_esize);

			/**
				* Destructor. Releases the queue.
				*/
			~queue()
			{
				get_heap_allocator()->deallocate(_ref);
				_ref = NULL;
			}

			/**
			* Queue size.
			* @return number of items
			*/
			u32				max_size() const
			{
				return _fifo.max_size(); 
			}

			/**
			* Check if queue is empty.
			* @return true if stack is empty, false otherwise
			*/
			bool			empty() const
			{
				return _fifo.empty(); 
			}

			/**
			* Check how much items we can still push into the queue
			* @return the number of items that can still be added to the queue
			*/
			u32				room() const
			{
				return _fifo.room(); 
			}

			/**
			* Check how many items are in the queue
			* @return the number of items in the queue
			*/
			u32				size() const
			{
				return _fifo.size(); 
			}

			/**
			* Check if the cursor the user supplies is still in the queue
			* @return True if the cursor still is in the queue
			*/
			bool			inside(u32 cursor) const
			{
				return _fifo.inside(cursor);
			}

			// ---- PUSH interface ----

			/**
			* Begin push transaction. Get free item from the pool.
			* @return pointer to the beginning if the item, or 0
			* if there is no space.
			*/
			T*				push_begin()
			{
				u32 i;
				u8 *p = _pool.get(i);
				if (unlikely(!p))
					return 0;
#ifdef X_ATOMIC_QUEUE_REF_CNT
				_ref[i].set(1);
#endif
				return (T *) p;
			}

			/**
			* Cancel push transaction. Put an item back into the pool.
			* Item must have been obtained via push_begin().
			* @param[in] item pointer to an item
			*/
			void			push_cancel(T *p)
			{
				u32 i = _pool.c2i((u8 *) p);
				release(i);
			}

			/**
			* Commit push transaction. Push an item into the back of the queue.
			* @warning Item must have been obtained via push_begin().
			* @param[in] item pointer to an item
			*/
			void			push_commit(T *p, u32& outCursor)
			{
				u32 i = _pool.c2i((u8 *) p);
				ASSERT(i < _pool.size() && "Invalid index");

#ifdef X_ATOMIC_QUEUE_REF_CNT
				_ref[i].inc();
#endif
				bool fp = _fifo.push(i, outCursor);
				ASSERT(fp && "Corrupted state");
			}

			void			push_commit(T *p)
			{
				u32 cursor;
				push_commit(p, cursor);
			}

			/**
			* Push data onto the queue.
			* One shot push.
			* @param[in] data data to push
			*/
			bool			push(T *inData, u32 &outCursor)
			{
				// Open coded push_begin() -> copy -> push_commit()
				// transaction.

				u32 i;
				u8 *p = _pool.get(i);
				if (unlikely(!p))
					return false;

#ifdef X_ATOMIC_QUEUE_REF_CNT
				// Holding two references. One for the queue itself 
				// and one for the user.
				// Same as in push_begin() -> push_commit() transaction.
				_ref[i].set(2);
#endif
				*(T *)p = *inData;

				bool fp = _fifo.push(i, outCursor);
				ASSERT(fp && "Corrupted state");

				return true;
			}

			bool			push(T *inData)
			{
				u32 cursor;
				return push(inData, cursor);
			}

			// ---- POP interface ----

			/**
			* Pop an item from the queue.
			* Item must be released via pop_finish().
			* @return pointer to an item or zero if the queue is empty
			*/
			T*				pop_begin()
			{
				u32 i, r;
				if (!_fifo.pop(i, r))
					return 0;
				release(r);
				return (T *) _pool.i2c(i);
			}

			/**
			* Finish pop transaction. Put an item back into the pool.
			* Item must have been obtained via pop_begin().
			* @param[in] item pointer to an item
			*/
			void			pop_finish(T *p)
			{
				u32 i = _pool.c2i((u8 *) p);
#ifdef X_ATOMIC_QUEUE_REF_CNT
				release(i);
#endif
			}

			/**
			* Pop data from the queue.
			* One shot pop.
			* @return true on success, false otherwise
			*/
			bool			pop(T *data)
			{
				// Open coded pop_begin() -> copy -> pop_finish() transaction.

				u32 i, r;
				if (!_fifo.pop(i, r))
					return 0;

				release(r);

				T *p = (T *) _pool.i2c(i);
				*data = *p;

#ifdef X_ATOMIC_QUEUE_REF_CNT
				release(i);
#endif
				return true;
			}

			/**
			* Validate queue.
			* Used for checking for constructor failures.
			*/
			bool			valid()
			{
#ifdef X_ATOMIC_QUEUE_REF_CNT
				if (ref == NULL)
					return false;
#endif
				return (_fifo.size() != 0 && _pool.valid());
			}
		};


		template <typename T>
		bool		queue<T>::init(u32 size)
		{
			_pool.init(sizeof(T), size);
			_fifo.init(size);

#ifdef X_ATOMIC_QUEUE_REF_CNT
			_ref = (atom_s32*)get_heap_allocator()->allocate(sizeof(atom_s32) * size, 4);
#endif
			if (!valid())
				return;

			// FIFO requires a dummy item
			u32 i;
			u8 *p = _pool.get(i);

			ASSERT(p && "Something is busted");

			_fifo.reset(i);
#ifdef X_ATOMIC_QUEUE_REF_CNT
			_ref[i].set(1);
#endif

		}

		template <typename T>
		bool		queue<T>::init(fifo::link* fifo_chain, lifo::link* lifo_chain, u32 queue_lifo_fifo_size, xbyte *mempool_buf, u32 mempool_buf_size, u32 mempool_buf_esize)
		{
			_pool.init(mempool_buf_esize, mempool_buf, mempool_buf_size);
			_fifo.init(fifo_chain, queue_lifo_fifo_size);
		}


	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_QUEUE_H__
