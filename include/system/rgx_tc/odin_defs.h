/****************************************************************************
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Odin Memory Map - View from PCIe
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
****************************************************************************/

#ifndef _ODIN_DEFS_H_
#define _ODIN_DEFS_H_

/* These defines have not been autogenerated */

#define PCI_VENDOR_ID_ODIN                  (0x1AEE)
#define DEVICE_ID_ODIN                      (0x1010)
#define DEVICE_ID_VALI                      (0x2010)
#define DEVICE_ID_TBA                       (0x1CF2)

/* PCI BAR 0 contains the PDP regs and the Odin system regs */
#define ODN_SYS_BAR                         0
#define ODN_SYS_REGION_SIZE                 0x000800000 /* 8MB */

#define ODN_SYS_REGS_OFFSET                 0
#define ODN_SYS_REGS_SIZE                   0x000400000 /* 4MB */

#define ODN_PDP_REGS_OFFSET                 0x000440000
#define ODN_PDP_REGS_SIZE                   0x000040000 /* 256k */

#define ODN_PDP2_REGS_OFFSET                0x000480000
#define ODN_PDP2_REGS_SIZE                  0x000040000 /* 256k */

#define ODN_PDP2_PFIM_OFFSET                0x000500000
#define ODN_PDP2_PFIM_SIZE                  0x000040000 /* 256k */

#define ODIN_DMA_REGS_OFFSET                0x0004C0000
#define ODIN_DMA_REGS_SIZE                  0x000040000 /* 256k */

#define ODIN_DMA_CHAN_REGS_SIZE             0x000001000 /*   4k */

/* PCI BAR 2 contains the Device Under Test SOCIF 64MB region */
#define ODN_DUT_SOCIF_BAR                   2
#define ODN_DUT_SOCIF_OFFSET                0x000000000
#define ODN_DUT_SOCIF_SIZE                  0x004000000 /* 64MB */

/* PCI BAR 4 contains the on-board 1GB DDR memory */
#define ODN_DDR_BAR                         4
#define ODN_DDR_MEM_OFFSET                  0x000000000
#define ODN_DDR_MEM_SIZE                    0x040000000 /* 1GB */

/* Odin system register banks */
#define ODN_REG_BANK_CORE                   0x00000
#define ODN_REG_BANK_TCF_SPI_MASTER         0x02000
#define ODN_REG_BANK_ODN_CLK_BLK            0x0A000
#define ODN_REG_BANK_ODN_MCU_COMMUNICATOR   0x0C000
#define ODN_REG_BANK_DB_TYPE_ID             0x0C200
#define ODN_REG_BANK_DB_TYPE_ID_TYPE_TCFVUOCTA   0x000000C6U
#define ODN_REG_BANK_DB_TYPE_ID_TYPE_MASK   0x000000C0U
#define ODN_REG_BANK_DB_TYPE_ID_TYPE_SHIFT  0x6
#define ODN_REG_BANK_ODN_I2C                0x0E000
#define ODN_REG_BANK_MULTI_CLK_ALIGN        0x20000
#define ODN_REG_BANK_ALIGN_DATA_TX          0x22000
#define ODN_REG_BANK_SAI_RX_DDR_0           0x24000
#define ODN_REG_BANK_SAI_RX_DDR(n)          (ODN_REG_BANK_SAI_RX_DDR_0 + (0x02000*n))
#define ODN_REG_BANK_SAI_TX_DDR_0           0x3A000
#define ODN_REG_BANK_SAI_TX_DDR(n)          (ODN_REG_BANK_SAI_TX_DDR_0 + (0x02000*n))
#define ODN_REG_BANK_SAI_TX_SDR             0x4E000

/* Odin SPI regs */
#define ODN_SPI_MST_ADDR_RDNWR              0x0000
#define ODN_SPI_MST_WDATA                   0x0004
#define ODN_SPI_MST_RDATA                   0x0008
#define ODN_SPI_MST_STATUS                  0x000C
#define ODN_SPI_MST_GO                      0x0010


/*
   Odin CLK regs - the odn_clk_blk module defs are not auto generated
 */
#define ODN_PDP_P_CLK_OUT_DIVIDER_REG1           0x620
#define ODN_PDP_PCLK_ODIV1_LO_TIME_MASK          0x0000003FU
#define ODN_PDP_PCLK_ODIV1_LO_TIME_SHIFT         0
#define ODN_PDP_PCLK_ODIV1_HI_TIME_MASK          0x00000FC0U
#define ODN_PDP_PCLK_ODIV1_HI_TIME_SHIFT         6

