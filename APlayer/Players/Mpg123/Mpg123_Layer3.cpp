/******************************************************************************/
/* MPG123Player Layer3 functions.                                             */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* InitLayer3() initializes layer 3 tables.                                   */
/*                                                                            */
/* Input:  "downSampleSbLimit" is the down sample value.                      */
/******************************************************************************/
void MPG123Player::InitLayer3(int32 downSampleSbLimit)
{
	int32 i, j, k, l;

	if (gotMMX)
	{
		for (i = -256; i < (118 + 4); i++)
			gainPow2[i + 256] = 16384.0 * pow((double)2.0, -0.25 * (double)(i + 210));
	}
	else
	{
		for (i = -256; i < (118 + 4); i++)
			gainPow2[i + 256] = DOUBLE_TO_REAL(pow((double)2.0, -0.25 * (double)(i + 210)));
	}

	for (i = 0; i < 8207; i++)
		isPow[i] = DOUBLE_TO_REAL(pow((double)i, (double)4.0 / 3.0));

	for (i = 0; i < 8; i++)
	{
		static const double ci[8] = { -0.6, -0.535, -0.33, -0.185, -0.095, -0.041, -0.0142, -0.0037 };
		double sq = sqrt(1.0 + ci[i] * ci[i]);

		aa_cs[i] = DOUBLE_TO_REAL(1.0 / sq);
		aa_ca[i] = DOUBLE_TO_REAL(ci[i] / sq);
	}

	for (i = 0; i < 18; i++)
	{
		win[0][i]      = win[1][i]      = DOUBLE_TO_REAL(0.5 * sin(M_PI / 72.0 * (double)(2 * (i +  0) + 1)) / cos(M_PI * (double)(2 * (i +  0) + 19) / 72.0));
		win[0][i + 18] = win[3][i + 18] = DOUBLE_TO_REAL(0.5 * sin(M_PI / 72.0 * (double)(2 * (i + 18) + 1)) / cos(M_PI * (double)(2 * (i + 18) + 19) / 72.0));
	}

	for (i = 0; i < 6; i++)
	{
		win[1][i + 18] = DOUBLE_TO_REAL(0.5 / cos(M_PI * (double)(2 * (i + 18) + 19) / 72.0));
		win[3][i + 12] = DOUBLE_TO_REAL(0.5 / cos(M_PI * (double)(2 * (i + 12) + 19) / 72.0));
		win[1][i + 24] = DOUBLE_TO_REAL(0.5 * sin(M_PI / 24.0 * (double)(2 * i + 13)) / cos(M_PI * (double)(2 * (i + 24) + 19) / 72.0));
		win[1][i + 30] = win[3][i] = DOUBLE_TO_REAL(0.0);
		win[3][i +  6] = DOUBLE_TO_REAL(0.5 * sin(M_PI / 24.0 * (double)(2 * i +  1)) / cos(M_PI * (double)(2 * (i +  6) + 19) / 72.0));
	}

	for (i = 0; i < 9; i++)
		cos9[i] = DOUBLE_TO_REAL(cos(M_PI / 18.0 * (double)i));

	for (i = 0; i < 9; i++)
		tfcos36[i] = DOUBLE_TO_REAL(0.5 / cos(M_PI * (double)(i * 2 + 1) / 36.0));

	for (i = 0; i < 3; i++)
		tfcos12[i] = DOUBLE_TO_REAL(0.5 / cos(M_PI * (double)(i * 2 + 1) / 12.0));

	cos6_1 = DOUBLE_TO_REAL(cos(M_PI / 6.0 * (double)1));
	cos6_2 = DOUBLE_TO_REAL(cos(M_PI / 6.0 * (double)2));

#ifdef NEW_DCT9

	new_cos9[0]  = DOUBLE_TO_REAL(cos(1.0 * M_PI / 9.0));
	new_cos9[1]  = DOUBLE_TO_REAL(cos(5.0 * M_PI / 9.0));
	new_cos9[2]  = DOUBLE_TO_REAL(cos(7.0 * M_PI / 9.0));
	new_cos18[0] = DOUBLE_TO_REAL(cos(1.0 * M_PI / 18.0));
	new_cos18[1] = DOUBLE_TO_REAL(cos(11.0 * M_PI / 18.0));
	new_cos18[2] = DOUBLE_TO_REAL(cos(13.0 * M_PI / 18.0));

#endif

	for (i = 0; i < 12; i++)
	{
		win[2][i] = DOUBLE_TO_REAL(0.5 * sin(M_PI / 24.0 * (double)(2 * i + 1)) / cos(M_PI * (double)(2 * i + 7) / 24.0));

		for (j = 0; j < 6; j++)
			cos1[i][j] = DOUBLE_TO_REAL(cos(M_PI / 24.0 * (double)((2 * i + 7) * (2 * j + 1))));
	}

	for (j = 0; j < 4; j++)
	{
		static const int32 len[4] = { 36, 36, 12, 36 };

		for (i = 0; i < len[j]; i += 2)
			win1[j][i] = +win[j][i];

		for (i = 1; i < len[j]; i += 2)
			win1[j][i] = -win[j][i];
	}

	for (i = 0; i < 16; i++)
	{
		double t = tan((double)i * M_PI / 12.0);
		tan1_1[i] = DOUBLE_TO_REAL(t / (1.0 + t));
		tan2_1[i] = DOUBLE_TO_REAL(1.0 / (1.0 + t));
		tan1_2[i] = DOUBLE_TO_REAL(M_SQRT2 * t / (1.0 + t));
		tan2_2[i] = DOUBLE_TO_REAL(M_SQRT2 / (1.0 + t));

		for (j = 0; j < 2; j++)
		{
			double base = pow(2.0, -0.25 * (j + 1.0));
			double p1 = 1.0, p2 = 1.0;

			if (i > 0)
			{
				if (i & 1)
					p1 = pow(base, (i + 1.0) * 0.5);
				else
					p2 = pow(base, i * 0.5);
			}

			pow1_1[j][i] = DOUBLE_TO_REAL(p1);
			pow2_1[j][i] = DOUBLE_TO_REAL(p2);
			pow1_2[j][i] = DOUBLE_TO_REAL(M_SQRT2 * p1);
			pow2_2[j][i] = DOUBLE_TO_REAL(M_SQRT2 * p2);
		}
	}

	for (j = 0; j < 9; j++)
	{
		const BandInfoStruct *bi = &bandInfo[j];
		int32 *mp;
		int32 cb, lWin;
		const int32 *bdf;

		mp  = map[j][0] = mapBuf0[j];
		bdf = bi->longDiff;

		for (i = 0, cb = 0; cb < 8; cb++, i += *bdf++)
		{
			*mp++ = (*bdf) >> 1;
			*mp++ = i;
			*mp++ = 3;
			*mp++ = cb;
		}

		bdf = bi->shortDiff + 3;

		for (cb = 3; cb < 13; cb++)
		{
			int32 l = (*bdf++) >> 1;

			for (lWin = 0; lWin < 3; lWin++)
			{
				*mp++ = l;
				*mp++ = i + lWin;
				*mp++ = lWin;
				*mp++ = cb;
			}

			i += 6 * l;
		}

		mapEnd[j][0] = mp;

		mp  = map[j][1] = mapBuf1[j];
		bdf = bi->shortDiff + 0;

		for (i = 0, cb = 0; cb < 13; cb++)
		{
			int32 l = (*bdf++) >> 1;

			for (lWin = 0; lWin < 3; lWin++)
			{
				*mp++ = l;
				*mp++ = i + lWin;
				*mp++ = lWin;
				*mp++ = cb;
			}

			i += 6 * l;
		}

		mapEnd[j][1] = mp;

		mp  = map[j][2] = mapBuf2[j];
		bdf = bi->longDiff;

		for (cb = 0; cb < 22; cb++)
		{
			*mp++ = (*bdf++) >> 1;
			*mp++ = cb;
		}

		mapEnd[j][2] = mp;
	}

	for (j = 0; j < 9; j++)
	{
		for (i = 0; i < 23; i++)
		{
			longLimit[j][i] = (bandInfo[j].longIdx[i] - 1 + 8) / 18 + 1;
			if (longLimit[j][i] > downSampleSbLimit)
				longLimit[j][i] = downSampleSbLimit;
		}

		for (i = 0; i < 14; i++)
		{
			shortLimit[j][i] = (bandInfo[j].shortIdx[i] - 1) / 18 + 1;
			if (shortLimit[j][i] > downSampleSbLimit)
				shortLimit[j][i] = downSampleSbLimit;
		}
	}

	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < 6; j++)
		{
			for (k = 0; k < 6; k++)
			{
				int32 n    = k + j * 6 + i * 36;
				i_slen2[n] = i | (j << 3) | (k << 6) | (3 << 12);
			}
		}
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			for (k = 0; k < 4; k++)
			{
				int32 n          = k + j * 4 + i * 16;
				i_slen2[n + 180] = i | (j << 3) | (k << 6) | (4 << 12);
			}
		}
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 3; j++)
		{
			int32 n          = j + i * 3;
			i_slen2[n + 244] = i | (j << 3) | (5 << 12);
			n_slen2[n + 500] = i | (j << 3) | (2 << 12) | (1 << 15);
		}
	}

	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < 5; j++)
		{
			for (k = 0; k < 4; k++)
			{
				for (l = 0; l < 4; l++)
				{
					int32 n    = l + k * 4 + j * 16 + i * 80;
					n_slen2[n] = i | (j << 3) | (k << 6) | (l << 9) | (0 << 12);
				}
			}
		}
	}

	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < 5; j++)
		{
			for (k = 0; k < 4; k++)
			{
				int32 n          = k + j * 4 + i * 20;
				n_slen2[n + 400] = i | (j << 3) | (k << 6) | (1 << 12);
			}
		}
	}
}



