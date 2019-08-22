/*************************************************************************/ /*!
@File           vmm_type_l4uvmm.c
@Title          Fiasco.OC L4LINUX VM manager type
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Fiasco.OC L4 UVMM VM manager implementation
@License        Strictly Confidential.
*/ /**************************************************************************/
#include <linux/module.h>

#include "pvrsrv.h"
#include "img_types.h"
#include "pvrsrv_error.h"
#include "rgxheapconfig.h"
#include "interrupt_support.h"
#include "dma_support.h"
#include "vmm_impl.h"
#include "vz_physheap.h"
#include "vmm_pvz_server.h"
#include "vz_vm.h"

static IMG_UINT32 gui32SysDevMemFOrigin = 1;
static IMG_UINT64 gui64SysDevMemFBase   = 0;
static IMG_UINT64 gui64SysDevMemFSize   = RGX_FIRMWARE_RAW_HEAP_SIZE;
static IMG_UINT64 gui64SysVMInfoPg      = 0x48000000;

module_param_named(sys_devmem_forigin, gui32SysDevMemFOrigin, uint, S_IRUGO | S_IWUSR);
module_param_named(sys_devmem_fbase,   gui64SysDevMemFBase, ullong, S_IRUGO | S_IWUSR);
module_param_named(sys_devmem_infopg,  gui64SysVMInfoPg,    ullong, S_IRUGO | S_IWUSR);

static	IMG_HANDLE ghThrd = {0};
static IMG_UINT64 *gpuiVMInfoPg = {0};
static DMA_ALLOC gsDmaAlloc = {0};

static void L4x_VM_InfoPg_Poll(void *pvData);

