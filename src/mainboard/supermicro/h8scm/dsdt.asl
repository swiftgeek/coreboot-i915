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

/* DefinitionBlock Statement */
DefinitionBlock (
	"DSDT.AML",	/* Output filename */
	"DSDT",		/* Signature */
	0x02,		/* DSDT Revision, needs to be 2 for 64bit */
	"AMD   ",	/* OEMID */
	"COREBOOT",	/* TABLE ID */
	0x00010001	/* OEM Revision */
	)
{	/* Start of ASL file */
	/* #include <arch/x86/acpi/debug.asl> */ /* Include global debug methods if needed */

	/* Data to be patched by the BIOS during POST */
	/* FIXME the patching is not done yet! */
	/* Memory related values */
	Name(LOMH, 0x0)	/* Start of unused memory in C0000-E0000 range */
	Name(PBAD, 0x0)	/* Address of BIOS area (If TOM2 != 0, Addr >> 16) */
	Name(PBLN, 0x0)	/* Length of BIOS area */

	Name(PCBA, CONFIG_MMCONF_BASE_ADDRESS)	/* Base address of PCIe config space */
	Name(PCLN, Multiply(0x100000, CONFIG_MMCONF_BUS_NUMBER)) /* Length of PCIe config space, 1MB each bus */

	Name(HPBA, 0xFED00000)	/* Base address of HPET table */
	Name(SSFG, 0x0D)	/* S1 support: bit 0, S2 Support: bit 1, etc. S0 & S5 assumed */

	/* USB overcurrent mapping pins.   */
	Name(UOM0, 0)
	Name(UOM1, 2)
	Name(UOM2, 0)
	Name(UOM3, 7)
	Name(UOM4, 2)
	Name(UOM5, 2)
	Name(UOM6, 6)
	Name(UOM7, 2)
	Name(UOM8, 6)
	Name(UOM9, 6)

	/* Some global data */
	Name(OSTP, 3)		/* Assume nothing. WinXp = 1, Vista = 2, Linux = 3, WinCE = 4 */
	Name(OSV, Ones)	/* Assume nothing */
	Name(PMOD, One)	/* Assume APIC */

	/*
	 * Processor Object
	 *
	 */
	Scope (\_PR) {		/* define processor scope */
		Processor(
			C000,		/* name space name */
			0x00,		/* Unique number for this processor */
			0x810,		/* PBLK system I/O address !hardcoded! */
			0x06		/* PBLKLEN for boot processor */
			) {
			//#include "acpi/cpstate.asl"
		}
		Processor(C001, 0x01, 0x00000000, 0x00) {}
		Processor(C002, 0x02, 0x00000000, 0x00) {}
		Processor(C003, 0x03, 0x00000000, 0x00) {}
		Processor(C004, 0x04, 0x00000000, 0x00) {}
		Processor(C005, 0x05, 0x00000000, 0x00) {}
		Processor(C006, 0x06, 0x00000000, 0x00) {}
		Processor(C007, 0x07, 0x00000000, 0x00) {}
		Processor(C008, 0x08, 0x00000000, 0x00) {}
		Processor(C009, 0x09, 0x00000000, 0x00) {}
		Processor(C00A, 0x0A, 0x00000000, 0x00) {}
		Processor(C00B, 0x0B, 0x00000000, 0x00) {}
		Processor(C00C, 0x0C, 0x00000000, 0x00) {}
		Processor(C00D, 0x0D, 0x00000000, 0x00) {}
		Processor(C00E, 0x0E, 0x00000000, 0x00) {}
		Processor(C00F, 0x0F, 0x00000000, 0x00) {}
		Processor(C010, 0x10, 0x00000000, 0x00) {}
		Processor(C011, 0x11, 0x00000000, 0x00) {}
		Processor(C012, 0x12, 0x00000000, 0x00) {}
		Processor(C013, 0x13, 0x00000000, 0x00) {}
		Processor(C014, 0x14, 0x00000000, 0x00) {}
		Processor(C015, 0x15, 0x00000000, 0x00) {}
		Processor(C016, 0x16, 0x00000000, 0x00) {}
		Processor(C017, 0x17, 0x00000000, 0x00) {}
		Processor(C018, 0x18, 0x00000000, 0x00) {}
		Processor(C019, 0x19, 0x00000000, 0x00) {}
		Processor(C01A, 0x1A, 0x00000000, 0x00) {}
		Processor(C01B, 0x1B, 0x00000000, 0x00) {}
		Processor(C01C, 0x1C, 0x00000000, 0x00) {}
		Processor(C01D, 0x1D, 0x00000000, 0x00) {}
		Processor(C01E, 0x1E, 0x00000000, 0x00) {}
		Processor(C01F, 0x1F, 0x00000000, 0x00) {}
		Processor(C020, 0x20, 0x00000000, 0x00) {}
		Processor(C021, 0x21, 0x00000000, 0x00) {}
		Processor(C022, 0x22, 0x00000000, 0x00) {}
		Processor(C023, 0x23, 0x00000000, 0x00) {}
		Processor(C024, 0x24, 0x00000000, 0x00) {}
		Processor(C025, 0x25, 0x00000000, 0x00) {}
		Processor(C026, 0x26, 0x00000000, 0x00) {}
		Processor(C027, 0x27, 0x00000000, 0x00) {}
		Processor(C028, 0x28, 0x00000000, 0x00) {}
		Processor(C029, 0x29, 0x00000000, 0x00) {}
		Processor(C02A, 0x2A, 0x00000000, 0x00) {}
		Processor(C02B, 0x2B, 0x00000000, 0x00) {}
		Processor(C02C, 0x2C, 0x00000000, 0x00) {}
		Processor(C02D, 0x2D, 0x00000000, 0x00) {}
		Processor(C02E, 0x2E, 0x00000000, 0x00) {}
		Processor(C02F, 0x2F, 0x00000000, 0x00) {}
		Alias (C000, CPU0)
		Alias (C001, CPU1)
		Alias (C002, CPU2)
		Alias (C003, CPU3)
		Alias (C004, CPU4)
		Alias (C005, CPU5)
		Alias (C006, CPU6)
		Alias (C007, CPU7)
		Alias (C008, CPU8)
	} /* End _PR scope */

	/* PIC IRQ mapping registers, C00h-C01h */
	OperationRegion(PRQM, SystemIO, 0x00000C00, 0x00000002)
		Field(PRQM, ByteAcc, NoLock, Preserve) {
		PRQI, 0x00000008,
		PRQD, 0x00000008,  /* Offset: 1h */
	}
	IndexField(PRQI, PRQD, ByteAcc, NoLock, Preserve) {
		PINA, 0x00000008,	/* Index 0 */
		PINB, 0x00000008,	/* Index 1 */
		PINC, 0x00000008,	/* Index 2 */
		PIND, 0x00000008,	/* Index 3 */
		AINT, 0x00000008,	/* Index 4 */
		SINT, 0x00000008,	/* Index 5 */
		    , 0x00000008,	/* Index 6 */
		AAUD, 0x00000008,	/* Index 7 */
		AMOD, 0x00000008,	/* Index 8 */
		PINE, 0x00000008,	/* Index 9 */
		PINF, 0x00000008,	/* Index A */
		PING, 0x00000008,	/* Index B */
		PINH, 0x00000008,	/* Index C */
	}

	/* PCI Error control register */
	OperationRegion(PERC, SystemIO, 0x00000C14, 0x00000001)
		Field(PERC, ByteAcc, NoLock, Preserve) {
		SENS, 0x00000001,
		PENS, 0x00000001,
		SENE, 0x00000001,
		PENE, 0x00000001,
	}

	/* Client Management index/data registers */
	OperationRegion(CMT, SystemIO, 0x00000C50, 0x00000002)
		Field(CMT, ByteAcc, NoLock, Preserve) {
		CMTI, 8,
		/* Client Management Data register */
		G64E, 1,
		G64O, 1,
		G32O, 2,
		    , 2,
		GPSL, 2,
	}

	/* GPM Port register */
	OperationRegion(GPT, SystemIO, 0x00000C52, 0x00000001)
		Field(GPT, ByteAcc, NoLock, Preserve) {
		GPB0,1,
		GPB1,1,
		GPB2,1,
		GPB3,1,
		GPB4,1,
		GPB5,1,
		GPB6,1,
		GPB7,1,
	}

	/* Flash ROM program enable register */
	OperationRegion(FRE, SystemIO, 0x00000C6F, 0x00000001)
		Field(FRE, ByteAcc, NoLock, Preserve) {
		    , 0x00000006,
		FLRE, 0x00000001,
	}

	/* PM2 index/data registers */
	OperationRegion(PM2R, SystemIO, 0x00000CD0, 0x00000002)
		Field(PM2R, ByteAcc, NoLock, Preserve) {
		PM2I, 0x00000008,
		PM2D, 0x00000008,
	}

	/* Power Management I/O registers */
	OperationRegion(PIOR, SystemIO, 0x00000CD6, 0x00000002)
		Field(PIOR, ByteAcc, NoLock, Preserve) {
		PIOI, 0x00000008,
		PIOD, 0x00000008,
	}
	IndexField (PIOI, PIOD, ByteAcc, NoLock, Preserve) {
		Offset(0x00),	/* MiscControl */
		, 1,
		T1EE, 1,
		T2EE, 1,
		Offset(0x01),	/* MiscStatus */
		, 1,
		T1E, 1,
		T2E, 1,
		Offset(0x04),	/* SmiWakeUpEventEnable3 */
		, 7,
		SSEN, 1,
		Offset(0x07),	/* SmiWakeUpEventStatus3 */
		, 7,
		CSSM, 1,
		Offset(0x10),	/* AcpiEnable */
		, 6,
		PWDE, 1,
		Offset(0x1C),	/* ProgramIoEnable */
		, 3,
		MKME, 1,
		IO3E, 1,
		IO2E, 1,
		IO1E, 1,
		IO0E, 1,
		Offset(0x1D),	/* IOMonitorStatus */
		, 3,
		MKMS, 1,
		IO3S, 1,
		IO2S, 1,
		IO1S, 1,
		IO0S,1,
		Offset(0x20),	/* AcpiPmEvtBlk */
		APEB, 16,
		Offset(0x36),	/* GEvtLevelConfig */
		, 6,
		ELC6, 1,
		ELC7, 1,
		Offset(0x37),	/* GPMLevelConfig0 */
		, 3,
		PLC0, 1,
		PLC1, 1,
		PLC2, 1,
		PLC3, 1,
		PLC8, 1,
		Offset(0x38),	/* GPMLevelConfig1 */
		, 1,
		 PLC4, 1,
		 PLC5, 1,
		, 1,
		 PLC6, 1,
		 PLC7, 1,
		Offset(0x3B),	/* PMEStatus1 */
		GP0S, 1,
		GM4S, 1,
		GM5S, 1,
		APS, 1,
		GM6S, 1,
		GM7S, 1,
		GP2S, 1,
		STSS, 1,
		Offset(0x55),	/* SoftPciRst */
		SPRE, 1,
		, 1,
		, 1,
		PNAT, 1,
		PWMK, 1,
		PWNS, 1,

		/* 	Offset(0x61), */	/*  Options_1 */
		/* 		,7,  */
		/* 		R617,1, */

		Offset(0x65),	/* UsbPMControl */
		, 4,
		URRE, 1,
		Offset(0x68),	/* MiscEnable68 */
		, 3,
		TMTE, 1,
		, 1,
		Offset(0x92),	/* GEVENTIN */
		, 7,
		E7IS, 1,
		Offset(0x96),	/* GPM98IN */
		G8IS, 1,
		G9IS, 1,
		Offset(0x9A),	/* EnhanceControl */
		,7,
		HPDE, 1,
		Offset(0xA8),	/* PIO7654Enable */
		IO4E, 1,
		IO5E, 1,
		IO6E, 1,
		IO7E, 1,
		Offset(0xA9),	/* PIO7654Status */
		IO4S, 1,
		IO5S, 1,
		IO6S, 1,
		IO7S, 1,
	}

	/* PM1 Event Block
	* First word is PM1_Status, Second word is PM1_Enable
	*/
	OperationRegion(P1EB, SystemIO, APEB, 0x04)
		Field(P1EB, ByteAcc, NoLock, Preserve) {
		TMST, 1,
		,    3,
		BMST,    1,
		GBST,   1,
		Offset(0x01),
		PBST, 1,
		, 1,
		RTST, 1,
		, 3,
		PWST, 1,
		SPWS, 1,
		Offset(0x02),
		TMEN, 1,
		, 4,
		GBEN, 1,
		Offset(0x03),
		PBEN, 1,
		, 1,
		RTEN, 1,
		, 3,
		PWDA, 1,
	}

	OperationRegion (GRAM, SystemMemory, 0x0400, 0x0100)
		Field (GRAM, ByteAcc, Lock, Preserve)
		{
			Offset (0x10),
			FLG0,   8
		}

	Scope(\_SB) {
		/* PCIe Configuration Space for CONFIG_MMCONF_BUS_NUMBER busses */
		OperationRegion(PCFG, SystemMemory, PCBA, PCLN)
			Field(PCFG, ByteAcc, NoLock, Preserve) {
			/* Byte offsets are computed using the following technique:
			 * ((bus number + 1) * ((device number * 8) * 4096)) + register offset
			 * The 8 comes from 8 functions per device, and 4096 bytes per function config space
			*/
			Offset(0x00088024),	/* Byte offset to SATA register 24h - Bus 0, Device 17, Function 0 */
			STB5, 32,
			Offset(0x00098042),	/* Byte offset to OHCI0 register 42h - Bus 0, Device 19, Function 0 */
			PT0D, 1,
			PT1D, 1,
			PT2D, 1,
			PT3D, 1,
			PT4D, 1,
			PT5D, 1,
			PT6D, 1,
			PT7D, 1,
			PT8D, 1,
			PT9D, 1,
			Offset(0x000A0004),	/* Byte offset to SMBUS	register 4h - Bus 0, Device 20, Function 0 */
			SBIE, 1,
			SBME, 1,
			Offset(0x000A0008),	/* Byte offset to SMBUS	register 8h - Bus 0, Device 20, Function 0 */
			SBRI, 8,
			Offset(0x000A0014),	/* Byte offset to SMBUS	register 14h - Bus 0, Device 20, Function 0 */
			SBB1, 32,
			Offset(0x000A0078),	/* Byte offset to SMBUS	register 78h - Bus 0, Device 20, Function 0 */
			,14,
			P92E, 1,		/* Port92 decode enable */
		}

		OperationRegion(SB5, SystemMemory, STB5, 0x1000)
			Field(SB5, AnyAcc, NoLock, Preserve){
			/* Port 0 */
			Offset(0x120),		/* Port 0 Task file status */
			P0ER, 1,
			, 2,
			P0DQ, 1,
			, 3,
			P0BY, 1,
			Offset(0x128),		/* Port 0 Serial ATA status */
			P0DD, 4,
			, 4,
			P0IS, 4,
			Offset(0x12C),		/* Port 0 Serial ATA control */
			P0DI, 4,
			Offset(0x130),		/* Port 0 Serial ATA error */
			, 16,
			P0PR, 1,

			/* Port 1 */
			offset(0x1A0),		/* Port 1 Task file status */
			P1ER, 1,
			, 2,
			P1DQ, 1,
			, 3,
			P1BY, 1,
			Offset(0x1A8),		/* Port 1 Serial ATA status */
			P1DD, 4,
			, 4,
			P1IS, 4,
			Offset(0x1AC),		/* Port 1 Serial ATA control */
			P1DI, 4,
			Offset(0x1B0),		/* Port 1 Serial ATA error */
			, 16,
			P1PR, 1,

			/* Port 2 */
			Offset(0x220),		/* Port 2 Task file status */
			P2ER, 1,
			, 2,
			P2DQ, 1,
			, 3,
			P2BY, 1,
			Offset(0x228),		/* Port 2 Serial ATA status */
			P2DD, 4,
			, 4,
			P2IS, 4,
			Offset(0x22C),		/* Port 2 Serial ATA control */
			P2DI, 4,
			Offset(0x230),		/* Port 2 Serial ATA error */
			, 16,
			P2PR, 1,

			/* Port 3 */
			Offset(0x2A0),		/* Port 3 Task file status */
			P3ER, 1,
			, 2,
			P3DQ, 1,
			, 3,
			P3BY, 1,
			Offset(0x2A8),		/* Port 3 Serial ATA status */
			P3DD, 4,
			, 4,
			P3IS, 4,
			Offset(0x2AC),		/* Port 3 Serial ATA control */
			P3DI, 4,
			Offset(0x2B0),		/* Port 3 Serial ATA error */
			, 16,
			P3PR, 1,
		}
	}

	#include "acpi/routing.asl"

	Scope(\_SB) {
		Method(CkOT, 0){
			if(LNotEqual(OSTP, Ones)) {Return(OSTP)}	/* OS version was already detected */
			if(CondRefOf(\_OSI,Local1))
			{
				Store(1, OSTP)			/* Assume some form of XP */
				if (\_OSI("Windows 2006"))      /* Vista */
				{
					Store(2, OSTP)
				}
			} else {
				If(WCMP(\_OS,"Linux")) {
					Store(3, OSTP)		/* Linux */
				} Else {
					Store(4, OSTP)		/* Gotta be WinCE */
				}
			}
			Return(OSTP)
		}

		Method(_PIC, 0x01, NotSerialized)
		{
			If (Arg0)
			{
				\_SB.CIRQ()
			}
			Store(Arg0, PMOD)
		}
		Method(CIRQ, 0x00, NotSerialized){
			Store(0, PINA)
			Store(0, PINB)
			Store(0, PINC)
			Store(0, PIND)
			Store(0, PINE)
			Store(0, PINF)
			Store(0, PING)
			Store(0, PINH)
		}

		Name(IRQB, ResourceTemplate(){
			IRQ(Level,ActiveLow,Shared){15}
		})

		Name(IRQP, ResourceTemplate(){
			IRQ(Level,ActiveLow,Exclusive){3, 4, 5, 7, 10, 11, 12, 15}
		})

		Name(PITF, ResourceTemplate(){
			IRQ(Level,ActiveLow,Exclusive){9}
		})

		Device(INTA) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 1)

			Method(_STA, 0) {
				if (PINA) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTA._STA) */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKA\\_DIS\n") */
				Store(0, PINA)
			} /* End Method(_SB.INTA._DIS) */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKA\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTA._PRS) */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKA\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PINA, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTA._CRS) */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKA\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PINA)
			} /* End Method(_SB.INTA._SRS) */
		} /* End Device(INTA) */

		Device(INTB) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 2)

			Method(_STA, 0) {
				if (PINB) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTB._STA) */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKB\\_DIS\n") */
				Store(0, PINB)
			} /* End Method(_SB.INTB._DIS) */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKB\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTB._PRS) */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKB\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PINB, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTB._CRS) */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKB\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PINB)
			} /* End Method(_SB.INTB._SRS) */
		} /* End Device(INTB)  */

		Device(INTC) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 3)

			Method(_STA, 0) {
				if (PINC) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTC._STA) */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKC\\_DIS\n") */
				Store(0, PINC)
			} /* End Method(_SB.INTC._DIS) */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKC\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTC._PRS) */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKC\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PINC, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTC._CRS) */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKC\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PINC)
			} /* End Method(_SB.INTC._SRS) */
		} /* End Device(INTC)  */

		Device(INTD) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 4)

			Method(_STA, 0) {
				if (PIND) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTD._STA) */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKD\\_DIS\n") */
				Store(0, PIND)
			} /* End Method(_SB.INTD._DIS) */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKD\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTD._PRS) */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKD\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PIND, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTD._CRS) */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKD\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PIND)
			} /* End Method(_SB.INTD._SRS) */
		} /* End Device(INTD)  */

		Device(INTE) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 5)

			Method(_STA, 0) {
				if (PINE) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTE._STA) */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKE\\_DIS\n") */
				Store(0, PINE)
			} /* End Method(_SB.INTE._DIS) */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKE\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTE._PRS) */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKE\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PINE, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTE._CRS) */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKE\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PINE)
			} /* End Method(_SB.INTE._SRS) */
		} /* End Device(INTE)  */

		Device(INTF) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 6)

			Method(_STA, 0) {
				if (PINF) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTF._STA) */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKF\\_DIS\n") */
				Store(0, PINF)
			} /* End Method(_SB.INTF._DIS) */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKF\\_PRS\n") */
				Return(PITF)
			} /* Method(_SB.INTF._PRS) */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKF\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PINF, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTF._CRS) */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKF\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PINF)
			} /*  End Method(_SB.INTF._SRS) */
		} /* End Device(INTF)  */

		Device(INTG) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 7)

			Method(_STA, 0) {
				if (PING) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTG._STA)  */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKG\\_DIS\n") */
				Store(0, PING)
			} /* End Method(_SB.INTG._DIS)  */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKG\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTG._CRS)  */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKG\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PING, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTG._CRS)  */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKG\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PING)
			} /* End Method(_SB.INTG._SRS)  */
		} /* End Device(INTG)  */

		Device(INTH) {
			Name(_HID, EISAID("PNP0C0F"))
			Name(_UID, 8)

			Method(_STA, 0) {
				if (PINH) {
					Return(0x0B) /* sata is invisible */
				} else {
					Return(0x09) /* sata is disabled */
				}
			} /* End Method(_SB.INTH._STA)  */

			Method(_DIS ,0) {
				/* DBGO("\\_SB\\LNKH\\_DIS\n") */
				Store(0, PINH)
			} /* End Method(_SB.INTH._DIS)  */

			Method(_PRS ,0) {
				/* DBGO("\\_SB\\LNKH\\_PRS\n") */
				Return(IRQP)
			} /* Method(_SB.INTH._CRS)  */

			Method(_CRS ,0) {
				/* DBGO("\\_SB\\LNKH\\_CRS\n") */
				CreateWordField(IRQB, 0x1, IRQN)
				ShiftLeft(1, PINH, IRQN)
				Return(IRQB)
			} /* Method(_SB.INTH._CRS)  */

			Method(_SRS, 1) {
				/* DBGO("\\_SB\\LNKH\\_CRS\n") */
				CreateWordField(ARG0, 1, IRQM)

				/* Use lowest available IRQ */
				FindSetRightBit(IRQM, Local0)
				if (Local0) {
					Decrement(Local0)
				}
				Store(Local0, PINH)
			} /* End Method(_SB.INTH._SRS)  */
		} /* End Device(INTH)   */

	}   /* End Scope(_SB)  */


	/* Supported sleep states: */
	Name(\_S0, Package () {0x00, 0x00, 0x00, 0x00} )	/* (S0) - working state */

	If (LAnd(SSFG, 0x01)) {
		Name(\_S1, Package () {0x01, 0x01, 0x00, 0x00} )	/* (S1) - sleeping w/CPU context */
	}
	If (LAnd(SSFG, 0x02)) {
		Name(\_S2, Package () {0x02, 0x02, 0x00, 0x00} )	/* (S2) - "light" Suspend to RAM */
	}
	If (LAnd(SSFG, 0x04)) {
		Name(\_S3, Package () {0x03, 0x03, 0x00, 0x00} )	/* (S3) - Suspend to RAM */
	}
	If (LAnd(SSFG, 0x08)) {
		Name(\_S4, Package () {0x04, 0x04, 0x00, 0x00} )	/* (S4) - Suspend to Disk */
	}

	Name(\_S5, Package () {0x05, 0x05, 0x00, 0x00} )	/* (S5) - Soft Off */

	Name(\_SB.CSPS ,0)				/* Current Sleep State (S0, S1, S2, S3, S4, S5) */
	Name(CSMS, 0)			/* Current System State */

	/* Wake status package */
	Name(WKST,Package(){Zero, Zero})

	/*
	* \_PTS - Prepare to Sleep method
	*
	*	Entry:
	*		Arg0=The value of the sleeping state S1=1, S2=2, etc
	*
	* Exit:
	*		-none-
	*
	* The _PTS control method is executed at the beginning of the sleep process
	* for S1-S5. The sleeping value is passed to the _PTS control method.	This
	* control method may be executed a relatively long time before entering the
	* sleep state and the OS may abort	the operation without notification to
	* the ACPI driver.  This method cannot modify the configuration or power
	* state of any device in the system.
	*/
	Method(\_PTS, 1) {
		/* DBGO("\\_PTS\n") */
		/* DBGO("From S0 to S") */
		/* DBGO(Arg0) */
		/* DBGO("\n") */

		/* Don't allow PCIRST# to reset USB */
		if (LEqual(Arg0,3)){
			Store(0,URRE)
		}

		/* Clear sleep SMI status flag and enable sleep SMI trap. */
		/*Store(One, CSSM)
		Store(One, SSEN)*/

		/* On older chips, clear PciExpWakeDisEn */
		/*if (LLessEqual(\_SB.SBRI, 0x13)) {
		*    	Store(0,\_SB.PWDE)
		*}
		*/

		/* Clear wake status structure. */
		Store(0, Index(WKST,0))
		Store(0, Index(WKST,1))
		\_SB.PCI0.SIOS (Arg0)
	} /* End Method(\_PTS) */

	/*
	*  The following method results in a "not a valid reserved NameSeg"
	*  warning so I have commented it out for the duration.  It isn't
	*  used, so it could be removed.
	*
	*
	*  	\_GTS OEM Going To Sleep method
	*
	*  	Entry:
	*  		Arg0=The value of the sleeping state S1=1, S2=2
	*
	*  	Exit:
	*  		-none-
	*
	*  Method(\_GTS, 1) {
	*  DBGO("\\_GTS\n")
	*  DBGO("From S0 to S")
	*  DBGO(Arg0)
	*  DBGO("\n")
	*  }
	*/

	/*
	*	\_BFS OEM Back From Sleep method
	*
	*	Entry:
	*		Arg0=The value of the sleeping state S1=1, S2=2
	*
	*	Exit:
	*		-none-
	*/
	Method(\_BFS, 1) {
		/* DBGO("\\_BFS\n") */
		/* DBGO("From S") */
		/* DBGO(Arg0) */
		/* DBGO(" to S0\n") */
	}

	/*
	*  \_WAK System Wake method
	*
	*	Entry:
	*		Arg0=The value of the sleeping state S1=1, S2=2
	*
	*	Exit:
	*		Return package of 2 DWords
	*		Dword 1 - Status
	*			0x00000000	wake succeeded
	*			0x00000001	Wake was signaled but failed due to lack of power
	*			0x00000002	Wake was signaled but failed due to thermal condition
	*		Dword 2 - Power Supply state
	*			if non-zero the effective S-state the power supply entered
	*/
	Method(\_WAK, 1) {
		/* DBGO("\\_WAK\n") */
		/* DBGO("From S") */
		/* DBGO(Arg0) */
		/* DBGO(" to S0\n") */

		/* Re-enable HPET */
		Store(1,HPDE)

		/* Restore PCIRST# so it resets USB */
		if (LEqual(Arg0,3)){
			Store(1,URRE)
		}

		/* Arbitrarily clear PciExpWakeStatus */
		Store(PWST, PWST)

		/* if(DeRefOf(Index(WKST,0))) {
		*	Store(0, Index(WKST,1))
		* } else {
		*	Store(Arg0, Index(WKST,1))
		* }
		*/
		\_SB.PCI0.SIOW (Arg0)
		Return(WKST)
	} /* End Method(\_WAK) */

	Scope(\_GPE) {	/* Start Scope GPE */
		/*  General event 0  */
		Method(_L00) {
		      //DBGO("\\_GPE\\_L00\n")
		}

		/*  General event 1  */
		Method(_L01) {
		      //DBGO("\\_GPE\\_L01\n")
		}

		/*  General event 2  */
		Method(_L02) {
		      //DBGO("\\_GPE\\_L02\n")
		}

		/*  General event 3  */
		Method(_L03) {
			//DBGO("\\_GPE\\_L00\n")
			Notify(\_SB.PWRB, 0x02) /* NOTIFY_DEVICE_WAKE */
		}

		/*  General event 4  */
		Method(_L04) {
		      //DBGO("\\_GPE\\_L04\n")
		}

		/*  General event 5  */
		Method(_L05) {
		      //DBGO("\\_GPE\\_L05\n")
		}

		/* _L06 General event 6 - Used for GPM6, moved to USB.asl */
		/* _L07 General event 7 - Used for GPM7, moved to USB.asl */

		/*  Legacy PM event  */
		Method(_L08) {
			//DBGO("\\_GPE\\_L08\n")
		}

		/*  Temp warning (TWarn) event  */
		Method(_L09) {
			//DBGO("\\_GPE\\_L09\n")
			Notify (\_TZ.TZ00, 0x80)
		}

		/*  Reserved  */
		Method(_L0A) {
		      //DBGO("\\_GPE\\_L0A\n")
		}

		/*  USB controller PME#  */
		Method(_L0B) {
			//DBGO("\\_GPE\\_L0B\n")
			Notify(\_SB.PCI0.UOH1, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.UOH2, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.UOH3, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.UOH4, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.UOH5, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.UEH1, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PWRB, 0x02) /* NOTIFY_DEVICE_WAKE */
		}

		/*  AC97 controller PME#  */
		Method(_L0C) {
		      //DBGO("\\_GPE\\_L0C\n")
		}

		/*  OtherTherm PME#  */
		Method(_L0D) {
		      //DBGO("\\_GPE\\_L0D\n")
		}

		/* _L0E GPM9 SCI event - Moved to USB.asl */

		/*  PCIe HotPlug event  */
		Method(_L0F) {
			//DBGO("\\_GPE\\_L0F\n")
		}

		/*  ExtEvent0 SCI event  */
		Method(_L10) {
			//DBGO("\\_GPE\\_L10\n")
		}


		/*  ExtEvent1 SCI event  */
		Method(_L11) {
			//DBGO("\\_GPE\\_L11\n")
		}

		/*  PCIe PME# event  */
		Method(_L12) {
		      //DBGO("\\_GPE\\_L12\n")
		}

		/* _L13 GPM0 SCI event - Moved to USB.asl */
		/* _L14 GPM1 SCI event - Moved to USB.asl */
		/* _L15 GPM2 SCI event - Moved to USB.asl */
		/* _L16 GPM3 SCI event - Moved to USB.asl */
		/* _L17 GPM8 SCI event - Moved to USB.asl */

		/*  GPIO0 or GEvent8 event  */
		Method(_L18) {
			//DBGO("\\_GPE\\_L18\n")
			Notify(\_SB.PCI0.PBR2, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.PBR4, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.PBRb, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.PBRc, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PCI0.PBRd, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PWRB, 0x02) /* NOTIFY_DEVICE_WAKE */
		}

		/* _L19 GPM4 SCI event - Moved to USB.asl */
		/* _L1A GPM5 SCI event - Moved to USB.asl */

		/*  Azalia SCI event  */
		Method(_L1B) {
			//DBGO("\\_GPE\\_L1B\n")
			Notify(\_SB.PCI0.AZHD, 0x02) /* NOTIFY_DEVICE_WAKE */
			Notify(\_SB.PWRB, 0x02) /* NOTIFY_DEVICE_WAKE */
		}

		/*  GPM6 SCI event - Reassigned to _L06 */
		Method(_L1C) {
		      //DBGO("\\_GPE\\_L1C\n")
		}

		/*  GPM7 SCI event - Reassigned to _L07 */
		Method(_L1D) {
		      //DBGO("\\_GPE\\_L1D\n")
		}

		/*  GPIO2 or GPIO66 SCI event  */
		Method(_L1E) {
			//DBGO("\\_GPE\\_L1E\n")
		}

		/* _L1F SATA SCI event - Moved to sata.asl */

	} 	/* End Scope GPE */

	#include "acpi/usb.asl"

	/* System Bus */
	Scope(\_SB) { /* Start \_SB scope */
		#include <arch/x86/acpi/globutil.asl> /* global utility methods expected within the \_SB scope */

		/*  _SB.PCI0 */
		/* Note: Only need HID on Primary Bus */
		Device(PCI0) {
			External (TOM1) //assigned when update_ssdt()
			External (TOM2) /* (<real tom2> >> 20) to make it fit into 32 bit for XP */

			Name(_HID, EISAID("PNP0A03"))
			Name(_ADR, 0x00180000)	/* Dev# = BSP Dev#, Func# = 0 */
			Method(_BBN, 0) { /* Bus number = 0 */
				Return(0)
			}
			Method(_STA, 0) {
				/* DBGO("\\_SB\\PCI0\\_STA\n") */
				Return(0x0B)     /* Status is visible */
			}

			Method(_PRT,0) {
				If(PMOD){ Return(APR0) }	/* APIC mode */
				Return (PR0)			/* PIC Mode */
			} /* end _PRT */

			/* Describe the Northbridge devices */
			Device(AMRT) {
				Name(_ADR, 0x00000000)
			} /* end AMRT */

			/* The external GFX bridge */
			Device(PBR2) {
				Name(_ADR, 0x00020000)
				Name(_PRW, Package() {0x18, 4})
				Method(_PRT,0) {
					If(PMOD){ Return(APS2) }	/* APIC mode */
					Return (PS2)			/* PIC Mode */
				} /* end _PRT */
			} /* end PBR2 */

			/* Dev3 is also an external GFX bridge */

			Device(PBR4) {
				Name(_ADR, 0x00040000)
				Name(_PRW, Package() {0x18, 4})
				Method(_PRT,0) {
					If(PMOD){ Return(APS4) }	/* APIC mode */
					Return (PS4)			/* PIC Mode */
				} /* end _PRT */
			} /* end PBR4 */

			Device(PBRb) {
				Name(_ADR, 0x000b0000)
				Name(_PRW, Package() {0x18, 4})
				Method(_PRT,0) {
					If(PMOD){ Return(APSb) }	/* APIC mode */
					Return (PSb)			/* PIC Mode */
				} /* end _PRT */
			} /* end PBRb */

			Device(PBRc) {
				Name(_ADR, 0x000c0000)
				Name(_PRW, Package() {0x18, 4})
				Method(_PRT,0) {
					If(PMOD){ Return(APSc) }	/* APIC mode */
					Return (PSc)			/* PIC Mode */
				} /* end _PRT */
			} /* end PBRc */

			Device(PBRd) {
				Name(_ADR, 0x000d0000)
				Name(_PRW, Package() {0x18, 4})
				Method(_PRT,0) {
					If(PMOD){ Return(APSd) }	/* APIC mode */
					Return (PSd)			/*  PIC Mode */
				} /* end _PRT */
			} /* end PBRd */

			/* PCI slot 1, 2, 3 */
			Device(PIBR) {
				Name(_ADR, 0x00140004)
				Name(_PRW, Package() {0x18, 4})

				Method(_PRT, 0) {
					Return (PCIB)
				}
			}

			/* Describe the Southbridge devices */
			Device(STCR) {
				Name(_ADR, 0x00110000)
				#include "acpi/sata.asl"
			} /* end STCR */

			Device(UOH1) {
				Name(_ADR, 0x00130000)
				Name(_PRW, Package() {0x0B, 3})
			} /* end UOH1 */

			Device(UOH2) {
				Name(_ADR, 0x00130001)
				Name(_PRW, Package() {0x0B, 3})
			} /* end UOH2 */

			Device(UOH3) {
				Name(_ADR, 0x00130002)
				Name(_PRW, Package() {0x0B, 3})
			} /* end UOH3 */

			Device(UOH4) {
				Name(_ADR, 0x00130003)
				Name(_PRW, Package() {0x0B, 3})
			} /* end UOH4 */

			Device(UOH5) {
				Name(_ADR, 0x00130004)
				Name(_PRW, Package() {0x0B, 3})
			} /* end UOH5 */

			Device(UEH1) {
				Name(_ADR, 0x00130005)
				Name(_PRW, Package() {0x0B, 3})
			} /* end UEH1 */

			Device(SBUS) {
				Name(_ADR, 0x00140000)
			} /* end SBUS */

			/* Primary (and only) IDE channel */
			Device(IDEC) {
				Name(_ADR, 0x00140001)
				#include "acpi/ide.asl"
			} /* end IDEC */

			Device(AZHD) {
				Name(_ADR, 0x00140002)
				OperationRegion(AZPD, PCI_Config, 0x00, 0x100)
					Field(AZPD, AnyAcc, NoLock, Preserve) {
					offset (0x42),
					NSDI, 1,
					NSDO, 1,
					NSEN, 1,
					offset (0x44),
					IPCR, 4,
					offset (0x54),
					PWST, 2,
					, 6,
					PMEB, 1,
					, 6,
					PMST, 1,
					offset (0x62),
					MMCR, 1,
					offset (0x64),
					MMLA, 32,
					offset (0x68),
					MMHA, 32,
					offset (0x6C),
					MMDT, 16,
				}

				Method(_INI) {
					If(LEqual(OSTP,3)){   /* If we are running Linux */
						Store(zero, NSEN)
						Store(one, NSDO)
						Store(one, NSDI)
					}
				}
			} /* end AZHD */

			Device(LIBR) {
				Name(_ADR, 0x00140003)
				/* Method(_INI) {
				*	DBGO("\\_SB\\PCI0\\LpcIsaBr\\_INI\n")
				} */ /* End Method(_SB.SBRDG._INI) */

				/* Real Time Clock Device */
				Device(RTC0) {
					Name(_HID, EISAID("PNP0B00"))	/* AT Real Time Clock (not PIIX4 compatible) */
					Name(_CRS, ResourceTemplate() {
						IRQNoFlags(){8}
						IO(Decode16,0x0070, 0x0070, 0, 2)
						/* IO(Decode16,0x0070, 0x0070, 0, 4) */
					})
				} /* End Device(_SB.PCI0.LpcIsaBr.RTC0) */

				Device(TMR) {	/* Timer */
					Name(_HID,EISAID("PNP0100"))	/* System Timer */
					Name(_CRS, ResourceTemplate() {
						IRQNoFlags(){0}
						IO(Decode16, 0x0040, 0x0040, 0, 4)
						/* IO(Decode16, 0x0048, 0x0048, 0, 4) */
					})
				} /* End Device(_SB.PCI0.LpcIsaBr.TMR) */

				Device(SPKR) {	/* Speaker */
					Name(_HID,EISAID("PNP0800"))	/* AT style speaker */
					Name(_CRS, ResourceTemplate() {
						IO(Decode16, 0x0061, 0x0061, 0, 1)
					})
				} /* End Device(_SB.PCI0.LpcIsaBr.SPKR) */

				Device(PIC) {
					Name(_HID,EISAID("PNP0000"))	/* AT Interrupt Controller */
					Name(_CRS, ResourceTemplate() {
						IRQNoFlags(){2}
						IO(Decode16,0x0020, 0x0020, 0, 2)
						IO(Decode16,0x00A0, 0x00A0, 0, 2)
						/* IO(Decode16, 0x00D0, 0x00D0, 0x10, 0x02) */
						/* IO(Decode16, 0x04D0, 0x04D0, 0x10, 0x02) */
					})
				} /* End Device(_SB.PCI0.LpcIsaBr.PIC) */

				Device(MAD) { /* 8257 DMA */
					Name(_HID,EISAID("PNP0200"))	/* Hardware Device ID */
					Name(_CRS, ResourceTemplate() {
						DMA(Compatibility,BusMaster,Transfer8){4}
						IO(Decode16, 0x0000, 0x0000, 0x10, 0x10)
						IO(Decode16, 0x0081, 0x0081, 0x01, 0x03)
						IO(Decode16, 0x0087, 0x0087, 0x01, 0x01)
						IO(Decode16, 0x0089, 0x0089, 0x01, 0x03)
						IO(Decode16, 0x008F, 0x008F, 0x01, 0x01)
						IO(Decode16, 0x00C0, 0x00C0, 0x10, 0x20)
					}) /* End Name(_SB.PCI0.LpcIsaBr.MAD._CRS) */
				} /* End Device(_SB.PCI0.LpcIsaBr.MAD) */

				Device(COPR) {
					Name(_HID,EISAID("PNP0C04"))	/* Math Coprocessor */
					Name(_CRS, ResourceTemplate() {
						IO(Decode16, 0x00F0, 0x00F0, 0, 0x10)
						IRQNoFlags(){13}
					})
				} /* End Device(_SB.PCI0.LpcIsaBr.COPR) */

				Device (PS2M) {
					Name (_HID, EisaId ("PNP0F13"))
					Name (_CRS, ResourceTemplate () {
						IO (Decode16, 0x0060, 0x0060, 0x00, 0x01)
						IO (Decode16, 0x0064, 0x0064, 0x00, 0x01)
						IRQNoFlags () {12}
					})
					Method (_STA, 0, NotSerialized) {
						And (FLG0, 0x04, Local0)
						If (LEqual (Local0, 0x04)) {
							Return (0x0F)
						} Else {
							Return (0x00)
						}
					}
				}

				Device (PS2K) {
					Name (_HID, EisaId ("PNP0303"))
					Method (_STA, 0, NotSerialized) {
						And (FLG0, 0x04, Local0)
						If (LEqual (Local0, 0x04)) {
							Return (0x0F)
						} Else {
							Return (0x00)
						}
					}
					Name (_CRS, ResourceTemplate () {
						IO (Decode16, 0x0060, 0x0060, 0x00, 0x01)
						IO (Decode16, 0x0064, 0x0064, 0x00, 0x01)
						IRQNoFlags () {1}
					})
				}

#if 0 //acpi_create_hpet
				Device(HPET) {
					Name(_HID,EISAID("PNP0103"))
					Name(CRS, ResourceTemplate() {
		    				IRQNoFlags () {0}
		    				IRQNoFlags () {2}
		   			 	IRQNoFlags () {8}
						Memory32Fixed(ReadOnly,0xFED00000, 0x00000400, MNT)	/* 1kb reserved space */
					})
					Method(_STA, 0, NotSerialized) {
						Return(0x0F) /* sata is visible */
					}
					Method(_CRS, 0, NotSerialized) {
						CreateDwordField(CRS, ^MNT._BAS, HPT)
						Store(HPBA, HPT)
						Return(CRS)
					}
				} /* End Device(_SB.PCI0.LIBR.HPET) */
#endif
			} /* end LIBR */

			Device(HPBR) {
				Name(_ADR, 0x00140004)
			} /* end HostPciBr */

			Device(ACAD) {
				Name(_ADR, 0x00140005)
			} /* end Ac97audio */

			Device(ACMD) {
				Name(_ADR, 0x00140006)
			} /* end Ac97modem */

			/* ITE8718 Support */
			OperationRegion (IOID, SystemIO, 0x2E, 0x02)	/* sometimes it is 0x4E */
				Field (IOID, ByteAcc, NoLock, Preserve)
				{
					SIOI,   8,    SIOD,   8		/* 0x2E and 0x2F */
				}

			IndexField (SIOI, SIOD, ByteAcc, NoLock, Preserve)
			{
					Offset (0x07),
				LDN,	8,	/* Logical Device Number */
					Offset (0x20),
				CID1,	8,	/* Chip ID Byte 1, 0x87 */
				CID2,	8,	/* Chip ID Byte 2, 0x12 */
					Offset (0x30),
				ACTR,	8,	/* Function activate */
					Offset (0xF0),
				APC0,	8,	/* APC/PME Event Enable Register */
				APC1,	8,	/* APC/PME Status Register */
				APC2,	8,	/* APC/PME Control Register 1 */
				APC3,	8,	/* Environment Controller Special Configuration Register */
				APC4,	8	/* APC/PME Control Register 2 */
			}

			/* Enter the 8718 MB PnP Mode */
			Method (EPNP)
			{
				Store(0x87, SIOI)
				Store(0x01, SIOI)
				Store(0x55, SIOI)
				Store(0x55, SIOI) /* 8718 magic number */
			}
			/* Exit the 8718 MB PnP Mode */
			Method (XPNP)
			{
				Store (0x02, SIOI)
				Store (0x02, SIOD)
			}
			/*
			 * Keyboard PME is routed to SB700 Gevent3. We can wake
			 * up the system by pressing the key.
			 */
			Method (SIOS, 1)
			{
				/* We only enable KBD PME for S5. */
				If (LLess (Arg0, 0x05))
				{
					EPNP()
					/* DBGO("8718F\n") */

					Store (0x4, LDN)
					Store (One, ACTR)  /* Enable EC */
					/*
					Store (0x4, LDN)
					Store (0x04, APC4)
					*/  /* falling edge. which mode? Not sure. */

					Store (0x4, LDN)
					Store (0x08, APC1) /* clear PME status, Use 0x18 for mouse & KBD */
					Store (0x4, LDN)
					Store (0x08, APC0) /* enable PME, Use 0x18 for mouse & KBD */

					XPNP()
				}
			}
			Method (SIOW, 1)
			{
				EPNP()
				Store (0x4, LDN)
				Store (Zero, APC0) /* disable keyboard PME */
				Store (0x4, LDN)
				Store (0xFF, APC1) /* clear keyboard PME status */
				XPNP()
			}

			Name (CRS, ResourceTemplate ()
			{
				WordBusNumber (ResourceProducer, MinFixed, MaxFixed, PosDecode,
					0x0000,             // Granularity
					0x0000,             // Range Minimum
					0x00FF,             // Range Maximum
					0x0000,             // Translation Offset
					0x0100,             // Length
					,,)
				IO (Decode16,
					0x0CF8,             // Range Minimum
					0x0CF8,             // Range Maximum
					0x01,               // Alignment
					0x08,               // Length
					)

				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000,             // Granularity
					0x0000,             // Range Minimum
					0x03AF,             // Range Maximum
					0x0000,             // Translation Offset
					0x03B0,             // Length
					,, , TypeStatic)
				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000,             // Granularity
					0x03E0,             // Range Minimum
					0x0CF7,             // Range Maximum
					0x0000,             // Translation Offset
					0x0918,             // Length
					,, , TypeStatic)

				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000,             // Granularity
					0x03B0,             // Range Minimum
					0x03BB,             // Range Maximum
					0x0000,             // Translation Offset
					0x000C,             // Length
					,, , TypeStatic)
				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000,             // Granularity
					0x03C0,             // Range Minimum
					0x03DF,             // Range Maximum
					0x0000,             // Translation Offset
					0x0020,             // Length
					,, , TypeStatic)
				WordIO (ResourceProducer, MinFixed, MaxFixed, PosDecode, EntireRange,
					0x0000,             // Granularity
					0x0D00,             // Range Minimum
					0xFFFF,             // Range Maximum
					0x0000,             // Translation Offset
					0xF300,             // Length
					,, , TypeStatic)
				Memory32Fixed(READONLY, 0x000A0000, 0x00020000, VGAM) 	// VGA memory space

				Memory32Fixed (ReadOnly,
						0xE0000000,         // Address Base
						0x10000000,         // Address Length, (1MB each Bus, 256 Buses by default)
						MMIO)
			})

			Method (_CRS, 0, NotSerialized)
			{
				CreateDWordField (CRS, \_SB.PCI0.MMIO._BAS, BAS1)
				CreateDWordField (CRS, \_SB.PCI0.MMIO._LEN, LEN1)

                                /*
                                 * Declare memory between TOM1 and 4GB as available
                                 * for PCI MMIO.
                                 * Use ShiftLeft to avoid 64bit constant (for XP).
                                 * This will work even if the OS does 32bit arithmetic, as
                                 * 32bit (0x00000000 - TOM1) will wrap and give the same
                                 * result as 64bit (0x100000000 - TOM1).
                                 */
				Store(TOM1, BAS1)
                                ShiftLeft(0x10000000, 4, Local0)
                                Subtract(Local0, TOM1, Local0)
                                Store(Local0, LEN1)
				//DBGO(TOM1)

				Return (CRS)
			}

			/*
			 *
			 *               FIRST METHOD CALLED UPON BOOT
			 *
			 *  1. If debugging, print current OS and ACPI interpreter.
			 *  2. Get PCI Interrupt routing from ACPI VSM, this
			 *     value is based on user choice in BIOS setup.
			 */
			Method(_INI, 0) {
				/* DBGO("\\_SB\\_INI\n") */
				/* DBGO("   DSDT.ASL code from ") */
				/* DBGO(__DATE__) */
				/* DBGO(" ") */
				/* DBGO(__TIME__) */
				/* DBGO("\n   Sleep states supported: ") */
				/* DBGO("\n") */
				/* DBGO("   \\_OS=") */
				/* DBGO(\_OS) */
				/* DBGO("\n   \\_REV=") */
				/* DBGO(\_REV) */
				/* DBGO("\n") */

				/* Determine the OS we're running on */
				CkOT()
				/* On older chips, clear PciExpWakeDisEn */
				/*if (LLessEqual(\SBRI, 0x13)) {
				 *    	Store(0,\PWDE)
				 *}
				 */
			} /* End Method(_SB._INI) */
		} /* End Device(PCI0)  */

		Device(PWRB) {	/* Start Power button device */
			Name(_HID, EISAID("PNP0C0C"))
			Name(_UID, 0xAA)
			Name(_PRW, Package () {3, 0x04})	/* wake from S1-S4 */
			Name(_STA, 0x0B) /* sata is invisible */
		}
	} /* End \_SB scope */

	Scope(\_SI) {
		Method(_SST, 1) {
			/* DBGO("\\_SI\\_SST\n") */
			/* DBGO("   New Indicator state: ") */
			/* DBGO(Arg0) */
			/* DBGO("\n") */
		}
	} /* End Scope SI */

	/* SMBUS Support */
	Mutex (SBX0, 0x00)
	OperationRegion (SMB0, SystemIO, 0xB00, 0x0C)
		Field (SMB0, ByteAcc, NoLock, Preserve) {
			HSTS,   8, /* SMBUS status */
			SSTS,   8,  /* SMBUS slave status */
			HCNT,   8,  /* SMBUS control */
			HCMD,   8,  /* SMBUS host cmd */
			HADD,   8,  /* SMBUS address */
			DAT0,   8,  /* SMBUS data0 */
			DAT1,   8,  /* SMBUS data1 */
			BLKD,   8,  /* SMBUS block data */
			SCNT,   8,  /* SMBUS slave control */
			SCMD,   8,  /* SMBUS shaow cmd */
			SEVT,   8,  /* SMBUS slave event */
			SDAT,   8  /* SMBUS slave data */
	}

	Method (WCLR, 0, NotSerialized) { /* clear SMBUS status register */
		Store (0x1E, HSTS)
		Store (0xFA, Local0)
		While (LAnd (LNotEqual (And (HSTS, 0x1E), Zero), LGreater (Local0, Zero))) {
			Stall (0x64)
			Decrement (Local0)
		}

		Return (Local0)
	}

	Method (SWTC, 1, NotSerialized) {
		Store (Arg0, Local0)
		Store (0x07, Local2)
		Store (One, Local1)
		While (LEqual (Local1, One)) {
			Store (And (HSTS, 0x1E), Local3)
			If (LNotEqual (Local3, Zero)) { /* read sucess */
				If (LEqual (Local3, 0x02)) {
					Store (Zero, Local2)
				}

				Store (Zero, Local1)
			}
			Else {
				If (LLess (Local0, 0x0A)) { /* read failure */
					Store (0x10, Local2)
					Store (Zero, Local1)
				}
				Else {
					Sleep (0x0A) /* 10 ms, try again */
					Subtract (Local0, 0x0A, Local0)
				}
			}
		}

		Return (Local2)
	}

	Method (SMBR, 3, NotSerialized) {
		Store (0x07, Local0)
		If (LEqual (Acquire (SBX0, 0xFFFF), Zero)) {
			Store (WCLR (), Local0) /* clear SMBUS status register before read data */
			If (LEqual (Local0, Zero)) {
				Release (SBX0)
				Return (0x0)
			}

			Store (0x1F, HSTS)
			Store (Or (ShiftLeft (Arg1, One), One), HADD)
			Store (Arg2, HCMD)
			If (LEqual (Arg0, 0x07)) {
				Store (0x48, HCNT) /* read byte */
			}

			Store (SWTC (0x03E8), Local1) /* 1000 ms */
			If (LEqual (Local1, Zero)) {
				If (LEqual (Arg0, 0x07)) {
					Store (DAT0, Local0)
				}
			}
			Else {
				Store (Local1, Local0)
			}

			Release (SBX0)
		}

		/* DBGO("the value of SMBusData0 register ") */
		/* DBGO(Arg2) */
		/* DBGO(" is ") */
		/* DBGO(Local0) */
		/* DBGO("\n") */

		Return (Local0)
	}

	/* THERMAL */
	Scope(\_TZ) {
		Name (KELV, 2732)
		Name (THOT, 800)
		Name (TCRT, 850)

		ThermalZone(TZ00) {
			Method(_AC0,0) {	/* Active Cooling 0 (0=highest fan speed) */
				/* DBGO("\\_TZ\\TZ00\\_AC0\n") */
				Return(Add(0, 2730))
			}
			Method(_AL0,0) {	/* Returns package of cooling device to turn on */
				/* DBGO("\\_TZ\\TZ00\\_AL0\n") */
				Return(Package() {\_TZ.TZ00.FAN0})
			}
			Device (FAN0) {
				Name(_HID, EISAID("PNP0C0B"))
				Name(_PR0, Package() {PFN0})
			}

			PowerResource(PFN0,0,0) {
				Method(_STA) {
					Store(0xF,Local0)
					Return(Local0)
				}
				Method(_ON) {
					/* DBGO("\\_TZ\\TZ00\\FAN0 _ON\n") */
				}
				Method(_OFF) {
					/* DBGO("\\_TZ\\TZ00\\FAN0 _OFF\n") */
				}
			}

			Method(_HOT,0) {	/* return hot temp in tenths degree Kelvin */
				/* DBGO("\\_TZ\\TZ00\\_HOT\n") */
				Return (Add (THOT, KELV))
			}
			Method(_CRT,0) {	/* return critical temp in tenths degree Kelvin */
				/* DBGO("\\_TZ\\TZ00\\_CRT\n") */
				Return (Add (TCRT, KELV))
			}
			Method(_TMP,0) {	/* return current temp of this zone */
				Store (SMBR (0x07, 0x4C,, 0x00), Local0)
				If (LGreater (Local0, 0x10)) {
					Store (Local0, Local1)
				}
				Else {
					Add (Local0, THOT, Local0)
					Return (Add (400, KELV))
				}

				Store (SMBR (0x07, 0x4C, 0x01), Local0)
				/* only the two MSBs in the external temperature low byte are used, resolution 0.25. We ignore it */
				/* Store (SMBR (0x07, 0x4C, 0x10), Local2) */
				If (LGreater (Local0, 0x10)) {
					If (LGreater (Local0, Local1)) {
						Store (Local0, Local1)
					}

					Multiply (Local1, 10, Local1)
					Return (Add (Local1, KELV))
				}
				Else {
					Add (Local0, THOT, Local0)
					Return (Add (400 , KELV))
				}
			} /* end of _TMP */
		} /* end of TZ00 */
	}
}
/* End of ASL file */
