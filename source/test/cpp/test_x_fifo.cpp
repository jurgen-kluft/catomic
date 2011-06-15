#include "xbase\x_types.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_fifo.h"


struct xnode_dll
{
	xcore::u32		mTicket;
	xnode_dll*		mNext;
	xnode_dll*		mPrev;
	xcore::u32		mData;
};


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
			xcore::atomic::fifo f;
			f.init(16);

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(15, f.room());
			CHECK_EQUAL(15, f.max_size());
		}
		
		UNITTEST_TEST(fill)
		{
			xcore::atomic::fifo f;
			f.init(16);
			f.fill();

			xcore::s32 ii = 1;
			xcore::s32 rr = 0;
			for (xcore::s32 x=0; !f.empty(); ++x)
			{
				CHECK_EQUAL(x, f.room());
				xcore::u32 i,r;
				CHECK_EQUAL(true, f.pop(i, r));
				CHECK_EQUAL(ii, i);
				CHECK_EQUAL(rr, r);
				++ii;
				++rr;
			}
		}

		UNITTEST_TEST(push1_pop1)
		{
			xcore::u32 i, r;
			xcore::atomic::fifo f;
			f.init(16);

			f.reset(0);	// dummy

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(15, f.room());

			xcore::s32 indices[15] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

			xcore::s32 prev = 0;
			xcore::s32 e = 0;
			for (xcore::s32 y=0; y<100; ++y, ++e)
			{
				if (e == 15) 
					e = 0;

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

		UNITTEST_TEST(cursor)
		{
			xcore::u32 i, r;
			xcore::atomic::fifo f;
			f.init(16);

			f.reset(0);	// dummy
			f._head.next_salt32.salt += 0xffffffec;
			f._tail.next_salt32.salt += 0xffffffec;

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(15, f.room());

			xcore::s32 indices[15] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 };

			xcore::s32 prev = 0;
			xcore::s32 e = 0;
			xcore::u32 cursor = 0;
			for (xcore::s32 y=0; y<100; ++y, ++e)
			{
				if (e == 15) e = 0;

				xcore::s32 x = indices[e];

				CHECK_EQUAL(true, f.push(x, cursor));
				CHECK_EQUAL(true, f.inside(cursor));
				CHECK_EQUAL(false, f.empty());

				CHECK_EQUAL(true, f.pop(i,r));
				CHECK_EQUAL(false, f.inside(cursor));
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
