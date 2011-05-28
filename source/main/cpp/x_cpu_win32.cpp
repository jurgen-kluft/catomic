#include "xmulticore\cpu.h"

namespace xcore
{
	namespace cpu 
	{
		unsigned long _khz = 1;
		unsigned long _jitter = 5000; // 5 usec

		struct overhead _overhead;
		struct scale    _scale = { 1, 1, 1, 1 };

		unsigned long _caps;

		volatile unsigned long _locked;

		// Measure overhead of the nanosleep.
		// Same here, 100 measurements and take the smallest value.
		static void measure_nanosleep()
		{
			u64 cycles, o = ~0UL;
			u32 i;

			_overhead.nanosleep = 0;

			for (i = 0; i < 100; i++)
			{
				lock();
				cycles = tsc();
				nanosleep(1000);
				cycles = tsc() - cycles;
				unlock();

				if (cycles < o)
					o = cycles;
			}

			_overhead.nanosleep = tsc2nsec(o) - 1000;
		}

		bool calibrate()
		{
			unsigned long mhz = 0, khz = 0;
			char str[100], *p;

			// Parse out /proc/cpuinfo to determine CPU clock frequency
			FILE *f = fopen("/proc/cpuinfo", "r");
			if (!f) 
			{
				perror("Failed to open /proc/cpuinfo");
				return false;
			}

			while (fgets(str, sizeof(str), f)) 
			{
				if (!strstr(str, "cpu MHz"))
					continue;

				p = strchr(str, ':');
				if (p) 
				{
					mhz = atoi(p + 1);
					khz = mhz * 1000;
					p = strchr(p + 1, '.');
					if (p)
						khz += atoi(p + 1);
					break;
				}
			}
			fclose(f);

			if (!mhz) 
			{
				fprintf(stderr, "Failed to detect CPU clock frequency");
				return false;
			}

			_khz = khz;

			// Various scale factors used for conversions.
			_scale.tsc2nsec = (1000000ULL << TSC2NSEC_SCALE_SHIFT) / khz;
			_scale.tsc2usec = (1000ULL    << TSC2USEC_SCALE_SHIFT) / khz;
			_scale.nsec2tsc = (_khz << NSEC2TSC_SCALE_SHIFT) / 1000000ULL;
			_scale.usec2tsc = (_khz << USEC2TSC_SCALE_SHIFT) / 1000ULL;

			_jitter = 5000; // 5 usec default

#if defined(__i386__) || defined(__x86_64__)
			// We have to set IO priv level to 3 in order to be able to
			// lock IRQ. This is x86 specific feature.
			// FIXME: Move into cpu-x86.cc
			if (iopl(3) >= 0)
				_caps |= (1 << CAPS_IRQ_LOCK);
#endif

			measure_nanosleep();
			return true;
		}

		void nanosleep(unsigned long nsec)
		{
			u64 start = tsc();

			if (unlikely(nsec <= _overhead.nanosleep))
				return;
			nsec -= _overhead.nanosleep;

			while (nsec_elapsed(start) < nsec)
				relax();
		}

		void nanosleep_locked(unsigned long nsec)
		{
			u64 start = tsc();

			if (unlikely(nsec <= _overhead.nanosleep))
				return;
			nsec -= _overhead.nanosleep;

			if (nsec > _jitter) {
				u64 r = nsec - _jitter;
				unlock();
				while (nsec_elapsed(start) < r)
					relax();
				lock();
			}

			while (nsec_elapsed(start) < nsec)
				relax();
		}

	} // namespace cpu
} // namespace xcore
