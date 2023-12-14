/*************************************************************************/ /*!
@File
@Title          RGX firmware interface structures used by pvrsrvkm
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    RGX firmware interface structures used by pvrsrvkm
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

#if !defined(RGX_FWIF_KM_H)
#define RGX_FWIF_KM_H

#include "img_types.h"
#include "rgx_fwif_shared.h"
#include "rgxdefs_km.h"
#include "dllist.h"
#include "rgx_hwperf.h"
#include "rgxheapconfig.h"

/************************************************************************
* RGX FW signature checks
************************************************************************/
#define RGXFW_SIG_BUFFER_SIZE_MIN       (8192)

#define RGXFWIF_TIMEDIFF_ID			((0x1UL << 28) | RGX_CR_TIMER)

#define RGXFW_POLL_TYPE_SET 0x80000000U

/* Firmware per-DM HWR states */
#define RGXFWIF_DM_STATE_WORKING					(0x00U)		/*!< DM is working if all flags are cleared */
#define RGXFWIF_DM_STATE_READY_FOR_HWR				(IMG_UINT32_C(0x1) << 0)	/*!< DM is idle and ready for HWR */
#define RGXFWIF_DM_STATE_NEEDS_SKIP					(IMG_UINT32_C(0x1) << 2)	/*!< DM need to skip to next cmd before resuming processing */
#define RGXFWIF_DM_STATE_NEEDS_PR_CLEANUP			(IMG_UINT32_C(0x1) << 3)	/*!< DM need partial render cleanup before resuming processing */
#define RGXFWIF_DM_STATE_NEEDS_TRACE_CLEAR			(IMG_UINT32_C(0x1) << 4)	/*!< DM need to increment Recovery Count once fully recovered */
#define RGXFWIF_DM_STATE_GUILTY_LOCKUP				(IMG_UINT32_C(0x1) << 5)	/*!< DM was identified as locking up and causing HWR */
#define RGXFWIF_DM_STATE_INNOCENT_LOCKUP			(IMG_UINT32_C(0x1) << 6)	/*!< DM was innocently affected by another lockup which caused HWR */
#define RGXFWIF_DM_STATE_GUILTY_OVERRUNING			(IMG_UINT32_C(0x1) << 7)	/*!< DM was identified as over-running and causing HWR */
#define RGXFWIF_DM_STATE_INNOCENT_OVERRUNING		(IMG_UINT32_C(0x1) << 8)	/*!< DM was innocently affected by another DM over-running which caused HWR */
#define RGXFWIF_DM_STATE_HARD_CONTEXT_SWITCH		(IMG_UINT32_C(0x1) << 9)	/*!< DM was forced into HWR as it delayed more important workloads */
#define RGXFWIF_DM_STATE_GPU_ECC_HWR				(IMG_UINT32_C(0x1) << 10)	/*!< DM was forced into HWR due to an uncorrected GPU ECC error */

/* Firmware's connection state */
typedef IMG_UINT32 RGXFWIF_CONNECTION_FW_STATE;
#define RGXFW_CONNECTION_FW_OFFLINE		0U	/*!< Firmware is offline */
#define RGXFW_CONNECTION_FW_READY		1U	/*!< Firmware is initialised */
#define RGXFW_CONNECTION_FW_ACTIVE		2U	/*!< Firmware connection is fully established */
#define RGXFW_CONNECTION_FW_OFFLOADING	3U	/*!< Firmware is clearing up connection data */
#define RGXFW_CONNECTION_FW_COOLDOWN	4U	/*!< Firmware connection is in cooldown period */
#define RGXFW_CONNECTION_FW_STATE_COUNT	5U

/* OS' connection state */
typedef enum
{
	RGXFW_CONNECTION_OS_OFFLINE = 0,	/*!< OS is offline */
	RGXFW_CONNECTION_OS_READY,			/*!< OS's KM driver is setup and waiting */
	RGXFW_CONNECTION_OS_ACTIVE,			/*!< OS connection is fully established */
	RGXFW_CONNECTION_OS_STATE_COUNT
} RGXFWIF_CONNECTION_OS_STATE;

#define RGXFWIF_CTXSWITCH_PROFILE_FAST_EN		(IMG_UINT32_C(0x1))
#define RGXFWIF_CTXSWITCH_PROFILE_MEDIUM_EN		(IMG_UINT32_C(0x2))
#define RGXFWIF_CTXSWITCH_PROFILE_SLOW_EN		(IMG_UINT32_C(0x3))
#define RGXFWIF_CTXSWITCH_PROFILE_NODELAY_EN	(IMG_UINT32_C(0x4))

#define RGXFWIF_CDM_ARBITRATION_TASK_DEMAND_EN	(IMG_UINT32_C(0x1))
#define RGXFWIF_CDM_ARBITRATION_ROUND_ROBIN_EN	(IMG_UINT32_C(0x2))

#define RGXFWIF_ISP_SCHEDMODE_VER1_IPP			(IMG_UINT32_C(0x1))
#define RGXFWIF_ISP_SCHEDMODE_VER2_ISP			(IMG_UINT32_C(0x2))
/*!
 ******************************************************************************
 * RGX firmware Init Config Data
 *****************************************************************************/

/* Flag definitions affecting the firmware globally */
#define RGXFWIF_INICFG_CTXSWITCH_MODE_RAND				(IMG_UINT32_C(0x1) << 0)	/*!< Randomise context switch requests */
#define RGXFWIF_INICFG_CTXSWITCH_SRESET_EN				(IMG_UINT32_C(0x1) << 1)
#define RGXFWIF_INICFG_HWPERF_EN						(IMG_UINT32_C(0x1) << 2)
#define RGXFWIF_INICFG_DM_KILL_MODE_RAND_EN				(IMG_UINT32_C(0x1) << 3)	/*!< Randomise DM-killing requests */
#define RGXFWIF_INICFG_POW_RASCALDUST					(IMG_UINT32_C(0x1) << 4)
#define RGXFWIF_INICFG_SPU_CLOCK_GATE					(IMG_UINT32_C(0x1) << 5)
#define RGXFWIF_INICFG_FBCDC_V3_1_EN					(IMG_UINT32_C(0x1) << 6)
#define RGXFWIF_INICFG_CHECK_MLIST_EN					(IMG_UINT32_C(0x1) << 7)
#define RGXFWIF_INICFG_DISABLE_CLKGATING_EN				(IMG_UINT32_C(0x1) << 8)
#define RGXFWIF_INICFG_TRY_OVERLAPPING_DM_PIPELINES_EN	(IMG_UINT32_C(0x1) << 9)
#define RGXFWIF_INICFG_DM_PIPELINE_ROADBLOCKS_EN		(IMG_UINT32_C(0x1) << 10)
/* 11 unused */
#define RGXFWIF_INICFG_REGCONFIG_EN						(IMG_UINT32_C(0x1) << 12)
#define RGXFWIF_INICFG_ASSERT_ON_OUTOFMEMORY			(IMG_UINT32_C(0x1) << 13)
#define RGXFWIF_INICFG_HWP_DISABLE_FILTER				(IMG_UINT32_C(0x1) << 14)
/* 15 unused */
#define RGXFWIF_INICFG_CTXSWITCH_PROFILE_SHIFT			(16)
#define RGXFWIF_INICFG_CTXSWITCH_PROFILE_FAST			(RGXFWIF_CTXSWITCH_PROFILE_FAST_EN << RGXFWIF_INICFG_CTXSWITCH_PROFILE_SHIFT)
#define RGXFWIF_INICFG_CTXSWITCH_PROFILE_MEDIUM			(RGXFWIF_CTXSWITCH_PROFILE_MEDIUM_EN << RGXFWIF_INICFG_CTXSWITCH_PROFILE_SHIFT)
#define RGXFWIF_INICFG_CTXSWITCH_PROFILE_SLOW			(RGXFWIF_CTXSWITCH_PROFILE_SLOW_EN << RGXFWIF_INICFG_CTXSWITCH_PROFILE_SHIFT)
#define RGXFWIF_INICFG_CTXSWITCH_PROFILE_NODELAY		(RGXFWIF_CTXSWITCH_PROFILE_NODELAY_EN << RGXFWIF_INICFG_CTXSWITCH_PROFILE_SHIFT)
#define RGXFWIF_INICFG_CTXSWITCH_PROFILE_MASK			(IMG_UINT32_C(0x7) << RGXFWIF_INICFG_CTXSWITCH_PROFILE_SHIFT)
#define RGXFWIF_INICFG_DISABLE_DM_OVERLAP				(IMG_UINT32_C(0x1) << 19)
#define RGXFWIF_INICFG_ASSERT_ON_HWR_TRIGGER			(IMG_UINT32_C(0x1) << 20)
#define RGXFWIF_INICFG_FULL_GPU_CPU_COHERENCY			(IMG_UINT32_C(0x1) << 21)
#define RGXFWIF_INICFG_VALIDATE_IRQ						(IMG_UINT32_C(0x1) << 22)
#define RGXFWIF_INICFG_DISABLE_PDP_EN					(IMG_UINT32_C(0x1) << 23)
#define RGXFWIF_INICFG_SPU_POWER_STATE_MASK_CHANGE_EN	(IMG_UINT32_C(0x1) << 24)
#define RGXFWIF_INICFG_WORKEST							(IMG_UINT32_C(0x1) << 25)
#define RGXFWIF_INICFG_PDVFS							(IMG_UINT32_C(0x1) << 26)
#define RGXFWIF_INICFG_CDM_ARBITRATION_SHIFT			(27)
#define RGXFWIF_INICFG_CDM_ARBITRATION_TASK_DEMAND		(RGXFWIF_CDM_ARBITRATION_TASK_DEMAND_EN << RGXFWIF_INICFG_CDM_ARBITRATION_SHIFT)
#define RGXFWIF_INICFG_CDM_ARBITRATION_ROUND_ROBIN		(RGXFWIF_CDM_ARBITRATION_ROUND_ROBIN_EN << RGXFWIF_INICFG_CDM_ARBITRATION_SHIFT)
#define RGXFWIF_INICFG_CDM_ARBITRATION_MASK				(IMG_UINT32_C(0x3) << RGXFWIF_INICFG_CDM_ARBITRATION_SHIFT)
#define RGXFWIF_INICFG_ISPSCHEDMODE_SHIFT				(29)
#define RGXFWIF_INICFG_ISPSCHEDMODE_NONE				(0)
#define RGXFWIF_INICFG_ISPSCHEDMODE_VER1_IPP			(RGXFWIF_ISP_SCHEDMODE_VER1_IPP << RGXFWIF_INICFG_ISPSCHEDMODE_SHIFT)
#define RGXFWIF_INICFG_ISPSCHEDMODE_VER2_ISP			(RGXFWIF_ISP_SCHEDMODE_VER2_ISP << RGXFWIF_INICFG_ISPSCHEDMODE_SHIFT)
#define RGXFWIF_INICFG_ISPSCHEDMODE_MASK				(RGXFWIF_INICFG_ISPSCHEDMODE_VER1_IPP |\
                                                         RGXFWIF_INICFG_ISPSCHEDMODE_VER2_ISP)
#define RGXFWIF_INICFG_VALIDATE_SOCUSC_TIMER			(IMG_UINT32_C(0x1) << 31)
#define RGXFWIF_INICFG_ALL								(0xFFFFF7FFU)

/* Extended Flag definitions affecting the firmware globally */
#define RGXFWIF_INICFG_EXT_ALL							(0x0U)

#define RGXFWIF_INICFG_SYS_CTXSWITCH_CLRMSK				~(RGXFWIF_INICFG_CTXSWITCH_MODE_RAND | \
														  RGXFWIF_INICFG_CTXSWITCH_SRESET_EN)

/* Flag definitions affecting only workloads submitted by a particular OS */

/*!
 * @AddToGroup ContextSwitching
 * @{
 * @Name Per-OS DM context switch configuration flags
 * @{
 */
#define RGXFWIF_INICFG_OS_CTXSWITCH_TDM_EN				(IMG_UINT32_C(0x1) << 0) /*!< Enables TDM context switch */
#define RGXFWIF_INICFG_OS_CTXSWITCH_GEOM_EN				(IMG_UINT32_C(0x1) << 1) /*!< Enables GEOM-TA and GEOM-SHG context switch */
#define RGXFWIF_INICFG_OS_CTXSWITCH_3D_EN				(IMG_UINT32_C(0x1) << 2) /*!< Enables FRAG DM context switch */
#define RGXFWIF_INICFG_OS_CTXSWITCH_CDM_EN				(IMG_UINT32_C(0x1) << 3) /*!< Enables CDM context switch */
#define RGXFWIF_INICFG_OS_CTXSWITCH_RDM_EN				(IMG_UINT32_C(0x1) << 4)

#define RGXFWIF_INICFG_OS_LOW_PRIO_CS_TDM				(IMG_UINT32_C(0x1) << 5)
#define RGXFWIF_INICFG_OS_LOW_PRIO_CS_GEOM				(IMG_UINT32_C(0x1) << 6)
#define RGXFWIF_INICFG_OS_LOW_PRIO_CS_3D				(IMG_UINT32_C(0x1) << 7)
#define RGXFWIF_INICFG_OS_LOW_PRIO_CS_CDM				(IMG_UINT32_C(0x1) << 8)
#define RGXFWIF_INICFG_OS_LOW_PRIO_CS_RDM				(IMG_UINT32_C(0x1) << 9)

#define RGXFWIF_INICFG_OS_ALL							(0x3FFU)

#define RGXFWIF_INICFG_OS_CTXSWITCH_DM_ALL				(RGXFWIF_INICFG_OS_CTXSWITCH_GEOM_EN | \
														 RGXFWIF_INICFG_OS_CTXSWITCH_3D_EN | \
														 RGXFWIF_INICFG_OS_CTXSWITCH_CDM_EN | \
														 RGXFWIF_INICFG_OS_CTXSWITCH_TDM_EN | \
														 RGXFWIF_INICFG_OS_CTXSWITCH_RDM_EN)

#define RGXFWIF_INICFG_OS_CTXSWITCH_CLRMSK				~(RGXFWIF_INICFG_OS_CTXSWITCH_DM_ALL)

/*!
 * @} End of Per-OS Context switch configuration flags
 * @} End of AddToGroup ContextSwitching
 */

#define RGXFWIF_FILTCFG_TRUNCATE_HALF					(IMG_UINT32_C(0x1) << 3)
#define RGXFWIF_FILTCFG_TRUNCATE_INT					(IMG_UINT32_C(0x1) << 2)
#define RGXFWIF_FILTCFG_NEW_FILTER_MODE					(IMG_UINT32_C(0x1) << 1)

typedef IMG_UINT32 RGX_ACTIVEPM_CONF;
#define RGX_ACTIVEPM_FORCE_OFF	0U
#define RGX_ACTIVEPM_FORCE_ON	1U
#define RGX_ACTIVEPM_DEFAULT	2U