/*
The following options are avail. for VMM_TYPE without hypercalls
	- This VZ setup layer runs a 2 or more VM setup
		+ VM0 runs GPU host/primary driver
			* Only 1 host driver in a setup
			* Host driver is priviledge, loads FW
			* And also submits graphics work
		+ VM1+ runs GPU guest/secondary driver
			* Multiple guest drivers in setup up to 7.
			* Guest drivers are not priviledge
			* Guest drivers can only submit graphics work
	- This VZ setup uses an identity hypervisor/VM IPA to PA mapping
		+ PA outside of the VM is identity mapped to IPA within VM
			*  PA: Physical Addresses, values programmed into DMA agents
			* IPA: Intermediate PA, i.e. PAs seen by GuestOS in VM.
			* IPA within VM is identical to PA outside of VM
	- This VZ setup uses a shared-memory for cross-VM communication
		+ This single page 4K is called a VM information page
			* Located at 0x48000000 in this setup, size 4KB
			* Only used for quasi-dynamic VZ setup
	- Static VZ setup (Option 1)
		- Setup does not have runtime hypervisor hypercall mechanisim.
		- System integrator has apriori set-aside RAM region for FW use.
			+ RAM region is not managed by any OS(es)/Hypervisor agent
			+ Nor available to any other SW agent for its use
		- System integrator guarantees the following for static VZ setup
			+ Host VM/driver must come online before guest VM/driver
			+ Host has initialized FW before guest submits work
		- In this example:
			+ gui64SysDevMemFBase has been specified during module load
			+ Guest inteprets ui64SysDevMemF{Base,Size} as FW heap
			+ These are returned to upper DDK layers for VZ bootstraping.
			+ NOTE: gui64SysDevMemFSize is RGX_FIRMWARE_RAW_HEAP_SIZE
				* This is a system/firmware wide constant.
				* Adjustable during build via RGX_FW_HEAP_SHIFT=X
				* RGX_FIRMWARE_RAW_HEAP_SIZE = 1 << RGX_FW_HEAP_SHIFT
				* RGX_FW_HEAP_SHIFT=23 equals 1 << 2 = 8MB
				* Value are 4MB (tight budget) upto 32MB (plenty ram)
			+ RAM region is as follows for PVRSRV_VZ_NUM_OSID=3 setup:
				* w<---- host ---->x<---- guest1 ---->y---- guest 2 ---->x
				* x - y = PVRSRV_VZ_NUM_OSID * RGX_FIRMWARE_RAW_HEAP_SIZE
				* x - y = y - x = y - w = RGX_FIRMWARE_RAW_HEAP_SIZE
				* Range between w and x is physically contiguous
			+ Each driver is loaded with right value for gui64SysDevMemFBase
				* Host driver,    gui64SysDevMemFBase=w
				* Guest 1 driver, gui64SysDevMemFBase=x
				* Guest 2 driver, gui64SysDevMemFBase=y
	- Quasi-dynamic VZ setup (Option 2)
		- Setup does not have runtime hypervisor hypercall mechanisim.
		- Each driver type uses different FW heap configuration spec.
			+ Host driver uses variable-sized dynamic alloc. for FW
				* Via calls to OS kernel dynamic allocator
				* Spec. is *pui64Addr=0 and *pui64Size=0
				* Host driver inteprets this as dynamic
				* Same setup spec. for non-VZ driver mode
			+ Guest driver(s) uses fixed-sized dynamic alloc. for FW
				* Via calls to OS kernel DMA/CMA allocator
				* Guest sub-allocates FW objects from this DMA buffer
				* Buffer has to be mapped into guest FW context by host
				* Guest uses VM info. page to report buffer [I]PA to host
*/
static PVRSRV_ERROR
LxSetUpFWPhysHeapAddrSize(PVRSRV_DEVICE_CONFIG *psDevConfig,
						  IMG_UINT64 *pui64Addr,
						  IMG_UINT64 *pui64Size)
{
	PVRSRV_ERROR eError = PVRSRV_OK;

	if (PVRSRV_VZ_MODE_IS(DRIVER_MODE_GUEST))
	{
		if (gui64SysDevMemFBase)
		{
			/*
				+ Static VZ setup
				+ RAM region with base at gui64SysDevMemFBase must be:
					* Physically contiguous in it's entire length/size
					* Large enough to house this guest driver FW heap
					* *pui64Size = RGX_FIRMWARE_RAW_HEAP_SIZE
			 */
			*pui64Addr = gui64SysDevMemFBase;
			*pui64Size = gui64SysDevMemFSize;
		}
		else if (gui32SysDevMemFOrigin)
		{
			/*
				+ Quasi-dynamic VZ setup
				+ DMA alloc. must be large to house guest entire FW heap	
			*/
			IMG_CPU_PHYADDR sPAddr = {gui64SysVMInfoPg};
			IMG_UINT32 uiVMID = PVRSRV_VZ_DRIVER_OSID;
			gsDmaAlloc.ui64Size = gui64SysDevMemFSize;
			gsDmaAlloc.pvOSDevice = psDevConfig->pvOSDevice;

			/*
				Using DMA physheap for FW alloc., check VM information page is valid; this page is
				multi-mapped into all VMs. Each guest driver writes into it's own offset and the
				host driver reads from it. This means type module paramter for the guest needs to
				be specified with an ID (i.e. DriverMode=0 (host), DriverMode=[1 upto 7] (guests))
			*/
			if (! gui64SysVMInfoPg)
			{
				PVR_LOG(("Invalid VM info. pg. address 0x0"));
				eError = PVRSRV_ERROR_INVALID_PARAMS;
				goto e0;
			}

			/* 
				VM infopg. is valid, so map it and write into it the GuestOS obtained DMA alloc. address.
				This assumes an identity mapping between SoC PA and VM IPA (i.e. that is there is no DMA
				ioremapping in front of the GPU).
			*/
			gpuiVMInfoPg = (IMG_UINT64 *) OSMapPhysToLin(sPAddr, OSGetPageSize(), PVRSRV_MEMALLOCFLAG_CPU_UNCACHED);
			PVR_LOG(("VM infopg. @ PA 0x%p mapped to VA 0x%p", (void*)gui64SysVMInfoPg, (void*)gpuiVMInfoPg));

			/* DMA alloc. FW physheap & write into VM infopg. */
			eError = SysDmaAllocMem(&gsDmaAlloc);
			PVR_LOGG_IF_ERROR(eError, "SysDmaAllocMem", e0);

			/* Can't ioremap DMA physheap, driver manages translations */
			eError = SysDmaRegisterForIoRemapping(&gsDmaAlloc);
			PVR_LOGG_IF_ERROR(eError, "SysDmaRegisterForIoRemapping", e0);
			PVR_LOG(("Allocated FW DMA physheap: 0x%p/0x%llx", (void*)gsDmaAlloc.sBusAddr.uiAddr, gsDmaAlloc.ui64Size));

			/* Write this guest driver FW physheap PA address into it's VM info. page offset */
			PVR_LOG(("Writing 0x%llx to VM infopg. @ 0x%p", gsDmaAlloc.sBusAddr.uiAddr, &gpuiVMInfoPg[uiVMID]));
			gpuiVMInfoPg[uiVMID] = gsDmaAlloc.sBusAddr.uiAddr;

			/* Store PA/SZ for later look-up requests, also present DMA region to upper-stack as UMA carve-out */
			gui64SysDevMemFBase = gsDmaAlloc.sBusAddr.uiAddr;
			gui64SysDevMemFSize = gsDmaAlloc.ui64Size;
			*pui64Addr = gsDmaAlloc.sBusAddr.uiAddr;
			*pui64Size = gsDmaAlloc.ui64Size;
		}
		else
		{
			/*
				+ Dynamic VZ setup
				+ Centralized FW heap configuration in host driver
				+ Guest driver looks-up its FW info. from host driver
				+ Not possible seeing no actual hypercall exits.
			*/
			IMG_PCHAR pzNotice = "Unsupported PVZ setup config, requires hypercall mechanism";
			PVR_LOGG_IF_ERROR(PVRSRV_ERROR_INVALID_PVZ_CONFIG, pzNotice, e0);
		}
	}	

	if (PVRSRV_VZ_MODE_IS(DRIVER_MODE_HOST))
	{
		if (gui64SysDevMemFBase)
		{
			/*
				+ Static VZ setup
				+ RAM region with base at gui64SysDevMemFBase must be:
					* Physically contiguous in it's entire length/size
					* Large enough to house _all_ drivers FW heap
						# size = PVRSRV_VZ_NUM_OSID * RGX_FIRMWARE_RAW_HEAP_SIZE
				+ Heap sub-range corresponding to host driver is returned
					* Value pui64Size must be RGX_FIRMWARE_RAW_HEAP_SIZE
			 */
			*pui64Addr = gui64SysDevMemFBase;
			*pui64Size = gui64SysDevMemFSize;
		}
		else if (gui32SysDevMemFOrigin)
		{
			/*
				+ Quasi-dynamic VZ setup
				+ Host driver uses polls VM info. page
				+ Guest driver writes DMA PA into page
				+ Host driver init. guest FW state
			*/
			IMG_UINT32 uiVMID;
			IMG_CPU_PHYADDR sPAddr = {gui64SysVMInfoPg};

			if (! gui64SysVMInfoPg)
			{
				PVR_LOG(("Invalid VM info. pg. address 0x0"));
				eError = PVRSRV_ERROR_INVALID_PARAMS;
				goto e0;
			}

			gpuiVMInfoPg = (IMG_UINT64 *) OSMapPhysToLin(sPAddr, OSGetPageSize(), PVRSRV_MEMALLOCFLAG_CPU_UNCACHED);
			PVR_LOG(("VM infopg. @ PA 0x%p mapped to VA 0x%p", (void*)gui64SysVMInfoPg, (void*)gpuiVMInfoPg));

			for (uiVMID = 1; uiVMID < RGXFW_NUM_OS; uiVMID++)
			{
				PVR_LOG(("Zeroing guest %d VM infopg. address @ 0x%p", uiVMID, &gpuiVMInfoPg[uiVMID]));
				gpuiVMInfoPg[uiVMID] = 0;
			}

			gpuiVMInfoPg[0] = 0;

			if (PVRSRV_OK != OSThreadCreatePriority(&ghThrd,
													"l4pvz_poll",
													L4x_VM_InfoPg_Poll,
													NULL,
													IMG_FALSE,
													gpuiVMInfoPg,
													OS_THREAD_HIGHEST_PRIORITY))
			{
				PVR_LOG(("Could not create vm/infopg l4pvz_poll thread"));
				eError = PVRSRV_ERROR_INIT_FAILURE;
			}

			/* Use UMA alloc. for FW in host driver */
			*pui64Addr = 0;
			*pui64Size = 0;
		}
		else
		{
			/*
				+ Dynamic VZ setup
				+ Centralized FW heap configuration in host driver
				+ Guest driver looks-up its FW info. from host driver
				+ Not possible seeing no actual hypercall exits.
			*/
			IMG_PCHAR pzNotice = "Unsupported PVZ setup config, requires hypercall mechanism";
			PVR_LOGG_IF_ERROR(PVRSRV_ERROR_INVALID_PVZ_CONFIG, pzNotice, e0);
		}
	}

	PVR_LOG(("PVZ: FW physheap spec. addr/size: 0x%llx/0x%llx", *pui64Addr, *pui64Size));
e0:
	return eError;
}

