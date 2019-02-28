/*
 * arch/arm/cpu/armv7/rmobile/pfc-r8a7790.c
 *     This file is r8a7790 processor support - PFC hardware block.
 *
 * Copy from linux-kernel:drivers/pinctrl/sh-pfc/pfc-r8a7790.c
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 * Copyright (C) 2013 Magnus Damm
 * Copyright (C) 2012 Renesas Solutions Corp.
 * Copyright (C) 2012 Kuninori Morimoto <kuninori.morimoto.gx@renesas.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <common.h>
#include <sh_pfc.h>
#include <asm/gpio.h>
#include "pfc-r8a7790.h"

enum {
	PINMUX_RESERVED = 0,

	PINMUX_DATA_BEGIN,
	GP_ALL(DATA),
	PINMUX_DATA_END,

	PINMUX_INPUT_BEGIN,
	GP_ALL(IN),
	PINMUX_INPUT_END,

	PINMUX_OUTPUT_BEGIN,
	GP_ALL(OUT),
	PINMUX_OUTPUT_END,

	PINMUX_FUNCTION_BEGIN,
	GP_ALL(FN),

	/* GPSR0 */
	FN_IP0_2_0, FN_IP0_5_3, FN_IP0_8_6, FN_IP0_11_9, FN_IP0_15_12,
	FN_IP0_19_16, FN_IP0_22_20, FN_IP0_26_23, FN_IP0_30_27,
	FN_IP1_3_0, FN_IP1_7_4, FN_IP1_11_8, FN_IP1_14_12,
	FN_IP1_17_15, FN_IP1_21_18, FN_IP1_25_22, FN_IP1_27_26,
	FN_IP1_29_28, FN_IP2_2_0, FN_IP2_5_3, FN_IP2_8_6, FN_IP2_11_9,
	FN_IP2_14_12, FN_IP2_17_15, FN_IP2_21_18, FN_IP2_25_22,
	FN_IP2_28_26, FN_IP3_3_0, FN_IP3_7_4, FN_IP3_11_8,
	FN_IP3_14_12, FN_IP3_17_15,

	/* GPSR1 */
	FN_IP3_19_18, FN_IP3_22_20, FN_IP3_25_23, FN_IP3_28_26,
	FN_IP3_31_29, FN_IP4_2_0, FN_IP4_5_3, FN_IP4_8_6, FN_IP4_11_9,
	FN_IP4_14_12, FN_IP4_17_15, FN_IP4_20_18, FN_IP4_23_21,
	FN_IP4_26_24, FN_IP4_29_27, FN_IP5_2_0, FN_IP5_5_3, FN_IP5_9_6,
	FN_IP5_12_10, FN_IP5_14_13, FN_IP5_17_15, FN_IP5_20_18,
	FN_IP5_23_21, FN_IP5_26_24, FN_IP5_29_27, FN_IP6_2_0,
	FN_IP6_5_3, FN_IP6_8_6, FN_IP6_10_9, FN_IP6_13_11,

	/* GPSR2 */
	FN_IP7_28_27, FN_IP7_30_29, FN_IP8_1_0, FN_IP8_3_2, FN_IP8_5_4,
	FN_IP8_7_6, FN_IP8_9_8, FN_IP8_11_10, FN_IP8_13_12, FN_IP8_15_14,
	FN_IP8_17_16, FN_IP8_19_18, FN_IP8_21_20, FN_IP8_23_22,
	FN_IP8_25_24, FN_IP8_26, FN_IP8_27, FN_VI1_DATA7_VI1_B7,
	FN_IP6_16_14, FN_IP6_19_17, FN_IP6_22_20, FN_IP6_25_23,
	FN_IP6_28_26, FN_IP6_31_29, FN_IP7_2_0, FN_IP7_5_3, FN_IP7_7_6,
	FN_IP7_9_8, FN_IP7_12_10, FN_IP7_15_13,

	/* GPSR3 */
	FN_IP8_28, FN_IP8_30_29, FN_IP9_1_0, FN_IP9_3_2, FN_IP9_5_4,
	FN_IP9_7_6, FN_IP9_11_8, FN_IP9_15_12, FN_IP9_17_16, FN_IP9_19_18,
	FN_IP9_21_20, FN_IP9_23_22, FN_IP9_25_24, FN_IP9_27_26,
	FN_IP9_31_28, FN_IP10_3_0, FN_IP10_6_4, FN_IP10_10_7, FN_IP10_14_11,
	FN_IP10_18_15, FN_IP10_22_19, FN_IP10_25_23, FN_IP10_29_26,
	FN_IP11_3_0, FN_IP11_4, FN_IP11_6_5, FN_IP11_8_7, FN_IP11_10_9,
	FN_IP11_12_11, FN_IP11_14_13, FN_IP11_17_15, FN_IP11_21_18,

	/* GPSR4 */
	FN_IP11_23_22, FN_IP11_26_24, FN_IP11_29_27, FN_IP11_31_30,
	FN_IP12_1_0, FN_IP12_3_2, FN_IP12_5_4, FN_IP12_7_6, FN_IP12_10_8,
	FN_IP12_13_11, FN_IP12_16_14, FN_IP12_19_17, FN_IP12_22_20,
	FN_IP12_24_23, FN_IP12_27_25, FN_IP12_30_28, FN_IP13_2_0,
	FN_IP13_6_3, FN_IP13_9_7, FN_IP13_12_10, FN_IP13_15_13,
	FN_IP13_18_16, FN_IP13_22_19, FN_IP13_25_23, FN_IP13_28_26,
	FN_IP13_30_29, FN_IP14_2_0, FN_IP14_5_3, FN_IP14_8_6, FN_IP14_11_9,
	FN_IP14_15_12, FN_IP14_18_16,

	/* GPSR5 */
	FN_IP14_21_19, FN_IP14_24_22, FN_IP14_27_25, FN_IP14_30_28,
	FN_IP15_2_0, FN_IP15_5_3, FN_IP15_8_6, FN_IP15_11_9, FN_IP15_13_12,
	FN_IP15_15_14, FN_IP15_17_16, FN_IP15_19_18, FN_IP15_22_20,
	FN_IP15_25_23, FN_IP15_27_26, FN_IP15_29_28, FN_IP16_2_0,
	FN_IP16_5_3, FN_USB0_PWEN, FN_USB0_OVC_VBUS, FN_IP16_6, FN_IP16_7,
	FN_USB2_PWEN, FN_USB2_OVC, FN_AVS1, FN_AVS2, FN_DU_DOTCLKIN0,
	FN_IP7_26_25, FN_DU_DOTCLKIN2, FN_IP7_18_16, FN_IP7_21_19, FN_IP7_24_22,

	/* IPSR0 - IPSR5 */
	/* IPSR6 */
	FN_DACK0, FN_IRQ0, FN_INTC_IRQ0_N, FN_SSI_SCK6_B,
	FN_VI1_VSYNC_N, FN_VI1_VSYNC_N_B, FN_SSI_WS78_C,
	FN_DREQ1_N, FN_VI1_CLKENB, FN_VI1_CLKENB_B,
	FN_SSI_SDATA7_C, FN_SSI_SCK78_B, FN_DACK1, FN_IRQ1,
	FN_INTC_IRQ1_N, FN_SSI_WS6_B, FN_SSI_SDATA8_C,
	FN_DREQ2_N, FN_HSCK1_B, FN_HCTS0_N_B,
	FN_MSIOF0_TXD_B, FN_DACK2, FN_IRQ2, FN_INTC_IRQ2_N,
	FN_SSI_SDATA6_B, FN_HRTS0_N_B, FN_MSIOF0_RXD_B,
	FN_ETH_CRS_DV, FN_RMII_CRS_DV, FN_STP_ISCLK_0_B,
	FN_TS_SDEN0_D, FN_GLO_Q0_C, FN_SCL2_E,
	FN_SCL2_CIS_E, FN_ETH_RX_ER, FN_RMII_RX_ER,
	FN_STP_ISD_0_B, FN_TS_SPSYNC0_D, FN_GLO_Q1_C,
	FN_SDA2_E, FN_SDA2_CIS_E, FN_ETH_RXD0, FN_RMII_RXD0,
	FN_STP_ISEN_0_B, FN_TS_SDAT0_D, FN_GLO_I0_C,
	FN_SCIFB1_SCK_G, FN_SCK1_E, FN_ETH_RXD1,
	FN_RMII_RXD1, FN_HRX0_E, FN_STP_ISSYNC_0_B,
	FN_TS_SCK0_D, FN_GLO_I1_C, FN_SCIFB1_RXD_G,
	FN_RX1_E, FN_ETH_LINK, FN_RMII_LINK, FN_HTX0_E,
	FN_STP_IVCXO27_0_B, FN_SCIFB1_TXD_G, FN_TX1_E,
	FN_ETH_REF_CLK, FN_RMII_REF_CLK, FN_HCTS0_N_E,
	FN_STP_IVCXO27_1_B, FN_HRX0_F,

	/* IPSR7 */
	FN_ETH_MDIO, FN_RMII_MDIO, FN_HRTS0_N_E,
	FN_SIM0_D_C, FN_HCTS0_N_F, FN_ETH_TXD1,
	FN_RMII_TXD1, FN_HTX0_F, FN_BPFCLK_G, FN_RDS_CLK_F,
	FN_ETH_TX_EN, FN_RMII_TX_EN, FN_SIM0_CLK_C,
	FN_HRTS0_N_F, FN_ETH_MAGIC, FN_RMII_MAGIC,
	FN_SIM0_RST_C, FN_ETH_TXD0, FN_RMII_TXD0,
	FN_STP_ISCLK_1_B, FN_TS_SDEN1_C, FN_GLO_SCLK_C,
	FN_ETH_MDC, FN_RMII_MDC, FN_STP_ISD_1_B,
	FN_TS_SPSYNC1_C, FN_GLO_SDATA_C, FN_PWM0,
	FN_SCIFA2_SCK_C, FN_STP_ISEN_1_B, FN_TS_SDAT1_C,
	FN_GLO_SS_C, FN_PWM1, FN_SCIFA2_TXD_C,
	FN_STP_ISSYNC_1_B, FN_TS_SCK1_C, FN_GLO_RFON_C,
	FN_PCMOE_N, FN_PWM2, FN_PWMFSW0, FN_SCIFA2_RXD_C,
	FN_PCMWE_N, FN_IECLK_C, FN_DU1_DOTCLKIN,
	FN_AUDIO_CLKC, FN_AUDIO_CLKOUT_C, FN_VI0_CLK,
	FN_ATACS00_N, FN_AVB_RXD1, FN_MII_RXD1,
	FN_VI0_DATA0_VI0_B0, FN_ATACS10_N, FN_AVB_RXD2,
	FN_MII_RXD2,

	/* IPSR8 - IPSR16 */

	FN_SEL_SCIF1_0, FN_SEL_SCIF1_1, FN_SEL_SCIF1_2, FN_SEL_SCIF1_3,
	FN_SEL_SCIF1_4,
	FN_SEL_SCIFB_0, FN_SEL_SCIFB_1, FN_SEL_SCIFB_2,
	FN_SEL_SCIFB2_0, FN_SEL_SCIFB2_1, FN_SEL_SCIFB2_2,
	FN_SEL_SCIFB1_0, FN_SEL_SCIFB1_1, FN_SEL_SCIFB1_2, FN_SEL_SCIFB1_3,
	FN_SEL_SCIFB1_4,
	FN_SEL_SCIFB1_5, FN_SEL_SCIFB1_6,
	FN_SEL_SCIFA1_0, FN_SEL_SCIFA1_1, FN_SEL_SCIFA1_2, FN_SEL_SCIFA1_3,
	FN_SEL_SCIF0_0, FN_SEL_SCIF0_1,
	FN_SEL_SCFA_0, FN_SEL_SCFA_1,
	FN_SEL_SOF1_0, FN_SEL_SOF1_1,
	FN_SEL_SSI7_0, FN_SEL_SSI7_1, FN_SEL_SSI7_2,
	FN_SEL_SSI6_0, FN_SEL_SSI6_1,
	FN_SEL_SSI5_0, FN_SEL_SSI5_1, FN_SEL_SSI5_2,
	FN_SEL_VI3_0, FN_SEL_VI3_1,
	FN_SEL_VI2_0, FN_SEL_VI2_1,
	FN_SEL_VI1_0, FN_SEL_VI1_1,
	FN_SEL_VI0_0, FN_SEL_VI0_1,
	FN_SEL_TSIF1_0, FN_SEL_TSIF1_1, FN_SEL_TSIF1_2,
	FN_SEL_LBS_0, FN_SEL_LBS_1,
	FN_SEL_TSIF0_0, FN_SEL_TSIF0_1, FN_SEL_TSIF0_2, FN_SEL_TSIF0_3,
	FN_SEL_SOF3_0, FN_SEL_SOF3_1,
	FN_SEL_SOF0_0, FN_SEL_SOF0_1,

	FN_SEL_TMU1_0, FN_SEL_TMU1_1,
	FN_SEL_HSCIF1_0, FN_SEL_HSCIF1_1,
	FN_SEL_SCIFCLK_0, FN_SEL_SCIFCLK_1,
	FN_SEL_CAN0_0, FN_SEL_CAN0_1, FN_SEL_CAN0_2, FN_SEL_CAN0_3,
	FN_SEL_CANCLK_0, FN_SEL_CANCLK_1,
	FN_SEL_SCIFA2_0, FN_SEL_SCIFA2_1, FN_SEL_SCIFA2_2,
	FN_SEL_CAN1_0, FN_SEL_CAN1_1,
	FN_SEL_ADI_0, FN_SEL_ADI_1,
	FN_SEL_SSP_0, FN_SEL_SSP_1,
	FN_SEL_FM_0, FN_SEL_FM_1, FN_SEL_FM_2, FN_SEL_FM_3,
	FN_SEL_FM_4, FN_SEL_FM_5, FN_SEL_FM_6,
	FN_SEL_HSCIF0_0, FN_SEL_HSCIF0_1, FN_SEL_HSCIF0_2, FN_SEL_HSCIF0_3,
	FN_SEL_HSCIF0_4, FN_SEL_HSCIF0_5,
	FN_SEL_GPS_0, FN_SEL_GPS_1, FN_SEL_GPS_2,
	FN_SEL_RDS_0, FN_SEL_RDS_1, FN_SEL_RDS_2,
	FN_SEL_RDS_3, FN_SEL_RDS_4, FN_SEL_RDS_5,
	FN_SEL_SIM_0, FN_SEL_SIM_1, FN_SEL_SIM_2,
	FN_SEL_SSI8_0, FN_SEL_SSI8_1, FN_SEL_SSI8_2,

	FN_SEL_IICDVFS_0, FN_SEL_IICDVFS_1,
	FN_SEL_IIC0_0, FN_SEL_IIC0_1,
	FN_SEL_IEB_0, FN_SEL_IEB_1, FN_SEL_IEB_2,
	FN_SEL_IIC2_0, FN_SEL_IIC2_1, FN_SEL_IIC2_2, FN_SEL_IIC2_3,
	FN_SEL_IIC2_4,
	FN_SEL_IIC1_0, FN_SEL_IIC1_1, FN_SEL_IIC1_2,
	FN_SEL_I2C2_0, FN_SEL_I2C2_1, FN_SEL_I2C2_2, FN_SEL_I2C2_3,
	FN_SEL_I2C2_4,
	FN_SEL_I2C1_0, FN_SEL_I2C1_1, FN_SEL_I2C1_2,

	PINMUX_FUNCTION_END,

	PINMUX_MARK_BEGIN,

	DACK0_MARK, IRQ0_MARK, INTC_IRQ0_N_MARK, SSI_SCK6_B_MARK,
	VI1_VSYNC_N_MARK, VI1_VSYNC_N_B_MARK, SSI_WS78_C_MARK,
	DREQ1_N_MARK, VI1_CLKENB_MARK, VI1_CLKENB_B_MARK,
	SSI_SDATA7_C_MARK, SSI_SCK78_B_MARK, DACK1_MARK, IRQ1_MARK,
	INTC_IRQ1_N_MARK, SSI_WS6_B_MARK, SSI_SDATA8_C_MARK,
	DREQ2_N_MARK, HSCK1_B_MARK, HCTS0_N_B_MARK,
	MSIOF0_TXD_B_MARK, DACK2_MARK, IRQ2_MARK, INTC_IRQ2_N_MARK,
	SSI_SDATA6_B_MARK, HRTS0_N_B_MARK, MSIOF0_RXD_B_MARK,
	ETH_CRS_DV_MARK, RMII_CRS_DV_MARK, STP_ISCLK_0_B_MARK,
	TS_SDEN0_D_MARK, GLO_Q0_C_MARK, SCL2_E_MARK,
	SCL2_CIS_E_MARK, ETH_RX_ER_MARK, RMII_RX_ER_MARK,
	STP_ISD_0_B_MARK, TS_SPSYNC0_D_MARK, GLO_Q1_C_MARK,
	SDA2_E_MARK, SDA2_CIS_E_MARK, ETH_RXD0_MARK, RMII_RXD0_MARK,
	STP_ISEN_0_B_MARK, TS_SDAT0_D_MARK, GLO_I0_C_MARK,
	SCIFB1_SCK_G_MARK, SCK1_E_MARK, ETH_RXD1_MARK,
	RMII_RXD1_MARK, HRX0_E_MARK, STP_ISSYNC_0_B_MARK,
	TS_SCK0_D_MARK, GLO_I1_C_MARK, SCIFB1_RXD_G_MARK,
	RX1_E_MARK, ETH_LINK_MARK, RMII_LINK_MARK, HTX0_E_MARK,
	STP_IVCXO27_0_B_MARK, SCIFB1_TXD_G_MARK, TX1_E_MARK,
	ETH_REF_CLK_MARK, RMII_REF_CLK_MARK, HCTS0_N_E_MARK,
	STP_IVCXO27_1_B_MARK, HRX0_F_MARK,

	ETH_MDIO_MARK, RMII_MDIO_MARK, HRTS0_N_E_MARK,
	SIM0_D_C_MARK, HCTS0_N_F_MARK, ETH_TXD1_MARK,
	RMII_TXD1_MARK, HTX0_F_MARK, BPFCLK_G_MARK, RDS_CLK_F_MARK,
	ETH_TX_EN_MARK, RMII_TX_EN_MARK, SIM0_CLK_C_MARK,
	HRTS0_N_F_MARK, ETH_MAGIC_MARK, RMII_MAGIC_MARK,
	SIM0_RST_C_MARK, ETH_TXD0_MARK, RMII_TXD0_MARK,
	STP_ISCLK_1_B_MARK, TS_SDEN1_C_MARK, GLO_SCLK_C_MARK,
	ETH_MDC_MARK, RMII_MDC_MARK, STP_ISD_1_B_MARK,
	TS_SPSYNC1_C_MARK, GLO_SDATA_C_MARK, PWM0_MARK,
	SCIFA2_SCK_C_MARK, STP_ISEN_1_B_MARK, TS_SDAT1_C_MARK,
	GLO_SS_C_MARK, PWM1_MARK, SCIFA2_TXD_C_MARK,
	STP_ISSYNC_1_B_MARK, TS_SCK1_C_MARK, GLO_RFON_C_MARK,
	PCMOE_N_MARK, PWM2_MARK, PWMFSW0_MARK, SCIFA2_RXD_C_MARK,
	PCMWE_N_MARK, IECLK_C_MARK, DU1_DOTCLKIN_MARK,
	AUDIO_CLKC_MARK, AUDIO_CLKOUT_C_MARK, VI0_CLK_MARK,
	ATACS00_N_MARK, AVB_RXD1_MARK, MII_RXD1_MARK,
	VI0_DATA0_VI0_B0_MARK, ATACS10_N_MARK, AVB_RXD2_MARK,
	MII_RXD2_MARK,

	PINMUX_MARK_END,
};

