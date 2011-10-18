#include "xbase\x_allocator.h"


namespace xcore
{
	namespace atomic
	{
		static x_iallocator*	sAllocator = NULL;
		void		x_Init(x_iallocator* allocator)
		{
			sAllocator = allocator;
		}

		void		x_Exit()
		{
			sAllocator = NULL;
		}

		x_iallocator*	sGetAllocator()
		{
			return sAllocator;
		}

	} // namespace atomic
} // namespace xcore
