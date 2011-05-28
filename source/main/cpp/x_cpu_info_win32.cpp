#include "xbase\x_target.h"
#if defined(TARGET_PC)

#include "xbase\x_string_std.h"
#include "xbase\x_memory_std.h"

// Include Windows header files.
#include <windows.h>

#include "xatomic\x_cpu_info.h"

namespace xcore
{
	namespace cpu
	{
		xtimescale _scale = { 1, 1, 1, 1 };

		static void initialize()
		{
			xspeed speed;
			speed.calculate();

			u64 khz = speed.getCPUSpeedInkHz();

			// Various scale factors used for conversions.
			_scale.tsc2nsec = (1000000ULL << TSC2NSEC_SCALE_SHIFT) / khz;
			_scale.tsc2usec = (1000ULL    << TSC2USEC_SCALE_SHIFT) / khz;
			_scale.nsec2tsc = (khz << NSEC2TSC_SCALE_SHIFT) / 1000000ULL;
			_scale.usec2tsc = (khz << USEC2TSC_SCALE_SHIFT) / 1000ULL;
		}

		#define CPU_INFO_TRY				
		#define CPU_INFO_CATCH_ALL			if (false)

		#define CHIPNAME_STRING_LENGTH		(48 + 1)
		#define VENDOR_STRING_LENGTH		(12 + 1)
		#define SERIALNUMBER_STRING_LENGTH	(29 + 1)

		#define STORE_CLASSICAL_NAME(x)		x_sprintf (ChipID.ProcessorName, CHIPNAME_STRING_LENGTH, x)
		#define STORE_TLBCACHE_INFO(x,y)	x = (x < y) ? y : x
		#define TLBCACHE_INFO_UNITS			(15)
		#define CLASSICAL_CPU_FREQ_LOOP		10000000

		#define CPUID_AWARE_COMPILER
		#ifdef CPUID_AWARE_COMPILER
			#define CPUID_INSTRUCTION		cpuid
		#else
			#define CPUID_INSTRUCTION		_asm _emit 0x0f _asm _emit 0xa2
		#endif

		// Forward declares
		class xspeed;
		class xinfo;

		namespace cpu_info_sys
		{
			// Functions.
			static bool sRetrieveCPUFeatures ();
			static bool sRetrieveCPUIdentity ();
			static bool sRetrieveCPUCacheDetails ();
			static bool sRetrieveClassicalCPUCacheDetails ();
			static bool sRetrieveCPUClockSpeed ();
			static bool sRetrieveClassicalCPUClockSpeed ();
			static bool sRetrieveCPUExtendedLevelSupport (int);
			static bool sRetrieveExtendedCPUFeatures ();
			static bool sRetrieveProcessorSerialNumber ();
			static bool sRetrieveCPUPowerManagement ();
			static bool sRetrieveClassicalCPUIdentity ();
			static bool sRetrieveExtendedCPUIdentity ();
			static bool sDoesCPUSupportCPUID ();

			static bool sIsInitialized = false;
			static void sInitialize()
			{
				if (sIsInitialized)
					return;

				// Check to see if this processor supports CPUID.
				if (sDoesCPUSupportCPUID ())
				{
					// Retrieve the CPU details.
					sRetrieveCPUIdentity ();
					sRetrieveCPUFeatures ();
					if (!sRetrieveCPUClockSpeed ()) 
						sRetrieveClassicalCPUClockSpeed ();

					// Attempt to retrieve cache information.
					if (!sRetrieveCPUCacheDetails ()) 
						sRetrieveClassicalCPUCacheDetails ();

					// Retrieve the extended CPU details.
					if (!sRetrieveExtendedCPUIdentity ())
						sRetrieveClassicalCPUIdentity ();
					sRetrieveExtendedCPUFeatures ();

					// Now attempt to retrieve the serial number (if possible).
					sRetrieveProcessorSerialNumber ();
				}

				cpu::initialize();
			
				sIsInitialized = true;
			}

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
				AMD, Intel, NSC, UMC, Cyrix, NexGen, IDT, Rise, Transmeta, UnknownManufacturer
			};

			// Variables.
			static Manufacturer		ChipManufacturer;
			static CPUFeatures		Features;
			static xspeed		Speed;
			static ID				ChipID;

			static inline bool	sCpuID(u32 i, u32 regs[4])
			{
	#if defined(TARGET_PC)
				u32 reg0,reg1,reg2,reg3;
	#ifdef CPUID_AWARE_COMPILER
				_asm 
				{
					; we must push/pop the registers <<CPUID>> writes to, as the
					; optimiser doesn't know about <<CPUID>>, and so doesn't expect
					; these registers to change.
					push eax
					push ebx
					push ecx
					push edx
	#endif
					rdtsc

					; <<CPUID>> 
					; eax = 1 --> eax: CPU ID - bits 31..16 - unused, bits 15..12 - type, bits 11..8 - family, bits 7..4 - model, bits 3..0 - mask revision
					;			  ebx: 31..24 - default APIC ID, 23..16 - logical processsor ID, 15..8 - CFLUSH chunk size , 7..0 - brand ID
					;			  edx: CPU feature flags
					mov eax,i
					CPUID_INSTRUCTION
					mov reg3, edx
					mov reg2, ecx
					mov reg1, ebx
					mov reg0, eax

	#ifdef CPUID_AWARE_COMPILER
					pop edx
					pop ecx
					pop ebx
					pop eax
	#endif
				}
				regs[0]=reg0; regs[1]=reg1; regs[2]=reg2; regs[3]=reg3;
				return true;
	#else
				regs[0]=regs[1]=regs[2]=regs[3]=0;
				return false;
	#endif
			}
			static inline bool sCpuID(u32 i, u32 *reg0, u32 *reg1, u32 *reg2, u32 *reg3)
			{
				u32 regs[4];
				if (!sCpuID(i, regs))
					return false;
				if (reg0!=NULL) *reg0 = regs[0];
				if (reg1!=NULL) *reg1 = regs[1];
				if (reg2!=NULL) *reg2 = regs[2];
				if (reg3!=NULL) *reg3 = regs[3];
				return true;
			}

