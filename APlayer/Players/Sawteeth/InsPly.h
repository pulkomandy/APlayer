/******************************************************************************/
/* InsPly header file.                                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __InsPly_h
#define __InsPly_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "Sawteeth.h"
#include "LFO.h"
#include "Wave.h"


/******************************************************************************/
/* InsPly class                                                               */
/******************************************************************************/
class InsPly
{
public:
	InsPly(Sawteeth *s);
	virtual ~InsPly(void);

	void TrigADSR(uint8 i);
	void SetPWMOffs(float a);
	void SetReso(float a);
	void SetAmp(float a);
	void SetFreq(float f);
	void SetCutOff(float co);

	bool Next(float *buffer, uint32 count);

protected:
	// Filter
	void SLP(float *b, uint32 count);
	void OLP(float *b, uint32 count);
	void LP(float *b, uint32 count);
	void HP(float *b, uint32 count);
	void BP(float *b, uint32 count);
	void BS(float *b, uint32 count);

	void VanillaClip(float *b, uint32 count, float mul);
	void SinusClip(float *b, uint32 count, float mul);

	// Clipping
	LFO *vib;
	LFO *pwm;
	float vamp;
	float pamp;
	float pwmOffs;

	Sawteeth *song;
	Wave *w;
	Ins *ins;
	InsStep *currStep;
	uint8 stepC;			// Instrument step
	int32 nextS;			// Next ins

	Ins *currIns;
	InsStep *steps;

	// Amp ADSR
	bool trigged;
	float currAmp;
	float ampStep;
	int8 adsr;
	int32 nextAdsr;

	// Filter ADSR
	float currF;
	float fStep;
	int8 fAdsr;
	int32 nextFAdsr;

	// From inspattern
	float insFreq;

	// From player
	float currPartFreq;
	float currPartAmp;
	float currPartCO;

	// Curr
	float res;
	float amp;
	float cutOff;

	// For filter
	float lo;
	float hi;
	float bp;
	float bs;
};

#endif