static PVRSRV_ERROR
L4xVmmpGetDevPhysHeapOrigin(PVRSRV_DEVICE_CONFIG *psDevConfig,
							PVRSRV_DEVICE_PHYS_HEAP eHeap,
							PVRSRV_DEVICE_PHYS_HEAP_ORIGIN *peOrigin)
{
	PVR_UNREFERENCED_PARAMETER(psDevConfig);
	PVR_UNREFERENCED_PARAMETER(eHeap);
	*peOrigin = gui32SysDevMemFOrigin ?
					PVRSRV_DEVICE_PHYS_HEAP_ORIGIN_GUEST :
					PVRSRV_DEVICE_PHYS_HEAP_ORIGIN_HOST;
	return PVRSRV_OK;
}

static PVRSRV_ERROR
L4xVmmGetDevPhysHeapAddrSize(PVRSRV_DEVICE_CONFIG *psDevConfig,
							 PVRSRV_DEVICE_PHYS_HEAP eHeapType,
							 IMG_UINT64 *pui64Size,
							 IMG_UINT64 *pui64Addr)
{
	PVRSRV_ERROR eError = PVRSRV_OK;
	static IMG_BOOL bAddrSizeSetup = IMG_FALSE;
	PVR_UNREFERENCED_PARAMETER(psDevConfig);

	switch (eHeapType)
	{
		case PVRSRV_DEVICE_PHYS_HEAP_FW_LOCAL:
			if (! bAddrSizeSetup)
			{
				eError = LxSetUpFWPhysHeapAddrSize(psDevConfig,
												   &gui64SysDevMemFBase,
												   &gui64SysDevMemFSize);
				PVR_LOG_IF_ERROR(eError, "LxSetUpFWPhysHeapAddrSize");
				bAddrSizeSetup = IMG_TRUE;
			}

			*pui64Size = gui64SysDevMemFSize;
			*pui64Addr = gui64SysDevMemFBase;
			break;

		case PVRSRV_DEVICE_PHYS_HEAP_GPU_LOCAL:
			*pui64Size = 0;
			*pui64Addr = 0;
			break;

		default:
			*pui64Size = 0;
			*pui64Addr = 0;
			eError = PVRSRV_ERROR_NOT_IMPLEMENTED;
			PVR_ASSERT(0);
			break;
	}

	return eError;
}