			bool sRetrieveCPUFeatures ()
			{
				u32 CPUFeatures = 0;
				u32 CPUAdvanced = 0;
				if (!sCpuID(1, NULL, &CPUAdvanced, NULL, &CPUFeatures))
					return false;

				// Retrieve the features of CPU present.
				Features.HasFPU =		((CPUFeatures & 0x00000001) != 0);		// FPU Present --> Bit 0
				Features.HasTSC =		((CPUFeatures & 0x00000010) != 0);		// TSC Present --> Bit 4
				Features.HasAPIC =		((CPUFeatures & 0x00000200) != 0);		// APIC Present --> Bit 9
				Features.HasMTRR =		((CPUFeatures & 0x00001000) != 0);		// MTRR Present --> Bit 12
				Features.HasCMOV =		((CPUFeatures & 0x00008000) != 0);		// CMOV Present --> Bit 15
				Features.HasSerial =	((CPUFeatures & 0x00040000) != 0);		// Serial Present --> Bit 18
				Features.HasACPI =		((CPUFeatures & 0x00400000) != 0);		// ACPI Capable --> Bit 22
				Features.HasMMX =		((CPUFeatures & 0x00800000) != 0);		// MMX Present --> Bit 23
				Features.HasSSE =		((CPUFeatures & 0x02000000) != 0);		// SSE Present --> Bit 25
				Features.HasSSE2 =		((CPUFeatures & 0x04000000) != 0);		// SSE2 Present --> Bit 26
				Features.HasThermal =	((CPUFeatures & 0x20000000) != 0);		// Thermal Monitor Present --> Bit 29
				Features.HasIA64 =		((CPUFeatures & 0x40000000) != 0);		// IA64 Present --> Bit 30

				// Retrieve extended SSE capabilities if SSE is available.
				if (Features.HasSSE)
				{
					// Attempt to CPU_INFO_TRY some SSE FP instructions.
					CPU_INFO_TRY
					{
						// Perform: orps xmm0, xmm0
						_asm
						{
							_emit 0x0f
							_emit 0x56
							_emit 0xc0	
						}

						// SSE FP capable processor.
						Features.HasSSEFP = true;
					}

					// A generic catch-all just to be sure...
					CPU_INFO_CATCH_ALL
					{
						// bad instruction - processor or OS cannot handle SSE FP.
						Features.HasSSEFP = false;
					}
				} 
				else 
				{
					// Set the advanced SSE capabilities to not available.
					Features.HasSSEFP = false;
				}

				// Retrieve Intel specific extended features.
				if (ChipManufacturer == Intel)
				{
					Features.ExtendedFeatures.SupportsHyperthreading =	((CPUFeatures &	0x10000000) != 0);	// Intel specific: Hyperthreading --> Bit 28
					Features.ExtendedFeatures.LogicalProcessorsPerPhysical = (Features.ExtendedFeatures.SupportsHyperthreading) ? ((CPUAdvanced & 0x00FF0000) >> 16) : 1;

					if ((Features.ExtendedFeatures.SupportsHyperthreading) && (Features.HasAPIC))
					{
						// Retrieve APIC information if there is one present.
						Features.ExtendedFeatures.APIC_ID = ((CPUAdvanced & 0xFF000000) >> 24);
					}
				}
				else
				{
					Features.ExtendedFeatures.SupportsHyperthreading = false;
					Features.ExtendedFeatures.LogicalProcessorsPerPhysical = 1;
				}

				// Physical cores
				u32 NumCores = Features.ExtendedFeatures.LogicalProcessorsPerPhysical;
				if (ChipManufacturer == Intel)
				{
					sCpuID(4, &NumCores, NULL, NULL, NULL);
					NumCores = 1 + ((NumCores >> 26) & 0x3F);
				}
				else if (ChipManufacturer == AMD)
				{
					u32 NumCores = 0;
					sCpuID(0x80000008, NULL, NULL, &NumCores, NULL);
					NumCores = 1 + NumCores & 0xFF;
				}

				Features.ExtendedFeatures.NumCores = NumCores;

				return true;
			}

			bool sRetrieveCPUIdentity ()
			{
				u32 CPUVendor[3];
				u32 CPUSignature;

				// CPU vendor
				if (!sCpuID(0, NULL, &CPUVendor[0], &CPUVendor[2], &CPUVendor[1]))
					return false;
				if (!sCpuID(1, &CPUSignature, NULL, NULL, NULL))
					return false;

				// Process the returned information.
				x_memcpy (ChipID.Vendor, &(CPUVendor[0]), sizeof (int));
				x_memcpy (&(ChipID.Vendor[4]), &(CPUVendor[1]), sizeof (int));
				x_memcpy (&(ChipID.Vendor[8]), &(CPUVendor[2]), sizeof (int));
				ChipID.Vendor[12] = '\0';

				// Attempt to retrieve the manufacturer from the vendor string.
				if (x_strcmp (ChipID.Vendor, "GenuineIntel") == 0)		ChipManufacturer = Intel;				// Intel Corp.
				else if (x_strcmp (ChipID.Vendor, "UMC UMC UMC ") == 0)	ChipManufacturer = UMC;					// United Microelectronics Corp.
				else if (x_strcmp (ChipID.Vendor, "AuthenticAMD") == 0)	ChipManufacturer = AMD;					// Advanced Micro Devices
				else if (x_strcmp (ChipID.Vendor, "AMD ISBETTER") == 0)	ChipManufacturer = AMD;					// Advanced Micro Devices (1994)
				else if (x_strcmp (ChipID.Vendor, "CyrixInstead") == 0)	ChipManufacturer = Cyrix;				// Cyrix Corp., VIA Inc.
				else if (x_strcmp (ChipID.Vendor, "NexGenDriven") == 0)	ChipManufacturer = NexGen;				// NexGen Inc. (now AMD)
				else if (x_strcmp (ChipID.Vendor, "CentaurHauls") == 0)	ChipManufacturer = IDT;					// IDT/Centaur (now VIA)
				else if (x_strcmp (ChipID.Vendor, "RiseRiseRise") == 0)	ChipManufacturer = Rise;				// Rise
				else if (x_strcmp (ChipID.Vendor, "GenuineTMx86") == 0)	ChipManufacturer = Transmeta;			// Transmeta
				else if (x_strcmp (ChipID.Vendor, "TransmetaCPU") == 0)	ChipManufacturer = Transmeta;			// Transmeta
				else if (x_strcmp (ChipID.Vendor, "Geode By NSC") == 0)	ChipManufacturer = NSC;					// National Semiconductor
				else													ChipManufacturer = UnknownManufacturer;	// Unknown manufacturer

				// Retrieve the family of CPU present.
				ChipID.ExtendedFamily =		((CPUSignature & 0x0FF00000) >> 20);	// Bits 27..20 Used
				ChipID.ExtendedModel =		((CPUSignature & 0x000F0000) >> 16);	// Bits 19..16 Used
				ChipID.Type =				((CPUSignature & 0x0000F000) >> 12);	// Bits 15..12 Used
				ChipID.Family =				((CPUSignature & 0x00000F00) >> 8);		// Bits 11..8 Used
				ChipID.Model =				((CPUSignature & 0x000000F0) >> 4);		// Bits 7..4 Used
				ChipID.Revision =			((CPUSignature & 0x0000000F) >> 0);		// Bits 3..0 Used

				return true;
			}

