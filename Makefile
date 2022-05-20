########################################################################### ###
#@Copyright     Copyright (c) Imagination Technologies Ltd. All Rights Reserved
#@License       Dual MIT/GPLv2
#
# The contents of this file are subject to the MIT license as set out below.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# Alternatively, the contents of this file may be used under the terms of
# the GNU General Public License Version 2 ("GPL") in which case the provisions
# of GPL are applicable instead of those above.
#
# If you wish to allow use of your version of this file only under the terms of
# GPL, and not to allow others to use your version of this file under the terms
# of the MIT license, indicate your decision by deleting the provisions above
# and replace them with the notice and other provisions required by GPL as set
# out in the file called "GPL-COPYING" included in this distribution. If you do
# not delete the provisions above, a recipient may use your version of this file
# under the terms of either the MIT license or GPL.
#
# This License is also included in this distribution in the file called
# "MIT-COPYING".
#
# EXCEPT AS OTHERWISE STATED IN A NEGOTIATED AGREEMENT: (A) THE SOFTWARE IS
# PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
# BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT; AND (B) IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
### ###########################################################################

obj-m += pvrsrvkm.o


pvrsrvkm-y += \
	generated/rogue/cache_bridge/server_cache_bridge.o \
	generated/rogue/cache_bridge/client_cache_direct_bridge.o \
	generated/rogue/cmm_bridge/server_cmm_bridge.o \
	generated/rogue/devicememhistory_bridge/server_devicememhistory_bridge.o \
	generated/rogue/devicememhistory_bridge/client_devicememhistory_direct_bridge.o \
	generated/rogue/di_bridge/server_di_bridge.o \
	generated/rogue/dmabuf_bridge/server_dmabuf_bridge.o \
	generated/rogue/htbuffer_bridge/client_htbuffer_direct_bridge.o \
	generated/rogue/htbuffer_bridge/server_htbuffer_bridge.o \
	generated/rogue/mm_bridge/client_mm_direct_bridge.o \
	generated/rogue/mm_bridge/server_mm_bridge.o \
	generated/rogue/pvrtl_bridge/client_pvrtl_direct_bridge.o \
	generated/rogue/pvrtl_bridge/server_pvrtl_bridge.o \
	generated/rogue/rgxbreakpoint_bridge/server_rgxbreakpoint_bridge.o \
	generated/rogue/rgxcmp_bridge/server_rgxcmp_bridge.o \
	generated/rogue/rgxfwdbg_bridge/server_rgxfwdbg_bridge.o \
	generated/rogue/rgxhwperf_bridge/server_rgxhwperf_bridge.o \
	generated/rogue/rgxkicksync_bridge/server_rgxkicksync_bridge.o \
	generated/rogue/rgxregconfig_bridge/server_rgxregconfig_bridge.o \
	generated/rogue/rgxta3d_bridge/server_rgxta3d_bridge.o \
	generated/rogue/rgxtimerquery_bridge/server_rgxtimerquery_bridge.o \
	generated/rogue/rgxtq2_bridge/server_rgxtq2_bridge.o \
	generated/rogue/rgxtq_bridge/server_rgxtq_bridge.o \
	generated/rogue/srvcore_bridge/server_srvcore_bridge.o \
	generated/rogue/sync_bridge/client_sync_direct_bridge.o \
	generated/rogue/sync_bridge/server_sync_bridge.o \
	generated/rogue/synctracking_bridge/client_synctracking_direct_bridge.o \
	generated/rogue/synctracking_bridge/server_synctracking_bridge.o \
	services/server/common/cache_km.o \
	services/server/common/connection_server.o \
	services/server/common/debug_common.o \
	services/server/common/devicemem_heapcfg.o \
	services/server/common/devicemem_history_server.o \
	services/server/common/devicemem_server.o \
	services/server/common/di_impl_brg.o \
	services/server/common/di_server.o \
	services/server/common/handle.o \
	services/server/common/htb_debug.o \
	services/server/common/htbserver.o \
	services/server/common/info_page_km.o \
	services/server/common/lists.o \
	services/server/common/mmu_common.o \
	services/server/common/physheap.o \
	services/server/common/physmem.o \
	services/server/common/physmem_hostmem.o \
	services/server/common/physmem_lma.o \
	services/server/common/pmr.o \
	services/server/common/power.o \
	services/server/common/process_stats.o \
	services/server/common/pvr_notifier.o \
	services/server/common/pvrsrv.o \
	services/server/common/pvrsrv_bridge_init.o \
	services/server/common/pvrsrv_pool.o \
	services/server/common/srvcore.o \
	services/server/common/sync_checkpoint.o \
	services/server/common/sync_server.o \
	services/server/common/tlintern.o \
	services/server/common/tlserver.o \
	services/server/common/tlstream.o \
	services/server/common/vmm_pvz_client.o \
	services/server/common/vmm_pvz_server.o \
	services/server/common/vz_vmm_pvz.o \
	services/server/common/vz_vmm_vm.o \
	services/server/devices/rgx_bridge_init.o \
	services/server/devices/rgxbreakpoint.o \
	services/server/devices/rgxbvnc.o \
	services/server/devices/rgxccb.o \
	services/server/devices/rgxcompute.o \
	services/server/devices/rgxfwdbg.o \
	services/server/devices/rgxfwimageutils.o \
	services/server/devices/rgxfwtrace_strings.o \
	services/server/devices/rgxhwperf_common.o \
	services/server/devices/rgxkicksync.o \
	services/server/devices/rgxmem.o \
	services/server/devices/rgxregconfig.o \
	services/server/devices/rgxshader.o \
	services/server/devices/rgxsyncutils.o \
	services/server/devices/rgxtdmtransfer.o \
	services/server/devices/rgxtimecorr.o \
	services/server/devices/rgxtimerquery.o \
	services/server/devices/rgxutils.o \
	services/server/devices/rogue/rgxdebug.o \
	services/server/devices/rogue/rgxfwutils.o \
	services/server/devices/rogue/rgxhwperf.o \
	services/server/devices/rogue/rgxinit.o \
	services/server/devices/rogue/rgxlayer_impl.o \
	services/server/devices/rogue/rgxmipsmmuinit.o \
	services/server/devices/rogue/rgxmmuinit.o \
	services/server/devices/rogue/rgxmulticore.o \
	services/server/devices/rogue/rgxpower.o \
	services/server/devices/rogue/rgxsrvinit.o \
	services/server/devices/rogue/rgxstartstop.o \
	services/server/devices/rogue/rgxta3d.o \
	services/server/devices/rogue/rgxtransfer.o \
	services/server/env/linux/allocmem.o \
	services/server/env/linux/event.o \
	services/server/env/linux/fwload.o \
	services/server/env/linux/handle_idr.o \
	services/server/env/linux/km_apphint.o \
	services/server/env/linux/module_common.o \
	services/server/env/linux/osconnection_server.o \
	services/server/env/linux/osfunc.o \
	services/server/env/linux/osmmap_stub.o \
	services/server/env/linux/physmem_dmabuf.o \
	services/server/env/linux/physmem_osmem_linux.o \
	services/server/env/linux/physmem_test.o \
	services/server/env/linux/pmr_os.o \
	services/server/env/linux/pvr_bridge_k.o \
	services/server/env/linux/pvr_counting_timeline.o \
	services/server/env/linux/pvr_debug.o \
	services/server/env/linux/pvr_drm.o \
	services/server/env/linux/pvr_fence.o \
	services/server/env/linux/pvr_gputrace.o \
	services/server/env/linux/pvr_platform_drv.o \
	services/server/env/linux/pvr_procfs.o \
	services/server/env/linux/pvr_sw_fence.o \
	services/server/env/linux/pvr_sync_file.o \
	services/server/env/linux/pvr_sync_ioctl_common.o \
	services/server/env/linux/pvr_sync_ioctl_dev.o \
	services/shared/common/devicemem.o \
	services/shared/common/devicemem_utils.o \
	services/shared/common/hash.o \
	services/shared/common/htbuffer.o \
	services/shared/common/mem_utils.o \
	services/shared/common/pvrsrv_error.o \
	services/shared/common/ra.o \
	services/shared/common/sync.o \
	services/shared/common/tlclient.o \
	services/shared/common/uniq_key_splay_tree.o \
	services/shared/devices/rogue/rgx_hwperf_table.o \
	services/system/common/env/linux/interrupt_support.o \
	services/system/common/sysconfig_cmn.o \
	services/system/rogue/common/env/linux/dma_support.o \
	services/system/rogue/common/vmm_type_stub.o

