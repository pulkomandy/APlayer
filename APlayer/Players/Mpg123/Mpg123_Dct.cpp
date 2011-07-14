/******************************************************************************/
/* MPG123Player Discrete Cosine Transform for subband synthesis functions.    */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* Dct12() ???                                                                */
/******************************************************************************/
void MPG123Player::Dct12(real *in, real *rawOut1, real *rawOut2, register real *wi, register real *ts)
{
#define DCT12_PART1 \
	        in5 = in[5 * 3];	\
	in5 += (in4 = in[4 * 3]);	\
	in4 += (in3 = in[3 * 3]);	\
	in3 += (in2 = in[2 * 3]);	\
	in2 += (in1 = in[1 * 3]);	\
	in1 += (in0 = in[0 * 3]);	\
								\
	in5 += in3; in3 += in1;		\
								\
	in2 = REAL_MUL(in2, cos6_1); \
	in3 = REAL_MUL(in3, cos6_1);

#define DCT12_PART2 \
	in0 += REAL_MUL(in4, cos6_2);	\
							\
	in4  = in0 + in2;		\
	in0 -= in2;				\
							\
	in1 += REAL_MUL(in5, cos6_2);	\
							\
	in5  = REAL_MUL((in1 + in3), tfcos12[0]); \
	in1  = REAL_MUL((in1 - in3), tfcos12[2]); \
							\
	in3  = in4 + in5;		\
	in4 -= in5;				\
							\
	in2  = in0 + in1;		\
	in0 -= in1;

	{
		real in0, in1, in2, in3, in4, in5;
		register real *out1 = rawOut1;

		ts[SBLIMIT * 0] = out1[0];	ts[SBLIMIT * 1] = out1[1];	ts[SBLIMIT * 2] = out1[2];
		ts[SBLIMIT * 3] = out1[3];	ts[SBLIMIT * 4] = out1[4];	ts[SBLIMIT * 5] = out1[5];

		DCT12_PART1

		{
			real tmp0, tmp1 = (in0 - in4);

			{
				real tmp2 = REAL_MUL((in1 - in5), tfcos12[1]);
				tmp0      = tmp1 + tmp2;
				tmp1     -= tmp2;
			}

			ts[(17 - 1) * SBLIMIT] = out1[17 - 1] + REAL_MUL(tmp0, wi[11 - 1]);
			ts[(12 + 1) * SBLIMIT] = out1[12 + 1] + REAL_MUL(tmp0, wi[ 6 + 1]);
			ts[( 6 + 1) * SBLIMIT] = out1[ 6 + 1] + REAL_MUL(tmp1, wi[1]);
			ts[(11 - 1) * SBLIMIT] = out1[11 - 1] + REAL_MUL(tmp1, wi[ 5 - 1]);
		}

		DCT12_PART2

		ts[(17 - 0) * SBLIMIT] = out1[17 - 0] + REAL_MUL(in2, wi[11 - 0]);
		ts[(12 + 0) * SBLIMIT] = out1[12 + 0] + REAL_MUL(in2, wi[ 6 + 0]);
		ts[(12 + 2) * SBLIMIT] = out1[12 + 2] + REAL_MUL(in3, wi[ 6 + 2]);
		ts[(17 - 2) * SBLIMIT] = out1[17 - 2] + REAL_MUL(in3, wi[11 - 2]);

		ts[( 6 + 0) * SBLIMIT] = out1[ 6 + 0] + REAL_MUL(in0, wi[0]);
		ts[(11 - 0) * SBLIMIT] = out1[11 - 0] + REAL_MUL(in0, wi[ 5 - 0]);
		ts[( 6 + 2) * SBLIMIT] = out1[ 6 + 2] + REAL_MUL(in4, wi[2]);
		ts[(11 - 2) * SBLIMIT] = out1[11 - 2] + REAL_MUL(in4, wi[ 5 - 2]);
	}

	in++;

	{
		real in0, in1, in2, in3, in4, in5;
		register real *out2 = rawOut2;

		DCT12_PART1

		{
			real tmp0, tmp1 = (in0 - in4);

			{
				real tmp2 = REAL_MUL((in1 - in5), tfcos12[1]);
				tmp0      = tmp1 + tmp2;
				tmp1     -= tmp2;
			}

			out2[5 - 1] = REAL_MUL(tmp0, wi[11 - 1]);
			out2[0 + 1] = REAL_MUL(tmp0, wi[ 6 + 1]);

			ts[(12 + 1) * SBLIMIT] += REAL_MUL(tmp1, wi[1]);
			ts[(17 - 1) * SBLIMIT] += REAL_MUL(tmp1, wi[5 - 1]);
		}

		DCT12_PART2

		out2[5 - 0] = REAL_MUL(in2, wi[11 - 0]);
		out2[0 + 0] = REAL_MUL(in2, wi[ 6 + 0]);
		out2[0 + 2] = REAL_MUL(in3, wi[ 6 + 2]);
		out2[5 - 2] = REAL_MUL(in3, wi[11 - 2]);

		ts[(12 + 0) * SBLIMIT] += REAL_MUL(in0, wi[0]);
		ts[(17 - 0) * SBLIMIT] += REAL_MUL(in0, wi[5 - 0]);
		ts[(12 + 2) * SBLIMIT] += REAL_MUL(in4, wi[2]);
		ts[(17 - 2) * SBLIMIT] += REAL_MUL(in4, wi[5 - 2]);
	}

	in++;

	{
		real in0, in1, in2, in3, in4, in5;
		register real *out2 = rawOut2;

		out2[12] = out2[13] = out2[14] = out2[15] = out2[16] = out2[17] = 0.0;

		DCT12_PART1

		{
			real tmp0, tmp1 = (in0 - in4);

			{
				real tmp2 = REAL_MUL((in1 - in5), tfcos12[1]);
				tmp0      = tmp1 + tmp2;
				tmp1     -= tmp2;
			}

			out2[11 - 1]  = REAL_MUL(tmp0, wi[11 - 1]);
			out2[ 6 + 1]  = REAL_MUL(tmp0, wi[ 6 + 1]);
			out2[ 0 + 1] += REAL_MUL(tmp1, wi[1]);
			out2[ 5 - 1] += REAL_MUL(tmp1, wi[ 5 - 1]);
		}

		DCT12_PART2

		out2[11 - 0]  = REAL_MUL(in2, wi[11 - 0]);
		out2[ 6 + 0]  = REAL_MUL(in2, wi[ 6 + 0]);
		out2[ 6 + 2]  = REAL_MUL(in3, wi[ 6 + 2]);
		out2[11 - 2]  = REAL_MUL(in3, wi[11 - 2]);

		out2[ 0 + 0] += REAL_MUL(in0, wi[0]);
		out2[ 5 - 0] += REAL_MUL(in0, wi[5 - 0]);
		out2[ 0 + 2] += REAL_MUL(in4, wi[2]);
		out2[ 5 - 2] += REAL_MUL(in4, wi[5 - 2]);
	}
}



