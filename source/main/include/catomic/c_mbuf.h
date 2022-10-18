#ifndef __CMULTICORE_MBUF_H__
#define __CMULTICORE_MBUF_H__
#include "cbase/c_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "cbase/c_debug.h"
#include "cbase/c_memory.h"

#include "catomic/private/c_dlist.h"
#include "catomic/c_atomic.h"
#include "catomic/private/c_compiler.h"

namespace ncore
{
	class alloc_t;

	namespace atomic
	{
		namespace mbuf
		{
			class head;

			/**
			* mbuf allocator. Responsible for allocating head and data. 
			* If you want to allocate mbufs using your own memory pool or something 
			* this allocator is your friend.
			*/ 
			class allocator
			{
				alloc_t*	_allocator;
			public:
				allocator(alloc_t* allocator) : _allocator(allocator)		{ }

				virtual			~allocator()									{ }

				/**
				* Allocate both the head and the data buffer.
				* @param The size of the data buffer to be allocated.
				* @return pointer to the newly allocated head object.
				*/
				head*			alloc(u32 size = 128);

				/**
				* Allocate head object (structure).
				* Structure should not be initialized.
				* @param self self pointer
				*/
				inline head*	allocate_head()									{ return alloc_head(); }

				/**
				* Free head object.
				* @param m pointer to the head to free.
				*/
				void			deallocate_head(head *m)						{ free_head(m); }

				/**
				* Allocate data and shared areas.
				* This callback is supposed to allocate and initialize
				* p->buf and p->shared.
				* @param m pointer to the head.
				* @param size size of the data buffer
				*/
				bool			allocate_data(head *m, u32 size)				{ return alloc_data(m, size); }

				/**
				* Free data and shared areas.
				* This callback is supposed to free p->buf and p->shared areas.
				* @param m pointer to the head.
				*/
				void			deallocate_data(head *m)						{ free_data(m); }
			
			protected:
				virtual head*	alloc_head();
				virtual void	free_head(head *m);
				virtual bool	alloc_data(head *m, u32 size);
				virtual void	free_data(head *m);

			};

			/**
			* Allocate new mbuf.
			* @param al Allocator to use to allocate the new object.
			* @return newly allocated mbuf.
			*/
			static inline head *allocate(allocator *al)
			{
				return al->alloc();
			}

			/**
			* Allocate a new head object.
			* @param size of the data buffer to allocate for the new object.
			* @param al Allocator to use to allocate the new object.
			* @return newly allocated head object.
			*/ 
			static inline head *allocate(u32 size, allocator *al)
			{
				return al->alloc(size);
			}

			/**
			* Shared part of the mbuf.
			*/ 
			struct shared 
			{
				u64				timestamp;					///< Timestamp
				u32				context;					///< Context. Owned by exclusive head owner.
				atom_s32		refcnt;						///< Data refcount. Used for cloning

				/**
				* Placement new/delete pair
				*/
				void*		operator new(ncore::xsize_t num_bytes, void* mem)			{ return mem; }
				void		operator delete(void* mem, void* )							{ }

			};

			/**
			* mbuf head which can be used to share memory between different
			* levels of a protocol stack. Allows for headers to be added and removed 
			* in such a way that extra copying of memory is avoided.
			*/ 
			class head : public dlist::node
			{
			protected:
				friend class allocator;

				u8*				_buf;					///< Pointer to the memory buffer
				shared*			_shared;				///< Pointer to the shared area
				atom_s32		_refcnt;				///< The number of references to this object
				u32				_size;					///< Size of the buffer
				u32				_data;					///< Data offset
				u32				_len;					///< Data length
				u16				_flags;					///< Misc flags
				u16				_tag;					///< A tag. Used for tracking purposes.
				allocator*		_allocator;				///< Allocator used in creating this object and to be used when cloning and copying this object

				/**
				* Constructor function. Needs to be private since it should not be used.
				* Only allocators should be used to create new mbufs.
				*/
								head()											{ }

				/**
				* Destructor function. Again this needs to be private since it should not
				* be called. In its place free() should be used.
				*/
								~head()											{ }

