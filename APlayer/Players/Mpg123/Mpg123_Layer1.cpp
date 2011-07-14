/******************************************************************************/
/* MPG123Player Layer1 functions.                                             */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* I_StepOne() is the step one in the layer 1 decode process.                 */
/*                                                                            */
/* Input:  "bAlloc" ???                                                       */
/*         "scaleIndex" ???                                                   */
/*         "fr" is a pointer to the frame header to use.                      */
/******************************************************************************/
void MPG123Player::I_StepOne(uint32 bAlloc[], uint32 scaleIndex[2][SBLIMIT], Frame *fr)
{
	uint32 *ba  = bAlloc;
	uint32 *sca = (uint32 *)scaleIndex;

	if (fr->stereo == 2)
	{
		int32 i;
		int32 jsBound = fr->jsBound;

		for (i = 0; i < jsBound; i++)
		{
			*ba++ = GetBits(&bsi, 4);
			*ba++ = GetBits(&bsi, 4);
		}

		for (i = jsBound; i < SBLIMIT; i++)
			*ba++ = GetBits(&bsi, 4);

		ba = bAlloc;

		for (i = 0; i < jsBound; i++)
		{
			if ((*ba++))
				*sca++ = GetBits(&bsi, 6);

			if ((*ba++))
				*sca++ = GetBits(&bsi, 6);
		}

		for (i = jsBound; i < SBLIMIT; i++)
		{
			if ((*ba++))
			{
				*sca++ = GetBits(&bsi, 6);
				*sca++ = GetBits(&bsi, 6);
			}
		}
	}
	else
	{
		int32 i;

		for (i = 0; i < SBLIMIT; i++)
			*ba++ = GetBits(&bsi, 4);

		ba = bAlloc;

		for (i = 0; i < SBLIMIT; i++)
		{
			if ((*ba++))
				*sca++ = GetBits(&bsi, 6);
		}
	}
}



/******************************************************************************/
/* I_StepTwo() is the step two in the layer 1 decode process.                 */
/*                                                                            */
/* Input:  "fraction" ???                                                     */
/*         "bAlloc" ???                                                       */
/*         "scaleIndex" ???                                                   */
/*         "fr" is a pointer to the frame header to use.                      */
/******************************************************************************/
void MPG123Player::I_StepTwo(real fraction[2][SBLIMIT], uint32 bAlloc[2 * SBLIMIT], uint32 scaleIndex[2][SBLIMIT], Frame *fr)
{
	int32 i, n;
	int32 smpb[2 * SBLIMIT];		// Values: 0 - 65535
	int32 *sample;
	register uint32 *ba;
	register uint32 *sca = (uint32 *)scaleIndex;

	if (fr->stereo == 2)
	{
		int32 jsBound = fr->jsBound;
		register real *f0 = fraction[0];
		register real *f1 = fraction[1];
		ba = bAlloc;

		for (sample = smpb, i = 0; i < jsBound; i++)
		{
			if ((n = *ba++))
				*sample++ = GetBits(&bsi, n + 1);

			if ((n = *ba++))
				*sample++ = GetBits(&bsi, n + 1);
		}

		for (i = jsBound; i < SBLIMIT; i++)
		{
			if ((n = *ba++))
				*sample++ = GetBits(&bsi, n + 1);
		}

		ba = bAlloc;

		for (sample = smpb, i = 0; i < jsBound; i++)
		{
			if ((n = *ba++))
				*f0++ = (real)(((-1) << n) + (*sample++) + 1) * muls[n + 1][*sca++];
			else
				*f0++ = 0.0;

			if ((n = *ba++))
				*f1++ = (real)(((-1) << n) + (*sample++) + 1) * muls[n + 1][*sca++];
			else
				*f1++ = 0.0;
		}

		for (i = jsBound; i < SBLIMIT; i++)
		{
			if ((n = *ba++))
			{
				real samp = (((-1) << n) + (*sample++) + 1);
				*f0++ = samp * muls[n + 1][*sca++];
				*f1++ = samp * muls[n + 1][*sca++];
			}
			else
				*f0++ = *f1++ = 0.0;
		}

		for (i = fr->downSampleSbLimit; i < 32; i++)
			fraction[0][i] = fraction[1][i] = 0.0;
	}
	else
	{
		register real *f0 = fraction[0];
		ba = bAlloc;

		for (sample = smpb, i = 0; i < SBLIMIT; i++)
		{
			if ((n = *ba++))
				*sample++ = GetBits(&bsi, n + 1);
		}

		ba = bAlloc;

		for (sample = smpb, i = 0; i < SBLIMIT; i++)
		{
			if ((n = *ba++))
			{
				real samp = (((-1) << n) + (*sample++) + 1);
				*f0++ = samp * muls[n + 1][*sca++];
			}
			else
				*f0++ = 0.0;
		}

		for (i = fr->downSampleSbLimit; i < 32; i++)
			fraction[0][i] = 0.0;
	}
}



/******************************************************************************/
/* DoLayer1() decode a layer 1 frame.                                         */
/*                                                                            */
/* Input:  "mp" is a pointer the the mpeg structure to use.                   */
/*         "fr" is a pointer to the frame header to use.                      */
/*                                                                            */
/* Output: Number of samples clipped.                                         */
/******************************************************************************/
int32 MPG123Player::DoLayer1(MPStr *mp, Frame *fr)
{
	int32 clip = 0;
	int32 i, stereo = fr->stereo;
	uint32 bAlloc[2 * SBLIMIT];
	uint32 scaleIndex[2][SBLIMIT];
	real fraction[2][SBLIMIT];
	int32 single = fr->single;

	fr->jsBound = (fr->mode == MPG_MD_JOINT_STEREO) ? (fr->modeExt << 2) + 4 : 32;

	if ((stereo == 1) || (single == 3))
		single = 0;

	I_StepOne(bAlloc, scaleIndex, fr);

	for (i = 0; i < SCALE_BLOCK; i++)
	{
		I_StepTwo(fraction, bAlloc, scaleIndex, fr);

		if (single >= 0)
			clip += Synth_1to1((real *)fraction[single], 0, pcmSample1, &pcmPoint);
		else
		{
			int32 p1 = pcmPoint;
			clip += Synth_1to1((real *)fraction[0], 0, pcmSample1, &p1);
			clip += Synth_1to1((real *)fraction[1], 1, pcmSample2, &pcmPoint);
		}
	}

	return (clip);
}