typedef IMG_UINT32 RGX_RD_POWER_ISLAND_CONF;
#define RGX_RD_POWER_ISLAND_FORCE_OFF	0U
#define RGX_RD_POWER_ISLAND_FORCE_ON	1U
#define RGX_RD_POWER_ISLAND_DEFAULT		2U

typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_SIGBUFFER;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_TRACEBUF;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_SYSDATA;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_OSDATA;
#if defined(SUPPORT_TBI_INTERFACE)
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_TBIBUF;
#endif
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_HWPERFBUF;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_HWRINFOBUF;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_RUNTIME_CFG;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_GPU_UTIL_FW;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_REG_CFG;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_HWPERF_CTL;
typedef RGXFWIF_DEV_VIRTADDR  PRGX_HWPERF_CONFIG_CNTBLK;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_CCB_RTN_SLOTS;

typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_FWCOMMONCONTEXT;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_ZSBUFFER;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_CORE_CLK_RATE;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_COUNTERBUFFER;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_FIRMWAREGCOVBUFFER;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_CCB_CTL;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_CCB;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_HWRTDATA;
typedef RGXFWIF_DEV_VIRTADDR  PRGXFWIF_TIMESTAMP_ADDR;

#if defined(SUPPORT_FW_HOST_SIDE_RECOVERY)
/*!
 * @Brief Buffer to store KM active client contexts
 */
typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT	psContext;			/*!< address of the firmware context */
} RGXFWIF_ACTIVE_CONTEXT_BUF_DATA;
#endif

/*!
 * This number is used to represent an invalid page catalogue physical address
 */
#define RGXFWIF_INVALID_PC_PHYADDR 0xFFFFFFFFFFFFFFFFLLU

/*!
 * This number is used to represent an unallocated set of page catalog base registers
 */
#define RGXFW_BIF_INVALID_PCSET 0xFFFFFFFFU

/*!
 * This number is used to represent an invalid OS ID for the purpose of tracking PC set ownership
 */
#define RGXFW_BIF_INVALID_OSID 0xFFFFFFFFU

#define RGXFWIF_FWMEMCONTEXT_FLAGS_USED_TESS		(0x00000001U)

/*!
 * Firmware memory context.
 */
typedef struct
{
	IMG_DEV_PHYADDR			RGXFW_ALIGN sPCDevPAddr;	/*!< device physical address of context's page catalogue */
	IMG_UINT32				uiPageCatBaseRegSet;		/*!< index of the associated set of page catalog base registers (RGXFW_BIF_INVALID_PCSET == unallocated) */
	IMG_UINT32				uiBreakpointAddr; /*!< breakpoint address */
	IMG_UINT32				uiBPHandlerAddr; /*!< breakpoint handler address */
	IMG_UINT32				uiBreakpointCtl; /*!< DM and enable control for BP */
	IMG_UINT64				RGXFW_ALIGN ui64FBCStateIDMask;	/*!< FBCDC state descriptor IDs (non-zero means defer on mem context activation) */
	IMG_UINT64				RGXFW_ALIGN ui64SpillAddr;
	IMG_UINT32				ui32FwMemCtxFlags; /*!< Compatibility and other flags */

#if defined(SUPPORT_CUSTOM_OSID_EMISSION)
	IMG_UINT32              ui32OSid;
	IMG_BOOL                bOSidAxiProt;
#endif

} UNCACHED_ALIGN RGXFWIF_FWMEMCONTEXT;

/*!
 * FW context state flags
 */
#define RGXFWIF_CONTEXT_FLAGS_NEED_RESUME			(0x00000001U)
#define RGXFWIF_CONTEXT_FLAGS_TDM_HEADER_STALE		(0x00000002U)
#define RGXFWIF_CONTEXT_FLAGS_LAST_KICK_SECURE		(0x00000200U)

/*!
 * @InGroup ContextSwitching
 * @Brief Firmware GEOM/TA context suspend state (per GEOM core)
 */
typedef struct
{
	IMG_UINT64	RGXFW_ALIGN uTAReg_DCE_CMD0;
	IMG_UINT32				uTAReg_DCE_CMD1;
	IMG_UINT32				uTAReg_DCE_WRITE;
	IMG_UINT64	RGXFW_ALIGN uTAReg_DCE_DRAW0;
	IMG_UINT64	RGXFW_ALIGN uTAReg_DCE_DRAW1;
	IMG_UINT64				ui64EnabledUnitsMask;
	IMG_UINT32				uTAReg_GTA_SO_PRIM[4];
	IMG_UINT16	ui16TACurrentIdx;
} UNCACHED_ALIGN RGXFWIF_TACTX_STATE_PER_GEOM;

/*!
 * @InGroup ContextSwitching
 * @Brief Firmware GEOM/TA context suspend states for all GEOM cores
 */
typedef struct
{
	/*! FW-accessible TA state which must be written out to memory on context store */
	RGXFWIF_TACTX_STATE_PER_GEOM asGeomCore[RGX_NUM_GEOM_CORES];
} UNCACHED_ALIGN RGXFWIF_TACTX_STATE;

/* The following defines need to be auto generated using the HW defines
 * rather than hard coding it */
#define RGXFWIF_ISP_PIPE_COUNT_MAX		(48)
#define RGXFWIF_PIPE_COUNT_PER_ISP		(2)
#define RGXFWIF_IPP_RESUME_REG_COUNT	(1)

#if !defined(__KERNEL__)
#define RGXFWIF_ISP_COUNT		(RGX_FEATURE_NUM_SPU * RGX_FEATURE_NUM_ISP_PER_SPU)
#define RGXFWIF_ISP_PIPE_COUNT	(RGXFWIF_ISP_COUNT * RGXFWIF_PIPE_COUNT_PER_ISP)
#if RGXFWIF_ISP_PIPE_COUNT > RGXFWIF_ISP_PIPE_COUNT_MAX
#error RGXFWIF_ISP_PIPE_COUNT should not be greater than RGXFWIF_ISP_PIPE_COUNT_MAX
#endif
#endif /* !defined(__KERNEL__) */

/*!
 * @InGroup ContextSwitching
 * @Brief Firmware FRAG/3D context suspend state
 */
typedef struct
{
#if defined(PM_INTERACTIVE_MODE)
	IMG_UINT32	RGXFW_ALIGN u3DReg_PM_DEALLOCATED_MASK_STATUS;	/*!< Managed by PM HW in the non-interactive mode */
#endif
	IMG_UINT32	ui32CtxStateFlags;	/*!< Compatibility and other flags */

	/* FW-accessible ISP state which must be written out to memory on context store */
	/* au3DReg_ISP_STORE should be the last element of the structure
	 * as this is an array whose size is determined at runtime
	 * after detecting the RGX core */
	IMG_UINT64	RGXFW_ALIGN au3DReg_ISP_STORE[]; /*!< ISP state (per-pipe) */
} UNCACHED_ALIGN RGXFWIF_3DCTX_STATE;

#define RGXFWIF_CTX_USING_BUFFER_A		(0)
#define RGXFWIF_CTX_USING_BUFFER_B		(1U)

typedef struct
{
	IMG_UINT32  ui32CtxStateFlags;		/*!< Target buffer and other flags */
	IMG_UINT64  ui64EnabledUnitsMask;
} RGXFWIF_COMPUTECTX_STATE;

#define RGXFWIF_CONTEXT_COMPAT_FLAGS_STATS_PENDING      (1U << 0)
#define RGXFWIF_CONTEXT_COMPAT_FLAGS_HAS_DEFER_COUNT    (1U << 1)
#define RGXFWIF_CONTEXT_COMPAT_FLAGS_HAS_CANCEL_RANGES  (1U << 2)

typedef IMG_UINT64 RGXFWIF_TRP_CHECKSUM_2D[RGX_TRP_MAX_NUM_CORES][2];
typedef IMG_UINT64 RGXFWIF_TRP_CHECKSUM_3D[RGX_TRP_MAX_NUM_CORES][4];
typedef IMG_UINT64 RGXFWIF_TRP_CHECKSUM_GEOM[RGX_TRP_MAX_NUM_CORES][4];

typedef struct
{
	IMG_UINT32	ui32ExtJobRefToDisableZSStore;
	IMG_BOOL	bDisableZStore;
	IMG_BOOL	bDisableSStore;
} RGXFWIF_DISABLE_ZSSTORE;

#define MAX_ZSSTORE_DISABLE 8

/*!
 * @InGroup WorkloadContexts
 * @Brief Firmware render context.
 */
typedef struct
{
	RGXFWIF_FWCOMMONCONTEXT	sTAContext;				/*!< Firmware context for the TA */
	RGXFWIF_FWCOMMONCONTEXT	s3DContext;				/*!< Firmware context for the 3D */

	RGXFWIF_STATIC_RENDERCONTEXT_STATE sStaticRenderContextState;

	RGXFWIF_DISABLE_ZSSTORE sDisableZSStoreQueue[MAX_ZSSTORE_DISABLE];

	IMG_UINT32			ui32ZSStoreQueueCount;
	IMG_UINT32			ui32WriteOffsetOfDisableZSStore;

	IMG_UINT32			ui32WorkEstCCBSubmitted; /*!< Number of commands submitted to the WorkEst FW CCB */

	IMG_UINT32			ui32FwRenderCtxFlags; /*!< Compatibility and other flags */

#if defined(SUPPORT_TRP)
	RGXFWIF_TRP_CHECKSUM_3D		aui64TRPChecksums3D;	/*!< Used by Firmware to store checksums during 3D WRR */
	RGXFWIF_TRP_CHECKSUM_GEOM	aui64TRPChecksumsGeom;	/*!< Used by Firmware to store checksums during TA WRR */
	RGXFWIF_DM			eTRPGeomCoreAffinity; /* !< Represent the DM affinity for pending 2nd TRP pass of GEOM otherwise points RGXFWIF_DM_MAX. */
#endif
} UNCACHED_ALIGN RGXFWIF_FWRENDERCONTEXT;

/*!
	Firmware compute context.
*/
typedef struct
{
	RGXFWIF_FWCOMMONCONTEXT sCDMContext;				/*!< Firmware context for the CDM */

	RGXFWIF_STATIC_COMPUTECONTEXT_STATE sStaticComputeContextState;

	IMG_UINT32			ui32WorkEstCCBSubmitted; /*!< Number of commands submitted to the WorkEst FW CCB */

	IMG_UINT32 ui32ComputeCtxFlags; /*!< Compatibility and other flags */

	IMG_UINT32		ui32WGPState;
	IMG_UINT32		aui32WGPChecksum[RGX_WGP_MAX_NUM_CORES];
} UNCACHED_ALIGN RGXFWIF_FWCOMPUTECONTEXT;

/*!
	Firmware ray context.
*/
typedef struct
{
	RGXFWIF_FWCOMMONCONTEXT sRDMContext;				/*!< Firmware context for the RDM */
	RGXFWIF_STATIC_RAYCONTEXT_STATE sStaticRayContextState;
	IMG_UINT32			ui32WorkEstCCBSubmitted; /*!< Number of commands submitted to the WorkEst FW CCB */

} UNCACHED_ALIGN RGXFWIF_FWRAYCONTEXT;

/*!
 * @InGroup WorkloadContexts
 * @Brief Firmware TDM context.
 */
typedef struct
{
	RGXFWIF_FWCOMMONCONTEXT	sTDMContext;				/*!< Firmware context for the TDM */

	IMG_UINT32			ui32WorkEstCCBSubmitted; /*!< Number of commands submitted to the WorkEst FW CCB */
#if defined(SUPPORT_TRP)
	IMG_UINT32				ui32TRPState;		/*!< Used by Firmware to track current state of a protected kick */
	RGXFWIF_TRP_CHECKSUM_2D	RGXFW_ALIGN	aui64TRPChecksums2D; /*!< Used by Firmware to store checksums during TDM WRR */
#endif

} UNCACHED_ALIGN RGXFWIF_FWTDMCONTEXT;

/*!
 ******************************************************************************
 * Defines for CMD_TYPE corruption detection and forward compatibility check
 *****************************************************************************/

/* CMD_TYPE 32bit contains:
 * 31:16	Reserved for magic value to detect corruption (16 bits)
 * 15		Reserved for RGX_CCB_TYPE_TASK (1 bit)
 * 14:0		Bits available for CMD_TYPEs (15 bits) */


/* Magic value to detect corruption */
#define RGX_CMD_MAGIC_DWORD			IMG_UINT32_C(0x2ABC)
#define RGX_CMD_MAGIC_DWORD_MASK	(0xFFFF0000U)
#define RGX_CMD_MAGIC_DWORD_SHIFT	(16U)
#define RGX_CMD_MAGIC_DWORD_SHIFTED	(RGX_CMD_MAGIC_DWORD << RGX_CMD_MAGIC_DWORD_SHIFT)

/*!
 * @InGroup KCCBTypes ClientCCBTypes
 * @Brief Generic CCB control structure
 */
typedef struct
{
	volatile IMG_UINT32		ui32WriteOffset;		/*!< write offset into array of commands (MUST be aligned to 16 bytes!) */
	volatile IMG_UINT32		ui32ReadOffset;			/*!< read offset into array of commands */
	IMG_UINT32				ui32WrapMask;			/*!< Offset wrapping mask (Total capacity of the CCB - 1) */
} UNCACHED_ALIGN RGXFWIF_CCB_CTL;

/*!
 * @Defgroup KCCBTypes Kernel CCB data interface
 * @Brief Types grouping data structures and defines used in realising the KCCB functionality
 * @{
 */

#define RGXFWIF_MMUCACHEDATA_FLAGS_PT      (0x1U) /* MMU_CTRL_INVAL_PT_EN */
#define RGXFWIF_MMUCACHEDATA_FLAGS_PD      (0x2U) /* MMU_CTRL_INVAL_PD_EN */
#define RGXFWIF_MMUCACHEDATA_FLAGS_PC      (0x4U) /* MMU_CTRL_INVAL_PC_EN */
#define RGXFWIF_MMUCACHEDATA_FLAGS_CTX_ALL (0x800U) /* MMU_CTRL_INVAL_ALL_CONTEXTS_EN */
#define RGXFWIF_MMUCACHEDATA_FLAGS_TLB     (0x0) /* not used */

#define RGXFWIF_MMUCACHEDATA_FLAGS_INTERRUPT (0x4000000U) /* indicates FW should interrupt the host */

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_MMUCACHE type command
 */
typedef struct
{
	IMG_UINT32            ui32CacheFlags;
	RGXFWIF_DEV_VIRTADDR  sMMUCacheSync;
	IMG_UINT32            ui32MMUCacheSyncUpdateValue;
} RGXFWIF_MMUCACHEDATA;

