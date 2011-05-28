#include "xmulticore\x_lifo.h"
#include "xmulticore\private\x_allocator.h"

namespace xcore
{
	namespace atomic
	{
		bool lifo::init(u32 size)
		{
			_size = 0;

			_chain = (link*)get_heap_allocator()->allocate(sizeof(link) * size, 4);
			if (_chain!=NULL)
			{
				_size = size;
				reset();
				return true;
			}
			return false;
		}

		lifo::~lifo()
		{
			if (_chain!=NULL)
			{
				get_heap_allocator()->deallocate(_chain);
				_chain = NULL;
			}
		}

		void lifo::reset()
		{
			u32 i;
			for (i=0; i < _size; i++)
				_chain[i].next = ~0U;

			_head.next_salt32.next = _size;
			_head.next_salt32.salt = 0;
		}

		void lifo::fill()
		{
			u32 i;
			for (i=0; i < _size; i++)
				_chain[i].next = i + 1;

			_head.next_salt32.next = 0;
			_head.next_salt32.salt = _size;
		}

		bool lifo::push(u32 i)
		{
			return ipush(i);
		}

		bool lifo::pop(u32 &i)
		{
			return ipop(i);
		}
	} // namespace atomic
} // namespace xcore
