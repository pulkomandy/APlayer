/******************************************************************************/
/* MikModConverter 669 class.                                                 */
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
/* Intern structures                                                          */
/******************************************************************************/
typedef struct S69HEADER
{
	uint8	marker[2];
	char	message[109];
	uint8	nos;
	uint8	nop;
	uint8	loopOrder;
	uint8	orders[0x80];
	uint8	tempos[0x80];
	uint8	breaks[0x80];
} S69HEADER;



// Sample information
typedef struct S69SAMPLE
{
	char	fileName[14];
	int32	length;
	int32	loopBeg;
	int32	loopEnd;
} S69SAMPLE;



// Encoded note
typedef struct S69NOTE
{
	uint8	a;
	uint8	b;
	uint8	c;
} S69NOTE;



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a 669 module.        */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
bool Mik669::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint8 buf[0x80];
	PFile *file = convInfo->moduleFile;
	int32 i;

	// First check the length
	if (file->GetLength() < (int32)sizeof(S69HEADER))
		return (false);

	// Now check the signature
	file->SeekToBegin();
	file->Read(buf, 2);

	if (((buf[0] == 'i') && (buf[1] == 'f')) || ((buf[0] == 'J') && (buf[1] == 'N')))
	{
		file->Seek(108, PFile::pSeekCurrent);

		if ((file->Read_UINT8() <= 64) &&
			(file->Read_UINT8() <= 128) &&
			(file->Read_UINT8() <= 127))
		{
			// Check order table
			file->Read(buf, 0x80);

			for (i = 0; i < 0x80; i++)
			{
				if ((buf[i] >= 0x80) && (buf[i] != 0xff))
					return (false);
			}

			// Check tempos table
			file->Read(buf, 0x80);

			for (i = 0; i < 0x80; i++)
			{
				if ((!buf[i]) || (buf[i] > 32))
					return (false);
			}

			// Check pattern length table
			file->Read(buf, 0x80);

			for (i = 0; i < 0x80; i++)
			{
				if (buf[i] > 0x3f)
					return (false);
			}

			return (true);
		}
	}

	return (false);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from 669 to UniMod structure. */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a info structure where to read and      */
/*         store information needed.                                          */
/******************************************************************************/
ap_result Mik669::ModuleConverter(APAgent_ConvertModule *convInfo)
{
	int32 i;
	SAMPLE *current;
	S69SAMPLE sample;
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;

	try
	{
		// Clear the buffer pointers
		s69Pat = NULL;
		mh     = NULL;

		// Start to allocate some buffers we need
		if ((s69Pat = new S69NOTE[64 * 8]) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		if ((mh = new S69HEADER) == NULL)
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		// Clear the buffers
		memset(mh, 0, sizeof(S69HEADER));

		// Try to read the module header
		file->Read(mh->marker, 2);
		file->ReadString(mh->message, 108);

		mh->nos       = file->Read_UINT8();
		mh->nop       = file->Read_UINT8();
		mh->loopOrder = file->Read_UINT8();

		file->Read(mh->orders, 0x80);
		file->Read(mh->tempos, 0x80);
		file->Read(mh->breaks, 0x80);

		// TN: Added extra check on EOF
		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Set module variables
		of.initSpeed = 4;
		of.initTempo = 78;
		of.songName.SetString(mh->message, &charSet850);
		of.songName  = of.songName.Left(36);
		of.numChn    = 8;
		of.numPat    = mh->nop;
		of.numIns    = of.numSmp = mh->nos;
		of.numTrk    = of.numChn * of.numPat;
		of.flags     = UF_XMPERIODS | UF_LINEAR;

		convInfo->fileType.LoadString(res, IDS_MIKC_MIME_669);

		if (mh->marker[0] == 'i')
			convInfo->modKind.LoadString(res, IDS_MIKC_NAME_669_COMPOSER);
		else
			convInfo->modKind.LoadString(res, IDS_MIKC_NAME_669_EXTENDED);

		// Split the message into 3 lines
		for (i = 35; (i >= 0) && (mh->message[i] == ' '); i--)
			mh->message[i] = 0x00;

		for (i = 36 + 35; (i >= 36 + 0) && (mh->message[i] == ' '); i--)
			mh->message[i] = 0x00;

		for (i = 72 + 35; (i >= 72 + 0) && (mh->message[i] == ' '); i--)
			mh->message[i] = 0x00;

		if ((mh->message[0]) || (mh->message[36]) || (mh->message[72]))
		{
			char comment[3 * (36 + 1) + 1];
			int32 index1, index2, index3;

			memset(comment, 0, 3 * (36 + 1) + 1);

			strncpy(comment, mh->message, 36);
			strcat(comment, "\r");
			index1 = strlen(comment) - 1;

			if (mh->message[36])
				strncat(comment, mh->message + 36, 36);
			strcat(comment, "\r");
			index2 = strlen(comment) - 1;

			if (mh->message[72])
				strncat(comment, mh->message + 72, 36);
			strcat(comment, "\r");
			index3 = strlen(comment) - 1;

			comment[3 * (36 + 1)] = 0x00;
			of.comment.SetString(comment, &charSet850);

			// TN: The reason I store the '\r' character
			// in the string again, is because they are
			// converted to a strange character in the
			// character set used above
			of.comment.SetAt(index1, '\r');
			of.comment.SetAt(index2, '\r');
			of.comment.SetAt(index3, '\r');
		}

		if (!(AllocPositions(0x80)))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		for (i = 0; i < 0x80; i++)
		{
			if (mh->orders[i] >= mh->nop)
				break;

			of.positions[i] = mh->orders[i];
		}

		of.numPos = i;
		of.repPos = mh->loopOrder < of.numPos ? mh->loopOrder : 0;

		if (!(AllocSamples()))
		{
			ShowError(IDS_MIKC_ERR_MEMORY);
			throw PUserException();
		}

		current = of.samples;

		for (i = 0; i < of.numIns; i++)
		{
			// Sample information
			file->ReadString(sample.fileName, 13);

			sample.length  = file->Read_L_UINT32();
			sample.loopBeg = file->Read_L_UINT32();
			sample.loopEnd = file->Read_L_UINT32();

			if (sample.loopEnd == 0xfffff)
				sample.loopEnd = 0;

			if ((sample.length < 0) || (sample.loopBeg < -1) || (sample.loopEnd < -1))
			{
				ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
				throw PUserException();
			}

			current->sampleName.SetString(sample.fileName, &charSet850);
			current->seekPos    = 0;
			current->speed      = 0;
			current->length     = sample.length;
			current->loopStart  = sample.loopBeg;
			current->loopEnd    = sample.loopEnd;
			current->flags      = (sample.loopBeg < sample.loopEnd) ? SF_LOOP : 0;
			current->volume     = 64;

			current++;
		}

		// TN: Added extra EOF check
		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_SAMPLEINFO);
			throw PUserException();
		}

		LoadPatterns(file);
	}
	catch(PUserException e)
	{
		retVal = AP_ERROR;
	}

	// Clean up again
	delete[] s69Pat;
	s69Pat = NULL;

	delete mh;
	mh = NULL;

	return (retVal);
}



