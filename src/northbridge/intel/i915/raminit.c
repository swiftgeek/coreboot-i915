/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007-2009 coresystems GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <console/console.h>
#include <cpu/x86/mtrr.h>
#include <cpu/x86/cache.h>
#include <pc80/mc146818rtc.h>
#include <spd.h>
#include <string.h>
#include <arch/romcc_io.h>
#include "raminit.h"
#include "i915.h"
#include <cbmem.h>

struct cbmem_entry *get_cbmem_toc(void)
{
	return (struct cbmem_entry *)(get_top_of_ram() - HIGH_MEMORY_SIZE);
}

/* Debugging macros. */
#if CONFIG_DEBUG_RAM_SETUP
#define PRINTK_DEBUG(x...)	printk(BIOS_DEBUG, x)
#else
#define PRINTK_DEBUG(x...)
#endif

#define RAM_INITIALIZATION_COMPLETE	(1 << 19)

#define RAM_COMMAND_SELF_REFRESH	(0x0 << 16)
#define RAM_COMMAND_NOP			(0x1 << 16)
#define RAM_COMMAND_PRECHARGE		(0x2 << 16)
#define RAM_COMMAND_MRS			(0x3 << 16)
#define RAM_COMMAND_EMRS		(0x4 << 16)
#define RAM_COMMAND_CBR			(0x6 << 16)
#define RAM_COMMAND_NORMAL		(0x7 << 16)

#define RAM_EMRS_1			(0x0 << 21)
#define RAM_EMRS_2			(0x1 << 21)
#define RAM_EMRS_3			(0x2 << 21)

static int get_dimm_spd_address(struct sys_info *sysinfo, int device)
{
	if (sysinfo->spd_addresses)
		return sysinfo->spd_addresses[device];
	else
		return DIMM0 + device;

}

static inline int spd_read_byte(unsigned device, unsigned address)
{
	return smbus_read_byte(device, address);
}

static __attribute__((noinline)) void do_ram_command(u32 command)
{
	u32 reg32;

	reg32 = MCHBAR32(DCC);
	reg32 &= ~( (3<<21) | (1<<20) | (1<<19) | (7 << 16) );
	reg32 |= command;

	PRINTK_DEBUG("   Sending RAM command 0x%08x", reg32);

	MCHBAR32(DCC) = reg32;  /* This is the actual magic */

	/* Also set Init Complete */
	if (command == RAM_COMMAND_NORMAL)
		MCHBAR32(DCC) = reg32 | RAM_INITIALIZATION_COMPLETE;

	PRINTK_DEBUG("...done\n");

#if 0
	udelay(1);
#endif
}


#if CONFIG_DEBUG_RAM_SETUP
void sdram_dump_mchbar_registers(void)
{
	int i;
	printk(BIOS_DEBUG, "Dumping MCHBAR Registers\n");

	for (i=0; i<0xfff; i+=4) {
		if (MCHBAR32(i) == 0)
			continue;
		printk(BIOS_DEBUG, "0x%04x: 0x%08x\n", i, MCHBAR32(i));
	}
}
#endif

static void ram_read32(u32 offset)
{
	PRINTK_DEBUG("   ram read: %08x\n", offset);

	read32(offset);
}

#if 0
static int memclk(void)
{
	switch ((MCHBAR32(CLKCFG) >> 4) & 7) {
	case 1: return 400;
	case 2: return 533;
	case 3: return 667;
	default: printk(BIOS_DEBUG, "memclk: unknown register value %x\n", ((MCHBAR32(CLKCFG) >> 4) & 7) - offset);
	}
	return -1;
}

#endif

static u16 fsbclk(void)
{
	switch (MCHBAR32(CLKCFG) & 7) {
	case 1: return 400;
	case 2: return 533;
	default: printk(BIOS_DEBUG, "fsbclk: unknown register value %x\n", MCHBAR32(CLKCFG) & 7);
	}
	return 0xffff;
}


static int sdram_capabilities_max_supported_memory_frequency(void)
{
#if 0
	u32 reg32;

#if CONFIG_MAXIMUM_SUPPORTED_FREQUENCY
	return CONFIG_MAXIMUM_SUPPORTED_FREQUENCY;
#endif

	reg32 = pci_read_config32(PCI_DEV(0, 0x00, 0), 0xe4); /* CAPID0 + 4 */
	reg32 &= (7 << 0);

	switch (reg32) {
	case 4: return 400;
	case 3: return 533;
	case 2: return 667;
	}
	/* Newer revisions of this chipset rather support faster memory clocks,
	 * so if it's a reserved value, return the fastest memory clock that we
	 * know of and can handle
	 */
	return 667;
#else
	return 533;
#endif
}

/**
 * @brief determine whether chipset is capable of dual channel interleaved mode
 *
 * @return 1 if interleaving is supported, 0 otherwise
 */
static int sdram_capabilities_interleave(void)
{
#if 0
	u32 reg32;

	reg32 = pci_read_config32(PCI_DEV(0, 0x00,0), 0xe4); /* CAPID0 + 4 */
	reg32 >>= 25;
	reg32 &= 1;

	return (!reg32);
#else
	return 1;
#endif
}

/**
 * @brief determine whether chipset is capable of two memory channels
 *
 * @return 1 if dual channel operation is supported, 0 otherwise
 */
static int sdram_capabilities_dual_channel(void)
{
#if 0
	u32 reg32;

	reg32 = pci_read_config32(PCI_DEV(0, 0x00,0), 0xe4); /* CAPID0 + 4 */
	reg32 >>= 24;
	reg32 &= 1;

	return (!reg32);
#else
	return 1;
#endif
}

static int sdram_capabilities_enhanced_addressing_xor(void)
{
#if 0
	u8 reg8;

	reg8 = pci_read_config8(PCI_DEV(0, 0x00, 0), 0xe5); /* CAPID0 + 5 */
	reg8 &= (1 << 7);

	return (!reg8);
#else
	return 0;
#endif
}

static int sdram_capabilities_two_dimms_per_channel(void)
{
#if 0
	u8 reg8;

	reg8 = pci_read_config8(PCI_DEV(0, 0x00, 0), 0xe8); /* CAPID0 + 8 */
	reg8 &= (1 << 0);

	return (reg8 != 0);
#else
	return 1;
#endif
}

// TODO check if we ever need this function
#if 0
static int sdram_capabilities_MEM4G_disable(void)
{
	u8 reg8;

	reg8 = pci_read_config8(PCI_DEV(0, 0x00, 0), 0xe5); /* CAPID0 + 5 */
	reg8 &= (1 << 0);

	return (reg8 != 0);
}
#endif

#define GFX_FREQUENCY_CAP_166MHZ	0x04
#define GFX_FREQUENCY_CAP_200MHZ	0x03
#define GFX_FREQUENCY_CAP_250MHZ	0x02
#define GFX_FREQUENCY_CAP_ALL		0x00

#if 0
static int sdram_capabilities_core_frequencies(void)
{
	u8 reg8;

	reg8 = pci_read_config8(PCI_DEV(0, 0x00, 0), 0xe5); /* CAPID0 + 5 */
	reg8 &= (1 << 3) | (1 << 2) | (1 << 1);
	reg8 >>= 1;

	return (reg8);
}

static void sdram_detect_errors(struct sys_info *sysinfo)
{
	u8 reg8;
	u8 do_reset = 0;

	reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa2);

	if (reg8 & ((1<<7)|(1<<2))) {
		if (reg8 & (1<<2)) {
			printk(BIOS_DEBUG, "SLP S4# Assertion Width Violation.\n");
			/* Write back clears bit 2 */
			pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, reg8);
			do_reset = 1;

		}

		if (reg8 & (1<<7)) {
			printk(BIOS_DEBUG, "DRAM initialization was interrupted.\n");
			reg8 &= ~(1<<7);
			pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, reg8);
			do_reset = 1;
		}

		/* Set SLP_S3# Assertion Stretch Enable */
		reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa4); /* GEN_PMCON_3 */
		reg8 |= (1 << 3);
		pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa4, reg8);

		if (do_reset) {
			printk(BIOS_DEBUG, "Reset required.\n");
			outb(0x00, 0xcf9);
			outb(0x0e, 0xcf9);
			for (;;) asm("hlt"); /* Wait for reset! */
		}
	}

	/* Set DRAM initialization bit in ICH7 */
	reg8 = pci_read_config8(PCI_DEV(0, 0x1f, 0), 0xa2);
	reg8 |= (1<<7);
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, reg8);

	/* clear self refresh status if check is disabled or not a resume */
	if (!CONFIG_CHECK_SLFRCS_ON_RESUME || sysinfo->boot_path != 2) {
		MCHBAR8(0xf14) |= 3;
	} else {
		/* Validate self refresh config */
		if (((sysinfo->dimm[0] != SYSINFO_DIMM_NOT_POPULATED) ||
		     (sysinfo->dimm[1] != SYSINFO_DIMM_NOT_POPULATED)) &&
		    !(MCHBAR8(0xf14) & (1<<0))) {
			do_reset = 1;
		}
		if (((sysinfo->dimm[2] != SYSINFO_DIMM_NOT_POPULATED) ||
		     (sysinfo->dimm[3] != SYSINFO_DIMM_NOT_POPULATED)) &&
		    !(MCHBAR8(0xf14) & (1<<1))) {
			do_reset = 1;
		}
	}

	if (do_reset) {
		printk(BIOS_DEBUG, "Reset required.\n");
		outb(0x00, 0xcf9);
		outb(0x0e, 0xcf9);
		for (;;) asm("hlt"); /* Wait for reset! */
	}
}

#endif

/**
 * @brief Get generic DIMM parameters.
 * @param sysinfo Central memory controller information structure
 *
 * This function gathers several pieces of information for each system DIMM:
 *  o DIMM width (x8 / x16)
 *  o DIMM sides (single sided / dual sided)
 *
 *  Also, some non-supported scenarios are detected.
 */

