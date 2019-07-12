override DISPLAY_CONTROLLER := drm_pdp
override LMA := 1
override METAG_VERSION_NEEDED := 2.8.1.0.3
override MIPS_VERSION_NEEDED := 2014.07-1
override PDVFS_COM := PDVFS_COM_HOST
override PDVFS_COM_AP := 2
override PDVFS_COM_HOST := 1
override PDVFS_COM_PMC := 3
override PVRSRV_MODNAME := pvrsrvkm
override PVRSYNC_MODNAME := pvr_sync
override PVR_BUILD_DIR := tc_linux
override PVR_GPIO_MODE := PVR_GPIO_MODE_GENERAL
override PVR_GPIO_MODE_GENERAL := 1
override PVR_GPIO_MODE_POWMON_PIN := 2
override PVR_GPIO_MODE_POWMON_WO_PIN := 3
override PVR_HANDLE_BACKEND := idr
override PVR_SYSTEM := rgx_linux_tc
override PVR_USE_FENCE_SYNC_MODEL := 1
override RGX_TIMECORR_CLOCK := mono
override SUPPORT_BUFFER_SYNC := 1
override SUPPORT_DMA_FENCE := 1
override SUPPORT_GPUTRACE_EVENTS := 1
override SUPPORT_NATIVE_FENCE_SYNC := 1
override SUPPORT_RGX := 1
override TARGET_OS :=
override VMM_TYPE := stub
override undefine SUPPORT_DISPLAY_CLASS
ifeq ($(CONFIG_DRM_POWERVR_ROGUE_DEBUG),y)
override BUILD := debug
override PVR_BUILD_TYPE := debug
override PVR_RI_DEBUG := 1
override SUPPORT_DEVICEMEMHISTORY_BRIDGE := 1
override SUPPORT_PAGE_FAULT_DEBUG := 1
else
override BUILD := release
override PVR_BUILD_TYPE := release
endif