#define RGXFWIF_BPDATA_FLAGS_ENABLE (1U << 0)
#define RGXFWIF_BPDATA_FLAGS_WRITE  (1U << 1)
#define RGXFWIF_BPDATA_FLAGS_CTL    (1U << 2)
#define RGXFWIF_BPDATA_FLAGS_REGS   (1U << 3)

typedef struct
{
	PRGXFWIF_FWMEMCONTEXT		psFWMemContext;		/*!< Memory context */
	IMG_UINT32					ui32BPAddr;			/*!< Breakpoint address */
	IMG_UINT32					ui32HandlerAddr;	/*!< Breakpoint handler */
	IMG_UINT32					ui32BPDM;			/*!< Breakpoint control */
	IMG_UINT32					ui32BPDataFlags;
	IMG_UINT32					ui32TempRegs;		/*!< Number of temporary registers to overallocate */
	IMG_UINT32					ui32SharedRegs;		/*!< Number of shared registers to overallocate */
	IMG_UINT64 RGXFW_ALIGN		ui64SpillAddr;
	RGXFWIF_DM					eDM;				/*!< DM associated with the breakpoint */
} RGXFWIF_BPDATA;

#define RGXFWIF_KCCB_CMD_KICK_DATA_MAX_NUM_CLEANUP_CTLS (RGXFWIF_PRBUFFER_MAXSUPPORTED + 1U) /* +1 is RTDATASET cleanup */

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_KICK type command
 */
typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT	psContext;			/*!< address of the firmware context */
	IMG_UINT32					ui32CWoffUpdate;	/*!< Client CCB woff update */
	IMG_UINT32					ui32CWrapMaskUpdate; /*!< Client CCB wrap mask update after CCCB growth */
	IMG_UINT32					ui32NumCleanupCtl;		/*!< number of CleanupCtl pointers attached */
	PRGXFWIF_CLEANUP_CTL		apsCleanupCtl[RGXFWIF_KCCB_CMD_KICK_DATA_MAX_NUM_CLEANUP_CTLS]; /*!< CleanupCtl structures associated with command */
#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	IMG_UINT32					ui32WorkEstCmdHeaderOffset; /*!< offset to the CmdHeader which houses the workload estimation kick data. */
#endif
} RGXFWIF_KCCB_CMD_KICK_DATA;

/*!
 * @Brief Command data for @Ref RGXFWIF_KCCB_CMD_COMBINED_TA_3D_KICK type command
 */
typedef struct
{
	RGXFWIF_KCCB_CMD_KICK_DATA	sTACmdKickData; /*!< GEOM DM kick command data */
	RGXFWIF_KCCB_CMD_KICK_DATA	s3DCmdKickData; /*!< FRAG DM kick command data */
} RGXFWIF_KCCB_CMD_COMBINED_TA_3D_KICK_DATA;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_FORCE_UPDATE type command
 */
typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT	psContext;			/*!< address of the firmware context */
	IMG_UINT32					ui32CCBFenceOffset;	/*!< Client CCB fence offset */
} RGXFWIF_KCCB_CMD_FORCE_UPDATE_DATA;

typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT	psContext;
	RGXFWIF_DISABLE_ZSSTORE		sDisableZSStore;
} RGXFWIF_KCCB_CMD_DISABLE_ZSSTORE_DATA;

/*!
 * @Brief Resource types supported by \ref RGXFWIF_KCCB_CMD_CLEANUP type command
 */
typedef enum
{
	RGXFWIF_CLEANUP_FWCOMMONCONTEXT,		/*!< FW common context cleanup */
	RGXFWIF_CLEANUP_HWRTDATA,				/*!< FW HW RT data cleanup */
	RGXFWIF_CLEANUP_FREELIST,				/*!< FW freelist cleanup */
	RGXFWIF_CLEANUP_ZSBUFFER,				/*!< FW ZS Buffer cleanup */
} RGXFWIF_CLEANUP_TYPE;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_CLEANUP type command
 */
typedef struct
{
	RGXFWIF_CLEANUP_TYPE			eCleanupType;			/*!< Cleanup type */
	union {
		PRGXFWIF_FWCOMMONCONTEXT	psContext;				/*!< FW common context to cleanup */
		PRGXFWIF_HWRTDATA			psHWRTData;				/*!< HW RT to cleanup */
		PRGXFWIF_FREELIST			psFreelist;				/*!< Freelist to cleanup */
		PRGXFWIF_ZSBUFFER			psZSBuffer;				/*!< ZS Buffer to cleanup */
	} uCleanupData;
} RGXFWIF_CLEANUP_REQUEST;

/*!
 * @Brief Type of power requests supported in \ref RGXFWIF_KCCB_CMD_POW type command
 */
typedef enum
{
	RGXFWIF_POW_OFF_REQ = 1,           /*!< GPU power-off request */
	RGXFWIF_POW_FORCED_IDLE_REQ,       /*!< Force-idle related request */
	RGXFWIF_POW_NUM_UNITS_CHANGE,      /*!< Request to change default powered scalable units */
	RGXFWIF_POW_APM_LATENCY_CHANGE     /*!< Request to change the APM latency period */
} RGXFWIF_POWER_TYPE;

/*!
 * @Brief Supported force-idle related requests with \ref RGXFWIF_POW_FORCED_IDLE_REQ type request
 */
typedef enum
{
	RGXFWIF_POWER_FORCE_IDLE = 1,      /*!< Request to force-idle GPU */
	RGXFWIF_POWER_CANCEL_FORCED_IDLE,  /*!< Request to cancel a previously successful force-idle transition */
	RGXFWIF_POWER_HOST_TIMEOUT,        /*!< Notification that host timed-out waiting for force-idle state */
} RGXFWIF_POWER_FORCE_IDLE_TYPE;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_POW type command
 */
typedef struct
{
	RGXFWIF_POWER_TYPE					ePowType;					/*!< Type of power request */
	union
	{
		struct
		{
			IMG_UINT32					ui32PowUnits;	/*!< New power units state mask */
			IMG_UINT32					ui32RACUnits;	/*!< New RAC state mask */
		};
		IMG_BOOL						bForced;				/*!< If the operation is mandatory */
		RGXFWIF_POWER_FORCE_IDLE_TYPE	ePowRequestType;		/*!< Type of Request. Consolidating Force Idle, Cancel Forced Idle, Host Timeout */
	} uPowerReqData;
} RGXFWIF_POWER_REQUEST;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_SLCFLUSHINVAL type command
 */
typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT psContext; /*!< Context to fence on (only useful when bDMContext == TRUE) */
	IMG_BOOL    bInval;                 /*!< Invalidate the cache as well as flushing */
	IMG_BOOL    bDMContext;             /*!< The data to flush/invalidate belongs to a specific DM context */
	IMG_UINT64	RGXFW_ALIGN ui64Address;	/*!< Optional address of range (only useful when bDMContext == FALSE) */
	IMG_UINT64	RGXFW_ALIGN ui64Size;		/*!< Optional size of range (only useful when bDMContext == FALSE) */
} RGXFWIF_SLCFLUSHINVALDATA;

typedef enum
{
	RGXFWIF_HWPERF_CTRL_TOGGLE = 0,
	RGXFWIF_HWPERF_CTRL_SET    = 1,
	RGXFWIF_HWPERF_CTRL_EMIT_FEATURES_EV = 2
} RGXFWIF_HWPERF_UPDATE_CONFIG;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_HWPERF_UPDATE_CONFIG type command
 */
typedef struct
{
	RGXFWIF_HWPERF_UPDATE_CONFIG eOpCode; /*!< Control operation code */
	IMG_UINT64	RGXFW_ALIGN	ui64Mask;   /*!< Mask of events to toggle */
} RGXFWIF_HWPERF_CTRL;

typedef enum
{
	RGXFWIF_HWPERF_CNTR_NOOP    = 0,   /* No-Op */
	RGXFWIF_HWPERF_CNTR_ENABLE  = 1,   /* Enable Counters */
	RGXFWIF_HWPERF_CNTR_DISABLE = 2    /* Disable Counters */
} RGXFWIF_HWPERF_CNTR_CONFIG;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_HWPERF_CONFIG_BLKS type command
 */
typedef struct
{
	IMG_UINT32                ui32CtrlWord;
	IMG_UINT32                ui32NumBlocks;    /*!< Number of RGX_HWPERF_CONFIG_CNTBLK in the array */
	PRGX_HWPERF_CONFIG_CNTBLK sBlockConfigs;    /*!< Address of the RGX_HWPERF_CONFIG_CNTBLK array */
} RGXFWIF_HWPERF_CONFIG_ENABLE_BLKS;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_CORECLKSPEEDCHANGE type command
 */
typedef struct
{
	IMG_UINT32	ui32NewClockSpeed;			/*!< New clock speed */
} RGXFWIF_CORECLKSPEEDCHANGE_DATA;

#define RGXFWIF_HWPERF_CTRL_BLKS_MAX	16U

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_HWPERF_CTRL_BLKS type command
 */
typedef struct
{
	bool		bEnable;
	IMG_UINT32	ui32NumBlocks;                              /*!< Number of block IDs in the array */
	IMG_UINT16	aeBlockIDs[RGXFWIF_HWPERF_CTRL_BLKS_MAX];   /*!< Array of RGX_HWPERF_CNTBLK_ID values */
} RGXFWIF_HWPERF_CTRL_BLKS;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_ZSBUFFER_BACKING_UPDATE & \ref RGXFWIF_KCCB_CMD_ZSBUFFER_UNBACKING_UPDATE type commands
 */
typedef struct
{
	RGXFWIF_DEV_VIRTADDR	sZSBufferFWDevVAddr;				/*!< ZS-Buffer FW address */
	IMG_BOOL				bDone;								/*!< action backing/unbacking succeeded */
} RGXFWIF_ZSBUFFER_BACKING_DATA;

#if defined(SUPPORT_VALIDATION)
typedef struct
{
	IMG_UINT32 ui32RegWidth;
	IMG_BOOL   bWriteOp;
	IMG_UINT32 ui32RegAddr;
	IMG_UINT64 RGXFW_ALIGN ui64RegVal;
} RGXFWIF_RGXREG_DATA;

typedef struct
{
	IMG_UINT64 ui64BaseAddress;
	PRGXFWIF_FWCOMMONCONTEXT psContext;
	IMG_UINT32 ui32Size;
} RGXFWIF_GPUMAP_DATA;
#endif

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_FREELIST_GROW_UPDATE type command
 */
typedef struct
{
	RGXFWIF_DEV_VIRTADDR	sFreeListFWDevVAddr;				/*!< Freelist FW address */
	IMG_UINT32				ui32DeltaPages;						/*!< Amount of the Freelist change */
	IMG_UINT32				ui32NewPages;						/*!< New amount of pages on the freelist (including ready pages) */
	IMG_UINT32              ui32ReadyPages;                     /*!< Number of ready pages to be held in reserve until OOM */
} RGXFWIF_FREELIST_GS_DATA;

/* Max freelists must include freelists loaded (for all kick IDs) and freelists being setup. */
#define RGXFWIF_MAX_FREELISTS_TO_RECONSTRUCT         (MAX_HW_TA3DCONTEXTS * RGXFW_MAX_FREELISTS * 3U)
#define RGXFWIF_FREELISTS_RECONSTRUCTION_FAILED_FLAG 0x80000000U

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_FREELISTS_RECONSTRUCTION_UPDATE type command
 */
typedef struct
{
	IMG_UINT32			ui32FreelistsCount;
	IMG_UINT32			aui32FreelistIDs[RGXFWIF_MAX_FREELISTS_TO_RECONSTRUCT];
} RGXFWIF_FREELISTS_RECONSTRUCTION_DATA;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_NOTIFY_WRITE_OFFSET_UPDATE type command
 */
typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT  psContext; /*!< Context to that may need to be resumed following write offset update */
} UNCACHED_ALIGN RGXFWIF_WRITE_OFFSET_UPDATE_DATA;

/*!
 ******************************************************************************
 * Proactive DVFS Structures
 *****************************************************************************/
#define NUM_OPP_VALUES 16

typedef struct
{
	IMG_UINT32			ui32Volt; /* V  */
	IMG_UINT32			ui32Freq; /* Hz */
} UNCACHED_ALIGN PDVFS_OPP;

typedef struct
{
	PDVFS_OPP		asOPPValues[NUM_OPP_VALUES];
#if defined(DEBUG)
	IMG_UINT32		ui32MinOPPPoint;
#endif
	IMG_UINT32		ui32MaxOPPPoint;
} UNCACHED_ALIGN RGXFWIF_PDVFS_OPP;

typedef struct
{
	IMG_UINT32 ui32MaxOPPPoint;
} UNCACHED_ALIGN RGXFWIF_PDVFS_MAX_FREQ_DATA;

typedef struct
{
	IMG_UINT32 ui32MinOPPPoint;
} UNCACHED_ALIGN RGXFWIF_PDVFS_MIN_FREQ_DATA;

/*!
 ******************************************************************************
 * Register configuration structures
 *****************************************************************************/

#define RGXFWIF_REG_CFG_MAX_SIZE 512

typedef enum
{
	RGXFWIF_REGCFG_CMD_ADD				= 101,
	RGXFWIF_REGCFG_CMD_CLEAR			= 102,
	RGXFWIF_REGCFG_CMD_ENABLE			= 103,
	RGXFWIF_REGCFG_CMD_DISABLE			= 104
} RGXFWIF_REGDATA_CMD_TYPE;

typedef enum
{
	RGXFWIF_REG_CFG_TYPE_PWR_ON=0,      /* Sidekick power event */
	RGXFWIF_REG_CFG_TYPE_DUST_CHANGE,   /* Rascal / dust power event */
	RGXFWIF_REG_CFG_TYPE_TA,            /* TA kick */
	RGXFWIF_REG_CFG_TYPE_3D,            /* 3D kick */
	RGXFWIF_REG_CFG_TYPE_CDM,           /* Compute kick */
	RGXFWIF_REG_CFG_TYPE_TDM,           /* TDM kick */
	RGXFWIF_REG_CFG_TYPE_ALL            /* Applies to all types. Keep as last element */
} RGXFWIF_REG_CFG_TYPE;

typedef struct
{
	IMG_UINT64		ui64Addr;
	IMG_UINT64		ui64Mask;
	IMG_UINT64		ui64Value;
} RGXFWIF_REG_CFG_REC;

typedef struct
{
	RGXFWIF_REGDATA_CMD_TYPE         eCmdType;
	RGXFWIF_REG_CFG_TYPE             eRegConfigType;
	RGXFWIF_REG_CFG_REC RGXFW_ALIGN  sRegConfig;

} RGXFWIF_REGCONFIG_DATA;

