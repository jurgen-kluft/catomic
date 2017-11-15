#include "xbase/x_allocator.h"

#include "xatomic/x_lifo.h"

namespace xcore
{
	namespace atomic
	{

		lifo::~lifo()
		{
			clear();
		}

		bool lifo::init(x_iallocator* allocator, u32 size)
		{
			link* c = (link*)allocator->allocate(sizeof(link) * size, 4);
			bool res = init(c, size);
			_allocator = allocator;
			return res;
		}

		bool lifo::init(link* chain, u32 size)
		{
			_allocator = NULL;
			_max_size = 0;
			_chain = chain;
			if (_chain!=NULL)
			{
				_max_size = size;
				reset();
				return true;
			}
			return false;
		}

		void lifo::clear()
		{
			if (_allocator!=NULL)
			{
				_allocator->deallocate(_chain);
				_allocator = NULL;
			}
			_chain = NULL;
			_max_size = 0;

			_head.next_salt64 = 0;
		}

		void lifo::reset()
		{
			for (u32 i=0; i < _max_size; i++)
				_chain[i].next = UNUSED;

			_head.next_salt32.next = _max_size;
			_head.next_salt32.salt = 0;
		}

		void lifo::fill()
		{
			for (u32 i=0; i < _max_size; i++)
				_chain[i].next = i + 1;

			_head.next_salt32.next = 0;
			_head.next_salt32.salt = _max_size;
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
