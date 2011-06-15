#include "xbase\x_string_std.h"

#include "xatomic\private\x_compiler.h"
#include "xatomic\x_mbufpool.h"

#include "xatomic\private\x_allocator.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
			head* pool::allocate_head(void)
			{
				return (head *) _head->get();
			}

			void pool::deallocate_head(head *h)
			{
				_head->put((u8 *) h);
			}

			// Hack to get access to private members
			class MB : public head
			{
				friend class pool;
			};

			bool pool::allocate_data(head *h, u32 size)
			{
				MB *m = static_cast <MB *> (h);

				u32 i;

				m->_buf = _data->get(i);
				if (unlikely(!m->_buf))
					return false;
				m->_size = data_size(); 

				m->_shared = _shared + i;
				return true;
			}

			void pool::deallocate_data(head *h)
			{
				MB *m = static_cast <MB *> (h);
				_data->put(m->_buf);
			}

			void pool::dump()
			{
				x_printf("Available mbufs %u\n",   _head->avail());
				x_printf("Available buffers %u\n", _data->avail());
			}

			pool::pool(u32 data_size, u32 size, unsigned char factor)
			{
				void * head_mem = allocate_object<mempool>();				// new mempool(sizeof(head), size * factor);
				_head = new (head_mem) mempool();
				_head->init(sizeof(head), size * factor);

				void * data_mem = allocate_object<mempool>();				// new mempool(data_size, size);
				_data   = new (data_mem) mempool();
				_data->init(data_size, size);

				_shared = allocate_array<mbuf::shared>(size);				// new mbuf::shared [size] ();

				_extern = false;
			}

			pool::pool(u32 data_size, u8 *buf, u32 bsize, unsigned char factor)
			{
				u32 size = bsize / data_size;

				void * head_mem = allocate_object<mempool>();				// new mempool(sizeof(head), size * factor);
				_head = new (head_mem) mempool();
				_head->init(sizeof(head), size * factor);

				void * data_mem = allocate_object<mempool>();				// new mempool(data_size, buf, size);
				_data   = new (data_mem) mempool();
				_data->init(data_size, buf, size);

				_shared = allocate_array<mbuf::shared>(size);				// new mbuf::shared [size] ();

				_extern = false;
			}

			pool::pool(mempool *mp, unsigned char factor)
			{
				void * head_mem = allocate_object<mempool>();				// new mempool(sizeof(head), mp->size() * factor);
				_head = new (head_mem) mempool();
				_head->init(sizeof(head), mp->max_size() * factor);

				_shared = allocate_array<mbuf::shared>(mp->max_size());			// new mbuf::shared [mp->size()] ();

				_data   = mp;
				_extern = true;
			}

			pool::~pool()
			{
				deallocate_array(_shared, _data->max_size());
				destruct_object(_head);

				if (!_extern)
					destruct_object(_data);
			}

			bool pool::valid()
			{
				if (_head && _head->valid() && _data && _data->valid() && _shared)
					return true;
				return false;
			}
		} // namespace mbuf
	} // namespace atomic
} // namespace xcore