typedef struct
{
	/**
	 * PDump WRW command write granularity is 32 bits.
	 * Add padding to ensure array size is 32 bit granular.
	 */
	IMG_UINT8           RGXFW_ALIGN  aui8NumRegsType[PVR_ALIGN((IMG_UINT32)RGXFWIF_REG_CFG_TYPE_ALL,sizeof(IMG_UINT32))];
	RGXFWIF_REG_CFG_REC RGXFW_ALIGN  asRegConfigs[RGXFWIF_REG_CFG_MAX_SIZE];
} UNCACHED_ALIGN RGXFWIF_REG_CFG;

typedef enum
{
	RGXFWIF_OS_ONLINE = 1,
	RGXFWIF_OS_OFFLINE
} RGXFWIF_OS_STATE_CHANGE;

/*!
 * @Brief Command data for \ref RGXFWIF_KCCB_CMD_OS_ONLINE_STATE_CONFIGURE type command
 */
typedef struct
{
	IMG_UINT32 ui32DriverID;
	RGXFWIF_OS_STATE_CHANGE eNewOSState;
} UNCACHED_ALIGN RGXFWIF_OS_STATE_CHANGE_DATA;

typedef enum
{
	RGXFWIF_PWR_COUNTER_DUMP_START = 1,
	RGXFWIF_PWR_COUNTER_DUMP_STOP,
	RGXFWIF_PWR_COUNTER_DUMP_SAMPLE,
} RGXFWIF_COUNTER_DUMP_REQUEST;

typedef struct
{
	RGXFWIF_COUNTER_DUMP_REQUEST eCounterDumpRequest;
}  RGXFW_ALIGN RGXFWIF_COUNTER_DUMP_DATA;

typedef struct
{
	PRGXFWIF_FWCOMMONCONTEXT psContext;
	IMG_UINT32               ui32FirstIntJobRefToCancel;
	IMG_UINT32               ui32LastIntJobRefToCancel;
} UNCACHED_ALIGN RGXFWIF_CANCEL_WORK_DATA;

/*!
 * @Brief List of command types supported by the Kernel CCB
 */
typedef enum
{
	/* Common commands */
	RGXFWIF_KCCB_CMD_KICK								= 101U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< DM workload kick command */
	RGXFWIF_KCCB_CMD_MMUCACHE							= 102U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< MMU cache invalidation request */
	RGXFWIF_KCCB_CMD_BP									= 103U | RGX_CMD_MAGIC_DWORD_SHIFTED,
	RGXFWIF_KCCB_CMD_SLCFLUSHINVAL						= 104U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< SLC flush and invalidation request */
	RGXFWIF_KCCB_CMD_CLEANUP							= 105U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Requests cleanup of a FW resource (type specified in the command data) */
	RGXFWIF_KCCB_CMD_ZSBUFFER_BACKING_UPDATE			= 106U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Backing for on-demand ZS-Buffer done */
	RGXFWIF_KCCB_CMD_ZSBUFFER_UNBACKING_UPDATE			= 107U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Unbacking for on-demand ZS-Buffer done */
	RGXFWIF_KCCB_CMD_FREELIST_GROW_UPDATE				= 108U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Freelist Grow done */
	RGXFWIF_KCCB_CMD_FREELISTS_RECONSTRUCTION_UPDATE	= 109U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Freelists Reconstruction done */
	RGXFWIF_KCCB_CMD_NOTIFY_WRITE_OFFSET_UPDATE			= 110U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Informs the firmware that the host has added more data to a CDM2 Circular Buffer */
	RGXFWIF_KCCB_CMD_HEALTH_CHECK						= 111U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Health check request */
	RGXFWIF_KCCB_CMD_FORCE_UPDATE						= 112U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Forcing signalling of all unmet UFOs for a given CCB offset */
	RGXFWIF_KCCB_CMD_COMBINED_TA_3D_KICK				= 113U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< There is a TA and a 3D command in this single kick */
	RGXFWIF_KCCB_CMD_OS_ONLINE_STATE_CONFIGURE			= 114U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Informs the FW that a Guest OS has come online / offline. */
	RGXFWIF_KCCB_CMD_DISABLE_ZSSTORE					= 115U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Informs the FW to disable zs store of a running 3D or add it to queue of render context. */
	RGXFWIF_KCCB_CMD_CANCEL_WORK						= 116U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Cancel all work up to and including a given intjobref for a given context */
	/* Commands only permitted to the native or host OS */
	RGXFWIF_KCCB_CMD_POW								= 200U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Power request (type specified in the command data) */
	RGXFWIF_KCCB_CMD_REGCONFIG							= 201U | RGX_CMD_MAGIC_DWORD_SHIFTED,
	RGXFWIF_KCCB_CMD_CORECLKSPEEDCHANGE					= 202U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Core clock speed change event */
	RGXFWIF_KCCB_CMD_LOGTYPE_UPDATE						= 203U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Ask the firmware to update its cached ui32LogType value from the (shared) tracebuf control structure */
	RGXFWIF_KCCB_CMD_PDVFS_LIMIT_MAX_FREQ				= 204U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Set a maximum frequency/OPP point */
	RGXFWIF_KCCB_CMD_VZ_DRV_ARRAY_CHANGE				= 205U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Changes the priority/group for a particular driver. It can only be serviced for the Host DDK */
	RGXFWIF_KCCB_CMD_STATEFLAGS_CTRL					= 206U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Set or clear firmware state flags */
	RGXFWIF_KCCB_CMD_PDVFS_LIMIT_MIN_FREQ				= 207U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Set a minimum frequency/OPP point */
	RGXFWIF_KCCB_CMD_PHR_CFG							= 208U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Configure Periodic Hardware Reset behaviour */
#if defined(SUPPORT_VALIDATION)
	RGXFWIF_KCCB_CMD_RGXREG								= 209U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Read RGX Register from FW */
#endif
	RGXFWIF_KCCB_CMD_WDG_CFG							= 210U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Configure Safety Firmware Watchdog */
	RGXFWIF_KCCB_CMD_COUNTER_DUMP						= 211U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Controls counter dumping in the FW */
#if defined(SUPPORT_VALIDATION)
	RGXFWIF_KCCB_CMD_GPUMAP								= 212U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Request a FW GPU mapping which is written into by the FW with a pattern */
#endif
	RGXFWIF_KCCB_CMD_VZ_DRV_TIME_SLICE					= 213U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Changes the GPU time slice for a particular driver. It can only be serviced for the Host DDK */
	RGXFWIF_KCCB_CMD_VZ_DRV_TIME_SLICE_INTERVAL			= 214U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Changes the GPU time slice interval for all drivers. It can only be serviced for the Host DDK */

	/* HWPerf commands */
	RGXFWIF_KCCB_CMD_HWPERF_UPDATE_CONFIG				= 300U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Configure HWPerf events (to be generated) and HWPerf buffer address (if required) */
	RGXFWIF_KCCB_CMD_HWPERF_CONFIG_BLKS					= 301U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Configure, clear and enable multiple HWPerf blocks */
	RGXFWIF_KCCB_CMD_HWPERF_CTRL_BLKS					= 302U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Enable or disable multiple HWPerf blocks (reusing existing configuration) */
	RGXFWIF_KCCB_CMD_HWPERF_CONFIG_ENABLE_BLKS_DIRECT	= 303U | RGX_CMD_MAGIC_DWORD_SHIFTED, /*!< Configure, clear and enable multiple HWPerf blocks during the init process */

} RGXFWIF_KCCB_CMD_TYPE;

#define RGXFWIF_LAST_ALLOWED_GUEST_KCCB_CMD (RGXFWIF_KCCB_CMD_POW - 1)

/*! @Brief Kernel CCB command packet */
typedef struct
{
	RGXFWIF_KCCB_CMD_TYPE  eCmdType;      /*!< Command type */
	IMG_UINT32             ui32KCCBFlags; /*!< Compatibility and other flags */

	/* NOTE: Make sure that uCmdData is the last member of this struct
	 * This is to calculate actual command size for device mem copy.
	 * (Refer RGXGetCmdMemCopySize())
	 * */
	union
	{
		RGXFWIF_KCCB_CMD_KICK_DATA			sCmdKickData;			/*!< Data for Kick command */
		RGXFWIF_KCCB_CMD_COMBINED_TA_3D_KICK_DATA	sCombinedTA3DCmdKickData;	/*!< Data for combined TA/3D Kick command */
		RGXFWIF_MMUCACHEDATA				sMMUCacheData;			/*!< Data for MMU cache command */
		RGXFWIF_BPDATA						sBPData;				/*!< Data for Breakpoint Commands */
		RGXFWIF_SLCFLUSHINVALDATA			sSLCFlushInvalData;		/*!< Data for SLC Flush/Inval commands */
		RGXFWIF_CLEANUP_REQUEST				sCleanupData;			/*!< Data for cleanup commands */
		RGXFWIF_POWER_REQUEST				sPowData;				/*!< Data for power request commands */
		RGXFWIF_HWPERF_CTRL					sHWPerfCtrl;			/*!< Data for HWPerf control command */
		RGXFWIF_HWPERF_CONFIG_ENABLE_BLKS	sHWPerfCfgEnableBlks;	/*!< Data for HWPerf configure, clear and enable performance counter block command */
		RGXFWIF_HWPERF_CTRL_BLKS			sHWPerfCtrlBlks;		/*!< Data for HWPerf enable or disable performance counter block commands */
		RGXFWIF_CORECLKSPEEDCHANGE_DATA		sCoreClkSpeedChangeData;/*!< Data for core clock speed change */
		RGXFWIF_ZSBUFFER_BACKING_DATA		sZSBufferBackingData;	/*!< Feedback for Z/S Buffer backing/unbacking */
		RGXFWIF_FREELIST_GS_DATA			sFreeListGSData;		/*!< Feedback for Freelist grow/shrink */
		RGXFWIF_FREELISTS_RECONSTRUCTION_DATA	sFreeListsReconstructionData;	/*!< Feedback for Freelists reconstruction */
		RGXFWIF_REGCONFIG_DATA				sRegConfigData;			/*!< Data for custom register configuration */
		RGXFWIF_WRITE_OFFSET_UPDATE_DATA    sWriteOffsetUpdateData; /*!< Data for informing the FW about the write offset update */
#if defined(SUPPORT_PDVFS)
		RGXFWIF_PDVFS_MAX_FREQ_DATA			sPDVFSMaxFreqData;
		RGXFWIF_PDVFS_MIN_FREQ_DATA			sPDVFSMinFreqData;		/*!< Data for setting the min frequency/OPP */
#endif
		RGXFWIF_OS_STATE_CHANGE_DATA        sCmdOSOnlineStateData;  /*!< Data for updating the Guest Online states */
		RGXFWIF_DEV_VIRTADDR                sTBIBuffer;             /*!< Dev address for TBI buffer allocated on demand */
		RGXFWIF_COUNTER_DUMP_DATA			sCounterDumpConfigData; /*!< Data for dumping of register ranges */
		RGXFWIF_KCCB_CMD_DISABLE_ZSSTORE_DATA	sDisableZSStoreData;	/*!< Data for disabling zs store of a 3D workload */
		RGXFWIF_KCCB_CMD_FORCE_UPDATE_DATA  sForceUpdateData;       /*!< Data for signalling all unmet fences for a given CCB */
#if defined(SUPPORT_VALIDATION)
		RGXFWIF_RGXREG_DATA                 sFwRgxData;             /*!< Data for reading off an RGX register */
		RGXFWIF_GPUMAP_DATA                 sGPUMapData;            /*!< Data for requesting a FW GPU mapping which is written into by the FW with a pattern */
#endif
		RGXFWIF_CANCEL_WORK_DATA			sCancelWorkData;		/*!< Data for cancelling work */
	} UNCACHED_ALIGN uCmdData;
} UNCACHED_ALIGN RGXFWIF_KCCB_CMD;

RGX_FW_STRUCT_SIZE_ASSERT(RGXFWIF_KCCB_CMD);

/*! @} End of KCCBTypes */

/*!
 * @Defgroup FWCCBTypes Firmware CCB data interface
 * @Brief Types grouping data structures and defines used in realising the Firmware CCB functionality
 * @{
 */

/*!
 ******************************************************************************
 * @Brief Command data of the \ref RGXFWIF_FWCCB_CMD_ZSBUFFER_BACKING and the
 * \ref RGXFWIF_FWCCB_CMD_ZSBUFFER_UNBACKING Firmware CCB commands
 *****************************************************************************/
typedef struct
{
	IMG_UINT32				ui32ZSBufferID; /*!< ZS buffer ID */
} RGXFWIF_FWCCB_CMD_ZSBUFFER_BACKING_DATA;

/*!
 ******************************************************************************
 * @Brief Command data of the \ref RGXFWIF_FWCCB_CMD_FREELIST_GROW Firmware CCB
 * command
 *****************************************************************************/
typedef struct
{
	IMG_UINT32				ui32FreelistID; /*!< Freelist ID */
} RGXFWIF_FWCCB_CMD_FREELIST_GS_DATA;

/*!
 ******************************************************************************
 * @Brief Command data of the \ref RGXFWIF_FWCCB_CMD_FREELISTS_RECONSTRUCTION
 * Firmware CCB command
 *****************************************************************************/
typedef struct
{
	IMG_UINT32			ui32FreelistsCount;                                     /*!< Freelists count */
	IMG_UINT32			ui32HwrCounter;                                         /*!< HWR counter */
	IMG_UINT32			aui32FreelistIDs[RGXFWIF_MAX_FREELISTS_TO_RECONSTRUCT]; /*!< Array of freelist IDs to reconstruct */
} RGXFWIF_FWCCB_CMD_FREELISTS_RECONSTRUCTION_DATA;

#define RGXFWIF_FWCCB_CMD_CONTEXT_RESET_FLAG_PF			(1U<<0)	/*!< 1 if a page fault happened */
#define RGXFWIF_FWCCB_CMD_CONTEXT_RESET_FLAG_ALL_CTXS	(1U<<1)	/*!< 1 if applicable to all contexts */

/*!
 ******************************************************************************
 * @Brief Command data of the \ref RGXFWIF_FWCCB_CMD_CONTEXT_RESET_NOTIFICATION
 * Firmware CCB command
 *****************************************************************************/
typedef struct
{
	IMG_UINT32						ui32ServerCommonContextID;	/*!< Context affected by the reset */
	RGX_CONTEXT_RESET_REASON		eResetReason;				/*!< Reason for reset */
	RGXFWIF_DM						eDM;						/*!< Data Master affected by the reset */
	IMG_UINT32						ui32ResetJobRef;			/*!< Job ref running at the time of reset */
	IMG_UINT32						ui32Flags;					/*!< RGXFWIF_FWCCB_CMD_CONTEXT_RESET_FLAG bitfield */
	IMG_UINT64 RGXFW_ALIGN			ui64PCAddress;				/*!< At what page catalog address */
	IMG_DEV_VIRTADDR RGXFW_ALIGN	sFaultAddress;				/*!< Page fault address (only when applicable) */
} RGXFWIF_FWCCB_CMD_CONTEXT_RESET_DATA;

