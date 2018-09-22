/*************************************************************************/ /*!
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

#if defined(LINUX)

#include <linux/pci.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

/* For MODULE_LICENSE */
#include "pvrmodule.h"

#include "dc_osfuncs.h"
#include "dc_pdp.h"

#if defined(ENABLE_PLATO_HDMI)
/* APIs needed from plato_hdmi */
EXPORT_SYMBOL(PDPInitialize);
#endif

#define DCPDP_WIDTH_MIN			(640)
#define DCPDP_WIDTH_MAX			(1920)
#define DCPDP_HEIGHT_MIN		(480)
#define DCPDP_HEIGHT_MAX		(1080)
#define DCPDP_WIDTH_DEFAULT		(640)
#define DCPDP_HEIGHT_DEFAULT	(480)

#define DCPDP_DEBUGFS_DISPLAY_ENABLED	"display_enabled"

#if defined (DCPDP_WIDTH) && !defined (DCPDP_HEIGHT)
#error ERROR: DCPDP_WIDTH defined but DCPDP_HEIGHT not defined
#elif !defined (DCPDP_WIDTH) && defined (DCPDP_HEIGHT)
#error ERROR: DCPDP_HEIGHT defined but DCPDP_WIDTH not defined
#elif !defined (DCPDP_WIDTH) && !defined (DCPDP_HEIGHT)
#define DCPDP_WIDTH			DCPDP_WIDTH_DEFAULT
#define DCPDP_HEIGHT		DCPDP_HEIGHT_DEFAULT
#elif (DCPDP_WIDTH > DCPDP_WIDTH_MAX)
#error ERROR: DCPDP_WIDTH too large (max: 1920)
#elif (DCPDP_WIDTH < DCPDP_WIDTH_MIN)
#error ERROR: DCPDP_WIDTH too small (min: 640)
#elif (DCPDP_HEIGHT > DCPDP_HEIGHT_MAX)
#error ERROR: DCPDP_HEIGHT too large (max: 1080)
#elif (DCPDP_HEIGHT < DCPDP_HEIGHT_MIN)
#error ERROR: DCPDP_HEIGHT too small (max: 480)
#endif

static DCPDP_DEVICE	*g_psDeviceData;

static struct dentry	*g_psDebugFSEntryDir;
static struct dentry	*g_psDisplayEnabledEntry;

/* PDP module parameters */
DCPDP_MODULE_PARAMETERS  sModuleParams =
{
	.ui32PDPEnabled = 1,
	.ui32PDPWidth   = DCPDP_WIDTH,
	.ui32PDPHeight  = DCPDP_HEIGHT
};
module_param_named(mem_en, sModuleParams.ui32PDPEnabled, uint, S_IRUGO | S_IWUSR);
module_param_named(width,  sModuleParams.ui32PDPWidth,   uint, S_IRUGO | S_IWUSR);
module_param_named(height, sModuleParams.ui32PDPHeight,  uint, S_IRUGO | S_IWUSR);

const DCPDP_MODULE_PARAMETERS *DCPDPGetModuleParameters(void)
{
	return &sModuleParams;
}

static int DisplayEnabledOpen(struct inode *psINode, struct file *psFile)
{
	psFile->private_data = psINode->i_private;

	return 0;
}

static ssize_t DisplayEnabledRead(struct file *psFile,
				  char __user *psUserBuffer,
				  size_t uiCount,
				  loff_t *puiPosition)
{
	IMG_UINT32 ui32PDPEnabled = *((IMG_UINT32 *)psFile->private_data);
	loff_t uiPosition = *puiPosition;
	char pszBuffer[] = "N\n";
	size_t uiBufferSize = ARRAY_SIZE(pszBuffer);
	int iErr;

	if (uiPosition < 0)
	{
		return -EINVAL;
	}
	else if (uiPosition >= uiBufferSize || uiCount == 0)
	{
		return 0;
	}

	if (ui32PDPEnabled)
	{
		pszBuffer[0] = 'Y';
	}

	if (uiCount > uiBufferSize - uiPosition)
	{
		uiCount = uiBufferSize - uiPosition;
	}

	iErr = copy_to_user(psUserBuffer, &pszBuffer[uiPosition], uiCount);
	if (iErr)
	{
		return -EFAULT;
	}

	*puiPosition = uiPosition + uiCount;

	return uiCount;
}

static ssize_t DisplayEnabledWrite(struct file *psFile,
				   const char __user *psUserBuffer,
				   size_t uiCount,
				   loff_t *puiPosition)
{
	IMG_UINT32 *pui32PDPEnabled = psFile->private_data;
	char pszBuffer[3];
	bool bPDPEnabled;
	int iErr;

	uiCount = min(uiCount, ARRAY_SIZE(pszBuffer) - 1);

	iErr = copy_from_user(pszBuffer, psUserBuffer, uiCount);
	if (iErr)
	{
		return -EFAULT;
	}

	pszBuffer[uiCount] = '\0';

	if (strtobool(pszBuffer, &bPDPEnabled) == 0)
	{
		*pui32PDPEnabled = bPDPEnabled ? 1 : 0;

		DCPDPEnableMemoryRequest(g_psDeviceData, bPDPEnabled);
	}

	return uiCount;
}