static void sdram_get_dram_configuration(struct sys_info *sysinfo)
{
	u32 dimm_mask = 0;
	int i;

	/**
	 * i915 supports two DIMMs, in two configurations:
	 *
	 * - single channel with two DIMMs
	 * - dual channel with one DIMM per channel
	 *
	 * In practice dual channel mainboards have their SPD at 0x50/0x52
	 * whereas single channel configurations have their SPD at 0x50/0x51.
	 *
	 * The capability register knows a lot about the channel configuration
	 * but for now we stick with the information we gather via SPD.
	 */

	if (sdram_capabilities_dual_channel()) {
		sysinfo->dual_channel = 1;
		printk(BIOS_DEBUG, "This mainboard supports Dual Channel Operation.\n");
	} else {
		sysinfo->dual_channel = 0;
		printk(BIOS_DEBUG, "This mainboard supports only Single Channel Operation.\n");
	}

	/**
	 * Since we only support two DIMMs in total, there is a limited number
	 * of combinations. This function returns the type of DIMMs.
	 * return value:
	 *   [0:7]  lower DIMM population
	 *   [8-15] higher DIMM population
	 *   [16]   dual channel?
	 *
	 * There are 5 different possible populations for a DIMM socket:
	 * 1. x16 double sided (X16DS)
	 * 2. x8 double sided  (X8DS)
	 * 3. x16 single sided (X16SS)
	 * 4. x8 double stacked (X8DDS)
	 * 5. not populated (NC)
	 *
	 * For the return value we start counting at zero.
	 *
	 */

	for (i=0; i<(2 * DIMM_SOCKETS); i++) {
		int device = get_dimm_spd_address(sysinfo, i);
		u8 reg8;

		/* Initialize the socket information with a sane value */
		sysinfo->dimm[i] = SYSINFO_DIMM_NOT_POPULATED;

		/* Dual Channel not supported, but Channel 1? Bail out */
		if (!sdram_capabilities_dual_channel() && (i >> 1))
			continue;

		/* Two DIMMs per channel not supported, but odd DIMM number? */
		if (!sdram_capabilities_two_dimms_per_channel() && (i& 1))
			continue;

		printk(BIOS_DEBUG, "DDR II Channel %d Socket %d: ", (i >> 1), (i & 1));

		if (spd_read_byte(device, SPD_MEMORY_TYPE) != SPD_MEMORY_TYPE_SDRAM_DDR2) {
			printk(BIOS_DEBUG, "N/A\n");
			continue;
		}

		reg8 = spd_read_byte(device, SPD_DIMM_CONFIG_TYPE);
		if (reg8 == ERROR_SCHEME_ECC)
			die("Error: ECC memory not supported by this chipset\n");

		reg8 = spd_read_byte(device, SPD_MODULE_ATTRIBUTES);
		if (reg8 & MODULE_BUFFERED)
			die("Error: Buffered memory not supported by this chipset\n");
		if (reg8 & MODULE_REGISTERED)
			die("Error: Registered memory not supported by this chipset\n");

		switch (spd_read_byte(device, SPD_PRIMARY_SDRAM_WIDTH)) {
		case 0x08:
			switch (spd_read_byte(device, SPD_NUM_DIMM_BANKS) & 0x0f) {
			case 1:
				printk(BIOS_DEBUG, "x8DDS\n");
				sysinfo->dimm[i] = SYSINFO_DIMM_X8DDS;
				break;
			case 0:
				printk(BIOS_DEBUG, "x8DS\n");
				sysinfo->dimm[i] = SYSINFO_DIMM_X8DS;
				break;
			default:
				printk(BIOS_DEBUG, "Unsupported.\n");
			}
			break;
		case 0x10:
			switch (spd_read_byte(device, SPD_NUM_DIMM_BANKS) & 0x0f) {
			case 1:
				printk(BIOS_DEBUG, "x16DS\n");
				sysinfo->dimm[i] = SYSINFO_DIMM_X16DS;
				break;
			case 0:
				printk(BIOS_DEBUG, "x16SS\n");
				sysinfo->dimm[i] = SYSINFO_DIMM_X16SS;
				break;
			default:
				printk(BIOS_DEBUG, "Unsupported.\n");
			}
			break;
		default:
			die("Unsupported DDR-II memory width.\n");
		}

		dimm_mask |= (1 << i);
	}

	if (!dimm_mask) {
		die("No memory installed.\n");
	}

	if (!(dimm_mask & ((1 << DIMM_SOCKETS) - 1))) {
		printk(BIOS_INFO, "Channel 0 has no memory populated.\n");
	}
}

/**
 * @brief determine if any DIMMs are stacked
 *
 * @param sysinfo central sysinfo data structure.
 */
static void sdram_verify_package_type(struct sys_info * sysinfo)
{
	int i;

	/* Assume no stacked DIMMs are available until we find one */
	sysinfo->package = 0;
	for (i=0; i<2*DIMM_SOCKETS; i++) {
		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		/* Is the current DIMM a stacked DIMM? */
		if (spd_read_byte(get_dimm_spd_address(sysinfo, i), SPD_NUM_DIMM_BANKS) & (1 << 4))
			sysinfo->package = 1;
	}
}

static u8 sdram_possible_cas_latencies(struct sys_info * sysinfo)
{
	int i;
	u8 cas_mask;

	/* Setup CAS mask with all supported CAS Latencies */
	cas_mask = SPD_CAS_LATENCY_DDR2_3 |
		   SPD_CAS_LATENCY_DDR2_4 |
		   SPD_CAS_LATENCY_DDR2_5;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		if (sysinfo->dimm[i] != SYSINFO_DIMM_NOT_POPULATED)
			cas_mask &= spd_read_byte(get_dimm_spd_address(sysinfo, i),
						  SPD_ACCEPTABLE_CAS_LATENCIES);
	}

	if(!cas_mask) {
		die("No DDR-II modules with accepted CAS latencies found.\n");
	}

	return cas_mask;
}

static void sdram_detect_cas_latency_and_ram_speed(struct sys_info * sysinfo, u8 cas_mask)
{
	int i, j, idx;
	int lowest_common_cas = 0;
	int max_ram_speed = 0;

	const u8 ddr2_speeds_table[] = {
		0x50, 0x60,	/* DDR2 400: tCLK = 5.0ns  tAC = 0.6ns  */
		0x3d, 0x50,	/* DDR2 533: tCLK = 3.75ns tAC = 0.5ns  */
		0x30, 0x45,	/* DDR2 667: tCLK = 3.0ns  tAC = 0.45ns */
	};

	const u8 spd_lookup_table[] = {
		SPD_MIN_CYCLE_TIME_AT_CAS_MAX,	SPD_ACCESS_TIME_FROM_CLOCK,
		SPD_SDRAM_CYCLE_TIME_2ND,	SPD_ACCESS_TIME_FROM_CLOCK_2ND,
		SPD_SDRAM_CYCLE_TIME_3RD,	SPD_ACCESS_TIME_FROM_CLOCK_3RD
	};

	switch (sdram_capabilities_max_supported_memory_frequency()) {
	case 400: max_ram_speed = 0; break;
	case 533: max_ram_speed = 1; break;
	case 667: max_ram_speed = 2; break;
	}

	if (fsbclk() == 533)
		max_ram_speed = 1;

	sysinfo->memory_frequency = 0;
	sysinfo->cas = 0;

	if (cas_mask & SPD_CAS_LATENCY_DDR2_3) {
		lowest_common_cas = 3;
	} else if (cas_mask & SPD_CAS_LATENCY_DDR2_4) {
		lowest_common_cas = 4;
	} else if (cas_mask & SPD_CAS_LATENCY_DDR2_5) {
		lowest_common_cas = 5;
	}
	PRINTK_DEBUG("lowest common cas = %d\n", lowest_common_cas);

	for (j = max_ram_speed; j>=0; j--) {
		int freq_cas_mask = cas_mask;

		PRINTK_DEBUG("Probing Speed %d\n", j);
		for (i=0; i<2*DIMM_SOCKETS; i++) {
			int device = get_dimm_spd_address(sysinfo, i);
			int current_cas_mask;

			PRINTK_DEBUG("  DIMM: %d\n", i);
			if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED) {
				continue;
			}

			current_cas_mask = spd_read_byte(device, SPD_ACCEPTABLE_CAS_LATENCIES);

			while (current_cas_mask) {
				int highest_supported_cas = 0, current_cas = 0;
				PRINTK_DEBUG("    Current CAS mask: %04x; ", current_cas_mask);
				if (current_cas_mask & SPD_CAS_LATENCY_DDR2_5) {
					highest_supported_cas = 5;
				} else if (current_cas_mask & SPD_CAS_LATENCY_DDR2_4) {
					highest_supported_cas = 4;
				} else if (current_cas_mask & SPD_CAS_LATENCY_DDR2_3) {
					highest_supported_cas = 3;
				} else {
					die("Invalid max. CAS.\n");
				}
				if (current_cas_mask & SPD_CAS_LATENCY_DDR2_3) {
					current_cas = 3;
				} else if (current_cas_mask & SPD_CAS_LATENCY_DDR2_4) {
					current_cas = 4;
				} else if (current_cas_mask & SPD_CAS_LATENCY_DDR2_5) {
					current_cas = 5;
				} else {
					die("Invalid CAS.\n");
				}

				idx = highest_supported_cas - current_cas;
				PRINTK_DEBUG("idx=%d, ", idx);
				PRINTK_DEBUG("tCLK=%x, ", spd_read_byte(device, spd_lookup_table[2*idx]));
				PRINTK_DEBUG("tAC=%x", spd_read_byte(device, spd_lookup_table[(2*idx)+1]));

				if (spd_read_byte(device, spd_lookup_table[2*idx]) <= ddr2_speeds_table[2*j] &&
						spd_read_byte(device, spd_lookup_table[(2*idx)+1]) <= ddr2_speeds_table[(2*j)+1]) {
					PRINTK_DEBUG(":    OK\n");
					break;
				}

				PRINTK_DEBUG(":    Not fast enough!\n");

				current_cas_mask &= ~(1 << (current_cas));
			}

			freq_cas_mask &= current_cas_mask;
			if (!current_cas_mask) {
				PRINTK_DEBUG("    No valid CAS for this speed on DIMM %d\n", i);
				break;
			}
		}
		PRINTK_DEBUG("  freq_cas_mask for speed %d: %04x\n", j, freq_cas_mask);
		if (freq_cas_mask) {
			switch (j) {
			case 0: sysinfo->memory_frequency = 400; break;
			case 1: sysinfo->memory_frequency = 533; break;
			case 2: sysinfo->memory_frequency = 667; break;
			}
			if (freq_cas_mask & SPD_CAS_LATENCY_DDR2_3) {
				sysinfo->cas = 3;
			} else if (freq_cas_mask & SPD_CAS_LATENCY_DDR2_4) {
				sysinfo->cas = 4;
			} else if (freq_cas_mask & SPD_CAS_LATENCY_DDR2_5) {
				sysinfo->cas = 5;
			}
			break;
		}
	}

	if (sysinfo->memory_frequency && sysinfo->cas) {
		printk(BIOS_DEBUG, "Memory will be driven at %dMHz with CAS=%d clocks\n",
				sysinfo->memory_frequency, sysinfo->cas);
	} else {
		die("Could not find common memory frequency and CAS\n");
	}
}

static void sdram_detect_smallest_tRAS(struct sys_info * sysinfo)
{
	int i;
	int tRAS_time;
	int tRAS_cycles;
	int freq_multiplier = 0;

	switch (sysinfo->memory_frequency) {
	case 400: freq_multiplier = 0x14; break; /* 5ns */
	case 533: freq_multiplier = 0x0f; break; /* 3.75ns */
	case 667: freq_multiplier = 0x0c; break; /* 3ns */
	}

	tRAS_cycles = 4; /* 4 clocks minimum */
	tRAS_time = tRAS_cycles * freq_multiplier;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		u8 reg8;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		reg8 = spd_read_byte(get_dimm_spd_address(sysinfo, i), SPD_MIN_ACTIVE_TO_PRECHARGE_DELAY);
		if (!reg8) {
			die("Invalid tRAS value.\n");
		}

		while ((tRAS_time >> 2) < reg8) {
			tRAS_time += freq_multiplier;
			tRAS_cycles++;
		}
	}
	if(tRAS_cycles > 0x18) {
		die("DDR-II Module does not support this frequency (tRAS error)\n");
	}

	printk(BIOS_DEBUG, "tRAS = %d cycles\n", tRAS_cycles);
	sysinfo->tras = tRAS_cycles;
}

