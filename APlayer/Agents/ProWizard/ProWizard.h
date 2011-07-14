/******************************************************************************/
/* ProWizard header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ProWizard_h
#define __ProWizard_h

// PolyKit headers
#include "POS.h"
#include "PFile.h"
#include "PBinary.h"

// APlayer headers
#include "APAddOns.h"


/******************************************************************************/
/* ProWizard class                                                            */
/******************************************************************************/
class ProWizard
{
public:
	ProWizard(void) {};
	virtual ~ProWizard(void) {};

	virtual uint32 CheckModule(const PBinary &module) = 0;
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile) = 0;
	virtual int32 GetModuleType(void) = 0;

protected:
	void InitPeriods(void);
	uint16 GetPGP(const uint8 *posTable, uint16 posSize);

	uint8 period[46][2];

	uint32 sampSize;
	uint16 sampNum;
	uint16 pattNum;
	uint16 posNum;
};



/******************************************************************************/
/* Special base classes                                                       */
/******************************************************************************/

/******************************************************************************/
/* BASE_CRYPTO class                                                          */
/******************************************************************************/
class BASE_CRYPTO : public ProWizard
{
public:
	BASE_CRYPTO(void) {};
	virtual ~BASE_CRYPTO(void) {};

protected:
	bool Check(const uint8 *modStart, const uint8 *checkStart);
};



/******************************************************************************/
/* Converter classes                                                          */
/******************************************************************************/

