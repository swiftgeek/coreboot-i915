/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2007 Advanced Micro Devices, Inc.
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
#include <lib.h>
#include <cpu/cpu.h>

#include <cpu/x86/lapic.h>
#include <cpu/amd/mtrr.h>

#if CONFIG_LOGICAL_CPUS
#include <cpu/amd/multicore.h>
#include <pc80/mc146818rtc.h>
#endif

#include "northbridge.h"

#include "amdfam10.h"

#if CONFIG_HW_MEM_HOLE_SIZEK != 0
#include <cpu/amd/model_10xxx_rev.h>
#endif

#include <cpu/amd/amdfam10_sysconf.h>
#if CONFIG_AMD_SB_CIMX
#include <sb_cimx.h>
#endif

struct amdfam10_sysconf_t sysconf;

#define FX_DEVS NODE_NUMS
static device_t __f0_dev[FX_DEVS];
static device_t __f1_dev[FX_DEVS];
static device_t __f2_dev[FX_DEVS];
static device_t __f4_dev[FX_DEVS];
static unsigned fx_devs=0;

device_t get_node_pci(u32 nodeid, u32 fn)
{
#if NODE_NUMS + CONFIG_CDB >= 32
	if((CONFIG_CDB + nodeid) < 32) {
		return dev_find_slot(CONFIG_CBB, PCI_DEVFN(CONFIG_CDB + nodeid, fn));
	} else {
		return dev_find_slot(CONFIG_CBB-1, PCI_DEVFN(CONFIG_CDB + nodeid - 32, fn));
	}

#else
	return dev_find_slot(CONFIG_CBB, PCI_DEVFN(CONFIG_CDB + nodeid, fn));
#endif
}

static void get_fx_devs(void)
{
	int i;
	for(i = 0; i < FX_DEVS; i++) {
		__f0_dev[i] = get_node_pci(i, 0);
		__f1_dev[i] = get_node_pci(i, 1);
		__f2_dev[i] = get_node_pci(i, 2);
		__f4_dev[i] = get_node_pci(i, 4);
		if (__f0_dev[i] != NULL && __f1_dev[i] != NULL)
			fx_devs = i+1;
	}
	if (__f1_dev[0] == NULL || __f0_dev[0] == NULL || fx_devs == 0) {
		die("Cannot find 0:0x18.[0|1]\n");
	}
}

static u32 f1_read_config32(unsigned reg)
{
	if (fx_devs == 0)
		get_fx_devs();
	return pci_read_config32(__f1_dev[0], reg);
}

static void f1_write_config32(unsigned reg, u32 value)
{
	int i;
	if (fx_devs == 0)
		get_fx_devs();
	for(i = 0; i < fx_devs; i++) {
		device_t dev;
		dev = __f1_dev[i];
		if (dev && dev->enabled) {
			pci_write_config32(dev, reg, value);
		}
	}
}

static u32 amdfam10_nodeid(device_t dev)
{
#if NODE_NUMS == 64
	unsigned busn;
	busn = dev->bus->secondary;
	if(busn != CONFIG_CBB) {
		return (dev->path.pci.devfn >> 3) - CONFIG_CDB + 32;
	} else {
		return (dev->path.pci.devfn >> 3) - CONFIG_CDB;
	}

#else
	return (dev->path.pci.devfn >> 3) - CONFIG_CDB;
#endif
}

#include "conf.c"

static void set_vga_enable_reg(u32 nodeid, u32 linkn)
{
	u32 val;

	val =  1 | (nodeid<<4) | (linkn<<12);
	/* it will routing (1)mmio  0xa0000:0xbffff (2) io 0x3b0:0x3bb,
	 0x3c0:0x3df */
	f1_write_config32(0xf4, val);

}

static u32 amdfam10_scan_chain(device_t dev, u32 nodeid, struct bus *link, u32 link_num, u32 sblink,
				u32 max, u32 offset_unitid)
{
//	I want to put sb chain in bus 0 can I?


		u32 link_type;
		int i;
		u32 ht_c_index;
		u32 ht_unitid_base[4]; // here assume only 4 HT device on chain
		u32 max_bus;
		u32 min_bus;
		u32 is_sublink1 = (link_num>3);
		device_t devx;
		u32 busses;
		u32 segn = max>>8;
#if CONFIG_SB_HT_CHAIN_ON_BUS0 > 1
		u32 busn = max&0xff;
#endif
		u32 max_devfn;

#if CONFIG_HT3_SUPPORT
		if(is_sublink1) {
			u32 regpos;
			u32 reg;
			regpos = 0x170 + 4 * (link_num&3); // it is only on sublink0
			reg = pci_read_config32(dev, regpos);
			if(reg & 1) return max; // already ganged no sblink1
			devx = get_node_pci(nodeid, 4);
		} else
#endif
			devx = dev;


		link->cap = 0x80 + ((link_num&3) *0x20);
		do {
			link_type = pci_read_config32(devx, link->cap + 0x18);
		} while(link_type & ConnectionPending);
		if (!(link_type & LinkConnected)) {
			return max;
		}
		do {
			link_type = pci_read_config32(devx, link->cap + 0x18);
		} while(!(link_type & InitComplete));
		if (!(link_type & NonCoherent)) {
			return max;
		}
		/* See if there is an available configuration space mapping
		 * register in function 1.
		 */
		ht_c_index = get_ht_c_index(nodeid, link_num, &sysconf);

#if !CONFIG_EXT_CONF_SUPPORT
		if(ht_c_index>=4) return max;
#endif

		/* Set up the primary, secondary and subordinate bus numbers.
		 * We have no idea how many busses are behind this bridge yet,
		 * so we set the subordinate bus number to 0xff for the moment.
		 */
#if CONFIG_SB_HT_CHAIN_ON_BUS0 > 0
		// first chain will on bus 0
		if((nodeid == 0) && (sblink==link_num)) { // actually max is 0 here
			min_bus = max;
		}
	#if CONFIG_SB_HT_CHAIN_ON_BUS0 > 1
		// second chain will be on 0x40, third 0x80, forth 0xc0
		// i would refined that to  2, 3, 4 ==> 0, 0x, 40, 0x80, 0xc0
		//			    >4 will use	 more segments, We can have 16 segmment and every segment have 256 bus, For that case need the kernel support mmio pci config.
		else {
			min_bus = ((busn>>3) + 1) << 3; // one node can have 8 link and segn is the same
		}
		max = min_bus | (segn<<8);
	#else
		//other ...
		else {
			min_bus = ++max;
		}
	#endif
#else
		min_bus = ++max;
#endif
		max_bus = 0xfc | (segn<<8);

		link->secondary = min_bus;
		link->subordinate = max_bus;

		/* Read the existing primary/secondary/subordinate bus
		 * number configuration.
		 */
		busses = pci_read_config32(devx, link->cap + 0x14);

		/* Configure the bus numbers for this bridge: the configuration
		 * transactions will not be propagates by the bridge if it is
		 * not correctly configured
		 */
		busses &= 0xffff00ff;
		busses |= ((u32)(link->secondary) << 8);
		pci_write_config32(devx, link->cap + 0x14, busses);


		/* set the config map space */

		set_config_map_reg(nodeid, link_num, ht_c_index, link->secondary, link->subordinate, sysconf.segbit, sysconf.nodes);

		/* Now we can scan all of the subordinate busses i.e. the
		 * chain on the hypertranport link
		 */
		for(i=0;i<4;i++) {
			ht_unitid_base[i] = 0x20;
		}

		//if ext conf is enabled, only need use 0x1f
		if (min_bus == 0)
			max_devfn = (0x17<<3) | 7;
		else
			max_devfn = (0x1f<<3) | 7;

		max = hypertransport_scan_chain(link, 0, max_devfn, max, ht_unitid_base, offset_unitid);

		/* We know the number of busses behind this bridge.  Set the
		 * subordinate bus number to it's real value
		 */
		if(ht_c_index>3) { // clear the extend reg
			clear_config_map_reg(nodeid, link_num, ht_c_index, (max+1)>>sysconf.segbit, (link->subordinate)>>sysconf.segbit, sysconf.nodes);
		}

		link->subordinate = max;
		set_config_map_reg(nodeid, link_num, ht_c_index, link->secondary, link->subordinate, sysconf.segbit, sysconf.nodes);
		sysconf.ht_c_num++;

		{
			// use ht_unitid_base to update hcdn_reg
			u32 temp = 0;
			for(i=0;i<4;i++) {
				temp |= (ht_unitid_base[i] & 0xff) << (i*8);
			}

			sysconf.hcdn_reg[ht_c_index] = temp;

		}
	store_ht_c_conf_bus(nodeid, link_num, ht_c_index, link->secondary, link->subordinate, &sysconf);
	return max;
}

