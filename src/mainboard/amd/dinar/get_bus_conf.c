/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 Advanced Micro Devices, Inc.
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
#include <device/pci.h>
#include <device/pci_ids.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpu/amd/amdfam15.h>
#include "agesawrapper.h"
#if CONFIG_AMD_SB_CIMX
#include <sb_cimx.h>
#endif


/* Global variables for MB layouts and these will be shared by irqtable mptable
 * and acpi_tables busnum is default.
 */
u8 bus_isa;
u8 bus_sb700[2];
u8 bus_rd890[14];

/*
 * Here you only need to set value in pci1234 for HT-IO that could be installed or not
 * You may need to preset pci1234 for HTIO board,
 * please refer to src/northbridge/amd/amdk8/get_sblk_pci1234.c for detail
 */
u32 pci1234x[] = {
	0x0000ff0,
};

/*
 * HT Chain device num, actually it is unit id base of every ht device in chain,
 * assume every chain only have 4 ht device at most
 */
u32 hcdnx[] = {
	0x20202020,
};

u32 bus_type[256];

u32 sbdn_sb700;
u32 sbdn_rd890;

static u32 get_bus_conf_done = 0;




void get_bus_conf(void)
{
	u32 status;

	device_t dev;
	int i, j;

	if (get_bus_conf_done == 1)
		return;   /* do it only once */

	get_bus_conf_done = 1;

	printk(BIOS_DEBUG, "Mainboard - Get_bus_conf.c - get_bus_conf - Start.\n");
	/*
	 * This is the call to AmdInitLate.  It is really in the wrong place, conceptually,
	 * but functionally within the coreboot model, this is the best place to make the
	 * call.  The logically correct place to call AmdInitLate is after PCI scan is done,
	 * after the decision about S3 resume is made, and before the system tables are
	 * written into RAM.  The routine that is responsible for writing the tables is
	 * "write_tables", called near the end of "hardwaremain".  There is no platform
	 * specific entry point between the S3 resume decision point and the call to
	 * "write_tables", and the next platform specific entry points are the calls to
	 * the ACPI table write functions.  The first of ose would seem to be the right
	 * place, but other table write functions, e.g. the PIRQ table write function, are
	 * called before the ACPI tables are written.  This routine is called at the beginning
	 * of each of the write functions called prior to the ACPI write functions, so this
	 * becomes the best place for this call.
	 */
	status = agesawrapper_amdinitlate();
	if(status) {
		printk(BIOS_DEBUG, "agesawrapper_amdinitlate failed: %x \n", status);
	}
	printk(BIOS_DEBUG, "Got past agesawrapper_amdinitlate\n");

	sbdn_sb700 = 0;

	for (i = 0; i < ARRAY_SIZE(bus_sb700); i++) {
		bus_sb700[i] = 0;
	}
	for (i = 0; i < ARRAY_SIZE(bus_rd890); i++) {
		bus_rd890[i] = 0;
	}

	for (i = 0; i < 256; i++) {
		bus_type[i] = 0; /* default ISA bus. */
	}


	bus_type[0] = 1;  /* pci */

	bus_rd890[0] = (pci1234x[0] >> 16) & 0xff;
	bus_sb700[0] = bus_rd890[0];

	/* sb700 */
	dev = dev_find_slot(bus_sb700[0], PCI_DEVFN(sbdn_sb700 + 0x14, 4));



	if (dev) {
		bus_sb700[1] = pci_read_config8(dev, PCI_SECONDARY_BUS);

		bus_isa = pci_read_config8(dev, PCI_SUBORDINATE_BUS);
		bus_isa++;
		for (j = bus_sb700[1]; j < bus_isa; j++)
			bus_type[j] = 1;
	}

	/* rd890 */
	for (i = 1; i < ARRAY_SIZE(bus_rd890); i++) {
		dev = dev_find_slot(bus_rd890[0], PCI_DEVFN(sbdn_rd890 + i, 0));
		if (dev) {
			bus_rd890[i] = pci_read_config8(dev, PCI_SECONDARY_BUS);
			if(255 != bus_rd890[i]) {
				bus_isa = pci_read_config8(dev, PCI_SUBORDINATE_BUS);
				bus_isa++;
				bus_type[bus_rd890[i]] = 1; /* PCI bus. */
			}
		}
	}


	/* I/O APICs:   APIC ID Version State   Address */
	bus_isa = 10;

#if CONFIG_AMD_SB_CIMX
//	sb_After_Pci_Init();
//	sb_Late_Post();
#endif
	printk(BIOS_DEBUG, "Mainboard - Get_bus_conf.c - get_bus_conf - End.\n");
}
