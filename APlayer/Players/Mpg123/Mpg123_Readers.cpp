/******************************************************************************/
/* MPG123Player Reader functions.                                             */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* BufDiff() will calculate the difference between the end and start.         */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "start" is the start offset.                                       */
/*         "end" is the end offset.                                           */
/*                                                                            */
/* Output: The difference.                                                    */
/******************************************************************************/
int32 MPG123Player::BufDiff(Reader *rds, int32 start, int32 end)
{
	return ((end >= start) ? end - start : rds->bufSize + end - start);
}



/******************************************************************************/
/* FullRead() will read some data from the file given.                        */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "file" is the file to use.                                         */
/*         "buf" is a pointer to the buffer to store the data read.           */
/*         "count" is the number of bytes to read.                            */
/*                                                                            */
/* Output: The real number of bytes read.                                     */
/******************************************************************************/
int32 MPG123Player::FullRead(Reader *rds, PFile *file, uint8 *buf, int32 count)
{
	int32 ret, cnt = 0;

	while (cnt < count)
	{
		int32 toRead = count - cnt;
		int32 num    = BufDiff(rds, rds->bufPos, rds->bufEnd);

		// If we have some data in the back buffer .. use it first
		if (num > 0)
		{
			int32 part1, part2;

			if (toRead > num)
				toRead = num;

			part1 = rds->bufSize - rds->bufPos;
			if (part1 > toRead)
				part1 = toRead;

			part2 = toRead - part1;

			memcpy(buf + cnt, &rds->backBuf[rds->bufPos], part1);

			if (part2 > 0)
				memcpy(buf + cnt + part1, &rds->backBuf[0], part2);

			rds->bufPos += toRead;
			if (rds->bufPos >= rds->bufSize)
				rds->bufPos -= rds->bufSize;

			ret = toRead;

			if (!rds->mark)
				rds->bufStart = rds->bufPos;
		}
		else
		{
			try
			{
				ret = file->Read(buf + cnt, toRead);
			}
			catch(PFileException e)
			{
				ret = -1;
			}

			if (ret < 0)
				return (ret);

			if (ret == 0)
				break;

			if (rds->mark)
			{
				Readers_AddData(rds, buf + cnt, ret);
				rds->bufPos = rds->bufEnd;
			}
		}

		cnt += ret;
	}

	return (cnt);
}



/******************************************************************************/
/* Readers_AddData() will add some data to the back buffer.                   */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "buf" is a pointer to the data to add.                             */
/*         "len" is the length of the data.                                   */
/******************************************************************************/
void MPG123Player::Readers_AddData(Reader *rds, uint8 *buf, int32 len)
{
	int32 diff, part1, part2, store = len;

	if (store >= rds->bufSize)
		store = rds->bufSize - 1;

	// Check whether the new bytes would overwrite the buffer front
	diff = BufDiff(rds, rds->bufStart, rds->bufEnd);
	if ((diff + store) >= rds->bufSize)
	{
		// +1 because end should never be the same as start if the data is in the buffer
		rds->bufStart += diff + store + 1 - rds->bufSize;
		if (rds->bufStart >= rds->bufSize)
			rds->bufStart -= rds->bufSize;
	}

	part1 = rds->bufSize - rds->bufEnd;
	if (part1 > store)
		part1 = store;

	part2 = store - part1;

	memcpy(rds->backBuf + rds->bufEnd, &buf[len - part1 + part2], part1);
	if (part2 > 0)
		memcpy(rds->backBuf, &buf[len - part2], part2);

	rds->bufEnd += store;
	if (rds->bufEnd >= rds->bufSize)
		rds->bufEnd -= rds->bufSize;
}



/******************************************************************************/
/* Readers_PushbackHeader() will move the file pointer a header backwards.    */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "aLong" is the current 32-bit number read.                         */
/******************************************************************************/
void MPG123Player::Readers_PushbackHeader(Reader *rds, uint32 aLong)
{
	uint8 buf[4];

	if (rds->mark || (rds->bufPos != rds->bufEnd))
	{
		rds->bufPos -= 4;
		if (rds->bufPos < 0)
			rds->bufPos += rds->bufSize;
	}
	else
	{
		buf[0] = (aLong >> 24) & 0xff;
		buf[1] = (aLong >> 16) & 0xff;
		buf[2] = (aLong >> 8) & 0xff;
		buf[3] = (aLong >> 0) & 0xff;
	}

	Readers_AddData(rds, buf, 4);
}



/******************************************************************************/
/* Readers_MarkPos() will remember the current file pointer.                  */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/******************************************************************************/
void MPG123Player::Readers_MarkPos(Reader *rds)
{
	rds->bufStart = rds->bufPos;
	rds->mark     = true;
}



/******************************************************************************/
/* Readers_GotoMark() will set the file pointer to the marked position.       */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/******************************************************************************/
void MPG123Player::Readers_GotoMark(Reader *rds)
{
	rds->mark   = false;
	rds->bufPos = rds->bufStart;
}



/******************************************************************************/
/* DefaultInit() will initialize the reader structure.                        */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to initialize.          */
/******************************************************************************/
void MPG123Player::DefaultInit(Reader *rds)
{
	rds->mark     = false;
	rds->bufEnd   = 0;
	rds->bufStart = 0;
	rds->bufPos   = 0;
	rds->bufSize  = BACKBUF_SIZE;

	rds->backBuf  = new uint8[rds->bufSize];
}



/******************************************************************************/
/* HeadRead() read the frame header.                                          */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "newHead" is a pointer to store the header.                        */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MPG123Player::HeadRead(Reader *rds, uint32 *newHead)
{
	uint8 hbuf[4];

	if (FullRead(rds, mpgFile, hbuf, 4) != 4)
		return (false);

	*newHead = ((uint32)hbuf[0] << 24) | ((uint32)hbuf[1] << 16) | ((uint32)hbuf[2] << 8) | ((uint32)hbuf[3]);

	return (true);
}



/******************************************************************************/
/* HeadShift() shift the next byte from the file into the header.             */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "head" is a pointer to the header.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MPG123Player::HeadShift(Reader *rds, uint32 *head)
{
	uint8 hbuf;

	if (FullRead(rds, mpgFile, &hbuf, 1) != 1)
		return (false);

	*head <<= 8;
	*head  |= hbuf;
	*head  &= 0xffffffff;

	return (true);
}



/******************************************************************************/
/* ReadFrameBody() read the body of the frame.                                */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "buf" is a pointer to store the frame body.                        */
/*         "size" is the length of the buffer.                                */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MPG123Player::ReadFrameBody(Reader *rds, uint8 *buf, int32 size)
{
	int32 len;

	len = FullRead(rds, mpgFile, buf, size);
	if (len != size)
	{
		if (len <= 0)
			return (false);

		memset(buf + len, 0, size - len);
	}

	return (true);
}



/******************************************************************************/
/* SkipBytes() skips the number of bytes given.                               */
/*                                                                            */
/* Input:  "rds" is a pointer to the reader structure to use.                 */
/*         "len" is the number of bytes to skip.                              */
/*                                                                            */
/* Output: The new position or -1 for an error.                               */
/******************************************************************************/
int32 MPG123Player::SkipBytes(Reader *rds, int32 len)
{
	int32 newPos;

	try
	{
		newPos = mpgFile->Seek(len, PFile::pSeekCurrent);
	}
	catch(PFileException e)
	{
		newPos = -1;
	}

	return (newPos);
}
