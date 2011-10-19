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

		UNITTEST_TEST(construct3)
		{
			xcore::atomic::queue<xcore::s32>* f = new xcore::atomic::queue<xcore::s32>();
			f->init(gAtomicAllocator, 16);
			CHECK_TRUE(f->valid());

			CHECK_EQUAL(true, f->empty());
			CHECK_EQUAL(16, f->max_size());
			CHECK_EQUAL(16, f->room());

			delete f;
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
			f.push_cancel(i1);

			xcore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);
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

		UNITTEST_TEST(push_full)
		{
			xcore::atomic::queue<xcore::s32> f;
			f.init(gAtomicAllocator, 4);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(4, f.max_size());
			CHECK_EQUAL(4, f.room());

			CHECK_TRUE(f.push(11));
			CHECK_TRUE(f.push(22));
			CHECK_TRUE(f.push(33));
			CHECK_TRUE(f.push(44));
			
			CHECK_EQUAL(4, f.max_size());
			CHECK_EQUAL(0, f.room());

			for (xcore::s32 i=0; i<10; ++i)
				CHECK_FALSE(f.push(i));	// Should not push

			CHECK_EQUAL(4, f.max_size());
			CHECK_EQUAL(0, f.room());
		}

		UNITTEST_TEST(pop_past_empty)
		{
			xcore::atomic::queue<xcore::s32> f;
			f.init(gAtomicAllocator, 4);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(4, f.max_size());
			CHECK_EQUAL(4, f.room());

			CHECK_TRUE(f.push(11));
			CHECK_TRUE(f.push(22));
			CHECK_TRUE(f.push(33));
			CHECK_TRUE(f.push(44));

			CHECK_EQUAL(4, f.max_size());
			CHECK_EQUAL(0, f.room());

			xcore::s32 i;
			CHECK_TRUE(f.pop(i));	// Should pop
			CHECK_EQUAL(11, i);
			CHECK_TRUE(f.pop(i));	// Should pop
			CHECK_EQUAL(22, i);
			CHECK_TRUE(f.pop(i));	// Should pop
			CHECK_EQUAL(33, i);
			CHECK_TRUE(f.pop(i));	// Should pop
			CHECK_EQUAL(44, i);

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(4, f.room());

			for (xcore::s32 i=0; i<10; ++i)
				CHECK_FALSE(f.pop(i));	// Should not pop

			CHECK_EQUAL(4, f.max_size());
			CHECK_EQUAL(4, f.room());
		}

		struct QueueData
		{
			xcore::atomic::fifo::link*		fifo_chain;
			xcore::atomic::lifo::link*		lifo_chain;
			xcore::xbyte*					mempool_buf;
			xcore::atomic::atom_s32*		mempool_buf_eref;

			QueueData()
				: fifo_chain(NULL)
				, lifo_chain(NULL)
				, mempool_buf(NULL)
				, mempool_buf_eref(NULL) { }

			void release()
			{
				gAtomicAllocator->deallocate(mempool_buf_eref);
				gAtomicAllocator->deallocate(mempool_buf);
				gAtomicAllocator->deallocate(lifo_chain);
				gAtomicAllocator->deallocate(fifo_chain);
			}
		};

		static bool sInitializeQueue(xcore::u32 _size, xcore::atomic::queue<xcore::s32>& _queue, QueueData &_queue_data)
		{
			_queue_data.fifo_chain = (xcore::atomic::fifo::link*)gAtomicAllocator->allocate((_size+1) * sizeof(xcore::atomic::fifo::link), 4);
			_queue_data.lifo_chain = (xcore::atomic::lifo::link*)gAtomicAllocator->allocate((_size+1) * sizeof(xcore::atomic::lifo::link), 4);

			xcore::u32 mempool_esize = sizeof(xcore::s32);
			xcore::u32 mempool_size = mempool_esize * (_size + 1);
			_queue_data.mempool_buf = (xcore::xbyte*)gAtomicAllocator->allocate(mempool_size, 4);

			_queue_data.mempool_buf_eref = (xcore::atomic::atom_s32*)gAtomicAllocator->allocate((_size+1) * sizeof(xcore::atomic::atom_s32), 4);

			return _queue.init(_queue_data.fifo_chain, _size+1, _queue_data.lifo_chain, _size+1, _queue_data.mempool_buf, mempool_size, mempool_esize, _queue_data.mempool_buf_eref);
		}

		UNITTEST_TEST(push_begin2)
		{
			xcore::atomic::queue<xcore::s32> f;
			QueueData _queue_data;
			CHECK_TRUE(sInitializeQueue(16, f, _queue_data))
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

			_queue_data.release();
		}

		UNITTEST_TEST(push_cancel2)
		{
			xcore::atomic::queue<xcore::s32> f;
			QueueData _queue_data;
			CHECK_TRUE(sInitializeQueue(16, f, _queue_data))
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
			CHECK_TRUE(sInitializeQueue(16, f, _queue_data))
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
			CHECK_TRUE(sInitializeQueue(16, f, _queue_data))
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

		UNITTEST_TEST(size)
		{
			xcore::atomic::queue<xcore::s32> f;
			CHECK_TRUE(f.init(gAtomicAllocator, 16));

			xcore::s32 what;
			for (xcore::s32 retry=0; retry < 6; ++retry)
			{
				xcore::s32 ti = 0;
				xcore::s32 hi = 0;

				for(xcore::s32 i = 0; i < retry; i++)
				{
					CHECK_TRUE(f.push(hi++));
				}

				CHECK_EQUAL(retry, f.size());
				CHECK_EQUAL(f.max_size(), f.size() + f.room());

				for(xcore::s32 i = 0; i < 10; i++)
				{
					CHECK_TRUE(f.push(hi++));
				}
				CHECK_EQUAL(10+retry, f.size());
				CHECK_EQUAL(f.max_size(), f.size() + f.room());

				for(xcore::s32 i = 0; i < 10; i++)
				{
					CHECK_TRUE(f.pop(what));
					CHECK_EQUAL(ti++, what);
				}
				CHECK_EQUAL(retry, f.size());
				CHECK_EQUAL(f.max_size(), f.size() + f.room());

				for(xcore::s32 i = 0; i < 10; i++)
				{
					CHECK_TRUE(f.push(hi++));
				}

				CHECK_EQUAL(10+retry, f.size());
				CHECK_EQUAL(f.max_size(), f.size() + f.room());

				for(xcore::s32 i = 0; i < 10; i++)
				{
					CHECK_TRUE(f.pop(what));
					CHECK_EQUAL(ti++, what);
				}

				CHECK_EQUAL(retry, f.size());
				CHECK_EQUAL(f.max_size(), f.size() + f.room());

				for(xcore::s32 i = 0; i < retry; i++)
				{
					CHECK_TRUE(f.pop(what));
					CHECK_EQUAL(ti++, what);
				}

				CHECK_EQUAL(0, f.size());
				CHECK_EQUAL(f.max_size(), f.size() + f.room());
			}
		}
	}
}
UNITTEST_SUITE_END
