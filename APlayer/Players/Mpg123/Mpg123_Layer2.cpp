/******************************************************************************/
/* MPG123Player Layer2 functions.                                             */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* InitLayer2() initializes layer 2 tables.                                   */
/******************************************************************************/
void MPG123Player::InitLayer2(void)
{
	int32 i, j, k, l, len;
	real *table;
	int32 *iTable;

	for (i = 0; i < 3; i++)
	{
		iTable = tables[i];
		len    = tabLen[i];

		for (j = 0; j < len; j++)
		{
			for (k = 0; k < len; k++)
			{
				for (l = 0; l < len; l++)
				{
					*iTable++ = base[i][l];
					*iTable++ = base[i][k];
					*iTable++ = base[i][j];
				}
			}
		}
	}

	for (k = 0; k < 27; k++)
	{
		double m = mulMul[k];
		table    = muls[k];

		if (gotMMX)
		{
			for (j = 3, i = 0; i < 63; i++, j--)
				*table++ = 16384 * m * pow(2.0, (double)j / 3.0);
		}
		else
		{
			for (j = 3, i = 0; i < 63; i++, j--)
				*table++ = m * pow(2.0, (double)j / 3.0);
		}

		*table++ = 0.0;
	}
}



/******************************************************************************/
/* II_StepOne() is the step one in the layer 2 decode process.                */
/*                                                                            */
/* Input:  "bitAlloc" ???                                                     */
/*         "scale" ???                                                        */
/*         "fr" is a pointer to the frame header to use.                      */
/******************************************************************************/
void MPG123Player::II_StepOne(uint32 *bitAlloc, int32 *scale, Frame *fr)
{
	int32 stereo          = fr->stereo - 1;
	int32 sbLimit         = fr->ii_sbLimit;
	int32 jsBound         = fr->jsBound;
	int32 sbLimit2        = fr->ii_sbLimit << stereo;
	const AlTable *alloc1 = fr->alloc;
	int32 i;
	uint32 *scfsi, *bitA;
	int32 sc, step;

	bitA = bitAlloc;

	if (stereo)
	{
		for (i = jsBound; i; i--, alloc1 += (1 << step))
		{
			*bitA++ = (int8)GetBits(&bsi, step = alloc1->bits);
			*bitA++ = (int8)GetBits(&bsi, step);
		}

		for (i = sbLimit - jsBound; i; i--, alloc1 += (1 << step))
		{
			bitA[0] = (int8)GetBits(&bsi, step = alloc1->bits);
			bitA[1] = bitA[0];
			bitA   += 2;
		}

		bitA  = bitAlloc;
		scfsi = scfsiBuf;

		for (i = sbLimit2; i; i--)
		{
			if (*bitA++)
				*scfsi++ = (int8)GetBitsFast(&bsi, 2);
		}
	}
	else
	{
		// Mono
		for (i = sbLimit; i; i--, alloc1 += (1 << step))
			*bitA++ = (int8)GetBits(&bsi, step = alloc1->bits);

		bitA  = bitAlloc;
		scfsi = scfsiBuf;

		for (i = sbLimit; i; i--)
		{
			if (*bitA++)
				*scfsi++ = (int8)GetBitsFast(&bsi, 2);
		}
	}

	bitA  = bitAlloc;
	scfsi = scfsiBuf;

	for (i = sbLimit2; i; i--)
	{
		if (*bitA++)
		{
			switch (*scfsi++)
			{
				case 0:
					*scale++ = GetBitsFast(&bsi, 6);
					*scale++ = GetBitsFast(&bsi, 6);
					*scale++ = GetBitsFast(&bsi, 6);
					break;

				case 1:
					*scale++ = sc = GetBitsFast(&bsi, 6);
					*scale++ = sc;
					*scale++ = GetBitsFast(&bsi, 6);
					break;

				case 2:
					*scale++ = sc = GetBitsFast(&bsi, 6);
					*scale++ = sc;
					*scale++ = sc;
					break;

				default:		// Case 3
					*scale++ = GetBitsFast(&bsi, 6);
					*scale++ = sc = GetBitsFast(&bsi, 6);
					*scale++ = sc;
					break;
			}
		}
	}
}



