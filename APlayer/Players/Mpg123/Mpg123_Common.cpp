/******************************************************************************/
/* MPG123Player Common functions.                                             */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* ReadFrameInit() initialize the frame reader.                               */
/*                                                                            */
/* Input:  "fr" is a pointer to the frame structure to use.                   */
/******************************************************************************/
void MPG123Player::ReadFrameInit(Frame *fr)
{
	fr->firstHead      = 0;
	fr->thisHead       = 0;
	fr->freeFormatSize = 0;
}



/******************************************************************************/
/* HeadCheck() will check the header to see if it's valid.                    */
/*                                                                            */
/* Input:  "head" is the header to check.                                     */
/*                                                                            */
/* Output: True if it's ok, false if not.                                     */
/******************************************************************************/
bool MPG123Player::HeadCheck(uint32 head)
{
	// Check sync
	if ((head & 0xffe00000) != 0xffe00000)
		return (false);

	// Check layer
	if (!((head >> 17) & 3))
		return (false);

	// Check bitrate
	if (((head >> 12) & 0xf) == 0xf)
		return (false);

	// Check frequency
	if (((head >> 10) & 0x3) == 0x3)
		return (false);

	return (true);
}



/******************************************************************************/
/* SyncStream() will synchronize the MPEG stream.                             */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "fr" is a pointer to the frame structure to use.                   */
/*         "flags" indicate what to skip.                                     */
/*         "skipped" is a pointer where the number of bytes skipped will be   */
/*         stored.                                                            */
/*                                                                            */
/* Output:  0: EOF or other stream error                                      */
/*         -1: Giving up                                                      */
/*          1: Synched                                                        */
/******************************************************************************/
int32 MPG123Player::SyncStream(Reader *rds, Frame *fr, int32 flags, int32 *skipped)
{
	int32 i, j, l, ret;
	uint32 firstHead, nextHead;
	Frame frameInfo, nextInfo;
	uint8 dummyBuf[MAX_INPUT_FRAMESIZE];
	bool found = false;
	int32 freeFormatSize = 0;

	for (i = 0; i < SCAN_LENGTH; i++)
	{
		// Store our current position
		Readers_MarkPos(rds);

		if (!HeadRead(rds, &firstHead))
			return (0);

		// First a few simple checks
		if (!HeadCheck(firstHead) || !DecodeHeader(&frameInfo, firstHead))
		{
			// Check for RIFF headers
			if ((flags & CHECK_FOR_RIFF) && (firstHead == (('R' << 24) + ('I' << 16) + ('F' << 8) + 'F')))
			{
				ret = SkipRiff(rds);
				if (ret > 0)
				{
					// RIFF was ok, continue with next byte
					*skipped += ret + 4;
					continue;
				}

				if (ret == 0)
					return (0);
			}

			// Check for old ID3 header (or better, footer ;)
			if ((flags & CHECK_FOR_ID3_V1) && ((firstHead >> 8) == (('T' << 16) + ('A' << 8) + 'G')))
				;

			// Check for new ID3 header
			if ((flags & CHECK_FOR_ID3_V2) && ((firstHead >> 8) == (('I' << 16) + ('D' << 8) + '3')))
			{
				if ((firstHead & 0xff) != 0xff)
				{
					ret = SkipNewID3(rds);
					if (!ret)
						return (0);

					if (ret > 0)
					{
						*skipped += ret + 4;
						continue;
					}
				}
			}

			// Reset to old mark and continue
			Readers_GotoMark(rds);

			if (!ReadFrameBody(rds, dummyBuf, 1))
				return (0);

			(*skipped)++;
			continue;
		}

		found          = false;
		freeFormatSize = 0;

		//
		// At the first free format packet, we do not know the size
		//
		if (frameInfo.bitRateIndex == 0)
		{
			int32 maxFrameSize = MAX_INPUT_FRAMESIZE;	// FIXME depends on layer and sampling freq

			if (!HeadRead(rds, &nextHead))
				return (0);

			for (j = 0; j < maxFrameSize; j++)
			{
				if (HeadCheck(nextHead) && ((nextHead & (SYNC_HEAD_MASK | SYNC_HEAD_MASK_FF)) == (firstHead & (SYNC_HEAD_MASK | SYNC_HEAD_MASK_FF))) && DecodeHeader(&nextInfo, nextHead))
				{
					freeFormatSize = j - frameInfo.padSize;
					found          = true;
					break;
				}

				if (!HeadShift(rds, &nextHead))
					return (0);
			}
		}
		else
		{
			if (!ReadFrameBody(rds, dummyBuf, frameInfo.frameSize))
				return (0);

			if (!HeadRead(rds, &nextHead))
				return (0);

			if (HeadCheck(nextHead) && ((nextHead & SYNC_HEAD_MASK) == (firstHead & SYNC_HEAD_MASK)) &&
				((nextHead & SYNC_HEAD_MASK_FF) != 0x0) && DecodeHeader(&nextInfo, nextHead))
			{
				found = true;
			}
		}

		if (!found)
		{
			// Reset to old mark and continue
			Readers_GotoMark(rds);

			if (!ReadFrameBody(rds, dummyBuf, 1))
				return (0);

			(*skipped)++;
			continue;
		}

		// Check some more frames
		for (l = 0; l < LOOK_AHEAD_NUM; l++)
		{
			int32 size;

			if (freeFormatSize > 0)
				size = freeFormatSize + nextInfo.padSize;
			else
				size = nextInfo.frameSize;

			// Step over data
			if (!ReadFrameBody(rds, dummyBuf, size))
				return (0);

			if (!HeadRead(rds, &nextHead))
				return (0);

			if (!HeadCheck(nextHead) || ((nextHead & SYNC_HEAD_MASK) != (firstHead & SYNC_HEAD_MASK)) || !DecodeHeader(&nextInfo, nextHead))
			{
				found = false;
				break;
			}

			if (freeFormatSize > 0)
			{
				if ((nextHead & SYNC_HEAD_MASK_FF) != 0x0)
				{
					found = false;
					break;
				}
			}
			else
			{
				if ((nextHead & SYNC_HEAD_MASK_FF) == 0x0)
				{
					found = false;
					break;
				}
			}
		}

		if (found)
			break;

		// Reset to old mark and continue
		Readers_GotoMark(rds);

		if (!ReadFrameBody(rds, dummyBuf, 1))
			return (0);

		(*skipped)++;
	}

	if (i == SCAN_LENGTH)
		return (-1);

	Readers_GotoMark(rds);

	fr->freeFormatSize = freeFormatSize;
	fr->firstHead      = firstHead;

	return (1);
}