static pinmux_enum_t pinmux_data[] = {
	PINMUX_DATA_GP_ALL(), /* PINMUX_DATA(GP_M_N_DATA, GP_M_N_FN...), */

	PINMUX_IPSR_DATA(IP6_2_0, DACK0),
	PINMUX_IPSR_DATA(IP6_2_0, IRQ0),
	PINMUX_IPSR_DATA(IP6_2_0, INTC_IRQ0_N),
	PINMUX_IPSR_MODSEL_DATA(IP6_2_0, SSI_SCK6_B, SEL_SSI6_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_2_0, VI1_VSYNC_N, SEL_VI1_0),
	PINMUX_IPSR_MODSEL_DATA(IP6_2_0, VI1_VSYNC_N_B, SEL_VI1_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_2_0, SSI_WS78_C, SEL_SSI7_2),
	PINMUX_IPSR_DATA(IP6_5_3, DREQ1_N),
	PINMUX_IPSR_MODSEL_DATA(IP6_5_3, VI1_CLKENB, SEL_VI1_0),
	PINMUX_IPSR_MODSEL_DATA(IP6_5_3, VI1_CLKENB_B, SEL_VI1_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_5_3, SSI_SDATA7_C, SEL_SSI7_2),
	PINMUX_IPSR_MODSEL_DATA(IP6_5_3, SSI_SCK78_B, SEL_SSI7_1),
	PINMUX_IPSR_DATA(IP6_8_6, DACK1),
	PINMUX_IPSR_DATA(IP6_8_6, IRQ1),
	PINMUX_IPSR_DATA(IP6_8_6, INTC_IRQ1_N),
	PINMUX_IPSR_MODSEL_DATA(IP6_8_6, SSI_WS6_B, SEL_SSI6_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_8_6, SSI_SDATA8_C, SEL_SSI8_2),
	PINMUX_IPSR_DATA(IP6_10_9, DREQ2_N),
	PINMUX_IPSR_MODSEL_DATA(IP6_10_9, HSCK1_B, SEL_HSCIF1_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_10_9, HCTS0_N_B, SEL_HSCIF0_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_10_9, MSIOF0_TXD_B, SEL_SOF0_1),
	PINMUX_IPSR_DATA(IP6_13_11, DACK2),
	PINMUX_IPSR_DATA(IP6_13_11, IRQ2),
	PINMUX_IPSR_DATA(IP6_13_11, INTC_IRQ2_N),
	PINMUX_IPSR_MODSEL_DATA(IP6_13_11, SSI_SDATA6_B, SEL_SSI6_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_13_11, HRTS0_N_B, SEL_HSCIF0_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_13_11, MSIOF0_RXD_B, SEL_SOF0_1),
	PINMUX_IPSR_DATA(IP6_16_14, ETH_CRS_DV),
	PINMUX_IPSR_DATA(IP6_16_14, RMII_CRS_DV),
	PINMUX_IPSR_MODSEL_DATA(IP6_16_14, STP_ISCLK_0_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_16_14, TS_SDEN0_D, SEL_TSIF0_3),
	PINMUX_IPSR_MODSEL_DATA(IP6_16_14, GLO_Q0_C, SEL_GPS_2),
	PINMUX_IPSR_MODSEL_DATA(IP6_16_14, SCL2_E, SEL_IIC2_4),
	PINMUX_IPSR_MODSEL_DATA(IP6_16_14, SCL2_CIS_E, SEL_I2C2_4),
	PINMUX_IPSR_DATA(IP6_19_17, ETH_RX_ER),
	PINMUX_IPSR_DATA(IP6_19_17, RMII_RX_ER),
	PINMUX_IPSR_MODSEL_DATA(IP6_19_17, STP_ISD_0_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_19_17, TS_SPSYNC0_D, SEL_TSIF0_3),
	PINMUX_IPSR_MODSEL_DATA(IP6_19_17, GLO_Q1_C, SEL_GPS_2),
	PINMUX_IPSR_MODSEL_DATA(IP6_19_17, SDA2_E, SEL_IIC2_4),
	PINMUX_IPSR_MODSEL_DATA(IP6_19_17, SDA2_CIS_E, SEL_I2C2_4),
	PINMUX_IPSR_DATA(IP6_22_20, ETH_RXD0),
	PINMUX_IPSR_DATA(IP6_22_20, RMII_RXD0),
	PINMUX_IPSR_MODSEL_DATA(IP6_22_20, STP_ISEN_0_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_22_20, TS_SDAT0_D, SEL_TSIF0_3),
	PINMUX_IPSR_MODSEL_DATA(IP6_22_20, GLO_I0_C, SEL_GPS_2),
	PINMUX_IPSR_MODSEL_DATA(IP6_22_20, SCIFB1_SCK_G, SEL_SCIFB1_6),
	PINMUX_IPSR_MODSEL_DATA(IP6_22_20, SCK1_E, SEL_SCIF1_4),
	PINMUX_IPSR_DATA(IP6_25_23, ETH_RXD1),
	PINMUX_IPSR_DATA(IP6_25_23, RMII_RXD1),
	PINMUX_IPSR_MODSEL_DATA(IP6_25_23, HRX0_E, SEL_HSCIF0_4),
	PINMUX_IPSR_MODSEL_DATA(IP6_25_23, STP_ISSYNC_0_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_25_23, TS_SCK0_D, SEL_TSIF0_3),
	PINMUX_IPSR_MODSEL_DATA(IP6_25_23, GLO_I1_C, SEL_GPS_2),
	PINMUX_IPSR_MODSEL_DATA(IP6_25_23, SCIFB1_RXD_G, SEL_SCIFB1_6),
	PINMUX_IPSR_MODSEL_DATA(IP6_25_23, RX1_E, SEL_SCIF1_4),
	PINMUX_IPSR_DATA(IP6_28_26, ETH_LINK),
	PINMUX_IPSR_DATA(IP6_28_26, RMII_LINK),
	PINMUX_IPSR_MODSEL_DATA(IP6_28_26, HTX0_E, SEL_HSCIF0_4),
	PINMUX_IPSR_MODSEL_DATA(IP6_28_26, STP_IVCXO27_0_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_28_26, SCIFB1_TXD_G, SEL_SCIFB1_6),
	PINMUX_IPSR_MODSEL_DATA(IP6_28_26, TX1_E, SEL_SCIF1_4),
	PINMUX_IPSR_DATA(IP6_31_29, ETH_REF_CLK),
	PINMUX_IPSR_DATA(IP6_31_29, RMII_REF_CLK),
	PINMUX_IPSR_MODSEL_DATA(IP6_31_29, HCTS0_N_E, SEL_HSCIF0_4),
	PINMUX_IPSR_MODSEL_DATA(IP6_31_29, STP_IVCXO27_1_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP6_31_29, HRX0_F, SEL_HSCIF0_5),

	PINMUX_IPSR_DATA(IP7_2_0, ETH_MDIO),
	PINMUX_IPSR_DATA(IP7_2_0, RMII_MDIO),
	PINMUX_IPSR_MODSEL_DATA(IP7_2_0, HRTS0_N_E, SEL_HSCIF0_4),
	PINMUX_IPSR_MODSEL_DATA(IP7_2_0, SIM0_D_C, SEL_SIM_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_2_0, HCTS0_N_F, SEL_HSCIF0_5),
	PINMUX_IPSR_DATA(IP7_5_3, ETH_TXD1),
	PINMUX_IPSR_DATA(IP7_5_3, RMII_TXD1),
	PINMUX_IPSR_MODSEL_DATA(IP7_5_3, HTX0_F, SEL_HSCIF0_4),
	PINMUX_IPSR_MODSEL_DATA(IP7_5_3, BPFCLK_G, SEL_SIM_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_5_3, RDS_CLK_F, SEL_HSCIF0_5),
	PINMUX_IPSR_DATA(IP7_7_6, ETH_TX_EN),
	PINMUX_IPSR_DATA(IP7_7_6, RMII_TX_EN),
	PINMUX_IPSR_MODSEL_DATA(IP7_7_6, SIM0_CLK_C, SEL_SIM_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_7_6, HRTS0_N_F, SEL_HSCIF0_5),
	PINMUX_IPSR_DATA(IP7_9_8, ETH_MAGIC),
	PINMUX_IPSR_DATA(IP7_9_8, RMII_MAGIC),
	PINMUX_IPSR_MODSEL_DATA(IP7_9_8, SIM0_RST_C, SEL_SIM_2),
	PINMUX_IPSR_DATA(IP7_12_10, ETH_TXD0),
	PINMUX_IPSR_DATA(IP7_12_10, RMII_TXD0),
	PINMUX_IPSR_MODSEL_DATA(IP7_12_10, STP_ISCLK_1_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP7_12_10, TS_SDEN1_C, SEL_TSIF1_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_12_10, GLO_SCLK_C, SEL_GPS_2),
	PINMUX_IPSR_DATA(IP7_15_13, ETH_MDC),
	PINMUX_IPSR_DATA(IP7_15_13, RMII_MDC),
	PINMUX_IPSR_MODSEL_DATA(IP7_15_13, STP_ISD_1_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP7_15_13, TS_SPSYNC1_C, SEL_TSIF1_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_15_13, GLO_SDATA_C, SEL_GPS_2),
	PINMUX_IPSR_DATA(IP7_18_16, PWM0),
	PINMUX_IPSR_MODSEL_DATA(IP7_18_16, SCIFA2_SCK_C, SEL_SCIFA2_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_18_16, STP_ISEN_1_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP7_18_16, TS_SDAT1_C, SEL_TSIF1_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_18_16, GLO_SS_C, SEL_GPS_2),
	PINMUX_IPSR_DATA(IP7_21_19, PWM1),
	PINMUX_IPSR_MODSEL_DATA(IP7_21_19, SCIFA2_TXD_C, SEL_SCIFA2_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_21_19, STP_ISSYNC_1_B, SEL_SSP_1),
	PINMUX_IPSR_MODSEL_DATA(IP7_21_19, TS_SCK1_C, SEL_TSIF1_2),
	PINMUX_IPSR_MODSEL_DATA(IP7_21_19, GLO_RFON_C, SEL_GPS_2),
	PINMUX_IPSR_DATA(IP7_21_19, PCMOE_N),
	PINMUX_IPSR_DATA(IP7_24_22, PWM2),
	PINMUX_IPSR_DATA(IP7_24_22, PWMFSW0),
	PINMUX_IPSR_MODSEL_DATA(IP7_24_22, SCIFA2_RXD_C, SEL_SCIFA2_2),
	PINMUX_IPSR_DATA(IP7_24_22, PCMWE_N),
	PINMUX_IPSR_MODSEL_DATA(IP7_24_22, IECLK_C, SEL_IEB_2),
	PINMUX_IPSR_DATA(IP7_26_25, DU1_DOTCLKIN),
	PINMUX_IPSR_DATA(IP7_26_25, AUDIO_CLKC),
	PINMUX_IPSR_DATA(IP7_26_25, AUDIO_CLKOUT_C),
	PINMUX_IPSR_MODSEL_DATA(IP7_28_27, VI0_CLK, SEL_VI0_0),
	PINMUX_IPSR_DATA(IP7_28_27, ATACS00_N),
	PINMUX_IPSR_DATA(IP7_28_27, AVB_RXD1),
	PINMUX_IPSR_DATA(IP7_28_27, MII_RXD1),
	PINMUX_IPSR_MODSEL_DATA(IP7_30_29, VI0_DATA0_VI0_B0, SEL_VI0_0),
	PINMUX_IPSR_DATA(IP7_30_29, ATACS10_N),
	PINMUX_IPSR_DATA(IP7_30_29, AVB_RXD2),
	PINMUX_IPSR_DATA(IP7_30_29, MII_RXD2),

};

