#pragma once
#pragma comment(lib, "ntdll.lib")
#include <Windows.h>

#define PAGE_SIZE 0x1000
#define PFN_TO_PAGE(pfn) ( pfn << 12 )

namespace eprocess_offset
{
	// windows 1803 / 1809
	// should find these dynamically
	constexpr auto DirectoryTableBase	= 0x28;
	constexpr auto UniqueProcessId		= 0x2e0;
	constexpr auto ActiveProcessLinks	= 0x2e8;
	constexpr auto SectionBaseAddress	= 0x3c0;
}

namespace translation
{
	typedef union _CR3
	{
		uint64_t value;
		struct
		{
			uint64_t ignored_1 : 3;
			uint64_t write_through : 1;
			uint64_t cache_disable : 1;
			uint64_t ignored_2 : 7;
			uint64_t pml4_p : 40;
			uint64_t reserved : 12;
		};
	} PTE_CR3;

	typedef union _VIRT_ADDR
	{
		uint64_t value;
		struct
		{
			uint64_t offset : 12;
			uint64_t pt_index : 9;
			uint64_t pd_index : 9;
			uint64_t pdpt_index : 9;
			uint64_t pml4_index : 9;
			uint64_t reserved : 16;
		};
	} VIRT_ADDR;

	typedef union _PML4E
	{
		uint64_t value;
		struct
		{
			uint64_t present : 1;
			uint64_t rw : 1;
			uint64_t user : 1;
			uint64_t write_through : 1;
			uint64_t cache_disable : 1;
			uint64_t accessed : 1;
			uint64_t ignored_1 : 1;
			uint64_t reserved_1 : 1;
			uint64_t ignored_2 : 4;
			uint64_t pdpt_p : 40;
			uint64_t ignored_3 : 11;
			uint64_t xd : 1;
		};
	} PML4E;

	typedef union _PDPTE
	{
		uint64_t value;
		struct
		{
			uint64_t present : 1;
			uint64_t rw : 1;
			uint64_t user : 1;
			uint64_t write_through : 1;
			uint64_t cache_disable : 1;
			uint64_t accessed : 1;
			uint64_t dirty : 1;
			uint64_t page_size : 1;
			uint64_t ignored_2 : 4;
			uint64_t pd_p : 40;
			uint64_t ignored_3 : 11;
			uint64_t xd : 1;
		};
	} PDPTE;

	typedef union _PDE
	{
		uint64_t value;
		struct
		{
			uint64_t present : 1;
			uint64_t rw : 1;
			uint64_t user : 1;
			uint64_t write_through : 1;
			uint64_t cache_disable : 1;
			uint64_t accessed : 1;
			uint64_t dirty : 1;
			uint64_t page_size : 1;
			uint64_t ignored_2 : 4;
			uint64_t pt_p : 40;
			uint64_t ignored_3 : 11;
			uint64_t xd : 1;
		};
	} PDE;

	typedef union _PTE
	{
		uint64_t value;
		struct
		{
			uint64_t present : 1;
			uint64_t rw : 1;
			uint64_t user : 1;
			uint64_t write_through : 1;
			uint64_t cache_disable : 1;
			uint64_t accessed : 1;
			uint64_t dirty : 1;
			uint64_t pat : 1;
			uint64_t global : 1;
			uint64_t ignored_1 : 3;
			uint64_t page_frame : 40;
			uint64_t ignored_3 : 11;
			uint64_t xd : 1;
		};
	} PTE;
}

namespace native
{
	using virt_addr		= std::uint64_t;
	using phys_addr		= std::uint64_t;
	using pid_t			= std::uint32_t;

