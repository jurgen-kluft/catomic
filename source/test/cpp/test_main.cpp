#include "cbase/c_target.h"
#include "cbase/c_allocator.h"
#include "cunittest/cunittest.h"

#include "catomic/c_atomic.h"

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
	class TestHeapAllocator : public alloc_t
	{
	public:
							TestHeapAllocator(xcore::alloc_t* allocator) : mAllocator(allocator) { 		}

		virtual void*		v_allocate(u32 size, u32 alignment)
		{
			UnitTest::IncNumAllocations();
			return mAllocator->allocate(size, alignment);
		}

		virtual u32			v_deallocate(void* mem)
		{
			UnitTest::DecNumAllocations();
			return mAllocator->deallocate(mem);
		}

		virtual void		v_release()		{ }
	private:
		xcore::alloc_t*	mAllocator;
	};
}

class UnitTestAllocator : public UnitTest::Allocator
{
public:
					UnitTestAllocator(xcore::alloc_t* allocator) : mAllocator(allocator) {}
	virtual void*	Allocate(size_t size)		{ return mAllocator->allocate((xcore::u32)size, 4);}
	virtual size_t	Deallocate(void* ptr)		{ return mAllocator->deallocate(ptr); }
private:
	xcore::alloc_t*	mAllocator;
};

xcore::alloc_t* gSystemAllocator = NULL;
xcore::alloc_t* gAtomicAllocator = NULL;

bool gRunUnitTest(UnitTest::TestReporter& reporter)
{
	gSystemAllocator = xcore::alloc_t::get_system();
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

