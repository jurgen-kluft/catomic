#include "xbase/x_allocator.h"


namespace xcore
{
	namespace atomic
	{
		static alloc_t* sAllocator = NULL;

		void		x_Init(alloc_t* allocator)
		{
			sAllocator = allocator;
		}

		void		x_Exit()
		{
			sAllocator = NULL;
		}

		alloc_t*	sGetAllocator()
		{
			return sAllocator;
		}

	} // namespace atomic
} // namespace xcore
