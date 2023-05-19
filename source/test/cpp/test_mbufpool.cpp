#include "ccore/c_allocator.h"

#include "cunittest/cunittest.h"

#include "catomic/c_mbufpool.h"
#include "catomic/c_mbuf.h"

extern ncore::alloc_t* gAtomicAllocator;

// #include <iostream>
// using namespace std;

using namespace ncore;
using namespace atomic;
using namespace mbuf;

UNITTEST_SUITE_BEGIN(mbufpool)
{
	inline ncore::s32 alignUp(ncore::s32 integer, ncore::s32 alignment = 4) 
	{
		return ((integer + (alignment-1)) & (~(alignment-1)));
	}

	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() { }
		UNITTEST_FIXTURE_TEARDOWN() { }

		UNITTEST_TEST(constructor1)
		{
			pool pl(gAtomicAllocator, 0xff, 0xee);
			CHECK_TRUE(pl.valid());
		}
	}
}
UNITTEST_SUITE_END