static struct pinmux_gpio pinmux_gpios[] = {
	PINMUX_GPIO_GP_ALL(),

	/*IPSR0 - IPSR5*/
	/*IPSR6*/
	GPIO_FN(DACK0), GPIO_FN(IRQ0), GPIO_FN(INTC_IRQ0_N),
	GPIO_FN(SSI_SCK6_B), GPIO_FN(VI1_VSYNC_N), GPIO_FN(VI1_VSYNC_N_B),
	GPIO_FN(SSI_WS78_C), GPIO_FN(DREQ1_N), GPIO_FN(VI1_CLKENB),
	GPIO_FN(VI1_CLKENB_B), GPIO_FN(SSI_SDATA7_C), GPIO_FN(SSI_SCK78_B),
	GPIO_FN(DACK1), GPIO_FN(IRQ1), GPIO_FN(INTC_IRQ1_N), GPIO_FN(SSI_WS6_B),
	GPIO_FN(SSI_SDATA8_C), GPIO_FN(DREQ2_N), GPIO_FN(HSCK1_B),
	GPIO_FN(HCTS0_N_B), GPIO_FN(MSIOF0_TXD_B), GPIO_FN(DACK2),
	GPIO_FN(IRQ2), GPIO_FN(INTC_IRQ2_N), GPIO_FN(SSI_SDATA6_B),
	GPIO_FN(HRTS0_N_B), GPIO_FN(MSIOF0_RXD_B), GPIO_FN(ETH_CRS_DV),
	GPIO_FN(RMII_CRS_DV), GPIO_FN(STP_ISCLK_0_B), GPIO_FN(TS_SDEN0_D),
	GPIO_FN(GLO_Q0_C), GPIO_FN(SCL2_E), GPIO_FN(SCL2_CIS_E),
	GPIO_FN(ETH_RX_ER), GPIO_FN(RMII_RX_ER), GPIO_FN(STP_ISD_0_B),
	GPIO_FN(TS_SPSYNC0_D), GPIO_FN(GLO_Q1_C), GPIO_FN(SDA2_E),
	GPIO_FN(SDA2_CIS_E), GPIO_FN(ETH_RXD0), GPIO_FN(RMII_RXD0),
	GPIO_FN(STP_ISEN_0_B), GPIO_FN(TS_SDAT0_D), GPIO_FN(GLO_I0_C),
	GPIO_FN(SCIFB1_SCK_G), GPIO_FN(SCK1_E), GPIO_FN(ETH_RXD1),
	GPIO_FN(RMII_RXD1), GPIO_FN(HRX0_E), GPIO_FN(STP_ISSYNC_0_B),
	GPIO_FN(TS_SCK0_D), GPIO_FN(GLO_I1_C), GPIO_FN(SCIFB1_RXD_G),
	GPIO_FN(RX1_E), GPIO_FN(ETH_LINK), GPIO_FN(RMII_LINK), GPIO_FN(HTX0_E),
	GPIO_FN(STP_IVCXO27_0_B), GPIO_FN(SCIFB1_TXD_G), GPIO_FN(TX1_E),
	GPIO_FN(ETH_REF_CLK), GPIO_FN(RMII_REF_CLK), GPIO_FN(HCTS0_N_E),
	GPIO_FN(STP_IVCXO27_1_B), GPIO_FN(HRX0_F),

