#include "catomic/private/c_compiler.h"
#include "catomic/c_mbuf.h"
#include "catomic/private/c_mbufqueue.h"

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
