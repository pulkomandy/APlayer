/******************************************************************************/
/* Vorbis floor0 functions.                                                   */
/******************************************************************************/


// Player headers
#include "Registry.h"
#include "Scales.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/
typedef struct VorbisLookFloor0
{
	int32 ln;
	int32 m;
	int32 **linearMap;
	int32 n[2];

	VorbisInfoFloor0 *vi;
	float *lspLook;
} VorbisLookFloor0;



/******************************************************************************/
/* Floor0_Map_LazyInit()                                                      */
/*                                                                            */
/* Initialize Bark scale and normalization lookups. We could do this with     */
/* static tables, but Vorbis allows a number of possible combinations, so     */
/* it's best to do it computationally.                                        */
/*                                                                            */
/* The below is authoritative in terms of defining scale mapping. Note that   */
/* the scale depends on the sampling rate as well as the linear block and     */
/* mapping sizes.                                                             */
/******************************************************************************/
void OggVorbis::Floor0_Map_LazyInit(VorbisBlock *vb, VorbisInfoFloor *infoX, VorbisLookFloor0 *look)
{
	if (!look->linearMap[vb->w])
	{
		VorbisDSPState *vd = vb->vd;
		VorbisInfo *vi = vd->vi;
		CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
		VorbisInfoFloor0 *info = (VorbisInfoFloor0 *)infoX;
		int32 w = vb->w;
		int32 n = ci->blockSizes[w] / 2, j;

		// We choose a scaling constant so that:
		//    floor(bark(rate/2-1)*C)=mapped-1
		//    floor(bark(rate/2)*C)=mapped
		float scale = look->ln / ToBark(info->rate / 2.0f);

		// The mapping from a linear scale to a smaller bark scale is
		// straightforward. We do *not* make sure that the linear mapping
		// does not skip bark-scale bins; the decoder simply skips them and
		// the encoder may do what it wishes in filling them. They're
		// necessary in some mapping combinations to keep the scale spacing
		// accurate
		look->linearMap[w] = (int32 *)_ogg_malloc((n + 1) * sizeof(**look->linearMap));

		for (j = 0; j < n; j++)
		{
			int32 val = (int32)floor(ToBark((info->rate / 2.0f) / n * j) * scale);	// Bark numbers represent band edges
			if (val >= look->ln)
				val = look->ln - 1;		// Guard against the approximation

			look->linearMap[w][j] = val;
		}

		look->linearMap[w][j] = -1;
		look->n[w]            = n;
	}
}



/******************************************************************************/
/* Floor0_Unpack()                                                            */
/******************************************************************************/
VorbisInfoFloor *OggVorbis::Floor0_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb)
{
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
	int32 j;

	VorbisInfoFloor0 *info = (VorbisInfoFloor0 *)_ogg_malloc(sizeof(*info));
	info->order    = obj->OggPack_Read(opb, 8);
	info->rate     = obj->OggPack_Read(opb, 16);
	info->barkMap  = obj->OggPack_Read(opb, 16);
	info->ampBits  = obj->OggPack_Read(opb, 6);
	info->ampdB    = obj->OggPack_Read(opb, 8);
	info->numBooks = obj->OggPack_Read(opb, 4) + 1;

	if (info->order < 1)
		goto err_out;

	if (info->rate < 1)
		goto err_out;

	if (info->barkMap < 1)
		goto err_out;

	if (info->numBooks < 1)
		goto err_out;

	for (j = 0; j < info->numBooks; j++)
	{
		info->books[j] = obj->OggPack_Read(opb, 8);
		if ((info->books[j] < 0) || (info->books[j] >= ci->books))
			goto err_out;
	}

	return (info);

err_out:
	Floor0_FreeInfo(info);
	return (NULL);
}



/******************************************************************************/
/* Floor0_Look()                                                              */
/******************************************************************************/
VorbisLookFloor *OggVorbis::Floor0_Look(VorbisDSPState *vd, VorbisInfoFloor *i)
{
	int32 j;
	VorbisInfoFloor0 *info = (VorbisInfoFloor0 *)i;
	VorbisLookFloor0 *look = (VorbisLookFloor0 *)_ogg_calloc(1, sizeof(*look));

	look->m  = info->order;
	look->ln = info->barkMap;
	look->vi = info;

	look->linearMap = (int32 **)_ogg_calloc(2, sizeof(*look->linearMap));

	look->lspLook = (float *)_ogg_malloc(look->ln * sizeof(*look->lspLook));
	for (j = 0; j < look->ln; j++)
		look->lspLook[j] = 2 * cos(M_PI / look->ln * j);

	return (look);
}



