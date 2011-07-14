/******************************************************************************/
/* Vorbis mapping0 functions.                                                 */
/******************************************************************************/


// Player headers
#include "Registry.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Mapping0_Unpack()                                                          */
/******************************************************************************/
VorbisInfoMapping *OggVorbis::Mapping0_Unpack(OggVorbis *obj, VorbisInfo *vi, OggPackBuffer *opb)
{
	int32 i;
	VorbisInfoMapping0 *info = (VorbisInfoMapping0 *)_ogg_calloc(1, sizeof(*info));
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;

	memset(info, 0, sizeof(*info));

	if (obj->OggPack_Read(opb, 1))
		info->subMaps = obj->OggPack_Read(opb, 4) + 1;
	else
		info->subMaps = 1;

	if (obj->OggPack_Read(opb, 1))
	{
		info->couplingSteps = obj->OggPack_Read(opb, 8) + 1;

		for (i = 0; i < info->couplingSteps; i++)
		{
			int32 testM = info->couplingMag[i] = obj->OggPack_Read(opb, ilog2(vi->channels));		// TN: ilog2 is correct, not ilog
			int32 testA = info->couplingAng[i] = obj->OggPack_Read(opb, ilog2(vi->channels));		// TN: ilog2 is correct, not ilog

			if ((testM < 0) || (testA < 0) || (testM == testA) || (testM >= vi->channels) || (testA >= vi->channels))
				goto err_out;
		}
	}

	if (obj->OggPack_Read(opb, 2) > 0)		// 2,3:reserved
		goto err_out;

	if (info->subMaps > 1)
	{
		for (i = 0; i < vi->channels; i++)
		{
			info->chMuxList[i] = obj->OggPack_Read(opb, 4);
			if (info->chMuxList[i] >= info->subMaps)
				goto err_out;
		}
	}

	for (i = 0; i < info->subMaps; i++)
	{
		obj->OggPack_Read(opb, 8);		// Time submap unused

		info->floorSubMap[i] = obj->OggPack_Read(opb, 8);
		if (info->floorSubMap[i] >= ci->floors)
			goto err_out;

		info->residueSubMap[i] = obj->OggPack_Read(opb, 8);
		if (info->residueSubMap[i] >= ci->residues)
			goto err_out;
	}

	return (info);

err_out:
	Mapping0_FreeInfo(info);
	return (NULL);
}



/******************************************************************************/
/* Mapping0_FreeInfo()                                                        */
/******************************************************************************/
void OggVorbis::Mapping0_FreeInfo(VorbisInfoMapping *i)
{
	VorbisInfoMapping0 *info = (VorbisInfoMapping0 *)i;
	_ogg_free(info);
}



/******************************************************************************/
/* Mapping0_Inverse()                                                         */
/******************************************************************************/
int32 OggVorbis::Mapping0_Inverse(OggVorbis *obj, VorbisBlock *vb, VorbisInfoMapping *l)
{
	VorbisDSPState *vd       = vb->vd;
	VorbisInfo *vi           = vd->vi;
	CodecSetupInfo *ci       = (CodecSetupInfo *)vi->codecSetup;
	BackendLookupState *b    = (BackendLookupState *)vd->backendState;
	VorbisInfoMapping0 *info = (VorbisInfoMapping0 *)l;
	int32 i, j;
	int32 n = vb->pcmEnd = ci->blockSizes[vb->w];

	float **pcmBundle = new float *[vi->channels];
	bool *zeroBundle  = new bool[vi->channels];

	bool *nonZero    = new bool[vi->channels];
	void **floorMemo = new void *[vi->channels];

	// Recover the spectral envelope; store it in the PCM vector for now
	for (i = 0; i < vi->channels; i++)
	{
		int32 subMap = info->chMuxList[i];
		floorMemo[i] = floorP[ci->floorType[info->floorSubMap[subMap]]]->Inverse1(obj, vb, b->flr[info->floorSubMap[subMap]]);

		if (floorMemo[i])
			nonZero[i] = true;
		else
			nonZero[i] = false;

		memset(vb->pcm[i], 0, sizeof(*vb->pcm[i]) * n / 2);
	}

	// Channel coupling can 'dirty' the nonzero listing
	for (i = 0; i < info->couplingSteps; i++)
	{
		if (nonZero[info->couplingMag[i]] || nonZero[info->couplingAng[i]])
		{
			nonZero[info->couplingMag[i]] = 1;
			nonZero[info->couplingAng[i]] = 1;
		}
	}

	// Recover the residue into our working vectors
	for (i = 0; i < info->subMaps; i++)
	{
		int32 chInBundle = 0;

		for (j = 0; j < vi->channels; j++)
		{
			if (info->chMuxList[j] == i)
			{
				if (nonZero[j])
					zeroBundle[chInBundle] = true;
				else
					zeroBundle[chInBundle] = false;

				pcmBundle[chInBundle++] = vb->pcm[j];
			}
		}

		residueP[ci->residueType[info->residueSubMap[i]]]->Inverse(obj, vb, b->residue[info->residueSubMap[i]], pcmBundle, zeroBundle, chInBundle);
	}

	// Channel coupling
	for (i = info->couplingSteps - 1; i >= 0; i--)
	{
		float *pcmM = vb->pcm[info->couplingMag[i]];
		float *pcmA = vb->pcm[info->couplingAng[i]];

		for (j = 0; j < n / 2; j++)
		{
			float mag = pcmM[j];
			float ang = pcmA[j];

			if (mag > 0)
			{
				if (ang > 0)
				{
					pcmM[j] = mag;
					pcmA[j] = mag - ang;
				}
				else
				{
					pcmA[j] = mag;
					pcmM[j] = mag + ang;
				}
			}
			else
			{
				if (ang > 0)
				{
					pcmM[j] = mag;
					pcmA[j] = mag + ang;
				}
				else
				{
					pcmA[j] = mag;
					pcmM[j] = mag - ang;
				}
			}
		}
	}

	// Compute and apply spectral envelope
	for (i = 0; i < vi->channels; i++)
	{
		float *pcm   = vb->pcm[i];
		int32 subMap = info->chMuxList[i];

		floorP[ci->floorType[info->floorSubMap[subMap]]]->Inverse2(obj, vb, b->flr[info->floorSubMap[subMap]], floorMemo[i], pcm);
	}

	// Transform the PCM data; takes PCM vector, vb; modifies PCM vector
	// only MDCT right now....
	for (i = 0; i < vi->channels; i++)
	{
		float *pcm = vb->pcm[i];
		obj->Mdct_Backward((MdctLookup *)b->transform[vb->w][0], pcm, pcm);
	}

	// Window the data
	for (i = 0; i < vi->channels; i++)
	{
		float *pcm = vb->pcm[i];

		if (nonZero[i])
			obj->_Vorbis_ApplyWindow(pcm, b->window, ci->blockSizes, vb->lW, vb->w, vb->nW);
		else
		{
			for (j = 0; j < n; j++)
				pcm[j] = 0.0f;
		}
	}

	// All done
	delete[] floorMemo;
	delete[] nonZero;
	delete[] zeroBundle;
	delete[] pcmBundle;

	return (0);
}



// Export hooks
Vorbis_Func_Mapping mapping0_exportBundle =
{
	&OggVorbis::Mapping0_Unpack,
	&OggVorbis::Mapping0_FreeInfo,
	&OggVorbis::Mapping0_Inverse
};
