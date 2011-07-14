/******************************************************************************/
/* Vorbis res0 functions.                                                     */
/******************************************************************************/


// Player headers
#include "Registry.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Structures                                                                 */
/******************************************************************************/
typedef struct VorbisLookResidue0
{
	VorbisInfoResidue0 *info;

	int32 parts;
	int32 stages;
	Codebook *fullBooks;
	Codebook *phraseBook;
	Codebook ***partBooks;

	int32 partVals;
	int32 **decodeMap;
} VorbisLookResidue0;



/******************************************************************************/
/* Res0_Unpack()                                                              */
/******************************************************************************/
VorbisInfoResidue *OggVorbis::Res0_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb)
{
	int32 j, acc = 0;
	VorbisInfoResidue0 *info = (VorbisInfoResidue0 *)_ogg_calloc(1, sizeof(*info));
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;

	info->begin      = obj->OggPack_Read(opb, 24);
	info->end        = obj->OggPack_Read(opb, 24);
	info->grouping   = obj->OggPack_Read(opb, 24) + 1;
	info->partitions = obj->OggPack_Read(opb, 6) + 1;
	info->groupBook  = obj->OggPack_Read(opb, 8);

	for (j = 0; j < info->partitions; j++)
	{
		int32 cascade = obj->OggPack_Read(opb, 3);
		if (obj->OggPack_Read(opb, 1))
			cascade |= (obj->OggPack_Read(opb, 5) << 3);

		info->secondStages[j] = cascade;

		acc += icount(cascade);
	}

	for (j = 0; j < acc; j++)
		info->bookList[j] = obj->OggPack_Read(opb, 8);

	if (info->groupBook >= ci->books)
		goto errout;

	for (j = 0; j < acc; j++)
	{
		if (info->bookList[j] >= ci->books)
			goto errout;
	}

	return (info);

errout:
	Res0_FreeInfo(info);
	return (NULL);
}



/******************************************************************************/
/* Res0_Look()                                                                */
/******************************************************************************/
VorbisLookResidue *OggVorbis::Res0_Look(VorbisDSPState *vd, VorbisInfoResidue *vr)
{
	VorbisInfoResidue0 *info = (VorbisInfoResidue0 *)vr;
	VorbisLookResidue0 *look = (VorbisLookResidue0 *)_ogg_calloc(1, sizeof(*look));
	CodecSetupInfo *ci       = (CodecSetupInfo *)vd->vi->codecSetup;

	int32 j, k, acc = 0;
	int32 dim;
	int32 maxStage = 0;

	look->info = info;

	look->parts      = info->partitions;
	look->fullBooks  = ci->fullBooks;
	look->phraseBook = ci->fullBooks + info->groupBook;
	dim = look->phraseBook->dim;

	look->partBooks = (Codebook ***)_ogg_calloc(look->parts, sizeof(*look->partBooks));

	for (j = 0; j < look->parts; j++)
	{
		int32 stages = ilog(info->secondStages[j]);
		if (stages)
		{
			if (stages > maxStage)
				maxStage = stages;

			look->partBooks[j] = (Codebook **)_ogg_calloc(stages, sizeof(*look->partBooks[j]));

			for (k = 0; k < stages; k++)
			{
				if (info->secondStages[j] & (1 << k))
					look->partBooks[j][k] = ci->fullBooks + info->bookList[acc++];
			}
		}
	}

	look->partVals  = (int32)rint(pow((float)look->parts, (float)dim));
	look->stages    = maxStage;
	look->decodeMap = (int32 **)_ogg_malloc(look->partVals * sizeof(*look->decodeMap));

	for (j = 0; j < look->partVals; j++)
	{
		int32 val = j;
		int32 mult = look->partVals / look->parts;
		look->decodeMap[j] = (int32 *)_ogg_malloc(dim * sizeof(*look->decodeMap[j]));

		for (k = 0; k < dim; k++)
		{
			int32 deco = val / mult;
			val -= deco * mult;
			mult /= look->parts;
			look->decodeMap[j][k] = deco;
		}
	}

	return (look);
}



/******************************************************************************/
/* Res0_FreeInfo()                                                            */
/******************************************************************************/
void OggVorbis::Res0_FreeInfo(VorbisInfoResidue *i)
{
	VorbisInfoResidue0 *info = (VorbisInfoResidue0 *)i;
	_ogg_free(info);
}



/******************************************************************************/
/* Res0_FreeLook()                                                            */
/******************************************************************************/
void OggVorbis::Res0_FreeLook(VorbisLookResidue *i)
{
	int32 j;

	if (i)
	{
		VorbisLookResidue0 *look = (VorbisLookResidue0 *)i;

		for (j = 0; j < look->parts; j++)
			_ogg_free(look->partBooks[j]);

		_ogg_free(look->partBooks);

		for (j = 0; j < look->partVals; j++)
			_ogg_free(look->decodeMap[j]);

		_ogg_free(look->decodeMap);

		_ogg_free(look);
	}
}



/******************************************************************************/
/* Res0_Inverse()                                                             */
/******************************************************************************/
int32 OggVorbis::Res0_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisLookResidue *vl, float **in, bool *nonZero, int32 ch)
{
	int32 i, used = 0;

	for (i = 0; i < ch; i++)
	{
		if (nonZero[i])
			in[used++] = in[i];
	}

	if (used)
		return (obj->_01Inverse(vb, vl, in, used, Vorbis_Book_DecodeVsAdd));
	else
		return (0);
}



