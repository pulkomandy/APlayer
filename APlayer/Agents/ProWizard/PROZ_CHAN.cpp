/******************************************************************************/
/* ProWizard CHAN class.                                                      */
/*                                                                            */
/* Channel Player v1 - v3 format.                                             */
/* Created by Alan / Impact (1994)                                            */
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
uint32 PROZ_CHAN::CheckModule(const PBinary &module)
{
	const uint8 *mod;
	uint16 i;
	uint16 temp, temp1, temp2;
	uint16 counter;
	uint16 sampNum;
	uint32 sampLen;
	uint32 pattOffset, sampOffset, offset;
	uint32 calcSize;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Check number of samples
	sampNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));
	if ((sampNum & 0xf00f) != 0x000a)
		return (0);

	sampNum &= 0x0ff0;
	if ((sampNum == 0) || (sampNum  > 0x1f0))
		return (0);

	// Get total sample length
	sampLen = P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[6]));

	// Check sample information
	sampNum >>= 4;
	sampSize  = 0;

	for (i = 0; i < sampNum; i++)
	{
		// Get sample size
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[10 + i * 16 + 4]));
		if (temp >= 0x8000)
			return (0);

		sampSize += (temp * 2);

		if (temp == 0)
		{
			// Sample start - loop start should be 0
			if ((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10 + i * 16])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10 + i * 16 + 8]))) != 0)
				return (0);
		}

		// Check sample start
		if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10 + i * 16])) > 0xfe000)
			return (0);

		// Check loop start
		if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10 + i * 16 + 8])) > 0xfe000)
			return (0);

		// Check volume
		if (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[10 + i * 16 + 6])) > 0x40)
			return (0);

		// Check finetune
		temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[10 + i * 16 + 14]));
		if (((temp / 0x48) * 0x48) != temp)
			return (0);
	}

	// Check sample length
	if (sampSize != sampLen)
		return (0);

	// Check size of position table
	temp = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2]));
	if (temp == 0)
		return (0);

	// Check size of patterns
	temp1 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[4]));
	if (temp1 == 0)
		return (0);

	// Get offset to position table
	temp2 = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0]));

	// Check the module length
	if ((temp2 + temp + temp1 + sampSize) > (module.GetLength() + 256))
		return (0);

	// Get offsets to patterns and samples
	pattOffset = temp2 + temp;
	sampOffset = pattOffset + temp1 - 4;	// -4 because of the test

	// Channel v1 note test
	counter = 0;
	offset  = pattOffset;

	while (offset < sampOffset)
	{
		// Find any 0x80 commands. They are not possible in v1
		if ((mod[offset++] == 0x80) && (mod[offset++] <= 0x0f))
			counter++;
	}

	if (counter == 0)
	{
		// Found a v1 module
		type = 'Cha1';

		// Get offset to the track table
		trackTable = ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) & 0xfff0) >> 4) * 16 + 10;
		Chan1Speco(mod);
	}
	else
	{
		// v2 or v3
		type = 'Cha2';

		// Test for v2
		counter = 0;
		offset  = pattOffset;

		while (offset < sampOffset)
		{
			// Only v3 can have 4 0x80 right after each other
			if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[offset])) == 0x80808080)
				counter++;

			offset += 4;
		}

		if (counter != 0)
			type = 'Cha3';

		// Get offset to the track table
		trackTable = ((P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) & 0xfff0) >> 4) * 16 + 10;
		Chan2Speco(mod);
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
ap_result PROZ_CHAN::ConvertModule(const PBinary &module, PFile *destFile)
{
	const uint8 *mod;
	uint8 zeroBuf[32];
	uint16 sampNum;
	uint16 i;

	// Get the module pointer
	mod = module.GetBufferForReadOnly();

	// Init period table
	InitPeriods();

	// Zero the buffer
	memset(zeroBuf, 0, sizeof(zeroBuf));

	// Write the module name
	destFile->Write(zeroBuf, 20);

	// Get number of samples
	sampNum = (P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) & 0xfff0) >> 4;

	// Write sample info
	for (i = 0; i < sampNum; i++)
	{
		// Sample name
		destFile->Write(zeroBuf, 22);

		// Sample length
		destFile->Write(&mod[10 + i * 16 + 4], 2);

		// Finetune
		destFile->Write_UINT8(P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[10 + i * 16 + 14])) / 0x48);

		// Volume
		destFile->Write_UINT8(mod[10 + i * 16 + 7]);

		// Repeat start
		destFile->Write_B_UINT16((P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10 + i * 16 + 8])) - P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[10 + i * 16]))) / 2);

		// Repeat length
		destFile->Write(&mod[10 + i * 16 + 12], 2);
	}

	// Write empty samples
	for (; i < 31; i++)
	{
		// Write sample name + size + finetune + volume + repeat point
		destFile->Write(zeroBuf, 28);

		// Write repeat length
		destFile->Write_B_UINT16(1);
	}

	// Find the track table offset
	trackTable = 10 + sampNum * 16;

	// Write song length
	destFile->Write_UINT8(posNum);

	// Write NTK byte
	destFile->Write_UINT8(0x7f);

	// Write the position table
	destFile->Write(posTable, 128);

	// Write PTK mark
	destFile->Write_B_UINT32('M.K.');

	// Call the version converter routine
	switch (type)
	{
		case 'Cha1':
		{
			ConvertChannel1(mod, destFile);
			break;
		}

		case 'Cha2':
		{
			ConvertChannel2(mod, destFile);
			break;
		}

		case 'Cha3':
		{
			ConvertChannel3(mod, destFile);
			break;
		}

		default:
			return (AP_ERROR);
	}

	// Write sample data
	destFile->Write(&mod[P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[4]))], sampSize);

	return (AP_OK);
}



