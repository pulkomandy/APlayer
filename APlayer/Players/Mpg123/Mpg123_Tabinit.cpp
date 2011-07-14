/******************************************************************************/
/* MPG123Player TabInit functions.                                            */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* MakeDecodeTables() creates needed tables when decoding.                    */
/*                                                                            */
/* Input:  "scaleVal" is the scale value.                                     */
/******************************************************************************/
void MPG123Player::MakeDecodeTables(int32 scaleVal)
{
	if (gotMMX)
		MakeDecodeTables_MMX(scaleVal, decWin_MMX, decWins_MMX);
	else
	{
		int32 i, j, k, kr, divv;
		real *cosTab;
		int32 idx;

		for (i = 0; i < 5; i++)
		{
			kr     = 0x10 >> i;
			divv   = 0x40 >> i;
			cosTab = pnts[i];

			for (k = 0; k < kr; k++)
				cosTab[k] = DOUBLE_TO_REAL(1.0 / (2.0 * cos(M_PI * ((double)k * 2.0 + 1.0) / (double)divv)));
		}

		idx      = 0;
		scaleVal = -scaleVal;

		for (i = 0, j = 0; i < 256; i++, j++, idx += 32)
		{
			if (idx < (512 + 16))
				decWin[idx + 16] = decWin[idx] = DOUBLE_TO_REAL((double)intWinBase[j] / 65536.0 * (double)scaleVal);

			if ((i % 32) == 31)
				idx -= 1023;

			if ((i % 64) == 63)
				scaleVal = -scaleVal;
		}

		for (; i < 512; i++, j--, idx += 32)
		{
			if (idx < (512 + 16))
				decWin[idx + 16] = decWin[idx] = DOUBLE_TO_REAL((double)intWinBase[j] / 65536.0 * (double)scaleVal);

			if ((i % 32) == 31)
				idx -= 1023;

			if ((i % 64) == 63)
				scaleVal = -scaleVal;
		}
	}
}