static unsigned amdfam10_scan_chains(device_t dev, unsigned max)
{
	unsigned nodeid;
	struct bus *link;
	unsigned sblink = sysconf.sblk;
	unsigned offset_unitid = 0;

	nodeid = amdfam10_nodeid(dev);

// Put sb chain in bus 0
#if CONFIG_SB_HT_CHAIN_ON_BUS0 > 0
	if(nodeid==0) {
	#if ((CONFIG_HT_CHAIN_UNITID_BASE != 1) || (CONFIG_HT_CHAIN_END_UNITID_BASE != 0x20))
		offset_unitid = 1;
	#endif
		for (link = dev->link_list; link; link = link->next)
			if (link->link_num == sblink)
				max = amdfam10_scan_chain(dev, nodeid, link, sblink, sblink, max, offset_unitid ); // do sb ht chain at first, in case s2885 put sb chain (8131/8111) on link2, but put 8151 on link0
	}
#endif

#if CONFIG_PCI_BUS_SEGN_BITS
	max = check_segn(dev, max, sysconf.nodes, &sysconf);
#endif

	for(link = dev->link_list; link; link = link->next) {
#if CONFIG_SB_HT_CHAIN_ON_BUS0 > 0
		if( (nodeid == 0) && (sblink == link->link_num) ) continue; //already done
#endif
		offset_unitid = 0;
		#if ((CONFIG_HT_CHAIN_UNITID_BASE != 1) || (CONFIG_HT_CHAIN_END_UNITID_BASE != 0x20))
			#if CONFIG_SB_HT_CHAIN_UNITID_OFFSET_ONLY
			if((nodeid == 0) && (sblink == link->link_num))
			#endif
				offset_unitid = 1;
		#endif

		max = amdfam10_scan_chain(dev, nodeid, link, link->link_num, sblink, max, offset_unitid);
	}
	return max;
}


static int reg_useable(unsigned reg, device_t goal_dev, unsigned goal_nodeid,
			unsigned goal_link)
{
	struct resource *res;
	unsigned nodeid, link = 0;
	int result;
	res = 0;
	for(nodeid = 0; !res && (nodeid < fx_devs); nodeid++) {
		device_t dev;
		dev = __f0_dev[nodeid];
		if (!dev)
			continue;
		for(link = 0; !res && (link < 8); link++) {
			res = probe_resource(dev, IOINDEX(0x1000 + reg, link));
		}
	}
	result = 2;
	if (res) {
		result = 0;
		if (	(goal_link == (link - 1)) &&
			(goal_nodeid == (nodeid - 1)) &&
			(res->flags <= 1)) {
			result = 1;
		}
	}
	return result;
}

static struct resource *amdfam10_find_iopair(device_t dev, unsigned nodeid, unsigned link)
{
	struct resource *resource;
	u32 free_reg, reg;
	resource = 0;
	free_reg = 0;
	for(reg = 0xc0; reg <= 0xd8; reg += 0x8) {
		int result;
		result = reg_useable(reg, dev, nodeid, link);
		if (result == 1) {
			/* I have been allocated this one */
			break;
		}
		else if (result > 1) {
			/* I have a free register pair */
			free_reg = reg;
		}
	}
	if (reg > 0xd8) {
		reg = free_reg; // if no free, the free_reg still be 0
	}

	//Ext conf space
	if(!reg) {
		//because of Extend conf space, we will never run out of reg, but we need one index to differ them. so same node and same link can have multi range
		u32 index = get_io_addr_index(nodeid, link);
		reg = 0x110+ (index<<24) + (4<<20); // index could be 0, 255
	}

		resource = new_resource(dev, IOINDEX(0x1000 + reg, link));

	return resource;
}

static struct resource *amdfam10_find_mempair(device_t dev, u32 nodeid, u32 link)
{
	struct resource *resource;
	u32 free_reg, reg;
	resource = 0;
	free_reg = 0;
	for(reg = 0x80; reg <= 0xb8; reg += 0x8) {
		int result;
		result = reg_useable(reg, dev, nodeid, link);
		if (result == 1) {
			/* I have been allocated this one */
			break;
		}
		else if (result > 1) {
			/* I have a free register pair */
			free_reg = reg;
		}
	}
	if (reg > 0xb8) {
		reg = free_reg;
	}

