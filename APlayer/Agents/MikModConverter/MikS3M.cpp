/******************************************************************************/
/* MikModConverter S3M class.                                                 */
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
typedef struct S3MHEADER
{
	char	songName[29];
	uint8	t1a;
	uint8	type;
	uint8	unused1[2];
	uint16	ordNum;
	uint16	insNum;
	uint16	patNum;
	uint16	flags;
	uint16	tracker;
	uint16	fileFormat;
	char	scrm[4];
	uint8	masterVol;
	uint8	initSpeed;
	uint8	initTempo;
	uint8	masterMult;
	uint8	ultraClick;
	uint8	panTable;
	uint8	unused2[8];
	uint16	special;
	uint8	channels[32];
} S3MHEADER;



// Sample information
typedef struct S3MSAMPLE
{
	uint8	type;
	char	fileName[13];
	uint8	memSegH;
	uint16	memSegL;
	uint32	length;
	uint32	loopBeg;
	uint32	loopEnd;
	uint8	volume;
	uint8	dsk;
	uint8	pack;
	uint8	flags;
	uint32	c2Spd;
	uint8	unused[12];
	char	sampName[29];
	char	scrs[4];
} S3MSAMPLE;



typedef struct S3MNOTE
{
	uint8	note;
	uint8	ins;
	uint8	vol;
	uint8	cmd;
	uint8	inf;
} S3MNOTE;



#define NUMTRACKERS 4