/******************************************************************************/
/* III_GetSideInfo() gets the side information on MPEG-1 and MPEG-2 files.    */
/******************************************************************************/
bool MPG123Player::III_GetSideInfo(III_SideInfo *si, int32 stereo, int32 msStereo, int32 sFreq, int32 single, int32 lsf)
{
	static const int32 tabs[2][5] = { { 2, 9, 5, 3, 4 } , { 1, 8, 1, 2, 9 } };
	const int32 *tab = tabs[lsf];
	int32 ch, gr;
	int32 powDiff = (single == 3) ? 4 : 0;

	si->mainDataBegin = GetBits(&bsi, tab[1]);

	if (stereo == 1)
		si->privateBits = GetBitsFast(&bsi, tab[2]);
	else
		si->privateBits = GetBitsFast(&bsi, tab[3]);

	if (!lsf)
	{
		for (ch = 0; ch < stereo; ch++)
		{
			si->ch[ch].gr[0].scfsi = -1;
			si->ch[ch].gr[1].scfsi = GetBitsFast(&bsi, 4);
		}
	}

	for (gr = 0; gr < tab[0]; gr++)
	{
		for (ch = 0; ch < stereo; ch++)
		{
			register GrInfoS *grInfo = &(si->ch[ch].gr[gr]);

			grInfo->part2_3length = GetBits(&bsi, 12);
			grInfo->bigValues     = GetBits(&bsi, 9);

			if (grInfo->bigValues > 288)
				grInfo->bigValues = 288;

			grInfo->pow2Gain = gainPow2 + 256 - GetBitsFast(&bsi, 8) + powDiff;

			if (msStereo)
				grInfo->pow2Gain += 2;

			grInfo->scaleFacCompress = GetBits(&bsi, tab[4]);

			// Window-switch flag
			if (Get1Bit(&bsi))
			{
				int32 i;

				grInfo->blockType      = GetBitsFast(&bsi, 2);
				grInfo->mixedBlockFlag = Get1Bit(&bsi);
				grInfo->tableSelect[0] = GetBitsFast(&bsi, 5);
				grInfo->tableSelect[1] = GetBitsFast(&bsi, 5);

				// TableSelect[2] not needed, because there is no region2
				// but to satisfy some verifications tools we set it either.
				grInfo->tableSelect[2] = 0;

				for (i = 0; i < 3; i++)
					grInfo->fullGain[i] = grInfo->pow2Gain + (GetBitsFast(&bsi, 3) << 3);

				if (grInfo->blockType == 0)
					return (false);

				// RegionCount / start parameters are implicit in this case
				if ((!lsf) || (grInfo->blockType == 2))
					grInfo->region1Start = 36 >> 1;
				else
				{
					// Check this again for 2.5 and sFreq = 8
					if (sFreq == 8)
						grInfo->region1Start = 108 >> 1;
					else
						grInfo->region1Start = 54 >> 1;
				}

				grInfo->region2Start = 576 >> 1;
			}
			else
			{
				int32 i, r0c, r1c;

				for (i = 0; i < 3; i++)
					grInfo->tableSelect[i] = GetBitsFast(&bsi, 5);

				r0c = GetBitsFast(&bsi, 4);
				r1c = GetBitsFast(&bsi, 3);

				grInfo->region1Start = bandInfo[sFreq].longIdx[r0c + 1] >> 1;

				if ((r0c + r1c + 2) > 22)
					grInfo->region2Start = 576 >> 1;
				else
					grInfo->region2Start = bandInfo[sFreq].longIdx[r0c + 1 + r1c + 1] >> 1;

				grInfo->blockType      = 0;
				grInfo->mixedBlockFlag = 0;
			}

			if (!lsf)
				grInfo->preFlag = Get1Bit(&bsi);

			grInfo->scaleFacScale     = Get1Bit(&bsi);
			grInfo->count1TableSelect = Get1Bit(&bsi);
		}
	}

	return (true);
}



