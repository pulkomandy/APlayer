/******************************************************************************/
/* Vorbis mdct functions.                                                     */
/******************************************************************************/


// Player headers
#include "OggVorbis.h"


/******************************************************************************/
/* Mdct_Init() build lookups for trig functions; also pre-figure scaling and  */
/*      some window function algebra.                                         */
/******************************************************************************/
void OggVorbis::Mdct_Init(MdctLookup *lookup, int32 n)
{
	int32 *bitRev = (int32 *)_ogg_malloc(sizeof(*bitRev) * (n / 4));
	DATA_TYPE *T  = (DATA_TYPE *)_ogg_malloc(sizeof(*T) * (n + n / 4));

	int32 i;
	int32 n2 = n >> 1;
	int32 log2n = lookup->log2n = (int32)rint(log((float)n) / log(2.0f));

	lookup->n      = n;
	lookup->trig   = T;
	lookup->bitRev = bitRev;

	// Trig lookups...
	for (i = 0; i < n / 4; i++)
	{
		T[i * 2]          = FLOAT_CONV(cos((M_PI / n) * (4 * i)));
		T[i * 2 + 1]      = FLOAT_CONV(-sin((M_PI / n) * (4 * i)));
		T[n2 + i * 2]     = FLOAT_CONV(cos((M_PI / (2 * n)) * (2 * i + 1)));
		T[n2 + i * 2 + 1] = FLOAT_CONV(sin((M_PI / (2 * n)) * (2 * i + 1)));
	}

	for (i = 0; i < n / 8; i++)
	{
		T[n + i * 2]     = FLOAT_CONV(cos((M_PI / n) * (4 * i + 2)) * 0.5);
		T[n + i * 2 + 1] = FLOAT_CONV(-sin((M_PI / n) * (4 * i + 2)) * 0.5);
	}

	// Bitreverse lookup...
	{
		int32 mask = (1 << (log2n - 1)) - 1, i, j;
		int32 msb  = 1 << (log2n - 2);

		for (i = 0; i < n / 8; i++)
		{
			int32 acc = 0;

			for (j = 0; msb >> j; j++)
			{
				if ((msb >> j) & i)
					acc |= 1 << j;
			}

			bitRev[i * 2]     = ((~acc) & mask) - 1;
			bitRev[i * 2 + 1] = acc;
		}
	}

	lookup->scale = FLOAT_CONV(4.0f / n);
}



/******************************************************************************/
/* Mdct_Clear()                                                               */
/******************************************************************************/
void OggVorbis::Mdct_Clear(MdctLookup *l)
{
	if (l)
	{
		_ogg_free(l->trig);
		_ogg_free(l->bitRev);

		memset(l, 0, sizeof(*l));
	}
}



