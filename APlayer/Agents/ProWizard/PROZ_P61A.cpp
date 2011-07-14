/******************************************************************************/
/* ProWizard P61A class.                                                      */
/*                                                                            */
/* The Player v5.0A format.                                                   */
/* Created by Jarno Paananen (Guru/Sahara Surfers) (1993)                     */
/*                                                                            */
/* The Player v6.0A format.                                                   */
/* Created by Jarno Paananen (Guru/Sahara Surfers) (1994)                     */
/*                                                                            */
/* The Player v6.1A format.                                                   */
/* Created by Jarno Paananen (Guru/Sahara Surfers) (1995)                     */
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
/* Tables                                                                     */
/******************************************************************************/
static const int8 packTab[16] =
{
	0, 1, 2, 4, 8, 16, 32, 64, 128, -64, -32, -16, -8, -4, -2, -1
};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a known module.      */
/*                                                                            */
/* Input:  "module" is a reference to where the packed module is stored.      */
/*                                                                            */
/* Output: Is the unpacked module size or 0 if not recognized.                */
/******************************************************************************/
uint32 PROZ_P61A::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint32 temp;
	uint16 temp1, temp2, temp3;
	uint8 temp4;
	uint16 i, j;
	uint32 offset, offset1, endOffset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Initialize default type
	startOffset = 4;
	type        = 'P50A';

	// Start to check for signature
	temp = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[0]));
	if ((temp != 'P50A') && (temp != 'P60A') && (temp != 'P61A'))
		startOffset = 0;

	mod += startOffset;

	// Check sample offset
	temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));
	if ((temp1 & 0x1) || (temp1 < 0x24))
		return (0);

	// Check number of patterns
	if (mod[2] > 64)
		return (0);

	// Check number of samples
	if (mod[3] > 0xdf)
		return (0);

	// Check for sample type
	//
	// Is the samples packed?
	if (mod[3] & 0x40)
	{
		sampInfoOffset = 8;
		sampType       = 'PACK';
	}
	else
	{
		sampInfoOffset = 4;

		// Is the sample delta?
		if (mod[3] & 0x80)
			sampType = 'DELT';
		else
			sampType = 'NORM';
	}

	// Get number of patterns
	pattNum = mod[2];
	if (pattNum == 0)
		return (0);

	// Get number of samples
	sampNum = mod[3] & 0x1f;
	if (sampNum == 0)
		return (0);

	// Check sample offset
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) <= (sampNum * 6 + pattNum * 8))
		return (0);

	// Initialize the sample info structure
	for (i = 0; i < 31; i++)
	{
		sampInfo[i].startOffset = 0;
		sampInfo[i].length      = 0;
		sampInfo[i].fineTune    = 0;
		sampInfo[i].volume      = 0;
		sampInfo[i].loopStart   = 0;
		sampInfo[i].loopLength  = 1;
	}

	// Check sample informations
	sampSize     = 0;
	destSampSize = 0;
	for (i = 0; i < sampNum; i++)
	{
		// Get sample length
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[sampInfoOffset + i * 6]));
		if (temp1 >= 0x8000)
		{
			// Special case. Repeat another sample
			temp1 = -(int16)temp1;	// Find sample index
			if (temp1 > 31)
				return (0);

			// Copy the start offset
			sampInfo[i].startOffset = sampInfo[temp1 - 1].startOffset;

			// Get new sample length
			temp1 = sampInfo[temp1 - 1].length;
		}
		else
		{
			// Store the new start offset
			sampInfo[i].startOffset = sampSize;

			// Add the sample size to the total size
			sampSize += temp1;

			// If finetune is negative, the sample is fib. packed
			if (!(mod[sampInfoOffset + i * 6 + 2] & 0x80))
				sampSize += temp1;	// Not packed
		}

		// Add the size to the total destination size
		destSampSize += (temp1 * 2);

		// Remember the length
		sampInfo[i].length = temp1;

		// Copy the finetune and volume
		sampInfo[i].fineTune = mod[sampInfoOffset + i * 6 + 2];
		sampInfo[i].volume   = mod[sampInfoOffset + i * 6 + 3];

		if (sampInfo[i].volume > 64)
			return (0);

		if (sampInfo[i].fineTune & 0x70)
			return (0);

		// Get loop start
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[sampInfoOffset + i * 6 + 4]));
		if (temp1 < 0x8000)
		{
			// Got loop, now find out the start and length
			sampInfo[i].loopStart  = temp1;
			sampInfo[i].loopLength = sampInfo[i].length - temp1;
			if (sampInfo[i].loopLength >= 0x8000)
				return (0);
		}
	}

	// Check first track offset. It has to be zero
	offset = sampInfoOffset + sampNum * 6;
	if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset])) != 0)
		return (0);

	// Get sample offset
	temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));

	// Check track table
	for (i = 0; i < pattNum; i++)
	{
		for (j = 0; j < 3; j++)
		{
			temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset]));
			temp3 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[offset + 2])) - 3;
			offset += 2;

			if (temp3 <= temp2)
				return (0);

			if (temp2 > temp1)
				return (0);

			if (temp3 > temp1)
				return (0);
		}

		// Skip the last offset
		offset += 2;
	}

	// Check position table
	i = 0;		// Number of positions taken
	while ((temp4 = mod[offset++]) != 0xff)
	{
		if (((temp4 / 2) * 2) == temp4)
		{
			if (temp4 > 0x7e)
				return (0);
		}
		else
		{
			type = 'P60A';
			if (temp4 > 0x3f)
				return (0);
		}

		i++;
		if (i > 0x7f)
			return (0);
	}

	// Check module size
	if ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) + sampSize) > (module.GetLength() + 256))
		return (0);

	// Find out if the module is a 6.0A or 6.1A by scanning the track data
	endOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) - 4;
	offset1   = offset;
	temp      = 0;
	while (offset < endOffset)
	{
		if ((mod[offset++] == 0x00) && (mod[offset + 1] == 0x00))
			temp++;
	}

	// If we haven't found any 0x00 pair, it's a 6.1A module
	if (temp == 0)
		type = 'P61A';
	else
	{
		// Make second test
		endOffset -= 4;
		offset     = offset1;
		temp       = 0;
		while (offset < endOffset)
		{
			if ((mod[offset++] == 0x80) && (mod[offset++] <= 0x0f))
			{
				temp1 = mod[offset++] * 256 + mod[offset++];
				if ((offset - temp1) > offset1)
					temp++;
			}
		}

		if (temp == 0)
			type = 'P61A';
	}

	// Calculate the total size of the PTK module
	calcSize = pattNum * 1024 + destSampSize + 1084;

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
ap_result PROZ_P61A::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint8 posTable[128];
	uint16 temp, temp1;
	uint8 temp2;
	uint32 offset;
	uint16 i;

	// Get the module pointer
	mod  = module.GetBufferForReadOnly();
	mod += startOffset;

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write module name
	destFile->Write(zeroBuf, 20);

	// Get the sample offset
	sampOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));

	// Write sample informations
	for (i = 0; i < sampNum; i++)
	{
		// Write sample name
		destFile->Write(zeroBuf, 22);

		// Get sample length
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[sampInfoOffset + i * 6]));
		if (temp >= 0x8000)
		{
			// Special case. Repeat another sample
			temp = -(int16)temp;	// Find sample index

			// Get new sample length
			temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[sampInfoOffset + (temp - 1) * 6]));
		}

		// Write sample length
		destFile->Write_B_UINT16(temp);

		// Write finetune + volume
		destFile->Write_B_UINT16(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[sampInfoOffset + i * 6 + 2])) & 0x0fff);

		// Write repeat values
		temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[sampInfoOffset + i * 6 + 4]));
		if (temp1 == 0xffff)
		{
			// No loop
			destFile->Write_B_UINT16(0x0000);
			destFile->Write_B_UINT16(0x0001);
		}
		else
		{
			// Loop
			destFile->Write_B_UINT16(temp1);
			destFile->Write_B_UINT16(temp - temp1);
		}
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

	// Find the track offset
	trackOffset = sampInfoOffset + sampNum * 6;

	// Create the position table
	memset(posTable, 0, sizeof(posTable));
	posNum = 0;

	offset = trackOffset + pattNum * 8;

	while ((temp2 = mod[offset++]) != 0xff)
	{
		if ((type & 0xffff0000) != 'P6\0\0')
			temp2 /= 2;

		posTable[posNum++] = temp2;
	}

	// Remember the note offset
	noteOffset = offset;

	// Write number of positions
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Allocate memory to hold all the patterns.
	// We allocate a pattern more, just as safety space
	pattern = new uint8[(pattNum + 1) * 1024];
	if (pattern == NULL)
		throw PMemoryException();

	try
	{
		// Clear the buffer
		memset(pattern, 0, pattNum * 1024);

		// Call the right pattern decode routine
		if (type == 'P61A')
			P61A_ConvertPatterns(mod);
		else
			P60A_ConvertPatterns(mod);

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

	// Now it's time to decode and write the sample data
	for (i = 0; i < sampNum; i++)
	{
		uint32 sampLen;

		// Get start of sample data
		offset = sampInfo[i].startOffset + sampOffset;

		// Get length of sample
		sampLen = sampInfo[i].length * 2;

		if (sampLen != 0)
		{
			// Allocate memory to hold the sample data, but only if it's
			// packed or delta packed.
			if ((mod[3] & 0x80) || (sampInfo[i].fineTune & 0x80))
			{
				int8 *destBuf;
				uint32 writeOffset = 0;

				// Allocate buffer to hold the destination sample
				destBuf = new int8[sampLen];
				if (destBuf == NULL)
					throw PMemoryException();

				try
				{
					// Is the sample packed?
					if (sampInfo[i].fineTune & 0x80)
					{
						// Yup, depack it
						int8 samp;
						uint8 nib1, nib2;

						samp = 0;

						for ( ; writeOffset < sampLen; )
						{
							nib1 = mod[offset++];
							nib2 = nib1 & 0x0f;
							nib1 >>= 4;

							samp -= packTab[nib1];
							destBuf[writeOffset++] = samp;
							samp -= packTab[nib2];
							destBuf[writeOffset++] = samp;
						}
					}
					else
					{
						// Nup, it's delta
						int8 samp;

						// Copy the first sample
						samp                   = mod[offset++];
						destBuf[writeOffset++] = samp;

						// Dedelta the rest
						for ( ; writeOffset < sampLen; )
						{
							samp                  -= mod[offset++];
							destBuf[writeOffset++] = samp;
						}
					}

					// Write the sample data
					destFile->Write(destBuf, sampLen);
				}
				catch(...)
				{
					// Delete the buffer
					delete[] destBuf;
					throw;
				}

				// Delete the buffer
				delete[] destBuf;
			}
			else
			{
				// Just a normal sample
				destFile->Write(&mod[offset], sampLen);
			}
		}
	}

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_P61A::GetModuleType(void)
{
	switch (type)
	{
		case 'P50A':
			return (IDS_PROZ_TYPE_P50A);

		case 'P60A':
			return (IDS_PROZ_TYPE_P60A);

		case 'P61A':
			return (IDS_PROZ_TYPE_P61A);
	}

	// Hopefully, we will never get here
	ASSERT(false);
	return (IDS_PROZ_NAME);
}



/******************************************************************************/
/* GetNote() will convert a relative note into a period.                      */
/*                                                                            */
/* Input:  "trackByte" is the track data byte that is read from the module.   */
/*         "byte1" is a reference to the first byte in the pattern.           */
/*         "byte2" is a reference to the second byte in the pattern.          */
/******************************************************************************/
void PROZ_P61A::GetNote(uint8 trackByte, uint8 &byte1, uint8 &byte2)
{
	// Is there a hi-bit sample number?
	if (trackByte & 0x01)
	{
		byte1      = 0x10;
		trackByte &= 0xfe;
	}
	else
		byte1 = 0x00;

	// Any note?
	if (trackByte != 0)
	{
		trackByte -= 2;
		trackByte /= 2;

		byte1 |= period[trackByte][0];
		byte2  = period[trackByte][1];
	}
	else
		byte2 = 0x00;
}



/******************************************************************************/
/* CheckEffect() will check the effect and change some of them.               */
/*                                                                            */
/* Input:  "byte3" is a reference to the third byte in the pattern.           */
/*         "byte4" is a reference to the fourth byte in the pattern.          */
/******************************************************************************/
void PROZ_P61A::CheckEffect(uint8 &byte3, uint8 &byte4)
{
	uint8 eff;

	// Extract the effect
	eff = byte3 & 0x0f;

	switch (eff)
	{
		// Filter
		case 0x0e:
		{
			if (byte4 == 0x02)
				byte4 = 0x01;

			break;
		}

		// Vibrato + Volume Slide
		// Tone Portamento + Volume Slide
		// Volume slide
		case 0x5:
		case 0x6:
		case 0xa:
		{
			if (byte4 >= 0x80)
				byte4 = (-(int8 &)byte4) << 4;

			break;
		}

		// Arpeggio
		case 0x8:
		{
			byte3 &= 0xf0;
			break;
		}
	}
}



/******************************************************************************/
/* P60A_ConvertPatterns() will convert the patterns in 5.0A/6.0A format.      */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_P61A::P60A_ConvertPatterns(const uint8 *mod)
{
	uint32 readOffset, writeOffset, offset;
	uint16 temp, temp1;
	uint8 temp2;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j, k;

	writeOffset = 0;

	for (i = 0; i < pattNum; i++)
	{
		for (j = 0; j < 4; j++)
		{
			// Get track offset
			offset = noteOffset + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset + i * 8 + j * 2]));

			for (k = 0; k < 64; )
			{
				// Did we reach the boundary?
				if (offset < sampOffset)
				{
					// Nop, it's safe to read the track data
					//
					// Get first byte
					temp2 = mod[offset++];
					if (temp2 == 0x80)
					{
						// Copy notes
						temp    = mod[offset++] + 1;		// Number of notes to copy
						temp1   = mod[offset] * 256 + mod[offset + 1];	// Offset to subtract
						offset += 2;

						// Find new read offset
						readOffset = offset - temp1;

						for ( ; temp != 0; temp--)
						{
							// Get first byte
							temp2 = mod[readOffset++];

							if (temp2 > 0x80)
							{
								// Multiple notes
								temp2 = (-(int8)temp2) - 1;
								GetNote(temp2, byte1, byte2);

								// Get the sample number, effect and effect value
								byte3 = mod[readOffset++];
								byte4 = mod[readOffset++];

								// Check the effect
								CheckEffect(byte3, byte4);

								// Write pattern data
								pattern[writeOffset]     = byte1;
								pattern[writeOffset + 1] = byte2;
								pattern[writeOffset + 2] = byte3;
								pattern[writeOffset + 3] = byte4;

								writeOffset += 16;
								k++;

								// Get value byte
								temp2 = mod[readOffset++];

								if (temp2 <= 64)
								{
									// Empty rows
									while (temp2 != 0)
									{
										pattern[writeOffset]     = 0x00;
										pattern[writeOffset + 1] = 0x00;
										pattern[writeOffset + 2] = 0x00;
										pattern[writeOffset + 3] = 0x00;

										writeOffset += 16;
										k++;
										temp2--;
									}
								}
								else
								{
									// Copy the same data value times
									temp2 = -(int8)temp2;

									while (temp2 != 0)
									{
										// Write pattern data
										pattern[writeOffset]     = byte1;
										pattern[writeOffset + 1] = byte2;
										pattern[writeOffset + 2] = byte3;
										pattern[writeOffset + 3] = byte4;

										writeOffset += 16;
										k++;
										temp2--;
									}
								}

								continue;
							}

							// Single note
							GetNote(temp2, byte1, byte2);

							// Get the sample number, effect and effect value
							byte3 = mod[readOffset++];
							byte4 = mod[readOffset++];

							// Check the effect
							CheckEffect(byte3, byte4);

							// Write pattern data
							pattern[writeOffset]     = byte1;
							pattern[writeOffset + 1] = byte2;
							pattern[writeOffset + 2] = byte3;
							pattern[writeOffset + 3] = byte4;

							writeOffset += 16;
							k++;
						}

						continue;
					}

					if (temp2 > 0x80)
					{
						// Multiple notes
						temp2 = (-(int8)temp2) - 1;
						GetNote(temp2, byte1, byte2);

						// Get the sample number, effect and effect value
						byte3 = mod[offset++];
						byte4 = mod[offset++];

						// Check the effect
						CheckEffect(byte3, byte4);

						// Write pattern data
						pattern[writeOffset]     = byte1;
						pattern[writeOffset + 1] = byte2;
						pattern[writeOffset + 2] = byte3;
						pattern[writeOffset + 3] = byte4;

						writeOffset += 16;
						k++;

						// Get value byte
						temp2 = mod[offset++];

						if (temp2 <= 64)
						{
							// Empty rows
							while (temp2 != 0)
							{
								pattern[writeOffset]     = 0x00;
								pattern[writeOffset + 1] = 0x00;
								pattern[writeOffset + 2] = 0x00;
								pattern[writeOffset + 3] = 0x00;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}
						else
						{
							// Copy the same data value times
							temp2 = -(int8)temp2;

							while (temp2 != 0)
							{
								// Write pattern data
								pattern[writeOffset]     = byte1;
								pattern[writeOffset + 1] = byte2;
								pattern[writeOffset + 2] = byte3;
								pattern[writeOffset + 3] = byte4;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}

						continue;
					}

					// Single note
					GetNote(temp2, byte1, byte2);

					// Get the sample number, effect and effect value
					byte3 = mod[offset++];
					byte4 = mod[offset++];

					// Check the effect
					CheckEffect(byte3, byte4);

					// Write pattern data
					pattern[writeOffset]     = byte1;
					pattern[writeOffset + 1] = byte2;
					pattern[writeOffset + 2] = byte3;
					pattern[writeOffset + 3] = byte4;

					writeOffset += 16;
					k++;
				}
				else
					k++;	// Out of range
			}

			// Done with a track. Now adjust the writeOffset to the
			// next track
			k           -= 64;
			writeOffset -= (k * 16 + 64 * 16 - 4);
		}

		// Adjust write offset
		writeOffset += (63 * 16);
	}
}