static void sdram_detect_smallest_tRP(struct sys_info * sysinfo)
{
	int i;
	int tRP_time;
	int tRP_cycles;
	int freq_multiplier = 0;

	switch (sysinfo->memory_frequency) {
	case 400: freq_multiplier = 0x14; break; /* 5ns */
	case 533: freq_multiplier = 0x0f; break; /* 3.75ns */
	case 667: freq_multiplier = 0x0c; break; /* 3ns */
	}

	tRP_cycles = 2; /* 2 clocks minimum */
	tRP_time = tRP_cycles * freq_multiplier;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		u8 reg8;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		reg8 = spd_read_byte(get_dimm_spd_address(sysinfo, i), SPD_MIN_ROW_PRECHARGE_TIME);
		if (!reg8) {
			die("Invalid tRP value.\n");
		}

		while (tRP_time < reg8) {
			tRP_time += freq_multiplier;
			tRP_cycles++;
		}
	}

	if(tRP_cycles > 6) {
		die("DDR-II Module does not support this frequency (tRP error)\n");
	}

	printk(BIOS_DEBUG, "tRP = %d cycles\n", tRP_cycles);
	sysinfo->trp = tRP_cycles;
}

static void sdram_detect_smallest_tRCD(struct sys_info * sysinfo)
{
	int i;
	int tRCD_time;
	int tRCD_cycles;
	int freq_multiplier = 0;

	switch (sysinfo->memory_frequency) {
	case 400: freq_multiplier = 0x14; break; /* 5ns */
	case 533: freq_multiplier = 0x0f; break; /* 3.75ns */
	case 667: freq_multiplier = 0x0c; break; /* 3ns */
	}

	tRCD_cycles = 2; /* 2 clocks minimum */
	tRCD_time = tRCD_cycles * freq_multiplier;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		u8 reg8;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		reg8 = spd_read_byte(get_dimm_spd_address(sysinfo, i), SPD_MIN_RAS_TO_CAS_DELAY);
		if (!reg8) {
			die("Invalid tRCD value.\n");
		}

		while (tRCD_time < reg8) {
			tRCD_time += freq_multiplier;
			tRCD_cycles++;
		}
	}
	if(tRCD_cycles > 6) {
		die("DDR-II Module does not support this frequency (tRCD error)\n");
	}

	printk(BIOS_DEBUG, "tRCD = %d cycles\n", tRCD_cycles);
	sysinfo->trcd = tRCD_cycles;
}

static void sdram_detect_smallest_tWR(struct sys_info * sysinfo)
{
	int i;
	int tWR_time;
	int tWR_cycles;
	int freq_multiplier = 0;

	switch (sysinfo->memory_frequency) {
	case 400: freq_multiplier = 0x14; break; /* 5ns */
	case 533: freq_multiplier = 0x0f; break; /* 3.75ns */
	case 667: freq_multiplier = 0x0c; break; /* 3ns */
	}

	tWR_cycles = 2; /* 2 clocks minimum */
	tWR_time = tWR_cycles * freq_multiplier;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		u8 reg8;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		reg8 = spd_read_byte(get_dimm_spd_address(sysinfo, i), SPD_WRITE_RECOVERY_TIME);
		if (!reg8) {
			die("Invalid tWR value.\n");
		}

		while (tWR_time < reg8) {
			tWR_time += freq_multiplier;
			tWR_cycles++;
		}
	}
	if(tWR_cycles > 5) {
		die("DDR-II Module does not support this frequency (tWR error)\n");
	}

	printk(BIOS_DEBUG, "tWR = %d cycles\n", tWR_cycles);
	sysinfo->twr = tWR_cycles;
}

static void sdram_detect_smallest_tRFC(struct sys_info * sysinfo)
{
	int i, index = 0;

	const u8 tRFC_cycles[] = {
	     /* 75 105 127.5 */
		15, 21, 26,	/* DDR2-400 */
		20, 28, 34,	/* DDR2-533 */
		25, 35, 43 	/* DDR2-667 */
	};

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		u8 reg8;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		reg8 = sysinfo->banksize[i*2];
		switch (reg8) {
		case 0x04: reg8 = 0; break;
		case 0x08: reg8 = 1; break;
		case 0x10: reg8 = 2; break;
		case 0x20: reg8 = 3; break;
		}

		if (sysinfo->dimm[i] == SYSINFO_DIMM_X16DS || sysinfo->dimm[i] == SYSINFO_DIMM_X16SS)
			reg8++;

		if (reg8 > 3) {
			/* Can this happen? Go back to 127.5ns just to be sure
			 * we don't run out of the array. This may be wrong
			 */
			printk(BIOS_DEBUG, "DIMM %d is 1Gb x16.. Please report.\n", i);
			reg8 = 3;
		}

		if (reg8 > index)
			index = reg8;

	}
	index--;
	switch (sysinfo->memory_frequency) {
	case 667: index += 3; /* Fallthrough */
	case 533: index += 3; /* Fallthrough */
	case 400: break;
	}

	sysinfo->trfc = tRFC_cycles[index];
	printk(BIOS_DEBUG, "tRFC = %d cycles\n", tRFC_cycles[index]);
}

static void sdram_detect_smallest_refresh(struct sys_info * sysinfo)
{
	int i;

	sysinfo->refresh = 0;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		int refresh;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		refresh = spd_read_byte(get_dimm_spd_address(sysinfo, i),
					SPD_REFRESH) & ~(1 << 7);

		/* 15.6us */
		if (!refresh)
			continue;

		/* Refresh is slower than 15.6us, use 15.6us */
		if (refresh > 2)
			continue;

		if (refresh == 2) {
			sysinfo->refresh = 1;
			break;
		}

		die("DDR-II module has unsupported refresh value\n");
	}
	printk(BIOS_DEBUG, "Refresh: %s\n", sysinfo->refresh?"7.8us":"15.6us");
}

static void sdram_verify_burst_length(struct sys_info * sysinfo)
{
	int i;

	for (i=0; i<2*DIMM_SOCKETS; i++) {
		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		if (!(spd_read_byte(get_dimm_spd_address(sysinfo, i),
				    SPD_SUPPORTED_BURST_LENGTHS) & SPD_BURST_LENGTH_8))
			die("Only DDR-II RAM with burst length 8 is supported by this chipset.\n");
	}
}

static void sdram_enable_system_memory_io(struct sys_info *sysinfo)
{
	u32 reg32;

	printk(BIOS_DEBUG, "Enabling System Memory IO... \n");

	reg32 = MCHBAR32(DRTST);
	reg32 |= 0xf0;
	MCHBAR32(DRTST) = reg32;

	/* Activate DRAM Channel IO Buffers */
	if (sysinfo->dimm[0] != SYSINFO_DIMM_NOT_POPULATED ||
			sysinfo->dimm[1] != SYSINFO_DIMM_NOT_POPULATED) {
		reg32 = MCHBAR32(C0DRC1);
		reg32 |= (1 << 8);
		MCHBAR32(C0DRC1) = reg32;
	}
	if (sysinfo->dimm[2] != SYSINFO_DIMM_NOT_POPULATED ||
			sysinfo->dimm[3] != SYSINFO_DIMM_NOT_POPULATED) {
		reg32 = MCHBAR32(C1DRC1);
		reg32 |= (1 << 8);
		MCHBAR32(C1DRC1) = reg32;
	}
}


struct dimm_size {
	unsigned long side1;
	unsigned long side2;
};

static struct dimm_size sdram_get_dimm_size(struct sys_info *sysinfo, u16 dimmno)
{
	/* Calculate the log base 2 size of a DIMM in bits */
	struct dimm_size sz;
	int value, low, rows, columns, device;

	device = get_dimm_spd_address(sysinfo, dimmno);
	sz.side1 = 0;
	sz.side2 = 0;

	rows = spd_read_byte(device, SPD_NUM_ROWS);	/* rows */
	if (rows < 0) goto hw_err;
	if ((rows & 0xf) == 0) goto val_err;
	sz.side1 += rows & 0xf;

	columns = spd_read_byte(device, SPD_NUM_COLUMNS);	/* columns */
	if (columns < 0) goto hw_err;
	if ((columns & 0xf) == 0) goto val_err;
	sz.side1 += columns & 0xf;

	value = spd_read_byte(device, SPD_NUM_BANKS_PER_SDRAM);	/* banks */
	if (value < 0) goto hw_err;
	if ((value & 0xff) == 0) goto val_err;
	sz.side1 += log2(value & 0xff);

	/* Get the module data width and convert it to a power of two */
	value = spd_read_byte(device, SPD_MODULE_DATA_WIDTH_MSB);	/* (high byte) */
	if (value < 0) goto hw_err;
	value &= 0xff;
	value <<= 8;

	low = spd_read_byte(device, SPD_MODULE_DATA_WIDTH_LSB);	/* (low byte) */
	if (low < 0) goto hw_err;
	value = value | (low & 0xff);
	if ((value != 72) && (value != 64)) goto val_err;
	sz.side1 += log2(value);

	/* side 2 */
	value = spd_read_byte(device, SPD_NUM_DIMM_BANKS);	/* number of physical banks */

	if (value < 0) goto hw_err;
	value &= 7;
	value++;
	if (value == 1) goto out;
	if (value != 2) goto val_err;

	/* Start with the symmetrical case */
	sz.side2 = sz.side1;

	if ((rows & 0xf0) == 0) goto out;	/* If symmetrical we are done */

	/* Don't die here, I have not come across any of these to test what
	 * actually happens.
	 */
	printk(BIOS_ERR, "Assymetric DIMMs are not supported by this chipset\n");

	sz.side2 -= (rows & 0x0f);		/* Subtract out rows on side 1 */
	sz.side2 += ((rows >> 4) & 0x0f);	/* Add in rows on side 2 */

	sz.side2 -= (columns & 0x0f);		/* Subtract out columns on side 1 */
	sz.side2 += ((columns >> 4) & 0x0f);	/* Add in columns on side 2 */

	goto out;

 val_err:
	die("Bad SPD value\n");
 hw_err:
	/* If a hardware error occurs the spd rom probably does not exist.
	 * In this case report that there is no memory
	 */
	sz.side1 = 0;
	sz.side2 = 0;
out:
	return sz;
}

