#ifndef __XMULTICORE_HISTOGRAM_H__
#define __XMULTICORE_HISTOGRAM_H__
#include "xbase\x_target.h"
#ifdef USE_PRAGMA_ONCE 
#pragma once 
#endif

#include "xbase\x_types.h"

namespace xcore
{

	/**
	* Simple histogram.
	* @param Td type used for data variables (min/max, etc)
	* @param Tc type used for counter variables
	* @param HIGH uppder bound
	* @param LOW lower bound
	* @param _NBINS number of bins. Note that two additional bins are added.
	* @param SCALE number of scaler bits used for divisions
	*/
	template <typename Td, typename Tc, u32 HIGH, u32 LOW, u32 _NBINS = 10, u32 SCALE = 16>
	class histogram 
	{
	private:
		enum
		{
			NUM_BINS  = _NBINS + 2,
			LAST_BIN  = _NBINS + 1,
			FIRST_BIN = 0,
		};

		Td		_min;
		Td		_max;
		Td		_factor;
		Tc		_count[NUM_BINS];

		Td		step() const
		{
			return (HIGH - LOW) / (Td) _NBINS;
		}

		u32		v2b(Td v) const
		{
			if (v <= (Td) LOW)
				return FIRST_BIN;
			if (v >= (Td) HIGH)
				return LAST_BIN;

			v -= LOW;
			return 1 + ((v * _factor) >> SCALE);
		}

		Td		b2v(u32 i) const 
		{
			return step() * i + LOW;
		}

	public:
		/**
		* Reset histogram state.
		*/
		void	reset()
		{
			_min    = (Td)~0;
			_max    = 0;
			x_memset(_count, 0, sizeof(_count));
		}

		/**
		* Constructor.
		* Initializes histogram with a clean state
		*/
		histogram()
		{
			_factor = (1 << SCALE) / step();
			reset();
		}

		/**
		* Update histogram.
		* @param v value to update with
		*/
		void	update(Td v)
		{
			if (v < _min) _min = v;
			if (v > _max) _max = v;
			++_count[v2b(v)];
		}

		/**
		* Current min.
		* @return current min value
		*/
		Td		min() const						{ return _min; }

		/**
		* Current max.
		* @return current max value
		*/
		Td		max() const { return _max; }

		/**
		* Iterate histogram.
		* @param[in/out] i running index of the bins
		* @param[out] bin value assigned to the bin
		* @param[out] count number of hits for the bin
		* @return false if iteration is done, true otherwise
		*/
		bool iterate(u32& i, Td& bin, Tc& count) const
		{
			if (i >= NUM_BINS)
				return false;

			switch (i)
			{
			case FIRST_BIN:
				bin = 0;
				break;
			case LAST_BIN:
				bin = 999999999;
				break;
			default:
				bin = b2v(i);
				break;
			}

			count = _count[i++];
			return true;
		}

		/**
		* Iterate histogram.
		* @param[in/out] i running index of the bins
		* @param[out] bin string representation of the value assigned to the bin
		* @param[out] count number of hits for the bin
		* @return false if iteration is done, true otherwise
		*/
		bool iterate(u32& i, char* bin, Tc& count) const
		{
			if (i >= NUM_BINS)
				return false;

			switch (i)
			{
			case FIRST_BIN:
				sprintf(bin, "<%lu", LOW);
				break;
			case LAST_BIN:
				sprintf(bin, ">%lu", HIGH);
				break;
			default:
				sprintf(bin, " %u", b2v(i));
				break;
			}

			count = _count[i++];
			return true;
		}
	};

} // namespace xcore

#endif // __XMULTICORE_HISTOGRAM_H__
