#ifndef __XMULTICORE_BITFIELD_H__
#define __XMULTICORE_BITFIELD_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

#include "xatomic\x_compiler.h"
#include "xatomic\private\x_bitfield_base.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Bitfield
		*/
		class bitfield : public __bitfield
		{
		private:
			value_type		_chunk[NCHUNKS];

		public:
			/**
			* Count non-zero bits in a bitfield.
			* @return number of non-zero bits
			*/
			u32 count() const
			{
				return __bitfield::count(_chunk, NCHUNKS);
			}

			/**
			* Find first non-zero bit
			* @return index of the non-zero bit or -1 if not found.
			*/
			s32 ffs() const
			{
				return __bitfield::ffs(_chunk, NCHUNKS);
			}

			/**
			* Find first non-zero bit starting at offset
			* @param offset bit offset to start with
			* @return index of the non-zero bit or -1 if not found.
			*/
			s32 ffso(u32 offset) const
			{
				return __bitfield::ffso(_chunk, NCHUNKS, offset);
			}

			/**
			* Find first zero bit
			* @return index of the non-zero bit or -1 if not found.
			*/
			s32 ffz() const
			{
				return __bitfield::ffz(_chunk, NCHUNKS);
			}

			/**
			* Find first zero bit starting at offset
			* @param offset bit offset to start with
			* @return index of the non-zero bit or -1 if not found.
			*/
			s32 ffzo(u32 offset)
			{
				return __bitfield::ffzo(_chunk, NCHUNKS, offset);
			}

			/**
			* Copy bitfield into this bitfield.
			* @param b bitfield to copy from
			*/ 
			void copy(const bitfield *b)
			{
				__bitfield::copy(_chunk, b->_chunk, NCHUNKS);
			}

			/**
			* Bitwise OR.
			* @param b bitfield to OR with
			*/ 
			void bor(const bitfield *b)
			{
				__bitfield::bor(_chunk, b->_chunk, NCHUNKS);
			}

			/**
			* Bitwise AND.
			* @param b bitfield to AND with
			*/ 
			void band(const bitfield *b)
			{
				__bitfield::band(_chunk, b->_chunk, NCHUNKS);
			}

			/**
			* Bitwise XOR.
			* @param b bitfield to AND with
			*/ 
			void bxor(const bitfield *b)
			{
				__bitfield::bxor(_chunk, b->_chunk, NCHUNKS);
			}

			/**
			* Copy an inverted bitfield into this bitfield.
			* @param b bitfield to copy
			*/ 
			void invert(const bitfield *b)
			{
				__bitfield::invert(_chunk, b->_chunk, NCHUNKS);
			}

			/**
			* Zero out entire bitfield.
			*/ 
			void zero()
			{
				__bitfield::zero(_chunk, NCHUNKS);
			}

			/**
			* Fill entire bitfield with ones.
			*/ 
			void fill()
			{
				__bitfield::fill(_chunk, NCHUNKS);
			}

			/**
			* Set bit n to 1. Non-atomic.
			* @param n bit position to set
			*/ 
			void set(u32 n)
			{
				__bitfield::set(n, _chunk);
			}

			/**
			* Clear bit n. Non-atomic.
			* @param n bit position to clear.
			*/ 
			void clear(u32 n)
			{
				__bitfield::clear(n, _chunk);
			}

			/**
			* Test bit value.
			* @param n bit position to test
			* @return true if bit is set and false otherwise
			*/ 
			bool test(u32 n) const
			{
				return __bitfield::test(n, (value_type*)&_chunk[0]);
			}

			/**
			* Atomic test and clear.
			* @param n bit position
			* @return old value of the bit
			*/
			bool atac(u32 n)
			{
				return __bitfield::atac(n, _chunk);
			}

			/**
			* Non-atomic test and clear.
			* @param n bit position.
			* @return old bit value
			*/ 
			bool tac(u32 n)
			{
				return __bitfield::atac(n, _chunk);
			}

			/**
			* Atomic test and set
			* @param n bit position
			* @return old value of the bit
			*/ 
			bool atas(u32 n)
			{
				return __bitfield::atas(n, _chunk);
			}

			/**
			* Non-atomic test and set.
			* @param n bit position
			* @return old value of the bit
			*/ 
			bool tas(u32 n)
			{
				return __bitfield::tas(n, _chunk);
			}
		}; // class bitfield
	} // namespace atomic
} // namespace xcore


#endif // __XMULTICORE_bitfield_H__
