#ifndef HACKS_H_ 
#define HACKS_H_ 

#include <ntifs.h>


extern int procID, memaddr;

extern NTSTATUS SystemRoutine();
char * GetProcessNameFromPid(HANDLE pid);
extern UCHAR *PsGetProcessImageFileName(IN PEPROCESS Process);
extern   NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS *Process);

NTSTATUS Sleep(ULONGLONG milliseconds);
typedef NTSTATUS(__fastcall *NtQuerySystemInformation)(ULONG infoclass, void *buffer, ULONG infolen, ULONG *plen);
typedef void*(_fastcall *PsGetProcessPeb)(PEPROCESS a1);
typedef void*(_fastcall *PsGetProcessWow64Process)(PEPROCESS a1);

NtQuerySystemInformation ZwQuerySystemInformation;
PsGetProcessPeb PsGetPeb64;
PsGetProcessWow64Process PsGetPeb32;


typedef struct _PASSDATA
{
	ULONG pid;
	ULONG address;
	BOOLEAN controlswitch;
} PASSDATA, *PASSDATACTRL;
NTSTATUS shotbot_thread(PASSDATACTRL pd);

struct SYSTEM_PROCESS_INFORMATION
{
	ULONG NextEntryOffset;
	char Reserved1[52];
	PVOID Reserved2[3];
	HANDLE UniqueProcessId;
	PVOID Reserved3;
	ULONG HandleCount;
	char Reserved4[4];
	PVOID Reserved5[11];
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER Reserved6[6];
};


// Query system information
typedef enum _SYSTEM_INFORMATION_CLASS
{
	SystemBasicInformation,                         // 0x00 SYSTEM_BASIC_INFORMATION
	SystemProcessorInformation,                     // 0x01 SYSTEM_PROCESSOR_INFORMATION
	SystemPerformanceInformation,                   // 0x02
	SystemTimeOfDayInformation,                     // 0x03
	SystemPathInformation,                          // 0x04
	SystemProcessInformation,                       // 0x05
	SystemCallCountInformation,                     // 0x06
	SystemDeviceInformation,                        // 0x07
	SystemProcessorPerformanceInformation,          // 0x08
	SystemFlagsInformation,                         // 0x09
	SystemCallTimeInformation,                      // 0x0A
	SystemModuleInformation,                        // 0x0B SYSTEM_MODULE_INFORMATION
	SystemLocksInformation,                         // 0x0C
	SystemStackTraceInformation,                    // 0x0D
	SystemPagedPoolInformation,                     // 0x0E
	SystemNonPagedPoolInformation,                  // 0x0F
	SystemHandleInformation,                        // 0x10
	SystemObjectInformation,                        // 0x11
	SystemPageFileInformation,                      // 0x12
	SystemVdmInstemulInformation,                   // 0x13
	SystemVdmBopInformation,                        // 0x14
	SystemFileCacheInformation,                     // 0x15
	SystemPoolTagInformation,                       // 0x16
	SystemInterruptInformation,                     // 0x17
	SystemDpcBehaviorInformation,                   // 0x18
	SystemFullMemoryInformation,                    // 0x19
	SystemLoadGdiDriverInformation,                 // 0x1A
	SystemUnloadGdiDriverInformation,               // 0x1B
	SystemTimeAdjustmentInformation,                // 0x1C
	SystemSummaryMemoryInformation,                 // 0x1D
	SystemMirrorMemoryInformation,                  // 0x1E
	SystemPerformanceTraceInformation,              // 0x1F
	SystemObsolete0,                                // 0x20
	SystemExceptionInformation,                     // 0x21
	SystemCrashDumpStateInformation,                // 0x22
	SystemKernelDebuggerInformation,                // 0x23
	SystemContextSwitchInformation,                 // 0x24
	SystemRegistryQuotaInformation,                 // 0x25
	SystemExtendServiceTableInformation,            // 0x26
	SystemPrioritySeperation,                       // 0x27
	SystemPlugPlayBusInformation,                   // 0x28
	SystemDockInformation,                          // 0x29
	SystemPowerInformationNative,                   // 0x2A
	SystemProcessorSpeedInformation,                // 0x2B
	SystemCurrentTimeZoneInformation,               // 0x2C
	SystemLookasideInformation,                     // 0x2D
	SystemTimeSlipNotification,                     // 0x2E
	SystemSessionCreate,                            // 0x2F
	SystemSessionDetach,                            // 0x30
	SystemSessionInformation,                       // 0x31
	SystemRangeStartInformation,                    // 0x32
	SystemVerifierInformation,                      // 0x33
	SystemAddVerifier,                              // 0x34
	SystemSessionProcessesInformation,              // 0x35
	SystemLoadGdiDriverInSystemSpaceInformation,    // 0x36
	SystemNumaProcessorMap,                         // 0x37
	SystemPrefetcherInformation,                    // 0x38
	SystemExtendedProcessInformation,               // 0x39
	SystemRecommendedSharedDataAlignment,           // 0x3A
	SystemComPlusPackage,                           // 0x3B
	SystemNumaAvailableMemory,                      // 0x3C
	SystemProcessorPowerInformation,                // 0x3D
	SystemEmulationBasicInformation,                // 0x3E
	SystemEmulationProcessorInformation,            // 0x3F
	SystemExtendedHanfleInformation,                // 0x40
	SystemLostDelayedWriteInformation,              // 0x41
	SystemBigPoolInformation,                       // 0x42
	SystemSessionPoolTagInformation,                // 0x43
	SystemSessionMappedViewInformation,             // 0x44
	SystemHotpatchInformation,                      // 0x45
	SystemObjectSecurityMode,                       // 0x46
	SystemWatchDogTimerHandler,                     // 0x47
	SystemWatchDogTimerInformation,                 // 0x48
	SystemLogicalProcessorInformation,              // 0x49
	SystemWo64SharedInformationObosolete,           // 0x4A
	SystemRegisterFirmwareTableInformationHandler,  // 0x4B
	SystemFirmwareTableInformation,                 // 0x4C
	SystemModuleInformationEx,                      // 0x4D
	SystemVerifierTriageInformation,                // 0x4E
	SystemSuperfetchInformation,                    // 0x4F
	SystemMemoryListInformation,                    // 0x50
	SystemFileCacheInformationEx,                   // 0x51
	SystemThreadPriorityClientIdInformation,        // 0x52
	SystemProcessorIdleCycleTimeInformation,        // 0x53
	SystemVerifierCancellationInformation,          // 0x54
	SystemProcessorPowerInformationEx,              // 0x55
	SystemRefTraceInformation,                      // 0x56
	SystemSpecialPoolInformation,                   // 0x57
	SystemProcessIdInformation,                     // 0x58
	SystemErrorPortInformation,                     // 0x59
	SystemBootEnvironmentInformation,               // 0x5A SYSTEM_BOOT_ENVIRONMENT_INFORMATION
	SystemHypervisorInformation,                    // 0x5B
	SystemVerifierInformationEx,                    // 0x5C
	SystemTimeZoneInformation,                      // 0x5D
	SystemImageFileExecutionOptionsInformation,     // 0x5E
	SystemCoverageInformation,                      // 0x5F
	SystemPrefetchPathInformation,                  // 0x60
	SystemVerifierFaultsInformation,                // 0x61
	MaxSystemInfoClass                              // 0x67

} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_MODULE
{
	ULONG  Reserved1;                   // Should be 0xBAADF00D
	ULONG  Reserved2;                   // Should be zero
	PVOID  Base;
	ULONG  Size;
	ULONG  Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR   ImageName[256];

} SYSTEM_MODULE, *PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION
{
	ULONG         ModulesCount;
	SYSTEM_MODULE Modules[1];

} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;