/******************************************************************************/
/* LoadPatterns() will load all the patterns.                                 */
/*                                                                            */
/* Input:  "file" is a pointer to the file to read the data from.             */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
void Mik669::LoadPatterns(PFile *file)
{
	int32 track, row, channel;
	uint8 note, inst, vol, effect, lastFx, lastVal;
	S69NOTE *cur;
	int32 tracks = 0;

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

	for (track = 0; track < of.numPat; track++)
	{
		// Set pattern break locations
		of.pattRows[track] = mh->breaks[track] + 1;

		// Load the 669 pattern 
		cur = s69Pat;
		for (row = 0; row < 64; row++)
		{
			for (channel = 0; channel < 8; channel++, cur++)
			{
				cur->a = file->Read_UINT8();
				cur->b = file->Read_UINT8();
				cur->c = file->Read_UINT8();
			}
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MIKC_ERR_LOADING_PATTERN);
			throw PUserException();
		}

		// Translate the pattern
		for (channel = 0; channel < 8; channel++)
		{
			UniReset();

			// Set the pattern tempo
			UniPTEffect(0xf, 78, of.flags);
			UniPTEffect(0xf, mh->tempos[track], of.flags);

			lastFx  = 0xff;
			lastVal = 0;

			for (row = 0; row <= mh->breaks[track]; row++)
			{
				int32 a, b, c;

				// Fetch the encoded note
				a = s69Pat[(row * 8) + channel].a;
				b = s69Pat[(row * 8) + channel].b;
				c = s69Pat[(row * 8) + channel].c;

				// Decode it
				note = a >> 2;
				inst = ((a & 0x3) << 4) | ((b & 0xf0) >> 4);
				vol  = b & 0xf;

				if (a < 0xff)
				{
					if (a < 0xfe)
					{
						UniInstrument(inst);
						UniNote(note + 2 * OCTAVE);
						lastFx = 0xff;		// Reset background effect memory
					}

					UniPTEffect(0xc, vol << 2, of.flags);
				}

				if ((c != 0xff) || (lastFx != 0xff))
				{
					if (c == 0xff)
					{
						c      = lastFx;
						effect = lastVal;
					}
					else
						effect = c & 0xf;

					switch (c >> 4)
					{
						// Portamento up
						case 0:
							UniPTEffect(0x1, effect, of.flags);
							lastFx  = c;
							lastVal = effect;
							break;

						// Portamento down
						case 1:
							UniPTEffect(0x2, effect, of.flags);
							lastFx  = c;
							lastVal = effect;
							break;

						// Portamento to note
						case 2:
							UniPTEffect(0x3, effect, of.flags);
							lastFx  = c;
							lastVal = effect;
							break;

						// Frequency adjust
						case 3:
							// DMP converts this effect to S3M FFx. Why not?
							UniEffect(UNI_S3MEFFECTF, 0xf0 | effect);
							break;

						// Vibrato
						case 4:
							UniPTEffect(0x4, effect, of.flags);
							lastFx  = c;
							lastVal = effect;
							break;

						// Set tempo
						case 5:
							if (effect)
								UniPTEffect(0xf, effect, of.flags);
							else
							{
								if (mh->marker[0] != 'i')
								{
									// Super fast tempo not supported
								}
							}
							break;

						// Extra commands
						case 6:
							break;

						// Slot retrig
						case 7:
							break;
					}
				}

				UniNewLine();
			}

			if (!(of.tracks[tracks++] = UniDup()))
			{
				ShowError(IDS_MIKC_ERR_INITIALIZE);
				throw PUserException();
			}
		}
	}
}
