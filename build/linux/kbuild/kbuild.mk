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

$(TARGET_PRIMARY_OUT)/kbuild/Makefile: $(MAKE_TOP)/kbuild/Makefile.template
	@[ ! -e $(dir $@) ] && mkdir -p $(dir $@) || true
	$(CP) -f $< $@

# Android builds explicitly set KERNEL_CROSS_COMPILE to undef if target platform
# is one of x86-ish, even though CROSS_COMPILE is set correctly. Ignore Android
# in this check.
ifeq ($(SUPPORT_ANDROID_PLATFORM),)
 # This is only relevant for kernel modules as kernel makefiles will be looking
 # at ARCH to deduce what to pass as clang target. We do not use ARCH in
 # userspace modules, so it's not a problem - a host compiler would be used.
 ifneq ($(ARCH),)
  ifeq ($(notdir $(KERNEL_CC)),clang)
   ifeq ($(KERNEL_CROSS_COMPILE),)
    _clang_no_cross_err := 1
    # The difference between KERNEL_CROSS_COMPILE and CROSS_COMPILE in the
    # message is intentional and is not a typo. This is because at the time of
    # change, setting KERNEL_CROSS_COMPILE with clang builds on command line is
    # not supported.
    _clang_no_cross_err_msg := ARCH=$(ARCH) is populated and clang is used, but KERNEL_CROSS_COMPILE is empty. Please populate CROSS_COMPILE if you want to cross compile, or unset ARCH if you want to use the host compiler
   endif
  endif
 endif
endif

# We need to make INTERNAL_KBUILD_MAKEFILES absolute because the files will be
# read while chdir'd into $(KERNELDIR)
INTERNAL_KBUILD_MAKEFILES = $(abspath $(foreach _m,$(KERNEL_COMPONENTS) $(EXTRA_PVRSRVKM_COMPONENTS),$(if $(INTERNAL_KBUILD_MAKEFILE_FOR_$(_m)),$(INTERNAL_KBUILD_MAKEFILE_FOR_$(_m)),$(error Unknown kbuild module "$(_m)"))))
INTERNAL_KBUILD_OBJECTS = $(foreach _m,$(KERNEL_COMPONENTS),$(if $(INTERNAL_KBUILD_OBJECTS_FOR_$(_m)),$(INTERNAL_KBUILD_OBJECTS_FOR_$(_m)),$(error BUG: Unknown kbuild module "$(_m)" should have been caught earlier)))
INTERNAL_EXTRA_KBUILD_OBJECTS = $(foreach _m,$(EXTRA_PVRSRVKM_COMPONENTS),$(if $(INTERNAL_KBUILD_OBJECTS_FOR_$(_m)),$(INTERNAL_KBUILD_OBJECTS_FOR_$(_m)),$(error BUG: Unknown kbuild module "$(_m)" should have been caught earlier)))
.PHONY: kbuild kbuild_clean kbuild_check

kbuild_check:
	@: $(if $(_clang_no_cross_err),$(error $(_clang_no_cross_err_msg)),)
	@: $(if $(strip $(KERNELDIR)),,$(error KERNELDIR must be set))
	@: $(call directory-must-exist,$(KERNELDIR))
	@: $(foreach _m,$(ALL_KBUILD_MODULES),$(if $(wildcard $(abspath $(INTERNAL_KBUILD_MAKEFILE_FOR_$(_m)))),,$(error In makefile $(INTERNAL_MAKEFILE_FOR_MODULE_$(_m)): Module $(_m) requires kbuild makefile $(INTERNAL_KBUILD_MAKEFILE_FOR_$(_m)), which is missing)))
	@: $(if $(filter-out command line override,$(origin build)),,$(error Overriding $$(build) (with "make build=...") will break kbuild))

