#include "xbase\x_allocator.h"
#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"

#include "xatomic\x_atomic.h"
#include "xatomic\x_mempool.h"

namespace xcore
{
	namespace atomic
	{
		mempool::mempool()
		{
			_allocator = NULL;
			_buffer = NULL;
			_csize = 0;
			_extern = false;
		}

		bool mempool::init(x_iallocator* allocator, u32 mempool_esize, u32 size)
		{
			_allocator = allocator;

			// Initialize the lifo first
			if (!_lifo.init(allocator, size))
				return false;

			u32 csize = x_intu::alignUp(mempool_esize, 4);

			_extern = false;
			_buffer = (xbyte*)allocator->allocate(csize * size, 4);
			x_memset(_buffer, 0, csize * size);

			// Caller will have to do the delete anyway. Let the
			// destructor take care of partial allocations.
			if (!_buffer)
				return false;

			_csize = csize;

			_lifo.fill();
			return true;
		}

		bool mempool::init(x_iallocator* allocator, u32 mempool_esize, xbyte *mempool_buf, u32 mempool_size)
		{
			u32 csize = x_intu::alignUp(mempool_esize, 4);

			u32 size = mempool_size / mempool_esize;

			// Initialize the lifo first
			if (!_lifo.init(allocator, size))
				return false;

			// Attach to an external buffer 
			_buffer = mempool_buf;
			_extern = true;

			_lifo.fill();
			return true;
		}

		bool	mempool::init(lifo::link* lifo_chain, u32 lifo_size, u32 mempool_esize, u8 *mempool_buf, u32 mempool_size)
		{
			u32 csize = x_intu::alignUp(mempool_esize, 4);

			u32 size = mempool_size / csize;
			if (size > lifo_size)
				return false;

			if (!_lifo.init(lifo_chain, lifo_size))
				return false;

			// Attach to an external buffer 
			_buffer = mempool_buf;
			_csize  = csize;
			_extern = true;

			_lifo.fill();
			return true;
		}

		void	mempool::clear()
		{
			if (!_extern)
			{
				if (_buffer != NULL)
					_allocator->deallocate(_buffer);
				_buffer = NULL;
			}
			_lifo.clear();
			_csize = 0;
		}

		mempool::~mempool()
		{
			clear();
		}

		bool mempool::valid()
		{
			return (_lifo.max_size()>0 && _buffer!=NULL);
		}

		u32 mempool::avail() const
		{
			return _lifo.room();
		}
	} // namespace atomic
} // namespace xcore
