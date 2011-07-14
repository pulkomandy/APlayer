/******************************************************************************/
/* SID6510 header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SID6510_h
#define __SID6510_h

// PolyKit headers
#include "POS.h"


/******************************************************************************/
/* Function pointers                                                          */
/******************************************************************************/
class SID6510;

typedef uint8 (*ReadData)(SID6510 *, uint16);
typedef void (*WriteData)(SID6510 *, uint16, uint8);
typedef void (*InstrFunc)(SID6510 *);



/******************************************************************************/
/* SID6510 class                                                              */
/******************************************************************************/
class SID6510
{
public:
	SID6510(void);
	virtual ~SID6510(void);

	bool C64MemAlloc(void);
	bool C64MemFree(void);
	void C64MemClear(void);
	void C64MemReset(int32 clockSpeed, uint8 randomSeed);
	uint8 C64MemRamRom(uint16 address);

	void InitInterpreter(int32 inMemoryMode);
	bool Interpreter(uint16 p, uint8 ramRom, uint8 a, uint8 x, uint8 y);

	uint8 *c64Mem1;			// 64KB C64-RAM
	uint8 *c64Mem2;			// Basic-ROM, VIC, SID, I/O, Kernal-ROM

	uint8 optr3ReadWave;	// D41B
	uint8 optr3ReadEnve;	// D41C

	bool sidKeysOff[32];	// Key_off detection
	bool sidKeysOn[32];		// Key_on detection

protected:
	typedef struct StatusRegister
	{
		uint8 Carry     : 1;
		uint8 Zero      : 1;
		uint8 Interrupt : 1;
		uint8 Decimal   : 1;
		uint8 Break     : 1;
		uint8 notUsed   : 1;
		uint8 oVerflow  : 1;
		uint8 Negative  : 1;
	} StatusRegister;

	inline void EvalBankSelect(void);
	inline void EvalBankJump(void);

	inline void ResetSR(void);
	inline uint8 CodeSR(void);
	inline void DecodeSR(uint8 stackByte);
	inline void AffectNZ(uint8 reg);

	inline void ResetSP(void);
	inline void CheckSP(void);

	static uint8 ReadData_bs(SID6510 *obj, uint16 addr);
	static uint8 ReadData_transp(SID6510 *obj, uint16 addr);
	static uint8 ReadData_plain(SID6510 *obj, uint16 addr);
	inline uint8 ReadData_zp(uint16 addr);

	static void WriteData_bs(SID6510 *obj, uint16 addr, uint8 data);
	static void WriteData_plain(SID6510 *obj, uint16 addr, uint8 data);
	inline void WriteData_zp(uint16 addr, uint8 data);

	inline uint16 Abso(void);
	inline uint16 Absx(void);
	inline uint16 Absy(void);
	inline uint8 Imm(void);
	inline uint16 Indx(void);
	inline uint16 Indy(void);
	inline uint8 ZP(void);
	inline uint8 ZPX(void);
	inline uint8 ZPY(void);

	inline void BranchIfClear(uint8 flag);
	inline void BranchIfSet(uint8 flag);

	inline void ADC_m(uint8 x);
	static void ADC_abso(SID6510 *cpu);
	static void ADC_absx(SID6510 *cpu);
	static void ADC_absy(SID6510 *cpu);
	static void ADC_imm(SID6510 *cpu);
	static void ADC_indx(SID6510 *cpu);
	static void ADC_indy(SID6510 *cpu);
	static void ADC_zp(SID6510 *cpu);
	static void ADC_zpx(SID6510 *cpu);

	inline void AND_m(uint8 x);
	static void AND_abso(SID6510 *cpu);
	static void AND_absx(SID6510 *cpu);
	static void AND_absy(SID6510 *cpu);
	static void AND_imm(SID6510 *cpu);
	static void AND_indx(SID6510 *cpu);
	static void AND_indy(SID6510 *cpu);
	static void AND_zp(SID6510 *cpu);
	static void AND_zpx(SID6510 *cpu);

	inline uint8 ASL_m(uint8 x);
	static void ASL_AC(SID6510 *cpu);
	static void ASL_abso(SID6510 *cpu);
	static void ASL_absx(SID6510 *cpu);
	static void ASL_zp(SID6510 *cpu);
	static void ASL_zpx(SID6510 *cpu);

	static void BCC_(SID6510 *cpu);
	static void BCS_(SID6510 *cpu);
	static void BEQ_(SID6510 *cpu);