/******************************************************************************/
/* II_StepTwo() is the step two in the layer 2 decode process.                */
/*                                                                            */
/* Input:  "bitAlloc" ???                                                     */
/*         "fraction" ???                                                     */
/*         "scale" ???                                                        */
/*         "fr" is a pointer to the frame header to use.                      */
/*         "x1" ???                                                           */
/******************************************************************************/
void MPG123Player::II_StepTwo(uint32 *bitAlloc, real fraction[2][4][SBLIMIT], int32 *scale, Frame *fr, int32 x1)
{
	int32 i, j, k, ba;
	int32 stereo  = fr->stereo;
	int32 sbLimit = fr->ii_sbLimit;
	int32 jsBound = fr->jsBound;
	const AlTable *alloc2, *alloc1 = fr->alloc;
	uint32 *bitA = bitAlloc;
	int32 d1, step;

	for (i = 0; i < jsBound; i++, alloc1 += (1 << step))
	{
		step = alloc1->bits;

		for (j = 0; j < stereo; j++)
		{
			if ((ba = *bitA++))
			{
				k = (alloc2 = alloc1 + ba)->bits;

				if ((d1 = alloc2->d) < 0)
				{
					real cm = muls[k][scale[x1]];
					fraction[j][0][i] = ((real)((int32)GetBits(&bsi, k) + d1)) * cm;
					fraction[j][1][i] = ((real)((int32)GetBits(&bsi, k) + d1)) * cm;
					fraction[j][2][i] = ((real)((int32)GetBits(&bsi, k) + d1)) * cm;
				}
				else
				{
					uint32 idx, *tab, m = scale[x1];

					idx = (uint32)GetBits(&bsi, k);
					tab = (uint32 *)(ii_table[d1] + idx + idx + idx);

					fraction[j][0][i] = muls[*tab++][m];
					fraction[j][1][i] = muls[*tab++][m];
					fraction[j][2][i] = muls[*tab][m];
				}

				scale += 3;
			}
			else
				fraction[j][0][i] = fraction[j][1][i] = fraction[j][2][i] = 0.0;
		}
	}

	for (i = jsBound; i < sbLimit; i++, alloc1 += (1 << step))
	{
		step = alloc1->bits;
		bitA++;			// Channel 1 and channel 2 bitAlloc are the same

		if ((ba = *bitA++))
		{
			k = (alloc2 = alloc1 + ba)->bits;

			if ((d1 = alloc2->d) < 0)
			{
				real cm;

				cm = muls[k][scale[x1 + 3]];
				fraction[1][0][i] = (fraction[0][0][i] = (real)((int32)GetBits(&bsi, k) + d1)) * cm;
				fraction[1][1][i] = (fraction[0][1][i] = (real)((int32)GetBits(&bsi, k) + d1)) * cm;
				fraction[1][2][i] = (fraction[0][2][i] = (real)((int32)GetBits(&bsi, k) + d1)) * cm;

				cm = muls[k][scale[x1]];
				fraction[0][0][i] *= cm;
				fraction[0][1][i] *= cm;
				fraction[0][2][i] *= cm;
			}
			else
			{
				uint32 idx, *tab, m1, m2;

				m1  = scale[x1];
				m2  = scale[x1 + 3];
				idx = (uint32)GetBits(&bsi, k);
				tab = (uint32 *)(ii_table[d1] + idx + idx + idx);

				fraction[0][0][i] = muls[*tab][m1];
				fraction[1][0][i] = muls[*tab++][m2];
				fraction[0][1][i] = muls[*tab][m1];
				fraction[1][1][i] = muls[*tab++][m2];
				fraction[0][2][i] = muls[*tab][m1];
				fraction[1][2][i] = muls[*tab][m2];
			}

			scale += 6;
		}
		else
		{
			fraction[0][0][i] = fraction[0][1][i] = fraction[0][2][i] =
			fraction[1][0][i] = fraction[1][1][i] = fraction[1][2][i] = 0.0;
		}

		// Should we use individual scaleFac for channel 2 or
		// is the current way the right one, where we just copy channel 1 to
		// channel 2?
		// The current 'strange' thing is, that we throw away the scaleFac
		// values for the second channel ...
		// -> changed .. now we use the scaleFac values of channel one!
	}

	if (sbLimit > fr->downSampleSbLimit)
		sbLimit = fr->downSampleSbLimit;

	for (i = sbLimit; i < SBLIMIT; i++)
	{
		for (j = 0; j < stereo; j++)
			fraction[j][0][i] = fraction[j][1][i] = fraction[j][2][i] = 0.0;
	}
}



/******************************************************************************/
/* II_SelectTable() get some layer 2 informations.                            */
/*                                                                            */
/* Input:  "fr" is a pointer to the frame header to use.                      */
/******************************************************************************/
void MPG123Player::II_SelectTable(Frame *fr)
{
	int32 table, sbLim;

	if (fr->lsf)
		table = 4;
	else
		table = translate[fr->samplingFrequency][2 - fr->stereo][fr->bitRateIndex];

	sbLim = sbLims[table];

	fr->alloc      = allocTables[table];
	fr->ii_sbLimit = sbLim;
}



/******************************************************************************/
/* DoLayer2() decode a layer 2 frame.                                         */
/*                                                                            */
/* Input:  "mp" is a pointer the the mpeg structure to use.                   */
/*         "fr" is a pointer to the frame header to use.                      */
/*                                                                            */
/* Output: Number of samples clipped.                                         */
/******************************************************************************/
int32 MPG123Player::DoLayer2(MPStr *mp, Frame *fr)
{
	int32 clip = 0;
	int32 i, j;
	int32 stereo = fr->stereo;
	real fraction[2][4][SBLIMIT];	// pickTable clears unused subbands
	uint32 bitAlloc[64];
	int32 scale[192];
	int32 single = fr->single;

	II_SelectTable(fr);
	fr->jsBound = (fr->mode == MPG_MD_JOINT_STEREO) ? (fr->modeExt << 2) + 4 : fr->ii_sbLimit;

	if ((stereo == 1) || (single == 3))
		single = 0;

	II_StepOne(bitAlloc, scale, fr);

	for (i = 0; i < SCALE_BLOCK; i++)
	{
		II_StepTwo(bitAlloc, fraction, scale, fr, i >> 2);

		for (j = 0; j < 3; j++)
		{
			if (single >= 0)
				clip += Synth_1to1((real *)fraction[single][j], 0, pcmSample1, &pcmPoint);
			else
			{
				int32 p1 = pcmPoint;
				clip += Synth_1to1((real *)fraction[0][j], 0, pcmSample1, &p1);
				clip += Synth_1to1((real *)fraction[1][j], 1, pcmSample2, &pcmPoint);
			}
		}
	}

	return (clip);
}