pvrsrvkm-y += \
	services/system/rogue/axe_am62/sysconfig.o

pvrsrvkm-$(CONFIG_ARM)   += services/server/env/linux/osfunc_arm.o
pvrsrvkm-$(CONFIG_ARM64) += services/server/env/linux/osfunc_arm64.o
pvrsrvkm-$(CONFIG_EVENT_TRACING) += services/server/env/linux/trace_events.o
pvrsrvkm-$(CONFIG_X86)   += services/server/env/linux/osfunc_x86.o


ccflags-y += \
 -include $(srctree)/../imgtech-module/include/config_kernel_am62_android.h \
 -I $(srctree)/../imgtech-module/ \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxcmp_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/cache_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/cmm_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/devicememhistory_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/di_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/mm_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/dmabuf_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/htbuffer_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/pvrtl_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxbreakpoint_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxfwdbg_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxhwperf_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxkicksync_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxregconfig_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxta3d_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxtq2_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxtq_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxsignals_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/rgxtimerquery_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/srvcore_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/sync_bridge \
 -I $(srctree)/../imgtech-module/generated/rogue/synctracking_bridge \
 -I $(srctree)/../imgtech-module/kernel/drivers/staging/imgtec \
 -I $(srctree)/../imgtech-module/include \
 -I $(srctree)/../imgtech-module/include/drm \
 -I $(srctree)/../imgtech-module/include/rogue \
 -I $(srctree)/../imgtech-module/include/public \
 -I $(srctree)/../imgtech-module/include/system \
 -I $(srctree)/../imgtech-module/services/include \
 -I $(srctree)/../imgtech-module/services/include/env/linux \
 -I $(srctree)/../imgtech-module/services/include/rogue \
 -I $(srctree)/../imgtech-module/services/server/include \
 -I $(srctree)/../imgtech-module/services/server/env/linux/ \
 -I $(srctree)/../imgtech-module/services/server/common/ \
 -I $(srctree)/../imgtech-module/services/server/devices/ \
 -I $(srctree)/../imgtech-module/services/server/devices/rogue/ \
 -I $(srctree)/../imgtech-module/services/shared/common \
 -I $(srctree)/../imgtech-module/services/shared/devices/rogue \
 -I $(srctree)/../imgtech-module/services/shared/include \
 -I $(srctree)/../imgtech-module/services/system/include \
 -I $(srctree)/../imgtech-module/services/system/rogue/axe_am62 \
 -I $(srctree)/../imgtech-module/services/system/rogue/include \
 -I $(srctree)/../imgtech-module/hwdefs/rogue/km \
 -I $(srctree)/../imgtech-module/hwdefs/rogue \
 -I $(srctree)/include/uapi/linux \
 -I $(srctree)/include/linux \
 -I $(srctree)/include/drm \
 -I $(srctree)/include/ \
 -I $(srctree)/arch/arm64/include/asm/ \
 -I $(srctree)/arch/arm/include/asm/ \
 -D__linux__

ccflags-y += \
	-Wno-missing-field-initializers \
	-Wdeclaration-after-statement \
	-Wno-format-zero-length \
	-Wmissing-prototypes \
	-Wstrict-prototypes \
	-Wno-unused-parameter \
	-Wno-sign-compare \
	-Wno-type-limits \
	-Wno-error \
	-Wno-typedef-redefinition


KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build


 # we get the symbols from modules using KBUILD_EXTRA_SYMBOLS to prevent warnings about unknown functions
all:
	$(Q)$(MAKE) -C $(KERNEL_SRC)  M=$(M)

modules_install:
	$(Q)$(MAKE) -C $(KERNEL_SRC) M=$(M) modules_install

clean:
	$(Q)$(MAKE) -C $(KERNEL_SRC) M=$(M) clean

FORCE:
.PHONY: FORCE
