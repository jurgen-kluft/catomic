#ifndef __XMULTICORE_MBUFPOOL_H__
#define __XMULTICORE_MBUFPOOL_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xatomic\x_mempool.h"
#include "xatomic\x_mbuf.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{

			/**
			* Pool of fixed size mbufs.
			* Thread safe (based on bones::lifo).
			*/
			class pool : public allocator
			{
			protected:
				virtual head*	allocate_head(void);
				virtual bool	allocate_data(head *m, u32 size);
				virtual void	deallocate_head(head *m);
				virtual void	deallocate_data(head *m);

				mempool*		_head;
				mempool*		_data;
				shared*			_shared;

				// External buffer
				bool			_extern;

			public:
				/**
				* Create pool of buffers.
				* @param dsize size of each data buffer
				* @param size number of data buffers
				* @param factor a factor applied to count for total number heads.
				*/
							pool(u32 dsize, u32 size, unsigned char factor = 4U);

				/**
				* Convert existing chunk of memory into mbuf::pool.
				* @param dsize size of each data buffer
				* @param buf external buffer
				* @param bsize size of the external buffer
				* @param factor a factor applied to count for total number heads.
				*/
							pool(u32 dsize, u8 *buf, u32 bsize, unsigned char factor = 4U);

				/**
				* Convert existing memory pool into mbuf::pool.
				* @param factor a factor applied to count for total number heads.
				*/
							pool(mempool *mp, unsigned char factor = 4U);

				/**
				* Destruct memory pool
				*/
				virtual		~pool();

				/**
				* Allocate an mbuf from the pool.
				* Use mbuf::head::free() to put the buffer back into the pool.
				* @return pointer to the mbuf::head
				*/
				head*		get()													{ return allocator::alloc(); }
				u32			data_size() const										{ return _data->chunk_size(); }
				u32			max_size()  const										{ return _data->max_size(); }
				u32			avail() const											{ return _data->avail(); }

				/**
				* Validate mbufpool.
				* Used for checking constructor failures.
				*/
				bool		valid();

				void		dump();
			};
		} // namespace mbuf
	} // namespace atomic
} // namespace xcore

#endif // __XMULTICORE_MBUFPOOL_H__
