/******************************************************************************/
/* MikModConverter FAR class.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PFile.h"

// Agent headers
#include "MikConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Intern structures                                                          */
/******************************************************************************/
typedef struct FARHEADER1
{
	uint8	id[4];					// File magic
	char	songName[41];			// Song name
	char	blah[3];				// 13, 10, 26
	uint16	headerLen;				// Remaining length of header in bytes
	uint8	version;
	uint8	onOff[16];
	uint8	edit1[9];
	uint8	speed;
	uint8	panning[16];
	uint8	edit2[4];
	uint16	stLen;
} FARHEADER1;



typedef struct FARHEADER2
{
	uint8	orders[256];
	uint8	numPat;
	uint8	sngLen;
	uint8	loopTo;
	uint16	patSiz[256];
} FARHEADER2;



typedef struct FARSAMPLE
{
	char	sampleName[33];
	uint32	length;
	uint8	fineTune;
	uint8	volume;
	uint32	repPos;
	uint32	repEnd;
	uint8	type;
	uint8	loop;
} FARSAMPLE;



typedef struct FARNOTE
{
	uint8	note;
	uint8	ins;
	uint8	vol;
	uint8	eff;
} FARNOTE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a FAR module.        */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikFAR::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 buf[4];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(FARHEADER1))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(buf, 4);

	if ((buf[0] == 'F') && (buf[1] == 'A') && (buf[2] == 'R') && (buf[3] == 0xfe))
	{
		file->Seek(40, PFile::pSeekCurrent);
		file->Read(buf, 3);

		if ((buf[0] == 13) && (buf[1] == 10) && (buf[2] == 26))
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from FAR to UniMod structure. */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikFAR::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, tracks = 0;
	SAMPLE *q;
	FARSAMPLE s;
	FARNOTE *cRow;
	uint8 smap[8];
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		pat = NULL;
		mh2 = NULL;
		mh1 = NULL;

		// Start to allocate some buffers we need
		if ((mh1 = new FARHEADER1) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((mh2 = new FARHEADER2) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((pat = new FARNOTE[256 * 16 * 4]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Try to read the header (first part)
		file->Read(mh1->id, 4);
		file->ReadString(mh1->songName, 40);
		file->Read(mh1->blah, 3);

		mh1->headerLen = file->Read_L_UINT16();
		mh1->version   = file->Read_UINT8();

		file->Read(mh1->onOff, 16);
		file->Read(mh1->edit1, 9);

		mh1->speed = file->Read_UINT8();

		file->Read(mh1->panning, 16);
		file->Read(mh1->edit2, 4);

		mh1->stLen = file->Read_L_UINT16();

		// Init modfile data
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_FAR);
		convInfo->modKind.LoadString(res, IDS_MIKC_NAME_FAR);

		of.songName.SetString(mh1->songName, &charSet850);
		of.numChn    = 16;
		of.initSpeed = mh1->speed;
		of.initTempo = 80;
		of.repPos    = 0;
		of.flags    |= UF_PANNING;

		for (t = 0; t < 16; t++)
			of.panning[t] = mh1->panning[t] << 4;

		// Read songtext into comment field
		if (mh1->stLen)
			ReadLinedComment(file, mh1->stLen, 66, false);

		// Try to read module header (second part)
		file->Read(mh2->orders, 256);

		mh2->numPat = file->Read_UINT8();
		mh2->sngLen = file->Read_UINT8();
		mh2->loopTo = file->Read_UINT8();

		file->ReadArray_L_UINT16s(mh2->patSiz, 256);

		of.numPos = mh2->sngLen;

		// TN: Added EOF check
		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		if (!(AllocPositions(of.numPos)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = 0; t < of.numPos; t++)
		{
			if (mh2->orders[t] == 0xff)
				break;

			of.positions[t] = mh2->orders[t];
		}

		// Count number of patterns stored in file
		of.numPat = 0;
		for (t = 0; t < 256; t++)
		{
			if (mh2->patSiz[t])
				if ((t + 1) > of.numPat)
					of.numPat = t + 1;
		}

		of.numTrk = of.numPat * of.numChn;

		// Seek across eventual new data
		file->Seek(mh1->headerLen - (869 + mh1->stLen), PFile::pSeekCurrent);

		// Alloc track and pattern structures
		if (!(AllocTracks()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if (!(AllocPatterns()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = 0; t < of.numPat; t++)
		{
			uint8 rows = 0, tempo;

			memset(pat, 0, 256 * 16 * 4 * sizeof(FARNOTE));

			if (mh2->patSiz[t])
			{
				rows  = file->Read_UINT8();
				tempo = file->Read_UINT8();

				cRow = pat;

				// File often allocates 64 rows even if there are less in pattern
				if ((mh2->patSiz[t] < 2 + (rows * 16 * 4)))
				{
					ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
					throw PUserException();
				}

				for (u = (mh2->patSiz[t] - 2) / 4; u; u--, cRow++)
				{
					cRow->note = file->Read_UINT8();
					cRow->ins  = file->Read_UINT8();
					cRow->vol  = file->Read_UINT8();
					cRow->eff  = file->Read_UINT8();
				}

				if (file->IsEOF())
				{
					ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
					throw PUserException();
				}

				cRow           = pat;
				of.pattRows[t] = rows;

				for (u = 16; u; u--, cRow++)
				{
					if (!(of.tracks[tracks++] = ConvertTrack(cRow, rows)))
					{
						ShowError(IDS_MIKC_ERR_INITIALIZE);
						throw PUserException();
					}
				}
			}
			else
			{
				// Remember to skip the tracks, else all tracks after
				// an empty one will have the wrong positions
				tracks += 16;
			}
		}

		// Read sample map
		file->Read(smap, 8);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Count number of samples used
		of.numIns = 0;
		for (t = 0; t < 64; t++)
		{
			if (smap[t >> 3] & (1 << (t & 7)))
				of.numIns = t + 1;
		}

		of.numSmp = of.numIns;

		// Alloc sample structs
		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		q = of.samples;
		for (t = 0; t < of.numSmp; t++)
		{
			q->speed = 8363;
			q->flags = SF_SIGNED;

			if (smap[t >> 3] & (1 << (t & 7)))
			{
				file->ReadString(s.sampleName, 32);
				s.length   = file->Read_L_UINT32();
				s.fineTune = file->Read_UINT8();
				s.volume   = file->Read_UINT8();
				s.repPos   = file->Read_L_UINT32();
				s.repEnd   = file->Read_L_UINT32();
				s.type     = file->Read_UINT8();
				s.loop     = file->Read_UINT8();

				// TN: Added EOF check
				if (file->IsEOF())
				{
					ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
					throw PUserException();
				}

				q->sampleName.SetString(s.sampleName, &charSet850);
				q->length     = s.length;
				q->loopStart  = s.repPos;
				q->loopEnd    = s.repEnd;
				q->volume     = s.volume << 2;

				if (s.type & 1)
					q->flags |= SF_16BITS;

				if (s.loop & 8)
					q->flags |= SF_LOOP;

				q->seekPos = file->GetPosition();
				file->Seek(q->length, PFile::pSeekCurrent);
			}
			else
				q->sampleName.MakeEmpty();

			q++;
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete mh1;
	mh1 = NULL;

	delete mh2;
	mh2 = NULL;

	delete[] pat;
	pat = NULL;

	return (retVal);
}



/******************************************************************************/
/* ConvertTrack() convert a track to uni format.                              */
/*                                                                            */
/* Input:  "n" is a pointer to the track.                                     */
/*         "rows" is the number of rows in the track.                         */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikFAR::ConvertTrack(FARNOTE *n, int32 rows)
{
	int32 t, vibDepth = 1;

	UniReset();

	for (t = 0; t < rows; t++)
	{
		if (n->note)
		{
			UniInstrument(n->ins);
			UniNote(n->note + 3 * OCTAVE - 1);
		}

		if (n->vol & 0xf)
			UniPTEffect(0xc, (n->vol & 0xf) << 2, of.flags);

		if (n->eff)
		{
			switch (n->eff >> 4)
			{
				// Pitch adjust up
				case 0x1:
					break;

				// Pitch adjust down
				case 0x2:
					break;

				// Portamento
				case 0x3:
					UniPTEffect(0x3, (n->eff & 0xf) << 4, of.flags);
					break;

				// Retrigger
				case 0x4:
					UniPTEffect(0xe, 0x90 | (n->eff & 0xf), of.flags);
					break;

				// Set vibrato depth
				case 0x5:
					vibDepth = n->eff & 0xf;
					break;

				// Vibrato
				case 0x6:
					UniPTEffect(0x4, ((n->eff & 0xf) << 4) | vibDepth, of.flags);
					break;

				// Volume slide up
				case 0x7:
					UniPTEffect(0xa, (n->eff & 0xf) << 4, of.flags);
					break;

				// Volume slide down
				case 0x8:
					UniPTEffect(0xa, n->eff & 0xf, of.flags);
					break;

				// Vibrato Sustain
				case 0x9:
					break;

				// Port to vol
				case 0xa:
					break;

				// Set panning
				case 0xb:
					UniPTEffect(0xe, 0x80 | (n->eff & 0xf), of.flags);
					break;

				// Note offset
				case 0xc:
					break;

				// Fine tempo down
				case 0xd:
					break;

				// Fine tempo up
				case 0xe:
					break;

				// Set speed
				case 0xf:
					UniPTEffect(0xf, n->eff & 0xf, of.flags);
					break;
			}
		}

		UniNewLine();
		n += 16;
	}

	return (UniDup());
}
