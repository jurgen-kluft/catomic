#include "xbase\x_target.h"
#if defined(TARGET_360)

#include <Xtl.h>

#include "xbase\x_string_std.h"
#include "xbase\x_memory_std.h"

#include "xatomic\x_cpu.h"

namespace xcore
{
	namespace cpu
	{
		xtimescale _scale = { 1, 1, 1, 1 };

		static void initialize()
		{
			LARGE_INTEGER TicksPerSecond;
			::QueryPerformanceFrequency( &TicksPerSecond );
			u64 tps = TicksPerSecond.QuadPart;

			// Various scale factors used for conversions.
			_scale.tsc2nsec = (       tps << TSC2NSEC_SCALE_SHIFT) /    1000ULL;
			_scale.tsc2usec = (       tps << TSC2USEC_SCALE_SHIFT) / 1000000ULL;
			_scale.nsec2tsc = (   1000ULL << NSEC2TSC_SCALE_SHIFT) / tps;
			_scale.usec2tsc = (1000000ULL << USEC2TSC_SCALE_SHIFT) / tps;
		}

		#define CHIPNAME_STRING_LENGTH		(48 + 1)
		#define VENDOR_STRING_LENGTH		(12 + 1)
		#define SERIALNUMBER_STRING_LENGTH	(29 + 1)


		// Forward declares
		class xspeed;
		class xinfo;

		namespace cpu_info_sys
		{
			typedef struct tagID
			{
				int Type;
				int Family;
				int Model;
				int Revision;
				int ExtendedFamily;
				int ExtendedModel;
				char ProcessorName[CHIPNAME_STRING_LENGTH];
				char Vendor[VENDOR_STRING_LENGTH];
				char SerialNumber[SERIALNUMBER_STRING_LENGTH];
			} ID;

			typedef struct tagCPUPowerManagement
			{
				bool HasVoltageID;
				bool HasFrequencyID;
				bool HasTempSenseDiode;
			} CPUPowerManagement;

			typedef struct tagCPUExtendedFeatures 
			{
				bool Has3DNow;
				bool Has3DNowPlus;
				bool SupportsMP;
				bool HasMMXPlus;
				bool HasSSEMMX;
				bool SupportsHyperthreading;
				int NumCores;
				int LogicalProcessorsPerPhysical;
				int APIC_ID;
				CPUPowerManagement PowerManagement;
			} CPUExtendedFeatures;	

			typedef struct CPUtagFeatures
			{
				bool HasFPU;
				bool HasTSC;
				bool HasMMX;
				bool HasSSE;
				bool HasSSEFP;
				bool HasSSE2;
				bool HasIA64;
				bool HasAPIC;
				bool HasCMOV;
				bool HasMTRR;
				bool HasACPI;
				bool HasSerial;
				bool HasThermal;
				int CPUSpeed;
				int L1CacheSize;
				int L2CacheSize;
				int L3CacheSize;
				CPUExtendedFeatures ExtendedFeatures;
			} CPUFeatures;

			enum Manufacturer 
			{
				AMD, Intel, NSC, UMC, Cyrix, NexGen, IDT, Rise, Transmeta, Sony, IBM, UnknownManufacturer
			};

			// Variables.
			static Manufacturer		ChipManufacturer;
			static CPUFeatures		Features;
			static xspeed			Speed;
			static ID				ChipID;

			static bool sIsInitialized = false;
			static void sInitialize()
			{
				if (sIsInitialized)
					return;
			
				ChipManufacturer = IBM;

				ChipID.Type = 0;
				ChipID.Family = 0;
				ChipID.Model = 0;
				ChipID.Revision = 0x0501;
				ChipID.ExtendedFamily = 0;
				ChipID.ExtendedModel = 0;
				x_strcpy(ChipID.ProcessorName, sizeof(ChipID.ProcessorName), "IBM PowerPC \"Xenon\"");
				x_strcpy(ChipID.Vendor, sizeof(ChipID.Vendor), "IBM");
				x_strcpy(ChipID.SerialNumber, sizeof(ChipID.SerialNumber), "");

				Features.HasFPU = true;
				Features.HasTSC = false;
				Features.HasMMX = false;
				Features.HasSSE = false;
				Features.HasSSEFP = false;
				Features.HasSSE2 = false;
				Features.HasIA64 = false;
				Features.HasAPIC = false;
				Features.HasCMOV = false;
				Features.HasMTRR = false;
				Features.HasACPI = false;
				Features.HasSerial = false;
				Features.HasThermal = false;
				Features.CPUSpeed = 3200;
				Features.L1CacheSize = 64 * 1024;
				Features.L2CacheSize = 1024 * 1024;
				Features.L3CacheSize = -1;
			
				Features.ExtendedFeatures.Has3DNow = false;
				Features.ExtendedFeatures.Has3DNowPlus = false;
				Features.ExtendedFeatures.SupportsMP = false;
				Features.ExtendedFeatures.HasMMXPlus = false;
				Features.ExtendedFeatures.HasSSEMMX = false;
				Features.ExtendedFeatures.SupportsHyperthreading = false;
				Features.ExtendedFeatures.NumCores = 3;
				Features.ExtendedFeatures.LogicalProcessorsPerPhysical = 2;
				Features.ExtendedFeatures.APIC_ID = 0;

				Features.ExtendedFeatures.PowerManagement.HasVoltageID = false;
				Features.ExtendedFeatures.PowerManagement.HasFrequencyID = false;
				Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode = false;

				cpu::initialize();

				sIsInitialized = true;
			}
		}


