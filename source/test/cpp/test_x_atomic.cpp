#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xmulticore\x_atomic.h"

UNITTEST_SUITE_BEGIN(atomic)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct_int32)
		{
			xcore::atomic::int32 i;

			i.add(2);
		}

		UNITTEST_TEST(construct_int64)
		{
			xcore::atomic::int64 i;

			i.add(2);
		}

	}
}
UNITTEST_SUITE_END
