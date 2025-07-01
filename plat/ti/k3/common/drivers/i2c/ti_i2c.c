/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/utils_def.h>
#include <lib/mmio.h>
#include <stdint.h>
#include <stdbool.h>
#include <common/debug.h>
#include <drivers/delay_timer.h>

#include "ti_i2c.h"
#include "am62l_wkup_pll.h"

#define I2C_TX_MODE 1
#define I2C_RX_MODE 0

/* Internal functions */

static void i2c_controller_disable(int i2c_base_addr);
static void i2c_controller_enable(int i2c_base_addr);
static void i2c_auto_idle_disable(int i2c_base_addr);
static void i2c_controller_enable_free_run(int i2c_base_addr);
static int32_t i2c_controller_bus_busy(int i2c_base_addr);
static void i2c_fifo_clear(int i2c_base_addr, uint32_t flag);
static void i2c_controller_target_addr_set(int i2c_base_addr, uint8_t dev);
static void i2c_controller_control(int i2c_base_addr, uint32_t cmd);
static void i2c_controller_start(int i2c_base_addr);
static void i2c_controller_stop(int i2c_base_addr);
static uint32_t i2c_controller_int_raw_status_ex(int i2c_base_addr, uint32_t int_flag);
static void i2c_controller_data_put(int i2c_base_addr, uint8_t data);
static uint8_t i2c_controller_data_get(int i2c_base_addr);
static void i2c_controller_int_clear_ex(int i2c_base_addr, uint32_t int_flag);

static inline uint32_t register_read(uintptr_t addr)
{
	return mmio_read_32(addr);
}

void register_write(uintptr_t addr, uint32_t value)
{
	mmio_write_32(addr, value);
	mmio_read_32(addr);
}

void register_update(uintptr_t addr, uint32_t mask, uint32_t value)
{
	uint32_t reg = mmio_read_32(addr);
	reg = (reg & ~mask) | (value & mask);
	mmio_write_32(addr, reg);

	/* Busy wait to ensure the write is complete */
	for (volatile int i = 0; i < 50000; i++);
}

#pragma weak ti_i2c_get_base_addr
int ti_i2c_get_base_addr(int bus_num)
{
	return -1;
}

void ti_i2c_init(int bus_num)
{
	/* List of registers and their values to be written */
	struct registers {
		uintptr_t addr;
		uint32_t value;
	};

	uintptr_t i2c_base_addr = ti_i2c_get_base_addr(bus_num);
	if (i2c_base_addr < 0) {
		ERROR("Invalid I2C base address for bus %d\n", bus_num);
		return;
	}

	i2c_controller_disable(i2c_base_addr);
	i2c_auto_idle_disable(i2c_base_addr);

	struct registers regs_i2c[] = {
		{ i2c_base_addr + I2C_PSC, 0x17 },
		{ i2c_base_addr + I2C_SCLL, 0x0D },
		{ i2c_base_addr + I2C_SCLH, 0x0F },
		{ i2c_base_addr + I2C_BUF , 0x101},
		{ i2c_base_addr + I2C_WE, 0x636f },
		{ i2c_base_addr + I2C_CON, 0x8000 },
		{ i2c_base_addr + I2C_SA, 0x50 }
	};

	for (size_t i = 0; i < ARRAY_SIZE(regs_i2c); i++)
		register_write(regs_i2c[i].addr, regs_i2c[i].value);

	i2c_controller_enable(i2c_base_addr);
	i2c_controller_enable_free_run(i2c_base_addr);
}

int32_t i2c_transfer(int bus_num, uint8_t dev, uint8_t *data, size_t len, bool is_write)
{
	uint32_t regVal = I2C_CON_MST_MASK;
	int32_t count = len;

	uintptr_t i2c_base_addr = ti_i2c_get_base_addr(bus_num);
	if (i2c_base_addr < 0) {
		ERROR("Invalid I2C base address for bus %d\n", bus_num);
		return -1;
	}

	i2c_controller_target_addr_set(i2c_base_addr, dev);

	/* Set I2C_CNT register */
	register_write(i2c_base_addr + I2C_CNT, count & I2C_CNT_MASK);

	if (is_write)
		while (i2c_controller_bus_busy(i2c_base_addr));

	/* Clear TX and RX FIFO */
	i2c_fifo_clear(i2c_base_addr, I2C_TX_MODE);
	i2c_fifo_clear(i2c_base_addr, I2C_RX_MODE);

	/* Set transmit mode */
	if (is_write)
		regVal |= I2C_CON_TRX_MASK;

	i2c_controller_control(i2c_base_addr, regVal);

	/* Generate start */
	i2c_controller_start(i2c_base_addr);

	while(count != 0)
	{
		if (is_write) {
			/* Wait for transmit ready */
			while( i2c_controller_int_raw_status_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_XRDY_MASK) == 0 &&
				i2c_controller_int_raw_status_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_AL_MASK   |
							I2C_IRQSTATUS_RAW_NACK_MASK |
							I2C_IRQSTATUS_RAW_AERR_MASK ) == 0); /* This should time out */

			/* Write byte */
			i2c_controller_data_put(i2c_base_addr, *data); // Write data to I2C_DATA register
		} else {
			/* Wait for read ready */
			while( i2c_controller_int_raw_status_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_RRDY_MASK) == 0 &&
				i2c_controller_int_raw_status_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_AL_MASK   |
							I2C_IRQSTATUS_RAW_NACK_MASK |
							I2C_IRQSTATUS_RAW_AERR_MASK ) == 0); /* This should time out */

			/* Read byte */
			*data = i2c_controller_data_get(i2c_base_addr); // Read data from I2C_DATA register	
		}

		/* Clear the respective interrupt */
		if (is_write)
			i2c_controller_int_clear_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_XRDY_MASK);
		else
			i2c_controller_int_clear_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_RRDY_MASK);

		data++;
		count--;
	}

	/* Wait until ARDY = 1 */
	while (i2c_controller_int_raw_status_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_ARDY_MASK) == 0);

	/* Clear ARDY */
	i2c_controller_int_clear_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_ARDY_MASK);

	/* Generate stop condition */
	if (!is_write) {
		i2c_controller_stop(i2c_base_addr);
		/* Wait for stop condition to happen */
		while (i2c_controller_int_raw_status_ex(i2c_base_addr, I2C_IRQSTATUS_RAW_BB_MASK) != 0);
	}

	return 0;
}

