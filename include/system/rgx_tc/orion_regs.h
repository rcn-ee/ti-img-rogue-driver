/******************************************************************************
@Title          Orion system control register definitions
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Orion FPGA register defs for Sirius RTL
@Author         Autogenerated
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
******************************************************************************/

#ifndef _OUT_DRV_H_
#define _OUT_DRV_H_

/*
	Register ID
*/
#define SRS_CORE_ID 0x0000
#define SRS_ID_VARIANT_MASK 0x0000FFFFU
#define SRS_ID_VARIANT_SHIFT 0
#define SRS_ID_VARIANT_SIGNED 0

#define SRS_ID_ID_MASK 0xFFFF0000U
#define SRS_ID_ID_SHIFT 16
#define SRS_ID_ID_SIGNED 0

/*
	Register REVISION
*/
#define SRS_CORE_REVISION 0x0004
#define SRS_REVISION_MINOR_MASK 0x000000FFU
#define SRS_REVISION_MINOR_SHIFT 0
#define SRS_REVISION_MINOR_SIGNED 0

#define SRS_REVISION_MAJOR_MASK 0x00000F00U
#define SRS_REVISION_MAJOR_SHIFT 8
#define SRS_REVISION_MAJOR_SIGNED 0

/*
	Register CHANGE_SET
*/
#define SRS_CORE_CHANGE_SET 0x0008
#define SRS_CHANGE_SET_SET_MASK 0xFFFFFFFFU
#define SRS_CHANGE_SET_SET_SHIFT 0
#define SRS_CHANGE_SET_SET_SIGNED 0

/*
	Register USER_ID
*/
#define SRS_CORE_USER_ID 0x000C
#define SRS_USER_ID_ID_MASK 0x0000000FU
#define SRS_USER_ID_ID_SHIFT 0
#define SRS_USER_ID_ID_SIGNED 0

/*
	Register USER_BUILD
*/
#define SRS_CORE_USER_BUILD 0x0010
#define SRS_USER_BUILD_BUILD_MASK 0xFFFFFFFFU
#define SRS_USER_BUILD_BUILD_SHIFT 0
#define SRS_USER_BUILD_BUILD_SIGNED 0

/*
	Register SOFT_RESETN
*/
#define SRS_CORE_SOFT_RESETN 0x0080
#define SRS_SOFT_RESETN_DDR_MASK 0x00000001U
#define SRS_SOFT_RESETN_DDR_SHIFT 0
#define SRS_SOFT_RESETN_DDR_SIGNED 0

#define SRS_SOFT_RESETN_USB_MASK 0x00000002U
#define SRS_SOFT_RESETN_USB_SHIFT 1
#define SRS_SOFT_RESETN_USB_SIGNED 0

#define SRS_SOFT_RESETN_PDP_MASK 0x00000004U
#define SRS_SOFT_RESETN_PDP_SHIFT 2
#define SRS_SOFT_RESETN_PDP_SIGNED 0

#define SRS_SOFT_RESETN_GIST_MASK 0x00000008U
#define SRS_SOFT_RESETN_GIST_SHIFT 3
#define SRS_SOFT_RESETN_GIST_SIGNED 0

/*
	Register DUT_SOFT_RESETN
*/
#define SRS_CORE_DUT_SOFT_RESETN 0x0084
#define SRS_DUT_SOFT_RESETN_EXTERNAL_MASK 0x00000001U
#define SRS_DUT_SOFT_RESETN_EXTERNAL_SHIFT 0
#define SRS_DUT_SOFT_RESETN_EXTERNAL_SIGNED 0

/*
	Register SOFT_AUTO_RESETN
*/
#define SRS_CORE_SOFT_AUTO_RESETN 0x0088
#define SRS_SOFT_AUTO_RESETN_CFG_MASK 0x00000001U
#define SRS_SOFT_AUTO_RESETN_CFG_SHIFT 0
#define SRS_SOFT_AUTO_RESETN_CFG_SIGNED 0

/*
	Register CLK_GEN_RESET
*/
#define SRS_CORE_CLK_GEN_RESET 0x0090
#define SRS_CLK_GEN_RESET_DUT_CORE_MMCM_MASK 0x00000001U
#define SRS_CLK_GEN_RESET_DUT_CORE_MMCM_SHIFT 0
#define SRS_CLK_GEN_RESET_DUT_CORE_MMCM_SIGNED 0