	//Ext conf space
	if(!reg) {
		//because of Extend conf space, we will never run out of reg,
		// but we need one index to differ them. so same node and
		// same link can have multi range
		u32 index = get_mmio_addr_index(nodeid, link);
		reg = 0x110+ (index<<24) + (6<<20); // index could be 0, 63

	}
	resource = new_resource(dev, IOINDEX(0x1000 + reg, link));
	return resource;
}


static void amdfam10_link_read_bases(device_t dev, u32 nodeid, u32 link)
{
	struct resource *resource;

	/* Initialize the io space constraints on the current bus */
	resource = amdfam10_find_iopair(dev, nodeid, link);
	if (resource) {
		u32 align;
#if CONFIG_EXT_CONF_SUPPORT
		if((resource->index & 0x1fff) == 0x1110) { // ext
			align = 8;
		}
		else
#endif
			align = log2(HT_IO_HOST_ALIGN);
		resource->base	= 0;
		resource->size	= 0;
		resource->align = align;
		resource->gran	= align;
		resource->limit = 0xffffUL;
		resource->flags = IORESOURCE_IO | IORESOURCE_BRIDGE;
	}

	/* Initialize the prefetchable memory constraints on the current bus */
	resource = amdfam10_find_mempair(dev, nodeid, link);
	if (resource) {
		resource->base = 0;
		resource->size = 0;
		resource->align = log2(HT_MEM_HOST_ALIGN);
		resource->gran = log2(HT_MEM_HOST_ALIGN);
		resource->limit = 0xffffffffffULL;
		resource->flags = IORESOURCE_MEM | IORESOURCE_PREFETCH;
		resource->flags |= IORESOURCE_BRIDGE;

#if CONFIG_EXT_CONF_SUPPORT
		if((resource->index & 0x1fff) == 0x1110) { // ext
			normalize_resource(resource);
		}
#endif

	}

	/* Initialize the memory constraints on the current bus */
	resource = amdfam10_find_mempair(dev, nodeid, link);
	if (resource) {
		resource->base = 0;
		resource->size = 0;
		resource->align = log2(HT_MEM_HOST_ALIGN);
		resource->gran = log2(HT_MEM_HOST_ALIGN);
		resource->limit = 0xffffffffffULL;
		resource->flags = IORESOURCE_MEM | IORESOURCE_BRIDGE;
#if CONFIG_EXT_CONF_SUPPORT
		if((resource->index & 0x1fff) == 0x1110) { // ext
			normalize_resource(resource);
		}
#endif
	}
}

static void amdfam10_read_resources(device_t dev)
{
	u32 nodeid;
	struct bus *link;
	nodeid = amdfam10_nodeid(dev);
	for(link = dev->link_list; link; link = link->next) {
		if (link->children) {
			amdfam10_link_read_bases(dev, nodeid, link->link_num);
		}
	}
}

static void amdfam10_set_resource(device_t dev, struct resource *resource,
				u32 nodeid)
{
	resource_t rbase, rend;
	unsigned reg, link_num;
	char buf[50];

	/* Make certain the resource has actually been set */
	if (!(resource->flags & IORESOURCE_ASSIGNED)) {
		return;
	}

	/* If I have already stored this resource don't worry about it */
	if (resource->flags & IORESOURCE_STORED) {
		return;
	}

	/* Only handle PCI memory and IO resources */
	if (!(resource->flags & (IORESOURCE_MEM | IORESOURCE_IO)))
		return;

	/* Ensure I am actually looking at a resource of function 1 */
	if ((resource->index & 0xffff) < 0x1000) {
		return;
	}
	/* Get the base address */
	rbase = resource->base;

	/* Get the limit (rounded up) */
	rend  = resource_end(resource);

	/* Get the register and link */
	reg  = resource->index & 0xfff; // 4k
	link_num = IOINDEX_LINK(resource->index);

	if (resource->flags & IORESOURCE_IO) {

		set_io_addr_reg(dev, nodeid, link_num, reg, rbase>>8, rend>>8);
		store_conf_io_addr(nodeid, link_num, reg, (resource->index >> 24), rbase>>8, rend>>8);
	}
	else if (resource->flags & IORESOURCE_MEM) {
		set_mmio_addr_reg(nodeid, link_num, reg, (resource->index >>24), rbase>>8, rend>>8, sysconf.nodes) ;// [39:8]
		store_conf_mmio_addr(nodeid, link_num, reg, (resource->index >>24), rbase>>8, rend>>8);
	}
	resource->flags |= IORESOURCE_STORED;
	sprintf(buf, " <node %x link %x>",
		nodeid, link_num);
	report_resource_stored(dev, resource, buf);
}

/**
 * I tried to reuse the resource allocation code in amdfam10_set_resource()
 * but it is too difficult to deal with the resource allocation magic.
 */

static void amdfam10_create_vga_resource(device_t dev, unsigned nodeid)
{
	struct bus *link;

	/* find out which link the VGA card is connected,
	 * we only deal with the 'first' vga card */
	for (link = dev->link_list; link; link = link->next) {
		if (link->bridge_ctrl & PCI_BRIDGE_CTL_VGA) {
#if CONFIG_MULTIPLE_VGA_ADAPTERS
			extern device_t vga_pri; // the primary vga device, defined in device.c
			printk(BIOS_DEBUG, "VGA: vga_pri bus num = %d bus range [%d,%d]\n", vga_pri->bus->secondary,
				link->secondary,link->subordinate);
			/* We need to make sure the vga_pri is under the link */
			if((vga_pri->bus->secondary >= link->secondary ) &&
				(vga_pri->bus->secondary <= link->subordinate )
			)
#endif
			break;
		}
	}

	/* no VGA card installed */
	if (link == NULL)
		return;

	printk(BIOS_DEBUG, "VGA: %s (aka node %d) link %d has VGA device\n", dev_path(dev), nodeid, link->link_num);
	set_vga_enable_reg(nodeid, link->link_num);
}

