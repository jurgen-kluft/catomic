#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_histogram.h"

UNITTEST_SUITE_BEGIN(histogram)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct)
		{
			xcore::histogram<xcore::u32, xcore::u32, 2000, 0, 10> h;
		}

		UNITTEST_TEST(update)
		{
			xcore::histogram<xcore::u32, xcore::u32, 500, 10, 50> h0;
			for (xcore::s32 i=0; i < 2000; i++)
				h0.update(i);
		}

		UNITTEST_TEST(update1)
		{
			xcore::histogram<xcore::u32, xcore::u32, 2000, 0, 10> hist;
			xcore::u32 samples[] = { 0, 1, 100, 999999, 13, 20000, 1000, 50, 40, 20, 4000, 20, 20, 20, 20, 99 };

			for (xcore::u32 i=0; i < 100; i++)
			{
				for (xcore::u32 n=0; n < sizeof(samples) / sizeof(samples[0]); n++)
				{
					hist.update(samples[n]);
				}
			}
		}

		UNITTEST_TEST(iterate)
		{
			xcore::histogram<xcore::u32, xcore::u32, 2000, 0, 10> hist;
			xcore::u32 samples[] = { 0, 1, 100, 999999, 13, 20000, 1000, 50, 40, 20, 4000, 20, 20, 20, 20, 99 };

			for (xcore::u32 i=0; i < 100; i++)
			{
				for (xcore::u32 n=0; n < sizeof(samples) / sizeof(samples[0]); n++)
				{
					hist.update(samples[n]);
				}
			}

			xcore::u32 count;
			char bin[16];
			for (xcore::u32 i=0; hist.iterate(i, bin, sizeof(bin), count);)
			{
				// TEST ("%6s: %u\n", bin, count);
			}
		}

	}
}
UNITTEST_SUITE_END
