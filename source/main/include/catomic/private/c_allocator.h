#ifndef __CMULTICORE_ALLOCATION_PRIVATE_H__
#define __CMULTICORE_ALLOCATION_PRIVATE_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "ccore/c_allocator.h"

/**
 * xatomic namespace
 */
namespace ncore
{
	namespace atomic
	{
		extern alloc_t*	sGetAllocator();

		/**
		 * The object type needs to support placement new
		 */
		template<typename T>
		inline T*	allocate_array(alloc_t* allocator, ncore::u32 num_items)
		{
			s32 total_size = num_items * sizeof(T);
			void* _mem = allocator->allocate(total_size, 8);
			T* _array = reinterpret_cast<T*>(_mem);
			for (ncore::u32 i=0; i<num_items; ++i)
			{
				_mem = reinterpret_cast<void*>(&_array[i]);
				new (_mem) T();
			}
			return _array;
		}

		template<typename T>
		inline void	deallocate_array(alloc_t* allocator, T* _array, ncore::u32 num_items)
		{
			for (ncore::u32 i=0; i<num_items; ++i)
			{
				T* item = reinterpret_cast<T*>(&_array[i]);
				item->~T();
			}
			allocator->deallocate(_array);
		}

		template<typename T>
		inline void construct_object(alloc_t* allocator, T*& p, s32 alignment = 4)
		{
			s32 total_size = sizeof(T);
			void* _mem = allocator->allocate(total_size, alignment);
			p = new (_mem) T();
		}
		
		template<typename T>
		inline void destruct_object(alloc_t* allocator, T*& p)
		{
			p->~T();
			allocator->deallocate(p);
			p = NULL;
		}

		template<typename T>
		inline void* allocate_object(alloc_t* allocator, s32 alignment = 4)
		{
			s32 total_size = sizeof(T);
			void* _mem = allocator->allocate(total_size, alignment);
			return _mem;
		}

		template<typename T>
		inline void deallocate_object(alloc_t* allocator, T*& p)
		{
			allocator->deallocate(p);
			p = NULL;
		}

		#define XATOMIC_OBJECT_NEW_DELETE(a)																						\
			void*	operator new(ncore::xsize_t num_bytes, void* mem)	{ return mem; }												\
			void	operator delete(void* mem, void* )					{ }						

		//==============================================================================
		// END ncore namespace
		//==============================================================================
	}
};

//==============================================================================
// END
//==============================================================================
#endif    ///< __CMULTICORE_ALLOCATION_PRIVATE_H__
