#include "xbase\x_memory_std.h"

#include "xatomic\private\x_allocator.h"
#include "xatomic\x_compiler.h"
#include "xatomic\x_mbuf.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
			allocator def_allocator;

			head* allocator::alloc_head(void)
			{
				return (head*) get_heap_allocator()->allocate(sizeof(head), 4);
			}

			void allocator::free_head(head *h)
			{
				get_heap_allocator()->deallocate(h);
			}

			bool allocator::alloc_data(head *h, u32 size)
			{
				h->_buf = (u8*) get_heap_allocator()->allocate(size + sizeof(mbuf::shared), 4);
				if (!h->_buf)
					return false;
				h->_size = size;

				h->_shared = (mbuf::shared *) (h->_buf + size);
				return true;
			}

			void allocator::free_data(head *h)
			{
				get_heap_allocator()->deallocate(h->_buf);
			}

			head *allocator::alloc(u32 size)
			{
				head *h = alloc_head();
				if (unlikely(!h))
					return 0;

				x_memset(h, 0, sizeof(*h));
				h->_allocator = this;

				bool r = alloc_data(h, size);
				if (unlikely(!r)) 
				{
					free_head(h);
					return 0;
				}

				h->_refcnt.set(1);
				h->_shared->refcnt.set(1);

				h->_data = 0;

				return h;
			}

			head* head::split(u32 len)
			{
				head *h;

				// sanity check the original head to make sure there is enough room
				if (len > _len)
					return 0;

				h = clone();
				if (unlikely(!h))
					return 0;

				// set the len on the new head
				h->_len = len;

				// set the data pointer and len on the old head.
				_len  -= len;
				_data += len;

				return h;
			}

		} // namespace mbuf
	} // namespace atomic
} // namespace xcore
