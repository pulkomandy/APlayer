/******************************************************************************/
/* ProWizard P40A class.                                                      */
/*                                                                            */
/* The Player v4.0A + v4.0B + v4.1A format.                                   */
/* Created by Jarno Paananen (Guru/Sahara Surfers)                            */
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
#include "PBinary.h"

// Agent headers
#include "ProWizard.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_P41A::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp;
	uint16 temp1;
	int32 temp2, lastSampStart;
	uint16 i;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Start to check the id
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	if ((temp != 'P40A') && (temp != 'P40B') && (temp != 'P41A'))
		return (0);

	// Remember the type
	type = temp;

	// Get number of samples
	sampNum = mod[6];

	// Check the sample information, but it's a little bit different for 4.0x and 4.1A
	if (type == 'P41A')
	{
		// 4.1A
		lastSampStart = -1;
		sampSize      = 0;

		for (i = 0; i < sampNum; i++)
		{
			// Check finetune
			temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 14]));
			if (((temp1 / 74) * 74) != temp1)
				return (0);

			// Is finetune > 0xf?
			if (temp1 > 0x456)
			{
				sampInfo[i].volume      = 0;
				sampInfo[i].fineTune    = 0;
				sampInfo[i].length      = 0;
				sampInfo[i].loopLength  = 1;
				sampInfo[i].loopOffset  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[20 + i * 16]));
				sampInfo[i].startOffset = sampInfo[i].loopOffset;
			}
			else
			{
				sampInfo[i].fineTune = temp1;

				// Check volume
				temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 12]));
				if (temp1 > 0x40)
					return (0);

				sampInfo[i].volume = temp1;

				// Check sample length
				temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 4]));
				if (temp1 >= 0x8000)
					return (0);

				sampInfo[i].length = temp1;

				// Check the sample start
				temp2 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[20 + i * 16]));
				if (temp2 > lastSampStart)
				{
					lastSampStart = temp2;
					sampSize     += (temp1 * 2);
				}

				sampInfo[i].startOffset = temp2;

				// Copy the loop info
				sampInfo[i].loopOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[20 + i * 16 + 6]));
				sampInfo[i].loopLength = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 10]));
			}
		}
	}
	else
	{
		// 4.0x
		lastSampStart = -1;
		sampSize      = 0;

		for (i = 0; i < sampNum; i++)
		{
			// Check finetune
			temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 12]));
			if (((temp1 / 74) * 74) != temp1)
				return (0);

			// Is finetune > 0xf?
			if (temp1 > 0x456)
			{
				sampInfo[i].volume      = 0;
				sampInfo[i].fineTune    = 0;
				sampInfo[i].length      = 0;
				sampInfo[i].loopLength  = 1;
				sampInfo[i].loopOffset  = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[20 + i * 16]));
				sampInfo[i].startOffset = sampInfo[i].loopOffset;
			}
			else
			{
				sampInfo[i].fineTune = temp1;

				// Check volume
				temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 14]));
				if (temp1 > 0x40)
					return (0);

				sampInfo[i].volume = temp1;

				// Check sample length
				temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 4]));
				if (temp1 >= 0x8000)
					return (0);

				sampInfo[i].length = temp1;

				// Check the sample start
				temp2 = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[20 + i * 16]));
				if (temp2 > lastSampStart)
				{
					lastSampStart = temp2;
					sampSize     += (temp1 * 2);
				}

				sampInfo[i].startOffset = temp2;

				// Copy the loop info
				sampInfo[i].loopOffset = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[20 + i * 16 + 6]));
				sampInfo[i].loopLength = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[20 + i * 16 + 10]));
			}
		}
	}

	// Get number of positions
	posNum = mod[5];

	// Find track offset
	trackOffset = 20 + sampNum * 16;

	// Find number of patterns to build
	Speco(mod);

	// Check end mark in position table
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[posNum * 8 + (sampNum + 1) * 16 + 4])) != 0xffff)
		return (0);

	// Get all the table offsets
	tda = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[8]));
	tta = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[12]));
	sda = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[16]));

	// Put back relative addresses
	DeInit();

	// Find the end of the module
	endOffset = sda + 4 + sampSize;

	if (endOffset > (module.GetLength() + 256))
	{
		// SPECIAL! Sometimes, there is a useless sample at the end
		//
		// Get length on last sample
		temp1 = sampInfo[sampNum - 1].length * 2;

		// Find start offset to last sample
		temp = endOffset - temp1;

		if (temp > (module.GetLength() + 256))
			return (0);

		// Adjust variables
		sampSize  -= temp1;
		endOffset -= temp1;

		// "Remove" the last sample
		sampInfo[sampNum - 1].length     = 0;
		sampInfo[sampNum - 1].loopOffset = sampInfo[sampNum - 1].startOffset;
		sampInfo[sampNum - 1].loopLength = 1;
		sampInfo[sampNum - 1].volume     = 0;
		sampInfo[sampNum - 1].fineTune   = 0;
	}

	// Calculate the total size of the PTK module
	calcSize = pattNum * 1024 + sampSize + 1084;

	return (calcSize);
}