#if defined(PDP_DEBUG)
static long DisplayEnabledIoctl(struct file *file, unsigned int ioctl, unsigned long argp)
{
	PDPDebugCtrl();
	return 0;
}
#endif /* PDP_DEBUG */

static const struct file_operations gsDisplayEnabledFileOps =
{
	.owner = THIS_MODULE,
	.open = DisplayEnabledOpen,
	.read = DisplayEnabledRead,
	.write = DisplayEnabledWrite,
	.llseek = default_llseek,
#if defined(PDP_DEBUG)
	.unlocked_ioctl = DisplayEnabledIoctl
#endif /* PDP_DEBUG */
};

PVRSRV_ERROR PDPInitialize(VIDEO_PARAMS * pVideoParams)
{
	PVRSRV_ERROR eError;

	if (g_psDeviceData->bPreInitDone == IMG_FALSE)
	{
		printk(KERN_ERR DRVNAME "-%s: Called before PreInit is complete\n", __FUNCTION__);
		return -ENODEV;
	}

	g_psDeviceData->videoParams = *pVideoParams;

	eError = DCPDPStart(g_psDeviceData);
	if (eError != PVRSRV_OK)
	{
		printk(KERN_ERR DRVNAME " - %s: Failed to initialise device (%d)\n", __FUNCTION__, eError);
		return -ENODEV;
	}

	return eError;
}

static int __init dc_pdp_init(void)
{
	struct pci_dev *psPCIDev;
	PVRSRV_ERROR eError;
	int error;

	psPCIDev = pci_get_device(DCPDP2_VENDOR_ID_PLATO, DCPDP2_DEVICE_ID_PCI_PLATO, NULL);
	if (psPCIDev == NULL)
	{
		printk(KERN_ERR DRVNAME " - %s : Failed to get PCI device\n", __FUNCTION__);
		return -ENODEV;
	}

	error = pci_enable_device(psPCIDev);
	if (error != 0)
	{
		printk(KERN_ERR DRVNAME " - %s: Failed to enable PCI device (%d)\n", __FUNCTION__, error);
		return -ENODEV;
	}

	eError = DCPDPInit(&psPCIDev->dev, &g_psDeviceData);
	if (eError != PVRSRV_OK)
	{
		printk(KERN_ERR DRVNAME " - %s: Failed to initialise device (%d)\n", __FUNCTION__, eError);
		return -ENODEV;
	}

	/* To prevent possible problems with system suspend/resume, we don't
	   keep the device enabled, but rely on the fact that the Rogue driver
	   will have done a pci_enable_device. */
	pci_disable_device(psPCIDev);

	g_psDebugFSEntryDir = debugfs_create_dir(DRVNAME, NULL);
	if (IS_ERR_OR_NULL(g_psDebugFSEntryDir))
	{
		printk(KERN_WARNING DRVNAME " - %s: Failed to create '%s' debugfs root directory "
		       "(debugfs entries won't be available)\n", __FUNCTION__, DRVNAME);
		g_psDebugFSEntryDir = NULL;
	}

	g_psDisplayEnabledEntry = debugfs_create_file(DCPDP_DEBUGFS_DISPLAY_ENABLED,
						      S_IFREG | S_IRUGO | S_IWUSR,
						      g_psDebugFSEntryDir,
						      &sModuleParams.ui32PDPEnabled,
						      &gsDisplayEnabledFileOps);
	if (IS_ERR_OR_NULL(g_psDisplayEnabledEntry))
	{
		printk(KERN_WARNING DRVNAME " - %s: Failed to create '%s' debugfs entry\n",
		       __FUNCTION__, DCPDP_DEBUGFS_DISPLAY_ENABLED);
		g_psDisplayEnabledEntry = NULL;
	}

	g_psDeviceData->bPreInitDone = IMG_TRUE;

	return 0;
}

static void __exit dc_pdp_deinit(void)
{
	if (g_psDeviceData)
	{
		struct pci_dev *psPCIDev;

		if (g_psDebugFSEntryDir)
		{
			if (g_psDisplayEnabledEntry)
			{
				debugfs_remove(g_psDisplayEnabledEntry);
                g_psDisplayEnabledEntry = NULL;
			}

			debugfs_remove(g_psDebugFSEntryDir);
            g_psDebugFSEntryDir = NULL;
		}

		DCPDPDeInit(g_psDeviceData, (void *)&psPCIDev);
		g_psDeviceData = NULL;
	}
}

#if defined(PLATO_DISPLAY_PDUMP)

void PDP_OSWriteHWReg32(void *pvLinRegBaseAddr, IMG_UINT32 ui32Offset, IMG_UINT32 ui32Value)
{
	OSWriteHWReg32(pvLinRegBaseAddr,  ui32Offset,  ui32Value);
	if (pvLinRegBaseAddr == g_psDeviceData->pvPDPRegCpuVAddr)
	{
		plato_pdump_reg32(pvLinRegBaseAddr, ui32Offset, ui32Value, DRVNAME);
	}
	else if (pvLinRegBaseAddr == g_psDeviceData->pvPDPBifRegCpuVAddr)
	{
		plato_pdump_reg32(pvLinRegBaseAddr, ui32Offset, ui32Value, "pdp_bif");
	}
}
#endif

module_init(dc_pdp_init);
module_exit(dc_pdp_deinit);

#endif /* defined(LINUX) */
