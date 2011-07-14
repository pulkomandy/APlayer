/******************************************************************************/
/* Vorbis blook functions.                                                    */
/******************************************************************************/


// Player headers
#include "Mdct.h"
#include "Registry.h"
#include "OggVorbis.h"


/******************************************************************************/
/* Vorbis_Block_Init() initialize the block structure to a known state.       */
/*                                                                            */
/* Input:  "v" is a pointer to the DSP state structure.                       */
/*         "vb" is a pointer to the structure to initialize.                  */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Vorbis_Block_Init(VorbisDSPState *v, VorbisBlock *vb)
{
	memset(vb, 0, sizeof(*vb));

	vb->vd         = v;
	vb->localAlloc = 0;
	vb->localStore = NULL;

	return (0);
}



/******************************************************************************/
/* Vorbis_Block_Clear() clear non-flat storage within.                        */
/*                                                                            */
/* Input:  "vb" is a pointer to the structure to cleanup.                     */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Vorbis_Block_Clear(VorbisBlock *vb)
{
	_Vorbis_Block_Ripcord(vb);

	_ogg_free(vb->localStore);

	memset(vb, 0, sizeof(*vb));

	return (0);
}



/******************************************************************************/
/* Vorbis_DSP_Clear() clear non-flat storage within.                          */
/*                                                                            */
/* Input:  "v" is a pointer to the structure to cleanup.                      */
/******************************************************************************/
void OggVorbis::Vorbis_DSP_Clear(VorbisDSPState *v)
{
	int32 i;

	if (v)
	{
		VorbisInfo *vi = v->vi;
		CodecSetupInfo *ci = (CodecSetupInfo *)(vi ? vi->codecSetup : NULL);
		BackendLookupState *b = (BackendLookupState *)v->backendState;

		if (b)
		{
			_ogg_free(b->window[0]);
			_ogg_free(b->window[1]);

			if (b->transform[0])
			{
				Mdct_Clear((MdctLookup *)b->transform[0][0]);
				_ogg_free(b->transform[0][0]);
				_ogg_free(b->transform[0]);
			}

			if (b->transform[1])
			{
				Mdct_Clear((MdctLookup *)b->transform[1][0]);
				_ogg_free(b->transform[1][0]);
				_ogg_free(b->transform[1]);
			}

			if (b->flr)
			{
				for (i = 0; i < ci->floors; i++)
					floorP[ci->floorType[i]]->FreeLook(b->flr[i]);

				_ogg_free(b->flr);
			}

			if (b->residue)
			{
				for (i = 0; i < ci->residues; i++)
					residueP[ci->residueType[i]]->FreeLook(b->residue[i]);

				_ogg_free(b->residue);
			}
		}

		if (v->pcm)
		{
			for (i = 0; i < vi->channels; i++)
				_ogg_free(v->pcm[i]);

			_ogg_free(v->pcm);
			_ogg_free(v->pcmRet);
		}

		if (b)
			_ogg_free(b);

		memset(v, 0, sizeof(*v));
	}
}



/******************************************************************************/
/* Vorbis_Synthesis_Init() initialize the DSP state structure to a known      */
/*      state.                                                                */
/*                                                                            */
/* Input:  "v" is a pointer to the structure to initialize.                   */
/*         "vi" is a pointer to the information structure.                    */
/*                                                                            */
/* Output: Always 0.                                                          */
/******************************************************************************/
int32 OggVorbis::Vorbis_Synthesis_Init(VorbisDSPState *v, VorbisInfo *vi)
{
	_Vds_Shared_Init(v, vi, false);

	v->pcmReturned = -1;
	v->granulePos  = -1;
	v->sequence    = -1;

	return (0);
}



