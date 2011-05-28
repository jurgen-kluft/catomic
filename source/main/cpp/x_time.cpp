#include "xmulticore\x_cpu.h"
#include "xmulticore\x_time.h"
#include "xmulticore\x_barrier.h"

namespace xcore
{
	namespace time
	{
		static u64 __usec(void *)
		{
			// implementations needed

			return (u64)0;
		}

		static u64 __nsec(void *)
		{
			return __usec(0) * 1000;
		}

		static source def_source = { __usec, __nsec, 0 };

		// Pointer to the source
		source *_source = &def_source;

		void set_source(source *s)
		{
			if (!s)
				s = &def_source;

			// Make sure updates to the time source object are flushed
			barrier::memrw();

			// There are no threading concerns as far as switching the time source goes.
			// But the source itself must be thread safe of course.
			_source = s;
		}

	} // namespace time
} // namespace xcore
