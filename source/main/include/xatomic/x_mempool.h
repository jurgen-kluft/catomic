#ifndef __XMULTICORE_MEMPOOL_H__
#define __XMULTICORE_MEMPOOL_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xatomic\x_compiler.h"
#include "xatomic\x_lifo.h"
#include "xatomic\x_barrier.h"

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
						mempool();

			lifo		_lifo;
			u8*			_buffer;
			u8			_order;
			bool		_extern;

		public:
			/**
			* Get chunk size.
			* @return chunk size
			*/
			u32			chunk_size() const											{ return 1 << _order; }

			/**
			* Get total number of chunks in the pool.
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
			* Constructor.
			* Allocates memory pool. Use 'size() != 0' to check whether 
			* allocation was successful or not.
			* @param csize size of the chunk in bytes
			* @param size number of chunks in the pool
			*/
						mempool(u32 csize, u32 size);

			/**
			* Constructor.
			* Create memory pool and attach it to an existing buffer.
			* Use 'size() != 0' to check whether creation was successful or not.
			* @param csize size of the chunk in bytes
			* @param buf pointer to an existing buffer
			* @param bsize size of the buffer
			*/
						mempool(u32 csize, u8 *buf, u32 bsize);

			/**
			* Destructor
			*/
						~mempool();

			/**
			* Placement new/delete pair
			*/
			void*		operator new(xcore::xsize_t num_bytes, void* mem)			{ return mem; }
			void		operator delete(void* mem, void* )							{ }

			/**
			* Validate mempool.
			* Used for checking for constructor failures.
			*/
			bool		valid();
		};
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_MEMPOOL_H__
