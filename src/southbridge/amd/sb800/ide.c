/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2010 Advanced Micro Devices, Inc.
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
#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <device/pci_ops.h>
#include "sb800.h"

static void ide_init(struct device *dev)
{
	struct southbridge_amd_sb800_config *conf;
	/* Enable ide devices so the linux ide driver will work */
	u32 dword;
	u8 byte;

	conf = dev->chip_info;

	/* RPR9.1 disable MSI */
	/* TODO: For A14, it should set as 1. I doubt it. */
	dword = pci_read_config32(dev, 0x70);
	dword &= ~(1 << 16);
	pci_write_config32(dev, 0x70, dword);

	/* Ultra DMA mode */
	/* enable UDMA */
	byte = pci_read_config8(dev, 0x54);
	byte |= 1 << 0;
	pci_write_config8(dev, 0x54, byte);

	/* Enable I/O Access&& Bus Master */
	dword = pci_read_config16(dev, 0x4);
	dword |= 1 << 2;
	pci_write_config16(dev, 0x4, dword);

	/* set ide as primary, if you want to boot from IDE, you'd better set it
	 * in mainboard/Config.lb */
	if (conf->boot_switch_sata_ide == 1) {
		byte = pci_read_config8(dev, 0xAD);
		byte |= 1 << 4;
		pci_write_config8(dev, 0xAD, byte);
	}

#if CONFIG_PCI_ROM_RUN
	pci_dev_init(dev);
#endif
}

static struct pci_operations lops_pci = {
	.set_subsystem = pci_dev_set_subsystem,
};

static struct device_operations ide_ops = {
	.read_resources = pci_dev_read_resources,
	.set_resources = pci_dev_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init = ide_init,
	.scan_bus = 0,
	.ops_pci = &lops_pci,
};

static const struct pci_driver ide_driver __pci_driver = {
	.ops = &ide_ops,
	.vendor = PCI_VENDOR_ID_ATI,
	.device = PCI_DEVICE_ID_ATI_SB800_IDE,
};
