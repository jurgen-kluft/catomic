#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"

#include "xatomic\x_atomic.h"
#include "xatomic\x_mempool.h"

#include "xatomic\private\x_allocator.h"

namespace xcore
{
	namespace atomic
	{
		static inline u32 po2(u32 size)
		{
			u32 i = x_intu::ceilPower2(size);
			return i;
		}

		mempool::mempool()
		{
			_buffer = NULL;
			_order  = 0;
			_extern = false;
		}

		bool mempool::init(u32 mempool_esize, u32 size)
		{
			// Initialize the lifo first
			if (!_lifo.init(size))
				return false;

			u32 order = po2(mempool_esize);
			mempool_esize = (1 << order);

			_extern = false;
			_buffer = (xbyte*)get_heap_allocator()->allocate(mempool_esize * size, 4);
			x_memset(_buffer, 0, mempool_esize * size);

			// Caller will have to do the delete anyway. Let the
			// destructor take care of partial allocations.
			if (!_buffer)
				return false;

			_order = order;

			_lifo.fill();
			return true;
		}

		bool mempool::init(u32 mempool_esize, xbyte *mempool_buf, u32 mempool_size)
		{
			u32 size, order;

			order = po2(mempool_esize);
			mempool_esize = 1 << order;

			size = mempool_size / mempool_esize;

			// Initialize the lifo first
			if (!_lifo.init(size))
				return false;

			// Attach to an external buffer 
			_buffer = mempool_buf;
			_extern = true;

			_order = order;

			_lifo.fill();
			return true;
		}

		bool	mempool::init(lifo::link* lifo_chain, u32 lifo_size, u32 mempool_esize, u8 *mempool_buf, u32 mempool_size)
		{
			u32 size, order;

			order = po2(mempool_esize);
			mempool_esize = 1 << order;

			size = mempool_size / mempool_esize;
			if (size > lifo_size)
				return false;

			if (!_lifo.init(lifo_chain, lifo_size))
				return false;

			// Attach to an external buffer 
			_buffer = mempool_buf;
			_extern = true;

			_order = order;

			_lifo.fill();
			return true;
		}

		void	mempool::clear()
		{
			if (!_extern)
			{
				if (_buffer != NULL)
					get_heap_allocator()->deallocate(_buffer);
				_buffer = NULL;
			}
			_lifo.clear();
		}

		mempool::~mempool()
		{
			clear();
		}

		bool mempool::valid()
		{
			if (_lifo.max_size() && _buffer)
				return true;

			return false;
		}

		u32 mempool::avail() const
		{
			return _lifo.room();
		}
	} // namespace atomic
} // namespace xcore
