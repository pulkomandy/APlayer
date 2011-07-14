/******************************************************************************/
/* Vorbis synthesis functions.                                                */
/******************************************************************************/


// Player headers
#include "Registry.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Vorbis_Synthesis()                                                         */
/******************************************************************************/
int32 OggVorbis::Vorbis_Synthesis(VorbisBlock *vb, OggPacket *op)
{
	VorbisDSPState *vd    = vb->vd;
	BackendLookupState *b = (BackendLookupState *)vd->backendState;
	VorbisInfo *vi        = vd->vi;
	CodecSetupInfo *ci    = (CodecSetupInfo *)vi->codecSetup;
	OggPackBuffer *opb    = &vb->opb;
	int32 type, mode, i;

	// First things first. Make sure decode is ready
	_Vorbis_Block_Ripcord(vb);
	OggPack_ReadInit(opb, op->packet, op->bytes);

	// Check the packet type
	if (OggPack_Read(opb, 1) != 0)
	{
		// Oops. This is not an audio data packet
		return (OV_ENOTAUDIO);
	}

	// Read our mode and pre/post windowsize
	mode = OggPack_Read(opb, b->modeBits);
	if (mode == -1)
		return (OV_EBADPACKET);

	vb->mode = mode;
	vb->w    = ci->modeParam[mode]->blockFlag;

	if (vb->w)
	{
		// This doesn't get mapped through mode selection as it's used
		// only for window selection
		vb->lW = OggPack_Read(opb, 1);
		vb->nW = OggPack_Read(opb, 1);
		if (vb->nW == -1)
			return (OV_EBADPACKET);
	}
	else
	{
		vb->lW = 0;
		vb->nW = 0;
	}

	// More setup
	vb->granulePos = op->granulePos;
	vb->sequence   = op->packetNo - 3;		// First block is third packet
	vb->eofFlag    = op->e_o_s;

	// Alloc pcm passback storage
	vb->pcmEnd = ci->blockSizes[vb->w];
	vb->pcm    = (float **)_Vorbis_Block_Alloc(vb, sizeof(*vb->pcm) * vi->channels);

	for (i = 0; i < vi->channels; i++)
		vb->pcm[i] = (float *)_Vorbis_Block_Alloc(vb, vb->pcmEnd * sizeof(*vb->pcm[i]));

	// UnpackHeader enforces range checking
	type = ci->mapType[ci->modeParam[mode]->mapping];

	return (mappingP[type]->Inverse(this, vb, ci->mapParam[ci->modeParam[mode]->mapping]));
}



/******************************************************************************/
/* Vorbis_Packet_BlockSize()                                                  */
/******************************************************************************/
int32 OggVorbis::Vorbis_Packet_BlockSize(VorbisInfo *vi, OggPacket *op)
{
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
	OggPackBuffer opb;
	int32 mode;

	OggPack_ReadInit(&opb, op->packet, op->bytes);

	// Check the packet type
	if (OggPack_Read(&opb, 1) != 0)
	{
		// Oops. This is not an audio data packet
		return (OV_ENOTAUDIO);
	}

	{
		int32 modeBits = 0;
		int32 v = ci->modes;

		while (v > 1)
		{
			modeBits++;
			v >>= 1;
		}

		// Read our mode and pre/post windowsize
		mode = OggPack_Read(&opb, modeBits);
	}

	if (mode == -1)
		return (OV_EBADPACKET);

	return (ci->blockSizes[ci->modeParam[mode]->blockFlag]);
}
