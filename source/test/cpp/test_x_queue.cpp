#include "xbase\x_types.h"
#include "xbase\x_allocator.h"

#include "xunittest\xunittest.h"

#include "xatomic\x_queue.h"

extern xcore::x_iallocator* gAtomicAllocator;

UNITTEST_SUITE_BEGIN(queue)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct1)
		{
			xcore::atomic::queue<xcore::s32> f;
			f.init(gAtomicAllocator, 1);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(1, f.max_size());
			CHECK_EQUAL(1, f.room());
		}

		UNITTEST_TEST(construct2)
		{
			xcore::atomic::queue<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}
		

		UNITTEST_TEST(push_begin)
		{
			xcore::atomic::queue<xcore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}

		UNITTEST_TEST(push_cancel)
		{
			xcore::atomic::queue<xcore::s32> f;
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
			xcore::atomic::queue<xcore::s32> f;
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
			xcore::atomic::queue<xcore::s32> f;
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



		struct QueueData
		{
			xcore::atomic::fifo::link*		fifo_chain;
			xcore::atomic::lifo::link*		lifo_chain;
			xcore::xbyte*					mempool_buf;

			QueueData()
				: fifo_chain(NULL)
				, lifo_chain(NULL)
				, mempool_buf(NULL) { }

			void release()
			{
				gAtomicAllocator->deallocate(mempool_buf);
				gAtomicAllocator->deallocate(lifo_chain);
				gAtomicAllocator->deallocate(fifo_chain);
			}
		};

		static bool sInitializeQueue(xcore::u32 _size, xcore::atomic::queue<xcore::s32>& _queue, QueueData _queue_data)
		{
			_queue_data.fifo_chain = (xcore::atomic::fifo::link*)gAtomicAllocator->allocate((_size+1) * sizeof(xcore::atomic::fifo::link), 4);
			_queue_data.lifo_chain = (xcore::atomic::lifo::link*)gAtomicAllocator->allocate((_size+1) * sizeof(xcore::atomic::lifo::link), 4);

			xcore::u32 mempool_esize = sizeof(xcore::s32);
			xcore::u32 mempool_size = mempool_esize * (_size + 1);
			_queue_data.mempool_buf = (xcore::xbyte*)gAtomicAllocator->allocate(mempool_size, 4);

			return _queue.init(_queue_data.fifo_chain, _size+1, _queue_data.lifo_chain, _size+1, _queue_data.mempool_buf, mempool_size, mempool_esize);
		}
		UNITTEST_TEST(push_begin2)
		{
			xcore::atomic::queue<xcore::s32> f;
			QueueData _queue_data;
			sInitializeQueue(16, f, _queue_data);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			_queue_data.release();
		}

		UNITTEST_TEST(push_cancel2)
		{
			xcore::atomic::queue<xcore::s32> f;
			QueueData _queue_data;
			sInitializeQueue(16, f, _queue_data);
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

			_queue_data.release();
		}

		UNITTEST_TEST(push_commit2)
		{
			xcore::atomic::queue<xcore::s32> f;
			QueueData _queue_data;
			sInitializeQueue(16, f, _queue_data);
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

			_queue_data.release();
		}

		UNITTEST_TEST(push2)
		{
			xcore::atomic::queue<xcore::s32> f;
			QueueData _queue_data;
			sInitializeQueue(16, f, _queue_data);
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
			
			_queue_data.release();
		}


	}
}
UNITTEST_SUITE_END
