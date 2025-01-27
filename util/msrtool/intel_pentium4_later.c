/*
 * This file is part of msrtool.
 *
 * Copyright (C) 2011 Anton Kochkov <anton.kochkov@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
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

#include "msrtool.h"

int intel_pentium4_later_probe(const struct targetdef *target, const struct cpuid_t *id) {
	return ((0xf == id->family) && (
		(0x3 == id->model) ||
		(0x4 == id->model)
		));
}

const struct msrdef intel_pentium4_later_msrs[] = {
	{0x0, MSRTYPE_RDWR, MSR2(0,0), "IA32_P5_MC_ADDR", "", {
		{ BITS_EOT }
	}},
	{0x1, MSRTYPE_RDWR, MSR2(0,0), "IA32_P5_MC_TYPE", "", {
		{ BITS_EOT }
	}},
	{0x6, MSRTYPE_RDWR, MSR2(0,0), "IA32_MONITOR_FILTER_LINE_SIZE", "", {
		{ BITS_EOT }
	}},
	{0x17, MSRTYPE_RDWR, MSR2(0,0), "IA32_PLATFORM_ID", "", {
		{ BITS_EOT }
	}},
	{0x2a, MSRTYPE_RDWR, MSR2(0,0), "MSR_EBC_HARD_POWERON", "", {
		{ BITS_EOT }
	}},
	{0x2b, MSRTYPE_RDWR, MSR2(0,0), "MSR_EBC_SOFT_POWRON", "", {
		{ BITS_EOT }
	}},
	{0x2c, MSRTYPE_RDWR, MSR2(0,0), "MSR_EBC_FREQUENCY_ID", "", {
		{ BITS_EOT }
	}},
	{0x19c, MSRTYPE_RDWR, MSR2(0,0), "IA32_THERM_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x19d, MSRTYPE_RDWR, MSR2(0,0), "MSR_THERM2_CTL", "", {
		{ BITS_EOT }
	}},
	{0x1a0, MSRTYPE_RDWR, MSR2(0,0), "IA32_MISC_ENABLE", "", {
		{ BITS_EOT }
	}},
	{0x1a1, MSRTYPE_RDWR, MSR2(0,0), "MSR_PLATFORM_BRV", "", {
		{ BITS_EOT }
	}},
	{0x200, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE0", "", {
		{ BITS_EOT }
	}},
	{0x201, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK0", "", {
		{ BITS_EOT }
	}},
	{0x202, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE1", "", {
		{ BITS_EOT }
	}},
	{0x203, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK1", "", {
		{ BITS_EOT }
	}},
	{0x204, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE2", "", {
		{ BITS_EOT }
	}},
	{0x205, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK2", "", {
		{ BITS_EOT }
	}},
	{0x206, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE3", "", {
		{ BITS_EOT }
	}},
	{0x207, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK3", "", {
		{ BITS_EOT }
	}},
	{0x208, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE4", "", {
		{ BITS_EOT }
	}},
	{0x209, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK4", "", {
		{ BITS_EOT }
	}},
	{0x20a, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE5", "", {
		{ BITS_EOT }
	}},
	{0x20b, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK5", "", {
		{ BITS_EOT }
	}},
	{0x20c, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE6", "", {
		{ BITS_EOT }
	}},
	{0x20d, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK6", "", {
		{ BITS_EOT }
	}},
	{0x20e, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSBASE7", "", {
		{ BITS_EOT }
	}},
	{0x20f, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_PHYSMASK7", "", {
		{ BITS_EOT }
	}},
	{0x250, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX64K_00000", "", {
		{ BITS_EOT }
	}},
	{0x258, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX16K_80000", "", {
		{ BITS_EOT }
	}},
	{0x259, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX16K_A0000", "", {
		{ BITS_EOT }
	}},
	{0x268, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_C0000", "", {
		{ BITS_EOT }
	}},
	{0x269, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_C8000", "", {
		{ BITS_EOT }
	}},
	{0x26a, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_D0000", "", {
		{ BITS_EOT }
	}},
	{0x26b, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_D8000", "", {
		{ BITS_EOT }
	}},
	{0x26c, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_E0000", "", {
		{ BITS_EOT }
	}},
	{0x26d, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_E8000", "", {
		{ BITS_EOT }
	}},
	{0x26e, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_F0000", "", {
		{ BITS_EOT }
	}},
	{0x26f, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_FIX4K_F8000", "", {
		{ BITS_EOT }
	}},
	{0x2ff, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRR_DEF_TYPE", "", {
		{ BITS_EOT }
	}},
	{0x300, MSRTYPE_RDWR, MSR2(0,0), "MSR_BPU_COUNTER0", "", {
		{ BITS_EOT }
	}},
	{0x301, MSRTYPE_RDWR, MSR2(0,0), "MSR_BPU_COUNTER1", "", {
		{ BITS_EOT }
	}},
	{0x302, MSRTYPE_RDWR, MSR2(0,0), "MSR_BPU_COUNTER2", "", {
		{ BITS_EOT }
	}},
	{0x303, MSRTYPE_RDWR, MSR2(0,0), "MSR_BPU_COUNTER3", "", {
		{ BITS_EOT }
	}},
	{0x400, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC0_CTL", "", {
		{ BITS_EOT }
	}},
	{0x401, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC0_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x402, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC0_ADDR", "", {
		{ BITS_EOT }
	}},
	{0x403, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC0_MISC", "", {
		{ BITS_EOT }
	}},
	{0x404, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC1_CTL", "", {
		{ BITS_EOT }
	}},
	{0x405, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC1_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x406, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC1_ADDR", "", {
		{ BITS_EOT }
	}},
	{0x407, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC1_MISC", "", {
		{ BITS_EOT }
	}},
	{0x408, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC2_CTL", "", {
		{ BITS_EOT }
	}},
	{0x409, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC2_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x40a, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC2_ADDR", "", {
		{ BITS_EOT }
	}},
	{0x40b, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC2_MISC", "", {
		{ BITS_EOT }
	}},
	{0x40c, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC3_CTL", "", {
		{ BITS_EOT }
	}},
	{0x40d, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC3_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x40e, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC3_ADDR", "", {
		{ BITS_EOT }
	}},
	{0x40f, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC3_MISC", "", {
		{ BITS_EOT }
	}},
	{0x410, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC4_CTL", "", {
		{ BITS_EOT }
	}},
	{0x411, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC4_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x412, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC4_ADDR", "", {
		{ BITS_EOT }
	}},
	{0x413, MSRTYPE_RDWR, MSR2(0,0), "IA32_MC4_MISC", "", {
		{ BITS_EOT }
	}},
	{0x10, MSRTYPE_RDWR, MSR2(0,0), "IA32_TIME_STAMP_COUNTER", "", {
		{ BITS_EOT }
	}},
	{0x1b, MSRTYPE_RDWR, MSR2(0,0), "IA32_APIC_BASE", "", {
		{ BITS_EOT }
	}},
	{0x3a, MSRTYPE_RDWR, MSR2(0,0), "IA32_FEATURE_CONTROL", "", {
		{ BITS_EOT }
	}},
	{0x8b, MSRTYPE_RDWR, MSR2(0,0), "IA32_BIOS_SIGN_ID", "", {
		{ BITS_EOT }
	}},
	{0x9b, MSRTYPE_RDWR, MSR2(0,0), "IA32_SMM_MONITOR_CTL", "", {
		{ BITS_EOT }
	}},
	{0xfe, MSRTYPE_RDWR, MSR2(0,0), "IA32_MTRRCAP", "", {
		{ BITS_EOT }
	}},
	{0x174, MSRTYPE_RDWR, MSR2(0,0), "IA32_SYSENTER_CS", "", {
		{ BITS_EOT }
	}},
	{0x175, MSRTYPE_RDWR, MSR2(0,0), "IA32_SYSENTER_ESP", "", {
		{ BITS_EOT }
	}},
	{0x176, MSRTYPE_RDWR, MSR2(0,0), "IA32_SYSENTER_EIP", "", {
		{ BITS_EOT }
	}},
	{0x179, MSRTYPE_RDWR, MSR2(0,0), "IA32_MCG_CAP", "", {
		{ BITS_EOT }
	}},
	{0x17a, MSRTYPE_RDWR, MSR2(0,0), "IA32_MCG_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x180, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RAX", "", {
		{ BITS_EOT }
	}},
	{0x181, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RBX", "", {
		{ BITS_EOT }
	}},
	{0x182, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RCX", "", {
		{ BITS_EOT }
	}},
	{0x183, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RDX", "", {
		{ BITS_EOT }
	}},
	{0x184, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RSI", "", {
		{ BITS_EOT }
	}},
	{0x185, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RDI", "", {
		{ BITS_EOT }
	}},
	{0x186, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RBP", "", {
		{ BITS_EOT }
	}},
	{0x187, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RSP", "", {
		{ BITS_EOT }
	}},
	{0x188, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RFLAGS", "", {
		{ BITS_EOT }
	}},
	{0x189, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_RIP", "", {
		{ BITS_EOT }
	}},
	{0x18a, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_MISC", "", {
		{ BITS_EOT }
	}},
	{0x190, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R8", "", {
		{ BITS_EOT }
	}},
	{0x191, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R9", "", {
		{ BITS_EOT }
	}},
	{0x192, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R10", "", {
		{ BITS_EOT }
	}},
	{0x193, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R11", "", {
		{ BITS_EOT }
	}},
	{0x194, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R12", "", {
		{ BITS_EOT }
	}},
	{0x195, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R13", "", {
		{ BITS_EOT }
	}},
	{0x196, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R14", "", {
		{ BITS_EOT }
	}},
	{0x197, MSRTYPE_RDWR, MSR2(0,0), "MSR_MCG_R15", "", {
		{ BITS_EOT }
	}},
	{0x198, MSRTYPE_RDWR, MSR2(0,0), "IA32_PERF_STATUS", "", {
		{ BITS_EOT }
	}},
	{0x199, MSRTYPE_RDWR, MSR2(0,0), "IA32_PERF_CTL", "", {
		{ BITS_EOT }
	}},
	{0x19a, MSRTYPE_RDWR, MSR2(0,0), "IA32_CLOCK_MODULATION", "", {
		{ BITS_EOT }
	}},
	{0x19b, MSRTYPE_RDWR, MSR2(0,0), "IA32_THERM_INTERRUPT", "", {
		{ BITS_EOT }
	}},
	{0x1a0, MSRTYPE_RDWR, MSR2(0,0), "IA32_MISC_ENABLE", "", {
		{ BITS_EOT }
	}},
	{0x1d7, MSRTYPE_RDWR, MSR2(0,0), "MSR_LER_FROM_LIP", "", {
		{ BITS_EOT }
	}},
	{0x1d8, MSRTYPE_RDWR, MSR2(0,0), "MSR_LER_TO_LIP", "", {
		{ BITS_EOT }
	}},
	{0x1d9, MSRTYPE_RDWR, MSR2(0,0), "MSR_DEBUGCTLA", "", {
		{ BITS_EOT }
	}},
	{0x1da, MSRTYPE_RDWR, MSR2(0,0), "MSR_LASTBRANCH_TOS", "", {
		{ BITS_EOT }
	}},
	{0x277, MSRTYPE_RDWR, MSR2(0,0), "IA32_PAT", "", {
		{ BITS_EOT }
	}},
	{0x600, MSRTYPE_RDWR, MSR2(0,0), "IA32_DS_AREA", "", {
		{ BITS_EOT }
	}},
	{ MSR_EOT }
};

