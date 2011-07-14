/******************************************************************************/
/* InsPly Interface.                                                          */
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
#include "InsPly.h"
#include "LFO.h"
#include "Wave.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
InsPly::InsPly(Sawteeth *s)
{
	vib = new LFO();
	if (vib == NULL)
		throw PMemoryException();

	pwm = new LFO();
	if (pwm == NULL)
	{
		delete vib;
		throw PMemoryException();
	}

	w = new Wave();
	if (w == NULL)
	{
		delete pwm;
		delete vib;
		throw PMemoryException();
	}

	song         = s;
	ins          = &song->ins[0];
	currIns      = ins;
	steps        = (currIns->steps);

	insFreq      = 440.0f;
	currPartFreq = 440.0f;

	pwmOffs      = 0.0f;
	amp          = 0.0f;
	cutOff       = 0.0f;
	res          = 0.0f;

	currPartAmp  = 0.0f;
	currPartCO   = 0.0f;
	currAmp      = 0.0f;
	currF        = 0.0f;

	lo           = 0.0f;
	hi           = 0.0f;
	bp           = 0.0f;
	bs           = 0.0f;
	ampStep      = 0.0f;
	fStep        = 0.0f;

	TrigADSR(0);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
InsPly::~InsPly(void)
{
	delete w;
	delete pwm;
	delete vib;
}



/******************************************************************************/
/* TrigADSR()                                                                 */
/******************************************************************************/
void InsPly::TrigADSR(uint8 i)
{
	pwmOffs = 0.0f;
	trigged = true;
	currIns = ins + i;
	steps   = (currIns->steps);

	vib->SetFreq(ins[i].vibS * 50.0f);
	vamp = ins[i].vibD / 2000.0f;

	pwm->SetFreq(ins[i].pwmS * 5.0f);
	pamp = ins[i].pwmD / 255.1f;

	SetReso((float)ins[i].res / 255.0f);

	// Amp ADSR
	adsr      = 0;
	nextAdsr  = 0;

	// Filter ADSR
	fAdsr     = 0;
	nextFAdsr = 0;

	// Step
	stepC     = 0;
	nextS     = 0;
}



/******************************************************************************/
/* SetPWMOffs()                                                               */
/******************************************************************************/
void InsPly::SetPWMOffs(float a)
{
	pwmOffs = a;
}



/******************************************************************************/
/* SetReso()                                                                  */
/******************************************************************************/
void InsPly::SetReso(float a)
{
	res  = 1.0f - a;
	res *= res;
	res  = 1.0f - res;
}



/******************************************************************************/
/* SetAmp()                                                                   */
/******************************************************************************/
void InsPly::SetAmp(float a)
{
	currPartAmp = a;
}



/******************************************************************************/
/* SetFreq()                                                                  */
/******************************************************************************/
void InsPly::SetFreq(float f)
{
	currPartFreq = f;
}



/******************************************************************************/
/* SetCutOff()                                                                */
/******************************************************************************/
void InsPly::SetCutOff(float co)
{
	currPartCO = co;
}



/******************************************************************************/
/* Next()                                                                     */
/******************************************************************************/
bool InsPly::Next(float *buffer, uint32 count)
{
	// Check ins step
	if (nextS < 1)
	{
		currStep = steps + stepC;
		nextS    = currIns->sps;

		if (currStep->note != 0)
		{
			if (currStep->relative)
				insFreq = song->r2f[currStep->note];
			else
				insFreq = song->n2f[currStep->note];
		}

		if (currStep->wForm != 0)
			w->SetForm(currStep->wForm);

		stepC++;

		if (stepC >= currIns->len)
			stepC = currIns->loop;
	}

	// Check ADSR
	if (nextAdsr < 1)
	{
		if (adsr < currIns->ampPoints)
		{
			nextAdsr = 1 + currIns->amp[adsr].time;
			ampStep  = (float)((currIns->amp[adsr].lev / 257.0f) - currAmp) / nextAdsr;
			adsr++;
		}
		else
			ampStep = 0.0f;
	}

	// Check filter ADSR
	if (nextFAdsr < 1)
	{
		if (fAdsr < currIns->filterPoints)
		{
			nextFAdsr = 1 + currIns->filter[fAdsr].time;
			float target = currIns->filter[fAdsr].lev / 257.0f;
			target *= target * target;
			fStep = (target - currF) / nextFAdsr;
			fAdsr++;
		}
		else
			fStep = 0.0f;
	}

	// ADSR
	nextAdsr--;
	nextFAdsr--;

	// Step
	nextS--;

	// Freq
	if (currStep->relative)
		w->SetFreq(currPartFreq * insFreq * (1.0f + vamp * vib->Next()));
	else
		w->SetFreq(insFreq * (1.0f + vamp * vib->Next()));

	// PWM
	w->SetPWM(pwmOffs + pamp * pwm->Next());

	// Amp
	currAmp += ampStep;
	amp      = currAmp * currPartAmp;

	// Filter
	currF += fStep;
	cutOff = currF * currPartCO;

	// Filter part
	w->SetAmp(amp);
	if (trigged)
	{
		trigged = false;
		w->NoInterp();
	}

	if (!w->Next(buffer, count))
		return (false);

	switch (currIns->filterMode)
	{
		case 1:
		{
			SLP(buffer, count);
			break;
		}

		case 2:
		{
			OLP(buffer, count);
			break;
		}

		case 3:
		{
			LP(buffer, count);
			break;
		}

		case 4:
		{
			HP(buffer, count);
			break;
		}

		case 5:
		{
			BP(buffer, count);
			break;
		}

		case 6:
		{
			BS(buffer, count);
			break;
		}
	}

	switch (currIns->clipMode)
	{
		case 0x1:
		{
			VanillaClip(buffer, count, 2.0f * (float)(1 + currIns->boost));
			break;
		}

		case 0x2:
		{
			SinusClip(buffer, count, 0.7f * (float)(1.3f + currIns->boost));
			break;
		}
	}

	return (true);
}



/******************************************************************************/
/* SLP()                                                                      */
/******************************************************************************/
void InsPly::SLP(float *b, uint32 count)
{
	float *stop = b + count;
	float in;

	while (b < stop)
	{
		in = *b;
		lo = (cutOff * in) + (lo * (1.0f - cutOff));
		*b = lo;
		b++;
	}
}



/******************************************************************************/
/* OLP()                                                                      */
/******************************************************************************/
void InsPly::OLP(float *b, uint32 count)
{
	float *stop = b + count;

	while (b < stop)
	{
		lo  = cutOff * *b + (1.0f - cutOff) * hi;
		lo += (lo - bp) * res;

		bp  = hi;
		hi  = lo;

		*b  = lo;
		b++;
	}
}



/******************************************************************************/
/* LP()                                                                       */
/******************************************************************************/
void InsPly::LP(float *b, uint32 count)
{
	float *stop = b + count;
	float in;
	float t;

	while (b < stop)
	{
		in  = *b;
		t   = lo + cutOff * bp;
		hi  = in - lo - (1.8f - res * 1.8f) * bp;
		bp += cutOff * hi;

		if (t < -amp)
			lo = -amp;
		else
		{
			if (t > amp)
				lo = amp;
			else
				lo = t;
		}

		bs = lo + hi;
		*b = lo;
		b++;
	}
}



/******************************************************************************/
/* HP()                                                                       */
/******************************************************************************/
void InsPly::HP(float *b, uint32 count)
{
	float *stop = b + count;
	float in;
	float t;

	while (b < stop)
	{
		in  = *b;
		t   = lo + cutOff * bp;
		hi  = in - lo - (1.8f - res * 1.8f) * bp;
		bp += cutOff * hi;

		if (t < -amp)
			lo = -amp;
		else
		{
			if (t > amp)
				lo = amp;
			else
				lo = t;
		}

		bs = lo + hi;
		*b = hi;
		b++;
	}
}



/******************************************************************************/
/* BP()                                                                       */
/******************************************************************************/
void InsPly::BP(float *b, uint32 count)
{
	float *stop = b + count;
	float in;
	float t;

	while (b < stop)
	{
		in  = *b;
		t   = lo + cutOff * bp;
		hi  = in - lo - (1.8f - res * 1.8f) * bp;
		bp += cutOff * hi;

		if (t < -amp)
			lo = -amp;
		else
		{
			if (t > amp)
				lo = amp;
			else
				lo = t;
		}

		bs = lo + hi;
		*b = bp;
		b++;
	}
}



/******************************************************************************/
/* BS()                                                                       */
/******************************************************************************/
void InsPly::BS(float *b, uint32 count)
{
	float *stop = b + count;
	float in;
	float t;

	while (b < stop)
	{
		in  = *b;
		t   = lo + cutOff * bp;
		hi  = in - lo - (1.8f - res * 1.8f) * bp;
		bp += cutOff * hi;

		if (t < -amp)
			lo = -amp;
		else
		{
			if (t > amp)
				lo = amp;
			else
				lo = t;
		}

		bs = lo + hi;
		*b = bs;
		b++;
	}
}



/******************************************************************************/
/* VanillaClip()                                                              */
/******************************************************************************/
void InsPly::VanillaClip(float *b, uint32 count, float mul)
{
	float *stop = b + count;

	if (fabsf(mul - 1.0f) < 0.1f)
	{
		while (b < stop)
		{
			if (*b > 1.0f)
				*b = 1.0f;
			else
			{
				if (*b < -1.0f)
					*b = -1.0f;
			}

			b++;
		}
	}
	else
	{
		while (b < stop)
		{
			*b *= mul;

			if (*b > 1.0f)
				*b = 1.0f;
			else
			{
				if (*b < -1.0f)
					*b = -1.0f;
			}

			b++;
		}
	}
}



/******************************************************************************/
/* SinusClip()                                                                */
/******************************************************************************/
void InsPly::SinusClip(float *b, uint32 count, float mul)
{
	float *stop = b + count;

	if (fabsf(mul - 1.0f) < 0.1f)
	{
		while (b < stop)
		{
			*b = sinf(*b);
			b++;
		}
	}
	else
	{
		while (b < stop)
		{
			*b *= mul;
			*b  = sinf(*b * mul);
			b++;
		}
	}
}