/******************************************************************************/
/* Vorbis_Synthesis_BlockIn()                                                 */
/*                                                                            */
/* Unlike in analysis, the window is only partially applied for each block.   */
/* The time domain is not yet handled at the point of calling (as it relies   */
/* on the previous block).                                                    */
/*                                                                            */
/* Input:  "v" is a pointer to the DSP state structure.                       */
/*         "vb" is a pointer to the block structure.                          */
/*                                                                            */
/* Output: 0 for success, -1 for failure.                                     */
/******************************************************************************/
int32 OggVorbis::Vorbis_Synthesis_BlockIn(VorbisDSPState *v, VorbisBlock *vb)
{
	VorbisInfo *vi = v->vi;
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
	int32 i, j;

	if (!vb)
		return (OV_EINVAL);

	if ((v->pcmCurrent > v->pcmReturned) && (v->pcmReturned != -1))
		return (OV_EINVAL);

	v->lW = v->w;
	v->w  = vb->w;
	v->nW = -1;

	if ((v->sequence + 1) != vb->sequence)
		v->granulePos = -1;		// Out of sequence; lose count

	v->sequence = vb->sequence;

	if (vb->pcm)		// Not pcm to process if vorbis_synthesis_trackonly was called on block
	{
		int32 n  = ci->blockSizes[v->w] / 2;
		int32 n0 = ci->blockSizes[0] / 2;
		int32 n1 = ci->blockSizes[1] / 2;

		int32 thisCenter;
		int32 prevCenter;

		v->glueBits  += vb->glueBits;
		v->timeBits  += vb->timeBits;
		v->floorBits += vb->floorBits;
		v->resBits   += vb->resBits;

		if (v->centerW)
		{
			thisCenter = n1;
			prevCenter = 0;
		}
		else
		{
			thisCenter = 0;
			prevCenter = n1;
		}

		// v->pcm is now used like a two-stage double buffer. We don't want
		// to have to constantly shift *or* adjust memory usage. Don't
		// accept a new block until the old is shifted out
		//
		// Overlap / add PCM
		for (j = 0; j < vi->channels; j++)
		{
			// The overlap/add section
			if (v->lW)
			{
				if (v->w)
				{
					// Large / large
					float *pcm = v->pcm[j] + prevCenter;
					float *p   = vb->pcm[j];

					for (i = 0; i < n1; i++)
						pcm[i] += p[i];
				}
				else
				{
					// Large / small
					float *pcm = v->pcm[j] + prevCenter + n1 / 2 - n0 / 2;
					float *p   = vb->pcm[j];

					for (i = 0; i < n0; i++)
						pcm[i] += p[i];
				}
			}
			else
			{
				if (v->w)
				{
					// Small / large
					float *pcm = v->pcm[j] + prevCenter;
					float *p   = vb->pcm[j] + n1 / 2 - n0 / 2;

					for (i = 0; i < n0; i++)
						pcm[i] += p[i];

					for (; i < n1 / 2 + n0 / 2; i++)
						pcm[i] = p[i];
				}
				else
				{
					// Small / small
					float *pcm = v->pcm[j] + prevCenter;
					float *p   = vb->pcm[j];

					for (i = 0; i < n0; i++)
						pcm[i] += p[i];
				}
			}

			// The copy section
			{
				float *pcm = v->pcm[j] + thisCenter;
				float *p   = vb->pcm[j] + n;

				for (i = 0; i < n; i++)
					pcm[i] = p[i];
			}
		}

		if (v->centerW)
			v->centerW = 0;
		else
			v->centerW = n1;

		// Deal with initial packet state; we do this using the explicit
		// pcmReturned == -1 flag otherwise we're sensitive to first block
		// being short or long
		if (v->pcmReturned == -1)
		{
			v->pcmReturned = thisCenter;
			v->pcmCurrent  = thisCenter;
		}
		else
		{
			v->pcmReturned = prevCenter;
			v->pcmCurrent  = prevCenter + ci->blockSizes[v->lW] / 4 + ci->blockSizes[v->w] / 4;
		}
	}

	// Track the frame number... This is for convenience, but also
	// making sure our last packet doesn't end with added padding. If
	// the last packet is partial, the number of samples we'll have to
	// return will be past the vb->granulePos.
	//
	// This is not foolproof! It will be confused if we begin
	// decoding at the last page after a seek or hole. In that case,
	// we don't have a starting point to judge where the last frame
	// is. For this reason, vorbisfile will always try to make sure
	// it reads the last two marked pages in proper sequence
	if (v->granulePos == -1)
	{
		if (vb->granulePos != -1)	// Only set if we have a position to set to
			v->granulePos = vb->granulePos;
	}
	else
	{
		v->granulePos += ci->blockSizes[v->lW] / 4 + ci->blockSizes[v->w] / 4;

		if ((vb->granulePos != -1) && (v->granulePos != vb->granulePos))
		{
			if (v->granulePos > vb->granulePos)
			{
				int32 extra = v->granulePos - vb->granulePos;

				if (vb->eofFlag)
				{
					// Partial last frame. Strip the extra samples off
					v->pcmCurrent -= extra;
				}
				else
				{
					if (vb->sequence == -1)
					{
						// ^^^ Argh, this can be 1 from seeking!
						// Partial first frame. Discard extra leading samples
						v->pcmReturned += extra;
						if (v->pcmReturned > v->pcmCurrent)
							v->pcmReturned = v->pcmCurrent;
					}
					else
					{
						// Shouldn't happen *unless* the bitstream is out of
						// spec. Either way, believe the bitstream
						;
					}
				}
			}
			else
			{
				// Shouldn't happen *unless* the bitstream is out of
				// spec. Either way, believe the bitstream
				;
			}

			v->granulePos = vb->granulePos;
		}
	}

	// Update, cleanup
	if (vb->eofFlag)
		v->eofFlag = true;

	return (0);
}



