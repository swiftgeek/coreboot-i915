/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 - 2012 Advanced Micro Devices, Inc.
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
#include <string.h>
#include <arch/acpi.h>
#include <arch/acpigen.h>
#include <arch/ioapic.h>
#include <arch/io.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/mtrr.h>
#include <cpu/amd/amdfam10_sysconf.h>
#include "agesawrapper.h"

#define DUMP_ACPI_TABLES 0

#if DUMP_ACPI_TABLES == 1
static void dump_mem(u32 start, u32 end)
{

	u32 i;
	print_debug("dump_mem:");
	for (i = start; i < end; i++) {
		if ((i & 0xf) == 0) {
			printk(BIOS_DEBUG, "\n%08x:", i);
		}
		printk(BIOS_DEBUG, " %02x", (u8)*((u8 *)i));
	}
	print_debug("\n");
}
#endif

extern const unsigned char AmlCode[];


unsigned long acpi_fill_mcfg(unsigned long current)
{
	/* Just a dummy */
	return current;
}

unsigned long acpi_fill_madt(unsigned long current)
{
	device_t dev;
	u32 dword;
	u32 gsi_base = 0;
	u32 apicid_sp5100;
	u32 apicid_sr5650;

	/*
	 * AGESA v5 Apply apic enumeration rules
	 * For systems with >= 16 APICs, put the IO-APICs at 0..n and
	 * put the local-APICs at m..z
	 * For systems with < 16 APICs, put the Local-APICs at 0..n and
	 * put the IO-APICs at (n + 1)..z
	 */
#if CONFIG_MAX_CPUS >= 16
	apicid_sp5100 = 0x0;
#else
	apicid_sp5100 = CONFIG_MAX_CPUS + 1
#endif
		apicid_sr5650 = apicid_sp5100 + 1;

	/* create all subtables for processors */
	current = acpi_create_madt_lapics(current);

	/* Write sp5100 IOAPIC, only one */
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current,
			apicid_sp5100,
			IO_APIC_ADDR,
			0
			);

	/* IOAPIC on rs5690 */
	gsi_base += IO_APIC_INTERRUPTS;  /* SP5100 has 24 IOAPIC entries. */
	dev = dev_find_slot(0, PCI_DEVFN(0, 0));
	if (dev) {
		pci_write_config32(dev, 0xF8, 0x1);
		dword = pci_read_config32(dev, 0xFC) & 0xfffffff0;
		current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current,
				apicid_sr5650,
				dword,
				gsi_base
				);
	}

	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *) current,
			0, //BUS
			0, //SOURCE
			2, //gsirq
			0  //flags
			);

	/* 0: mean bus 0--->ISA */
	/* 0: PIC 0 */
	/* 2: APIC 2 */
	/* 5 mean: 0101 --> Edige-triggered, Active high */

	/* create all subtables for processors */
	current += acpi_create_madt_lapic_nmi((acpi_madt_lapic_nmi_t *)current, 0, 5, 1);
	current += acpi_create_madt_lapic_nmi((acpi_madt_lapic_nmi_t *)current, 1, 5, 1);
	/* 1: LINT1 connect to NMI */

	return current;
}

unsigned long acpi_fill_slit(unsigned long current)
{
	// Not implemented
	return current;
}

unsigned long acpi_fill_srat(unsigned long current)
{
	/* No NUMA, no SRAT */
	return current;
}

unsigned long acpi_fill_ssdt_generator(unsigned long current, const char *oem_table_id)
{
	int lens;
	msr_t msr;
	char pscope[] = "\\_SB.PCI0";

	lens = acpigen_write_scope(pscope);
	msr = rdmsr(TOP_MEM);
	lens += acpigen_write_name_dword("TOM1", msr.lo);
	msr = rdmsr(TOP_MEM2);
	/*
	 * Since XP only implements parts of ACPI 2.0, we can't use a qword
	 * here.
	 * See http://www.acpi.info/presentations/S01USMOBS169_OS%2520new.ppt
	 * slide 22ff.
	 * Shift value right by 20 bit to make it fit into 32bit,
	 * giving us 1MB granularity and a limit of almost 4Exabyte of memory.
	 */
	lens += acpigen_write_name_dword("TOM2", (msr.hi << 12) | msr.lo >> 20);
	acpigen_patch_len(lens - 1);
	return (unsigned long) (acpigen_get_current());
}

