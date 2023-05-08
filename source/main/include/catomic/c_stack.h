#ifndef __CMULTICORE_STACK_H__
#define __CMULTICORE_STACK_H__
#include "ccore/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "cbase/c_allocator.h"

#include "catomic/private/c_allocator.h"
#include "catomic/private/c_compiler.h"
#include "catomic/c_lifo.h"
#include "catomic/c_mempool.h"

namespace ncore
{
	class alloc_t;

	namespace atomic
	{
		/**
		* Multi-reader, multi-writer lock-free stack.
		*/
		template <typename T>
		class stack
		{
		private:
			mempool		_items;
			lifo		_lifo;

		public:
			DCORE_CLASS_NEW_DELETE(sGetAllocator, 4)

						stack() {}

			/**
			* Init. Allocates the stack.
			* @param size number of items in the stack
			*/
			bool		init(alloc_t* allocator, u32 size) 
			{
				bool r = false;
				if (_items.init(allocator, sizeof(T), size))
					r = _lifo.init(allocator, size); 
				return r;
			}

			bool		init(u32 stack_size, lifo::link* lifo_chain, lifo::link* mempool_lifo_chain, xbyte *mempool_buf, u32 mempool_buf_size, u32 mempool_buf_esize)
			{
				if (!_items.init(mempool_lifo_chain, stack_size, mempool_buf_esize, mempool_buf, mempool_buf_size))
				{
					clear();
					return false;
				}
				if (!_lifo.init(lifo_chain, stack_size))
				{
					clear();
					return false;
				}

				if (!valid())
				{
					clear();
					return false;
				}

				return true;
			}

			/**
			* Clear stack, deallocates all memory, need to call init again.
			*/
			void		clear()
			{
				_items.clear();
				_lifo.clear();
			}

			/**
			* Stack size.
			* @return number of items
			*/
			u32			max_size() const											{ return _lifo.max_size(); }

			/**
			* Check if stack is empty.
			* @return true if stack is empty, false otherwise
			*/
			bool		empty() const												{ return _lifo.empty(); }

			/**
			* Number of unused elements.
			* @return approximate number of unused elements
			* @warning this method is slow and inefficient
			*/
			u32			room() const												{ return _lifo.room(); }

			/**
			* Number of used elements.
			* @return approximate number of used elements
			*/
			u32			size() const												{ return _lifo.size(); }

			// ---- PUSH interface ----

			/**
			* Begin push transaction. Get free item from the pool.
			* @return pointer to the beginning if the item, or 0
			* if there is no space.
			*/
			T*			push_begin()												{ return (T *) _items.get(); }

			/**
			* Cancel push transaction.
			* Item must have been obtained via push_begin().
			* @param[in] item pointer to an item
			*/
			void		push_cancel(T *p)											{ _items.put((u8 *) p); }

			/**
			* Commit push transaction. Push an item onto the stack.
			* Item must have been obtained via push_begin().
			* @param[in] item pointer to an item
			*/
			void		push_commit(T *p)
			{
				u32 i = _items.c2i((u8 *) p);
				bool lp = _lifo.push(i);
				ASSERTS(lp, "xatomic::stack<T>: Error, state is corrupted!");
			}

			/**
			* Push data onto the stack.
			* One shot push.
			* @param[in] data data to push
			*/
			bool		push(T const& inData)
			{
				// Open coded push_begin() -> copy -> push_commit()
				// transaction

				u32 i;
				T *p = (T *) _items.get(i);

				if (unlikely(!p))
					return false;

				*p = inData;

				bool lp = _lifo.push(i);
				ASSERTS(lp, "xatomic::stack<T>: Error, state is corrupted!");

				return true;
			}

			// ---- POP interface ----

			/**
			* Begin pop transaction. Pop an item from the stack.
			* @return pointer to an item or zero if the stack is empty
			*/
			T*			pop_begin()
			{
				u32 i;
				if (_lifo.pop(i))
					return (T*) _items.i2c(i);
				return 0;
			}

			/**
			* Finish push transaction.
			* @param[in] item pointer to an item obtained with push_begin()
			*/
			void		pop_finish(T *p)
			{
				_items.put((u8 *) p); 
			}

			/**
			* Pop data from the stack.
			* One shot pop.
			* @param[out] pointer to the data
			* @return true on success, false otherwise
			*/
			bool		pop(T& outData)
			{
				// Open coded pop_begin() -> copy -> pop_finish()
				// transaction.

				u32 i;
				if (!_lifo.pop(i))
					return false;

				T *p = (T *) _items.i2c(i);
				outData = *p;

				_items.put(i);
				return true;
			}

			/**
			* Validate stack.
			* Used for checking for constructor failures.
			*/
			bool		valid()
			{
				return (_lifo.valid() && _items.valid());
			}
		};

	} // namespace atomic

} // namespace ncore

#endif // __CMULTICORE_STACK_H__
