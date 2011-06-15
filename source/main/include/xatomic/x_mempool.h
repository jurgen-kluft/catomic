#ifndef __XMULTICORE_MEMPOOL_H__
#define __XMULTICORE_MEMPOOL_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xatomic\private\x_compiler.h"
#include "xatomic\x_lifo.h"
#include "xatomic\x_barrier.h"

#include "xatomic\private\x_allocator.h"

namespace xcore
{
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
			lifo		_lifo;
			xbyte*		_buffer;
			u8			_order;
			bool		_extern;

		public:
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
			bool		init(u32 mempool_esize, u32 size);

			/**
			* Init.
			* Allocates data for fifo but memory pool is supplied by user
			* Use 'size() != 0' to check whether creation was successful or not.
			* @param mempool_esize size of an element (chunk)
			* @param mempool_buf pointer to an existing buffer
			* @param mempool_size size of the buffer
			*/
			bool		init(u32 mempool_esize, xbyte *mempool_buf, u32 mempool_size);

			/**
			* Init.
			* Allocates data for fifo but memory pool is supplied by user
			* Use 'size() != 0' to check whether creation was successful or not.
			*/
			bool		init(lifo::link* lifo_chain, u32 lifo_size, u32 mempool_esize, u8 *mempool_buf, u32 mempool_size);

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
			u32			chunk_size() const											{ return 1 << _order; }

			/**
			* Get total number of chunks in the pool.
			* @return number of chunks
			*/
			u32			max_size() const											{ return _lifo.max_size(); }

			/**
			* Get number of used chunks in the pool.
			* @return number of chunks
			*/
			u32			size() const												{ return _lifo.size(); }

			/**
			* Get number of available chunks.
			* @return number of chunks
			* @warning this is a fairly slow operation.
			*/
			u32			avail() const;

			/**
			* Convert chunk pointer to index
			* @return chunk index
			*/
			u32			c2i(u8 *chunk) const										{ return (chunk - _buffer) >> _order; }

			/**
			* Convert chunk pointer to index
			* @return chunk index
			*/
			u8*			i2c(u32 i) const											{ return _buffer + (i << _order); }

			/**
			* Get free chunk from the pool.
			* @param[out] position (index) of the returned chunk
			* @return pointer to the beginning if the chunk, or 0
			* if there is no space.
			*/
			u8*			get(u32 &i)
			{
				if (!_lifo.pop(i))
					return 0;
				return i2c(i);
			}

			/**
			* Get free chunk from the pool.
			* @return pointer to the beginning if the chunk, or 0
			* if there is no space.
			*/
			u8*			get()
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
				bool r = _lifo.push(i);
				ASSERTS(r, "Invalid index or double free");
			}

			/**
			* Put chunk back into the pool.
			* @param[in] chunk pointer to the beginning of the chunk
			* @param[out] i position (index) of the chunk
			*/
			void		put(u8 *chunk, u32 &i)
			{
				u32 n = c2i(chunk);
				put(n);
				i = n;
			}

			/**
			* Put chunk back into the pool.
			* @param[in] chunk pointer to the beginning of the chunk
			*/
			void		put(u8 *chunk)
			{
				u32 i = c2i(chunk);
				put(i);
			}

			/**
			* Placement new/delete pair
			*/
			void*		operator new(xcore::xsize_t num_bytes, void* mem)			{ return mem; }
			void		operator delete(void* mem, void* )							{ }
			void*		operator new(xcore::xsize_t num_bytes)						{ return get_heap_allocator()->allocate(num_bytes, X_ALIGNMENT_DEFAULT); }
			void		operator delete(void* mem)									{ get_heap_allocator()->deallocate(mem); }

			/**
			* Validate mempool.
			* Used for checking for constructor failures.
			*/
			bool		valid();
		};
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_MEMPOOL_H__