static void sdram_detect_dimm_size(struct sys_info * sysinfo)
{
	int i;

	for(i = 0; i < 2 * DIMM_SOCKETS; i++) {
		struct dimm_size sz;

		sysinfo->banksize[i * 2] = 0;
		sysinfo->banksize[(i * 2) + 1] = 0;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		sz = sdram_get_dimm_size(sysinfo, i);

		sysinfo->banks[i] = spd_read_byte(get_dimm_spd_address(sysinfo, i),
						  SPD_NUM_BANKS_PER_SDRAM);	/* banks */

		if (sz.side1 < 30)
			die("DDR-II rank size smaller than 128MB is not supported.\n");

		sysinfo->banksize[i * 2] = 1 << (sz.side1 - 28);

		printk(BIOS_DEBUG, "DIMM %d side 0 = %d MB\n", i, sysinfo->banksize[i * 2] * 32 );

		if (!sz.side2)
			continue;

		/* If there is a second side, it has to have at least 128M, too */
		if (sz.side2 < 30)
			die("DDR-II rank size smaller than 128MB is not supported.\n");

		sysinfo->banksize[(i * 2) + 1] = 1 << (sz.side2 - 28);

		printk(BIOS_DEBUG, "DIMM %d side 1 = %d MB\n", i, sysinfo->banksize[(i * 2) + 1] * 32);
	}
}

static int sdram_program_row_boundaries(struct sys_info *sysinfo)
{
	int i;
	int cum0, cum1, tolud, tom;

	printk(BIOS_DEBUG, "Setting RAM size... \n");

	cum0 = 0;
	for(i = 0; i < 2 * DIMM_SOCKETS; i++) {
		cum0 += sysinfo->banksize[i];
		MCHBAR8(C0DRB0+i) = cum0;
	}

	/* Assume we continue in Channel 1 where we stopped in Channel 0 */
	cum1 = cum0;

	/* Exception: Interleaved starts from the beginning */
	if (sysinfo->interleaved)
		cum1 = 0;

#if 0
	/* Exception: Channel 1 is not populated. C1DRB stays zero */
	if (sysinfo->dimm[2] == SYSINFO_DIMM_NOT_POPULATED &&
			sysinfo->dimm[3] == SYSINFO_DIMM_NOT_POPULATED)
		cum1 = 0;
#endif

	for(i = 0; i < 2 * DIMM_SOCKETS; i++) {
		cum1 += sysinfo->banksize[i + 4];
		MCHBAR8(C1DRB0+i) = cum1;
	}

	/* Set TOLUD Top Of Low Usable DRAM */
	if (sysinfo->interleaved)
		tolud = (cum0 + cum1) << 1;
	else
		tolud = (cum1 ? cum1 : cum0)  << 1;

	/* The TOM register has a different format */
	tom = tolud >> 3;

	/* Limit the value of TOLUD to leave some space for PCI memory. */
	if (tolud > 0xd0)
		tolud = 0xd0;	/* 3.25GB : 0.75GB */

	pci_write_config8(PCI_DEV(0,0,0), TOLUD, tolud);

	printk(BIOS_DEBUG, "C0DRB = 0x%08x\n", MCHBAR32(C0DRB0));
	printk(BIOS_DEBUG, "C1DRB = 0x%08x\n", MCHBAR32(C1DRB0));
	printk(BIOS_DEBUG, "TOLUD = 0x%04x\n", pci_read_config8(PCI_DEV(0,0,0), TOLUD));

	//pci_write_config16(PCI_DEV(0,0,0), TOM, tom);

	return 0;
}

static int sdram_set_row_attributes(struct sys_info *sysinfo)
{
	int i, value;
	u16 dra0=0, dra1=0, dra = 0;

	printk(BIOS_DEBUG, "Setting row attributes... \n");
	for(i=0; i < 2 * DIMM_SOCKETS; i++) {
		u16 device;
		u8 columnsrows;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED) {
			continue;
		}

		device = get_dimm_spd_address(sysinfo, i);

		value = spd_read_byte(device, SPD_NUM_ROWS);	/* rows */
		columnsrows = (value & 0x0f);

		value = spd_read_byte(device, SPD_NUM_COLUMNS);	/* columns */
		columnsrows |= (value & 0xf) << 4;

		switch (columnsrows) {
		case 0x9d: dra = 2; break;
		case 0xad: dra = 3; break;
		case 0xbd: dra = 4; break;
		case 0xae: dra = 3; break;
		case 0xbe: dra = 4; break;
		default: die("Unsupported Rows/Columns. (DRA)");
		}

		/* Double Sided DIMMs? */
		if (sysinfo->banksize[(2 * i) + 1] != 0) {
			dra = (dra << 4) | dra;
		}

		if (i < DIMM_SOCKETS)
			dra0 |= (dra << (i*8));
		else
			dra1 |= (dra << ((i - DIMM_SOCKETS)*8));
	}

	MCHBAR16(C0DRA0) = dra0;
	MCHBAR16(C1DRA0) = dra1;

	printk(BIOS_DEBUG, "C0DRA = 0x%04x\n", dra0);
	printk(BIOS_DEBUG, "C1DRA = 0x%04x\n", dra1);

	return 0;
}

static void sdram_set_bank_architecture(struct sys_info *sysinfo)
{
	u32 off32;
	int i;

	MCHBAR16(C1BNKARC) &= 0xff00;
	MCHBAR16(C0BNKARC) &= 0xff00;

	off32 = C0BNKARC;
	for (i=0; i < 2 * DIMM_SOCKETS; i++) {
		/* Switch to second channel */
		if (i == DIMM_SOCKETS)
			off32 = C1BNKARC;

		if (sysinfo->dimm[i] == SYSINFO_DIMM_NOT_POPULATED)
			continue;

		if (sysinfo->banks[i] != 8)
			continue;

		printk(BIOS_SPEW, "DIMM%d has 8 banks.\n", i);

		if (i & 1)
			MCHBAR16(off32) |= 0x50;
		else
			MCHBAR16(off32) |= 0x05;
	}
}


#define REFRESH_7_8US	1
#define REFRESH_15_6US	0
static void sdram_program_refresh_rate(struct sys_info *sysinfo)
{
	u32 reg32;

	if (sysinfo->refresh == REFRESH_7_8US) {
		reg32 = (2 << 8); /* Refresh enabled at 7.8us */
	} else {
		reg32 = (1 << 8); /* Refresh enabled at 15.6us */
	}

	MCHBAR32(C0DRC0) &= ~(7 << 8);
	MCHBAR32(C0DRC0) |= reg32;

	MCHBAR32(C1DRC0) &= ~(7 << 8);
	MCHBAR32(C1DRC0) |= reg32;
}

static void sdram_program_cke_tristate(struct sys_info *sysinfo)
{
	u32 reg32;
	int i;

	reg32 = MCHBAR32(C0DRC1);

	for (i=0; i < 4; i++) {
		if (sysinfo->banksize[i] == 0) {
			reg32 |= (1 << (16 + i));
		}
	}

	reg32 |= (1 << 12);

	reg32 |= (1 << 11);
	MCHBAR32(C0DRC1) = reg32;

	/* Do we have to do this if we're in Single Channel Mode?  */
	reg32 = MCHBAR32(C1DRC1);

	for (i=4; i < 8; i++) {
		if (sysinfo->banksize[i] == 0) {
			reg32 |= (1 << (12 + i));
		}
	}

	reg32 |= (1 << 12);

	reg32 |= (1 << 11);
	MCHBAR32(C1DRC1) = reg32;
}

static void sdram_program_odt_tristate(struct sys_info *sysinfo)
{
	u32 reg32;

	reg32 = MCHBAR32(C0DRC2);
	reg32 |= 0x1f0;
	MCHBAR32(C0DRC2) = reg32;

	reg32 = MCHBAR32(C1DRC2);
	reg32 |= 0x1f0;
	MCHBAR32(C1DRC2) = reg32;
}

static void sdram_set_timing_and_control(struct sys_info *sysinfo)
{
	const uint8_t is_ddr2 = 1;

	u32 reg32, off32;
	u32 tWTR;
	u32 temp_drt;
	int i, page_size;

	static const u8 const drt0_table[] = {
	  /* CL 3, 4, 5 */
		3, 4, 5, 	/* FSB533/400, DDR533/400 */
		4, 5, 6,	/* FSB667, DDR533/400 */
		4, 5, 6,	/* FSB667, DDR667 */
	};

	static const u8 const cas_table[] = {
		2, 1, 0, 3
	};

	reg32 = MCHBAR32(C0DRC0);
	reg32 |= (1 << 2);	/* Burst Length 8 */
	MCHBAR32(C0DRC0) = reg32;

	reg32 = MCHBAR32(C1DRC0);
	reg32 |= (1 << 2);	/* Burst Length 8 */
	MCHBAR32(C1DRC0) = reg32;

	if (!sysinfo->dual_channel && sysinfo->dimm[1] !=
			SYSINFO_DIMM_NOT_POPULATED) {
		reg32 = MCHBAR32(C0DRC0);
		reg32 |= (1 << 15);
		MCHBAR32(C0DRC0) = reg32;
	}

	/* Calculate DRT0 */
	uint32_t drt0_mask = 0x20600;

	/* B2B Write Precharge (same bank) = CL-1 + BL/2 + tWR */
	reg32 = (sysinfo->cas - 1) + (BURSTLENGTH / 2) + sysinfo->twr;
	temp_drt = (reg32 << 28);

	/* Write Auto Precharge (same bank) = CL-1 + BL/2 + tWR + tRP */
#if 0
	reg32 += sysinfo->trp;
#else
	reg32 += 3;
#endif
	temp_drt |= (reg32 << 4);

	if (sysinfo->memory_frequency == 667) {
		tWTR = 3; /* 667MHz */
	} else {
		tWTR = 2; /* 400 and 533 */
	}

	/* B2B Write to Read Command Spacing */
	reg32 = (sysinfo->cas - 1) + (BURSTLENGTH / 2) + tWTR;
	temp_drt |= (reg32 << 24);

	/* CxDRT0 [23:22], [21:20], [19:18] [16] have fixed values */
	temp_drt |= ( (1 << 22) | (1 << 20) | (0 << 18) | (0 << 16) );

	/* Program Write Auto Precharge to Activate */
	off32 = 0;
	if (sysinfo->fsb_frequency == 667) { /* 667MHz FSB */
		off32 += 3;
	}
	if (sysinfo->memory_frequency == 667) {
		off32 += 3;
	}
	off32 += sysinfo->cas - 3;
	reg32 = drt0_table[off32];
#if 0
	temp_drt |= (reg32 << 11);
#else
	temp_drt |= (5 << 11);
#endif

	/* Read Auto Precharge to Activate */

	temp_drt |= (8 << 0);

	MCHBAR32(C0DRT0) &= drt0_mask;
	MCHBAR32(C0DRT0) |= temp_drt;

	MCHBAR32(C1DRT0) &= drt0_mask;
	MCHBAR32(C1DRT0) |= temp_drt;

	if (1) {	// FIXME
		MCHBAR32(C0DTD) |= 0x02;
		MCHBAR32(C1DTD) |= 0x02;
	}

	/* Calculate DRT1 */
	uint32_t drt1_mask = 0xcf030088;
	temp_drt = 0;

	/* DRAM RASB Precharge */
	temp_drt |= (sysinfo->trp - 2) << 0;

	/* DRAM RASB to CASB Delay */
	temp_drt |= (sysinfo->trcd - 2) << 4;

	/* CASB Latency */
	temp_drt |= (cas_table[sysinfo->cas - 3]) << 8;

	/* Refresh Cycle Time */
#if 0
	temp_drt |= (sysinfo->trfc) << 11;
#else
	temp_drt |= 0x1e << 11;
#endif

	/* Pre-All to Activate Delay */
	if (0)	/* FIXME if 8 bank devices */
		temp_drt |= (0 << 16);
	else
		temp_drt |= (1 << 16);  

	/* Precharge to Precharge Delay stays at 1 clock */
	temp_drt |= (0 << 17);

	/* Activate to Precharge Delay */
	temp_drt |= (sysinfo->tras << 20);

#if 0
	/* Read to Precharge (tRTP) */
	if (sysinfo->memory_frequency == 667) {
		temp_drt |= (1 << 28);
	} else {
		temp_drt |= (0 << 28);
	}
#endif
#if 0
	/* Determine page size */
	reg32 = 0;
	page_size = 1; /* Default: 1k pagesize */
	for (i=0; i< 2*DIMM_SOCKETS; i++) {
		if (sysinfo->dimm[i] == SYSINFO_DIMM_X16DS ||
				sysinfo->dimm[i] == SYSINFO_DIMM_X16SS)
			page_size = 2; /* 2k pagesize */
	}

	if (sysinfo->memory_frequency == 533 && page_size == 2) {
		reg32 = 1;
	}
	if (sysinfo->memory_frequency == 667) {
		reg32 = page_size;
	}
	temp_drt |= (reg32 << 30);
#endif

	/* Program DRT1 */
	MCHBAR32(C0DRT1) &= drt1_mask;
	MCHBAR32(C0DRT1) |= temp_drt;
	MCHBAR32(C1DRT1) &= drt1_mask;
	MCHBAR32(C1DRT1) |= temp_drt;
	
	/* Program DRT2 */
	uint32_t drt2_mask = ~((3<<30) | (3<<8)); /* ~0xc0000300 */;
	if (is_ddr2)
		temp_drt = (1<<31) | (2<<8);
	else
		temp_drt = (0<<31) | (3<<8);

	MCHBAR32(C0DRT2) &= drt2_mask;
	MCHBAR32(C1DRT2) &= drt2_mask;
	MCHBAR32(C0DRT2) |= temp_drt;
	MCHBAR32(C1DRT2) |= temp_drt;
}

