#include "xatomic\x_compiler.h"
#include "xatomic\x_mbuf.h"
#include "xatomic\private\x_mbufqueue.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
			void queue::tag(u16 tag)
			{
				queue::iterator i(this);

				mbuf::head *m = i.next();
				while (m!=NULL)
				{
					m->tag(tag);
					m = i.next();
				}
			}

			bool queue::merge(mbuf::head *mbuf)
			{
				queue::iterator i(this);
				mbuf::head *m = i.next();
				while (m!=NULL)
				{
					mbuf->put(m);
					m = i.next();
				}

				return true;
			}
		} // namespace mbuf
	} // namespace atomic
} // namespace xcore
