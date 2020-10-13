#include "xbase/x_allocator.h"


namespace xcore
{
	namespace atomic
	{
		static xalloc* sAllocator = NULL;

		void		x_Init(xalloc* allocator)
		{
			sAllocator = allocator;
		}

		void		x_Exit()
		{
			sAllocator = NULL;
		}

		xalloc*	sGetAllocator()
		{
			return sAllocator;
		}

	} // namespace atomic
} // namespace xcore