# Services server headers are generated as part of running the bridge
# generator, which might be included in KM code. So as well as depending on
# the kbuild Makefile, we need to make kbuild also depend on each bridge
# module (including direct bridges), so that 'make kbuild' in a clean tree
# works.
kbuild: kbuild_check $(TARGET_PRIMARY_OUT)/kbuild/Makefile bridges
	$(if $(V),,@)$(MAKE) -Rr --no-print-directory -C $(KERNELDIR) \
		M=$(abspath $(TARGET_PRIMARY_OUT)/kbuild) \
		INTERNAL_KBUILD_MAKEFILES="$(INTERNAL_KBUILD_MAKEFILES)" \
		INTERNAL_KBUILD_OBJECTS="$(INTERNAL_KBUILD_OBJECTS)" \
		INTERNAL_EXTRA_KBUILD_OBJECTS="$(INTERNAL_EXTRA_KBUILD_OBJECTS)" \
		BRIDGE_SOURCE_ROOT=$(abspath $(BRIDGE_SOURCE_ROOT)) \
		TARGET_PRIMARY_ARCH=$(TARGET_PRIMARY_ARCH) \
		PVR_ARCH=$(PVR_ARCH) \
		HWDEFS_DIR=$(HWDEFS_DIR) \
		CLANG_TRIPLE=$(if $(filter %-androideabi,$(CROSS_TRIPLE)),$(patsubst \
		%-androideabi,%-gnueabi,$(CROSS_TRIPLE)),$(patsubst \
		%-android,%-gnu,$(CROSS_TRIPLE)))- \
		CROSS_COMPILE="$(CCACHE) $(KERNEL_CROSS_COMPILE)" \
		EXTRA_CFLAGS="$(ALL_KBUILD_CFLAGS)" \
		CC=$(if $(KERNEL_CC),$(KERNEL_CC),$(KERNEL_CROSS_COMPILE)gcc) \
		AR=$(if $(KERNEL_AR),$(KERNEL_AR),$(KERNEL_CROSS_COMPILE)ar) \
		LD=$(if $(KERNEL_LD),$(KERNEL_LD),$(KERNEL_CROSS_COMPILE)ld) \
		NM=$(if $(KERNEL_NM),$(KERNEL_NM),$(KERNEL_CROSS_COMPILE)nm) \
		OBJCOPY=$(if $(KERNEL_OBJCOPY),$(KERNEL_OBJCOPY),$(KERNEL_CROSS_COMPILE)objcopy) \
		OBJDUMP=$(if $(KERNEL_OBJDUMP),$(KERNEL_OBJDUMP),$(KERNEL_CROSS_COMPILE)objdump) \
		CHECK="$(patsubst @%,%,$(CHECK))" $(if $(CHECK),C=1,) \
		$(if $(KBUILD_CLANG_VERSION),$(shell echo CONFIG_CLANG_VERSION=$(KBUILD_CLANG_VERSION)),) \
		$(if $(KBUILD_LLD_VERSION),$(shell echo CONFIG_LLD_VERSION=$(KBUILD_LLD_VERSION)),) \
		V=$(V) W=$(W) TOP=$(TOP)
	@for kernel_module in $(addprefix $(TARGET_PRIMARY_OUT)/kbuild/,$(INTERNAL_KBUILD_OBJECTS:.o=.ko)); do \
		cp $$kernel_module $(TARGET_PRIMARY_OUT); \
	done
ifeq ($(KERNEL_DEBUGLINK),1)
	@for kernel_module in $(addprefix $(TARGET_PRIMARY_OUT)/,$(INTERNAL_KBUILD_OBJECTS:.o=.ko)); do \
		$(CROSS_COMPILE)objcopy --only-keep-debug $$kernel_module $(basename $$kernel_module).dbg; \
		$(CROSS_COMPILE)strip --strip-debug $$kernel_module; \
		$(if $(V),,echo "  DBGLINK " $(call relative-to-top,$(basename $$kernel_module).dbg)); \
		$(CROSS_COMPILE)objcopy --add-gnu-debuglink=$(basename $$kernel_module).dbg $$kernel_module; \
	done
endif