	/*IPSR7*/
	GPIO_FN(ETH_MDIO), GPIO_FN(RMII_MDIO), GPIO_FN(HRTS0_N_E),
	GPIO_FN(SIM0_D_C), GPIO_FN(HCTS0_N_F), GPIO_FN(ETH_TXD1),
	GPIO_FN(RMII_TXD1), GPIO_FN(HTX0_F), GPIO_FN(BPFCLK_G),
	GPIO_FN(RDS_CLK_F), GPIO_FN(ETH_TX_EN), GPIO_FN(RMII_TX_EN),
	GPIO_FN(SIM0_CLK_C), GPIO_FN(HRTS0_N_F), GPIO_FN(ETH_MAGIC),
	GPIO_FN(RMII_MAGIC), GPIO_FN(SIM0_RST_C), GPIO_FN(ETH_TXD0),
	GPIO_FN(RMII_TXD0), GPIO_FN(STP_ISCLK_1_B), GPIO_FN(TS_SDEN1_C),
	GPIO_FN(GLO_SCLK_C), GPIO_FN(ETH_MDC), GPIO_FN(RMII_MDC),
	GPIO_FN(STP_ISD_1_B), GPIO_FN(TS_SPSYNC1_C), GPIO_FN(GLO_SDATA_C),
	GPIO_FN(PWM0), GPIO_FN(SCIFA2_SCK_C), GPIO_FN(STP_ISEN_1_B),
	GPIO_FN(TS_SDAT1_C), GPIO_FN(GLO_SS_C), GPIO_FN(PWM1),
	GPIO_FN(SCIFA2_TXD_C), GPIO_FN(STP_ISSYNC_1_B), GPIO_FN(TS_SCK1_C),
	GPIO_FN(GLO_RFON_C), GPIO_FN(PCMOE_N), GPIO_FN(PWM2), GPIO_FN(PWMFSW0),
	GPIO_FN(SCIFA2_RXD_C), GPIO_FN(PCMWE_N), GPIO_FN(IECLK_C),
	GPIO_FN(DU1_DOTCLKIN), GPIO_FN(AUDIO_CLKC), GPIO_FN(AUDIO_CLKOUT_C),
	GPIO_FN(VI0_CLK), GPIO_FN(ATACS00_N), GPIO_FN(AVB_RXD1),
	GPIO_FN(MII_RXD1), GPIO_FN(VI0_DATA0_VI0_B0), GPIO_FN(ATACS10_N),
	GPIO_FN(AVB_RXD2), GPIO_FN(MII_RXD2),
	/*IPSR8 - IPSR16*/
};

