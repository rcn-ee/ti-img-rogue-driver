########################################################################### ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Strictly Confidential.
### ###########################################################################

include ../config/preconfig.mk
include ../config/window_system.mk

PVR_SYSTEM := axe_am62

# BVNC is fixed for AXE1-16 dont allow changes
RGX_BVNC := 33.15.11.3
WINDOW_SYSTEM ?= nullws

PVRSRV_NEED_PVR_DPF := 1
PVRSRV_NEED_PVR_ASSERT := 1
PVR_SERVICES_DEBUG := 1

SUPPORT_MIPS_64K_PAGE_SIZE := 1
SUPPORT_NATIVE_FENCE_SYNC := 1

# Virtualization Flags
#PVRSRV_VZ_NUM_OSID := 1
#VMM_TYPE := "jailhouse"
#RGX_FW_HEAP_SHIFT := 24

ifeq ($(SUPPORT_KMS),1)
 PVR_DRM_MODESET_DRIVER_NAME := tidss
else
 ifeq ($(SUPPORT_DISPLAY_CLASS),1)
  DISPLAY_CONTROLLER := dc_example
 endif
endif

KERNEL_COMPONENTS := srvkm $(DISPLAY_CONTROLLER)

HWR_DEFAULT_ENABLED := 0

# Should be last
include ../config/core.mk
-include ../common/lws.mk
include ../common/3rdparty.mk

$(eval $(call TunableUserConfigC,SGLTRACE,))
