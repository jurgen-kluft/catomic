#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xunittest\xunittest.h"

#include "xatomic\x_atomic.h"
#include "xatomic\x_cpu.h"

UNITTEST_SUITE_LIST(xMultiCoreUnitTest);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, cpu_info);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, atomic);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, lifo);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, fifo);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, stack);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, queue);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, ring);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, shadow);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, mempool);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, mbufpool);

namespace xcore
{
	class TestHeapAllocator : public x_iallocator
	{
	public:
		TestHeapAllocator(xcore::x_iallocator* allocator)
			: mAllocator(allocator)
			, mNumAllocations(0)
		{
		}

		xcore::x_iallocator*	mAllocator;
		s32						mNumAllocations;

		virtual const char*	name() const
		{
			return "xthre unittest test heap allocator";
		}

		virtual void*		allocate(u32 size, u32 alignment)
		{
			++mNumAllocations;
			return mAllocator->allocate(size, alignment);
		}

		virtual void*		reallocate(void* mem, u32 size, u32 alignment)
		{
			return mAllocator->reallocate(mem, size, alignment);
		}

		virtual void		deallocate(void* mem)
		{
			--mNumAllocations;
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
	int						mNumAllocations;

	UnitTestAllocator(xcore::x_iallocator* allocator)
		: mNumAllocations(0)
	{
		mAllocator = allocator;
	}

	virtual void*	Allocate(int size)
	{
		++mNumAllocations;
		return mAllocator->allocate(size, 4);
	}
	virtual void	Deallocate(void* ptr)
	{
		--mNumAllocations;
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
	
	xcore::atomic::x_Init();
	int r = UNITTEST_SUITE_RUN(reporter, xMultiCoreUnitTest);
	xcore::atomic::x_Exit();

	if (unittestAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xunittest", "memory leaks detected!");
		r = -1;
	}
	if (threadHeapAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xatomic::heap", "memory leaks detected!");
		r = -1;
	}

	gAtomicAllocator = NULL;

	UnitTest::SetAllocator(NULL);
	gSystemAllocator->release();
	gSystemAllocator = NULL;

	return r==0;
}