				u8*				end()											{ return _buf + _size; }

			public:
				/**
				* Get flags
				*/
				u16				flags() const									{ return _flags; }

				/**
				* Set flags
				*/
				void			flags(u16 flags)								{ _flags = flags; }

				/**
				* Sets a flag in the head.
				* @param flag The flag that is to be set
				*/ 
				void			flags_set(u16 mask)								{ _flags |= mask; }

				/**
				* Clears the flag in the head.
				* @param flag The flag to be cleared.
				*/ 
				void			flags_clear(u16 mask)							{ _flags &= ~mask; }

				/**
				* Test a flag in the head.
				* @param flag The flag that is to be retrieved
				* @param val The value the flag should be set to. 
				*/ 
				bool			flags_test(u16 mask)							{ return (_flags & mask) != 0; }

				/** Set tag */
				void			tag(u16 tag)									{ _tag = tag; }

				/** Get tag */
				u16				tag() const										{ return _tag; }

				/**
				* Set user context.
				* Note that context is shared by all the clones.
				*/
				void			context(u32 ctx)								{ _shared->context = ctx; }

				/** 
				* Get user context.
				* Note that context is shared by all the clones.
				*/
				u32				context() const									{ return _shared->context; }

				/**
				* Get timestamp.
				* Note that timestamp is shared by all the clones.
				* @return timestamp.
				*/ 
				u64				timestamp() const								{ return _shared->timestamp; }

				/**
				* Set timestamp.
				* Note that timestamp is shared by all the clones.
				* @param ts timestamp
				*/ 
				void			timestamp(u64 ts)								{ _shared->timestamp = ts; }

				/**
				* Get next head in the chain.
				*/
				head*			next()											{ return (head *) _next; }

				/**
				* Get pointer to the data.
				* @return Pointer to the beginning data.
				*/
				u8*				data() const									{ return _buf + _data; }

				/**
				* Get length of the valid data (excluding header).
				* @return Data length.
				*/
				u32				len() const										{ return _len; }

				/**
				* Get the size of the head.
				* @return the Calculated size of the head.
				*/ 
				u32				size() const									{ return _size; }

				/**
				* Get a pointer to the tail of the buffer.
				* @return pointer to the tail of the buffer.
				*/ 
				u8*				tail() const									{ return data() + _len; }

				/**
				* Free this object and its buffer if the reference counts are 0.
				*/ 
				void			free()
				{
					if (_refcnt.decr_test())
						return;

					if (!_shared->refcnt.decr_test())
						_allocator->deallocate_data(this);

					_allocator->deallocate_head(this);
				}

				/**
				* Chained version of the @see free.
				* If first head in the chain is freed the rest of the chain is freed. 
				*/ 
				void			free_chain()
				{
					if (_refcnt.decr_test())
						return;

					head *m = this->next();
					if (!_shared->refcnt.decr_test())
						_allocator->deallocate_data(this);
					_allocator->deallocate_head(this);

					head *n = m;
					while (n!=0)
					{
						m = n->next();
						n->free();
						n = m;
					}
				}

				/**
				* Increment the reference count on the head.
				*/ 
				void			hold()
				{
					_refcnt.incr();
				}

				/**
				* Creates a copy of the head object which points to the same data
				* buffer.  Increments reference count accordingly.
				* @return pointer to a new head object with the same information.
				*/
				head*			clone()
				{
					head *mb = _allocator->allocate_head();
					if (likely(mb != 0))
					{
						nmem::memcpy(mb, this, sizeof(*this));
						mb->_refcnt.set(1);
						mb->_shared->refcnt.incr();
					}
					return mb;
				}

				/**
				* Query function for determining whether or not there are clones of this
				* head.  This should be used to determine whether or not it is safe to 
				* write to the buffer.  In general it is not a good idea to write to the
				* shared buffer.
				* @return boolean value indicating whether or not multiple heads point to
				* the same buffer.
				*/
				bool			cloned() const 
				{ 
					u32 c = _shared->refcnt.get();
					return (c - 1) != 0;
				}

