#ifndef __XMULTICORE_QUEUE_H__
#define __XMULTICORE_QUEUE_H__
#include "xbase/x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase/x_debug.h"
#include "xbase/x_allocator.h"

#include "xatomic/private\x_allocator.h"
#include "xatomic/private\x_compiler.h"
#include "xatomic/x_fifo.h"
#include "xatomic/x_mempool.h"
#include "xatomic/x_atomic.h"

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
		public:
			XCORE_CLASS_NEW_DELETE(sGetAllocator, 4)

			/**
			* Constructor.
			*/
			queue()
				: mAllocator(NULL)
				, mRef(NULL)
			{
			}

			/**
			* Init.
			* Allocates memory pool and fifo.
			* @param size number of items for the queue
			*/
			bool		init(x_iallocator* allocator, u32 size);

			/**
			* Init.
			* Memory pool, lifo, fifo and type buffer are supplied by the user.
			*/
			bool		init(fifo::link* fifo_chain, u32 fifo_size, lifo::link* lifo_chain, u32 lifo_size, xbyte *mempool_buf, u32 mempool_buf_size, u32 mempool_buf_esize, atom_s32* mempool_buf_eref);

			/**
			* Clear.
			* Invalidate this queue, deallocating any claimed memory/resources.
			*/
			void		clear()
			{
				mPool.clear();
				mFifo.clear();

				if (mRef != NULL && mAllocator != NULL)
				{
					mAllocator->deallocate(mRef);
					mRef = NULL;
				}

				mAllocator = NULL;
			}

			/**
			* Destructor. Releases the queue.
			*/
			~queue()
			{
				clear();
			}

			/**
			* Queue size.
			* @return number of items
			*/
			u32				max_size() const
			{
				return mFifo.max_size(); 
			}

			/**
			* Check if queue is empty.
			* @return true if stack is empty, false otherwise
			*/
			bool			empty() const
			{
				return mFifo.empty(); 
			}

			/**
			* Check how much items we can still push into the queue
			* @return the number of items that can still be added to the queue
			*/
			u32				room() const
			{
				return mFifo.room(); 
			}

			/**
			* Check how many items are in the queue
			* @return the number of items in the queue
			*/
			u32				size() const
			{
				return mFifo.size(); 
			}

			/**
			* Check if the cursor the user supplies is still in the queue
			* @return True if the cursor still is in the queue
			*/
			bool			inside(u32 cursor) const
			{
				return mFifo.inside(cursor);
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
				u8 *p = mPool.get(i);
				if (unlikely(!p))
					return 0;
				mRef[i].set(1);
				return (T *) p;
			}

			/**
			* Cancel push transaction. Put an item back into the pool.
			* Item must have been obtained via push_begin().
			* @param[in] item pointer to an item
			*/
			void			push_cancel(T *p)
			{
				u32 i = mPool.c2i((u8 *) p);
				release(i);
			}

			/**
			* Commit push transaction. Push an item into the back of the queue.
			* @warning Item must have been obtained via push_begin().
			* @param[in] item pointer to an item
			*/
			void			push_commit(T *p, u32& outCursor)
			{
				u32 i = mPool.c2i((u8 *) p);
				ASSERTS(i < mPool.size(), "xcore::atomic::queue<T>: Error, invalid index");

				mRef[i].incr();

				bool fp = mFifo.push(i, outCursor);
				ASSERTS(fp, "xcore::atomic::queue<T>: Error, state is corrupted!");
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
			bool			push(T const& inData, u32 &outCursor)
			{
				// Open coded push_begin() -> copy -> push_commit()
				// transaction.

				u32 i;
				u8 *p = mPool.get(i);
				if (unlikely(!p))
					return false;

				// Holding two references. One for the queue itself 
				// and one for the user.
				// Same as in push_begin() -> push_commit() transaction.
				ASSERTS(i < mPool.max_size(), "xcore::atomic::queue<T>: Error, invalid index");
				mRef[i].set(2);

				*(T *)p = inData;

				bool fp = mFifo.push(i, outCursor);
				ASSERTS(fp, "xcore::atomic::queue<T>: Error, state is corrupted!");

				return true;
			}

			bool			push(T const& inData)
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
				if (!mFifo.pop(i, r))
					return 0;
				release(r);
				return (T *) mPool.i2c(i);
			}

			/**
			* Finish pop transaction. Put an item back into the pool.
			* Item must have been obtained via pop_begin().
			* @param[in] item pointer to an item
			*/
			void			pop_finish(T *p)
			{
				u32 i = mPool.c2i((u8 *) p);
				release(i);
			}

			/**
			* Pop data from the queue.
			* One shot pop.
			* @return true on success, false otherwise
			*/
			bool			pop(T& outData)
			{
				// Open coded pop_begin() -> copy -> pop_finish() transaction.

				u32 i, r;
				if (!mFifo.pop(i, r))
					return false;

				release(r);

				T *p = (T *) mPool.i2c(i);
				outData = *p;

				release(i);

				return true;
			}

			/**
			* Validate queue.
			* Used for checking for constructor failures.
			*/
			bool			valid()
			{
				if (mRef == NULL)
					return false;
				return (mFifo.valid() && mPool.valid());
			}

		private:
			/**
			* Put an item back into the pool.
			* Item must have been obtained via get() or pop().
			* @param[in] i item index
			*/
			void			release(u32 i)
			{
				if (!mRef[i].decr_test())
					mPool.put(i);
			}

			x_iallocator*	mAllocator;
			mempool			mPool;
			fifo			mFifo;
			atom_s32*		mRef;
		};


		template <typename T>
		bool		queue<T>::init(x_iallocator* allocator, u32 size)
		{
			mAllocator = allocator;

			if (!mPool.init(allocator, sizeof(T), size + 1))
			{
				clear();
				return false;
			}
			if (mPool.max_size() != size + 1)
			{
				clear();
				return false;
			}
			if (!mFifo.init(allocator, size))
			{
				clear();
				return false;
			}

			mRef = (atom_s32*)allocator->allocate(sizeof(atom_s32) * (size + 1), 4);

			if (!valid())
			{
				clear();
				return false;
			}

			// FIFO requires a dummy item
			u32 i;
			u8 *p = mPool.get(i);

			ASSERTS(p!=NULL, "xcore::atomic::queue<T>: Error, something is wrong!");

			mFifo.reset(i);
			mRef[i].set(1);
			return true;
		}

		template <typename T>
		bool		queue<T>::init(fifo::link* fifo_chain, u32 fifo_size, lifo::link* lifo_chain, u32 lifo_size, xbyte *mempool_buf, u32 mempool_buf_size, u32 mempool_buf_esize, atom_s32* mempool_buf_eref)
		{
			ASSERT(lifo_size == fifo_size);

			if (!mPool.init(lifo_chain, lifo_size, mempool_buf_esize, mempool_buf, mempool_buf_size))
			{
				clear();
				return false;
			}

			if (mPool.max_size() != lifo_size)
			{
				clear();
				return false;
			}

			if (!mFifo.init(fifo_chain, fifo_size))
			{
				clear();
				return false;
			}

			mRef = mempool_buf_eref;

			if (!valid())
			{
				clear();
				return false;
			}

			// FIFO requires a dummy item
			u32 i;
			u8 *p = mPool.get(i);

			ASSERTS(p!=NULL, "xcore::atomic::queue<T>: Error, something is wrong!");

			mFifo.reset(i);
			mRef[i].set(1);
			return true;
		}


	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_QUEUE_H__