/*!
 ******************************************************************************
 * @Brief Command data of the \ref RGXFWIF_FWCCB_CMD_CONTEXT_FW_PF_NOTIFICATION
 * Firmware CCB command
 *****************************************************************************/
typedef struct
{
	IMG_DEV_VIRTADDR sFWFaultAddr;	/*!< Page fault address */
} RGXFWIF_FWCCB_CMD_FW_PAGEFAULT_DATA;

/*!
 ******************************************************************************
 * List of command types supported by the Firmware CCB
 *****************************************************************************/
typedef enum
{
	RGXFWIF_FWCCB_CMD_ZSBUFFER_BACKING              = 101U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests ZSBuffer to be backed with physical pages
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_ZSBUFFER_BACKING_DATA */
	RGXFWIF_FWCCB_CMD_ZSBUFFER_UNBACKING            = 102U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests ZSBuffer to be unbacked
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_ZSBUFFER_BACKING_DATA */
	RGXFWIF_FWCCB_CMD_FREELIST_GROW                 = 103U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests an on-demand freelist grow
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_FREELIST_GS_DATA */
	RGXFWIF_FWCCB_CMD_FREELISTS_RECONSTRUCTION      = 104U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests freelists reconstruction
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_FREELISTS_RECONSTRUCTION_DATA */
	RGXFWIF_FWCCB_CMD_CONTEXT_RESET_NOTIFICATION    = 105U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Notifies host of a HWR event on a context
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_CONTEXT_RESET_DATA */
	RGXFWIF_FWCCB_CMD_DEBUG_DUMP                    = 106U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests an on-demand debug dump
	                                                                                          \n Command data: None */
	RGXFWIF_FWCCB_CMD_UPDATE_STATS                  = 107U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests an on-demand update on process stats
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_UPDATE_STATS_DATA */
#if defined(SUPPORT_PDVFS)
	RGXFWIF_FWCCB_CMD_CORE_CLK_RATE_CHANGE          = 108U | RGX_CMD_MAGIC_DWORD_SHIFTED,
#endif
	RGXFWIF_FWCCB_CMD_REQUEST_GPU_RESTART           = 109U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Requests GPU restart
	                                                                                          \n Command data: None */
#if defined(SUPPORT_VALIDATION)
	RGXFWIF_FWCCB_CMD_REG_READ                      = 110U | RGX_CMD_MAGIC_DWORD_SHIFTED,
#if defined(SUPPORT_SOC_TIMER)
	RGXFWIF_FWCCB_CMD_SAMPLE_TIMERS                 = 111U | RGX_CMD_MAGIC_DWORD_SHIFTED,
#endif
#endif
	RGXFWIF_FWCCB_CMD_CONTEXT_FW_PF_NOTIFICATION    = 112U | RGX_CMD_MAGIC_DWORD_SHIFTED,   /*!< Notifies host of a FW pagefault
	                                                                                          \n Command data: RGXFWIF_FWCCB_CMD_FW_PAGEFAULT_DATA */
} RGXFWIF_FWCCB_CMD_TYPE;

/*!
 ******************************************************************************
 * List of the various stats of the process to update/increment
 *****************************************************************************/
typedef enum
{
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_PARTIAL_RENDERS=1,		/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32TotalNumPartialRenders stat */
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_OUT_OF_MEMORY,			/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32TotalNumOutOfMemory stat */
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_TA_STORES,				/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32NumTAStores stat */
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_3D_STORES,				/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32Num3DStores stat */
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_CDM_STORES,			/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32NumCDMStores stat */
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_TDM_STORES,			/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32NumTDMStores stat */
	RGXFWIF_FWCCB_CMD_UPDATE_NUM_RAY_STORES				/*!< PVRSRVStatsUpdateRenderContextStats should increase the value of the ui32NumRayStores stat */
} RGXFWIF_FWCCB_CMD_UPDATE_STATS_TYPE;

/*!
 ******************************************************************************
 * @Brief Command data of the \ref RGXFWIF_FWCCB_CMD_UPDATE_STATS Firmware CCB
 * command
 *****************************************************************************/
typedef struct
{
	RGXFWIF_FWCCB_CMD_UPDATE_STATS_TYPE		eElementToUpdate;			/*!< Element to update */
	IMG_PID									pidOwner;					/*!< The pid of the process whose stats are being updated */
	IMG_INT32								i32AdjustmentValue;			/*!< Adjustment to be made to the statistic */
} RGXFWIF_FWCCB_CMD_UPDATE_STATS_DATA;

typedef struct
{
	IMG_UINT32 ui32CoreClkRate;
} UNCACHED_ALIGN RGXFWIF_FWCCB_CMD_CORE_CLK_RATE_CHANGE_DATA;

#if defined(SUPPORT_VALIDATION)
typedef struct
{
	IMG_UINT64 ui64RegValue;
} RGXFWIF_FWCCB_CMD_RGXREG_READ_DATA;

#if defined(SUPPORT_SOC_TIMER)
typedef struct
{
	IMG_UINT64 ui64timerGray;
	IMG_UINT64 ui64timerBinary;
	IMG_UINT64 aui64uscTimers[RGX_FEATURE_NUM_CLUSTERS];
}  RGXFWIF_FWCCB_CMD_SAMPLE_TIMERS_DATA;
#endif
#endif

/*!
 ******************************************************************************
 * @Brief Firmware CCB command structure
 *****************************************************************************/
typedef struct
{
	RGXFWIF_FWCCB_CMD_TYPE  eCmdType;       /*!< Command type */
	IMG_UINT32              ui32FWCCBFlags; /*!< Compatibility and other flags */

	union
	{
		RGXFWIF_FWCCB_CMD_ZSBUFFER_BACKING_DATA				sCmdZSBufferBacking;			/*!< Data for Z/S-Buffer on-demand (un)backing*/
		RGXFWIF_FWCCB_CMD_FREELIST_GS_DATA					sCmdFreeListGS;					/*!< Data for on-demand freelist grow/shrink */
		RGXFWIF_FWCCB_CMD_FREELISTS_RECONSTRUCTION_DATA		sCmdFreeListsReconstruction;	/*!< Data for freelists reconstruction */
		RGXFWIF_FWCCB_CMD_CONTEXT_RESET_DATA				sCmdContextResetNotification;	/*!< Data for context reset notification */
		RGXFWIF_FWCCB_CMD_UPDATE_STATS_DATA					sCmdUpdateStatsData;			/*!< Data for updating process stats */
		RGXFWIF_FWCCB_CMD_CORE_CLK_RATE_CHANGE_DATA			sCmdCoreClkRateChange;
		RGXFWIF_FWCCB_CMD_FW_PAGEFAULT_DATA					sCmdFWPagefault;				/*!< Data for context reset notification */
#if defined(SUPPORT_VALIDATION)
		RGXFWIF_FWCCB_CMD_RGXREG_READ_DATA					sCmdRgxRegReadData;
#if defined(SUPPORT_SOC_TIMER)
		RGXFWIF_FWCCB_CMD_SAMPLE_TIMERS_DATA				sCmdTimers;
#endif
#endif
	} RGXFW_ALIGN uCmdData;
} RGXFW_ALIGN RGXFWIF_FWCCB_CMD;

RGX_FW_STRUCT_SIZE_ASSERT(RGXFWIF_FWCCB_CMD);

/*! @} End of FWCCBTypes */

/*!
 ******************************************************************************
 * Workload estimation Firmware CCB command structure for RGX
 *****************************************************************************/
typedef struct
{
	IMG_UINT16 ui16ReturnDataIndex; /*!< Index for return data array */
	IMG_UINT32 ui32CyclesTaken;     /*!< The cycles the workload took on the hardware */
} RGXFWIF_WORKEST_FWCCB_CMD;

/*!
 * @Defgroup ClientCCBTypes Client CCB data interface
 * @Brief Types grouping data structures and defines used in realising Client CCB commands/functionality
 * @{
 */

/* Required memory alignment for 64-bit variables accessible by Meta
  (The gcc meta aligns 64-bit variables to 64-bit; therefore, memory shared
   between the host and meta that contains 64-bit variables has to maintain
   this alignment) */
#define RGXFWIF_FWALLOC_ALIGN	sizeof(IMG_UINT64)

#define RGX_CCB_TYPE_TASK			(IMG_UINT32_C(1) << 15)
#define RGX_CCB_FWALLOC_ALIGN(size)	(PVR_ALIGN(size, RGXFWIF_FWALLOC_ALIGN))

typedef IMG_UINT32 RGXFWIF_CCB_CMD_TYPE;

/*!
 * @Name Client CCB command types
 * @{
 */
#define RGXFWIF_CCB_CMD_TYPE_GEOM			(201U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< TA DM command */
#define RGXFWIF_CCB_CMD_TYPE_TQ_3D			(202U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< 3D DM command for TQ operation */
#define RGXFWIF_CCB_CMD_TYPE_3D				(203U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< 3D DM command */
#define RGXFWIF_CCB_CMD_TYPE_3D_PR			(204U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< 3D DM command for Partial render */
#define RGXFWIF_CCB_CMD_TYPE_CDM			(205U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< Compute DM command */
#define RGXFWIF_CCB_CMD_TYPE_TQ_TDM			(206U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< TDM command */
#define RGXFWIF_CCB_CMD_TYPE_FBSC_INVALIDATE (207U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK)
#define RGXFWIF_CCB_CMD_TYPE_TQ_2D			(208U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< 2D DM command for TQ operation */
#define RGXFWIF_CCB_CMD_TYPE_PRE_TIMESTAMP	(209U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK)
#define RGXFWIF_CCB_CMD_TYPE_NULL			(210U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK)
#define RGXFWIF_CCB_CMD_TYPE_ABORT			(211U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK)

/* Leave a gap between CCB specific commands and generic commands */
#define RGXFWIF_CCB_CMD_TYPE_FENCE          (212U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Fence dependencies of a command */
#define RGXFWIF_CCB_CMD_TYPE_UPDATE         (213U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Fence updates of a command */
#define RGXFWIF_CCB_CMD_TYPE_RMW_UPDATE     (214U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Fence updates related to workload resources */
#define RGXFWIF_CCB_CMD_TYPE_FENCE_PR       (215U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Fence dependencies of a PR command */
#define RGXFWIF_CCB_CMD_TYPE_PRIORITY       (216U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Context priority update command */
/* Pre and Post timestamp commands are supposed to sandwich the DM cmd. The
   padding code with the CCB wrap upsets the FW if we don't have the task type
   bit cleared for POST_TIMESTAMPs. That's why we have 2 different cmd types.
*/
#define RGXFWIF_CCB_CMD_TYPE_POST_TIMESTAMP (217U | RGX_CMD_MAGIC_DWORD_SHIFTED)
#define RGXFWIF_CCB_CMD_TYPE_UNFENCED_UPDATE (218U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Unfenced fence updates of a command */
#define RGXFWIF_CCB_CMD_TYPE_UNFENCED_RMW_UPDATE (219U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Unfenced fence updates related to workload resources */

#if defined(SUPPORT_VALIDATION)
#define RGXFWIF_CCB_CMD_TYPE_REG_READ (220U | RGX_CMD_MAGIC_DWORD_SHIFTED)
#endif

#define RGXFWIF_CCB_CMD_TYPE_PADDING	(221U | RGX_CMD_MAGIC_DWORD_SHIFTED) /*!< Skip without action type command */
#define RGXFWIF_CCB_CMD_TYPE_RAY		(222U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK)
#define RGXFWIF_CCB_CMD_TYPE_VK_TIMESTAMP	(223U | RGX_CMD_MAGIC_DWORD_SHIFTED | RGX_CCB_TYPE_TASK) /*!< Process a vulkan timestamp */
/*! @} End of Client CCB command types */


#define RGXFWIF_TRP_STATUS_UNKNOWN				0x000U
#define RGXFWIF_TRP_STATUS_CHECKSUMS_OK			0x001U
#define RGXFWIF_TRP_STATUS_CHECKSUMS_ERROR		0x002U

#define RGXFWIF_CR_TRP_SIGNATURE_STATUS			(RGX_CR_SCRATCH10)


typedef struct
{
	/* Index for the KM Workload estimation return data array */
	IMG_UINT16 RGXFW_ALIGN         ui16ReturnDataIndex;
	/* Predicted time taken to do the work in cycles */
	IMG_UINT32 RGXFW_ALIGN         ui32CyclesPrediction;
	/* Deadline for the workload (in usecs) */
	IMG_UINT64 RGXFW_ALIGN         ui64Deadline;
} RGXFWIF_WORKEST_KICK_DATA;

/*! @Brief Command header of a command in the client CCB buffer.
 *
 *  Followed by this header is the command-data specific to the
 *  command-type as specified in the header.
 */
typedef struct
{
	RGXFWIF_CCB_CMD_TYPE					eCmdType;      /*!< Command data type following this command header */
	IMG_UINT32								ui32CmdSize;   /*!< Size of the command following this header */
	IMG_UINT32								ui32ExtJobRef; /*!< external job reference - provided by client and used in debug for tracking submitted work */
	IMG_UINT32								ui32IntJobRef; /*!< internal job reference - generated by services and used in debug for tracking submitted work */
#if defined(SUPPORT_WORKLOAD_ESTIMATION)
	RGXFWIF_WORKEST_KICK_DATA RGXFW_ALIGN	sWorkEstKickData; /*!< Workload Estimation - Workload Estimation Data */
#endif
} RGXFWIF_CCB_CMD_HEADER;

/*
 ******************************************************************************
 * Client CCB commands which are only required by the kernel
 *****************************************************************************/

/*! @Brief Command data for \ref RGXFWIF_CCB_CMD_TYPE_PRIORITY type client CCB command */
typedef struct
{
	IMG_INT32              i32Priority; /*!< Priority level */
} RGXFWIF_CMD_PRIORITY;

/*! @} End of ClientCCBTypes */

/*!
 ******************************************************************************
 * Signature and Checksums Buffer
 *****************************************************************************/
typedef struct
{
	PRGXFWIF_SIGBUFFER		sBuffer;			/*!< Ptr to Signature Buffer memory */
	IMG_UINT32				ui32LeftSizeInRegs;	/*!< Amount of space left for storing regs in the buffer */
} UNCACHED_ALIGN RGXFWIF_SIGBUF_CTL;

typedef struct
{
	PRGXFWIF_COUNTERBUFFER	sBuffer;			/*!< Ptr to counter dump buffer */
	IMG_UINT32				ui32SizeInDwords;	/*!< Amount of space for storing in the buffer */
} UNCACHED_ALIGN RGXFWIF_COUNTER_DUMP_CTL;

