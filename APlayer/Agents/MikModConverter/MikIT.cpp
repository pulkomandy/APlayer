/******************************************************************************/
/* MikModConverter IT class.                                                  */
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
typedef struct ITHEADER
{
	char	songName[27];
	uint8	blank01[2];
	uint16	ordNum;
	uint16	insNum;
	uint16	smpNum;
	uint16	patNum;
	uint16	cwt;					// Created with tracker (y.xx = 0x0yxx)
	uint16	cmwt;					// Compatible with tracker ver > than val.
	uint16	flags;
	uint16	special;				// Bit 0 set = song message attached
	uint8	globVol;
	uint8	mixVol;					// Mixing volume [ignored]
	uint8	initSpeed;
	uint8	initTempo;
	uint8	panSep;					// Panning separation between channels
	uint8	zeroByte;
	uint16	msgLength;
	uint32	msgOffset;
	uint8	blank02[4];
	uint8	panTable[64];
	uint8	volTable[64];
} ITHEADER;



// Sample information
typedef struct ITSAMPLE
{
	char	fileName[13];
	uint8	zeroByte;
	uint8	globVol;
	uint8	flag;
	uint8	volume;
	uint8	panning;
	char	sampName[29];
	uint16	convert;				// Sample conversion flag
	uint32	length;
	uint32	loopBeg;
	uint32	loopEnd;
	uint32	c5Spd;
	uint32	susBegin;
	uint32	susEnd;
	uint32	sampOffset;
	uint8	vibSpeed;
	uint8	vibDepth;
	uint8	vibRate;
	uint8	vibWave;				// 0 = Sine; 1 = Rampdown; 2 = Square; 3 = Random (speed ignored)
} ITSAMPLE;



// Instrument information
#define ITENVCNT	25
#define ITNOTECNT	120

typedef struct ITINSTHEADER
{
	uint32	size;					// Instrument size
	char	fileName[13];			// Instrument filename
	uint8	zeroByte;				// Instrument type (always 0)
	uint8	volFlg;
	uint8	volPts;
	uint8	volBeg;					// Volume loop start (node)
	uint8	volEnd;					// Volume loop end (node)
	uint8	volSusBeg;				// Volume sustain begin (node)
	uint8	volSusEnd;				// Volume sustain end (node)
	uint8	panFlg;
	uint8	panPts;
	uint8	panBeg;					// Channel loop start (node)
	uint8	panEnd;					// Channel loop end (node)
	uint8	panSusBeg;				// Channel sustain begin (node)
	uint8	panSusEnd;				// Channel sustain end (node)
	uint8	pitFlg;
	uint8	pitPts;
	uint8	pitBeg;					// Pitch loop start (node)
	uint8	pitEnd;					// Pitch loop end (node)
	uint8	pitSusBeg;				// Pitch sustain begin (node)
	uint8	pitSusEnd;				// Pitch sustain end (node)
	uint16	blank;
	uint8	globVol;
	uint8	chanPan;
	uint16	fadeOut;				// Envelope end / NNA volume fadeout
	uint8	dnc;					// Duplicate note check
	uint8	dca;					// Duplicate check action
	uint8	dct;					// Duplicate check type
	uint8	nna;					// New Note Action [0, 1, 2, 3]
	uint16	trkVers;				// Tracker version used to save [files only]
	uint8	ppSep;					// Pitch-Pan Seperation
	uint8	ppCenter;				// Pitch-Pan Center
	uint8	rVolVar;				// Random Volume Varations
	uint8	rPanVar;				// Random panning varations
	uint16	numSmp;					// Number of samples in instrument [files only]
	char	name[27];				// Instrument name
	uint8	blank01[6];
	uint16	sampTable[ITNOTECNT];	// Sample for each note [note / samp pairs]
	uint8	volEnv[200];			// Volume envelope (IT 1.x stuff)
	uint8	oldVolTick[ITENVCNT];	// Volume tick position (IT 1.x stuff)
	uint8	volNode[ITENVCNT];		// Amplitude of volume nodes
	uint16	volTick[ITENVCNT];		// Tick value of volume nodes
	int8	panNode[ITENVCNT];		// panEnv - node points
	uint16	panTick[ITENVCNT];		// Tick value of panning nodes
	int8	pitNode[ITENVCNT];		// pitchEnv - node points
	uint16	pitTick[ITENVCNT];		// Tick value of pitch nodes
} ITINSTHEADER;