/******************************************************************************/
/* ConvertModule() will convert the module to ProTracker format.              */
/*                                                                            */
/* Input:  "module" is a reference to the packed module.                      */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result PROZ_P41A::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[128];
	P4xxChannel chans[4];
	uint16 pattOffset[64];
	uint32 sampOffset;
	int16 lastOffset, newOffset, offset;
	const uint8 *trk1, *trk2, *trk3, *trk4;
	uint16 rowCount;
	uint16 i, j;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	destFile->Write(zeroBuf, 20);

	// Get the sample offset
	sampOffset = sda + 4;

	// Write sample informations
	for (i = 0; i < sampNum; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Write sample length
		destFile->Write_B_UINT16(sampInfo[i].length);

		// Write finetune
		destFile->Write_UINT8(sampInfo[i].fineTune / 74);

		// Write volume
		destFile->Write_UINT8(sampInfo[i].volume);

		// Write repeat start
		destFile->Write_B_UINT16((sampInfo[i].loopOffset - sampInfo[i].startOffset) / 2);

		// Write replen
		destFile->Write_B_UINT16(sampInfo[i].loopLength);
	}

	// Write the rest of the sample info
	for (; i < 31; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample info
		destFile->Write_B_UINT16(0);	// Sample length
		destFile->Write_B_UINT16(0);	// Finetune + Volume
		destFile->Write_B_UINT16(0);	// Repeat
		destFile->Write_B_UINT16(1);	// Replen
	}

	// Write number of positions
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Find track offsets and build a table with them in increasing order
	lastOffset = -1;
	for (i = 0; i < pattNum; i++)
	{
		newOffset = 0x7fff;

		for (j = 0; j < posNum; j++)
		{
			// Get the track offset
			offset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + j * 8]));
			if ((offset > lastOffset) && (offset < newOffset))
				newOffset = offset;
		}

		// Store the next offset
		pattOffset[i] = newOffset;
		lastOffset    = newOffset;
	}

	// Write position table
	for (i = 0; i < posNum; i++)
	{
		// Get track offset
		offset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + i * 8]));

		// Find the offset in the pattern offset table
		for (j = 0; j < pattNum; j++)
		{
			if (pattOffset[j] == offset)
				break;
		}

		// Write the pattern number
		destFile->Write_UINT8(j);
	}

	// Write the rest of the position table
	destFile->Write(zeroBuf, 128 - posNum);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Allocate memory to hold all the patterns
	pattern = new uint8[pattNum * 1024];
	if (pattern == NULL)
		throw PMemoryException();

	try
	{
		// Clear the pattern block
		memset(pattern, 0, pattNum * 1024);

		// Clear the channel structures
		memset(chans, 0, sizeof(chans));

		// Take each position
		for (i = 0; i < posNum; i++)
		{
			// Get track offset to pattern to convert
			offset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + i * 8]));

			// Find the offset in the pattern offset table
			for (j = 0; j < pattNum; j++)
			{
				if (pattOffset[j] == offset)
					break;
			}

			// Found offset, now find write offset
			writeOffset = j * 1024;

			// Get all the track offsets
			trk1 = &mod[P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + i * 8])) + tda + 4];
			trk2 = &mod[P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + i * 8 + 2])) + tda + 4];
			trk3 = &mod[P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + i * 8 + 4])) + tda + 4];
			trk4 = &mod[P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[tta + 4 + i * 8 + 6])) + tda + 4];

			breakFlag = false;
			for (rowCount = 0; rowCount < 64; rowCount++)
			{
				// Convert the tracks
				ConvData(&chans[0], &trk1, mod);
				ConvData(&chans[1], &trk2, mod);
				ConvData(&chans[2], &trk3, mod);
				ConvData(&chans[3], &trk4, mod);

				// A break command occurred (like D or B)
				if (breakFlag)
					break;
			}
		}

		// Write the patterns
		destFile->Write(pattern, pattNum * 1024);
	}
	catch(...)
	{
		// Delete the pattern memory block
		delete[] pattern;
		throw;
	}

	// Delete the pattern memory block
	delete[] pattern;

	// Write sample data
	for (i = 0; i < sampNum; i++)
	{
		if (sampInfo[i].length != 0)
			destFile->Write(&mod[sampOffset + sampInfo[i].startOffset], sampInfo[i].length * 2);
	}

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_P41A::GetModuleType(void)
{
	switch (type)
	{
		case 'P40A':
			return (IDS_PROZ_TYPE_P40A);

		case 'P40B':
			return (IDS_PROZ_TYPE_P40B);

		case 'P41A':
			return (IDS_PROZ_TYPE_P41A);
	}

	// Hopefully, we will never get here
	ASSERT(false);
	return (IDS_PROZ_NAME);
}