static PVRSRV_ERROR
L4xVmmCreateDevConfig(IMG_UINT32 ui32FuncID,
					  IMG_UINT32 ui32DevID,
					  IMG_UINT32 *pui32IRQ,
					  IMG_UINT32 *pui32RegsSize,
					  IMG_UINT64 *pui64RegsCpuPBase)
{
	PVR_UNREFERENCED_PARAMETER(ui32FuncID);
	PVR_UNREFERENCED_PARAMETER(ui32DevID);
	PVR_UNREFERENCED_PARAMETER(pui32IRQ);
	PVR_UNREFERENCED_PARAMETER(pui32RegsSize);
	PVR_UNREFERENCED_PARAMETER(pui64RegsCpuPBase);
	return PVRSRV_ERROR_NOT_IMPLEMENTED;
}

static PVRSRV_ERROR
L4xVmmDestroyDevConfig(IMG_UINT32 ui32FuncID,
					   IMG_UINT32 ui32DevID)
{
	PVR_UNREFERENCED_PARAMETER(ui32FuncID);
	PVR_UNREFERENCED_PARAMETER(ui32DevID);
	return PVRSRV_ERROR_NOT_IMPLEMENTED;
}

static PVRSRV_ERROR
L4xVmmCreateDevPhysHeaps(IMG_UINT32 ui32FuncID,
						 IMG_UINT32 ui32DevID,
						 IMG_UINT32 *peType,
						 IMG_UINT64 *pui64FwSize,
						 IMG_UINT64 *pui64FwAddr,
						 IMG_UINT64 *pui64GpuSize,
						 IMG_UINT64 *pui64GpuAddr)
{
	PVR_UNREFERENCED_PARAMETER(ui32FuncID);
	PVR_UNREFERENCED_PARAMETER(ui32DevID);
	PVR_UNREFERENCED_PARAMETER(peType);
	PVR_UNREFERENCED_PARAMETER(pui64FwSize);
	PVR_UNREFERENCED_PARAMETER(pui64FwAddr);
	PVR_UNREFERENCED_PARAMETER(pui64GpuSize);
	PVR_UNREFERENCED_PARAMETER(pui64GpuAddr);
	return PVRSRV_ERROR_NOT_IMPLEMENTED;
}