#define SRS_CLK_GEN_RESET_DUT_IF_MMCM_MASK 0x00000002U
#define SRS_CLK_GEN_RESET_DUT_IF_MMCM_SHIFT 1
#define SRS_CLK_GEN_RESET_DUT_IF_MMCM_SIGNED 0

#define SRS_CLK_GEN_RESET_MULTI_MMCM_MASK 0x00000004U
#define SRS_CLK_GEN_RESET_MULTI_MMCM_SHIFT 2
#define SRS_CLK_GEN_RESET_MULTI_MMCM_SIGNED 0

#define SRS_CLK_GEN_RESET_PDP_MMCM_MASK 0x00000008U
#define SRS_CLK_GEN_RESET_PDP_MMCM_SHIFT 3
#define SRS_CLK_GEN_RESET_PDP_MMCM_SIGNED 0

/*
	Register DUT_MEM
*/
#define SRS_CORE_DUT_MEM 0x0120
#define SRS_DUT_MEM_READ_RESPONSE_LATENCY_MASK 0x0000FFFFU
#define SRS_DUT_MEM_READ_RESPONSE_LATENCY_SHIFT 0
#define SRS_DUT_MEM_READ_RESPONSE_LATENCY_SIGNED 0

#define SRS_DUT_MEM_WRITE_RESPONSE_LATENCY_MASK 0xFFFF0000U
#define SRS_DUT_MEM_WRITE_RESPONSE_LATENCY_SHIFT 16
#define SRS_DUT_MEM_WRITE_RESPONSE_LATENCY_SIGNED 0

/*
	Register APM
*/
#define SRS_CORE_APM 0x0150
#define SRS_APM_RESET_EVENT_MASK 0x00000001U
#define SRS_APM_RESET_EVENT_SHIFT 0
#define SRS_APM_RESET_EVENT_SIGNED 0

#define SRS_APM_CAPTURE_EVENT_MASK 0x00000002U
#define SRS_APM_CAPTURE_EVENT_SHIFT 1
#define SRS_APM_CAPTURE_EVENT_SIGNED 0

/*
	Register NUM_GPIO
*/
#define SRS_CORE_NUM_GPIO 0x0180
#define SRS_NUM_GPIO_NUMBER_MASK 0x0000000FU
#define SRS_NUM_GPIO_NUMBER_SHIFT 0
#define SRS_NUM_GPIO_NUMBER_SIGNED 0

/*
	Register GPIO_EN
*/
#define SRS_CORE_GPIO_EN 0x0184
#define SRS_GPIO_EN_DIRECTION_MASK 0x000000FFU
#define SRS_GPIO_EN_DIRECTION_SHIFT 0
#define SRS_GPIO_EN_DIRECTION_SIGNED 0

/*
	Register GPIO
*/
#define SRS_CORE_GPIO 0x0188
#define SRS_GPIO_GPIO_MASK 0x000000FFU
#define SRS_GPIO_GPIO_SHIFT 0
#define SRS_GPIO_GPIO_SIGNED 0

/*
	Register SPI_MASTER_IFACE
*/
#define SRS_CORE_SPI_MASTER_IFACE 0x018C
#define SRS_SPI_MASTER_IFACE_ENABLE_MASK 0x00000001U
#define SRS_SPI_MASTER_IFACE_ENABLE_SHIFT 0
#define SRS_SPI_MASTER_IFACE_ENABLE_SIGNED 0

/*
	Register SRS_IP_STATUS
*/
#define SRS_CORE_SRS_IP_STATUS 0x0200
#define SRS_SRS_IP_STATUS_PCIE_USER_LNK_UP_MASK 0x00000001U
#define SRS_SRS_IP_STATUS_PCIE_USER_LNK_UP_SHIFT 0
#define SRS_SRS_IP_STATUS_PCIE_USER_LNK_UP_SIGNED 0

#define SRS_SRS_IP_STATUS_MIG_INIT_CALIB_COMPLETE_MASK 0x00000002U
#define SRS_SRS_IP_STATUS_MIG_INIT_CALIB_COMPLETE_SHIFT 1
#define SRS_SRS_IP_STATUS_MIG_INIT_CALIB_COMPLETE_SIGNED 0

#define SRS_SRS_IP_STATUS_GIST_SLV_C2C_CONFIG_ERROR_OUT_MASK 0x00000004U
#define SRS_SRS_IP_STATUS_GIST_SLV_C2C_CONFIG_ERROR_OUT_SHIFT 2
#define SRS_SRS_IP_STATUS_GIST_SLV_C2C_CONFIG_ERROR_OUT_SIGNED 0

