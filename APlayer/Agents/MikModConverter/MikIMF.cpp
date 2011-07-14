/******************************************************************************/
/* MikModConverter IMF class.                                                 */
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
typedef struct IMFHEADER
{
	char	songName[33];
	uint16	ordNum;
	uint16	patNum;
	uint16	insNum;
	uint16	flags;
	uint8	initSpeed;
	uint8	initTempo;
	uint8	masterVol;
	uint8	masterMult;
	uint8	orders[256];
} IMFHEADER;



// Channel settings
typedef struct IMFCHANNEL
{
	char	name[13];
	uint8	chorus;
	uint8	reverb;
	uint8	pan;
	uint8	status;
} IMFCHANNEL;



// Instrument header
#define IMFNOTECNT	(10 * OCTAVE)
#define IMFENVCNT	(16 * 2)
#define IMF_SMPINCR	64

typedef struct IMFINSTHEADER
{
	char	name[33];
	uint8	what[IMFNOTECNT];
	uint16	volEnv[IMFENVCNT];
	uint16	panEnv[IMFENVCNT];
	uint16	pitEnv[IMFENVCNT];
	uint8	volPts;
	uint8	volSus;
	uint8	volBeg;
	uint8	volEnd;
	uint8	volFlg;
	uint8	panPts;
	uint8	panSus;
	uint8	panBeg;
	uint8	panEnd;
	uint8	panFlg;
	uint8	pitPts;
	uint8	pitSus;
	uint8	pitBeg;
	uint8	pitEnd;
	uint8	pitFlg;
	uint16	volFade;
	uint16	numSmp;
	uint32	signature;
} IMFINSTHEADER;



// Sample header
typedef struct IMFWAVHEADER
{
	char	sampleName[14];
	uint32	length;
	uint32	loopStart;
	uint32	loopEnd;
	uint32	sampleRate;
	uint8	volume;
	uint8	pan;
	uint8	flags;
} IMFWAVHEADER;



