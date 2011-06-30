#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_cpu.h"

UNITTEST_SUITE_BEGIN(cpu_info)
{
    UNITTEST_FIXTURE(speed)
    {
        UNITTEST_FIXTURE_SETUP() 
		{
		}

        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(test1)
		{
			xcore::u64 t1 = xcore::cpu::tsc();

			xcore::s32 a = 0;
			for (xcore::s32 i=0; i<1000000; ++i)
			{
				a = a + (i * 3 / 33);
			}
			CHECK_EQUAL(0x9545b10f, a);

			xcore::u64 t2 = xcore::cpu::tsc();

			xcore::u64 ns = xcore::cpu::tsc2nsec(t2-t1);
			xcore::u64 us = xcore::cpu::tsc2usec(t2-t1);

			CHECK_TRUE(ns > 1);
			CHECK_TRUE(ns > us);

			CHECK_EQUAL(ns / 1000, us);
		}
	}

	UNITTEST_FIXTURE(info)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(check)
		{
			xcore::cpu::xinfo cpuInfo;

			xcore::u64 freq = cpuInfo.getProcessorClockFrequency();
			CHECK_TRUE(freq > 1);

			xcore::s32 lppp = cpuInfo.getLogicalProcessorsPerPhysical();
			CHECK_TRUE(lppp >= 1);
		}
	}
}
UNITTEST_SUITE_END
