#ifndef __XMULTICORE_ALLOCATION_PRIVATE_H__
#define __XMULTICORE_ALLOCATION_PRIVATE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif
#include "xbase\x_types.h"
#include "xmulticore\x_allocator.h"

//==============================================================================
// xmulticore namespace
//==============================================================================
namespace xcore
{
	namespace atomic
	{
		extern xcore::x_iallocator*		get_heap_allocator();

		// The object type needs to support placement new
		template<typename T>
		inline T*	allocate_array(xcore::u32 num_items)
		{
			s32 total_size = num_items * sizeof(T);
			void* _mem = get_heap_allocator()->allocate(total_size, 8);
			T* _array = reinterpret_cast<T*>(_mem);
			for (xcore::u32 i=0; i<num_items; ++i)
			{
				_mem = reinterpret_cast<void*>(&_array[i]);
				new (_mem) T();
			}
			return _array;
		}

		template<typename T>
		inline void	deallocate_array(T* _array, xcore::u32 num_items)
		{
			for (xcore::u32 i=0; i<num_items; ++i)
			{
				T* item = reinterpret_cast<T*>(&_array[i]);
				item->~T();
			}
			get_heap_allocator()->deallocate(_array);
		}

		template<typename T>
		inline void construct_object(T*& p, s32 alignment = 4)
		{
			s32 total_size = sizeof(T);
			void* _mem = get_heap_allocator()->allocate(total_size, alignment);
			p = new (_mem) T();
		}
		
		template<typename T>
		inline void destruct_object(T*& p)
		{
			p->~T();
			get_heap_allocator()->deallocate(p);
			p = NULL;
		}

		template<typename T>
		inline void* allocate_object(s32 alignment = 4)
		{
			s32 total_size = sizeof(T);
			void* _mem = get_heap_allocator()->allocate(total_size, alignment);
			return _mem;
		}

		template<typename T>
		inline void deallocate_object(T*& p)
		{
			get_heap_allocator()->deallocate(p);
			p = NULL;
		}

		#define XMULTICORE_OBJECT_NEW_DELETE(a)																						\
			void*	operator new(xcore::xsize_t num_bytes)				{ return get_heap_allocator()->allocate(num_bytes, a); }	\
			void*	operator new(xcore::xsize_t num_bytes, void* mem)	{ return mem; }												\
			void	operator delete(void* mem)							{ get_heap_allocator()->deallocate(mem); }					\
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