			bool sRetrieveCPUCacheDetails ()
			{
				u32 L1Cache[4] = { 0, 0, 0, 0 };
				u32 L2Cache[4] = { 0, 0, 0, 0 };

				// Check to see if what we are about to do is supported...
				if (sRetrieveCPUExtendedLevelSupport (0x80000005)) 
				{
					if (!sCpuID(0x80000005, &L1Cache[0], &L1Cache[1], &L1Cache[2], &L1Cache[3]))
						return false;

					// Save the L1 data cache size (in KB) from ecx: bits 31..24 as well as data cache size from edx: bits 31..24.
					Features.L1CacheSize = ((L1Cache[2] & 0xFF000000) >> 24);
					Features.L1CacheSize += ((L1Cache[3] & 0xFF000000) >> 24);
				}
				else 
				{
					// Store -1 to indicate the cache could not be queried.
					Features.L1CacheSize = -1;
				}

				// Check to see if what we are about to do is supported...
				if (sRetrieveCPUExtendedLevelSupport (0x80000006))
				{
					if (!sCpuID(0x80000006, &L2Cache[0], &L2Cache[1], &L2Cache[2], &L2Cache[3]))
						return false;

					// Save the L2 unified cache size (in KB) from ecx: bits 31..16.
					Features.L2CacheSize = ((L2Cache[2] & 0xFFFF0000) >> 16);
				}
				else
				{
					// Store -1 to indicate the cache could not be queried.
					Features.L2CacheSize = -1;
				}

				// Define L3 as being not present as we cannot test for it.
				Features.L3CacheSize = -1;

				// Return failure if we cannot detect either cache with this method.
				return ((Features.L1CacheSize == -1) && (Features.L2CacheSize == -1)) ? false : true;
			}

			bool sRetrieveClassicalCPUCacheDetails ()
			{
				u32 TLBCode = -1, TLBData = -1, L1Code = -1, L1Data = -1, L1Trace = -1, L2Unified = -1, L3Unified = -1;
				u32 TLBCacheData[4] = { 0, 0, 0, 0 };
				u32 TLBPassCounter = 0;
				u32 TLBCacheUnit = 0;

				do 
				{
					if (!sCpuID(2, &TLBCacheData[0], &TLBCacheData[1], &TLBCacheData[2], &TLBCacheData[3]))
						return false;

					s32 bob = ((TLBCacheData[0] & 0x00FF0000) >> 16);

					// Process the returned TLB and cache information.
					for (s32 nCounter = 0; nCounter < TLBCACHE_INFO_UNITS; nCounter ++) 
					{
						// First of all - decide which unit we are dealing with.
						switch (nCounter)
						{
							// eax: bits 8..15 : bits 16..23 : bits 24..31
						case 0: TLBCacheUnit = ((TLBCacheData[0] & 0x0000FF00) >> 8); break;
						case 1: TLBCacheUnit = ((TLBCacheData[0] & 0x00FF0000) >> 16); break;
						case 2: TLBCacheUnit = ((TLBCacheData[0] & 0xFF000000) >> 24); break;

							// ebx: bits 0..7 : bits 8..15 : bits 16..23 : bits 24..31
						case 3: TLBCacheUnit = ((TLBCacheData[1] & 0x000000FF) >> 0); break;
						case 4: TLBCacheUnit = ((TLBCacheData[1] & 0x0000FF00) >> 8); break;
						case 5: TLBCacheUnit = ((TLBCacheData[1] & 0x00FF0000) >> 16); break;
						case 6: TLBCacheUnit = ((TLBCacheData[1] & 0xFF000000) >> 24); break;

							// ecx: bits 0..7 : bits 8..15 : bits 16..23 : bits 24..31
						case 7: TLBCacheUnit = ((TLBCacheData[2] & 0x000000FF) >> 0); break;
						case 8: TLBCacheUnit = ((TLBCacheData[2] & 0x0000FF00) >> 8); break;
						case 9: TLBCacheUnit = ((TLBCacheData[2] & 0x00FF0000) >> 16); break;
						case 10: TLBCacheUnit = ((TLBCacheData[2] & 0xFF000000) >> 24); break;

							// edx: bits 0..7 : bits 8..15 : bits 16..23 : bits 24..31
						case 11: TLBCacheUnit = ((TLBCacheData[3] & 0x000000FF) >> 0); break;
						case 12: TLBCacheUnit = ((TLBCacheData[3] & 0x0000FF00) >> 8); break;
						case 13: TLBCacheUnit = ((TLBCacheData[3] & 0x00FF0000) >> 16); break;
						case 14: TLBCacheUnit = ((TLBCacheData[3] & 0xFF000000) >> 24); break;

							// Default case - an error has occurred.
						default: return false;
						}

						// Now process the resulting unit to see what it means....
						switch (TLBCacheUnit)
						{
						case 0x00: break;
						case 0x01: STORE_TLBCACHE_INFO (TLBCode, 4); break;
						case 0x02: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
						case 0x03: STORE_TLBCACHE_INFO (TLBData, 4); break;
						case 0x04: STORE_TLBCACHE_INFO (TLBData, 4096); break;
						case 0x06: STORE_TLBCACHE_INFO (L1Code, 8); break;
						case 0x08: STORE_TLBCACHE_INFO (L1Code, 16); break;
						case 0x0a: STORE_TLBCACHE_INFO (L1Data, 8); break;
						case 0x0c: STORE_TLBCACHE_INFO (L1Data, 16); break;
						case 0x10: STORE_TLBCACHE_INFO (L1Data, 16); break;			// <-- FIXME: IA-64 Only
						case 0x15: STORE_TLBCACHE_INFO (L1Code, 16); break;			// <-- FIXME: IA-64 Only
						case 0x1a: STORE_TLBCACHE_INFO (L2Unified, 96); break;		// <-- FIXME: IA-64 Only
						case 0x22: STORE_TLBCACHE_INFO (L3Unified, 512); break;
						case 0x23: STORE_TLBCACHE_INFO (L3Unified, 1024); break;
						case 0x25: STORE_TLBCACHE_INFO (L3Unified, 2048); break;
						case 0x29: STORE_TLBCACHE_INFO (L3Unified, 4096); break;
						case 0x39: STORE_TLBCACHE_INFO (L2Unified, 128); break;
						case 0x3c: STORE_TLBCACHE_INFO (L2Unified, 256); break;
						case 0x40: STORE_TLBCACHE_INFO (L2Unified, 0); break;		// <-- FIXME: No integrated L2 cache (P6 core) or L3 cache (P4 core).
						case 0x41: STORE_TLBCACHE_INFO (L2Unified, 128); break;
						case 0x42: STORE_TLBCACHE_INFO (L2Unified, 256); break;
						case 0x43: STORE_TLBCACHE_INFO (L2Unified, 512); break;
						case 0x44: STORE_TLBCACHE_INFO (L2Unified, 1024); break;
						case 0x45: STORE_TLBCACHE_INFO (L2Unified, 2048); break;
						case 0x50: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
						case 0x51: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
						case 0x52: STORE_TLBCACHE_INFO (TLBCode, 4096); break;
						case 0x5b: STORE_TLBCACHE_INFO (TLBData, 4096); break;
						case 0x5c: STORE_TLBCACHE_INFO (TLBData, 4096); break;
						case 0x5d: STORE_TLBCACHE_INFO (TLBData, 4096); break;
						case 0x66: STORE_TLBCACHE_INFO (L1Data, 8); break;
						case 0x67: STORE_TLBCACHE_INFO (L1Data, 16); break;
						case 0x68: STORE_TLBCACHE_INFO (L1Data, 32); break;
						case 0x70: STORE_TLBCACHE_INFO (L1Trace, 12); break;
						case 0x71: STORE_TLBCACHE_INFO (L1Trace, 16); break;
						case 0x72: STORE_TLBCACHE_INFO (L1Trace, 32); break;
						case 0x77: STORE_TLBCACHE_INFO (L1Code, 16); break;			// <-- FIXME: IA-64 Only
						case 0x79: STORE_TLBCACHE_INFO (L2Unified, 128); break;
						case 0x7a: STORE_TLBCACHE_INFO (L2Unified, 256); break;
						case 0x7b: STORE_TLBCACHE_INFO (L2Unified, 512); break;
						case 0x7c: STORE_TLBCACHE_INFO (L2Unified, 1024); break;
						case 0x7e: STORE_TLBCACHE_INFO (L2Unified, 256); break;
						case 0x81: STORE_TLBCACHE_INFO (L2Unified, 128); break;
						case 0x82: STORE_TLBCACHE_INFO (L2Unified, 256); break;
						case 0x83: STORE_TLBCACHE_INFO (L2Unified, 512); break;
						case 0x84: STORE_TLBCACHE_INFO (L2Unified, 1024); break;
						case 0x85: STORE_TLBCACHE_INFO (L2Unified, 2048); break;
						case 0x88: STORE_TLBCACHE_INFO (L3Unified, 2048); break;	// <-- FIXME: IA-64 Only
						case 0x89: STORE_TLBCACHE_INFO (L3Unified, 4096); break;	// <-- FIXME: IA-64 Only
						case 0x8a: STORE_TLBCACHE_INFO (L3Unified, 8192); break;	// <-- FIXME: IA-64 Only
						case 0x8d: STORE_TLBCACHE_INFO (L3Unified, 3096); break;	// <-- FIXME: IA-64 Only
						case 0x90: STORE_TLBCACHE_INFO (TLBCode, 262144); break;	// <-- FIXME: IA-64 Only
						case 0x96: STORE_TLBCACHE_INFO (TLBCode, 262144); break;	// <-- FIXME: IA-64 Only
						case 0x9b: STORE_TLBCACHE_INFO (TLBCode, 262144); break;	// <-- FIXME: IA-64 Only

							// Default case - an error has occurred.
						default: return false;
						}
					}

					// Increment the TLB pass counter.
					TLBPassCounter ++;

				} while ((TLBCacheData[0] & 0x000000FF) > TLBPassCounter);

				// Ok - we now have the maximum TLB, L1, L2, and L3 sizes...
				if ((L1Code == -1) && (L1Data == -1) && (L1Trace == -1)) Features.L1CacheSize = -1;
				else if ((L1Code == -1) && (L1Data == -1) && (L1Trace != -1)) Features.L1CacheSize = L1Trace;
				else if ((L1Code != -1) && (L1Data == -1)) Features.L1CacheSize = L1Code;
				else if ((L1Code == -1) && (L1Data != -1)) Features.L1CacheSize = L1Data;
				else if ((L1Code != -1) && (L1Data != -1)) Features.L1CacheSize = L1Code + L1Data;
				else Features.L1CacheSize = -1;

				// Ok - we now have the maximum TLB, L1, L2, and L3 sizes...
				if (L2Unified == -1) Features.L2CacheSize = -1;
				else Features.L2CacheSize = L2Unified;

				// Ok - we now have the maximum TLB, L1, L2, and L3 sizes...
				if (L3Unified == -1) Features.L3CacheSize = -1;
				else Features.L3CacheSize = L3Unified;

				return true;
			}