/******************************************************************************/
/* Floor0_FreeInfo()                                                          */
/******************************************************************************/
void OggVorbis::Floor0_FreeInfo(VorbisInfoFloor *i)
{
	VorbisInfoFloor0 *info = (VorbisInfoFloor0 *)i;
	_ogg_free(info);
}



/******************************************************************************/
/* Floor0_FreeLook()                                                          */
/******************************************************************************/
void OggVorbis::Floor0_FreeLook(VorbisLookFloor *i)
{
	VorbisLookFloor0 *look = (VorbisLookFloor0 *)i;

	if (look)
	{
		if (look->linearMap)
		{
			_ogg_free(look->linearMap[0]);
			_ogg_free(look->linearMap[1]);
			_ogg_free(look->linearMap);
		}

		_ogg_free(look->lspLook);
		_ogg_free(look);
	}
}



/******************************************************************************/
/* Floor0_Inverse1()                                                          */
/******************************************************************************/
void *OggVorbis::Floor0_Inverse1(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *i)
{
	VorbisLookFloor0 *look = (VorbisLookFloor0 *)i;
	VorbisInfoFloor0 *info = look->vi;
	int32 j, k;

	int32 ampRaw = obj->OggPack_Read(&vb->opb, info->ampBits);
	if (ampRaw > 0)		// Also handles the -1 out of data case
	{
		int32 maxVal  = (1 << info->ampBits) - 1;
		float amp     = (float)ampRaw / maxVal * info->ampdB;
		int32 bookNum = obj->OggPack_Read(&vb->opb, ilog(info->numBooks));

		if ((bookNum != -1) && (bookNum < info->numBooks))	// Be paranoid
		{
			CodecSetupInfo *ci = (CodecSetupInfo *)vb->vd->vi->codecSetup;
			Codebook *b = ci->fullBooks + info->books[bookNum];
			float last = 0.0f;

			// The additional b->dim is a guard against any possible stack
			// smash; b->dim is provably more than we can overflow the
			// vector
			float *lsp = (float *)obj->_Vorbis_Block_Alloc(vb, sizeof(*lsp) * (look->m + b->dim + 1));

			for (j = 0; j < look->m; j += b->dim)
			{
				if (obj->Vorbis_Book_DecodeVSet(b, lsp + j, &vb->opb, b->dim) == -1)
					goto eop;
			}

			for (j = 0; j < look->m;)
			{
				for (k = 0; k < b->dim; k++, j++)
					lsp[j] += last;

				last = lsp[j - 1];
			}

			lsp[look->m] = amp;

			return (lsp);
		}
	}

eop:
	return (NULL);
}



/******************************************************************************/
/* Floor0_Inverse2()                                                          */
/******************************************************************************/
int32 OggVorbis::Floor0_Inverse2(OggVorbis *obj, VorbisBlock *vb, VorbisLookFloor *i, void *memo, float *out)
{
	VorbisLookFloor0 *look = (VorbisLookFloor0 *)i;
	VorbisInfoFloor0 *info = look->vi;

	obj->Floor0_Map_LazyInit(vb, info, look);

	if (memo)
	{
		float *lsp = (float *)memo;
		float amp  = lsp[look->m];

		// Take the coefficients back to a spectral envelope curve
		obj->Vorbis_LspToCurve(out, look->linearMap[vb->w], look->n[vb->w], look->ln, lsp, look->m, amp, (float)info->ampdB);

		return (1);
	}

	memset(out, 0, sizeof(*out) * look->n[vb->w]);

	return (0);
}



// Export hooks
Vorbis_Func_Floor floor0_exportBundle =
{
	&OggVorbis::Floor0_Unpack,
	&OggVorbis::Floor0_Look,
	&OggVorbis::Floor0_FreeInfo,
	&OggVorbis::Floor0_FreeLook,
	&OggVorbis::Floor0_Inverse1,
	&OggVorbis::Floor0_Inverse2
};
