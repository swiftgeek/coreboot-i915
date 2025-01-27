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
#include <arch/io.h>
#include <stdint.h>
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/hypertransport.h>
#include <stdlib.h>
#include <string.h>
#include <cbmem.h>
#include <cpu/cpu.h>
#include <arch/acpi.h>
#include "i945.h"

static int get_pcie_bar(u32 *base, u32 *len)
{
	device_t dev;
	u32 pciexbar_reg;

	*base = 0;
	*len = 0;

	dev = dev_find_slot(0, PCI_DEVFN(0, 0));
	if (!dev)
		return 0;

	pciexbar_reg = pci_read_config32(dev, PCIEXBAR);

	if (!(pciexbar_reg & (1 << 0)))
		return 0;

	switch ((pciexbar_reg >> 1) & 3) {
	case 0: // 256MB
		*base = pciexbar_reg & ((1 << 31)|(1 << 30)|(1 << 29)|(1 << 28));
		*len = 256 * 1024 * 1024;
		return 1;
	case 1: // 128M
		*base = pciexbar_reg & ((1 << 31)|(1 << 30)|(1 << 29)|(1 << 28)|(1 << 27));
		*len = 128 * 1024 * 1024;
		return 1;
	case 2: // 64M
		*base = pciexbar_reg & ((1 << 31)|(1 << 30)|(1 << 29)|(1 << 28)|(1 << 27)|(1 << 26));
		*len = 64 * 1024 * 1024;
		return 1;
	}

	return 0;
}

static void add_fixed_resources(struct device *dev, int index)
{
	struct resource *resource;
	u32 pcie_config_base, pcie_config_size;

	if (get_pcie_bar(&pcie_config_base, &pcie_config_size)) {
		printk(BIOS_DEBUG, "Adding PCIe config bar\n");
		resource = new_resource(dev, index++);
		resource->base = (resource_t) pcie_config_base;
		resource->size = (resource_t) pcie_config_size;
		resource->flags = IORESOURCE_MEM | IORESOURCE_RESERVE |
		    IORESOURCE_FIXED | IORESOURCE_STORED | IORESOURCE_ASSIGNED;
	}
}

static void pci_domain_set_resources(device_t dev)
{
	uint32_t pci_tolm;
	uint8_t tolud, reg8;
	uint16_t reg16;
	unsigned long long tomk, tomk_stolen;
	uint64_t tseg_memory_base = 0, tseg_memory_size = 0;

	/* Can we find out how much memory we can use at most
	 * this way?
	 */
	pci_tolm = find_pci_tolm(dev->link_list);
	printk(BIOS_DEBUG, "pci_tolm: 0x%x\n", pci_tolm);

	printk(BIOS_SPEW, "Base of stolen memory: 0x%08x\n",
		    pci_read_config32(dev_find_slot(0, PCI_DEVFN(2, 0)), 0x5c));

	tolud = pci_read_config8(dev_find_slot(0, PCI_DEVFN(0, 0)), 0x9c);
	printk(BIOS_SPEW, "Top of Low Used DRAM: 0x%08x\n", tolud << 24);

	tomk = tolud << 14;
	tomk_stolen = tomk;

	/* Note: subtract IGD device and TSEG */
	reg8 = pci_read_config8(dev_find_slot(0, PCI_DEVFN(0, 0)), 0x9e);
	if (reg8 & 1) {
		int tseg_size = 0;
		printk(BIOS_DEBUG, "TSEG decoded, subtracting ");
		reg8 >>= 1;
		reg8 &= 3;
		switch (reg8) {
		case 0:
			tseg_size = 1024;
			break;	/* TSEG = 1M */
		case 1:
			tseg_size = 2048;
			break;	/* TSEG = 2M */
		case 2:
			tseg_size = 8192;
			break;	/* TSEG = 8M */
		}

		printk(BIOS_DEBUG, "%dM\n", tseg_size >> 10);
		tomk_stolen -= tseg_size;

		/* For reserving TSEG memory in the memory map */
		tseg_memory_base = tomk_stolen * 1024ULL;
		tseg_memory_size = tseg_size * 1024ULL;
	}

	reg16 = pci_read_config16(dev_find_slot(0, PCI_DEVFN(0, 0)), GGC);
	if (!(reg16 & 2)) {
		int uma_size = 0;
		printk(BIOS_DEBUG, "IGD decoded, subtracting ");
		reg16 >>= 4;
		reg16 &= 7;
		switch (reg16) {
		case 1:
			uma_size = 1024;
			break;
		case 3:
			uma_size = 8192;
			break;
		}

		printk(BIOS_DEBUG, "%dM UMA\n", uma_size >> 10);
		tomk_stolen -= uma_size;

		/* For reserving UMA memory in the memory map */
		uma_memory_base = tomk_stolen * 1024ULL;
		uma_memory_size = uma_size * 1024ULL;
	}

	/* The following needs to be 2 lines, otherwise the second
	 * number is always 0
	 */
	printk(BIOS_INFO, "Available memory: %dK", (uint32_t)tomk_stolen);
	printk(BIOS_INFO, " (%dM)\n", (uint32_t)(tomk_stolen >> 10));

	/* Report the memory regions */
	ram_resource(dev, 3, 0, 640);
	ram_resource(dev, 4, 768, (tomk - 768));
	uma_resource(dev, 5, uma_memory_base >> 10, uma_memory_size >> 10);
	mmio_resource(dev, 6, tseg_memory_base >> 10, tseg_memory_size >> 10);

	add_fixed_resources(dev, 7);

	assign_resources(dev->link_list);

	/* Leave some space for ACPI, PIRQ and MP tables */
	high_tables_base = (tomk_stolen * 1024) - HIGH_MEMORY_SIZE;
	high_tables_size = HIGH_MEMORY_SIZE;
}

	/* TODO We could determine how many PCIe busses we need in
	 * the bar. For now that number is hardcoded to a max of 64.
	 * See e7525/northbridge.c for an example.
	 */