	inline void BIT_m(uint8 x);
	static void BIT_abso(SID6510 *cpu);
	static void BIT_zp(SID6510 *cpu);

	static void BMI_(SID6510 *cpu);
	static void BNE_(SID6510 *cpu);
	static void BPL_(SID6510 *cpu);

	static void BRK_(SID6510 *cpu);

	static void BVC_(SID6510 *cpu);
	static void BVS_(SID6510 *cpu);

	static void CLC_(SID6510 *cpu);
	static void CLD_(SID6510 *cpu);
	static void CLI_(SID6510 *cpu);
	static void CLV_(SID6510 *cpu);

	inline void CMP_m(uint8 x);
	static void CMP_abso(SID6510 *cpu);
	static void CMP_absx(SID6510 *cpu);
	static void CMP_absy(SID6510 *cpu);
	static void CMP_imm(SID6510 *cpu);
	static void CMP_indx(SID6510 *cpu);
	static void CMP_indy(SID6510 *cpu);
	static void CMP_zp(SID6510 *cpu);
	static void CMP_zpx(SID6510 *cpu);

	inline void CPX_m(uint8 x);
	static void CPX_abso(SID6510 *cpu);
	static void CPX_imm(SID6510 *cpu);
	static void CPX_zp(SID6510 *cpu);

	inline void CPY_m(uint8 x);
	static void CPY_abso(SID6510 *cpu);
	static void CPY_imm(SID6510 *cpu);
	static void CPY_zp(SID6510 *cpu);

	inline void DEC_m(uint16 addr);
	inline void DEC_m_zp(uint16 addr);
	static void DEC_abso(SID6510 *cpu);
	static void DEC_absx(SID6510 *cpu);
	static void DEC_zp(SID6510 *cpu);
	static void DEC_zpx(SID6510 *cpu);

	static void DEX_(SID6510 *cpu);
	static void DEY_(SID6510 *cpu);

	inline void EOR_m(uint8 x);
	static void EOR_abso(SID6510 *cpu);
	static void EOR_absx(SID6510 *cpu);
	static void EOR_absy(SID6510 *cpu);
	static void EOR_imm(SID6510 *cpu);
	static void EOR_indx(SID6510 *cpu);
	static void EOR_indy(SID6510 *cpu);
	static void EOR_zp(SID6510 *cpu);
	static void EOR_zpx(SID6510 *cpu);

	inline void INC_m(uint16 addr);
	inline void INC_m_zp(uint16 addr);
	static void INC_abso(SID6510 *cpu);
	static void INC_absx(SID6510 *cpu);
	static void INC_zp(SID6510 *cpu);
	static void INC_zpx(SID6510 *cpu);

	static void INX_(SID6510 *cpu);
	static void INY_(SID6510 *cpu);

	static void JMP_(SID6510 *cpu);
	static void JMP_plain(SID6510 *cpu);
	static void JMP_transp(SID6510 *cpu);
	static void JMP_vec(SID6510 *cpu);
	static void JMP_vec_plain(SID6510 *cpu);
	static void JMP_vec_transp(SID6510 *cpu);

	inline void JSR_main(void);
	static void JSR_(SID6510 *cpu);
	static void JSR_plain(SID6510 *cpu);
	static void JSR_transp(SID6510 *cpu);

	static void LDA_abso(SID6510 *cpu);
	static void LDA_absx(SID6510 *cpu);
	static void LDA_absy(SID6510 *cpu);
	static void LDA_imm(SID6510 *cpu);
	static void LDA_indx(SID6510 *cpu);
	static void LDA_indy(SID6510 *cpu);
	static void LDA_zp(SID6510 *cpu);
	static void LDA_zpx(SID6510 *cpu);

	static void LDX_abso(SID6510 *cpu);
	static void LDX_absy(SID6510 *cpu);
	static void LDX_imm(SID6510 *cpu);
	static void LDX_zp(SID6510 *cpu);
	static void LDX_zpy(SID6510 *cpu);

	static void LDY_abso(SID6510 *cpu);
	static void LDY_absx(SID6510 *cpu);
	static void LDY_imm(SID6510 *cpu);
	static void LDY_zp(SID6510 *cpu);
	static void LDY_zpx(SID6510 *cpu);