/******************************************************************************/
/* SkipRiff() will skip a RIFF header.                                        */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*                                                                            */
/* Output:     0: Read error                                                  */
/*         -1/-2: Illegal RIFF header (= -2 backstep not valid)               */
/*         other: Number of bytes skipped                                     */
/******************************************************************************/
int32 MPG123Player::SkipRiff(Reader *rds)
{
	uint32 length;
	uint8 buf[16];

	// Read header information
	if (!ReadFrameBody(rds, buf, 16))
		return (0);

	// Check 2. signature
	if (strncmp("WAVEfmt ", (char *)buf + 4, 8))
		return (-1);

	// Decode the header length
	length = (uint32)buf[12] + (((uint32)buf[13]) << 8) + (((uint32)buf[14]) << 16) + (((uint32)buf[15]) << 24);

	// Will not store data in backbuff!
	if (!SkipBytes(rds, length))
		return (0);

	// Skip "data" plus length
	if (!ReadFrameBody(rds, buf, 8))
		return (0);

	if (strncmp("data", (char *)buf, 4))
		return (-2);

	return (length + 8 + 16);
}



/******************************************************************************/
/* SkipNewID3() will skip the ID3 header.                                     */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*                                                                            */
/* Output:     0: Read error                                                  */
/*            -1: Illegal ID3 header                                          */
/*         other: Number of bytes skipped                                     */
/******************************************************************************/
int32 MPG123Player::SkipNewID3(Reader *rds)
{
	uint32 length;
	uint8 buf[6];

	// Read more header information
	if (!ReadFrameBody(rds, buf, 6))
		return (0);

	if (buf[0] == 0xff)
		return (-1);

	if ((buf[2] | buf[3] | buf[4] | buf[5]) & 0x80)
		return (-1);

	length   = (uint32)buf[2] & 0x7f;
	length <<= 7;
	length  += (uint32)buf[3] & 0x7f;
	length <<= 7;
	length  += (uint32)buf[4] & 0x7f;
	length <<= 7;
	length  += (uint32)buf[5] & 0x7f;

	// Will not store data in backbuff!
	if (!SkipBytes(rds, length))
		return (0);

	return (length + 6);
}