			bool sRetrieveCPUClockSpeed ()
			{
				// First of all we check to see if the RDTSC (0x0F, 0x31) instruction is supported.
				if (!Features.HasTSC)
					return false;

				// Get the clock speed.
				Speed.calculate();

				return true;
			}

			bool sRetrieveClassicalCPUClockSpeed ()
			{
				LARGE_INTEGER liStart, liEnd, liCountsPerSecond;
				double dFrequency, dDifference;

				// Attempt to get a starting tick count.
				QueryPerformanceCounter (&liStart);

				CPU_INFO_TRY
				{
					_asm 
					{
						mov eax, 0x80000000
						mov ebx, CLASSICAL_CPU_FREQ_LOOP
		Timer_Loop: 
						bsf ecx,eax
						dec ebx
						jnz Timer_Loop
					}	
				}

				// A generic catch-all just to be sure...
				CPU_INFO_CATCH_ALL 
				{
					return false;
				}

				// Attempt to get a starting tick count.
				QueryPerformanceCounter (&liEnd);

				// Get the difference...  NB: This is in seconds....
				QueryPerformanceFrequency (&liCountsPerSecond);
				dDifference = (((double) liEnd.QuadPart - (double) liStart.QuadPart) / (double) liCountsPerSecond.QuadPart);

				// Calculate the clock speed.
				if (ChipID.Family == 3)
				{
					// 80386 processors....  Loop time is 115 cycles!
					dFrequency = (((CLASSICAL_CPU_FREQ_LOOP * 115) / dDifference) / (1024*1024));
				} 
				else if (ChipID.Family == 4)
				{
					// 80486 processors....  Loop time is 47 cycles!
					dFrequency = (((CLASSICAL_CPU_FREQ_LOOP * 47) / dDifference) / (1024*1024));
				} 
				else if (ChipID.Family == 5) 
				{
					// Pentium processors....  Loop time is 43 cycles!
					dFrequency = (((CLASSICAL_CPU_FREQ_LOOP * 43) / dDifference) / (1024*1024));
				}

				// Save the clock speed (MHz)
				Features.CPUSpeed = (s32) dFrequency;

				return true;
			}

			bool sRetrieveCPUExtendedLevelSupport (s32 CPULevelToCheck)
			{
				u32 MaxCPUExtendedLevel = 0;

				// The extended CPUID is supported by various vendors starting with the following CPU models: 
				//
				//		Manufacturer & Chip Name			|		Family		 Model		Revision
				//
				//		AMD K6, K6-2						|		   5		   6			x		
				//		Cyrix GXm, Cyrix III "Joshua"		|		   5		   4			x
				//		IDT C6-2							|		   5		   8			x
				//		VIA Cyrix III						|		   6		   5			x
				//		Transmeta Crusoe					|		   5		   x			x
				//		Intel Pentium 4						|		   f		   x			x
				//

				// We check to see if a supported processor is present...
				if (ChipManufacturer == AMD) 
				{
					if (ChipID.Family < 5) return false;
					if ((ChipID.Family == 5) && (ChipID.Model < 6)) return false;
				}
				else if (ChipManufacturer == Cyrix) 
				{
					if (ChipID.Family < 5) return false;
					if ((ChipID.Family == 5) && (ChipID.Model < 4)) return false;
					if ((ChipID.Family == 6) && (ChipID.Model < 5)) return false;
				}
				else if (ChipManufacturer == IDT)
				{
					if (ChipID.Family < 5) return false;
					if ((ChipID.Family == 5) && (ChipID.Model < 8)) return false;
				}
				else if (ChipManufacturer == Transmeta)
				{
					if (ChipID.Family < 5) return false;
				} 
				else if (ChipManufacturer == Intel)
				{
					if (ChipID.Family < 0xf) return false;
				}

				if (!sCpuID(0x80000000, &MaxCPUExtendedLevel, NULL, NULL, NULL))
					return false;

				// Now we have to check the level wanted vs level returned...
				int nLevelWanted = (CPULevelToCheck & 0x7FFFFFFF);
				int nLevelReturn = (MaxCPUExtendedLevel & 0x7FFFFFFF);

				// Check to see if the level provided is supported...
				if (nLevelWanted > nLevelReturn) 
					return false;

				return true;
			}

