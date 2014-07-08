#include "xbase\x_allocator.h"

#include "xunittest\xunittest.h"

#include "xatomic\x_shadow.h"

extern xcore::x_iallocator* gAtomicAllocator;

using namespace xcore;
using namespace atomic;

UNITTEST_SUITE_BEGIN(shadow)
{
	UNITTEST_FIXTURE(main)
	{
		UNITTEST_FIXTURE_SETUP() { }
		UNITTEST_FIXTURE_TEARDOWN() { }

		UNITTEST_TEST(read)
		{
			s32 v;
			shadow<s32> sh(1);
			sh.read(v);
			CHECK_EQUAL(1, v);
		}
		
		UNITTEST_TEST(construct1)
		{
			shadow<s32> sh;
			s32 v;
			sh.read(v);
			CHECK_EQUAL(0, v);
		}

		UNITTEST_TEST(construct2)
		{
			for (s32 i = -500; i < 600; i++) {
				shadow<s32> sh(i);

				s32 v;
				sh.read(v);
				CHECK_EQUAL(i, v);
			}
		}

		UNITTEST_TEST(construct3)
		{
			for (s32 i = -500; i < 600; i++) {
				shadow<s32> sh(i);
				shadow<s32> sh2(sh);

				s32 v;
				sh2.read(v);
				CHECK_EQUAL(i, v);
			}
		}

		UNITTEST_TEST(write)
		{
			for (s32 i = -500; i < 600; i++) {
				shadow<s32> sh;
				sh.write(i);

				s32 v;
				sh.read(v);
				CHECK_EQUAL(i, v);
			}
		}

		UNITTEST_TEST(assignment1)
		{
			for (xcore::s32 i = -500; i < 600; i++) {
				shadow<s32> sh(i);
				shadow<s32> sh2;
				sh2 = sh;
				s32 v;
				sh2.read(v);

				CHECK_EQUAL(i, v);
			}
		} 

		UNITTEST_TEST(assignment2)
		{
			for (s32 i = -500; i < 600; i++) {
				shadow<s32> sh2;
				sh2 = i;
				s32 v;
				sh2.read(v);

				CHECK_EQUAL(i, v);
			}
		}
	}
}
UNITTEST_SUITE_END