static struct device_operations pci_domain_ops = {
	.read_resources   = pci_domain_read_resources,
	.set_resources    = pci_domain_set_resources,
	.enable_resources = NULL,
	.init             = NULL,
	.scan_bus         = pci_domain_scan_bus,
#if CONFIG_MMCONF_SUPPORT_DEFAULT
	.ops_pci_bus	  = &pci_ops_mmconf,
#else
	.ops_pci_bus	  = &pci_cf8_conf1,
#endif
};

static void mc_read_resources(device_t dev)
{
	struct resource *resource;

	pci_dev_read_resources(dev);

	/* So, this is one of the big mysteries in the coreboot resource
	 * allocator. This resource should make sure that the address space
	 * of the PCIe memory mapped config space bar. But it does not.
	 */

	/* We use 0xcf as an unused index for our PCIe bar so that we find it again */
	resource = new_resource(dev, 0xcf);
	resource->base = DEFAULT_PCIEXBAR;
	resource->size = 64 * 1024 * 1024;	/* 64MB hard coded PCIe config space */
	resource->flags =
	    IORESOURCE_MEM | IORESOURCE_FIXED | IORESOURCE_STORED |
	    IORESOURCE_ASSIGNED;
	printk(BIOS_DEBUG, "Adding PCIe enhanced config space BAR 0x%08lx-0x%08lx.\n",
		     (unsigned long)(resource->base), (unsigned long)(resource->base + resource->size));
}

static void mc_set_resources(device_t dev)
{
	struct resource *resource;

	/* Report the PCIe BAR */
	resource = find_resource(dev, 0xcf);
	if (resource) {
		report_resource_stored(dev, resource, "<mmconfig>");
	}

	/* And call the normal set_resources */
	pci_dev_set_resources(dev);
}

static void intel_set_subsystem(device_t dev, unsigned vendor, unsigned device)
{
	if (!vendor || !device) {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				pci_read_config32(dev, PCI_VENDOR_ID));
	} else {
		pci_write_config32(dev, PCI_SUBSYSTEM_VENDOR_ID,
				((device & 0xffff) << 16) | (vendor & 0xffff));
	}
}

#if CONFIG_HAVE_ACPI_RESUME
static void northbridge_init(struct device *dev)
{
	switch (pci_read_config32(dev, SKPAD)) {
	case SKPAD_NORMAL_BOOT_MAGIC:
		printk(BIOS_DEBUG, "Normal boot.\n");
		acpi_slp_type=0;
		break;
	case SKPAD_ACPI_S3_MAGIC:
		printk(BIOS_DEBUG, "S3 Resume.\n");
		acpi_slp_type=3;
		break;
	default:
		printk(BIOS_DEBUG, "Unknown boot method, assuming normal.\n");
		acpi_slp_type=0;
		break;
	}
}
#endif

static struct pci_operations intel_pci_ops = {
	.set_subsystem    = intel_set_subsystem,
};

static struct device_operations mc_ops = {
	.read_resources   = mc_read_resources,
	.set_resources    = mc_set_resources,
	.enable_resources = pci_dev_enable_resources,
#if CONFIG_HAVE_ACPI_RESUME
	.init             = northbridge_init,
#endif
	.scan_bus         = 0,
	.ops_pci          = &intel_pci_ops,
};

static const struct pci_driver mc_driver __pci_driver = {
	.ops    = &mc_ops,
	.vendor = PCI_VENDOR_ID_INTEL,
	.device = 0x27a0,
};

static void cpu_bus_init(device_t dev)
{
	initialize_cpus(dev->link_list);
}

static void cpu_bus_noop(device_t dev)
{
}

static struct device_operations cpu_bus_ops = {
	.read_resources   = cpu_bus_noop,
	.set_resources    = cpu_bus_noop,
	.enable_resources = cpu_bus_noop,
	.init             = cpu_bus_init,
	.scan_bus         = 0,
};

static void enable_dev(device_t dev)
{
	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	} else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	}
}

struct chip_operations northbridge_intel_i945_ops = {
	CHIP_NAME("Intel i945 Northbridge")
	.enable_dev = enable_dev,
};
