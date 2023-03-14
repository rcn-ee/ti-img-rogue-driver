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

#define PVRSRV_APPHINT_FIRMWARE_HEAP_POLICY 5
#define RGX_FW_FILENAME "rgx.fw"
#define RGX_SH_FILENAME "rgx.sh"
#define PVR_BUILD_DIR "am62_android"
#define PVR_BUILD_TYPE "release"
#define PVRSRV_MODNAME "pvrsrvkm"
#define PVRSYNC_MODNAME "pvr_sync"
#define SUPPORT_RGX 1
#define USE_PVRSYNC_DEVNODE
#define PVRSRV_MAX_DEVICES 4
#define PVRSRV_HWPERF_COUNTERS_PERBLK 12
#define RELEASE
#define RGX_BVNC_CORE_KM_HEADER "cores/rgxcore_km_33.15.11.3.h"
#define RGX_BNC_CONFIG_KM_HEADER "configs/rgxconfig_km_33.V.11.3.h"
#define PVRSRV_NEED_PVR_DPF
#define SUPPORT_PHYSMEM_TEST
#define SUPPORT_RGXTQ_BRIDGE
#define PVRSRV_POISON_ON_ALLOC_VALUE 0xd9
#define PVRSRV_POISON_ON_FREE_VALUE 0x63
#define RGX_NUM_DRIVERS_SUPPORTED 1
#define RGX_DRIVERID_0_DEFAULT_PRIORITY (1 - 0)
#define RGX_DRIVERID_1_DEFAULT_PRIORITY (1 - 1)
#define RGX_DRIVERID_2_DEFAULT_PRIORITY (1 - 2)
#define RGX_DRIVERID_3_DEFAULT_PRIORITY (1 - 3)
#define RGX_DRIVERID_4_DEFAULT_PRIORITY (1 - 4)
#define RGX_DRIVERID_5_DEFAULT_PRIORITY (1 - 5)
#define RGX_DRIVERID_6_DEFAULT_PRIORITY (1 - 6)
#define RGX_DRIVERID_7_DEFAULT_PRIORITY (1 - 7)
#define RGX_DRIVERID_0_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_1_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_2_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_3_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_4_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_5_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_6_DEFAULT_ISOLATION_GROUP 0
#define RGX_DRIVERID_7_DEFAULT_ISOLATION_GROUP 0
#define RGX_HCS_DEFAULT_DEADLINE_MS 0xFFFFFFFFU
#define DRIVER0_SECURITY_SUPPORT 0
#define DRIVER1_SECURITY_SUPPORT 0
#define DRIVER2_SECURITY_SUPPORT 0
#define DRIVER3_SECURITY_SUPPORT 0
#define DRIVER4_SECURITY_SUPPORT 0
#define DRIVER5_SECURITY_SUPPORT 0
#define DRIVER6_SECURITY_SUPPORT 0
#define DRIVER7_SECURITY_SUPPORT 0
#define RGX_FW_HEAP_USES_FIRMWARE_OSID 0
#define RGX_FW_HEAP_USES_HOST_OSID 1
#define RGX_FW_HEAP_USES_DEDICATED_OSID 2
#define RGX_FW_HEAP_OSID_ASSIGNMENT RGX_FW_HEAP_USES_FIRMWARE_OSID
#define PVRSRV_APPHINT_PHYSHEAPMINMEMONCONNECTION 0
#define RGX_FW_PHYSHEAP_MINMEM_ON_CONNECTION  512
#define PVRSRV_APPHINT_DRIVERMODE 0x7FFFFFFF
#define RGX_FW_HEAP_SHIFT 25
#define RGX_VZ_CONNECTION_TIMEOUT_US 60000000
#define GPUVIRT_VALIDATION_NUM_OS 8
#define PVRSRV_ENABLE_CCCB_GROW
#define SUPPORT_POWMON_COMPONENT
#define PVR_POWER_ACTOR_MEASUREMENT_PERIOD_MS 10U
#define PVR_POWER_MONITOR_HWPERF
#define PVR_LDM_PLATFORM_PRE_REGISTERED
#define PVR_LDM_DRIVER_REGISTRATION_NAME "pvrsrvkm"
#define PVRSRV_FULL_SYNC_TRACKING_HISTORY_LEN 256
#define ION_DEFAULT_HEAP_NAME "reserved"
#define ION_DEFAULT_HEAP_ID_MASK (1 << ION_HEAP_TYPE_SYSTEM)
#define PVRSRV_APPHINT_HWRDEBUGDUMPLIMIT APPHNT_BLDVAR_DBGDUMPLIMIT
#define PVRSRV_APPHINT_ENABLETRUSTEDDEVICEACECONFIG IMG_FALSE
#define PVRSRV_APPHINT_GENERALNON4KHEAPPAGESIZE 0x4000
#define PVRSRV_APPHINT_HWPERFCLIENTBUFFERSIZE 786432
#define PVRSRV_APPHINT_ENABLESIGNATURECHECKS APPHNT_BLDVAR_ENABLESIGNATURECHECKS
#define PVRSRV_APPHINT_SIGNATURECHECKSBUFSIZE RGXFW_SIG_BUFFER_SIZE_MIN
#define PVRSRV_APPHINT_ENABLEFULLSYNCTRACKING IMG_FALSE
#define PVRSRV_APPHINT_ENABLEPAGEFAULTDEBUG APPHNT_BLDVAR_ENABLEPAGEFAULTDEBUG
#define PVRSRV_APPHINT_VALIDATEIRQ 0
#define PVRSRV_APPHINT_DISABLECLOCKGATING 0
#define PVRSRV_APPHINT_DISABLEDMOVERLAP 0
#define PVRSRV_APPHINT_ENABLECDMKILLINGRANDMODE 0
#define PVRSRV_APPHINT_ENABLERANDOMCONTEXTSWITCH 0
#define PVRSRV_APPHINT_ENABLESOFTRESETCNTEXTSWITCH 0
#define PVRSRV_APPHINT_ENABLEFWCONTEXTSWITCH RGXFWIF_INICFG_OS_CTXSWITCH_DM_ALL
#define PVRSRV_APPHINT_ENABLERDPOWERISLAND RGX_RD_POWER_ISLAND_DEFAULT
#define PVRSRV_APPHINT_ENABLESPUCLOCKGATING IMG_FALSE
#define PVRSRV_APPHINT_FIRMWAREPERF FW_PERF_CONF_NONE
#define PVRSRV_APPHINT_FWCONTEXTSWITCHPROFILE RGXFWIF_CTXSWITCH_PROFILE_MEDIUM_EN
#define PVRSRV_APPHINT_HWPERFDISABLECUSTOMCOUNTERFILTER 0
#define PVRSRV_APPHINT_HWPERFFWBUFSIZEINKB 2048
#define PVRSRV_APPHINT_HWPERFHOSTBUFSIZEINKB 2048
#define PVRSRV_APPHINT_HWPERFHOSTTHREADTIMEOUTINMS 50
#define PVRSRV_APPHINT_TFBCCOMPRESSIONCONTROLGROUP 1
#define PVRSRV_APPHINT_TFBCCOMPRESSIONCONTROLSCHEME 0
#define PVRSRV_APPHINT_JONESDISABLEMASK 0
#define PVRSRV_APPHINT_NEWFILTERINGMODE 1
#define PVRSRV_APPHINT_TRUNCATEMODE 0
#define PVRSRV_APPHINT_EMUMAXFREQ 0
#define PVRSRV_APPHINT_GPIOVALIDATIONMODE 0
#define PVRSRV_APPHINT_RGXBVNC ""
#define PVRSRV_APPHINT_CLEANUPTHREADPRIORITY 5
#define PVRSRV_APPHINT_WATCHDOGTHREADPRIORITY 0
#define PVRSRV_APPHINT_CACHEOPTHREADPRIORITY 1
#define PVRSRV_APPHINT_DEVMEM_HISTORY_BUFSIZE_LOG2 11
#define PVRSRV_APPHINT_DEVMEM_HISTORY_MAX_ENTRIES 10000
#define PVRSRV_APPHINT_ASSERTONHWRTRIGGER IMG_FALSE
#define PVRSRV_APPHINT_ASSERTOUTOFMEMORY IMG_FALSE
#define PVRSRV_APPHINT_CHECKMLIST APPHNT_BLDVAR_DEBUG
#define PVRSRV_APPHINT_DISABLEFEDLOGGING IMG_FALSE
#define PVRSRV_APPHINT_KCCB_SIZE_LOG2 7
#define PVRSRV_APPHINT_ENABLEAPM RGX_ACTIVEPM_DEFAULT
#define PVRSRV_APPHINT_ENABLEHTBLOGGROUP 0
#define PVRSRV_APPHINT_ENABLELOGGROUP RGXFWIF_LOG_TYPE_NONE
#define PVRSRV_APPHINT_FIRMWARELOGTYPE 0
#define PVRSRV_APPHINT_FWTRACEBUFSIZEINDWORDS RGXFW_TRACE_BUF_DEFAULT_SIZE_IN_DWORDS
#define PVRSRV_APPHINT_DEBUGDUMPFWTLOGTYPE 1
#define PVRSRV_APPHINT_FBCDCVERSIONOVERRIDE 0
#define PVRSRV_APPHINT_HTBOPERATIONMODE HTB_OPMODE_DROPOLDEST
#define PVRSRV_APPHINT_HTBUFFERSIZE 64
#define PVRSRV_APPHINT_ENABLEFTRACEGPU IMG_FALSE
#define PVRSRV_APPHINT_HWPERFFWFILTER 0
#define PVRSRV_APPHINT_HWPERFHOSTFILTER 0
#define PVRSRV_APPHINT_HWPERFCLIENTFILTER_SERVICES 0
#define PVRSRV_APPHINT_HWPERFCLIENTFILTER_EGL 0
#define PVRSRV_APPHINT_HWPERFCLIENTFILTER_OPENGLES 0
#define PVRSRV_APPHINT_HWPERFCLIENTFILTER_OPENCL 0
#define PVRSRV_APPHINT_HWPERFCLIENTFILTER_VULKAN 0
#define PVRSRV_APPHINT_HWPERFCLIENTFILTER_OPENGL 0
#define PVRSRV_APPHINT_TIMECORRCLOCK 0
#define PVRSRV_APPHINT_ENABLEFWPOISONONFREE IMG_FALSE
#define PVRSRV_APPHINT_FWPOISONONFREEVALUE 0xBD
#define PVRSRV_APPHINT_ZEROFREELIST IMG_FALSE
#define PVRSRV_APPHINT_GPUUNITSPOWERCHANGE IMG_FALSE
#define PVRSRV_APPHINT_DISABLEPDUMPPANIC IMG_FALSE
#define PVRSRV_APPHINT_CACHEOPCONFIG 0
#define PVRSRV_APPHINT_CACHEOPUMKMHRESHOLDSIZE 0
#define PVRSRV_APPHINT_IGNOREHWREPORTEDBVNC IMG_FALSE
#define PVRSRV_APPHINT_PHYSMEMTESTPASSES APPHNT_PHYSMEMTEST_ENABLE
#define PVRSRV_APPHINT_TESTSLRINTERVAL 0
#define PVRSRV_APPHINT_RISCVDMITEST 0
#define PVRSRV_APPHINT_VALIDATESOCUSCTIMERS 0
#define PVRSRV_APPHINT_CHECKPOINTPOOLMAXLOG2 8
#define PVRSRV_APPHINT_CHECKPOINTPOOLINITLOG2 7
#define SOC_TIMER_FREQ 20
#define PDVFS_COM_HOST 1
#define PDVFS_COM_AP 2
#define PDVFS_COM_PMC 3
#define PDVFS_COM_IMG_CLKDIV 4
#define PDVFS_COM PDVFS_COM_HOST
#define PVR_GPIO_MODE_GENERAL 1
#define PVR_GPIO_MODE_POWMON_PIN 2
#define PVR_GPIO_MODE PVR_GPIO_MODE_GENERAL
#define PVRSRV_ENABLE_PROCESS_STATS
#define SUPPORT_USC_BREAKPOINT
#define RGXFW_SAFETY_WATCHDOG_PERIOD_IN_US 2000000
#define PVR_ANNOTATION_MAX_LEN 63
#define PVRSRV_DEVICE_INIT_MODE PVRSRV_LINUX_DEV_INIT_ON_OPEN
#define SUPPORT_DI_BRG_IMPL
#define PVRSRV_ENABLE_MEMTRACK_STATS_FILE
#define PVR_LINUX_PHYSMEM_MAX_POOL_PAGES 10240
#define PVR_LINUX_PHYSMEM_MAX_EXCESS_POOL_PAGES 20480
#define PVR_LINUX_PHYSMEM_ZERO_ALL_PAGES
#define PVR_DIRTY_BYTES_FLUSH_THRESHOLD 524288
#define PVR_LINUX_HIGHORDER_ALLOCATION_THRESHOLD 256
#define PVR_LINUX_PHYSMEM_MAX_ALLOC_ORDER_NUM 2
#define PVR_LINUX_KMALLOC_ALLOCATION_THRESHOLD 16384
#define PVRSRV_USE_LINUX_CONFIG_INIT_ON_ALLOC 1
#define SUPPORT_NATIVE_FENCE_SYNC
#define PVRSRV_STALLED_CCB_ACTION
#define UPDATE_FENCE_CHECKPOINT_COUNT 1
#define PVR_DRM_NAME "pvr"
#define DEVICE_MEMSETCPY_ALIGN_IN_BYTES 16
#define RGX_INITIAL_SLR_HOLDOFF_PERIOD_MS 0
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_TQ3D 14
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_TQ2D 14
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_CDM 13
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_TA 15
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_3D 16
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_KICKSYNC 13
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_TDM 14
#define PVRSRV_RGX_LOG2_CLIENT_CCB_SIZE_RDM 13
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_TQ3D 17
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_TQ2D 17
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_CDM 15
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_TA 16
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_3D 17
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_KICKSYNC 13
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_TDM 17
#define PVRSRV_RGX_LOG2_CLIENT_CCB_MAX_SIZE_RDM 15
#define ANDROID
#define SUPPORT_DMA_HEAP
#define DMABUF_DEFAULT_HEAP_NAME "system"
#define PVR_ANDROID_ION_HEADER "linux/ion.h"
#define PVR_ANDROID_ION_PRIV_HEADER "../drivers/gpu/ion/ion_priv.h"
#define PVR_ANDROID_SYNC_HEADER "linux/sync.h"
#define PVRSRV_ANDROID_TRACE_GPU_WORK_PERIOD
#define PVRSRV_ANDROID_TRACE_GPU_FREQ