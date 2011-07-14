/******************************************************************************/
/* APlayer normal mixer class.                                                */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// Server headers
#include "APMixerNormal.h"


/******************************************************************************/
/* Macros used in the mixer                                                   */
/*                                                                            */
/* Constant definitions                                                       */
/* ====================                                                       */
/*                                                                            */
/* BITSHIFT          Controls the maximum volume of the sound output. All     */
/*                   data is shifted right by BITSHIFT after being mixed.     */
/*                   Higher values result in quieter sound and less chance of */
/*                   distortion.                                              */
/******************************************************************************/
#define BITSHIFT				9			// Normally bitshift value
#define BITSHIFT_SAMP			8			// Sample boost bitshift value

#define FRACBITS				11
#define FRACMASK				((1L << FRACBITS) - 1L)

#define CLICK_SHIFT				6
#define CLICK_BUFFER			(1L << CLICK_SHIFT)



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APMixerNormal::APMixerNormal(void)
{
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APMixerNormal::~APMixerNormal(void)
{
}



/******************************************************************************/
/* InitMixer() initialize local mixer stuff.                                  */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool APMixerNormal::InitMixer(void)
{
	return (true);
}



/******************************************************************************/
/* EndMixer() clean up all the stuff the InitMixer() created.                 */
/******************************************************************************/
void APMixerNormal::EndMixer(void)
{
}



/******************************************************************************/
/* GetClickConstant() returns the click constant value.                       */
/*                                                                            */
/* Output: The constant.                                                      */
/******************************************************************************/
int32 APMixerNormal::GetClickConstant(void)
{
	return (CLICK_BUFFER);
}



/******************************************************************************/
/* DoMixing() is the main mixer function.                                     */
/*                                                                            */
/* Input:  "dest" is a pointer to write the mixed data into.                  */
/*         "todo" is the size of the buffer in sample pairs.                  */
/*         "mode" is the mixer mode.                                          */
/******************************************************************************/
void APMixerNormal::DoMixing(int32 *dest, int32 todo, uint32 mode)
{
	int32 t, pan, lVol, rVol;

	// Loop through all the channels and mix the samples into the buffer
	for (t = 0; t < channelNum; t++)
	{
		vnf = &vinf[t];

		if (vnf->kick)
		{
			vnf->current = ((int64)vnf->start) << FRACBITS;
			vnf->kick    = false;
			vnf->active  = true;
		}

		if (!vnf->frq)
			vnf->active = false;

		if (vnf->active)
		{
			vnf->increment = ((int64)(vnf->frq << FRACBITS)) / mixerFreq;

			if (vnf->flags & SF_REVERSE)
				vnf->increment = -vnf->increment;

			if (vnf->enabled)
			{
				lVol = vnf->leftVol * masterVol / 256;
				rVol = vnf->rightVol * masterVol / 256;
			}
			else
			{
				lVol = 0;
				rVol = 0;
			}

			vnf->oldLVol = vnf->lVolSel;
			vnf->oldRVol = vnf->rVolSel;

			if (mode & DMODE_STEREO)
			{
				if (vnf->flags & SF_SPEAKER)
				{
					vnf->lVolSel = lVol;
					vnf->rVolSel = rVol;
				}
				else
				{
					if (vnf->pan != PAN_SURROUND)
					{
						// Stereo, calculate the volume with panning
						pan = (((vnf->pan - 128) * stereoSep) / 128) + 128;

						vnf->lVolSel = (lVol * (PAN_RIGHT - pan)) >> 8;
						vnf->rVolSel = (lVol * pan) >> 8;
					}
					else
					{
						// Dolby Surround
						vnf->lVolSel = vnf->rVolSel = lVol / 2;
					}
				}
			}
			else
			{
				// Well, just mono
				vnf->lVolSel = lVol;
			}

			idxSize = (vnf->size)   ? ((int64)vnf->size << FRACBITS) - 1 : 0;
			idxLEnd = (vnf->repEnd) ? ((int64)vnf->repEnd << FRACBITS) - 1 : 0;
			idxLPos = (int64)vnf->repPos << FRACBITS;
			idxREnd = (vnf->releaseLen) ? ((int64)vnf->releaseLen << FRACBITS) - 1 : 0;
			AddChannel(dest, todo, mode);
		}
	}
}



/******************************************************************************/
/* AddChannel() mix a channel into the buffer.                                */
/*                                                                            */
/* Input:  "buf" is a pointer to the buffer to fill with the sampling.        */
/*         "todo" is the size of the buffer in sample pairs.                  */
/*         "mode" is the mixer mode.                                          */
/******************************************************************************/
void APMixerNormal::AddChannel(int32 *buf, int32 todo, uint32 mode)
{
	int64 end;
	int32 done;
	const void *s;

	if ((s = vnf->adr) == NULL)
	{
		vnf->current = 0;
		vnf->active  = false;
		return;
	}

	// Update the 'current' index so the sample loops, or
	// stops playing if it reached the end of the sample
	while (todo > 0)
	{
		int64 endPos;

		if (vnf->flags & SF_REVERSE)
		{
			// The sampling is playing in reverse
			if ((vnf->flags & SF_LOOP) && (vnf->current < idxLPos))
			{
				// The sample is looping, and has reached the loopstart index
				if (vnf->flags & SF_BIDI)
				{
					// Sample is doing bidirectional loops, so 'bounce'
					// the current index against the idxLPos
					vnf->current   = idxLPos + (idxLPos - vnf->current);
					vnf->increment = -vnf->increment;
					vnf->flags    &= ~SF_REVERSE;
				}
				else
				{
					// Normal backwards looping, so set the
					// current position to loopEnd index
					vnf->current = idxLEnd - (idxLPos - vnf->current);
				}
			}
			else
			{
				// The sample is not looping, so check if it reached index 0
				if (vnf->current < 0)
				{
					// Playing index reached 0, so stop playing this sample
					vnf->current = 0;
					vnf->active  = false;
					break;
				}
			}
		}
		else
		{
			// The sample is playing forward
			if (vnf->flags & SF_LOOP)
			{
				if (vnf->current >= idxLEnd)
				{
					// Do we have a loop address
					if (vnf->loopAdr == NULL)
					{
						vnf->current = 0;
						vnf->active  = false;
						break;
					}

					// Copy the loop address
					s = vnf->adr = vnf->loopAdr;

					// Should we release the sample?
					if (vnf->releaseLen != 0)
					{
						// Yes, so set the current position
						vnf->current = vnf->current - idxLEnd;
						vnf->flags  |= SF_RELEASE;
						vnf->flags  &= ~SF_LOOP;
					}
					else
					{
						// The sample is looping, so check if it reached the loopEnd index
						if (vnf->flags & SF_BIDI)
						{
							// Sample is doing bidirectional loops, so 'bounce'
							// the current index against the idxLEnd
							vnf->current   = idxLEnd - (vnf->current - idxLEnd);
							vnf->increment = -vnf->increment;
							vnf->flags    |= SF_REVERSE;
						}
						else
						{
							// Normal looping, so set the
							// current position to loopEnd index
							vnf->current = idxLPos + (vnf->current - idxLEnd);
						}
					}
				}
			}
			else
			{
				// Sample is not looping, so check if it reached the last position
				if (vnf->flags & SF_RELEASE)
				{
					// We play the release part
					if (vnf->current >= idxREnd)
					{
						// Stop playing this sample
						vnf->current = 0;
						vnf->active  = false;
						break;
					}
				}
				else
				{
					if (vnf->current >= idxSize)
					{
						// Stop playing this sample
						vnf->current = 0;
						vnf->active  = false;
						break;
					}
				}
			}
		}

		end = (vnf->flags & SF_REVERSE) ?
			  (vnf->flags & SF_LOOP) ? idxLPos : 0 :
			  (vnf->flags & SF_LOOP) ? idxLEnd :
			  (vnf->flags & SF_RELEASE) ? idxREnd : idxSize;

		// If the sample is not blocked
		if ((end == vnf->current) || (!vnf->increment))
			done = 0;
		else
		{
			done = min((end - vnf->current) / vnf->increment + 1, todo);
			if (done < 0)
				done = 0;
		}

		if (!done)
		{
			vnf->active = false;
			break;
		}

		endPos = vnf->current + done * vnf->increment;

		if (vnf->leftVol || vnf->rightVol)
		{
			// Use the 32 bit mixers as often as we can (they're much faster)
			if ((vnf->current < 0x7fffffff) && (endPos < 0x7fffffff))
			{
				//
				// Use 32 bit mixers
				//
				// Check to see if we need to make interpolation on the mixing
				if (mode & DMODE_INTERP)
				{
					if (vnf->flags & SF_16BITS)
					{
						// 16 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix16SurroundInterp((int16 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
							else
								vnf->current = Mix16StereoInterp((int16 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
						}
						else
							vnf->current = Mix16MonoInterp((int16 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
					}
					else
					{
						// 8 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix8SurroundInterp((int8 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
							else
								vnf->current = Mix8StereoInterp((int8 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
						}
						else
							vnf->current = Mix8MonoInterp((int8 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
					}
				}
				else
				{
					// No interpolation
					if (vnf->flags & SF_16BITS)
					{
						// 16 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix16SurroundNormal((int16 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
							else
								vnf->current = Mix16StereoNormal((int16 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
						}
						else
							vnf->current = Mix16MonoNormal((int16 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
					}
					else
					{
						// 8 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix8SurroundNormal((int8 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
							else
								vnf->current = Mix8StereoNormal((int8 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
						}
						else
							vnf->current = Mix8MonoNormal((int8 *)s, buf, (int32)vnf->current, (int32)vnf->increment, done);
					}
				}
			}
			else
			{
				//
				// Use 64 bit mixers
				//
				// Check to see if we need to make interpolation on the mixing
				if (mode & DMODE_INTERP)
				{
					if (vnf->flags & SF_16BITS)
					{
						// 16 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix16SurroundInterp64((int16 *)s, buf, vnf->current, vnf->increment, done);
							else
								vnf->current = Mix16StereoInterp64((int16 *)s, buf, vnf->current, vnf->increment, done);
						}
						else
							vnf->current = Mix16MonoInterp64((int16 *)s, buf, vnf->current, vnf->increment, done);
					}
					else
					{
						// 8 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix8SurroundInterp64((int8 *)s, buf, vnf->current, vnf->increment, done);
							else
								vnf->current = Mix8StereoInterp64((int8 *)s, buf, vnf->current, vnf->increment, done);
						}
						else
							vnf->current = Mix8MonoInterp64((int8 *)s, buf, vnf->current, vnf->increment, done);
					}
				}
				else
				{
					// No interpolation
					if (vnf->flags & SF_16BITS)
					{
						// 16 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix16SurroundNormal64((int16 *)s, buf, vnf->current, vnf->increment, done);
							else
								vnf->current = Mix16StereoNormal64((int16 *)s, buf, vnf->current, vnf->increment, done);
						}
						else
							vnf->current = Mix16MonoNormal64((int16 *)s, buf, vnf->current, vnf->increment, done);
					}
					else
					{
						// 8 bit input sample to be mixed
						if (mode & DMODE_STEREO)
						{
							if ((vnf->pan == PAN_SURROUND) && (mode & DMODE_SURROUND))
								vnf->current = Mix8SurroundNormal64((int8 *)s, buf, vnf->current, vnf->increment, done);
							else
								vnf->current = Mix8StereoNormal64((int8 *)s, buf, vnf->current, vnf->increment, done);
						}
						else
							vnf->current = Mix8MonoNormal64((int8 *)s, buf, vnf->current, vnf->increment, done);
					}
				}
			}
		}
		else
		{
			// Update the sample position
			vnf->current = endPos;
		}

		todo -= done;
		buf  += (mode & DMODE_STEREO) ? (done << 1) : done;
	}
}



/******************************************************************************/
/* Mix32To16() converts the mixed data to a 16 bit sample buffer.             */
/*                                                                            */
/* Input:  "dest" in a pointer to store the converted data.                   */
/*         "source" is a pointer to the mixed data.                           */
/*         "count" is the number of samples.                                  */
/*         "mode" is the mixer mode.                                          */
/******************************************************************************/
#define EXTRACT_SAMPLE(var, size)	var = *source++ >> (bitshift + 16 - size)
#define CHECK_SAMPLE(var, bound)	var = (var >= bound) ? bound - 1 : (var < -bound) ? -bound : var
#define PUT_SAMPLE(var)				*dest++ = (int16)var

void APMixerNormal::Mix32To16(int16 *dest, int32 *source, int32 count, uint32 mode)
{
	register int32 bitshift = (mode & DMODE_BOOST) ? BITSHIFT_SAMP : BITSHIFT;
	register int32 x1, x2, x3, x4;
	register int32 remain;

	remain = count & 3;

	for (count >>= 2; count; count--)
	{
		EXTRACT_SAMPLE(x1, 16);
		EXTRACT_SAMPLE(x2, 16);
		EXTRACT_SAMPLE(x3, 16);
		EXTRACT_SAMPLE(x4, 16);

		CHECK_SAMPLE(x1, 32767);
		CHECK_SAMPLE(x2, 32767);
		CHECK_SAMPLE(x3, 32767);
		CHECK_SAMPLE(x4, 32767);

		PUT_SAMPLE(x1);
		PUT_SAMPLE(x2);
		PUT_SAMPLE(x3);
		PUT_SAMPLE(x4);
	}

	while (remain--)
	{
		EXTRACT_SAMPLE(x1, 16);
		CHECK_SAMPLE(x1, 32767);
		PUT_SAMPLE(x1);
	}
}



/******************************************************************************/
/* Mix16MonoNormal() mixes a 16 bit sample into a mono output buffer.         */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix16MonoNormal(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;

	while (todo--)
	{
		sample = source[index >> FRACBITS];
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16StereoNormal() mixes a 16 bit sample into a stereo output buffer.     */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix16StereoNormal(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	while (todo--)
	{
		sample = source[index >> FRACBITS];
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16SurroundNormal() mixes a 16 bit surround sample into a stereo output  */
/*      buffer.                                                               */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix16SurroundNormal(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	if (lVolSel >= rVolSel)
	{
		while (todo--)
		{
			sample = source[index >> FRACBITS];
			index += increment;

			*dest++ += lVolSel * sample;
			*dest++ -= lVolSel * sample;
		}
	}
	else
	{
		while (todo--)
		{
			sample = source[index >> FRACBITS];
			index += increment;

			*dest++ -= rVolSel * sample;
			*dest++ += rVolSel * sample;
		}
	}

	return (index);
}



/******************************************************************************/
/* Mix8MonoNormal() mixes a 8 bit sample into a mono output buffer.           */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix8MonoNormal(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;

	while (todo--)
	{
		sample = ((int16)(source[index >> FRACBITS])) << 8;
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8StereoNormal() mixes a 8 bit sample into a stereo output buffer.       */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix8StereoNormal(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	while (todo--)
	{
		sample = ((int16)(source[index >> FRACBITS])) << 8;
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8SurroundNormal() mixes a 8 bit surround sample into a stereo output    */
/*      buffer.                                                               */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix8SurroundNormal(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	if (lVolSel >= rVolSel)
	{
		while (todo--)
		{
			sample = ((int16)(source[index >> FRACBITS])) << 8;
			index += increment;

			*dest++ += lVolSel * sample;
			*dest++ -= lVolSel * sample;
		}
	}
	else
	{
		while (todo--)
		{
			sample = ((int16)(source[index >> FRACBITS])) << 8;
			index += increment;

			*dest++ -= rVolSel * sample;
			*dest++ += rVolSel * sample;
		}
	}

	return (index);
}



/******************************************************************************/
/* Mix16MonoInterp() mixes a 16 bit sample into a mono output buffer with     */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix16MonoInterp(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;

		while (todo--)
		{
			sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (index & FRACMASK) >> FRACBITS);
			index += increment;

			*dest++ += (((int32)lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (index & FRACMASK) >> FRACBITS);
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16StereoInterp() mixes a 16 bit sample into a stereo output buffer with */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix16StereoInterp(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;
		int32 oldRVol = vnf->oldRVol - rVolSel;

		while (todo--)
		{
			sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (index & FRACMASK) >> FRACBITS);
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += ((rVolSel << CLICK_SHIFT) + oldRVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)source[index >> FRACBITS] +
				 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
				 (index & FRACMASK) >> FRACBITS);
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16SurroundInterp() mixes a 16 bit surround sample into a stereo output  */
/*      buffer with interpolation.                                            */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix16SurroundInterp(int16 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;
	int32 oldVol, vol;

	if (lVolSel >= rVolSel)
	{
		vol    = lVolSel;
		oldVol = vnf->oldLVol;
	}
	else
	{
		vol    = rVolSel;
		oldVol = vnf->oldRVol;
	}

	if (rampVol != 0)
	{
		oldVol -= vol;

		while (todo--)
		{
			sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (index & FRACMASK) >> FRACBITS);
			index += increment;

			sample = ((vol << CLICK_SHIFT) + oldVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += sample;
			*dest++ -= sample;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)source[index >> FRACBITS] +
				 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
				 (index & FRACMASK) >> FRACBITS);
		index += increment;

		*dest++ += vol * sample;
		*dest++ -= vol * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8MonoInterp() mixes a 8 bit sample into a mono output buffer with       */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix8MonoInterp(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;

		while (todo--)
		{
			sample = (int32)((int32)source[index >> FRACBITS] << 8) +
					 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
					 (index & FRACMASK) >> FRACBITS);
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)((int32)source[index >> FRACBITS] << 8) +
				 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
				 (index & FRACMASK) >> FRACBITS);
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8StereoInterp() mixes a 8 bit sample into a stereo output buffer with   */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix8StereoInterp(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;
		int32 oldRVol = vnf->oldRVol - rVolSel;

		while (todo--)
		{
			sample = (int32)((int32)source[index >> FRACBITS] << 8) +
					 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
					 (index & FRACMASK) >> FRACBITS);
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += ((rVolSel << CLICK_SHIFT) + oldRVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)((int32)source[index >> FRACBITS] << 8) +
				 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
				 (index & FRACMASK) >> FRACBITS);
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8SurroundInterp() mixes a 8 bit surround sample into a stereo output    */
/*      buffer with interpolation.                                            */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int32 APMixerNormal::Mix8SurroundInterp(int8 *source, int32 *dest, int32 index, int32 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;
	int32 oldVol, vol;

	if (lVolSel >= rVolSel)
	{
		vol    = lVolSel;
		oldVol = vnf->oldLVol;
	}
	else
	{
		vol    = rVolSel;
		oldVol = vnf->oldRVol;
	}

	if (rampVol != 0)
	{
		oldVol -= vol;

		while (todo--)
		{
			sample = (int32)((int32)source[index >> FRACBITS] << 8) +
					 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
					 (index & FRACMASK) >> FRACBITS);
			index += increment;

			sample = ((vol << CLICK_SHIFT) + oldVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += sample;
			*dest++ -= sample;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)((int32)source[index >> FRACBITS] << 8) +
				 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
				 (index & FRACMASK) >> FRACBITS);
		index += increment;

		*dest++ += vol * sample;
		*dest++ -= vol * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16MonoNormal64() mixes a 16 bit sample into a mono output buffer.       */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix16MonoNormal64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;

	while (todo--)
	{
		sample = source[index >> FRACBITS];
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16StereoNormal64() mixes a 16 bit sample into a stereo output buffer.   */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix16StereoNormal64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	while (todo--)
	{
		sample = source[index >> FRACBITS];
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16SurroundNormal64() mixes a 16 bit surround sample into a stereo       */
/*      output buffer.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix16SurroundNormal64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	if (lVolSel >= rVolSel)
	{
		while (todo--)
		{
			sample = source[index >> FRACBITS];
			index += increment;

			*dest++ += lVolSel * sample;
			*dest++ -= lVolSel * sample;
		}
	}
	else
	{
		while (todo--)
		{
			sample = source[index >> FRACBITS];
			index += increment;

			*dest++ -= rVolSel * sample;
			*dest++ += rVolSel * sample;
		}
	}

	return (index);
}



/******************************************************************************/
/* Mix8MonoNormal64() mixes a 8 bit sample into a mono output buffer.         */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix8MonoNormal64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;

	while (todo--)
	{
		sample = ((int16)(source[index >> FRACBITS])) << 8;
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8StereoNormal64() mixes a 8 bit sample into a stereo output buffer.     */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix8StereoNormal64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	while (todo--)
	{
		sample = ((int16)(source[index >> FRACBITS])) << 8;
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8SurroundNormal64() mixes a 8 bit surround sample into a stereo output  */
/*      buffer.                                                               */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix8SurroundNormal64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int16 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;

	if (lVolSel >= rVolSel)
	{
		while (todo--)
		{
			sample = ((int16)(source[index >> FRACBITS])) << 8;
			index += increment;

			*dest++ += lVolSel * sample;
			*dest++ -= lVolSel * sample;
		}
	}
	else
	{
		while (todo--)
		{
			sample = ((int16)(source[index >> FRACBITS])) << 8;
			index += increment;

			*dest++ -= rVolSel * sample;
			*dest++ += rVolSel * sample;
		}
	}

	return (index);
}



/******************************************************************************/
/* Mix16MonoInterp64() mixes a 16 bit sample into a mono output buffer with   */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix16MonoInterp64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;

		while (todo--)
		{
			sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (int32)((index & FRACMASK) >> FRACBITS));
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)source[index >> FRACBITS] +
				 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
				 (int32)((index & FRACMASK) >> FRACBITS));
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16StereoInterp64() mixes a 16 bit sample into a stereo output buffer    */
/*      with interpolation.                                                   */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix16StereoInterp64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;
		int32 oldRVol = vnf->oldRVol - rVolSel;

		while (todo--)
		{
			sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (int32)((index & FRACMASK) >> FRACBITS));
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += ((rVolSel << CLICK_SHIFT) + oldRVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)source[index >> FRACBITS] +
				 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
				 (int32)((index & FRACMASK) >> FRACBITS));
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix16SurroundInterp64() mixes a 16 bit surround sample into a stereo       */
/*      output buffer with interpolation.                                     */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix16SurroundInterp64(int16 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;
	int32 oldVol, vol;

	if (lVolSel >= rVolSel)
	{
		vol    = lVolSel;
		oldVol = vnf->oldLVol;
	}
	else
	{
		vol    = rVolSel;
		oldVol = vnf->oldRVol;
	}

	if (rampVol != 0)
	{
		oldVol -= vol;

		while (todo--)
		{
			sample = (int32)source[index >> FRACBITS] +
					 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
					 (int32)((index & FRACMASK) >> FRACBITS));
			index += increment;

			sample = ((vol << CLICK_SHIFT) + oldVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += sample;
			*dest++ -= sample;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)source[index >> FRACBITS] +
				 ((int32)(source[(index >> FRACBITS) + 1] - source[index >> FRACBITS]) *
				 (int32)((index & FRACMASK) >> FRACBITS));
		index += increment;

		*dest++ += vol * sample;
		*dest++ -= vol * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8MonoInterp64() mixes a 8 bit sample into a mono output buffer with     */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix8MonoInterp64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;

		while (todo--)
		{
			sample = (int32)((int32)source[index >> FRACBITS] << 8) +
					 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
					 (int32)((index & FRACMASK) >> FRACBITS));
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)((int32)source[index >> FRACBITS] << 8) +
				 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
				 (int32)((index & FRACMASK) >> FRACBITS));
		index += increment;

		*dest++ += lVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8StereoInterp64() mixes a 8 bit sample into a stereo output buffer with */
/*      interpolation.                                                        */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix8StereoInterp64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;

	if (rampVol != 0)
	{
		int32 oldLVol = vnf->oldLVol - lVolSel;
		int32 oldRVol = vnf->oldRVol - rVolSel;

		while (todo--)
		{
			sample = (int32)((int32)source[index >> FRACBITS] << 8) +
					 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
					 (int32)((index & FRACMASK) >> FRACBITS));
			index += increment;

			*dest++ += ((lVolSel << CLICK_SHIFT) + oldLVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += ((rVolSel << CLICK_SHIFT) + oldRVol * rampVol) * sample >> CLICK_SHIFT;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)((int32)source[index >> FRACBITS] << 8) +
				 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
				 (int32)((index & FRACMASK) >> FRACBITS));
		index += increment;

		*dest++ += lVolSel * sample;
		*dest++ += rVolSel * sample;
	}

	return (index);
}



/******************************************************************************/
/* Mix8SurroundInterp64() mixes a 8 bit surround sample into a stereo output  */
/*      buffer with interpolation.                                            */
/*                                                                            */
/* Input:  "source" in a pointer to the sample.                               */
/*         "dest" is a pointer to the store the mixed data.                   */
/*         "index" is the index into the sample.                              */
/*         "increment" is how many bytes to increment with.                   */
/*         "todo" is the number of sample pairs the destination buffer is.    */
/*                                                                            */
/* Output: The new position in the sample.                                    */
/******************************************************************************/
int64 APMixerNormal::Mix8SurroundInterp64(int8 *source, int32 *dest, int64 index, int64 increment, int32 todo)
{
	int32 sample;
	int32 lVolSel = vnf->lVolSel;
	int32 rVolSel = vnf->rVolSel;
	int32 rampVol = vnf->rampVol;
	int32 oldVol, vol;

	if (lVolSel >= rVolSel)
	{
		vol    = lVolSel;
		oldVol = vnf->oldLVol;
	}
	else
	{
		vol    = rVolSel;
		oldVol = vnf->oldRVol;
	}

	if (rampVol != 0)
	{
		oldVol -= vol;

		while (todo--)
		{
			sample = (int32)((int32)source[index >> FRACBITS] << 8) +
					 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
					 (int32)((index & FRACMASK) >> FRACBITS));
			index += increment;

			sample = ((vol << CLICK_SHIFT) + oldVol * rampVol) * sample >> CLICK_SHIFT;
			*dest++ += sample;
			*dest++ -= sample;

			if (--rampVol == 0)
				break;
		}

		vnf->rampVol = rampVol;
		if (todo < 0)
			return (index);
	}

	while (todo--)
	{
		sample = (int32)((int32)source[index >> FRACBITS] << 8) +
				 ((int32)(((int32)source[(index >> FRACBITS) + 1] << 8) - ((int32)source[index >> FRACBITS] << 8)) *
				 (int32)((index & FRACMASK) >> FRACBITS));
		index += increment;

		*dest++ += vol * sample;
		*dest++ -= vol * sample;
	}

	return (index);
}