static struct pinmux_cfg_reg pinmux_config_regs[] = {
	{ PINMUX_CFG_REG("GPSR0", 0xE6060004, 32, 1) {
		GP_0_31_FN, FN_IP3_17_15,
		GP_0_30_FN, FN_IP3_14_12,
		GP_0_29_FN, FN_IP3_11_8,
		GP_0_28_FN, FN_IP3_7_4,
		GP_0_27_FN, FN_IP3_3_0,
		GP_0_26_FN, FN_IP2_28_26,
		GP_0_25_FN, FN_IP2_25_22,
		GP_0_24_FN, FN_IP2_21_18,
		GP_0_23_FN, FN_IP2_17_15,
		GP_0_22_FN, FN_IP2_14_12,
		GP_0_21_FN, FN_IP2_11_9,
		GP_0_20_FN, FN_IP2_8_6,
		GP_0_19_FN, FN_IP2_5_3,
		GP_0_18_FN, FN_IP2_2_0,
		GP_0_17_FN, FN_IP1_29_28,
		GP_0_16_FN, FN_IP1_27_26,
		GP_0_15_FN, FN_IP1_25_22,
		GP_0_14_FN, FN_IP1_21_18,
		GP_0_13_FN, FN_IP1_17_15,
		GP_0_12_FN, FN_IP1_14_12,
		GP_0_11_FN, FN_IP1_11_8,
		GP_0_10_FN, FN_IP1_7_4,
		GP_0_9_FN, FN_IP1_3_0,
		GP_0_8_FN, FN_IP0_30_27,
		GP_0_7_FN, FN_IP0_26_23,
		GP_0_6_FN, FN_IP0_22_20,
		GP_0_5_FN, FN_IP0_19_16,
		GP_0_4_FN, FN_IP0_15_12,
		GP_0_3_FN, FN_IP0_11_9,
		GP_0_2_FN, FN_IP0_8_6,
		GP_0_1_FN, FN_IP0_5_3,
		GP_0_0_FN, FN_IP0_2_0 }
	},
	{ PINMUX_CFG_REG("GPSR1", 0xE6060008, 32, 1) {
		0, 0,
		0, 0,
		GP_1_29_FN, FN_IP6_13_11,
		GP_1_28_FN, FN_IP6_10_9,
		GP_1_27_FN, FN_IP6_8_6,
		GP_1_26_FN, FN_IP6_5_3,
		GP_1_25_FN, FN_IP6_2_0,
		GP_1_24_FN, FN_IP5_29_27,
		GP_1_23_FN, FN_IP5_26_24,
		GP_1_22_FN, FN_IP5_23_21,
		GP_1_21_FN, FN_IP5_20_18,
		GP_1_20_FN, FN_IP5_17_15,
		GP_1_19_FN, FN_IP5_14_13,
		GP_1_18_FN, FN_IP5_12_10,
		GP_1_17_FN, FN_IP5_9_6,
		GP_1_16_FN, FN_IP5_5_3,
		GP_1_15_FN, FN_IP5_2_0,
		GP_1_14_FN, FN_IP4_29_27,
		GP_1_13_FN, FN_IP4_26_24,
		GP_1_12_FN, FN_IP4_23_21,
		GP_1_11_FN, FN_IP4_20_18,
		GP_1_10_FN, FN_IP4_17_15,
		GP_1_9_FN, FN_IP4_14_12,
		GP_1_8_FN, FN_IP4_11_9,
		GP_1_7_FN, FN_IP4_8_6,
		GP_1_6_FN, FN_IP4_5_3,
		GP_1_5_FN, FN_IP4_2_0,
		GP_1_4_FN, FN_IP3_31_29,
		GP_1_3_FN, FN_IP3_28_26,
		GP_1_2_FN, FN_IP3_25_23,
		GP_1_1_FN, FN_IP3_22_20,
		GP_1_0_FN, FN_IP3_19_18, }
	},
	{ PINMUX_CFG_REG("GPSR2", 0xE606000C, 32, 1) {
		0, 0,
		0, 0,
		GP_2_29_FN, FN_IP7_15_13,
		GP_2_28_FN, FN_IP7_12_10,
		GP_2_27_FN, FN_IP7_9_8,
		GP_2_26_FN, FN_IP7_7_6,
		GP_2_25_FN, FN_IP7_5_3,
		GP_2_24_FN, FN_IP7_2_0,
		GP_2_23_FN, FN_IP6_31_29,
		GP_2_22_FN, FN_IP6_28_26,
		GP_2_21_FN, FN_IP6_25_23,
		GP_2_20_FN, FN_IP6_22_20,
		GP_2_19_FN, FN_IP6_19_17,
		GP_2_18_FN, FN_IP6_16_14,
		GP_2_17_FN, FN_VI1_DATA7_VI1_B7,
		GP_2_16_FN, FN_IP8_27,
		GP_2_15_FN, FN_IP8_26,
		GP_2_14_FN, FN_IP8_25_24,
		GP_2_13_FN, FN_IP8_23_22,
		GP_2_12_FN, FN_IP8_21_20,
		GP_2_11_FN, FN_IP8_19_18,
		GP_2_10_FN, FN_IP8_17_16,
		GP_2_9_FN, FN_IP8_15_14,
		GP_2_8_FN, FN_IP8_13_12,
		GP_2_7_FN, FN_IP8_11_10,
		GP_2_6_FN, FN_IP8_9_8,
		GP_2_5_FN, FN_IP8_7_6,
		GP_2_4_FN, FN_IP8_5_4,
		GP_2_3_FN, FN_IP8_3_2,
		GP_2_2_FN, FN_IP8_1_0,
		GP_2_1_FN, FN_IP7_30_29,
		GP_2_0_FN, FN_IP7_28_27 }
	},
	{ PINMUX_CFG_REG("GPSR3", 0xE6060010, 32, 1) {
		GP_3_31_FN, FN_IP11_21_18,
		GP_3_30_FN, FN_IP11_17_15,
		GP_3_29_FN, FN_IP11_14_13,
		GP_3_28_FN, FN_IP11_12_11,
		GP_3_27_FN, FN_IP11_10_9,
		GP_3_26_FN, FN_IP11_8_7,
		GP_3_25_FN, FN_IP11_6_5,
		GP_3_24_FN, FN_IP11_4,
		GP_3_23_FN, FN_IP11_3_0,
		GP_3_22_FN, FN_IP10_29_26,
		GP_3_21_FN, FN_IP10_25_23,
		GP_3_20_FN, FN_IP10_22_19,
		GP_3_19_FN, FN_IP10_18_15,
		GP_3_18_FN, FN_IP10_14_11,
		GP_3_17_FN, FN_IP10_10_7,
		GP_3_16_FN, FN_IP10_6_4,
		GP_3_15_FN, FN_IP10_3_0,
		GP_3_14_FN, FN_IP9_31_28,
		GP_3_13_FN, FN_IP9_27_26,
		GP_3_12_FN, FN_IP9_25_24,
		GP_3_11_FN, FN_IP9_23_22,
		GP_3_10_FN, FN_IP9_21_20,
		GP_3_9_FN, FN_IP9_19_18,
		GP_3_8_FN, FN_IP9_17_16,
		GP_3_7_FN, FN_IP9_15_12,
		GP_3_6_FN, FN_IP9_11_8,
		GP_3_5_FN, FN_IP9_7_6,
		GP_3_4_FN, FN_IP9_5_4,
		GP_3_3_FN, FN_IP9_3_2,
		GP_3_2_FN, FN_IP9_1_0,
		GP_3_1_FN, FN_IP8_30_29,
		GP_3_0_FN, FN_IP8_28 }
	},
	{ PINMUX_CFG_REG("GPSR4", 0xE6060014, 32, 1) {
		GP_4_31_FN, FN_IP14_18_16,
		GP_4_30_FN, FN_IP14_15_12,
		GP_4_29_FN, FN_IP14_11_9,
		GP_4_28_FN, FN_IP14_8_6,
		GP_4_27_FN, FN_IP14_5_3,
		GP_4_26_FN, FN_IP14_2_0,
		GP_4_25_FN, FN_IP13_30_29,
		GP_4_24_FN, FN_IP13_28_26,
		GP_4_23_FN, FN_IP13_25_23,
		GP_4_22_FN, FN_IP13_22_19,
		GP_4_21_FN, FN_IP13_18_16,
		GP_4_20_FN, FN_IP13_15_13,
		GP_4_19_FN, FN_IP13_12_10,
		GP_4_18_FN, FN_IP13_9_7,
		GP_4_17_FN, FN_IP13_6_3,
		GP_4_16_FN, FN_IP13_2_0,
		GP_4_15_FN, FN_IP12_30_28,
		GP_4_14_FN, FN_IP12_27_25,
		GP_4_13_FN, FN_IP12_24_23,
		GP_4_12_FN, FN_IP12_22_20,
		GP_4_11_FN, FN_IP12_19_17,
		GP_4_10_FN, FN_IP12_16_14,
		GP_4_9_FN, FN_IP12_13_11,
		GP_4_8_FN, FN_IP12_10_8,
		GP_4_7_FN, FN_IP12_7_6,
		GP_4_6_FN, FN_IP12_5_4,
		GP_4_5_FN, FN_IP12_3_2,
		GP_4_4_FN, FN_IP12_1_0,
		GP_4_3_FN, FN_IP11_31_30,
		GP_4_2_FN, FN_IP11_29_27,
		GP_4_1_FN, FN_IP11_26_24,
		GP_4_0_FN, FN_IP11_23_22 }
	},
	{ PINMUX_CFG_REG("GPSR5", 0xE6060018, 32, 1) {
		GP_5_31_FN, FN_IP7_24_22,
		GP_5_30_FN, FN_IP7_21_19,
		GP_5_29_FN, FN_IP7_18_16,
		GP_5_28_FN, FN_DU_DOTCLKIN2,
		GP_5_27_FN, FN_IP7_26_25,
		GP_5_26_FN, FN_DU_DOTCLKIN0,
		GP_5_25_FN, FN_AVS2,
		GP_5_24_FN, FN_AVS1,
		GP_5_23_FN, FN_USB2_OVC,
		GP_5_22_FN, FN_USB2_PWEN,
		GP_5_21_FN, FN_IP16_7,
		GP_5_20_FN, FN_IP16_6,
		GP_5_19_FN, FN_USB0_OVC_VBUS,
		GP_5_18_FN, FN_USB0_PWEN,
		GP_5_17_FN, FN_IP16_5_3,
		GP_5_16_FN, FN_IP16_2_0,
		GP_5_15_FN, FN_IP15_29_28,
		GP_5_14_FN, FN_IP15_27_26,
		GP_5_13_FN, FN_IP15_25_23,
		GP_5_12_FN, FN_IP15_22_20,
		GP_5_11_FN, FN_IP15_19_18,
		GP_5_10_FN, FN_IP15_17_16,
		GP_5_9_FN, FN_IP15_15_14,
		GP_5_8_FN, FN_IP15_13_12,
		GP_5_7_FN, FN_IP15_11_9,
		GP_5_6_FN, FN_IP15_8_6,
		GP_5_5_FN, FN_IP15_5_3,
		GP_5_4_FN, FN_IP15_2_0,
		GP_5_3_FN, FN_IP14_30_28,
		GP_5_2_FN, FN_IP14_27_25,
		GP_5_1_FN, FN_IP14_24_22,
		GP_5_0_FN, FN_IP14_21_19 }
	},