		using namespace cpu_info_sys;


		// --------------------------------------------------------
		//
		//         Constructor Functions - xinfo Class
		//
		// --------------------------------------------------------

		void xinfo::initialize()
		{
			sInitialize();
		}


		// --------------------------------------------------------
		//
		//         Public Functions - xinfo Class
		//
		// --------------------------------------------------------

		const char* xinfo::getVendorString () const
		{
			// Return the vendor string.
			return ChipID.Vendor;
		}

		const char* xinfo::getVendorID () const
		{
			// Return the vendor ID.
			switch (ChipManufacturer)
			{
				case IBM:
					return "IBM";
				default:
					return "Unknown Manufacturer";
			}
		}

		const char * xinfo::getTypeID () const
		{
			return "0";
		}

		const char * xinfo::getFamilyID () const
		{
			return "0";
		}

		const char * xinfo::getModelID () const
		{
			return "0";
		}

		const char * xinfo::getSteppingCode () const
		{
			return "0";
		}

		const char * xinfo::getExtendedProcessorName () const
		{
			// Return the stepping code of the CPU present.
			return ChipID.ProcessorName;
		}

		const char * xinfo::getProcessorSerialNumber () const
		{
			// Return the serial number of the processor in hexadecimal: xxxx-xxxx-xxxx-xxxx-xxxx-xxxx.
			return ChipID.SerialNumber;
		}

		s32 xinfo::getPhysicalProcessors () const
		{
			// Return the physical processors count
			return Features.ExtendedFeatures.NumCores;
		}

		s32 xinfo::getLogicalProcessorsPerPhysical () const
		{
			// Return the logical processors per physical.
			return Features.ExtendedFeatures.LogicalProcessorsPerPhysical;
		}

		u64 xinfo::getProcessorClockFrequency () const
		{
			// Return the processor clock frequency.
			u64 s = Speed.getCPUSpeedInMHz();
			if (s == 0)
				return -1;
			return s;
		}

		s32 xinfo::getProcessorAPICID () const
		{
			// Return the APIC ID.
			return Features.ExtendedFeatures.APIC_ID;
		}

		s32 xinfo::getProcessorCacheXSize (u32 dwCacheID) const
		{
			// Return the chosen cache size.
			switch (dwCacheID) 
			{
				case L1CACHE_FEATURE:
					return Features.L1CacheSize;

				case L2CACHE_FEATURE:
					return Features.L2CacheSize;

				case L3CACHE_FEATURE:
					return Features.L3CacheSize;
			}

			// The user did something strange just return and error.
			return -1;
		}

		bool xinfo::doesCPUSupportFeature (u32 dwFeature) const
		{
			bool bHasFeature = false;

			// Check for L1 cache size.
			if (((dwFeature & L1CACHE_FEATURE) != 0) && (Features.L1CacheSize != -1)) bHasFeature = true;

			// Check for L2 cache size.
			if (((dwFeature & L2CACHE_FEATURE) != 0) && (Features.L2CacheSize != -1)) bHasFeature = true;

			// Check for L3 cache size.
			if (((dwFeature & L3CACHE_FEATURE) != 0) && (Features.L3CacheSize != -1)) bHasFeature = true;

			return bHasFeature;
		}

		// --------------------------------------------------------
		//
		//         Private Functions - CPUInfo Class
		//
		// --------------------------------------------------------


		// --------------------------------------------------------
		//
		//         Constructor Functions - CPUSpeed Class
		//
		// --------------------------------------------------------

		static u64 sCPUSpeedInkHz = 3200 * 1000; 

		xspeed::xspeed ()
		{
		}

		void xspeed::calculate ()
		{
		}

		u64 xspeed::getCPUSpeedInkHz() const
		{
			return sCPUSpeedInkHz;
		}

		u64 xspeed::getCPUSpeedInMHz() const
		{
			return sCPUSpeedInkHz / 1000;
		}
	}
}


#endif