/******************************************************************************/
/* Mdct_Backward()                                                            */
/******************************************************************************/
void OggVorbis::Mdct_Backward(MdctLookup *init, DATA_TYPE *in, DATA_TYPE *out)
{
	int32 n  = init->n;
	int32 n2 = n >> 1;
	int32 n4 = n >> 2;

	// Rotate
	DATA_TYPE *iX = in + n2 - 7;
	DATA_TYPE *oX = out + n2 + n4;
	DATA_TYPE *t  = init->trig + n4;

	do
	{
		oX   -= 4;
		oX[0] = MULT_NORM(-iX[2] * t[3] - iX[0] * t[2]);
		oX[1] = MULT_NORM( iX[0] * t[3] - iX[2] * t[2]);
		oX[2] = MULT_NORM(-iX[6] * t[1] - iX[4] * t[0]);
		oX[3] = MULT_NORM( iX[4] * t[1] - iX[6] * t[0]);
		iX   -= 8;
		t    += 4;
	}
	while (iX >= in);

	iX = in + n2 - 8;
	oX = out + n2 + n4;
	t  = init->trig + n4;

	do
	{
		t    -= 4;
		oX[0] = MULT_NORM(iX[4] * t[3] + iX[6] * t[2]);
		oX[1] = MULT_NORM(iX[4] * t[2] - iX[6] * t[3]);
		oX[2] = MULT_NORM(iX[0] * t[1] + iX[2] * t[0]);
		oX[3] = MULT_NORM(iX[0] * t[0] - iX[2] * t[1]);
		iX   -= 8;
		oX   += 4;
	}
	while (iX >= in);

	Mdct_Butterflies(init, out + n2, n2);
	Mdct_BitReverse(init, out);

	// Rotate + window
	{
		DATA_TYPE *oX1 = out + n2 + n4;
		DATA_TYPE *oX2 = out + n2 + n4;
		DATA_TYPE *iX  = out;
		t              = init->trig + n2;

		do
		{
			oX1   -= 4;

			oX1[3] =  MULT_NORM(iX[0] * t[1] - iX[1] * t[0]);
			oX2[0] = -MULT_NORM(iX[0] * t[0] + iX[1] * t[1]);

			oX1[2] =  MULT_NORM(iX[2] * t[3] - iX[3] * t[2]);
			oX2[1] = -MULT_NORM(iX[2] * t[2] + iX[3] * t[3]);

			oX1[1] =  MULT_NORM(iX[4] * t[5] - iX[5] * t[4]);
			oX2[2] = -MULT_NORM(iX[4] * t[4] + iX[5] * t[5]);

			oX1[0] =  MULT_NORM(iX[6] * t[7] - iX[7] * t[6]);
			oX2[3] = -MULT_NORM(iX[6] * t[6] + iX[7] * t[7]);

			oX2   += 4;
			iX    += 8;
			t     += 8;
		}
		while (iX < oX1);

		iX  = out + n2 + n4;
		oX1 = out + n4;
		oX2 = oX1;

		do
		{
			oX1   -= 4;
			iX    -= 4;

			oX2[0] = -(oX1[3] = iX[3]);
			oX2[1] = -(oX1[2] = iX[2]);
			oX2[2] = -(oX1[1] = iX[1]);
			oX2[3] = -(oX1[0] = iX[0]);

			oX2   += 4;
		}
		while (oX2 < iX);

		iX  = out + n2 + n4;
		oX1 = out + n2 + n4;
		oX2 = out + n2;

		do
		{
			oX1   -= 4;
			oX1[0] = iX[3];
			oX1[1] = iX[2];
			oX1[2] = iX[1];
			oX1[3] = iX[0];
			iX    += 4;
		}
		while (oX1 > oX2);
	}
}



/******************************************************************************/
/* Mdct_BitReverse()                                                          */
/******************************************************************************/
void OggVorbis::Mdct_BitReverse(MdctLookup *init, DATA_TYPE *x)
{
	int32 n       = init->n;
	int32 *bit    = init->bitRev;
	DATA_TYPE *w0 = x;
	DATA_TYPE *w1 = x = w0 + (n >> 1);
	DATA_TYPE *t  = init->trig + n;

	do
	{
		DATA_TYPE *x0 = x + bit[0];
		DATA_TYPE *x1 = x + bit[1];

		REG_TYPE r0   = x0[1] - x1[1];
		REG_TYPE r1   = x0[0] + x1[0];
		REG_TYPE r2   = MULT_NORM(r1 * t[0] + r0 * t[1]);
		REG_TYPE r3   = MULT_NORM(r1 * t[1] - r0 * t[0]);

		w1           -= 4;

		r0            = HALVE(x0[1] + x1[1]);
		r1            = HALVE(x0[0] - x1[0]);

		w0[0]         = r0 + r2;
		w1[2]         = r0 - r2;
		w0[1]         = r1 + r3;
		w1[3]         = r3 - r1;

		x0            = x + bit[2];
		x1            = x + bit[3];

		r0            = x0[1] - x1[1];
		r1            = x0[0] + x1[0];
		r2            = MULT_NORM(r1 * t[2] + r0 * t[3]);
		r3            = MULT_NORM(r1 * t[3] - r0 * t[2]);

		r0            = HALVE(x0[1] + x1[1]);
		r1            = HALVE(x0[0] - x1[0]);

		w0[2]         = r0 + r2;
		w1[0]         = r0 - r2;
		w0[3]         = r1 + r3;
		w1[1]         = r3 - r1;

		t            += 4;
		bit          += 4;
		w0           += 4;
	}
	while (w0 < w1);
}



/******************************************************************************/
/* Mdct_Butterflies()                                                         */
/******************************************************************************/
void OggVorbis::Mdct_Butterflies(MdctLookup *init, DATA_TYPE *x, int32 points)
{
	DATA_TYPE *t = init->trig;
	int32 stages = init->log2n - 5;
	int32 i, j;

	if (--stages > 0)
		Mdct_ButterflyFirst(t, x, points);

	for (i = 1; --stages > 0; i++)
	{
		for (j = 0; j < (1 << i); j++)
			Mdct_ButterflyGeneric(t, x + (points >> i) * j, points >> i, 4 << i);
	}

	for (j = 0; j < points; j += 32)
		Mdct_Butterfly32(x + j);
}



