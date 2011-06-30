#include "xbase\x_allocator.h"

#include "xatomic\x_cpu.h"

namespace xcore
{
	namespace atomic
	{
		class xinfo_init : public cpu::xinfo
		{
		public:
			void init() 
			{
				cpu::xinfo::init(); 
			} 
		};

		class xspeed_init : public cpu::xspeed
		{
		public:
			void init() 
			{
				cpu::xspeed::init(); 
			} 
		};

		void		x_Init()
		{
			// First calculate the processor speed
			xspeed_init si;
			si.init();

			// Then initialize the cpu info
			xinfo_init ii;
			ii.init();
		}

		void		x_Exit()
		{

		}

	} // namespace atomic
} // namespace xcore
