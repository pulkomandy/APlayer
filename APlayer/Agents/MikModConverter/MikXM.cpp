/******************************************************************************/
/* MikModConverter XM class.                                                  */
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
typedef struct XMHEADER
{
	char	id[18];					// ID text: 'Extended module: '
	char	songName[21];			// Module name, padded with zeros
	uint8	num1A;					// 0x1a
	char	trackerName[21];		// Tracker name
	uint16	version;				// Version number
	uint32	headerSize;				// Header size
	uint16	songLength;				// Song length (in pattern order table)
	uint16	restart;				// Restart position
	uint16	numChn;					// Number of channels (2, 4, 6, 8, 10, ..., 32)
	uint16	numPat;					// Number of patterns (max 256)
	uint16	numIns;					// Number of instruments (max 128)
	uint16	flags;
	uint16	tempo;					// Default tempo
	uint16	bpm;					// Default BPM
	uint8	orders[256];			// Pattern order table
} XMHEADER;



typedef struct XMINSTHEADER
{
	uint32	size;					// Instrument size
	char	name[23];				// Instrument name
	uint8	type;					// Instrument type (always 0)
	uint16	numSmp;					// Number of samples in instrument
	uint32	sSize;
} XMINSTHEADER;



#define XMENVCNT	(12 * 2)
#define XMNOTECNT	(8 * OCTAVE)
#define XM_SMPINCR	64

typedef struct XMPATCHHEADER
{
	uint8	what[XMNOTECNT];		// Sample number for all notes
	uint16	volEnv[XMENVCNT];		// Points for volume envelope
	uint16	panEnv[XMENVCNT];		// Points for panning envelope
	uint8	volPts;					// Number of volume points
	uint8	panPts;					// Number of panning points
	uint8	volSus;					// Volume sustain point
	uint8	volBeg;					// Volume loop start point
	uint8	volEnd;					// Volume loop end point
	uint8	panSus;					// Panning sustain point
	uint8	panBeg;					// Panning loop start point
	uint8	panEnd;					// Panning loop end point
	uint8	volFlg;					// Volume type: Bit 0: On, 1: Sustain, 2: Loop
	uint8	panFlg;					// Panning type: Bit 0: On, 1: Sustain, 2: Loop
	uint8	vibFlg;					// Vibrato type
	uint8	vibSweep;				// Vibrato sweep
	uint8	vibDepth;				// Vibrato depth
	uint8	vibRate;				// Vibrato rate
	uint16	volFade;				// Volume fadeout
} XMPATCHHEADER;



typedef struct XMWAVHEADER
{
	uint32	length;					// Sample length
	uint32	loopStart;				// Sample loop start
	uint32	loopLength;				// Sample loop length
	uint8	volume;					// Volume
	int8	fineTune;				// Finetune (signed byte -128..+127)
	uint8	type;					// Loop type
	uint8	panning;				// Panning (0-255)
	int8	relNote;				// Relative note number (signed byte)
	uint8	reserved;
	char	sampleName[23];			// Sample name
	uint8	vibType;				// Vibrato type
	uint8	vibSweep;				// Vibrato sweep
	uint8	vibDepth;				// Vibrato depth
	uint8	vibRate;				// Vibrato rate
} XMWAVHEADER;



typedef struct XMPATHEADER
{
	uint32	size;					// Pattern header length
	uint8	packing;				// Packing type (always 0)
	uint16	numRows;				// Number of rows in pattern (1..256)
	int16	packSize;				// Packed patterndata size
} XMPATHEADER;