	inline uint8 LSR_m(uint8 x);
	static void LSR_AC(SID6510 *cpu);
	static void LSR_abso(SID6510 *cpu);
	static void LSR_absx(SID6510 *cpu);
	static void LSR_zp(SID6510 *cpu);
	static void LSR_zpx(SID6510 *cpu);

	static void NOP_(SID6510 *cpu);

	inline void ORA_m(uint8 x);
	static void ORA_abso(SID6510 *cpu);
	static void ORA_absx(SID6510 *cpu);
	static void ORA_absy(SID6510 *cpu);
	static void ORA_imm(SID6510 *cpu);
	static void ORA_indx(SID6510 *cpu);
	static void ORA_indy(SID6510 *cpu);
	static void ORA_zp(SID6510 *cpu);
	static void ORA_zpx(SID6510 *cpu);

	static void PHA_(SID6510 *cpu);
	static void PHP_(SID6510 *cpu);
	static void PLA_(SID6510 *cpu);
	static void PLP_(SID6510 *cpu);

	inline uint8 ROL_m(uint8 x);
	static void ROL_AC(SID6510 *cpu);
	static void ROL_abso(SID6510 *cpu);
	static void ROL_absx(SID6510 *cpu);
	static void ROL_zp(SID6510 *cpu);
	static void ROL_zpx(SID6510 *cpu);

	inline uint8 ROR_m(uint8 x);
	static void ROR_AC(SID6510 *cpu);
	static void ROR_abso(SID6510 *cpu);
	static void ROR_absx(SID6510 *cpu);
	static void ROR_zp(SID6510 *cpu);
	static void ROR_zpx(SID6510 *cpu);

	static void RTI_(SID6510 *cpu);
	static inline void RTS_(SID6510 *cpu);

	inline void SBC_m(uint8 s);
	static void SBC_abso(SID6510 *cpu);
	static void SBC_absx(SID6510 *cpu);
	static void SBC_absy(SID6510 *cpu);
	static void SBC_imm(SID6510 *cpu);
	static void SBC_indx(SID6510 *cpu);
	static void SBC_indy(SID6510 *cpu);
	static void SBC_zp(SID6510 *cpu);
	static void SBC_zpx(SID6510 *cpu);

	static void SEC_(SID6510 *cpu);
	static void SED_(SID6510 *cpu);
	static void SEI_(SID6510 *cpu);

	static void STA_abso(SID6510 *cpu);
	static void STA_absx(SID6510 *cpu);
	static void STA_absy(SID6510 *cpu);
	static void STA_indx(SID6510 *cpu);
	static void STA_indy(SID6510 *cpu);
	static void STA_zp(SID6510 *cpu);
	static void STA_zpx(SID6510 *cpu);

	static void STX_abso(SID6510 *cpu);
	static void STX_zp(SID6510 *cpu);
	static void STX_zpy(SID6510 *cpu);

	static void STY_abso(SID6510 *cpu);
	static void STY_zp(SID6510 *cpu);
	static void STY_zpx(SID6510 *cpu);

	static void TAX_(SID6510 *cpu);
	static void TAY_(SID6510 *cpu);
	static void TSX_(SID6510 *cpu);
	static void TXA_(SID6510 *cpu);
	static void TXS_(SID6510 *cpu);
	static void TYA_(SID6510 *cpu);

	static void ILL_TILT(SID6510 *cpu);
	static void ILL_1NOP(SID6510 *cpu);
	static void ILL_2NOP(SID6510 *cpu);
	static void ILL_3NOP(SID6510 *cpu);

	inline void ASLORA_m(uint16 addr);
	inline void ASLORA_m_zp(uint16 addr);
	static void ASLORA_abso(SID6510 *cpu);
	static void ASLORA_absx(SID6510 *cpu);
	static void ASLORA_absy(SID6510 *cpu);
	static void ASLORA_indx(SID6510 *cpu);
	static void ASLORA_indy(SID6510 *cpu);
	static void ASLORA_zp(SID6510 *cpu);
	static void ASLORA_zpx(SID6510 *cpu);

	inline void DECCMP_m(uint16 addr);
	inline void DECCMP_m_zp(uint16 addr);
	static void DECCMP_abso(SID6510 *cpu);
	static void DECCMP_absx(SID6510 *cpu);
	static void DECCMP_absy(SID6510 *cpu);
	static void DECCMP_indx(SID6510 *cpu);
	static void DECCMP_indy(SID6510 *cpu);
	static void DECCMP_zp(SID6510 *cpu);
	static void DECCMP_zpx(SID6510 *cpu);