/******************************************************************************/
/* Mdct_ButterflyFirst()                                                      */
/******************************************************************************/
void OggVorbis::Mdct_ButterflyFirst(DATA_TYPE *t, DATA_TYPE *x, int32 points)
{
	DATA_TYPE *x1 = x + points - 8;
	DATA_TYPE *x2 = x + (points >> 1) - 8;
	REG_TYPE r0;
	REG_TYPE r1;

	do
	{
		r0     = x1[6] - x2[6];

		r1     = x1[7] - x2[7];
		x1[6] += x2[6];
		x1[7] += x2[7];
		x2[6]  = MULT_NORM(r1 * t[1] + r0 * t[0]);
		x2[7]  = MULT_NORM(r1 * t[0] - r0 * t[1]);

		r0     = x1[4] - x2[4];
		r1     = x1[5] - x2[5];
		x1[4] += x2[4];
		x1[5] += x2[5];
		x2[4]  = MULT_NORM(r1 * t[5] + r0 * t[4]);
		x2[5]  = MULT_NORM(r1 * t[4] - r0 * t[5]);

		r0     = x1[2] - x2[2];
		r1     = x1[3] - x2[3];
		x1[2] += x2[2];
		x1[3] += x2[3];
		x2[2]  = MULT_NORM(r1 * t[9] + r0 * t[8]);
		x2[3]  = MULT_NORM(r1 * t[8] - r0 * t[9]);

		r0     = x1[0] - x2[0];
		r1     = x1[1] - x2[1];
		x1[0] += x2[0];
		x1[1] += x2[1];
		x2[0]  = MULT_NORM(r1 * t[13] + r0 * t[12]);
		x2[1]  = MULT_NORM(r1 * t[12] - r0 * t[13]);

		x1    -= 8;
		x2    -= 8;
		t     += 16;
	}
	while (x2 >= x);
}



/******************************************************************************/
/* Mdct_ButterflyGeneric()                                                    */
/******************************************************************************/
void OggVorbis::Mdct_ButterflyGeneric(DATA_TYPE *t, DATA_TYPE *x, int32 points, int32 trigInt)
{
	DATA_TYPE *x1 = x + points - 8;
	DATA_TYPE *x2 = x + (points >> 1) - 8;
	REG_TYPE r0;
	REG_TYPE r1;

	do
	{
		r0     = x1[6] - x2[6];

		r1     = x1[7] - x2[7];
		x1[6] += x2[6];
		x1[7] += x2[7];
		x2[6]  = MULT_NORM(r1 * t[1] + r0 * t[0]);
		x2[7]  = MULT_NORM(r1 * t[0] - r0 * t[1]);

		t     += trigInt;

		r0     = x1[4] - x2[4];
		r1     = x1[5] - x2[5];
		x1[4] += x2[4];
		x1[5] += x2[5];
		x2[4]  = MULT_NORM(r1 * t[1] + r0 * t[0]);
		x2[5]  = MULT_NORM(r1 * t[0] - r0 * t[1]);

		t     += trigInt;

		r0     = x1[2] - x2[2];
		r1     = x1[3] - x2[3];
		x1[2] += x2[2];
		x1[3] += x2[3];
		x2[2]  = MULT_NORM(r1 * t[1] + r0 * t[0]);
		x2[3]  = MULT_NORM(r1 * t[0] - r0 * t[1]);

		t     += trigInt;

		r0     = x1[0] - x2[0];
		r1     = x1[1] - x2[1];
		x1[0] += x2[0];
		x1[1] += x2[1];
		x2[0]  = MULT_NORM(r1 * t[1] + r0 * t[0]);
		x2[1]  = MULT_NORM(r1 * t[0] - r0 * t[1]);

		t     += trigInt;

		x1    -= 8;
		x2    -= 8;
	}
	while (x2 >= x);
}



