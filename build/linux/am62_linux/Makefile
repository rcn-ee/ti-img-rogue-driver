########################################################################### ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Strictly Confidential.
### ###########################################################################

include ../config/preconfig.mk
include ../config/window_system.mk

ifneq ($(SUPPORT_KMS),1)
$(error This platform only supports window systems that use the standard Linux driver model.)
endif

RGX_BVNC := 33.15.11.3
PVR_SYSTEM := axe_am62
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

# Should be last
include ../config/core.mk
include ../common/lws.mk
