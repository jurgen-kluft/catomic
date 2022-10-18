#ifndef __CMULTICORE_MEMPOOL_H__
#define __CMULTICORE_MEMPOOL_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "cbase/c_debug.h"
#include "cbase/c_allocator.h"

#include "catomic/private/c_allocator.h"
#include "catomic/private/c_compiler.h"
#include "catomic/c_lifo.h"
#include "catomic/c_barrier.h"

namespace ncore
{
	class alloc_t;

	namespace atomic
	{
		/** 
		* Lock free memory pool.
		* O(1), low overhead memory pool that is thread safe and lock free.
		* Implementation is very simple and only supports fixed size chunks.
		* @see lifo is used to keep track of memory chunks.
		*/
		class mempool
		{
		protected:
			alloc_t*	mAllocator;
			lifo			mLifo;
			xbyte*			mBuffer;
			u32				mCsize;
			bool			mExtern;

		public:
			XCORE_CLASS_NEW_DELETE(sGetAllocator, 4)

			/**
			* Constructor.
			*/
						mempool();

			/**
			* Destructor
			*/
						~mempool();

			/**
			* Init.
			* Allocates memory pool. Use 'size() != 0' to check whether 
			* allocation was successful or not.
			* @param mempool_esize size of an element (chunk)
			* @param size number of chunks in the pool
			*/
			bool		init(alloc_t* allocator, u32 mempool_esize, u32 size);

			/**
			* Init.
			* Allocates data for fifo but memory pool is supplied by user
			* Use 'size() != 0' to check whether creation was successful or not.
			* @param mempool_esize size of an element (chunk)
			* @param mempool_buf pointer to an existing buffer
			* @param mempool_size size of the buffer
			*/
			bool		init(alloc_t* allocator, u32 mempool_esize, xbyte *mempool_buf, u32 mempool_size);

			/**
			* Init.
			* Allocates data for fifo but memory pool is supplied by user
			* Use 'size() != 0' to check whether creation was successful or not.
			*/
			bool		init(lifo::link* lifo_chain, u32 lifo_size, u32 mempool_esize, xbyte *mempool_buf, u32 mempool_size);

			/**
			* Exit.
			* Allocates data for fifo but memory pool is supplied by user
			* Use 'size() != 0' to check whether creation was successful or not.

			*/
			void		clear();

			/**
			* Get chunk size.
			* @return chunk size
			*/
			u32			chunk_size() const											{ return mCsize; }

			/**
			* Get total number of chunks in the pool.
			* @return number of chunks
			*/
			u32			max_size() const											{ return mLifo.max_size(); }

			/**
			* Get number of used chunks in the pool.
			* @return number of chunks
			*/
			u32			size() const												{ return mLifo.size(); }

			/**
			* Convert chunk pointer to index
			* @return chunk index
			*/
			u32			c2i(xbyte *chunk) const										{ return (u32)((chunk - mBuffer) / mCsize); }

			/**
			* Convert chunk pointer to index
			* @return chunk index
			*/
			xbyte*		i2c(u32 i) const											{ return mBuffer + (i * mCsize); }

			/**
			* Get free chunk from the pool.
			* @param[out] position (index) of the returned chunk
			* @return pointer to the beginning if the chunk, or 0
			* if there is no space.
			*/
			xbyte*		get(u32 &i)
			{
				if (!mLifo.pop(i))
					return NULL;
				return i2c(i);
			}

			/**
			* Get free chunk from the pool.
			* @return pointer to the beginning if the chunk, or 0
			* if there is no space.
			*/
			xbyte*		get()
			{
				u32 i;
				return get(i);
			}

			/**
			* Put chunk back into the pool.
			* @param[in] i chunk index
			* @param[out] i position (index) of the chunk
			*/
			void		put(u32 i)
			{
				bool r = mLifo.push(i);
				ASSERTS(r, "ncore::atomic::mempool: Error, invalid index or double free");
			}

			/**
			* Put chunk back into the pool.
			* @param[in] chunk pointer to the beginning of the chunk
			* @param[out] i position (index) of the chunk
			*/
			void		put(xbyte *chunk, u32 &i)
			{
				u32 n = c2i(chunk);
				put(n);
				i = n;
			}

			/**
			* Put chunk back into the pool.
			* @param[in] chunk pointer to the beginning of the chunk
			*/
			void		put(xbyte *chunk)
			{
				u32 i = c2i(chunk);
				put(i);
			}

			/**
			* Validate mempool.
			* Used for checking for constructor failures.
			*/
			bool		valid();
		};
	} // namespace atomic
} // namespace ncore

#endif // __CMULTICORE_MEMPOOL_H__
