########################################################################### ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Strictly Confidential.
### ###########################################################################

RGX_BVNC := 33.15.11.3
RGX_FW_SIGNED   := 0

ifeq ($(TARGET_DEVICE),)
override TARGET_DEVICE := am62x
endif

PVR_ARCH := rogue
include ../config/preconfig.mk

PVR_SYSTEM := axe_am62


include ../common/android/arch.mk
include ../common/android/features.mk

NO_HARDWARE := 0
PVR_LDM_PLATFORM_PRE_REGISTERED := 1

SUPPORT_NATIVE_FENCE_SYNC := 1
SUPPORT_MIPS_64K_PAGE_SIZE := 1

KERNEL_COMPONENTS := srvkm

ifeq ($(PVR_REMVIEW),1)
 DISPLAY_CONTROLLER := drm_nulldisp
 KERNEL_COMPONENTS += $(DISPLAY_CONTROLLER)
 PVR_DRM_MODESET_DRIVER_NAME := nulldisp
else
 PVR_DRM_MODESET_DRIVER_NAME := tidss
 PVR_DRM_MODESET_MODULE_NAME := dumb
endif

ifeq ($(call kernel-version-at-least,5,10),true)
override SUPPORT_ION := 0
SUPPORT_DMA_HEAP := 1
override PVRSRV_ENABLE_PVR_ION_STATS := 0
override PVR_ANDROID_ION_FBCDC_ALLOC := 0
override SUPPORT_ION := 0
endif
# Should be last
include ../config/core.mk
include ../common/android/extra_config.mk
include ../common/3rdparty.mk