#define SRS_SRS_IP_STATUS_GIST_MST_C2C_CONFIG_ERROR_OUT_MASK 0x00000008U
#define SRS_SRS_IP_STATUS_GIST_MST_C2C_CONFIG_ERROR_OUT_SHIFT 3
#define SRS_SRS_IP_STATUS_GIST_MST_C2C_CONFIG_ERROR_OUT_SIGNED 0

/*
	Register CORE_CONTROL
*/
#define SRS_CORE_CORE_CONTROL 0x0204
#define SRS_CORE_CONTROL_BAR4_OFFSET_MASK 0x0000001FU
#define SRS_CORE_CONTROL_BAR4_OFFSET_SHIFT 0
#define SRS_CORE_CONTROL_BAR4_OFFSET_SIGNED 0

#define SRS_CORE_CONTROL_HDMI_MONITOR_OVERRIDE_MASK 0x00000300U
#define SRS_CORE_CONTROL_HDMI_MONITOR_OVERRIDE_SHIFT 8
#define SRS_CORE_CONTROL_HDMI_MONITOR_OVERRIDE_SIGNED 0

#define SRS_CORE_CONTROL_HDMI_MODULE_EN_MASK 0x00001C00U
#define SRS_CORE_CONTROL_HDMI_MODULE_EN_SHIFT 10
#define SRS_CORE_CONTROL_HDMI_MODULE_EN_SIGNED 0

/*
	Register REG_BANK_STATUS
*/
#define SRS_CORE_REG_BANK_STATUS 0x0208
#define SRS_REG_BANK_STATUS_ARB_SLV_RD_TIMEOUT_MASK 0xFFFFFFFFU
#define SRS_REG_BANK_STATUS_ARB_SLV_RD_TIMEOUT_SHIFT 0
#define SRS_REG_BANK_STATUS_ARB_SLV_RD_TIMEOUT_SIGNED 0

/*
	Register MMCM_LOCK_STATUS
*/
#define SRS_CORE_MMCM_LOCK_STATUS 0x020C
#define SRS_MMCM_LOCK_STATUS_DUT_CORE_MASK 0x00000001U
#define SRS_MMCM_LOCK_STATUS_DUT_CORE_SHIFT 0
#define SRS_MMCM_LOCK_STATUS_DUT_CORE_SIGNED 0

#define SRS_MMCM_LOCK_STATUS_DUT_IF_MASK 0x00000002U
#define SRS_MMCM_LOCK_STATUS_DUT_IF_SHIFT 1
#define SRS_MMCM_LOCK_STATUS_DUT_IF_SIGNED 0

#define SRS_MMCM_LOCK_STATUS_MULTI_MASK 0x00000004U
#define SRS_MMCM_LOCK_STATUS_MULTI_SHIFT 2
#define SRS_MMCM_LOCK_STATUS_MULTI_SIGNED 0

#define SRS_MMCM_LOCK_STATUS_PDP_MASK 0x00000008U
#define SRS_MMCM_LOCK_STATUS_PDP_SHIFT 3
#define SRS_MMCM_LOCK_STATUS_PDP_SIGNED 0

/*
	Register GIST_STATUS
*/
#define SRS_CORE_GIST_STATUS 0x0210
#define SRS_GIST_STATUS_MST_MASK 0x000001FFU
#define SRS_GIST_STATUS_MST_SHIFT 0
#define SRS_GIST_STATUS_MST_SIGNED 0

#define SRS_GIST_STATUS_SLV_MASK 0x001FF000U
#define SRS_GIST_STATUS_SLV_SHIFT 12
#define SRS_GIST_STATUS_SLV_SIGNED 0

#define SRS_GIST_STATUS_SLV_OUT_MASK 0x03000000U
#define SRS_GIST_STATUS_SLV_OUT_SHIFT 24
#define SRS_GIST_STATUS_SLV_OUT_SIGNED 0

#define SRS_GIST_STATUS_MST_OUT_MASK 0x70000000U
#define SRS_GIST_STATUS_MST_OUT_SHIFT 28
#define SRS_GIST_STATUS_MST_OUT_SIGNED 0

/*
	Register SENSOR_BOARD
*/
#define SRS_CORE_SENSOR_BOARD 0x0214
#define SRS_SENSOR_BOARD_ID_MASK 0x00000003U
#define SRS_SENSOR_BOARD_ID_SHIFT 0
#define SRS_SENSOR_BOARD_ID_SIGNED 0