				/**
				* Number of users of this head (head refcount)
				*/
				u32				nusers() const									{ return _refcnt.get(); }

				/**
				* Number of times it was cloned (data refcount)
				*/
				u32				nclones() const									{ return _shared->refcnt.get(); }

				/**
				* Resets the data pointers in the buffer.
				*/ 
				void			reset()
				{
					_len  = 0;
					_data = 0;
				}

				/**
				* Increase the size of the memory that is usable in the buffer.
				* @param len The amount to increase by.
				* @return Pointer to the beginning of the new memory "put" in the buffer.
				*/ 
				u8*				put(u32 len)
				{
					if ((_len + len) > _size)
						return 0;

					u8 *data = tail();
					_len += len;
					return data;
				}

				/**
				* Allocates space for and copies that contents of the specified buffer 
				* into the head.
				* @param data pointer to src for data to be copied into head.
				* @param len the size of the buffer to be copied.
				* @return false if there is not enough space, true otherwise
				*/ 
				bool			put(void *data, u32 len)
				{
					u8 *p = put(len);
					if (unlikely(p == 0))
						return false;
					nmem::memcpy(p, data, len);
					return true;
				}

				/**
				* Copies the header and data segment of one head into this head's data
				* segment.
				* @note This is useful when consolidating several heads into one.
				* @param m head to be copied into this one.
				* @return Pointer destination for copy within data segment. NULL if 
				* operation failed.
				*/ 
				bool			put(const head *m)
				{     
					u8 *p = put(m->len());
					if (unlikely(p == 0))
						return false;

					nmem::memcpy(p, m->data(), m->len());
					return true;
				}

				/**
				* Reserves space at the beginning of the buffer.  
				* @param len the amount that is to be reserved.
				*/ 
				void			reserve(u32 len)
				{
					ASSERT((_len + len) < _size);
					_data += len;
				}

				/**
				 * How much head room is left in the buffer.
				 * @return number of bytes available
				 */
				u32				headroom() const
				{
					return _data;
				}

				/**
				 * How much tail room is left in the buffer.
				 * @return number of bytes available
				 */
				u32				tailroom() const
				{
					return _size - _data - _len;
				}

				/**
				 * How much tail room is left in the buffer.
				 * @return number of bytes available
				 */
				u32				room() const
				{
					return tailroom();
				}

				/**
				* Moves the data pointer to increase the amount of memory accessible in 
				* the front of the buffer. Useful when adding headers while traversing 
				* down a protocol stack.
				* @param len The amount to be added to the front.
				* @return pointer to the new beginning of the data buffer.
				*/ 
				u8*				push(u32 len)
				{
					if (len > _data)
						return 0;

					_data -= len;
					_len  += len;
					return data();
				}

				/**
				 * Moves the data pointer to decrease the amount of memory accessible
				 * in the front of the buffer. Useful for traversing up a protocol stack.
				 * @param len the amount to remove from the front.
				 * @return pointer to the start of the removed chunk.
				 */ 
				u8*				pull(u32 len)
				{
					if (len > _len)
						return 0;

					u8* head = data();
					_data += len;
					_len  -= len;
					return head;
				}

				/**
				* Takes a head, and  splits the head into two of len bytes and the 
				* remaining number of bytes.  
				* @param p Pointer to head to be split. The head pointed to by this 
				* pointer will be modified to contain to the last p->len - len bytes. 
				* @param len length of the head to be split from the original.
				* @return Pointer to the head containing the first len bytes of the 
				* original head.
				*/ 
				head*			split(u32 len);

				/**
				* Crops the number of bytes specified from the end of the head.
				* @param number of bytes to be cropped.
				*/ 
				void			crop(u32 len)
				{
					ASSERT(len <= _len);
					_len -= len;
				}

				/** 
				* Sets the length of the head to be that specified.
				*/
				void			resize(u32 len)
				{
					ASSERT(len <= _len);
					_len = len;
				}
			};

		} // namespace mbuf
	} // namespace atomic
} // namespace ncore

#endif // __CMULTICORE_MBUF_H__