/******************************************************************************/
/* Speco() will create the position table.                                    */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_P41A::Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint16 *posTab1, *posTab2;
	uint16 temp1, temp2, temp3, temp4;
	bool found;
	uint8 posTable[128];

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = (const uint16 *)&mod[trackOffset + 8];
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = P_BENDIAN_TO_HOST_INT16(*posTab1++);
		temp2 = P_BENDIAN_TO_HOST_INT16(*posTab1++);
		temp3 = P_BENDIAN_TO_HOST_INT16(*posTab1++);
		temp4 = P_BENDIAN_TO_HOST_INT16(*posTab1++);

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = (const uint16 *)&mod[trackOffset];
		found   = false;

		for (j = 0; j < i; j++)
		{
			if ((temp1 == P_BENDIAN_TO_HOST_INT16(posTab2[0])) && (temp2 == P_BENDIAN_TO_HOST_INT16(posTab2[1])) &&
				(temp3 == P_BENDIAN_TO_HOST_INT16(posTab2[2])) && (temp4 == P_BENDIAN_TO_HOST_INT16(posTab2[3])))
			{
				// Found an equal track joinment
				posTable[i] = posTable[j];
				found       = true;
				break;
			}

			// Go to the next offset pair
			posTab2 += 4;
		}

		if (!found)
			posTable[i] = ++pattNum;
	}

	// Add one extra pattern number, because we skipped the first one
	pattNum++;
}



/******************************************************************************/
/* DeInit() will recreate the relative addresses from absolute addresses in   */
/*      the sample informations.                                              */
/******************************************************************************/
void PROZ_P41A::DeInit(void)
{
	uint32 firstAddress;
	uint32 temp, temp1;
	uint16 i;

	// Get the first sample address
	firstAddress = sampInfo[0].startOffset;

	if (firstAddress != 0)
	{
		// Fix sample offsets
		for (i = 0; i < sampNum; i++)
		{
			sampInfo[i].startOffset -= firstAddress;
			sampInfo[i].loopOffset  -= firstAddress;
		}

		// Fix table offsets
		temp  = (sampNum + 1) * 16;
		temp1 = tta - temp;

		tta  = temp;
		tda -= temp1;
		sda -= temp1;
	}
}



