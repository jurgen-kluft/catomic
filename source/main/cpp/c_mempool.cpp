#include "cbase/c_allocator.h"
#include "cbase/c_integer.h"
#include "cbase/c_memory.h"

#include "catomic/c_atomic.h"
#include "catomic/c_mempool.h"

namespace ncore
{
	namespace atomic
	{
		mempool::mempool()
		{
			mAllocator = NULL;
			mBuffer = NULL;
			mCsize = 0;
			mExtern = false;
		}

		bool mempool::init(alloc_t* allocator, u32 mempool_esize, u32 size)
		{
			mAllocator = allocator;

			// Initialize the lifo first
			if (!mLifo.init(allocator, size))
				return false;

			u32 csize = xalignUp(mempool_esize, 4);

			mExtern = false;
			mBuffer = (xbyte*)allocator->allocate(csize * size, 4);
			x_memset(mBuffer, 0, csize * size);

			// Caller will have to do the delete anyway. Let the
			// destructor take care of partial allocations.
			if (!mBuffer)
				return false;

			mCsize = csize;

			mLifo.fill();
			return true;
		}

		bool mempool::init(alloc_t* allocator, u32 mempool_esize, xbyte *mempool_buf, u32 mempool_size)
		{
			u32 csize = xalignUp(mempool_esize, 4);

			u32 size = mempool_size / mempool_esize;

			// Initialize the lifo first
			if (!mLifo.init(allocator, size))
				return false;

			// Attach to an external buffer 
			mBuffer = mempool_buf;
			mExtern = true;

			mLifo.fill();
			return true;
		}

		bool	mempool::init(lifo::link* lifo_chain, u32 lifo_size, u32 mempool_esize, u8 *mempool_buf, u32 mempool_size)
		{
			u32 csize = xalignUp(mempool_esize, 4);

			u32 size = mempool_size / csize;
			if (size > lifo_size)
				return false;

			if (!mLifo.init(lifo_chain, lifo_size))
				return false;

			// Attach to an external buffer 
			mBuffer = mempool_buf;
			mCsize  = csize;
			mExtern = true;

			mLifo.fill();
			return true;
		}

		void	mempool::clear()
		{
			if (!mExtern)
			{
				if (mBuffer != NULL)
					mAllocator->deallocate(mBuffer);
				mBuffer = NULL;
			}
			mLifo.clear();
			mCsize = 0;
		}

		mempool::~mempool()
		{
			clear();
		}

		bool mempool::valid()
		{
			return (mLifo.max_size()>0 && mBuffer!=NULL);
		}

// 		u32 mempool::avail() const
// 		{
// 			return mLifo.room();
// 		}
	} // namespace atomic
} // namespace ncore
