#include "cbase/c_memory.h"

#include "catomic/private/c_allocator.h"
#include "catomic/private/c_compiler.h"
#include "catomic/c_mbuf.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
			head* allocator::alloc_head(void)
			{
				return (head *) _allocator->allocate(sizeof(head), 4);
			}

			void allocator::free_head(head *h)
			{
				_allocator->deallocate(h);
			}

			bool allocator::alloc_data(head *h, u32 size)
			{
				h->_buf = (u8 *) _allocator->allocate(size + sizeof(mbuf::shared), 4);
				if (h->_buf == NULL)
					return false;

				h->_size = size;
				h->_shared = (mbuf::shared *) (h->_buf + size);
				return true;
			}

			void allocator::free_data(head *h)
			{
				_allocator->deallocate(h->_buf);
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
