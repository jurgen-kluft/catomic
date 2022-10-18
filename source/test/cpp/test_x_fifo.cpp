#include "cbase/c_allocator.h"

#include "cunittest/cunittest.h"

#include "catomic/c_fifo.h"


struct xnode_dll
{
	ncore::u32		mTicket;
	xnode_dll*		mNext;
	xnode_dll*		mPrev;
	ncore::u32		mData;
};

extern ncore::alloc_t* gAtomicAllocator;

UNITTEST_SUITE_BEGIN(fifo)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct1)
		{
			ncore::atomic::fifo f;
		}

		UNITTEST_TEST(construct2)
		{
			ncore::atomic::fifo f;
			f.init(gAtomicAllocator, 16);

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.room());
			CHECK_EQUAL(16, f.max_size());
		}
		
		UNITTEST_TEST(fill)
		{
			ncore::atomic::fifo f;
			f.init(gAtomicAllocator, 16);
			f.fill();

			ncore::s32 ii = 1;
			ncore::s32 rr = 0;
			for (ncore::s32 x=0; !f.empty(); ++x)
			{
				CHECK_EQUAL(x, f.room());
				ncore::u32 i,r;
				CHECK_EQUAL(true, f.pop(i, r));
				CHECK_EQUAL(ii, i);
				CHECK_EQUAL(rr, r);
				++ii;
				++rr;
			}
		}

		UNITTEST_TEST(push1_pop1)
		{
			ncore::u32 i, r;
			ncore::atomic::fifo f;
			f.init(gAtomicAllocator, 16);

			f.reset(0);	// dummy

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.room());

			ncore::s32 indices[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

			ncore::s32 prev = 0;
			ncore::s32 e = 0;
			for (ncore::s32 y=0; y<100; ++y, ++e)
			{
				if (e == 16) 
					e = 0;

				ncore::s32 x = indices[e];

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

		UNITTEST_TEST(push_full)
		{
			ncore::atomic::fifo f;
			CHECK_TRUE(f.init(gAtomicAllocator, 4))

			CHECK_TRUE(f.push(0));
			CHECK_TRUE(f.push(1));
			CHECK_TRUE(f.push(2));
			CHECK_TRUE(f.push(3));
		}

		UNITTEST_TEST(cursor)
		{
			ncore::u32 i, r;
			ncore::atomic::fifo f;
			f.init(gAtomicAllocator, 16);

			f.reset(0);	// dummy
			f._head.next_salt32.salt += 0xffffffec;
			f._tail.next_salt32.salt += 0xffffffec;

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.room());

			ncore::s32 indices[16] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };

			ncore::s32 prev = 0;
			ncore::s32 e = 0;
			ncore::u32 cursor = 0;
			for (ncore::s32 y=0; y<100; ++y, ++e)
			{
				if (e == 16) e = 0;

				ncore::s32 x = indices[e];

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