/******************************************************************************/
/* GetModuleType() returns the type of the packed module.                     */
/*                                                                            */
/* Output: The type of the packed module.                                     */
/******************************************************************************/
int32 PROZ_CHAN::GetModuleType(void)
{
	switch (type)
	{
		case 'Cha1':
			return (IDS_PROZ_TYPE_CHAN1);

		case 'Cha2':
			return (IDS_PROZ_TYPE_CHAN2);

		case 'Cha3':
			return (IDS_PROZ_TYPE_CHAN3);
	}

	// Hopefully, we will never get here
	ASSERT(false);
	return (IDS_PROZ_NAME);
}



/******************************************************************************/
/* Chan1Speco() will create the position table for v1.                        */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_CHAN::Chan1Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint8 *posTab1, *posTab2;
	uint8 temp1, temp2, temp3, temp4;
	bool found;

	// Get number of positions
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2])) / 4;

	// Jump to the track offset table
	mod += trackTable;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = mod + 4;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *posTab1++;
		temp2 = *posTab1++;
		temp3 = *posTab1++;
		temp4 = *posTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = mod;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if ((temp1 == posTab2[0]) && (temp2 == posTab2[1]) && (temp3 == posTab2[2]) && (temp4 == posTab2[3]))
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
/* Chan2Speco() will create the position table for v2 and v3.                 */
/*                                                                            */
/* Input:  "mod" is a pointer to the module.                                  */
/******************************************************************************/
void PROZ_CHAN::Chan2Speco(const uint8 *mod)
{
	uint16 i, j;
	const uint16 *posTab1, *posTab2;
	uint16 temp1, temp2, temp3, temp4;
	bool found;

	// Get number of positions
	posNum = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2])) / 8;

	// Jump to the track offset table
	mod += trackTable;

	// Clear the position table
	memset(posTable, 0, sizeof(posTable));

	// Begin to create the position table
	posTab1 = (uint16 *)mod + 4;
	pattNum = 0;

	for (i = 1; i < posNum; i++)
	{
		// Get offsets
		temp1 = *posTab1++;
		temp2 = *posTab1++;
		temp3 = *posTab1++;
		temp4 = *posTab1++;

		// Compare these offsets with the rest to see if there is an equal one
		posTab2 = (uint16 *)mod;
		found   = false;

		for (j = 0; j < i; j++)
		{
			if ((temp1 == posTab2[0]) && (temp2 == posTab2[1]) && (temp3 == posTab2[2]) && (temp4 == posTab2[3]))
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
/* ConvertChannel1() will convert the v1 patterns.                            */
/*                                                                            */
/* Input:  "mod" is a pointer to the packed module.                           */
/*         "destFile" is where to write the converted data.                   */
/******************************************************************************/
void PROZ_CHAN::ConvertChannel1(const uint8 *mod, PFile *destFile)
{
	uint8 pattern[1024];
	uint32 pattStartOffset;
	uint32 trackOffset, pattOffset;
	int8 lastPatt;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j, k;

	// Get the first pattern offset
	pattStartOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2]));

	// Get the track offset
	trackOffset = trackTable;

	// Convert the pattern data
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Do we have 4 zeros as the track numbers?
			if (P_BENDIAN_TO_HOST_INT32(*((uint32 *)&mod[trackOffset])) == 0)
				break;

			// Take each voice
			for (j = 0; j < 4; j++)
			{
				// Get the pattern offset
				pattOffset = pattStartOffset + mod[trackOffset++] * 0xc0;

				// Take all the rows
				for (k = 0; k < 64; k++)
				{
					// Get sample
					byte1 = (mod[pattOffset] & 0x01) << 4;
					byte3 = mod[pattOffset + 1];	// Sample number + effect

					// Get note
					byte2 = mod[pattOffset] & 0xfe;
					if (byte2 != 0)
					{
						byte2 -= 2;
						byte2 /= 2;

						byte1 |= period[byte2][0];
						byte2  = period[byte2][1];
					}

					// Get effect value
					byte4  = mod[pattOffset + 2];

					// Fix some of the effects
					switch (byte3 & 0x0f)
					{
						// Pattern break
						case 0xd:
						{
							// Convert to decimal
							byte4 = ((byte4 / 10) * 16) + (byte4 % 10);
							break;
						}

						// Volume slide + TonePlusVol + VibratoPlusVol
						case 0xa:
						case 0x5:
						case 0x6:
						{
							// Convert from signed value
							if (byte4 >= 0xf1)
								byte4 = -(int8)byte4;
							else
								byte4 <<= 4;

							break;
						}
					}

					// Write the pattern data
					pattern[k * 16 + j * 4]     = byte1;
					pattern[k * 16 + j * 4 + 1] = byte2;
					pattern[k * 16 + j * 4 + 2] = byte3;
					pattern[k * 16 + j * 4 + 3] = byte4;

					// Next row
					pattOffset += 3;
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
		else
			trackOffset += 4;
	}
}



