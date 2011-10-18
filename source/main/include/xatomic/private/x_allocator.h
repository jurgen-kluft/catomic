#ifndef __XMULTICORE_ALLOCATION_PRIVATE_H__
#define __XMULTICORE_ALLOCATION_PRIVATE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif
#include "xbase\x_types.h"
#include "xbase\x_allocator.h"

//==============================================================================
// xatomic namespace
//==============================================================================
namespace xcore
{
	namespace atomic
	{
		extern x_iallocator*	sGetAllocator();

		// The object type needs to support placement new
		template<typename T>
		inline T*	allocate_array(x_iallocator* allocator, xcore::u32 num_items)
		{
			s32 total_size = num_items * sizeof(T);
			void* _mem = allocator->allocate(total_size, 8);
			T* _array = reinterpret_cast<T*>(_mem);
			for (xcore::u32 i=0; i<num_items; ++i)
			{
				_mem = reinterpret_cast<void*>(&_array[i]);
				new (_mem) T();
			}
			return _array;
		}

		template<typename T>
		inline void	deallocate_array(x_iallocator* allocator, T* _array, xcore::u32 num_items)
		{
			for (xcore::u32 i=0; i<num_items; ++i)
			{
				T* item = reinterpret_cast<T*>(&_array[i]);
				item->~T();
			}
			allocator->deallocate(_array);
		}

		template<typename T>
		inline void construct_object(x_iallocator* allocator, T*& p, s32 alignment = 4)
		{
			s32 total_size = sizeof(T);
			void* _mem = allocator->allocate(total_size, alignment);
			p = new (_mem) T();
		}
		
		template<typename T>
		inline void destruct_object(x_iallocator* allocator, T*& p)
		{
			p->~T();
			allocator->deallocate(p);
			p = NULL;
		}

		template<typename T>
		inline void* allocate_object(x_iallocator* allocator, s32 alignment = 4)
		{
			s32 total_size = sizeof(T);
			void* _mem = allocator->allocate(total_size, alignment);
			return _mem;
		}

		template<typename T>
		inline void deallocate_object(x_iallocator* allocator, T*& p)
		{
			allocator->deallocate(p);
			p = NULL;
		}

		#define XATOMIC_OBJECT_NEW_DELETE(a)																						\
			void*	operator new(xcore::xsize_t num_bytes, void* mem)	{ return mem; }												\
			void	operator delete(void* mem, void* )					{ }						

		//==============================================================================
		// END xcore namespace
		//==============================================================================
	}
};

//==============================================================================
// END
//==============================================================================
#endif    /// __XMULTICORE_ALLOCATION_PRIVATE_H__
