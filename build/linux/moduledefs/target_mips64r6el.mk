########################################################################### ###
#@File
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

# Little endian, mips64r6, regular ABI, with synci instruction
MIPS_ABI_FLAGS := -EL -march=mips64r6 -mabi=64 -msynci

ifeq ($(cc-is-clang),true)
MIPS_ABI_FLAGS := $(filter-out -msynci,$(MIPS_ABI_FLAGS))
MODULE_CC := $(CC) $(MIPS_ABI_FLAGS) -fintegrated-as
MODULE_CXX := $(CXX) $(MIPS_ABI_FLAGS) -fintegrated-as
else
MODULE_CC := $(CC) $(MIPS_ABI_FLAGS)
MODULE_CXX := $(CXX) $(MIPS_ABI_FLAGS)
endif

MODULE_AR := $(AR)
MODULE_NM := $(NM)
MODULE_OBJCOPY := $(OBJCOPY)
MODULE_RANLIB := $(RANLIB)
MODULE_STRIP := $(STRIP)

MODULE_CFLAGS := $(ALL_CFLAGS) $($(THIS_MODULE)_cflags) $(MIPS_ABI_FLAGS)
MODULE_CXXFLAGS := $(ALL_CXXFLAGS) $($(THIS_MODULE)_cxxflags) $(MIPS_ABI_FLAGS)
MODULE_LDFLAGS := $($(THIS_MODULE)_ldflags) -L$(MODULE_OUT) -Xlinker -rpath-link=$(MODULE_OUT) $(ALL_LDFLAGS) $(MIPS_ABI_FLAGS)

# Since this is a target module, add system-specific include flags.
MODULE_INCLUDE_FLAGS := \
 $(SYS_INCLUDES_RESIDUAL) \
 $(addprefix -isystem ,$(filter-out $(patsubst -I%,%,$(filter -I%,$(MODULE_INCLUDE_FLAGS))),$(SYS_INCLUDES_ISYSTEM))) \
 $(MODULE_INCLUDE_FLAGS)

ifneq ($(SUPPORT_ANDROID_PLATFORM),)

MODULE_EXE_LDFLAGS := \
 -Bdynamic -nostdlib -Wl,-dynamic-linker,/system/bin/linker64 -lc

override LIBGCC := $(shell $(patsubst @%,%,$(MODULE_CC)) -print-libgcc-file-name)
ifeq ($(cc-is-clang),true)
 ifeq ($(wildcard $(LIBGCC)),)
  override LIBGCC := \
   $(shell $(CROSS_COMPILE)gcc $(MIPS_ABI_FLAGS) -print-libgcc-file-name)
  ifeq ($(wildcard $(LIBGCC)),)
   $(error Primary clang -print-libgcc-file-name workaround failed)
  endif
 endif
endif

MODULE_LIBGCC := -Wl,--version-script,$(MAKE_TOP)/common/libgcc.lds $(LIBGCC)

ifeq ($(NDK_ROOT),)

_obj := $(TARGET_ROOT)/product/$(TARGET_DEVICE)/obj
_lib := lib

# Linker flags used to find system libraries.
MODULE_SYSTEM_LIBRARY_DIR_FLAGS += \
 -L$(_obj)/$(_lib) \
 -Xlinker -rpath-link=$(_obj)/$(_lib) \
 -L$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64 \
 -Xlinker -rpath-link=$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64
ifneq ($(wildcard $(TARGET_ROOT)/product/$(TARGET_DEVICE)/vendor),)
MODULE_SYSTEM_LIBRARY_DIR_FLAGS += \
 -L$(TARGET_ROOT)/product/$(TARGET_DEVICE)/vendor/lib64 \
 -Xlinker -rpath-link=$(TARGET_ROOT)/product/$(TARGET_DEVICE)/vendor/lib64
else
MODULE_SYSTEM_LIBRARY_DIR_FLAGS += \
 -L$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/vendor/lib64 \
 -Xlinker -rpath-link=$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/vendor/lib64
endif

MODULE_INCLUDE_FLAGS := \
 -isystem $(ANDROID_ROOT)/bionic/libc/arch-mips64/include \
 -isystem $(ANDROID_ROOT)/bionic/libc/kernel/uapi/asm-mips \
 -isystem $(ANDROID_ROOT)/bionic/libm/include/mips \
 $(MODULE_INCLUDE_FLAGS)

MODULE_ARCH_TAG := $(_obj)

else # NDK_ROOT

MODULE_INCLUDE_FLAGS := \
 -isystem $(NDK_SYSROOT)/usr/include/$(CROSS_TRIPLE) \
 $(MODULE_INCLUDE_FLAGS)

