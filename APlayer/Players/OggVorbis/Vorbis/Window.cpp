/******************************************************************************/
/* Vorbis window functions.                                                   */
/******************************************************************************/


// Player headers
#include "OggVorbis.h"


/******************************************************************************/
/* _Vorbis_Window()                                                           */
/******************************************************************************/
float *OggVorbis::_Vorbis_Window(int32 type, int32 left)
{
	float *ret = (float *)_ogg_calloc(left, sizeof(*ret));
	int32 i;

	switch (type)
	{
		// The 'Vorbis window' (window 0) is sin(sin(x)*sin(x)*2pi)
		case 0:
		{
			for (i = 0; i < left; i++)
			{
				float x = (i + 0.5f) / left * M_PI / 2.0f;
				x = sin(x);
				x *= x;
				x *= M_PI / 2.0f;
				x = sin(x);
				ret[i] = x;
			}
			break;
		}

		default:
		{
			_ogg_free(ret);
			return (NULL);
		}
	}

	return (ret);
}



/******************************************************************************/
/* _Vorbis_ApplyWindow()                                                      */
/******************************************************************************/
void OggVorbis::_Vorbis_ApplyWindow(float *d, float *window[2], int32 *blockSizes, int32 lW, int32 w, int32 nW)
{
	lW = (w ? lW : 0);
	nW = (w ? nW : 0);

	{
		int32 n  = blockSizes[w];
		int32 ln = blockSizes[lW];
		int32 rn = blockSizes[nW];

		int32 leftBegin = n / 4 - ln / 4;
		int32 leftEnd   = leftBegin + ln / 2;

		int32 rightBegin = n / 2 + n / 4 - rn / 4;
		int32 rightEnd   = rightBegin + rn / 2;

		int32 i, p;

		for (i = 0; i < leftBegin; i++)
			d[i] = 0.0f;

		for (p = 0; i < leftEnd; i++, p++)
			d[i] *= window[lW][p];

		for (i = rightBegin, p = rn / 2 - 1; i < rightEnd; i++, p--)
			d[i] *= window[nW][p];

		for (; i < n; i++)
			d[i] = 0.0f;
	}
}
