/*************************************************************************/ /*!
@File           pvr_gpufreq.c
@Title          PVR GPU Frequency tracepoint implementation
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
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

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
#include <linux/trace_events.h>
#else
#include <linux/ftrace_event.h>
#endif
#define CREATE_TRACE_POINTS
#include "gpu_frequency.h"
#undef CREATE_TRACE_POINTS

#include "pvr_gpufreq.h"
#include "rgxdevice.h"
#include "pvrsrv.h"

typedef struct _PVR_GPU_FREQ_DATA_ {
	IMG_UINT64       ui64ClockSpeed[PVRSRV_MAX_DEVICES];
	POS_LOCK         hLock;
} PVR_GPU_FREQ_DATA;

static PVR_GPU_FREQ_DATA gGpuFreqPrivData;

static PVR_GPU_FREQ_DATA *getGpuFreqData(const char *const szFunc)
{
	PVR_GPU_FREQ_DATA *psGpuFreqData = &gGpuFreqPrivData;

	if (!psGpuFreqData->hLock)
		return NULL;

	OSLockAcquire(psGpuFreqData->hLock);

	PVR_DPF((PVR_DBG_VERBOSE, "%s: Acquire private data.", szFunc));

	return psGpuFreqData;
}

static void putGpuFreqData(PVR_GPU_FREQ_DATA *psGpuFreqData,
		const char *const szFunc)
{
	if (!psGpuFreqData || !psGpuFreqData->hLock)
		return;

	OSLockRelease(psGpuFreqData->hLock);

	PVR_DPF((PVR_DBG_VERBOSE, "%s: Release private data.", szFunc));
}

PVRSRV_ERROR GpuTraceFreqInitialize(void)
{
	PVR_GPU_FREQ_DATA *psGpuFreqData = &gGpuFreqPrivData;

	return OSLockCreate(&psGpuFreqData->hLock);
}

void GpuTraceFreqDeInitialize(void)
{
	PVR_GPU_FREQ_DATA *psGpuFreqData = &gGpuFreqPrivData;

	OSLockDestroy(psGpuFreqData->hLock);
	psGpuFreqData->hLock = NULL;
}

void GpuTraceFrequency(IMG_UINT32 ui32GpuId, IMG_UINT64 ui64NewClockSpeed)
{
	IMG_UINT64 ui64NewClockSpeedInKHz;
	PVR_GPU_FREQ_DATA *psGpuFreqData;
	IMG_UINT32 ui32Remainder;

	ui64NewClockSpeedInKHz = OSDivide64r64(ui64NewClockSpeed, 1000,
			&ui32Remainder);

	psGpuFreqData = getGpuFreqData(__func__);
	PVR_LOG_RETURN_VOID_IF_FALSE(psGpuFreqData != NULL, "getGpuFreqData");

	if (psGpuFreqData->ui64ClockSpeed[ui32GpuId] != ui64NewClockSpeedInKHz)
	{
		trace_gpu_frequency(ui64NewClockSpeedInKHz, ui32GpuId);

		psGpuFreqData->ui64ClockSpeed[ui32GpuId] = ui64NewClockSpeedInKHz;
	}

	putGpuFreqData(psGpuFreqData, __func__);
}