/******************************************************************************/
/* III_GetScaleFactors1() get the scale factors on MPEG-1 files.              */
/*                                                                            */
/* Output: Number of bits read from the stream.                               */
/******************************************************************************/
int32 MPG123Player::III_GetScaleFactors1(int32 *scf, GrInfoS *grInfo)
{
	static const uint8 sLen[2][16] =
	{
		{ 0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4 },
		{ 0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3 }
	};

	int32 numBits;
	int32 num0 = sLen[0][grInfo->scaleFacCompress];
	int32 num1 = sLen[1][grInfo->scaleFacCompress];

	if (grInfo->blockType == 2)
	{
		int32 i = 18;
		numBits = (num0 + num1) * 18;

		if (grInfo->mixedBlockFlag)
		{
			for (i = 8; i; i--)
				*scf++ = GetBitsFast(&bsi, num0);

			i        = 9;
			numBits -= num0;
		}

		for (; i; i--)
			*scf++ = GetBitsFast(&bsi, num0);

		for (i = 18; i; i--)
			*scf++ = GetBitsFast(&bsi, num1);

		*scf++ = 0;
		*scf++ = 0;
		*scf++ = 0;
	}
	else
	{
		int32 i;
		int32 scfsi = grInfo->scfsi;

		if (scfsi < 0)		// scfi < 0 => granule == 0
		{
			for (i = 11; i; i--)
				*scf++ = GetBitsFast(&bsi, num0);

			for (i = 10; i; i--)
				*scf++ = GetBitsFast(&bsi, num1);

			numBits = (num0 + num1) * 10 + num0;

			*scf++ = 0;
		}
		else
		{
			numBits = 0;

			if (!(scfsi & 0x8))
			{
				for (i = 0; i < 6; i++)
					*scf++ = GetBitsFast(&bsi, num0);

				numBits += num0 * 6;
			}
			else
				scf += 6;

			if (!(scfsi & 0x4))
			{
				for (i = 0; i < 5; i++)
					*scf++ = GetBitsFast(&bsi, num0);

				numBits += num0 * 5;
			}
			else
				scf += 5;

			if (!(scfsi & 0x2))
			{
				for (i = 0; i < 5; i++)
					*scf++ = GetBitsFast(&bsi, num1);

				numBits += num1 * 5;
			}
			else
				scf += 5;

			if (!(scfsi & 0x1))
			{
				for (i = 0; i < 5; i++)
					*scf++ = GetBitsFast(&bsi, num1);

				numBits += num1 * 5;
			}
			else
				scf += 5;

			*scf++ = 0;		// No l[21] in original sources
		}
	}

	return (numBits);
}