kbuild_clean: kbuild_check $(TARGET_PRIMARY_OUT)/kbuild/Makefile
	$(if $(V),,@)$(MAKE) -Rr --no-print-directory -C $(KERNELDIR) \
		M=$(abspath $(TARGET_PRIMARY_OUT)/kbuild) \
		INTERNAL_KBUILD_MAKEFILES="$(INTERNAL_KBUILD_MAKEFILES)" \
		INTERNAL_KBUILD_OBJECTS="$(INTERNAL_KBUILD_OBJECTS)" \
		INTERNAL_EXTRA_KBUILD_OBJECTS="$(INTERNAL_EXTRA_KBUILD_OBJECTS)" \
		BRIDGE_SOURCE_ROOT=$(abspath $(BRIDGE_SOURCE_ROOT)) \
		TARGET_PRIMARY_ARCH=$(TARGET_PRIMARY_ARCH) \
		CLANG_TRIPLE=$(if $(filter %-androideabi,$(CROSS_TRIPLE)),$(patsubst \
		%-androideabi,%-gnueabi,$(CROSS_TRIPLE)),$(patsubst \
		%-android,%-gnu,$(CROSS_TRIPLE)))- \
		CROSS_COMPILE="$(CCACHE) $(KERNEL_CROSS_COMPILE)" \
		EXTRA_CFLAGS="$(ALL_KBUILD_CFLAGS)" \
		CC=$(if $(KERNEL_CC),$(KERNEL_CC),$(KERNEL_CROSS_COMPILE)gcc) \
		LD=$(if $(KERNEL_LD),$(KERNEL_LD),$(KERNEL_CROSS_COMPILE)ld) \
		NM=$(if $(KERNEL_NM),$(KERNEL_NM),$(KERNEL_CROSS_COMPILE)nm) \
		OBJCOPY=$(if $(KERNEL_OBJCOPY),$(KERNEL_OBJCOPY),$(KERNEL_CROSS_COMPILE)objcopy) \
		$(if $(KBUILD_CLANG_VERSION),$(shell echo CONFIG_CLANG_VERSION=$(KBUILD_CLANG_VERSION)),) \
		$(if $(KBUILD_LLD_VERSION),$(shell echo CONFIG_LLD_VERSION=$(KBUILD_LLD_VERSION)),) \
		V=$(V) W=$(W) TOP=$(TOP) clean

ifeq ($(PVR_SUPPORT_KBUILD_MODULES_INSTALL),1)

.PHONY: kbuild_modules_install

kbuild_modules_install: kbuild_check $(TARGET_PRIMARY_OUT)/kbuild/Makefile
	$(if $(V),,@)$(MAKE) -Rr --no-print-directory -C $(KERNELDIR) \
		M=$(abspath $(TARGET_PRIMARY_OUT)/kbuild) \
		$(if $(MODLIB),MODLIB=$(MODLIB)) \
		$(if $(DEPMOD),DEPMOD=$(DEPMOD)) \
		INTERNAL_KBUILD_MAKEFILES="$(INTERNAL_KBUILD_MAKEFILES)" \
		INTERNAL_KBUILD_OBJECTS="$(INTERNAL_KBUILD_OBJECTS)" \
		INTERNAL_EXTRA_KBUILD_OBJECTS="$(INTERNAL_EXTRA_KBUILD_OBJECTS)" \
		BRIDGE_SOURCE_ROOT=$(abspath $(BRIDGE_SOURCE_ROOT)) \
		TARGET_PRIMARY_ARCH=$(TARGET_PRIMARY_ARCH) \
		CLANG_TRIPLE=$(if $(filter %-androideabi,$(CROSS_TRIPLE)),$(patsubst \
		%-androideabi,%-gnueabi,$(CROSS_TRIPLE)),$(patsubst \
		%-android,%-gnu,$(CROSS_TRIPLE)))- \
		CROSS_COMPILE="$(CCACHE) $(KERNEL_CROSS_COMPILE)" \
		EXTRA_CFLAGS="$(ALL_KBUILD_CFLAGS)" \
		CC=$(if $(KERNEL_CC),$(KERNEL_CC),$(KERNEL_CROSS_COMPILE)gcc) \
		LD=$(if $(KERNEL_LD),$(KERNEL_LD),$(KERNEL_CROSS_COMPILE)ld) \
		NM=$(if $(KERNEL_NM),$(KERNEL_NM),$(KERNEL_CROSS_COMPILE)nm) \
		OBJCOPY=$(if $(KERNEL_OBJCOPY),$(KERNEL_OBJCOPY),$(KERNEL_CROSS_COMPILE)objcopy) \
		V=$(V) W=$(W) TOP=$(TOP) modules_install
endif

kbuild_install: installkm
kbuild: install_script_km

ifeq ($(PVR_ANDROID_SUPPORT_KBUILD_OVERLAY),1)
include $(MAKE_TOP)/kbuild/android_kbuild.mk
endif

-include $(MAKE_TOP)/kbuild/kbuild_all.mk