#define ODN_PDP_P_CLK_OUT_DIVIDER_REG2           0x624
#define ODN_PDP_PCLK_ODIV2_NOCOUNT_MASK          0x00000040U
#define ODN_PDP_PCLK_ODIV2_NOCOUNT_SHIFT         6
#define ODN_PDP_PCLK_ODIV2_EDGE_MASK             0x00000080U
#define ODN_PDP_PCLK_ODIV2_EDGE_SHIFT            7

#define ODN_PDP_P_CLK_OUT_DIVIDER_REG3           0x61C

#define ODN_PDP_M_CLK_OUT_DIVIDER_REG1           0x628
#define ODN_PDP_MCLK_ODIV1_LO_TIME_MASK          0x0000003FU
#define ODN_PDP_MCLK_ODIV1_LO_TIME_SHIFT         0
#define ODN_PDP_MCLK_ODIV1_HI_TIME_MASK	         0x00000FC0U
#define ODN_PDP_MCLK_ODIV1_HI_TIME_SHIFT         6

#define ODN_PDP_M_CLK_OUT_DIVIDER_REG2           0x62C
#define ODN_PDP_MCLK_ODIV2_NOCOUNT_MASK          0x00000040U
#define ODN_PDP_MCLK_ODIV2_NOCOUNT_SHIFT         6
#define ODN_PDP_MCLK_ODIV2_EDGE_MASK             0x00000080U
#define ODN_PDP_MCLK_ODIV2_EDGE_SHIFT            7

#define ODN_PDP_P_CLK_MULTIPLIER_REG1            0x650
#define ODN_PDP_PCLK_MUL1_LO_TIME_MASK           0x0000003FU
#define ODN_PDP_PCLK_MUL1_LO_TIME_SHIFT          0
#define ODN_PDP_PCLK_MUL1_HI_TIME_MASK           0x00000FC0U
#define ODN_PDP_PCLK_MUL1_HI_TIME_SHIFT          6

#define ODN_PDP_P_CLK_MULTIPLIER_REG2            0x654
#define ODN_PDP_PCLK_MUL2_NOCOUNT_MASK           0x00000040U
#define ODN_PDP_PCLK_MUL2_NOCOUNT_SHIFT          6
#define ODN_PDP_PCLK_MUL2_EDGE_MASK              0x00000080U
#define ODN_PDP_PCLK_MUL2_EDGE_SHIFT             7

#define ODN_PDP_P_CLK_MULTIPLIER_REG3            0x64C

#define ODN_PDP_P_CLK_IN_DIVIDER_REG             0x658
#define ODN_PDP_PCLK_IDIV_LO_TIME_MASK           0x0000003FU
#define ODN_PDP_PCLK_IDIV_LO_TIME_SHIFT          0
#define ODN_PDP_PCLK_IDIV_HI_TIME_MASK           0x00000FC0U
#define ODN_PDP_PCLK_IDIV_HI_TIME_SHIFT          6
#define ODN_PDP_PCLK_IDIV_NOCOUNT_MASK           0x00001000U
#define ODN_PDP_PCLK_IDIV_NOCOUNT_SHIFT          12
#define ODN_PDP_PCLK_IDIV_EDGE_MASK              0x00002000U
#define ODN_PDP_PCLK_IDIV_EDGE_SHIFT             13

/*
 * DUT core clock input divider, multiplier and out divider.
 */
#define ODN_DUT_CORE_CLK_OUT_DIVIDER1                (0x0028)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER1_HI_TIME_MASK   (0x00000FC0U)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER1_HI_TIME_SHIFT  (6)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER1_LO_TIME_MASK   (0x0000003FU)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER1_LO_TIME_SHIFT  (0)

#define ODN_DUT_CORE_CLK_OUT_DIVIDER2                (0x002C)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER2_EDGE_MASK      (0x00000080U)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER2_EDGE_SHIFT     (7)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER2_NOCOUNT_MASK   (0x00000040U)
#define ODN_DUT_CORE_CLK_OUT_DIVIDER2_NOCOUNT_SHIFT  (6)