/******************************************************************************/
/* ConvData() will convert a single P4xx track line to a ProTracker track     */
/*      line.                                                                 */
/*                                                                            */
/* Input:  "chan" is a pointer to the current channel structure.              */
/*         "trkAdr" is a pointer to the current track pointer.                */
/*         "mod" is the module pointer.                                       */
/******************************************************************************/
void PROZ_P41A::ConvData(P4xxChannel *chan, const uint8 **trkAdr, const uint8 *mod)
{
	uint8 temp;

	// Check track info byte
	if (chan->p4xInfo < 0)
	{
		// The same data as the last one
		chan->p4xInfo++;

		// Copy the last pattern data
		pattern[writeOffset++] = chan->proPattData[0];
		pattern[writeOffset++] = chan->proPattData[1];
		pattern[writeOffset++] = chan->proPattData[2];
		pattern[writeOffset++] = chan->proPattData[3];
		return;
	}

	// Check the track info byte again
	if (chan->p4xInfo > 0)
	{
		// Empty line
		chan->p4xInfo--;

		// Clear the pattern data
		pattern[writeOffset++] = 0x00;
		pattern[writeOffset++] = 0x00;
		pattern[writeOffset++] = 0x00;
		pattern[writeOffset++] = 0x00;
		return;
	}

	// Read a new line in the module
	//
	// Do we copy from another place in the module?
	if (chan->repeatLines != 0)
	{
		// Yup, read the next previous line
		chan->repeatLines--;

		chan->p4xPattData[0] = *chan->repeatAdr++;
		chan->p4xPattData[1] = *chan->repeatAdr++;
		chan->p4xPattData[2] = *chan->repeatAdr++;
		chan->p4xInfo        = *chan->repeatAdr++;
	}
	else
	{
		// Check to see if it's a repeat command
		if (*(*trkAdr) & 0x80)
		{
			uint16 offset;

			// It is, get the lines to repeat
			chan->repeatLines = *((*trkAdr) + 1);

			// Get the offset to repeat from
			offset = *((*trkAdr) + 2) * 256 + *((*trkAdr) + 3);

			// Store the repeat address
			chan->repeatAdr = &mod[tda + offset + 4];

			// Read the first line
			chan->p4xPattData[0] = *chan->repeatAdr++;
			chan->p4xPattData[1] = *chan->repeatAdr++;
			chan->p4xPattData[2] = *chan->repeatAdr++;
			chan->p4xInfo        = *chan->repeatAdr++;

			// Skip the bytes read
			*trkAdr += 4;
		}
		else
		{
			// Just a normal line
			chan->p4xPattData[0] = *(*trkAdr);
			chan->p4xPattData[1] = *((*trkAdr) + 1);
			chan->p4xPattData[2] = *((*trkAdr) + 2);
			chan->p4xInfo        = *((*trkAdr) + 3);

			// Skip the bytes read
			*trkAdr += 4;
		}
	}

	// Convert the pattern data
	if (chan->p4xPattData[0] & 0x1)
		chan->proPattData[0] = 0x10;	// Set hi bit in sample number
	else
		chan->proPattData[0] = 0x00;

	// Get note number
	temp = chan->p4xPattData[0];
	if (temp != 0)
	{
		temp /= 2;
		temp--;
		chan->proPattData[0] |= period[temp][0];
		chan->proPattData[1]  = period[temp][1];
	}
	else
		chan->proPattData[1] = 0x00;

	// Sample number + effect + effect value
	chan->proPattData[2] = chan->p4xPattData[1];
	chan->proPattData[3] = chan->p4xPattData[2];

	// Check the effect
	switch (chan->proPattData[2] & 0x0f)
	{
		// Position Jump
		// Pattern Break
		case 0xb:
		case 0xd:
		{
			breakFlag = true;
			break;
		}

		// Exx
		case 0xe:
		{
			// Filter
			if ((chan->proPattData[3] & 0xf0) == 0x00)
			{
				if (chan->proPattData[3] == 0x02)
					chan->proPattData[3] = 0x01;
			}
			else
			{
				// Note cut
				if ((chan->proPattData[3] & 0xf0) == 0xc0)
					chan->proPattData[3]++;
			}
			break;
		}

		// Volume slide
		// Tone Portamento + Volume Slide
		// Vibrato + Volume Slide
		case 0xa:
		case 0x5:
		case 0x6:
		{
			if (chan->proPattData[3] >= 0x80)
			{
				chan->proPattData[3] = (-(int8)chan->proPattData[3]) << 4;

				if (chan->proPattData[3] >= 0x80)
					chan->proPattData[3] = -(int8)chan->proPattData[3];
			}
			break;
		}

		// Arpeggio
		case 0x8:
		{
			chan->proPattData[2] &= 0xf0;
			break;
		}
	}

	// Store the pattern data in the pattern
	pattern[writeOffset++] = chan->proPattData[0];
	pattern[writeOffset++] = chan->proPattData[1];
	pattern[writeOffset++] = chan->proPattData[2];
	pattern[writeOffset++] = chan->proPattData[3];
}
