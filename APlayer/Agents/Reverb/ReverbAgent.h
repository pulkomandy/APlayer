/******************************************************************************/
/* Reverb Agent header file.                                                  */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __ReverbAgent_h
#define __ReverbAgent_h

// PolyKit headers
#include "POS.h"
#include "PString.h"

// APlayerKit headers
#include "APAddOns.h"


/******************************************************************************/
/* ReverbAgent class                                                          */
/******************************************************************************/
class ReverbAgent : public APAddOnAgent
{
public:
	ReverbAgent(APGlobalData *global, PString fileName);
	virtual ~ReverbAgent(void);

	virtual float GetVersion(void);

	virtual const APConfigInfo *GetConfigInfo(void);

	virtual uint32 GetSupportFlags(int32 index);
	virtual PString GetName(int32 index);
	virtual PString GetDescription(int32 index);

	virtual bool InitAgent(int32 index);
	virtual void EndAgent(int32 index);
	virtual ap_result Run(int32 index, uint32 command, void *args);

protected:
	void LoadSettings(void);
	void FixSettings(void);

	void DSP(APAgent_DSP *dspInfo);
	void Stop(void);

	// Reverb functions
	void AllocReverbBuffers(uint32 mixerFreq);
	void FreeReverbBuffers(void);
	void MixReverbMono(int32 *source, int32 count, uint8 rev);
	void MixReverbStereo(int32 *source, int32 count, uint8 rev);

	PResource *res;
	APConfigInfo cfgInfo;

	// Reverb variables
	bool allocated;

	int32 rvc1;
	int32 rvc2;
	int32 rvc3;
	int32 rvc4;
	int32 rvc5;
	int32 rvc6;
	int32 rvc7;
	int32 rvc8;

	uint32 rvrIndex;

	int32 *rvBufL1;
	int32 *rvBufL2;
	int32 *rvBufL3;
	int32 *rvBufL4;
	int32 *rvBufL5;
	int32 *rvBufL6;
	int32 *rvBufL7;
	int32 *rvBufL8;

	int32 *rvBufR1;
	int32 *rvBufR2;
	int32 *rvBufR3;
	int32 *rvBufR4;
	int32 *rvBufR5;
	int32 *rvBufR6;
	int32 *rvBufR7;
	int32 *rvBufR8;
};

#endif