// Unpacked note
typedef struct ITNOTE
{
	uint8	note;
	uint8	ins;
	uint8	volPan;
	uint8	cmd;
	uint8	inf;
} ITNOTE;



/******************************************************************************/
/* Intern tables                                                              */
/******************************************************************************/
// Table for porta-to-note command within volume/panning column
uint8 portaTable[] = {1, 4, 8, 16, 43, 64, 96, 128, 255};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a ImpulseTracker     */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikIT::CheckModule(APAgent_ConvertModule *convInfo)
{
	char buf[4];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(ITHEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(buf, 4);

	if ((buf[0] == 'I') && (buf[1] == 'M') && (buf[2] == 'P') && (buf[3] == 'M'))
		return (true);

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from ImpulseTracker to UniMod */
/*         structure.                                                         */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikIT::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, lp;
	INSTRUMENT *d;
	SAMPLE *q;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		mh        = NULL;
		posLookup = NULL;
		itPat     = NULL;
		mask      = NULL;
		last      = NULL;
		paraPtr   = NULL;

		// Allocate buffers
		if ((mh = new ITHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((posLookup = new uint8[256]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((itPat = new ITNOTE[200 * 64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((mask = new uint8[64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((last = new ITNOTE[64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(mh, 0, sizeof(ITHEADER));

		numTrk  = 0;
		filters = false;

		// Try to read module header
		file->Read_L_UINT32();		// Kill 4 byte header
		file->ReadString(mh->songName, 26);
		file->Read(mh->blank01, 2);

		mh->ordNum    = file->Read_L_UINT16();
		mh->insNum    = file->Read_L_UINT16();
		mh->smpNum    = file->Read_L_UINT16();
		mh->patNum    = file->Read_L_UINT16();
		mh->cwt       = file->Read_L_UINT16();
		mh->cmwt      = file->Read_L_UINT16();
		mh->flags     = file->Read_L_UINT16();
		mh->special   = file->Read_L_UINT16();

		mh->globVol   = file->Read_UINT8();
		mh->mixVol    = file->Read_UINT8();
		mh->initSpeed = file->Read_UINT8();
		mh->initTempo = file->Read_UINT8();
		mh->panSep    = file->Read_UINT8();
		mh->zeroByte  = file->Read_UINT8();
		mh->msgLength = file->Read_L_UINT16();
		mh->msgOffset = file->Read_L_UINT32();

		file->Read(mh->blank02, 4);
		file->Read(mh->panTable, 64);
		file->Read(mh->volTable, 64);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		of.songName.SetString(mh->songName, &charSet850);
		of.repPos     = 0;
		of.numPat     = mh->patNum;
		of.numIns     = mh->insNum;
		of.numSmp     = mh->smpNum;
		of.initSpeed  = mh->initSpeed;
		of.initTempo  = mh->initTempo;
		of.initVolume = mh->globVol;
		of.flags     |= UF_BGSLIDES | UF_ARPMEM;

		if (!(mh->flags & 1))
			of.flags |= UF_PANNING;

		of.bpmLimit = 32;

		if (mh->songName[25])
			of.numVoices = 1 + mh->songName[25];

		// Set the module type
		// 2.17 : IT 2.14p4
		// 2.16 : IT 2.14p3 with resonant filters
		// 2.15 : IT 2.14p3 (improved compression)
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_IT);

		if ((mh->cwt <= 0x219) && (mh->cwt >= 0x217))
		{
			if (mh->cmwt < 0x214)
				convInfo->modKind.LoadString(res, IDS_MIKC_NAME_IT_214P4);
			else
				convInfo->modKind.LoadString(res, IDS_MIKC_NAME_IT_214P4_COMP);
		}
		else
		{
			if (mh->cwt >= 0x215)
			{
				if (mh->cmwt < 0x214)
					convInfo->modKind.LoadString(res, IDS_MIKC_NAME_IT_214P3);
				else
					convInfo->modKind.LoadString(res, IDS_MIKC_NAME_IT_214P3_COMP);
			}
			else
			{
				if (mh->cmwt < 0x214)
					convInfo->modKind.Format(res, IDS_MIKC_NAME_IT, (mh->cwt >> 8), ((mh->cwt >> 4) & 15) * 10 + (mh->cwt & 15));
				else
					convInfo->modKind.Format(res, IDS_MIKC_NAME_IT_COMP, (mh->cwt >> 8), ((mh->cwt >> 4) & 15) * 10 + (mh->cwt & 15));
			}
		}

		if (mh->flags & 8)
			of.flags  |= UF_XMPERIODS | UF_LINEAR;

		if ((mh->cwt >= 0x106) && (mh->flags & 16))
			oldEffect = S3MIT_OLDSTYLE;
		else
			oldEffect = 0;

		// Set panning positions
		if (mh->flags & 1)
		{
			for (t = 0; t < 64; t++)
			{
				mh->panTable[t] &= 0x7f;

				if (mh->panTable[t] < 64)
					of.panning[t] = mh->panTable[t] << 2;
				else
				{
					if (mh->panTable[t] == 64)
						of.panning[t] = 255;
					else
					{
						if (mh->panTable[t] == 100)
							of.panning[t] = PAN_SURROUND;
						else
						{
							if (mh->panTable[t] == 127)
								of.panning[t] = PAN_CENTER;
							else
							{
//								of.panning[t] = (mh->panTable[t] - 128) << 2;	// TN: Added support for disabled channels
								ShowError(IDS_MIKC_ERR_LOADING_HEADER);
								throw PUserException();
							}
						}
					}
				}
			}
		}
		else
		{
			for (t = 0; t < 64; t++)
				of.panning[t] = PAN_CENTER;
		}

		// Set channel volumes
		memcpy(of.chanVol, mh->volTable, 64);

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

			if ((origPositions[t] > mh->patNum) && (origPositions[t] < 254))
				origPositions[t] = 255;
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		posLookupCnt = mh->ordNum;
		S3MIT_CreateOrders(curious);

		if ((paraPtr = new uint32[mh->insNum + mh->smpNum + of.numPat]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Read the instrument, sample and pattern parapointers
		file->ReadArray_L_UINT32s(paraPtr, mh->insNum + mh->smpNum + of.numPat);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Check for and load midi information for resonant filters
		if (mh->cmwt >= 0x216)
		{
			if (mh->special & 8)
			{
				LoadMidiConfiguration(file);

				if (file->IsEOF())
				{
					ShowError(IDS_MIKC_ERR_LOADING_HEADER);
					throw PUserException();
				}
			}
			else
				LoadMidiConfiguration(NULL);

			filters = true;
		}

		// Check for and load song comment
		if ((mh->special & 1) && (mh->cwt >= 0x104) && (mh->msgLength))
		{
			file->Seek(mh->msgOffset, PFile::pSeekBegin);
			ReadComment(file, mh->msgLength, true);
		}

		if (!(mh->flags & 4))
			of.numIns = of.numSmp;

		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if (!(AllocLinear()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Load all samples
		q = of.samples;
		for (t = 0; t < mh->smpNum; t++)
		{
			ITSAMPLE s;

			// Seek to sample position
			file->Seek(paraPtr[mh->insNum + t] + 4, PFile::pSeekBegin);

			// And load sample info
			file->ReadString(s.fileName, 12);

			s.zeroByte   = file->Read_UINT8();
			s.globVol    = file->Read_UINT8();
			s.flag       = file->Read_UINT8();
			s.volume     = file->Read_UINT8();

			file->ReadString(s.sampName, 26);

			s.convert    = file->Read_UINT8();
			s.panning    = file->Read_UINT8();
			s.length     = file->Read_L_UINT32();
			s.loopBeg    = file->Read_L_UINT32();
			s.loopEnd    = file->Read_L_UINT32();
			s.c5Spd      = file->Read_L_UINT32();
			s.susBegin   = file->Read_L_UINT32();
			s.susEnd     = file->Read_L_UINT32();
			s.sampOffset = file->Read_L_UINT32();
			s.vibSpeed   = file->Read_UINT8();
			s.vibDepth   = file->Read_UINT8();
			s.vibRate    = file->Read_UINT8();
			s.vibWave    = file->Read_UINT8();

			// Generate an error if c5Spd is > 512k, or samplelength > 256 megs
			// (Nothing would EVER be that high)
			if (file->IsEOF() || (s.c5Spd > 0x7ffffL) || (s.length > 0xfffffffUL))
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			// Reality check for sample loop information
			if ((s.flag & 16) && ((s.loopBeg > 0xfffffffUL) || (s.loopEnd > 0xfffffffUL)))
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			q->sampleName.SetString(s.sampName, &charSet850);
			q->speed      = s.c5Spd / 2;
			q->panning    = ((s.panning & 127) == 64) ? 255 : (s.panning & 127) << 2;
			q->length     = s.length;
			q->loopStart  = s.loopBeg;
			q->loopEnd    = s.loopEnd;
			q->volume     = s.volume;
			q->globVol    = s.globVol;
			q->seekPos    = s.sampOffset;

			// Convert speed to XM linear finetune
			if (of.flags & UF_LINEAR)
				q->speed = SpeedToFinetune(s.c5Spd, t);

			if (s.panning & 128)
				q->flags |= SF_OWNPAN;

			if (s.vibRate)
			{
				q->vibFlags |= AV_IT;
				q->vibType   = s.vibWave;
				q->vibSweep  = s.vibRate * 2;
				q->vibDepth  = s.vibDepth;
				q->vibRate   = s.vibSpeed;
			}

			if (s.flag & 2)
				q->flags |= SF_16BITS;

			if ((s.flag & 8) && (mh->cwt >= 0x214))
				q->flags |= SF_ITPACKED;

			if (s.flag & 16)
				q->flags |= SF_LOOP;

			if (s.flag & 64)
				q->flags |= SF_BIDI;

			if (mh->cwt >= 0x200)
			{
				if (s.convert & 1)
					q->flags |= SF_SIGNED;

				if (s.convert & 4)
					q->flags |= SF_DELTA;
			}

			q++;
		}

		// Load instruments if instrument mode flag enabled
		if (mh->flags & 4)
		{
			if (!(AllocInstruments()))
			{
				ShowError(IDS_MIKC_ERR_MEMORY);
				throw PUserException();
			}

			d = of.instruments;
			of.flags |= (UF_NNA | UF_INST);

			for (t = 0; t < mh->insNum; t++)
			{
				ITINSTHEADER ih;

				// Seek to instrument position
				file->Seek(paraPtr[t] + 4, PFile::pSeekBegin);

				// Load instrument info
				file->ReadString(ih.fileName, 12);
				ih.zeroByte = file->Read_UINT8();

				if (mh->cwt < 0x200)
				{
					// Load IT 1.xx inst header
					ih.volFlg    = file->Read_UINT8();
					ih.volBeg    = file->Read_UINT8();
					ih.volEnd    = file->Read_UINT8();
					ih.volSusBeg = file->Read_UINT8();
					ih.volSusEnd = file->Read_UINT8();
					file->Read_L_UINT16();
					ih.fadeOut   = file->Read_L_UINT16();
					ih.nna       = file->Read_UINT8();
					ih.dnc       = file->Read_UINT8();
				}
				else
				{
					// Read IT200+ header
					ih.nna      = file->Read_UINT8();
					ih.dct      = file->Read_UINT8();
					ih.dca      = file->Read_UINT8();
					ih.fadeOut  = file->Read_L_UINT16();
					ih.ppSep    = file->Read_UINT8();
					ih.ppCenter = file->Read_UINT8();
					ih.globVol  = file->Read_UINT8();
					ih.chanPan  = file->Read_UINT8();
					ih.rVolVar  = file->Read_UINT8();
					ih.rPanVar  = file->Read_UINT8();
				}

				ih.trkVers = file->Read_L_UINT16();
				ih.numSmp  = file->Read_UINT8();

				file->Read_UINT8();
				file->ReadString(ih.name, 26);
				file->Read(ih.blank01, 6);
				file->ReadArray_L_UINT16s(ih.sampTable, ITNOTECNT);

				if (mh->cwt < 0x200)
				{
					// Load IT 1xx volume envelope
					file->Read(ih.volEnv, 200);

					for (lp = 0; lp < ITENVCNT; lp++)
					{
						ih.oldVolTick[lp] = file->Read_UINT8();
						ih.volNode[lp]    = file->Read_UINT8();
					}
				}
				else
				{
					// Load IT 2xx volume, pan and pitch envelopes
#define IT_LoadEnvelope(name) \
					ih.name##Flg    = file->Read_UINT8(); \
					ih.name##Pts    = file->Read_UINT8(); \
					ih.name##Beg    = file->Read_UINT8(); \
					ih.name##End    = file->Read_UINT8(); \
					ih.name##SusBeg = file->Read_UINT8(); \
					ih.name##SusEnd = file->Read_UINT8(); \
					for (lp = 0; lp < ITENVCNT; lp++) \
					{ \
						ih.name##Node[lp] = file->Read_UINT8(); \
						ih.name##Tick[lp] = file->Read_L_UINT16(); \
					} \
					file->Read_UINT8();

					IT_LoadEnvelope(vol);
					IT_LoadEnvelope(pan);
					IT_LoadEnvelope(pit);
#undef IT_LoadEnvelope
				}

				if (file->IsEOF())
				{
					ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
					throw PUserException();
				}

				d->volFlg |= EF_VOLENV;
				d->insName.SetString(ih.name, &charSet850);
				d->nnaType = ih.nna & NNA_MASK;

				if (mh->cwt < 0x200)
				{
					d->volFade = ih.fadeOut << 6;

					if (ih.dnc)
					{
						d->dct = DCT_NOTE;
						d->dca = DCA_CUT;
					}

					if (ih.volFlg & 1)
						d->volFlg |= EF_ON;

					if (ih.volFlg & 2)
						d->volFlg |= EF_LOOP;

					if (ih.volFlg & 4)
						d->volFlg |= EF_SUSTAIN;

					// XM conversion of IT envelope array
					d->volBeg    = ih.volBeg;
					d->volEnd    = ih.volEnd;
					d->volSusBeg = ih.volSusBeg;
					d->volSusEnd = ih.volSusEnd;

					if (ih.volFlg & 1)
					{
						for (u = 0; u < ITENVCNT; u++)
						{
							if (ih.oldVolTick[d->volPts] != 0xff)
							{
								d->volEnv[d->volPts].val = (ih.volNode[d->volPts] << 2);
								d->volEnv[d->volPts].pos = ih.oldVolTick[d->volPts];
								d->volPts++;
							}
							else
								break;
						}
					}
				}
				else
				{
					d->panning = ((ih.chanPan & 127) == 64) ? 255 : (ih.chanPan & 127) << 2;

					if (!(ih.chanPan & 128))
						d->flags |= IF_OWNPAN;

					if (!(ih.ppSep & 128))
					{
						d->pitPanSep    = ih.ppSep << 2;
						d->pitPanCenter = ih.ppCenter;
						d->flags       |= IF_PITCHPAN;
					}

					d->globVol = ih.globVol >> 1;
					d->volFade = ih.fadeOut << 5;
					d->dct     = ih.dct;
					d->dca     = ih.dca;

					if (mh->cwt >= 0x204)
					{
						d->rVolVar = ih.rVolVar;
						d->rPanVar = ih.rPanVar;
					}

#define IT_ProcessEnvelope(name) \
					if (ih.name##Flg & 1) \
						d->name##Flg |= EF_ON; \
					if (ih.name##Flg & 2) \
						d->name##Flg |= EF_LOOP; \
					if (ih.name##Flg & 4) \
						d->name##Flg |= EF_SUSTAIN; \
					d->name##Pts    = ih.name##Pts; \
					d->name##Beg    = ih.name##Beg; \
					d->name##End    = ih.name##End; \
					d->name##SusBeg = ih.name##SusBeg; \
					d->name##SusEnd = ih.name##SusEnd; \
					for (u = 0; u < ih.name##Pts; u++) \
						d->name##Env[u].pos = ih.name##Tick[u]; \
					if ((d->name##Flg & EF_ON) && (d->name##Pts < 2)) \
						d->name##Flg &= ~EF_ON;

					IT_ProcessEnvelope(vol);
					for (u = 0; u < ih.volPts; u++)
						d->volEnv[u].val = (ih.volNode[u] << 2);

					IT_ProcessEnvelope(pan);
					for (u = 0; u < ih.panPts; u++)
						d->panEnv[u].val = ih.panNode[u] == 32 ? 255 : (ih.panNode[u] + 32) << 2;

					IT_ProcessEnvelope(pit);
					for (u = 0; u < ih.pitPts; u++)
						d->pitEnv[u].val = ih.pitNode[u] + 32;
#undef IT_ProcessEnvelope

					if (ih.pitFlg & 0x80)
					{
						// Filter envelopes not supported yet
						d->pitFlg &= ~EF_ON;
						ih.pitPts  = ih.pitBeg = ih.pitEnd = 0;
					}
				}

				for (u = 0; u < ITNOTECNT; u++)
				{
					d->sampleNote[u]   = (ih.sampTable[u] & 255);
					d->sampleNumber[u] = (ih.sampTable[u] >> 8) ? ((ih.sampTable[u] >> 8) - 1) : 0xffff;

					if (d->sampleNumber[u] >= of.numSmp)
						d->sampleNote[u] = 255;
					else
					{
						if (of.flags & UF_LINEAR)
						{
							int32 note = (int32)d->sampleNote[u] + noteIndex[d->sampleNumber[u]];
							d->sampleNote[u] = (note < 0) ? 0 : (note > 255 ? 255 : note);
						}
					}
				}

				d++;
			}
		}
		else
		{
			if (of.flags & UF_LINEAR)
			{
				if (!(AllocInstruments()))
				{
					ShowError(IDS_MIKC_ERR_MEMORY);
					throw PUserException();
				}

				d = of.instruments;
				of.flags |= UF_INST;

				for (t = 0; t < mh->smpNum; t++, d++)
				{
					for (u = 0; u < ITNOTECNT; u++)
					{
						if (d->sampleNumber[u] >= of.numSmp)
							d->sampleNote[u] = 255;
						else
						{
							int32 note = (int32)d->sampleNote[u] + noteIndex[d->sampleNumber[u]];
							d->sampleNote[u] = (note < 0) ? 0 : (note > 255 ? 255 : note);
						}
					}
				}
			}
		}

		// Figure out how many channels this song actually uses
		of.numChn = 0;
		memset(remap, -1, UF_MAXCHAN * sizeof(uint8));

		for (t = 0; t < of.numPat; t++)
		{
			uint16 packLen;

			// Seek to pattern position
			if (paraPtr[mh->insNum + mh->smpNum + t])	// 0 -> empty 64 row pattern
			{
				file->Seek(paraPtr[mh->insNum + mh->smpNum + t], PFile::pSeekBegin);
				file->Read_L_UINT16();

				// Read pattern length (# of rows)
				// Impulse Tracker never creates patterns with less than 32 rows,
				// but some other trackers do, so we only check for more than 256
				// rows
				packLen = file->Read_L_UINT16();

				if (packLen > 256)
				{
					ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
					throw PUserException();
				}

				file->Read_L_UINT32();
				GetNumChannels(file, packLen);
			}
		}

		// Give each of them a different number
		for (t = 0; t < UF_MAXCHAN; t++)
		{
			if (!remap[t])
				remap[t] = of.numChn++;
		}

		of.numTrk = of.numPat * of.numChn;
		if (of.numVoices)
			if (of.numVoices < of.numChn)
				of.numVoices = of.numChn;

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

		for (t = 0; t < of.numPat; t++)
		{
			uint16 packLen;

			// Seek to pattern position
			if (!(paraPtr[mh->insNum + mh->smpNum + t]))	// 0 -> empty 64 row pattern
			{
				of.pattRows[t] = 64;

				for (u = 0; u < of.numChn; u++)
				{
					int32 k;

					UniReset();

					for (k = 0; k < 64; k++)
						UniNewLine();

					of.tracks[numTrk++] = UniDup();
				}
			}
			else
			{
				file->Seek(paraPtr[mh->insNum + mh->smpNum + t], PFile::pSeekBegin);

				packLen        = file->Read_L_UINT16();
				of.pattRows[t] = file->Read_L_UINT16();
				file->Read_L_UINT32();

				ReadPattern(file, of.pattRows[t]);
			}
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	FreeLinear();

	delete mh;
	mh = NULL;

	delete[] posLookup;
	posLookup = NULL;

	delete[] itPat;
	itPat = NULL;

	delete[] mask;
	mask = NULL;

	delete[] last;
	last = NULL;

	delete[] paraPtr;
	paraPtr = NULL;

	delete[] origPositions;
	origPositions = NULL;

	return (retVal);
}



/******************************************************************************/
/* GetNumChannels() returns the number of channels used.                      */
/*                                                                            */
/* Because so many IT files have 64 channels as the set number used, but      */
/* really only use far less (usually from 8 to 24 still), I had to make this  */
/* function, which determines the number of channels that are actually USED   */
/* by a pattern.                                                              */
/*                                                                            */
/* NOTE: You must first seek to the file location of the pattern before       */
/*       calling this procedure.                                              */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*         "patRows" is the number of rows in the pattern.                    */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikIT::GetNumChannels(PFile *file, uint16 patRows)
{
	int32 row = 0, flag, ch;

	do
	{
		flag = file->Read_UINT8();

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (!flag)
			row++;
		else
		{
			ch = (flag - 1) & 63;
			remap[ch] = 0;

			if (flag & 128)
				mask[ch] = file->Read_UINT8();

			if (mask[ch] & 1)
				file->Read_UINT8();

			if (mask[ch] & 2)
				file->Read_UINT8();

			if (mask[ch] & 4)
				file->Read_UINT8();

			if (mask[ch] & 8)
			{
				file->Read_UINT8();
				file->Read_UINT8();
			}
		}
	}
	while (row < patRows);
}



/******************************************************************************/
/* ConvertTrack() convert one IT track to UniMod track.                       */
/*                                                                            */
/* Input:  "tr" is a pointer to the track to convert.                         */
/*         "numRows" is the number of rows in the track.                      */
/*                                                                            */
/* Output: Is a pointer to the UniMod track or NULL for an error.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
uint8 *MikIT::ConvertTrack(ITNOTE *tr, uint16 numRows)
{
	int32 t;
	uint8 note, ins, volPan;

	UniReset();

	for (t = 0; t < numRows; t++)
	{
		note   = tr[t * of.numChn].note;
		ins    = tr[t * of.numChn].ins;
		volPan = tr[t * of.numChn].volPan;

		if (note != 255)
		{
			if (note == 253)
				UniWriteByte(UNI_KEYOFF);
			else
			{
				if (note == 254)
				{
					UniPTEffect(0xc, -1, of.flags);	// Note cut command
					volPan = 255;
				}
				else
					UniNote(note);
			}
		}

		if ((ins) && (ins < 100))
			UniInstrument(ins - 1);
		else
		{
			if (ins == 253)
				UniWriteByte(UNI_KEYOFF);
			else
			{
				if (ins != 255)		// Crap
				{
					ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
					throw PUserException();
				}
			}
		}

		// Process volume / panning column
		// Volume / panning effects do NOT all share the same memory address yet.
		if (volPan <= 64)
			UniVolEffect(VOL_VOLUME, volPan);
		else
		{
			// Fine volume slide up (65-74) - A0 case
			if (volPan == 65)
				UniVolEffect(VOL_VOLSLIDE, 0);
			else
			{
				if (volPan <= 74)
					UniVolEffect(VOL_VOLSLIDE, 0x0f + ((volPan - 65) << 4));
				else
				{
					// Fine volume slide down (75-84) - B0 case
					if (volPan == 75)
						UniVolEffect(VOL_VOLSLIDE, 0);
					else
					{
						if (volPan <= 84)
							UniVolEffect(VOL_VOLSLIDE, 0xf0 + (volPan - 75));
						else
						{
							// Volume slide up (85-94)
							if (volPan <= 94)
								UniVolEffect(VOL_VOLSLIDE, ((volPan - 85) << 4));
							else
							{
								// Volume slide down (95-104)
								if (volPan <= 104)
									UniVolEffect(VOL_VOLSLIDE, (volPan - 95));
								else
								{
									// Pitch slide up (105-114)
									if (volPan <= 114)
										UniVolEffect(VOL_PITCHSLIDEDN, (volPan - 105));
									else
									{
										// Pitch slide down (115-124)
										if (volPan <= 124)
											UniVolEffect(VOL_PITCHSLIDEUP, (volPan - 115));
										else
										{
											// Crap
											if (volPan <= 127)
											{
												ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
												throw PUserException();
											}
											else
											{
												// Set panning (128-192)
												if (volPan <= 192)
													UniVolEffect(VOL_PANNING, ((volPan - 128) == 64) ? 255 : ((volPan - 128) << 2));
												else
												{
													// Portamento to note (193-202)
													if (volPan <= 202)
														UniVolEffect(VOL_PORTAMENTO, portaTable[volPan - 193]);
													else
													{
														// Vibrato (203-212)
														if (volPan <= 212)
															UniVolEffect(VOL_VIBRATO, (volPan - 203));
														else
														{
															// Crap
															if ((volPan != 239) && (volPan != 255))
															{
																ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
																throw PUserException();
															}
														}
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		S3MIT_ProcessCmd(tr[t * of.numChn].cmd, tr[t * of.numChn].inf, oldEffect | S3MIT_IT);
		UniNewLine();
	}

	return (UniDup());
}



/******************************************************************************/
/* ReadPattern() read one pattern.                                            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*         "patRows" is the number of rows in the pattern.                    */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikIT::ReadPattern(PFile *file, uint16 patRows)
{
	int32 row = 0, flag, ch, blah;
	ITNOTE *itt = itPat, dummy, *n, *l;

	memset(itt, 255, 200 * 64 * sizeof(ITNOTE));

	do
	{
		flag = file->Read_UINT8();

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (!flag)
		{
			itt = &itt[of.numChn];
			row++;
		}
		else
		{
			ch = remap[(flag - 1) & 63];

			if (ch != -1)
			{
				n = &itt[ch];
				l = &last[ch];
			}
			else
				n = l = &dummy;

			if (flag & 128)
				mask[ch] = file->Read_UINT8();

			if (mask[ch] & 1)
			{
				// Convert IT note off to internal note off
				if ((l->note = n->note = file->Read_UINT8()) == 255)
					l->note = n->note = 253;
			}

			if (mask[ch] & 2)
				l->ins = n->ins = file->Read_UINT8();

			if (mask[ch] & 4)
				l->volPan = n->volPan = file->Read_UINT8();

			if (mask[ch] & 8)
			{
				l->cmd = n->cmd = file->Read_UINT8();
				l->inf = n->inf = file->Read_UINT8();
			}

			if (mask[ch] & 16)
				n->note = l->note;

			if (mask[ch] & 32)
				n->ins = l->ins;

			if (mask[ch] & 64)
				n->volPan = l->volPan;

			if (mask[ch] & 128)
			{
				n->cmd = l->cmd;
				n->inf = l->inf;
			}
		}
	}
	while (row < patRows);

	for (blah = 0; blah < of.numChn; blah++)
		of.tracks[numTrk++] = ConvertTrack(&itPat[blah], patRows);
}



/******************************************************************************/
/* LoadMidiString() loads a midi string from the module.                      */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*         "dest" is where to store the string.                               */
/*                                                                            */
/* Output: Is the midi string.                                                */
/******************************************************************************/
void MikIT::LoadMidiString(PFile *file, char *dest)
{
	char *cur, *last;

	file->Read(dest, 32);
	cur = last = dest;

	// Remove blanks and uppercase all
	while (*last)
	{
		if (isalnum((int)*last))
			*(cur++) = toupper((int)*last);

		last++;
	}

	*cur = 0x00;
}



/******************************************************************************/
/* LoadMidiConfiguration() loads embedded midi information for resonant       */
/*      filters.                                                              */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/******************************************************************************/
void MikIT::LoadMidiConfiguration(PFile *file)
{
	int32 i;

	memset(filterMacros, 0, sizeof(filterMacros));
	memset(filterSettings, 0, sizeof(filterSettings));

	if (file)
	{
		// Information is embedded in file
		uint16 dat;
		char midiLine[33];

		dat = file->Read_L_UINT16();
		file->Seek(8 * dat + 0x120, PFile::pSeekCurrent);

		// Read midi macros
		for (i = 0; i < UF_MAXMACRO; i++)
		{
			LoadMidiString(file, midiLine);

			if ((!strncmp(midiLine, "F0F00", 5)) && ((midiLine[5] == '0') || (midiLine[5] == '1')))
				filterMacros[i] = (midiLine[5] - '0') | 0x80;
		}

		// Read standalone filters
		for (i = 0x80; i < 0x100; i++)
		{
			LoadMidiString(file, midiLine);

			if ((!strncmp(midiLine, "F0F00", 5)) && ((midiLine[5] == '0') || (midiLine[5] == '1')))
			{
				filterSettings[i].filter = (midiLine[5] - '0') | 0x80;
				dat = (midiLine[6]) ? (midiLine[6] - '0') : 0;

				if (midiLine[7])
					dat = (dat << 4) | (midiLine[7] - '0');

				filterSettings[i].inf = dat;
			}
		}
	}
	else
	{
		// Use default information
		filterMacros[0] = FILT_CUT;

		for (i = 0x80; i < 0x90; i++)
		{
			filterSettings[i].filter = FILT_RESONANT;
			filterSettings[i].inf    = (i & 0x7f) << 3;
		}
	}

	activeMacro = 0;
	for (i = 0; i < 0x80; i++)
	{
		filterSettings[i].filter = filterMacros[0];
		filterSettings[i].inf    = i;
	}
}
