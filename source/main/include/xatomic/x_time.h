#ifndef __XMULTICORE_TIME_H__
#define __XMULTICORE_TIME_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

namespace xcore
{
	namespace time
	{

		/**
		* Time source
		*/
		struct source 
		{
			u64 (*usec) (void *ctx);
			u64 (*nsec) (void *ctx);
			void      *ctx;
		};

		extern source *_source;

		/**
		* Get current time in microseconds
		*/
		static inline u64 usec()
		{
			// Make sure both func and ctx came from the same 
			// time source in case it was switched under us.
			source *s = _source;
			return s->usec(s->ctx);
		}

		/**
		* Get current time in nanoseconds
		*/
		static inline u64 nsec()
		{
			// Make sure both func and ctx came from the same 
			// time source in case it was switched under us.
			source *s = _source;
			return s->nsec(s->ctx);
		}

		/**
		* Set time source.
		* This drivers time::usec() and time::nsec().
		* Note that nesting (save/restore) is not supported.
		*/
		extern void set_source(source *s);


	} // namespace time
} // namespace xcore

#endif /* BONES_TIME_H */