	/*IPSR0 - IPSR5*/
	{ PINMUX_CFG_REG_VAR("IPSR6", 0xE6060038, 32,
			     3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3) {
		/* IP6_31_29 [3] */
		FN_ETH_REF_CLK, FN_RMII_REF_CLK, FN_HCTS0_N_E,
		FN_STP_IVCXO27_1_B, FN_HRX0_F, 0, 0, 0,
		/* IP6_28_26 [3] */
		FN_ETH_LINK, FN_RMII_LINK, FN_HTX0_E,
		FN_STP_IVCXO27_0_B, FN_SCIFB1_TXD_G, FN_TX1_E, 0, 0,
		/* IP6_25_23 [3] */
		FN_ETH_RXD1, FN_RMII_RXD1, FN_HRX0_E, FN_STP_ISSYNC_0_B,
		FN_TS_SCK0_D, FN_GLO_I1_C, FN_SCIFB1_RXD_G, FN_RX1_E,
		/* IP6_22_20 [3] */
		FN_ETH_RXD0, FN_RMII_RXD0, FN_STP_ISEN_0_B, FN_TS_SDAT0_D,
		FN_GLO_I0_C, FN_SCIFB1_SCK_G, FN_SCK1_E, 0,
		/* IP6_19_17 [3] */
		FN_ETH_RX_ER, FN_RMII_RX_ER, FN_STP_ISD_0_B,
		FN_TS_SPSYNC0_D, FN_GLO_Q1_C, FN_SDA2_E, FN_SDA2_CIS_E, 0,
		/* IP6_16_14 [3] */
		FN_ETH_CRS_DV, FN_RMII_CRS_DV, FN_STP_ISCLK_0_B,
		FN_TS_SDEN0_D, FN_GLO_Q0_C, FN_SCL2_E,
		FN_SCL2_CIS_E, 0,
		/* IP6_13_11 [3] */
		FN_DACK2, FN_IRQ2, FN_INTC_IRQ2_N,
		FN_SSI_SDATA6_B, FN_HRTS0_N_B, FN_MSIOF0_RXD_B, 0, 0,
		/* IP6_10_9 [2] */
		FN_DREQ2_N, FN_HSCK1_B, FN_HCTS0_N_B, FN_MSIOF0_TXD_B,
		/* IP6_8_6 [3] */
		FN_DACK1, FN_IRQ1, FN_INTC_IRQ1_N, FN_SSI_WS6_B,
		FN_SSI_SDATA8_C, 0, 0, 0,
		/* IP6_5_3 [3] */
		FN_DREQ1_N, FN_VI1_CLKENB, FN_VI1_CLKENB_B,
		FN_SSI_SDATA7_C, FN_SSI_SCK78_B, 0, 0, 0,
		/* IP6_2_0 [3] */
		FN_DACK0, FN_IRQ0, FN_INTC_IRQ0_N, FN_SSI_SCK6_B,
		FN_VI1_VSYNC_N, FN_VI1_VSYNC_N_B, FN_SSI_WS78_C, 0, }
	},
	{ PINMUX_CFG_REG_VAR("IPSR7", 0xE606003C, 32,
			     1, 2, 2, 2, 3, 3, 3, 3, 3, 2, 2, 3, 3) {
		/* IP7_31 [1] */
		0, 0,
		/* IP7_30_29 [2] */
		FN_VI0_DATA0_VI0_B0, FN_ATACS10_N, FN_AVB_RXD2,
		FN_MII_RXD2,
		/* IP7_28_27 [2] */
		FN_VI0_CLK, FN_ATACS00_N, FN_AVB_RXD1, FN_MII_RXD1,
		/* IP7_26_25 [2] */
		FN_DU1_DOTCLKIN, FN_AUDIO_CLKC, FN_AUDIO_CLKOUT_C, 0,
		/* IP7_24_22 [3] */
		FN_PWM2, FN_PWMFSW0, FN_SCIFA2_RXD_C, FN_PCMWE_N, FN_IECLK_C,
		0, 0, 0,
		/* IP7_21_19 [3] */
		FN_PWM1, FN_SCIFA2_TXD_C, FN_STP_ISSYNC_1_B, FN_TS_SCK1_C,
		FN_GLO_RFON_C, FN_PCMOE_N, 0, 0,
		/* IP7_18_16 [3] */
		FN_PWM0, FN_SCIFA2_SCK_C, FN_STP_ISEN_1_B, FN_TS_SDAT1_C,
		FN_GLO_SS_C, 0, 0, 0,
		/* IP7_15_13 [3] */
		FN_ETH_MDC, FN_RMII_MDC, FN_STP_ISD_1_B,
		FN_TS_SPSYNC1_C, FN_GLO_SDATA_C, 0, 0, 0,
		/* IP7_12_10 [3] */
		FN_ETH_TXD0, FN_RMII_TXD0, FN_STP_ISCLK_1_B, FN_TS_SDEN1_C,
		FN_GLO_SCLK_C, 0, 0, 0,
		/* IP7_9_8 [2] */
		FN_ETH_MAGIC, FN_RMII_MAGIC, FN_SIM0_RST_C, 0,
		/* IP7_7_6 [2] */
		FN_ETH_TX_EN, FN_RMII_TX_EN, FN_SIM0_CLK_C, FN_HRTS0_N_F,
		/* IP7_5_3 [3] */
		FN_ETH_TXD1, FN_RMII_TXD1, FN_HTX0_F, FN_BPFCLK_G, FN_RDS_CLK_F,
		0, 0, 0,
		/* IP7_2_0 [3] */
		FN_ETH_MDIO, FN_RMII_MDIO, FN_HRTS0_N_E,
		FN_SIM0_D_C, FN_HCTS0_N_F, 0, 0, 0, }
	},
	/*IPSR8 - IPSR16*/
	{ PINMUX_CFG_REG("INOUTSEL0", 0xE6050004, 32, 1) { GP_INOUTSEL(0) } },
	{ PINMUX_CFG_REG("INOUTSEL1", 0xE6051004, 32, 1) {
		0, 0,
		0, 0,
		GP_1_29_IN, GP_1_29_OUT,
		GP_1_28_IN, GP_1_28_OUT,
		GP_1_27_IN, GP_1_27_OUT,
		GP_1_26_IN, GP_1_26_OUT,
		GP_1_25_IN, GP_1_25_OUT,
		GP_1_24_IN, GP_1_24_OUT,
		GP_1_23_IN, GP_1_23_OUT,
		GP_1_22_IN, GP_1_22_OUT,
		GP_1_21_IN, GP_1_21_OUT,
		GP_1_20_IN, GP_1_20_OUT,
		GP_1_19_IN, GP_1_19_OUT,
		GP_1_18_IN, GP_1_18_OUT,
		GP_1_17_IN, GP_1_17_OUT,
		GP_1_16_IN, GP_1_16_OUT,
		GP_1_15_IN, GP_1_15_OUT,
		GP_1_14_IN, GP_1_14_OUT,
		GP_1_13_IN, GP_1_13_OUT,
		GP_1_12_IN, GP_1_12_OUT,
		GP_1_11_IN, GP_1_11_OUT,
		GP_1_10_IN, GP_1_10_OUT,
		GP_1_9_IN, GP_1_9_OUT,
		GP_1_8_IN, GP_1_8_OUT,
		GP_1_7_IN, GP_1_7_OUT,
		GP_1_6_IN, GP_1_6_OUT,
		GP_1_5_IN, GP_1_5_OUT,
		GP_1_4_IN, GP_1_4_OUT,
		GP_1_3_IN, GP_1_3_OUT,
		GP_1_2_IN, GP_1_2_OUT,
		GP_1_1_IN, GP_1_1_OUT,
		GP_1_0_IN, GP_1_0_OUT, }
	},
	{ PINMUX_CFG_REG("INOUTSEL2", 0xE6052004, 32, 1) {
		0, 0,
		0, 0,
		GP_2_29_IN, GP_2_29_OUT,
		GP_2_28_IN, GP_2_28_OUT,
		GP_2_27_IN, GP_2_27_OUT,
		GP_2_26_IN, GP_2_26_OUT,
		GP_2_25_IN, GP_2_25_OUT,
		GP_2_24_IN, GP_2_24_OUT,
		GP_2_23_IN, GP_2_23_OUT,
		GP_2_22_IN, GP_2_22_OUT,
		GP_2_21_IN, GP_2_21_OUT,
		GP_2_20_IN, GP_2_20_OUT,
		GP_2_19_IN, GP_2_19_OUT,
		GP_2_18_IN, GP_2_18_OUT,
		GP_2_17_IN, GP_2_17_OUT,
		GP_2_16_IN, GP_2_16_OUT,
		GP_2_15_IN, GP_2_15_OUT,
		GP_2_14_IN, GP_2_14_OUT,
		GP_2_13_IN, GP_2_13_OUT,
		GP_2_12_IN, GP_2_12_OUT,
		GP_2_11_IN, GP_2_11_OUT,
		GP_2_10_IN, GP_2_10_OUT,
		GP_2_9_IN, GP_2_9_OUT,
		GP_2_8_IN, GP_2_8_OUT,
		GP_2_7_IN, GP_2_7_OUT,
		GP_2_6_IN, GP_2_6_OUT,
		GP_2_5_IN, GP_2_5_OUT,
		GP_2_4_IN, GP_2_4_OUT,
		GP_2_3_IN, GP_2_3_OUT,
		GP_2_2_IN, GP_2_2_OUT,
		GP_2_1_IN, GP_2_1_OUT,
		GP_2_0_IN, GP_2_0_OUT, }
	},
	{ PINMUX_CFG_REG("INOUTSEL3", 0xE6053004, 32, 1) { GP_INOUTSEL(3) } },
	{ PINMUX_CFG_REG("INOUTSEL4", 0xE6054004, 32, 1) { GP_INOUTSEL(4) } },
	{ PINMUX_CFG_REG("INOUTSEL5", 0xE6055004, 32, 1) { GP_INOUTSEL(5) } },
	{ },
};

