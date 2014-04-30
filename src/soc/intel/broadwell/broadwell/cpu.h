/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2014 Google Inc.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _BROADWELL_CPU_H_
#define _BROADWELL_CPU_H_

#include <arch/cpu.h>
#include <device/device.h>

/* CPU types */
#define HASWELL_FAMILY_ULT	0x40650
#define BROADWELL_FAMILY_ULT	0x306d0

/* Supported CPUIDs */
#define CPUID_HASWELL_A0	0x306c1
#define CPUID_HASWELL_B0	0x306c2
#define CPUID_HASWELL_C0	0x306c3
#define CPUID_HASWELL_ULT_B0	0x40650
#define CPUID_HASWELL_ULT	0x40651
#define CPUID_HASWELL_HALO	0x40661
#define CPUID_BROADWELL_C0	0x306d2
#define CPUID_BROADWELL_D0	0x306d3

/* CPU bus clock is fixed at 100MHz */
#define CPU_BCLK		100

/* Latency times in units of 1024ns. */
#define C_STATE_LATENCY_CONTROL_0_LIMIT 0x42
#define C_STATE_LATENCY_CONTROL_1_LIMIT 0x73
#define C_STATE_LATENCY_CONTROL_2_LIMIT 0x91
#define C_STATE_LATENCY_CONTROL_3_LIMIT 0xe4
#define C_STATE_LATENCY_CONTROL_4_LIMIT 0x145
#define C_STATE_LATENCY_CONTROL_5_LIMIT 0x1ef

#define C_STATE_LATENCY_MICRO_SECONDS(limit, base) \
	(((1 << ((base)*5)) * (limit)) / 1000)
#define C_STATE_LATENCY_FROM_LAT_REG(reg) \
	C_STATE_LATENCY_MICRO_SECONDS(C_STATE_LATENCY_CONTROL_ ##reg## _LIMIT, \
	                              (IRTL_1024_NS >> 10))

/* Configure power limits for turbo mode */
void set_power_limits(u8 power_limit_1_time);
int cpu_config_tdp_levels(void);

/*
 * Determine if HyperThreading is disabled.
 * The variable is not valid until setup_ap_init() has been called.
 */
extern int ht_disabled;

/* CPU identification */
u32 cpu_family_model(void);
u32 cpu_stepping(void);
int cpu_is_ult(void);

#endif
