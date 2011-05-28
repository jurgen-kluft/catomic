#include "xatomic\x_allocator.h"
#include "xatomic\private\x_allocator.h"

namespace xcore
{
	namespace atomic
	{
		static xcore::x_iallocator*	sHeapAllocator = NULL;

		void						set_heap_allocator(xcore::x_iallocator* allocator)			{ sHeapAllocator = allocator; }
		xcore::x_iallocator*		get_heap_allocator()										{ return sHeapAllocator; }
	}
} // namespace xatomic