static void amdfam10_set_resources(device_t dev)
{
	unsigned nodeid;
	struct bus *bus;
	struct resource *res;

	/* Find the nodeid */
	nodeid = amdfam10_nodeid(dev);

	amdfam10_create_vga_resource(dev, nodeid);

	/* Set each resource we have found */
	for(res = dev->resource_list; res; res = res->next) {
		amdfam10_set_resource(dev, res, nodeid);
	}

	for(bus = dev->link_list; bus; bus = bus->next) {
		if (bus->children) {
			assign_resources(bus);
		}
	}
}

static void mcf0_control_init(struct device *dev)
{
}

static struct device_operations northbridge_operations = {
	.read_resources	  = amdfam10_read_resources,
	.set_resources	  = amdfam10_set_resources,
	.enable_resources = pci_dev_enable_resources,
	.init		  = mcf0_control_init,
	.scan_bus	  = amdfam10_scan_chains,
	.enable		  = 0,
	.ops_pci	  = 0,
};


static const struct pci_driver mcf0_driver __pci_driver = {
	.ops	= &northbridge_operations,
	.vendor = PCI_VENDOR_ID_AMD,
	.device = 0x1200,
};

struct chip_operations northbridge_amd_amdfam10_ops = {
	CHIP_NAME("AMD FAM10 Northbridge")
	.enable_dev = 0,
};

static void amdfam10_domain_read_resources(device_t dev)
{
	unsigned reg;

	/* Find the already assigned resource pairs */
	get_fx_devs();
	for(reg = 0x80; reg <= 0xd8; reg+= 0x08) {
		u32 base, limit;
		base  = f1_read_config32(reg);
		limit = f1_read_config32(reg + 0x04);
		/* Is this register allocated? */
		if ((base & 3) != 0) {
			unsigned nodeid, reg_link;
			device_t reg_dev;
			if(reg<0xc0) { // mmio
				nodeid = (limit & 0xf) + (base&0x30);
			} else { // io
				nodeid =  (limit & 0xf) + ((base>>4)&0x30);
			}
			reg_link = (limit >> 4) & 7;
			reg_dev = __f0_dev[nodeid];
			if (reg_dev) {
				/* Reserve the resource  */
				struct resource *res;
				res = new_resource(reg_dev, IOINDEX(0x1000 + reg, reg_link));
				if (res) {
					res->flags = 1;
				}
			}
		}
	}
	/* FIXME: do we need to check extend conf space?
	   I don't believe that much preset value */

#if !CONFIG_PCI_64BIT_PREF_MEM
	pci_domain_read_resources(dev);
#else
	struct bus *link;
	struct resource *resource;
	for(link=dev->link_list; link; link = link->next) {
		/* Initialize the system wide io space constraints */
		resource = new_resource(dev, 0|(link->link_num<<2));
		resource->base	= 0x400;
		resource->limit = 0xffffUL;
		resource->flags = IORESOURCE_IO;

		/* Initialize the system wide prefetchable memory resources constraints */
		resource = new_resource(dev, 1|(link->link_num<<2));
		resource->limit = 0xfcffffffffULL;
		resource->flags = IORESOURCE_MEM | IORESOURCE_PREFETCH;

		/* Initialize the system wide memory resources constraints */
		resource = new_resource(dev, 2|(link->link_num<<2));
		resource->limit = 0xfcffffffffULL;
		resource->flags = IORESOURCE_MEM;
	}
#endif
#if CONFIG_MMCONF_SUPPORT
	struct resource *res = new_resource(dev, 0xc0010058);
	res->base = CONFIG_MMCONF_BASE_ADDRESS;
	res->size = CONFIG_MMCONF_BUS_NUMBER * 4096*256;
	res->flags = IORESOURCE_MEM | IORESOURCE_RESERVE |
		IORESOURCE_FIXED | IORESOURCE_STORED |  IORESOURCE_ASSIGNED;
#endif
}

static u32 my_find_pci_tolm(struct bus *bus, u32 tolm)
{
	struct resource *min;
	min = 0;
	search_bus_resources(bus, IORESOURCE_MEM, IORESOURCE_MEM, tolm_test, &min);
	if (min && tolm > min->base) {
		tolm = min->base;
	}
	return tolm;
}

#if CONFIG_HW_MEM_HOLE_SIZEK != 0

struct hw_mem_hole_info {
	unsigned hole_startk;
	int node_id;
};

static struct hw_mem_hole_info get_hw_mem_hole_info(void)
{
		struct hw_mem_hole_info mem_hole;
		int i;

		mem_hole.hole_startk = CONFIG_HW_MEM_HOLE_SIZEK;
		mem_hole.node_id = -1;

		for (i = 0; i < sysconf.nodes; i++) {
			struct dram_base_mask_t d;
			u32 hole;
			d = get_dram_base_mask(i);
			if(!(d.mask & 1)) continue; // no memory on this node

			hole = pci_read_config32(__f1_dev[i], 0xf0);
			if(hole & 1) { // we find the hole
				mem_hole.hole_startk = (hole & (0xff<<24)) >> 10;
				mem_hole.node_id = i; // record the node No with hole
				break; // only one hole
			}
		}

		//We need to double check if there is speical set on base reg and limit reg are not continous instead of hole, it will find out it's hole_startk
		if(mem_hole.node_id==-1) {
			resource_t limitk_pri = 0;
			for(i=0; i<sysconf.nodes; i++) {
				struct dram_base_mask_t d;
				resource_t base_k, limit_k;
				d = get_dram_base_mask(i);
				if(!(d.base & 1)) continue;

				base_k = ((resource_t)(d.base & 0x1fffff00)) <<9;
				if(base_k > 4 *1024 * 1024) break; // don't need to go to check
				if(limitk_pri != base_k) { // we find the hole
					mem_hole.hole_startk = (unsigned)limitk_pri; // must beblow 4G
					mem_hole.node_id = i;
					break; //only one hole
				}

				limit_k = ((resource_t)((d.mask + 0x00000100) & 0x1fffff00)) << 9;
				limitk_pri = limit_k;
			}
		}
		return mem_hole;
}

