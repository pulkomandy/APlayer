/******************************************************************************/
/* MikModConverter GDM class.                                                 */
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
typedef struct GDMNOTE
{
	uint8	note;
	uint8	samp;
	struct
	{
		uint8	effect;
		uint8	param;
	} effect[4];
} GDMNOTE;



typedef struct GDMHEADER
{
	char	id1[4];
	char	songName[33];
	char	author[33];
	char	eofMarker[3];
	char	id2[4];

	uint8	majorVer;
	uint8	minorVer;
	uint16	trackerID;
	uint8	t_majorVer;
	uint8	t_minorVer;
	uint8	panTable[32];
	uint8	masterVol;
	uint8	masterTempo;
	uint8	masterBpm;
	uint16	flags;

	uint32	orderLoc;
	uint8	orderNum;
	uint32	patternLoc;
	uint8	patternNum;
	uint32	samHead;
	uint32	samData;
	uint8	samNum;
	uint32	messageLoc;
	uint32	messageLen;
	uint32	scrollyLoc;
	uint16	scrollyLen;
	uint32	graphicLoc;
	uint16	graphicLen;
} GDMHEADER;



typedef struct GDMSAMPLE
{
	char	sampName[33];
	char	fileName[13];
	uint8	ems;
	uint32	length;
	uint32	loopBeg;
	uint32	loopEnd;
	uint8	flags;
	uint16	c4Spd;
	uint8	vol;
	uint8	pan;
} GDMSAMPLE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a General DigiMusic  */
/*         module.                                                            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikGDM::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 id[4];
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(GDMHEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(id, 4);

	if ((id[0] == 'G') && (id[1] == 'D') && (id[2] == 'M') && (id[3] == 0xfe))
	{
		file->Seek(71, PFile::pSeekBegin);
		file->Read(id, 4);

		if ((id[0] == 'G') && (id[1] == 'M') && (id[2] == 'F') && (id[3] == 'S'))
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from General DigiMusic to     */
/*         UniMod structure.                                                  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikGDM::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 i, x, u, track;
	SAMPLE *q;
	GDMSAMPLE s;
	uint32 position;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		mh     = NULL;
		gdmBuf = NULL;

		// Allocate buffers
		if ((mh = new GDMHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((gdmBuf = new GDMNOTE[32 * 64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(mh, 0, sizeof(GDMHEADER));
		memset(gdmBuf, 0, 32 * 64 * sizeof(GDMNOTE));

		// Read header
		file->Read(mh->id1, 4);
		file->ReadString(mh->songName, 32);
		file->ReadString(mh->author, 32);
		file->Read(mh->eofMarker, 3);
		file->Read(mh->id2, 4);

		mh->majorVer   = file->Read_UINT8();
		mh->minorVer   = file->Read_UINT8();
		mh->trackerID  = file->Read_L_UINT16();
		mh->t_majorVer = file->Read_UINT8();
		mh->t_minorVer = file->Read_UINT8();

		file->Read(mh->panTable, 32);

		mh->masterVol   = file->Read_UINT8();
		mh->masterTempo = file->Read_UINT8();
		mh->masterBpm   = file->Read_UINT8();
		mh->flags       = file->Read_L_UINT16();

		mh->orderLoc    = file->Read_L_UINT32();
		mh->orderNum    = file->Read_UINT8();
		mh->patternLoc  = file->Read_L_UINT32();
		mh->patternNum  = file->Read_UINT8();
		mh->samHead     = file->Read_L_UINT32();
		mh->samData     = file->Read_L_UINT32();
		mh->samNum      = file->Read_UINT8();
		mh->messageLoc  = file->Read_L_UINT32();
		mh->messageLen  = file->Read_L_UINT32();
		mh->scrollyLoc  = file->Read_L_UINT32();
		mh->scrollyLen  = file->Read_L_UINT16();
		mh->graphicLoc  = file->Read_L_UINT32();
		mh->graphicLen  = file->Read_L_UINT16();

		// Have we ended abruptly?
		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Any orders?
		if (mh->orderNum == 255)
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		// Now we fill
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_GDM);
		convInfo->modKind.Format(res, IDS_MIKC_NAME_GDM, mh->majorVer, mh->minorVer);

		of.songName.SetString(mh->songName, &charSet850);
		of.numPat     = mh->patternNum + 1;
		of.repPos     = 0;
		of.numIns     = of.numSmp = mh->samNum + 1;
		of.initSpeed  = mh->masterTempo;
		of.initTempo  = mh->masterBpm;
		of.initVolume = mh->masterVol << 1;
		of.flags     |= UF_S3MSLIDES | UF_PANNING;

		// Whenever possible, we should try to determine the original format.
		// Here we assume it was S3M-style wrt bpm limit...
		of.bpmLimit   = 32;

		// Read the order data
		if (!(AllocPositions(mh->orderNum + 1)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		file->Seek(mh->orderLoc, PFile::pSeekBegin);
		for (i = 0; i < mh->orderNum + 1; i++)
			of.positions[i] = file->Read_UINT8();

		of.numPos = 0;
		for (i = 0; i < mh->orderNum + 1; i++)
		{
			int32 order = of.positions[i];
			if (order == 255)
				order = LAST_PATTERN;

			of.positions[of.numPos] = order;

			if (of.positions[i] < 254)
				of.numPos++;
		}

		// Have we ended abruptly yet?
		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Time to load the samples
		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		q        = of.samples;
		position = mh->samData;

		// Seek to instrument position
		file->Seek(mh->samHead, PFile::pSeekBegin);

		for (i = 0; i < of.numIns; i++)
		{
			// Load sample info
			file->ReadString(s.sampName, 32);
			file->ReadString(s.fileName, 12);

			s.ems     = file->Read_UINT8();
			s.length  = file->Read_L_UINT32();
			s.loopBeg = file->Read_L_UINT32();
			s.loopEnd = file->Read_L_UINT32();
			s.flags   = file->Read_UINT8();
			s.c4Spd   = file->Read_L_UINT16();
			s.vol     = file->Read_UINT8();
			s.pan     = file->Read_UINT8();

			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			q->sampleName.SetString(s.sampName, &charSet850);
			q->speed      = s.c4Spd;
			q->length     = s.length;
			q->loopStart  = s.loopBeg;
			q->loopEnd    = s.loopEnd;
			q->volume     = s.vol;
			q->panning    = s.pan;
			q->seekPos    = position;

			position += s.length;

			if (s.flags & 1)
				q->flags |= SF_LOOP;

			if (s.flags & 2)
				q->flags |= SF_16BITS;

			if (s.flags & 16)
				q->flags |= SF_STEREO;

			q++;
		}

		// Set the panning
		for (i = x = 0; i < 32; i++)
		{
			of.panning[i] = mh->panTable[i];

			if (!of.panning[i])
				of.panning[i] = PAN_LEFT;
			else if (of.panning[i] == 8)
				of.panning[i] = PAN_CENTER;
			else if (of.panning[i] == 15)
				of.panning[i] = PAN_RIGHT;
			else if (of.panning[i] == 16)
				of.panning[i] = PAN_SURROUND;
			else if (of.panning[i] == 255)
				of.panning[i] = 128;
			else
				of.panning[i] <<= 3;

			if (mh->panTable[i] != 255)
				x = i;
		}

		of.numChn = x + 1;
		if (of.numChn < 1)
			of.numChn = 1;			// For broken counts

		// Load the pattern info
		of.numTrk = of.numPat * of.numChn;

		// Jump to patterns
		file->Seek(mh->patternLoc, PFile::pSeekBegin);

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

		for (i = track = 0; i < of.numPat; i++)
		{
			ReadPattern(file);

			for (u = 0; u < of.numChn; u++, track++)
			{
				if (!(of.tracks[track] = ConvertTrack(&gdmBuf[u << 6])))
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
	delete mh;
	mh = NULL;

	delete[] gdmBuf;
	gdmBuf = NULL;

	return (retVal);
}



/******************************************************************************/
/* ReadPattern() read one pattern.                                            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void MikGDM::ReadPattern(PFile *file)
{
	int32 pos, flag, ch, i, maxCh;
	GDMNOTE n;
	uint16 length, x = 0;

	// Get pattern length
	length = file->Read_L_UINT16() - 2;

	// Clear pattern data
	memset(gdmBuf, 255, 32 * 64 * sizeof(GDMNOTE));

	pos   = 0;
	maxCh = 0;

	while (x < length)
	{
		memset(&n, 255, sizeof(GDMNOTE));

		flag = file->Read_UINT8();
		x++;

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		ch = flag & 31;
		if (ch > maxCh)
			maxCh = ch;

		if (!flag)
		{
			pos++;
			continue;
		}

		if (flag & 0x60)
		{
			if (flag & 0x20)
			{
				// New note
				n.note = file->Read_UINT8() & 127;
				n.samp = file->Read_UINT8();
				x     += 2;
			}

			if (flag & 0x40)
			{
				do
				{
					// Effect channel set
					i = file->Read_UINT8();
					n.effect[i >> 6].effect = i & 31;
					n.effect[i >> 6].param  = file->Read_UINT8();
					x += 2;
				}
				while (i & 32);
			}

			memcpy(gdmBuf + (64U * ch) + pos, &n, sizeof(GDMNOTE));
		}
	}
}



/******************************************************************************/
/* ConvertTrack() convert one GDM track to UniMod track.                      */
/*                                                                            */
/* Input:  "tr" is a pointer to the track to convert.                         */
/*                                                                            */
/* Output: Is a pointer to the UniMod track or NULL for an error.             */
/******************************************************************************/
uint8 *MikGDM::ConvertTrack(GDMNOTE *tr)
{
	int32 t, i = 0;
	uint8 note, ins, inf;

	UniReset();

	for (t = 0; t < 64; t++)
	{
		note = tr[t].note;
		ins  = tr[t].samp;

		if ((ins) && (ins != 255))
			UniInstrument(ins - 1);

		if (note != 255)
			UniNote(((note >> 4) * OCTAVE) + (note & 0xf) - 1);

		for (i = 0; i < 4; i++)
		{
			inf = tr[t].effect[i].param;

			switch (tr[t].effect[i].effect)
			{
				// Toneslide up
				case 1:
				{
					UniEffect(UNI_S3MEFFECTF, inf);
					break;
				}

				// Toneslide down
				case 2:
				{
					UniEffect(UNI_S3MEFFECTE, inf);
					break;
				}

				// Glissando to note
				case 3:
				{
					UniEffect(UNI_ITEFFECTG, inf);
					break;
				}

				// Vibrato
				case 4:
				{
					UniEffect(UNI_ITEFFECTH, inf);
					break;
				}

				// Portamento + volslide
				case 5:
				{
					UniEffect(UNI_ITEFFECTG, 0);
					UniEffect(UNI_S3MEFFECTD, inf);
					break;
				}

				// Vibrato + volslide
				case 6:
				{
					UniEffect(UNI_ITEFFECTH, 0);
					UniEffect(UNI_S3MEFFECTD, inf);
					break;
				}

				// Tremolo
				case 7:
				{
					UniEffect(UNI_S3MEFFECTR, inf);
					break;
				}

				// Tremor
				case 8:
				{
					UniEffect(UNI_S3MEFFECTI, inf);
					break;
				}

				// Offset
				case 9:
				{
					UniPTEffect(0x09, inf, of.flags);
					break;
				}

				// Volslide
				case 0x0a:
				{
					UniEffect(UNI_S3MEFFECTD, inf);
					break;
				}

				// Jump to order
				case 0x0b:
				{
					UniPTEffect(0x0b, inf, of.flags);
					break;
				}

				// Volume set
				case 0x0c:
				{
					UniPTEffect(0x0c, inf, of.flags);
					break;
				}

				// Pattern break
				case 0x0d:
				{
					UniPTEffect(0x0d, inf, of.flags);
					break;
				}

				// Extended
				case 0x0e:
				{
					switch (inf & 0xf0)
					{
						// Fine portamento up
						case 0x10:
						{
							UniEffect(UNI_S3MEFFECTF, 0x0f | ((inf << 4) & 0x0f));
							break;
						}

						// Fine portamento down
						case 0x20:
						{
							UniEffect(UNI_S3MEFFECTE, 0xf0 | (inf & 0x0f));
							break;
						}

						// Glissando control
						case 0x30:
						{
							UniEffect(SS_GLISSANDO, inf & 0x0f);
							break;
						}

						// Vibrato waveform
						case 0x40:
						{
							UniEffect(SS_VIBWAVE, inf & 0x0f);
							break;
						}

						// Set c4Spd
						case 0x50:
						{
							UniEffect(SS_FINETUNE, inf & 0x0f);
							break;
						}

						// Loop fun
						case 0x60:
						{
							UniEffect(UNI_ITEFFECTS0, (inf & 0x0f) | 0xb0);
							break;
						}

						// Tremolo waveform
						case 0x70:
						{
							UniEffect(SS_TREMWAVE, inf & 0x0f);
							break;
						}

						// Extra fine porta up
						case 0x80:
						{
							UniEffect(UNI_S3MEFFECTF, 0x0e | ((inf << 4) & 0x0f));
							break;
						}

						// Extra fine porta down
						case 0x90:
						{
							UniEffect(UNI_S3MEFFECTE, 0xe0 | (inf & 0x0f));
							break;
						}

						// Fine volslide up
						case 0xa0:
						{
							UniEffect(UNI_S3MEFFECTD, 0x0f | ((inf << 4) & 0x0f));
							break;
						}

						// Fine volslide down
						case 0xb0:
						{
							UniEffect(UNI_S3MEFFECTE, 0xf0 | (inf & 0x0f));
							break;
						}

						// Note cut
						// Note delay
						// Extend row
						case 0xc0:
						case 0xd0:
						case 0xe0:
						{
							UniPTEffect(0xe, inf, of.flags);
							break;
						}
					}
					break;
				}

				// Set tempo
				case 0x0f:
				{
					UniEffect(UNI_S3MEFFECTA, inf);
					break;
				}

				// Arpeggio
				case 0x10:
				{
					UniPTEffect(0x0, inf, of.flags);
					break;
				}

				// Retrigger
				case 0x12:
				{
					UniEffect(UNI_S3MEFFECTQ, inf);
					break;
				}

				// Set global volume
				case 0x13:
				{
					UniEffect(UNI_XMEFFECTG, inf << 1);
					break;
				}

				// Fine vibrato
				case 0x14:
				{
					UniEffect(UNI_ITEFFECTU, inf);
					break;
				}

				// Special
				case 0x1e:
				{
					switch (inf & 0xf0)
					{
						// Set pan position
						case 8:
						{
							if (inf >= 128)
								UniPTEffect(0x08, 255, of.flags);
							else
								UniPTEffect(0x08, inf << 1, of.flags);

							break;
						}
					}
					break;
				}

				// Set bpm
				case 0x1f:
				{
					if (inf >= 0x20)
						UniEffect(UNI_S3MEFFECTT, inf);

					break;
				}
			}
		}

		UniNewLine();
	}

	return (UniDup());
}
