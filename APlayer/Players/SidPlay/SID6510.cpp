/******************************************************************************/
/* SID6510 implementation file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Player headers
#include "SIDTune.h"
#include "SIDEmuEngine.h"
#include "SID6510.h"


/******************************************************************************/
/* Some handy defines to ease SR access.                                      */
/******************************************************************************/
#define CF SR.Carry
#define ZF SR.Zero
#define IF SR.Interrupt
#define DF SR.Decimal
#define BF SR.Break
#define NU SR.notUsed
#define VF SR.oVerflow
#define NF SR.Negative



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
SID6510::SID6510(void)
{
	// Initialize member variables
	c64RomBuf     = NULL;
	c64RamBuf     = NULL;

	c64Mem1       = NULL;
	c64Mem2       = NULL;

	memoryMode    = MPU_TRANSPARENT_ROM;

	stackIsOkay   = true;

	sidLastValue  = 0;
	optr3ReadWave = 0;
	optr3ReadEnve = 0;

	readData      = ReadData_bs;
	writeData     = WriteData_bs;

	// Initialize function pointers
	instrList[0x00] = BRK_;
	instrList[0x01] = ORA_indx;
	instrList[0x02] = ILL_TILT;
	instrList[0x03] = ASLORA_indx;
	instrList[0x04] = ILL_2NOP;
	instrList[0x05] = ORA_zp;
	instrList[0x06] = ASL_zp;
	instrList[0x07] = ASLORA_zp;
	instrList[0x08] = PHP_;
	instrList[0x09] = ORA_imm;
	instrList[0x0A] = ASL_AC;
	instrList[0x0B] = ILL_0B;
	instrList[0x0C] = ILL_3NOP;
	instrList[0x0D] = ORA_abso;
	instrList[0x0E] = ASL_abso;
	instrList[0x0F] = ASLORA_abso;

	instrList[0x10] = BPL_;
	instrList[0x11] = ORA_indy;
	instrList[0x12] = ILL_TILT;
	instrList[0x13] = ASLORA_indy;
	instrList[0x14] = ILL_2NOP;
	instrList[0x15] = ORA_zpx;
	instrList[0x16] = ASL_zpx;
	instrList[0x17] = ASLORA_zpx;
	instrList[0x18] = CLC_;
	instrList[0x19] = ORA_absy;
	instrList[0x1A] = ILL_1NOP;
	instrList[0x1B] = ASLORA_absy;
	instrList[0x1C] = ILL_3NOP;
	instrList[0x1D] = ORA_absx;
	instrList[0x1E] = ASL_absx;
	instrList[0x1F] = ASLORA_absx;

	instrList[0x20] = JSR_;
	instrList[0x21] = AND_indx;
	instrList[0x22] = ILL_TILT;
	instrList[0x23] = ROLAND_indx;
	instrList[0x24] = BIT_zp;
	instrList[0x25] = AND_zp;
	instrList[0x26] = ROL_zp;
	instrList[0x27] = ROLAND_zp;
	instrList[0x28] = PLP_;
	instrList[0x29] = AND_imm;
	instrList[0x2A] = ROL_AC;
	instrList[0x2B] = ILL_0B;
	instrList[0x2C] = BIT_abso;
	instrList[0x2D] = AND_abso;
	instrList[0x2E] = ROL_abso;
	instrList[0x2F] = ROLAND_abso;

	instrList[0x30] = BMI_;
	instrList[0x31] = AND_indy;
	instrList[0x32] = ILL_TILT;
	instrList[0x33] = ROLAND_indy;
	instrList[0x34] = ILL_2NOP;
	instrList[0x35] = AND_zpx;
	instrList[0x36] = ROL_zpx;
	instrList[0x37] = ROLAND_zpx;
	instrList[0x38] = SEC_;
	instrList[0x39] = AND_absy;
	instrList[0x3A] = ILL_1NOP;
	instrList[0x3B] = ROLAND_absy;
	instrList[0x3C] = ILL_3NOP;
	instrList[0x3D] = AND_absx;
	instrList[0x3E] = ROL_absx;
	instrList[0x3F] = ROLAND_absx;

	instrList[0x40] = RTI_;
	instrList[0x41] = EOR_indx;
	instrList[0x42] = ILL_TILT;
	instrList[0x43] = LSREOR_indx;
	instrList[0x44] = ILL_2NOP;
	instrList[0x45] = EOR_zp;
	instrList[0x46] = LSR_zp;
	instrList[0x47] = LSREOR_zp;
	instrList[0x48] = PHA_;
	instrList[0x49] = EOR_imm;
	instrList[0x4A] = LSR_AC;
	instrList[0x4B] = ILL_4B;
	instrList[0x4C] = JMP_;
	instrList[0x4D] = EOR_abso;
	instrList[0x4E] = LSR_abso;
	instrList[0x4F] = LSREOR_abso;

	instrList[0x50] = BVC_;
	instrList[0x51] = EOR_indy;
	instrList[0x52] = ILL_TILT;
	instrList[0x53] = LSREOR_indy;
	instrList[0x54] = ILL_2NOP;
	instrList[0x55] = EOR_zpx;
	instrList[0x56] = LSR_zpx;
	instrList[0x57] = LSREOR_zpx;
	instrList[0x58] = CLI_;
	instrList[0x59] = EOR_absy;
	instrList[0x5A] = ILL_1NOP;
	instrList[0x5B] = LSREOR_absy;
	instrList[0x5C] = ILL_3NOP;
	instrList[0x5D] = EOR_absx;
	instrList[0x5E] = LSR_absx;
	instrList[0x5F] = LSREOR_absx;

	instrList[0x60] = RTS_;
	instrList[0x61] = ADC_indx;
	instrList[0x62] = ILL_TILT;
	instrList[0x63] = RORADC_indx;
	instrList[0x64] = ILL_2NOP;
	instrList[0x65] = ADC_zp;
	instrList[0x66] = ROR_zp;
	instrList[0x67] = RORADC_zp;
	instrList[0x68] = PLA_;
	instrList[0x69] = ADC_imm;
	instrList[0x6A] = ROR_AC;
	instrList[0x6B] = ILL_6B;
	instrList[0x6C] = JMP_vec;
	instrList[0x6D] = ADC_abso;
	instrList[0x6E] = ROR_abso;
	instrList[0x6F] = RORADC_abso;

	instrList[0x70] = BVS_;
	instrList[0x71] = ADC_indy;
	instrList[0x72] = ILL_TILT;
	instrList[0x73] = RORADC_indy;
	instrList[0x74] = ILL_2NOP;
	instrList[0x75] = ADC_zpx;
	instrList[0x76] = ROR_zpx;
	instrList[0x77] = RORADC_zpx;
	instrList[0x78] = SEI_;
	instrList[0x79] = ADC_absy;
	instrList[0x7A] = ILL_1NOP;
	instrList[0x7B] = RORADC_absy;
	instrList[0x7C] = ILL_3NOP;
	instrList[0x7D] = ADC_absx;
	instrList[0x7E] = ROR_absx;
	instrList[0x7F] = RORADC_absx;

	instrList[0x80] = ILL_2NOP;
	instrList[0x81] = STA_indx;
	instrList[0x82] = ILL_2NOP;
	instrList[0x83] = ILL_83;
	instrList[0x84] = STY_zp;
	instrList[0x85] = STA_zp;
	instrList[0x86] = STX_zp;
	instrList[0x87] = ILL_87;
	instrList[0x88] = DEY_;
	instrList[0x89] = ILL_2NOP;
	instrList[0x8A] = TXA_;
	instrList[0x8B] = ILL_8B;
	instrList[0x8C] = STY_abso;
	instrList[0x8D] = STA_abso;
	instrList[0x8E] = STX_abso;
	instrList[0x8F] = ILL_8F;

	instrList[0x90] = BCC_;
	instrList[0x91] = STA_indy;
	instrList[0x92] = ILL_TILT;
	instrList[0x93] = ILL_93;
	instrList[0x94] = STY_zpx;
	instrList[0x95] = STA_zpx;
	instrList[0x96] = STX_zpy;
	instrList[0x97] = ILL_97;
	instrList[0x98] = TYA_;
	instrList[0x99] = STA_absy;
	instrList[0x9A] = TXS_;
	instrList[0x9B] = ILL_9B;
	instrList[0x9C] = ILL_9C;
	instrList[0x9D] = STA_absx;
	instrList[0x9E] = ILL_9E;
	instrList[0x9F] = ILL_9F;

	instrList[0xA0] = LDY_imm;
	instrList[0xA1] = LDA_indx;
	instrList[0xA2] = LDX_imm;
	instrList[0xA3] = ILL_A3;
	instrList[0xA4] = LDY_zp;
	instrList[0xA5] = LDA_zp;
	instrList[0xA6] = LDX_zp;
	instrList[0xA7] = ILL_A7;
	instrList[0xA8] = TAY_;
	instrList[0xA9] = LDA_imm;
	instrList[0xAA] = TAX_;
	instrList[0xAB] = ILL_AB;
	instrList[0xAC] = LDY_abso;
	instrList[0xAD] = LDA_abso;
	instrList[0xAE] = LDX_abso;
	instrList[0xAF] = ILL_AF;

	instrList[0xB0] = BCS_;
	instrList[0xB1] = LDA_indy;
	instrList[0xB2] = ILL_TILT;
	instrList[0xB3] = ILL_B3;
	instrList[0xB4] = LDY_zpx;
	instrList[0xB5] = LDA_zpx;
	instrList[0xB6] = LDX_zpy;
	instrList[0xB7] = ILL_B7;
	instrList[0xB8] = CLV_;
	instrList[0xB9] = LDA_absy;
	instrList[0xBA] = TSX_;
	instrList[0xBB] = ILL_BB;
	instrList[0xBC] = LDY_absx;
	instrList[0xBD] = LDA_absx;
	instrList[0xBE] = LDX_absy;
	instrList[0xBF] = ILL_BF;

	instrList[0xC0] = CPY_imm;
	instrList[0xC1] = CMP_indx;
	instrList[0xC2] = ILL_2NOP;
	instrList[0xC3] = DECCMP_indx;
	instrList[0xC4] = CPY_zp;
	instrList[0xC5] = CMP_zp;
	instrList[0xC6] = DEC_zp;
	instrList[0xC7] = DECCMP_zp;
	instrList[0xC8] = INY_;
	instrList[0xC9] = CMP_imm;
	instrList[0xCA] = DEX_;
	instrList[0xCB] = ILL_CB;
	instrList[0xCC] = CPY_abso;
	instrList[0xCD] = CMP_abso;
	instrList[0xCE] = DEC_abso;
	instrList[0xCF] = DECCMP_abso;

	instrList[0xD0] = BNE_;
	instrList[0xD1] = CMP_indy;
	instrList[0xD2] = ILL_TILT;
	instrList[0xD3] = DECCMP_indy;
	instrList[0xD4] = ILL_2NOP;
	instrList[0xD5] = CMP_zpx;
	instrList[0xD6] = DEC_zpx;
	instrList[0xD7] = DECCMP_zpx;
	instrList[0xD8] = CLD_;
	instrList[0xD9] = CMP_absy;
	instrList[0xDA] = ILL_1NOP;
	instrList[0xDB] = DECCMP_absy;
	instrList[0xDC] = ILL_3NOP;
	instrList[0xDD] = CMP_absx;
	instrList[0xDE] = DEC_absx;
	instrList[0xDF] = DECCMP_absx;

	instrList[0xE0] = CPX_imm;
	instrList[0xE1] = SBC_indx;
	instrList[0xE2] = ILL_2NOP;
	instrList[0xE3] = INCSBC_indx;
	instrList[0xE4] = CPX_zp;
	instrList[0xE5] = SBC_zp;
	instrList[0xE6] = INC_zp;
	instrList[0xE7] = INCSBC_zp;
	instrList[0xE8] = INX_;
	instrList[0xE9] = SBC_imm;
	instrList[0xEA] = NOP_;
	instrList[0xEB] = ILL_EB;
	instrList[0xEC] = CPX_abso;
	instrList[0xED] = SBC_abso;
	instrList[0xEE] = INC_abso;
	instrList[0xEF] = INCSBC_abso;

	instrList[0xF0] = BEQ_;
	instrList[0xF1] = SBC_indy;
	instrList[0xF2] = ILL_TILT;
	instrList[0xF3] = INCSBC_indy;
	instrList[0xF4] = ILL_2NOP;
	instrList[0xF5] = SBC_zpx;
	instrList[0xF6] = INC_zpx;
	instrList[0xF7] = INCSBC_zpx;
	instrList[0xF8] = SED_;
	instrList[0xF9] = SBC_absy;
	instrList[0xFA] = ILL_1NOP;
	instrList[0xFB] = INCSBC_absy;
	instrList[0xFC] = ILL_3NOP;
	instrList[0xFD] = SBC_absx;
	instrList[0xFE] = INC_absx;
	instrList[0xFF] = INCSBC_absx;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
SID6510::~SID6510(void)
{
}



/******************************************************************************/
/* C64MemAlloc()                                                              */
/******************************************************************************/
bool SID6510::C64MemAlloc(void)
{
	bool wasSuccess = true;

	C64MemFree();

	if ((c64RamBuf = new uint8[65536 + 256]) == NULL)
		wasSuccess = false;

	if ((c64RomBuf = new uint8[65536 + 256]) == NULL)
		wasSuccess = false;

	if (!wasSuccess)
		C64MemFree();
	else
	{
		// Make the memory buffers accessible to the whole emulator engine
		c64Mem1 = c64RamBuf;
		c64Mem2 = c64RomBuf;
	}

	return (wasSuccess);
}



/******************************************************************************/
/* C64MemFree()                                                               */
/******************************************************************************/
bool SID6510::C64MemFree(void)
{
	if (c64RomBuf != NULL)
	{
		delete[] c64RomBuf;
		c64RomBuf = (c64Mem2 = NULL);
	}

	if (c64RamBuf != NULL)
	{
		delete[] c64RamBuf;
		c64RamBuf = (c64Mem1 = NULL);
	}

	return (true);
}



/******************************************************************************/
/* C64MemClear()                                                              */
/******************************************************************************/
void SID6510::C64MemClear(void)
{ 
	// Clear entire RAM and ROM
	for (uint32 i = 0; i < 0x10000; i++)
	{
		c64Mem1[i] = 0;

		if (memoryMode != MPU_PLAYSID_ENVIRONMENT)
			c64Mem2[i] = 0;
	}

	sidLastValue = 0;

	if (memoryMode == MPU_PLAYSID_ENVIRONMENT)
	{
		// Fill Kernal-ROM address space with RTI instructions
		for (uint32 j = 0xE000; j < 0x10000; j++)
			c64Mem1[j] = 0x40;
	}
	else
	{
		// Fill Basic-ROM address space with RTS instructions
		for (uint32 j1 = 0xA000; j1 < 0xC000; j1++)
			c64Mem2[j1] = 0x60;

		// Fill Kernal-ROM address space with RTI instructions
		for (uint32 j2 = 0xE000; j2 < 0x10000; j2++)
			c64Mem2[j2] = 0x40;
	}
}



/******************************************************************************/
/* C64MemReset()                                                              */
/******************************************************************************/
void SID6510::C64MemReset(int32 clockSpeed, uint8 randomSeed)
{
	fakeReadTimer += randomSeed;

	if ((c64Mem1 != NULL) && (c64Mem2 != NULL))
	{
		c64Mem1[0] = 0x2F;

		// Defaults: Basic-ROM on, Kernal-ROM on, I/O on
		c64Mem1[1] = 0x07;
		EvalBankSelect();

		// CIA-Timer A $DC04/5 = $4025 PAL, $4295 NTSC
		if (clockSpeed == SIDTUNE_CLOCK_NTSC)
		{
			c64Mem1[0x02a6] = 0;		// NTSC
			c64Mem2[0xdc04] = 0x95;
			c64Mem2[0xdc05] = 0x42;
		}
		else	// if (clockSpeed == SIDTUNE_CLOCK_PAL)
		{
			c64Mem1[0x02a6] = 1;		// PAL
			c64Mem2[0xdc04] = 0x25;
			c64Mem2[0xdc05] = 0x40;
		}

		// Fake VBI-interrupts that do $D019, NMI ...
		c64Mem2[0xd019] = 0xff;

		// Software vectors
		// IRQ to $EA31
		c64Mem1[0x0314] = 0x31;
		c64Mem1[0x0315] = 0xea;

		// BRK to $FE66
		c64Mem1[0x0316] = 0x66;
		c64Mem1[0x0317] = 0xfe;

		// NMI to $FE47
		c64Mem1[0x0318] = 0x47;
		c64Mem1[0x0319] = 0xfe;

		// Hardware vectors
		if (memoryMode == MPU_PLAYSID_ENVIRONMENT)
		{
			c64Mem1[0xff48] = 0x6c;
			c64Mem1[0xff49] = 0x14;
			c64Mem1[0xff4a] = 0x03;
			c64Mem1[0xfffa] = 0xf8;
			c64Mem1[0xfffb] = 0xff;
			c64Mem1[0xfffe] = 0x48;
			c64Mem1[0xffff] = 0xff;
		}
		else
		{
			// NMI to $FE43
			c64Mem1[0xfffa] = 0x43;
			c64Mem1[0xfffb] = 0xfe;

			// RESET to $FCE2
			c64Mem1[0xfffc] = 0xe2;
			c64Mem1[0xfffd] = 0xfc;

			// IRQ to $FF48
			c64Mem1[0xfffe] = 0x48;
			c64Mem1[0xffff] = 0xff;
		}

		// Clear SID
		for (int32 i = 0; i < 0x1d; i++)
			c64Mem2[0xd400 + i] = 0;

		// Default Mastervolume, no filter
		c64Mem2[0xd418] = (sidLastValue = 0x0f);
	}
}



/******************************************************************************/
/* C64MemRamRom()                                                             */
/*                                                                            */
/* Input:  "address" is a 16-bit effective address.                           */
/*                                                                            */
/* Output: A default bank-select value for $01.                               */
/******************************************************************************/
uint8 SID6510::C64MemRamRom(uint16 address)
{
	if (memoryMode == MPU_PLAYSID_ENVIRONMENT)
		return (4);		// RAM only, but special I/O mode
	else
	{
		if (address < 0xa000)
			return (7);	// Basic-ROM, Kernal-ROM, I/O
		else if (address < 0xd000)
			return (6);	// Kernal-ROM, I/O
		else if (address >= 0xe000)
			return (5);	// I/O only
		else
			return (4);	// RAM only
	}
}



/******************************************************************************/
/* InitInterpreter()                                                          */
/******************************************************************************/
void SID6510::InitInterpreter(int32 inMemoryMode)
{
	memoryMode = inMemoryMode;

	if (memoryMode == MPU_TRANSPARENT_ROM)
	{
		readData  = ReadData_transp;
		writeData = WriteData_bs;

		instrList[0x20] = JSR_transp;
		instrList[0x4C] = JMP_transp;
		instrList[0x6C] = JMP_vec_transp;

		// Make the memory buffers accessible to the whole emulator engine.
		// Use two distinct 64KB memory areas
		c64Mem1 = c64RamBuf;
		c64Mem2 = c64RomBuf;
	}
	else if (memoryMode == MPU_PLAYSID_ENVIRONMENT)
	{
		readData  = ReadData_plain;
		writeData = WriteData_plain;

		instrList[0x20] = JSR_plain;
		instrList[0x4C] = JMP_plain;
		instrList[0x6C] = JMP_vec_plain;

		// Make the memory buffers accessible to the whole emulator engine.
		// Use a single 64KB memory area
		c64Mem2 = (c64Mem1 = c64RamBuf);
	}
	else	// if (memoryMode == MPU_BANK_SWITCHING)
	{
		readData  = ReadData_bs;
		writeData = WriteData_bs;

		instrList[0x20] = JSR_;
		instrList[0x4C] = JMP_;
		instrList[0x6C] = JMP_vec;

		// Make the memory buffers accessible to the whole emulator engine.
		// Use two distinct 64KB memory areas
		c64Mem1 = c64RamBuf;
		c64Mem2 = c64RomBuf;
	}

	bankSelReg = c64RamBuf + 1;	// Extra pointer

	// Set code execution segment to RAM
	pPCBase = c64RamBuf;
	pPCEnd  = c64RamBuf + 65536;
}



/******************************************************************************/
/* Interpreter()                                                              */
/******************************************************************************/
bool SID6510::Interpreter(uint16 p, uint8 ramRom, uint8 a, uint8 x, uint8 y)
{
	if (memoryMode == MPU_PLAYSID_ENVIRONMENT)
	{
		AC = a;
		XR = 0;
		YR = 0;
	}
	else
	{
		*bankSelReg = ramRom;
		EvalBankSelect();

		AC = a;
		XR = x;
		YR = y;
	}

	// Set program-counter (pointer instead of raw PC)
	pPC = pPCBase + p;

	ResetSP();
	ResetSR();

	sidKeysOff[4] = (sidKeysOff[4 + 7] = (sidKeysOff[4 + 14] = false));
	sidKeysOn[4]  = (sidKeysOn[4 + 7] = (sidKeysOn[4 + 14] = false));

	do
	{
		(*instrList[*(pPC++)])(this);
	}
	while (stackIsOkay && (pPC < pPCEnd));

	return (true);
}



/******************************************************************************/
/* EvalBankSelect()                                                           */
/* -------------------------------------------------------------------------- */
/* Relevant configurable memory banks:                                        */
/*                                                                            */
/*  $A000 to $BFFF = RAM, Basic-ROM                                           */
/*  $C000 to $CFFF = RAM                                                      */
/*  $D000 to $DFFF = RAM, I/O, Char-ROM                                       */
/*  $E000 to $FFFF = RAM, Kernal-ROM                                          */
/*                                                                            */
/* Bank-Select Register $01:                                                  */
/*                                                                            */
/*   Bits                                                                     */
/*   210    $A000-$BFFF   $D000-$DFFF   $E000-$FFFF                           */
/*  ------------------------------------------------                          */
/*   000       RAM           RAM            RAM                               */
/*   001       RAM        Char-ROM          RAM                               */
/*   010       RAM        Char-ROM      Kernal-ROM                            */
/*   011    Basic-ROM     Char-ROM      Kernal-ROM                            */
/*   100       RAM           RAM            RAM                               */
/*   101       RAM           I/O            RAM                               */
/*   110       RAM           I/O        Kernal-ROM                            */
/*   111    Basic-ROM        I/O        Kernal-ROM                            */
/*                                                                            */
/* "Transparent ROM" mode:                                                    */
/*                                                                            */
/* Basic-ROM and Kernal-ROM are considered transparent to read/write access.  */
/* Basic-ROM is also considered transparent to branches (JMP, BCC, ...).      */
/* I/O and Kernal-ROM are togglable via bank-select register $01.             */
/******************************************************************************/
inline void SID6510::EvalBankSelect(void)
{
	// Determine new memory configuration
	isBasic  = ((*bankSelReg & 3) == 3);
	isIO     = ((*bankSelReg & 7) > 4);
	isKernal = ((*bankSelReg & 2) != 0);
}



/******************************************************************************/
/* EvalBankJump()                                                             */
/* Upon JMP/JSR prevent code execution in Basic-ROM/Kernal-ROM.               */
/******************************************************************************/
inline void SID6510::EvalBankJump(void)
{
	if (PC < 0xA000)
	{
		;
	}
	else
	{
		// Get high-nibble of address
		switch (PC >> 12)
		{
			case 0xa:
			case 0xb:
				if (isBasic)
					RTS_(this);

				break;

			case 0xc:
				break;

			case 0xd:
				if (isIO)
					RTS_(this);

				break;

			case 0xe:
			case 0xf:
			default:	// <-- Just to please the compiler
				if (isKernal)
					RTS_(this);

				break;
		}
	}
}



/******************************************************************************/
/* ResetSR()                                                                  */
/******************************************************************************/
inline void SID6510::ResetSR(void)
{
	// Explicit paranthesis looks great
	CF = (ZF = (IF = (DF = (BF = (VF = (NF = 0))))));
	NU = 1;
}



/******************************************************************************/
/* CodeSR()                                                                   */
/******************************************************************************/
inline uint8 SID6510::CodeSR(void)
{
	register uint8 tempSR = CF;
	tempSR |= (ZF << 1);
	tempSR |= (IF << 2);
	tempSR |= (DF << 3);
	tempSR |= (BF << 4);
	tempSR |= (NU << 5);
	tempSR |= (VF << 6);
	tempSR |= (NF << 7);

	return (tempSR);
}



/******************************************************************************/
/* DecodeSR()                                                                 */
/******************************************************************************/
inline void SID6510::DecodeSR(uint8 stackByte)
{
	CF = (stackByte & 1);
	ZF = ((stackByte & 2) !=0);
	IF = ((stackByte & 4) !=0);
	DF = ((stackByte & 8) !=0);
	BF = ((stackByte & 16) !=0);
	NU = 1;		// If used or writable, ((stackByte & 32) !=0 );
	VF = ((stackByte & 64) !=0);
	NF = ((stackByte & 128) !=0);
}



/******************************************************************************/
/* AffectNZ()                                                                 */
/******************************************************************************/
inline void SID6510::AffectNZ(uint8 reg)
{
	ZF = (reg == 0);
	NF = ((reg & 0x80) != 0);
}



/******************************************************************************/
/* ResetSP()                                                                  */
/******************************************************************************/
inline void SID6510::ResetSP(void)
{
	SP          = 0x1ff;		// SP to top of stack
	stackIsOkay = true;
}



/******************************************************************************/
/* CheckSP()                                                                  */
/******************************************************************************/
inline void SID6510::CheckSP(void)
{
	stackIsOkay = ((SP > 0xff) && (SP <= 0x1ff));	// Check boundaries
}



/******************************************************************************/
/* ReadData_bs()                                                              */
/******************************************************************************/
uint8 SID6510::ReadData_bs(SID6510 *obj, uint16 addr)
{
	if (addr < 0xa000)
		return (obj->c64Mem1[addr]);
	else
	{
		// Get high-nibble of address
		switch (addr >> 12)
		{
			case 0xa:
			case 0xb:
			{
				if (obj->isBasic)
					return (obj->c64Mem2[addr]);
				else
					return (obj->c64Mem1[addr]);
			}

			case 0xc:
				return (obj->c64Mem1[addr]);

			case 0xd:
			{
				if (obj->isIO)
				{
					uint16 tempAddr = (addr & 0xfc1f);

					// Not SID?
					if ((tempAddr & 0xff00) != 0xd400)
					{
						switch (addr)
						{
							case 0xd011:
							case 0xd012:
							case 0xdc04:
							case 0xdc05:
								obj->fakeReadTimer = obj->fakeReadTimer * 13 + 1;
								return ((uint8)(obj->fakeReadTimer >> 3));

							default:
								return (obj->c64Mem2[addr]);
						}
					}
					else
					{
						// $D41D/1E/1F, $D43D/, ... SID not mirrored
						if ((tempAddr & 0x00ff) >= 0x001d)
							return (obj->c64Mem2[addr]);
						else
						{
							// (Mirrored) SID
							switch (tempAddr)
							{
								case 0xd41b:
									return (obj->optr3ReadWave);

								case 0xd41c:
									return (obj->optr3ReadEnve);

								default:
									return (obj->sidLastValue);
							}
						}
					}
				}
				else
					return (obj->c64Mem1[addr]);
			}

			case 0xe:
			case 0xf:
			default:	// <-- Just to please the compiler
			{
				if (obj->isKernal)
					return (obj->c64Mem2[addr]);
				else
					return (obj->c64Mem1[addr]);
			}
		}
	}
}



/******************************************************************************/
/* ReadData_transp()                                                          */
/******************************************************************************/
uint8 SID6510::ReadData_transp(SID6510 *obj, uint16 addr)
{
	if (addr < 0xd000)
		return (obj->c64Mem1[addr]);
	else
	{
		// Get high-nibble of address
		switch (addr >> 12)
		{
			case 0xd:
			{
				if (obj->isIO)
				{
					uint16 tempAddr = (addr & 0xfc1f);

					// Not SID?
					if ((tempAddr & 0xff00) != 0xd400)
					{
						switch (addr)
						{
							case 0xd011:
							case 0xd012:
							case 0xdc04:
							case 0xdc05:
								obj->fakeReadTimer = obj->fakeReadTimer * 13 + 1;
								return ((uint8)(obj->fakeReadTimer >> 3));

							default:
								return (obj->c64Mem2[addr]);
						}
					}
					else
					{
						// $D41D/1E/1F, $D43D/, ... SID not mirrored
						if ((tempAddr & 0x00ff) >= 0x001d)
							return (obj->c64Mem2[addr]);
						else
						{
							// (Mirrored) SID
							switch (tempAddr)
							{
								case 0xd41b:
									return (obj->optr3ReadWave);

								case 0xd41c:
									return (obj->optr3ReadEnve);

								default:
									return (obj->sidLastValue);
							}
						}
					}
				}
				else
					return (obj->c64Mem1[addr]);
			}

			case 0xe:
			case 0xf:
			default:	// <-- Just to please the compiler
				return (obj->c64Mem1[addr]);
		}
	}
}



/******************************************************************************/
/* ReadData_plain()                                                           */
/******************************************************************************/
uint8 SID6510::ReadData_plain(SID6510 *obj, uint16 addr)
{
	return (obj->c64Mem1[addr]);
}



/******************************************************************************/
/* ReadData_zp()                                                              */
/******************************************************************************/
inline uint8 SID6510::ReadData_zp(uint16 addr)
{
	return (c64Mem1[addr]);
}



/******************************************************************************/
/* WriteData_bs()                                                             */
/******************************************************************************/
void SID6510::WriteData_bs(SID6510 *obj, uint16 addr, uint8 data)
{
	if ((addr < 0xd000) || (addr >= 0xe000))
	{
		obj->c64Mem1[addr] = data;
		if (addr == 0x01)	// Write to Bank-Select Register?
			obj->EvalBankSelect();
	}
	else
	{
		if (obj->isIO)
		{
			// Check whether real SID or mirrored SID
			uint16 tempAddr = (addr & 0xfc1f);

			// Not SID?
			if ((tempAddr & 0xff00) != 0xd400)
				obj->c64Mem2[addr] = data;

			// $D41D/1E/1F, $D43D/3E/3F, ...
			// Map to real address to support PlaySID
			// Extended SID Chip Registers
			else if ((tempAddr & 0x00ff) >= 0x001d)
			{
				// Mirrored SID
				obj->c64Mem2[addr] = (obj->sidLastValue = data);
			}
			else
			{
				// SID
				obj->c64Mem2[tempAddr] = (obj->sidLastValue = data);

				// Handle key_ons
				obj->sidKeysOn[tempAddr & 0x001f] = obj->sidKeysOn[tempAddr & 0x001f] || ((data & 1) != 0);

				// Handle key_offs
				obj->sidKeysOff[tempAddr & 0x001f] = obj->sidKeysOff[tempAddr & 0x001f] || ((data & 1) == 0);
			}
		}
		else
			obj->c64Mem1[addr] = data;
	}
}



/******************************************************************************/
/* WriteData_plain()                                                          */
/******************************************************************************/
void SID6510::WriteData_plain(SID6510 *obj, uint16 addr, uint8 data)
{
	// Check whether real SID or mirrored SID
	uint16 tempAddr = (addr & 0xfc1f);

	// Not SID?
	if ((tempAddr & 0xff00) != 0xd400)
		obj->c64Mem1[addr] = data;

	// $D41D/1E/1F, $D43D/3E/3F, ...
	// Map to real address to support PlaySID
	// Extended SID Chip Registers.
	else if ((tempAddr & 0x00ff) >= 0x001d)
	{
		// Mirrored SID
		obj->c64Mem1[addr] = (obj->sidLastValue = data);
	}
	else
	{
		// SID
		obj->c64Mem2[tempAddr] = (obj->sidLastValue = data);

		// Handle key_ons
		obj->sidKeysOn[tempAddr & 0x001f] = obj->sidKeysOn[tempAddr & 0x001f] || ((data & 1) != 0);

		// Handle key_offs
		obj->sidKeysOff[tempAddr & 0x001f] = obj->sidKeysOff[tempAddr & 0x001f] || ((data & 1) == 0);
	}
}



/******************************************************************************/
/* WriteData_zp()                                                             */
/******************************************************************************/
inline void SID6510::WriteData_zp(uint16 addr, uint8 data)
{
	c64Mem1[addr] = data;
	if (addr == 0x01)	// Write to Bank-Select Register?
		EvalBankSelect();
}



/******************************************************************************/
/* Addressing modes:                                                          */
/* Calculating 8/16-bit effective addresses out of data operands.             */
/******************************************************************************/
inline uint16 SID6510::Abso(void)
{
	return (P_LENDIAN_TO_HOST_INT16(*((uint16 *)pPC)));
}



inline uint16 SID6510::Absx(void)
{
	return (P_LENDIAN_TO_HOST_INT16(*((uint16 *)pPC)) + XR);
}



inline uint16 SID6510::Absy(void)
{
	return (P_LENDIAN_TO_HOST_INT16(*((uint16 *)pPC)) + YR);
}



inline uint8 SID6510::Imm(void)
{
	return (*pPC);
}



inline uint16 SID6510::Indx(void)
{
	return (P_LENDIAN_TO_HOST_INT16(*((uint16 *)&c64Mem1[(*pPC + XR) & 0xFF])));
}



inline uint16 SID6510::Indy(void)
{
	return (YR + P_LENDIAN_TO_HOST_INT16(*((uint16 *)&c64Mem1[*pPC])));
}



inline uint8 SID6510::ZP(void)
{
	return (*pPC);
}



inline uint8 SID6510::ZPX(void)
{
	return (*pPC + XR);
}



inline uint8 SID6510::ZPY(void)
{
	return (*pPC + YR);
}



/******************************************************************************/
/* Handling conditional branches.                                             */
/******************************************************************************/
inline void SID6510::BranchIfClear(uint8 flag)
{
	if (flag == 0)
	{
		PC  = pPC - pPCBase;	// Calculate 16-bit PC
		PC += (int8)*pPC;		// Add offset, keep it 16-bit (uint16)
		pPC = pPCBase + PC;		// Calc new pointer-PC
	}

	pPC++;
}



inline void SID6510::BranchIfSet(uint8 flag)
{
	if (flag != 0)
	{
		PC  = pPC - pPCBase;	// Calculate 16-bit PC
		PC += (int8)*pPC;		// Add offset, keep it 16-bit (uint16)
		pPC = pPCBase + PC;		// Calc new pointer-PC
	}

	pPC++;
}



/******************************************************************************/
/* ADC()                                                                      */
/******************************************************************************/
inline void SID6510::ADC_m(uint8 x)
{
	if (DF == 1)
	{
		uint16 AC2 = AC + x + CF;
		ZF = (AC2 == 0);

		if (((AC & 15) + (x & 15) + CF) > 9)
			AC2 += 6;

		VF = (((AC ^ x ^ AC2) & 0x80) != 0) ^ CF;
		NF = ((AC2 & 128) != 0);

		if (AC2 > 0x99)
			AC2 += 96;

		CF = (AC2 > 0x99);
		AC = (AC2 & 255);
	}
	else
	{
		uint16 AC2 = AC + x + CF;
		CF = (AC2 > 255);
		VF = (((AC ^ x ^ AC2) & 0x80) != 0) ^ CF;
		AffectNZ(AC = (AC2 & 255));
	}
}



void SID6510::ADC_abso(SID6510 *cpu)
{
	cpu->ADC_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::ADC_absx(SID6510 *cpu)
{
	cpu->ADC_m(cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::ADC_absy(SID6510 *cpu)
{
	cpu->ADC_m(cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::ADC_imm(SID6510 *cpu)
{
	cpu->ADC_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::ADC_indx(SID6510 *cpu)
{
	cpu->ADC_m(cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::ADC_indy(SID6510 *cpu)
{
	cpu->ADC_m(cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::ADC_zp(SID6510 *cpu)
{
	cpu->ADC_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::ADC_zpx(SID6510 *cpu)
{
	cpu->ADC_m(cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* AND()                                                                      */
/******************************************************************************/
inline void SID6510::AND_m(uint8 x)
{
	AffectNZ(AC &= x);
}



void SID6510::AND_abso(SID6510 *cpu)
{
	cpu->AND_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::AND_absx(SID6510 *cpu)
{
	cpu->AND_m(cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::AND_absy(SID6510 *cpu)
{
	cpu->AND_m(cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::AND_imm(SID6510 *cpu)
{
	cpu->AND_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::AND_indx(SID6510 *cpu)
{
	cpu->AND_m(cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::AND_indy(SID6510 *cpu)
{
	cpu->AND_m(cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::AND_zp(SID6510 *cpu)
{
	cpu->AND_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::AND_zpx(SID6510 *cpu)
{
	cpu->AND_m(cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* ASL()                                                                      */
/******************************************************************************/
inline uint8 SID6510::ASL_m(uint8 x)
{
	CF = ((x & 128) != 0);
	AffectNZ(x <<= 1);
	return (x);
}



void SID6510::ASL_AC(SID6510 *cpu)
{
	cpu->AC = cpu->ASL_m(cpu->AC);
}



void SID6510::ASL_abso(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Abso();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, cpu->ASL_m(cpu->readData(cpu, tempAddr)));
}



void SID6510::ASL_absx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Absx();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, cpu->ASL_m(cpu->readData(cpu, tempAddr)));
}



void SID6510::ASL_zp(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZP();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, cpu->ASL_m(cpu->ReadData_zp(tempAddr)));
}



void SID6510::ASL_zpx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZPX();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, cpu->ASL_m(cpu->ReadData_zp(tempAddr)));
}



/******************************************************************************/
/* BCC()                                                                      */
/******************************************************************************/
void SID6510::BCC_(SID6510 *cpu)
{
	cpu->BranchIfClear(cpu->SR.Carry);
}



/******************************************************************************/
/* BCS()                                                                      */
/******************************************************************************/
void SID6510::BCS_(SID6510 *cpu)
{
	cpu->BranchIfSet(cpu->SR.Carry);
}



/******************************************************************************/
/* BEQ()                                                                      */
/******************************************************************************/
void SID6510::BEQ_(SID6510 *cpu)
{
	cpu->BranchIfSet(cpu->SR.Zero);
}



/******************************************************************************/
/* BIT()                                                                      */
/******************************************************************************/
inline void SID6510::BIT_m(uint8 x)
{
	ZF = ((AC & x) == 0);
	VF = ((x & 64) != 0);
	NF = ((x & 128) != 0);
}



void SID6510::BIT_abso(SID6510 *cpu)
{
	cpu->BIT_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::BIT_zp(SID6510 *cpu)
{
	cpu->BIT_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



/******************************************************************************/
/* BMI()                                                                      */
/******************************************************************************/
void SID6510::BMI_(SID6510 *cpu)
{
	cpu->BranchIfSet(cpu->SR.Negative);
}



/******************************************************************************/
/* BNE()                                                                      */
/******************************************************************************/
void SID6510::BNE_(SID6510 *cpu)
{
	cpu->BranchIfClear(cpu->SR.Zero);
}



/******************************************************************************/
/* BPL()                                                                      */
/******************************************************************************/
void SID6510::BPL_(SID6510 *cpu)
{
	cpu->BranchIfClear(cpu->SR.Negative);
}



/******************************************************************************/
/* BRK()                                                                      */
/******************************************************************************/
void SID6510::BRK_(SID6510 *cpu)
{
	cpu->SR.Break = (cpu->SR.Interrupt = 1);
	cpu->RTS_(cpu);
}



/******************************************************************************/
/* BVC()                                                                      */
/******************************************************************************/
void SID6510::BVC_(SID6510 *cpu)
{
	cpu->BranchIfClear(cpu->SR.oVerflow);
}



/******************************************************************************/
/* BVS()                                                                      */
/******************************************************************************/
void SID6510::BVS_(SID6510 *cpu)
{
	cpu->BranchIfSet(cpu->SR.oVerflow);
}



/******************************************************************************/
/* CLC()                                                                      */
/******************************************************************************/
void SID6510::CLC_(SID6510 *cpu)
{
	cpu->SR.Carry = 0;
}



/******************************************************************************/
/* CLD()                                                                      */
/******************************************************************************/
void SID6510::CLD_(SID6510 *cpu)
{
	cpu->SR.Decimal = 0;
}



/******************************************************************************/
/* CLI()                                                                      */
/******************************************************************************/
void SID6510::CLI_(SID6510 *cpu)
{
	cpu->SR.Interrupt = 0;
}



/******************************************************************************/
/* CLV()                                                                      */
/******************************************************************************/
void SID6510::CLV_(SID6510 *cpu)
{
	cpu->SR.oVerflow = 0;
}



/******************************************************************************/
/* CMP()                                                                      */
/******************************************************************************/
inline void SID6510::CMP_m(uint8 x)
{
	ZF = (AC == x);
	CF = (AC >= x);
	NF = ((int8)(AC - x) < 0);
}



void SID6510::CMP_abso(SID6510 *cpu)
{
	cpu->CMP_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::CMP_absx(SID6510 *cpu)
{
	cpu->CMP_m(cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::CMP_absy(SID6510 *cpu)
{
	cpu->CMP_m(cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::CMP_imm(SID6510 *cpu)
{
	cpu->CMP_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::CMP_indx(SID6510 *cpu)
{
	cpu->CMP_m(cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::CMP_indy(SID6510 *cpu)
{
	cpu->CMP_m(cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::CMP_zp(SID6510 *cpu)
{
	cpu->CMP_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::CMP_zpx(SID6510 *cpu)
{
	cpu->CMP_m(cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* CPX()                                                                      */
/******************************************************************************/
inline void SID6510::CPX_m(uint8 x)
{
	ZF = (XR == x);
	CF = (XR >= x);
	NF = ((int8)(XR - x) < 0);
}



void SID6510::CPX_abso(SID6510 *cpu)
{
	cpu->CPX_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::CPX_imm(SID6510 *cpu)
{
	cpu->CPX_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::CPX_zp(SID6510 *cpu)
{
	cpu->CPX_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



/******************************************************************************/
/* CPY()                                                                      */
/******************************************************************************/
inline void SID6510::CPY_m(uint8 x)
{
	ZF = (YR == x);
	CF = (YR >= x);
	NF = ((int8)(YR - x) < 0);
}



void SID6510::CPY_abso(SID6510 *cpu)
{
	cpu->CPY_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::CPY_imm(SID6510 *cpu)
{
	cpu->CPY_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::CPY_zp(SID6510 *cpu)
{
	cpu->CPY_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



/******************************************************************************/
/* DEC()                                                                      */
/******************************************************************************/
inline void SID6510::DEC_m(uint16 addr)
{
	uint8 x = readData(this, addr);
	AffectNZ(--x);
	writeData(this, addr, x);
}



inline void SID6510::DEC_m_zp(uint16 addr)
{
	uint8 x = ReadData_zp(addr);
	AffectNZ(--x);
	WriteData_zp(addr, x);
}



void SID6510::DEC_abso(SID6510 *cpu)
{
	cpu->DEC_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::DEC_absx(SID6510 *cpu)
{
	cpu->DEC_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::DEC_zp(SID6510 *cpu)
{
	cpu->DEC_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::DEC_zpx(SID6510 *cpu)
{
	cpu->DEC_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* DEX()                                                                      */
/******************************************************************************/
void SID6510::DEX_(SID6510 *cpu)
{
	cpu->AffectNZ(--cpu->XR);
}



/******************************************************************************/
/* DEY()                                                                      */
/******************************************************************************/
void SID6510::DEY_(SID6510 *cpu)
{
	cpu->AffectNZ(--cpu->YR);
}



/******************************************************************************/
/* EOR()                                                                      */
/******************************************************************************/
inline void SID6510::EOR_m(uint8 x)
{
	AC ^= x;
	AffectNZ(AC);
}



void SID6510::EOR_abso(SID6510 *cpu)
{
	cpu->EOR_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::EOR_absx(SID6510 *cpu)
{
	cpu->EOR_m(cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::EOR_absy(SID6510 *cpu)
{
	cpu->EOR_m(cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::EOR_imm(SID6510 *cpu)
{
	cpu->EOR_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::EOR_indx(SID6510 *cpu)
{
	cpu->EOR_m(cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::EOR_indy(SID6510 *cpu)
{
	cpu->EOR_m(cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::EOR_zp(SID6510 *cpu)
{
	cpu->EOR_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::EOR_zpx(SID6510 *cpu)
{
	cpu->EOR_m(cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* INC()                                                                      */
/******************************************************************************/
inline void SID6510::INC_m(uint16 addr)
{
	uint8 x = readData(this, addr);
	AffectNZ(++x);
	writeData(this, addr, x);
}



inline void SID6510::INC_m_zp(uint16 addr)
{
	uint8 x = ReadData_zp(addr);
	AffectNZ(++x);
	WriteData_zp(addr, x);
}



void SID6510::INC_abso(SID6510 *cpu)
{
	cpu->INC_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::INC_absx(SID6510 *cpu)
{
	cpu->INC_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::INC_zp(SID6510 *cpu)
{
	cpu->INC_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::INC_zpx(SID6510 *cpu)
{
	cpu->INC_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* INX()                                                                      */
/******************************************************************************/
void SID6510::INX_(SID6510 *cpu)
{
	cpu->AffectNZ(++cpu->XR);
}



/******************************************************************************/
/* INY()                                                                      */
/******************************************************************************/
void SID6510::INY_(SID6510 *cpu)
{
	cpu->AffectNZ(++cpu->YR);
}



/******************************************************************************/
/* JMP()                                                                      */
/******************************************************************************/
void SID6510::JMP_(SID6510 *cpu)
{
	cpu->PC  = cpu->Abso();
	cpu->pPC = cpu->pPCBase + cpu->PC;
	cpu->EvalBankJump();
}



void SID6510::JMP_plain(SID6510 *cpu)
{
	cpu->PC  = cpu->Abso();
	cpu->pPC = cpu->pPCBase + cpu->PC;
}



void SID6510::JMP_transp(SID6510 *cpu)
{
	cpu->PC = cpu->Abso();
	if ((cpu->PC >= 0xd000) && cpu->isKernal)
		cpu->RTS_(cpu);	// Will set pPC
	else
		cpu->pPC = cpu->pPCBase + cpu->PC;
}



void SID6510::JMP_vec(SID6510 *cpu)
{
	uint16 tempAddrLo = cpu->Abso();
	uint16 tempAddrHi = (tempAddrLo & 0xFF00) | ((tempAddrLo + 1) & 0x00FF);

	cpu->PC  = cpu->readData(cpu, tempAddrHi) * 256 + cpu->readData(cpu, tempAddrLo);
	cpu->pPC = cpu->pPCBase + cpu->PC;

	cpu->EvalBankJump();
}



void SID6510::JMP_vec_plain(SID6510 *cpu)
{
	uint16 tempAddrLo = cpu->Abso();
	uint16 tempAddrHi = (tempAddrLo & 0xFF00) | ((tempAddrLo + 1) & 0x00FF);

	cpu->PC  = cpu->readData(cpu, tempAddrHi) * 256 + cpu->readData(cpu, tempAddrLo);
	cpu->pPC = cpu->pPCBase + cpu->PC;
}



void SID6510::JMP_vec_transp(SID6510 *cpu)
{
	uint16 tempAddrLo = cpu->Abso();
	uint16 tempAddrHi = (tempAddrLo & 0xFF00) | ((tempAddrLo + 1) & 0x00FF);

	cpu->PC  = cpu->readData(cpu, tempAddrHi) * 256 + cpu->readData(cpu, tempAddrLo);
	if ((cpu->PC >= 0xd000) && cpu->isKernal)
		cpu->RTS_(cpu);		// Will set pPC
	else
		cpu->pPC = cpu->pPCBase + cpu->PC;
}



/******************************************************************************/
/* JSR()                                                                      */
/******************************************************************************/
inline void SID6510::JSR_main(void)
{
	uint16 tempPC = Abso();

	pPC += 2;
	PC   = pPC - pPCBase;
	PC--;
	SP--;

	c64Mem1[SP + 0] = PC & 0x00ff;
	c64Mem1[SP + 1] = PC >> 8;
	SP--;
	CheckSP();

	PC = tempPC;
}



void SID6510::JSR_(SID6510 *cpu)
{ 
	cpu->JSR_main();
	cpu->pPC = cpu->pPCBase + cpu->PC;
	cpu->EvalBankJump();
}



void SID6510::JSR_plain(SID6510 *cpu)
{
	cpu->JSR_main();
	cpu->pPC = cpu->pPCBase + cpu->PC;
}



void SID6510::JSR_transp(SID6510 *cpu)
{
	cpu->JSR_main();
	if ((cpu->PC >= 0xd000) && cpu->isKernal)
		cpu->RTS_(cpu);	// Will set pPC
	else
		cpu->pPC = cpu->pPCBase + cpu->PC;
}



/******************************************************************************/
/* LDA()                                                                      */
/******************************************************************************/
void SID6510::LDA_abso(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::LDA_absx(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::LDA_absy(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::LDA_imm(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->Imm());
	cpu->pPC++;
}



void SID6510::LDA_indx(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::LDA_indy(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::LDA_zp(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::LDA_zpx(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* LDX()                                                                      */
/******************************************************************************/
void SID6510::LDX_abso(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->XR = cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::LDX_absy(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->XR = cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::LDX_imm(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->XR = cpu->Imm());
	cpu->pPC++;
}



void SID6510::LDX_zp(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->XR = cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::LDX_zpy(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->XR = cpu->ReadData_zp(cpu->ZPY()));
	cpu->pPC++;
}



/******************************************************************************/
/* LDY()                                                                      */
/******************************************************************************/
void SID6510::LDY_abso(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->YR = cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::LDY_absx(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->YR = cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::LDY_imm(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->YR = cpu->Imm());
	cpu->pPC++;
}



void SID6510::LDY_zp(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->YR = cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::LDY_zpx(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->YR = cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* LSR()                                                                      */
/******************************************************************************/
inline uint8 SID6510::LSR_m(uint8 x)
{
	CF  = x & 1;
	x >>= 1;
	NF  = 0;
	ZF  = (x == 0);
	return (x);
}



void SID6510::LSR_AC(SID6510 *cpu)
{
	cpu->AC = cpu->LSR_m(cpu->AC);
}



void SID6510::LSR_abso(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Abso();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, (cpu->LSR_m(cpu->readData(cpu, tempAddr))));
}



void SID6510::LSR_absx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Absx();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, (cpu->LSR_m(cpu->readData(cpu, tempAddr))));
}



void SID6510::LSR_zp(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZP();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, (cpu->LSR_m(cpu->ReadData_zp(tempAddr))));
}



void SID6510::LSR_zpx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZPX();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, (cpu->LSR_m(cpu->ReadData_zp(tempAddr))));
}



/******************************************************************************/
/* NOP()                                                                      */
/******************************************************************************/
void SID6510::NOP_(SID6510 *cpu)
{
}



/******************************************************************************/
/* ORA()                                                                      */
/******************************************************************************/
inline void SID6510::ORA_m(uint8 x)
{
	AffectNZ(AC |= x);
}



void SID6510::ORA_abso(SID6510 *cpu)
{
	cpu->ORA_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::ORA_absx(SID6510 *cpu)
{
	cpu->ORA_m(cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::ORA_absy(SID6510 *cpu)
{
	cpu->ORA_m(cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::ORA_imm(SID6510 *cpu)
{
	cpu->ORA_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::ORA_indx(SID6510 *cpu)
{
	cpu->ORA_m(cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::ORA_indy(SID6510 *cpu)
{
	cpu->ORA_m(cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::ORA_zp(SID6510 *cpu)
{
	cpu->ORA_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::ORA_zpx(SID6510 *cpu)
{
	cpu->ORA_m(cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* PHA()                                                                      */
/******************************************************************************/
void SID6510::PHA_(SID6510 *cpu)
{
	cpu->c64Mem1[cpu->SP--] = cpu->AC;
}



/******************************************************************************/
/* PHP()                                                                      */
/******************************************************************************/
void SID6510::PHP_(SID6510 *cpu)
{
	cpu->c64Mem1[cpu->SP--] = cpu->CodeSR();
}



/******************************************************************************/
/* PLA()                                                                      */
/******************************************************************************/
void SID6510::PLA_(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->c64Mem1[++cpu->SP]);
}



/******************************************************************************/
/* PLP()                                                                      */
/******************************************************************************/
void SID6510::PLP_(SID6510 *cpu)
{
	cpu->DecodeSR(cpu->c64Mem1[++cpu->SP]);
}



/******************************************************************************/
/* ROL()                                                                      */
/******************************************************************************/
inline uint8 SID6510::ROL_m(uint8 x)
{
	uint8 y = (x << 1) + CF;
	CF = ((x & 0x80) != 0);
	AffectNZ(y);
	return (y);
}



void SID6510::ROL_AC(SID6510 *cpu)
{
	cpu->AC = cpu->ROL_m(cpu->AC);
}



void SID6510::ROL_abso(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Abso();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, cpu->ROL_m(cpu->readData(cpu, tempAddr)));
}



void SID6510::ROL_absx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Absx();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, cpu->ROL_m(cpu->readData(cpu, tempAddr)));
}



void SID6510::ROL_zp(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZP();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, cpu->ROL_m(cpu->ReadData_zp(tempAddr)));
}



void SID6510::ROL_zpx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZPX();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, cpu->ROL_m(cpu->ReadData_zp(tempAddr)));
}



/******************************************************************************/
/* ROR()                                                                      */
/******************************************************************************/
inline uint8 SID6510::ROR_m(uint8 x)
{
	uint8 y = (x >> 1) | (CF << 7);
	CF = (x & 1);
	AffectNZ(y);
	return (y);
}



void SID6510::ROR_AC(SID6510 *cpu)
{
	cpu->AC = cpu->ROR_m(cpu->AC);
}



void SID6510::ROR_abso(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Abso();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, cpu->ROR_m(cpu->readData(cpu, tempAddr)));
}



void SID6510::ROR_absx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->Absx();
	cpu->pPC += 2;
	cpu->writeData(cpu, tempAddr, cpu->ROR_m(cpu->readData(cpu, tempAddr)));
}



void SID6510::ROR_zp(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZP();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, cpu->ROR_m(cpu->ReadData_zp(tempAddr)));
}



void SID6510::ROR_zpx(SID6510 *cpu)
{
	uint16 tempAddr = cpu->ZPX();
	cpu->pPC++;
	cpu->WriteData_zp(tempAddr, cpu->ROR_m(cpu->ReadData_zp(tempAddr)));
}



/******************************************************************************/
/* RTI()                                                                      */
/******************************************************************************/
void SID6510::RTI_(SID6510 *cpu)
{
	// Equal to RTS_();
	cpu->SP++;
	cpu->PC  = P_LENDIAN_TO_HOST_INT16(*((uint16 *)&cpu->c64Mem1[cpu->SP])) + 1;
	cpu->pPC = cpu->pPCBase + cpu->PC;
	cpu->SP++;
	cpu->CheckSP();
}



/******************************************************************************/
/* RTS()                                                                      */
/******************************************************************************/
inline void SID6510::RTS_(SID6510 *cpu)
{
	cpu->SP++;
	cpu->PC  = P_LENDIAN_TO_HOST_INT16(*((uint16 *)&cpu->c64Mem1[cpu->SP])) + 1;
	cpu->pPC = cpu->pPCBase + cpu->PC;
	cpu->SP++;
	cpu->CheckSP();
}



/******************************************************************************/
/* SBC()                                                                      */
/******************************************************************************/
inline void SID6510::SBC_m(uint8 s)
{
	s = (~s) & 255;
	if (DF == 1)
	{
		uint16 AC2 = AC + s + CF;
		ZF = (AC2 == 0);

		if (((AC & 15) + (s & 15) + CF) > 9)
			AC2 += 6;

		VF = (((AC ^ s ^ AC2) & 0x80) != 0) ^ CF;
		NF = ((AC2 & 128) != 0);

		if (AC2 > 0x99)
			AC2 += 96;

		CF = (AC2 > 0x99);
		AC = (AC2 & 255);
	}
	else
	{
		uint16 AC2 = AC + s + CF;
		CF = (AC2 > 255);
		VF = (((AC ^ s ^ AC2) & 0x80) != 0) ^ CF;
		AffectNZ(AC = (AC2 & 255));
	}
}



void SID6510::SBC_abso(SID6510 *cpu)
{
	cpu->SBC_m(cpu->readData(cpu, cpu->Abso()));
	cpu->pPC += 2;
}



void SID6510::SBC_absx(SID6510 *cpu)
{
	cpu->SBC_m(cpu->readData(cpu, cpu->Absx()));
	cpu->pPC += 2;
}



void SID6510::SBC_absy(SID6510 *cpu)
{
	cpu->SBC_m(cpu->readData(cpu, cpu->Absy()));
	cpu->pPC += 2;
}



void SID6510::SBC_imm(SID6510 *cpu)
{
	cpu->SBC_m(cpu->Imm());
	cpu->pPC++;
}



void SID6510::SBC_indx(SID6510 *cpu)
{
	cpu->SBC_m(cpu->readData(cpu, cpu->Indx()));
	cpu->pPC++;
}



void SID6510::SBC_indy(SID6510 *cpu)
{
	cpu->SBC_m(cpu->readData(cpu, cpu->Indy()));
	cpu->pPC++;
}



void SID6510::SBC_zp(SID6510 *cpu)
{
	cpu->SBC_m(cpu->ReadData_zp(cpu->ZP()));
	cpu->pPC++;
}



void SID6510::SBC_zpx(SID6510 *cpu)
{
	cpu->SBC_m(cpu->ReadData_zp(cpu->ZPX()));
	cpu->pPC++;
}



/******************************************************************************/
/* SEC()                                                                      */
/******************************************************************************/
void SID6510::SEC_(SID6510 *cpu)
{
	cpu->SR.Carry = 1;
}



/******************************************************************************/
/* SED()                                                                      */
/******************************************************************************/
void SID6510::SED_(SID6510 *cpu)
{
	cpu->SR.Decimal = 1;
}



/******************************************************************************/
/* SEI()                                                                      */
/******************************************************************************/
void SID6510::SEI_(SID6510 *cpu)
{
	cpu->SR.Interrupt = 1;
}



/******************************************************************************/
/* STA()                                                                      */
/******************************************************************************/
void SID6510::STA_abso(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Abso(), cpu->AC);
	cpu->pPC += 2;
}



void SID6510::STA_absx(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Absx(), cpu->AC);
	cpu->pPC += 2;
}



void SID6510::STA_absy(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Absy(), cpu->AC);
	cpu->pPC += 2;
}



void SID6510::STA_indx(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Indx(), cpu->AC);
	cpu->pPC++;
}



void SID6510::STA_indy(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Indy(), cpu->AC);
	cpu->pPC++;
}



void SID6510::STA_zp(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZP(), cpu->AC);
	cpu->pPC++;
}



void SID6510::STA_zpx(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZPX(), cpu->AC);
	cpu->pPC++;
}



/******************************************************************************/
/* STX()                                                                      */
/******************************************************************************/
void SID6510::STX_abso(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Abso(), cpu->XR);
	cpu->pPC += 2;
}



void SID6510::STX_zp(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZP(), cpu->XR);
	cpu->pPC++;
}



void SID6510::STX_zpy(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZPY(), cpu->XR);
	cpu->pPC++;
}



/******************************************************************************/
/* STY()                                                                      */
/******************************************************************************/
void SID6510::STY_abso(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Abso(), cpu->YR);
	cpu->pPC += 2;
}



void SID6510::STY_zp(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZP(), cpu->YR);
	cpu->pPC++;
}



void SID6510::STY_zpx(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZPX(), cpu->YR);
	cpu->pPC++;
}



/******************************************************************************/
/* TAX()                                                                      */
/******************************************************************************/
void SID6510::TAX_(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->XR = cpu->AC);
}



/******************************************************************************/
/* TAY()                                                                      */
/******************************************************************************/
void SID6510::TAY_(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->YR = cpu->AC);
}



/******************************************************************************/
/* TSX()                                                                      */
/******************************************************************************/
void SID6510::TSX_(SID6510 *cpu)
{
	cpu->XR = cpu->SP & 255;
	cpu->AffectNZ(cpu->XR);
}



/******************************************************************************/
/* TXA()                                                                      */
/******************************************************************************/
void SID6510::TXA_(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->XR);
}



/******************************************************************************/
/* TXS()                                                                      */
/******************************************************************************/
void SID6510::TXS_(SID6510 *cpu)
{
	cpu->SP = cpu->XR | 0x100;
	cpu->CheckSP();
}



/******************************************************************************/
/* TYA()                                                                      */
/******************************************************************************/
void SID6510::TYA_(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->YR);
}



/******************************************************************************/
/* Illegal codes/instructions part (1).                                       */
/******************************************************************************/
void SID6510::ILL_TILT(SID6510 *cpu)
{
}



void SID6510::ILL_1NOP(SID6510 *cpu)
{
}



void SID6510::ILL_2NOP(SID6510 *cpu)
{
	cpu->pPC++;
}



void SID6510::ILL_3NOP(SID6510 *cpu)
{
	cpu->pPC += 2;
}



/******************************************************************************/
/* Illegal codes/instructions part (2).                                       */
/******************************************************************************/
/* ASLORA()                                                                   */
/******************************************************************************/
inline void SID6510::ASLORA_m(uint16 addr)
{
	uint8 x = ASL_m(readData(this, addr));
	writeData(this, addr, x);
	ORA_m(x);
}



inline void SID6510::ASLORA_m_zp(uint16 addr)
{
	uint8 x = ASL_m(ReadData_zp(addr));
	WriteData_zp(addr, x);
	ORA_m(x);
}



void SID6510::ASLORA_abso(SID6510 *cpu)
{
	cpu->ASLORA_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::ASLORA_absx(SID6510 *cpu)
{
	cpu->ASLORA_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::ASLORA_absy(SID6510 *cpu)
{
	cpu->ASLORA_m(cpu->Absy());
	cpu->pPC += 2;
}



void SID6510::ASLORA_indx(SID6510 *cpu)
{
	cpu->ASLORA_m(cpu->Indx());
	cpu->pPC++;
}



void SID6510::ASLORA_indy(SID6510 *cpu)
{
	cpu->ASLORA_m(cpu->Indy());
	cpu->pPC++;
}



void SID6510::ASLORA_zp(SID6510 *cpu)
{
	cpu->ASLORA_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::ASLORA_zpx(SID6510 *cpu)
{
	cpu->ASLORA_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* DECCMP()                                                                   */
/******************************************************************************/
inline void SID6510::DECCMP_m(uint16 addr)
{
	uint8 x = readData(this, addr);
	writeData(this, addr, (--x));
	CMP_m(x);
}



inline void SID6510::DECCMP_m_zp(uint16 addr)
{
	uint8 x = ReadData_zp(addr);
	WriteData_zp(addr, (--x));
	CMP_m(x);
}



void SID6510::DECCMP_abso(SID6510 *cpu)
{
	cpu->DECCMP_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::DECCMP_absx(SID6510 *cpu)
{
	cpu->DECCMP_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::DECCMP_absy(SID6510 *cpu)
{
	cpu->DECCMP_m(cpu->Absy());
	cpu->pPC += 2;
}



void SID6510::DECCMP_indx(SID6510 *cpu)
{
	cpu->DECCMP_m(cpu->Indx());
	cpu->pPC++;
}



void SID6510::DECCMP_indy(SID6510 *cpu)
{
	cpu->DECCMP_m(cpu->Indy());
	cpu->pPC++;
}



void SID6510::DECCMP_zp(SID6510 *cpu)
{
	cpu->DECCMP_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::DECCMP_zpx(SID6510 *cpu)
{
	cpu->DECCMP_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* INCSBC()                                                                   */
/******************************************************************************/
inline void SID6510::INCSBC_m(uint16 addr)
{
	uint8 x = readData(this, addr);
	writeData(this, addr, (++x));
	SBC_m(x);
}



inline void SID6510::INCSBC_m_zp(uint16 addr)
{
	uint8 x = ReadData_zp(addr);
	WriteData_zp(addr, (++x));
	SBC_m(x);
}



void SID6510::INCSBC_abso(SID6510 *cpu)
{
	cpu->INCSBC_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::INCSBC_absx(SID6510 *cpu)
{
	cpu->INCSBC_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::INCSBC_absy(SID6510 *cpu)
{
	cpu->INCSBC_m(cpu->Absy());
	cpu->pPC += 2;
}



void SID6510::INCSBC_indx(SID6510 *cpu)
{
	cpu->INCSBC_m(cpu->Indx());
	cpu->pPC++;
}



void SID6510::INCSBC_indy(SID6510 *cpu)
{
	cpu->INCSBC_m(cpu->Indy());
	cpu->pPC++;
}



void SID6510::INCSBC_zp(SID6510 *cpu)
{
	cpu->INCSBC_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::INCSBC_zpx(SID6510 *cpu)
{
	cpu->INCSBC_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* LSREOR()                                                                   */
/******************************************************************************/
inline void SID6510::LSREOR_m(uint16 addr)
{
	uint16 x = LSR_m(readData(this, addr));
	writeData(this, addr, x);
	EOR_m(x);
}



inline void SID6510::LSREOR_m_zp(uint16 addr)
{
	uint16 x = LSR_m(ReadData_zp(addr));
	WriteData_zp(addr, x);
	EOR_m(x);
}



void SID6510::LSREOR_abso(SID6510 *cpu)
{
	cpu->LSREOR_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::LSREOR_absx(SID6510 *cpu)
{
	cpu->LSREOR_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::LSREOR_absy(SID6510 *cpu)
{
	cpu->LSREOR_m(cpu->Absy());
	cpu->pPC += 2;
}



void SID6510::LSREOR_indx(SID6510 *cpu)
{
	cpu->LSREOR_m(cpu->Indx());
	cpu->pPC++;
}



void SID6510::LSREOR_indy(SID6510 *cpu)
{
	cpu->LSREOR_m(cpu->Indy());
	cpu->pPC++;
}



void SID6510::LSREOR_zp(SID6510 *cpu)
{
	cpu->LSREOR_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::LSREOR_zpx(SID6510 *cpu)
{
	cpu->LSREOR_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* ROLAND()                                                                   */
/******************************************************************************/
inline void SID6510::ROLAND_m(uint16 addr)
{
	uint16 x = ROL_m(readData(this, addr));
	writeData(this, addr, x);
	AND_m(x);
}



inline void SID6510::ROLAND_m_zp(uint16 addr)
{
	uint16 x = ROL_m(ReadData_zp(addr));
	WriteData_zp(addr, x);
	AND_m(x);
}



void SID6510::ROLAND_abso(SID6510 *cpu)
{
	cpu->ROLAND_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::ROLAND_absx(SID6510 *cpu)
{
	cpu->ROLAND_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::ROLAND_absy(SID6510 *cpu)
{
	cpu->ROLAND_m(cpu->Absy());
	cpu->pPC += 2;
}



void SID6510::ROLAND_indx(SID6510 *cpu)
{
	cpu->ROLAND_m(cpu->Indx());
	cpu->pPC++;
}



void SID6510::ROLAND_indy(SID6510 *cpu)
{
	cpu->ROLAND_m(cpu->Indy());
	cpu->pPC++;
}



void SID6510::ROLAND_zp(SID6510 *cpu)
{
	cpu->ROLAND_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::ROLAND_zpx(SID6510 *cpu)
{
	cpu->ROLAND_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* RORADC()                                                                   */
/******************************************************************************/
inline void SID6510::RORADC_m(uint16 addr)
{
	uint8 x = ROR_m(readData(this, addr));
	writeData(this, addr, x);
	ADC_m(x);
}



inline void SID6510::RORADC_m_zp(uint16 addr)
{
	uint8 x = ROR_m(ReadData_zp(addr));
	WriteData_zp(addr, x);
	ADC_m(x);
}



void SID6510::RORADC_abso(SID6510 *cpu)
{
	cpu->RORADC_m(cpu->Abso());
	cpu->pPC += 2;
}



void SID6510::RORADC_absx(SID6510 *cpu)
{
	cpu->RORADC_m(cpu->Absx());
	cpu->pPC += 2;
}



void SID6510::RORADC_absy(SID6510 *cpu)
{
	cpu->RORADC_m(cpu->Absy());
	cpu->pPC += 2;
}



void SID6510::RORADC_indx(SID6510 *cpu)
{
	cpu->RORADC_m(cpu->Indx());
	cpu->pPC++;
}



void SID6510::RORADC_indy(SID6510 *cpu)
{
	cpu->RORADC_m(cpu->Indy());
	cpu->pPC++;
}



void SID6510::RORADC_zp(SID6510 *cpu)
{
	cpu->RORADC_m_zp(cpu->ZP());
	cpu->pPC++;
}



void SID6510::RORADC_zpx(SID6510 *cpu)
{
	cpu->RORADC_m_zp(cpu->ZPX());
	cpu->pPC++;
}



/******************************************************************************/
/* Illegal codes/instructions part (3). This implementation is considered to  */
/* be only partially working due to inconsistencies in the available          */
/* documentation.                                                             */
/* Note: In some of the functions emulated, defined instructions are used and */
/* already increment the PC! Take care, and do not increment further!         */
/* Double-setting of processor flags can occur, too.                          */
/******************************************************************************/
void SID6510::ILL_0B(SID6510 *cpu)	// Equal to 2B
{
	AND_imm(cpu);
	cpu->SR.Carry = cpu->SR.Negative;
}



void SID6510::ILL_4B(SID6510 *cpu)
{
	AND_imm(cpu);
	LSR_AC(cpu);
}



void SID6510::ILL_6B(SID6510 *cpu)
{
	if (cpu->SR.Decimal == 0)
	{
		AND_imm(cpu);
		ROR_AC(cpu);
		cpu->SR.Carry    = (cpu->AC & 1);
		cpu->SR.oVerflow = (cpu->AC >> 5) ^ (cpu->AC >> 6);
	}
}



void SID6510::ILL_83(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Indx(), cpu->AC & cpu->XR);
	cpu->pPC++;
}



void SID6510::ILL_87(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZP(), cpu->AC & cpu->XR);
	cpu->pPC++;
}



void SID6510::ILL_8B(SID6510 *cpu)
{
	TXA_(cpu);
	AND_imm(cpu);
}



void SID6510::ILL_8F(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Abso(), cpu->AC & cpu->XR);
	cpu->pPC += 2;
}



void SID6510::ILL_93(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Indy(), cpu->AC & cpu->XR & (1 + (cpu->readData(cpu, (*cpu->pPC) + 1) & 0xFF)));
	cpu->pPC++;
}



void SID6510::ILL_97(SID6510 *cpu)
{
	cpu->WriteData_zp(cpu->ZPX(), cpu->AC & cpu->XR);
	cpu->pPC++;
}



void SID6510::ILL_9B(SID6510 *cpu)
{
	cpu->SP = 0x100 | (cpu->AC & cpu->XR);
	cpu->writeData(cpu, cpu->Absy(), (cpu->SP & ((*cpu->pPC + 1) + 1)));
	cpu->pPC += 2;
	cpu->CheckSP();
}



void SID6510::ILL_9C(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Absx(), (cpu->YR & ((*cpu->pPC + 1) + 1)));
	cpu->pPC += 2;
}



void SID6510::ILL_9E(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Absy(), (cpu->XR & ((*cpu->pPC + 1) + 1)));
	cpu->pPC += 2;
}



void SID6510::ILL_9F(SID6510 *cpu)
{
	cpu->writeData(cpu, cpu->Absy(), (cpu->AC & cpu->XR & ((*cpu->pPC + 1) + 1)));
	cpu->pPC += 2;
}



void SID6510::ILL_A3(SID6510 *cpu)
{
	LDA_indx(cpu);
	TAX_(cpu);
}



void SID6510::ILL_A7(SID6510 *cpu)
{
	LDA_zp(cpu);
	TAX_(cpu);
}



// Taken from VICE because a single music player has been found
// (not in HVSC) which seems to need ILL_AB implemented like this
// (or similarly) in order to work
void SID6510::ILL_AB(SID6510 *cpu)
{
	cpu->AC = cpu->XR = ((cpu->AC | 0xee) & (*cpu->pPC++));
	cpu->AffectNZ(cpu->AC);
}



void SID6510::ILL_AF(SID6510 *cpu)
{
	LDA_abso(cpu);
	TAX_(cpu);
}



void SID6510::ILL_B3(SID6510 *cpu)
{
	LDA_indy(cpu);
	TAX_(cpu);
}



void SID6510::ILL_B7(SID6510 *cpu)
{
	cpu->AffectNZ(cpu->AC = cpu->ReadData_zp(cpu->ZPY()));	// Would be LDA_zpy()
	TAX_(cpu);
	cpu->pPC++;
}



void SID6510::ILL_BB(SID6510 *cpu)
{
	cpu->XR   = (cpu->SP & cpu->Absy());
	cpu->pPC += 2;
	TXS_(cpu);
	TXA_(cpu);
}



void SID6510::ILL_BF(SID6510 *cpu)
{
	LDA_absy(cpu);
	TAX_(cpu);
}



void SID6510::ILL_CB(SID6510 *cpu)
{
	uint16 tmp = cpu->XR & cpu->AC;
	tmp -= cpu->Imm();
	cpu->SR.Carry = (tmp > 255);
	cpu->AffectNZ(cpu->XR = (tmp & 255));
}



void SID6510::ILL_EB(SID6510 *cpu)
{
	SBC_imm(cpu);
}
