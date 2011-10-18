#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_atomic.h"

UNITTEST_SUITE_LIST(xAtomicUnitTest);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, atomic);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, lifo);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, fifo);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, stack);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, queue);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, ring);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, shadow);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, mempool);
UNITTEST_SUITE_DECLARE(xAtomicUnitTest, mbufpool);

namespace xcore
{
	class TestHeapAllocator : public x_iallocator
	{
	public:
		TestHeapAllocator(xcore::x_iallocator* allocator)
			: mAllocator(allocator)
		{
		}

		xcore::x_iallocator*	mAllocator;

		virtual const char*	name() const
		{
			return "xthread unittest test heap allocator";
		}

		virtual void*		allocate(u32 size, u32 alignment)
		{
			UnitTest::IncNumAllocations();
			return mAllocator->allocate(size, alignment);
		}

		virtual void*		reallocate(void* mem, u32 size, u32 alignment)
		{
			return mAllocator->reallocate(mem, size, alignment);
		}

		virtual void		deallocate(void* mem)
		{
			UnitTest::DecNumAllocations();
			mAllocator->deallocate(mem);
		}

		virtual void		release()
		{
		}
	};
}

class UnitTestAllocator : public UnitTest::Allocator
{
public:
	xcore::x_iallocator*	mAllocator;

	UnitTestAllocator(xcore::x_iallocator* allocator)
	{
		mAllocator = allocator;
	}

	virtual void*	Allocate(int size)
	{
		return mAllocator->allocate(size, 4);
	}
	virtual void	Deallocate(void* ptr)
	{
		mAllocator->deallocate(ptr);
	}
};

xcore::x_iallocator* gSystemAllocator = NULL;
xcore::x_iallocator* gAtomicAllocator = NULL;

bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	gSystemAllocator = xcore::gCreateSystemAllocator();
	UnitTestAllocator unittestAllocator( gSystemAllocator );
	UnitTest::SetAllocator(&unittestAllocator);

	xcore::TestHeapAllocator threadHeapAllocator(gSystemAllocator);
	gAtomicAllocator = &threadHeapAllocator;
	
	xcore::atomic::x_Init(gAtomicAllocator);
	int r = UNITTEST_SUITE_RUN(reporter, xAtomicUnitTest);
	xcore::atomic::x_Exit();

	gAtomicAllocator = NULL;

	UnitTest::SetAllocator(NULL);
	gSystemAllocator->release();
	gSystemAllocator = NULL;

	return r==0;
}

