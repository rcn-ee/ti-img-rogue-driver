/*******************************************************************************
@File
@Title          Server bridge for dc
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Implements the server side of the bridge for dc
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
********************************************************************************/

#include <linux/uaccess.h>

#include "img_defs.h"

#include "dc_server.h"

#include "common_dc_bridge.h"

#include "allocmem.h"
#include "pvr_debug.h"
#include "connection_server.h"
#include "pvr_bridge.h"
#if defined(SUPPORT_RGX)
#include "rgx_bridge.h"
#endif
#include "srvcore.h"
#include "handle.h"

#include <linux/slab.h>

/* ***************************************************************************
 * Server-side bridge entry points
 */

static IMG_INT
PVRSRVBridgeDCDevicesQueryCount(IMG_UINT32 ui32DispatchTableEntry,
				PVRSRV_BRIDGE_IN_DCDEVICESQUERYCOUNT *
				psDCDevicesQueryCountIN,
				PVRSRV_BRIDGE_OUT_DCDEVICESQUERYCOUNT *
				psDCDevicesQueryCountOUT,
				CONNECTION_DATA * psConnection)
{

	PVR_UNREFERENCED_PARAMETER(psConnection);
	PVR_UNREFERENCED_PARAMETER(psDCDevicesQueryCountIN);

	psDCDevicesQueryCountOUT->eError =
	    DCDevicesQueryCount(&psDCDevicesQueryCountOUT->ui32DeviceCount);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDevicesEnumerate(IMG_UINT32 ui32DispatchTableEntry,
			       PVRSRV_BRIDGE_IN_DCDEVICESENUMERATE *
			       psDCDevicesEnumerateIN,
			       PVRSRV_BRIDGE_OUT_DCDEVICESENUMERATE *
			       psDCDevicesEnumerateOUT,
			       CONNECTION_DATA * psConnection)
{
	IMG_UINT32 *pui32DeviceIndexInt = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCDevicesEnumerateIN->ui32DeviceArraySize * sizeof(IMG_UINT32)) +
	    0;

	psDCDevicesEnumerateOUT->pui32DeviceIndex =
	    psDCDevicesEnumerateIN->pui32DeviceIndex;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCDevicesEnumerateIN),
			      sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer =
			    (IMG_BYTE *) psDCDevicesEnumerateIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCDevicesEnumerateOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCDevicesEnumerate_exit;
			}
		}
	}

	if (psDCDevicesEnumerateIN->ui32DeviceArraySize != 0)
	{
		pui32DeviceIndexInt =
		    (IMG_UINT32 *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDevicesEnumerateIN->ui32DeviceArraySize *
		    sizeof(IMG_UINT32);
	}

	psDCDevicesEnumerateOUT->eError =
	    DCDevicesEnumerate(psConnection, OSGetDevData(psConnection),
			       psDCDevicesEnumerateIN->ui32DeviceArraySize,
			       &psDCDevicesEnumerateOUT->ui32DeviceCount,
			       pui32DeviceIndexInt);

	if ((psDCDevicesEnumerateOUT->ui32DeviceCount * sizeof(IMG_UINT32)) > 0)
	{
		if (OSCopyToUser
		    (NULL,
		     (void __user *)psDCDevicesEnumerateOUT->pui32DeviceIndex,
		     pui32DeviceIndexInt,
		     (psDCDevicesEnumerateOUT->ui32DeviceCount *
		      sizeof(IMG_UINT32))) != PVRSRV_OK)
		{
			psDCDevicesEnumerateOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDevicesEnumerate_exit;
		}
	}

 DCDevicesEnumerate_exit:

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDeviceAcquire(IMG_UINT32 ui32DispatchTableEntry,
			    PVRSRV_BRIDGE_IN_DCDEVICEACQUIRE *
			    psDCDeviceAcquireIN,
			    PVRSRV_BRIDGE_OUT_DCDEVICEACQUIRE *
			    psDCDeviceAcquireOUT,
			    CONNECTION_DATA * psConnection)
{
	DC_DEVICE *psDeviceInt = NULL;

	psDCDeviceAcquireOUT->eError =
	    DCDeviceAcquire(psConnection, OSGetDevData(psConnection),
			    psDCDeviceAcquireIN->ui32DeviceIndex, &psDeviceInt);
	/* Exit early if bridged call fails */
	if (psDCDeviceAcquireOUT->eError != PVRSRV_OK)
	{
		goto DCDeviceAcquire_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCDeviceAcquireOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDCDeviceAcquireOUT->hDevice,
				      (void *)psDeviceInt,
				      PVRSRV_HANDLE_TYPE_DC_DEVICE,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) & DCDeviceRelease);
	if (psDCDeviceAcquireOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCDeviceAcquire_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCDeviceAcquire_exit:

	if (psDCDeviceAcquireOUT->eError != PVRSRV_OK)
	{
		if (psDeviceInt)
		{
			DCDeviceRelease(psDeviceInt);
		}
	}

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDeviceRelease(IMG_UINT32 ui32DispatchTableEntry,
			    PVRSRV_BRIDGE_IN_DCDEVICERELEASE *
			    psDCDeviceReleaseIN,
			    PVRSRV_BRIDGE_OUT_DCDEVICERELEASE *
			    psDCDeviceReleaseOUT,
			    CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCDeviceReleaseOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					(IMG_HANDLE) psDCDeviceReleaseIN->
					hDevice, PVRSRV_HANDLE_TYPE_DC_DEVICE);
	if ((psDCDeviceReleaseOUT->eError != PVRSRV_OK)
	    && (psDCDeviceReleaseOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCDeviceRelease: %s",
			 PVRSRVGetErrorStringKM(psDCDeviceReleaseOUT->eError)));
		UnlockHandle();
		goto DCDeviceRelease_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCDeviceRelease_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDCGetInfo(IMG_UINT32 ui32DispatchTableEntry,
		      PVRSRV_BRIDGE_IN_DCGETINFO * psDCGetInfoIN,
		      PVRSRV_BRIDGE_OUT_DCGETINFO * psDCGetInfoOUT,
		      CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCGetInfoIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCGetInfoOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCGetInfoOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCGetInfo_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCGetInfoOUT->eError =
	    DCGetInfo(psDeviceInt, &psDCGetInfoOUT->sDisplayInfo);

 DCGetInfo_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	return 0;
}

static IMG_INT
PVRSRVBridgeDCPanelQueryCount(IMG_UINT32 ui32DispatchTableEntry,
			      PVRSRV_BRIDGE_IN_DCPANELQUERYCOUNT *
			      psDCPanelQueryCountIN,
			      PVRSRV_BRIDGE_OUT_DCPANELQUERYCOUNT *
			      psDCPanelQueryCountOUT,
			      CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCPanelQueryCountIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCPanelQueryCountOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCPanelQueryCountOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCPanelQueryCount_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCPanelQueryCountOUT->eError =
	    DCPanelQueryCount(psDeviceInt,
			      &psDCPanelQueryCountOUT->ui32NumPanels);

 DCPanelQueryCount_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	return 0;
}

static IMG_INT
PVRSRVBridgeDCPanelQuery(IMG_UINT32 ui32DispatchTableEntry,
			 PVRSRV_BRIDGE_IN_DCPANELQUERY * psDCPanelQueryIN,
			 PVRSRV_BRIDGE_OUT_DCPANELQUERY * psDCPanelQueryOUT,
			 CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCPanelQueryIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;
	PVRSRV_PANEL_INFO *psPanelInfoInt = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCPanelQueryIN->ui32PanelsArraySize *
	     sizeof(PVRSRV_PANEL_INFO)) + 0;

	psDCPanelQueryOUT->psPanelInfo = psDCPanelQueryIN->psPanelInfo;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCPanelQueryIN), sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer = (IMG_BYTE *) psDCPanelQueryIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCPanelQueryOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCPanelQuery_exit;
			}
		}
	}

	if (psDCPanelQueryIN->ui32PanelsArraySize != 0)
	{
		psPanelInfoInt =
		    (PVRSRV_PANEL_INFO *) (((IMG_UINT8 *) pArrayArgsBuffer) +
					   ui32NextOffset);
		ui32NextOffset +=
		    psDCPanelQueryIN->ui32PanelsArraySize *
		    sizeof(PVRSRV_PANEL_INFO);
	}

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCPanelQueryOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCPanelQueryOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCPanelQuery_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCPanelQueryOUT->eError =
	    DCPanelQuery(psDeviceInt,
			 psDCPanelQueryIN->ui32PanelsArraySize,
			 &psDCPanelQueryOUT->ui32NumPanels, psPanelInfoInt);

	if ((psDCPanelQueryOUT->ui32NumPanels * sizeof(PVRSRV_PANEL_INFO)) > 0)
	{
		if (OSCopyToUser
		    (NULL, (void __user *)psDCPanelQueryOUT->psPanelInfo,
		     psPanelInfoInt,
		     (psDCPanelQueryOUT->ui32NumPanels *
		      sizeof(PVRSRV_PANEL_INFO))) != PVRSRV_OK)
		{
			psDCPanelQueryOUT->eError = PVRSRV_ERROR_INVALID_PARAMS;

			goto DCPanelQuery_exit;
		}
	}

 DCPanelQuery_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCFormatQuery(IMG_UINT32 ui32DispatchTableEntry,
			  PVRSRV_BRIDGE_IN_DCFORMATQUERY * psDCFormatQueryIN,
			  PVRSRV_BRIDGE_OUT_DCFORMATQUERY * psDCFormatQueryOUT,
			  CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCFormatQueryIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;
	PVRSRV_SURFACE_FORMAT *psFormatInt = NULL;
	IMG_UINT32 *pui32SupportedInt = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCFormatQueryIN->ui32NumFormats *
	     sizeof(PVRSRV_SURFACE_FORMAT)) +
	    (psDCFormatQueryIN->ui32NumFormats * sizeof(IMG_UINT32)) + 0;

	psDCFormatQueryOUT->pui32Supported = psDCFormatQueryIN->pui32Supported;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCFormatQueryIN),
			      sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer = (IMG_BYTE *) psDCFormatQueryIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCFormatQueryOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCFormatQuery_exit;
			}
		}
	}

	if (psDCFormatQueryIN->ui32NumFormats != 0)
	{
		psFormatInt =
		    (PVRSRV_SURFACE_FORMAT *) (((IMG_UINT8 *) pArrayArgsBuffer)
					       + ui32NextOffset);
		ui32NextOffset +=
		    psDCFormatQueryIN->ui32NumFormats *
		    sizeof(PVRSRV_SURFACE_FORMAT);
	}

	/* Copy the data over */
	if (psDCFormatQueryIN->ui32NumFormats * sizeof(PVRSRV_SURFACE_FORMAT) >
	    0)
	{
		if (OSCopyFromUser
		    (NULL, psFormatInt,
		     (const void __user *)psDCFormatQueryIN->psFormat,
		     psDCFormatQueryIN->ui32NumFormats *
		     sizeof(PVRSRV_SURFACE_FORMAT)) != PVRSRV_OK)
		{
			psDCFormatQueryOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCFormatQuery_exit;
		}
	}
	if (psDCFormatQueryIN->ui32NumFormats != 0)
	{
		pui32SupportedInt =
		    (IMG_UINT32 *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCFormatQueryIN->ui32NumFormats * sizeof(IMG_UINT32);
	}

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCFormatQueryOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCFormatQueryOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCFormatQuery_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCFormatQueryOUT->eError =
	    DCFormatQuery(psDeviceInt,
			  psDCFormatQueryIN->ui32NumFormats,
			  psFormatInt, pui32SupportedInt);

	if ((psDCFormatQueryIN->ui32NumFormats * sizeof(IMG_UINT32)) > 0)
	{
		if (OSCopyToUser
		    (NULL, (void __user *)psDCFormatQueryOUT->pui32Supported,
		     pui32SupportedInt,
		     (psDCFormatQueryIN->ui32NumFormats *
		      sizeof(IMG_UINT32))) != PVRSRV_OK)
		{
			psDCFormatQueryOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCFormatQuery_exit;
		}
	}

 DCFormatQuery_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDimQuery(IMG_UINT32 ui32DispatchTableEntry,
		       PVRSRV_BRIDGE_IN_DCDIMQUERY * psDCDimQueryIN,
		       PVRSRV_BRIDGE_OUT_DCDIMQUERY * psDCDimQueryOUT,
		       CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCDimQueryIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;
	PVRSRV_SURFACE_DIMS *psDimInt = NULL;
	IMG_UINT32 *pui32SupportedInt = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCDimQueryIN->ui32NumDims * sizeof(PVRSRV_SURFACE_DIMS)) +
	    (psDCDimQueryIN->ui32NumDims * sizeof(IMG_UINT32)) + 0;

	psDCDimQueryOUT->pui32Supported = psDCDimQueryIN->pui32Supported;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCDimQueryIN), sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer = (IMG_BYTE *) psDCDimQueryIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCDimQueryOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCDimQuery_exit;
			}
		}
	}

	if (psDCDimQueryIN->ui32NumDims != 0)
	{
		psDimInt =
		    (PVRSRV_SURFACE_DIMS *) (((IMG_UINT8 *) pArrayArgsBuffer) +
					     ui32NextOffset);
		ui32NextOffset +=
		    psDCDimQueryIN->ui32NumDims * sizeof(PVRSRV_SURFACE_DIMS);
	}

	/* Copy the data over */
	if (psDCDimQueryIN->ui32NumDims * sizeof(PVRSRV_SURFACE_DIMS) > 0)
	{
		if (OSCopyFromUser
		    (NULL, psDimInt, (const void __user *)psDCDimQueryIN->psDim,
		     psDCDimQueryIN->ui32NumDims *
		     sizeof(PVRSRV_SURFACE_DIMS)) != PVRSRV_OK)
		{
			psDCDimQueryOUT->eError = PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDimQuery_exit;
		}
	}
	if (psDCDimQueryIN->ui32NumDims != 0)
	{
		pui32SupportedInt =
		    (IMG_UINT32 *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDimQueryIN->ui32NumDims * sizeof(IMG_UINT32);
	}

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCDimQueryOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCDimQueryOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCDimQuery_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCDimQueryOUT->eError =
	    DCDimQuery(psDeviceInt,
		       psDCDimQueryIN->ui32NumDims,
		       psDimInt, pui32SupportedInt);

	if ((psDCDimQueryIN->ui32NumDims * sizeof(IMG_UINT32)) > 0)
	{
		if (OSCopyToUser
		    (NULL, (void __user *)psDCDimQueryOUT->pui32Supported,
		     pui32SupportedInt,
		     (psDCDimQueryIN->ui32NumDims * sizeof(IMG_UINT32))) !=
		    PVRSRV_OK)
		{
			psDCDimQueryOUT->eError = PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDimQuery_exit;
		}
	}

 DCDimQuery_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCSetBlank(IMG_UINT32 ui32DispatchTableEntry,
		       PVRSRV_BRIDGE_IN_DCSETBLANK * psDCSetBlankIN,
		       PVRSRV_BRIDGE_OUT_DCSETBLANK * psDCSetBlankOUT,
		       CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCSetBlankIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCSetBlankOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCSetBlankOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCSetBlank_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCSetBlankOUT->eError =
	    DCSetBlank(psDeviceInt, psDCSetBlankIN->bEnabled);

 DCSetBlank_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	return 0;
}

static IMG_INT
PVRSRVBridgeDCSetVSyncReporting(IMG_UINT32 ui32DispatchTableEntry,
				PVRSRV_BRIDGE_IN_DCSETVSYNCREPORTING *
				psDCSetVSyncReportingIN,
				PVRSRV_BRIDGE_OUT_DCSETVSYNCREPORTING *
				psDCSetVSyncReportingOUT,
				CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCSetVSyncReportingIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCSetVSyncReportingOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCSetVSyncReportingOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCSetVSyncReporting_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCSetVSyncReportingOUT->eError =
	    DCSetVSyncReporting(psDeviceInt, psDCSetVSyncReportingIN->bEnabled);

 DCSetVSyncReporting_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	return 0;
}

static IMG_INT
PVRSRVBridgeDCLastVSyncQuery(IMG_UINT32 ui32DispatchTableEntry,
			     PVRSRV_BRIDGE_IN_DCLASTVSYNCQUERY *
			     psDCLastVSyncQueryIN,
			     PVRSRV_BRIDGE_OUT_DCLASTVSYNCQUERY *
			     psDCLastVSyncQueryOUT,
			     CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCLastVSyncQueryIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCLastVSyncQueryOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCLastVSyncQueryOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCLastVSyncQuery_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCLastVSyncQueryOUT->eError =
	    DCLastVSyncQuery(psDeviceInt, &psDCLastVSyncQueryOUT->i64Timestamp);

 DCLastVSyncQuery_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	return 0;
}

static IMG_INT
PVRSRVBridgeDCSystemBufferAcquire(IMG_UINT32 ui32DispatchTableEntry,
				  PVRSRV_BRIDGE_IN_DCSYSTEMBUFFERACQUIRE *
				  psDCSystemBufferAcquireIN,
				  PVRSRV_BRIDGE_OUT_DCSYSTEMBUFFERACQUIRE *
				  psDCSystemBufferAcquireOUT,
				  CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCSystemBufferAcquireIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;
	DC_BUFFER *psBufferInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCSystemBufferAcquireOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCSystemBufferAcquireOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCSystemBufferAcquire_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCSystemBufferAcquireOUT->eError =
	    DCSystemBufferAcquire(psDeviceInt,
				  &psDCSystemBufferAcquireOUT->ui32Stride,
				  &psBufferInt);
	/* Exit early if bridged call fails */
	if (psDCSystemBufferAcquireOUT->eError != PVRSRV_OK)
	{
		goto DCSystemBufferAcquire_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCSystemBufferAcquireOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDCSystemBufferAcquireOUT->hBuffer,
				      (void *)psBufferInt,
				      PVRSRV_HANDLE_TYPE_DC_BUFFER,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) &
				      DCSystemBufferRelease);
	if (psDCSystemBufferAcquireOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCSystemBufferAcquire_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCSystemBufferAcquire_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	if (psDCSystemBufferAcquireOUT->eError != PVRSRV_OK)
	{
		if (psBufferInt)
		{
			DCSystemBufferRelease(psBufferInt);
		}
	}

	return 0;
}

static IMG_INT
PVRSRVBridgeDCSystemBufferRelease(IMG_UINT32 ui32DispatchTableEntry,
				  PVRSRV_BRIDGE_IN_DCSYSTEMBUFFERRELEASE *
				  psDCSystemBufferReleaseIN,
				  PVRSRV_BRIDGE_OUT_DCSYSTEMBUFFERRELEASE *
				  psDCSystemBufferReleaseOUT,
				  CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCSystemBufferReleaseOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					(IMG_HANDLE) psDCSystemBufferReleaseIN->
					hBuffer, PVRSRV_HANDLE_TYPE_DC_BUFFER);
	if ((psDCSystemBufferReleaseOUT->eError != PVRSRV_OK)
	    && (psDCSystemBufferReleaseOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCSystemBufferRelease: %s",
			 PVRSRVGetErrorStringKM(psDCSystemBufferReleaseOUT->
						eError)));
		UnlockHandle();
		goto DCSystemBufferRelease_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCSystemBufferRelease_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDisplayContextCreate(IMG_UINT32 ui32DispatchTableEntry,
				   PVRSRV_BRIDGE_IN_DCDISPLAYCONTEXTCREATE *
				   psDCDisplayContextCreateIN,
				   PVRSRV_BRIDGE_OUT_DCDISPLAYCONTEXTCREATE *
				   psDCDisplayContextCreateOUT,
				   CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDevice = psDCDisplayContextCreateIN->hDevice;
	DC_DEVICE *psDeviceInt = NULL;
	DC_DISPLAY_CONTEXT *psDisplayContextInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCDisplayContextCreateOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDeviceInt,
				       hDevice,
				       PVRSRV_HANDLE_TYPE_DC_DEVICE, IMG_TRUE);
	if (psDCDisplayContextCreateOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCDisplayContextCreate_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCDisplayContextCreateOUT->eError =
	    DCDisplayContextCreate(psDeviceInt, &psDisplayContextInt);
	/* Exit early if bridged call fails */
	if (psDCDisplayContextCreateOUT->eError != PVRSRV_OK)
	{
		goto DCDisplayContextCreate_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCDisplayContextCreateOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDCDisplayContextCreateOUT->
				      hDisplayContext,
				      (void *)psDisplayContextInt,
				      PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) &
				      DCDisplayContextDestroy);
	if (psDCDisplayContextCreateOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCDisplayContextCreate_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCDisplayContextCreate_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDeviceInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDevice,
					    PVRSRV_HANDLE_TYPE_DC_DEVICE);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	if (psDCDisplayContextCreateOUT->eError != PVRSRV_OK)
	{
		if (psDisplayContextInt)
		{
			DCDisplayContextDestroy(psDisplayContextInt);
		}
	}

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDisplayContextConfigureCheck(IMG_UINT32 ui32DispatchTableEntry,
					   PVRSRV_BRIDGE_IN_DCDISPLAYCONTEXTCONFIGURECHECK
					   * psDCDisplayContextConfigureCheckIN,
					   PVRSRV_BRIDGE_OUT_DCDISPLAYCONTEXTCONFIGURECHECK
					   *
					   psDCDisplayContextConfigureCheckOUT,
					   CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDisplayContext =
	    psDCDisplayContextConfigureCheckIN->hDisplayContext;
	DC_DISPLAY_CONTEXT *psDisplayContextInt = NULL;
	PVRSRV_SURFACE_CONFIG_INFO *psSurfInfoInt = NULL;
	DC_BUFFER **psBuffersInt = NULL;
	IMG_HANDLE *hBuffersInt2 = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCDisplayContextConfigureCheckIN->ui32PipeCount *
	     sizeof(PVRSRV_SURFACE_CONFIG_INFO)) +
	    (psDCDisplayContextConfigureCheckIN->ui32PipeCount *
	     sizeof(DC_BUFFER *)) +
	    (psDCDisplayContextConfigureCheckIN->ui32PipeCount *
	     sizeof(IMG_HANDLE)) + 0;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCDisplayContextConfigureCheckIN),
			      sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer =
			    (IMG_BYTE *) psDCDisplayContextConfigureCheckIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCDisplayContextConfigureCheckOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCDisplayContextConfigureCheck_exit;
			}
		}
	}

	if (psDCDisplayContextConfigureCheckIN->ui32PipeCount != 0)
	{
		psSurfInfoInt =
		    (PVRSRV_SURFACE_CONFIG_INFO
		     *) (((IMG_UINT8 *) pArrayArgsBuffer) + ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureCheckIN->ui32PipeCount *
		    sizeof(PVRSRV_SURFACE_CONFIG_INFO);
	}

	/* Copy the data over */
	if (psDCDisplayContextConfigureCheckIN->ui32PipeCount *
	    sizeof(PVRSRV_SURFACE_CONFIG_INFO) > 0)
	{
		if (OSCopyFromUser
		    (NULL, psSurfInfoInt,
		     (const void __user *)psDCDisplayContextConfigureCheckIN->
		     psSurfInfo,
		     psDCDisplayContextConfigureCheckIN->ui32PipeCount *
		     sizeof(PVRSRV_SURFACE_CONFIG_INFO)) != PVRSRV_OK)
		{
			psDCDisplayContextConfigureCheckOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDisplayContextConfigureCheck_exit;
		}
	}
	if (psDCDisplayContextConfigureCheckIN->ui32PipeCount != 0)
	{
		psBuffersInt =
		    (DC_BUFFER **) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureCheckIN->ui32PipeCount *
		    sizeof(DC_BUFFER *);
		hBuffersInt2 =
		    (IMG_HANDLE *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureCheckIN->ui32PipeCount *
		    sizeof(IMG_HANDLE);
	}

	/* Copy the data over */
	if (psDCDisplayContextConfigureCheckIN->ui32PipeCount *
	    sizeof(IMG_HANDLE) > 0)
	{
		if (OSCopyFromUser
		    (NULL, hBuffersInt2,
		     (const void __user *)psDCDisplayContextConfigureCheckIN->
		     phBuffers,
		     psDCDisplayContextConfigureCheckIN->ui32PipeCount *
		     sizeof(IMG_HANDLE)) != PVRSRV_OK)
		{
			psDCDisplayContextConfigureCheckOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDisplayContextConfigureCheck_exit;
		}
	}

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCDisplayContextConfigureCheckOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDisplayContextInt,
				       hDisplayContext,
				       PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT,
				       IMG_TRUE);
	if (psDCDisplayContextConfigureCheckOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCDisplayContextConfigureCheck_exit;
	}

	{
		IMG_UINT32 i;

		for (i = 0;
		     i < psDCDisplayContextConfigureCheckIN->ui32PipeCount; i++)
		{
			/* Look up the address from the handle */
			psDCDisplayContextConfigureCheckOUT->eError =
			    PVRSRVLookupHandleUnlocked(psConnection->
						       psHandleBase,
						       (void **)
						       &psBuffersInt[i],
						       hBuffersInt2[i],
						       PVRSRV_HANDLE_TYPE_DC_BUFFER,
						       IMG_TRUE);
			if (psDCDisplayContextConfigureCheckOUT->eError !=
			    PVRSRV_OK)
			{
				UnlockHandle();
				goto DCDisplayContextConfigureCheck_exit;
			}
		}
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCDisplayContextConfigureCheckOUT->eError =
	    DCDisplayContextConfigureCheck(psDisplayContextInt,
					   psDCDisplayContextConfigureCheckIN->
					   ui32PipeCount, psSurfInfoInt,
					   psBuffersInt);

 DCDisplayContextConfigureCheck_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDisplayContextInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDisplayContext,
					    PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT);
	}

	if (hBuffersInt2)
	{
		IMG_UINT32 i;

		for (i = 0;
		     i < psDCDisplayContextConfigureCheckIN->ui32PipeCount; i++)
		{

			/* Unreference the previously looked up handle */
			if (hBuffersInt2[i])
			{
				PVRSRVReleaseHandleUnlocked(psConnection->
							    psHandleBase,
							    hBuffersInt2[i],
							    PVRSRV_HANDLE_TYPE_DC_BUFFER);
			}
		}
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDisplayContextConfigure(IMG_UINT32 ui32DispatchTableEntry,
				      PVRSRV_BRIDGE_IN_DCDISPLAYCONTEXTCONFIGURE
				      * psDCDisplayContextConfigureIN,
				      PVRSRV_BRIDGE_OUT_DCDISPLAYCONTEXTCONFIGURE
				      * psDCDisplayContextConfigureOUT,
				      CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDisplayContext =
	    psDCDisplayContextConfigureIN->hDisplayContext;
	DC_DISPLAY_CONTEXT *psDisplayContextInt = NULL;
	PVRSRV_SURFACE_CONFIG_INFO *psSurfInfoInt = NULL;
	DC_BUFFER **psBuffersInt = NULL;
	IMG_HANDLE *hBuffersInt2 = NULL;
	SERVER_SYNC_PRIMITIVE **psSyncInt = NULL;
	IMG_HANDLE *hSyncInt2 = NULL;
	IMG_BOOL *bUpdateInt = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCDisplayContextConfigureIN->ui32PipeCount *
	     sizeof(PVRSRV_SURFACE_CONFIG_INFO)) +
	    (psDCDisplayContextConfigureIN->ui32PipeCount *
	     sizeof(DC_BUFFER *)) +
	    (psDCDisplayContextConfigureIN->ui32PipeCount *
	     sizeof(IMG_HANDLE)) +
	    (psDCDisplayContextConfigureIN->ui32SyncCount *
	     sizeof(SERVER_SYNC_PRIMITIVE *)) +
	    (psDCDisplayContextConfigureIN->ui32SyncCount *
	     sizeof(IMG_HANDLE)) +
	    (psDCDisplayContextConfigureIN->ui32SyncCount * sizeof(IMG_BOOL)) +
	    0;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCDisplayContextConfigureIN),
			      sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer =
			    (IMG_BYTE *) psDCDisplayContextConfigureIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCDisplayContextConfigureOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCDisplayContextConfigure_exit;
			}
		}
	}

	if (psDCDisplayContextConfigureIN->ui32PipeCount != 0)
	{
		psSurfInfoInt =
		    (PVRSRV_SURFACE_CONFIG_INFO
		     *) (((IMG_UINT8 *) pArrayArgsBuffer) + ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureIN->ui32PipeCount *
		    sizeof(PVRSRV_SURFACE_CONFIG_INFO);
	}

	/* Copy the data over */
	if (psDCDisplayContextConfigureIN->ui32PipeCount *
	    sizeof(PVRSRV_SURFACE_CONFIG_INFO) > 0)
	{
		if (OSCopyFromUser
		    (NULL, psSurfInfoInt,
		     (const void __user *)psDCDisplayContextConfigureIN->
		     psSurfInfo,
		     psDCDisplayContextConfigureIN->ui32PipeCount *
		     sizeof(PVRSRV_SURFACE_CONFIG_INFO)) != PVRSRV_OK)
		{
			psDCDisplayContextConfigureOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDisplayContextConfigure_exit;
		}
	}
	if (psDCDisplayContextConfigureIN->ui32PipeCount != 0)
	{
		psBuffersInt =
		    (DC_BUFFER **) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureIN->ui32PipeCount *
		    sizeof(DC_BUFFER *);
		hBuffersInt2 =
		    (IMG_HANDLE *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureIN->ui32PipeCount *
		    sizeof(IMG_HANDLE);
	}

	/* Copy the data over */
	if (psDCDisplayContextConfigureIN->ui32PipeCount * sizeof(IMG_HANDLE) >
	    0)
	{
		if (OSCopyFromUser
		    (NULL, hBuffersInt2,
		     (const void __user *)psDCDisplayContextConfigureIN->
		     phBuffers,
		     psDCDisplayContextConfigureIN->ui32PipeCount *
		     sizeof(IMG_HANDLE)) != PVRSRV_OK)
		{
			psDCDisplayContextConfigureOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDisplayContextConfigure_exit;
		}
	}
	if (psDCDisplayContextConfigureIN->ui32SyncCount != 0)
	{
		psSyncInt =
		    (SERVER_SYNC_PRIMITIVE **) (((IMG_UINT8 *) pArrayArgsBuffer)
						+ ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureIN->ui32SyncCount *
		    sizeof(SERVER_SYNC_PRIMITIVE *);
		hSyncInt2 =
		    (IMG_HANDLE *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureIN->ui32SyncCount *
		    sizeof(IMG_HANDLE);
	}

	/* Copy the data over */
	if (psDCDisplayContextConfigureIN->ui32SyncCount * sizeof(IMG_HANDLE) >
	    0)
	{
		if (OSCopyFromUser
		    (NULL, hSyncInt2,
		     (const void __user *)psDCDisplayContextConfigureIN->phSync,
		     psDCDisplayContextConfigureIN->ui32SyncCount *
		     sizeof(IMG_HANDLE)) != PVRSRV_OK)
		{
			psDCDisplayContextConfigureOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDisplayContextConfigure_exit;
		}
	}
	if (psDCDisplayContextConfigureIN->ui32SyncCount != 0)
	{
		bUpdateInt =
		    (IMG_BOOL *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				  ui32NextOffset);
		ui32NextOffset +=
		    psDCDisplayContextConfigureIN->ui32SyncCount *
		    sizeof(IMG_BOOL);
	}

	/* Copy the data over */
	if (psDCDisplayContextConfigureIN->ui32SyncCount * sizeof(IMG_BOOL) > 0)
	{
		if (OSCopyFromUser
		    (NULL, bUpdateInt,
		     (const void __user *)psDCDisplayContextConfigureIN->
		     pbUpdate,
		     psDCDisplayContextConfigureIN->ui32SyncCount *
		     sizeof(IMG_BOOL)) != PVRSRV_OK)
		{
			psDCDisplayContextConfigureOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCDisplayContextConfigure_exit;
		}
	}

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCDisplayContextConfigureOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDisplayContextInt,
				       hDisplayContext,
				       PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT,
				       IMG_TRUE);
	if (psDCDisplayContextConfigureOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCDisplayContextConfigure_exit;
	}

	{
		IMG_UINT32 i;

		for (i = 0; i < psDCDisplayContextConfigureIN->ui32PipeCount;
		     i++)
		{
			/* Look up the address from the handle */
			psDCDisplayContextConfigureOUT->eError =
			    PVRSRVLookupHandleUnlocked(psConnection->
						       psHandleBase,
						       (void **)
						       &psBuffersInt[i],
						       hBuffersInt2[i],
						       PVRSRV_HANDLE_TYPE_DC_BUFFER,
						       IMG_TRUE);
			if (psDCDisplayContextConfigureOUT->eError != PVRSRV_OK)
			{
				UnlockHandle();
				goto DCDisplayContextConfigure_exit;
			}
		}
	}

	{
		IMG_UINT32 i;

		for (i = 0; i < psDCDisplayContextConfigureIN->ui32SyncCount;
		     i++)
		{
			/* Look up the address from the handle */
			psDCDisplayContextConfigureOUT->eError =
			    PVRSRVLookupHandleUnlocked(psConnection->
						       psHandleBase,
						       (void **)&psSyncInt[i],
						       hSyncInt2[i],
						       PVRSRV_HANDLE_TYPE_SERVER_SYNC_PRIMITIVE,
						       IMG_TRUE);
			if (psDCDisplayContextConfigureOUT->eError != PVRSRV_OK)
			{
				UnlockHandle();
				goto DCDisplayContextConfigure_exit;
			}
		}
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCDisplayContextConfigureOUT->eError =
	    DCDisplayContextConfigure(psDisplayContextInt,
				      psDCDisplayContextConfigureIN->
				      ui32ClientCacheOpSeqNum,
				      psDCDisplayContextConfigureIN->
				      ui32PipeCount, psSurfInfoInt,
				      psBuffersInt,
				      psDCDisplayContextConfigureIN->
				      ui32SyncCount, psSyncInt, bUpdateInt,
				      psDCDisplayContextConfigureIN->
				      ui32DisplayPeriod,
				      psDCDisplayContextConfigureIN->
				      ui32MaxDepth,
				      psDCDisplayContextConfigureIN->
				      hAcquireFence,
				      psDCDisplayContextConfigureIN->
				      hReleaseFenceTimeline,
				      &psDCDisplayContextConfigureOUT->
				      hReleaseFence);

 DCDisplayContextConfigure_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDisplayContextInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDisplayContext,
					    PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT);
	}

	if (hBuffersInt2)
	{
		IMG_UINT32 i;

		for (i = 0; i < psDCDisplayContextConfigureIN->ui32PipeCount;
		     i++)
		{

			/* Unreference the previously looked up handle */
			if (hBuffersInt2[i])
			{
				PVRSRVReleaseHandleUnlocked(psConnection->
							    psHandleBase,
							    hBuffersInt2[i],
							    PVRSRV_HANDLE_TYPE_DC_BUFFER);
			}
		}
	}

	if (hSyncInt2)
	{
		IMG_UINT32 i;

		for (i = 0; i < psDCDisplayContextConfigureIN->ui32SyncCount;
		     i++)
		{

			/* Unreference the previously looked up handle */
			if (hSyncInt2[i])
			{
				PVRSRVReleaseHandleUnlocked(psConnection->
							    psHandleBase,
							    hSyncInt2[i],
							    PVRSRV_HANDLE_TYPE_SERVER_SYNC_PRIMITIVE);
			}
		}
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCDisplayContextDestroy(IMG_UINT32 ui32DispatchTableEntry,
				    PVRSRV_BRIDGE_IN_DCDISPLAYCONTEXTDESTROY *
				    psDCDisplayContextDestroyIN,
				    PVRSRV_BRIDGE_OUT_DCDISPLAYCONTEXTDESTROY *
				    psDCDisplayContextDestroyOUT,
				    CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCDisplayContextDestroyOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					(IMG_HANDLE)
					psDCDisplayContextDestroyIN->
					hDisplayContext,
					PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT);
	if ((psDCDisplayContextDestroyOUT->eError != PVRSRV_OK)
	    && (psDCDisplayContextDestroyOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCDisplayContextDestroy: %s",
			 PVRSRVGetErrorStringKM(psDCDisplayContextDestroyOUT->
						eError)));
		UnlockHandle();
		goto DCDisplayContextDestroy_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCDisplayContextDestroy_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferAlloc(IMG_UINT32 ui32DispatchTableEntry,
			  PVRSRV_BRIDGE_IN_DCBUFFERALLOC * psDCBufferAllocIN,
			  PVRSRV_BRIDGE_OUT_DCBUFFERALLOC * psDCBufferAllocOUT,
			  CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDisplayContext = psDCBufferAllocIN->hDisplayContext;
	DC_DISPLAY_CONTEXT *psDisplayContextInt = NULL;
	DC_BUFFER *psBufferInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCBufferAllocOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDisplayContextInt,
				       hDisplayContext,
				       PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT,
				       IMG_TRUE);
	if (psDCBufferAllocOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferAlloc_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCBufferAllocOUT->eError =
	    DCBufferAlloc(psDisplayContextInt,
			  &psDCBufferAllocIN->sSurfInfo,
			  &psDCBufferAllocOUT->ui32Stride, &psBufferInt);
	/* Exit early if bridged call fails */
	if (psDCBufferAllocOUT->eError != PVRSRV_OK)
	{
		goto DCBufferAlloc_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCBufferAllocOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDCBufferAllocOUT->hBuffer,
				      (void *)psBufferInt,
				      PVRSRV_HANDLE_TYPE_DC_BUFFER,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) & DCBufferFree);
	if (psDCBufferAllocOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferAlloc_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCBufferAlloc_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDisplayContextInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDisplayContext,
					    PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	if (psDCBufferAllocOUT->eError != PVRSRV_OK)
	{
		if (psBufferInt)
		{
			DCBufferFree(psBufferInt);
		}
	}

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferImport(IMG_UINT32 ui32DispatchTableEntry,
			   PVRSRV_BRIDGE_IN_DCBUFFERIMPORT * psDCBufferImportIN,
			   PVRSRV_BRIDGE_OUT_DCBUFFERIMPORT *
			   psDCBufferImportOUT, CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hDisplayContext = psDCBufferImportIN->hDisplayContext;
	DC_DISPLAY_CONTEXT *psDisplayContextInt = NULL;
	PMR **psImportInt = NULL;
	IMG_HANDLE *hImportInt2 = NULL;
	DC_BUFFER *psBufferInt = NULL;

	IMG_UINT32 ui32NextOffset = 0;
	IMG_BYTE *pArrayArgsBuffer = NULL;
#if !defined(INTEGRITY_OS)
	IMG_BOOL bHaveEnoughSpace = IMG_FALSE;
#endif

	IMG_UINT32 ui32BufferSize =
	    (psDCBufferImportIN->ui32NumPlanes * sizeof(PMR *)) +
	    (psDCBufferImportIN->ui32NumPlanes * sizeof(IMG_HANDLE)) + 0;

	if (ui32BufferSize != 0)
	{
#if !defined(INTEGRITY_OS)
		/* Try to use remainder of input buffer for copies if possible, word-aligned for safety. */
		IMG_UINT32 ui32InBufferOffset =
		    PVR_ALIGN(sizeof(*psDCBufferImportIN),
			      sizeof(unsigned long));
		IMG_UINT32 ui32InBufferExcessSize =
		    ui32InBufferOffset >=
		    PVRSRV_MAX_BRIDGE_IN_SIZE ? 0 : PVRSRV_MAX_BRIDGE_IN_SIZE -
		    ui32InBufferOffset;

		bHaveEnoughSpace = ui32BufferSize <= ui32InBufferExcessSize;
		if (bHaveEnoughSpace)
		{
			IMG_BYTE *pInputBuffer =
			    (IMG_BYTE *) psDCBufferImportIN;

			pArrayArgsBuffer = &pInputBuffer[ui32InBufferOffset];
		}
		else
#endif
		{
			pArrayArgsBuffer = OSAllocMemNoStats(ui32BufferSize);

			if (!pArrayArgsBuffer)
			{
				psDCBufferImportOUT->eError =
				    PVRSRV_ERROR_OUT_OF_MEMORY;
				goto DCBufferImport_exit;
			}
		}
	}

	if (psDCBufferImportIN->ui32NumPlanes != 0)
	{
		psImportInt =
		    (PMR **) (((IMG_UINT8 *) pArrayArgsBuffer) +
			      ui32NextOffset);
		ui32NextOffset +=
		    psDCBufferImportIN->ui32NumPlanes * sizeof(PMR *);
		hImportInt2 =
		    (IMG_HANDLE *) (((IMG_UINT8 *) pArrayArgsBuffer) +
				    ui32NextOffset);
		ui32NextOffset +=
		    psDCBufferImportIN->ui32NumPlanes * sizeof(IMG_HANDLE);
	}

	/* Copy the data over */
	if (psDCBufferImportIN->ui32NumPlanes * sizeof(IMG_HANDLE) > 0)
	{
		if (OSCopyFromUser
		    (NULL, hImportInt2,
		     (const void __user *)psDCBufferImportIN->phImport,
		     psDCBufferImportIN->ui32NumPlanes * sizeof(IMG_HANDLE)) !=
		    PVRSRV_OK)
		{
			psDCBufferImportOUT->eError =
			    PVRSRV_ERROR_INVALID_PARAMS;

			goto DCBufferImport_exit;
		}
	}

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCBufferImportOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psDisplayContextInt,
				       hDisplayContext,
				       PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT,
				       IMG_TRUE);
	if (psDCBufferImportOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferImport_exit;
	}

	{
		IMG_UINT32 i;

		for (i = 0; i < psDCBufferImportIN->ui32NumPlanes; i++)
		{
			/* Look up the address from the handle */
			psDCBufferImportOUT->eError =
			    PVRSRVLookupHandleUnlocked(psConnection->
						       psHandleBase,
						       (void **)&psImportInt[i],
						       hImportInt2[i],
						       PVRSRV_HANDLE_TYPE_PHYSMEM_PMR,
						       IMG_TRUE);
			if (psDCBufferImportOUT->eError != PVRSRV_OK)
			{
				UnlockHandle();
				goto DCBufferImport_exit;
			}
		}
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCBufferImportOUT->eError =
	    DCBufferImport(psDisplayContextInt,
			   psDCBufferImportIN->ui32NumPlanes,
			   psImportInt,
			   &psDCBufferImportIN->sSurfAttrib, &psBufferInt);
	/* Exit early if bridged call fails */
	if (psDCBufferImportOUT->eError != PVRSRV_OK)
	{
		goto DCBufferImport_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCBufferImportOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDCBufferImportOUT->hBuffer,
				      (void *)psBufferInt,
				      PVRSRV_HANDLE_TYPE_DC_BUFFER,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) & DCBufferFree);
	if (psDCBufferImportOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferImport_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCBufferImport_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psDisplayContextInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hDisplayContext,
					    PVRSRV_HANDLE_TYPE_DC_DISPLAY_CONTEXT);
	}

	if (hImportInt2)
	{
		IMG_UINT32 i;

		for (i = 0; i < psDCBufferImportIN->ui32NumPlanes; i++)
		{

			/* Unreference the previously looked up handle */
			if (hImportInt2[i])
			{
				PVRSRVReleaseHandleUnlocked(psConnection->
							    psHandleBase,
							    hImportInt2[i],
							    PVRSRV_HANDLE_TYPE_PHYSMEM_PMR);
			}
		}
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	if (psDCBufferImportOUT->eError != PVRSRV_OK)
	{
		if (psBufferInt)
		{
			DCBufferFree(psBufferInt);
		}
	}

	/* Allocated space should be equal to the last updated offset */
	PVR_ASSERT(ui32BufferSize == ui32NextOffset);

#if defined(INTEGRITY_OS)
	if (pArrayArgsBuffer)
#else
	if (!bHaveEnoughSpace && pArrayArgsBuffer)
#endif
		OSFreeMemNoStats(pArrayArgsBuffer);

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferFree(IMG_UINT32 ui32DispatchTableEntry,
			 PVRSRV_BRIDGE_IN_DCBUFFERFREE * psDCBufferFreeIN,
			 PVRSRV_BRIDGE_OUT_DCBUFFERFREE * psDCBufferFreeOUT,
			 CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCBufferFreeOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					(IMG_HANDLE) psDCBufferFreeIN->hBuffer,
					PVRSRV_HANDLE_TYPE_DC_BUFFER);
	if ((psDCBufferFreeOUT->eError != PVRSRV_OK) &&
	    (psDCBufferFreeOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCBufferFree: %s",
			 PVRSRVGetErrorStringKM(psDCBufferFreeOUT->eError)));
		UnlockHandle();
		goto DCBufferFree_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCBufferFree_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferUnimport(IMG_UINT32 ui32DispatchTableEntry,
			     PVRSRV_BRIDGE_IN_DCBUFFERUNIMPORT *
			     psDCBufferUnimportIN,
			     PVRSRV_BRIDGE_OUT_DCBUFFERUNIMPORT *
			     psDCBufferUnimportOUT,
			     CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCBufferUnimportOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					(IMG_HANDLE) psDCBufferUnimportIN->
					hBuffer, PVRSRV_HANDLE_TYPE_DC_BUFFER);
	if ((psDCBufferUnimportOUT->eError != PVRSRV_OK)
	    && (psDCBufferUnimportOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCBufferUnimport: %s",
			 PVRSRVGetErrorStringKM(psDCBufferUnimportOUT->
						eError)));
		UnlockHandle();
		goto DCBufferUnimport_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCBufferUnimport_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferPin(IMG_UINT32 ui32DispatchTableEntry,
			PVRSRV_BRIDGE_IN_DCBUFFERPIN * psDCBufferPinIN,
			PVRSRV_BRIDGE_OUT_DCBUFFERPIN * psDCBufferPinOUT,
			CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hBuffer = psDCBufferPinIN->hBuffer;
	DC_BUFFER *psBufferInt = NULL;
	DC_PIN_HANDLE hPinHandleInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCBufferPinOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psBufferInt,
				       hBuffer,
				       PVRSRV_HANDLE_TYPE_DC_BUFFER, IMG_TRUE);
	if (psDCBufferPinOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferPin_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCBufferPinOUT->eError = DCBufferPin(psBufferInt, &hPinHandleInt);
	/* Exit early if bridged call fails */
	if (psDCBufferPinOUT->eError != PVRSRV_OK)
	{
		goto DCBufferPin_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCBufferPinOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psHandleBase,
				      &psDCBufferPinOUT->hPinHandle,
				      (void *)hPinHandleInt,
				      PVRSRV_HANDLE_TYPE_DC_PIN_HANDLE,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) & DCBufferUnpin);
	if (psDCBufferPinOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferPin_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCBufferPin_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psBufferInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hBuffer,
					    PVRSRV_HANDLE_TYPE_DC_BUFFER);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	if (psDCBufferPinOUT->eError != PVRSRV_OK)
	{
		if (hPinHandleInt)
		{
			DCBufferUnpin(hPinHandleInt);
		}
	}

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferUnpin(IMG_UINT32 ui32DispatchTableEntry,
			  PVRSRV_BRIDGE_IN_DCBUFFERUNPIN * psDCBufferUnpinIN,
			  PVRSRV_BRIDGE_OUT_DCBUFFERUNPIN * psDCBufferUnpinOUT,
			  CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCBufferUnpinOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					(IMG_HANDLE) psDCBufferUnpinIN->
					hPinHandle,
					PVRSRV_HANDLE_TYPE_DC_PIN_HANDLE);
	if ((psDCBufferUnpinOUT->eError != PVRSRV_OK)
	    && (psDCBufferUnpinOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCBufferUnpin: %s",
			 PVRSRVGetErrorStringKM(psDCBufferUnpinOUT->eError)));
		UnlockHandle();
		goto DCBufferUnpin_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCBufferUnpin_exit:

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferAcquire(IMG_UINT32 ui32DispatchTableEntry,
			    PVRSRV_BRIDGE_IN_DCBUFFERACQUIRE *
			    psDCBufferAcquireIN,
			    PVRSRV_BRIDGE_OUT_DCBUFFERACQUIRE *
			    psDCBufferAcquireOUT,
			    CONNECTION_DATA * psConnection)
{
	IMG_HANDLE hBuffer = psDCBufferAcquireIN->hBuffer;
	DC_BUFFER *psBufferInt = NULL;
	PMR *psExtMemInt = NULL;

	/* Lock over handle lookup. */
	LockHandle();

	/* Look up the address from the handle */
	psDCBufferAcquireOUT->eError =
	    PVRSRVLookupHandleUnlocked(psConnection->psHandleBase,
				       (void **)&psBufferInt,
				       hBuffer,
				       PVRSRV_HANDLE_TYPE_DC_BUFFER, IMG_TRUE);
	if (psDCBufferAcquireOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferAcquire_exit;
	}
	/* Release now we have looked up handles. */
	UnlockHandle();

	psDCBufferAcquireOUT->eError =
	    DCBufferAcquire(psBufferInt, &psExtMemInt);
	/* Exit early if bridged call fails */
	if (psDCBufferAcquireOUT->eError != PVRSRV_OK)
	{
		goto DCBufferAcquire_exit;
	}

	/* Lock over handle creation. */
	LockHandle();

	psDCBufferAcquireOUT->eError =
	    PVRSRVAllocHandleUnlocked(psConnection->psProcessHandleBase->
				      psHandleBase,
				      &psDCBufferAcquireOUT->hExtMem,
				      (void *)psExtMemInt,
				      PVRSRV_HANDLE_TYPE_DEVMEM_MEM_IMPORT,
				      PVRSRV_HANDLE_ALLOC_FLAG_MULTI,
				      (PFN_HANDLE_RELEASE) & DCBufferRelease);
	if (psDCBufferAcquireOUT->eError != PVRSRV_OK)
	{
		UnlockHandle();
		goto DCBufferAcquire_exit;
	}

	/* Release now we have created handles. */
	UnlockHandle();

 DCBufferAcquire_exit:

	/* Lock over handle lookup cleanup. */
	LockHandle();

	/* Unreference the previously looked up handle */
	if (psBufferInt)
	{
		PVRSRVReleaseHandleUnlocked(psConnection->psHandleBase,
					    hBuffer,
					    PVRSRV_HANDLE_TYPE_DC_BUFFER);
	}
	/* Release now we have cleaned up look up handles. */
	UnlockHandle();

	if (psDCBufferAcquireOUT->eError != PVRSRV_OK)
	{
		if (psExtMemInt)
		{
			DCBufferRelease(psExtMemInt);
		}
	}

	return 0;
}

static IMG_INT
PVRSRVBridgeDCBufferRelease(IMG_UINT32 ui32DispatchTableEntry,
			    PVRSRV_BRIDGE_IN_DCBUFFERRELEASE *
			    psDCBufferReleaseIN,
			    PVRSRV_BRIDGE_OUT_DCBUFFERRELEASE *
			    psDCBufferReleaseOUT,
			    CONNECTION_DATA * psConnection)
{

	/* Lock over handle destruction. */
	LockHandle();

	psDCBufferReleaseOUT->eError =
	    PVRSRVReleaseHandleUnlocked(psConnection->psProcessHandleBase->
					psHandleBase,
					(IMG_HANDLE) psDCBufferReleaseIN->
					hExtMem,
					PVRSRV_HANDLE_TYPE_DEVMEM_MEM_IMPORT);
	if ((psDCBufferReleaseOUT->eError != PVRSRV_OK)
	    && (psDCBufferReleaseOUT->eError != PVRSRV_ERROR_RETRY))
	{
		PVR_DPF((PVR_DBG_ERROR,
			 "PVRSRVBridgeDCBufferRelease: %s",
			 PVRSRVGetErrorStringKM(psDCBufferReleaseOUT->eError)));
		UnlockHandle();
		goto DCBufferRelease_exit;
	}

	/* Release now we have destroyed handles. */
	UnlockHandle();

 DCBufferRelease_exit:

	return 0;
}

/* *************************************************************************** 
 * Server bridge dispatch related glue 
 */

static IMG_BOOL bUseLock = IMG_TRUE;

PVRSRV_ERROR InitDCBridge(void);
PVRSRV_ERROR DeinitDCBridge(void);

/*
 * Register all DC functions with services
 */
PVRSRV_ERROR InitDCBridge(void)
{

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDEVICESQUERYCOUNT,
			      PVRSRVBridgeDCDevicesQueryCount, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDEVICESENUMERATE,
			      PVRSRVBridgeDCDevicesEnumerate, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDEVICEACQUIRE,
			      PVRSRVBridgeDCDeviceAcquire, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDEVICERELEASE,
			      PVRSRVBridgeDCDeviceRelease, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCGETINFO,
			      PVRSRVBridgeDCGetInfo, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCPANELQUERYCOUNT,
			      PVRSRVBridgeDCPanelQueryCount, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCPANELQUERY,
			      PVRSRVBridgeDCPanelQuery, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCFORMATQUERY,
			      PVRSRVBridgeDCFormatQuery, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCDIMQUERY,
			      PVRSRVBridgeDCDimQuery, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCSETBLANK,
			      PVRSRVBridgeDCSetBlank, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCSETVSYNCREPORTING,
			      PVRSRVBridgeDCSetVSyncReporting, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCLASTVSYNCQUERY,
			      PVRSRVBridgeDCLastVSyncQuery, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCSYSTEMBUFFERACQUIRE,
			      PVRSRVBridgeDCSystemBufferAcquire, NULL,
			      bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCSYSTEMBUFFERRELEASE,
			      PVRSRVBridgeDCSystemBufferRelease, NULL,
			      bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTCREATE,
			      PVRSRVBridgeDCDisplayContextCreate, NULL,
			      bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTCONFIGURECHECK,
			      PVRSRVBridgeDCDisplayContextConfigureCheck, NULL,
			      bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTCONFIGURE,
			      PVRSRVBridgeDCDisplayContextConfigure, NULL,
			      bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTDESTROY,
			      PVRSRVBridgeDCDisplayContextDestroy, NULL,
			      bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCBUFFERALLOC,
			      PVRSRVBridgeDCBufferAlloc, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCBUFFERIMPORT,
			      PVRSRVBridgeDCBufferImport, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCBUFFERFREE,
			      PVRSRVBridgeDCBufferFree, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCBUFFERUNIMPORT,
			      PVRSRVBridgeDCBufferUnimport, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCBUFFERPIN,
			      PVRSRVBridgeDCBufferPin, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCBUFFERUNPIN,
			      PVRSRVBridgeDCBufferUnpin, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCBUFFERACQUIRE,
			      PVRSRVBridgeDCBufferAcquire, NULL, bUseLock);

	SetDispatchTableEntry(PVRSRV_BRIDGE_DC,
			      PVRSRV_BRIDGE_DC_DCBUFFERRELEASE,
			      PVRSRVBridgeDCBufferRelease, NULL, bUseLock);

	return PVRSRV_OK;
}

/*
 * Unregister all dc functions with services
 */
PVRSRV_ERROR DeinitDCBridge(void)
{

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDEVICESQUERYCOUNT);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDEVICESENUMERATE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDEVICEACQUIRE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDEVICERELEASE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCGETINFO);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCPANELQUERYCOUNT);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCPANELQUERY);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCFORMATQUERY);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCDIMQUERY);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCSETBLANK);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCSETVSYNCREPORTING);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCLASTVSYNCQUERY);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCSYSTEMBUFFERACQUIRE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCSYSTEMBUFFERRELEASE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTCREATE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTCONFIGURECHECK);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTCONFIGURE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCDISPLAYCONTEXTDESTROY);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERALLOC);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERIMPORT);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERFREE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERUNIMPORT);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC, PVRSRV_BRIDGE_DC_DCBUFFERPIN);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERUNPIN);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERACQUIRE);

	UnsetDispatchTableEntry(PVRSRV_BRIDGE_DC,
				PVRSRV_BRIDGE_DC_DCBUFFERRELEASE);

	return PVRSRV_OK;
}
