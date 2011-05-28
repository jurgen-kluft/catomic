#include "xmulticore\x_compiler.h"
#include "xmulticore\x_mbuf.h"
#include "xmulticore\x_mbufqueue.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
			u32 queue::to_iovec(iovec *iv, u32 limit)
			{
				if (!_nbufs || _nbufs > limit || !iv)
					return 0;

				queue::iterator i(this);
				mbuf::head *m;
				for (u32 n=0; (m = i.next()); n++) 
				{
					iv[n].iov_base = m->data();
					iv[n].iov_len  = m->len();
				}

				return _nbufs;
			}

			iovec *queue::to_iovec(u32 *count)
			{
				if (!_nbufs)
					return 0;

				iovec *iv = new iovec [_nbufs] ();
				if (!iv)
					return 0;

				*count = queue::to_iovec(iv, ~0U);
				return iv;
			}

			void queue::tag(u16 tag)
			{
				mbuf::head *m;
				queue::iterator i(this);
				while ((m = i.next()))
					m->tag(tag);
			}

			bool queue::merge(mbuf::head *mbuf)
			{
				mbuf::head *m;
				queue::iterator i(this);
				while ((m = i.next()))
					mbuf->put(m);

				return true;
			}
		} // namespace mbuf
	} // namespace atomic
} // namespace xcore