/******************************************************************************/
/* Dct36() calculates the inverse MDCT.                                       */
/*                                                                            */
/* This is an optimized DCT from Jeff Tsay's maplay 1.2+ package.             */
/* Saved one multiplication by doing the 'twiddle factor' stuff together with */
/* the window mul. (MH)                                                       */
/*                                                                            */
/* This uses Byeong Gi Lee's Fast Cosine Transform algorithm, but the 9 point */
/* IDCT needs to be reduced further. Unfortunately, I don't know how to do    */
/* that, because 9 is not an even number. -Jeff                               */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* 9 Point Inverse Discrete Cosine Transform                                  */
/*                                                                            */
/* This piece of code is Copyright 1997 Mikko Tommila and is freely usable by */
/* anybody. The algorithm itself is of course in the public domain.           */
/*                                                                            */
/* Again derived heuristically from the 9-point WFTA.                         */
/*                                                                            */
/* The algorithm is optimized (?) for speed, not for small rounding errors or */
/* good readability.                                                          */
/*                                                                            */
/* 36 additions, 11 multiplications.                                          */
/*                                                                            */
/* Again this is very likely sub-optimal.                                     */
/*                                                                            */
/* The code is optimized to use a minimum number of temporary variables, so   */
/* it should compile quite well even on 8-register Intel x86 processors.      */
/* This makes the code quite obfuscated and very difficult to understand.     */
/*                                                                            */
/* References:                                                                */
/* [1] S. Winograd: "On Computing the Discrete Fourier Transform",            */
/*     Mathematics of Computation, Volume 32, Number 141, January 1978,       */
/*     Pages 175-199                                                          */
/******************************************************************************/
void MPG123Player::Dct36(real *inBuf, real *o1, real *o2, real *winTab, real *tsBuf)
{
#ifdef NEW_DCT9
	real tmp[18];
#endif

	{
		register real *in = inBuf;

		in[17] += in[16];	in[16] += in[15];	in[15] += in[14];
		in[14] += in[13];	in[13] += in[12];	in[12] += in[11];
		in[11] += in[10];	in[10] += in[9];	in[9]  += in[8];
		in[8]  += in[7];	in[7]  += in[6];	in[6]  += in[5];
		in[5]  += in[4];	in[4]  += in[3];	in[3]  += in[2];
		in[2]  += in[1];	in[1]  += in[0];

		in[17] += in[15];	in[15] += in[13];	in[13] += in[11];	in[11] += in[9];
		in[9]  += in[7];	in[7]  += in[5];	in[5]  += in[3];	in[3]  += in[1];

#ifdef NEW_DCT9

		{
			real t3;

			{
				real t0, t1, t2;

				t0 = REAL_MUL(cos6_2, (in[8] + in[16] - in[4]));
				t1 = REAL_MUL(cos6_2, in[12]);

				t3     = in[0];
				t2     = t3 - t1 - t1;
				tmp[1] = tmp[7] = t2 - t0;
				tmp[4]          = t2 + t0 + t0;
				t3    += t1;

				t2      = REAL_MUL(cos6_1, (in[10] + in[14] - in[2]));
				tmp[1] -= t2;
				tmp[7] += t2;
			}

			{
				real t0, t1, t2;

				t0 = REAL_MUL(new_cos9[0], (in[4] + in[8]));
				t1 = REAL_MUL(new_cos9[1], (in[8] - in[16]));
				t2 = REAL_MUL(new_cos9[2], (in[4] + in[16]));

				tmp[2] = tmp[6] = t3 - t0      - t2;
				tmp[0] = tmp[8] = t3 + t0 + t1;
				tmp[3] = tmp[5] = t3      - t1 + t2;
			}
		}

		{
			real t1, t2, t3;

			t1 = REAL_MUL(new_cos18[0], (in[2] + in[10]));
			t2 = REAL_MUL(new_cos18[1], (in[10] - in[14]));
			t3 = REAL_MUL(cos6_1, in[6]);

			{
				real t0 = t1 + t2 + t3;
				tmp[0] += t0;
				tmp[8] -= t0;
			}

			t2 -= t3;
			t1 -= t3;

			t3 = REAL_MUL(new_cos18[2], (in[2] + in[14]));

			t1     += t3;
			tmp[3] += t1;
			tmp[5] -= t1;

			t2     -= t3;
			tmp[2] += t2;
			tmp[6] -= t2;
		}

		{
			real t0, t1, t2, t3, t4, t5, t6, t7;

			t1 = REAL_MUL(cos6_2, in[13]);
			t2 = REAL_MUL(cos6_2, (in[9] + in[17] - in[5]));

			t3 = in[1] + t1;
			t4 = in[1] - t1 - t1;
			t5 = t4 - t2;

			t0 = REAL_MUL(new_cos9[0], (in[5] + in[9]));
			t1 = REAL_MUL(new_cos9[1], (in[9] - in[17]));

			tmp[13] = REAL_MUL((t4 + t2 + t2), tfcos36[17 - 13]);
			t2      = REAL_MUL(new_cos9[2], (in[5] + in[17]));

			t6  = t3 - t0 - t2;
			t0 += t3 + t1;
			t3 += t2 - t1;

			t2 = REAL_MUL(new_cos18[0], (in[3] + in[11]));
			t4 = REAL_MUL(new_cos18[1], (in[11] - in[15]));
			t7 = REAL_MUL(cos6_1, in[7]);

			t1      = t2 + t4 + t7;
			tmp[17] = REAL_MUL((t0 + t1), tfcos36[17 - 17]);
			tmp[9]  = REAL_MUL((t0 - t1), tfcos36[17 - 9]);
			t1      = REAL_MUL(new_cos18[2], (in[3] + in[15]));
			t2     += t1 - t7;

			tmp[14] = REAL_MUL((t3 + t2), tfcos36[17 - 14]);
			t0      = REAL_MUL(cos6_1, (in[11] + in[15] - in[3]));
			tmp[12] = REAL_MUL((t3 - t2), tfcos36[17 - 12]);

			t4 -= t1 + t7;

			tmp[16] = REAL_MUL((t5 - t0), tfcos36[17 - 16]);
			tmp[10] = REAL_MUL((t5 + t0), tfcos36[17 - 10]);
			tmp[15] = REAL_MUL((t6 + t4), tfcos36[17 - 15]);
			tmp[11] = REAL_MUL((t6 - t4), tfcos36[17 - 11]);
		}

#define MACRO(v) \
	{ \
		real tmpVal; \
		tmpVal = tmp[(v)] + tmp[17 - (v)]; \
		out2[9 + (v)] = REAL_MUL(tmpVal, w[27 + (v)]); \
		out2[8 - (v)] = REAL_MUL(tmpVal, w[26 - (v)]); \
		tmpVal = tmp[(v)] - tmp[17 - (v)]; \
		ts[SBLIMIT * (8 - (v))] = out1[8 - (v)] + REAL_MUL(tmpVal, w[8 - (v)]); \
		ts[SBLIMIT * (9 + (v))] = out1[9 + (v)] + REAL_MUL(tmpVal, w[9 + (v)]); \
	}

		{
			register real *out2 = o2;
			register real *w    = winTab;
			register real *out1 = o1;
			register real *ts   = tsBuf;

			MACRO(0);
			MACRO(1);
			MACRO(2);
			MACRO(3);
			MACRO(4);
			MACRO(5);
			MACRO(6);
			MACRO(7);
			MACRO(8);
		}

#else

		{
#define MACRO0(v) \
	{ \
		real tmp; \
		out2[9 + (v)] = REAL_MUL((tmp = sum0 + sum1), w[27 + (v)]); \
		out2[8 - (v)] = REAL_MUL(tmp, w[26 - (v)]); \
	} \
	sum0 -= sum1; \
	ts[SBLIMIT * (8 - (v))] = out1[8 - (v)] + REAL_MUL(sum0, w[8 - (v)]); \
	ts[SBLIMIT * (9 + (v))] = out1[9 + (v)] + REAL_MUL(sum0, w[9 + (v)]);

#define MACRO1(v) \
	{ \
		real sum0, sum1; \
		sum0 = tmp1a + tmp2a; \
		sum1 = REAL_MUL((tmp1b + tmp2b), tfcos36[(v)]); \
		MACRO0(v); \
	}

#define MACRO2(v) \
	{ \
		real sum0, sum1; \
		sum0 = tmp2a - tmp1a; \
		sum1 = REAL_MUL((tmp2b - tmp1b), tfcos36[(v)]); \
		MACRO0(v); \
	}

			register const real *c = cos9;
			register real *out2    = o2;
			register real *w       = winTab;
			register real *out1    = o1;
			register real *ts      = tsBuf;

			real ta33, ta66, tb33, tb66;

			ta33 = REAL_MUL(in[2 * 3 + 0], c[3]);
			ta66 = REAL_MUL(in[2 * 6 + 0], c[6]);
			tb33 = REAL_MUL(in[2 * 3 + 1], c[3]);
			tb66 = REAL_MUL(in[2 * 6 + 1], c[6]);

			{
				real tmp1a, tmp2a, tmp1b, tmp2b;

				tmp1a = REAL_MUL(in[2 * 1 + 0], c[1]) + ta33 + REAL_MUL(in[2 * 5 + 0], c[5]) + REAL_MUL(in[2 * 7 + 0], c[7]);
				tmp1b = REAL_MUL(in[2 * 1 + 1], c[1]) + tb33 + REAL_MUL(in[2 * 5 + 1], c[5]) + REAL_MUL(in[2 * 7 + 1], c[7]);
				tmp2a = REAL_MUL(in[2 * 2 + 0], c[2]) + REAL_MUL(in[2 * 4 + 0], c[4]) + ta66 + REAL_MUL(in[2 * 8 + 0], c[8]);
				tmp2b = REAL_MUL(in[2 * 2 + 1], c[2]) + REAL_MUL(in[2 * 4 + 1], c[4]) + tb66 + REAL_MUL(in[2 * 8 + 1], c[8]);

				MACRO1(0);
				MACRO2(8);
			}

			{
				real tmp1a, tmp2a, tmp1b, tmp2b;

				tmp1a = REAL_MUL((in[2 * 1 + 0] - in[2 * 5 + 0] - in[2 * 7 + 0]), c[3]);
				tmp1b = REAL_MUL((in[2 * 1 + 1] - in[2 * 5 + 1] - in[2 * 7 + 1]), c[3]);
				tmp2a = REAL_MUL((in[2 * 2 + 0] - in[2 * 4 + 0] - in[2 * 8 + 0]), c[6]) - in[2 * 6 + 0] + in[2 * 0 + 0];
				tmp2b = REAL_MUL((in[2 * 2 + 1] - in[2 * 4 + 1] - in[2 * 8 + 1]), c[6]) - in[2 * 6 + 1] + in[2 * 0 + 1];

				MACRO1(1);
				MACRO2(7);
			}

			{
				real tmp1a, tmp2a, tmp1b, tmp2b;

				tmp1a =  REAL_MUL(in[2 * 1 + 0], c[5]) - ta33 - REAL_MUL(in[2 * 5 + 0], c[7]) + REAL_MUL(in[2 * 7 + 0], c[1]);
				tmp1b =  REAL_MUL(in[2 * 1 + 1], c[5]) - tb33 - REAL_MUL(in[2 * 5 + 1], c[7]) + REAL_MUL(in[2 * 7 + 1], c[1]);
				tmp2a = -REAL_MUL(in[2 * 2 + 0], c[8]) - REAL_MUL(in[2 * 4 + 0], c[2]) + ta66 + REAL_MUL(in[2 * 8 + 0], c[4]);
				tmp2b = -REAL_MUL(in[2 * 2 + 1], c[8]) - REAL_MUL(in[2 * 4 + 1], c[2]) + tb66 + REAL_MUL(in[2 * 8 + 1], c[4]);

				MACRO1(2);
				MACRO2(6);
			}

			{
				real tmp1a, tmp2a, tmp1b, tmp2b;

				tmp1a =  REAL_MUL(in[2 * 1 + 0], c[7]) - ta33 + REAL_MUL(in[2 * 5 + 0], c[1]) - REAL_MUL(in[2 * 7 + 0], c[5]);
				tmp1b =  REAL_MUL(in[2 * 1 + 1], c[7]) - tb33 + REAL_MUL(in[2 * 5 + 1], c[1]) - REAL_MUL(in[2 * 7 + 1], c[5]);
				tmp2a = -REAL_MUL(in[2 * 2 + 0], c[4]) + REAL_MUL(in[2 * 4 + 0], c[8]) + ta66 - REAL_MUL(in[2 * 8 + 0], c[2]);
				tmp2b = -REAL_MUL(in[2 * 2 + 1], c[4]) + REAL_MUL(in[2 * 4 + 1], c[8]) + tb66 - REAL_MUL(in[2 * 8 + 1], c[2]);

				MACRO1(3);
				MACRO2(5);
			}

			{
				real sum0, sum1;

				sum0 = in[2 * 0 + 0] - in[2 * 2 + 0] + in[2 * 4 + 0] - in[2 * 6 + 0] + in[2 * 8 + 0];
				sum1 = REAL_MUL((in[2 * 0 + 1] - in[2 * 2 + 1] + in[2 * 4 + 1] - in[2 * 6 + 1] + in[2 * 8 + 1]), tfcos36[4]);

				MACRO0(4);
			}
		}

#endif

	}
}