static void sdram_set_channel_mode(struct sys_info *sysinfo)
{
	u32 reg32;

	printk(BIOS_DEBUG, "Setting mode of operation for memory channels...");

	if (sdram_capabilities_interleave() &&
		    ( ( sysinfo->banksize[0] + sysinfo->banksize[1] +
			sysinfo->banksize[2] + sysinfo->banksize[3] ) ==
		      ( sysinfo->banksize[4] + sysinfo->banksize[5] +
			sysinfo->banksize[6] + sysinfo->banksize[7] ) ) ) {
		/* Both channels equipped with DIMMs of the same size */
		sysinfo->interleaved = 1;
	} else {
		sysinfo->interleaved = 0;
	}

	reg32 = MCHBAR32(DCC);
	reg32 &= ~(7 << 0);

	if(sysinfo->interleaved) {
		/* Dual Channel Interleaved */
		printk(BIOS_DEBUG, "Dual Channel Interleaved.\n");
		reg32 |= (1 << 1);
	} else if (sysinfo->dimm[0] == SYSINFO_DIMM_NOT_POPULATED &&
			sysinfo->dimm[1] == SYSINFO_DIMM_NOT_POPULATED) {
		/* Channel 1 only */
		printk(BIOS_DEBUG, "Single Channel 1 only.\n");
		reg32 |= (1 << 2);
	} else if (sdram_capabilities_dual_channel() && sysinfo->dimm[2] !=
			SYSINFO_DIMM_NOT_POPULATED) {
		/* Dual Channel Assymetric */
		printk(BIOS_DEBUG, "Dual Channel Assymetric.\n");
		reg32 |= (1 << 0);
	} else {
		/* All bits 0 means Single Channel 0 operation */
		printk(BIOS_DEBUG, "Single Channel 0 only.\n");
	}

#if 0
	/* Now disable channel XORing */
	reg32 |= (1 << 10);
#endif
	MCHBAR32(DCC) = reg32;

	PRINTK_DEBUG("DCC=0x%08x\n", MCHBAR32(DCC));
}

#if 0
static void sdram_program_pll_settings(struct sys_info *sysinfo)
{
	volatile u16 reg16;

	MCHBAR32(PLLMON) = 0x80800000;

	sysinfo->fsb_frequency = fsbclk();
	if (sysinfo->fsb_frequency == 0xffff)
		die("Unsupported FSB speed");

	/* Program CPCTL according to FSB speed */
	/* Only write the lower byte */
	switch (sysinfo->fsb_frequency) {
	case 400: MCHBAR8(CPCTL) = 0x90; break; /* FSB400 */
	case 533: MCHBAR8(CPCTL) = 0x95; break;	/* FSB533 */
	case 667: MCHBAR8(CPCTL) = 0x8d; break;	/* FSB667 */
	}

	MCHBAR16(CPCTL) &= ~(1 << 11);

	reg16 = MCHBAR16(CPCTL); /* Read back register to activate settings */
}

static void sdram_program_graphics_frequency(struct sys_info *sysinfo)
{
	u8  reg8;
	u16 reg16;
	u8  freq, second_vco, voltage;

#define CRCLK_166MHz	0x00
#define CRCLK_200MHz	0x01
#define CRCLK_250MHz	0x03
#define CRCLK_400MHz	0x05

#define CDCLK_200MHz	0x00
#define CDCLK_320MHz	0x40

#define VOLTAGE_1_05	0x00
#define VOLTAGE_1_50	0x01

	printk(BIOS_DEBUG, "Setting Graphics Frequency... \n");

	printk(BIOS_DEBUG, "FSB: %d MHz ", sysinfo->fsb_frequency);

	voltage = VOLTAGE_1_05;
	if (MCHBAR32(DFT_STRAP1) & (1 << 20))
		voltage = VOLTAGE_1_50;
	printk(BIOS_DEBUG, "Voltage: %s ", (voltage==VOLTAGE_1_05)?"1.05V":"1.5V");

	/* Gate graphics hardware for frequency change */
	reg8 = pci_read_config16(PCI_DEV(0,2,0), GCFC + 1);
	reg8 = (1<<3) | (1<<1); /* disable crclk, gate cdclk */
	pci_write_config8(PCI_DEV(0,2,0), GCFC + 1, reg8);

	/* Get graphics frequency capabilities */
	reg8 = sdram_capabilities_core_frequencies();

	freq = CRCLK_250MHz;
	switch (reg8) {
	case GFX_FREQUENCY_CAP_ALL:
		if (voltage == VOLTAGE_1_05)
			freq = CRCLK_250MHz;
		else
			freq = CRCLK_400MHz; /* 1.5V requires 400MHz */
		break;
	case GFX_FREQUENCY_CAP_250MHZ: freq = CRCLK_250MHz; break;
	case GFX_FREQUENCY_CAP_200MHZ: freq = CRCLK_200MHz; break;
	case GFX_FREQUENCY_CAP_166MHZ: freq = CRCLK_166MHz; break;
	}

	if (freq != CRCLK_400MHz) {
		/* What chipset are we? Force 166MHz for GMS */
		reg8 = (pci_read_config8(PCI_DEV(0, 0x00,0), 0xe7) & 0x70) >> 4;
		if (reg8==2)
			freq = CRCLK_166MHz;
	}

	printk(BIOS_DEBUG, "Render: ");
	switch (freq) {
	case CRCLK_166MHz: printk(BIOS_DEBUG, "166Mhz"); break;
	case CRCLK_200MHz: printk(BIOS_DEBUG, "200Mhz"); break;
	case CRCLK_250MHz: printk(BIOS_DEBUG, "250Mhz"); break;
	case CRCLK_400MHz: printk(BIOS_DEBUG, "400Mhz"); break;
	}

	sysinfo->mvco4x = 1;
	second_vco = 0;

	if (voltage == VOLTAGE_1_50) {
		second_vco = 1;
	}
	
	if (second_vco) {
		sysinfo->clkcfg_bit7=1;
	} else {
		sysinfo->clkcfg_bit7=0;
	}

	/* Graphics Core Render Clock */
	reg16 = pci_read_config16(PCI_DEV(0,2,0), GCFC);
	reg16 &= ~( (7 << 0) | (1 << 13) );
	reg16 |= freq;
	pci_write_config16(PCI_DEV(0,2,0), GCFC, reg16);

	/* Graphics Core Display Clock */
	reg8 = pci_read_config8(PCI_DEV(0,2,0), GCFC);
	reg8 &= ~( (1<<7) | (7<<4) );

	if (voltage == VOLTAGE_1_05) {
		reg8 |= CDCLK_200MHz;
		printk(BIOS_DEBUG, " Display: 200MHz\n");
	} else {
		reg8 |= CDCLK_320MHz;
		printk(BIOS_DEBUG, " Display: 320MHz\n");
	}
	pci_write_config8(PCI_DEV(0,2,0), GCFC, reg8);

	reg8 = pci_read_config8(PCI_DEV(0,2,0), GCFC + 1);

	reg8 |= (1<<3) | (1<<1);
	pci_write_config8(PCI_DEV(0,2,0), GCFC + 1, reg8);

	reg8 |= 0x0f;
	pci_write_config8(PCI_DEV(0,2,0), GCFC + 1, reg8);

	/* Ungate core render and display clocks */
	reg8 &= 0xf0;
	pci_write_config8(PCI_DEV(0,2,0), GCFC + 1, reg8);
}
#endif

static void sdram_program_memory_frequency(struct sys_info *sysinfo)
{
	u32 clkcfg;

	printk(BIOS_DEBUG, "Setting Memory Frequency... ");

	clkcfg = MCHBAR32(CLKCFG);

	printk(BIOS_DEBUG, "CLKCFG=0x%08x, ", clkcfg);

	clkcfg &= ~( (1 << 12) | (1 << 7) | ( 7 << 4) );

	if (sysinfo->mvco4x) {
		printk(BIOS_DEBUG, "MVCO 4x, ");
		clkcfg &= ~(1 << 12);	/* FIXME: clkcfg |= 1<<12 ?  */
	}
	if (sysinfo->clkcfg_bit7) {
		printk(BIOS_DEBUG, "second VCO, ");
		clkcfg |= (1 << 7);
	}

	switch (sysinfo->memory_frequency) {
#if 0
	case 400: clkcfg |= (1 << 4); break;
	case 533: clkcfg |= (2 << 4); break;
	case 667: clkcfg |= (3 << 4); break;
#else
	case 333: clkcfg |= (1 << 4); break; /* ?? */
	case 400: clkcfg |= (2 << 4); break;
	case 533: clkcfg |= (3 << 4); break;
#endif
	default: die("Target Memory Frequency Error");
	}

#if 0
	if (MCHBAR32(CLKCFG) == clkcfg) {
		printk(BIOS_DEBUG, "ok (unchanged)\n");
		return;
	}
#endif
	MCHBAR32(CLKCFG) = clkcfg;
	printk(BIOS_DEBUG, "CLKCFG=0x%08x, ", MCHBAR32(CLKCFG));
	printk(BIOS_DEBUG, "ok\n");
}

