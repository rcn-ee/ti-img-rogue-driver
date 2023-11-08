/*************************************************************************/ /*!
@File           sysconfig.c
@Title          System Configuration
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Implements the system layer for TI Keystone3 SoCs
@License        Dual MIT/GPLv2

The contents of this file are subject to the MIT license as set out below.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

Alternatively, the contents of this file may be used under the terms of
the GNU General Public License Version 2 ("GPL") in which case the provisions
of GPL are applicable instead of those above.

If you wish to allow use of your version of this file only under the terms of
GPL, and not to allow others to use your version of this file under the terms
of the MIT license, indicate your decision by deleting the provisions above
and replace them with the notice and other provisions required by GPL as set
out in the file called "GPL-COPYING" included in this distribution. If you do
not delete the provisions above, a recipient may use your version of this file
under the terms of either the MIT license or GPL.

This License is also included in this distribution in the file called
"MIT-COPYING".

EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/ /**************************************************************************/

#include <linux/clk.h>
#include <linux/clk/clk-conf.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>

#include "sysinfo.h"
#include "img_defs.h"
#include "physheap.h"
#include "pvrsrv.h"
#include "rgxdevice.h"
#include "interrupt_support.h"

#define SYS_RGX_ACTIVE_POWER_LATENCY_MS 100

struct pvr_power_data {
	struct device *dev;
	struct clk *core_clk;
};

/* Setup RGX specific timing data */
static RGX_TIMING_INFORMATION gsRGXTimingInfo = {
	.ui32CoreClockSpeed = 500000000,
	.bEnableActivePM = IMG_TRUE,
	.ui32ActivePMLatencyms = SYS_RGX_ACTIVE_POWER_LATENCY_MS,
	.bEnableRDPowIsland = IMG_FALSE,
};

static RGX_DATA gsRGXData = {
	.psRGXTimingInfo = &gsRGXTimingInfo,
};

static PVRSRV_DEVICE_CONFIG gsDevice;

/*
	CPU to Device physical address translation
*/
static void UMAPhysHeapCpuPAddrToDevPAddr(IMG_HANDLE hPrivData,
					  IMG_UINT32 ui32NumOfAddr,
					  IMG_DEV_PHYADDR *psDevPAddr,
					  IMG_CPU_PHYADDR *psCpuPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psDevPAddr[0].uiAddr = psCpuPAddr[0].uiAddr;
	if (ui32NumOfAddr > 1) {
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx) {
			psDevPAddr[ui32Idx].uiAddr = psCpuPAddr[ui32Idx].uiAddr;
		}
	}
}

/*
	Device to CPU physical address translation
*/
static void UMAPhysHeapDevPAddrToCpuPAddr(IMG_HANDLE hPrivData,
					  IMG_UINT32 ui32NumOfAddr,
					  IMG_CPU_PHYADDR *psCpuPAddr,
					  IMG_DEV_PHYADDR *psDevPAddr)
{
	PVR_UNREFERENCED_PARAMETER(hPrivData);

	/* Optimise common case */
	psCpuPAddr[0].uiAddr = psDevPAddr[0].uiAddr;
	if (ui32NumOfAddr > 1) {
		IMG_UINT32 ui32Idx;
		for (ui32Idx = 1; ui32Idx < ui32NumOfAddr; ++ui32Idx) {
			psCpuPAddr[ui32Idx].uiAddr = psDevPAddr[ui32Idx].uiAddr;
		}
	}
}

static PHYS_HEAP_FUNCTIONS gsPhysHeapFuncs = {
	.pfnCpuPAddrToDevPAddr = UMAPhysHeapCpuPAddrToDevPAddr,
	.pfnDevPAddrToCpuPAddr = UMAPhysHeapDevPAddrToCpuPAddr,
};

static PHYS_HEAP_CONFIG gsPhysHeapConfig = {
	.eType = PHYS_HEAP_TYPE_UMA,
	.ui32UsageFlags = PHYS_HEAP_USAGE_GPU_LOCAL,
	.uConfig.sUMA.psMemFuncs = &gsPhysHeapFuncs,
	.uConfig.sUMA.pszPDumpMemspaceName = "SYSMEM",
	.uConfig.sUMA.pszHeapName = "uma_gpu_local",
	.uConfig.sUMA.hPrivData = NULL,
};

