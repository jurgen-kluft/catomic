#include "xbase\x_integer.h"
#include "xbase\x_memory_std.h"

#include "xmulticore\x_atomic.h"
#include "xmulticore\x_mempool.h"

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
			_order  = 0;
			_extern = false;
		}

		mempool::mempool(u32 chunk_size, u32 size)
		{
			// Initialize the lifo first
			if (!_lifo.init(size))
				return;

			u32 order = po2(chunk_size);
			chunk_size = (1 << order);

			_extern = false;
			_buffer = new u8 [chunk_size * size];
			x_memset(_buffer, 0, chunk_size * size);

			// Caller will have to do the delete anyway. Let the
			// destructor take care of partial allocations.
			if (!_buffer)
				return;

			_order = order;

			_lifo.fill();
		}

		mempool::mempool(u32 chunk_size, u8 *buf, u32 bsize)
		{
			u32 size, order;

			order = po2(chunk_size);
			chunk_size  = 1 << order;

			size = bsize / chunk_size;

			// Initialize the lifo first
			if (!_lifo.init(size))
				return;

			// Attach to an external buffer 
			_buffer = buf;
			_extern = true;

			_order = order;

			_lifo.fill();
		}

		mempool::~mempool()
		{
			if (!_extern)
				delete [] _buffer;
		}

		bool mempool::valid()
		{
			if (_lifo.size() && _buffer)
				return true;
			return false;
		}

		u32 mempool::avail() const
		{
			return _lifo.size() - _lifo.room();
		}
	} // namespace atomic
} // namespace xcore