// WHY this check? CONFIG_AMDMCT is enabled on all Fam10 boards.
// Does it make sense not to?
#if !CONFIG_AMDMCT
static void disable_hoist_memory(unsigned long hole_startk, int node_id)
{
	int i;
	device_t dev;
	struct dram_base_mask_t d;
	u32 sel_m;
	u32 sel_hi_en;
	u32 hoist;
	u32 hole_sizek;

	u32 one_DCT;
	struct sys_info *sysinfox = (struct sys_info *)((CONFIG_RAMTOP) - sizeof(*sysinfox)); // in RAM
	struct mem_info *meminfo;
	meminfo = &sysinfox->meminfo[node_id];

	one_DCT = get_one_DCT(meminfo);

	// 1. find which node has hole
	// 2. change limit in that node.
	// 3. change base and limit in later node
	// 4. clear that node f0

	// if there is not mem hole enabled, we need to change it's base instead

	hole_sizek = (4*1024*1024) - hole_startk;

	for(i=NODE_NUMS-1;i>node_id;i--) {

		d = get_dram_base_mask(i);

		if(!(d.mask & 1)) continue;

		d.base -= (hole_sizek>>9);
		d.mask -= (hole_sizek>>9);
		set_dram_base_mask(i, d, sysconf.nodes);

		if(get_DctSelHiEn(i) & 1) {
			sel_m = get_DctSelBaseAddr(i);
			sel_m -= hole_startk>>10;
			set_DctSelBaseAddr(i, sel_m);
		}
	}

	d = get_dram_base_mask(node_id);
	dev = __f1_dev[node_id];
	sel_hi_en = get_DctSelHiEn(node_id);

	if(sel_hi_en & 1) {
		sel_m = get_DctSelBaseAddr(node_id);
	}
	hoist = pci_read_config32(dev, 0xf0);
	if(hoist & 1) {
		pci_write_config32(dev, 0xf0, 0);
		d.mask -= (hole_sizek>>9);
		set_dram_base_mask(node_id, d, sysconf.nodes);
		if(one_DCT || (sel_m >= (hole_startk>>10))) {
			if(sel_hi_en & 1) {
				sel_m -= hole_startk>>10;
				set_DctSelBaseAddr(node_id, sel_m);
			}
		}
		if(sel_hi_en & 1) {
			set_DctSelBaseOffset(node_id, 0);
		}
	} else {
		d.base -= (hole_sizek>>9);
		d.mask -= (hole_sizek>>9);
		set_dram_base_mask(node_id, d, sysconf.nodes);

		if(sel_hi_en & 1) {
			sel_m -= hole_startk>>10;
			set_DctSelBaseAddr(node_id, sel_m);
		}
	}

}
#endif

#endif

#include <cbmem.h>

static void setup_uma_memory(void)
{
#if CONFIG_GFXUMA
	uint32_t topmem = (uint32_t) bsp_topmem();
	/* refer to UMA Size Consideration in 780 BDG. */
	switch (topmem) {
	case 0x10000000:	/* 256M system memory */
		uma_memory_size = 0x4000000;	/* 64M recommended UMA */
		break;

	case 0x20000000:	/* 512M system memory */
		uma_memory_size = 0x8000000;	/* 128M recommended UMA */
		break;

	default:		/* 1GB and above system memory */
		uma_memory_size = 0x10000000;	/* 256M recommended UMA */
		break;
	}

	uma_memory_base = topmem - uma_memory_size;	/* TOP_MEM1 */
	printk(BIOS_INFO, "%s: uma size 0x%08llx, memory start 0x%08llx\n",
		    __func__, uma_memory_size, uma_memory_base);
#endif
}

