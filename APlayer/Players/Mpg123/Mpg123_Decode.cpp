/******************************************************************************/
/* MPG123Player Decode functions.                                             */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* Macros                                                                     */
/******************************************************************************/
#define step		1		// Changed from 2 by Thomas Neumann

#define WRITE_SAMPLE(samples, sum, clip) \
	if ((sum) > REAL_PLUS_32767) \
	{ \
		*(samples) = 0x7fff; \
		(clip)++; \
	} \
	else \
	{ \
		if ((sum) < REAL_MINUS_32768) \
		{ \
			*(samples) = -0x8000; \
			(clip)++; \
		} \
		else \
			*(samples) = REAL_TO_SHORT(sum); \
	}



/******************************************************************************/
/* Synth_1to1() creates the sample data 1-1.                                  */
/*                                                                            */
/* Input:  "bandPtr" ???                                                      */
/*         "channel" is the channel to decode (0-1).                          */
/*         "out" is a pointer to store the sample data.                       */
/*         "pnt" ???                                                          */
/*                                                                            */
/* Output: Number to samples clipped.                                         */
/******************************************************************************/
int32 MPG123Player::Synth_1to1(real *bandPtr, int32 channel, uint8 *out, int32 *pnt)
{
	if (got3DNow)
		return (Synth_1to1_3DNow(bandPtr, channel, out, pnt, (real *)buffs, &bo, decWin, pnts));

	if (gotMMX)
	{
		int16 *samples = (int16 *)(out + *pnt);

		Synth_1to1_MMX(bandPtr, channel, samples, (int16 *)buffs_MMX, &bo, decWins_MMX);

		*pnt += 64;		// Changed by Thomas Neumann from 128
		return (0);
	}

	{
		int16 *samples = (int16 *)(out + *pnt);
		real *b0, (*buf)[0x110];
		int32 clip = 0;
		int32 bo1;

		if (!channel)
		{
			bo--;
			bo &= 0xf;
			buf = buffs[0];
		}
		else
		{
//			samples++;			// Removed by Thomas Neumann
			buf = buffs[1];
		}

		if (bo & 0x1)
		{
			b0  = buf[0];
			bo1 = bo;
			Dct64(buf[1] + ((bo + 1) & 0xf), buf[0] + bo, bandPtr);
		}
		else
		{
			b0  = buf[1];
			bo1 = bo + 1;
			Dct64(buf[0] + bo, buf[1] + bo + 1, bandPtr);
		}

		{
			register int32 j;
			real *window = decWin + 16 - bo1;

			for (j = 16; j; j--, window += 0x10, samples += step)
			{
				real sum;

				sum  = REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);
				sum += REAL_MUL(*window++, *b0++);
				sum -= REAL_MUL(*window++, *b0++);

				WRITE_SAMPLE(samples, sum, clip);
			}

			{
				real sum;

				sum  = REAL_MUL(window[0x0], b0[0x0]);
				sum += REAL_MUL(window[0x2], b0[0x2]);
				sum += REAL_MUL(window[0x4], b0[0x4]);
				sum += REAL_MUL(window[0x6], b0[0x6]);
				sum += REAL_MUL(window[0x8], b0[0x8]);
				sum += REAL_MUL(window[0xa], b0[0xa]);
				sum += REAL_MUL(window[0xc], b0[0xc]);
				sum += REAL_MUL(window[0xe], b0[0xe]);

				WRITE_SAMPLE(samples, sum, clip);
				b0 -= 0x10, window -= 0x20, samples += step;
			}

			window += bo1 << 1;

			for (j = 15; j; j--, b0 -= 0x20, window -= 0x10, samples += step)
			{
				real sum;

				sum  = -REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);
				sum -=  REAL_MUL(*(--window), *b0++);

				WRITE_SAMPLE(samples, sum, clip);
			}
		}

		*pnt += 64;		// Changed by Thomas Neumann from 128

		return (clip);
	}
}