/******************************************************************************/
/* P61A_ConvertPatterns() will convert the patterns in 6.1A format.           */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_P61A::P61A_ConvertPatterns(const uint8 *mod)
{
	uint32 readOffset, writeOffset, offset;
	uint16 temp, temp1;
	uint8 temp2;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j, k;

	writeOffset = 0;

	for (i = 0; i < pattNum; i++)
	{
		for (j = 0; j < 4; j++)
		{
			// Get track offset
			offset = noteOffset + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset + i * 8 + j * 2]));

			for (k = 0; k < 64; )
			{
				// Did we reach the boundary?
				if (offset < sampOffset)
				{
					// Nop, it's safe to read the track data
					//
					// Get first byte
					temp2 = mod[offset++];
					if (temp2 == 0x7f)
					{
						// One empty line
						pattern[writeOffset]     = 0x00;
						pattern[writeOffset + 1] = 0x00;
						pattern[writeOffset + 2] = 0x00;
						pattern[writeOffset + 3] = 0x00;

						writeOffset += 16;
						k++;
						continue;
					}

					if (temp2 < 0x60)
					{
						// Just a full note (===> 1A 5F05 <=== note + smp + efx)
						GetNote(temp2, byte1, byte2);

						// Get the sample number, effect and effect value
						byte3 = mod[offset++];
						byte4 = mod[offset++];

						// Check the effect
						CheckEffect(byte3, byte4);

						// Write pattern data
						pattern[writeOffset]     = byte1;
						pattern[writeOffset + 1] = byte2;
						pattern[writeOffset + 2] = byte3;
						pattern[writeOffset + 3] = byte4;

						writeOffset += 16;
						k++;
						continue;
					}

					if (temp2 < 0x80)
					{
						if ((temp2 & 0xf0) == 0x60)
						{
							// Only effect (===> 6 310 <=== 6 + efx)
							byte3 = temp2 & 0x0f;
							byte4 = mod[offset++];

							// Check the effect
							CheckEffect(byte3, byte4);

							// Write pattern data
							pattern[writeOffset]     = 0x00;
							pattern[writeOffset + 1] = 0x00;
							pattern[writeOffset + 2] = byte3;
							pattern[writeOffset + 3] = byte4;
						}
						else
						{
							// Note + sample (===> 7 32 8 <=== 7 + note + smp)
							temp2 = (temp2 << 4) | (mod[offset] >> 4);
							GetNote(temp2, byte1, byte2);

							// Write pattern data
							pattern[writeOffset]     = byte1;
							pattern[writeOffset + 1] = byte2;
							pattern[writeOffset + 2] = mod[offset++] << 4;
							pattern[writeOffset + 3] = 0x00;
						}

						writeOffset += 16;
						k++;
						continue;
					}

					if (temp2 < 0xd0)
					{
						// Multi notes and effect (===> A2 1602 84 <===)
						temp2 -= 0x80;
						GetNote(temp2, byte1, byte2);

						// Get the sample number, effect and effect value
						byte3 = mod[offset++];
						byte4 = mod[offset++];

						// Check the effect
						CheckEffect(byte3, byte4);

						// Write pattern data
						pattern[writeOffset]     = byte1;
						pattern[writeOffset + 1] = byte2;
						pattern[writeOffset + 2] = byte3;
						pattern[writeOffset + 3] = byte4;

						writeOffset += 16;
						k++;

						// Get value byte
						temp2 = mod[offset++];

						if (temp2 < 0x80)
						{
							// Empty rows
							while (temp2 != 0)
							{
								pattern[writeOffset]     = 0x00;
								pattern[writeOffset + 1] = 0x00;
								pattern[writeOffset + 2] = 0x00;
								pattern[writeOffset + 3] = 0x00;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}
						else
						{
							// Copy the same data value times
							temp2 -= 0x80;

							while (temp2 != 0)
							{
								// Write pattern data
								pattern[writeOffset]     = byte1;
								pattern[writeOffset + 1] = byte2;
								pattern[writeOffset + 2] = byte3;
								pattern[writeOffset + 3] = byte4;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}

						continue;
					}

					if (temp2 < 0xf0)
					{
						// Multi effect (===> E 602 84 <===)
						byte3 = temp2 & 0x0f;
						byte4 = mod[offset++];

						// Check the effect
						CheckEffect(byte3, byte4);

						// Write pattern data
						pattern[writeOffset]     = 0x00;
						pattern[writeOffset + 1] = 0x00;
						pattern[writeOffset + 2] = byte3;
						pattern[writeOffset + 3] = byte4;

						writeOffset += 16;
						k++;

						// Get value byte
						temp2 = mod[offset++];

						if (temp2 < 0x80)
						{
							// Empty rows
							while (temp2 != 0)
							{
								pattern[writeOffset]     = 0x00;
								pattern[writeOffset + 1] = 0x00;
								pattern[writeOffset + 2] = 0x00;
								pattern[writeOffset + 3] = 0x00;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}
						else
						{
							// Copy the same data value times
							temp2 -= 0x80;

							while (temp2 != 0)
							{
								// Write pattern data
								pattern[writeOffset]     = 0x00;
								pattern[writeOffset + 1] = 0x00;
								pattern[writeOffset + 2] = byte3;
								pattern[writeOffset + 3] = byte4;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}

						continue;
					}

					if (temp2 < 0xff)
					{
						// Multi notes (===> F 1A 7 04 <=== note + smp + repeat)
						temp2 = (temp2 << 4) | (mod[offset] >> 4);
						GetNote(temp2, byte1, byte2);

						// Get the sample number
						byte3 = mod[offset++] << 4;

						// Write pattern data
						pattern[writeOffset]     = byte1;
						pattern[writeOffset + 1] = byte2;
						pattern[writeOffset + 2] = byte3;
						pattern[writeOffset + 3] = 0x00;

						writeOffset += 16;
						k++;

						// Get value byte
						temp2 = mod[offset++];

						if (temp2 < 0x80)
						{
							// Empty rows
							while (temp2 != 0)
							{
								pattern[writeOffset]     = 0x00;
								pattern[writeOffset + 1] = 0x00;
								pattern[writeOffset + 2] = 0x00;
								pattern[writeOffset + 3] = 0x00;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}
						else
						{
							// Copy the same data value times
							temp2 -= 0x80;

							while (temp2 != 0)
							{
								// Write pattern data
								pattern[writeOffset]     = byte1;
								pattern[writeOffset + 1] = byte2;
								pattern[writeOffset + 2] = byte3;
								pattern[writeOffset + 3] = 0x00;

								writeOffset += 16;
								k++;
								temp2--;
							}
						}

						continue;
					}

					//
					//
					// Copy notes (===> FF 01 or FF 41 09 or FF CB 0153 <===)
					//
					//
					temp2 = mod[offset++];
					if (temp2 < 0x40)
					{
						// Multi empty rows
						temp2++;

						while (temp2 != 0)
						{
							pattern[writeOffset]     = 0x00;
							pattern[writeOffset + 1] = 0x00;
							pattern[writeOffset + 2] = 0x00;
							pattern[writeOffset + 3] = 0x00;

							writeOffset += 16;
							k++;
							temp2--;
						}

						continue;
					}

					if (temp2 < 0x80)
					{
						// Number of notes to copy
						temp = temp2 - 0x40;

						// Find new read offset
						readOffset = offset - mod[offset] + 1;
						offset++;
					}
					else
					{
						// Number of notes to copy
						temp = temp2 - 0xc0;

						// Find new read offset
						temp1      = mod[offset] * 256 + mod[offset + 1];
						offset    += 2;
						readOffset = offset - temp1;
					}

					temp++;
					for ( ; temp != 0; temp--)
					{
						// Get first byte
						temp2 = mod[readOffset++];
						if (temp2 == 0x7f)
						{
							// One empty line
							pattern[writeOffset]     = 0x00;
							pattern[writeOffset + 1] = 0x00;
							pattern[writeOffset + 2] = 0x00;
							pattern[writeOffset + 3] = 0x00;

							writeOffset += 16;
							k++;
							continue;
						}

						if (temp2 < 0x60)
						{
							// Just a full note (===> 1A 5F05 <=== note + smp + efx)
							GetNote(temp2, byte1, byte2);

							// Get the sample number, effect and effect value
							byte3 = mod[readOffset++];
							byte4 = mod[readOffset++];

							// Check the effect
							CheckEffect(byte3, byte4);

							// Write pattern data
							pattern[writeOffset]     = byte1;
							pattern[writeOffset + 1] = byte2;
							pattern[writeOffset + 2] = byte3;
							pattern[writeOffset + 3] = byte4;

							writeOffset += 16;
							k++;
							continue;
						}

						if (temp2 < 0x80)
						{
							if ((temp2 & 0xf0) == 0x60)
							{
								// Only effect (===> 6 310 <=== 6 + efx)
								byte3 = temp2 & 0x0f;
								byte4 = mod[readOffset++];

								// Check the effect
								CheckEffect(byte3, byte4);

								// Write pattern data
								pattern[writeOffset]     = 0x00;
								pattern[writeOffset + 1] = 0x00;
								pattern[writeOffset + 2] = byte3;
								pattern[writeOffset + 3] = byte4;
							}
							else
							{
								// Note + sample (===> 7 32 8 <=== 7 + note + smp)
								temp2 = (temp2 << 4) | (mod[readOffset] >> 4);
								GetNote(temp2, byte1, byte2);

								// Write pattern data
								pattern[writeOffset]     = byte1;
								pattern[writeOffset + 1] = byte2;
								pattern[writeOffset + 2] = mod[readOffset++] << 4;
								pattern[writeOffset + 3] = 0x00;
							}

							writeOffset += 16;
							k++;
							continue;
						}

						if (temp2 < 0xd0)
						{
							// Multi notes and effect (===> A2 1602 84 <===)
							temp2 -= 0x80;
							GetNote(temp2, byte1, byte2);

							// Get the sample number, effect and effect value
							byte3 = mod[readOffset++];
							byte4 = mod[readOffset++];

							// Check the effect
							CheckEffect(byte3, byte4);

							// Write pattern data
							pattern[writeOffset]     = byte1;
							pattern[writeOffset + 1] = byte2;
							pattern[writeOffset + 2] = byte3;
							pattern[writeOffset + 3] = byte4;

							writeOffset += 16;
							k++;

							// Get value byte
							temp2 = mod[readOffset++];

							if (temp2 < 0x80)
							{
								// Empty rows
								while (temp2 != 0)
								{
									pattern[writeOffset]     = 0x00;
									pattern[writeOffset + 1] = 0x00;
									pattern[writeOffset + 2] = 0x00;
									pattern[writeOffset + 3] = 0x00;

									writeOffset += 16;
									k++;
									temp2--;
								}
							}
							else
							{
								// Copy the same data value times
								temp2 -= 0x80;

								while (temp2 != 0)
								{
									// Write pattern data
									pattern[writeOffset]     = byte1;
									pattern[writeOffset + 1] = byte2;
									pattern[writeOffset + 2] = byte3;
									pattern[writeOffset + 3] = byte4;

									writeOffset += 16;
									k++;
									temp2--;
								}
							}

							continue;
						}

						if (temp2 < 0xf0)
						{
							// Multi effect (===> E 602 84 <===)
							byte3 = temp2 & 0x0f;
							byte4 = mod[readOffset++];

							// Check the effect
							CheckEffect(byte3, byte4);

							// Write pattern data
							pattern[writeOffset]     = 0x00;
							pattern[writeOffset + 1] = 0x00;
							pattern[writeOffset + 2] = byte3;
							pattern[writeOffset + 3] = byte4;

							writeOffset += 16;
							k++;

							// Get value byte
							temp2 = mod[readOffset++];

							if (temp2 < 0x80)
							{
								// Empty rows
								while (temp2 != 0)
								{
									pattern[writeOffset]     = 0x00;
									pattern[writeOffset + 1] = 0x00;
									pattern[writeOffset + 2] = 0x00;
									pattern[writeOffset + 3] = 0x00;

									writeOffset += 16;
									k++;
									temp2--;
								}
							}
							else
							{
								// Copy the same data value times
								temp2 -= 0x80;

								while (temp2 != 0)
								{
									// Write pattern data
									pattern[writeOffset]     = 0x00;
									pattern[writeOffset + 1] = 0x00;
									pattern[writeOffset + 2] = byte3;
									pattern[writeOffset + 3] = byte4;

									writeOffset += 16;
									k++;
									temp2--;
								}
							}

							continue;
						}

						if (temp2 < 0xff)
						{
							// Multi notes (===> F 1A 7 04 <=== note + smp + repeat)
							temp2 = (temp2 << 4) | (mod[readOffset] >> 4);
							GetNote(temp2, byte1, byte2);

							// Get the sample number
							byte3 = mod[readOffset++] << 4;

							// Write pattern data
							pattern[writeOffset]     = byte1;
							pattern[writeOffset + 1] = byte2;
							pattern[writeOffset + 2] = byte3;
							pattern[writeOffset + 3] = 0x00;

							writeOffset += 16;
							k++;

							// Get value byte
							temp2 = mod[readOffset++];

							if (temp2 < 0x80)
							{
								// Empty rows
								while (temp2 != 0)
								{
									pattern[writeOffset]     = 0x00;
									pattern[writeOffset + 1] = 0x00;
									pattern[writeOffset + 2] = 0x00;
									pattern[writeOffset + 3] = 0x00;

									writeOffset += 16;
									k++;
									temp2--;
								}
							}
							else
							{
								// Copy the same data value times
								temp2 -= 0x80;

								while (temp2 != 0)
								{
									// Write pattern data
									pattern[writeOffset]     = byte1;
									pattern[writeOffset + 1] = byte2;
									pattern[writeOffset + 2] = byte3;
									pattern[writeOffset + 3] = 0x00;

									writeOffset += 16;
									k++;
									temp2--;
								}
							}

							continue;
						}

						//
						//
						// Copy notes (===> FF 01 or FF 41 09 or FF CB 0153 <===)
						//
						//
						temp2 = mod[readOffset++] + 1;

						while (temp2 != 0)
						{
							pattern[writeOffset]     = 0x00;
							pattern[writeOffset + 1] = 0x00;
							pattern[writeOffset + 2] = 0x00;
							pattern[writeOffset + 3] = 0x00;

							writeOffset += 16;
							k++;
							temp2--;
						}
					}
				}
				else
					k++;	// Out of range
			}

			// Done with a track. Now adjust the writeOffset to the
			// next track
			k           -= 64;
			writeOffset -= (k * 16 + 64 * 16 - 4);
		}

		// Adjust write offset
		writeOffset += (63 * 16);
	}
}