typedef struct
{
	PRGXFWIF_FIRMWAREGCOVBUFFER	sBuffer;		/*!< Ptr to firmware gcov buffer */
	IMG_UINT32					ui32Size;		/*!< Amount of space for storing in the buffer */
} UNCACHED_ALIGN RGXFWIF_FIRMWARE_GCOV_CTL;

/*!
 *****************************************************************************
 * RGX Compatibility checks
 *****************************************************************************/

/* WARNING: Whenever the layout of RGXFWIF_COMPCHECKS_BVNC changes, the
	following define should be increased by 1 to indicate to the
	compatibility logic that layout has changed. */
#define RGXFWIF_COMPCHECKS_LAYOUT_VERSION 3

typedef struct
{
	IMG_UINT32	ui32LayoutVersion; /* WARNING: This field must be defined as first one in this structure */
	IMG_UINT64	RGXFW_ALIGN ui64BVNC;
} UNCACHED_ALIGN RGXFWIF_COMPCHECKS_BVNC;

typedef struct
{
	IMG_UINT8	ui8OsCountSupport;
} UNCACHED_ALIGN RGXFWIF_INIT_OPTIONS;

#define RGXFWIF_COMPCHECKS_BVNC_DECLARE_AND_INIT(name) \
	RGXFWIF_COMPCHECKS_BVNC (name) = { \
		RGXFWIF_COMPCHECKS_LAYOUT_VERSION, \
		0, \
	}
#define RGXFWIF_COMPCHECKS_BVNC_INIT(name) \
	do { \
		(name).ui32LayoutVersion = RGXFWIF_COMPCHECKS_LAYOUT_VERSION; \
		(name).ui64BVNC = 0; \
	} while (false)

typedef struct
{
	RGXFWIF_COMPCHECKS_BVNC		sHWBVNC;				/*!< hardware BVNC (from the RGX registers) */
	RGXFWIF_COMPCHECKS_BVNC		sFWBVNC;				/*!< firmware BVNC */
	IMG_UINT32					ui32DDKVersion;			/*!< software DDK version */
	IMG_UINT32					ui32DDKBuild;			/*!< software DDK build no. */
	IMG_UINT32					ui32BuildOptions;		/*!< build options bit-field */
	RGXFWIF_INIT_OPTIONS		sInitOptions;			/*!< initialisation options bit-field */
	IMG_BOOL					bUpdated;				/*!< Information is valid */
} UNCACHED_ALIGN RGXFWIF_COMPCHECKS;

typedef struct
{
	IMG_UINT32                 ui32NumCores;
	IMG_UINT32                 ui32TotalCores;
	IMG_UINT32                 ui32MulticoreInfo;
} UNCACHED_ALIGN RGXFWIF_MULTICORE_INFO;

/*! @Brief Firmware Runtime configuration data \ref RGXFWIF_RUNTIME_CFG
 * allocated by services and used by the Firmware on boot
 **/
typedef struct
{
	IMG_UINT32         ui32ActivePMLatencyms;                    /*!< APM latency in ms before signalling IDLE to the host */
	IMG_UINT32         ui32RuntimeCfgFlags;                      /*!< Compatibility and other flags */
	IMG_BOOL           bActivePMLatencyPersistant;               /*!< If set, APM latency does not reset to system default each GPU power transition */
	IMG_UINT32         ui32CoreClockSpeed;                       /*!< Core clock speed, currently only used to calculate timer ticks */
	IMG_UINT32         ui32PowUnitsState;                        /*!< Power Unit state mask set by the host */
	IMG_UINT32         ui32RACUnitsState;                        /*!< RAC state mask set by the host */
	IMG_UINT32         ui32PHRMode;                              /*!< Periodic Hardware Reset configuration values */
	IMG_UINT32         ui32HCSDeadlineMS;                        /*!< New number of milliseconds C/S is allowed to last */
	IMG_UINT32         ui32WdgPeriodUs;                          /*!< The watchdog period in microseconds */
	IMG_INT32          ai32DriverPriority[RGXFW_MAX_NUM_OSIDS];  /*!< Array of priorities per OS */
	IMG_UINT32         aui32DriverIsolationGroup[RGXFW_MAX_NUM_OSIDS]; /*!< Array of isolation groups per OS */
	IMG_UINT32         aui32TSPercentage[RGXFW_MAX_NUM_OSIDS];   /*!< Array of time slice per OS */
	IMG_UINT32         ui32TSIntervalMs;                         /*!< Time slice interval */
	IMG_UINT32         ui32VzConnectionCooldownPeriodInSec;      /*!< Vz Connection Cooldown period in secs */

	PRGXFWIF_HWPERFBUF sHWPerfBuf;                               /*!< On-demand allocated HWPerf buffer address, to be passed to the FW */
	RGXFWIF_DMA_ADDR   sHWPerfDMABuf;
	RGXFWIF_DMA_ADDR   sHWPerfCtlDMABuf;
#if defined(SUPPORT_VALIDATION)
	IMG_BOOL           bInjectFWFault;                           /*!< Injecting firmware fault to validate recovery through Host */
#endif
} RGXFWIF_RUNTIME_CFG;

/*!
 *****************************************************************************
 * Control data for RGX
 *****************************************************************************/

#define RGXFWIF_HWR_DEBUG_DUMP_ALL (99999U)

#if defined(PDUMP)

#define RGXFWIF_PID_FILTER_MAX_NUM_PIDS 32U

typedef enum
{
	RGXFW_PID_FILTER_INCLUDE_ALL_EXCEPT,
	RGXFW_PID_FILTER_EXCLUDE_ALL_EXCEPT
} RGXFWIF_PID_FILTER_MODE;

typedef struct
{
	IMG_PID uiPID;
	IMG_UINT32 ui32DriverID;
} RGXFW_ALIGN RGXFWIF_PID_FILTER_ITEM;

typedef struct
{
	RGXFWIF_PID_FILTER_MODE eMode;
	/* each process in the filter list is specified by a PID and OS ID pair.
	 * each PID and OS pair is an item in the items array (asItems).
	 * if the array contains less than RGXFWIF_PID_FILTER_MAX_NUM_PIDS entries
	 * then it must be terminated by an item with pid of zero.
	 */
	RGXFWIF_PID_FILTER_ITEM asItems[RGXFWIF_PID_FILTER_MAX_NUM_PIDS];
} RGXFW_ALIGN RGXFWIF_PID_FILTER;
#endif

#if defined(SUPPORT_SECURITY_VALIDATION)
#define RGXFWIF_SECURE_ACCESS_TEST_READ_WRITE_FW_DATA  (0x1U << 0)
#define RGXFWIF_SECURE_ACCESS_TEST_READ_WRITE_FW_CODE  (0x1U << 1)
#define RGXFWIF_SECURE_ACCESS_TEST_RUN_FROM_NONSECURE  (0x1U << 2)
#define RGXFWIF_SECURE_ACCESS_TEST_RUN_FROM_SECURE     (0x1U << 3)
#endif

typedef enum
{
	RGXFWIF_USRM_DM_VDM = 0,
	RGXFWIF_USRM_DM_DDM = 1,
	RGXFWIF_USRM_DM_CDM = 2,
	RGXFWIF_USRM_DM_PDM = 3,
	RGXFWIF_USRM_DM_TDM = 4,
	RGXFWIF_USRM_DM_LAST
} RGXFWIF_USRM_DM;

typedef enum
{
	RGXFWIF_UVBRM_DM_VDM = 0,
	RGXFWIF_UVBRM_DM_DDM = 1,
	RGXFWIF_UVBRM_DM_LAST
} RGXFWIF_UVBRM_DM;

typedef enum
{
	RGXFWIF_TPU_DM_PDM = 0,
	RGXFWIF_TPU_DM_VDM = 1,
	RGXFWIF_TPU_DM_CDM = 2,
	RGXFWIF_TPU_DM_TDM = 3,
	RGXFWIF_TPU_DM_RDM = 4,
	RGXFWIF_TPU_DM_LAST
} RGXFWIF_TPU_DM;

typedef enum
{
	RGXFWIF_GPIO_VAL_OFF           = 0, /*!< No GPIO validation */
	RGXFWIF_GPIO_VAL_GENERAL       = 1, /*!< Simple test case that
	                                         initiates by sending data via the
	                                         GPIO and then sends back any data
	                                         received over the GPIO */
	RGXFWIF_GPIO_VAL_AP            = 2, /*!< More complex test case that writes
	                                         and reads data across the entire
	                                         GPIO AP address range.*/
#if defined(SUPPORT_STRIP_RENDERING)
	RGXFWIF_GPIO_VAL_SR_BASIC      = 3, /*!< Strip Rendering AP based basic test.*/
	RGXFWIF_GPIO_VAL_SR_COMPLEX    = 4, /*!< Strip Rendering AP based complex test.*/
#endif
	RGXFWIF_GPIO_VAL_TESTBENCH     = 5, /*!< Validates the GPIO Testbench. */
	RGXFWIF_GPIO_VAL_LOOPBACK      = 6, /*!< Send and then receive each byte
	                                         in the range 0-255. */
	RGXFWIF_GPIO_VAL_LOOPBACK_LITE = 7, /*!< Send and then receive each power-of-2
	                                         byte in the range 0-255. */
	RGXFWIF_GPIO_VAL_LAST
} RGXFWIF_GPIO_VAL_MODE;

typedef IMG_UINT32 FW_PERF_CONF;
#define FW_PERF_CONF_NONE			0U
#define FW_PERF_CONF_ICACHE			1U
#define FW_PERF_CONF_DCACHE			2U
#define FW_PERF_CONF_JTLB_INSTR		5U
#define FW_PERF_CONF_INSTRUCTIONS	6U

typedef enum
{
	FW_BOOT_STAGE_TLB_INIT_FAILURE = -2,
	FW_BOOT_STAGE_NOT_AVAILABLE = -1,
	FW_BOOT_NOT_STARTED = 0,
	FW_BOOT_BLDR_STARTED = 1,
	FW_BOOT_CACHE_DONE,
	FW_BOOT_TLB_DONE,
	FW_BOOT_MAIN_STARTED,
	FW_BOOT_ALIGNCHECKS_DONE,
	FW_BOOT_INIT_DONE,
} FW_BOOT_STAGE;

/*!
 * @AddToGroup KCCBTypes
 * @{
 * @Name Kernel CCB return slot responses
 * @{
 * Usage of bit-fields instead of bare integers
 * allows FW to possibly pack-in several responses for each single kCCB command.
 */

#define RGXFWIF_KCCB_RTN_SLOT_CMD_EXECUTED   (1U << 0) /*!< Command executed (return status from FW) */
#define RGXFWIF_KCCB_RTN_SLOT_CLEANUP_BUSY   (1U << 1) /*!< A cleanup was requested but resource busy */
#define RGXFWIF_KCCB_RTN_SLOT_POLL_FAILURE   (1U << 2) /*!< Poll failed in FW for a HW operation to complete */

#define RGXFWIF_KCCB_RTN_SLOT_NO_RESPONSE            0x0U      /*!< Reset value of a kCCB return slot (set by host) */
/*!
 * @} End of Name Kernel CCB return slot responses
 * @} End of AddToGroup KCCBTypes
 */

/*! @Brief OS connection data \ref RGXFWIF_CONNECTION_CTL allocated
 * by services and used to track OS state in Firmware and Services
 **/
typedef struct
{
	/* Fw-Os connection states */
	volatile RGXFWIF_CONNECTION_FW_STATE eConnectionFwState;    /*!< Firmware-OS connection state */
	volatile RGXFWIF_CONNECTION_OS_STATE eConnectionOsState;    /*!< Services-OS connection state */
	volatile IMG_UINT32                  ui32AliveFwToken;      /*!< OS Alive token updated by Firmware */
	volatile IMG_UINT32                  ui32AliveOsToken;      /*!< OS Alive token updated by Services */
} UNCACHED_ALIGN RGXFWIF_CONNECTION_CTL;

/*! @Brief Firmware OS Initialization data \ref RGXFWIF_OSINIT
 * allocated by services and used by the Firmware on boot
 **/
typedef struct
{
	/* Kernel CCB */
	PRGXFWIF_CCB_CTL        psKernelCCBCtl; /*!< Kernel CCB Control */
	PRGXFWIF_CCB            psKernelCCB; /*!<  Kernel CCB */
	PRGXFWIF_CCB_RTN_SLOTS  psKernelCCBRtnSlots; /*!<  Kernel CCB return slots */

	/* Firmware CCB */
	PRGXFWIF_CCB_CTL        psFirmwareCCBCtl; /*!<  Firmware CCB control */
	PRGXFWIF_CCB            psFirmwareCCB; /*!<  Firmware CCB */

	/* Workload Estimation Firmware CCB */
	PRGXFWIF_CCB_CTL        psWorkEstFirmwareCCBCtl; /*!<  Workload estimation control */
	PRGXFWIF_CCB            psWorkEstFirmwareCCB; /*!<  Workload estimation buffer */

	PRGXFWIF_HWRINFOBUF     sRGXFWIfHWRInfoBufCtl; /*!<  HWRecoveryInfo control */

	IMG_UINT32              ui32HWRDebugDumpLimit; /*!< Firmware debug dump maximum limit */

	PRGXFWIF_OSDATA         sFwOsData; /*!<  Firmware per-os shared data */

	RGXFWIF_COMPCHECKS      sRGXCompChecks; /*!< Compatibility checks to be populated by the Firmware */
	RGXFWIF_MULTICORE_INFO  sRGXMulticoreInfo; /*! < Multicore capability info */

} UNCACHED_ALIGN RGXFWIF_OSINIT;

/*! @Brief Firmware System Initialization data \ref RGXFWIF_SYSINIT
 * allocated by services and used by the Firmware on boot
 **/
