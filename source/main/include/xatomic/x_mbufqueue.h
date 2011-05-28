#ifndef __XMULTICORE_MBUF_QUEUE_H__
#define __XMULTICORE_MBUF_QUEUE_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"
#include "xbase\x_debug.h"

#include "xmulticore\x_mbuf.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
			/**
			* Queue of mbufs.
			* Not thread safe.
			*/
			class queue : public dlist::head
			{
			private:
				/** Number of mbufs in the queue */
				u32			_nbufs;

				/** 
				* Total amount of data contained in the heads which are in this queue.
				*/
				u32			_nbytes;

			public:
				typedef dlist::iterator<mbuf::head> iterator;

				/**
				* Constructor function which does the necessary initialization.
				*/ 
							queue(void)												{ reset(); }

				/**
				* Seems like you would always want to purge the head Queue when deleting
				* it.
				*/ 
							~queue()												{ purge(); }

				/**
				* Number of mbufs in the queue
				*/
				u32			nbufs() const											{ return _nbufs; }
				u32			len()   const											{ return _nbufs; }

				/**
				* Number of bytes in the queue
				*/
				u32			nbytes() const											{ return _nbytes; }

				/**
				* Resets the queue to be empty. Doesn't do anything with the contents of
				* the queue, it just leaves them there.
				*/ 
				void		reset(void)
				{
					dlist::head::reset();
					_nbufs  = 0;
					_nbytes = 0;
				}

				/**
				* Insert head object into the tail of the queue.
				* @param mb Pointer to the head object to be added.
				*/ 
				void		append(mbuf::head *mb)
				{
					dlist::head::append(mb);
					_nbufs++;
					_nbytes += mb->len();
				}

				/**
				* Insert head object into the front of the queue.
				* @param mb Pointer to the head object to be added.
				*/ 
				void		prepend(mbuf::head *mb)
				{
					dlist::head::add(mb);
					_nbufs++;
					_nbytes += mb->len();
				}

				/**
				* Delete head object from queue.
				* @param mb Pointer to the head object to be removed from the queue..
				*/ 
				void		del(mbuf::head *mb)
				{
					dlist::head::del(mb);
					_nbufs--;
					_nbytes -= mb->len();
				}

				/**
				* Get the next entry in the queue.
				* @return Pointer to next head object in the queue.
				*/ 
				mbuf::head*	head(void) const
				{
					if (empty())
						return 0;
					return (mbuf::head *) next();
				}

				/**
				* Remove an element from the queue and return it.
				* @return Pointer to the head object which was dequeued.
				*/ 
				mbuf::head*	deque(void)
				{
					if (empty())
						return 0;

					mbuf::head *mb = (mbuf::head *) next();
					del(mb);
					return mb;
				}

				/**
				* Purge the queue freeing all head objects contained in it.
				*/ 
				void		purge()
				{
					mbuf::head *mb;
					while ((mb = deque()))
						mb->free();
				}

				/** Set tag  for all heads in the queue */
				void		tag(u16 tag);

				/**
				* Get an iovec representation of this head_queue.
				* @warn Using preallocated iovecs is desirable in many situations.
				* For such situations, use populateIovec instead.
				* @param[out] iv Pointer to an iovec pointer to be set to the iovec 
				* created from this queue.
				* @param[out] count pointer to an int which will be set with the size
				* of the iovec.
				* @return boolean indicating success.
				*/ 
				iovec*		to_iovec(u32 *count);

				/**
				* Populates an iovec from this head_queue.
				* @param[out] iv pointer to the iovec to populate 
				* @param niv number of entries on the iovec
				* @return number of iovec entries populated
				*/
				u32			to_iovec(struct iovec *iv, u32 niv);

				/**
				* Consolidates mbuf queue into a single mbuf. It is up to the caller
				* to allocate an head large enough to handle contents of queue.
				* @param m pointer to the mbuf to consolidated into
				* @return true on success, false on failure
				*/
				bool		merge(mbuf::head *m);
			}; 

		} // namespace mbuf
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_MBUF_QUEUE_H__
