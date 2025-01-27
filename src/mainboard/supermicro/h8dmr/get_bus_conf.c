/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007 AMD
 * Written by Yinghai Lu <yinghailu@amd.com> for AMD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#if CONFIG_LOGICAL_CPUS
#include <cpu/amd/multicore.h>
#endif

#include <cpu/amd/amdk8_sysconf.h>
#include <stdlib.h>


// Global variables for MB layouts and these will be shared by irqtable mptable and acpi_tables
//busnum is default
        unsigned char bus_mcp55[8]; //1
        unsigned apicid_mcp55;

	unsigned char bus_pcix[3]; // under bus_mcp55_2

unsigned pci1234x[] =
{        //Here you only need to set value in pci1234 for HT-IO that could be installed or not
	 //You may need to preset pci1234 for HTIO board, please refer to src/northbridge/amd/amdk8/get_sblk_pci1234.c for detail
        0x0000ff0,
        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0,
//        0x0000ff0
};
unsigned hcdnx[] =
{ //HT Chain device num, actually it is unit id base of every ht device in chain, assume every chain only have 4 ht device at most
	0x20202020,
	0x20202020,
//        0x20202020,
//        0x20202020,
//        0x20202020,
//        0x20202020,
//        0x20202020,
//        0x20202020,
};
unsigned sbdnb;



static unsigned get_bus_conf_done = 0;

void get_bus_conf(void)
{

	unsigned apicid_base;
	unsigned sbdn;

        device_t dev;
        int i;

        if(get_bus_conf_done==1) return; //do it only once

        get_bus_conf_done = 1;

        sysconf.hc_possible_num = ARRAY_SIZE(pci1234x);
        for(i=0;i<sysconf.hc_possible_num; i++) {
                sysconf.pci1234[i] = pci1234x[i];
                sysconf.hcdn[i] = hcdnx[i];
        }

        get_sblk_pci1234();

	sysconf.sbdn = (sysconf.hcdn[0] & 0xff); // first byte of first chain
	sbdn = sysconf.sbdn;

	sbdnb = (sysconf.hcdn[1] & 0xff); // first byte of second chain

	for(i=0; i<8; i++) {
		bus_mcp55[i] = 0;
	}

	for(i=0; i<3; i++) {
		bus_pcix[i] = 0;
	}


	bus_mcp55[0] = (sysconf.pci1234[0] >> 16) & 0xff;

                /* MCP55 */
                dev = dev_find_slot(bus_mcp55[0], PCI_DEVFN(sbdn + 0x06,0));
                if (dev) {
                        bus_mcp55[1] = pci_read_config8(dev, PCI_SECONDARY_BUS);
                        bus_mcp55[2] = pci_read_config8(dev, PCI_SUBORDINATE_BUS);
                        bus_mcp55[2]++;
                }
                else {
                        printk(BIOS_DEBUG, "ERROR - could not find PCI 1:%02x.0, using defaults\n", sbdn + 0x06);

                        bus_mcp55[1] = 2;
                        bus_mcp55[2] = 3;
                }

		for(i=2; i<8;i++) {
	                dev = dev_find_slot(bus_mcp55[0], PCI_DEVFN(sbdn + 0x0a + i - 2 , 0));
        	        if (dev) {
                	        bus_mcp55[i] = pci_read_config8(dev, PCI_SECONDARY_BUS);
	                }
        	        else {
                	        printk(BIOS_DEBUG, "ERROR - could not find PCI %02x:%02x.0, using defaults\n", bus_mcp55[0], sbdn + 0x0a + i - 2 );
        	        }
		}

		if(bus_mcp55[2]) {
			for(i=0;i<2; i++) {
	                        dev = dev_find_slot(bus_mcp55[2], PCI_DEVFN(0, i));
        	                if(dev) {
                	                bus_pcix[0] = bus_mcp55[2];
                        	        bus_pcix[i+1] = pci_read_config8(dev, PCI_SECONDARY_BUS);
	                        }
			}
		}


/*I/O APICs:	APIC ID	Version	State		Address*/
#if CONFIG_LOGICAL_CPUS
	apicid_base = get_apicid_base(1);
#else
	apicid_base = CONFIG_MAX_PHYSICAL_CPUS;
#endif
	apicid_mcp55 = apicid_base+0;

}