static void amdfam10_domain_set_resources(device_t dev)
{
#if CONFIG_PCI_64BIT_PREF_MEM
	struct resource *io, *mem1, *mem2;
	struct resource *res;
#endif
	unsigned long mmio_basek;
	u32 pci_tolm;
	int i, idx;
	struct bus *link;
#if CONFIG_HW_MEM_HOLE_SIZEK != 0
	struct hw_mem_hole_info mem_hole;
	u32 reset_memhole = 1;
#endif

#if CONFIG_PCI_64BIT_PREF_MEM

	for(link = dev->link_list; link; link = link->next) {
		/* Now reallocate the pci resources memory with the
		 * highest addresses I can manage.
		 */
		mem1 = find_resource(dev, 1|(link->link_num<<2));
		mem2 = find_resource(dev, 2|(link->link_num<<2));

		printk(BIOS_DEBUG, "base1: 0x%08Lx limit1: 0x%08Lx size: 0x%08Lx align: %d\n",
			mem1->base, mem1->limit, mem1->size, mem1->align);
		printk(BIOS_DEBUG, "base2: 0x%08Lx limit2: 0x%08Lx size: 0x%08Lx align: %d\n",
			mem2->base, mem2->limit, mem2->size, mem2->align);

		/* See if both resources have roughly the same limits */
		if (((mem1->limit <= 0xffffffff) && (mem2->limit <= 0xffffffff)) ||
			((mem1->limit > 0xffffffff) && (mem2->limit > 0xffffffff)))
		{
			/* If so place the one with the most stringent alignment first
			 */
			if (mem2->align > mem1->align) {
				struct resource *tmp;
				tmp = mem1;
				mem1 = mem2;
				mem2 = tmp;
			}
			/* Now place the memory as high up as it will go */
			mem2->base = resource_max(mem2);
			mem1->limit = mem2->base - 1;
			mem1->base = resource_max(mem1);
		}
		else {
			/* Place the resources as high up as they will go */
			mem2->base = resource_max(mem2);
			mem1->base = resource_max(mem1);
		}

		printk(BIOS_DEBUG, "base1: 0x%08Lx limit1: 0x%08Lx size: 0x%08Lx align: %d\n",
			mem1->base, mem1->limit, mem1->size, mem1->align);
		printk(BIOS_DEBUG, "base2: 0x%08Lx limit2: 0x%08Lx size: 0x%08Lx align: %d\n",
			mem2->base, mem2->limit, mem2->size, mem2->align);
	}

	for(res = &dev->resource_list; res; res = res->next)
	{
		res->flags |= IORESOURCE_ASSIGNED;
		res->flags |= IORESOURCE_STORED;
		report_resource_stored(dev, res, "");
	}
#endif

	pci_tolm = 0xffffffffUL;
	for(link = dev->link_list; link; link = link->next) {
		pci_tolm = my_find_pci_tolm(link, pci_tolm);
	}

	// FIXME handle interleaved nodes. If you fix this here, please fix
	// amdk8, too.
	mmio_basek = pci_tolm >> 10;
	/* Round mmio_basek to something the processor can support */
	mmio_basek &= ~((1 << 6) -1);

	// FIXME improve mtrr.c so we don't use up all of the mtrrs with a 64M
	// MMIO hole. If you fix this here, please fix amdk8, too.
	/* Round the mmio hole to 64M */
	mmio_basek &= ~((64*1024) - 1);

#if CONFIG_HW_MEM_HOLE_SIZEK != 0
/* if the hw mem hole is already set in raminit stage, here we will compare
 * mmio_basek and hole_basek. if mmio_basek is bigger that hole_basek and will
 * use hole_basek as mmio_basek and we don't need to reset hole.
 * otherwise We reset the hole to the mmio_basek
 */

	mem_hole = get_hw_mem_hole_info();

	// Use hole_basek as mmio_basek, and we don't need to reset hole anymore
	if ((mem_hole.node_id !=  -1) && (mmio_basek > mem_hole.hole_startk)) {
		mmio_basek = mem_hole.hole_startk;
		reset_memhole = 0;
	}

	#if !CONFIG_AMDMCT
	//mmio_basek = 3*1024*1024; // for debug to meet boundary

	if(reset_memhole) {
		if(mem_hole.node_id!=-1) {
		/* We need to select CONFIG_HW_MEM_HOLE_SIZEK for raminit, it can not
		    make hole_startk to some basek too!
		   We need to reset our Mem Hole, because We want more big HOLE
		    than we already set
		   Before that We need to disable mem hole at first, becase
		    memhole could already be set on i+1 instead
		 */
			disable_hoist_memory(mem_hole.hole_startk, mem_hole.node_id);
		}

	#if CONFIG_HW_MEM_HOLE_SIZE_AUTO_INC
		// We need to double check if the mmio_basek is valid for hole
		// setting, if it is equal to basek, we need to decrease it some
		resource_t basek_pri;
		for (i = 0; i < sysconf.nodes; i++) {
			struct dram_base_mask_t d;
			resource_t basek;
			d = get_dram_base_mask(i);

			if(!(d.mask &1)) continue;

			basek = ((resource_t)(d.base & 0x1fffff00)) << 9;
			if(mmio_basek == (u32)basek) {
				mmio_basek -= (uin32_t)(basek - basek_pri); // increase mem hole size to make sure it is on middle of pri node
				break;
			}
			basek_pri = basek;
		}
	#endif
	}
	#endif


#endif

	idx = 0x10;
	for(i = 0; i < sysconf.nodes; i++) {
		struct dram_base_mask_t d;
		resource_t basek, limitk, sizek; // 4 1T
		d = get_dram_base_mask(i);

		if(!(d.mask & 1)) continue;
		basek = ((resource_t)(d.base & 0x1fffff00)) << 9; // could overflow, we may lost 6 bit here
		limitk = ((resource_t)((d.mask + 0x00000100) & 0x1fffff00)) << 9 ;
		sizek = limitk - basek;

		/* see if we need a hole from 0xa0000 to 0xbffff */
		if ((basek < ((8*64)+(8*16))) && (sizek > ((8*64)+(16*16)))) {
			ram_resource(dev, (idx | i), basek, ((8*64)+(8*16)) - basek);
			idx += 0x10;
			basek = (8*64)+(16*16);
			sizek = limitk - ((8*64)+(16*16));

		}

//		printk(BIOS_DEBUG, "node %d : mmio_basek=%08x, basek=%08x, limitk=%08x\n", i, mmio_basek, basek, limitk);

		/* split the region to accomodate pci memory space */
		if ( (basek < 4*1024*1024 ) && (limitk > mmio_basek) ) {
			if (basek <= mmio_basek) {
				unsigned pre_sizek;
				pre_sizek = mmio_basek - basek;
				if(pre_sizek>0) {
					ram_resource(dev, (idx | i), basek, pre_sizek);
					idx += 0x10;
					sizek -= pre_sizek;

					if (high_tables_base==0) {
					/* Leave some space for ACPI, PIRQ and MP tables */
#if CONFIG_GFXUMA
						high_tables_base = uma_memory_base - HIGH_MEMORY_SIZE;
#else
						high_tables_base = (mmio_basek * 1024) - HIGH_MEMORY_SIZE;
#endif
						high_tables_size = HIGH_MEMORY_SIZE;
						printk(BIOS_DEBUG, " split: %dK table at =%08llx\n",
							HIGH_MEMORY_SIZE / 1024, high_tables_base);
					}
				}
				#if !CONFIG_AMDMCT
				#if CONFIG_HW_MEM_HOLE_SIZEK != 0
				if(reset_memhole) {
					struct sys_info *sysinfox = (struct sys_info *)((CONFIG_RAMTOP) - sizeof(*sysinfox)); // in RAM
					struct mem_info *meminfo;
					meminfo = &sysinfox->meminfo[i];
					sizek += hoist_memory(mmio_basek,i, get_one_DCT(meminfo), sysconf.nodes);
				}
				#endif
				#endif

				basek = mmio_basek;
			}
			if ((basek + sizek) <= 4*1024*1024) {
				sizek = 0;
			}
			else {
				basek = 4*1024*1024;
				sizek -= (4*1024*1024 - mmio_basek);
			}
		}

		ram_resource(dev, (idx | i), basek, sizek);
		idx += 0x10;
		printk(BIOS_DEBUG, "%d: mmio_basek=%08lx, basek=%08llx, limitk=%08llx\n",
			     i, mmio_basek, basek, limitk);
		if (high_tables_base==0) {
		/* Leave some space for ACPI, PIRQ and MP tables */
#if CONFIG_GFXUMA
			high_tables_base = uma_memory_base - HIGH_MEMORY_SIZE;
#else
			high_tables_base = (limitk * 1024) - HIGH_MEMORY_SIZE;
#endif
			high_tables_size = HIGH_MEMORY_SIZE;
		}
	}

#if CONFIG_GFXUMA
	uma_resource(dev, 7, uma_memory_base >> 10, uma_memory_size >> 10);
#endif

	for(link = dev->link_list; link; link = link->next) {
		if (link->children) {
			assign_resources(link);
		}
	}
}

