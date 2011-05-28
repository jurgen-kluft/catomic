#include "xbase\x_target.h"
#include "xbase\x_types.h"
#include "xbase\x_allocator.h"
#include "xunittest\xunittest.h"

#include "xmulticore\x_allocator.h"
#include "xmulticore\x_cpu_info.h"

UNITTEST_SUITE_LIST(xMultiCoreUnitTest);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, cpu_info);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, atomic);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, lifo);
UNITTEST_SUITE_DECLARE(xMultiCoreUnitTest, fifo);

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
			return "xthread unittest test heap allocator";
		}

		virtual void*		allocate(s32 size, s32 alignment)
		{
			++mNumAllocations;
			return mAllocator->allocate(size, alignment);
		}

		virtual void*		callocate(s32 n_elems, s32 elem_size)
		{
			++mNumAllocations;
			return mAllocator->callocate(n_elems, elem_size);
		}

		virtual void*		reallocate(void* mem, s32 size, s32 alignment)
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

bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	gSystemAllocator = xcore::gCreateSystemAllocator();
	UnitTestAllocator unittestAllocator( gSystemAllocator );
	UnitTest::SetAllocator(&unittestAllocator);

	xcore::TestHeapAllocator threadHeapAllocator(gSystemAllocator);
	xcore::atomic::set_heap_allocator(&threadHeapAllocator);
	
	int r = UNITTEST_SUITE_RUN(reporter, xMultiCoreUnitTest);
	if (unittestAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xunittest", "memory leaks detected!");
		r = -1;
	}
	if (threadHeapAllocator.mNumAllocations!=0)
	{
		reporter.reportFailure(__FILE__, __LINE__, "xmulticore::heap", "memory leaks detected!");
		r = -1;
	}

	xcore::atomic::set_heap_allocator(NULL);

	UnitTest::SetAllocator(NULL);
	gSystemAllocator->release();
	gSystemAllocator = NULL;

	return r==0;
}