typedef struct
{
	IMG_DEV_PHYADDR         RGXFW_ALIGN sFaultPhysAddr; /*!< Fault read address */

	IMG_DEV_VIRTADDR        RGXFW_ALIGN sPDSExecBase; /*!< PDS execution base */
	IMG_DEV_VIRTADDR        RGXFW_ALIGN sUSCExecBase; /*!< USC execution base */
	IMG_DEV_VIRTADDR        RGXFW_ALIGN sFBCDCStateTableBase; /*!< FBCDC bindless texture state table base */
	IMG_DEV_VIRTADDR        RGXFW_ALIGN sFBCDCLargeStateTableBase;
	IMG_DEV_VIRTADDR        RGXFW_ALIGN sTextureHeapBase; /*!< Texture state base */
	IMG_DEV_VIRTADDR        RGXFW_ALIGN sPDSIndirectHeapBase; /* Pixel Indirect State base */

	IMG_UINT64              RGXFW_ALIGN ui64HWPerfFilter; /*! Event filter for Firmware events */

	IMG_DEV_VIRTADDR        RGXFW_ALIGN sSLC3FenceDevVAddr; /*!< Address to use as a fence when issuing SLC3_CFI */

	IMG_UINT64				RGXFW_ALIGN aui64UVBRMNumRegions[RGXFWIF_UVBRM_DM_LAST];
	IMG_UINT32              RGXFW_ALIGN aui32TPUTrilinearFracMask[RGXFWIF_TPU_DM_LAST];
	IMG_UINT32				RGXFW_ALIGN aui32USRMNumRegions[RGXFWIF_USRM_DM_LAST];

	IMG_UINT64				ui64ClkCtrl0;
	IMG_UINT64				ui64ClkCtrl1;
	IMG_UINT32				ui32ClkCtrl2;

	IMG_UINT32              ui32FilterFlags;

	RGXFWIF_SIGBUF_CTL      asSigBufCtl[RGXFWIF_DM_MAX]; /*!< Signature and Checksum Buffers for DMs */
#if defined(SUPPORT_VALIDATION)
	RGXFWIF_SIGBUF_CTL      asValidationSigBufCtl[RGXFWIF_DM_MAX];
	IMG_UINT64              RGXFW_ALIGN ui64RCEDisableMask;
	IMG_UINT32				ui32PCGPktDropThresh;
	IMG_UINT32				ui32RaySLCMMUAutoCacheOps;
	IMG_UINT32				ui32TpuF20BilPrecision;
	IMG_UINT32				ui32TPUTAGCtrl;
#endif

	PRGXFWIF_RUNTIME_CFG    sRuntimeCfg; /*!<  Firmware Runtime configuration */

	PRGXFWIF_TRACEBUF       sTraceBufCtl; /*!<  Firmware Trace buffer control */
	PRGXFWIF_SYSDATA        sFwSysData; /*!< Firmware System shared data */
#if defined(SUPPORT_TBI_INTERFACE)
	PRGXFWIF_TBIBUF         sTBIBuf; /*!< Tbi log buffer */
#endif

	PRGXFWIF_GPU_UTIL_FW    sGpuUtilFWCtl; /*!< GPU utilization buffer */
	PRGXFWIF_REG_CFG        sRegCfg; /*!< Firmware register user configuration */
	PRGXFWIF_HWPERF_CTL     sHWPerfCtl; /*!< HWPerf counter block configuration.*/

	RGXFWIF_COUNTER_DUMP_CTL sCounterDumpCtl;

#if defined(SUPPORT_FIRMWARE_GCOV)
	RGXFWIF_FIRMWARE_GCOV_CTL sFirmwareGcovCtl; /*!< Firmware gcov buffer control */
#endif

	RGXFWIF_DEV_VIRTADDR    sAlignChecks; /*!< Array holding Server structures alignment data */

	IMG_UINT32              ui32InitialCoreClockSpeed; /*!< Core clock speed at FW boot time */

	IMG_UINT32              ui32InitialActivePMLatencyms; /*!< APM latency in ms before signalling IDLE to the host */

	IMG_BOOL                bFirmwareStarted; /*!< Flag to be set by the Firmware after successful start */

	IMG_UINT32              ui32MarkerVal; /*!< Host/FW Trace synchronisation Partition Marker */

	IMG_UINT32              ui32FirmwareStartedTimeStamp; /*!< Firmware initialization complete time */

	IMG_UINT32              ui32JonesDisableMask;

	RGXFWIF_DMA_ADDR        sCorememDataStore; /*!< Firmware coremem data */

	FW_PERF_CONF            eFirmwarePerf; /*!< Firmware performance counter config */

#if defined(SUPPORT_PDVFS)
	RGXFWIF_PDVFS_OPP       RGXFW_ALIGN sPDVFSOPPInfo;

	/**
	 * FW Pointer to memory containing core clock rate in Hz.
	 * Firmware (PDVFS) updates the memory when running on non primary FW thread
	 * to communicate to host driver.
	 */
	PRGXFWIF_CORE_CLK_RATE  RGXFW_ALIGN sCoreClockRate;
#endif

#if defined(PDUMP)
	RGXFWIF_PID_FILTER      sPIDFilter;
#endif

	RGXFWIF_GPIO_VAL_MODE   eGPIOValidationMode;

	RGX_HWPERF_BVNC         sBvncKmFeatureFlags; /*!< Used in HWPerf for decoding BVNC Features*/

#if defined(SUPPORT_SECURITY_VALIDATION)
	IMG_UINT32              ui32SecurityTestFlags;
	RGXFWIF_DEV_VIRTADDR    pbSecureBuffer;
	RGXFWIF_DEV_VIRTADDR    pbNonSecureBuffer;
#endif

#if defined(SUPPORT_FW_HOST_SIDE_RECOVERY)
	RGXFWIF_DEV_VIRTADDR    sActiveContextBufBase; /*!< Active context buffer base */
#endif

#if defined(SUPPORT_GPUVIRT_VALIDATION_MTS)
	/*
	 * Used when validation is enabled to allow the host to check
	 * that MTS sent the correct sideband in response to a kick
	 * from a given OSes schedule register.
	 * Testing is enabled if RGXFWIF_KICK_TEST_ENABLED_BIT is set
	 *
	 * Set by the host to:
	 * (osid << RGXFWIF_KICK_TEST_OSID_SHIFT) | RGXFWIF_KICK_TEST_ENABLED_BIT
	 * reset to 0 by FW when kicked by the given OSid
	 */
	IMG_UINT32              ui32OSKickTest;
#endif

#if defined(SUPPORT_AUTOVZ)
	IMG_UINT32              ui32VzWdgPeriod;
#endif

#if defined(SUPPORT_FW_HOST_SIDE_RECOVERY)
	/* notify firmware power-up on host-side recovery */
	IMG_BOOL                bFwHostRecoveryMode;
#endif

#if defined(SUPPORT_SECURE_CONTEXT_SWITCH)
	RGXFWIF_DEV_VIRTADDR    pbFwScratchBuf;
#endif
} UNCACHED_ALIGN RGXFWIF_SYSINIT;

#if defined(SUPPORT_GPUVIRT_VALIDATION_MTS)
#define RGXFWIF_KICK_TEST_ENABLED_BIT  0x1
#define RGXFWIF_KICK_TEST_OSID_SHIFT   0x1
#endif

/*!
 *****************************************************************************
 * Timer correlation shared data and defines
 *****************************************************************************/

typedef struct
{
	IMG_UINT64 RGXFW_ALIGN ui64OSTimeStamp;
	IMG_UINT64 RGXFW_ALIGN ui64OSMonoTimeStamp;
	IMG_UINT64 RGXFW_ALIGN ui64CRTimeStamp;

	/* Utility variable used to convert CR timer deltas to OS timer deltas (nS),
	 * where the deltas are relative to the timestamps above:
	 * deltaOS = (deltaCR * K) >> decimal_shift, see full explanation below */
	IMG_UINT64 RGXFW_ALIGN ui64CRDeltaToOSDeltaKNs;

	IMG_UINT32             ui32CoreClockSpeed;
	IMG_UINT32             ui32Reserved;
} UNCACHED_ALIGN RGXFWIF_TIME_CORR;


/* The following macros are used to help converting FW timestamps to the Host
 * time domain. On the FW the RGX_CR_TIMER counter is used to keep track of
 * time; it increments by 1 every 256 GPU clock ticks, so the general
 * formula to perform the conversion is:
 *
 * [ GPU clock speed in Hz, if (scale == 10^9) then deltaOS is in nS,
 *   otherwise if (scale == 10^6) then deltaOS is in uS ]
 *
 *             deltaCR * 256                                   256 * scale
 *  deltaOS = --------------- * scale = deltaCR * K    [ K = --------------- ]
 *             GPUclockspeed                                  GPUclockspeed
 *
 * The actual K is multiplied by 2^20 (and deltaCR * K is divided by 2^20)
 * to get some better accuracy and to avoid returning 0 in the integer
 * division 256000000/GPUfreq if GPUfreq is greater than 256MHz.
 * This is the same as keeping K as a decimal number.
 *
 * The maximum deltaOS is slightly more than 5hrs for all GPU frequencies
 * (deltaCR * K is more or less a constant), and it's relative to the base
 * OS timestamp sampled as a part of the timer correlation data.
 * This base is refreshed on GPU power-on, DVFS transition and periodic
 * frequency calibration (executed every few seconds if the FW is doing
 * some work), so as long as the GPU is doing something and one of these
 * events is triggered then deltaCR * K will not overflow and deltaOS will be
 * correct.
 */

#define RGXFWIF_CRDELTA_TO_OSDELTA_ACCURACY_SHIFT  (20)

#define RGXFWIF_GET_DELTA_OSTIME_NS(deltaCR, K) \
	(((deltaCR) * (K)) >> RGXFWIF_CRDELTA_TO_OSDELTA_ACCURACY_SHIFT)


/*!
 ******************************************************************************
 * GPU Utilisation
 *****************************************************************************/

/* See rgx_common.h for a list of GPU states */
#define RGXFWIF_GPU_UTIL_TIME_MASK       (IMG_UINT64_C(0xFFFFFFFFFFFFFFFF) & ~RGXFWIF_GPU_UTIL_STATE_MASK)
#define RGXFWIF_GPU_UTIL_TIME_MASK32     (IMG_UINT32_C(0xFFFFFFFF) & ~RGXFWIF_GPU_UTIL_STATE_MASK32)

#define RGXFWIF_GPU_UTIL_GET_TIME(word)    ((word) & RGXFWIF_GPU_UTIL_TIME_MASK)
#define RGXFWIF_GPU_UTIL_GET_STATE(word)   ((word) & RGXFWIF_GPU_UTIL_STATE_MASK)
#define RGXFWIF_GPU_UTIL_GET_TIME32(word)  ((IMG_UINT32)(word) & RGXFWIF_GPU_UTIL_TIME_MASK32)
#define RGXFWIF_GPU_UTIL_GET_STATE32(word) ((IMG_UINT32)(word) & RGXFWIF_GPU_UTIL_STATE_MASK32)

/* The OS timestamps computed by the FW are approximations of the real time,
 * which means they could be slightly behind or ahead the real timer on the Host.
 * In some cases we can perform subtractions between FW approximated
 * timestamps and real OS timestamps, so we need a form of protection against
 * negative results if for instance the FW one is a bit ahead of time.
 */
#define RGXFWIF_GPU_UTIL_GET_PERIOD(newtime,oldtime) \
	(((newtime) > (oldtime)) ? ((newtime) - (oldtime)) : 0U)

#define RGXFWIF_GPU_UTIL_MAKE_WORD(time,state) \
	(RGXFWIF_GPU_UTIL_GET_TIME(time) | RGXFWIF_GPU_UTIL_GET_STATE(state))

#define RGXFWIF_GPU_UTIL_MAKE_WORD32(time,state) \
	(RGXFWIF_GPU_UTIL_GET_TIME32(time) | RGXFWIF_GPU_UTIL_GET_STATE32(state))


/* The timer correlation array must be big enough to ensure old entries won't be
 * overwritten before all the HWPerf events linked to those entries are processed
 * by the MISR. The update frequency of this array depends on how fast the system
 * can change state (basically how small the APM latency is) and perform DVFS transitions.
 *
 * The minimum size is 2 (not 1) to avoid race conditions between the FW reading
 * an entry while the Host is updating it. With 2 entries in the worst case the FW
 * will read old data, which is still quite ok if the Host is updating the timer
 * correlation at that time.
 */
#define RGXFWIF_TIME_CORR_ARRAY_SIZE            256U
#define RGXFWIF_TIME_CORR_CURR_INDEX(seqcount)  ((seqcount) % RGXFWIF_TIME_CORR_ARRAY_SIZE)

/* Make sure the timer correlation array size is a power of 2 */
static_assert((RGXFWIF_TIME_CORR_ARRAY_SIZE & (RGXFWIF_TIME_CORR_ARRAY_SIZE - 1U)) == 0U,
			  "RGXFWIF_TIME_CORR_ARRAY_SIZE must be a power of two");

/* The time is stored in DM state counters in "approximately microseconds",
 * dividing the time originally obtained in nanoseconds by 2^10 for the sake of reducing coremem usage */
#define RGXFWIF_DM_OS_TIMESTAMP_SHIFT    10U

typedef struct
{
	RGXFWIF_TIME_CORR      sTimeCorr[RGXFWIF_TIME_CORR_ARRAY_SIZE];
	IMG_UINT32             ui32TimeCorrSeqCount;

	/* Compatibility and other flags */
	IMG_UINT32             ui32GpuUtilFlags;

	/* Last GPU state + OS time of the last state update */
	IMG_UINT64 RGXFW_ALIGN ui64GpuLastWord;
	/* Counters for the amount of time the GPU was active/idle/blocked */
	IMG_UINT64 RGXFW_ALIGN aui64GpuStatsCounters[RGXFWIF_GPU_UTIL_STATE_NUM];

	/* Last GPU DM per-OS states + OS time of the last state update */
	IMG_UINT32 RGXFW_ALIGN aaui32DMOSLastWord[RGXFWIF_GPU_UTIL_DM_MAX][RGXFW_MAX_NUM_OSIDS];
	/* DMs time-stamps are cached in coremem - to reduce coremem usage we allocate 32 bits for each of them
	 * and save their values divided by 2^10, so they wrap around in ~73 mins, consequently
	 * we keep the count of the wrapping around instances */
	IMG_UINT32 RGXFW_ALIGN aaui32DMOSLastWordWrap[RGXFWIF_GPU_UTIL_DM_MAX][RGXFW_MAX_NUM_OSIDS];
	/* Counters for the amount of time the GPU DMs were active or inactive(idle or blocked) */
	IMG_UINT32 RGXFW_ALIGN aaaui32DMOSStatsCounters[RGXFWIF_GPU_UTIL_DM_MAX][RGXFW_MAX_NUM_OSIDS][RGXFWIF_GPU_UTIL_REDUCED_STATES_NUM];
	/* DMs Counters are cached in coremem - to reduce coremem usage we allocate 32 bits for each of them
	 * and save their values divided by 2^10, so they wrap around in ~73 mins, consequently
	 * we keep the count of the wrapping around instances */
	IMG_UINT32 aaaui32DMOSCountersWrap[RGXFWIF_GPU_UTIL_DM_MAX][RGXFW_MAX_NUM_OSIDS][RGXFWIF_GPU_UTIL_REDUCED_STATES_NUM];
} UNCACHED_ALIGN RGXFWIF_GPU_UTIL_FW;

#define RGXFWIF_MAX_NUM_CANCEL_REQUESTS 	(8U)		/* Maximum number of workload cancellation requests */