/******************************************************************************/
/* Res1_Inverse()                                                             */
/******************************************************************************/
int32 OggVorbis::Res1_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisLookResidue *vl, float **in, bool *nonZero, int32 ch)
{
	int32 i, used = 0;

	for (i = 0; i < ch; i++)
	{
		if (nonZero[i])
			in[used++] = in[i];
	}

	if (used)
		return (obj->_01Inverse(vb, vl, in, used, Vorbis_Book_DecodeVAdd));
	else
		return (0);
}



/******************************************************************************/
/* Res2_Inverse()                                                             */
/******************************************************************************/
int32 OggVorbis::Res2_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisLookResidue *vl, float **in, bool *nonZero, int32 ch)
{
	int32 i, k, l, s;
	VorbisLookResidue0 *look = (VorbisLookResidue0 *)vl;
	VorbisInfoResidue0 *info = look->info;

	// Move all this setup out later
	int32 samplesPerPartition = info->grouping;
	int32 partitionsPerWord   = look->phraseBook->dim;
	int32 n                   = info->end - info->begin;

	int32 partVals  = n / samplesPerPartition;
	int32 partWords = (partVals + partitionsPerWord - 1) / partitionsPerWord;
	int32 **partWord = (int32 **)obj->_Vorbis_Block_Alloc(vb, partWords * sizeof(*partWord));

	for (i = 0; i < ch; i++)
	{
		if (nonZero[i])
			break;
	}

	if (i == ch)
		return (0);

	for (s = 0; s < look->stages; s++)
	{
		for (i = 0, l = 0; i < partVals; l++)
		{
			if (s == 0)
			{
				// Fetch the partition word
				int32 temp = obj->Vorbis_Book_Decode(look->phraseBook, &vb->opb);
				if (temp == -1)
					goto eopbreak;

				partWord[l] = look->decodeMap[temp];
				if (partWord[l] == NULL)
					goto errout;
			}

			// Now we decode residual values for the partitions
			for (k = 0; k < partitionsPerWord && (i < partVals); k++, i++)
			{
				if (info->secondStages[partWord[l][k]] & (1 << s))
				{
					Codebook *stageBook = look->partBooks[partWord[l][k]][s];

					if (stageBook)
					{
						if (Vorbis_Book_DecodeVvAdd(obj, stageBook, in, i * samplesPerPartition + info->begin, ch, &vb->opb, samplesPerPartition) == -1)
							goto eopbreak;
					}
				}
			}
		}
	}

errout:
eopbreak:
	return (0);
}



/******************************************************************************/
/* _01Inverse()                                                               */
/*                                                                            */
/* A truncated packet here just means 'stop working'; it's not an error.      */
/******************************************************************************/
int32 OggVorbis::_01Inverse(VorbisBlock *vb, VorbisLookResidue *vl, float **in, int32 ch, int32 (*decodePart)(OggVorbis *, Codebook *, float *, OggPackBuffer *, int32))
{
	int32 i, j, k, l, s;
	VorbisLookResidue0 *look = (VorbisLookResidue0 *)vl;
	VorbisInfoResidue0 *info = look->info;

	// Move all this setup out later
	int32 samplesPerPartition = info->grouping;
	int32 partitionsPerWord   = look->phraseBook->dim;
	int32 n                   = info->end - info->begin;

	int32 partVals            = n / samplesPerPartition;
	int32 partWords           = (partVals + partitionsPerWord - 1) / partitionsPerWord;
	int32 ***partWord         = new int32 **[ch];

	for (j = 0; j < ch; j++)
		partWord[j] = (int32 **)_Vorbis_Block_Alloc(vb, partWords * sizeof(*partWord[j]));

	for (s = 0; s < look->stages; s++)
	{
		// Each loop decodes on partition codeword containing
		// partitionsPerWord partitions
		for (i = 0, l = 0; i < partVals; l++)
		{
			if (s == 0)
			{
				// Fetch the partition word for each channel
				for (j = 0; j < ch; j++)
				{
					int32 temp = Vorbis_Book_Decode(look->phraseBook, &vb->opb);
					if (temp == -1)
						goto eopbreak;

					partWord[j][l] = look->decodeMap[temp];
					if (partWord[j][l] == NULL)
						goto errout;
				}
			}

			// Now we decode residual values for the partitions
			for (k = 0; k < partitionsPerWord && (i < partVals); k++, i++)
			{
				for (j = 0; j < ch; j++)
				{
					int32 offset = info->begin + i * samplesPerPartition;
					if (info->secondStages[partWord[j][l][k]] & (1 << s))
					{
						Codebook *stageBook = look->partBooks[partWord[j][l][k]][s];
						if (stageBook)
						{
							if (decodePart(this, stageBook, in[j] + offset, &vb->opb, samplesPerPartition) == -1)
								goto eopbreak;
						}
					}
				}
			}
		}
	}

errout:
eopbreak:
	delete[] partWord;

	return (0);
}



// Export hooks
Vorbis_Func_Residue residue0_exportBundle =
{
	&OggVorbis::Res0_Unpack,
	&OggVorbis::Res0_Look,
	&OggVorbis::Res0_FreeInfo,
	&OggVorbis::Res0_FreeLook,
	&OggVorbis::Res0_Inverse
};



Vorbis_Func_Residue residue1_exportBundle =
{
	&OggVorbis::Res0_Unpack,
	&OggVorbis::Res0_Look,
	&OggVorbis::Res0_FreeInfo,
	&OggVorbis::Res0_FreeLook,
	&OggVorbis::Res1_Inverse
};



Vorbis_Func_Residue residue2_exportBundle =
{
	&OggVorbis::Res0_Unpack,
	&OggVorbis::Res0_Look,
	&OggVorbis::Res0_FreeInfo,
	&OggVorbis::Res0_FreeLook,
	&OggVorbis::Res2_Inverse
};