			bool sRetrieveExtendedCPUFeatures ()
			{
				u32 CPUExtendedFeatures = 0;

				// Check that we are not using an Intel processor as it does not support this.
				if (ChipManufacturer == Intel)
					return false;

				// Check to see if what we are about to do is supported...
				if (!sRetrieveCPUExtendedLevelSupport (0x80000001))
					return false;

				if (!sCpuID(0x80000001, NULL, NULL, NULL, &CPUExtendedFeatures))
					return false;

				// Retrieve the extended features of CPU present.
				Features.ExtendedFeatures.Has3DNow =		((CPUExtendedFeatures & 0x80000000) != 0);	// 3DNow Present --> Bit 31.
				Features.ExtendedFeatures.Has3DNowPlus =	((CPUExtendedFeatures & 0x40000000) != 0);	// 3DNow+ Present -- > Bit 30.
				Features.ExtendedFeatures.HasSSEMMX =		((CPUExtendedFeatures & 0x00400000) != 0);	// SSE MMX Present --> Bit 22.
				Features.ExtendedFeatures.SupportsMP =		((CPUExtendedFeatures & 0x00080000) != 0);	// MP Capable -- > Bit 19.

				// Retrieve AMD specific extended features.
				if (ChipManufacturer == AMD)
				{
					Features.ExtendedFeatures.HasMMXPlus =	((CPUExtendedFeatures &	0x00400000) != 0);	// AMD specific: MMX-SSE --> Bit 22
				}

				// Retrieve Cyrix specific extended features.
				if (ChipManufacturer == Cyrix) 
				{
					Features.ExtendedFeatures.HasMMXPlus =	((CPUExtendedFeatures &	0x01000000) != 0);	// Cyrix specific: Extended MMX --> Bit 24
				}

				return true;
			}

			bool sRetrieveProcessorSerialNumber ()
			{
				u32 SerialNumber[3];

				// Check to see if the processor supports the processor serial number.
				if (!Features.HasSerial) 
					return false;

				if (!sCpuID(3, NULL, &SerialNumber[0], &SerialNumber[1], &SerialNumber[2]))
					return false;

				// Process the returned information.
				x_sprintf (ChipID.SerialNumber, SERIALNUMBER_STRING_LENGTH, "%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x-%.2x%.2x",
					((SerialNumber[0] & 0xff000000) >> 24),
					((SerialNumber[0] & 0x00ff0000) >> 16),
					((SerialNumber[0] & 0x0000ff00) >> 8),
					((SerialNumber[0] & 0x000000ff) >> 0),
					((SerialNumber[1] & 0xff000000) >> 24),
					((SerialNumber[1] & 0x00ff0000) >> 16),
					((SerialNumber[1] & 0x0000ff00) >> 8),
					((SerialNumber[1] & 0x000000ff) >> 0),
					((SerialNumber[2] & 0xff000000) >> 24),
					((SerialNumber[2] & 0x00ff0000) >> 16),
					((SerialNumber[2] & 0x0000ff00) >> 8),
					((SerialNumber[2] & 0x000000ff) >> 0));

				return true;
			}

			bool sRetrieveCPUPowerManagement ()
			{	
				u32 CPUPowerManagement = 0;

				// Check to see if what we are about to do is supported...
				if (!sRetrieveCPUExtendedLevelSupport (0x80000007))
				{
					Features.ExtendedFeatures.PowerManagement.HasFrequencyID = false;
					Features.ExtendedFeatures.PowerManagement.HasVoltageID = false;
					Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode = false;
					return false;
				}

				if (!sCpuID(0x80000007, NULL, NULL, NULL, &CPUPowerManagement))
					return false;

				// Check for the power management capabilities of the CPU.
				Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode =	((CPUPowerManagement & 0x00000001) != 0);
				Features.ExtendedFeatures.PowerManagement.HasFrequencyID =		((CPUPowerManagement & 0x00000002) != 0);
				Features.ExtendedFeatures.PowerManagement.HasVoltageID =		((CPUPowerManagement & 0x00000004) != 0);

				return true;
			}

			bool sRetrieveExtendedCPUIdentity ()
			{
				u32 ProcessorNameStartPos = 0;
				u32 CPUExtendedIdentity[12];

				// Check to see if what we are about to do is supported...
				if (!sRetrieveCPUExtendedLevelSupport (0x80000002)) return false;
				if (!sRetrieveCPUExtendedLevelSupport (0x80000003)) return false;
				if (!sRetrieveCPUExtendedLevelSupport (0x80000004)) return false;

				s32 e = 0;
				if (!sCpuID(0x80000002, &CPUExtendedIdentity[e+0], &CPUExtendedIdentity[e+1], &CPUExtendedIdentity[e+2], &CPUExtendedIdentity[e+3]))
					return false;
				e += 4;
				if (!sCpuID(0x80000003, &CPUExtendedIdentity[e+0], &CPUExtendedIdentity[e+1], &CPUExtendedIdentity[e+2], &CPUExtendedIdentity[e+3]))
					return false;
				e += 4;
				if (!sCpuID(0x80000004, &CPUExtendedIdentity[e+0], &CPUExtendedIdentity[e+1], &CPUExtendedIdentity[e+2], &CPUExtendedIdentity[e+3]))
					return false;

				// Process the returned information.
				x_memcpy (ChipID.ProcessorName, &(CPUExtendedIdentity[0]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[4]), &(CPUExtendedIdentity[1]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[8]), &(CPUExtendedIdentity[2]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[12]), &(CPUExtendedIdentity[3]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[16]), &(CPUExtendedIdentity[4]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[20]), &(CPUExtendedIdentity[5]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[24]), &(CPUExtendedIdentity[6]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[28]), &(CPUExtendedIdentity[7]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[32]), &(CPUExtendedIdentity[8]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[36]), &(CPUExtendedIdentity[9]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[40]), &(CPUExtendedIdentity[10]), sizeof (int));
				x_memcpy (&(ChipID.ProcessorName[44]), &(CPUExtendedIdentity[11]), sizeof (int));
				ChipID.ProcessorName[48] = '\0';

				// Because some manufacturers (<cough>Intel</cough>) have leading white space - we have to post-process the name.
				if (ChipManufacturer == Intel)
				{
					for (int nCounter = 0; nCounter < CHIPNAME_STRING_LENGTH; nCounter ++)
					{
						// There will either be NULL (\0) or spaces ( ) as the leading characters.
						if ((ChipID.ProcessorName[nCounter] != '\0') && (ChipID.ProcessorName[nCounter] != ' ')) 
						{
							// We have found the starting position of the name.
							ProcessorNameStartPos = nCounter;

							// Terminate the loop.
							break;
						}
					}

					// Check to see if there is any white space at the start.
					if (ProcessorNameStartPos == 0) 
						return true;

					// Now move the name forward so that there is no white space.
					x_memmove (ChipID.ProcessorName, &(ChipID.ProcessorName[ProcessorNameStartPos]), (CHIPNAME_STRING_LENGTH - ProcessorNameStartPos));
				}

				return true;
			}