#define ODN_DUT_CORE_CLK_MULTIPLIER1                 (0x0050)
#define ODN_DUT_CORE_CLK_MULTIPLIER1_HI_TIME_MASK    (0x00000FC0U)
#define ODN_DUT_CORE_CLK_MULTIPLIER1_HI_TIME_SHIFT   (6)
#define ODN_DUT_CORE_CLK_MULTIPLIER1_LO_TIME_MASK    (0x0000003FU)
#define ODN_DUT_CORE_CLK_MULTIPLIER1_LO_TIME_SHIFT   (0)

#define ODN_DUT_CORE_CLK_MULTIPLIER2                 (0x0054)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_FRAC_MASK       (0x00007000U)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_FRAC_SHIFT      (12)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_FRAC_EN_MASK    (0x00000800U)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_FRAC_EN_SHIFT   (11)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_EDGE_MASK       (0x00000080U)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_EDGE_SHIFT      (7)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_NOCOUNT_MASK    (0x00000040U)
#define ODN_DUT_CORE_CLK_MULTIPLIER2_NOCOUNT_SHIFT   (6)

#define ODN_DUT_CORE_CLK_IN_DIVIDER1                 (0x0058)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_EDGE_MASK       (0x00002000U)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_EDGE_SHIFT      (13)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_NOCOUNT_MASK    (0x00001000U)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_NOCOUNT_SHIFT   (12)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_HI_TIME_MASK    (0x00000FC0U)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_HI_TIME_SHIFT   (6)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_LO_TIME_MASK    (0x0000003FU)
#define ODN_DUT_CORE_CLK_IN_DIVIDER1_LO_TIME_SHIFT   (0)

/*
 * DUT interface clock input divider, multiplier and out divider.
 */
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER1               (0x0220)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER1_HI_TIME_MASK  (0x00000FC0U)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER1_HI_TIME_SHIFT (6)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER1_LO_TIME_MASK  (0x0000003FU)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER1_LO_TIME_SHIFT (0)

#define ODN_DUT_IFACE_CLK_OUT_DIVIDER2               (0x0224)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER2_EDGE_MASK     (0x00000080U)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER2_EDGE_SHIFT    (7)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER2_NOCOUNT_MASK  (0x00000040U)
#define ODN_DUT_IFACE_CLK_OUT_DIVIDER2_NOCOUNT_SHIFT (6)

#define ODN_DUT_IFACE_CLK_MULTIPLIER1                (0x0250)
#define ODN_DUT_IFACE_CLK_MULTIPLIER1_HI_TIME_MASK   (0x00000FC0U)
#define ODN_DUT_IFACE_CLK_MULTIPLIER1_HI_TIME_SHIFT  (6)
#define ODN_DUT_IFACE_CLK_MULTIPLIER1_LO_TIME_MASK   (0x0000003FU)
#define ODN_DUT_IFACE_CLK_MULTIPLIER1_LO_TIME_SHIFT  (0)

#define ODN_DUT_IFACE_CLK_MULTIPLIER2                (0x0254)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_FRAC_MASK      (0x00007000U)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_FRAC_SHIFT     (12)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_FRAC_EN_MASK   (0x00000800U)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_FRAC_EN_SHIFT  (11)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_EDGE_MASK      (0x00000080U)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_EDGE_SHIFT     (7)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_NOCOUNT_MASK   (0x00000040U)
#define ODN_DUT_IFACE_CLK_MULTIPLIER2_NOCOUNT_SHIFT  (6)

#define ODN_DUT_IFACE_CLK_IN_DIVIDER1                (0x0258)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_EDGE_MASK      (0x00002000U)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_EDGE_SHIFT     (13)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_NOCOUNT_MASK   (0x00001000U)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_NOCOUNT_SHIFT  (12)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_HI_TIME_MASK   (0x00000FC0U)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_HI_TIME_SHIFT  (6)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_LO_TIME_MASK   (0x0000003FU)
#define ODN_DUT_IFACE_CLK_IN_DIVIDER1_LO_TIME_SHIFT  (0)


/*
 * Min max values from Xilinx Virtex7 data sheet DS183, for speed grade 2
 * All in Hz
 */
#define ODN_INPUT_CLOCK_SPEED                        (100000000U)
#define ODN_INPUT_CLOCK_SPEED_MIN                    (10000000U)
#define ODN_INPUT_CLOCK_SPEED_MAX                    (933000000U)
#define ODN_OUTPUT_CLOCK_SPEED_MIN                   (4690000U)
#define ODN_OUTPUT_CLOCK_SPEED_MAX                   (933000000U)
#define ODN_VCO_MIN                                  (600000000U)
#define ODN_VCO_MAX                                  (1440000000U)
#define ODN_PFD_MIN                                  (10000000U)
#define ODN_PFD_MAX                                  (500000000U)

