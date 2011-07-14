/******************************************************************************/
/* MikModConverter AMF class.                                                 */
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
typedef struct AMFHEADER
{
	uint8	id[3];					// AMF file marker
	uint8	version;				// Upper major, lower nibble minor version number
	char	songName[33];			// Songname
	uint8	numSamples;				// Number of samples saved
	uint8	numOrders;
	uint16	numTracks;				// Number of tracks saved
	uint8	numChannels;			// Number of channels used
	int8	panPos[32];				// Voice pan positions
	uint8	songBpm;
	uint8	songSpd;
} AMFHEADER;



typedef struct AMFSAMPLE
{
	uint8	type;
	char	sampleName[33];
	char	fileName[14];
	uint32	offset;
	uint32	length;
	uint16	c2Spd;
	uint8	volume;
	uint32	repPos;
	uint32	repEnd;
} AMFSAMPLE;



typedef struct AMFNOTE
{
	uint8	note;
	uint8	instr;
	uint8	volume;
	uint8	fxCnt;
	uint8	effect[3];
	int8	parameter[3];
} AMFNOTE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's an AMF module.       */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool MikAMF::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 buf[3];
	uint8 ver;
	PFile *file = convInfo->moduleFile;

	// First check the length
	if (file->GetLength() < (int32)sizeof(AMFHEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(buf, 3);

	if ((buf[0] == 'A') && (buf[1] == 'M') && (buf[2] == 'F'))
	{
		ver = file->Read_UINT8();

		if ((ver >= 10) && (ver <= 14))
			return (true);
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from AMF to UniMod structure. */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result MikAMF::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 t, u, realTrackCnt, realSmpCnt, defaultPanning;
	AMFSAMPLE s;
	SAMPLE *q;
	uint16 *trackRemap;
	uint32 samplePos;
	int32 channel_remap[16];
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		track = NULL;
		mh    = NULL;

		// Start to allocate some buffers we need
		if ((mh = new AMFHEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((track = new AMFNOTE[64]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(track, 0, 64 * sizeof(AMFNOTE));

		// Try to read the module header
		file->Read(mh->id, 3);

		mh->version = file->Read_UINT8();

		file->ReadString(mh->songName, 32);

		mh->numSamples  = file->Read_UINT8();
		mh->numOrders   = file->Read_UINT8();
		mh->numTracks   = file->Read_L_UINT16();
		mh->numChannels = file->Read_UINT8();

		if ((!mh->numChannels) || (mh->numChannels > (mh->version >= 12 ? 32 : 16)))
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		if (mh->version >= 11)
		{
			memset(mh->panPos, 0, 32);
			file->Read(mh->panPos, (mh->version >= 13) ? 32 : 16);
		}
		else
			file->Read(channel_remap, 16);

		if (mh->version >= 13)
		{
			mh->songBpm = file->Read_UINT8();

			if (mh->songBpm < 32)
			{
				ShowError(IDS_MIKC_ERR_LOADING_HEADER);
				throw PUserException();
			}

			mh->songSpd = file->Read_UINT8();

			if (mh->songSpd > 32)
			{
				ShowError(IDS_MIKC_ERR_LOADING_HEADER);
				throw PUserException();
			}
		}
		else
		{
			mh->songBpm = 125;
			mh->songSpd = 6;
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_AMF);
		convInfo->modKind.Format(res, IDS_MIKC_NAME_AMF, mh->version / 10, mh->version % 10);

		of.initSpeed = mh->songSpd;
		of.initTempo = mh->songBpm;
		of.numChn    = mh->numChannels;
		of.numTrk    = mh->numOrders * mh->numChannels;

		if (mh->numTracks > of.numTrk)
			of.numTrk = mh->numTracks;

		of.numTrk++;		// Add room for extra, empty track
		of.songName.SetString(mh->songName, &charSet850);
		of.numPos    = mh->numOrders;
		of.numPat    = mh->numOrders;
		of.repPos    = 0;
		of.flags    |= UF_S3MSLIDES;

		// Whenever possible, we should try to determine the original format.
		// Here we assume it was S3M-style wrt bpm limit...
		of.bpmLimit  = 32;

		// Play with the panning table. Although the AMF format embeds a
		// panning table, if the module was a MOD or an S3M with default
		// panning and didn't use any panning commands, don't flag
		// UF_PANNING, to use our preferred panning table for this case
		defaultPanning = 1;

		for (t = 0; t < 32; t++)
		{
			if (mh->panPos[t] > 64)
			{
				of.panning[t]  = PAN_SURROUND;
				defaultPanning = 0;
			}
			else
			{
				if (mh->panPos[t] == 64)
					of.panning[t] = PAN_RIGHT;
				else
					of.panning[t] = (mh->panPos[t] + 64) << 1;
			}
		}

		if (defaultPanning)
		{
			for (t = 0; t < of.numChn; t++)
			{
				if (of.panning[t] == (((t + 1) & 2) ? PAN_RIGHT : PAN_LEFT))
				{
					// Not MOD canonical panning
					defaultPanning = 0;
					break;
				}
			}
		}

		if (defaultPanning)
			of.flags |= UF_PANNING;

		of.numIns = of.numSmp = mh->numSamples;

		if (!(AllocPositions(of.numPos)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (t = 0; t < of.numPos; t++)
			of.positions[t] = t;

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

		// Read AMF order table
		for (t = 0; t < of.numPat; t++)
		{
			if (mh->version >= 14)
			{
				// Track size
				of.pattRows[t] = file->Read_L_UINT16();
			}

			if (mh->version >= 10)
				file->ReadArray_L_UINT16s(of.patterns + (t * of.numChn), of.numChn);
			else
			{
				for (u = 0; u < of.numChn; u++)
					of.patterns[t * of.numChn + channel_remap[u]] = file->Read_L_UINT16();
			}
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Read sample information
		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		q = of.samples;

		for (t = 0; t < of.numIns; t++)
		{
			// Try to read sample info
			s.type = file->Read_UINT8();

			file->ReadString(s.sampleName, 32);
			file->ReadString(s.fileName, 13);

			s.offset = file->Read_L_UINT32();
			s.length = file->Read_L_UINT32();
			s.c2Spd  = file->Read_L_UINT16();

			if (s.c2Spd == 8368)
				s.c2Spd = 8363;

			s.volume = file->Read_UINT8();

			if (mh->version >= 11)
			{
				s.repPos = file->Read_L_UINT32();
				s.repEnd = file->Read_L_UINT32();
			}
			else
			{
				s.repPos = file->Read_L_UINT16();
				s.repEnd = s.length;
			}

			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			q->sampleName.SetString(s.sampleName, &charSet850);
			q->speed      = s.c2Spd;
			q->volume     = s.volume;

			if (s.type)
			{
				q->seekPos   = s.offset;
				q->length    = s.length;
				q->loopStart = s.repPos;
				q->loopEnd   = s.repEnd;

				if ((s.repEnd - s.repPos) > 2)
					q->flags |= SF_LOOP;
			}

			q++;
		}

		// Read track table
		if ((trackRemap = new uint16[mh->numTracks + 1]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		trackRemap[0] = 0;
		file->ReadArray_L_UINT16s(trackRemap + 1, mh->numTracks);

		if (file->IsEOF())
		{
			delete[] trackRemap;

			ShowError(IDS_MIKC_ERR_LOADING_TRACK);
			throw PUserException();
		}

		for (realTrackCnt = t = 0; t <= mh->numTracks; t++)
		{
			if (realTrackCnt < trackRemap[t])
				realTrackCnt = trackRemap[t];
		}

		for (t = 0; t < of.numPat * of.numChn; t++)
			of.patterns[t] = (of.patterns[t] <= mh->numTracks) ? trackRemap[of.patterns[t]] - 1 : realTrackCnt;

		delete[] trackRemap;

		// Unpack tracks
		for (t = 0; t < realTrackCnt; t++)
		{
			if (file->IsEOF())
			{
				ShowError(IDS_MIKC_ERR_LOADING_TRACK);
				throw PUserException();
			}

			if (!(UnpackTrack(file)))
			{
				ShowError(IDS_MIKC_ERR_LOADING_TRACK);
				throw PUserException();
			}

			if (!(of.tracks[t] = ConvertTrack()))
			{
				ShowError(IDS_MIKC_ERR_INITIALIZE);
				throw PUserException();
			}
		}

		// Add an extra void track
		UniReset();

		for (t = 0; t < 64; t++)
			UniNewLine();

		of.tracks[realTrackCnt++] = UniDup();

		for (t = realTrackCnt; t < of.numTrk; t++)
			of.tracks[t] = NULL;

		// Compute sample offsets
		samplePos = file->GetPosition();

		for (realSmpCnt = t = 0; t < of.numSmp; t++)
		{
			if (realSmpCnt < (int32)of.samples[t].seekPos)
				realSmpCnt = of.samples[t].seekPos;
		}

		for (t = 1; t <= realSmpCnt; t++)
		{
			q = of.samples;

			while ((int32)q->seekPos != t)
				q++;

			q->seekPos = samplePos;
			samplePos += q->length;
		}
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete mh;
	mh = NULL;

	delete[] track;
	track = NULL;

	return (retVal);
}



/******************************************************************************/
/* UnpackTrack() will load a single track.                                    */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikAMF::UnpackTrack(PFile *file)
{
	uint32 trackSize;
	uint8 row, cmd;
	int8 arg;

	// Empty track
	memset(track, 0, 64 * sizeof(AMFNOTE));

	// Read packed track
	if (file)
	{
		trackSize  = file->Read_L_UINT16();
		trackSize += ((uint32)file->Read_UINT8()) << 16;

		if (trackSize)
		{
			while (trackSize--)
			{
				row = file->Read_UINT8();
				cmd = file->Read_UINT8();
				arg = file->Read_UINT8();

				// Unexpected end of track
				if (!trackSize)
				{
					// The last triplet should be FF FF FF, but this is not
					// always the case... maybe a bug in m2amf?
					if ((row == 0xff) && (cmd == 0xff) && (arg == -1))
						break;
//					else
//						return (false);
				}

				// Invalid row (probably unexpected end of row)
				if (row >= 64)
					return (false);

				if (cmd < 0x7f)
				{
					// Note, vol
					track[row].note   = cmd;
					track[row].volume = (uint8)arg + 1;
				}
				else
				{
					if (cmd == 0x7f)
					{
						// Duplicate row
						if ((arg < 0) && (row + arg >= 0))
							memcpy(track + row, track + (row + arg), sizeof(AMFNOTE));
					}
					else
					{
						if (cmd == 0x80)
						{
							// Instr
							track[row].instr = arg + 1;
						}
						else
						{
							if (cmd == 0x83)
							{
								// Volume without note
								track[row].volume = (uint8)arg + 1;
							}
							else
							{
								if (cmd == 0xff)
								{
									// Apparently, some M2AMF version fail to
									// estimate the size of the compressed
									// patterns correctly, and end up with blanks,
									// i.e. dead triplets. Those are marked with
									// cmd == 0xff. Let's ignore them.
								}
								else
								{
									if (track[row].fxCnt < 3)
									{
										// Effect, param
										if (cmd > 0x97)
											return (false);

										track[row].effect[track[row].fxCnt] = cmd & 0x7f;
										track[row].parameter[track[row].fxCnt] = arg;
										track[row].fxCnt++;
									}
									else
										return (false);
								}
							}
						}
					}
				}
			}
		}
	}

	return (true);
}



/******************************************************************************/
/* ConvertTrack() convert a track to uni format.                              */
/*                                                                            */
/* Output: A pointer to the uni track.                                        */
/******************************************************************************/
uint8 *MikAMF::ConvertTrack(void)
{
	int32 row, fx4Memory = 0;

	// Convert track
	UniReset();

	for (row = 0; row < 64; row++)
	{
		if (track[row].instr)
			UniInstrument(track[row].instr - 1);

		if (track[row].note > OCTAVE)
			UniNote(track[row].note - OCTAVE);

		// AMF effects
		while (track[row].fxCnt--)
		{
			int8 inf = track[row].parameter[track[row].fxCnt];

			switch (track[row].effect[track[row].fxCnt])
			{
				// Set speed
				case 1:
					UniEffect(UNI_S3MEFFECTA, inf);
					break;

				// Volume slide
				case 2:
					if (inf)
					{
						UniWriteByte(UNI_S3MEFFECTD);

						if (inf >= 0)
							UniWriteByte((inf & 0xf) << 4);
						else
							UniWriteByte((-inf) & 0xf);
					}
					break;

				// Effect 3 -> Set channel volume, done in UnpackTrack()

				// Porta up/down
				case 4:
					if (inf)
					{
						if (inf > 0)
						{
							UniEffect(UNI_S3MEFFECTE, inf);
							fx4Memory = UNI_S3MEFFECTE;
						}
						else
						{
							UniEffect(UNI_S3MEFFECTF, -inf);
							fx4Memory = UNI_S3MEFFECTF;
						}
					}
					else
					{
						if (fx4Memory)
							UniEffect(fx4Memory, 0);
					}
					break;

				// Effect 5 -> Porta abs, not supported

				// Porta to note
				case 6:
					UniEffect(UNI_ITEFFECTG, inf);
					break;

				// Tremor
				case 7:
					UniEffect(UNI_S3MEFFECTI, inf);
					break;

				// Arpeggio
				case 8:
					UniPTEffect(0x0, inf, of.flags);
					break;

				// Vibrato
				case 9:
					UniPTEffect(0x4, inf, of.flags);
					break;

				// Porta + Volume slide
				case 0xa:
					UniPTEffect(0x3, 0, of.flags);

					if (inf)
					{
						UniWriteByte(UNI_S3MEFFECTD);

						if (inf >= 0)
							UniWriteByte((inf & 0xf) << 4);
						else
							UniWriteByte((-inf) & 0xf);
					}
					break;

				// Vibrato + Volume slide
				case 0xb:
					UniPTEffect(0x4, 0, of.flags);

					if (inf)
					{
						UniWriteByte(UNI_S3MEFFECTD);

						if (inf >= 0)
							UniWriteByte((inf & 0xf) << 4);
						else
							UniWriteByte((-inf) & 0xf);
					}
					break;

				// Pattern break (in hex)
				case 0xc:
					UniPTEffect(0xd, inf, of.flags);
					break;

				// Pattern jump
				case 0xd:
					UniPTEffect(0xb, inf, of.flags);
					break;

				// Effect 0xe -> Sync, not supported

				// Retrig
				case 0xf:
					UniEffect(UNI_S3MEFFECTQ, inf & 0xf);
					break;

				// Sample offset
				case 0x10:
					UniPTEffect(0x9, inf, of.flags);
					break;

				// Fine volume slide
				case 0x11:
					if (inf)
					{
						UniWriteByte(UNI_S3MEFFECTD);

						if (inf >= 0)
							UniWriteByte((inf & 0xf) << 4 | 0xf);
						else
							UniWriteByte(0xf0 | ((-inf) & 0xf));
					}
					break;

				// Fine portamento
				case 0x12:
					if (inf)
					{
						if (inf > 0)
						{
							UniEffect(UNI_S3MEFFECTE, 0xf0 | (inf & 0xf));
							fx4Memory = UNI_S3MEFFECTE;
						}
						else
						{
							UniEffect(UNI_S3MEFFECTF, 0xf0 | ((-inf) & 0xf));
							fx4Memory = UNI_S3MEFFECTF;
						}
					}
					else
					{
						if (fx4Memory)
							UniEffect(fx4Memory, 0);
					}
					break;

				// Delay note
				case 0x13:
					UniPTEffect(0xe, 0xd0 | (inf & 0xf), of.flags);
					break;

				// Note cut
				case 0x14:
					UniPTEffect(0xc, 0, of.flags);
					track[row].volume = 0;
					break;

				// Set tempo
				case 0x15:
					UniEffect(UNI_S3MEFFECTT, inf);
					break;

				// Extra fine portamento
				case 0x16:
					if (inf)
					{
						if (inf > 0)
						{
							UniEffect(UNI_S3MEFFECTE, 0xe0 | ((inf >> 2) & 0xf));
							fx4Memory = UNI_S3MEFFECTE;
						}
						else
						{
							UniEffect(UNI_S3MEFFECTF, 0xe0 | (((-inf) >> 2) & 0xf));
							fx4Memory = UNI_S3MEFFECTF;
						}
					}
					else
					{
						if (fx4Memory)
							UniEffect(fx4Memory, 0);
					}
					break;

				// Panning
				case 0x17:
					if (inf > 64)
					{
						// Surround
						UniEffect(UNI_ITEFFECTS0, 0x91);
					}
					else
						UniPTEffect(0x8, (inf == 64) ? 255 : (inf + 64) << 1, of.flags);

					of.flags |= UF_PANNING;
					break;
			}
		}

		if (track[row].volume)
			UniVolEffect(VOL_VOLUME, track[row].volume - 1);

		UniNewLine();
	}

	return (UniDup());
}
