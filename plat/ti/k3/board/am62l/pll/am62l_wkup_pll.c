/*
 * Copyright (c) 2025, PHYTEC Messtechnik GmbH
 * Author: Florijan Plohl <florijan.plohl@norik.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/bl_common.h>
#include <lib/mmio.h>

#include "am62l_wkup_pll.h"

int am62l_wkup_pll_init(void)
{
	/* Initialize the WKUP PLL with the required settings */
	struct registers {
		uintptr_t addr;
		uint32_t value;
	};

	/* Define the register addresses and values for WKUP PLL configuration */

	struct registers regs_pll[] = {
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_CTRL, 0x18011 },
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_FREQ_CTRL0, 0x60 },
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_FREQ_CTRL1, 0x0 },
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_DIV_CTRL, 0x1020001 },
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_SS_CTRL, 0x80000000 },
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_HSDIV_CTRL0, 0x8005 },
		{ AM62L_WKUP_PLL_MMR_CFG_PLL0_HSDIV_CTRL1, 0x8018 }
	};

	for (size_t i = 0; i < ARRAY_SIZE(regs_pll); i++)
	{
		mmio_write_32(regs_pll[i].addr, regs_pll[i].value);
		mmio_read_32(regs_pll[i].addr);
	}

	return 0;
}
