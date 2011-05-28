#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_fifo.h"

UNITTEST_SUITE_BEGIN(fifo)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct1)
		{
			xcore::atomic::fifo f;
		}

		UNITTEST_TEST(construct2)
		{
			xcore::atomic::fifo f(16);

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(15, f.room());
		}

		UNITTEST_TEST(push1_pop1)
		{
			xcore::u32 i, r;
			xcore::atomic::fifo f(16);

			f.reset(0);	// dummy

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(15, f.room());

			xcore::s32 indices[15] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

			xcore::s32 prev = 0;
			xcore::s32 e = 0;
			for (xcore::s32 y=0; y<100; ++y, ++e)
			{
				if (e == 15) e = 0;

				xcore::s32 x = indices[e];

				CHECK_EQUAL(true, f.push(x));
				CHECK_EQUAL(false, f.empty());

				CHECK_EQUAL(true, f.pop(i,r));
				CHECK_EQUAL(x, i);
				CHECK_EQUAL(prev, r);
				CHECK_EQUAL(true, f.empty());

				indices[e] = r;
				prev = x;
			}
		}
	}
}
UNITTEST_SUITE_END