static struct pinmux_data_reg pinmux_data_regs[] = {
	{ PINMUX_DATA_REG("INDT0", 0xE6050008, 32) { GP_INDT(0) } },
	{ PINMUX_DATA_REG("INDT1", 0xE6051008, 32) {
		0, 0, GP_1_29_DATA, GP_1_28_DATA,
		GP_1_27_DATA, GP_1_26_DATA, GP_1_25_DATA, GP_1_24_DATA,
		GP_1_23_DATA, GP_1_22_DATA, GP_1_21_DATA, GP_1_20_DATA,
		GP_1_19_DATA, GP_1_18_DATA, GP_1_17_DATA, GP_1_16_DATA,
		GP_1_15_DATA, GP_1_14_DATA, GP_1_13_DATA, GP_1_12_DATA,
		GP_1_11_DATA, GP_1_10_DATA, GP_1_9_DATA, GP_1_8_DATA,
		GP_1_7_DATA, GP_1_6_DATA, GP_1_5_DATA, GP_1_4_DATA,
		GP_1_3_DATA, GP_1_2_DATA, GP_1_1_DATA, GP_1_0_DATA }
	},
	{ PINMUX_DATA_REG("INDT2", 0xE6052008, 32) {
		0, 0, GP_2_29_DATA, GP_2_28_DATA,
		GP_2_27_DATA, GP_2_26_DATA, GP_2_25_DATA, GP_2_24_DATA,
		GP_2_23_DATA, GP_2_22_DATA, GP_2_21_DATA, GP_2_20_DATA,
		GP_2_19_DATA, GP_2_18_DATA, GP_2_17_DATA, GP_2_16_DATA,
		GP_2_15_DATA, GP_2_14_DATA, GP_2_13_DATA, GP_2_12_DATA,
		GP_2_11_DATA, GP_2_10_DATA, GP_2_9_DATA, GP_2_8_DATA,
		GP_2_7_DATA, GP_2_6_DATA, GP_2_5_DATA, GP_2_4_DATA,
		GP_2_3_DATA, GP_2_2_DATA, GP_2_1_DATA, GP_2_0_DATA }
	},
	{ PINMUX_DATA_REG("INDT3", 0xE6053008, 32) { GP_INDT(3) } },
	{ PINMUX_DATA_REG("INDT4", 0xE6054008, 32) { GP_INDT(4) } },
	{ PINMUX_DATA_REG("INDT5", 0xE6055008, 32) { GP_INDT(5) } },
	{ },
};

static struct pinmux_info r8a7790_pinmux_info = {
	.name = "r8a7790_pfc",

	.unlock_reg = 0xe6060000, /* PMMR */

	.reserved_id = PINMUX_RESERVED,
	.data = { PINMUX_DATA_BEGIN, PINMUX_DATA_END },
	.input = { PINMUX_INPUT_BEGIN, PINMUX_INPUT_END },
	.output = { PINMUX_OUTPUT_BEGIN, PINMUX_OUTPUT_END },
	.mark = { PINMUX_MARK_BEGIN, PINMUX_MARK_END },
	.function = { PINMUX_FUNCTION_BEGIN, PINMUX_FUNCTION_END },

	.first_gpio = GPIO_GP_0_0,
	.last_gpio = GPIO_FN_MII_RXD2 /* GPIO_FN_TCLK1_B */,

	.gpios = pinmux_gpios,
	.cfg_regs = pinmux_config_regs,
	.data_regs = pinmux_data_regs,

	.gpio_data = pinmux_data,
	.gpio_data_size = ARRAY_SIZE(pinmux_data),
};

void r8a7790_pinmux_init(void)
{
	register_pinmux(&r8a7790_pinmux_info);
}
