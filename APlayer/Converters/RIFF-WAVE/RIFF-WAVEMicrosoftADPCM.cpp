/******************************************************************************/
/* RIFF-WAVE Microsoft ADPCM class.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PFile.h"
#include "PResource.h"

// APlayerKit headers
#include "APAddOns.h"

// Converter headers
#include "RIFF-WAVEConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Tables                                                                     */
/******************************************************************************/
static int32 adaptionTable[] =
{
	230, 230, 230, 230, 307, 409, 512, 614,
	768, 614, 512, 409, 307, 230, 230, 230
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
RIFFWAVEMicrosoftADPCM::RIFFWAVEMicrosoftADPCM(PResource *resource) : RIFFWAVEFormat(resource)
{
	decodeBuffer = NULL;
	encodeBuffer = NULL;
	coefSets     = NULL;
	packet       = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
RIFFWAVEMicrosoftADPCM::~RIFFWAVEMicrosoftADPCM(void)
{
}



/******************************************************************************/
/* LoaderInitialize() initialize the loader.                                  */
/*                                                                            */
/* Input:  "dataLength" is the size of the data block.                        */
/*         "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::LoaderInitialize(uint32 dataLength, const APConverter_SampleFormat *convInfo)
{
	// Allocate buffer to hold the loaded data
	decodeBuffer = new int8[blockAlign];
	if (decodeBuffer == NULL)
		throw PMemoryException();

	// Initialize member variables
	fileSize = dataLength;
}



/******************************************************************************/
/* LoaderCleanup() frees all the resources allocated.                         */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::LoaderCleanup(void)
{
	// Delete the decode buffer
	delete[] decodeBuffer;
	decodeBuffer = NULL;

	// Delete the coefsets
	delete[] coefSets;
	coefSets = NULL;
}



/******************************************************************************/
/* LoadExtraHeaderInfo() loads and extra header informations from the 'fmt '  */
/*      chunk.                                                                */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to read from.               */
/*         "convInfo" is a pointer to the info structure.                     */
/*         "errorStr" is a reference to a string to store the error in if any.*/
/*                                                                            */
/* Output: Number of bytes read.                                              */
/*                                                                            */
/* Except: PUserException.                                                    */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::LoadExtraHeaderInfo(PFile *file, const APConverter_SampleFormat *convInfo, PString &errorStr)
{
	uint16 i;
	uint16 extraLen;

	// Test some of the standard informations
	if (convInfo->bitSize != 4)
	{
		errorStr.Format(res, IDS_RIFFWAVE_ERR_INVALIDBITSIZE, convInfo->bitSize);
		throw PUserException();
	}

	// Get the length of the extra information
	extraLen = file->Read_L_UINT16();

	// Read the header
	samplesPerBlock = file->Read_L_UINT16();	// Number of samples per block
	coefNum         = file->Read_L_UINT16();	// Number of coef's

	// Is the header big enough
	if ((2 + 2 + 4 * coefNum) != extraLen)
	{
		errorStr.Format(res, IDS_RIFFWAVE_ERR_EXTRAHEADER, extraLen);
		throw PUserException();
	}

	// Allocate buffer to hold the coef's
	coefSets = new ADPCMCOEFSET[coefNum];
	if (coefSets == NULL)
		throw PMemoryException();

	// Read the coef's
	for (i = 0; i < coefNum; i++)
	{
		coefSets[i].coef1 = file->Read_L_UINT16();
		coefSets[i].coef2 = file->Read_L_UINT16();
	}

	return (2 * 3 + 4 * coefNum);
}



/******************************************************************************/
/* DecodeSampleData() loads and decode a block of sample data.                */
/*                                                                            */
/* Input:  "file" is a pointer to a PFile object with the file to load from.  */
/*         "buffer" is a pointer to the buffer you has to fill with the data. */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples filled in the buffer.                        */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::DecodeSampleData(PFile *file, float *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 filled = 0;
	uint32 samplesThisBlock, todo;
	uint16 i;

	while (length > 0)
	{
		// Do we need to load some data from the file?
		if (samplesLeft == 0)
		{
			// Yes, read the block data
			samplesThisBlock = GetFileData(file, decodeBuffer, blockAlign);
			if (samplesThisBlock == 0)
				break;			// End of file, stop filling

			// The number of bytes read is less than the required.
			// Composite for it.
			if (samplesThisBlock < blockAlign)
				samplesThisBlock = (blockAlign - (6 * convInfo->channels));
			else
				samplesThisBlock = samplesPerBlock * convInfo->channels;

			// Reset the offset into the decode buffer
			offset = 0;

			// Get the block header
			for (i = 0; i < convInfo->channels; i++)
			{
				predictor[i] = decodeBuffer[offset++];
				if (predictor[i] > coefNum)
				{
//					printf("RIFF-WAVE loader: Invalid predictor (%d). May max be %d.\n", predictor[i], coefNum);
					break;
				}
			}

			for (i = 0; i < convInfo->channels; i++)
			{
				delta[i] = (decodeBuffer[offset] & 0xff) | ((decodeBuffer[offset + 1] & 0xff) << 8);
				offset += 2;
			}

			for (i = 0; i < convInfo->channels; i++)
			{
				samp1[i] = (decodeBuffer[offset] & 0xff) | ((decodeBuffer[offset + 1] & 0xff) << 8);
				offset += 2;
			}

			for (i = 0; i < convInfo->channels; i++)
			{
				samp2[i] = (decodeBuffer[offset] & 0xff) | ((decodeBuffer[offset + 1] & 0xff) << 8);
				offset += 2;
			}

			// Decode two samples for the header
			for (i = 0; i < convInfo->channels; i++)
				*buffer++ = samp2[i] / 32768.0f;

			for (i = 0; i < convInfo->channels; i++)
				*buffer++ = samp1[i] / 32768.0f;

			// Subtract the two samples stored from the header
			samplesLeft = samplesThisBlock - 2 * convInfo->channels;

			// Update counter variables
			length -= (2 * convInfo->channels);
			filled += (2 * convInfo->channels);
		}

		// Find the number of samples to return
		todo = min(length, samplesLeft);

		// Update counter variables
		samplesLeft -= todo;
		length      -= todo;
		filled      += todo;

		// Begin to decode the samples
		while (todo > 0)
		{
			// Get the sample byte
			uint8 byte = decodeBuffer[offset++];

			// Decode the first nibble
			*buffer++ = Decode((byte >> 4) & 0x0f, 0) / 32768.0f;

			// Decode second nibble
			if (convInfo->channels > 1)
				*buffer++ = Decode(byte & 0x0f, 1) / 32768.0f;
			else
				*buffer++ = Decode(byte & 0x0f, 0) / 32768.0f;

			// Decrement the counter
			todo -= 2;
		}
	}

	return (filled);
}



/******************************************************************************/
/* GetTotalSampleLength() calculates how many samples that will be returned.  */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of samples the file is.                                 */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::GetTotalSampleLength(const APConverter_SampleFormat *convInfo)
{
	return ((fileSize / blockAlign) * samplesPerBlock * convInfo->channels);
}



/******************************************************************************/
/* CalcFilePosition() calculates the number of bytes to go into the file to   */
/*      reach the position given.                                             */
/*                                                                            */
/* Input:  "position" is the start position from the start of the sample.     */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The position in the file.                                          */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::CalcFilePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	uint32 blockNum;

	// Calculate the block number the position is in
	blockNum = position / (samplesPerBlock * convInfo->channels);
	return (blockNum * blockAlign);
}



/******************************************************************************/
/* CalcSamplePosition() calculates the number of samples from the byte        */
/*      position given.                                                       */
/*                                                                            */
/* Input:  "position" is the position in bytes.                               */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The position in samples.                                           */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::CalcSamplePosition(uint32 position, const APConverter_SampleFormat *convInfo)
{
	return ((position / blockAlign) * samplesPerBlock * convInfo->channels);
}



/******************************************************************************/
/* SaverInitialize() initialize the saver.                                    */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::SaverInitialize(const APConverter_SampleFormat *convInfo)
{
	int32 i;

	// Initialize the coef values
	coefNum = 7;

	coefSets = new ADPCMCOEFSET[7];
	if (coefSets == NULL)
		throw PMemoryException();

	// Fill out the sets
	coefSets[0].coef1 = 256;
	coefSets[0].coef2 = 0;

	coefSets[1].coef1 = 512;
	coefSets[1].coef2 = -256;

	coefSets[2].coef1 = 0;
	coefSets[2].coef2 = 0;

	coefSets[3].coef1 = 192;
	coefSets[3].coef2 = 64;

	coefSets[4].coef1 = 240;
	coefSets[4].coef2 = 0;

	coefSets[5].coef1 = 460;
	coefSets[5].coef2 = -208;

	coefSets[6].coef1 = 392;
	coefSets[6].coef2 = -232;

	// Initialize the state variables
	for (i = 0; i < 16; i++)
		state[i] = 0;

	// Initialize other variables
	encodedSamples = 0;
}



/******************************************************************************/
/* SaverCleanup() frees all the resources allocated.                          */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::SaverCleanup(void)
{
	delete[] packet;
	packet = NULL;

	delete[] encodeBuffer;
	encodeBuffer = NULL;

	delete[] coefSets;
	coefSets = NULL;
}



/******************************************************************************/
/* GetFormatNumber() returns the format number.                               */
/*                                                                            */
/* Output: The format number.                                                 */
/******************************************************************************/
uint16 RIFFWAVEMicrosoftADPCM::GetFormatNumber(void) const
{
	return (WAVE_FORMAT_ADPCM);
}



/******************************************************************************/
/* GetAverageBytesSecond() returns the average bytes per second.              */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The average bytes per second.                                      */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::GetAverageBytesSecond(const APConverter_SampleFormat *convInfo)
{
	uint16 ba = GetBlockAlign(convInfo);

	// Calculate the samples per block value
	samplesPerBlock = (((ba - (7 * convInfo->channels)) * 8) / (4 * convInfo->channels)) + 2;

	return ((convInfo->frequency / samplesPerBlock) * ba);
}



/******************************************************************************/
/* GetBlockAlign() returns the block align.                                   */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The block align.                                                   */
/******************************************************************************/
uint16 RIFFWAVEMicrosoftADPCM::GetBlockAlign(const APConverter_SampleFormat *convInfo)
{
	return (convInfo->channels * 128);
}



/******************************************************************************/
/* GetSampleSize() returns the bit size of each sample.                       */
/*                                                                            */
/* Input:  "sampSize" is the input sample size.                               */
/*                                                                            */
/* Output: The new sample size.                                               */
/******************************************************************************/
uint16 RIFFWAVEMicrosoftADPCM::GetSampleSize(uint16 sampSize)
{
	return (4);
}



/******************************************************************************/
/* WriteExtraFmtInfo() writes any extra information into the fmt chunk.       */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::WriteExtraFmtInfo(PFile *file)
{
	uint16 i;

	file->Write_L_UINT16(4 + 4 * coefNum);
	file->Write_L_UINT16(samplesPerBlock);
	file->Write_L_UINT16(coefNum);

	for (i = 0; i < coefNum; i++)
	{
		file->Write_L_UINT16(coefSets[i].coef1);
		file->Write_L_UINT16(coefSets[i].coef2);
	}
}



/******************************************************************************/
/* WriteFactChunk() writes the fact chunk.                                    */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::WriteFactChunk(PFile *file)
{
	file->Write_B_UINT32('fact');
	file->Write_L_UINT32(4);			// Chunk size

	factPos = file->GetPosition();		// Remember the position, so we can write back the right value when closing
	file->Write_L_UINT32(0);			// Samples written
}



/******************************************************************************/
/* WriteData() write a block of data.                                         */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/*         "buffer" is a pointer to the buffer with the data to write.        */
/*         "length" is the length of the buffer in samples.                   */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of bytes written.                                       */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::WriteData(PFile *file, int8 *buffer, uint32 length, const APConverter_SampleFormat *convInfo)
{
	uint32 written = 0;
	uint32 todo;

	encodedSamples += length;

	while (length > 0)
	{
		// Do we need to allocate the temp buffer?
		if (encodeBuffer == NULL)
		{
/*			if (convInfo->frequency <= 11025)
				encodeSize = 256;
			else if (convInfo->frequency <= 22050)
				encodeSize = 512;
			else if (convInfo->frequency <= 44100)
				encodeSize = 1024;
			else
				encodeSize = 1024;
*/
			encodeSize   = convInfo->channels * samplesPerBlock;
			encodeBuffer = new int16[encodeSize];
			if (encodeBuffer == NULL)
				return (0);

			packet = new uint8[GetBlockAlign(convInfo)];
			if (packet == NULL)
				return (0);

			encodeFilled = 0;
		}

		// Copy the sample data to the temp buffer
		todo = min(length, encodeSize - encodeFilled);
		if (convInfo->bitSize == 8)
		{
			uint32 i;
			int16 *buf = encodeBuffer + encodeFilled;

			// Convert the samples to 16-bit
			for (i = 0; i < todo; i++)
				*buf++ = (*buffer++) << 8;
		}
		else
		{
			memcpy(encodeBuffer + encodeFilled, buffer, todo * sizeof(int16));
			buffer += todo * sizeof(int16);
		}

		// Adjust the counter variables
		length       -= todo;
		encodeFilled += todo;

		// Did we get enough data to do the encode?
		if (encodeFilled == encodeSize)
		{
			written     += EncodeBuffer(file, convInfo);
			encodeFilled = 0;
		}
	}

	return (written);
}



/******************************************************************************/
/* WriteEnd() write the last data or fixing up chunks.                        */
/*                                                                            */
/* Input:  "file" is a pointer to the file object to write to.                */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of bytes written as sample data.                        */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::WriteEnd(PFile *file, const APConverter_SampleFormat *convInfo)
{
	uint32 written;

	// Write what is missing
	written = EncodeBuffer(file, convInfo);

	// Now seek back and write the FACT value
	file->Seek(factPos, PFile::pSeekBegin);
	file->Write_L_UINT32(encodedSamples / convInfo->channels);

	return (written);
}



/******************************************************************************/
/* Decode() decode one nibble to a single 16-bit sample using MSADPCM.        */
/*                                                                            */
/* Input:  "deltaCode" is the delta code to use to create the sample.         */
/*         "channel" is the channel index number to use.                      */
/*                                                                            */
/* Output: The created sample.                                                */
/******************************************************************************/
int16 RIFFWAVEMicrosoftADPCM::Decode(int8 deltaCode, uint16 channel)
{
	int32 iDelta, predict, sample;

	// Compute next Adaptive Scale Factor (ASF)
	iDelta         = delta[channel];
	delta[channel] = (adaptionTable[deltaCode] * iDelta) >> 8;

	// Check for minimum delta value
	if (delta[channel] < 16)
		delta[channel] = 16;

	// If the delta code is negative, extend the sign
	if (deltaCode & 0x08)
		deltaCode -= 0x10;

	// Predict next sample
	predict = ((samp1[channel] * coefSets[predictor[channel]].coef1) + (samp2[channel] * coefSets[predictor[channel]].coef2)) >> 8;

	// Reconstruct original PCM
	sample = (deltaCode * iDelta) + predict;

	// Checking for over- and underflow
	if (sample > 32767)
		sample = 32767;

	if (sample < -32768)
		sample = -32768;

	// Update the states
	samp2[channel] = samp1[channel];
	samp1[channel] = sample;

	return (sample);
}



/******************************************************************************/
/* EncodeBuffer() encode a whole buffer and writes it to the file.            */
/*                                                                            */
/* Input:  "file" is a pointer to the file to use.                            */
/*         "convInfo" is a pointer to the info structure.                     */
/*                                                                            */
/* Output: The number of bytes written to the file.                           */
/******************************************************************************/
uint32 RIFFWAVEMicrosoftADPCM::EncodeBuffer(PFile *file, const APConverter_SampleFormat *convInfo)
{
	uint32 written;
	uint16 ba;

	// Did we get a filled buffer?
	if (encodeFilled < encodeSize)
	{
		// Nup, zero out the rest of the buffer
		memset(encodeBuffer + encodeFilled, 0, (encodeSize - encodeFilled) * sizeof(int16));
	}

	// Encode the buffer
	ba = GetBlockAlign(convInfo);
	AdpcmBlockMashI(convInfo->channels, encodeBuffer, samplesPerBlock, state, packet, ba);

	// And write it to the file
	written = file->Write(packet, ba);

	return (written);
}



/******************************************************************************/
/* AdpcmBlockMashI()                                                          */
/*                                                                            */
/* Input:  "chans" is the total number of channels.                           */
/*         "ip" is a pointer to interleaved input samples.                    */
/*         "n" is the number of samples to encode per channel.                */
/*         "st" is a pointer to input/output steps.                           */
/*         "oBuff" is a pointer to the output buffer.                         */
/*         "ba" is the block align.                                           */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::AdpcmBlockMashI(int32 chans, const int16 *ip, int32 n, int32 *st, uint8 *oBuff, int32 ba)
{
	uint8 *p;
	int32 ch;

	for (p = oBuff + 7 * chans; p < (oBuff + ba); p++)
		*p = 0;

	for (ch = 0; ch < chans; ch++)
		AdpcmMashChannel(ch, chans, ip, n, st + ch, oBuff);
}



/******************************************************************************/
/* AdpcmMashChannel()                                                         */
/*                                                                            */
/* Input:  "ch" is the channel number to encode.                              */
/*         "chans" is the total number of channels.                           */
/*         "ip" is a pointer to interleaved input samples.                    */
/*         "n" is the number of samples to encode per channel.                */
/*         "st" is a pointer to input/output steps.                           */
/*         "oBuff" is a pointer to the output buffer.                         */
/******************************************************************************/
void RIFFWAVEMicrosoftADPCM::AdpcmMashChannel(int32 ch, int32 chans, const int16 *ip, int32 n, int32 *st, uint8 *oBuff)
{
	int16 v[2];
	int32 n0, s0, s1, ss, sMin;
	int32 d, dMin, k, kMin;

	n0 = n / 2;
	if (n0 > 32)
		n0 = 32;

	if (*st < 16)
		*st = 16;

	v[1] = ip[ch];
	v[0] = ip[ch + chans];

	dMin = 0;
	kMin = 0;
	sMin = 0;

	// For each of 7 standard coeff sets, we try compression
	// beginning with last step-value, and with slightly
	// forward-adjusted step-value, taking best of the 14
	for (k = 0; k < 7; k++)
	{
		int32 d0, d1;

		ss = s0 = *st;
		d0 = AdpcmMashS(ch, chans, v, &coefSets[k], ip, n, &ss, NULL);	// With step s0

		s1 = s0;
		AdpcmMashS(ch, chans, v, &coefSets[k], ip, n0, &s1, NULL);

		ss = s1 = (3 * s0 + s1) / 4;
		d1 = AdpcmMashS(ch, chans, v, &coefSets[k], ip, n, &ss, NULL);	// With step s1

		if ((k == 0) || (d0 < dMin) || (d1 < dMin))
		{
			kMin = k;

			if (d0 <= d1)
			{
				dMin = d0;
				sMin = s0;
			}
			else
			{
				dMin = d1;
				sMin = s1;
			}
		}
	}

	*st = sMin;

	d = AdpcmMashS(ch, chans, v, &coefSets[kMin], ip, n, st, oBuff);
	oBuff[ch] = kMin;
}



/******************************************************************************/
/* AdpcmMashS()                                                               */
/*                                                                            */
/* Input:  "ch" is the channel number to encode.                              */
/*         "chans" is the total number of channels.                           */
/*         "v" is the values to use as starting.                              */
/*         "iCoef" is the lin predictor coeffs.                               */
/*         "iBuff" is a pointer to interleaved input samples.                 */
/*         "n" is the number of samples to encode per channel.                */
/*         "ioStep" is a pointer to input/output steps.                       */
/*         "oBuff" is a pointer to the output buffer or NULL.                 */
/*                                                                            */
/* Output: ???                                                                */
/******************************************************************************/
int32 RIFFWAVEMicrosoftADPCM::AdpcmMashS(int32 ch, int32 chans, int16 v[2], const ADPCMCOEFSET *iCoef, const int16 *iBuff, int32 n, int32 *ioStep, uint8 *oBuff)
{
	const int16 *ip, *iTop;
	uint8 *op;
	int32 ox = 0;
	int32 d, v0, v1, step;
	double d2;

	ip   = iBuff + ch;			// Point ip to 1st input sample for this channel
	iTop = iBuff + n * chans;

	v0 = v[0];
	v1 = v[1];

	d   = *ip - v1;				// 1st input sample for this channel
	ip += chans;
	d2  = d * d;				// d2 will be sum of squares of errors, given input v0 and *st
	d   = *ip - v0;				// 2nd input sample for this channel
	ip += chans;
	d2 += d * d;

	step = *ioStep;

	op = oBuff;					// Output pointer (or NULL)
	if (op != NULL)				// NULL means don't output, just compute the rms error
	{
		op   += chans;			// Skip bpred indices
		op   += 2 * ch;			// Channel step size
		op[0] = step;
		op[1] = step >> 8;

		op   += 2 * chans;		// Skip to v0
		op[0] = v0;
		op[1] = v0 >> 8;

		op   += 2 * chans;		// Skip to v1
		op[0] = v1;
		op[1] = v1 >> 8;

		op    = oBuff + 7 * chans;	// Point to base of output nibbles
		ox    = 4 * ch;
	}

	for ( ; ip < iTop; ip += chans)
	{
		int32 vLin, d, dp, c;

		// Make linear prediction for next sample
		vLin = (v0 * iCoef->coef1 + v1 * iCoef->coef2) >> 8;
		d    = *ip - vLin;			// Difference between linear prediction and current sample
		dp   = d + (step << 3) + (step >> 1);
		c    = 0;

		if (dp > 0)
		{
			c = dp / step;
			if (c > 15)
				c = 15;
		}

		c -= 8;
		dp = c * step;			// Quantized estimate of samp - vLin
		c &= 0x0f;				// Mask to 4 bits

		v1 = v0;				// Shift history
		v0 = vLin + dp;

		if (v0 < -0x8000)
			v0 = -0x8000;
		else
		{
			if (v0 > 0x7fff)
				v0 = 0x7fff;
		}

		d   = *ip - v0;
		d2 += d * d;			// Update square-error

		if (op != NULL)
		{
			op[ox >> 3] |= (ox & 4) ? c : (c << 4);
			ox += 4 * chans;
		}

		// Update the step for the next sample
		step = (adaptionTable[c] * step) >> 8;
		if (step < 16)
			step = 16;
	}

	d2 /= n;		// Be sure it's non-negative

	*ioStep = step;

	return ((int32)sqrt(d2));
}