/******************************************************************************/
/* Mdct_Butterfly32()                                                         */
/******************************************************************************/
void OggVorbis::Mdct_Butterfly32(DATA_TYPE *x)
{
	REG_TYPE r0 = x[30] - x[14];
	REG_TYPE r1 = x[31] - x[15];

	x[30] += x[14];

	x[31] += x[15];

	x[14]  = r0;

	x[15]  = r1;

	r0     = x[28] - x[12];

	r1     = x[29] - x[13];

	x[28] += x[12];

	x[29] += x[13];

	x[12]  = MULT_NORM(r0 * cPI1_8 - r1 * cPI3_8);

	x[13]  = MULT_NORM(r0 * cPI3_8 + r1 * cPI1_8);

	r0     = x[26] - x[10];

	r1     = x[27] - x[11];
	x[26] += x[10];
	x[27] += x[11];
	x[10]  = MULT_NORM((r0 - r1) * cPI2_8);
	x[11]  = MULT_NORM((r0 + r1) * cPI2_8);

	r0     = x[24] - x[8];
	r1     = x[25] - x[9];
	x[24] += x[8];
	x[25] += x[9];
	x[8]   = MULT_NORM(r0 * cPI3_8 - r1 * cPI1_8);
	x[9]   = MULT_NORM(r1 * cPI3_8 + r0 * cPI1_8);

	r0     = x[22] - x[6];
	r1     = x[7] - x[23];
	x[22] += x[6];
	x[23] += x[7];
	x[6]   = r1;
	x[7]   = r0;

	r0     = x[4] - x[20];
	r1     = x[5] - x[21];
	x[20] += x[4];
	x[21] += x[5];
	x[4]   = MULT_NORM(r1 * cPI1_8 + r0 * cPI3_8);
	x[5]   = MULT_NORM(r1 * cPI3_8 - r0 * cPI1_8);

	r0     = x[2] - x[18];
	r1     = x[3] - x[19];
	x[18] += x[2];
	x[19] += x[3];
	x[2]   = MULT_NORM((r1 + r0) * cPI2_8);
	x[3]   = MULT_NORM((r1 - r0) * cPI2_8);

	r0     = x[0] - x[16];
	r1     = x[1] - x[17];
	x[16] += x[0];
	x[17] += x[1];
	x[0]   = MULT_NORM(r1 * cPI3_8 + r0 * cPI1_8);
	x[1]   = MULT_NORM(r1 * cPI1_8 - r0 * cPI3_8);

	Mdct_Butterfly16(x);
	Mdct_Butterfly16(x + 16);
}



/******************************************************************************/
/* Mdct_Butterfly16()                                                         */
/******************************************************************************/
void OggVorbis::Mdct_Butterfly16(DATA_TYPE *x)
{
	REG_TYPE r0 = x[1] - x[9];
	REG_TYPE r1 = x[0] - x[8];

	x[8]  += x[0];
	x[9]  += x[1];
	x[0]   = MULT_NORM((r0 + r1) * cPI2_8);
	x[1]   = MULT_NORM((r0 - r1) * cPI2_8);

	r0     = x[3] - x[11];
	r1     = x[10] - x[2];
	x[10] += x[2];
	x[11] += x[3];
	x[2]   = r0;
	x[3]   = r1;

	r0     = x[12] - x[4];
	r1     = x[13] - x[5];
	x[12] += x[4];
	x[13] += x[5];
	x[4]   = MULT_NORM((r0 - r1) * cPI2_8);
	x[5]   = MULT_NORM((r0 + r1) * cPI2_8);

	r0     = x[14] - x[6];
	r1     = x[15] - x[7];
	x[14] += x[6];
	x[15] += x[7];
	x[6]   = r0;
	x[7]   = r1;

	Mdct_Butterfly8(x);
	Mdct_Butterfly8(x + 8);
}



/******************************************************************************/
/* Mdct_Butterfly8()                                                          */
/******************************************************************************/
void OggVorbis::Mdct_Butterfly8(DATA_TYPE *x)
{
	REG_TYPE r0 = x[6] + x[2];
	REG_TYPE r1 = x[6] - x[2];
	REG_TYPE r2 = x[4] + x[0];
	REG_TYPE r3 = x[4] - x[0];

	x[6] = r0 + r2;
	x[4] = r0 - r2;

	r0   = x[5] - x[1];
	r2   = x[7] - x[3];
	x[0] = r1 + r0;
	x[2] = r1 - r0;

	r0   = x[5] + x[1];
	r1   = x[7] + x[3];
	x[3] = r2 + r3;
	x[1] = r2 - r3;
	x[7] = r1 + r0;
	x[5] = r1 - r0;
}