typedef struct IMFNOTE
{
	uint8	note;
	uint8	ins;
	uint8	eff1;
	uint8	dat1;
	uint8	eff2;
	uint8	dat2;
} IMFNOTE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a Imago Orpheus      */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikIMF::CheckModule(APAgent_ConvertModule *convInfo)
{
	char buf[4];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(IMFHEADER))
		return (false);

	// Now check the signature
	file->Seek(0x3c, PFile::pSeekBegin);
	file->Read(buf, 4);

	if ((buf[0] == 'I') && (buf[1] == 'M') && (buf[2] == '1') && (buf[3] == '0'))
		return (true);

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from Imago Orpheus to UniMod  */
/*         structure.                                                         */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikIMF::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, track = 0, oldNumSmp;
	IMFCHANNEL channels[32];
	INSTRUMENT *d;
	SAMPLE *q;
	IMFWAVHEADER *wh = NULL, *s = NULL;
	uint32 *nextWav = NULL;
	uint16 wavCnt = 0;
	uint8 id[4];
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		mh     = NULL;
		imfPat = NULL;

		// Allocate buffers
		if ((mh = new IMFHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((imfPat = new IMFNOTE[32 * 256]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(mh, 0, sizeof(IMFHEADER));
		memset(imfPat, 0, 32 * 256 * sizeof(IMFNOTE));

		// Try to read the module header
		file->ReadString(mh->songName, 32);

		mh->ordNum = file->Read_L_UINT16();
		mh->patNum = file->Read_L_UINT16();
		mh->insNum = file->Read_L_UINT16();
		mh->flags  = file->Read_L_UINT16();

		file->Seek(8, PFile::pSeekCurrent);

		mh->initSpeed  = file->Read_UINT8();
		mh->initTempo  = file->Read_UINT8();
		mh->masterVol  = file->Read_UINT8();
		mh->masterMult = file->Read_UINT8();

		file->Seek(64, PFile::pSeekBegin);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_IMF);
		convInfo->modKind.LoadString(res, IDS_MIKC_NAME_IMF);

		of.songName.SetString(mh->songName, &charSet850);
		of.numPat     = mh->patNum;
		of.numIns     = mh->insNum;
		of.repPos     = 0;
		of.initSpeed  = mh->initSpeed;
		of.initTempo  = mh->initTempo;
		of.initVolume = mh->masterVol << 1;
		of.flags     |= UF_INST | UF_ARPMEM | UF_PANNING;

		if (mh->flags & 1)
			of.flags |= UF_LINEAR;

		of.bpmLimit = 32;

		// Read channel information
		of.numChn = 0;
		memset(remap, -1, 32 * sizeof(uint8));

		for (t = 0; t < 32; t++)
		{
			file->ReadString(channels[t].name, 12);

			channels[t].chorus = file->Read_UINT8();
			channels[t].reverb = file->Read_UINT8();
			channels[t].pan    = file->Read_UINT8();
			channels[t].status = file->Read_UINT8();
		}

		// Bug in Imago Orpheus? If only channel 1 is enabled, in fact we
		// have to enable 16 channels
		if (!channels[0].status)
		{
			for (t = 1; t < 16; t++)
			{
				if (channels[t].status != 1)
					break;
			}

			if (t == 16)
			{
				for (t = 1; t < 16; t++)
					channels[t].status = 0;
			}
		}

		for (t = 0; t < 32; t++)
		{
			if (channels[t].status != 2)
				remap[t] = of.numChn++;
			else
				remap[t] = -1;
		}

		for (t = 0; t < 32; t++)
		{
			if (remap[t] != -1)
			{
				of.panning[remap[t]] = channels[t].pan;
				of.chanVol[remap[t]] = channels[t].status ? 0 : 64;
			}
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Read order list
		file->Read(mh->orders, 256);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		of.numPos = 0;
		for (t = 0; t < mh->ordNum; t++)
		{
			if (mh->orders[t] != 0xff)
				of.numPos++;
		}

		if (!(AllocPositions(of.numPos)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = u = 0; t < mh->ordNum; t++)
		{
			if (mh->orders[t] != 0xff)
				of.positions[u++] = mh->orders[t];
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
			int32 size;
			uint16 rows;

			size = (int32)file->Read_L_UINT16();
			rows = file->Read_L_UINT16();

			if ((rows > 256) || (size < 4))
			{
				ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
				throw PUserException();
			}

			of.pattRows[t] = rows;
			ReadPattern(file, size - 4, rows);

			for (u = 0; u < of.numChn; u++)
			{
				if (!(of.tracks[track++] = ConvertTrack(&imfPat[u * 256], rows)))
				{
					ShowError(IDS_MIKC_ERR_INITIALIZE);
					throw PUserException();
				}
			}
		}

		// Load instruments
		if (!(AllocInstruments()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		d = of.instruments;
		for (oldNumSmp = t = 0; t < of.numIns; t++)
		{
			IMFINSTHEADER ih;

			memset(d->sampleNumber, 0xff, INSTNOTES * sizeof(uint16));

			// Read instrument header
			file->ReadString(ih.name, 32);
			d->insName.SetString(ih.name, &charSet850);

			file->Read(ih.what, IMFNOTECNT);
			file->Seek(8, PFile::pSeekCurrent);

			file->ReadArray_L_UINT16s(ih.volEnv, IMFENVCNT);
			file->ReadArray_L_UINT16s(ih.panEnv, IMFENVCNT);
			file->ReadArray_L_UINT16s(ih.pitEnv, IMFENVCNT);

#define IMF_FinishLoadingEnvelope(name) \
			ih.name##Pts = file->Read_UINT8(); \
			ih.name##Sus = file->Read_UINT8(); \
			ih.name##Beg = file->Read_UINT8(); \
			ih.name##End = file->Read_UINT8(); \
			ih.name##Flg = file->Read_UINT8(); \
			file->Read_UINT8(); \
			file->Read_UINT8(); \
			file->Read_UINT8();

			IMF_FinishLoadingEnvelope(vol);
			IMF_FinishLoadingEnvelope(pan);
			IMF_FinishLoadingEnvelope(pit);

			ih.volFade = file->Read_L_UINT16();
			ih.numSmp  = file->Read_L_UINT16();

			file->Read(id, 4);

			// Looks like Imago Orpheus forgets the signature for empty
			// instruments following a multi-sample instrument...
			if (memcmp(id, "II10", 4) && (oldNumSmp && memcmp(id, "\x0\x0\x0\x0", 4)))
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			oldNumSmp = ih.numSmp;

			if ((ih.numSmp > 16) || (ih.volPts > IMFENVCNT / 2) ||
				(ih.panPts > IMFENVCNT / 2) || (ih.pitPts > IMFENVCNT / 2) || (file->IsEOF()))
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			for (u = 0; u < IMFNOTECNT; u++)
				d->sampleNumber[u] = ih.what[u] > ih.numSmp ? 0xffff : ih.what[u] + of.numSmp;

			d->volFade = ih.volFade;

#define IMF_ProcessEnvelope(name) \
			for (u = 0; u < (IMFENVCNT >> 1); u++) \
			{ \
				d->name##Env[u].pos = ih.name##Env[u << 1]; \
				d->name##Env[u].val = ih.name##Env[(u << 1) + 1]; \
			} \
			if (ih.name##Flg & 1) \
				d->name##Flg |= EF_ON; \
			if (ih.name##Flg & 2) \
				d->name##Flg |= EF_SUSTAIN; \
			if (ih.name##Flg & 4) \
				d->name##Flg |= EF_LOOP; \
			d->name##SusBeg = d->name##SusEnd = ih.name##Sus; \
			d->name##Beg    = ih.name##Beg; \
			d->name##End    = ih.name##End; \
			d->name##Pts    = ih.name##Pts; \
			if ((d->name##Flg & EF_ON) && (d->name##Pts < 2)) \
				d->name##Flg &= ~EF_ON;

			IMF_ProcessEnvelope(vol);
			IMF_ProcessEnvelope(pan);
			IMF_ProcessEnvelope(pit);
#undef IMF_ProcessEnvelope

			if (ih.pitFlg & 1)
				d->pitFlg &= ~EF_ON;

			// Gather sample information
			for (u = 0; u < ih.numSmp; u++, s++)
			{
				// Allocate more room for sample information if necessary
				if ((of.numSmp + u) == wavCnt)
				{
					uint32 *newNextWav;
					IMFWAVHEADER *newWave;

					wavCnt += IMF_SMPINCR;

					if ((newNextWav = new uint32[wavCnt]) == NULL)
					{
						ShowError(IDS_MIKC_ERR_MEMORY);
						throw PUserException();
					}

					if (nextWav)
						memcpy(newNextWav, nextWav, (wavCnt - IMF_SMPINCR) * sizeof(uint32));

					delete[] nextWav;
					nextWav = newNextWav;

					if ((newWave = new IMFWAVHEADER[wavCnt]) == NULL)
					{
						ShowError(IDS_MIKC_ERR_MEMORY);
						throw PUserException();
					}

					if (wh)
						memcpy(newWave, wh, (wavCnt - IMF_SMPINCR) * sizeof(IMFWAVHEADER));

					delete[] wh;
					wh = newWave;

					s = wh + (wavCnt - IMF_SMPINCR);
				}

				file->ReadString(s->sampleName, 13);
				file->Read_UINT8();
				file->Read_UINT8();
				file->Read_UINT8();

				s->length     = file->Read_L_UINT32();
				s->loopStart  = file->Read_L_UINT32();
				s->loopEnd    = file->Read_L_UINT32();
				s->sampleRate = file->Read_L_UINT32();
				s->volume     = file->Read_UINT8() & 0x7f;
				s->pan        = file->Read_UINT8();

				file->Seek(14, PFile::pSeekCurrent);
				s->flags      = file->Read_UINT8();

				file->Seek(11, PFile::pSeekCurrent);
				file->Read(id, 4);
				if ((memcmp(id, "IS10", 4)) && (memcmp(id, "IW10", 4)) || (file->IsEOF()))
				{
					ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
					throw PUserException();
				}

				nextWav[of.numSmp + u] = file->GetPosition();
				file->Seek(s->length, PFile::pSeekCurrent);
			}

			of.numSmp += ih.numSmp;
			d++;
		}

		// Sanity check
		if (!of.numSmp)
		{
			ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
			throw PUserException();
		}

		// Load samples
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

		q = of.samples;
		s = wh;
		for (u = 0; u < of.numSmp; u++, s++, q++)
		{
			q->sampleName.SetString(s->sampleName, &charSet850);
			q->length     = s->length;
			q->loopStart  = s->loopStart;
			q->loopEnd    = s->loopEnd;
			q->volume     = s->volume;
			q->speed      = s->sampleRate;

			if (of.flags & UF_LINEAR)
				q->speed = SpeedToFinetune(s->sampleRate << 1, u);

			q->panning    = s->pan;
			q->seekPos    = nextWav[u];

			q->flags     |= SF_SIGNED;

			if (s->flags & 0x01)
				q->flags |= SF_LOOP;

			if (s->flags & 0x02)
				q->flags |= SF_BIDI;

			if (s->flags & 0x08)
				q->flags |= SF_OWNPAN;

			if (s->flags & 0x04)
			{
				q->flags      |= SF_16BITS;
				q->length    >>= 1;
				q->loopStart >>= 1;
				q->loopEnd   >>= 1;
			}
		}

		d = of.instruments;
		s = wh;
		for (u = 0; u < of.numIns; u++, d++)
		{
			for (t = 0; t < IMFNOTECNT; t++)
			{
				if (d->sampleNumber[t] >= of.numSmp)
					d->sampleNote[t] = 255;
				else
				{
					if (of.flags & UF_LINEAR)
					{
						int32 note = (int32)d->sampleNote[u] + noteIndex[d->sampleNumber[u]];
						d->sampleNote[u] = (note < 0) ? 0 : (note > 255 ? 255 : note);
					}
					else
						d->sampleNote[t] = t;
				}
			}
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	FreeLinear();

	delete[] wh;
	delete[] nextWav;

	delete[] imfPat;
	imfPat = NULL;

	delete mh;
	mh = NULL;

	return (retVal);
}



/******************************************************************************/
/* ReadPattern() read one pattern.                                            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*         "size" is the size of the pattern.                                 */
/*         "rows" is the number of rows in the pattern.                       */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikIMF::ReadPattern(PFile *file, int32 size, uint16 rows)
{
	int32 row = 0, flag, ch;
	IMFNOTE *n, dummy;

	// Clear pattern data
	memset(imfPat, 255, 32 * 256 * sizeof(IMFNOTE));

	while ((size > 0) && (row < rows))
	{
		flag = file->Read_UINT8();
		size--;

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		if (flag)
		{
			ch = remap[flag & 31];

			if (ch != -1)
				n = &imfPat[256 * ch + row];
			else
				n = &dummy;

			if (flag & 32)
			{
				n->note = file->Read_UINT8();
				if (n->note >= 0xa0)		// Note off
					n->note = 0xa0;

				n->ins = file->Read_UINT8();
				size -= 2;
			}

			if (flag & 64)
			{
				n->eff2 = file->Read_UINT8();
				n->dat2 = file->Read_UINT8();
				size -= 2;
			}

			if (flag & 128)
			{
				n->eff1 = file->Read_UINT8();
				n->dat1 = file->Read_UINT8();
				size -= 2;
			}
		}
		else
			row++;
	}

	if ((size) || (row != rows))
	{
		ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
		throw PUserException();
	}
}



/******************************************************************************/
/* ProcessCmd() will process an effect and store the result in the UNI stream.*/
/*                                                                            */
/* Input:  "eff" is the effect number.                                        */
/*         "inf" is the effect data.                                          */
/******************************************************************************/
void MikIMF::ProcessCmd(uint8 eff, uint8 inf)
{
	if ((eff) && (eff != 255))
	{
		switch (eff)
		{
			// Set tempo
			case 0x01:
				UniEffect(UNI_S3MEFFECTA, inf);
				break;

			// Set BPM
			case 0x02:
				if (inf >= 0x20)
					UniEffect(UNI_S3MEFFECTT, inf);
				break;

			// Tone portamento
			case 0x03:
				UniEffect(UNI_ITEFFECTG, inf);
				break;

			// Porta + volslide
			case 0x04:
				UniEffect(UNI_ITEFFECTG, inf);
				UniEffect(UNI_S3MEFFECTD, 0);
				break;

			// Vibrato
			case 0x05:
				UniEffect(UNI_XMEFFECT4, inf);
				break;

			// Vibrato + volslide
			case 0x06:
				UniEffect(UNI_XMEFFECT6, inf);
				break;

			// Fine vibrato
			case 0x07:
				UniEffect(UNI_ITEFFECTU, inf);
				break;

			// Tremolo
			case 0x08:
				UniEffect(UNI_S3MEFFECTR, inf);
				break;

			// Arpeggio
			case 0x09:
				UniPTEffect(0x0, inf, of.flags);
				break;

			// Panning
			case 0x0a:
				UniPTEffect(0x8, (inf >= 128) ? 255 : (inf << 1), of.flags);
				break;

			// Pan slide
			case 0x0b:
				UniEffect(UNI_XMEFFECTP, inf);
				break;

			// Set channel volume
			case 0x0c:
				if (inf <= 64)
					UniPTEffect(0xc, inf, of.flags);
				break;

			// Volume slide
			case 0x0d:
				UniEffect(UNI_S3MEFFECTD, inf);
				break;

			// Fine volume slide
			case 0x0e:
				if (inf)
				{
					if (inf >> 4)
						UniEffect(UNI_S3MEFFECTD, 0x0f | inf);
					else
						UniEffect(UNI_S3MEFFECTD, 0xf0 | inf);
				}
				else
					UniEffect(UNI_S3MEFFECTD, 0);

				break;

			// Set finetune
			case 0x0f:
				UniPTEffect(0xe, 0x50 | (inf >> 4), of.flags);
				break;

			// Note slide up/down
			case 0x10:
			case 0x11:
				break;

			// Slide up
			case 0x12:
				UniEffect(UNI_S3MEFFECTF, inf);
				break;

			// Slide down
			case 0x13:
				UniEffect(UNI_S3MEFFECTE, inf);
				break;

			// Fine slide up
			case 0x14:
				if (inf)
				{
					if (inf < 0x40)
						UniEffect(UNI_S3MEFFECTF, 0xe0 | (inf >> 2));
					else
						UniEffect(UNI_S3MEFFECTF, 0xf0 | (inf >> 4));
				}
				else
					UniEffect(UNI_S3MEFFECTF, 0);

				break;

			// Fine slide down
			case 0x15:
				if (inf)
				{
					if (inf < 0x40)
						UniEffect(UNI_S3MEFFECTE, 0xe0 | (inf >> 2));
					else
						UniEffect(UNI_S3MEFFECTE, 0xf0 | (inf >> 4));
				}
				else
					UniEffect(UNI_S3MEFFECTE, 0);

				break;

			// 0x16: Set filter cutoff (awe32)
			// 0x17: Filter side + resonance (awe32)
			case 0x16:
			case 0x17:
				break;

			// Sample offset
			case 0x18:
				UniPTEffect(0x9, inf, of.flags);
				break;

			// Set fine sample offset
			case 0x19:
				break;

			// Keyoff
			case 0x1a:
				UniWriteByte(UNI_KEYOFF);
				break;

			// Retrig
			case 0x1b:
				UniEffect(UNI_S3MEFFECTQ, inf);
				break;

			// Tremor
			case 0x1c:
				UniEffect(UNI_S3MEFFECTI, inf);
				break;

			// Position jump
			case 0x1d:
				UniPTEffect(0xb, inf, of.flags);
				break;

			// Pattern break
			case 0x1e:
				UniPTEffect(0xd, (inf >> 4) * 10 + (inf & 0xf), of.flags);
				break;

			// Set master volume
			case 0x1f:
				if (inf <= 64)
					UniEffect(UNI_XMEFFECTG, inf << 1);
				break;

			// Master volume slide
			case 0x20:
				UniEffect(UNI_XMEFFECTH, inf);
				break;

			// Extended effects
			case 0x21:
				switch (inf >> 4)
				{
					// 1: Set filter
					// 5: Vibrato waveform
					// 8: Tremolo waveform
					case 0x1:
					case 0x5:
					case 0x8:
						UniPTEffect(0xe, inf - 0x10, of.flags);
						break;

					// Pattern loop
					case 0xa:
						UniPTEffect(0xe, 0x60 | (inf & 0xf), of.flags);
						break;

					// Pattern delay
					case 0xb:
						UniPTEffect(0xe, 0xe0 | (inf & 0xf), of.flags);
						break;

					// 3: Glissando
					// c: Note cut
					// d: Note delay
					// f: Invert loop
					case 0x3:
					case 0xc:
					case 0xd:
					case 0xf:
						UniPTEffect(0xe, inf, of.flags);
						break;

					// Ignore envelope
					case 0xe:
						UniEffect(UNI_ITEFFECTS0, 0x77);	// Vol
						UniEffect(UNI_ITEFFECTS0, 0x79);	// Pan
						UniEffect(UNI_ITEFFECTS0, 0x7b);	// Pit
						break;
				}
				break;

			// 0x22 Chorus (awe32)
			// 0x23 Reverb (awe32)
			case 0x22:
			case 0x23:
				break;
		}
	}
}



/******************************************************************************/
/* ConvertTrack() convert one IMF track to UniMod track.                      */
/*                                                                            */
/* Input:  "tr" is a pointer to the track to convert.                         */
/*         "rows" is the number of rows in the track.                         */
/*                                                                            */
/* Output: Is a pointer to the UniMod track or NULL for an error.             */
/******************************************************************************/
uint8 *MikIMF::ConvertTrack(IMFNOTE *tr, uint16 rows)
{
	int32 t;
	uint8 note, ins;

	UniReset();

	for (t = 0; t < rows; t++)
	{
		note = tr[t].note;
		ins  = tr[t].ins;

		if ((ins) && (ins != 255))
			UniInstrument(ins - 1);

		if (note != 255)
		{
			if (note == 0xa0)
			{
				UniPTEffect(0xc, 0, of.flags);	// Note cut

				if (tr[t].eff1 == 0x0c)
					tr[t].eff1 = 0;

				if (tr[t].eff2 == 0x0c)
					tr[t].eff2 = 0;
			}
			else
				UniNote(((note >> 4) * OCTAVE) + (note & 0xf));
		}

		ProcessCmd(tr[t].eff1, tr[t].dat1);
		ProcessCmd(tr[t].eff2, tr[t].dat2);
		UniNewLine();
	}

	return (UniDup());
}
