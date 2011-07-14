/******************************************************************************/
/* MPG123Player GetBits functions.                                            */
/******************************************************************************/


// Player headers
#include "Mpg123Player.h"


/******************************************************************************/
/* BackBits() go backs the number of bits you give in the stream.             */
/*                                                                            */
/* Input:  "bitBuf" is a pointer to the bit stream buffer to use.             */
/*         "numberOfBits" is the number of bits to go back.                   */
/******************************************************************************/
void MPG123Player::BackBits(Bitstream_Info *bitBuf, int32 numberOfBits)
{
	bitBuf->bitIndex    -= numberOfBits;
	bitBuf->wordPointer += (bitBuf->bitIndex >> 3);
	bitBuf->bitIndex    &= 0x7;
}



/******************************************************************************/
/* GetBitOffset() will calculate the bit offset.                              */
/*                                                                            */
/* Input:  "bitBuf" is a pointer to the bit stream buffer to use.             */
/*                                                                            */
/* Output: The offset.                                                        */
/******************************************************************************/
int32 MPG123Player::GetBitOffset(Bitstream_Info *bitBuf)
{
	return ((-bitBuf->bitIndex) & 0x7);
}



/******************************************************************************/
/* GetByte() returns the next byte in the stream                              */
/*                                                                            */
/* Input:  "bitBuf" is a pointer to the bit stream buffer to use.             */
/*                                                                            */
/* Output: The byte.                                                          */
/******************************************************************************/
int32 MPG123Player::GetByte(Bitstream_Info *bitBuf)
{
	// !! GetByte() called unsynched
	ASSERT(!bitBuf->bitIndex);

	return (*bitBuf->wordPointer++);
}



/******************************************************************************/
/* GetBits() will get the given number of bits from the stream and return     */
/*      them.                                                                 */
/*                                                                            */
/* Input:  "bitBuf" is a pointer to the bit stream buffer to use.             */
/*         "numberOfBits" is the number of bits to get.                       */
/*                                                                            */
/* Output: The bits.                                                          */
/******************************************************************************/
uint32 MPG123Player::GetBits(Bitstream_Info *bitBuf, int32 numberOfBits)
{
	uint32 rVal;

	if (!numberOfBits)
		return (0);

	{
		rVal   = bitBuf->wordPointer[0];
		rVal <<= 8;
		rVal  |= bitBuf->wordPointer[1];
		rVal <<= 8;
		rVal  |= bitBuf->wordPointer[2];

		rVal <<= bitBuf->bitIndex;
		rVal  &= 0xffffff;

		bitBuf->bitIndex += numberOfBits;

		rVal >>= (24 - numberOfBits);

		bitBuf->wordPointer += (bitBuf->bitIndex >> 3);
		bitBuf->bitIndex &= 7;
	}

	return (rVal);
}



/******************************************************************************/
/* GetBitsFast() will get the given number of bits from the stream and return */
/*      them.                                                                 */
/*                                                                            */
/* Input:  "bitBuf" is a pointer to the bit stream buffer to use.             */
/*         "numberOfBits" is the number of bits to get.                       */
/*                                                                            */
/* Output: The bits.                                                          */
/******************************************************************************/
uint32 MPG123Player::GetBitsFast(Bitstream_Info *bitBuf, int32 numberOfBits)
{
	uint32 rVal;

	rVal   = (uint8)(bitBuf->wordPointer[0] << bitBuf->bitIndex);
	rVal  |= ((uint32)bitBuf->wordPointer[1] << bitBuf->bitIndex) >> 8;
	rVal <<= numberOfBits;
	rVal >>= 8;

	bitBuf->bitIndex += numberOfBits;

	bitBuf->wordPointer += (bitBuf->bitIndex >> 3);
	bitBuf->bitIndex    &= 7;

	return (rVal);
}



/******************************************************************************/
/* Get1Bit() will return the next bit from the stream.                        */
/*                                                                            */
/* Input:  "bitBuf" is a pointer to the bit stream buffer to use.             */
/*                                                                            */
/* Output: The bit.                                                           */
/******************************************************************************/
uint32 MPG123Player::Get1Bit(Bitstream_Info *bitBuf)
{
	uint8 rVal;

	rVal = *(bitBuf->wordPointer) << bitBuf->bitIndex;

	bitBuf->bitIndex++;
	bitBuf->wordPointer += (bitBuf->bitIndex >> 3);
	bitBuf->bitIndex    &= 7;

	return (rVal >> 7);
}
