#include "xatomic\x_bitfield.h"

namespace xcore
{
	namespace atomic
	{
		/**
		* Count non-zero bits in a bitfield.
		* @return number of non-zero bits
		*/
		u32 __bitfield::count(const value_type *chunk, u32 nchunks)
		{
			u32 i, c = 0;
			for (i = 0; i < nchunks; i++)
				c += weight(chunk[i]);
			return c;
		}

		/**
		* Find first non-zero bit
		* @return index of the non-zero bit or -1 if not found.
		*/
		s32 __bitfield::ffs(const value_type *chunk, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++) 
			{
				if (chunk[i])
					return ffs(chunk[i]) + (i << SHIFT);
			}
			return -1;
		}

		/**
		* Find first non-zero bit starting at offset
		* @param offset bit offset to start with
		* @return index of the non-zero bit or -1 if not found.
		*/
		s32 __bitfield::ffso(const value_type *chunk, u32 nchunks, u32 offset)
		{
			u32 i = offset >> SHIFT;

			// Check the first chunk.
			// Mask out bits before the start bit.
			value_type d = chunk[i] & (~0UL << (offset & MASK));
			if (d)
				return ffs(d) + (i << SHIFT);

			// Scan the rest of the chunks
			for (++i; i < nchunks; i++)
			{
				if (chunk[i])
					return ffs(chunk[i]) + (i << SHIFT);
			}
			return -1;
		}

		/**
		* Find first zero bit
		* @return index of the non-zero bit or -1 if not found.
		*/
		s32 __bitfield::ffz(const value_type *chunk, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
			{
				if (chunk[i] != ~0UL)
					return ffz(chunk[i]) + (i << SHIFT);
			}
			return -1;
		}

		/**
		* Find first zero bit starting at offset
		* @param offset bit offset to start with
		* @return index of the non-zero bit or -1 if not found.
		*/
		s32 __bitfield::ffzo(const value_type *chunk, u32 nchunks, u32 offset)
		{
			u32 i = offset >> SHIFT;

			// Check the first chunk.
			// Set all bits before the start bit.
			value_type d = chunk[i] | ~(~0UL << (offset & MASK));
			if (d != ~0UL)
				return ffz(d) + (i << SHIFT);

			// Scan the rest of the mask
			for (++i; i < nchunks; i++)
			{
				if (chunk[i] != ~0UL)
					return ffz(chunk[i]) + (i << SHIFT);
			}
			return -1;
		}

		/**
		* Copy bitfield into this bitfield.
		* @param b bitfield to copy from
		*/ 
		void __bitfield::copy(value_type *to, const value_type *from, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				to[i] = from[i];
		}

		/**
		* OR bitfield with this bitfield.
		* @param b bitfield to OR with
		*/ 
		void __bitfield::bor(value_type *to, const value_type *from, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				to[i] |= from[i];
		}

		/**
		* AND bitfield with this bitfield.
		* @param b bitfield to AND with
		*/ 
		void __bitfield::band(value_type *to, const value_type *from, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				to[i] &= from[i];
		}

		/**
		* XOR bitfield with this bitfield.
		* @param b bitfield to XOR with
		*/ 
		void __bitfield::bxor(value_type *to, const value_type *from, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				to[i] ^= from[i];
		}

		/**
		* Copy an inverted bitfield into this bitfield.
		* @param b bitfield to copy
		*/ 
		void __bitfield::invert(value_type *to, const value_type *from, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				to[i] = ~from[i];
		}

		/**
		* Zero out entire bitfield.
		*/ 
		void __bitfield::zero(value_type *chunk, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				chunk[i] = 0;
		}

		/**
		* Fill entire bitfield with ones.
		*/ 
		void __bitfield::fill(value_type *chunk, u32 nchunks)
		{
			u32 i;
			for (i = 0; i < nchunks; i++)
				chunk[i] = ~0UL;
		}
	} // namespace atomic
} // namespace xcore