/******************************************************************************/
/* PROZ_AC1D class                                                            */
/******************************************************************************/
class PROZ_AC1D : public ProWizard
{
public:
	PROZ_AC1D(void) {};
	virtual ~PROZ_AC1D(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_CHAN class                                                            */
/******************************************************************************/
class PROZ_CHAN : public ProWizard
{
public:
	PROZ_CHAN(void) {};
	virtual ~PROZ_CHAN(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Chan1Speco(const uint8 *mod);
	void Chan2Speco(const uint8 *mod);

	void ConvertChannel1(const uint8 *mod, PFile *destFile);
	void ConvertChannel2(const uint8 *mod, PFile *destFile);
	void ConvertChannel3(const uint8 *mod, PFile *destFile);

	uint32 type;
	uint32 trackTable;
	uint8 posTable[128];
};



/******************************************************************************/
/* PROZ_DI class                                                              */
/******************************************************************************/
class PROZ_DI : public ProWizard
{
public:
	PROZ_DI(void) {};
	virtual ~PROZ_DI(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_EUREKA class                                                          */
/******************************************************************************/
class PROZ_EUREKA : public ProWizard
{
public:
	PROZ_EUREKA(void) {};
	virtual ~PROZ_EUREKA(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_FC_M class                                                            */
/******************************************************************************/
class PROZ_FC_M : public ProWizard
{
public:
	PROZ_FC_M(void) {};
	virtual ~PROZ_FC_M(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	const uint8 *FindPart(const uint8 *start, uint32 length, uint32 chunk);
};



/******************************************************************************/
/* PROZ_FUCHS class                                                           */
/******************************************************************************/
class PROZ_FUCHS : public ProWizard
{
public:
	PROZ_FUCHS(void) {};
	virtual ~PROZ_FUCHS(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	uint32 pw_j;
	uint32 pw_k;
	uint32 pw_m;
	uint32 pw_n;
	uint32 pw_o;
};



/******************************************************************************/
/* PROZ_FUZZAC class                                                          */
/******************************************************************************/
class PROZ_FUZZAC : public ProWizard
{
public:
	PROZ_FUZZAC(void) {};
	virtual ~PROZ_FUZZAC(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	const uint8 *notes;
	const uint8 *samples;
	uint16 *fuzzacTable;
	uint8 posTable[128];
};



/******************************************************************************/
/* PROZ_GMC class                                                             */
/******************************************************************************/
class PROZ_GMC : public ProWizard
{
public:
	PROZ_GMC(void) {};
	virtual ~PROZ_GMC(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_HEATSEEK class                                                        */
/******************************************************************************/
class PROZ_HEATSEEK : public BASE_CRYPTO
{
public:
	PROZ_HEATSEEK(void) {};
	virtual ~PROZ_HEATSEEK(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_KRIS class                                                            */
/******************************************************************************/
class PROZ_KRIS : public ProWizard
{
public:
	PROZ_KRIS(void) {};
	virtual ~PROZ_KRIS(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];

	uint16 biggestPatt;
};



/******************************************************************************/
/* PROZ_KSM class                                                             */
/******************************************************************************/
class PROZ_KSM : public ProWizard
{
public:
	PROZ_KSM(void) {};
	virtual ~PROZ_KSM(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];
};



/******************************************************************************/
/* PROZ_MP class                                                              */
/******************************************************************************/
class PROZ_MP : public BASE_CRYPTO
{
public:
	PROZ_MP(void) {};
	virtual ~PROZ_MP(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_NP2 class                                                             */
/******************************************************************************/
class PROZ_NP2 : public ProWizard
{
public:
	PROZ_NP2(void) {};
	virtual ~PROZ_NP2(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void BuildVal1(uint8 dat1);
	void BuildVal2(uint8 dat2, uint8 dat3);

	uint32 mark;
	uint16 val1;
	uint16 val2;
};



/******************************************************************************/
/* PROZ_NP3 class                                                             */
/******************************************************************************/
class PROZ_NP3 : public ProWizard
{
public:
	PROZ_NP3(void) {};
	virtual ~PROZ_NP3(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void BuildVal1(uint8 dat1);
	void BuildVal2(uint8 dat2, uint8 dat3);

	uint16 val1;
	uint16 val2;
};



/******************************************************************************/
/* PROZ_NRU class                                                             */
/******************************************************************************/
class PROZ_NRU : public ProWizard
{
public:
	PROZ_NRU(void) {};
	virtual ~PROZ_NRU(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_NTPK class                                                            */
/******************************************************************************/
class PROZ_NTPK : public ProWizard
{
public:
	PROZ_NTPK(void) {};
	virtual ~PROZ_NTPK(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_P41A class                                                            */
/******************************************************************************/
typedef struct P4xxSampInfo
{
	uint32	startOffset;
	uint16	length;
	uint32	loopOffset;
	uint16	loopLength;
	uint16	volume;
	uint16	fineTune;
} P4xxSampInfo;


typedef struct P4xxChannel
{
	uint8	p4xPattData[3];
	int8	p4xInfo;
	const uint8	*repeatAdr;
	uint16	repeatLines;
	uint8	proPattData[4];
} P4xxChannel;


class PROZ_P41A : public ProWizard
{
public:
	PROZ_P41A(void) {};
	virtual ~PROZ_P41A(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);
	void DeInit(void);
	void ConvData(P4xxChannel *chan, const uint8 **trkAdr, const uint8 *mod);

	P4xxSampInfo sampInfo[31];

	uint32 type;
	uint32 trackOffset;
	uint32 tda;				// Track Data Address
	uint32 tta;				// Track Table Address
	uint32 sda;				// Sample Data Address
	uint32 endOffset;

	uint8 *pattern;
	uint32 writeOffset;
	bool breakFlag;
};



/******************************************************************************/
/* PROZ_P61A class                                                            */
/******************************************************************************/
typedef struct P61ASampInfo
{
	uint32	startOffset;
	uint16	length;
	uint8	fineTune;
	uint8	volume;
	uint16	loopStart;
	uint16	loopLength;
} P61ASampInfo;


class PROZ_P61A : public ProWizard
{
public:
	PROZ_P61A(void) {};
	virtual ~PROZ_P61A(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void GetNote(uint8 trackByte, uint8 &byte1, uint8 &byte2);
	void CheckEffect(uint8 &byte3, uint8 &byte4);
	void P60A_ConvertPatterns(const uint8 *mod);
	void P61A_ConvertPatterns(const uint8 *mod);

	P61ASampInfo sampInfo[31];

	uint32 type;
	uint32 sampType;
	uint32 startOffset;
	uint32 sampInfoOffset;
	uint32 sampOffset;
	uint32 trackOffset;
	uint32 noteOffset;
	uint32 destSampSize;

	uint8 *pattern;
};



/******************************************************************************/
/* PROZ_PHA class                                                             */
/******************************************************************************/
class PROZ_PHA : public ProWizard
{
public:
	PROZ_PHA(void) {};
	virtual ~PROZ_PHA(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];

	uint32 endMod;
};



/******************************************************************************/
/* PROZ_PM01 class                                                            */
/******************************************************************************/
class PROZ_PM01 : public ProWizard
{
public:
	PROZ_PM01(void) {};
	virtual ~PROZ_PM01(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void AdjustFinetunes(uint8 *pattern);

	uint8 finetune[31];
};



/******************************************************************************/
/* PROZ_PM18 class                                                            */
/******************************************************************************/
class PROZ_PM18 : public PROZ_PM01
{
public:
	PROZ_PM18(void) {};
	virtual ~PROZ_PM18(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];

	uint32 type;
};



/******************************************************************************/
/* PROZ_PM20 class                                                            */
/******************************************************************************/
class PROZ_PM20 : public ProWizard
{
public:
	PROZ_PM20(void) {};
	virtual ~PROZ_PM20(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];
};



/******************************************************************************/
/* PROZ_PM40 class                                                            */
/******************************************************************************/
class PROZ_PM40 : public ProWizard
{
public:
	PROZ_PM40(void) {};
	virtual ~PROZ_PM40(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];
};



/******************************************************************************/
/* PROZ_POWER class                                                           */
/******************************************************************************/
class PROZ_POWER : public ProWizard
{
public:
	PROZ_POWER(void) {};
	virtual ~PROZ_POWER(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_PP10 class                                                            */
/******************************************************************************/
class PROZ_PP10 : public BASE_CRYPTO
{
public:
	PROZ_PP10(void) {};
	virtual ~PROZ_PP10(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(void);

	uint8 trackTable[128 * 4];
	uint8 posTable[128];

	uint32 trackNum;
};



/******************************************************************************/
/* PROZ_PP30 class                                                            */
/******************************************************************************/
class PROZ_PP30 : public BASE_CRYPTO
{
public:
	PROZ_PP30(void) {};
	virtual ~PROZ_PP30(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(void);

	uint8 trackTable[128 * 4];
	uint8 posTable[128];

	uint32 trackNum;
	uint16 type;
};



/******************************************************************************/
/* PROZ_PRU1 class                                                            */
/******************************************************************************/
class PROZ_PRU1 : public ProWizard
{
public:
	PROZ_PRU1(void) {};
	virtual ~PROZ_PRU1(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	bool hornet;
	uint32 biggestPatt;
};



/******************************************************************************/
/* PROZ_HRT class                                                             */
/******************************************************************************/
class PROZ_HRT : public PROZ_PRU1
{
public:
	PROZ_HRT(void) {};
	virtual ~PROZ_HRT(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_PRU2 class                                                            */
/******************************************************************************/
class PROZ_PRU2 : public ProWizard
{
public:
	PROZ_PRU2(void) {};
	virtual ~PROZ_PRU2(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_PYGMY class                                                           */
/******************************************************************************/
class PROZ_PYGMY : public ProWizard
{
public:
	PROZ_PYGMY(void) {};
	virtual ~PROZ_PYGMY(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	uint8 ConvertEffect(uint8 eff, uint8 &effVal);
};



/******************************************************************************/
/* PROZ_SKYT class                                                            */
/******************************************************************************/
class PROZ_SKYT : public ProWizard
{
public:
	PROZ_SKYT(void) {};
	virtual ~PROZ_SKYT(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_ST26 class                                                            */
/******************************************************************************/
class PROZ_ST26 : public ProWizard
{
public:
	PROZ_ST26(void) {};
	virtual ~PROZ_ST26(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];
	uint32 mark;
};



/******************************************************************************/
/* PROZ_STAR class                                                            */
/******************************************************************************/
class PROZ_STAR : public ProWizard
{
public:
	PROZ_STAR(void) {};
	virtual ~PROZ_STAR(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];
};



/******************************************************************************/
/* PROZ_STIM class                                                            */
/******************************************************************************/
class PROZ_STIM : public ProWizard
{
public:
	PROZ_STIM(void) {};
	virtual ~PROZ_STIM(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	uint32 pw_wholeSampleSize;
	uint32 pw_j;
	uint32 pw_k;
	uint32 pw_l;
	uint32 pw_m;
	uint32 pw_o;
};



/******************************************************************************/
/* PROZ_TDD class                                                             */
/******************************************************************************/
class PROZ_TDD : public ProWizard
{
public:
	PROZ_TDD(void) {};
	virtual ~PROZ_TDD(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	uint32 pw_wholeSampleSize;
	uint32 pw_j;
	uint32 pw_k;
	uint32 pw_l;
	uint32 pw_m;
	uint32 pw_n;
};



/******************************************************************************/
/* PROZ_TP1 class                                                             */
/******************************************************************************/
class PROZ_TP1 : public ProWizard
{
public:
	PROZ_TP1(void) {};
	virtual ~PROZ_TP1(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(const uint8 *mod);

	uint8 posTable[128];
	uint32 samples;
};



/******************************************************************************/
/* PROZ_TP3 class                                                             */
/******************************************************************************/
class PROZ_TP3 : public ProWizard
{
public:
	PROZ_TP3(void) {};
	virtual ~PROZ_TP3(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void ConvertEffect(uint8 &effect, uint8 &effectVal);

	uint16 type;
	uint32 tracks;
	uint32 notes;
	uint32 samples;
};



/******************************************************************************/
/* PROZ_UNIC class                                                            */
/******************************************************************************/
class PROZ_UNIC : public ProWizard
{
public:
	PROZ_UNIC(void) {};
	virtual ~PROZ_UNIC(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	bool CheckPatterns(const uint8 *patt);

	uint32 type;
};



/******************************************************************************/
/* PROZ_LAX class                                                             */
/******************************************************************************/
class PROZ_LAX : public PROZ_UNIC
{
public:
	PROZ_LAX(void) {};
	virtual ~PROZ_LAX(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual int32 GetModuleType(void);
};



/******************************************************************************/
/* PROZ_WP class                                                              */
/******************************************************************************/
class PROZ_WP : public ProWizard
{
public:
	PROZ_WP(void) {};
	virtual ~PROZ_WP(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void ConvertPatternData(const uint8 *pattStart, PFile *destFile);

	uint16 biggestPatt;
};



/******************************************************************************/
/* PROZ_XANN class                                                            */
/******************************************************************************/
class PROZ_XANN : public ProWizard
{
public:
	PROZ_XANN(void) {};
	virtual ~PROZ_XANN(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	uint32 lowPattOffset;
	uint32 hiPattOffset;
	uint32 origine;

	uint32 newPattOffset[128];
	uint32 newSampAddr[31];
	uint32 newSampLoopAddr[31];
};



/******************************************************************************/
/* PROZ_ZEN class                                                             */
/******************************************************************************/
class PROZ_ZEN : public ProWizard
{
public:
	PROZ_ZEN(void) {};
	virtual ~PROZ_ZEN(void) {};

	virtual uint32 CheckModule(const PBinary &module);
	virtual ap_result ConvertModule(const PBinary &module, PFile *destFile);
	virtual int32 GetModuleType(void);

protected:
	void Speco(void);

	uint32 tableOffset;

	uint32 newPattOffset[128];
	uint32 newSampAddr[31];
	uint32 newSampLoopAddr[31];
	uint8 posTable[128];
};

#endif