static void sdram_disable_fast_dispatch(void)
{
	u32 reg32;

	/* add pci revision test here */
	reg32 = MCHBAR32(FSBPMC3);
	reg32 |= (1 << 1);
	MCHBAR32(FSBPMC3) = reg32;

	reg32 = MCHBAR32(FSBPMC4);
	reg32 |= (1 << 0);
	MCHBAR32(FSBPMC4) = reg32;

	reg32 = MCHBAR32(SBTEST);
	reg32 |= (3 << 1);
	MCHBAR32(SBTEST) = reg32;
}


static void sdram_pre_jedec_initialization(void)
{
	u32 reg32;

	reg32 = MCHBAR32(FSBPMC3);
	reg32 &= ~(1 << 1);
	MCHBAR32(FSBPMC3) = reg32;

	reg32 = MCHBAR32(SBTEST);
	reg32 &= ~(1 << 2);
	MCHBAR32(SBTEST) = reg32;

	
	/* Adaptive Idle Timer Control */
	MCHBAR32(C0AIT) = 0x000006c4;
	MCHBAR32(C0AIT+4) = 0x871a066d;

	MCHBAR32(C1AIT) = 0x000006c4;
	MCHBAR32(C1AIT+4) = 0x871a066d;

}

#define EA_DUALCHANNEL_XOR_BANK_RANK_MODE	(0xd4 << 24)
#define EA_DUALCHANNEL_XOR_BANK_MODE		(0xf4 << 24)
#define EA_DUALCHANNEL_BANK_RANK_MODE		(0xc2 << 24)
#define EA_DUALCHANNEL_BANK_MODE		(0xe2 << 24)
#define EA_SINGLECHANNEL_XOR_BANK_RANK_MODE	(0x91 << 24)
#define EA_SINGLECHANNEL_XOR_BANK_MODE		(0xb1 << 24)
#define EA_SINGLECHANNEL_BANK_RANK_MODE		(0x80 << 24)
#define EA_SINGLECHANNEL_BANK_MODE		(0xa0 << 24)

static void sdram_enhanced_addressing_mode(struct sys_info *sysinfo)
{
	u32 chan0 = 0, chan1 = 0;
	int chan0_dualsided, chan1_dualsided, chan0_populated, chan1_populated;

	chan0_populated =  (sysinfo->dimm[0] != SYSINFO_DIMM_NOT_POPULATED ||
			sysinfo->dimm[1] != SYSINFO_DIMM_NOT_POPULATED);
	chan1_populated = (sysinfo->dimm[0] != SYSINFO_DIMM_NOT_POPULATED ||
			sysinfo->dimm[1] != SYSINFO_DIMM_NOT_POPULATED);
	chan0_dualsided = (sysinfo->banksize[1] || sysinfo->banksize[3]);
	chan1_dualsided = (sysinfo->banksize[5] || sysinfo->banksize[7]);

	if (sdram_capabilities_enhanced_addressing_xor()) {
		if (!sysinfo->interleaved) {
			/* Single Channel & Dual Channel Assymetric */
			if (chan0_populated) {
				if (chan0_dualsided) {
					chan0 = EA_SINGLECHANNEL_XOR_BANK_RANK_MODE;
				} else {
					chan0 = EA_SINGLECHANNEL_XOR_BANK_MODE;
				}
			}
			if (chan1_populated) {
				if (chan1_dualsided) {
					chan1 = EA_SINGLECHANNEL_XOR_BANK_RANK_MODE;
				} else {
					chan1 = EA_SINGLECHANNEL_XOR_BANK_MODE;
				}
			}
		} else {
			/* Interleaved has always both channels populated */
			if (chan0_dualsided) {
				chan0 = EA_DUALCHANNEL_XOR_BANK_RANK_MODE;
			} else {
				chan0 = EA_DUALCHANNEL_XOR_BANK_MODE;
			}

			if (chan1_dualsided) {
				chan1 = EA_DUALCHANNEL_XOR_BANK_RANK_MODE;
			} else {
				chan1 = EA_DUALCHANNEL_XOR_BANK_MODE;
			}
		}
	} else {
		if (!sysinfo->interleaved) {
			/* Single Channel & Dual Channel Assymetric */
			if (chan0_populated) {
				if (chan0_dualsided) {
					chan0 = EA_SINGLECHANNEL_BANK_RANK_MODE;
				} else {
					chan0 = EA_SINGLECHANNEL_BANK_MODE;
				}
			}
			if (chan1_populated) {
				if (chan1_dualsided) {
					chan1 = EA_SINGLECHANNEL_BANK_RANK_MODE;
				} else {
					chan1 = EA_SINGLECHANNEL_BANK_MODE;
				}
			}
		} else {
			/* Interleaved has always both channels populated */
			if (chan0_dualsided) {
				chan0 = EA_DUALCHANNEL_BANK_RANK_MODE;
			} else {
				chan0 = EA_DUALCHANNEL_BANK_MODE;
			}

			if (chan1_dualsided) {
				chan1 = EA_DUALCHANNEL_BANK_RANK_MODE;
			} else {
				chan1 = EA_DUALCHANNEL_BANK_MODE;
			}
		}
	}

	MCHBAR32(C0DRC1) &= 0x00ffffff;
	MCHBAR32(C0DRC1) |= chan0;
	MCHBAR32(C1DRC1) &= 0x00ffffff;
	MCHBAR32(C1DRC1) |= chan1;
}

static void sdram_post_jedec_initialization(struct sys_info *sysinfo)
{
	u32 reg32;
	
#if 0
	reg32 = MCHBAR32(FSBPMC3);
	reg32 &= ~(1 << 1);
	MCHBAR32(FSBPMC3) = reg32;

	reg32 = MCHBAR32(SBTEST);
	reg32 &= ~(1 << 2);
	MCHBAR32(SBTEST) = reg32;
#endif

	/* Enable Channel XORing for Dual Channel Interleave */
	if (sysinfo->interleaved) {

		reg32 = MCHBAR32(DCC);
#if CONFIG_CHANNEL_XOR_RANDOMIZATION
		reg32 &= ~(1 << 10);
		reg32 |= (1 << 9);
#else
		reg32 &= ~(1 << 9);
#endif
		MCHBAR32(DCC) = reg32;
	}

	MCHBAR32(WCC) |= (1 << 11);
	reg32 = MCHBAR32(WCC);
	reg32 &= 0xf33ffbff; /* FIXME */
	reg32 |= (2<<24) | (3 << 22) | (1 << 10);
	MCHBAR32(WCC) = reg32;

#if 0

	MCHBAR32(SMVREFC) |= (1 << 6);

	MCHBAR32(MMARB0) &= ~(3 << 17);
	MCHBAR32(MMARB0) |= (1 << 21) | (1 << 16);

	MCHBAR32(MMARB1) &= ~(7 << 8);
	MCHBAR32(MMARB1) |= (3 << 8);
#endif
	reg32 = MCHBAR32(MMARB0);
	reg32 &= ~(3 << 21);
	reg32 |= (3 << 21);
	MCHBAR32(MMARB0) = reg32;

	MCHBAR32(WCC) |= (1<<2);

	/* DRAM mode optimizations */
	sdram_enhanced_addressing_mode(sysinfo);

#if 0
	reg32 = MCHBAR32(SBOCC);
	reg32 &= 0xffbdb6ff;
	reg32 |= (0xbdb6 << 8) | (1 << 0);
	MCHBAR32(SBOCC) = reg32;
#endif
}

#if 0
static void sdram_power_management(struct sys_info *sysinfo)
{
	u8 reg8;
	u16 reg16;
	u32 reg32;
	int integrated_graphics = 1;
	int i;

	reg32 = MCHBAR32(C0DRT2);
	reg32 &= 0xffffff00;
	/* Idle timer = 8 clocks, CKE idle timer = 16 clocks */
	reg32 |= (1 << 5) | (1 << 4);
	MCHBAR32(C0DRT2) = reg32;

	reg32 = MCHBAR32(C1DRT2);
	reg32 &= 0xffffff00;
	/* Idle timer = 8 clocks, CKE idle timer = 16 clocks */
	reg32 |= (1 << 5) | (1 << 4);
	MCHBAR32(C1DRT2) = reg32;

	reg32 = MCHBAR32(C0DRC1);

	reg32 |= (1 << 12) | (1 << 11);
	MCHBAR32(C0DRC1) = reg32;

	reg32 = MCHBAR32(C1DRC1);

	reg32 |= (1 << 12) | (1 << 11);
	MCHBAR32(C1DRC1) = reg32;

	/* FIXME bits 5 and 0 only if PCIe graphics is disabled */
	u16 peg_bits = (1 << 5) | (1 << 0);

	/* Rev 0 and 1 */
	MCHBAR16(UPMC1) = 0x0010 | peg_bits;

	reg16 = MCHBAR16(UPMC2);
	reg16 &= 0xfc00;
	reg16 |= 0x0100;
	MCHBAR16(UPMC2) = reg16;

	MCHBAR32(UPMC3) = 0x000f06ff;

	for (i=0; i<5; i++) {
		MCHBAR32(UPMC3) &= ~(1 << 16);
		MCHBAR32(UPMC3) |= (1 << 16);
	}

	MCHBAR32(GIPMC1) = 0x8000000c;

	reg16 = MCHBAR16(CPCTL);
	reg16 &= ~(7 << 11);
	reg16 |= (4 << 11);
	MCHBAR16(CPCTL) = reg16;

	switch (sysinfo->fsb_frequency) {
	case 667: MCHBAR32(HGIPMC2) = 0x09c409c4; break;
	case 533: MCHBAR32(HGIPMC2) = 0x0fa00fa0; break;
	}

	MCHBAR32(FSBPMC1) = 0x8000000c;

	reg32 = MCHBAR32(C2C3TT);
	reg32 &= 0xffff0000;
	switch (sysinfo->fsb_frequency) {
	case 667: reg32 |= 0x0600; break;
	case 533: reg32 |= 0x0480; break;
	}
	MCHBAR32(C2C3TT) = reg32;

	reg32 = MCHBAR32(C3C4TT);
	reg32 &= 0xffff0000;
	switch (sysinfo->fsb_frequency) {
	case 667: reg32 |= 0x0b80; break;
	case 533: reg32 |= 0x0980; break;
	}
	MCHBAR32(C3C4TT) = reg32;

	MCHBAR32(ECO) &= ~(1 << 16);

	MCHBAR32(FSBPMC3) &= ~(1 << 29);

	MCHBAR32(FSBPMC3) |= (1 << 21);

	MCHBAR32(FSBPMC3) &= ~(1 << 19);

	MCHBAR32(FSBPMC3) &= ~(1 << 13);

	reg32 = MCHBAR32(FSBPMC4);
	reg32 &= ~(3 << 24);
	reg32 |= ( 2 << 24);
	MCHBAR32(FSBPMC4) = reg32;

	MCHBAR32(FSBPMC4) |= (1 << 21);

	MCHBAR32(FSBPMC4) |= (1 << 5);

	/* stepping 0 and 1 or CPUID 6e8 */
	MCHBAR32(FSBPMC4) &= ~(1 << 4);

	reg8 = pci_read_config8(PCI_DEV(0,0x0,0), 0xfc);
	reg8 |= (1 << 4);
	pci_write_config8(PCI_DEV(0, 0x0, 0), 0xfc, reg8);

	reg8 = pci_read_config8(PCI_DEV(0,0x2,0), 0xc1);
	reg8 |= (1 << 2);
	pci_write_config8(PCI_DEV(0, 0x2, 0), 0xc1, reg8);

#ifdef C2_SELF_REFRESH_DISABLE

	if (integrated_graphics) {
		printk(BIOS_DEBUG, "C2 self-refresh with IGD\n");
		MCHBAR16(MIPMC4) = 0x0468;
		MCHBAR16(MIPMC5) = 0x046c;
		MCHBAR16(MIPMC6) = 0x046c;
	} else {
		MCHBAR16(MIPMC4) = 0x6468;
		MCHBAR16(MIPMC5) = 0x646c;
		MCHBAR16(MIPMC6) = 0x646c;
	}
#else
	if (integrated_graphics) {
		MCHBAR16(MIPMC4) = 0x04f8;
		MCHBAR16(MIPMC5) = 0x04fc;
		MCHBAR16(MIPMC6) = 0x04fc;
	} else {
		MCHBAR16(MIPMC4) = 0x64f8;
		MCHBAR16(MIPMC5) = 0x64fc;
		MCHBAR16(MIPMC6) = 0x64fc;
	}

#endif

	reg32 = MCHBAR32(PMCFG);
	reg32 &= ~(3 << 17);
	reg32 |= (2 << 17);
	MCHBAR32(PMCFG) = reg32;

	MCHBAR32(PMCFG) |= (1 << 4);

	reg32 = MCHBAR32(0xc30);
	reg32 &= 0xffffff00;
	reg32 |= 0x01;
	MCHBAR32(0xc30) = reg32;

	MCHBAR32(0xb18) &= ~(1 << 21);
}

