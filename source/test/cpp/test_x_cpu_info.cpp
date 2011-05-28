#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xmulticore\x_cpu_info.h"

UNITTEST_SUITE_BEGIN(cpu_info)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() 
		{
			xcore::cpu::xinfo::initialize();
		}

        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(test1)
		{
			xcore::u64 t1 = xcore::cpu::tsc();

			xcore::cpu::xinfo cpuInfo;
			xcore::u64 freq = cpuInfo.getProcessorClockFrequency();
			xcore::s32 lppp = cpuInfo.getLogicalProcessorsPerPhysical();
			const char* family_ID = cpuInfo.getFamilyID();

			xcore::u64 t2 = xcore::cpu::tsc();

			xcore::u64 ns = xcore::cpu::tsc2nsec(t2-t1);
			xcore::u64 us = xcore::cpu::tsc2usec(t2-t1);
		}

	}
}
UNITTEST_SUITE_END