/*
	Register INTERRUPT_STATUS
*/
#define SRS_CORE_INTERRUPT_STATUS 0x0218
#define SRS_INTERRUPT_STATUS_DUT_MASK 0x00000001U
#define SRS_INTERRUPT_STATUS_DUT_SHIFT 0
#define SRS_INTERRUPT_STATUS_DUT_SIGNED 0

#define SRS_INTERRUPT_STATUS_PDP_MASK 0x00000002U
#define SRS_INTERRUPT_STATUS_PDP_SHIFT 1
#define SRS_INTERRUPT_STATUS_PDP_SIGNED 0

#define SRS_INTERRUPT_STATUS_I2C_MASK 0x00000004U
#define SRS_INTERRUPT_STATUS_I2C_SHIFT 2
#define SRS_INTERRUPT_STATUS_I2C_SIGNED 0

#define SRS_INTERRUPT_STATUS_SPI_MASK 0x00000008U
#define SRS_INTERRUPT_STATUS_SPI_SHIFT 3
#define SRS_INTERRUPT_STATUS_SPI_SIGNED 0

#define SRS_INTERRUPT_STATUS_APM_MASK 0x00000010U
#define SRS_INTERRUPT_STATUS_APM_SHIFT 4
#define SRS_INTERRUPT_STATUS_APM_SIGNED 0

#define SRS_INTERRUPT_STATUS_OS_IRQ_MASK 0x00001FE0U
#define SRS_INTERRUPT_STATUS_OS_IRQ_SHIFT 5
#define SRS_INTERRUPT_STATUS_OS_IRQ_SIGNED 0

#define SRS_INTERRUPT_STATUS_IRQ_TEST_MASK 0x40000000U
#define SRS_INTERRUPT_STATUS_IRQ_TEST_SHIFT 30
#define SRS_INTERRUPT_STATUS_IRQ_TEST_SIGNED 0

#define SRS_INTERRUPT_STATUS_MASTER_STATUS_MASK 0x80000000U
#define SRS_INTERRUPT_STATUS_MASTER_STATUS_SHIFT 31
#define SRS_INTERRUPT_STATUS_MASTER_STATUS_SIGNED 0

/*
	Register INTERRUPT_ENABLE
*/
#define SRS_CORE_INTERRUPT_ENABLE 0x021C
#define SRS_INTERRUPT_ENABLE_DUT_MASK 0x00000001U
#define SRS_INTERRUPT_ENABLE_DUT_SHIFT 0
#define SRS_INTERRUPT_ENABLE_DUT_SIGNED 0

#define SRS_INTERRUPT_ENABLE_PDP_MASK 0x00000002U
#define SRS_INTERRUPT_ENABLE_PDP_SHIFT 1
#define SRS_INTERRUPT_ENABLE_PDP_SIGNED 0

#define SRS_INTERRUPT_ENABLE_I2C_MASK 0x00000004U
#define SRS_INTERRUPT_ENABLE_I2C_SHIFT 2
#define SRS_INTERRUPT_ENABLE_I2C_SIGNED 0

#define SRS_INTERRUPT_ENABLE_SPI_MASK 0x00000008U
#define SRS_INTERRUPT_ENABLE_SPI_SHIFT 3
#define SRS_INTERRUPT_ENABLE_SPI_SIGNED 0

#define SRS_INTERRUPT_ENABLE_APM_MASK 0x00000010U
#define SRS_INTERRUPT_ENABLE_APM_SHIFT 4
#define SRS_INTERRUPT_ENABLE_APM_SIGNED 0

#define SRS_INTERRUPT_ENABLE_OS_IRQ_MASK 0x00001FE0U
#define SRS_INTERRUPT_ENABLE_OS_IRQ_SHIFT 5
#define SRS_INTERRUPT_ENABLE_OS_IRQ_SIGNED 0

#define SRS_INTERRUPT_ENABLE_IRQ_TEST_MASK 0x40000000U
#define SRS_INTERRUPT_ENABLE_IRQ_TEST_SHIFT 30
#define SRS_INTERRUPT_ENABLE_IRQ_TEST_SIGNED 0

#define SRS_INTERRUPT_ENABLE_MASTER_ENABLE_MASK 0x80000000U
#define SRS_INTERRUPT_ENABLE_MASTER_ENABLE_SHIFT 31
#define SRS_INTERRUPT_ENABLE_MASTER_ENABLE_SIGNED 0

