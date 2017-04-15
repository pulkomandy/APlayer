/******************************************************************************/
/* MikModConverter STX class.                                                 */
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
typedef struct STXHEADER
{
	char	songName[21];
	char	trackerName[9];
	uint16	patSize;
	uint16	unknown1;
	uint16	patPtr;
	uint16	insPtr;
	uint16	chnPtr;					// Not sure
	uint16	unknown2;
	uint16	unknown3;
	uint8	masterMult;
	uint8	initSpeed;
	uint16	unknown4;
	uint16	unknown5;
	uint16	patNum;
	uint16	insNum;
	uint16	ordNum;
	uint16	unknown6;
	uint16	unknown7;
	uint16	unknown8;
	char	scrm[4];
} STXHEADER;



// Sample information
typedef struct STXSAMPLE
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
} STXSAMPLE;



typedef struct STXNOTE
{
	uint8	note;
	uint8	ins;
	uint8	vol;
	uint8	cmd;
	uint8	inf;
} STXNOTE;



#define NUMTRACKERS		3

static const char *signatures[NUMTRACKERS] = {
	"!Scream!",
	"BMOD2STM",
	"WUZAMOD!"
};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a STMIK module.      */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikSTX::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 id[8];
	int32 t;
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(STXHEADER))
		return (false);

	// Check for the S3M signature
	file->Seek(60, PFile::pSeekBegin);
	file->Read(id, 4);
	if (memcmp(id, "SCRM", 4))
		return (false);

	// Now check the signature
	file->Seek(20, PFile::pSeekBegin);
	file->Read(id, 8);

	for (t = 0; t < NUMTRACKERS; t++)
	{
		if (!(memcmp(id, signatures[t], 8)))
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from STMIK to UniMod          */
/*         structure.                                                         */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikSTX::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, track = 0;
	int32 version = 0;
	SAMPLE *q;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		mh        = NULL;
		stxBuf    = NULL;
		posLookup = NULL;
		paraPtr   = NULL;

		// Allocate buffers
		if ((mh = new STXHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((stxBuf = new STXNOTE[4 * 64]) == NULL)
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
		memset(mh, 0, sizeof(STXHEADER));
		memset(stxBuf, 0, sizeof(STXNOTE) * 4 * 64);
		memset(posLookup, -1, 256);

		// Try to read module header
		file->ReadString(mh->songName, 20);
		file->ReadString(mh->trackerName, 8);

		mh->patSize    = file->Read_L_UINT16();
		mh->unknown1   = file->Read_L_UINT16();
		mh->patPtr     = file->Read_L_UINT16();
		mh->insPtr     = file->Read_L_UINT16();
		mh->chnPtr     = file->Read_L_UINT16();
		mh->unknown2   = file->Read_L_UINT16();
		mh->unknown3   = file->Read_L_UINT16();
		mh->masterMult = file->Read_UINT8();
		mh->initSpeed  = file->Read_UINT8() >> 4;
		mh->unknown4   = file->Read_L_UINT16();
		mh->unknown5   = file->Read_L_UINT16();
		mh->patNum     = file->Read_L_UINT16();
		mh->insNum     = file->Read_L_UINT16();
		mh->ordNum     = file->Read_L_UINT16();
		mh->unknown6   = file->Read_L_UINT16();
		mh->unknown7   = file->Read_L_UINT16();
		mh->unknown8   = file->Read_L_UINT16();
		file->Read(mh->scrm, 4);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		of.songName.SetString(mh->songName, &charSet850);
		of.numPat    = mh->patNum;
		of.repPos    = 0;
		of.numIns    = of.numSmp = mh->insNum;
		of.initSpeed = mh->initSpeed;
		of.initTempo = 125;
		of.numChn    = 4;
		of.flags    |= UF_S3MSLIDES;
		of.bpmLimit  = 32;

		if ((paraPtr = new uint16[of.numIns + of.numPat]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the instrument + patterns parapointers
		file->Seek(mh->insPtr << 4, PFile::pSeekBegin);
		file->ReadArray_L_UINT16s(paraPtr, of.numIns);

		file->Seek(mh->patPtr << 4, PFile::pSeekBegin);
		file->ReadArray_L_UINT16s(paraPtr + of.numIns, of.numPat);

		// Check module version
		file->Seek(paraPtr[of.numIns] << 4, PFile::pSeekBegin);
		version = file->Read_L_UINT16();
		if (version == mh->patSize)
		{
			version = 0x10;
			convInfo->modKind.LoadString(res, IDS_MIKC_NAME_STX_10);
		}
		else
		{
			version = 0x11;
			convInfo->modKind.LoadString(res, IDS_MIKC_NAME_STX_11);
		}

		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_STX);

		// Read the order data
		file->Seek((mh->chnPtr << 4) + 32, PFile::pSeekBegin);

		if (!(AllocPositions(mh->ordNum)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = 0; t < mh->ordNum; t++)
		{
			of.positions[t] = file->Read_UINT8();
			file->Seek(4, PFile::pSeekCurrent);
		}

		of.numPos    = 0;
		posLookupCnt = mh->ordNum;

		for (t = 0; t < mh->ordNum; t++)
		{
			int32 order = of.positions[t];
			if (order == 255)
				order = LAST_PATTERN;

			of.positions[of.numPos] = order;
			posLookup[t] = of.numPos;		// Bug fix for freaky S3Ms

			if (of.positions[t] < 254)
				of.numPos++;
			else
			{
				// Special end of song pattern
				if ((order == LAST_PATTERN) && (!curious))
					break;
			}
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

		for (q = of.samples, t = 0; t < of.numIns; t++, q++)
		{
			STXSAMPLE s;

			// Seek to instrument position
			file->Seek(((int32)paraPtr[t]) << 4, PFile::pSeekBegin);

			// And load sample info
			s.type = file->Read_UINT8();

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

			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			q->sampleName.SetString(s.sampName, &charSet850);
			q->speed      = (s.c2Spd * 8363) / 8448;
			q->length     = s.length;
			q->loopStart  = s.loopBeg;
			q->loopEnd    = s.loopEnd;
			q->volume     = s.volume;
			q->seekPos    = (((int32)s.memSegH) << 16 | s.memSegL) << 4;
			q->flags     |= SF_SIGNED;

			if (s.flags & 1)
				q->flags |= SF_LOOP;

			if (s.flags & 4)
				q->flags |= SF_16BITS;
		}

		// Load pattern info
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
			// Seek to pattern position (+2 skip pattern length)
			file->Seek((((int32)paraPtr[of.numIns + t]) << 4) + (version == 0x10 ? 2 : 0), PFile::pSeekBegin);
			ReadPattern(file);

			for (u = 0; u < of.numChn; u++)
			{
				if (!(of.tracks[track++] = ConvertTrack(&stxBuf[u * 64])))
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
	delete[] stxBuf;
	stxBuf = NULL;

	delete[] paraPtr;
	paraPtr = NULL;

	delete[] posLookup;
	posLookup = NULL;

	delete mh;
	mh = NULL;

	return (retVal);
}



/******************************************************************************/
/* ReadPattern() read one pattern.                                            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikSTX::ReadPattern(PFile *file)
{
	int32 row = 0, flag, ch;
	STXNOTE *n, dummy;

	// Clear pattern data
	memset(stxBuf, 255, 4 * 64 * sizeof(STXNOTE));

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

			if ((ch >= 0) && (ch < 4))
				n = &stxBuf[(64U * ch) + row];
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
/* ConvertTrack() convert one track in one pattern.                           */
/*                                                                            */
/* Input:  "tr" is a pointer to the note structure.                           */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikSTX::ConvertTrack(STXNOTE *tr)
{
	int32 t;

	UniReset();

	for (t = 0; t < 64; t++)
	{
		uint8 note, ins, vol, cmd, inf;

		note = tr[t].note;
		ins  = tr[t].ins;
		vol  = tr[t].vol;
		cmd  = tr[t].cmd;
		inf  = tr[t].inf;

		if ((ins) && (ins != 255))
			UniInstrument(ins - 1);

		if ((note) && (note != 255))
		{
			if (note == 254)
			{
				UniPTEffect(0xc, 0, of.flags);	// Note cut command
				vol = 255;
			}
			else
				UniNote(24 + ((note >> 4) * OCTAVE) + (note & 0xf));	// Normal note
		}

		if (vol < 255)
			UniPTEffect(0xc, vol, of.flags);

		if (cmd < 255)
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

		UniNewLine();
	}

	return (UniDup());
}