static PVRSRV_ERROR
L4xVmmDestroyDevPhysHeaps(IMG_UINT32 ui32FuncID,
						  IMG_UINT32 ui32DevID)
{
	PVR_UNREFERENCED_PARAMETER(ui32FuncID);
	PVR_UNREFERENCED_PARAMETER(ui32DevID);
	return PVRSRV_ERROR_NOT_IMPLEMENTED;
}

static PVRSRV_ERROR
L4xVmmMapDevPhysHeap(IMG_UINT32 ui32FuncID,
					 IMG_UINT32 ui32DevID,
					 IMG_UINT64 ui64Size,
					 IMG_UINT64 ui64Addr)
{
	PVR_UNREFERENCED_PARAMETER(ui32FuncID);
	PVR_UNREFERENCED_PARAMETER(ui32DevID);
	PVR_UNREFERENCED_PARAMETER(ui64Size);
	PVR_UNREFERENCED_PARAMETER(ui64Addr);
	if (gui32SysDevMemFOrigin)
	{
		return PVRSRV_OK;
	}
	else
	{
		return PVRSRV_ERROR_NOT_IMPLEMENTED;
	}
}

static PVRSRV_ERROR
L4xVmmUnmapDevPhysHeap(IMG_UINT32 ui32FuncID,
					   IMG_UINT32 ui32DevID)
{
	PVR_UNREFERENCED_PARAMETER(ui32FuncID);
	PVR_UNREFERENCED_PARAMETER(ui32DevID);
	if (gui32SysDevMemFOrigin)
	{
		return PVRSRV_OK;
	}
	else
	{
		return PVRSRV_ERROR_NOT_IMPLEMENTED;
	}
}

static VMM_PVZ_CONNECTION gsL4xVmmPvz =
{
	.sHostFuncTab = {
		/* pfnCreateDevConfig */
		&L4xVmmCreateDevConfig,

		/* pfnDestroyDevConfig */
		&L4xVmmDestroyDevConfig,

		/* pfnCreateDevPhysHeaps */
		&L4xVmmCreateDevPhysHeaps,

		/* pfnDestroyDevPhysHeaps */
		&L4xVmmDestroyDevPhysHeaps,

		/* pfnMapDevPhysHeap */
		&L4xVmmMapDevPhysHeap,

		/* pfnUnmapDevPhysHeap */
		&L4xVmmUnmapDevPhysHeap
	},

	.sGuestFuncTab = {
		/* pfnCreateDevConfig */
		&PvzServerCreateDevConfig,

		/* pfnDestroyDevConfig */
		&PvzServerDestroyDevConfig,

		/* pfnCreateDevPhysHeaps */
		&PvzServerCreateDevPhysHeaps,

		/* pfnDestroyDevPhysHeaps */
		PvzServerDestroyDevPhysHeaps,

		/* pfnMapDevPhysHeap */
		&PvzServerMapDevPhysHeap,

		/* pfnUnmapDevPhysHeap */
		&PvzServerUnmapDevPhysHeap
	},

	.sConfigFuncTab = {
		/* pfnGetDevPhysHeapOrigin */
		&L4xVmmpGetDevPhysHeapOrigin,

		/* pfnGetDevPhysHeapAddrSize */
		&L4xVmmGetDevPhysHeapAddrSize
	},

	.sVmmFuncTab = {
		/* pfnOnVmOnline */
		&PvzServerOnVmOnline,

		/* pfnOnVmOffline */
		&PvzServerOnVmOffline,

		/* pfnVMMConfigure */
		&PvzServerVMMConfigure
	}
};