/******************************************************************************/
/* Vorbis_Synthesis_PCMOut() returns a block of PCM data in float format.     */
/*                                                                            */
/* Input:  "v" is a pointer to the DSP state structure.                       */
/*         "pcm" is a pointer to store the pcm data pointers.                 */
/*                                                                            */
/* Output: Number of samples returned.                                        */
/******************************************************************************/
int32 OggVorbis::Vorbis_Synthesis_PCMOut(VorbisDSPState *v, float ***pcm)
{
	VorbisInfo *vi = v->vi;

	if ((v->pcmReturned > -1) && (v->pcmReturned < v->pcmCurrent))
	{
		if (pcm)
		{
			int32 i;

			for (i = 0; i < vi->channels; i++)
				v->pcmRet[i] = v->pcm[i] + v->pcmReturned;

			*pcm = v->pcmRet;
		}

		return (v->pcmCurrent - v->pcmReturned);
	}

	return (0);
}



/******************************************************************************/
/* Vorbis_Synthesis_Read() tells how many bytes we have consumed.             */
/*                                                                            */
/* Input:  "v" is a pointer to the DSP state structure.                       */
/*         "n" is the number of bytes consumed.                               */
/*                                                                            */
/* Output: 0 for success, else an error code.                                 */
/******************************************************************************/
int32 OggVorbis::Vorbis_Synthesis_Read(VorbisDSPState *v, int32 n)
{
	if (n && ((v->pcmReturned + n) > v->pcmCurrent))
		return (OV_EINVAL);

	v->pcmReturned += n;

	return (0);
}



/******************************************************************************/
/* _Vorbis_Block_Ripcord()                                                    */
/******************************************************************************/
void OggVorbis::_Vorbis_Block_Ripcord(VorbisBlock *vb)
{
	// Reap the chain
	AllocChain *reap = vb->reap;

	while (reap)
	{
		AllocChain *next = reap->next;
		_ogg_free(reap->ptr);
		_ogg_free(reap);

		reap = next;
	}

	// Consolidate storage
	if (vb->totalUse)
	{
		vb->localStore  = _ogg_realloc(vb->localStore, vb->totalUse + vb->localAlloc);
		vb->localAlloc += vb->totalUse;
		vb->totalUse    = 0;
	}

	// Pull the ripcord
	vb->localTop = 0;
	vb->reap     = NULL;
}



/******************************************************************************/
/* _Vorbis_Block_Alloc()                                                      */
/******************************************************************************/
#define WORD_ALIGN		8