typedef struct
{
	IMG_UINT32				aui32FirstIntJobRefToCancel[RGXFWIF_MAX_NUM_CANCEL_REQUESTS];	/*!< Saved values of the beginning of range of IntJobRefs at and above which workloads will be discarded */
	IMG_UINT32				aui32FirstValidIntJobRef[RGXFWIF_MAX_NUM_CANCEL_REQUESTS];	/*!< Saved values of the end of range of IntJobRef below which workloads will be discarded */
	IMG_BOOL				bCancelRangesActive;	/*!< True if any active ranges in aui32FirstIntJobRefToCancel and aui32FirstValidIntJobRef arrays */
} UNCACHED_ALIGN RGXFWIF_CANCEL_RANGES_FW;

#define RGXFW_SCRATCH_BUF_SIZE (32768U)

#define RGX_NUM_CORES (8U)

#define RGXFWIF_KM_GENERAL_HEAP_TDM_SECURE_QUEUE_OFFSET_BYTES      RGX_HEAP_KM_GENERAL_RESERVED_REGION_OFFSET
#define RGXFWIF_KM_GENERAL_HEAP_TDM_SECURE_QUEUE_MAX_SIZE_BYTES    2048U

#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_QUEUE_OFFSET_BYTES      (RGXFWIF_KM_GENERAL_HEAP_TDM_SECURE_QUEUE_OFFSET_BYTES + RGXFWIF_KM_GENERAL_HEAP_TDM_SECURE_QUEUE_MAX_SIZE_BYTES)
#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_QUEUE_MAX_SIZE_BYTES    2048U

#define RGXFWIF_KM_GENERAL_HEAP_3D_SECURE_IPP_BUF_OFFSET_BYTES     (RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_QUEUE_OFFSET_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_QUEUE_MAX_SIZE_BYTES)
#define RGXFWIF_KM_GENERAL_HEAP_3D_SECURE_IPP_BUF_MAX_SIZE_BYTES   8192U

#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_OFFSET_BYTES     (RGXFWIF_KM_GENERAL_HEAP_3D_SECURE_IPP_BUF_OFFSET_BYTES + RGXFWIF_KM_GENERAL_HEAP_3D_SECURE_IPP_BUF_MAX_SIZE_BYTES)
#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_MAX_SIZE_BYTES   (0x00004000U * RGX_NUM_CORES + 48U + 1023U)

#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_B_BUF_OFFSET_BYTES   (RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_OFFSET_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_MAX_SIZE_BYTES)
#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_B_BUF_MAX_SIZE_BYTES RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_MAX_SIZE_BYTES

#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_CONTEXT_OFFSET_BYTES    (RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_B_BUF_OFFSET_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_B_BUF_MAX_SIZE_BYTES)
#define RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_CONTEXT_MAX_SIZE_BYTES  (((0x00000080U + 127U) * RGX_NUM_CORES) + 127U)

#if defined(SUPPORT_TRUSTED_DEVICE)
#define RGXFWIF_KM_GENERAL_HEAP_RESERVED_SIZE                      PVR_ALIGN((RGXFWIF_KM_GENERAL_HEAP_TDM_SECURE_QUEUE_MAX_SIZE_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_QUEUE_MAX_SIZE_BYTES + RGXFWIF_KM_GENERAL_HEAP_3D_SECURE_IPP_BUF_MAX_SIZE_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_MAX_SIZE_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_B_BUF_MAX_SIZE_BYTES + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_CONTEXT_MAX_SIZE_BYTES), DEVMEM_HEAP_RESERVED_SIZE_GRANULARITY)
#else
#define RGXFWIF_KM_GENERAL_HEAP_RESERVED_SIZE                      0
#endif

#define RGXFWIF_TDM_SECURE_QUEUE_VADDR                             (RGX_GENERAL_HEAP_BASE + RGXFWIF_KM_GENERAL_HEAP_TDM_SECURE_QUEUE_OFFSET_BYTES)
#define RGXFWIF_CDM_SECURE_QUEUE_VADDR                             (RGX_GENERAL_HEAP_BASE + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_QUEUE_OFFSET_BYTES)
#define RGXFWIF_3D_SECURE_IPP_BUF_VADDR                            (RGX_GENERAL_HEAP_BASE + RGXFWIF_KM_GENERAL_HEAP_3D_SECURE_IPP_BUF_OFFSET_BYTES)
#define RGXFWIF_CDM_SECURE_SR_BUF_VADDR                            PVR_ALIGN((RGX_GENERAL_HEAP_BASE + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_BUF_OFFSET_BYTES), 1024ULL)
#define RGXFWIF_CDM_SECURE_SR_B_BUF_VADDR                          PVR_ALIGN((RGX_GENERAL_HEAP_BASE + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_SR_B_BUF_OFFSET_BYTES), 1024ULL)
#define RGXFWIF_CDM_SECURE_CONTEXT_STATE_VADDR                     PVR_ALIGN((RGX_GENERAL_HEAP_BASE + RGXFWIF_KM_GENERAL_HEAP_CDM_SECURE_CONTEXT_OFFSET_BYTES), 128ULL)

/*!
 ******************************************************************************
 * Virtualisation and Security
 *****************************************************************************/

#define RGX_GP_SUPPORTS_SECURITY		(0U)
#define RGX_TDM_SUPPORTS_SECURITY		(1U)
#define RGX_GEOM_SUPPORTS_SECURITY		(0U)
#define RGX_FRAG_SUPPORTS_SECURITY		(1U)
#define RGX_COMPUTE_SUPPORTS_SECURITY	(1U)
#define RGX_RAY_SUPPORTS_SECURITY		(0U)

#define RGX_DMS_WITH_SECURITY_COUNT		(RGX_GP_SUPPORTS_SECURITY		+ \
										 RGX_TDM_SUPPORTS_SECURITY		+ \
										 RGX_GEOM_SUPPORTS_SECURITY		+ \
										 RGX_FRAG_SUPPORTS_SECURITY		+ \
										 RGX_COMPUTE_SUPPORTS_SECURITY	+ \
										 RGX_RAY_SUPPORTS_SECURITY)

#if defined(SUPPORT_TRUSTED_DEVICE)
#if (RGX_FW_HEAP_OSID_ASSIGNMENT == RGX_FW_HEAP_USES_DEDICATED_OSID) || (defined(RGX_LEGACY_SECURE_OSID_SCHEME))
/* OSIDs 0 and 1 reserved for Firmware */
#define DRIVER_OSID_START_OFFSET		(FW_OSID+2)
#else
/* OSIDs 1 reserved for Firmware */
#define DRIVER_OSID_START_OFFSET		(FW_OSID+1)
#endif

#else
/* Firmware and Host driver share the same OSID */
#define DRIVER_OSID_START_OFFSET		(FW_OSID)
#endif /* defined(SUPPORT_TRUSTED_DEVICE) */

#if (RGX_FW_HEAP_OSID_ASSIGNMENT == RGX_FW_HEAP_USES_DEDICATED_OSID)
/* Firmware heaps reside in a dedicated non-secure IPA space. */
#define FW_HEAP_OSID					(FW_OSID+1)
#elif (RGX_FW_HEAP_OSID_ASSIGNMENT == RGX_FW_HEAP_USES_HOST_OSID)
/* Firmware heaps reside in the Host driver's non-secure IPA space. */
#define FW_HEAP_OSID					(DRIVER_OSID_START_OFFSET)
#elif (RGX_FW_HEAP_OSID_ASSIGNMENT == RGX_FW_HEAP_USES_FIRMWARE_OSID)
/* Firmware heaps reside in the IPA space as the Firmware. */
#define FW_HEAP_OSID					(FW_OSID)
#else
#error "RGX_FW_HEAP_OSID_ASSIGNMENT not configured correctly."
#endif

#if !defined(RGX_NUM_DRIVERS_SUPPORTED) && !defined(__KERNEL__)
/* placeholder define for UM tools including this header */
#define RGX_NUM_DRIVERS_SUPPORTED (1U)
#endif

#if defined(SUPPORT_TRUSTED_DEVICE)

#if defined(RGX_LEGACY_SECURE_OSID_SCHEME)
/* keep reverse compatibility with original scheme */
static_assert((RGX_NUM_DRIVERS_SUPPORTED == 1),
			  "RGX_LEGACY_SECURE_OSID_SCHEME only applicable on native drivers (no VZ support)");

#define DRIVER_ID(osid)					(0U)
#define OSID_SECURE(did)				(1U)
#define OSID(did)						(2U)

#else /* defined(RGX_LEGACY_SECURE_OSID_SCHEME) */
/* native and virtualized security support */
#define DRIVER_ID(osid)					(osid - DRIVER_OSID_START_OFFSET)
#define OSID(did)						(did + DRIVER_OSID_START_OFFSET)
#define OSID_SECURE(did)				(RGXFW_MAX_NUM_OSIDS - 1 - \
			(did==0 ? (0) : \
			(did==1 ? (DRIVER0_SECURITY_SUPPORT) : \
			(did==2 ? (DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT) : \
			(did==3 ? (DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT + DRIVER2_SECURITY_SUPPORT) : \
			(did==4 ? (DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT + DRIVER2_SECURITY_SUPPORT + DRIVER3_SECURITY_SUPPORT) : \
			(did==5 ? (DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT + DRIVER2_SECURITY_SUPPORT + DRIVER3_SECURITY_SUPPORT + DRIVER4_SECURITY_SUPPORT) : \
			(did==6 ? (DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT + DRIVER2_SECURITY_SUPPORT + DRIVER3_SECURITY_SUPPORT + DRIVER4_SECURITY_SUPPORT + DRIVER5_SECURITY_SUPPORT) : \
			          (DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT + DRIVER2_SECURITY_SUPPORT + DRIVER3_SECURITY_SUPPORT + DRIVER4_SECURITY_SUPPORT + DRIVER5_SECURITY_SUPPORT + DRIVER6_SECURITY_SUPPORT)))))))))

static_assert((RGX_NUM_DRIVERS_SUPPORTED + DRIVER_OSID_START_OFFSET +
				DRIVER0_SECURITY_SUPPORT + DRIVER1_SECURITY_SUPPORT + DRIVER2_SECURITY_SUPPORT + DRIVER3_SECURITY_SUPPORT +
				DRIVER4_SECURITY_SUPPORT + DRIVER5_SECURITY_SUPPORT + DRIVER6_SECURITY_SUPPORT + DRIVER7_SECURITY_SUPPORT) <= RGXFW_MAX_NUM_OSIDS,
				"The GPU hardware is not equipped with enough hardware OSIDs to satisfy the requirements.");
#endif /* defined(RGX_LEGACY_SECURE_OSID_SCHEME) */

/* Time slice support */
/* Bits 30 and 31 reserved by FW private driver priority */
#define RGXFW_VZ_PRIORITY_MAX_SHIFT		(30U)
#define RGXFW_VZ_PRIORITY_MASK			((1U << RGXFW_VZ_PRIORITY_MAX_SHIFT) - 1U)
#define RGXFW_VZ_TIME_SLICE_MAX			(100U)
#define RGXFW_VZ_TIME_SLICE_MIN			(5U)

#elif defined(RGX_NUM_DRIVERS_SUPPORTED) && (RGX_NUM_DRIVERS_SUPPORTED > 1)
/* virtualization without security support */
#define DRIVER_ID(osid)					(osid - DRIVER_OSID_START_OFFSET)
#define OSID(did)						(did + DRIVER_OSID_START_OFFSET)

/* Time slice support */
/* Bits 30 and 31 reserved by FW private driver priority */
#define RGXFW_VZ_PRIORITY_MAX_SHIFT		(30U)
#define RGXFW_VZ_PRIORITY_MASK			((1U << RGXFW_VZ_PRIORITY_MAX_SHIFT) - 1U)
#define RGXFW_VZ_TIME_SLICE_MAX			(100U)
#define RGXFW_VZ_TIME_SLICE_MIN			(5U)
#else
/* native without security support */
#define DRIVER_ID(osid)					(0U)
#define OSID(did)						(did)
#define RGXFW_VZ_PRIORITY_MASK			(0U)
#endif /* defined(SUPPORT_TRUSTED_DEVICE) */

#if defined(RGX_NUM_DRIVERS_SUPPORTED) && (RGX_NUM_DRIVERS_SUPPORTED > 1)

#define FOREACH_SUPPORTED_DRIVER(did)               for ((did)=RGXFW_HOST_DRIVER_ID; (did) < RGX_NUM_DRIVERS_SUPPORTED; (did)++)

#if defined(__KERNEL__)
/* Driver implementation */
#define FOREACH_ACTIVE_DRIVER(devinfo, did)        FOREACH_SUPPORTED_DRIVER(did)                                \
                                                   {                                                            \
                                                   if (devinfo->psRGXFWIfFwSysData->asOsRuntimeFlagsMirror[did].bfOsState != RGXFW_CONNECTION_FW_ACTIVE) continue;

#define END_FOREACH_ACTIVE_DRIVER                  }

#else
/* Firmware implementation */
#define FOREACH_ACTIVE_DRIVER(did)                 do {                                                                      \
                                                   unsigned int idx;                                                         \
                                                   for ((idx)=RGXFW_HOST_DRIVER_ID, (did)=gsRGXFWCtl.aui32ActiveDrivers[0U]; \
                                                        (idx) < RGXFW_NUM_ACTIVE_DRIVERS;                                         \
                                                        ++(idx), (did)=gsRGXFWCtl.aui32ActiveDrivers[(idx)])  {

#define END_FOREACH_ACTIVE_DRIVER                    }} while (false);
#endif /* defined(__KERNEL__) */


#else
#define FOREACH_SUPPORTED_DRIVER(did)               for ((did)=RGXFW_HOST_DRIVER_ID; (did) <= RGXFW_HOST_DRIVER_ID; (did)++)

#define FOREACH_ACTIVE_DRIVER(did)                  FOREACH_SUPPORTED_DRIVER(did)
#define END_FOREACH_ACTIVE_DRIVER

#endif /* (RGX_NUM_DRIVERS_SUPPORTED > 1) */

#define FOREACH_VALIDATION_OSID(osid)              for ((osid)=0; (osid) < GPUVIRT_VALIDATION_NUM_OS; (osid)++)
#define FOREACH_HW_OSID(osid)                      for ((osid)=0; (osid) < RGXFW_MAX_NUM_OSIDS; (osid)++)
#define FOREACH_DRIVER_RAW_HEAP(did)               for ((did)=RGX_FIRST_RAW_HEAP_DRIVER_ID; (did) < ((PVRSRV_VZ_MODE_IS(NATIVE) ? 1 : RGX_NUM_DRIVERS_SUPPORTED)); (did)++)

#endif /* RGX_FWIF_KM_H */

/******************************************************************************
 End of file (rgx_fwif_km.h)
******************************************************************************/
