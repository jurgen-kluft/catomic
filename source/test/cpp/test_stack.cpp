#include "ccore/c_allocator.h"

#include "cunittest/cunittest.h"

#include "catomic/c_stack.h"

extern ncore::alloc_t* gAtomicAllocator;

UNITTEST_SUITE_BEGIN(stack)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

		UNITTEST_TEST(construct1)
		{
			ncore::atomic::stack<ncore::s32> f;
			f.init(gAtomicAllocator, 1);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(1, f.max_size());
			CHECK_EQUAL(1, f.room());
		}

		UNITTEST_TEST(construct2)
		{
			ncore::atomic::stack<ncore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}
		
		UNITTEST_TEST(push_begin)
		{
			ncore::atomic::stack<ncore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}

		UNITTEST_TEST(push_cancel)
		{
			ncore::atomic::stack<ncore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
		}

		UNITTEST_TEST(push_commit)
		{
			ncore::atomic::stack<ncore::s32> f;
			f.init(gAtomicAllocator, 16);
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_commit(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());

			ncore::s32* i2 = f.push_begin();
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
			ncore::atomic::stack<ncore::s32> f;
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

			ncore::s32 i;
			f.pop(i);
			CHECK_EQUAL(99, i);
			f.pop(i);
			CHECK_EQUAL(88, i);
			f.pop(i);
			CHECK_EQUAL(77, i);
			f.pop(i);
			CHECK_EQUAL(55, i);

			f.clear();
			CHECK_FALSE(f.valid());
		}

		struct StackData
		{
			ncore::atomic::lifo::link*		lifo_chain;
			ncore::atomic::lifo::link*		mempool_lifo_chain;
			ncore::xbyte*					mempool_buf;

			StackData()
				: lifo_chain(NULL)
				, mempool_lifo_chain(NULL)
				, mempool_buf(NULL) { }

			void release()
			{
				gAtomicAllocator->deallocate(mempool_buf);
				gAtomicAllocator->deallocate(mempool_lifo_chain);
				gAtomicAllocator->deallocate(lifo_chain);
			}
		};

		static bool sInitializeStack(ncore::u32 _size, ncore::atomic::stack<ncore::s32>& _stack, StackData &_stack_data)
		{
			_stack_data.lifo_chain = (ncore::atomic::lifo::link*)gAtomicAllocator->allocate(_size * sizeof(ncore::atomic::lifo::link), 4);
			_stack_data.mempool_lifo_chain = (ncore::atomic::lifo::link*)gAtomicAllocator->allocate(_size * sizeof(ncore::atomic::lifo::link), 4);

			ncore::u32 mempool_esize = sizeof(ncore::s32);
			ncore::u32 mempool_size = mempool_esize * _size;
			_stack_data.mempool_buf = (ncore::xbyte*)gAtomicAllocator->allocate(mempool_size, 4);

			return _stack.init(_size, _stack_data.lifo_chain, _stack_data.mempool_lifo_chain, _stack_data.mempool_buf, mempool_size, mempool_esize);
		}



		UNITTEST_TEST(push_begin2)
		{
			StackData _stack_data;
			ncore::atomic::stack<ncore::s32> f;
			CHECK_TRUE(sInitializeStack(16, f, _stack_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			_stack_data.release();
		}

		UNITTEST_TEST(push_cancel2)
		{
			StackData _stack_data;
			ncore::atomic::stack<ncore::s32> f;
			CHECK_TRUE(sInitializeStack(16, f, _stack_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_cancel(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			_stack_data.release();
		}

		UNITTEST_TEST(push_commit2)
		{
			StackData _stack_data;
			ncore::atomic::stack<ncore::s32> f;
			CHECK_TRUE(sInitializeStack(16, f, _stack_data));
			CHECK_TRUE(f.valid());

			CHECK_EQUAL(true, f.empty());
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());

			ncore::s32* i1 = f.push_begin();
			CHECK_NOT_NULL(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(16, f.room());
			f.push_commit(i1);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());

			ncore::s32* i2 = f.push_begin();
			CHECK_NOT_NULL(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(15, f.room());
			f.push_commit(i2);
			CHECK_EQUAL(16, f.max_size());
			CHECK_EQUAL(14, f.room());

			f.clear();
			CHECK_FALSE(f.valid());

			_stack_data.release();
		}

		UNITTEST_TEST(push2)
		{
			StackData _stack_data;
			ncore::atomic::stack<ncore::s32> f;
			CHECK_TRUE(sInitializeStack(16, f, _stack_data));
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

			ncore::s32 i;
			f.pop(i);
			CHECK_EQUAL(99, i);
			f.pop(i);
			CHECK_EQUAL(88, i);
			f.pop(i);
			CHECK_EQUAL(77, i);
			f.pop(i);
			CHECK_EQUAL(55, i);

			f.clear();
			CHECK_FALSE(f.valid());

			_stack_data.release();
		}
	}
}
UNITTEST_SUITE_END