/*
 * Max values that can be set in DRP registers
 */
#define ODN_OREG_VALUE_MAX                            (126.875f)
#define ODN_MREG_VALUE_MAX                            (126.875f)
#define ODN_DREG_VALUE_MAX                            (126U)


#define ODN_MMCM_LOCK_STATUS_DUT_CORE                (0x00000001U)
#define ODN_MMCM_LOCK_STATUS_DUT_IF                  (0x00000002U)
#define ODN_MMCM_LOCK_STATUS_PDPP                    (0x00000008U)

/*
    Odin interrupt flags
*/
#define ODN_INTERRUPT_ENABLE_PDP1           (1 << ODN_INTERRUPT_ENABLE_PDP1_SHIFT)
#define ODN_INTERRUPT_ENABLE_PDP2           (1 << ODN_INTERRUPT_ENABLE_PDP2_SHIFT)
#define ODN_INTERRUPT_ENABLE_DUT            (1 << ODN_INTERRUPT_ENABLE_DUT_SHIFT)
#define ODN_INTERRUPT_STATUS_PDP1           (1 << ODN_INTERRUPT_STATUS_PDP1_SHIFT)
#define ODN_INTERRUPT_STATUS_PDP2           (1 << ODN_INTERRUPT_STATUS_PDP2_SHIFT)
#define ODN_INTERRUPT_STATUS_DUT            (1 << ODN_INTERRUPT_STATUS_DUT_SHIFT)
#define ODN_INTERRUPT_CLEAR_PDP1            (1 << ODN_INTERRUPT_CLR_PDP1_SHIFT)
#define ODN_INTERRUPT_CLEAR_PDP2            (1 << ODN_INTERRUPT_CLR_PDP2_SHIFT)
#define ODN_INTERRUPT_CLEAR_DUT             (1 << ODN_INTERRUPT_CLR_DUT_SHIFT)

#define ODN_INTERRUPT_ENABLE_CDMA           (1 << ODN_INTERRUPT_ENABLE_CDMA_SHIFT)
#define ODN_INTERRUPT_STATUS_CDMA           (1 << ODN_INTERRUPT_STATUS_CDMA_SHIFT)
#define ODN_INTERRUPT_CLEAR_CDMA            (1 << ODN_INTERRUPT_CLR_CDMA_SHIFT)

#define ODN_INTERRUPT_ENABLE_CDMA2          (1 << (ODN_INTERRUPT_ENABLE_CDMA_SHIFT + 1))
#define ODN_INTERRUPT_STATUS_CDMA2          (1 << (ODN_INTERRUPT_STATUS_CDMA_SHIFT + 1))
#define ODN_INTERRUPT_CLEAR_CDMA2           (1 << (ODN_INTERRUPT_CLR_CDMA_SHIFT + 1))

/*
   Other defines
*/
#define ODN_STREAM_OFF                      0
#define ODN_STREAM_ON                       1
#define ODN_SYNC_GEN_DISABLE                0
#define ODN_SYNC_GEN_ENABLE                 1
#define ODN_INTERLACE_DISABLE               0
#define ODN_INTERLACE_ENABLE                1
#define ODN_PIXEL_CLOCK_INVERTED            1
#define ODN_HSYNC_POLARITY_ACTIVE_HIGH      1

#define ODN_PDP_INTCLR_ALL                  0x000FFFFFU
#define	ODN_PDP_INTSTAT_ALL_OURUN_MASK      0x000FFFF0U

/*
   DMA defs
*/
#define ODN_CDMA_ADDR_WIDTH                35
#define ODN_DMA_HW_DESC_HEAP_SIZE          0x100000
#define ODN_DMA_CHAN_RX                    0
#define ODN_DMA_CHAN_TX                    1

#define ODIN_DMA_TX_CHAN_NAME              "tx"
#define ODIN_DMA_RX_CHAN_NAME              "rx"

/*
   FBC defs
*/
#define ODIN_PFIM_RELNUM                   (005U)

#endif /* _ODIN_DEFS_H_ */

/*****************************************************************************
 End of file (odn_defs.h)
*****************************************************************************/