static void L4x_VM_InfoPg_Poll(void *pvData)
{
	IMG_UINT64 *puiVMInfoPg = (IMG_UINT64*)pvData;
	IMG_UINT64 uiFBase[RGXFW_NUM_OS];
	IMG_PCHAR szName = "l4pvz_poll";
	IMG_UINT32 uiVMID;
	PVRSRV_ERROR eError;

	PVR_LOG(("%s: VM infopg. polling @ VA 0x%p", szName, puiVMInfoPg));

	for (;;)
	{
		for (uiVMID = 1; uiVMID < RGXFW_NUM_OS; uiVMID++)
		{
			uiFBase[uiVMID] = puiVMInfoPg[uiVMID];

			if (puiVMInfoPg[0] == ~0)
			{
				goto e0;
			}
			else if (uiFBase[uiVMID] == 1)
			{
				/* Do nothing */
			}
			else if (uiFBase[uiVMID] != 1 && uiFBase[uiVMID])
			{
				PVR_LOG(("%s: Guest %d updated VM infopg @ 0x%p: 0x%llx",
						szName, uiVMID, &puiVMInfoPg[uiVMID], uiFBase[uiVMID]));
				break;
			}
			else if (uiFBase[uiVMID] == 0)
			{
				PVR_LOG(("%s: Guest %d updated VM infopg @ 0x%p: 0x%llx",
						szName, uiVMID, &puiVMInfoPg[uiVMID], uiFBase[uiVMID]));
				break;
			}

			if (uiVMID == RGXFW_NUM_OS - 1)
			{
				OSSleepms(500);
			}
		}

		if (uiFBase[uiVMID])
		{
			PVR_LOG(("%s: Marking guest %d driver as online", szName, uiVMID));

			eError = gsL4xVmmPvz.sVmmFuncTab.pfnOnVmOnline(uiVMID, 0);
			PVR_LOG_IF_ERROR(eError, "pfnOnVmOnline");

			PVR_LOG(("%s: Initialising guest %d driver on-chip FW memory context", szName, uiVMID));

			eError = gsL4xVmmPvz.sGuestFuncTab.pfnMapDevPhysHeap(uiVMID,
																 PVZ_BRIDGE_MAPDEVICEPHYSHEAP,
																 0,
															  	 RGX_FIRMWARE_RAW_HEAP_SIZE,
																 uiFBase[uiVMID]);
			PVR_LOG_IF_ERROR(eError, "pfnMapDevPhysHeap");

			if (eError == PVRSRV_OK)
			{
				PVR_LOG(("%s: Mapped guest %d driver FW physheap ADDR:0x%llx/SZ:0x%llx",
						szName, uiVMID, uiFBase[uiVMID], (IMG_UINT64)RGX_FIRMWARE_RAW_HEAP_SIZE));
			}

			/* Unlikely DMA PA value */
			puiVMInfoPg[uiVMID] = 1;
		}
		else
		{

			PVR_LOG(("%s: Deinitialising guest %d driver on-chip FW memory context", szName, uiVMID));

			eError = gsL4xVmmPvz.sGuestFuncTab.pfnUnmapDevPhysHeap(uiVMID,
																   PVZ_BRIDGE_UNMAPDEVICEPHYSHEAP,
																   0);
			PVR_LOG_IF_ERROR(eError, "pfnUnmapDevPhysHeap");

			PVR_LOG(("%s: Marking guest %d driver as offline", szName, uiVMID));

			eError = gsL4xVmmPvz.sVmmFuncTab.pfnOnVmOffline(uiVMID);
			PVR_LOG_IF_ERROR(eError, "pfnOffVmOnline");
		}
	}

e0:
	PVR_LOG(("%s: Exiting...", szName));
}

PVRSRV_ERROR VMMCreatePvzConnection(VMM_PVZ_CONNECTION **psPvzConnection)
{
	PVR_ASSERT(psPvzConnection);
	*psPvzConnection = &gsL4xVmmPvz;
	return PVRSRV_OK;
}

void VMMDestroyPvzConnection(VMM_PVZ_CONNECTION *psPvzConnection)
{
	if (PVRSRV_VZ_MODE_IS(DRIVER_MODE_GUEST))
	{
		if (gsDmaAlloc.pvVirtAddr)
		{
			gpuiVMInfoPg[PVRSRV_VZ_DRIVER_OSID] = 0;
			(void) SysDmaFreeMem(&gsDmaAlloc);
		}
	}
	else
	{
		if (gpuiVMInfoPg)
		{
			gpuiVMInfoPg[0] = ~0;

			OSSleepms(1000);
			OSUnMapPhysToLin(gpuiVMInfoPg, OSGetPageSize(), PVRSRV_MEMALLOCFLAG_CPU_UNCACHED);

			LOOP_UNTIL_TIMEOUT(OS_THREAD_DESTROY_TIMEOUT_US)
			{
				if (PVRSRV_OK == OSThreadDestroy(ghThrd))
				{
					break;
				}
				OSWaitus(OS_THREAD_DESTROY_TIMEOUT_US/OS_THREAD_DESTROY_RETRY_COUNT);
			} END_LOOP_UNTIL_TIMEOUT();
		}
	}

	PVR_ASSERT(psPvzConnection == &gsL4xVmmPvz);
}

/******************************************************************************
 End of file (vmm_type_l4uvmm.c)
******************************************************************************/