	inline void INCSBC_m(uint16 addr);
	inline void INCSBC_m_zp(uint16 addr);
	static void INCSBC_abso(SID6510 *cpu);
	static void INCSBC_absx(SID6510 *cpu);
	static void INCSBC_absy(SID6510 *cpu);
	static void INCSBC_indx(SID6510 *cpu);
	static void INCSBC_indy(SID6510 *cpu);
	static void INCSBC_zp(SID6510 *cpu);
	static void INCSBC_zpx(SID6510 *cpu);

	inline void LSREOR_m(uint16 addr);
	inline void LSREOR_m_zp(uint16 addr);
	static void LSREOR_abso(SID6510 *cpu);
	static void LSREOR_absx(SID6510 *cpu);
	static void LSREOR_absy(SID6510 *cpu);
	static void LSREOR_indx(SID6510 *cpu);
	static void LSREOR_indy(SID6510 *cpu);
	static void LSREOR_zp(SID6510 *cpu);
	static void LSREOR_zpx(SID6510 *cpu);

	inline void ROLAND_m(uint16 addr);
	inline void ROLAND_m_zp(uint16 addr);
	static void ROLAND_abso(SID6510 *cpu);
	static void ROLAND_absx(SID6510 *cpu);
	static void ROLAND_absy(SID6510 *cpu);
	static void ROLAND_indx(SID6510 *cpu);
	static void ROLAND_indy(SID6510 *cpu);
	static void ROLAND_zp(SID6510 *cpu);
	static void ROLAND_zpx(SID6510 *cpu);

	inline void RORADC_m(uint16 addr);
	inline void RORADC_m_zp(uint16 addr);
	static void RORADC_abso(SID6510 *cpu);
	static void RORADC_absx(SID6510 *cpu);
	static void RORADC_absy(SID6510 *cpu);
	static void RORADC_indx(SID6510 *cpu);
	static void RORADC_indy(SID6510 *cpu);
	static void RORADC_zp(SID6510 *cpu);
	static void RORADC_zpx(SID6510 *cpu);

	static void ILL_0B(SID6510 *cpu);
	static void ILL_4B(SID6510 *cpu);
	static void ILL_6B(SID6510 *cpu);
	static void ILL_83(SID6510 *cpu);
	static void ILL_87(SID6510 *cpu);
	static void ILL_8B(SID6510 *cpu);
	static void ILL_8F(SID6510 *cpu);
	static void ILL_93(SID6510 *cpu);
	static void ILL_97(SID6510 *cpu);
	static void ILL_9B(SID6510 *cpu);
	static void ILL_9C(SID6510 *cpu);
	static void ILL_9E(SID6510 *cpu);
	static void ILL_9F(SID6510 *cpu);
	static void ILL_A3(SID6510 *cpu);
	static void ILL_A7(SID6510 *cpu);
	static void ILL_AB(SID6510 *cpu);
	static void ILL_AF(SID6510 *cpu);
	static void ILL_B3(SID6510 *cpu);
	static void ILL_B7(SID6510 *cpu);
	static void ILL_BB(SID6510 *cpu);
	static void ILL_BF(SID6510 *cpu);
	static void ILL_CB(SID6510 *cpu);
	static void ILL_EB(SID6510 *cpu);

	uint8 *c64RamBuf;
	uint8 *c64RomBuf;

	int32 memoryMode;
	uint8 *bankSelReg;		// Pointer to RAM[1], bank-select register

	uint8 AC;				// 6510 processor registers
	uint8 XR;
	uint8 YR;
	StatusRegister SR;
	uint16 PC;				// Program-counter (Only used temporarily!)
	uint16 SP;				// Stack-pointer

	bool stackIsOkay;

	uint8 *pPCBase;			// Pointer to RAM/ROM buffer base
	uint8 *pPCEnd;			// Pointer to RAM/ROM buffer end
	uint8 *pPC;

	bool isBasic;			// These flags are used to not have to repeatedly
	bool isIO;				// Evaluate the bank-select register for each
	bool isKernal;			// Address operand

	uint32 fakeReadTimer;

	uint8 sidLastValue;		// Last value written to the SID

	// Use pointers to allow plain-memory modifications
	ReadData readData;
	WriteData writeData;

	InstrFunc instrList[256];
};

#endif