/******************************************************************************/
/* III_GetScaleFactors2() get the scale factors on MPEG-2 files.              */
/*                                                                            */
/* Output: Number of bits read from the stream.                               */
/******************************************************************************/
int32 MPG123Player::III_GetScaleFactors2(int32 *scf, GrInfoS *grInfo, int32 iStereo)
{
	static const uint8 sTab[3][6][4] =
	{
		{
			{  6,  5,  5,  5 }, {  6,  5,  7,  3 }, { 11, 10,  0,  0 },
			{  7,  7,  7,  0 }, {  6,  6,  6,  3 }, {  8,  8,  5,  0 }
		},
		{
			{  9,  9,  9,  9 }, {  9,  9, 12,  6 }, { 18, 18,  0,  0 },
			{ 12, 12, 12,  0 }, { 12,  9,  9,  6 }, { 15, 12,  9,  0 }
		},
		{
			{  6,  9,  9,  9 }, {  6,  9, 12,  6 }, { 15, 18,  0,  0 },
			{  6, 15, 12,  0 }, {  6, 12,  9,  6 }, {  6, 18,  9,  0 }
		}
	};

	const uint8 *pnt;
	int32 i, j, n = 0, numBits = 0;
	uint32 sLen;

	if (iStereo)	// iStereo AND second channel -> DoLayer3() check this
		sLen = i_slen2[grInfo->scaleFacCompress >> 1];
	else
		sLen = n_slen2[grInfo->scaleFacCompress];

	grInfo->preFlag = (sLen >> 15) & 0x1;

	n = 0;
	if (grInfo->blockType == 2)
	{
		n++;
		if (grInfo->mixedBlockFlag)
			n++;
	}

	pnt = sTab[n][(sLen >> 12) & 0x7];

	for (i = 0; i < 4; i++)
	{
		int32 num = sLen & 0x7;
		sLen >>= 3;

		if (num)
		{
			for (j = 0; j < (int32)(pnt[i]); j++)
				*scf++ = GetBitsFast(&bsi, num);

			numBits += pnt[i] * num;
		}
		else
		{
			for (j = 0; j < (int32)(pnt[i]); j++)
				*scf++ = 0;
		}
	}

	n = (n << 1) + 1;

	for (i = 0; i < n; i++)
		*scf++ = 0;

	return (numBits);
}



/******************************************************************************/
/* III_DequantizeSample() dequantize the sample data. (Includes huffman       */
/*      decoding).                                                            */
/*                                                                            */
/* Output: 0 for success, 1 for failure.                                      */
/******************************************************************************/
// 24 is enough because tab13 has max. a 19 bit huffvector
#define BITSHIFT ((int32)((sizeof(int32) - 1) * 8))

#define REFRESH_MASK \
	while (num < BITSHIFT) \
	{ \
		mask        |= ((uint32)GetByte(&bsi)) << (BITSHIFT - num); \
		num         += 8; \
		part2Remain -= 8; \
	}

