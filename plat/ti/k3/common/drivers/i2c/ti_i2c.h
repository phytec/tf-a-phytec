/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef I2C_H
#define I2C_H

#include <stddef.h>
#include <stdint.h>

/* I2C register offsets */
#define I2C_REVNB_LO			(0x0U)
#define I2C_REVNB_HI			(0x4U)
#define I2C_SYSC			(0x10U)
#define I2C_IRQSTATUS_RAW		(0x24U)
#define I2C_IRQSTATUS			(0x28U)
#define I2C_IRQENABLE_SET		(0x2CU)
#define I2C_IRQENABLE_CLR		(0x30U)
#define I2C_WE				(0x34U)
#define I2C_DMARXENABLE_SET		(0x38U)
#define I2C_DMATXENABLE_SET		(0x3CU)
#define I2C_DMARXENABLE_CLR		(0x40U)
#define I2C_DMATXENABLE_CLR		(0x44U)
#define I2C_DMARXWAKE_EN		(0x48U)
#define I2C_DMATXWAKE_EN		(0x4CU)
#define I2C_SYSS			(0x90U)
#define I2C_BUF				(0x94U)
#define I2C_CNT				(0x98U)
#define I2C_DATA			(0x9CU)
#define I2C_CON				(0xA4U)
#define I2C_OA				(0xA8U)
#define I2C_SA				(0xAcU)
#define I2C_PSC				(0xB0U)
#define I2C_SCLL			(0xB4U)
#define I2C_SCLH			(0xB8U)
#define I2C_SYSTEST			(0xBCU)
#define I2C_BUFSTAT			(0xC0U)
#define I2C_OA1				(0xC4U)
#define I2C_OA2				(0xC8U)
#define I2C_OA3				(0xCCU)
#define I2C_ACTOA			(0xD0U)
#define I2C_SBLOCK			(0xD4U)

/* IRQSTATUS_RAW */
#define I2C_IRQSTATUS_RAW_AL_MASK	(0x00000001U)
#define I2C_IRQSTATUS_RAW_NACK_MASK	(0x00000002U)
#define I2C_IRQSTATUS_RAW_ARDY_MASK	(0x00000004U)
#define I2C_IRQSTATUS_RAW_RRDY_MASK	(0x00000008U)
#define I2C_IRQSTATUS_RAW_XRDY_MASK	(0x00000010U)
#define I2C_IRQSTATUS_RAW_GC_MASK	(0x00000020U)
#define I2C_IRQSTATUS_RAW_STC_MASK	(0x00000040U)
#define I2C_IRQSTATUS_RAW_AERR_MASK	(0x00000080U)
#define I2C_IRQSTATUS_RAW_BF_MASK	(0x00000100U)
#define I2C_IRQSTATUS_RAW_AAS_MASK	(0x00000200U)
#define I2C_IRQSTATUS_RAW_XUDF_MASK	(0x00000400U)
#define I2C_IRQSTATUS_RAW_ROVR_MASK	(0x00000800U)
#define I2C_IRQSTATUS_RAW_BB_MASK	(0x00001000U)
#define I2C_IRQSTATUS_RAW_RDR_MASK	(0x00002000U)
#define I2C_IRQSTATUS_RAW_XDR_MASK	(0x00004000U)

/* I2C_CON register */
#define I2C_CON_STT_MASK		(0x00000001U)
#define I2C_CON_STP_MASK		(0x00000002U)
#define I2C_CON_XOA3_MASK		(0x00000010U)
#define I2C_CON_XOA2_MASK		(0x00000020U)
#define I2C_CON_XOA1_MASK		(0x00000040U)
#define I2C_CON_XOA0_MASK		(0x00000080U)
#define I2C_CON_XSA_MASK		(0x00000100U)
#define I2C_CON_TRX_MASK		(0x00000200U)
#define I2C_CON_MST_MASK		(0x00000400U)
#define I2C_CON_STB_MASK		(0x00000800U)
#define I2C_CON_OPMODE_MASK		(0x00003000U)
#define I2C_CON_I2C_EN_MASK		(0x00008000U)

/* I2C_BUFSTAT register */
#define I2C_BUFSTAT_TXSTAT_MASK		(0x0000003FU)
#define I2C_BUFSTAT_RXSTAT_MASK		(0x00003F00U)

/* I2C_DATA register */
#define I2C_DATA_MASK			(0x000000FFU)

/* I2C_BUF register */
#define I2C_BUF_RXFIFO_CLR_MASK		(0x00004000U)
#define I2C_BUF_RXTRSH_MASK		(0x00003F00U)
#define I2C_BUF_TXFIFO_CLR_MASK		(0x00000040U)
#define I2C_BUF_TXTRSH_MASK		(0x0000003FU)

/* I2C_CNT register */
#define I2C_CNT_MASK			(0x0000FFFFU)

/* I2C_SYSC register */
#define I2C_SYSC_AUTOIDLE_MASK		(0x00000001U)

/* I2C_SYSTEST register */
#define I2C_SYSTEST_FREE_MASK		(0x00004000U)

void ti_i2c_init(int bus_num);
int ti_i2c_write(int bus_num, uint8_t dev, uint8_t *data, size_t len);
int ti_i2c_read(int bus_num, uint8_t dev, uint8_t *data, size_t len);

#endif /* I2C_H */
