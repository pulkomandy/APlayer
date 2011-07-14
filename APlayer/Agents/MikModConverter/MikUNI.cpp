/******************************************************************************/
/* MikModConverter UNI class.                                                 */
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
#include "PString.h"
#include "PFile.h"

// Agent headers
#include "MikConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Externs                                                                    */
/******************************************************************************/
extern uint16 uniOperands[UNI_LAST];



/******************************************************************************/
/* Intern structures                                                          */
/******************************************************************************/
typedef struct UNIHEADER
{
	char	id[4];
	uint8	numChn;
	uint16	numPos;
	uint16	repPos;
	uint16	numPat;
	uint16	numTrk;
	uint16	numIns;
	uint16	numSmp;
	uint8	initSpeed;
	uint8	initTempo;
	uint8	initVolume;
	uint16	flags;
	uint8	numVoices;
	uint16	bpmLimit;

	uint8	positions[256];
	uint8	panning[32];
} UNIHEADER;



typedef struct UNISMP05
{
	uint16	c2Spd;
	uint16	transpose;
	uint8	volume;
	uint8	panning;
	uint32	length;
	uint32	loopStart;
	uint32	loopEnd;
	uint16	flags;
	PString	sampleName;
	uint8	vibType;
	uint8	vibSweep;
	uint8	vibDepth;
	uint8	vibRate;
} UNISMP05;



