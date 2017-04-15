/******************************************************************************/
/* MikModConverter STM class.                                                 */
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
typedef struct STMSAMPLE
{
	char 	fileName[13];
	uint8	unused;					// 0x00;
	uint8	instDisk;				// Instrument disk
	uint16	reserved;
	uint16	length;					// Sample length
	uint16	loopBeg;				// Loop start point
	uint16	loopEnd;				// Loop end point
	uint8	volume;					// Volume
	uint8	reserved2;
	uint16	c2Spd;					// Good old c2spd
	uint32	reserved3;
	uint16	isa;
} STMSAMPLE;



// Header
typedef struct STMHEADER
{
	char	songName[21];
	char	trackerName[9];			// !Scream! for ST 2.xx
	uint8	unused;					// 0x1a
	uint8	fileType;				// 1 = Song, 2 = Module
	uint8	ver_Major;
	uint8	ver_Minor;
	uint8	initTempo;				// initSpeed = stm initTempo >> 4
	uint8	numPat;					// Number of patterns
	uint8	globalVol;
	uint8	reserved[13];
	STMSAMPLE sample[31];			// STM sample data
	uint8	patOrder[128];			// Docs say 64 - actually 128
} STMHEADER;



typedef struct STMNOTE
{
	uint8	note;
	uint8	insVol;
	uint8	volCmd;
	uint8	cmdInf;
} STMNOTE;



#define NUMTRACKERS		3

static const char* signatures[NUMTRACKERS] = {
	"!Scream!",
	"BMOD2STM",
	"WUZAMOD!"
};

