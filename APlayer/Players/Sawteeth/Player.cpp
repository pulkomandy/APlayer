/******************************************************************************/
/* Player Interface.                                                          */
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
#include "Player.h"
#include "InsPly.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
Player::Player(Sawteeth *s, Channel *chn, uint8 chanNum)
{
	myChannel = chanNum;

	song      = s;
	step      = chn->steps;
	ch        = chn;

	ip = new InsPly(s);
	if (ip == NULL)
		throw PMemoryException();

	buffer = new float[s->spsPal];
	if (buffer == NULL)
	{
		delete ip;
		throw PMemoryException();
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
Player::~Player(void)
{
	delete[] buffer;
	delete ip;
}



/******************************************************************************/
/* Init() initialize the class to begin over again.                           */
/******************************************************************************/
void Player::Init(void)
{
	looped     = false;
	tmpLoop    = false;

	amp        = 1.0f;
	ampStep    = 0.0f;

	freq       = 1.0f;
	freqStep   = 1.0f;

	cutOff     = 1.0f;
	cutOffStep = 1.0f;

	JumpPos(0, 0, 0);
}



/******************************************************************************/
/* NextBuffer() generates the next buffer of sound.                           */
/*                                                                            */
/* Output: True if a buffer is generated, false if not.                       */
/******************************************************************************/
bool Player::NextBuffer(void)
{
	if (nexts < 1)
	{
		damp  = (255.0f - (float)step[seqCount].damp) / 255.0f;
		nexts = currPart->sps;

		currStep = &(currPart->steps[stepC]);
		if (currStep->ins != 0)
		{
			ip->TrigADSR(currStep->ins);
			amp        = 1.0f;
			ampStep    = 0.0f;
			cutOff     = 1.0f;
			cutOffStep = 1.0f;
		}

		if (currStep->note != 0)
		{
			targetFreq = song->n2f[currStep->note + ch->steps[seqCount].transp];
			freqStep   = 1.0f;

			if ((currStep->eff & 0xf0) != 0x30)
				freq = targetFreq;
		}

		//
		// Effects
		//
		const float divider = 50.0f;
		if (currStep->eff != 0)
		{
			switch (currStep->eff & 0xf0)
			{
				// Pitch
				case 0x10:
				{
					targetFreq = 44100.0f;
					freqStep   = 1 + (((currStep->eff & 0x0f) + 1) / (divider * 3));
					break;
				}

				case 0x20:
				{
					targetFreq = 1.0f;
					freqStep   = 1 - (((currStep->eff & 0x0f) + 1) / (divider * 3));
					break;
				}

				case 0x30:
				{
					if (targetFreq > freq)
						freqStep = 1 + (((currStep->eff & 0x0f) + 1) / divider);
					else
						freqStep = 1 - (((currStep->eff & 0x0f) + 1) / divider);

					break;
				}

				// PWM
				case 0x40:
				{
					ip->SetPWMOffs((currStep->eff & 0x0f) / 16.1f);
					break;
				}

				// Resonance
				case 0x50:
				{
					ip->SetReso((currStep->eff & 0x0f) / 15.0f);
					break;
				}

				// Filter
				case 0x70:
				{
					cutOffStep = 1.0f - ((currStep->eff & 0x0f) + 1) / 256.0f;
					break;
				}

				case 0x80:
				{
					cutOffStep = 1.0f + ((currStep->eff & 0x0f) + 1) / 256.0f;
					break;
				}

				case 0x90:
				{
					cutOffStep = 1.0f;
					cutOff     = ((currStep->eff & 0x0f) + 1) / 16.0f;
					cutOff    *= cutOff;
					break;
				}

				// Amp
				case 0xa0:
				{
					ampStep = -((currStep->eff & 0x0f) + 1) / 256.0f;
					break;
				}

				case 0xb0:
				{
					ampStep = ((currStep->eff & 0x0f) + 1) / 256.0f;
					break;
				}

				case 0xc0:
				{
					ampStep = 0.0f;
					amp     = (currStep->eff & 0x0f) / 15.0f;
					break;
				}
			}
		}

		// Increase counters
		if (tmpLoop)
		{
			looped  = true;
			tmpLoop = false;

			// Tell APlayer that the song has ended
			if (song->posChannel == myChannel)
				song->endReached = true;
		}
		else
			looped = false;

		stepC++;
		if (stepC >= currPart->len)
		{
			stepC = 0;
			seqCount++;

			// Do only tell APlayer about a position change,
			// if we are the position teller channel
			if (song->posChannel == myChannel)
				song->ChangePosition();

			if (seqCount > ch->rLoop)
			{
				seqCount = ch->lLoop;	// Limit
				tmpLoop  = true;
			}

			currPart = &(song->parts[step[seqCount].part]);
		}
	}

	nexts--;

	cutOff *= cutOffStep;
	if (cutOff < 0.0f)
	{
		cutOff     = 0.0f;
		cutOffStep = 1.0f;
	}
	else
	{
		if (cutOff > 1.0f)
		{
			cutOff     = 1.0f;
			cutOffStep = 1.0f;
		}
	}

	freq *= freqStep;
	if (freqStep > 1.0001f)
	{
		if (freq > targetFreq)
		{
			freq     = targetFreq;
			freqStep = 1.0f;
		}
	}
	else
	{
		if (freqStep < 0.9999f)
		{
			if (freq < targetFreq)
			{
				freq     = targetFreq;
				freqStep = 1.0f;
			}
		}
	}

	amp += ampStep;
	if (amp < 0.0f)
	{
		amp     = 0.0f;
		ampStep = 0.0f;
	}
	else
	{
		if (amp > 1.0f)
		{
			amp     = 1.0f;
			ampStep = 0.0f;
		}
	}

	ip->SetAmp(amp * damp);
	ip->SetFreq(freq);
	ip->SetCutOff(cutOff);

	return (ip->Next(buffer, song->spsPal));
}



/******************************************************************************/
/* Buffer() returns the pointer to the buffer holding the sound.              */
/*                                                                            */
/* Output: Pointer to the buffer.                                             */
/******************************************************************************/
float *Player::Buffer(void)
{
	return (buffer);
}



/******************************************************************************/
/* Looped() returns whether the sound has looped or not.                      */
/*                                                                            */
/* Output: True if the sound has looped, false if not.                        */
/******************************************************************************/
bool Player::Looped(void)
{
	if (looped)
	{
		looped = false;
		return (true);
	}

	return (false);
}



/******************************************************************************/
/* GetSeqPos() returns the current sequencer position.                        */
/*                                                                            */
/* Output: The sequencer position.                                            */
/******************************************************************************/
uint32 Player::GetSeqPos(void)
{
	return (seqCount);
}



/******************************************************************************/
/* JumpPos() jumps to the specific position given.                            */
/*                                                                            */
/* Input:  "seqPos" is the sequece position.                                  */
/*         "stepPos" is the step position.                                    */
/*         "pal" is the pal count.                                            */
/******************************************************************************/
void Player::JumpPos(uint8 seqPos, uint8 stepPos, uint8 pal)
{
	stepC    = stepPos;
	seqCount = seqPos;
	nexts    = 0;

	currPart = &(song->parts[step[seqCount].part]);
	currStep = &(currPart->steps[stepC]);

	damp     = (255.0f - (float)step[seqCount].damp) / 255.0f;

	if (pal > 0)
	{
		// Increase counters
		stepC++;

		if (stepC >= currPart->len)
		{
			stepC = 0;
			seqCount++;

			if (seqCount >= ch->len)
				seqCount = 0;

			currPart = &(song->parts[step[seqCount].part]);
		}

		nexts = currPart->sps - pal;
		currStep = &(currPart->steps[stepC]);
	}
}