void *OggVorbis::_Vorbis_Block_Alloc(VorbisBlock *vb, int32 bytes)
{
	bytes = (bytes + (WORD_ALIGN - 1)) & ~(WORD_ALIGN - 1);

	if ((bytes + vb->localTop) > vb->localAlloc)
	{
		// Can't just _ogg_realloc... there are outstanding pointers
		if (vb->localStore)
		{
			AllocChain *link = (AllocChain *)_ogg_malloc(sizeof(*link));

			vb->totalUse += vb->localTop;
			link->next    = vb->reap;
			link->ptr     = vb->localStore;
			vb->reap      = link;
		}

		// Highly conservative
		vb->localAlloc = bytes;
		vb->localStore = _ogg_malloc(vb->localAlloc);
		vb->localTop   = 0;
	}

	{
		void *ret = (void *)(((int8 *)vb->localStore) + vb->localTop);
		vb->localTop += bytes;

		return (ret);
	}
}



/******************************************************************************/
/* _Vds_Shared_Init()                                                         */
/******************************************************************************/
int32 OggVorbis::_Vds_Shared_Init(VorbisDSPState *v, VorbisInfo *vi, bool encp)
{
	int32 i;
	CodecSetupInfo *ci = (CodecSetupInfo *)vi->codecSetup;
	BackendLookupState *b = NULL;

	memset(v, 0, sizeof(*v));
	v->backendState = _ogg_calloc(1, sizeof(*b));
	b = (BackendLookupState *)v->backendState;

	v->vi = vi;
	b->modeBits = ilog2(ci->modes);

	b->transform[0] = (VorbisLookTransform **)_ogg_calloc(VI_TRANSFORMB, sizeof(*b->transform[0]));
	b->transform[1] = (VorbisLookTransform **)_ogg_calloc(VI_TRANSFORMB, sizeof(*b->transform[1]));

	// MDCT is transform 0
	b->transform[0][0] = _ogg_calloc(1, sizeof(MdctLookup));
	b->transform[1][0] = _ogg_calloc(1, sizeof(MdctLookup));
	Mdct_Init((MdctLookup *)b->transform[0][0], ci->blockSizes[0]);
	Mdct_Init((MdctLookup *)b->transform[1][0], ci->blockSizes[1]);

	// Vorbis I uses only window type 0
	b->window[0] = _Vorbis_Window(0, ci->blockSizes[0] / 2);
	b->window[1] = _Vorbis_Window(0, ci->blockSizes[1] / 2);

	if (encp)
	{
		// Encode only initialization
		ASSERT(false);
	}
	else
	{
		// Finish the codebooks
		if (!ci->fullBooks)
		{
			ci->fullBooks = (Codebook *)_ogg_calloc(ci->books, sizeof(*ci->fullBooks));

			for (i = 0; i < ci->books; i++)
			{
				Vorbis_Book_InitDecode(ci->fullBooks + i, ci->bookParam[i]);

				// Decode codebooks are now standalone after init
				Vorbis_Staticbook_Destroy(ci->bookParam[i]);
				ci->bookParam[i] = NULL;
			}
		}
	}

	// Initialize the storage vector. blockSize[1] is small for encode,
	// but the correct size for decode
	v->pcmStorage = ci->blockSizes[1];
	v->pcm        = (float **)_ogg_malloc(vi->channels * sizeof(*v->pcm));
	v->pcmRet     = (float **)_ogg_malloc(vi->channels * sizeof(*v->pcmRet));

	{
		int32 i;

		for (i = 0; i < vi->channels; i++)
			v->pcm[i] = (float *)_ogg_calloc(v->pcmStorage, sizeof(*v->pcm[i]));
	}

	// All 1 (large block) or 0 (small block)
	// explicitly set for the sake of clarity
	v->lW = 0;		// Previous window size
	v->w  = 0;		// Current window size

	// All vector indexes
	v->centerW = ci->blockSizes[1] / 2;

	v->pcmCurrent = v->centerW;

	// Initialize all the backend lookups
	b->flr = (VorbisLookFloor **)_ogg_calloc(ci->floors, sizeof(*b->flr));
	b->residue = (VorbisLookResidue **)_ogg_calloc(ci->residues, sizeof(*b->residue));

	for (i = 0; i < ci->floors; i++)
		b->flr[i] = floorP[ci->floorType[i]]->Look(v, ci->floorParam[i]);

	for (i = 0; i < ci->residues; i++)
		b->residue[i] = residueP[ci->residueType[i]]->Look(v, ci->residueParam[i]);

	return (0);
}
