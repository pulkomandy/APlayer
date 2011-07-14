/******************************************************************************/
/* Wave header file.                                                          */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __Wave_h
#define __Wave_h

// PolyKit headers
#include "POS.h"

// Player headers
#include "Sawteeth.h"


/******************************************************************************/
/* Waveform types                                                             */
/******************************************************************************/
enum
{
	HOLD = 0,
	SAW = 1,
	SQR = 2,
	TRI = 3,
	NOS = 4,
	SIN = 5,

	TRIU = 6,
	SINU = 7
};



/******************************************************************************/
/* Wave class                                                                 */
/******************************************************************************/
class Wave
{
public:
	Wave(void);
	virtual ~Wave(void);

	void SetFreq(float freq);
	void SetPWM(float p);
	void SetForm(uint8 waveForm);
	void SetAmp(float a);

	void NoInterp(void);

	bool Next(float *buffer, uint32 count);

protected:
	void SInit(void);

	void FillSaw(float *out, uint32 count, float amp);
	void FillSquare(float *out, uint32 count, float amp);
	void FillTri(float *out, uint32 count, float amp);
	void FillNoise(float *out, uint32 count, float amp);
	void FillSin(float *out, uint32 count, float amp);
	void FillTriU(float *out, uint32 count, float amp);
	void FillSinU(float *out, uint32 count, float amp);

	uint32 jngRand(void);

	uint8 form;

	bool _pwmLo;
	float _amp;
	float fromAmp;

	float pwm;

	float currVal;

	float curr;
	float step;

	float noiseVal;

	uint32 sinCurrVal;
	uint32 sinStep;

	static bool sIsInit;
	static float sint[513];
	static float trit[513];
};

#endif