static u32 amdfam10_domain_scan_bus(device_t dev, u32 max)
{
	u32 reg;
	int i;
	struct bus *link;
	/* Unmap all of the HT chains */
	for(reg = 0xe0; reg <= 0xec; reg += 4) {
		f1_write_config32(reg, 0);
	}
#if CONFIG_EXT_CONF_SUPPORT
	// all nodes
	for(i = 0; i< sysconf.nodes; i++) {
		int index;
		for(index = 0; index < 64; index++) {
			pci_write_config32(__f1_dev[i], 0x110, index | (6<<28));
			pci_write_config32(__f1_dev[i], 0x114, 0);
		}

	}
#endif


	for(link = dev->link_list; link; link = link->next) {
		max = pci_scan_bus(link, PCI_DEVFN(CONFIG_CDB, 0), 0xff, max);
	}

	/* Tune the hypertransport transaction for best performance.
	 * Including enabling relaxed ordering if it is safe.
	 */
	get_fx_devs();
	for(i = 0; i < fx_devs; i++) {
		device_t f0_dev;
		f0_dev = __f0_dev[i];
		if (f0_dev && f0_dev->enabled) {
			u32 httc;
			httc = pci_read_config32(f0_dev, HT_TRANSACTION_CONTROL);
			httc &= ~HTTC_RSP_PASS_PW;
			if (!dev->link_list->disable_relaxed_ordering) {
				httc |= HTTC_RSP_PASS_PW;
			}
			printk(BIOS_SPEW, "%s passpw: %s\n",
				dev_path(dev),
				(!dev->link_list->disable_relaxed_ordering)?
				"enabled":"disabled");
			pci_write_config32(f0_dev, HT_TRANSACTION_CONTROL, httc);
		}
	}
	return max;
}

static struct device_operations pci_domain_ops = {
	.read_resources	  = amdfam10_domain_read_resources,
	.set_resources	  = amdfam10_domain_set_resources,
	.enable_resources = NULL,
	.init		  = NULL,
	.scan_bus	  = amdfam10_domain_scan_bus,
#if CONFIG_MMCONF_SUPPORT_DEFAULT
	.ops_pci_bus	  = &pci_ops_mmconf,
#else
	.ops_pci_bus	  = &pci_cf8_conf1,
#endif
};

static void sysconf_init(device_t dev) // first node
{
	sysconf.sblk = (pci_read_config32(dev, 0x64)>>8) & 7; // don't forget sublink1
	sysconf.segbit = 0;
	sysconf.ht_c_num = 0;

	unsigned ht_c_index;

	for(ht_c_index=0; ht_c_index<32; ht_c_index++) {
		sysconf.ht_c_conf_bus[ht_c_index] = 0;
	}

	sysconf.nodes = ((pci_read_config32(dev, 0x60)>>4) & 7) + 1;
#if CONFIG_MAX_PHYSICAL_CPUS > 8
	sysconf.nodes += (((pci_read_config32(dev, 0x160)>>4) & 7)<<3);
#endif

	sysconf.enabled_apic_ext_id = 0;
	sysconf.lift_bsp_apicid = 0;

	/* Find the bootstrap processors apicid */
	sysconf.bsp_apicid = lapicid();
	sysconf.apicid_offset = sysconf.bsp_apicid;

#if CONFIG_ENABLE_APIC_EXT_ID
	if (pci_read_config32(dev, 0x68) & (HTTC_APIC_EXT_ID|HTTC_APIC_EXT_BRD_CST))
	{
		sysconf.enabled_apic_ext_id = 1;
	}
	#if (CONFIG_APIC_ID_OFFSET>0)
	if(sysconf.enabled_apic_ext_id) {
		if(sysconf.bsp_apicid == 0) {
			/* bsp apic id is not changed */
			sysconf.apicid_offset = CONFIG_APIC_ID_OFFSET;
		} else {
			sysconf.lift_bsp_apicid = 1;
		}
	}
	#endif
#endif
}

static void add_more_links(device_t dev, unsigned total_links)
{
	struct bus *link, *last = NULL;
	int link_num;

	for (link = dev->link_list; link; link = link->next)
		last = link;

	if (last) {
		int links = total_links - last->link_num;
		link_num = last->link_num;
		if (links > 0) {
			link = malloc(links*sizeof(*link));
			if (!link)
				die("Couldn't allocate more links!\n");
			memset(link, 0, links*sizeof(*link));
			last->next = link;
		}
	}
	else {
		link_num = -1;
		link = malloc(total_links*sizeof(*link));
		memset(link, 0, total_links*sizeof(*link));
		dev->link_list = link;
	}

	for (link_num = link_num + 1; link_num < total_links; link_num++) {
		link->link_num = link_num;
		link->dev = dev;
		link->next = link + 1;
		last = link;
		link = link->next;
	}
	last->next = NULL;
}

