/******************************************************************************/
/* Vorbis lsp functions.                                                      */
/******************************************************************************/


// Player headers
#include "Scales.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Vorbis_LspToCurve()                                                        */
/******************************************************************************/
void OggVorbis::Vorbis_LspToCurve(float *curve, int32 *map, int32 n, int32 ln, float *lsp, int32 m, float amp, float ampOffset)
{
	int32 i;
	float wDel = M_PI / ln;

	for (i = 0; i < m; i++)
		lsp[i] = 2.0f * cos(lsp[i]);

	i = 0;
	while (i < n)
	{
		int32 j, k = map[i];
		float p = 0.5f;
		float q = 0.5f;
		float w = 2.0f * cos(wDel * k);

		for (j = 1; j < m; j += 2)
		{
			q *= w - lsp[j - 1];
			p *= w - lsp[j];
		}

		if (j == m)
		{
			// Odd order filter; slightly assymetric
			// the last coefficient
			q *= w - lsp[j - 1];
			p *= p * (4.0f - w * w);
			q *= q;
		}
		else
		{
			// Even order filter; still symmetric
			p *= p * (2.0f - w);
			q *= q * (2.0f + w);
		}

		q = FromdB(amp / sqrt(p + q) - ampOffset);

		curve[i] *= q;
		while (map[++i] == k)
			curve[i] *= q;
	}
}
