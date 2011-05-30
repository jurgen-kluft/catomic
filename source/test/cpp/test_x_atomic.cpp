#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_atomic.h"

UNITTEST_SUITE_BEGIN(atomic)
{
    UNITTEST_FIXTURE(atom_s32)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		typedef xcore::atomic::atom_s32		aint;

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

		UNITTEST_TEST(test_decr)
		{
			aint i(0);
			CHECK_FALSE(i.test_decr());

			i.set(1);
			CHECK_TRUE(i.test_decr());

			i.set(2);
			CHECK_TRUE(i.test_decr());
		}

		UNITTEST_TEST(decr_test)
		{
			aint i(1);
			CHECK_FALSE(i.decr_test());

			i.set(2);
			CHECK_TRUE(i.decr_test());

			i.set(2);
			CHECK_TRUE(i.decr_test());
		}

		UNITTEST_TEST(add)
		{
			aint i(0);
			CHECK_EQUAL(0, i.get());

			for (xcore::s32 x=0; x<32; ++x)
			{
				CHECK_EQUAL(x*3, i.get());
				i.add(3);
			}
		}

		UNITTEST_TEST(sub)
		{
			aint i(32*159);
			CHECK_EQUAL(32*159, i.get());

			for (xcore::s32 x=32*159; x>0; x-=159)
			{
				CHECK_EQUAL(x, i.get());
				i.sub(159);
			}
		}
	}
	
	UNITTEST_FIXTURE(atom_s64)
	{
		UNITTEST_FIXTURE_SETUP() {}
		UNITTEST_FIXTURE_TEARDOWN() {}

		typedef xcore::atomic::atom_s64	aint;

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

		UNITTEST_TEST(decr_test)
		{
			aint i(1);
			CHECK_FALSE(i.decr_test());

			i.set(2);
			CHECK_TRUE(i.decr_test());

			i.set(2);
			CHECK_TRUE(i.decr_test());
		}

		UNITTEST_TEST(add)
		{
			aint i(0);
			CHECK_EQUAL(0, i.get());

			for (xcore::s32 x=0; x<32; ++x)
			{
				CHECK_EQUAL(x*3, i.get());
				i.add(3);
			}
		}

		UNITTEST_TEST(sub)
		{
			aint i(32*159);
			CHECK_EQUAL(32*159, i.get());

			for (xcore::s32 x=32*159; x>0; x-=159)
			{
				CHECK_EQUAL(x, i.get());
				i.sub(159);
			}
		}
	}
}
UNITTEST_SUITE_END