/******************************************************************************/
/* ReadFrame() will read a single frame.                                      */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "fr" is a pointer to the frame structure to use.                   */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MPG123Player::ReadFrame(Reader *rds, Frame *fr)
{
	uint32 newHead, oldHead;

	oldHead = fr->thisHead;

	while (true)
	{
		if (!(HeadRead(rds, &newHead)))
			return (false);

		if (!HeadCheck(newHead) || !DecodeHeader(fr, newHead))
		{
			if (param.tryResync)
			{
				int32 tries = 0;

				Readers_PushbackHeader(rds, newHead);
				if (SyncStream(rds, fr, 0xffff, &tries) <= 0)
					return (false);
			}
			else
				return (false);
		}
		else
			break;
	}

	fr->headerChange = 2;

	if (oldHead)
	{
		if ((oldHead & 0xc00) == (fr->thisHead & 0xc00))
		{
			if (((oldHead & 0xc0) == 0) && ((fr->thisHead & 0xc0) == 0))
				fr->headerChange = 1;
			else
				if (((oldHead & 0xc0) > 0) && ((fr->thisHead & 0xc0) > 0))
					fr->headerChange = 1;
		}
	}

	if (!fr->bitRateIndex)
		fr->frameSize = fr->freeFormatSize + fr->padSize;

	// Flip/init buffer for layer 3
	// FIXME for reentrance
	bsBufOld        = bsBuf;
	bsBufOld_End    = bsBufEnd[bsNum];
	bsBuf           = bsSpace[bsNum] + 512;
	bsNum           = (bsNum + 1) & 1;
	bsBufEnd[bsNum] = fr->frameSize;

	// Read main data into memory
	if (!ReadFrameBody(rds, bsBuf, fr->frameSize))
		return (false);

	// Test
	if (!vbr)
		vbr = GetVBRHeader(&head, bsBuf, fr);

	bsi.bitIndex    = 0;
	bsi.wordPointer = (uint8 *)bsBuf;

	return (true);
}



/******************************************************************************/
/* DecodeHeader() will read the rest of the frame header and decode it.       */
/*                                                                            */
/* Input:  "fr" is a pointer to store the decoded frame data into.            */
/*         "newHead" is the header.                                           */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MPG123Player::DecodeHeader(Frame *fr, uint32 newHead)
{
	if (!HeadCheck(newHead))
		return (false);

	if (newHead & (1 << 20))
	{
		fr->lsf    = (newHead & (1 << 19)) ? 0x0 : 0x1;
		fr->mpeg25 = false;
	}
	else
	{
		fr->lsf    = 1;
		fr->mpeg25 = true;
	}

	//
	// CHECKME: Should we add more consistency check here?
	// Changed layer, changed CRC bit, changed sampling frequency
	//
	{
		fr->lay = 4 - ((newHead >> 17) & 3);

		if (((newHead >> 10) & 0x3) == 0x3)
			return (false);

		if (fr->mpeg25)
			fr->samplingFrequency = 6 + ((newHead >> 10) & 0x3);
		else
			fr->samplingFrequency = ((newHead >> 10) & 0x3) + (fr->lsf * 3);	

		fr->errorProtection = ((newHead >> 16) & 0x1) ^ 0x1;
	}

	fr->bitRateIndex = ((newHead >> 12) & 0xf);
	fr->padding      = ((newHead >> 9) & 0x1);
	fr->extension    = ((newHead >> 8) & 0x1);
	fr->mode         = ((newHead >> 6) & 0x3);
	fr->modeExt      = ((newHead >> 4) & 0x3);
	fr->copyright    = ((newHead >> 3) & 0x1);
	fr->original     = ((newHead >> 2) & 0x1);
	fr->emphasis     = newHead & 0x3;

	fr->stereo       = (fr->mode == MPG_MD_MONO) ? 1 : 2;

	switch (fr->lay)
	{
		case 1:
		{
			fr->frameSize    = (int32)tabSel_123[fr->lsf][0][fr->bitRateIndex] * 12000;
			fr->frameSize   /= freqs[fr->samplingFrequency];
			fr->frameSize    = ((fr->frameSize + fr->padding) << 2) - 4;
			fr->sideInfoSize = 0;
			fr->padSize      = fr->padding << 2;
			break;
		}

		case 2:
		{
			fr->frameSize    = (int32)tabSel_123[fr->lsf][1][fr->bitRateIndex] * 144000;
			fr->frameSize   /= freqs[fr->samplingFrequency];
			fr->frameSize   += fr->padding - 4;
			fr->sideInfoSize = 0;
			fr->padSize      = fr->padding;
			break;
		}

		case 3:
		{
			if (fr->lsf)
				fr->sideInfoSize = (fr->stereo == 1) ? 9 : 17;
			else
				fr->sideInfoSize = (fr->stereo == 1) ? 17 : 32;

			if (fr->errorProtection)
				fr->sideInfoSize += 2;

			fr->frameSize  = (int32)tabSel_123[fr->lsf][2][fr->bitRateIndex] * 144000;
			fr->frameSize /= freqs[fr->samplingFrequency] << (fr->lsf);
			fr->frameSize  = fr->frameSize + fr->padding - 4;
			fr->padSize    = fr->padding;
			break;
		}

		default:
			return (false);
	}

	if (!fr->bitRateIndex)
		fr->frameSize = 0;

	fr->thisHead = newHead;

	return (true);
}