int32 MPG123Player::III_DequantizeSample(real xr[SBLIMIT][SSLIMIT], int32 *scf, GrInfoS *grInfo, int32 sFreq, int32 part2Bits)
{
	int32 shift = 1 + grInfo->scaleFacScale;
	real *xrPnt = (real *)xr;
	int32 l[3], l3;
	int32 part2Remain = grInfo->part2_3length - part2Bits;
	int32 *me;

	int32 num = GetBitOffset(&bsi);

	// We must split this, because for num==0 the shift is undefined if you do it in one step
	int32 mask;
	mask   = ((int32)GetBits(&bsi, num)) << BITSHIFT;
	mask <<= 8 - num;

	part2Remain -= num;

	{
		int32 bv      = grInfo->bigValues;
		int32 region1 = grInfo->region1Start;
		int32 region2 = grInfo->region2Start;

		l3 = ((576 >> 1) - bv) >> 1;

		// We may lose the 'odd' bit here!
		// Check this later again
		if (bv <= region1)
		{
			l[0] = bv;
			l[1] = l[2] = 0;
		}
		else
		{
			l[0] = region1;

			if (bv <= region2)
			{
				l[1] = bv - l[0];
				l[2] = 0;
			}
			else
			{
				l[1] = region2 - l[0];
				l[2] = bv - region2;
			}
		}
	}

	if (grInfo->blockType == 2)
	{
		// Decoding with short or mixed mode bandIndex table
		int32 i, max[4];
		int32 step = 0, lWin = 3, cb = 0;
		register real v = 0.0;
		register int32 *m, mc;

		if (grInfo->mixedBlockFlag)
		{
			max[3] = -1;
			max[0] = max[1] = max[2] = 2;
			m      = map[sFreq][0];
			me     = mapEnd[sFreq][0];
		}
		else
		{
			max[0] = max[1] = max[2] = max[3] = -1;

			// max[3] not really needed in this case
			m  = map[sFreq][1];
			me = mapEnd[sFreq][1];
		}

		mc = 0;

		for (i = 0; i < 2; i++)
		{
			int32 lp         = l[i];
			const NewHuff *h = ht + grInfo->tableSelect[i];

			for (; lp; lp--, mc--)
			{
				register int32 x, y;

				if (!mc)
				{
					mc    = *m++;
					xrPnt = ((real *)xr) + (*m++);
					lWin  = *m++;
					cb    = *m++;

					if (lWin == 3)
					{
						v    = grInfo->pow2Gain[(*scf++) << shift];
						step = 1;
					}
					else
					{
						v    = grInfo->fullGain[lWin][(*scf++) << shift];
						step = 3;
					}
				}

				{
					register const int16 *val = h->table;
					REFRESH_MASK;

					while ((y = *val++) < 0)
					{
						if (mask < 0)
							val -= y;

						num--;
						mask <<= 1;
					}

					x  = y >> 4;
					y &= 0xf;
				}

				if ((x == 15) && (h->linBits))
				{
					max[lWin] = cb;

					REFRESH_MASK;
					x     += ((uint32)mask) >> (BITSHIFT + 8 - h->linBits);
					num   -= h->linBits + 1;
					mask <<= h->linBits;

					if (mask < 0)
						*xrPnt = REAL_MUL(-isPow[x], v);
					else
						*xrPnt = REAL_MUL(isPow[x], v);

					mask <<= 1;
				}
				else
				{
					if (x)
					{
						max[lWin] = cb;

						if (mask < 0)
							*xrPnt = REAL_MUL(-isPow[x], v);
						else
							*xrPnt = REAL_MUL(isPow[x], v);

						num--;
						mask <<= 1;
					}
					else
						*xrPnt = DOUBLE_TO_REAL(0.0);
				}

				xrPnt += step;

				if ((y == 15) && (h->linBits))
				{
					max[lWin] = cb;

					REFRESH_MASK;
					y     += ((uint32)mask) >> (BITSHIFT + 8 - h->linBits);
					num   -= h->linBits + 1;
					mask <<= h->linBits;

					if (mask < 0)
						*xrPnt = REAL_MUL(-isPow[y], v);
					else
						*xrPnt = REAL_MUL(isPow[y], v);

					mask <<= 1;
				}
				else
				{
					if (y)
					{
						max[lWin] = cb;

						if (mask < 0)
							*xrPnt = REAL_MUL(-isPow[y], v);
						else
							*xrPnt = REAL_MUL(isPow[y], v);

						num--;
						mask <<= 1;
					}
					else
						*xrPnt = DOUBLE_TO_REAL(0.0);
				}

				xrPnt += step;
			}
		}

		for (; l3 && (part2Remain + num > 0); l3--)
		{
			const NewHuff *h          = htc + grInfo->count1TableSelect;
			register const int16 *val = h->table;
			register int16 a;

			REFRESH_MASK;
			while ((a = *val++) < 0)
			{
				if (mask < 0)
					val -= a;

				num--;
				mask <<= 1;
			}

			if ((part2Remain + num) <= 0)
			{
				num -= part2Remain + num;
				break;
			}

			for (i = 0; i < 4; i++)
			{
				if (!(i & 1))
				{
					if (!mc)
					{
						mc    = *m++;
						xrPnt = ((real *)xr) + (*m++);
						lWin  = *m++;
						cb    = *m++;

						if (lWin == 3)
						{
							v    = grInfo->pow2Gain[(*scf++) << shift];
							step = 1;
						}
						else
						{
							v    = grInfo->fullGain[lWin][(*scf++) << shift];
							step = 3;
						}
					}

					mc--;
				}

				if ((a & (0x8 >> i)))
				{
					max[lWin] = cb;

					if ((part2Remain + num) <= 0)
						break;

					if (mask < 0)
						*xrPnt = -v;
					else
						*xrPnt = v;

					num--;
					mask <<= 1;
				}
				else
					*xrPnt = DOUBLE_TO_REAL(0.0);

				xrPnt += step;
			}
		}

		if (lWin < 3)	// Short band?
		{
			while (true)
			{
				for (; mc > 0; mc--)
				{
					*xrPnt = DOUBLE_TO_REAL(0.0); xrPnt += 3;	// Short band -> step = 3
					*xrPnt = DOUBLE_TO_REAL(0.0); xrPnt += 3;
				}

				if (m >= me)
					break;

				mc    = *m++;
				xrPnt = ((real *)xr) + *m++;

				if (*m++ == 0)
					break;	// Optimize: Field will be set to zero at the end of the function

				m++;
			}
		}

		grInfo->maxBand[0] = max[0] + 1;
		grInfo->maxBand[1] = max[1] + 1;
		grInfo->maxBand[2] = max[2] + 1;
		grInfo->maxBandl   = max[3] + 1;

		{
			int32 rMax   = max[0] > max[1] ? max[0] : max[1];
			rMax         = (rMax > max[2] ? rMax : max[2]) + 1;
			grInfo->maxB = rMax ? shortLimit[sFreq][rMax] : longLimit[sFreq][max[3] + 1];
		}
	}
	else
	{
		// Decoding with 'long' bandIndex table (blockType != 2)
		const int32 *preTab = grInfo->preFlag ? preTab1 : preTab2;
		int32 i, max = -1;
		int32 cb = 0;
		int32 *m = map[sFreq][2];
		register real v = 0.0;
		int32 mc = 0;

		// Long hash table values
		for (i = 0; i < 3; i++)
		{
			int32 lp         = l[i];
			const NewHuff *h = ht + grInfo->tableSelect[i];

			for (; lp; lp--, mc--)
			{
				int32 x, y;

				if (!mc)
				{
					mc = *m++;
					cb = *m++;

					v = grInfo->pow2Gain[((*scf++) + (*preTab++)) << shift];
				}

				{
					register const int16 *val = h->table;

					REFRESH_MASK;

					while ((y = *val++) < 0)
					{
						if (mask < 0)
							val -= y;

						num--;
						mask <<= 1;
					}

					x  = y >> 4;
					y &= 0xf;
				}

				if ((x == 15) && (h->linBits))
				{
					max = cb;

					REFRESH_MASK;
					x     += ((uint32)mask) >> (BITSHIFT + 8 - h->linBits);
					num   -= h->linBits + 1;
					mask <<= h->linBits;

					if (mask < 0)
						*xrPnt++ = REAL_MUL(-isPow[x], v);
					else
						*xrPnt++ = REAL_MUL(isPow[x], v);

					mask <<= 1;
				}
				else
				{
					if (x)
					{
						max = cb;

						if (mask < 0)
							*xrPnt++ = REAL_MUL(-isPow[x], v);
						else
							*xrPnt++ = REAL_MUL(isPow[x], v);

						num--;
						mask <<= 1;
					}
					else
						*xrPnt++ = DOUBLE_TO_REAL(0.0);
				}

				if ((y == 15) && (h->linBits))
				{
					max = cb;

					REFRESH_MASK;
					y     += ((uint32)mask) >> (BITSHIFT + 8 - h->linBits);
					num   -= h->linBits + 1;
					mask <<= h->linBits;

					if (mask < 0)
						*xrPnt++ = REAL_MUL(-isPow[y], v);
					else
						*xrPnt++ = REAL_MUL(isPow[y], v);

					mask <<= 1;
				}
				else
				{
					if (y)
					{
						max = cb;

						if (mask < 0)
							*xrPnt++ = REAL_MUL(-isPow[y], v);
						else
							*xrPnt++ = REAL_MUL(isPow[y], v);

						num--;
						mask <<= 1;
					}
					else
						*xrPnt++ = DOUBLE_TO_REAL(0.0);
				}
			}
		}

		// Short (count1Table) values
		for (; l3 && (part2Remain + num > 0); l3--)
		{
			const NewHuff *h          = htc + grInfo->count1TableSelect;
			register const int16 *val = h->table;
			register int16 a;

			REFRESH_MASK;

			while ((a = *val++) < 0)
			{
				if (mask < 0)
					val -= a;

				num--;
				mask <<= 1;
			}

			if ((part2Remain + num) <= 0)
			{
				num -= part2Remain + num;
				break;
			}

			for (i = 0; i < 4; i++)
			{
				if (!(i & 1))
				{
					if (!mc)
					{
						mc = *m++;
						cb = *m++;

						v = grInfo->pow2Gain[((*scf++) + (*preTab++)) << shift];
					}

					mc--;
				}

				if ((a & (0x8 >> i)))
				{
					max = cb;

					if ((part2Remain + num) <= 0)
						break;

					if (mask < 0)
						*xrPnt++ = -v;
					else
						*xrPnt++ = v;

					num--;
					mask <<= 1;
				}
				else
					*xrPnt++ = DOUBLE_TO_REAL(0.0);
			}
		}

		grInfo->maxBandl = max + 1;
		grInfo->maxB     = longLimit[sFreq][grInfo->maxBandl];
	}

	part2Remain += num;
	BackBits(&bsi, num);
	num = 0;

	while (xrPnt < &xr[SBLIMIT][0])
		*xrPnt++ = DOUBLE_TO_REAL(0.0);

	while (part2Remain > 16)
	{
		GetBits(&bsi, 16);	// Dismiss stuffing bits
		part2Remain -= 16;
	}

	if (part2Remain > 0)
		GetBits(&bsi, part2Remain);
	else
	{
		if (part2Remain < 0)
			return (1);
	}

	return (0);
}