typedef struct _PEB_LDR_DATA
{
	ULONG Length;
	UCHAR Initialized;
	PVOID SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_DATA_TABLE_ENTRY
{
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


typedef struct _PEB
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	PVOID Mutant;
	PVOID ImageBaseAddress;
	PPEB_LDR_DATA Ldr;
	PVOID ProcessParameters;
	PVOID SubSyste
		;
	PVOID ProcessHeap;
	PVOID FastPebLock;
	PVOID AtlThunkSListPtr;
	PVOID IFEOKey;
	PVOID CrossProcessFlags;
	PVOID UserSharedInfoPtr;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	PVOID ApiSetMap;
} PEB, *PPEB;

typedef struct _PEB_LDR_DATA32
{
	ULONG Length;
	UCHAR Initialized;
	ULONG SsHandle;
	LIST_ENTRY32 InLoadOrderModuleList;
	LIST_ENTRY32 InMemoryOrderModuleList;
	LIST_ENTRY32 InInitializationOrderModuleList;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _LDR_DATA_TABLE_ENTRY32
{
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderLinks;
	LIST_ENTRY32 InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY32 HashLinks;
	ULONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

typedef struct _PEB32
{
	UCHAR InheritedAddressSpace;
	UCHAR ReadImageFileExecOptions;
	UCHAR BeingDebugged;
	UCHAR BitField;
	ULONG Mutant;
	ULONG ImageBaseAddress;
	ULONG Ldr;
	ULONG ProcessParameters;
	ULONG SubSystemData;
	ULONG ProcessHeap;
	ULONG FastPebLock;
	ULONG AtlThunkSListPtr;
	ULONG IFEOKey;
	ULONG CrossProcessFlags;
	ULONG UserSharedInfoPtr;
	ULONG SystemReserved;
	ULONG AtlThunkSListPtr32;
	ULONG ApiSetMap;
} PEB32, *PPEB32;

extern NTSTATUS ObOpenObjectByPointer(
	_In_      PVOID           Object,
	_In_      ULONG           HandleAttributes,
	_In_opt_  PACCESS_STATE   PassedAccessState,
	_In_      ACCESS_MASK     DesiredAccess,
	_In_opt_  POBJECT_TYPE    ObjectType,
	_In_      KPROCESSOR_MODE AccessMode,
	_Out_     PHANDLE         Handle
	);

LONG RtlCompareString(
	_In_  const STRING  *String1,
	_In_  const STRING  *String2,
	_In_  BOOLEAN CaseInSensitive
	);



extern NTSTATUS ZwOpenProcess(
	_Out_     PHANDLE            ProcessHandle,
	_In_      ACCESS_MASK        DesiredAccess,
	_In_      POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_  PCLIENT_ID         ClientId
	);


#endif