# Take RScpp from TARGET_ROOT, as the NDK lacks 64r6 support
MODULE_LIBRARY_FLAGS_SUBST := \
 art:$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64/libart.so \
 RScpp:$(TARGET_ROOT)/product/$(TARGET_DEVICE)/obj/STATIC_LIBRARIES/libRScpp_static_intermediates/libRScpp_static.a

ifeq ($(wildcard $(NDK_ROOT)/out/local/mips64/libc++.so),)
MODULE_LIBRARY_FLAGS_SUBST := \
 c++:$(NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/mips64/libc++_static.a$$(space)$(NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/mips64/libc++abi.a \
 $(MODULE_LIBRARY_FLAGS_SUBST)
else
MODULE_LIBRARY_FLAGS_SUBST := \
 c++:$(NDK_ROOT)/out/local/mips64/libc++.so \
 $(MODULE_LIBRARY_FLAGS_SUBST)
MODULE_SYSTEM_LIBRARY_DIR_FLAGS += \
 -Xlinker -rpath-link=$(NDK_ROOT)/out/local/mips64
endif

ifeq ($(filter-out $(NDK_ROOT)/%,$(NDK_SYSROOT)),)

MODULE_SYSTEM_LIBRARY_DIR_FLAGS += \
 -L$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64 \
 -Xlinker -rpath-link=$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64

# Substitutions performed on MODULE_LIBRARY_FLAGS (NDK workarounds)
MODULE_LIBRARY_FLAGS_SUBST := \
 nativewindow:$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64/libnativewindow.so \
 sync:$(TARGET_ROOT)/product/$(TARGET_DEVICE)/system/lib64/libsync.so \
 $(MODULE_LIBRARY_FLAGS_SUBST)

endif # !VNDK

_obj := $(NDK_PLATFORMS_ROOT)/$(TARGET_PLATFORM)/arch-mips64/usr
_lib := lib64

MODULE_SYSTEM_LIBRARY_DIR_FLAGS := \
 -L$(_obj)/$(_lib) \
 -Xlinker -rpath-link=$(_obj)/$(_lib) \
 $(MODULE_SYSTEM_LIBRARY_DIR_FLAGS)

# Workaround; the VNDK platforms root lacks the crt files
_obj := $(NDK_ROOT)/platforms/$(TARGET_PLATFORM)/arch-mips64/usr
_lib := lib64

MODULE_EXE_LDFLAGS := $(MODULE_EXE_LDFLAGS) $(LIBGCC) -Wl,--as-needed -ldl

MODULE_ARCH_TAG := mips64

endif # NDK_ROOT

MODULE_LIB_LDFLAGS := $(MODULE_EXE_LDFLAGS)

MODULE_LDFLAGS += $(MODULE_SYSTEM_LIBRARY_DIR_FLAGS)

MODULE_EXE_CRTBEGIN := $(_obj)/$(_lib)/crtbegin_dynamic.o
MODULE_EXE_CRTEND := $(_obj)/$(_lib)/crtend_android.o

MODULE_LIB_CRTBEGIN := $(_obj)/$(_lib)/crtbegin_so.o
MODULE_LIB_CRTEND := $(_obj)/$(_lib)/crtend_so.o

else # SUPPORT_ANDROID_PLATFORM

# this is probably wrong...
MODULE_ARCH_TAG := mips64el

endif # SUPPORT_ANDROID_PLATFORM

ifneq ($(BUILD),debug)
ifeq ($(USE_LTO),1)
MODULE_LDFLAGS := \
 $(sort $(filter-out -W% -D% -isystem /%,$(ALL_CFLAGS) $(ALL_CXXFLAGS))) \
 $(MODULE_LDFLAGS)
endif
endif

MODULE_ARCH_BITNESS := 64

MESON_CROSS_CPU ?= mips64

MESON_CROSS_SYSTEM  := linux
MESON_CROSS_CPU_FAMILY := mips64
MESON_CROSS_ENDIAN := little
MESON_CROSS_CROSS_COMPILE := $(CROSS_COMPILE)
MESON_CROSS_CC := $(patsubst @%,%,$(CC))
MESON_CROSS_CXX := $(patsubst @%,%,$(CXX))

ifeq ($(cc-is-clang),true)
MESON_CROSS_C_ARGS := $(MIPS_ABI_FLAGS) -fintegrated-as $(SYS_CFLAGS)
MESON_CROSS_CXX_ARGS := $(MIPS_ABI_FLAGS) -fintegrated-as $(SYS_CXXFLAGS)
else
MESON_CROSS_C_ARGS := $(MIPS_ABI_FLAGS) $(SYS_CFLAGS)
MESON_CROSS_CXX_ARGS := $(MIPS_ABI_FLAGS) $(SYS_CXXFLAGS)
endif

MESON_CROSS_C_LINK_ARGS := -EL $(SYS_LDFLAGS)
MESON_CROSS_CXX_LINK_ARGS := -EL $(SYS_LDFLAGS)