#define UNI_SMPINCR		64



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a MikMod module.     */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store the information needed.                                      */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
bool MikUNI::CheckModule(APAgent_ConvertModule *convInfo)
{
	char id[6];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(UNIHEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(id, 6);

	// UniMod created by MikCvt
	if ((id[0] == 'U') && (id[1] == 'N') && (id[2] == '0'))
	{
		if ((id[3] >= '4') && (id[3] <= '6'))
			return (true);
	}

	// UniMod created by APlayer
	if ((id[0] == 'A') && (id[1] == 'P') && (id[2] == 'U') && (id[3] == 'N') && (id[4] == 1))
	{
		if ((id[5] >= 1) && (id[5] <= 5))
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from UniMod to UniMod         */
/*         structure.                                                         */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
ap_result MikUNI::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t;
	PString modType, oldType;
	char *oldStr;
	INSTRUMENT *d;
	SAMPLE *q;
	UNIHEADER mh;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		wh = NULL;
		s  = NULL;

		// Read module header
		file->Read(mh.id, 4);
		if (mh.id[3] != 'N')
			uniVersion = mh.id[3] - '0';
		else
			uniVersion = 0x100;

		if (uniVersion >= 6)
		{
			if (uniVersion == 6)
				file->Read_UINT8();
			else
				uniVersion = file->Read_B_UINT16();

			mh.flags      = file->Read_B_UINT16();
			mh.numChn     = file->Read_UINT8();
			mh.numVoices  = file->Read_UINT8();
			mh.numPos     = file->Read_B_UINT16();
			mh.numPat     = file->Read_B_UINT16();
			mh.numTrk     = file->Read_B_UINT16();
			mh.numIns     = file->Read_B_UINT16();
			mh.numSmp     = file->Read_B_UINT16();
			mh.repPos     = file->Read_B_UINT16();
			mh.initSpeed  = file->Read_UINT8();
			mh.initTempo  = file->Read_UINT8();
			mh.initVolume = file->Read_UINT8();

			if (uniVersion >= 0x106)
				mh.bpmLimit = file->Read_B_UINT16();
			else
				mh.bpmLimit = 32;

			mh.flags     &= UF_XMPERIODS | UF_LINEAR | UF_INST | UF_NNA;
			mh.flags     |= UF_PANNING;
		}
		else
		{
			mh.numChn     = file->Read_UINT8();
			mh.numPos     = file->Read_L_UINT16();
			mh.repPos     = (uniVersion == 5) ? file->Read_L_UINT16() : 0;
			mh.numPat     = file->Read_L_UINT16();
			mh.numTrk     = file->Read_L_UINT16();
			mh.numIns     = file->Read_L_UINT16();
			mh.initSpeed  = file->Read_UINT8();
			mh.initTempo  = file->Read_UINT8();

			file->Read(mh.positions, 256);
			file->Read(mh.panning, 32);

			mh.flags      = file->Read_UINT8();
			mh.bpmLimit   = 32;

			mh.flags     &= UF_XMPERIODS | UF_LINEAR;
			mh.flags     |= UF_INST | UF_NOWRAP | UF_PANNING;
		}

		// Set module parameters
		of.flags     = mh.flags;
		of.numChn    = mh.numChn;
		of.numPos    = mh.numPos;
		of.numPat    = mh.numPat;
		of.numTrk    = mh.numTrk;
		of.numIns    = mh.numIns;
		of.repPos    = mh.repPos;
		of.initSpeed = mh.initSpeed;
		of.initTempo = mh.initTempo;

		if (mh.bpmLimit)
			of.bpmLimit = mh.bpmLimit;
		else
		{
			// Be bug-compatible with older releases
			of.bpmLimit = 32;
		}

		of.songName  = file->ReadString(&charSet850);

		if (uniVersion < 0x102)
			oldType = file->ReadString(&charSet850);	// Read tracker used

		if (!oldType.IsEmpty())
		{
			modType.Format("%s (was %s)", (uniVersion >= 0x100) ? "APlayer" : "MikCvt2", (oldStr = oldType.GetString()));
			oldType.FreeBuffer(oldStr);
		}
		else
			modType = (uniVersion >= 0x100) ? "APlayer" : "MikCvt3";

		convInfo->modKind  = modType;
		convInfo->fileType = "audio/x-mikmod";

		of.comment = file->ReadString(&charSet850);

		if (uniVersion >= 6)
		{
			of.numVoices  = mh.numVoices;
			of.initVolume = mh.initVolume;
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Positions
		if (!(AllocPositions(of.numPos)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if (uniVersion >= 6)
		{
			if (uniVersion >= 0x100)
				file->ReadArray_B_UINT16s(of.positions, of.numPos);
			else
			{
				for (t = 0; t < of.numPos; t++)
					of.positions[t] = file->Read_UINT8();
			}

			file->ReadArray_B_UINT16s(of.panning, of.numChn);
			file->Read(of.chanVol, of.numChn);
		}
		else
		{
			if ((mh.numPos > 256) || (mh.numChn > 32))
			{
				ShowError(IDS_MIKC_ERR_LOADING_HEADER);
				throw PUserException();
			}

			for (t = 0; t < of.numPos; t++)
				of.positions[t] = mh.positions[t];

			for (t = 0; t < of.numChn; t++)
				of.panning[t] = mh.panning[t];
		}

		// Convert the 'end of song' pattern code if necessary
		if (uniVersion < 0x106)
		{
			for (t = 0; t < of.numPos; t++)
			{
				if (of.positions[t] == 255)
					of.positions[t] = LAST_PATTERN;
			}
		}

		// Instruments and samples
		if (uniVersion >= 6)
		{
			of.numSmp = mh.numSmp;

			if (!(AllocSamples()))
			{
				ShowError(IDS_MIKC_ERR_MEMORY);
				throw PUserException();
			}

			LoadSmp6(file);

			if (of.flags & UF_INST)
			{
				if (!(AllocInstruments()))
				{
					ShowError(IDS_MIKC_ERR_MEMORY);
					throw PUserException();
				}

				LoadInstr6(file);
			}
		}
		else
		{
			if (!(AllocInstruments()))
			{
				ShowError(IDS_MIKC_ERR_MEMORY);
				throw PUserException();
			}

			LoadInstr5(file);

			if (!(AllocSamples()))
			{
				ShowError(IDS_MIKC_ERR_MEMORY);
				throw PUserException();
			}

			LoadSmp5();

			// Check if the original file had no instruments
			if (of.numSmp == of.numIns)
			{
				for (t = 0, d = of.instruments; t < of.numIns; t++, d++)
				{
					int32 u;

					if ((d->volPts) || (d->panPts) || (d->globVol != 64))
						break;

					for (u = 0; u < 96; u++)
						if ((d->sampleNumber[u] != t) || (d->sampleNote[u] != u))
							break;

					if (u != 96)
						break;
				}

				if (t == of.numIns)
				{
					of.flags &= ~UF_INST;
					of.flags &= ~UF_NOWRAP;

					for (t = 0, d = of.instruments, q = of.samples; t < of.numIns; t++, d++, q++)
					{
						q->sampleName = d->insName;
						d->insName.MakeEmpty();
					}
				}
			}
		}

		// Patterns
		if (!(AllocPatterns()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if (uniVersion >= 6)
		{
			file->ReadArray_B_UINT16s(of.pattRows, of.numPat);
			file->ReadArray_B_UINT16s(of.patterns, of.numPat * of.numChn);
		}
		else
		{
			file->ReadArray_L_UINT16s(of.pattRows, of.numPat);
			file->ReadArray_L_UINT16s(of.patterns, of.numPat * of.numChn);
		}

		// Tracks
		if (!(AllocTracks()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = 0; t < of.numTrk; t++)
		{
			if (!(of.tracks[t] = ReadTrack(file)))
			{
				ShowError(IDS_MIKC_ERR_LOADING_TRACK);
				throw PUserException();
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
	s = NULL;

	return (retVal);
}



/******************************************************************************/
/* ReadTrack() read one track and return a pointer to it.                     */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikUNI::ReadTrack(PFile *file)
{
	uint8 *t;
	uint16 len;
	int32 cur = 0, chunk;

	if (uniVersion >= 6)
		len = file->Read_B_UINT16();
	else
		len = file->Read_L_UINT16();

	if (!len)
		return (NULL);

	t = new uint8[len];
	if (t == NULL)
		return (NULL);

	file->Read(t, len);

	// Check if the track is correct
	while (true)
	{
		chunk = t[cur++];
		if (!chunk)
			break;

		chunk = (chunk & 0x1f) - 1;
		while (chunk > 0)
		{
			int32 opcode, opLen;

			if (cur >= len)
			{
				delete[] t;
				return (NULL);
			}

			opcode = t[cur];

			// Remap opcodes
			if (uniVersion <= 5)
			{
				if (opcode > 29)
				{
					delete[] t;
					return (NULL);
				}

				switch (opcode)
				{
					// UNI_NOTE .. UNI_S3MEFFECTQ are the same
					case 25:
						opcode = UNI_S3MEFFECTT;
						break;

					case 26:
						opcode = UNI_XMEFFECTA;
						break;

					case 27:
						opcode = UNI_XMEFFECTG;
						break;

					case 28:
						opcode = UNI_XMEFFECTH;
						break;

					case 29:
						opcode = UNI_XMEFFECTP;
						break;
				}
			}
			else
			{
				// APlayer < 1.05 does not have XMEFFECT6
				if ((opcode >= UNI_XMEFFECT6) && (uniVersion < 0x105))
					opcode++;

				// APlayer < 1.03 does not have ITEFFECTT
				if ((opcode >= UNI_ITEFFECTT) && (uniVersion < 0x103))
					opcode++;

				// APlayer < 1.02 does not have ITEFFECTZ
				if ((opcode >= UNI_ITEFFECTZ) && (uniVersion < 0x102))
					opcode++;
			}

			if ((!opcode) || (opcode >= UNI_LAST))
			{
				delete[] t;
				return (NULL);
			}

			t[cur] = opcode;
			opLen  = uniOperands[opcode] + 1;
			cur   += opLen;
			chunk -= opLen;
		}

		if ((chunk < 0) || (cur >= len))
		{
			delete[] t;
			return (NULL);
		}
	}

	return (t);
}



/******************************************************************************/
/* LoadSmp6() read all the samples in version 6 format.                       */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikUNI::LoadSmp6(PFile *file)
{
	int32 t;
	SAMPLE *s;

	s = of.samples;
	for (t = 0; t < of.numSmp; t++, s++)
	{
		int32 flags;

		flags    = file->Read_B_UINT16();
		s->flags = 0;

		if (flags & 0x0004)
			s->flags |= SF_STEREO;

		if (flags & 0x0002)
			s->flags |= SF_SIGNED;

		if (flags & 0x0001)
			s->flags |= SF_16BITS;

		// Convert flags
		if (uniVersion >= 0x104)
		{
			if (flags & 0x2000)
				s->flags |= SF_UST_LOOP;

			if (flags & 0x1000)
				s->flags |= SF_OWNPAN;

			if (flags & 0x0800)
				s->flags |= SF_SUSTAIN;

			if (flags & 0x0400)
				s->flags |= SF_REVERSE;

			if (flags & 0x0200)
				s->flags |= SF_BIDI;

			if (flags & 0x0100)
				s->flags |= SF_LOOP;

			if (flags & 0x0020)
				s->flags |= SF_ITPACKED;

			if (flags & 0x0010)
				s->flags |= SF_DELTA;

			if (flags & 0x0008)
				s->flags |= SF_BIG_ENDIAN;
		}
		else if (uniVersion >= 0x102)
		{
			if (flags & 0x0800)
				s->flags |= SF_UST_LOOP;

			if (flags & 0x0400)
				s->flags |= SF_OWNPAN;

			if (flags & 0x0200)
				s->flags |= SF_SUSTAIN;

			if (flags & 0x0100)
				s->flags |= SF_REVERSE;

			if (flags & 0x0080)
				s->flags |= SF_BIDI;

			if (flags & 0x0040)
				s->flags |= SF_LOOP;

			if (flags & 0x0020)
				s->flags |= SF_ITPACKED;

			if (flags & 0x0010)
				s->flags |= SF_DELTA;

			if (flags & 0x0008)
				s->flags |= SF_BIG_ENDIAN;
		}
		else
		{
			if (flags & 0x0400)
				s->flags |= SF_UST_LOOP;

			if (flags & 0x0200)
				s->flags |= SF_OWNPAN;

			if (flags & 0x0100)
				s->flags |= SF_REVERSE;

			if (flags & 0x0080)
				s->flags |= SF_SUSTAIN;

			if (flags & 0x0040)
				s->flags |= SF_BIDI;

			if (flags & 0x0020)
				s->flags |= SF_LOOP;

			if (flags & 0x0010)
				s->flags |= SF_BIG_ENDIAN;

			if (flags & 0x0008)
				s->flags |= SF_DELTA;
		}

		s->speed     = file->Read_B_UINT32();
		s->volume    = file->Read_UINT8();
		s->panning   = file->Read_B_UINT16();
		s->length    = file->Read_B_UINT32();
		s->loopStart = file->Read_B_UINT32();
		s->loopEnd   = file->Read_B_UINT32();
		s->susBegin  = file->Read_B_UINT32();
		s->susEnd    = file->Read_B_UINT32();
		s->globVol   = file->Read_UINT8();
		s->vibFlags  = file->Read_UINT8();
		s->vibType   = file->Read_UINT8();
		s->vibSweep  = file->Read_UINT8();
		s->vibDepth  = file->Read_UINT8();
		s->vibRate   = file->Read_UINT8();

		s->sampleName = file->ReadString(&charSet850);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
			throw PUserException();
		}
	}
}



/******************************************************************************/
/* LoadInstr6() read all the instruments in version 6 format.                 */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikUNI::LoadInstr6(PFile *file)
{
	int32 t, w;
	INSTRUMENT *i;

	i = of.instruments;
	for (t = 0; t < of.numIns; t++, i++)
	{
		i->flags        = file->Read_UINT8();
		i->nnaType      = file->Read_UINT8();
		i->dca          = file->Read_UINT8();
		i->dct          = file->Read_UINT8();
		i->globVol      = file->Read_UINT8();
		i->panning      = file->Read_B_UINT16();
		i->pitPanSep    = file->Read_UINT8();
		i->pitPanCenter = file->Read_UINT8();
		i->rVolVar      = file->Read_UINT8();
		i->rPanVar      = file->Read_UINT8();
		i->volFade      = file->Read_B_UINT16();

#define UNI_LoadEnvelope6(name) \
		i->name##Flg    = file->Read_UINT8(); \
		i->name##Pts    = file->Read_UINT8(); \
		i->name##SusBeg = file->Read_UINT8(); \
		i->name##SusEnd = file->Read_UINT8(); \
		i->name##Beg    = file->Read_UINT8(); \
		i->name##End    = file->Read_UINT8(); \
		for (w = 0; w < (uniVersion >= 0x100 ? 32 : i->name##Pts); w++) \
		{ \
			i->name##Env[w].pos = file->Read_B_UINT16(); \
			i->name##Env[w].val = file->Read_B_UINT16(); \
		}

		UNI_LoadEnvelope6(vol);
		UNI_LoadEnvelope6(pan);
		UNI_LoadEnvelope6(pit);
#undef UNI_LoadEnvelope6

		if (uniVersion >= 0x103)
			file->ReadArray_B_UINT16s(i->sampleNumber, 120);
		else
		{
			for (w = 0; w < 120; w++)
				i->sampleNumber[w] = file->Read_UINT8();
		}

		file->Read(i->sampleNote, 120);

		i->insName = file->ReadString(&charSet850);

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
			throw PUserException();
		}
	}
}



/******************************************************************************/
/* LoadInstr5() read all the instruments in version 5 format.                 */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikUNI::LoadInstr5(PFile *file)
{
	INSTRUMENT *i;
	int32 t;
	uint16 wavCnt = 0;
	uint8 vibType, vibSweep, vibDepth, vibRate;

	i = of.instruments;
	for (of.numSmp = t = 0; t < of.numIns; t++, i++)
	{
		int32 u, numSmp;

		numSmp = file->Read_UINT8();

		memset(i->sampleNumber, 0xff, INSTNOTES * sizeof(uint16));
		for (u = 0; u < 96; u++)
			i->sampleNumber[u] = of.numSmp + file->Read_UINT8();

#define UNI_LoadEnvelope5(name) \
		i->name##Flg    = file->Read_UINT8(); \
		i->name##Pts    = file->Read_UINT8(); \
		i->name##SusBeg = file->Read_UINT8(); \
		i->name##SusEnd = i->name##SusBeg; \
		i->name##Beg    = file->Read_UINT8(); \
		i->name##End    = file->Read_UINT8(); \
		for (u = 0; u < 12; u++) \
		{ \
			i->name##Env[u].pos = file->Read_L_UINT16(); \
			i->name##Env[u].val = file->Read_L_UINT16(); \
		}

		UNI_LoadEnvelope5(vol);
		UNI_LoadEnvelope5(pan);
#undef UNI_LoadEnvelope5

		vibType  = file->Read_UINT8();
		vibSweep = file->Read_UINT8();
		vibDepth = file->Read_UINT8();
		vibRate  = file->Read_UINT8();

		i->volFade = file->Read_L_UINT16();
		i->insName = file->ReadString(&charSet850);

		for (u = 0; u < numSmp; u++, s++, of.numSmp++)
		{
			// Allocate more room for sample information if necessary
			if (of.numSmp + u == wavCnt)
			{
				UNISMP05 *newWh;

				newWh = new UNISMP05[wavCnt + UNI_SMPINCR];
				if (newWh == NULL)
				{
					ShowError(IDS_MIKC_ERR_MEMORY);
					throw PUserException();
				}

				if (wh != NULL)
				{
					for (int32 x = 0; x < wavCnt; x++)
						newWh[x] = wh[x];
				}

				delete[] wh;
				wh = newWh;

				s = wh + wavCnt;
				wavCnt += UNI_SMPINCR;
			}

			s->c2Spd      = file->Read_L_UINT16();
			s->transpose  = file->Read_UINT8();
			s->volume     = file->Read_UINT8();
			s->panning    = file->Read_UINT8();
			s->length     = file->Read_L_UINT32();
			s->loopStart  = file->Read_L_UINT32();
			s->loopEnd    = file->Read_L_UINT32();
			s->flags      = file->Read_L_UINT16();
			s->sampleName = file->ReadString(&charSet850);

			s->vibType    = vibType;
			s->vibSweep   = vibSweep;
			s->vibDepth   = vibDepth;
			s->vibRate    = vibRate;

			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
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
/* LoadSmp5() read all the samples in version 5 format.                       */
/******************************************************************************/
void MikUNI::LoadSmp5(void)
{
	int32 t, u;
	SAMPLE *q;
	INSTRUMENT *d;

	q = of.samples;
	s = wh;

	for (u = 0; u < of.numSmp; u++, q++, s++)
	{
		q->sampleName = s->sampleName;

		q->length     = s->length;
		q->loopStart  = s->loopStart;
		q->loopEnd    = s->loopEnd;
		q->volume     = s->volume;
		q->speed      = s->c2Spd;
		q->panning    = s->panning;
		q->vibType    = s->vibType;
		q->vibSweep   = s->vibSweep;
		q->vibDepth   = s->vibDepth;
		q->vibRate    = s->vibRate;

		// Convert flags
		q->flags = 0;

		if (s->flags & 128)
			q->flags |= SF_REVERSE;

		if (s->flags & 64)
			q->flags |= SF_SUSTAIN;

		if (s->flags & 32)
			q->flags |= SF_BIDI;

		if (s->flags & 16)
			q->flags |= SF_LOOP;

		if (s->flags & 8)
			q->flags |= SF_BIG_ENDIAN;

		if (s->flags & 4)
			q->flags |= SF_DELTA;

		if (s->flags & 2)
			q->flags |= SF_SIGNED;

		if (s->flags & 1)
			q->flags |= SF_16BITS;
	}

	d = of.instruments;
	s = wh;

	for (u = 0; u < of.numIns; u++, d++)
	{
		for (t = 0; t < INSTNOTES; t++)
			d->sampleNote[t] = (d->sampleNumber[t] >= of.numSmp) ? 255 : (t + s[d->sampleNumber[t]].transpose);
	}
}