	typedef enum _SYSTEM_INFORMATION_CLASS
	{
		SystemBasicInformation,
		SystemProcessorInformation,
		SystemPerformanceInformation,
		SystemTimeOfDayInformation,
		SystemPathInformation,
		SystemProcessInformation,
		SystemCallCountInformation,
		SystemDeviceInformation,
		SystemProcessorPerformanceInformation,
		SystemFlagsInformation,
		SystemCallTimeInformation,
		SystemModuleInformation,
		SystemLocksInformation,
		SystemStackTraceInformation,
		SystemPagedPoolInformation,
		SystemNonPagedPoolInformation,
		SystemHandleInformation,
		SystemObjectInformation,
		SystemPageFileInformation,
		SystemVdmInstemulInformation,
		SystemVdmBopInformation,
		SystemFileCacheInformation,
		SystemPoolTagInformation,
		SystemInterruptInformation,
		SystemDpcBehaviorInformation,
		SystemFullMemoryInformation,
		SystemLoadGdiDriverInformation,
		SystemUnloadGdiDriverInformation,
		SystemTimeAdjustmentInformation,
		SystemSummaryMemoryInformation,
		SystemMirrorMemoryInformation,
		SystemPerformanceTraceInformation,
		SystemObsolete0,
		SystemExceptionInformation,
		SystemCrashDumpStateInformation,
		SystemKernelDebuggerInformation,
		SystemContextSwitchInformation,
		SystemRegistryQuotaInformation,
		SystemExtendServiceTableInformation,
		SystemPrioritySeperation,
		SystemPlugPlayBusInformation,
		SystemDockInformation,
		SystemPowerInformationNative,
		SystemProcessorSpeedInformation,
		SystemCurrentTimeZoneInformation,
		SystemLookasideInformation,
		SystemTimeSlipNotification,
		SystemSessionCreate,
		SystemSessionDetach,
		SystemSessionInformation,
		SystemRangeStartInformation,
		SystemVerifierInformation,
		SystemAddVerifier,
		SystemSessionProcessesInformation,
		SystemLoadGdiDriverInSystemSpaceInformation,
		SystemNumaProcessorMap,
		SystemPrefetcherInformation,
		SystemExtendedProcessInformation,
		SystemRecommendedSharedDataAlignment,
		SystemComPlusPackage,
		SystemNumaAvailableMemory,
		SystemProcessorPowerInformation,
		SystemEmulationBasicInformation,
		SystemEmulationProcessorInformation,
		SystemExtendedHanfleInformation,
		SystemLostDelayedWriteInformation,
		SystemBigPoolInformation,
		SystemSessionPoolTagInformation,
		SystemSessionMappedViewInformation,
		SystemHotpatchInformation,
		SystemObjectSecurityMode,
		SystemWatchDogTimerHandler,
		SystemWatchDogTimerInformation,
		SystemLogicalProcessorInformation,
		SystemWo64SharedInformationObosolete,
		SystemRegisterFirmwareTableInformationHandler,
		SystemFirmwareTableInformation,
		SystemModuleInformationEx,
		SystemVerifierTriageInformation,
		SystemSuperfetchInformation,
		SystemMemoryListInformation,
		SystemFileCacheInformationEx,
		SystemThreadPriorityClientIdInformation,
		SystemProcessorIdleCycleTimeInformation,
		SystemVerifierCancellationInformation,
		SystemProcessorPowerInformationEx,
		SystemRefTraceInformation,
		SystemSpecialPoolInformation,
		SystemProcessIdInformation,
		SystemErrorPortInformation,
		SystemBootEnvironmentInformation,
		SystemHypervisorInformation,
		SystemVerifierInformationEx,
		SystemTimeZoneInformation,
		SystemImageFileExecutionOptionsInformation,
		SystemCoverageInformation,
		SystemPrefetchPathInformation,
		SystemVerifierFaultsInformation,
		MaxSystemInfoClass,
	} SYSTEM_INFORMATION_CLASS;

	typedef struct _RTL_PROCESS_MODULE_INFORMATION
	{
		ULONG  Section;
		PVOID  MappedBase;
		PVOID  ImageBase;
		ULONG  ImageSize;
		ULONG  Flags;
		USHORT LoadOrderIndex;
		USHORT InitOrderIndex;
		USHORT LoadCount;
		USHORT OffsetToFileName;
		CHAR   FullPathName[256];
	} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

	typedef struct _RTL_PROCESS_MODULES
	{
		ULONG NumberOfModules;
		RTL_PROCESS_MODULE_INFORMATION Modules[1];
	} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

	extern "C" NTSTATUS WINAPI NtQuerySystemInformation(
		IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
		OUT PVOID SystemInformation,
		IN ULONG SystemInformationLength,
		OUT PULONG ReturnLength OPTIONAL
	);
}