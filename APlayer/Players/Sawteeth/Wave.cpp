/******************************************************************************/
/* Wave Interface.                                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"

// Player headers
#include "Sawteeth.h"
#include "Wave.h"


/******************************************************************************/
/* Constants                                                                  */
/******************************************************************************/
const float pwmLim = 0.9f;



/******************************************************************************/
/* Static variables                                                           */
/******************************************************************************/
bool Wave::sIsInit = false;
float Wave::sint[513];
float Wave::trit[513];



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Wave::Wave(void)
{
	SInit();

	fromAmp  = 0.0f;
	noiseVal = 0.0f;
	curr     = 0.0f;
	pwm      = 0.0f;
	step     = 0.0f;
	currVal  = 0.0f;

	_pwmLo   = false;

	SetForm(SAW);
	SetFreq(440);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Wave::~Wave(void)
{
}



/******************************************************************************/
/* SetFreq()                                                                  */
/******************************************************************************/
void Wave::SetFreq(float freq)
{
	step = freq * 0.00002267573696145124;	// 1/44100
	if (step > 1.9f)
		step = 1.9f;

	sinStep = (uint32)((float)(1 << 22) * 512.0f * step);
}



/******************************************************************************/
/* SetPWM()                                                                   */
/******************************************************************************/
void Wave::SetPWM(float p)
{
	if (p > pwmLim)
		pwm = pwmLim;
	else
	{
		if (p < -pwmLim)
			pwm = -pwmLim;
		else
			pwm = p;
	}
}



/******************************************************************************/
/* SetForm()                                                                  */
/******************************************************************************/
void Wave::SetForm(uint8 waveForm)
{
	if (form != waveForm)
	{
		if (currVal > 1.0f)
			currVal = 0.0f;

		if (curr > 1.0f)
			curr = 0.0f;
	}

	form = waveForm;
}



/******************************************************************************/
/* SetAmp()                                                                   */
/******************************************************************************/
void Wave::SetAmp(float a)
{
	_amp = a;
}



/******************************************************************************/
/* NoInterp()                                                                 */
/******************************************************************************/
void Wave::NoInterp(void)
{
	fromAmp = _amp;
}



/******************************************************************************/
/* Next()                                                                     */
/******************************************************************************/
bool Wave::Next(float *buffer, uint32 count)
{
	if (_amp < 0.001f)
		return (false);

	switch (form)
	{
		case SAW:
		{
			FillSaw(buffer, count, _amp);
			break;
		}

		case SQR:
		{
			FillSquare(buffer, count, _amp);
			break;
		}

		case NOS:
		{
			FillNoise(buffer, count, _amp);
			break;
		}

		case TRI:
		{
			FillTri(buffer, count, _amp);
			break;
		}

		case SIN:
		{
			FillSin(buffer, count, _amp);
			break;
		}

		case TRIU:
		{
			FillTriU(buffer, count, _amp);
			break;
		}

		case SINU:
		{
			FillSinU(buffer, count, _amp);
			break;
		}

		default:
			return (false);
	}

	return (true);
}



/******************************************************************************/
/* SInit()                                                                    */
/******************************************************************************/
void Wave::SInit(void)
{
	int32 c;

	if (sIsInit)
		return;

	// Sine table
	for (c = 0; c < 513; c++)
		sint[c] = sinf((float)c * (2.0f * PI) / 512.0f);

	// Tri table
	float smp = -1.0f;
	float add = 4.0f / 512.0f;

	for (c = 0; c < 256; c++)
	{
		trit[c] = smp;
		smp    += add;
	}

	for (; c < 512; c++)
	{
		trit[c] = smp;
		smp    -= add;
	}

	trit[512] = trit[0];

	sIsInit = true;
}



/******************************************************************************/
/* FillSaw()                                                                  */
/******************************************************************************/
void Wave::FillSaw(float *out, uint32 count, float amp)
{
	float *stop = out + count;
	float _ampAdd = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (out < stop)
	{
		if (curr >= 1.0f)
		{
			float d = (curr - 1.0f) / step;
			curr -= 2.0f;

			*out = amp * (-2.0f * d + 1.0f);
			out++;
			amp  += _ampAdd;
			curr += step;
		}

		float walkDiff = 1.0f - curr;
		int32 walkSteps = (int32)((walkDiff / step) + 1);

		// Number of samples left in the buffer
		int32 steps = stop - out;

		if (steps > walkSteps)
			steps = walkSteps;

		while (steps > 8)
		{
			out[0] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[1] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[2] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[3] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[4] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[5] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[6] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out[7] = amp * curr;
			amp   += _ampAdd;
			curr  += step;

			out   += 8;
			steps -= 8;
		}

		while (steps--)
		{
			*out = amp * curr;
			out++;
			amp  += _ampAdd;
			curr += step;
		}
	}

	fromAmp = amp;
}



/******************************************************************************/
/* FillSquare()                                                               */
/******************************************************************************/
void Wave::FillSquare(float *out, uint32 count, float amp)
{
	float *stop = out + count;
	float _ampAdd = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (out < stop)
	{
		if (curr >= 1.0f)
		{
			float d = (curr - 1.0f) / step;

			curr -= 1.0f;
			if (_pwmLo)
			{
				*out  = amp * d + ((1.0f - d) * -amp);
				curr -= pwm;
			}
			else
			{
				*out  = -amp * d + ((1.0f - d) * amp);
				curr += pwm;
			}

			_pwmLo = !_pwmLo;
			out++;

			amp  += _ampAdd;
			curr += step;
		}

		float walkDiff = 1.0f - curr;
		int32 walkSteps = (int32)((walkDiff / step) + 1);

		// Number of samples left in the buffer
		int32 steps = stop - out;

		if (steps > walkSteps)
			steps = walkSteps;

		float tmpAmp;
		float tmpAmpAdd;

		if (_pwmLo)
		{
			tmpAmp    = -amp;
			tmpAmpAdd = -_ampAdd;
		}
		else
		{
			tmpAmp    = amp;
			tmpAmpAdd = _ampAdd;
		}

		int32 counter = steps;

		while (counter--)
		{
			*out    = tmpAmp;
			tmpAmp += tmpAmpAdd;
			out++;
		}

		amp  += steps * _ampAdd;
		curr += steps * step;
	}

	fromAmp = amp;
}



/******************************************************************************/
/* FillTri()                                                                  */
/******************************************************************************/
void Wave::FillTri(float *out, uint32 count, float amp)
{
	float *stop = out + count;
	float aStep = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (out < stop)
	{
		*out = amp * (2.0f * ((currVal > 0.0f) ? -currVal : currVal) + 1.0f);
		currVal += step;

		if (currVal > 1.0f)
			currVal -= 2.0f;

		out++;
		amp += aStep;
	}

	fromAmp = amp;
}



/******************************************************************************/
/* FillNoise()                                                                */
/******************************************************************************/
void Wave::FillNoise(float *out, uint32 count, float amp)
{
	float *stop = out + count;
	float _ampAdd = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (out < stop)
	{
		if (curr >= 1.0f)
		{
			curr -= 2.0f;
			noiseVal = amp * ((jngRand() / (64.0f * 256.0f * 256.0f * 256.0f)) - 1.0f);
		}

		float walkDiff = 1.0f - curr;
		int32 walkSteps = (int32)((walkDiff / step) + 1);

		// Number of samples left in the buffer
		int32 steps = stop - out;

		if (steps > walkSteps)
			steps = walkSteps;

		int32 counter = steps;

		while (counter--)
		{
			*out = noiseVal;
			out++;
		}

		curr += steps * step;
		amp  += steps * _ampAdd;
	}

	fromAmp = amp;

	if (curr >= 1.0f)
		curr -= 2.0f;
}



/******************************************************************************/
/* FillSin()                                                                  */
/******************************************************************************/
void Wave::FillSin(float *out, uint32 count, float amp)
{
	float aStep = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (count--)
	{
		sinCurrVal += sinStep;

		uint32 pos = sinCurrVal >> 23;
		float dec = (float)(sinCurrVal & ((1 << 23) - 1)) / (float)((1 << 23) - 1);

		float val0 = sint[pos];
		float val1 = sint[pos + 1];
		float val  = (val1 * dec) + (val0 * (1.0f - dec));

		*out = amp * val;
		out++;
		amp += aStep;
	}

	fromAmp = amp;
}



/******************************************************************************/
/* FillTriU()                                                                 */
/******************************************************************************/
void Wave::FillTriU(float *out, uint32 count, float amp)
{
	float aStep = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (count--)
	{
		sinCurrVal += sinStep;

		uint32 pos = sinCurrVal >> 23;

		*out = amp * trit[pos];
		out++;
		amp += aStep;
	}

	fromAmp = amp;
}



/******************************************************************************/
/* FillSinU()                                                                 */
/******************************************************************************/
void Wave::FillSinU(float *out, uint32 count, float amp)
{
	float aStep = (amp - fromAmp) / (float)count;

	amp = fromAmp;

	while (count--)
	{
		sinCurrVal += sinStep;

		uint32 pos = sinCurrVal >> 23;

		*out = amp * sint[pos];
		out++;
		amp += aStep;
	}

	fromAmp = amp;
}



/******************************************************************************/
/* jngRand()                                                                  */
/******************************************************************************/
#define BIGMOD			0x7fffffff
#define W				127773
#define C				2836

uint32 Wave::jngRand(void)
{
	static int32 jngSeed = 1;

	jngSeed = 16807 * (jngSeed % W) - (jngSeed / W) * C;
	if (jngSeed < 0)
		jngSeed += BIGMOD;

	return (jngSeed);
}