static void SysDevPowerDomainsDeinit(struct pvr_power_data *pd_data)
{
	pm_runtime_disable(pd_data->dev);
	dev_pm_domain_detach(pd_data->dev, false);
}

static int SysDevPowerDomainsInit(struct pvr_power_data *pd_data)
{
	int err = 0;

	err = dev_pm_domain_attach(pd_data->dev, false);
	if (err) {
		err = PTR_ERR(pd_data->dev);
		dev_err(pd_data->dev, "failed to get pm-domain: %d\n", err);
	}
	pm_runtime_enable(pd_data->dev);

	return err;
}

static PVRSRV_ERROR
SysDevPrePowerState(IMG_HANDLE hSysData, PVRSRV_SYS_POWER_STATE eNewPowerState,
		    PVRSRV_SYS_POWER_STATE eCurrentPowerState,
		    PVRSRV_POWER_FLAGS ePwrFlags)
{
	struct pvr_power_data *pd_data = hSysData;

	if ((PVRSRV_SYS_POWER_STATE_OFF == eNewPowerState) &&
	    (PVRSRV_SYS_POWER_STATE_ON == eCurrentPowerState)) {
#if defined(DEBUG)
		PVR_LOG(("%s: attempting to suspend", __func__));
#endif
		clk_disable(pd_data->core_clk);
		if (pm_runtime_put_sync_autosuspend(pd_data->dev))
			PVR_LOG(("%s: failed to suspend", __func__));
	}
	return PVRSRV_OK;
}

static PVRSRV_ERROR
SysDevPostPowerState(IMG_HANDLE hSysData, PVRSRV_SYS_POWER_STATE eNewPowerState,
		     PVRSRV_SYS_POWER_STATE eCurrentPowerState,
		     PVRSRV_POWER_FLAGS ePwrFlags)
{
	PVRSRV_ERROR ret;
	struct pvr_power_data *pd_data = hSysData;

	if ((PVRSRV_SYS_POWER_STATE_ON == eNewPowerState) &&
	    (PVRSRV_SYS_POWER_STATE_OFF == eCurrentPowerState)) {
#if defined(DEBUG)
		PVR_LOG(("%s: attempting to resume", __func__));
#endif
		if (clk_enable(pd_data->core_clk)) {
			PVR_LOG(("%s: failed to enable core clock", __func__));
			ret = PVRSRV_ERROR_DEVICE_POWER_CHANGE_FAILURE;
			goto done;
		}
		if (pm_runtime_resume_and_get(pd_data->dev)) {
			PVR_LOG(("%s: failed to resume", __func__));
			ret = PVRSRV_ERROR_DEVICE_POWER_CHANGE_FAILURE;
			goto done;
		}
	}

	ret = PVRSRV_OK;

done:
	return ret;
}

