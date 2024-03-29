#include "ccore/c_allocator.h"
#include "cbase/c_runes.h"
#include "cbase/c_printf.h"

#include "catomic/private/c_compiler.h"
#include "catomic/c_mbufpool.h"

#include "catomic/private/c_allocator.h"

namespace ncore
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

			/**
			 * Hack to get access to private members
			 */
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
				ascii::printf(ascii::crunes("Available mbufs %u\n"),   x_va(_head->size()));
				ascii::printf(ascii::crunes("Available buffers %u\n"), x_va(_data->size()));
			}

	 		pool::pool(alloc_t* allocator, u32 data_size, u32 size, u8 factor)
				: allocator(allocator)
			{
				_allocator = allocator;

				void * head_mem = allocate_object<mempool>(allocator);				// new mempool(sizeof(head), size * factor);
				_head = new (head_mem) mempool();
				_head->init(allocator, sizeof(head), size * factor);

				void * data_mem = allocate_object<mempool>(allocator);				// new mempool(data_size, size);
				_data   = new (data_mem) mempool();
				_data->init(allocator, data_size, size);

				_shared = allocate_array<mbuf::shared>(allocator, size);			// new mbuf::shared [size] ();

				_extern = false;
			}

			pool::pool(alloc_t* allocator, u32 data_size, u8 *buf, u32 bsize, u8 factor)
				: allocator(allocator)
			{
				_allocator = allocator;

				u32 size = bsize / data_size;

				void * head_mem = allocate_object<mempool>(allocator);				// new mempool(sizeof(head), size * factor);
				_head = new (head_mem) mempool();
				_head->init(allocator, sizeof(head), size * factor);

				void * data_mem = allocate_object<mempool>(allocator);				// new mempool(data_size, buf, size);
				_data   = new (data_mem) mempool();
				_data->init(allocator, data_size, buf, size);

				_shared = allocate_array<mbuf::shared>(allocator, size);			// new mbuf::shared [size] ();

				_extern = false;
			}

			pool::pool(alloc_t* allocator, mempool *mp, u8 factor)
				: allocator(allocator)
			{
				_allocator = allocator;

				void * head_mem = allocate_object<mempool>(allocator);				// new mempool(sizeof(head), mp->size() * factor);
				_head = new (head_mem) mempool();
				_head->init(allocator, sizeof(head), mp->max_size() * factor);

				_shared = allocate_array<mbuf::shared>(allocator, mp->max_size());	// new mbuf::shared [mp->size()] ();

				_data   = mp;
				_extern = true;
			}

			pool::~pool()
			{
				deallocate_array(_allocator, _shared, _data->max_size());
				destruct_object(_allocator, _head);

				if (!_extern)
					destruct_object(_allocator, _data);
			}

			bool pool::valid()
			{
				if (_head && _head->valid() && _data && _data->valid() && _shared)
					return true;
				return false;
			}
		} // namespace mbuf
	} // namespace atomic
} // namespace ncore