static int32 S3M_Version[] =
{
	IDS_MIKC_NAME_S3M,
	IDS_MIKC_NAME_S3M_IMAGO,
	IDS_MIKC_NAME_S3M_IT,
	IDS_MIKC_NAME_S3M_UNKNOWN,
	IDS_MIKC_NAME_S3M_IT_214P3,
	IDS_MIKC_NAME_S3M_IT_214P4
};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a ScreamTracker 3.x  */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikS3M::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 buf[4];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(S3MHEADER))
		return (false);

	// Now check the signature
	file->Seek(44, PFile::pSeekBegin);
	file->Read(buf, 4);

	if ((buf[0] == 'S') && (buf[1] == 'C') && (buf[2] == 'R') && (buf[3] == 'M'))
		return (true);

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from ScreamTracker 3.x to     */
/*         UniMod structure.                                                  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikS3M::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, track = 0;
	SAMPLE *q;
	uint8 pan[32];
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		s3mBuf    = NULL;
		mh        = NULL;
		posLookup = NULL;
		paraPtr   = NULL;

		// Start to allocate some buffers we need
		if ((s3mBuf = new S3MNOTE[32 * 64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((mh = new S3MHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((posLookup = new uint8[256]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(s3mBuf, 0, sizeof(S3MNOTE) * 32 * 64);
		memset(mh, 0, sizeof(S3MHEADER));
		memset(posLookup, -1, sizeof(uint8) * 256);

		// Try to read module header
		file->ReadString(mh->songName, 28);
		mh->t1a        = file->Read_UINT8();
		mh->type       = file->Read_UINT8();
		file->Read(mh->unused1, 2);

		mh->ordNum     = file->Read_L_UINT16();
		mh->insNum     = file->Read_L_UINT16();
		mh->patNum     = file->Read_L_UINT16();
		mh->flags      = file->Read_L_UINT16();
		mh->tracker    = file->Read_L_UINT16();
		mh->fileFormat = file->Read_L_UINT16();
		file->Read(mh->scrm, 4);

		mh->masterVol  = file->Read_UINT8();
		mh->initSpeed  = file->Read_UINT8();
		mh->initTempo  = file->Read_UINT8();
		mh->masterMult = file->Read_UINT8();
		mh->ultraClick = file->Read_UINT8();
		mh->panTable   = file->Read_UINT8();
		file->Read(mh->unused2, 8);

		mh->special    = file->Read_L_UINT16();
		file->Read(mh->channels, 32);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Then we can decide the module type
		tracker = mh->tracker >> 12;

		if ((!tracker) || (tracker >= NUMTRACKERS))
			tracker = NUMTRACKERS - 1;	// Unknown tracker
		else
		{
			if (mh->tracker >= 0x3217)
				tracker = NUMTRACKERS + 1;	// IT 2.14p4
			else
			{
				if (mh->tracker >= 0x3216)
					tracker = NUMTRACKERS;	// IT 2.14p3
				else
					tracker--;
			}
		}

		if (tracker < NUMTRACKERS)
			convInfo->modKind.Format(res, S3M_Version[tracker], (mh->tracker >> 8) & 0xf, ((mh->tracker >> 4) & 0xf) * 10 + (mh->tracker & 0xf));
		else
			convInfo->modKind.LoadString(res, S3M_Version[tracker]);

		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_S3M);

		// Set module variables
		of.songName.SetString(mh->songName, &charSet850);
		of.numPat     = mh->patNum;
		of.repPos     = 0;
		of.numIns     = of.numSmp = mh->insNum;
		of.initSpeed  = mh->initSpeed;
		of.initTempo  = mh->initTempo;
		of.initVolume = mh->masterVol << 1;
		of.flags     |= UF_ARPMEM | UF_PANNING;

		if ((mh->tracker == 0x1300) || (mh->flags & 64))
			of.flags |= UF_S3MSLIDES;

		of.bpmLimit = 32;

		// Read the order data
		if (!(AllocPositions(mh->ordNum)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((origPositions = new uint16[mh->ordNum]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		memset(origPositions, 0, mh->ordNum * sizeof(uint16));

		for (t = 0; t < mh->ordNum; t++)
		{
			origPositions[t] = file->Read_UINT8();
			if ((origPositions[t] >= mh->patNum) && (origPositions[t] < 254))
				origPositions[t] = 255;
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		posLookupCnt = mh->ordNum;
		S3MIT_CreateOrders(curious);

		if ((paraPtr = new uint16[of.numIns + of.numPat]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the instrument + pattern parapointers
		file->ReadArray_L_UINT16s(paraPtr, of.numIns + of.numPat);

		if (mh->panTable == 252)
		{
			// Read the panning table (ST 3.2 addition. See below for further
			// portions of channel panning [past reampper]).
			file->Read(pan, 32);
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Load samples
		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		q = of.samples;

		for (t = 0; t < of.numIns; t++)
		{
			S3MSAMPLE s;

			// Seek to instrument position
			file->Seek(paraPtr[t] << 4, PFile::pSeekBegin);

			// And load the sample info
			s.type    = file->Read_UINT8();
			file->ReadString(s.fileName, 12);

			s.memSegH = file->Read_UINT8();
			s.memSegL = file->Read_L_UINT16();
			s.length  = file->Read_L_UINT32();
			s.loopBeg = file->Read_L_UINT32();
			s.loopEnd = file->Read_L_UINT32();
			s.volume  = file->Read_UINT8();
			s.dsk     = file->Read_UINT8();
			s.pack    = file->Read_UINT8();
			s.flags   = file->Read_UINT8();
			s.c2Spd   = file->Read_L_UINT32();
			file->Read(s.unused, 12);
			file->ReadString(s.sampName, 28);
			file->Read(s.scrs, 4);

			// ScreamTracker imposes a 64000 bytes (not 64K!) limit
			if (s.length > 64000)
				s.length = 64000;

			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			q->sampleName.SetString(s.sampName, &charSet850);
			q->speed      = s.c2Spd;
			q->length     = s.length;
			q->loopStart  = s.loopBeg;
			q->loopEnd    = s.loopEnd;
			q->volume     = s.volume;
			q->seekPos    = ((((long)s.memSegH) << 16) | s.memSegL) << 4;

			if (s.flags & 1)
				q->flags |= SF_LOOP;

			if (s.flags & 4)
				q->flags |= SF_16BITS;

			if (mh->fileFormat == 1)
				q->flags |= SF_SIGNED;

			// Don't load sample if it doesn't have the SCRS tag
			if ((s.scrs[0] != 'S') || (s.scrs[1] != 'C') || (s.scrs[2] != 'R') || (s.scrs[3] != 'S'))
				q->length = 0;

			q++;
		}

		// Determine the number of channels actually used.
		of.numChn = 0;
		memset(remap, -1, 32 * sizeof(uint8));

		for (t = 0; t < of.numPat; t++)
		{
			// Seek to pattern position (+ 2 skip pattern length)
			file->Seek((paraPtr[of.numIns + t] << 4) + 2, PFile::pSeekBegin);
			GetNumChannels(file);
		}

		// Build the remap array
		for (t = 0; t < 32; t++)
		{
			if (!remap[t])
				remap[t] = of.numChn++;
		}

		// Set panning positions after building remap chart!
		for (t = 0; t < 32; t++)
		{
			if ((mh->channels[t] < 32) && (remap[t] != -1))
			{
				if (mh->channels[t] < 8)
					of.panning[remap[t]] = 0x30;
				else
					of.panning[remap[t]] = 0xc0;
			}
		}

		if (mh->panTable == 252)
		{
			// Set panning positions according to panning table (new for st3.2)
			for (t = 0; t < 32; t++)
			{
				if ((pan[t] & 0x20) && (mh->channels[t] < 32) && (remap[t] != -1))
					of.panning[remap[t]] = (pan[t] & 0xf) << 4;
			}
		}

		// Load the pattern info
		of.numTrk = of.numPat * of.numChn;

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
			// Seek to pattern position (+ 2 skip pattern length)
			file->Seek((paraPtr[of.numIns + t] << 4) + 2, PFile::pSeekBegin);
			ReadPattern(file);

			for (u = 0; u < of.numChn; u++)
			{
				if (!(of.tracks[track++] = ConvertTrack(&s3mBuf[u * 64])))
				{
					ShowError(IDS_MIKC_ERR_INITIALIZE);
					throw PUserException();
				}
			}
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete[] s3mBuf;
	s3mBuf = NULL;

	delete[] paraPtr;
	paraPtr = NULL;

	delete[] posLookup;
	posLookup = NULL;

	delete mh;
	mh = NULL;

	delete[] origPositions;
	origPositions = NULL;

	return (retVal);
}



/******************************************************************************/
/* GetNumChannels() returns the number of channels used.                      */
/*                                                                            */
/* Because so many s3m files have 16 channels as the set number used, but     */
/* really only use far less (usually 8 to 12 still), I had to make this       */
/* function, which determines the number of channels that are actually USED   */
/* by a pattern.                                                              */
/*                                                                            */
/* For every channel that's used, it sets the appropriate array entry of the  */
/* global variable 'remap'.                                                   */
/*                                                                            */
/* NOTE: You must first seek to the file location of the pattern before       */
/*       calling this procedure.                                              */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikS3M::GetNumChannels(PFile *file)
{
	int32 row = 0, flag, ch;

	while (row < 64)
	{
		flag = file->Read_UINT8();

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (flag)
		{
			ch = flag & 31;

			if (mh->channels[ch] < 32)
				remap[ch] = 0;

			if (flag & 32)
			{
				file->Read_UINT8();
				file->Read_UINT8();
			}

			if (flag & 64)
				file->Read_UINT8();

			if (flag & 128)
			{
				file->Read_UINT8();
				file->Read_UINT8();
			}
		}
		else
			row++;
	}
}