PVRSRV_ERROR SysDevInit(void *pvOSDevice, PVRSRV_DEVICE_CONFIG **ppsDevConfig)
{
	struct platform_device *psDev;
	struct resource *dev_res = NULL;
	struct pvr_power_data *pd_data;
	int dev_irq;

	psDev = to_platform_device((struct device *)pvOSDevice);
	PVR_LOG(("Device: %s", psDev->name));

	pd_data = devm_kzalloc(&psDev->dev, sizeof(struct pvr_power_data),
			       GFP_KERNEL);
	if (!pd_data)
		return -ENOMEM;
	pd_data->dev = &psDev->dev;
	SysDevPowerDomainsInit(pd_data);

	/* REQUIRED DUE TO FIX_HW_BRN_63553 */
	if (dma_set_mask(pvOSDevice, DMA_BIT_MASK(36)))
		PVR_DPF((PVR_DBG_ERROR, "%s: dma_set_mask failed", __func__));

	dev_irq = platform_get_irq(psDev, 0);
	if (dev_irq < 0) {
		PVR_DPF((PVR_DBG_ERROR, "%s: platform_get_irq failed (%d)",
			 __func__, -dev_irq));
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	dev_res = platform_get_resource(psDev, IORESOURCE_MEM, 0);
	if (dev_res == NULL) {
		PVR_DPF((PVR_DBG_ERROR, "%s: platform_get_resource failed",
			 __func__));
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	/* Power on the device for the following helpers to run properly */
	if (pm_runtime_resume_and_get(pd_data->dev))
		PVR_LOG(("%s: failed to resume", __func__));

	/* Prepare core clock and get reference for later power management */
	pd_data->core_clk = devm_clk_get_prepared(pd_data->dev, "core");
	if (IS_ERR(pd_data->core_clk)) {
		PVR_DPF((PVR_DBG_ERROR, "%s: failed to lookup core clock",
			 __func__));
		return PVRSRV_ERROR_INVALID_DEVICE;
	}

	/* Enable clock here instead of using devm_clk_get_enabled because
	 * devm_clk_get_enabled makes the cleanup call an unbalanced disable
	 */
	clk_enable(pd_data->core_clk);

	/* Binding does not currently support loading clock values externally,
	 * override here for now
	 */
	clk_set_rate(pd_data->core_clk,
		     gsRGXData.psRGXTimingInfo->ui32CoreClockSpeed);

	/* Update internal clock speed with measured value */
	gsRGXData.psRGXTimingInfo->ui32CoreClockSpeed =
		(IMG_UINT32)clk_get_rate(pd_data->core_clk);

	/* Power off the device now that clocks are initialized */
	if (pm_runtime_put_sync_autosuspend(pd_data->dev))
		PVR_LOG(("%s: failed to suspend", __func__));
	clk_disable(pd_data->core_clk);

	/* Make sure everything we don't care about is set to 0 */
	memset(&gsDevice, 0, sizeof(gsDevice));

	/* Setup the device config */
	gsDevice.pvOSDevice = pvOSDevice;
	gsDevice.pszName = SYS_RGX_DEV_NAME;
	gsDevice.pszVersion = NULL;

	/* Device setup information */
	gsDevice.sRegsCpuPBase.uiAddr = dev_res->start;
	gsDevice.ui32RegsSize = (unsigned int)(dev_res->end - dev_res->start);
	gsDevice.ui32IRQ = dev_irq;

	/* Device's physical heaps */
	gsDevice.pasPhysHeaps = &gsPhysHeapConfig;
	gsDevice.ui32PhysHeapCount = 1;
	gsDevice.eDefaultHeap = PVRSRV_PHYS_HEAP_GPU_LOCAL;

	/* Setup RGX specific timing data */
	gsDevice.hDevData = &gsRGXData;

	/* device info for power management */
	gsDevice.hSysData = pd_data;

	/* clock frequency */
	gsDevice.pfnClockFreqGet = NULL;

	/* Set gsDevice.pfnSysDevErrorNotify callback */
	gsDevice.pfnSysDevErrorNotify = SysRGXErrorNotify;

	/* power management on HW system */
	gsDevice.pfnPrePowerState = SysDevPrePowerState;
	gsDevice.pfnPostPowerState = SysDevPostPowerState;

	*ppsDevConfig = &gsDevice;

	return PVRSRV_OK;
}

void SysDevDeInit(PVRSRV_DEVICE_CONFIG *psDevConfig)
{
	struct pvr_power_data *pd_data = psDevConfig->hSysData;
	SysDevPowerDomainsDeinit(pd_data);
	devm_clk_put(pd_data->dev, pd_data->core_clk);
	psDevConfig->pvOSDevice = NULL;
}

PVRSRV_ERROR SysInstallDeviceLISR(IMG_HANDLE hSysData, IMG_UINT32 ui32IRQ,
				  const IMG_CHAR *pszName, PFN_LISR pfnLISR,
				  void *pvData, IMG_HANDLE *phLISRData)
{
	PVR_UNREFERENCED_PARAMETER(hSysData);
	return OSInstallSystemLISR(phLISRData, ui32IRQ, pszName, pfnLISR,
				   pvData, SYS_IRQ_FLAG_TRIGGER_DEFAULT);
}

PVRSRV_ERROR SysUninstallDeviceLISR(IMG_HANDLE hLISRData)
{
	return OSUninstallSystemLISR(hLISRData);
}

PVRSRV_ERROR SysDebugInfo(PVRSRV_DEVICE_CONFIG *psDevConfig,
			  DUMPDEBUG_PRINTF_FUNC *pfnDumpDebugPrintf,
			  void *pvDumpDebugFile)
{
	PVR_UNREFERENCED_PARAMETER(psDevConfig);
	PVR_UNREFERENCED_PARAMETER(pfnDumpDebugPrintf);
	PVR_UNREFERENCED_PARAMETER(pvDumpDebugFile);
	return PVRSRV_OK;
}

/******************************************************************************
 End of file (sysconfig.c)
******************************************************************************/
