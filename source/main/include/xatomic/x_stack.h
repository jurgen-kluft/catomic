#ifndef __XMULTICORE_STACK_H__
#define __XMULTICORE_STACK_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xmulticore\x_compiler.h"
#include "xmulticore\x_lifo.h"
#include "xmulticore\x_mempool.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Multi-reader, multi-writer lock-free stack.
		*/
		template <typename T>
		class stack
		{
		private:
			mempool _items;
			lifo    _lifo;

		public:
			/**
			* Constructor. Allocates the stack.
			* @param size number of items in the stack
			*/
						stack(u32 size) 
							: _items(sizeof(T), size)
							, _lifo(size)											{ }

			/**
			* Stack size.
			* @return number of items
			*/
			u32			size() const												{ return _lifo.size(); }

			/**
			* Check if stack is empty.
			* @return true if stack is empty, false otherwise
			*/
			bool		empty() const												{ return _lifo.empty(); }

			u32			room() const												{ return _lifo.room(); }

			// ---- POP interface ----

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
				ASSERT(lp && "Corrupted state");
			}

			/**
			* Push data onto the stack.
			* One shot push.
			* @param[in] data data to push
			*/
			bool		push(T *data)
			{
				// Open coded push_begin() -> copy -> push_commit()
				// transaction

				u32 i;
				T *p = (T *) _items.get(i);

				if (unlikely(!p))
					return false;

				*p = *data;

				bool lp = _lifo.push(i);
				ASSERT(lp && "Corrupted state");

				return true;
			}

			bool		push(T data)											{ return push(&data); }

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
			void		pop_finish(T *p)										{ _items.put((u8 *) p); }

			/**
			* Pop data from the stack.
			* One shot pop.
			* @param[out] pointer to the data
			* @return true on success, false otherwise
			*/
			bool		pop(T *data)
			{
				// Open coded pop_begin() -> copy -> pop_finish()
				// transaction.

				u32 i;
				if (!_lifo.pop(i))
					return false;

				T *p = (T *) _items.i2c(i);
				*data = *p;

				_items.put(i);
				return true;
			}

			/**
			* Validate stack.
			* Used for checking for constructor failures.
			*/
			bool		valid()														{ return (_lifo.size() != 0 && _items.valid()); }
		};

	} // namespace atomic

} // namespace xcore

#endif // __XMULTICORE_STACK_H__
