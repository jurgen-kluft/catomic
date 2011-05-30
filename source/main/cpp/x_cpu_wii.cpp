#include "xbase\x_target.h"
#if defined(TARGET_WII)

#include <revolution/os.h>

#include "xbase\x_string_std.h"
#include "xbase\x_memory_std.h"

#include "xatomic\x_cpu.h"

namespace xcore
{
	namespace xcpu
	{
		xtimescale _scale = { 1, 1, 1, 1 };

		static void initialize()
		{
			// Various scale factors used for conversions.
			_scale.tsc2nsec = ( 1000000000ULL << TSC2NSEC_SCALE_SHIFT) / OS_TIMER_CLOCK;
			_scale.tsc2usec = (    1000000ULL << TSC2USEC_SCALE_SHIFT) / OS_TIMER_CLOCK;
			_scale.nsec2tsc = (OS_TIMER_CLOCK << NSEC2TSC_SCALE_SHIFT) /     1000000000;
			_scale.usec2tsc = (OS_TIMER_CLOCK << USEC2TSC_SCALE_SHIFT) /        1000000;
		}
	}

	#define CHIPNAME_STRING_LENGTH		(48 + 1)
	#define VENDOR_STRING_LENGTH		(12 + 1)
	#define SERIALNUMBER_STRING_LENGTH	(29 + 1)


	// Forward declares
	class xcpu_speed;
	class xcpu_info;

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
		static xcpu_speed		Speed;
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
			x_strcpy(ChipID.ProcessorName, sizeof(ChipID.ProcessorName), "IBM PowerPC \"Broadway\"");
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
			Features.CPUSpeed = 729;
			Features.L1CacheSize = 64 * 1024;
			Features.L2CacheSize = 256 * 1024;
			Features.L3CacheSize = -1;
			
			Features.ExtendedFeatures.Has3DNow = false;
			Features.ExtendedFeatures.Has3DNowPlus = false;
			Features.ExtendedFeatures.SupportsMP = false;
			Features.ExtendedFeatures.HasMMXPlus = false;
			Features.ExtendedFeatures.HasSSEMMX = false;
			Features.ExtendedFeatures.SupportsHyperthreading = false;
			Features.ExtendedFeatures.NumCores = 1;
			Features.ExtendedFeatures.LogicalProcessorsPerPhysical = 1;
			Features.ExtendedFeatures.APIC_ID = 0;

			Features.ExtendedFeatures.PowerManagement.HasVoltageID = false;
			Features.ExtendedFeatures.PowerManagement.HasFrequencyID = false;
			Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode = false;

			xcpu::initialize();

			sIsInitialized = true;
		}
	}


	using namespace cpu_info_sys;


	// --------------------------------------------------------
	//
	//         Constructor Functions - xcpu_info Class
	//
	// --------------------------------------------------------

	void xcpu_info::initialize()
	{
		sInitialize();
	}


	// --------------------------------------------------------
	//
	//         Public Functions - xcpu_info Class
	//
	// --------------------------------------------------------

	const char* xcpu_info::getVendorString () const
	{
		// Return the vendor string.
		return ChipID.Vendor;
	}

	const char* xcpu_info::getVendorID () const
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

	const char * xcpu_info::getTypeID () const
	{
		return "0";
	}

	const char * xcpu_info::getFamilyID () const
	{
		return "0";
	}

	const char * xcpu_info::getModelID () const
	{
		return "0";
	}

	const char * xcpu_info::getSteppingCode () const
	{
		return "0";
	}

	const char * xcpu_info::getExtendedProcessorName () const
	{
		// Return the stepping code of the CPU present.
		return ChipID.ProcessorName;
	}

	const char * xcpu_info::getProcessorSerialNumber () const
	{
		// Return the serial number of the processor in hexadecimal: xxxx-xxxx-xxxx-xxxx-xxxx-xxxx.
		return ChipID.SerialNumber;
	}

	s32 xcpu_info::getPhysicalProcessors () const
	{
		// Return the physical processors count
		return Features.ExtendedFeatures.NumCores;
	}

	s32 xcpu_info::getLogicalProcessorsPerPhysical () const
	{
		// Return the logical processors per physical.
		return Features.ExtendedFeatures.LogicalProcessorsPerPhysical;
	}

	u64 xcpu_info::getProcessorClockFrequency () const
	{
		// Return the processor clock frequency.
		u64 s = Speed.getCPUSpeedInMHz();
		if (s == 0)
			return -1;
		return s;
	}

	s32 xcpu_info::getProcessorAPICID () const
	{
		// Return the APIC ID.
		return Features.ExtendedFeatures.APIC_ID;
	}

	s32 xcpu_info::getProcessorCacheXSize (u32 dwCacheID) const
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

	bool xcpu_info::doesCPUSupportFeature (u32 dwFeature) const
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

	static u64 sCPUSpeedInkHz = 729 * 1000; 

	xcpu_speed::xcpu_speed ()
	{
	}

	void xcpu_speed::calculate ()
	{
	}

	u64 xcpu_speed::getCPUSpeedInkHz() const
	{
		return sCPUSpeedInkHz;
	}

	u64 xcpu_speed::getCPUSpeedInMHz() const
	{
		return sCPUSpeedInkHz / 1000;
	}


}


#endif