static u32 cpu_bus_scan(device_t dev, u32 max)
{
	struct bus *cpu_bus;
	device_t dev_mc;
#if CONFIG_CBB
	device_t pci_domain;
#endif
	int i,j;
	int nodes;
	unsigned nb_cfg_54;
	unsigned siblings;
	int cores_found;
	int disable_siblings;
	unsigned ApicIdCoreIdSize;

	nb_cfg_54 = 0;
	ApicIdCoreIdSize = (cpuid_ecx(0x80000008)>>12 & 0xf);
	if(ApicIdCoreIdSize) {
		siblings = (1<<ApicIdCoreIdSize)-1;
	} else {
		siblings = 3; //quad core
	}

	disable_siblings = !CONFIG_LOGICAL_CPUS;
#if CONFIG_LOGICAL_CPUS
	get_option(&disable_siblings, "multi_core");
#endif

	// How can I get the nb_cfg_54 of every node's nb_cfg_54 in bsp???
	nb_cfg_54 = read_nb_cfg_54();

#if CONFIG_CBB
	dev_mc = dev_find_slot(0, PCI_DEVFN(CONFIG_CDB, 0)); //0x00
	if(dev_mc && dev_mc->bus) {
		printk(BIOS_DEBUG, "%s found", dev_path(dev_mc));
		pci_domain = dev_mc->bus->dev;
		if(pci_domain && (pci_domain->path.type == DEVICE_PATH_DOMAIN)) {
			printk(BIOS_DEBUG, "\n%s move to ",dev_path(dev_mc));
			dev_mc->bus->secondary = CONFIG_CBB; // move to 0xff
			printk(BIOS_DEBUG, "%s",dev_path(dev_mc));

		} else {
			printk(BIOS_DEBUG, " but it is not under pci_domain directly ");
		}
		printk(BIOS_DEBUG, "\n");
	}
	dev_mc = dev_find_slot(CONFIG_CBB, PCI_DEVFN(CONFIG_CDB, 0));
	if(!dev_mc) {
		dev_mc = dev_find_slot(0, PCI_DEVFN(0x18, 0));
		if (dev_mc && dev_mc->bus) {
			printk(BIOS_DEBUG, "%s found\n", dev_path(dev_mc));
			pci_domain = dev_mc->bus->dev;
			if(pci_domain && (pci_domain->path.type == DEVICE_PATH_DOMAIN)) {
				if((pci_domain->link_list) && (pci_domain->link_list->children == dev_mc)) {
					printk(BIOS_DEBUG, "%s move to ",dev_path(dev_mc));
					dev_mc->bus->secondary = CONFIG_CBB; // move to 0xff
					printk(BIOS_DEBUG, "%s\n",dev_path(dev_mc));
					while(dev_mc){
						printk(BIOS_DEBUG, "%s move to ",dev_path(dev_mc));
						dev_mc->path.pci.devfn -= PCI_DEVFN(0x18,0);
						printk(BIOS_DEBUG, "%s\n",dev_path(dev_mc));
						dev_mc = dev_mc->sibling;
					}
				}
			}
		}
	}

#endif

	dev_mc = dev_find_slot(CONFIG_CBB, PCI_DEVFN(CONFIG_CDB, 0));
	if (!dev_mc) {
		printk(BIOS_ERR, "%02x:%02x.0 not found", CONFIG_CBB, CONFIG_CDB);
		die("");
	}

	sysconf_init(dev_mc);

	nodes = sysconf.nodes;

#if CONFIG_CBB && (NODE_NUMS > 32)
	if(nodes>32) { // need to put node 32 to node 63 to bus 0xfe
		if(pci_domain->link_list && !pci_domain->link_list->next) {
			struct bus *new_link = new_link(pci_domain);
			pci_domain->link_list->next = new_link;
			new_link->link_num = 1;
			new_link->dev = pci_domain;
			new_link->children = 0;
			printk(BIOS_DEBUG, "%s links now 2\n", dev_path(pci_domain));
		}
		pci_domain->link_list->next->secondary = CONFIG_CBB - 1;
	}
#endif
	/* Find which cpus are present */
	cpu_bus = dev->link_list;

	/* Always use the devicetree node with lapic_id 0 for BSP. */
	remap_bsp_lapic(cpu_bus);

	for(i = 0; i < nodes; i++) {
		device_t cdb_dev;
		unsigned busn, devn;
		struct bus *pbus;

		busn = CONFIG_CBB;
		devn = CONFIG_CDB+i;
		pbus = dev_mc->bus;
#if CONFIG_CBB && (NODE_NUMS > 32)
		if(i>=32) {
			busn--;
			devn-=32;
			pbus = pci_domain->link_list->next);
		}
#endif

		/* Find the cpu's pci device */
		cdb_dev = dev_find_slot(busn, PCI_DEVFN(devn, 0));
		if (!cdb_dev) {
			/* If I am probing things in a weird order
			 * ensure all of the cpu's pci devices are found.
			 */
			int fn;
			for(fn = 0; fn <= 5; fn++) { //FBDIMM?
				cdb_dev = pci_probe_dev(NULL, pbus,
					PCI_DEVFN(devn, fn));
			}
			cdb_dev = dev_find_slot(busn, PCI_DEVFN(devn, 0));
		}
		if (cdb_dev) {
			/* Ok, We need to set the links for that device.
			 * otherwise the device under it will not be scanned
			 */
			int linknum;
#if CONFIG_HT3_SUPPORT
			linknum = 8;
#else
			linknum = 4;
#endif
			add_more_links(cdb_dev, linknum);
		}

		cores_found = 0; // one core
		cdb_dev = dev_find_slot(busn, PCI_DEVFN(devn, 3));
		int enable_node = cdb_dev && cdb_dev->enabled;
		if (enable_node) {
			j = pci_read_config32(cdb_dev, 0xe8);
			cores_found = (j >> 12) & 3; // dev is func 3
			if (siblings > 3)
				cores_found |= (j >> 13) & 4;
			printk(BIOS_DEBUG, "  %s siblings=%d\n", dev_path(cdb_dev), cores_found);
		}

		u32 jj;
		if(disable_siblings) {
			jj = 0;
		} else
		{
			jj = cores_found;
		}

		for (j = 0; j <=jj; j++ ) {
			u32 apic_id = i * (nb_cfg_54?(siblings+1):1) + j * (nb_cfg_54?1:64); // ?

#if CONFIG_ENABLE_APIC_EXT_ID && (CONFIG_APIC_ID_OFFSET>0)
			if(sysconf.enabled_apic_ext_id) {
				if (apic_id != 0 || sysconf.lift_bsp_apicid) {
					apic_id += sysconf.apicid_offset;
				}
			}
#endif
			device_t cpu = add_cpu_device(cpu_bus, apic_id, enable_node);
			if (cpu)
				amd_cpu_topology(cpu, i, j);
		} //j
	}
	return max;
}

static void cpu_bus_init(device_t dev)
{
	initialize_cpus(dev->link_list);
#if CONFIG_AMD_SB_CIMX
	sb_After_Pci_Init();
	sb_Mid_Post_Init();
#endif
}

static void cpu_bus_noop(device_t dev)
{
}

static void cpu_bus_read_resources(device_t dev)
{
}

static void cpu_bus_set_resources(device_t dev)
{
	struct resource *resource = find_resource(dev, 0xc0010058);
	if (resource) {
		report_resource_stored(dev, resource, " <mmconfig>");
	}
	pci_dev_set_resources(dev);
}

static struct device_operations cpu_bus_ops = {
	.read_resources	  = cpu_bus_read_resources,
	.set_resources	  = cpu_bus_set_resources,
	.enable_resources = cpu_bus_noop,
	.init		  = cpu_bus_init,
	.scan_bus	  = cpu_bus_scan,
};

static void root_complex_enable_dev(struct device *dev)
{
	static int done = 0;

	/* Do not delay UMA setup, as a device on the PCI bus may evaluate
	   the global uma_memory variables already in its enable function. */
	if (!done) {
		setup_bsp_ramtop();
		setup_uma_memory();
		done = 1;
	}

	/* Set the operations if it is a special bus type */
	if (dev->path.type == DEVICE_PATH_DOMAIN) {
		dev->ops = &pci_domain_ops;
	}
	else if (dev->path.type == DEVICE_PATH_CPU_CLUSTER) {
		dev->ops = &cpu_bus_ops;
	}
}

struct chip_operations northbridge_amd_amdfam10_root_complex_ops = {
	CHIP_NAME("AMD FAM10 Root Complex")
	.enable_dev = root_complex_enable_dev,
};
