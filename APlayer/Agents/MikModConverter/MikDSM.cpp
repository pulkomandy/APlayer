/******************************************************************************/
/* MikModConverter DSM class.                                                 */
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
#define DSM_MAXCHAN		16
#define DSM_MAXORDERS	128
#define DSM_SURROUND	0xa4



typedef struct DSMSONG
{
	char	songName[29];
	uint16	version;
	uint16	flags;
	uint32	reserved2;
	uint16	numOrd;
	uint16	numSmp;
	uint16	numPat;
	uint16	numTrk;
	uint8	globalVol;
	uint8	masterVol;
	uint8	speed;
	uint8	bpm;
	uint8	panPos[DSM_MAXCHAN];
	uint8	orders[DSM_MAXORDERS];
} DSMSONG;



typedef struct DSMINST
{
	char	fileName[14];
	uint16	flags;
	uint8	volume;
	uint32	length;
	uint32	loopStart;
	uint32	loopEnd;
	uint32	reserved1;
	uint16	c2Spd;
	uint16	period;
	char	sampleName[29];
} DSMINST;



typedef struct DSMNOTE
{
	uint8	note;
	uint8	ins;
	uint8	vol;
	uint8	cmd;
	uint8	inf;
} DSMNOTE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's an DSM module.       */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikDSM::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 buf[12];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < 12)
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(buf, 12);

	if ((buf[0] == 'R') && (buf[1] == 'I') && (buf[2] == 'F') && (buf[3] == 'F') &&
		(buf[8] == 'D') && (buf[9] == 'S') && (buf[10] == 'M') && (buf[11] == 'F'))
	{
		return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from DSM to UniMod structure. */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikDSM::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t;
	DSMINST s;
	SAMPLE *q;
	int32 curSmp = 0, curPat = 0, track = 0;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		dsmBuf = NULL;
		mh     = NULL;

		// Start to allocate some buffers we need
		if ((mh = new DSMSONG) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((dsmBuf = new DSMNOTE[DSM_MAXCHAN * 64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(mh, 0, sizeof(DSMSONG));

		blockLp = 0;
		blockLn = 12;

		GetBlockHeader(file);

		if (blockId != 'SONG')
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		file->Read(mh->songName, 28);

		mh->version   = file->Read_L_UINT16();
		mh->flags     = file->Read_L_UINT16();
		mh->reserved2 = file->Read_L_UINT32();
		mh->numOrd    = file->Read_L_UINT16();
		mh->numSmp    = file->Read_L_UINT16();
		mh->numPat    = file->Read_L_UINT16();
		mh->numTrk    = file->Read_L_UINT16();
		mh->globalVol = file->Read_UINT8();
		mh->masterVol = file->Read_UINT8();
		mh->speed     = file->Read_UINT8();
		mh->bpm       = file->Read_UINT8();

		file->Read(mh->panPos, DSM_MAXCHAN);
		file->Read(mh->orders, DSM_MAXORDERS);

		// TN: Added extra EOF check
		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_DSM);
		convInfo->modKind.LoadString(res, IDS_MIKC_NAME_DSM);

		of.initSpeed = mh->speed;
		of.initTempo = mh->bpm;
		of.numChn    = mh->numTrk;
		of.numPat    = mh->numPat;
		of.numTrk    = of.numChn * of.numPat;
		of.songName.SetString(mh->songName, &charSet850);
		of.repPos    = 0;
		of.flags    |= UF_PANNING;

		// Whenever possible, we should try to determine the original format.
		// Here we assume it was S3M-style wrt bpm limit...
		of.bpmLimit  = 32;

		for (t = 0; t < DSM_MAXCHAN; t++)
			of.panning[t] = mh->panPos[t] == DSM_SURROUND ? PAN_SURROUND : mh->panPos[t] < 0x80 ? (mh->panPos[t] << 1) : 255;

		if (!(AllocPositions(mh->numOrd)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		of.numPos = 0;
		for (t = 0; t < mh->numOrd; t++)
		{
			int32 order = mh->orders[t];
			if (order == 255)
				order = LAST_PATTERN;

			of.positions[of.numPos] = order;

			if (mh->orders[t] < 254)
				of.numPos++;
		}

		of.numIns = of.numSmp = mh->numSmp;

		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

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

		while ((curSmp < of.numIns) || (curPat < of.numPat))
		{
			GetBlockHeader(file);

			if ((blockId == 'INST') && (curSmp < of.numIns))
			{
				q = &of.samples[curSmp];

				// Try to read sample info
				file->Read(s.fileName, 13);

				s.flags     = file->Read_L_UINT16();
				s.volume    = file->Read_UINT8();
				s.length    = file->Read_L_UINT32();
				s.loopStart = file->Read_L_UINT32();
				s.loopEnd   = file->Read_L_UINT32();
				s.reserved1 = file->Read_L_UINT32();
				s.c2Spd     = file->Read_L_UINT16();
				s.period    = file->Read_L_UINT16();

				file->Read(s.sampleName, 28);

				q->sampleName.SetString(s.sampleName, &charSet850);
				q->seekPos    = file->GetPosition();
				q->speed      = s.c2Spd;
				q->length     = s.length;
				q->loopStart  = s.loopStart;
				q->loopEnd    = s.loopEnd;
				q->volume     = s.volume;

				if (s.flags & 1)
					q->flags |= SF_LOOP;

				if (s.flags & 2)
					q->flags |= SF_SIGNED;

				// (s.flags & 4) means packed samples,
				// but did they really exist in dsm?
				curSmp++;
			}
			else
			{
				if ((blockId == 'PATT') && (curPat < of.numPat))
				{
					ReadPattern(file);

					for (t = 0; t < of.numChn; t++)
					{
						if (!(of.tracks[track++] = ConvertTrack(&dsmBuf[t * 64])))
						{
							ShowError(IDS_MIKC_ERR_INITIALIZE);
							throw PUserException();
						}
					}

					curPat++;
				}
			}
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete[] dsmBuf;
	dsmBuf = NULL;

	delete mh;
	mh = NULL;

	return (retVal);
}



/******************************************************************************/
/* GetBlockHeader() will read the next block header.                          */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikDSM::GetBlockHeader(PFile *file)
{
	// Make sure we're at the right position for reading the
	// next riff block, no matter how many bytes read
	file->Seek(blockLp + blockLn, PFile::pSeekBegin);

	while (true)
	{
		blockId = file->Read_B_UINT32();
		blockLn = file->Read_L_UINT32();

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		if ((blockId != 'SONG') && (blockId != 'INST') && (blockId != 'PATT'))
		{
			// Skip unknown block type
			file->Seek(blockLn, PFile::pSeekCurrent);
		}
		else
			break;
	}

	blockLp = file->GetPosition();
}



/******************************************************************************/
/* ReadPattern() will read the pattern data.                                  */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikDSM::ReadPattern(PFile *file)
{
	int32 flag, row = 0;
	int16 length;
	DSMNOTE *n;

	// Clear pattern data
	memset(dsmBuf, 255, DSM_MAXCHAN * 64 * sizeof(DSMNOTE));
	length = file->Read_L_UINT16();

	while (row < 64)
	{
		flag = file->Read_UINT8();

		if ((file->IsEOF()) || (--length < 0))
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (flag)
		{
			n = &dsmBuf[((flag & 0xf) * 64) + row];

			if (flag & 0x80)
				n->note = file->Read_UINT8();

			if (flag & 0x40)
				n->ins = file->Read_UINT8();

			if (flag & 0x20)
				n->vol = file->Read_UINT8();

			if (flag & 0x10)
			{
				n->cmd = file->Read_UINT8();
				n->inf = file->Read_UINT8();
			}
		}
		else
			row++;
	}
}



/******************************************************************************/
/* ConvertTrack() convert a track to uni format.                              */
/*                                                                            */
/* Input:  "tr" is a pointer to the track.                                     */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikDSM::ConvertTrack(DSMNOTE *tr)
{
	int32 t;
	uint8 note, ins, vol, cmd, inf;

	UniReset();

	for (t = 0; t < 64; t++)
	{
		note = tr[t].note;
		ins  = tr[t].ins;
		vol  = tr[t].vol;
		cmd  = tr[t].cmd;
		inf  = tr[t].inf;

		if ((ins != 0) && (ins != 255))
			UniInstrument(ins - 1);

		if (note != 255)
			UniNote(note - 1);		// Normal note

		if (vol < 65)
			UniPTEffect(0xc, vol, of.flags);

		if (cmd != 255)
		{
			if (cmd == 0x8)
			{
				if (inf == DSM_SURROUND)
					UniEffect(UNI_ITEFFECTS0, 0x91);
				else
				{
					if (inf <= 0x80)
					{
						inf = (inf < 0x80) ? inf << 1 : 255;
						UniPTEffect(cmd, inf, of.flags);
					}
				}
			}
			else
			{
				if (cmd == 0xb)
				{
					if (inf <= 0x7f)
						UniPTEffect(cmd, inf, of.flags);
				}
				else
				{
					// Convert pattern jump from dec to hex
					if (cmd == 0xd)
						inf = (((inf & 0xf0) >> 4) * 10) + (inf & 0xf);

					UniPTEffect(cmd, inf, of.flags);
				}
			}
		}

		UniNewLine();
	}

	return (UniDup());
}