int ti_i2c_write(int bus_num, uint8_t dev, uint8_t *data, size_t len)
{
	int ret;

	ret = i2c_transfer(bus_num, dev, data, len, 1);
	if (ret < 0)
		return ret;

	return 0;
}

int ti_i2c_read(int bus_num, uint8_t dev, uint8_t *data, size_t len)
{
	int ret;

	ret = i2c_transfer(bus_num, dev, data, len, 0);
	if (ret < 0)
		return ret;

	return 0;
}

static void i2c_controller_disable(int i2c_base_addr)
{
	/* Disable the I2C controller */
	register_update(i2c_base_addr + I2C_CON, I2C_CON_I2C_EN_MASK, 0);
}

static void i2c_controller_enable(int i2c_base_addr)
{
	/* Enable the I2C controller */
	register_update(i2c_base_addr + I2C_CON, I2C_CON_I2C_EN_MASK, 1);
}

static void i2c_auto_idle_disable(int i2c_base_addr)
{
	/* Disable auto-idle mode */
	register_update(i2c_base_addr + I2C_SYSC, I2C_SYSC_AUTOIDLE_MASK, 0);
}

static void i2c_controller_enable_free_run(int i2c_base_addr)
{
	/* Enable free run mode */
	register_update(i2c_base_addr + I2C_CON, I2C_SYSTEST_FREE_MASK, 1);
}

static int32_t i2c_controller_bus_busy(int i2c_base_addr)
{
	int32_t status = 0;

	/* Read BB bit */
	if (register_read(i2c_base_addr + I2C_IRQSTATUS_RAW) & I2C_IRQSTATUS_RAW_BB_MASK)
		status = 1;

	return status;
}

static void i2c_fifo_clear(int i2c_base_addr, uint32_t flag)
{
	if (I2C_TX_MODE == flag)
		/* Clear TX FIFO */
		register_update(i2c_base_addr + I2C_BUF, I2C_BUF_TXFIFO_CLR_MASK, 1);
	else
		/* Clear RX FIFO */
		register_update(i2c_base_addr + I2C_BUF, I2C_BUF_RXFIFO_CLR_MASK, 1);
}

static void i2c_controller_target_addr_set(int i2c_base_addr, uint8_t dev)
{
	/* Set the target address */
	register_write(i2c_base_addr + I2C_SA, dev);
}

static void i2c_controller_control(int i2c_base_addr, uint32_t cmd)
{
	/* Set the control flags */
	register_update(i2c_base_addr + I2C_CON, 0xFFFF, cmd | I2C_CON_I2C_EN_MASK);
}

static void i2c_controller_start(int i2c_base_addr)
{
	/* Set the start condition */
	register_update(i2c_base_addr + I2C_CON, I2C_CON_STT_MASK, I2C_CON_STT_MASK);
}

static void i2c_controller_stop(int i2c_base_addr)
{
	/* Set the stop condition */
	register_update(i2c_base_addr + I2C_CON, I2C_CON_STP_MASK, I2C_CON_STP_MASK);
}

static uint32_t i2c_controller_int_raw_status_ex(int i2c_base_addr, uint32_t int_flag)
{
	/* Read the raw interrupt status */
	return register_read(i2c_base_addr + I2C_IRQSTATUS_RAW) & int_flag;
}

static void i2c_controller_data_put(int i2c_base_addr, uint8_t data)
{
	/* Write data to the I2C data register */
	register_write(i2c_base_addr + I2C_DATA, (uint32_t)data);
}

static uint8_t i2c_controller_data_get(int i2c_base_addr)
{
	/* Read data from the I2C data register */
	return (uint8_t)(register_read(i2c_base_addr + I2C_DATA));
}

static void i2c_controller_int_clear_ex(int i2c_base_addr, uint32_t int_flag)
{
	/* Clear the specified interrupt */
	register_write(i2c_base_addr + I2C_IRQSTATUS, int_flag);
}