/******************************************************************************/
/* III_IStereo() calculate real channel values for Joint-I-Stereo-mode.       */
/******************************************************************************/
void MPG123Player::III_IStereo(real xrBuf[2][SBLIMIT][SSLIMIT], int32 *scaleFac, GrInfoS *grInfo, int32 sFreq, int32 msStereo, int32 lsf)
{
	real (*xr)[SBLIMIT * SSLIMIT] = (real (*)[SBLIMIT * SSLIMIT])xrBuf;
	const BandInfoStruct *bi      = &bandInfo[sFreq];
	const real *tab1, *tab2;
	int32 tab;

	tab  = lsf + (grInfo->scaleFacCompress & lsf);
	tab1 = stereoTabs[tab][msStereo][0];
	tab2 = stereoTabs[tab][msStereo][1];

	if (grInfo->blockType == 2)
	{
		int32 lWin, do_l = 0;

		if (grInfo->mixedBlockFlag)
			do_l = 1;

		for (lWin = 0; lWin < 3; lWin++)	// Process each window
		{
			// Get first band with zero values
			int32 is_p, sb, idx, sfb = grInfo->maxBand[lWin];	// sfb is minimal 3 for mixed mode

			if (sfb > 3)
				do_l = 0;

			for (; sfb < 12; sfb++)
			{
				is_p = scaleFac[sfb * 3 + lWin - grInfo->mixedBlockFlag];	// Scale: 0-15

				if (is_p != 7)
				{
					real t1, t2;

					sb  = bi->shortDiff[sfb];
					idx = bi->shortIdx[sfb] + lWin;
					t1  = tab1[is_p];
					t2  = tab2[is_p];

					for (; sb > 0; sb--, idx += 3)
					{
						real v     = xr[0][idx];
						xr[0][idx] = REAL_MUL(v, t1);
						xr[1][idx] = REAL_MUL(v, t2);
					}
				}
			}

			// In the original: copy 10 to 11, here: copy 11 to 12
			// Maybe still wrong??? (copy 12 to 13?)
			is_p = scaleFac[11 * 3 + lWin - grInfo->mixedBlockFlag];	// Scale: 0-15
			sb   = bi->shortDiff[12];
			idx  = bi->shortIdx[12] + lWin;

			if (is_p != 7)
			{
				real t1, t2;

				t1 = tab1[is_p];
				t2 = tab2[is_p];

				for (; sb > 0; sb--, idx += 3)
				{
					real v     = xr[0][idx];
					xr[0][idx] = REAL_MUL(v, t1);
					xr[1][idx] = REAL_MUL(v, t2);
				}
			}
		}

		// Also check l-part, if ALL bands in the three windows are 'empty'
		// and mode = mixedMode
		if (do_l)
		{
			int32 sfb = grInfo->maxBandl;
			int32 idx = bi->longIdx[sfb];

			for (; sfb < 8; sfb++)
			{
				int32 sb   = bi->longDiff[sfb];
				int32 is_p = scaleFac[sfb];		// Scale: 0-15

				if (is_p != 7)
				{
					real t1, t2;

					t1 = tab1[is_p];
					t2 = tab2[is_p];

					for (; sb > 0; sb--, idx++)
					{
						real v     = xr[0][idx];
						xr[0][idx] = REAL_MUL(v, t1);
						xr[1][idx] = REAL_MUL(v, t2);
					}
				}
				else
					idx += sb;
			}
		}
	}
	else
	{
		int32 sfb = grInfo->maxBandl;
		int32 is_p, idx = bi->longIdx[sfb];

		// Hmm... maybe the maxBandl stuff for i-stereo is buggy?
		if (sfb <= 21)
		{
			for (; sfb < 21; sfb++)
			{
				int32 sb = bi->longDiff[sfb];
				is_p     = scaleFac[sfb];	// Scale: 0-15

				if (is_p != 7)
				{
					real t1, t2;

					t1 = tab1[is_p];
					t2 = tab2[is_p];

					for (; sb > 0; sb--, idx++)
					{
						real v     = xr[0][idx];
						xr[0][idx] = REAL_MUL(v, t1);
						xr[1][idx] = REAL_MUL(v, t2);
					}
				}
				else
					idx += sb;
			}

			is_p = scaleFac[20];

			if (is_p != 7)	// Copy l-band 20 to l-band 21
			{
				int32 sb;
				real t1 = tab1[is_p];
				real t2 = tab2[is_p];

				for (sb = bi->longDiff[21]; sb > 0; sb--, idx++)
				{
					real v     = xr[0][idx];
					xr[0][idx] = REAL_MUL(v, t1);
					xr[1][idx] = REAL_MUL(v, t2);
				}
			}
		}
	}
}