unsigned long write_acpi_tables(unsigned long start)
{
	unsigned long current;
	acpi_rsdp_t *rsdp;
	acpi_rsdt_t *rsdt;
	//acpi_hpet_t *hpet;
	acpi_madt_t *madt;
	acpi_srat_t *srat;
	acpi_slit_t *slit;
	acpi_fadt_t *fadt;
	acpi_facs_t *facs;
	acpi_header_t *dsdt;
	acpi_header_t *ssdt;
	acpi_header_t *ssdt2;
	acpi_header_t *alib;

	get_bus_conf(); /* it will get sblk, pci1234, hcdn, and sbdn */

	/* Align ACPI tables to 16 bytes */
	start = ALIGN(start, 16);
	current = start;

	printk(BIOS_INFO, "ACPI: Writing ACPI tables at %lx...\n", start);

	/* We need at least an RSDP and an RSDT Table */
	rsdp = (acpi_rsdp_t *) current;
	current += sizeof(acpi_rsdp_t);
	rsdt = (acpi_rsdt_t *) current;
	current += sizeof(acpi_rsdt_t);

	/* clear all table memory */
	memset((void *)start, 0, current - start);

	acpi_write_rsdp(rsdp, rsdt, NULL);
	acpi_write_rsdt(rsdt);

	/* FACS */
	printk(BIOS_DEBUG, "ACPI:    * FACS\n");
	facs = (acpi_facs_t *) current;
	current += sizeof(acpi_facs_t);
	acpi_create_facs(facs);

	/* DSDT */
	printk(BIOS_DEBUG, "ACPI:    * DSDT\n");
	dsdt = (acpi_header_t *)current;
	memcpy(dsdt, &AmlCode, sizeof(acpi_header_t));
	current += dsdt->length;
	memcpy(dsdt, &AmlCode, dsdt->length);
	printk(BIOS_DEBUG, "ACPI:    * DSDT @ %p Length %x\n", dsdt, dsdt->length);
	/* FADT */
	printk(BIOS_DEBUG, "ACPI:    * FADT\n");
	fadt = (acpi_fadt_t *) current;
	current += sizeof(acpi_fadt_t);

	acpi_create_fadt(fadt, facs, dsdt);
	acpi_add_table(rsdp, fadt);

	/*
	 * We explicitly add these tables later on:
	 */
#ifdef UNUSED_CODE // Don't need HPET table. we have one in dsdt
	current   = ALIGN(current, 8);
	printk(BIOS_DEBUG, "ACPI:    * HPET at %lx\n", current);
	hpet = (acpi_hpet_t *) current;
	current += sizeof(acpi_hpet_t);
	acpi_create_hpet(hpet);
	acpi_add_table(rsdp, hpet);
#endif

	/* If we want to use HPET Timers Linux wants an MADT */
	current   = ALIGN(current, 8);
	printk(BIOS_DEBUG, "ACPI:    * MADT at %lx\n",current);
	madt = (acpi_madt_t *) current;
	acpi_create_madt(madt);
	current += madt->header.length;
	acpi_add_table(rsdp, madt);

	/* SRAT */
	current   = ALIGN(current, 8);
	printk(BIOS_DEBUG, "ACPI:    * SRAT at %lx\n", current);
	srat = (acpi_srat_t *) agesawrapper_getlateinitptr (PICK_SRAT);
	if (srat != NULL) {
		memcpy((void *)current, srat, srat->header.length);
		srat = (acpi_srat_t *) current;
		//acpi_create_srat(srat);
		current += srat->header.length;
		acpi_add_table(rsdp, srat);
	}

	/* SLIT */
	current   = ALIGN(current, 8);
	printk(BIOS_DEBUG, "ACPI:   * SLIT at %lx\n", current);
	slit = (acpi_slit_t *) agesawrapper_getlateinitptr (PICK_SLIT);
	if (slit != NULL) {
		memcpy((void *)current, slit, slit->header.length);
		slit = (acpi_slit_t *) current;
		//acpi_create_slit(slit);
		current += slit->header.length;
		acpi_add_table(rsdp, slit);
	}

	/* SSDT */
	current	 = ALIGN(current, 16);
	printk(BIOS_DEBUG, "ACPI:  * AGESA ALIB SSDT at %lx\n", current);
	alib = (acpi_header_t *)agesawrapper_getlateinitptr (PICK_ALIB);
	if (alib != NULL) {
		memcpy((void *)current, alib, alib->length);
		ssdt = (acpi_header_t *) current;
		current += alib->length;
		acpi_add_table(rsdp,alib);
	} else {
		printk(BIOS_DEBUG, "	AGESA ALIB SSDT table NULL. Skipping.\n");
	}

	/* The DSDT needs additional work for the AGESA SSDT Pstate table */
	current	 = ALIGN(current, 16);
	printk(BIOS_DEBUG, "ACPI:  * AGESA SSDT Pstate at %lx\n", current);
	ssdt = (acpi_header_t *)agesawrapper_getlateinitptr (PICK_PSTATE);
	if (ssdt != NULL) {
		memcpy((void *)current, ssdt, ssdt->length);
		ssdt = (acpi_header_t *) current;
		current += ssdt->length;
		acpi_add_table(rsdp,ssdt);
	} else {
		printk(BIOS_DEBUG, "  AGESA SSDT table NULL. Skipping.\n");
	}

	current	 = ALIGN(current, 16);
	printk(BIOS_DEBUG, "ACPI:  * coreboot TOM SSDT2 at %lx\n", current);
	ssdt2 = (acpi_header_t *) current;
	acpi_create_ssdt_generator(ssdt2, ACPI_TABLE_CREATOR);
	current += ssdt2->length;
	acpi_add_table(rsdp,ssdt2);

#if DUMP_ACPI_TABLES == 1
	printk(BIOS_DEBUG, "rsdp\n");
	dump_mem(rsdp, ((void *)rsdp) + sizeof(acpi_rsdp_t));

	printk(BIOS_DEBUG, "rsdt\n");
	dump_mem(rsdt, ((void *)rsdt) + sizeof(acpi_rsdt_t));

	printk(BIOS_DEBUG, "madt\n");
	dump_mem(madt, ((void *)madt) + madt->header.length);

	printk(BIOS_DEBUG, "srat\n");
	dump_mem(srat, ((void *)srat) + srat->header.length);

	printk(BIOS_DEBUG, "slit\n");
	dump_mem(slit, ((void *)slit) + slit->header.length);

	printk(BIOS_DEBUG, "ssdt\n");
	dump_mem(ssdt, ((void *)ssdt) + ssdt->length);

	printk(BIOS_DEBUG, "fadt\n");
	dump_mem(fadt, ((void *)fadt) + fadt->header.length);
#endif

	printk(BIOS_INFO, "ACPI: done.\n");
	return current;
}