/******************************************************************************/
/* ReadPattern() reads a single pattern.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikS3M::ReadPattern(PFile *file)
{
	int32 row = 0, flag, ch;
	S3MNOTE *n, dummy;

	// Clear pattern data
	memset(s3mBuf, 255, 32 * 64 * sizeof(S3MNOTE));

	while (row < 64)
	{
		flag = file->Read_UINT8();

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (flag)
		{
			ch = remap[flag & 31];

			if (ch != -1)
				n = &s3mBuf[(64U * ch) + row];
			else
				n = &dummy;

			if (flag & 32)
			{
				n->note = file->Read_UINT8();
				n->ins  = file->Read_UINT8();
			}

			if (flag & 64)
			{
				n->vol = file->Read_UINT8();
				if (n->vol > 64)
					n->vol = 64;
			}

			if (flag & 128)
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
/* Input:  "tr" is a pointer to the track.                                    */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikS3M::ConvertTrack(S3MNOTE *tr)
{
	int32 t;

	UniReset();

	for (t = 0; t < 64; t++)
	{
		uint8 note, ins, vol;

		note = tr[t].note;
		ins  = tr[t].ins;
		vol  = tr[t].vol;

		if ((ins) && (ins != 255))
			UniInstrument(ins - 1);

		if (note != 255)
		{
			if (note == 254)
			{
				UniPTEffect(0xc, 0, of.flags);					// Note cut command
				vol = 255;
			}
			else
				UniNote(((note >> 4) * OCTAVE) + (note & 0xf));	// Normal note
		}

		if (vol < 255)
			UniPTEffect(0xc, vol, of.flags);

		S3MIT_ProcessCmd(tr[t].cmd, tr[t].inf, tracker == 1 ? S3MIT_OLDSTYLE | S3MIT_SCREAM : S3MIT_OLDSTYLE);
		UniNewLine();
	}

	return (UniDup());
}