/******************************************************************************/
/* ConvertChannel2() will convert the v2 patterns.                            */
/*                                                                            */
/* Input:  "mod" is a pointer to the packed module.                           */
/*         "destFile" is where to write the converted data.                   */
/******************************************************************************/
void PROZ_CHAN::ConvertChannel2(const uint8 *mod, PFile *destFile)
{
	uint8 pattern[1024];
	uint32 pattStartOffset;
	uint32 trackOffset, pattOffset;
	int8 lastPatt;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j, k;

	// Get the first pattern offset
	pattStartOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2]));

	// Get the track offset
	trackOffset = trackTable;

	// Convert the pattern data
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Take each voice
			for (j = 0; j < 4; j++)
			{
				// Get the pattern offset
				pattOffset   = pattStartOffset + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset]));
				trackOffset += 2;

				// Take all the rows
				for (k = 0; k < 64; k++)
				{
					if (mod[pattOffset] == 0x80)
					{
						// No note
						byte1 = 0x00;
						byte2 = 0x00;
						byte3 = mod[pattOffset + 1] & 0x0f;
						byte4 = 0x00;

						pattOffset += 2;
					}
					else
					{
						if (mod[pattOffset] > 0x80)
						{
							// Without effect value
							//
							// Get sample
							byte1 = (mod[pattOffset] & 0x01) << 4;
							byte3 = mod[pattOffset + 1];	// Sample number + effect

							// Get note
							byte2 = (mod[pattOffset] - 0x80) & 0xfe;
							if (byte2 != 0)
							{
								byte2 -= 2;
								byte2 /= 2;

								byte1 |= period[byte2][0];
								byte2  = period[byte2][1];
							}

							byte4 = 0x00;

							pattOffset += 2;
						}
						else
						{
							// Full note
							//
							// Get sample
							byte1 = (mod[pattOffset] & 0x01) << 4;
							byte3 = mod[pattOffset + 1];	// Sample number + effect

							// Get note
							byte2 = mod[pattOffset] & 0xfe;
							if (byte2 != 0)
							{
								byte2 -= 2;
								byte2 /= 2;

								byte1 |= period[byte2][0];
								byte2  = period[byte2][1];
							}

							// Get effect value
							byte4  = mod[pattOffset + 2];

							// Fix some of the effects
							switch (byte3 & 0x0f)
							{
								// Pattern break
								case 0xd:
								{
									// Convert to decimal
									byte4 = ((byte4 / 10) * 16) + (byte4 % 10);
									break;
								}

								// Volume slide + TonePlusVol + VibratoPlusVol
								case 0xa:
								case 0x5:
								case 0x6:
								{
									// Convert from signed value
									if (byte4 >= 0xf1)
										byte4 = -(int8)byte4;
									else
										byte4 <<= 4;

									break;
								}
							}

							pattOffset += 3;
						}
					}

					// Write the pattern data
					pattern[k * 16 + j * 4]     = byte1;
					pattern[k * 16 + j * 4 + 1] = byte2;
					pattern[k * 16 + j * 4 + 2] = byte3;
					pattern[k * 16 + j * 4 + 3] = byte4;
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
		else
			trackOffset += 8;
	}
}