			bool sRetrieveClassicalCPUIdentity ()
			{
				// Start by decided which manufacturer we are using....
				switch (ChipManufacturer) 
				{
				case Intel:
					// Check the family / model / revision to determine the CPU ID.
					switch (ChipID.Family) 
					{
					case 3:
						x_sprintf (ChipID.ProcessorName, CHIPNAME_STRING_LENGTH, "Newer i80386 family"); 
						break;
					case 4:
						switch (ChipID.Model)
						{
						case 0: STORE_CLASSICAL_NAME ("i80486DX-25/33"); break;
						case 1: STORE_CLASSICAL_NAME ("i80486DX-50"); break;
						case 2: STORE_CLASSICAL_NAME ("i80486SX"); break;
						case 3: STORE_CLASSICAL_NAME ("i80486DX2"); break;
						case 4: STORE_CLASSICAL_NAME ("i80486SL"); break;
						case 5: STORE_CLASSICAL_NAME ("i80486SX2"); break;
						case 7: STORE_CLASSICAL_NAME ("i80486DX2 WriteBack"); break;
						case 8: STORE_CLASSICAL_NAME ("i80486DX4"); break;
						case 9: STORE_CLASSICAL_NAME ("i80486DX4 WriteBack"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 80486 family"); return false;
						}
						break;
					case 5:
						switch (ChipID.Model)
						{
						case 0: STORE_CLASSICAL_NAME ("P5 A-Step"); break;
						case 1: STORE_CLASSICAL_NAME ("P5"); break;
						case 2: STORE_CLASSICAL_NAME ("P54C"); break;
						case 3: STORE_CLASSICAL_NAME ("P24T OverDrive"); break;
						case 4: STORE_CLASSICAL_NAME ("P55C"); break;
						case 7: STORE_CLASSICAL_NAME ("P54C"); break;
						case 8: STORE_CLASSICAL_NAME ("P55C (0.25µm)"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Pentium® family"); return false;
						}
						break;
					case 6:
						switch (ChipID.Model)
						{
						case 0: STORE_CLASSICAL_NAME ("P6 A-Step"); break;
						case 1: STORE_CLASSICAL_NAME ("P6"); break;
						case 3: STORE_CLASSICAL_NAME ("Pentium® II (0.28 µm)"); break;
						case 5: STORE_CLASSICAL_NAME ("Pentium® II (0.25 µm)"); break;
						case 6: STORE_CLASSICAL_NAME ("Pentium® II With On-Die L2 Cache"); break;
						case 7: STORE_CLASSICAL_NAME ("Pentium® III (0.25 µm)"); break;
						case 8: STORE_CLASSICAL_NAME ("Pentium® III (0.18 µm) With 256 KB On-Die L2 Cache "); break;
						case 0xa: STORE_CLASSICAL_NAME ("Pentium® III (0.18 µm) With 1 Or 2 MB On-Die L2 Cache "); break;
						case 0xb: STORE_CLASSICAL_NAME ("Pentium® III (0.13 µm) With 256 Or 512 KB On-Die L2 Cache "); break;
						default: STORE_CLASSICAL_NAME ("Unknown P6 family"); return false;
						}
						break;
					case 7:
						STORE_CLASSICAL_NAME ("Intel Merced (IA-64)");
						break;
					case 0xf:
						// Check the extended family bits...
						switch (ChipID.ExtendedFamily)
						{
						case 0:
							switch (ChipID.Model) 
							{
							case 0: STORE_CLASSICAL_NAME ("Pentium® IV (0.18 µm)"); break;
							case 1: STORE_CLASSICAL_NAME ("Pentium® IV (0.18 µm)"); break;
							case 2: STORE_CLASSICAL_NAME ("Pentium® IV (0.13 µm)"); break;
							default: STORE_CLASSICAL_NAME ("Unknown Pentium 4 family"); return false;
							}
							break;
						case 1:
							STORE_CLASSICAL_NAME ("Intel McKinley (IA-64)");
							break;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown Intel family");
						return false;
					}
					break;

				case AMD:
					// Check the family / model / revision to determine the CPU ID.
					switch (ChipID.Family)
					{
					case 4:
						switch (ChipID.Model) 
						{
						case 3: STORE_CLASSICAL_NAME ("80486DX2"); break;
						case 7: STORE_CLASSICAL_NAME ("80486DX2 WriteBack"); break;
						case 8: STORE_CLASSICAL_NAME ("80486DX4"); break;
						case 9: STORE_CLASSICAL_NAME ("80486DX4 WriteBack"); break;
						case 0xe: STORE_CLASSICAL_NAME ("5x86"); break;
						case 0xf: STORE_CLASSICAL_NAME ("5x86WB"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 80486 family"); return false;
						}
						break;
					case 5:
						switch (ChipID.Model)
						{
						case 0: STORE_CLASSICAL_NAME ("SSA5 (PR75, PR90, PR100)"); break;
						case 1: STORE_CLASSICAL_NAME ("5k86 (PR120, PR133)"); break;
						case 2: STORE_CLASSICAL_NAME ("5k86 (PR166)"); break;
						case 3: STORE_CLASSICAL_NAME ("5k86 (PR200)"); break;
						case 6: STORE_CLASSICAL_NAME ("K6 (0.30 µm)"); break;
						case 7: STORE_CLASSICAL_NAME ("K6 (0.25 µm)"); break;
						case 8: STORE_CLASSICAL_NAME ("K6-2"); break;
						case 9: STORE_CLASSICAL_NAME ("K6-III"); break;
						case 0xd: STORE_CLASSICAL_NAME ("K6-2+ or K6-III+ (0.18 µm)"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 80586 family"); return false;
						}
						break;
					case 6:
						switch (ChipID.Model)
						{
						case 1: STORE_CLASSICAL_NAME ("Athlon™ (0.25 µm)"); break;
						case 2: STORE_CLASSICAL_NAME ("Athlon™ (0.18 µm)"); break;
						case 3: STORE_CLASSICAL_NAME ("Duron™ (SF core)"); break;
						case 4: STORE_CLASSICAL_NAME ("Athlon™ (Thunderbird core)"); break;
						case 6: STORE_CLASSICAL_NAME ("Athlon™ (Palomino core)"); break;
						case 7: STORE_CLASSICAL_NAME ("Duron™ (Morgan core)"); break;
						case 8: 
							if (Features.ExtendedFeatures.SupportsMP)
								STORE_CLASSICAL_NAME ("Athlon™ MP (Thoroughbred core)"); 
							else STORE_CLASSICAL_NAME ("Athlon™ XP (Thoroughbred core)");
							break;
						default: STORE_CLASSICAL_NAME ("Unknown K7 family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown AMD family");
						return false;
					}
					break;

				case Transmeta:
					switch (ChipID.Family)
					{	
					case 5:
						switch (ChipID.Model) 
						{
						case 4: STORE_CLASSICAL_NAME ("Crusoe TM3x00 and TM5x00"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Crusoe family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown Transmeta family");
						return false;
					}
					break;

				case Rise:
					switch (ChipID.Family)
					{	
					case 5:
						switch (ChipID.Model) 
						{
						case 0: STORE_CLASSICAL_NAME ("mP6 (0.25 µm)"); break;
						case 2: STORE_CLASSICAL_NAME ("mP6 (0.18 µm)"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Rise family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown Rise family");
						return false;
					}
					break;

				case UMC:
					switch (ChipID.Family) 
					{	
					case 4:
						switch (ChipID.Model) 
						{
						case 1: STORE_CLASSICAL_NAME ("U5D"); break;
						case 2: STORE_CLASSICAL_NAME ("U5S"); break;
						default: STORE_CLASSICAL_NAME ("Unknown UMC family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown UMC family");
						return false;
					}
					break;

				case IDT:
					switch (ChipID.Family)
					{	
					case 5:
						switch (ChipID.Model) 
						{
						case 4: STORE_CLASSICAL_NAME ("C6"); break;
						case 8: STORE_CLASSICAL_NAME ("C2"); break;
						case 9: STORE_CLASSICAL_NAME ("C3"); break;
						default: STORE_CLASSICAL_NAME ("Unknown IDT\\Centaur family"); return false;
						}
						break;
					case 6:
						switch (ChipID.Model)
						{
						case 6: STORE_CLASSICAL_NAME ("VIA Cyrix III - Samuel"); break;
						default: STORE_CLASSICAL_NAME ("Unknown IDT\\Centaur family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown IDT\\Centaur family");
						return false;
					}
					break;

				case Cyrix:
					switch (ChipID.Family)
					{	
					case 4:
						switch (ChipID.Model)
						{
						case 4: STORE_CLASSICAL_NAME ("MediaGX GX, GXm"); break;
						case 9: STORE_CLASSICAL_NAME ("5x86"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Cx5x86 family"); return false;
						}
						break;
					case 5:
						switch (ChipID.Model)
						{
						case 2: STORE_CLASSICAL_NAME ("Cx6x86"); break;
						case 4: STORE_CLASSICAL_NAME ("MediaGX GXm"); break;
						default: STORE_CLASSICAL_NAME ("Unknown Cx6x86 family"); return false;
						}
						break;
					case 6:
						switch (ChipID.Model)
						{
						case 0: STORE_CLASSICAL_NAME ("6x86MX"); break;
						case 5: STORE_CLASSICAL_NAME ("Cyrix M2 Core"); break;
						case 6: STORE_CLASSICAL_NAME ("WinChip C5A Core"); break;
						case 7: STORE_CLASSICAL_NAME ("WinChip C5B\\C5C Core"); break;
						case 8: STORE_CLASSICAL_NAME ("WinChip C5C-T Core"); break;
						default: STORE_CLASSICAL_NAME ("Unknown 6x86MX\\Cyrix III family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown Cyrix family");
						return false;
					}
					break;

				case NexGen:
					switch (ChipID.Family)
					{	
					case 5:
						switch (ChipID.Model)
						{
						case 0: STORE_CLASSICAL_NAME ("Nx586 or Nx586FPU"); break;
						default: STORE_CLASSICAL_NAME ("Unknown NexGen family"); return false;
						}
						break;
					default:
						STORE_CLASSICAL_NAME ("Unknown NexGen family");
						return false;
					}
					break;

				case NSC:
					STORE_CLASSICAL_NAME ("Cx486SLC \\ DLC \\ Cx486S A-Step");
					break;

				default:
					// We cannot identify the processor.
					STORE_CLASSICAL_NAME ("Unknown family");
					return false;
				}

				return true;
			}

			bool sDoesCPUSupportCPUID ()
			{
				int CPUIDPresent = 0;

				if (!sCpuID(0, NULL, NULL, NULL, NULL))
					CPUIDPresent = 1;

	#ifdef USE_RIGHT_CPUID_DETECTION
				// The "right" way, which doesn't work under certain Windows versions
				CPU_INFO_TRY 
				{
					_asm
					{
						pushfd                      ; save EFLAGS to stack.
						pop     eax                 ; store EFLAGS in eax.
						mov     edx, eax            ; save in ebx for testing later.
						xor     eax, 0200000h       ; switch bit 21.
						push    eax                 ; copy "changed" value to stack.
						popfd                       ; save "changed" eax to EFLAGS.
						pushfd
						pop     eax
						xor     eax, edx            ; See if bit changeable.
						jnz     short cpuid_present ; if so, mark 
						mov     eax, -1             ; CPUID not present - disable its usage
						jmp     no_features

	cpuid_present:
						mov		eax, 0				; CPUID capable CPU - enable its usage.

	no_features:
						mov     CPUIDPresent, eax	; Save the value in eax to a variable.
					}
				}

				// A generic catch-all just to be sure...
				CPU_INFO_CATCH_ALL 
				{
					// Stop the class from trying to use CPUID again!
					CPUIDPresent = false;
					return false;
				}

	#endif

				// Return true to indicate support or false to indicate lack.
				return (CPUIDPresent == 0) ? true : false;
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
				case Intel:
					return "Intel Corporation";
				case AMD:
					return "Advanced Micro Devices";
				case NSC:
					return "National Semiconductor";
				case Cyrix:
					return "Cyrix Corp., VIA Inc.";
				case NexGen:
					return "NexGen Inc., Advanced Micro Devices";
				case IDT:
					return "IDT\\Centaur, Via Inc.";
				case UMC:
					return "United Microelectronics Corp.";
				case Rise:
					return "Rise";
				case Transmeta:
					return "Transmeta";
				default:
					return "Unknown Manufacturer";
			}
		}

		const char * xinfo::getTypeID () const
		{
			// Return the type ID of the CPU.
			static char szTypeID[6] = { '\0' };
			if (szTypeID[0] == '\0')
				x_dtoa (ChipID.Type, szTypeID, sizeof(szTypeID), 10);
			return szTypeID;
		}

		const char * xinfo::getFamilyID () const
		{
			// Return the family of the CPU present.
			static char szFamilyID[6] = { '\0' };
			if (szFamilyID[0] == '\0')
				x_dtoa (ChipID.Family, szFamilyID, sizeof(szFamilyID), 10);
			return szFamilyID;
		}

		const char * xinfo::getModelID () const
		{
			// Return the model of CPU present.
			static char szModelID[6] = { '\0' };
			if (szModelID[0] == '\0')
				x_dtoa (ChipID.Model, szModelID, sizeof(szModelID), 10);
			return szModelID;
		}

		const char * xinfo::getSteppingCode () const
		{
			// Return the stepping code of the CPU present.
			static char szSteppingCode[6] = { '\0' };
			if (szSteppingCode[0] == '\0')
				x_dtoa (ChipID.Revision, szSteppingCode, sizeof(szSteppingCode), 10);
			return szSteppingCode;
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

			// Check for MMX instructions.
			if (((dwFeature & MMX_FEATURE) != 0) && Features.HasMMX) bHasFeature = true;

			// Check for MMX+ instructions.
			if (((dwFeature & MMX_PLUS_FEATURE) != 0) && Features.ExtendedFeatures.HasMMXPlus) bHasFeature = true;

			// Check for SSE FP instructions.
			if (((dwFeature & SSE_FEATURE) != 0) && Features.HasSSE) bHasFeature = true;

			// Check for SSE FP instructions.
			if (((dwFeature & SSE_FP_FEATURE) != 0) && Features.HasSSEFP) bHasFeature = true;

			// Check for SSE MMX instructions.
			if (((dwFeature & SSE_MMX_FEATURE) != 0) && Features.ExtendedFeatures.HasSSEMMX) bHasFeature = true;

			// Check for SSE2 instructions.
			if (((dwFeature & SSE2_FEATURE) != 0) && Features.HasSSE2) bHasFeature = true;

			// Check for 3DNow! instructions.
			if (((dwFeature & AMD_3DNOW_FEATURE) != 0) && Features.ExtendedFeatures.Has3DNow) bHasFeature = true;

			// Check for 3DNow+ instructions.
			if (((dwFeature & AMD_3DNOW_PLUS_FEATURE) != 0) && Features.ExtendedFeatures.Has3DNowPlus) bHasFeature = true;

			// Check for IA64 instructions.
			if (((dwFeature & IA64_FEATURE) != 0) && Features.HasIA64) bHasFeature = true;

			// Check for MP capable.
			if (((dwFeature & MP_CAPABLE) != 0) && Features.ExtendedFeatures.SupportsMP) bHasFeature = true;

			// Check for a serial number for the processor.
			if (((dwFeature & SERIALNUMBER_FEATURE) != 0) && Features.HasSerial) bHasFeature = true;

			// Check for a local APIC in the processor.
			if (((dwFeature & APIC_FEATURE) != 0) && Features.HasAPIC) bHasFeature = true;

			// Check for CMOV instructions.
			if (((dwFeature & CMOV_FEATURE) != 0) && Features.HasCMOV) bHasFeature = true;

			// Check for MTRR instructions.
			if (((dwFeature & MTRR_FEATURE) != 0) && Features.HasMTRR) bHasFeature = true;

			// Check for L1 cache size.
			if (((dwFeature & L1CACHE_FEATURE) != 0) && (Features.L1CacheSize != -1)) bHasFeature = true;

			// Check for L2 cache size.
			if (((dwFeature & L2CACHE_FEATURE) != 0) && (Features.L2CacheSize != -1)) bHasFeature = true;

			// Check for L3 cache size.
			if (((dwFeature & L3CACHE_FEATURE) != 0) && (Features.L3CacheSize != -1)) bHasFeature = true;

			// Check for ACPI capability.
			if (((dwFeature & ACPI_FEATURE) != 0) && Features.HasACPI) bHasFeature = true;

			// Check for thermal monitor support.
			if (((dwFeature & THERMALMONITOR_FEATURE) != 0) && Features.HasThermal) bHasFeature = true;

			// Check for temperature sensing diode support.
			if (((dwFeature & TEMPSENSEDIODE_FEATURE) != 0) && Features.ExtendedFeatures.PowerManagement.HasTempSenseDiode) bHasFeature = true;

			// Check for frequency ID support.
			if (((dwFeature & FREQUENCYID_FEATURE) != 0) && Features.ExtendedFeatures.PowerManagement.HasFrequencyID) bHasFeature = true;

			// Check for voltage ID support.
			if (((dwFeature & VOLTAGEID_FREQUENCY) != 0) && Features.ExtendedFeatures.PowerManagement.HasVoltageID) bHasFeature = true;

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
		#define RDTSC_INSTRUCTION			_asm _emit 0x0f _asm _emit 0x31
		#define	CPUSPEED_I32TO64(x, y)		(((s64) x << 32) + y)

		typedef	void (*DELAY_FUNC)(u32 uiMS);

		static s64 sGetCyclesDifference (DELAY_FUNC DelayFunction, u32 uiParameter)
		{
			u32 edx1, eax1;
			u32 edx2, eax2;

			// Calculate the frequency of the CPU instructions.
			CPU_INFO_TRY 
			{
				_asm
				{
					push uiParameter		; push parameter param
					mov ebx, DelayFunction	; store func in ebx

					RDTSC_INSTRUCTION

					mov esi, eax			; esi = eax
					mov edi, edx			; edi = edx

					call ebx				; call the delay functions

					RDTSC_INSTRUCTION

					pop ebx

					mov edx2, edx			; edx2 = edx
					mov eax2, eax			; eax2 = eax

					mov edx1, edi			; edx2 = edi
					mov eax1, esi			; eax2 = esi
				}
			}

			// A generic catch-all just to be sure...
			CPU_INFO_CATCH_ALL
			{
				return -1;
			}

			return (CPUSPEED_I32TO64 (edx2, eax2) - CPUSPEED_I32TO64 (edx1, eax1));
		}


		static void sCPUSpeed_Delay (u32 uiMS)
		{
			LARGE_INTEGER Frequency, StartCounter, EndCounter;
			s64 x;

			// Get the frequency of the high performance counter.
			if (!QueryPerformanceFrequency (&Frequency)) 
				return;

			x = Frequency.QuadPart / 1000 * uiMS;

			// Get the starting position of the counter.
			QueryPerformanceCounter (&StartCounter);

			do
			{
				// Get the ending position of the counter.	
				QueryPerformanceCounter (&EndCounter);
			} while (EndCounter.QuadPart - StartCounter.QuadPart < x);
		}

		static void sCPUSpeed_DelayOverhead (u32 uiMS)
		{
			LARGE_INTEGER Frequency, StartCounter, EndCounter;
			s64 x;

			// Get the frequency of the high performance counter.
			if (!QueryPerformanceFrequency (&Frequency))
				return;

			x = Frequency.QuadPart / 1000 * uiMS;

			// Get the starting position of the counter.
			QueryPerformanceCounter (&StartCounter);

			do
			{
				// Get the ending position of the counter.	
				QueryPerformanceCounter (&EndCounter);
			} while (EndCounter.QuadPart - StartCounter.QuadPart == x);
		}

		static u64 sCPUSpeedInkHz = 0; 

		xspeed::xspeed ()
		{
		}

		void xspeed::calculate ()
		{
			if (sCPUSpeedInkHz == 0)
			{
				u32 uiRepetitions = 1;
				u32 uiMSecPerRepetition = 50;
				s64	i64Total = 0, i64Overhead = 0;

				for (u32 nCounter = 0; nCounter < uiRepetitions; nCounter ++) 
				{
					i64Total    += sGetCyclesDifference (sCPUSpeed_Delay, uiMSecPerRepetition);
					i64Overhead += sGetCyclesDifference (sCPUSpeed_DelayOverhead, uiMSecPerRepetition);
				}

				// Calculate the MHz speed.
				i64Total -= i64Overhead;
				i64Total /= uiRepetitions;
				i64Total /= uiMSecPerRepetition;

				// Save the CPU speed.
				sCPUSpeedInkHz = i64Total;
			}
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

#endif // TARGET_PC