static void sdram_thermal_management(void)
{

	MCHBAR8(TCO1) = 0x00;
	MCHBAR8(TCO0) = 0x00;

	/* The Thermal Sensors for DIMMs at 0x50, 0x52 are at I2C addr
	 * 0x30/0x32.
	 */

	/* TODO This is not implemented yet. Volunteers? */
}

static void sdram_save_receive_enable(void)
{
	int i;
	u32 reg32;
	u8 values[4];

	/* The following values are stored to an unused CMOS
	 * area and restored instead of recalculated in case
	 * of an S3 resume.
	 *
	 * C0WL0REOST [7:0] 		-> 8 bit
	 * C1WL0REOST [7:0] 		-> 8 bit
	 * RCVENMT    [11:8] [3:0]	-> 8 bit
	 * C0DRT1     [27:24]		-> 4 bit
	 * C1DRT1     [27:24]		-> 4 bit
	 */

	values[0] = MCHBAR8(C0WL0REOST);
	values[1] = MCHBAR8(C1WL0REOST);

	reg32 = MCHBAR32(RCVENMT);
	values[2] = (u8)((reg32 >> (8 - 4)) & 0xf0) | (reg32 & 0x0f);

	reg32 = MCHBAR32(C0DRT1);
	values[3] = (reg32 >> 24) & 0x0f;
	reg32 = MCHBAR32(C1DRT1);
	values[3] |= (reg32 >> (24 - 4)) & 0xf0;

#if 0
	/* coreboot only uses bytes 0 - 127 for its CMOS values so far
	 * so we grab bytes 128 - 131 to save the receive enable values
	 */

	for (i=0; i<4; i++)
		cmos_write(values[i], 128 + i);
#endif
}

#endif

static void sdram_recover_receive_enable(void)
{
	u32 reg32;
#if 0
	int i;
	u8 values[4];

	for (i=0; i<4; i++)
		values[i] = cmos_read(128 + i);
#else
	u8 values[4] = {0x00, 0x52, 0xcf, 0x33};
#endif

	reg32 = MCHBAR32(RCVENMT);
	reg32 &= ~((0x0f << 8) | (0x0f << 0));
	//reg32 |= ((u32)(values[2] & 0xf0) << (8 - 4)) | (values[2] & 0x0f);
	reg32 |= 0x0c0f;
	MCHBAR32(RCVENMT) = reg32;

	reg32 = MCHBAR32(C0DRT1) & ~(0x0f << 24);
	reg32 |= (u32)(values[3] & 0x0f) << 24;
	MCHBAR32(C0DRT1) = reg32;

	reg32 = MCHBAR32(C1DRT1) & ~(0x0f << 24);
	reg32 |= (u32)(values[3] & 0xf0) << (24 - 4);
	MCHBAR32(C1DRT1) = reg32;

	reg32 = MCHBAR32(RCVENMT);
	reg32 &= ~((0x0f << 8) | (0x0f << 0));
	//reg32 |= ((u32)(values[2] & 0xf0) << (8 - 4)) | (values[2] & 0x0f);
	reg32 |= 0x0c00;
	MCHBAR32(RCVENMT) = reg32;

	MCHBAR8(C0WL0REOST) = values[0];
	MCHBAR8(C1WL0REOST) = values[1];
}

#if 0

#include "rcven.c"

#endif

void sdram_program_receive_enable(struct sys_info *sysinfo)
{
#if 0
	MCHBAR32(REPC) |= (1 << 0);

	/* enable upper CMOS */
	RCBA32(0x3400) = (1 << 2);

	/* Program Receive Enable Timings */
	if (sysinfo->boot_path == BOOT_PATH_RESUME) {
		sdram_recover_receive_enable();
	} else {
		receive_enable_adjust(sysinfo);
		sdram_save_receive_enable();
	}

	MCHBAR32(C0DRC1) |= (1 << 6);
	MCHBAR32(C1DRC1) |= (1 << 6);
	MCHBAR32(C0DRC1) &= ~(1 << 6);
	MCHBAR32(C1DRC1) &= ~(1 << 6);

	MCHBAR32(MIPMC3) |= (0x0f << 0);
#else
	sdram_recover_receive_enable();
#endif
}


/**
 * @brief Enable clocks to populated sockets
 */

static void sdram_enable_memory_clocks(struct sys_info *sysinfo)
{
	u8 clocks[2] = { 0, 0 };

#define CLOCKS_WIDTH 3

	if (sysinfo->dimm[0] != SYSINFO_DIMM_NOT_POPULATED)
		clocks[0] |= (1 << CLOCKS_WIDTH)-1;

	if (sysinfo->dimm[1] != SYSINFO_DIMM_NOT_POPULATED)
		clocks[0] |= ((1 << CLOCKS_WIDTH)-1) << CLOCKS_WIDTH;

	if (sysinfo->dimm[2] != SYSINFO_DIMM_NOT_POPULATED)
		clocks[1] |= (1 << CLOCKS_WIDTH)-1;

	if (sysinfo->dimm[3] != SYSINFO_DIMM_NOT_POPULATED)
		clocks[1] |= ((1 << CLOCKS_WIDTH)-1) << CLOCKS_WIDTH;

#if CONFIG_OVERRIDE_CLOCK_DISABLE
	/* Usually system firmware turns off system memory clock signals
	 * to unused SO-DIMM slots to reduce EMI and power consumption.
	 * However, the Kontron 986LCD-M does not like unused clock
	 * signals to be disabled.
	 */

	clocks[0] = 0xf; /* force all clock gate pairs to enable */
	clocks[1] = 0xf; /* force all clock gate pairs to enable */
#endif

	MCHBAR8(C0DCLKDIS) = clocks[0];
	MCHBAR8(C1DCLKDIS) = clocks[1];
}


#define RTT_ODT_NONE	0
#define RTT_ODT_50_OHM  ( (1 << 4) | (1 << 3) )
#define RTT_ODT_75_OHM	(1 << 3)
#define RTT_ODT_150_OHM	(1 << 4)

#define EMRS_OCD_DEFAULT	( (1 << 12) | (1 << 11) | (1 << 10) )

#define MRS_CAS_3	(3 << 7)
#define MRS_CAS_4	(4 << 7)
#define MRS_CAS_5	(5 << 7)

#define MRS_TWR_3	(2 << 12)
#define MRS_TWR_4	(3 << 12)
#define MRS_TWR_5	(4 << 12)

#define MRS_BT		(1 << 6)

#define MRS_BL4		(2 << 3)
#define MRS_BL8		(3 << 3)