/******************************************************************************/
/* Dct64() do the DCT.                                                        */
/******************************************************************************/
void MPG123Player::Dct64(real *out0, real *out1, real *samples)
{
	real bufs[64];

	{
		register int32 i, j;
		register real *b1, *b2, *bs, *cosTab;

		b1     = samples;
		bs     = bufs;
		cosTab = pnts[0] + 16;
		b2     = b1 + 32;

		for (i = 15; i >= 0; i--)
			*bs++ = (*b1++ + *--b2);

		for (i = 15; i >= 0; i--)
			*bs++ = REAL_MUL((*--b2 - *b1++), *--cosTab);

		b1     = bufs;
		cosTab = pnts[1] + 8;
		b2     = b1 + 16;

		{
			for (i = 7; i >= 0; i--)
				*bs++ = (*b1++ + *--b2);

			for (i = 7; i >= 0; i--)
				*bs++ = REAL_MUL((*--b2 - *b1++), *--cosTab);

			b2     += 32;
			cosTab += 8;

			for (i = 7; i >= 0; i--)
				*bs++ = (*b1++ + *--b2);

			for (i = 7; i >= 0; i--)
				*bs++ = REAL_MUL((*b1++ - *--b2), *--cosTab);

			b2 += 32;
		}

		bs     = bufs;
		cosTab = pnts[2];
		b2     = b1 + 8;

		for (j = 2; j; j--)
		{
			for (i = 3; i >= 0; i--)
				*bs++ = (*b1++ + *--b2);

			for (i = 3; i >= 0; i--)
				*bs++ = REAL_MUL((*--b2 - *b1++), cosTab[i]);

			b2 += 16;

			for (i = 3; i >= 0; i--)
				*bs++ = (*b1++ + *--b2);

			for (i = 3; i >= 0; i--)
				*bs++ = REAL_MUL((*b1++ - *--b2), cosTab[i]);

			b2 += 16;
		}

		b1     = bufs;
		cosTab = pnts[3];
		b2     = b1 + 4;

		for (j = 4; j; j--)
		{
			*bs++ = (*b1++ + *--b2);
			*bs++ = (*b1++ + *--b2);
			*bs++ = REAL_MUL((*--b2 - *b1++), cosTab[1]);
			*bs++ = REAL_MUL((*--b2 - *b1++), cosTab[0]);

			b2 += 8;

			*bs++ = (*b1++ + *--b2);
			*bs++ = (*b1++ + *--b2);
			*bs++ = REAL_MUL((*b1++ - *--b2), cosTab[1]);
			*bs++ = REAL_MUL((*b1++ - *--b2), cosTab[0]);

			b2 += 8;
		}

		bs     = bufs;
		cosTab = pnts[4];

		for (j = 8; j; j--)
		{
			real v0, v1;

			v0 = *b1++;
			v1 = *b1++;

			*bs++ = (v0 + v1);
			*bs++ = REAL_MUL((v0 - v1), (*cosTab));

			v0 = *b1++;
			v1 = *b1++;

			*bs++ = (v0 + v1);
			*bs++ = REAL_MUL((v1 - v0), (*cosTab));
		}
	}

	{
		register real *b1;
		register int32 i;

		for (b1 = bufs, i = 8; i; i--, b1 += 4)
			b1[2] += b1[3];

		for (b1 = bufs, i = 4; i; i--, b1 += 8)
		{
			b1[4] += b1[6];
			b1[6] += b1[5];
			b1[5] += b1[7];
		}

		for (b1 = bufs, i = 2; i; i--, b1 += 16)
		{
			b1[8]  += b1[12];
			b1[12] += b1[10];
			b1[10] += b1[14];
			b1[14] += b1[9];
			b1[9]  += b1[13];
			b1[13] += b1[11];
			b1[11] += b1[15];
		}
	}

	out0[0x10 * 16] = bufs[0];
	out0[0x10 * 15] = bufs[16 + 0]  + bufs[16 + 8];
	out0[0x10 * 14] = bufs[8];
	out0[0x10 * 13] = bufs[16 + 8]  + bufs[16 + 4];
	out0[0x10 * 12] = bufs[4];
	out0[0x10 * 11] = bufs[16 + 4]  + bufs[16 + 12];
	out0[0x10 * 10] = bufs[12];
	out0[0x10 *  9] = bufs[16 + 12] + bufs[16 + 2];
	out0[0x10 *  8] = bufs[2];
	out0[0x10 *  7] = bufs[16 + 2]  + bufs[16 + 10];
	out0[0x10 *  6] = bufs[10];
	out0[0x10 *  5] = bufs[16 + 10] + bufs[16 + 6];
	out0[0x10 *  4] = bufs[6];
	out0[0x10 *  3] = bufs[16 + 6]  + bufs[16 + 14];
	out0[0x10 *  2] = bufs[14];
	out0[0x10 *  1] = bufs[16 + 14] + bufs[16 + 1];
	out0[0x10 *  0] = bufs[1];

	out1[0x10 *  0] = bufs[1];
	out1[0x10 *  1] = bufs[16 + 1]  + bufs[16 + 9];
	out1[0x10 *  2] = bufs[9];
	out1[0x10 *  3] = bufs[16 + 9]  + bufs[16 + 5];
	out1[0x10 *  4] = bufs[5];
	out1[0x10 *  5] = bufs[16 + 5]  + bufs[16 + 13];
	out1[0x10 *  6] = bufs[13];
	out1[0x10 *  7] = bufs[16 + 13] + bufs[16 + 3];
	out1[0x10 *  8] = bufs[3];
	out1[0x10 *  9] = bufs[16 + 3]  + bufs[16 + 11];
	out1[0x10 * 10] = bufs[11];
	out1[0x10 * 11] = bufs[16 + 11] + bufs[16 + 7];
	out1[0x10 * 12] = bufs[7];
	out1[0x10 * 13] = bufs[16 + 7]  + bufs[16 + 15];
	out1[0x10 * 14] = bufs[15];
	out1[0x10 * 15] = bufs[16 + 15];
}
