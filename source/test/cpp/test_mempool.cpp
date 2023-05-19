#include "ccore/c_allocator.h"

#include "cunittest/cunittest.h"

#include "catomic/c_mempool.h"

extern ncore::alloc_t* gAtomicAllocator;

using namespace ncore;
using namespace atomic;

/************************************************************************/
/* mempool is a class doing such thing:
 * requiring a memory pool in order to give out the memory easily and fast
 * faster than new and delete
 *
 * A bug here (probably it is not a bug, ^_^):
 * in mempool class, ("xatomic\source\main\include\xatomic\x_mempool.h")
 * functions size() and avail() may be doing the opposite things 
 * with each other according to the annotation
 * size() returns a number meaning unused chunks
 * while the avail() returns a number meaning used ones.
 */
/************************************************************************/

UNITTEST_SUITE_BEGIN(mempool)
{
	// memory alignment formula
	inline ncore::s32 alignUp(ncore::s32 integer, ncore::s32 alignment = 4)
	{
		return ((integer + (alignment-1)) & (~(alignment-1)));
	}

	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() { }
		UNITTEST_FIXTURE_TEARDOWN() { }

		UNITTEST_TEST(constructor)
		{
			mempool mp;
			CHECK_FALSE(mp.valid());

			mp.init(gAtomicAllocator, 0xff, 0xff);
			CHECK_TRUE(mp.size() != 0);
			CHECK_TRUE(mp.valid());
		}

		UNITTEST_TEST(init1)
		{
			mempool mp;
			CHECK_TRUE(mp.init(gAtomicAllocator, 0xff, 0xff));
			CHECK_TRUE(mp.size() != 0);
			CHECK_EQUAL(alignUp(0xff), mp.chunk_size());
			CHECK_EQUAL(0xff, mp.max_size());
		}

		UNITTEST_TEST(init2)
		{
			mempool mp;
			xbyte* p = (xbyte*)gAtomicAllocator->allocate(sizeof(xbyte), 4);
			mp.init(gAtomicAllocator, 0xff, p, 0xff);
			CHECK_TRUE(mp.size() != 0);
			gAtomicAllocator->deallocate(p);
		}

		UNITTEST_TEST(init3)
		{
			mempool mp;
			ncore::u32 lifo_chain_size = 0x10;
			xbyte* p = (xbyte*)gAtomicAllocator->allocate(sizeof(xbyte), 4);
			ncore::atomic::lifo::link* lifo_chain = (ncore::atomic::lifo::link*)gAtomicAllocator->allocate(lifo_chain_size * sizeof(ncore::atomic::lifo::link), 4);
			
			mp.init(lifo_chain, lifo_chain_size, 0xff, p, 0xff);
			CHECK_TRUE(mp.size() != 0);

			gAtomicAllocator->deallocate(lifo_chain);
			gAtomicAllocator->deallocate(p);
		}

		UNITTEST_TEST(clear)
		{
			mempool mp;
			CHECK_TRUE(mp.init(gAtomicAllocator, 0xff, 0xff));
			CHECK_TRUE(mp.size() != 0);

			xbyte* chunk[21];
			for (int i = 0; i < 20; i++) {
				chunk[i] = mp.get();
				*chunk[i] = (xbyte)i;
			}
			mp.clear();

			CHECK_TRUE(mp.size() == mp.max_size());
		}

		UNITTEST_TEST(GetAndPut)
		{
			mempool mp;
			CHECK_TRUE(mp.init(gAtomicAllocator, 0xff, 0xff));
			CHECK_TRUE(mp.size() != 0);

			// test init begin
			int T = 10;			// test times
			xbyte** chunk;
			chunk = (xbyte**)gAtomicAllocator->allocate((T + 1) * sizeof(xbyte*), 4);
			// test init end

			// get the free chunk from the mp;
			for (int i = 0; i < T; i++) 
			{
				chunk[i] = mp.get();
				CHECK_TRUE(chunk[i] != 0);
			}
			// do some works to the chunks
			for (int i = 0; i < T; i++) {
				*chunk[i] = i;
			}
			// put the chunks back to the mp
			for (int i = 0; i < T; i++) 
			
			{
				ncore::u32 index = 0xffffffff;
				mp.put(chunk[i], index);
				CHECK_EQUAL(i, index);
			}

			gAtomicAllocator->deallocate(chunk);
		}

		UNITTEST_TEST(size)
		{
			mempool mp;
			CHECK_TRUE(mp.init(gAtomicAllocator, 0xff, 0xee));
			CHECK_TRUE(mp.size() != 0);

			CHECK_EQUAL(alignUp(0xff), mp.chunk_size());
			CHECK_EQUAL(0xee, mp.max_size());
			CHECK_EQUAL(mp.max_size(), mp.size());

			xbyte* chunk[21];
			for (int i = 0; i < 20; i++) 
			{
				chunk[i] = mp.get();
				*chunk[i] = (xbyte)i;
				CHECK_EQUAL(mp.max_size() - (i + 1), mp.size());
			}
			for (int i = 0; i < 20; i++) 
			{
				mp.put(chunk[i]);
			}
			CHECK_EQUAL(mp.max_size(), mp.size());
		}

		UNITTEST_TEST(converter)
		{
			mempool mp;
			CHECK_TRUE(mp.init(gAtomicAllocator, 0xff, 0xff));
			CHECK_TRUE(mp.size() != 0);

			xbyte* chunk[21];
			for (int i = 0; i < 20; i++) 
			{
				chunk[i] = mp.get();
				*chunk[i] = (xbyte)i;
			}

			for (int i = 0; i < 20; i++)
			{
				CHECK_EQUAL(i, mp.c2i(chunk[i]));
				CHECK_EQUAL(chunk[i], mp.i2c(i));
			}
		}
	}
}
UNITTEST_SUITE_END