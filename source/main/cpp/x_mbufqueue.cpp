#include "xatomic\x_compiler.h"
#include "xatomic\x_mbuf.h"
#include "xatomic\x_mbufqueue.h"

namespace xcore
{
	namespace atomic
	{
		namespace mbuf
		{
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