/******************************************************************************/
/* III_AntiAlias() ???                                                        */
/******************************************************************************/
void MPG123Player::III_AntiAlias(real xr[SBLIMIT][SSLIMIT], GrInfoS *grInfo)
{
	int32 sbLim;

	if (grInfo->blockType == 2)
	{
		if (!grInfo->mixedBlockFlag)
			return;

		sbLim = 1;
	}
	else
		sbLim = grInfo->maxB - 1;

	// 31 alias-reduction operations between each pair of sub-bands
	// with 8 butterflies between each pair
	{
		int32 sb;
		real *xr1 = (real *)xr[1];

		for (sb = sbLim; sb; sb--, xr1 += 10)
		{
			int32 ss;
			real *cs = aa_cs, *ca = aa_ca;
			real *xr2 = xr1;

			for (ss = 7; ss >= 0; ss--)
			{
				// Upper and lower butterfly inputs
				register real bu = *--xr2, bd = *xr1;
				*xr2   = REAL_MUL(bu, *cs) - REAL_MUL(bd, *ca);
				*xr1++ = REAL_MUL(bd, *cs++) + REAL_MUL(bu, *ca++);
			}
		}
	}
}



/******************************************************************************/
/* III_Hybrid() ???                                                           */
/******************************************************************************/
void MPG123Player::III_Hybrid(MPStr *mp, real fsIn[SBLIMIT][SSLIMIT], real tsOut[SSLIMIT][SBLIMIT], int32 ch, GrInfoS *grInfo, Frame *fr)
{
	real *tsPnt = (real *)tsOut;
	real *rawOut1, *rawOut2;
	int32 bt;
	uint32 sb = 0;

	{
		int32 b            = mp->hybrid_blc[ch];
		rawOut1            = mp->hybrid_block[b][ch];
		b                  = -b + 1;
		rawOut2            = mp->hybrid_block[b][ch];
		mp->hybrid_blc[ch] = b;
	}

	if (grInfo->mixedBlockFlag)
	{
		sb = 2;

		if (got3DNow)
		{
			Dct36_3DNow(fsIn[0], rawOut1, rawOut2, win[0], tsPnt, cos9, tfcos36);
			Dct36_3DNow(fsIn[1], rawOut1 + 18, rawOut2 + 18, win1[0], tsPnt + 1, cos9, tfcos36);
		}
		else
		{
			Dct36(fsIn[0], rawOut1, rawOut2, win[0], tsPnt);
			Dct36(fsIn[1], rawOut1 + 18, rawOut2 + 18, win1[0], tsPnt + 1);
		}

		rawOut1 += 36;
		rawOut2 += 36;
		tsPnt   += 2;
	}

	bt = grInfo->blockType;
	if (bt == 2)
	{
		for (; sb < grInfo->maxB; sb += 2, tsPnt += 2, rawOut1 += 36, rawOut2 += 36)
		{
			Dct12(fsIn[sb], rawOut1, rawOut2, win[2], tsPnt);
			Dct12(fsIn[sb + 1], rawOut1 + 18, rawOut2 + 18, win1[2], tsPnt + 1);
		}
	}
	else
	{
		for (; sb < grInfo->maxB; sb += 2, tsPnt += 2, rawOut1 += 36, rawOut2 += 36)
		{
			if (got3DNow)
			{
				Dct36_3DNow(fsIn[sb], rawOut1, rawOut2, win[bt], tsPnt, cos9, tfcos36);
				Dct36_3DNow(fsIn[sb + 1], rawOut1 + 18, rawOut2 + 18, win1[bt], tsPnt + 1, cos9, tfcos36);
			}
			else
			{
				Dct36(fsIn[sb], rawOut1, rawOut2, win[bt], tsPnt);
				Dct36(fsIn[sb + 1], rawOut1 + 18, rawOut2 + 18, win1[bt], tsPnt + 1);
			}
		}
	}

	for (; sb < SBLIMIT; sb++, tsPnt++)
	{
		int32 i;

		for (i = 0; i < SSLIMIT; i++)
		{
			tsPnt[i * SBLIMIT] = *rawOut1++;
			*rawOut2++         = DOUBLE_TO_REAL(0.0);
		}
	}
}



