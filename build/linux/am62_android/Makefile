########################################################################### ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Strictly Confidential.
### ###########################################################################


ifeq ($(TARGET_DEVICE),)
override TARGET_DEVICE := am62x
endif

PVR_ARCH := rogue
PVR_SYSTEM := axe_am62

PVRSRV_NEED_PVR_DPF := 1
PVRSRV_NEED_PVR_ASSERT := 1
PVR_SERVICES_DEBUG := 1
SUPPORT_KMS := 1

PVR_DRM_MODESET_DRIVER_NAME := tidss
# BVNC is fixed for AXE1-16 dont allow changes
RGX_BVNC := 33.15.11.3
include ../config/preconfig.mk


include ../common/android/arch.mk
include ../common/android/features.mk

ION_DEFAULT_HEAP_NAME := \"reserved\"

KERNEL_COMPONENTS := srvkm $(DISPLAY_CONTROLLER)

HWR_DEFAULT_ENABLED := 0

# Should be last
include ../config/core.mk
include ../common/android/extra_config.mk
include ../common/3rdparty.mk

$(eval $(call TunableUserConfigC,SGLTRACE,))
