/******************************************************************************/
/* MikModConverter ULT class.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PFile.h"

// Agent headers
#include "MikConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Intern structures                                                          */
/******************************************************************************/
typedef struct ULTHEADER
{
	char	id[16];
	char	songTitle[33];
	uint8	reserved;
} ULTHEADER;



// Sample information
typedef struct ULTSAMPLE
{
	char	sampleName[33];
	char	dosName[13];
	int32	loopStart;
	int32	loopEnd;
	int32	sizeStart;
	int32	sizeEnd;
	uint8	volume;
	uint8	flags;
	uint16	speed;
	int16	fineTune;
} ULTSAMPLE;



typedef struct ULTEVENT
{
	uint8	note;
	uint8	sample;
	uint8	eff;
	uint8	dat1;
	uint8	dat2;
} ULTEVENT;



// Loader variables
#define ULTS_16BITS		4
#define ULTS_LOOP		8
#define ULTS_REVERSE	16



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a UltraTracker       */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikULT::CheckModule(APAgent_ConvertModule *convInfo)
{
	char buf[16];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(ULTHEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->ReadString(buf, 15);

	if (strncmp(buf, "MAS_UTrack_V00", 14) != 0)
		return (false);

	if ((buf[14] < '1') || (buf[14] > '4'))
		return (false);

	return (true);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from UltraTracker to UniMod   */
/*         structure.                                                         */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikULT::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, tracks = 0;
	SAMPLE *q;
	ULTSAMPLE s;
	ULTHEADER mh;
	ULTEVENT ev;
	uint8 nos, noc, nop;
	PFile *file = convInfo->moduleFile;

	// Try to read the module header
	file->ReadString(mh.id, 15);
	file->ReadString(mh.songTitle, 32);
	mh.reserved = file->Read_UINT8();

	if (file->IsEOF())
	{
		ShowError(IDS_MIKC_ERR_LOADING_HEADER);
		return (AP_ERROR);
	}

	// Initialize some of the structure with default values
	convInfo->modKind.Format(res, IDS_MIKC_NAME_ULT, mh.id[14] + 2);
	convInfo->fileType.LoadString(res, IDS_MIKC_MIME_ULT);
	of.initSpeed  = 6;
	of.initTempo  = 125;
	of.repPos     = 0;

	// Read songtext
	if ((mh.id[14] > '1') && (mh.reserved))
		ReadLinedComment(file, mh.reserved * 32, 32, false);

	nos = file->Read_UINT8();

	if (file->IsEOF())
	{
		ShowError(IDS_MIKC_ERR_LOADING_HEADER);
		return (AP_ERROR);
	}

	of.songName.SetString(mh.songTitle, &charSet850);
	of.numIns   = of.numSmp = nos;

	if (!(AllocSamples()))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		return (AP_ERROR);
	}

	q = of.samples;

	for (t = 0; t < nos; t++)
	{
		// Try to read sample info
		file->ReadString(s.sampleName, 32);
		file->ReadString(s.dosName, 12);

		s.loopStart = file->Read_L_UINT32();
		s.loopEnd   = file->Read_L_UINT32();
		s.sizeStart = file->Read_L_UINT32();
		s.sizeEnd   = file->Read_L_UINT32();
		s.volume    = file->Read_UINT8();
		s.flags     = file->Read_UINT8();
		s.speed     = (mh.id[14] >= '4') ? file->Read_L_UINT16() : 8363;
		s.fineTune  = file->Read_L_UINT16();

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
			return (AP_ERROR);
		}

		q->sampleName.SetString(s.sampleName, &charSet850);

		// The correct formula for the coefficient would be
		// pow(2, (double)s.fineTune / OCTAVE / 32768), but to avoid floating
		// point here, we'll use a first order approximation here.
		// 1 / 567290 == ln(2) / OCTAVE / 32768
		q->speed      = s.speed + s.speed * (((int32)s.speed * (int32)s.fineTune) / 567290);
		q->length     = s.sizeEnd - s.sizeStart;
		q->volume     = s.volume >> 2;
		q->loopStart  = s.loopStart;
		q->loopEnd    = s.loopEnd;
		q->flags      = SF_SIGNED;

		if (s.flags & ULTS_LOOP)
			q->flags |= SF_LOOP;

		if (s.flags & ULTS_16BITS)
		{
			// TN: Removed the s.sizeend and s.sizestart calculation,
			// because they will be overwritten when loading the next
			// sample right after this.
			q->flags      |= SF_16BITS;
			q->loopStart >>= 1;
			q->loopEnd   >>= 1;
		}

		q++;
	}

	if (!(AllocPositions(256)))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		return (AP_ERROR);
	}

	for (t = 0; t < 256; t++)
		of.positions[t] = file->Read_UINT8();

	for (t = 0; t < 256; t++)
	{
		if (of.positions[t] == 255)
		{
			of.positions[t] = LAST_PATTERN;
			break;
		}
	}

	of.numPos = t;

	noc = file->Read_UINT8();
	nop = file->Read_UINT8();

	of.numChn = ++noc;
	of.numPat = ++nop;
	of.numTrk = of.numChn * of.numPat;

	if (!(AllocTracks()))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		return (AP_ERROR);
	}

	if (!(AllocPatterns()))
	{
		ShowError(IDS_MIKC_ERR_MEMORY);
		return (AP_ERROR);
	}

	for (u = 0; u < of.numChn; u++)
	{
		for (t = 0; t < of.numPat; t++)
			of.patterns[(t * of.numChn) + u] = tracks++;
	}

	// Read pan position table for v1.5 and higher
	if (mh.id[14] >= '3')
	{
		for (t = 0; t < of.numChn; t++)
			of.panning[t] = file->Read_UINT8() << 4;

		of.flags |= UF_PANNING;
	}

	for (t = 0; t < of.numTrk; t++)
	{
		int32 rep, row = 0;

		UniReset();

		while (row < 64)
		{
			rep = ReadUltEvent(file, &ev);

			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_TRACK);
				return (AP_ERROR);
			}

			while (rep--)
			{
				uint8 eff;
				int32 offset;

				if (ev.sample)
					UniInstrument(ev.sample - 1);

				if (ev.note)
					UniNote(ev.note + 2 * OCTAVE - 1);

				// First effect - various fixes by Alexander Kerkhove and Thomas Neumann
				eff = ev.eff >> 4;
				switch (eff)
				{
					// Tone portamento
					case 0x3:
					{
						UniEffect(UNI_ITEFFECTG, ev.dat2);
						break;
					}

					// Special
					case 0x5:
						break;			// Not supported yet!

					// Sample offset
					case 0x9:
					{
						offset = (ev.dat2 << 8) | ((ev.eff & 0xf) == 9 ? ev.dat1 : 0);
						UniEffect(UNI_ULTEFFECT9, offset);
						break;
					}

					// Set panning
					case 0xb:
					{
						UniPTEffect(8, ev.dat2 * 0xf, of.flags);
						of.flags |= UF_PANNING;
						break;
					}

					// Set volume
					case 0xc:
					{
						UniPTEffect(eff, ev.dat2 >> 2, of.flags);
						break;
					}

					// All the other effects :)
					default:
					{
						UniPTEffect(eff, ev.dat2, of.flags);
						break;
					}
				}

				// Second effect
				eff = ev.eff & 0xf;
				switch (eff)
				{
					// Tone portamento
					case 0x3:
					{
						UniEffect(UNI_ITEFFECTG, ev.dat1);
						break;
					}

					// Special
					case 0x5:
						break;			// Not supported yet!

					// Sample offset
					case 0x9:
					{
						if ((ev.eff >> 4) != 9)
							UniEffect(UNI_ULTEFFECT9, ((uint16)ev.dat1) << 8);

						break;
					}

					// Set panning
					case 0xb:
					{
						UniPTEffect(8, ev.dat1 * 0xf, of.flags);
						of.flags |= UF_PANNING;
						break;
					}

					// Set volume
					case 0xc:
					{
						UniPTEffect(eff, ev.dat1 >> 2, of.flags);
						break;
					}

					// All the other effects :)
					default:
					{
						UniPTEffect(eff, ev.dat1, of.flags);
						break;
					}
				}

				UniNewLine();
				row++;
			}
		}

		if (!(of.tracks[t] = UniDup()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			return (AP_ERROR);
		}
	}

	return (AP_OK);
}



/******************************************************************************/
/* ReadUltEvent() will read one event from the memory into the structure      */
/*         given.                                                             */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*         "event" is a pointer to a ULTEVENT structure to store the data.    */
/*                                                                            */
/* Output: How many times this event has to be repeated.                      */
/******************************************************************************/
int32 MikULT::ReadUltEvent(PFile *file, ULTEVENT *event)
{
	uint8 flag, rep = 1;

	flag = file->Read_UINT8();

	if (flag == 0xfc)
	{
		rep         = file->Read_UINT8();
		event->note = file->Read_UINT8();
	}
	else
		event->note = flag;

	event->sample = file->Read_UINT8();
	event->eff    = file->Read_UINT8();
	event->dat1   = file->Read_UINT8();
	event->dat2   = file->Read_UINT8();

	return (rep);
}