/*
	Register INTERRUPT_CLR
*/
#define SRS_CORE_INTERRUPT_CLR 0x0220
#define SRS_INTERRUPT_CLR_DUT_MASK 0x00000001U
#define SRS_INTERRUPT_CLR_DUT_SHIFT 0
#define SRS_INTERRUPT_CLR_DUT_SIGNED 0

#define SRS_INTERRUPT_CLR_PDP_MASK 0x00000002U
#define SRS_INTERRUPT_CLR_PDP_SHIFT 1
#define SRS_INTERRUPT_CLR_PDP_SIGNED 0

#define SRS_INTERRUPT_CLR_I2C_MASK 0x00000004U
#define SRS_INTERRUPT_CLR_I2C_SHIFT 2
#define SRS_INTERRUPT_CLR_I2C_SIGNED 0

#define SRS_INTERRUPT_CLR_SPI_MASK 0x00000008U
#define SRS_INTERRUPT_CLR_SPI_SHIFT 3
#define SRS_INTERRUPT_CLR_SPI_SIGNED 0

#define SRS_INTERRUPT_CLR_APM_MASK 0x00000010U
#define SRS_INTERRUPT_CLR_APM_SHIFT 4
#define SRS_INTERRUPT_CLR_APM_SIGNED 0

#define SRS_INTERRUPT_CLR_OS_IRQ_MASK 0x00001FE0U
#define SRS_INTERRUPT_CLR_OS_IRQ_SHIFT 5
#define SRS_INTERRUPT_CLR_OS_IRQ_SIGNED 0

#define SRS_INTERRUPT_CLR_IRQ_TEST_MASK 0x40000000U
#define SRS_INTERRUPT_CLR_IRQ_TEST_SHIFT 30
#define SRS_INTERRUPT_CLR_IRQ_TEST_SIGNED 0

#define SRS_INTERRUPT_CLR_MASTER_CLEAR_MASK 0x80000000U
#define SRS_INTERRUPT_CLR_MASTER_CLEAR_SHIFT 31
#define SRS_INTERRUPT_CLR_MASTER_CLEAR_SIGNED 0

/*
	Register INTERRUPT_TEST
*/
#define SRS_CORE_INTERRUPT_TEST 0x0224
#define SRS_INTERRUPT_TEST_INTERRUPT_TEST_MASK 0x00000001U
#define SRS_INTERRUPT_TEST_INTERRUPT_TEST_SHIFT 0
#define SRS_INTERRUPT_TEST_INTERRUPT_TEST_SIGNED 0

/*
	Register INTERRUPT_TIMEOUT_CLR
*/
#define SRS_CORE_INTERRUPT_TIMEOUT_CLR 0x0228
#define SRS_INTERRUPT_TIMEOUT_CLR_INTERRUPT_MST_TIMEOUT_CLR_MASK 0x00000002U
#define SRS_INTERRUPT_TIMEOUT_CLR_INTERRUPT_MST_TIMEOUT_CLR_SHIFT 1
#define SRS_INTERRUPT_TIMEOUT_CLR_INTERRUPT_MST_TIMEOUT_CLR_SIGNED 0

#define SRS_INTERRUPT_TIMEOUT_CLR_INTERRUPT_MST_TIMEOUT_MASK 0x00000001U
#define SRS_INTERRUPT_TIMEOUT_CLR_INTERRUPT_MST_TIMEOUT_SHIFT 0
#define SRS_INTERRUPT_TIMEOUT_CLR_INTERRUPT_MST_TIMEOUT_SIGNED 0

/*
	Register INTERRUPT_TIMEOUT
*/
#define SRS_CORE_INTERRUPT_TIMEOUT 0x022C
#define SRS_INTERRUPT_TIMEOUT_INTERRUPT_TIMEOUT_THRESHOLD_COUNTER_MASK \
	0xFFFFFFFFU
#define SRS_INTERRUPT_TIMEOUT_INTERRUPT_TIMEOUT_THRESHOLD_COUNTER_SHIFT 0
#define SRS_INTERRUPT_TIMEOUT_INTERRUPT_TIMEOUT_THRESHOLD_COUNTER_SIGNED 0

#endif /* _OUT_DRV_H_ */

/******************************************************************************
 End of file (orion_regs.h)
******************************************************************************/