/******************************************************************************/
/* SetPointer() will change the pointer in the stream.                        */
/*                                                                            */
/* Input:  "ssize" is the size of a buffer.                                   */
/*         "backStep" is the number of bytes to step back.                    */
/******************************************************************************/
void MPG123Player::SetPointer(int32 ssize, int32 backStep)
{
	bsi.wordPointer = bsBuf + ssize - backStep;

	if (backStep)
		memcpy(bsi.wordPointer, bsBufOld + bsBufOld_End - backStep, backStep);

	bsi.bitIndex = 0;
}



/******************************************************************************/
/* ComputeBPF() will compute the BPF (Bytes Per Frame).                       */
/*                                                                            */
/* Input:  "fr" is a pointer to the frame structure to use.                   */
/*                                                                            */
/* Output: The number of bytes per frame.                                     */
/******************************************************************************/
int32 MPG123Player::ComputeBPF(Frame *fr)
{
	int32 bpf;

	if (!fr->bitRateIndex)
		return (fr->freeFormatSize + 4);

	switch (fr->lay)
	{
		case 1:
		{
			bpf  = tabSel_123[fr->lsf][0][fr->bitRateIndex] * 12000;
			bpf /= freqs[fr->samplingFrequency];
			bpf  = ((bpf + fr->padding) << 2);
			break;
		}

		case 2:
		{
			bpf  = tabSel_123[fr->lsf][1][fr->bitRateIndex] * 144000;
			bpf /= freqs[fr->samplingFrequency];
			bpf += fr->padding;
			break;
		}

		case 3:
		{
			bpf  = tabSel_123[fr->lsf][2][fr->bitRateIndex] * 144000;
			bpf /= freqs[fr->samplingFrequency] << (fr->lsf);
			bpf  = bpf + fr->padding;
			break;
		}

		default:
		{
			bpf = 1;
			break;
		}
	}

	return (bpf);
}



/******************************************************************************/
/* CalcNumFrames() calculates how many frames there are in the file.          */
/*                                                                            */
/* Input:  "fr" is a pointer to the frame structure to use.                   */
/*                                                                            */
/* Output: The number of frames.                                              */
/******************************************************************************/
int32 MPG123Player::CalcNumFrames(Frame *fr)
{
	if (vbrFile)
	{
		// VBR files
		return (vbrFrames);
	}

	// Normal files
	return (CalcFileLength() / ComputeBPF(fr));
}



/******************************************************************************/
/* CalcTotalTime() calculates the total time of the file.                     */
/*                                                                            */
/* Input:  "fr" is a pointer to the frame structure to use.                   */
/*         "totalFrames" is the total number of frames in the file.           */
/*                                                                            */
/* Output: The total time in milliseconds.                                    */
/******************************************************************************/
int32 MPG123Player::CalcTotalTime(Frame *fr, int32 totalFrames)
{
	double time;

	if (vbrFile)
	{
		static const double bs[4] = { 0.0, 384.0, 1152.0, 1152.0 };

		time = bs[fr->lay] / freqs[fr->samplingFrequency] * vbrFrames;
	}
	else
	{
		time  = CalcFileLength();
		time /= ((tabSel_123[fr->lsf][fr->lay - 1][fr->bitRateIndex] / 8) * 1000);
	}

	return ((int32)(time * 1000.0));
}



/******************************************************************************/
/* CalcFileLength() calculates the length of the file.                        */
/*                                                                            */
/* Output: The length of the file in bytes.                                   */
/******************************************************************************/
int32 MPG123Player::CalcFileLength(void)
{
	return (mpgFile->GetLength() - firstFramePosition - (tag ? 128 : 0));
}
