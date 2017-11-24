#include "xbase/x_allocator.h"

#include "xunittest/xunittest.h"

#include "xatomic/x_ring.h"

extern xcore::x_iallocator* gAtomicAllocator;

UNITTEST_SUITE_BEGIN(ring)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct1)
		{
			xcore::atomic::ring<xcore::s32> f;
			f.init(gAtomicAllocator, 1);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(1, f.max_size());
			CHECK_EQUAL(1, f.room());
		}

		UNITTEST_TEST(construct2)
		{
			xcore::atomic::ring<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}
		
		UNITTEST_TEST(push_begin)
		{
			xcore::atomic::ring<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i1);

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);
		}

		UNITTEST_TEST(push_cancel)
		{
			xcore::atomic::ring<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}

		UNITTEST_TEST(push_commit)
		{
			xcore::atomic::ring<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_commit(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());
			f.push_commit(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(14, f.room());

			f.clear();
			CHECK_FALSE(f.valid());
		}

		UNITTEST_TEST(push)
		{
			xcore::atomic::ring<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			f.push(55);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());
			f.push(77);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(14, f.room());

			f.push(88);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(13, f.room());
			f.push(99);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(12, f.room());

			xcore::s32 i;
			f.pop(i);
			CHECK_EQUAL(55, i);
			f.pop(i);
			CHECK_EQUAL(77, i);
			f.pop(i);
			CHECK_EQUAL(88, i);
			f.pop(i);
			CHECK_EQUAL(99, i);

			f.clear();
			CHECK_FALSE(f.valid());
		}

		struct ringData
		{
			xcore::atomic::ring<xcore::s32>::node*	items;

			ringData()
				: items(NULL)	{ }

			void release()
			{
				gAtomicAllocator->deallocate(items);
			}
		};

		static bool sInitializering(xcore::u32 _size, xcore::atomic::ring<xcore::s32>& _ring, ringData &_ring_data)
		{
			_ring_data.items = (xcore::atomic::ring<xcore::s32>::node*)gAtomicAllocator->allocate((_size+1) * sizeof(xcore::atomic::ring<xcore::s32>::node), 4);
			return _ring.init(_ring_data.items, _size+1);
		}



		UNITTEST_TEST(push_begin2)
		{
			ringData _ring_data;
			xcore::atomic::ring<xcore::s32> f;
			CHECK_TRUE(sInitializering(16, f, _ring_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i1);

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);

			_ring_data.release();
		}

		UNITTEST_TEST(push_cancel2)
		{
			ringData _ring_data;
			xcore::atomic::ring<xcore::s32> f;
			CHECK_TRUE(sInitializering(16, f, _ring_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			_ring_data.release();
		}

		UNITTEST_TEST(push_commit2)
		{
			ringData _ring_data;
			xcore::atomic::ring<xcore::s32> f;
			CHECK_TRUE(sInitializering(16, f, _ring_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_commit(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());
			f.push_commit(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(14, f.room());

			f.clear();
			CHECK_FALSE(f.valid());

			_ring_data.release();
		}

		UNITTEST_TEST(push2)
		{
			ringData _ring_data;
			xcore::atomic::ring<xcore::s32> f;
			CHECK_TRUE(sInitializering(16, f, _ring_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			f.push(55);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());
			f.push(77);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(14, f.room());

			f.push(88);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(13, f.room());
			f.push(99);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(12, f.room());

			xcore::s32 i;
			f.pop(i);
			CHECK_EQUAL(55, i);
			f.pop(i);
			CHECK_EQUAL(77, i);
			f.pop(i);
			CHECK_EQUAL(88, i);
			f.pop(i);
			CHECK_EQUAL(99, i);

			f.clear();
			CHECK_FALSE(f.valid());

			_ring_data.release();
		}
	}
}
UNITTEST_SUITE_END