static const int32 version[NUMTRACKERS] = {
	IDS_MIKC_NAME_STM,
	IDS_MIKC_NAME_STM_MOD2STM,
	IDS_MIKC_NAME_STM_WUZAMOD
};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a ScreamTracker 2.x  */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikSTM::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 buf[44];
	int32 t;
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(STMHEADER))
		return (false);

	// Now check the signature and filetype
	file->Seek(20, PFile::pSeekBegin);
	file->Read(buf, 44);

	// Check the file type
	if (buf[9] != 2)
		return (false);

	// Prevent false positives for S3M files
	if (!memcmp(buf + 40, "SCRM", 4))
		return (false);

	// Check for the signatures
	for (t = 0; t < NUMTRACKERS; t++)
	{
		if (!memcmp(buf, signatures[t], 8))
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from ScreamTracker 2.x to     */
/*         UniMod structure.                                                  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikSTM::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t;
	uint32 mikMod_ISA;		// We MUST generate our own ISA, it's not stored in stm
	SAMPLE *q;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		stmBuf = NULL;
		mh     = NULL;

		// Allocate buffers
		if ((mh = new STMHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((stmBuf = new STMNOTE[64U * 4]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(stmBuf, 0, sizeof(STMNOTE) * 64U * 4);
		memset(mh, 0, sizeof(STMHEADER));

		// Try to read the header
		file->ReadString(mh->songName, 20);
		file->ReadString(mh->trackerName, 8);

		mh->unused    = file->Read_UINT8();
		mh->fileType  = file->Read_UINT8();
		mh->ver_Major = file->Read_UINT8();
		mh->ver_Minor = file->Read_UINT8();
		mh->initTempo = file->Read_UINT8();

		if (!mh->initTempo)
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		mh->numPat    = file->Read_UINT8();
		mh->globalVol = file->Read_UINT8();
		file->Read(mh->reserved, 13);

		for (t = 0; t < 31; t++)
		{
			STMSAMPLE *s = &mh->sample[t];		// STM sample data

			file->ReadString(s->fileName, 12);
			s->unused    = file->Read_UINT8();
			s->instDisk  = file->Read_UINT8();
			s->reserved  = file->Read_L_UINT16();
			s->length    = file->Read_L_UINT16();
			s->loopBeg   = file->Read_L_UINT16();
			s->loopEnd   = file->Read_L_UINT16();
			s->volume    = file->Read_UINT8();
			s->reserved2 = file->Read_UINT8();
			s->c2Spd     = file->Read_L_UINT16();
			s->reserved3 = file->Read_L_UINT32();
			s->isa       = file->Read_L_UINT16();
		}

		file->Read(mh->patOrder, 128);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		for (t = 0; t < NUMTRACKERS; t++)
		{
			if (!memcmp(mh->trackerName, signatures[t], 8))
				break;
		}

		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_STM);
		convInfo->modKind.LoadString(res, version[t]);
		of.songName.SetString(mh->songName, &charSet850);
		of.numPat     = mh->numPat;
		of.initTempo  = 125;
		of.initSpeed  = mh->initTempo >> 4;
		of.numChn     = 4;
		of.repPos     = 0;
		of.flags      = UF_S3MSLIDES;
		of.bpmLimit   = 32;

		if (!(AllocPositions(0x80)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		t = 0;

		// 99 Terminates the patOrder list
		while ((mh->patOrder[t] <= 99) && (mh->patOrder[t] < mh->numPat))
		{
			of.positions[t] = mh->patOrder[t];
			t++;
		}

		if (mh->patOrder[t] <= 99)
			t++;

		of.numPos = t;
		of.numTrk = of.numPat * of.numChn;
		of.numIns = of.numSmp = 31;

		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		LoadPatterns(file);

		mikMod_ISA = file->GetPosition();
		mikMod_ISA = (mikMod_ISA + 15) & 0xfffffff0;	// Normalize

		for (q = of.samples, t = 0; t < of.numSmp; t++, q++)
		{
			// Load sample info
			q->sampleName.SetString(mh->sample[t].fileName, &charSet850);
			q->speed  = (mh->sample[t].c2Spd * 8363) / 8448;
			q->volume = mh->sample[t].volume;
			q->length = mh->sample[t].length;

			if (q->length == 1)
				q->length = 0;

			q->loopStart  = mh->sample[t].loopBeg;
			q->loopEnd    = mh->sample[t].loopEnd;
			q->seekPos    = mikMod_ISA;

			mikMod_ISA += q->length;
			mikMod_ISA  = (mikMod_ISA + 15) & 0xfffffff0;	// Normalize

			// Contrary to the STM specs, sample data is signed
			q->flags = SF_SIGNED;

			if (q->loopEnd && (q->loopEnd != 0xffff))
				q->flags |= SF_LOOP;
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete mh;
	mh = NULL;

	delete[] stmBuf;
	stmBuf = NULL;

	return (retVal);
}



/******************************************************************************/
/* LoadPatterns() read all the patterns.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikSTM::LoadPatterns(PFile *file)
{
	int32 t, s, tracks = 0;

	if (!(AllocPatterns()))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		throw PUserException();
	}

	if (!(AllocTracks()))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		throw PUserException();
	}

	// Allocate temporary buffer for loading and converting the patterns
	for (t = 0; t < of.numPat; t++)
	{
		for (s = 0; s < (64 * of.numChn); s++)
		{
			stmBuf[s].note   = file->Read_UINT8();
			stmBuf[s].insVol = file->Read_UINT8();
			stmBuf[s].volCmd = file->Read_UINT8();
			stmBuf[s].cmdInf = file->Read_UINT8();
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		for (s = 0; s < of.numChn; s++)
		{
			if (!(of.tracks[tracks++] = ConvertTrack(stmBuf + s)))
			{
				ShowError(IDS_MIKC_ERR_INITIALIZE);
				throw PUserException();
			}
		}
	}
}



/******************************************************************************/
/* ConvertTrack() convert one track in one pattern.                           */
/*                                                                            */
/* Input:  "n" is a pointer to the note structure.                            */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikSTM::ConvertTrack(STMNOTE *n)
{
	int32 t;

	UniReset();

	for (t = 0; t < 64; t++)
	{
		ConvertNote(n);
		UniNewLine();
		n += of.numChn;
	}

	return (UniDup());
}



/******************************************************************************/
/* ConvertNote() convert a note to UniMod commands.                           */
/*                                                                            */
/* Input:  "n" is a pointer to the note structure.                            */
/******************************************************************************/
void MikSTM::ConvertNote(STMNOTE *n)
{
	uint8 note, ins, vol, cmd, inf;

	// Extract the various information from the 4 bytes that make up a note
	note = n->note;
	ins  = n->insVol >> 3;
	vol  = (n->insVol & 7) + ((n->volCmd & 0x70) >> 1);
	cmd  = n->volCmd & 15;
	inf  = n->cmdInf;

	if ((ins) && (ins < 32))
		UniInstrument(ins - 1);

	// Special values of [SBYTE0] are handled here.
	// We have no idea if these strange values will ever be encountered
	// but it appears as those stms sound correct
	if ((note == 254) || (note == 252))
	{
		UniPTEffect(0xc, 0, of.flags);	// Note cut
		n->volCmd |= 0x80;
	}
	else
	{
		// If note < 251, then all three bytes are stored in the file
		if (note < 251)
			UniNote((((note >> 4) + 2) * OCTAVE) + (note & 0xf));
	}

	if ((!(n->volCmd & 0x80)) && (vol < 65))
		UniPTEffect(0xc, vol, of.flags);

	if (cmd != 255)
	{
		switch (cmd)
		{
			// Axx: Set speed to xx
			case 1:
				UniPTEffect(0xf, inf >> 4, of.flags);
				break;

			// Bxx: Position jump
			case 2:
				UniPTEffect(0xb, inf, of.flags);
				break;

			// Cxx: Pattern break to row xx
			case 3:
				UniPTEffect(0xd, (((inf & 0xf0) >> 4) * 10) + (inf & 0xf), of.flags);
				break;

			// Dxy: Volume slide
			case 4:
				UniEffect(UNI_S3MEFFECTD, inf);
				break;

			// Exy: Tone slide down
			case 5:
				UniEffect(UNI_S3MEFFECTE, inf);
				break;

			// Fxy: Tone slide up
			case 6:
				UniEffect(UNI_S3MEFFECTF, inf);
				break;

			// Gxx: Tone portamento, speed xx
			case 7:
				UniPTEffect(0x3, inf, of.flags);
				break;

			// Hxy: Vibrato
			case 8:
				UniPTEffect(0x4, inf, of.flags);
				break;

			// Ixy: Tremor, ontime x, offtime y
			case 9:
				UniEffect(UNI_S3MEFFECTI, inf);
				break;

			// Protracker arpeggio
			case 0:
				if (!inf)
					break;

				// Fall through

			// Jxy: Arpeggio
			case 0xa:
				UniPTEffect(0x0, inf, of.flags);
				break;

			// Kxy: Dual command H00 & Dxy
			case 0xb:
				UniPTEffect(0x4, 0, of.flags);
				UniEffect(UNI_S3MEFFECTD, inf);
				break;

			// Lxy: Dual command G00 & Dxy
			case 0xc:
				UniPTEffect(0x3, 0, of.flags);
				UniEffect(UNI_S3MEFFECTD, inf);
				break;

			// Support all these above, since ST2 can LOAD these values
			// but can actually only play up to J - and J is only
			// half-way implemented in ST2
			//
			// Xxx: Amiga panning command 8xx
			case 0x18:
				UniPTEffect(0x8, inf, of.flags);
				of.flags |= UF_PANNING;
				break;
		}
	}
}