/******************************************************************************/
/* ConvertChannel3() will convert the v3 patterns.                            */
/*                                                                            */
/* Input:  "mod" is a pointer to the packed module.                           */
/*         "destFile" is where to write the converted data.                   */
/******************************************************************************/
void PROZ_CHAN::ConvertChannel3(const uint8 *mod, PFile *destFile)
{
	uint8 pattern[1024];
	uint32 pattStartOffset;
	uint32 trackOffset, pattOffset;
	int8 lastPatt;
	uint8 byte1, byte2, byte3, byte4;
	uint16 i, j, k;

	// Get the first pattern offset
	pattStartOffset = P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[0])) + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[2]));

	// Get the track offset
	trackOffset = trackTable;

	// Convert the pattern data
	lastPatt = -1;
	for (i = 0; i < posNum; i++)
	{
		if (posTable[i] > lastPatt)
		{
			lastPatt++;

			// Take each voice
			for (j = 0; j < 4; j++)
			{
				// Get the pattern offset
				pattOffset   = pattStartOffset + P_BENDIAN_TO_HOST_INT16(*((uint16 *)&mod[trackOffset]));
				trackOffset += 2;

				// Take all the rows
				for (k = 0; k < 64; k++)
				{
					if (mod[pattOffset] == 0x80)
					{
						// No note
						byte1 = 0x00;
						byte2 = 0x00;
						byte3 = 0x00;
						byte4 = 0x00;

						pattOffset++;
					}
					else
					{
						if (mod[pattOffset] > 0x80)
						{
							// Only note
							//
							// Get note
							byte2 = (mod[pattOffset] - 0x80);
							if (byte2 != 0)
							{
								byte2 -= 2;
								byte2 /= 2;

								byte1 = period[byte2][0];
								byte2 = period[byte2][1];
							}
							else
								byte1 = 0x00;

							byte3 = 0x00;
							byte4 = 0x00;

							pattOffset++;
						}
						else
						{
							// Full note
							//
							// Get sample
							byte1 = (mod[pattOffset] & 0x01) << 4;
							byte3 = mod[pattOffset + 1];	// Sample number + effect

							// Get note
							byte2 = mod[pattOffset] & 0xfe;
							if (byte2 != 0)
							{
								byte2 -= 2;
								byte2 /= 2;

								byte1 |= period[byte2][0];
								byte2  = period[byte2][1];
							}

							// Get effect value
							byte4  = mod[pattOffset + 2];

							// Fix some of the effects
							switch (byte3 & 0x0f)
							{
								// Pattern break
								case 0xd:
								{
									// Convert to decimal
									byte4 = ((byte4 / 10) * 16) + (byte4 % 10);
									break;
								}

								// Volume slide + TonePlusVol + VibratoPlusVol
								case 0xa:
								case 0x5:
								case 0x6:
								{
									// Convert from signed value
									if (byte4 >= 0xf1)
										byte4 = -(int8)byte4;
									else
										byte4 <<= 4;

									break;
								}
							}

							pattOffset += 3;
						}
					}

					// Write the pattern data
					pattern[k * 16 + j * 4]     = byte1;
					pattern[k * 16 + j * 4 + 1] = byte2;
					pattern[k * 16 + j * 4 + 2] = byte3;
					pattern[k * 16 + j * 4 + 3] = byte4;
				}
			}

			// Write the pattern
			destFile->Write(pattern, sizeof(pattern));
		}
		else
			trackOffset += 8;
	}
}
