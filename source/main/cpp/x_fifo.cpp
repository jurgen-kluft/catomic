#include "xatomic\private\x_allocator.h"
#include "xatomic\x_fifo.h"

namespace xcore
{
	namespace atomic
	{
		bool fifo::init(u32 size)
		{
			_size = 0;

			_chain = (link*)get_heap_allocator()->allocate(sizeof(link) * size, 4);
			if (_chain!=NULL)
			{
				_size = size - 1;
				reset();
				return true;
			}
			return false;
		}

		fifo::~fifo()
		{
			if (_chain!=NULL)
			{
				get_heap_allocator()->deallocate(_chain);
				_chain = NULL;
			}
		}

		void fifo::reset(u32 d)
		{
			u32 i;
			for (i=0; i <= _size; i++)
				_chain[i].next = UNUSED;

			// Link in the dummy node
			_chain[d].next = LAST;
			_head.next_salt32.next = d;
			_tail.next_salt32.next = d;

			// Setup salt counters for computing available room
			_tail.next_salt32.salt = 0;
			_head.next_salt32.salt = 0;
		}

		void fifo::fill()
		{
			u32 i;
			for (i=0; i < _size; i++)
				_chain[i].next = i + 1;

			_chain[i].next = LAST;
			_tail.next_salt32.next = i; 
			_head.next_salt32.next = 0;

			_tail.next_salt32.salt = _size;
			_head.next_salt32.salt = 0;
		}

		bool fifo::push(u32 i)
		{
			return ipush(i);
		}

		bool fifo::pop(u32 &i, u32 &r)
		{
			return ipop(i, r);
		}
	} // namespace atomic
} // namespace xcore