typedef struct XMNOTE
{
	uint8	note;
	uint8	ins;
	uint8	vol;
	uint8	eff;
	uint8	dat;
} XMNOTE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a FastTracker II     */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikXM::CheckModule(APAgent_ConvertModule *convInfo)
{
	char buf[38];
	uint16 ver;
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(XMHEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(buf, 38);

	if (memcmp(buf, "Extended Module: ", 17) != 0)
		return (false);

	if (buf[37] != 0x1a)
		return (false);

	// Check the version
	file->Seek(58, PFile::pSeekBegin);
	ver = file->Read_L_UINT16();

	if ((ver < 0x102) || (ver > 0x104))
		return (false);

	return (true);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from FastTracker II to UniMod */
/*         structure.                                                         */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikXM::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	INSTRUMENT *d;
	SAMPLE *q;
	int32 t, u;
	bool dummyPat = false;
	PString tracker;
	char *trackerStr;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		mh      = NULL;
		wh      = NULL;
		s       = NULL;
		nextWav = NULL;

		// Allocate buffers
		if ((mh = new XMHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(mh, 0, sizeof(XMHEADER));

		// Try to read the header
		file->ReadString(mh->id, 17);
		file->ReadString(mh->songName, 20);
		mh->num1A = file->Read_UINT8();
		file->ReadString(mh->trackerName, 20);

		mh->version    = file->Read_L_UINT16();
		mh->headerSize = file->Read_L_UINT32();
		mh->songLength = file->Read_L_UINT16();
		mh->restart    = file->Read_L_UINT16();
		mh->numChn     = file->Read_L_UINT16();
		mh->numPat     = file->Read_L_UINT16();
		mh->numIns     = file->Read_L_UINT16();
		mh->flags      = file->Read_L_UINT16();
		mh->tempo      = file->Read_L_UINT16();
		mh->bpm        = file->Read_L_UINT16();

		if (!mh->bpm)
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		file->Read(mh->orders, 256);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		tracker.SetString(mh->trackerName, &charSet850);
		tracker.TrimRight();

		if (tracker.IsEmpty())
			tracker.LoadString(res, IDS_MIKC_NAME_XM_UNKNOWN);

		convInfo->modKind.Format(res, IDS_MIKC_NAME_XM, (trackerStr = tracker.GetString()), mh->version >> 8, mh->version & 0xff);
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_XM);
		tracker.FreeBuffer(trackerStr);

		of.initSpeed  = mh->tempo;
		of.initTempo  = mh->bpm;
		of.numChn     = mh->numChn;
		of.numPat     = mh->numPat;
		of.numTrk     = (uint16)of.numPat * of.numChn;	// Get number of channels
		of.songName.SetString(mh->songName, &charSet850);
		of.numPos     = mh->songLength;					// Copy the songlength
		of.repPos     = mh->restart < mh->songLength ? mh->restart : 0;
		of.numIns     = mh->numIns;
		of.flags     |= UF_XMPERIODS | UF_INST | UF_NOWRAP | UF_FT2QUIRKS | UF_PANNING;

		if (mh->flags & 1)
			of.flags |= UF_LINEAR;

		of.bpmLimit = 32;

		memset(of.chanVol, 64, of.numChn);				// Store channel volumes

		if (!(AllocPositions(of.numPos + 1)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = 0; t < of.numPos; t++)
			of.positions[t] = mh->orders[t];

		// We have to check for any pattern numbers in the order list greater than
		// the number of patterns total. If one or more is found, we set it equal to
		// the pattern total and make a dummy pattern to workaround the problem
		for (t = 0; t < of.numPos; t++)
		{
			if (of.positions[t] >= of.numPat)
			{
				of.positions[t] = of.numPat;
				dummyPat        = true;
			}
		}

		if (dummyPat)
		{
			of.numPat++;
			of.numTrk += of.numChn;
		}

		if (mh->version < 0x0104)
		{
			LoadInstruments(file);
			LoadPatterns(file, dummyPat);

			for (t = 0; t < of.numSmp; t++)
				nextWav[t] += file->GetPosition();
		}
		else
		{
			LoadPatterns(file, dummyPat);
			LoadInstruments(file);
		}

		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		q = of.samples;
		s = wh;

		for (u = 0; u < of.numSmp; u++, q++, s++)
		{
			q->sampleName.SetString(s->sampleName, &charSet850);
			q->length     = s->length;
			q->loopStart  = s->loopStart;
			q->loopEnd    = s->loopStart + s->loopLength;
			q->volume     = s->volume;
			q->speed      = s->fineTune + 128;
			q->panning    = s->panning;
			q->seekPos    = nextWav[u];
			q->vibType    = s->vibType;
			q->vibSweep   = s->vibSweep;
			q->vibDepth   = s->vibDepth;
			q->vibRate    = s->vibRate;

			if (s->type & 0x10)
			{
				q->length    >>= 1;
				q->loopStart >>= 1;
				q->loopEnd   >>= 1;
			}

			q->flags |= SF_OWNPAN | SF_DELTA | SF_SIGNED;

			if (s->type & 0x03)
				q->flags |= SF_LOOP;

			if (s->type & 0x02)
				q->flags |= SF_BIDI;

			if (s->type & 0x10)
				q->flags |= SF_16BITS;
		}

		d = of.instruments;
		s = wh;

		for (u = 0; u < of.numIns; u++, d++)
		{
			for (t = 0; t < XMNOTECNT; t++)
			{
				if (d->sampleNumber[t] >= of.numSmp)
					d->sampleNote[t] = 255;
				else
				{
					int32 note = t + s[d->sampleNumber[t]].relNote;
					d->sampleNote[t] = (note < 0) ? 0 : note;
				}
			}
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete[] wh;
	wh = NULL;

	delete[] nextWav;
	nextWav = NULL;

	delete mh;
	mh = NULL;

	return (retVal);
}



/******************************************************************************/
/* ReadNote() will read one note from the pattern.                            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*         "n" is a pointer to the note structure.                            */
/******************************************************************************/
uint8 MikXM::ReadNote(PFile *file, XMNOTE *n)
{
	uint8 cmp, result = 1;

	memset(n, 0, sizeof(XMNOTE));

	cmp = file->Read_UINT8();

	if (cmp & 0x80)
	{
		if (cmp & 1)
		{
			result++;
			n->note = file->Read_UINT8();
		}

		if (cmp & 2)
		{
			result++;
			n->ins = file->Read_UINT8();
		}

		if (cmp & 4)
		{
			result++;
			n->vol = file->Read_UINT8();
		}

		if (cmp & 8)
		{
			result++;
			n->eff = file->Read_UINT8();
		}

		if (cmp & 16)
		{
			result++;
			n->dat = file->Read_UINT8();
		}
	}
	else
	{
		n->note = cmp;
		n->ins  = file->Read_UINT8();
		n->vol  = file->Read_UINT8();
		n->eff  = file->Read_UINT8();
		n->dat  = file->Read_UINT8();
		result += 4;
	}

	return (result);
}



/******************************************************************************/
/* Convert() converts one track in one pattern.                               */
/*                                                                            */
/* Input:  "xmTrack" is a pointer to the note structure.                      */
/*         "rows" is the number of rows in the pattern.                       */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikXM::Convert(XMNOTE *xmTrack, uint16 rows)
{
	int32 t;
	uint8 note, ins, vol, eff, dat;

	UniReset();

	for (t = 0; t < rows; t++)
	{
		note = xmTrack->note;
		ins  = xmTrack->ins;
		vol  = xmTrack->vol;
		eff  = xmTrack->eff;
		dat  = xmTrack->dat;

		if (note)
		{
			if (note > XMNOTECNT)
				UniEffect(UNI_KEYFADE, 0);
			else
				UniNote(note - 1);
		}

		if (ins)
			UniInstrument(ins - 1);

		switch (vol >> 4)
		{
			// Volslide down
			case 0x6:
				if (vol & 0xf)
					UniEffect(UNI_XMEFFECTA, vol & 0xf);
				break;

			// Volslide up
			case 0x7:
				if (vol & 0xf)
					UniEffect(UNI_XMEFFECTA, vol << 4);
				break;

			// Volume-row fine volume slide is compatible with ProTracker
			// EBx and EAx effects, i.e. a zero nibble means DO NOT SLIDE, as
			// opposed to 'take the last sliding value'.
			//
			// Finevol down
			case 0x8:
				UniPTEffect(0xe, 0xb0 | (vol & 0xf), of.flags);
				break;

			// Finevol up
			case 0x9:
				UniPTEffect(0xe, 0xa0 | (vol & 0xf), of.flags);
				break;

			// Set vibrato speed
			case 0xa:
				UniEffect(UNI_XMEFFECT4, vol << 4);
				break;

			// Vibrato
			case 0xb:
				UniEffect(UNI_XMEFFECT4, vol & 0xf);
				break;

			// Set panning
			case 0xc:
				UniPTEffect(0x8, vol << 4, of.flags);
				break;

			// Panning slide left (Only slide when data nibble not zero)
			case 0xd:
				if (vol & 0xf)
					UniEffect(UNI_XMEFFECTP, vol & 0xf);
				break;

			// Panning slide right (Only slide when data nibble not zero)
			case 0xe:
				if (vol & 0xf)
					UniEffect(UNI_XMEFFECTP, vol << 4);
				break;

			// Tone portamento
			case 0xf:
				UniPTEffect(0x3, vol << 4, of.flags);
				break;

			default:
				if ((vol >= 0x10) && (vol <= 0x50))
					UniPTEffect(0xc, vol - 0x10, of.flags);
		}

		switch (eff)
		{
			// Vibrato
			case 0x4:
				UniEffect(UNI_XMEFFECT4, dat);
				break;

			// Vibrato + volume slide
			case 0x6:
				UniEffect(UNI_XMEFFECT6, dat);
				break;

			// Volume slide
			case 0xa:
				UniEffect(UNI_XMEFFECTA, dat);
				break;

			// Fine porta/volume slide + extra effects
			case 0xe:
				switch (dat >> 4)
				{
					// XM fine porta up
					case 0x1:
						UniEffect(UNI_XMEFFECTE1, dat & 0xf);
						break;

					// XM fine porta down
					case 0x2:
						UniEffect(UNI_XMEFFECTE2, dat & 0xf);
						break;

					// XM fine volume up
					case 0xa:
						UniEffect(UNI_XMEFFECTEA, dat & 0xf);
						break;

					// XM fine volume down
					case 0xb:
						UniEffect(UNI_XMEFFECTEB, dat & 0xf);
						break;

					default:
						UniPTEffect(eff, dat, of.flags);
						break;
				}
				break;

			// Set global volume
			case 'G' - 55:
				UniEffect(UNI_XMEFFECTG, dat > 64 ? 128 : dat << 1);
				break;

			// Global volume slide
			case 'H' - 55:
				UniEffect(UNI_XMEFFECTH, dat);
				break;

			// Keyoff and Keyfade
			case 'K' - 55:
				UniEffect(UNI_KEYFADE, dat);
				break;

			// Set envelope position
			case 'L' - 55:
				UniEffect(UNI_XMEFFECTL, dat);
				break;

			// Panning slide
			case 'P' - 55:
				UniEffect(UNI_XMEFFECTP, dat);
				break;

			// Multi retrig note
			case 'R' - 55:
				UniEffect(UNI_S3MEFFECTQ, dat);
				break;

			// Tremor
			case 'T' - 55:
				UniEffect(UNI_S3MEFFECTI, dat);
				break;

			// Extra fine porta
			case 'X' - 55:
				switch (dat >> 4)
				{
					// X1 - Extra fine portamento up
					case 1:
						UniEffect(UNI_XMEFFECTX1, dat & 0xf);
						break;

					// X2 - Extra fine portamento down
					case 2:
						UniEffect(UNI_XMEFFECTX2, dat & 0xf);
						break;
				}
				break;

			default:
				if (eff <= 0xf)
				{
					// The pattern jump destination is written in decimal,
					// but it seems some poor tracker software writes them
					// in hexadecimal... (sigh)
					if (eff == 0xd)
					{
						// Don't change anything if we're sure it's in hexa
						if ((((dat & 0xf0) >> 4) <= 9) && ((dat & 0xf) <= 9))
						{
							// Otherwise, convert from dec to hex
							dat = (((dat & 0xf0) >> 4) * 10) + (dat & 0xf);
						}
					}

					UniPTEffect(eff, dat, of.flags);
				}
				break;
		}

		UniNewLine();
		xmTrack++;
	}

	return (UniDup());
}



/******************************************************************************/
/* LoadPatterns() load all the patterns.                                      */
/*                                                                            */
/* Input:  "file" is a pointer to the file to load from.                      */
/*         "dummyPat" set this to true to create an extra dummy pattern.      */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikXM::LoadPatterns(PFile *file, bool dummyPat)
{
	int32 t, u, v, numTrk;
	XMNOTE *xmPat;

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

	numTrk = 0;
	for (t = 0; t < mh->numPat; t++)
	{
		XMPATHEADER ph;

		ph.size = file->Read_L_UINT32();
		if (ph.size < (uint32)(mh->version == 0x0102 ? 8 : 9))
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		ph.packing = file->Read_UINT8();
		if (ph.packing)
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (mh->version == 0x0102)
			ph.numRows = file->Read_UINT8() + 1;
		else
			ph.numRows = file->Read_L_UINT16();

		ph.packSize = file->Read_L_UINT16();

		ph.size -= (mh->version == 0x0102 ? 8 : 9);
		if (ph.size)
			file->Seek(ph.size, PFile::pSeekCurrent);

		of.pattRows[t] = ph.numRows;

		if (ph.numRows)
		{
			if ((xmPat = new XMNOTE[ph.numRows * of.numChn]) == NULL)
			{
				ShowError(IDS_MIKC_ERR_MEMORY);
				throw PUserException();
			}

			memset(xmPat, 0, ph.numRows * of.numChn * sizeof(XMNOTE));

			// When packSize is 0, don't try to load a pattern.. it's empty
			if (ph.packSize)
			{
				for (u = 0; u < ph.numRows; u++)
				{
					for (v = 0; v < of.numChn; v++)
					{
						if (!ph.packSize)
							break;

						ph.packSize -= ReadNote(file, &xmPat[(v * ph.numRows) + u]);
						if (ph.packSize < 0)
						{
							delete[] xmPat;
							xmPat = NULL;

							ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
							throw PUserException();
						}
					}
				}
			}

			if (ph.packSize)
				file->Seek(ph.packSize, PFile::pSeekCurrent);

			if (file->IsEOF())
			{
				delete[] xmPat;
				xmPat = NULL;

				ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
				throw PUserException();
			}

			for (v = 0; v < of.numChn; v++)
				of.tracks[numTrk++] = Convert(&xmPat[v * ph.numRows], ph.numRows);

			delete[] xmPat;
			xmPat = NULL;
		}
		else
		{
			for (v = 0; v < of.numChn; v++)
				of.tracks[numTrk++] = Convert(NULL, ph.numRows);
		}
	}

	if (dummyPat)
	{
		of.pattRows[t] = 64;

		if ((xmPat = new XMNOTE[64 * of.numChn]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		memset(xmPat, 0, 64 * of.numChn * sizeof(XMNOTE));

		for (v = 0; v < of.numChn; v++)
			of.tracks[numTrk++] = Convert(&xmPat[v * 64], 64);

		delete[] xmPat;
		xmPat = NULL;
	}
}



/******************************************************************************/
/* LoadInstruments() load all the instruments.                                */
/*                                                                            */
/* Input:  "file" is a pointer to the file to load from.                      */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikXM::LoadInstruments(PFile *file)
{
	int32 t, u;
	INSTRUMENT *d;
	uint32 next = 0;
	uint16 wavCnt = 0;

	if (!(AllocInstruments()))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		throw PUserException();
	}

	d = of.instruments;

	for (t = 0; t < of.numIns; t++, d++)
	{
		XMINSTHEADER ih;
		int32 headEnd;

		memset(d->sampleNumber, 0xff, INSTNOTES * sizeof(uint16));

		// Read instrument header
		headEnd  = file->GetPosition();
		ih.size  = file->Read_L_UINT32();
		headEnd += ih.size;

		file->ReadString(ih.name, 22);

		ih.type    = file->Read_UINT8();
		ih.numSmp  = file->Read_L_UINT16();
		d->insName.SetString(ih.name, &charSet850);

		if ((int16)ih.size > 29)
		{
			ih.sSize = file->Read_L_UINT32();

			if (((int16)ih.numSmp > 0) && (ih.numSmp <= XMNOTECNT))
			{
				XMPATCHHEADER pth;
				int32 p;

				file->Read(pth.what, XMNOTECNT);
				file->ReadArray_L_UINT16s(pth.volEnv, XMENVCNT);
				file->ReadArray_L_UINT16s(pth.panEnv, XMENVCNT);

				pth.volPts   = file->Read_UINT8();
				pth.panPts   = file->Read_UINT8();
				pth.volSus   = file->Read_UINT8();
				pth.volBeg   = file->Read_UINT8();
				pth.volEnd   = file->Read_UINT8();
				pth.panSus   = file->Read_UINT8();
				pth.panBeg   = file->Read_UINT8();
				pth.panEnd   = file->Read_UINT8();
				pth.volFlg   = file->Read_UINT8();
				pth.panFlg   = file->Read_UINT8();
				pth.vibFlg   = file->Read_UINT8();
				pth.vibSweep = file->Read_UINT8();
				pth.vibDepth = file->Read_UINT8();
				pth.vibRate  = file->Read_UINT8();
				pth.volFade  = file->Read_L_UINT16();

				// Read the remainder of the header
				// (2 bytes for 1.03, 22 for 1.04)
				for (u = headEnd - file->GetPosition(); u; u--)
					file->Read_UINT8();

				// We can't trust the envelope point count here, as some
				// modules have incorrect values (K_OSPACE.XM reports 32 volume
				// points, for example)
				if (pth.volPts > XMENVCNT / 2)
					pth.volPts = XMENVCNT / 2;

				if (pth.panPts > XMENVCNT / 2)
					pth.panPts = XMENVCNT / 2;

				if ((file->IsEOF()) || (pth.volPts > XMENVCNT / 2) || (pth.panPts > XMENVCNT / 2))
				{
					ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
					throw PUserException();
				}

				for (u = 0; u < XMNOTECNT; u++)
					d->sampleNumber[u] = pth.what[u] + of.numSmp;

				d->volFade = pth.volFade;

#define XM_ProcessEnvelope(name) \
				for (u = 0; u < (XMENVCNT >> 1); u++) \
				{ \
					d->name##Env[u].pos = pth.name##Env[u << 1]; \
					d->name##Env[u].val = pth.name##Env[(u << 1) + 1]; \
				} \
				if (pth.name##Flg & 1) \
					d->name##Flg |= EF_ON; \
				if (pth.name##Flg & 2) \
					d->name##Flg |= EF_SUSTAIN; \
				if (pth.name##Flg & 4) \
					d->name##Flg |= EF_LOOP; \
				d->name##SusBeg = d->name##SusEnd = pth.name##Sus; \
				d->name##Beg    = pth.name##Beg; \
				d->name##End    = pth.name##End; \
				d->name##Pts    = pth.name##Pts; \
				/* Scale envelope */ \
				for (p = 0; p < XMENVCNT / 2; p++) \
					d->name##Env[p].val <<= 2; \
				if ((d->name##Flg & EF_ON) && (d->name##Pts < 2)) \
					d->name##Flg &= ~EF_ON;

				XM_ProcessEnvelope(vol);
				XM_ProcessEnvelope(pan);
#undef XM_ProcessEnvelope

				if (d->volFlg & EF_ON)
					FixEnvelope(d->volEnv, d->volPts);

				if (d->panFlg & EF_ON)
					FixEnvelope(d->panEnv, d->panPts);

				// Samples are stored outside the instrument struct now, so we
				// have to load them all into a temp area, count the of.numSmp
				// along the way and then do an AllocSamples() and move
				// everything over
				if (mh->version > 0x0103)
					next = 0;

				for (u = 0; u < ih.numSmp; u++, s++)
				{
					// Allocate more room for sample information if necessary
					if (of.numSmp + u == wavCnt)
					{
						uint32 *newNextWav;
						XMWAVHEADER *newWave;

						wavCnt += XM_SMPINCR;

						if ((newNextWav = new uint32[wavCnt]) == NULL)
						{
							ShowError(IDS_MIKC_ERR_MEMORY);
							throw PUserException();
						}

						if (nextWav)
							memcpy(newNextWav, nextWav, (wavCnt - XM_SMPINCR) * sizeof(uint32));

						delete[] nextWav;
						nextWav = newNextWav;

						if ((newWave = new XMWAVHEADER[wavCnt]) == NULL)
						{
							ShowError(IDS_MIKC_ERR_MEMORY);
							throw PUserException();
						}

						if (wh)
							memcpy(newWave, wh, (wavCnt - XM_SMPINCR) * sizeof(XMWAVHEADER));

						delete[] wh;
						wh = newWave;

						s = wh + (wavCnt - XM_SMPINCR);
					}

					s->length     = file->Read_L_UINT32();
					s->loopStart  = file->Read_L_UINT32();
					s->loopLength = file->Read_L_UINT32();
					s->volume     = file->Read_UINT8();
					s->fineTune   = file->Read_UINT8();
					s->type       = file->Read_UINT8();
					s->panning    = file->Read_UINT8();
					s->relNote    = file->Read_UINT8();
					s->vibType    = pth.vibFlg;
					s->vibSweep   = pth.vibSweep;
					s->vibDepth   = pth.vibDepth * 4;
					s->vibRate    = pth.vibRate;
					s->reserved   = file->Read_UINT8();

					file->ReadString(s->sampleName, 22);

					nextWav[of.numSmp + u] = next;
					next += s->length;

					if (file->IsEOF())
					{
						ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
						throw PUserException();
					}
				}

				if (mh->version > 0x0103)
				{
					for (u = 0; u < ih.numSmp; u++)
						nextWav[of.numSmp++] += file->GetPosition();

					file->Seek(next, PFile::pSeekCurrent);
				}
				else
					of.numSmp += ih.numSmp;
			}
			else
			{
				// Read the remainder of the header
				for (u = headEnd - file->GetPosition(); u; u--)
					file->Read_UINT8();

				if (file->IsEOF())
				{
					ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
					throw PUserException();
				}
			}
		}
	}

	// Sanity check
	if (!of.numSmp)
	{
		ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
		throw PUserException();
	}
}



/******************************************************************************/
/* FixEnvelope() will try to detect and fix corrupted envelopes.              */
/*                                                                            */
/* Input:  "cur" is a pointer to the envelope to check.                       */
/*         "pts" is the number of points in the envelope.                     */
/******************************************************************************/
void MikXM::FixEnvelope(ENVPT *cur, int32 pts)
{
	int32 u, old, tmp;
	ENVPT *prev;

	// Some broken XM editing program will only save the low byte
	// of the position value. Try to compensate by adding the
	// missing high byte
	prev = cur++;
	old  = prev->pos;

	for (u = 1; u < pts; u++, prev++, cur++)
	{
		if (cur->pos < prev->pos)
		{
			if (cur->pos < 0x100)
			{
				if (cur->pos > old)		// Same hex century
					tmp = cur->pos + (prev->pos - old);
				else
					tmp = cur->pos | ((prev->pos + 0x100) & 0xff00);

				old      = cur->pos;
				cur->pos = tmp;
			}
			else
			{
				// Different brokenness style... fix unknown
				old = cur->pos;
			}
		}
		else
			old = cur->pos;
	}
}
