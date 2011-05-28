#ifndef __XMULTICORE_QUEUE_H__
#define __XMULTICORE_QUEUE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xmulticore\private\x_allocator.h"
#include "xmulticore\x_compiler.h"
#include "xmulticore\x_fifo.h"
#include "xmulticore\x_mempool.h"
#include "xmulticore\x_atomic.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Multi-reader, multi-writer lock-free queue.
		*/
		template <typename T>
		class queue
		{
		private:
			mempool			_pool;
			fifo			_fifo;
			int32*			_ref;

			/**
			* Put an item back into the pool.
			* Item must have been obtained via get() or pop().
			* @param[in] i item index
			*/
			void			release(u32 i)
			{
				if (!_ref[i].dec())
					_pool.put(i);
			}

		public:
			/**
			* Constructor. Allocates the queue.
			* @param size number of items in the queue
			*/
			queue(u32 size) 
				: _pool(sizeof(T), size)
				, _fifo(size)
			{
				_ref = (int32*)get_heap_allocator()->allocate(sizeof(int32) * size, 4);
				if (!valid())
					return;

				// Fifo requires a dummy item
				u32 i;
				u8 *p = _pool.get(i);

				ASSERT(p && "Something is busted");

				_fifo.reset(i);
				_ref[i].set(1);
			}

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
			u32				size() const											{ return _fifo.size(); }

			/**
			* Check if queue is empty.
			* @return true if stack is empty, false otherwise
			*/
			bool			empty() const											{ return _fifo.empty(); }

			u32				room() const											{ return _fifo.room(); }

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
				_ref[i].set(1);
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
			void			push_commit(T *p)
			{
				u32 i = _pool.c2i((u8 *) p);
				ASSERT(i < _pool.size() && "Invalid index");

				_ref[i].inc();

				bool fp = _fifo.push(i);
				ASSERT(fp && "Corrupted state");
			}

			/**
			* Push data onto the queue.
			* One shot push.
			* @param[in] data data to push
			*/
			bool			push(T *data)
			{
				// Open coded push_begin() -> copy -> push_commit()
				// transaction.

				u32 i;
				u8 *p = _pool.get(i);
				if (unlikely(!p))
					return false;

				// Holding two references. One for the queue itself 
				// and one for the user.
				// Same as in push_begin() -> push_commit() transaction.
				_ref[i].set(2);

				*(T *)p = *data;

				bool fp = _fifo.push(i);
				ASSERT(fp && "Corrupted state");

				return true;
			}

			bool			push(T data)										{ return push(&data); }

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
				release(i);
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

				release(i);
				return true;
			}

			/**
			* Validate queue.
			* Used for checking for constructor failures.
			*/
			bool			valid()
			{
				return (_ref && _fifo.size() != 0 && _pool.valid());
			}
		};
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_QUEUE_H__
