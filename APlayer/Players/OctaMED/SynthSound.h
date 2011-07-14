/******************************************************************************/
/* SynthSound header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __SynthSound_h
#define __SynthSound_h

// PolyKit headers
#include "POS.h"
#include "PList.h"

// Player headers
#include "MEDTypes.h"
#include "OctaMED.h"
#include "Sample.h"


/******************************************************************************/
/* SynthWF class                                                              */
/******************************************************************************/
class SynthWF
{
public:
	uint32 sywfLength;				// Length in WORDS, not bytes/samples
	int8 sywfData[128];
};



/******************************************************************************/
/* SynthSound class                                                           */
/******************************************************************************/
class SynthSound : public Sample, public PList<SynthWF *>
{
public:
	// Common commands (in both vol table and waveform table)
	enum
	{
		CMD_SPD = 0xf0, CMD_WAI = 0xf1, CMD_CHD = 0xf2, CMD_CHU = 0xf3,
		CMD_RES = 0xf6, CMD_HLT = 0xfb, CMD_JMP = 0xfe, CMD_END = 0xff
	};

	// Volume commands (in vol table)
	enum
	{
		VOLCMD_EN1 = 0xf4, VOLCMD_EN2 = 0xf5, VOLCMD_JWS = 0xfa
	};

	// Waveform commands (in waveform table)
	enum
	{
		WFCMD_VBD = 0xf4, WFCMD_VBS = 0xf5, WFCMD_VWF = 0xf7,
		WFCMD_JVS = 0xfa, WFCMD_ARP = 0xfc, WFCMD_ARE = 0xfd
	};

	SynthSound(OctaMED *octaMED);
	virtual ~SynthSound(void);

	void SetVolSpeed(uint32 spd);
	void SetWFSpeed(uint32 spd);
	void SetVolTableLen(uint32 tbl);
	void SetWFTableLen(uint32 tbl);
	void SetVolData(uint32 pos, uint8 data);
	void SetWFData(uint32 pos, uint8 data);

	uint32 GetVolSpeed(void) const;
	uint32 GetWFSpeed(void) const;
	uint8 GetVolData(uint32 pos) const;
	uint8 GetWFData(uint32 pos) const;

	virtual bool IsSynthSound(void) const;

protected:
	uint32 sy_VolTableLen;
	uint32 sy_WFTableLen;
	uint32 sy_VolSpeed;
	uint32 sy_WFSpeed;
	uint8 sy_VolTable[128];
	uint8 sy_WFTable[128];
};

#endif