/******************************************************************************/
/* DoLayer3() decode a layer 3 frame.                                         */
/*                                                                            */
/* Input:  "mp" is a pointer the the mpeg structure to use.                   */
/*         "fr" is a pointer to the frame header to use.                      */
/*                                                                            */
/* Output: Number of samples clipped.                                         */
/******************************************************************************/
int32 MPG123Player::DoLayer3(MPStr *mp, Frame *fr)
{
	int32 gr, ch, ss, clip = 0;
	int32 scaleFacs[2][39];		// Max 39 for short[13][3] mode, mixed: 38, long: 22
	III_SideInfo sideInfo;
	int32 stereo = fr->stereo;
	int32 single = fr->single;
	int32 msStereo, iStereo;
	int32 sFreq = fr->samplingFrequency;
	int32 stereo1, granules;

	if (stereo == 1)	// Stream is mono
	{
		stereo1 = 1;
		single  = 0;
	}
	else
	{
		if (single >= 0)	// Stream is stereo, but force to mono
			stereo1 = 1;
		else
			stereo1 = 2;
	}

	if (fr->mode == MPG_MD_JOINT_STEREO)
	{
		msStereo = (fr->modeExt & 0x2) >> 1;
		iStereo  = fr->modeExt & 0x1;
	}
	else
		msStereo = iStereo = 0;

	granules = fr->lsf ? 1 : 2;

	if (!III_GetSideInfo(&sideInfo, stereo, msStereo, sFreq, single, fr->lsf))
		return (-1);

	SetPointer(fr->sideInfoSize, sideInfo.mainDataBegin);

	for (gr = 0; gr < granules; gr++)
	{
		real hybridIn[2][SBLIMIT][SSLIMIT];
		real hybridOut[2][SSLIMIT][SBLIMIT];

		{
			GrInfoS *grInfo = &(sideInfo.ch[0].gr[gr]);
			int32 part2Bits;

			if (fr->lsf)
				part2Bits = III_GetScaleFactors2(scaleFacs[0], grInfo, 0);
			else
				part2Bits = III_GetScaleFactors1(scaleFacs[0], grInfo);

			if (III_DequantizeSample(hybridIn[0], scaleFacs[0], grInfo, sFreq, part2Bits))
				return (clip);
		}

		if (stereo == 2)
		{
			GrInfoS *grInfo = &(sideInfo.ch[1].gr[gr]);
			int32 part2Bits;

			if (fr->lsf)
				part2Bits = III_GetScaleFactors2(scaleFacs[1], grInfo, iStereo);
			else
				part2Bits = III_GetScaleFactors1(scaleFacs[1], grInfo);

			if (III_DequantizeSample(hybridIn[1], scaleFacs[1], grInfo, sFreq, part2Bits))
				return (clip);

			if (msStereo)
			{
				uint32 i;
				uint32 maxB = sideInfo.ch[0].gr[gr].maxB;

				if (sideInfo.ch[1].gr[gr].maxB > maxB)
					maxB = sideInfo.ch[1].gr[gr].maxB;

				for (i = 0; i < SSLIMIT * maxB; i++)
				{
					real tmp0 = ((real *)hybridIn[0])[i];
					real tmp1 = ((real *)hybridIn[1])[i];

					((real *)hybridIn[0])[i] = tmp0 + tmp1;
					((real *)hybridIn[1])[i] = tmp0 - tmp1;
				}
			}

			if (iStereo)
				III_IStereo(hybridIn, scaleFacs[1], grInfo, sFreq, msStereo, fr->lsf);

			if (msStereo || iStereo || (single == 3))
			{
				if (grInfo->maxB > sideInfo.ch[0].gr[gr].maxB)
					sideInfo.ch[0].gr[gr].maxB = grInfo->maxB;
				else
					grInfo->maxB = sideInfo.ch[0].gr[gr].maxB;
			}

			switch (single)
			{
				case 3:
				{
					register uint32 i;
					register real *in0 = (real *)hybridIn[0], *in1 = (real *)hybridIn[1];

					for (i = 0; i < SSLIMIT * grInfo->maxB; i++, in0++)
						*in0 = (*in0 + *in1++);		// 0.5 done by pow-scale

					break;
				}

				case 1:
				{
					register uint32 i;
					register real *in0 = (real *)hybridIn[0], *in1 = (real *)hybridIn[1];

					for (i = 0; i < SSLIMIT * grInfo->maxB; i++)
						*in0++ = *in1++;

					break;
				}
			}
		}

		for (ch = 0; ch < stereo1; ch++)
		{
			GrInfoS *grInfo = &(sideInfo.ch[ch].gr[gr]);

			III_AntiAlias(hybridIn[ch], grInfo);
			III_Hybrid(mp, hybridIn[ch], hybridOut[ch], ch, grInfo, fr);
		}

		for (ss = 0; ss < SSLIMIT; ss++)
		{
			if (single >= 0)
				clip += Synth_1to1(hybridOut[0][ss], 0, pcmSample1, &pcmPoint);
			else
			{
				int32 p1 = pcmPoint;
				clip += Synth_1to1(hybridOut[0][ss], 0, pcmSample1, &p1);
				clip += Synth_1to1(hybridOut[1][ss], 1, pcmSample2, &pcmPoint);
			}
		}
	}

	return (clip);
}