static void sdram_jedec_enable(struct sys_info *sysinfo)
{
	int i, nonzero;
	u32 bankaddr = 0, tmpaddr, mrsaddr = 0;

	for (i = 0, nonzero = -1; i < 8; i++) {
		if (sysinfo->banksize[i]  == 0) {
			continue;
		}

		printk(BIOS_DEBUG, "jedec enable sequence: bank %d\n", i);
		switch (i) {
		case 0:
			/* Start at address 0 */
			bankaddr = 0;
			break;
		case 4:
			if (sysinfo->interleaved) {
				bankaddr = 0x40;
				break;
			}
		default:
			if (nonzero != -1) {
				printk(BIOS_DEBUG, "bankaddr from bank size of rank %d\n", nonzero);
				bankaddr += sysinfo->banksize[nonzero] <<
					(sysinfo->interleaved ? 26 : 25);
				break;
			}
			/* No populated bank hit before. Start at address 0 */
			bankaddr = 0;
		}

		/* We have a bank with a non-zero size.. Remember it
		 * for the next offset we have to calculate
		 */
		nonzero = i;

		/* Get CAS latency set up */
		switch (sysinfo->cas) {
		case 5: mrsaddr = MRS_CAS_5; break;
		case 4: mrsaddr = MRS_CAS_4; break;
		case 3: mrsaddr = MRS_CAS_3; break;
		default: die("Jedec Error (CAS).\n");
		}

		/* Get tWR set */
		switch (sysinfo->twr) {
		case 5: mrsaddr |= MRS_TWR_5; break;
		case 4: mrsaddr |= MRS_TWR_4; break;
		case 3: mrsaddr |= MRS_TWR_3; break;
		default: die("Jedec Error (tWR).\n");
		}

		/* Set "Burst Type" */
		mrsaddr |= MRS_BT;

		/* Interleaved */
		if (sysinfo->interleaved) {
			mrsaddr = mrsaddr << 1;
		}

		/* Only burst length 8 supported */
		mrsaddr |= MRS_BL8;

		/* Apply NOP */
		PRINTK_DEBUG("Apply NOP\n");
		do_ram_command(RAM_COMMAND_NOP);
		ram_read32(bankaddr);

		/* Precharge all banks */
		PRINTK_DEBUG("All Banks Precharge\n");
		do_ram_command(RAM_COMMAND_PRECHARGE);
		ram_read32(bankaddr);

		/* Extended Mode Register Set (2) */
		PRINTK_DEBUG("Extended Mode Register Set(2)\n");
		do_ram_command(RAM_COMMAND_EMRS | RAM_EMRS_2);
		ram_read32(bankaddr);

		/* Extended Mode Register Set (3) */
		PRINTK_DEBUG("Extended Mode Register Set(3)\n");
		do_ram_command(RAM_COMMAND_EMRS | RAM_EMRS_3);
		ram_read32(bankaddr);

		/* Extended Mode Register Set */
		PRINTK_DEBUG("Extended Mode Register Set\n");
		do_ram_command(RAM_COMMAND_EMRS | RAM_EMRS_1);
		tmpaddr = bankaddr;
		if (!sdram_capabilities_dual_channel()) {
			tmpaddr |= RTT_ODT_75_OHM;
		} else if (sysinfo->interleaved) {
			tmpaddr |= (RTT_ODT_150_OHM << 1);
		} else {
			tmpaddr |= RTT_ODT_150_OHM;
		}
		ram_read32(tmpaddr);

		/* Mode Register Set: Reset DLLs */
		PRINTK_DEBUG("MRS: Reset DLLs\n");
		do_ram_command(RAM_COMMAND_MRS);
		tmpaddr = bankaddr;
		tmpaddr |= mrsaddr;
		/* Set DLL reset bit */
		if (sysinfo->interleaved)
			tmpaddr |= (1 << 12);
		else
			tmpaddr |= (1 << 11);
		ram_read32(tmpaddr);

		/* Precharge all banks */
		PRINTK_DEBUG("All Banks Precharge\n");
		do_ram_command(RAM_COMMAND_PRECHARGE);
		ram_read32(bankaddr);

		/* CAS before RAS Refresh */
		PRINTK_DEBUG("CAS before RAS\n");
		do_ram_command(RAM_COMMAND_CBR);

		/* CBR wants two READs */
		ram_read32(bankaddr);
		ram_read32(bankaddr);

		/* Mode Register Set: Enable DLLs */
		PRINTK_DEBUG("MRS: Enable DLLs\n");
		do_ram_command(RAM_COMMAND_MRS);

		tmpaddr = bankaddr;
		tmpaddr |= mrsaddr;
		ram_read32(tmpaddr);

		/* Extended Mode Register Set */
		PRINTK_DEBUG("Extended Mode Register Set: ODT/OCD\n");
		do_ram_command(RAM_COMMAND_EMRS | RAM_EMRS_1);

		tmpaddr = bankaddr;
		if (!sdram_capabilities_dual_channel()) {

			tmpaddr |= RTT_ODT_75_OHM | EMRS_OCD_DEFAULT;
		} else if (sysinfo->interleaved) {
			tmpaddr |= ((RTT_ODT_150_OHM | EMRS_OCD_DEFAULT) << 1);
		} else {
			tmpaddr |= RTT_ODT_150_OHM | EMRS_OCD_DEFAULT;
		}
		ram_read32(tmpaddr);

		/* Extended Mode Register Set */
		PRINTK_DEBUG("Extended Mode Register Set: OCD Exit\n");
		do_ram_command(RAM_COMMAND_EMRS | RAM_EMRS_1);

		tmpaddr = bankaddr;
		if (!sdram_capabilities_dual_channel()) {
			tmpaddr |= RTT_ODT_75_OHM;
		} else if (sysinfo->interleaved) {
			tmpaddr |= (RTT_ODT_150_OHM << 1);
		} else {
			tmpaddr |= RTT_ODT_150_OHM;
		}
		ram_read32(tmpaddr);
	}
}

#if 0
static void sdram_setup_re_side(void)
{
	MCHBAR32(FSBPMC3) |= (1 << 2);

	MCHBAR8(0xb00) |= 1;

	MCHBAR32(SLPCTL) |= (1 << 8);
}
#endif

static void sdram_init_complete(void)
{
	PRINTK_DEBUG("Normal Operation\n");
	do_ram_command(RAM_COMMAND_NORMAL);
}

static void sdram_enable_rcomp(void)
{
	/* Enable Global Periodic RCOMP */
	MCHBAR32(GBRCOMPCTL) |= (1 << 27) | (3 << 0);
	MCHBAR32(GBRCOMPCTL) &= ~(1 << 23);
}


/**
 * @param boot_path: 0 = normal, 1 = reset, 2 = resume from s3
 */
void sdram_initialize(int boot_path, const u8 *spd_addresses)
{
	struct sys_info sysinfo;
	u8 reg8, cas_mask;

	printk(BIOS_DEBUG, "Setting up RAM controller.\n");

	memset(&sysinfo, 0, sizeof(sysinfo));

	sysinfo.boot_path = boot_path;
	sysinfo.spd_addresses = spd_addresses;

	/* Look at the type of DIMMs and verify all DIMMs are x8 or x16 width */
	sdram_get_dram_configuration(&sysinfo);

#if 0
	/* If error, do cold boot */
	sdram_detect_errors(&sysinfo);
#endif

	/* Check whether we have stacked DIMMs */
	sdram_verify_package_type(&sysinfo);
	

	/* Determine common CAS */
	cas_mask = sdram_possible_cas_latencies(&sysinfo);

	/* Choose Common Frequency */
	sdram_detect_cas_latency_and_ram_speed(&sysinfo, cas_mask);

	/* Determine smallest common tRAS */
	sdram_detect_smallest_tRAS(&sysinfo);

	/* Determine tRP */
	sdram_detect_smallest_tRP(&sysinfo);

	/* Determine tRCD */
	sdram_detect_smallest_tRCD(&sysinfo);

	/* Determine smallest refresh period */
	sdram_detect_smallest_refresh(&sysinfo);

	/* Verify all DIMMs support burst length 8 */
	sdram_verify_burst_length(&sysinfo);

	/* determine tWR */
	sdram_detect_smallest_tWR(&sysinfo);

	/* Determine DIMM size parameters (rows, columns banks) */
	sdram_detect_dimm_size(&sysinfo);

	/* determine tRFC */
	sdram_detect_smallest_tRFC(&sysinfo);

#if 0
	/* Program PLL settings */
	sdram_program_pll_settings(&sysinfo);

	/* Program Graphics Frequency */
	sdram_program_graphics_frequency(&sysinfo);
#endif

	/* Program Thermal Settings ?? */
	const uint32_t ax = (0x0e1c << 16) | 0x0e1c;
	sdram_thermal_setup(&sysinfo, ax);

	post_code(0x17);

	sdram_program_memory_frequency(&sysinfo);

	/* Program Clock Crossing values */
	sdram_program_clock_crossing();
	
	post_code(0x18);


#if 0
	/* Enable WIODLL Power Down in ACPI states */
	MCHBAR32(C0DMC) |= (1 << 24);
	MCHBAR32(C1DMC) |= (1 << 24);
#endif

	/* Determine Mode of Operation (Interleaved etc) */
	sdram_set_channel_mode(&sysinfo);

	/* Program DRAM Row Boundary/Attribute Registers */

	post_code(0x19);
	/* program row size DRB and set TOLUD */
	sdram_program_row_boundaries(&sysinfo);

	/* program page size DRA */
	sdram_set_row_attributes(&sysinfo);

	post_code(0x20);

	/* Program CxBNKARC */
	sdram_set_bank_architecture(&sysinfo);


	MCHBAR32(C0DRT1) |= 1<<16;
	MCHBAR32(C1DRT1) |= 1<<16;

	post_code(0x21);

	/* Program DRAM Timing and Control registers based on SPD */
	sdram_set_timing_and_control(&sysinfo);

	post_code(0x22);

	/* Enable System Memory Clocks */
	sdram_enable_memory_clocks(&sysinfo);


	/* Perform System Memory IO Initialization */
	sdram_initialize_system_memory_io(&sysinfo);

	/* Perform System Memory IO Buffer Enable */
	sdram_enable_system_memory_io(&sysinfo);

	post_code(0x26);

	/* On-Die Termination Adjustment */
	sdram_on_die_termination(&sysinfo);

	post_code(0x27);
	

	sdram_program_odt_tristate(&sysinfo);
	
	/* FIXME */
	MCHBAR32(UPMC4) |= 0x01;
	MCHBAR32(FSBPMC3) |= 0;
	MCHBAR8(SMVREFC) |= (1 << 6);
	
	/* Disable fast dispatch */
	sdram_disable_fast_dispatch();

	MCHBAR32(DCC) |= (1<<10);
	/* receive enable */
	MCHBAR32(REPC) |= (1<<0);

	MCHBAR32(MMARB0) |= (3 << 24) | (1 << 8);
	MCHBAR32(MMARB1) |= (4 << 16) | (3 << 12);

	MCHBAR32(C0DRC1) |= (1<<10);
	MCHBAR32(C1DRC1) |= (1<<10);

	post_code(0x28);

#if 0
	sdram_program_cke_tristate(&sysinfo);
#endif

	if (boot_path == BOOT_PATH_NORMAL) {
		/* Jedec Initialization sequence */
		sdram_jedec_enable(&sysinfo);
	}

	post_code(0x29);

	/* Pre Jedec Initialization */
	sdram_pre_jedec_initialization();

#if 0
	/* Program Power Management Registers */
	sdram_power_management(&sysinfo);
#endif

	/* Post Jedec Init */
	sdram_post_jedec_initialization(&sysinfo);

	post_code(0x30);

#if 0
	/* Program DRAM Throttling */
	sdram_thermal_management();
#endif

	post_code(0x31);

	sdram_program_refresh_rate(&sysinfo);

	post_code(0x32);

#if 0
	/* Program Receive Enable Timings */
	sdram_program_receive_enable(&sysinfo);
#endif

	/* Normal Operations */
	sdram_init_complete();

	post_code(0x33);

	/* Enable Periodic RCOMP */
	sdram_enable_rcomp();

#if 0
	die("halt\n");

	/* Tell ICH7 that we're done */
	reg8 = pci_read_config8(PCI_DEV(0,0x1f,0), 0xa2);
	reg8 &= ~(1 << 7);
	pci_write_config8(PCI_DEV(0, 0x1f, 0), 0xa2, reg8);

	post_code(0xaf);

	printk(BIOS_DEBUG, "RAM initialization finished.\n");

	sdram_setup_processor_side();
#endif
}

unsigned long get_top_of_ram(void)
{
	u32 tom;

	if (pci_read_config8(PCI_DEV(0, 0x0, 0), DEVEN) & ((1 << 4) | (1 << 3))) {
		/* IGD enabled, get top of Memory from BSM register */
		tom = pci_read_config32(PCI_DEV(0,2,0), 0x5c);
	} else {
		tom = (pci_read_config8(PCI_DEV(0,0,0), TOLUD) & 0xf7) << 24;
	}

	/* if TSEG enabled subtract size */
	switch(pci_read_config8(PCI_DEV(0, 0, 0), ESMRAM)) {
	case 0x01:
		/* 1MB TSEG */
		tom -= 0x10000;
		break;
	case 0x03:
		/* 2MB TSEG */
		tom -= 0x20000;
		break;
	case 0x05:
		/* 8MB TSEG */
		tom -= 0x80000;
		break;
	default:
		/* TSEG either disabled or invalid */
		break;
	}
	return (unsigned long) tom;
}

