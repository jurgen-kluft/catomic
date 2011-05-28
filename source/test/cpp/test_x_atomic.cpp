#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_atomic.h"

UNITTEST_SUITE_BEGIN(atomic)
{
    UNITTEST_FIXTURE(int32)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		typedef xcore::atomic::int32	aint;

		UNITTEST_TEST(construct)
		{
			aint i;
		}

		UNITTEST_TEST(get)
		{
			aint i;
			CHECK_EQUAL(0, i.get());
			aint i2(2);
			CHECK_EQUAL(2, i2.get());
		}

		UNITTEST_TEST(set)
		{
			aint i;
			CHECK_EQUAL(0, i.get());
			i.set(1);
			CHECK_EQUAL(1, i.get());

			aint i2(2);
			CHECK_EQUAL(2, i2.get());
			i2.set(3);
			CHECK_EQUAL(3, i2.get());
		}

		UNITTEST_TEST(swap)
		{
			aint i;
			CHECK_EQUAL(0, i.get());
			
			CHECK_EQUAL(0, i.swap(1));
			CHECK_EQUAL(1, i.swap(2));
			CHECK_EQUAL(2, i.swap(3));
			CHECK_EQUAL(3, i.get());
		}

		UNITTEST_TEST(incr)
		{
			aint i;
			CHECK_EQUAL(0, i.get());

			for (xcore::s32 x=0; x<32; x++)
			{
				CHECK_EQUAL(x, i.get());
				i.incr();
			}
		}

		UNITTEST_TEST(decr)
		{
			aint i(32);
			CHECK_EQUAL(32, i.get());

			for (xcore::s32 x=32; x>0; --x)
			{
				CHECK_EQUAL(x, i.get());
				i.decr();
			}
		}
	}
	
	UNITTEST_FIXTURE(int64)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		typedef xcore::atomic::int64	aint;

		UNITTEST_TEST(construct)
		{
			aint i;
		}

		UNITTEST_TEST(get)
		{
			aint i;
			CHECK_EQUAL(0, i.get());
			aint i2(2);
			CHECK_EQUAL(2, i2.get());
		}

		UNITTEST_TEST(set)
		{
			aint i;
			CHECK_EQUAL(0, i.get());
			i.set(1);
			CHECK_EQUAL(1, i.get());

			aint i2(2);
			CHECK_EQUAL(2, i2.get());
			i2.set(3);
			CHECK_EQUAL(3, i2.get());
		}

		UNITTEST_TEST(swap)
		{
			aint i;
			CHECK_EQUAL(0, i.get());

			CHECK_EQUAL(0, i.swap(1));
			CHECK_EQUAL(1, i.swap(2));
			CHECK_EQUAL(2, i.swap(3));
			CHECK_EQUAL(3, i.get());
		}

		UNITTEST_TEST(incr)
		{
			aint i;
			CHECK_EQUAL(0, i.get());

			for (xcore::s32 x=0; x<32; x++)
			{
				CHECK_EQUAL(x, i.get());
				i.incr();
			}
		}

		UNITTEST_TEST(decr)
		{
			aint i(32);
			CHECK_EQUAL(32, i.get());

			for (xcore::s32 x=32; x>0; --x)
			{
				CHECK_EQUAL(x, i.get());
				i.decr();
			}
		}
	}
}
UNITTEST_SUITE_END
