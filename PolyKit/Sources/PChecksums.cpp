/******************************************************************************/
/* PChecksums implementation file.                                            */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

// PolyKit headers
#include "POS.h"
#include "PChecksums.h"


/******************************************************************************/
/* PMD5 class                                                                 */
/*                                                                            */
/* This is an implemention of the "RSA Data Security, Inc. MD5 Message-Digest */
/* Algorithm". It's based on the sources by Langfine Ltd.                     */
/******************************************************************************/

/******************************************************************************/
/* Different constants used in the algorithm                                  */
/******************************************************************************/
// Magic initialization constants
#define MD5_INIT_STATE_0		0x67452301
#define MD5_INIT_STATE_1		0xefcdab89
#define MD5_INIT_STATE_2		0x98badcfe
#define MD5_INIT_STATE_3		0x10325476

// Constants for Transform routine
#define MD5_S11					7
#define MD5_S12					12
#define MD5_S13					17
#define MD5_S14					22

#define MD5_S21					5
#define MD5_S22					9
#define MD5_S23					14
#define MD5_S24					20

#define MD5_S31					4
#define MD5_S32					11
#define MD5_S33					16
#define MD5_S34					23

#define MD5_S41					6
#define MD5_S42					10
#define MD5_S43					15
#define MD5_S44					21

// Transformation constants - Round 1
#define MD5_T01					0xd76aa478
#define MD5_T02					0xe8c7b756
#define MD5_T03					0x242070db
#define MD5_T04					0xc1bdceee
#define MD5_T05					0xf57c0faf
#define MD5_T06					0x4787c62a
#define MD5_T07					0xa8304613
#define MD5_T08					0xfd469501
#define MD5_T09					0x698098d8
#define MD5_T10					0x8b44f7af
#define MD5_T11					0xffff5bb1
#define MD5_T12					0x895cd7be
#define MD5_T13					0x6b901122
#define MD5_T14					0xfd987193
#define MD5_T15					0xa679438e
#define MD5_T16					0x49b40821

// Transformation constants - Round 2
#define MD5_T17					0xf61e2562
#define MD5_T18					0xc040b340
#define MD5_T19					0x265e5a51
#define MD5_T20					0xe9b6c7aa
#define MD5_T21					0xd62f105d
#define MD5_T22					0x02441453
#define MD5_T23					0xd8a1e681
#define MD5_T24					0xe7d3fbc8
#define MD5_T25					0x21e1cde6
#define MD5_T26					0xc33707d6
#define MD5_T27					0xf4d50d87
#define MD5_T28					0x455a14ed
#define MD5_T29					0xa9e3e905
#define MD5_T30					0xfcefa3f8
#define MD5_T31					0x676f02d9
#define MD5_T32					0x8d2a4c8a

// Transformation constants - Round 3
#define MD5_T33					0xfffa3942
#define MD5_T34					0x8771f681
#define MD5_T35					0x6d9d6122
#define MD5_T36					0xfde5380c
#define MD5_T37					0xa4beea44
#define MD5_T38					0x4bdecfa9
#define MD5_T39					0xf6bb4b60
#define MD5_T40					0xbebfbc70
#define MD5_T41					0x289b7ec6
#define MD5_T42					0xeaa127fa
#define MD5_T43					0xd4ef3085
#define MD5_T44					0x04881d05
#define MD5_T45					0xd9d4d039
#define MD5_T46					0xe6db99e5
#define MD5_T47					0x1fa27cf8
#define MD5_T48					0xc4ac5665

// Transformation constants - Round 4
#define MD5_T49					0xf4292244
#define MD5_T50					0x432aff97
#define MD5_T51					0xab9423a7
#define MD5_T52					0xfc93a039
#define MD5_T53					0x655b59c3
#define MD5_T54					0x8f0ccc92
#define MD5_T55					0xffeff47d
#define MD5_T56					0x85845dd1
#define MD5_T57					0x6fa87e4f
#define MD5_T58					0xfe2ce6e0
#define MD5_T59					0xa3014314
#define MD5_T60					0x4e0811a1
#define MD5_T61					0xf7537e82
#define MD5_T62					0xbd3af235
#define MD5_T63					0x2ad7d2bb
#define MD5_T64					0xeb86d391



/******************************************************************************/
/* Padding table                                                              */
/******************************************************************************/
static uint8 padBuffer[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PMD5::PMD5(void)
{
	// Initialize member variables
	count[0]  = 0;
	count[1]  = 0;

	curMD5[0] = MD5_INIT_STATE_0;
	curMD5[1] = MD5_INIT_STATE_1;
	curMD5[2] = MD5_INIT_STATE_2;
	curMD5[3] = MD5_INIT_STATE_3;

	// Clear buffer
	memset(tempBuffer, 0, sizeof(tempBuffer));
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PMD5::~PMD5(void)
{
}



/******************************************************************************/
/* AddBuffer() will add a memory buffer to be included in the final checksum. */
/*                                                                            */
/* Input:  "buffer" is a pointer to the buffer to add.                        */
/*         "length" is the length of the buffer.                              */
/******************************************************************************/
void PMD5::AddBuffer(const uint8 *buffer, int32 length)
{
	uint32 index;
	int32 i, partLen;

	// Compute number of bytes modulus 64
	index = (count[0] >> 3) & 0x3f;

	// Update number of bits
	count[0] += ((uint32)length << 3);
	if (count[0] < ((uint32)length << 3))
		count[1]++;

	count[1] += (length >> 29);

	// Transform as many times as possible
	partLen = 64 - index;
	if (length >= partLen)
	{
		memcpy(&tempBuffer[index], buffer, partLen);
		Transform(tempBuffer);

		for (i = partLen; (i + 63) < length; i += 64)
			Transform(&buffer[i]);

		index = 0;
	}
	else
		i = 0;

	// Buffer remaining input
	memcpy(&tempBuffer[index], &buffer[i], length - i);
}



/******************************************************************************/
/* CalculateChecksum() will calculate the MD5 checksum and return a pointer   */
/*      to a buffer holding the checksum.                                     */
/*                                                                            */
/* Output: A pointer to the buffer holding the 16 bytes (128 bit) long        */
/*         checksum.                                                          */
/******************************************************************************/
const uint8 *PMD5::CalculateChecksum(void)
{
	uint8 bits[8];
	int32 index, padLen;

	// Save number of bits
	From32BitToByte(bits, count, 8);

	// Pad out to 56 modulus 64
	index  = (count[0] >> 3) & 0x3f;
	padLen = (index < 56) ? (56 - index) : (120 - index);
	AddBuffer(padBuffer, padLen);

	// Append length
	AddBuffer(bits, 8);

	// Store final state
	From32BitToByte(checksum, curMD5, 16);

	// And return it
	return (checksum);
}



/******************************************************************************/
/* Transform() is the MD5 basic transformation algorithm.                     */
/*                                                                            */
/* Input:  "block" is a pointer to the data buffer to transform.              */
/******************************************************************************/
void PMD5::Transform(const uint8 *block)
{
	uint32 a, b, c, d;
	uint32 x[16];

	// Initialize local data with current checksum
	a = curMD5[0];
	b = curMD5[1];
	c = curMD5[2];
	d = curMD5[3];

	// Convert the block of bytes to unsigned 32-bit numbers
	FromByteTo32Bit(x, block, 64);

	// Perform round 1 of the transformation
	FF(a, b, c, d, x[ 0], MD5_S11, MD5_T01);
	FF(d, a, b, c, x[ 1], MD5_S12, MD5_T02);
	FF(c, d, a, b, x[ 2], MD5_S13, MD5_T03);
	FF(b, c, d, a, x[ 3], MD5_S14, MD5_T04);
	FF(a, b, c, d, x[ 4], MD5_S11, MD5_T05);
	FF(d, a, b, c, x[ 5], MD5_S12, MD5_T06);
	FF(c, d, a, b, x[ 6], MD5_S13, MD5_T07);
	FF(b, c, d, a, x[ 7], MD5_S14, MD5_T08);
	FF(a, b, c, d, x[ 8], MD5_S11, MD5_T09);
	FF(d, a, b, c, x[ 9], MD5_S12, MD5_T10);
	FF(c, d, a, b, x[10], MD5_S13, MD5_T11);
	FF(b, c, d, a, x[11], MD5_S14, MD5_T12);
	FF(a, b, c, d, x[12], MD5_S11, MD5_T13);
	FF(d, a, b, c, x[13], MD5_S12, MD5_T14);
	FF(c, d, a, b, x[14], MD5_S13, MD5_T15);
	FF(b, c, d, a, x[15], MD5_S14, MD5_T16);

	// Perform round 2 of the transformation
	GG(a, b, c, d, x[ 1], MD5_S21, MD5_T17);
	GG(d, a, b, c, x[ 6], MD5_S22, MD5_T18);
	GG(c, d, a, b, x[11], MD5_S23, MD5_T19);
	GG(b, c, d, a, x[ 0], MD5_S24, MD5_T20);
	GG(a, b, c, d, x[ 5], MD5_S21, MD5_T21);
	GG(d, a, b, c, x[10], MD5_S22, MD5_T22);
	GG(c, d, a, b, x[15], MD5_S23, MD5_T23);
	GG(b, c, d, a, x[ 4], MD5_S24, MD5_T24);
	GG(a, b, c, d, x[ 9], MD5_S21, MD5_T25);
	GG(d, a, b, c, x[14], MD5_S22, MD5_T26);
	GG(c, d, a, b, x[ 3], MD5_S23, MD5_T27);
	GG(b, c, d, a, x[ 8], MD5_S24, MD5_T28);
	GG(a, b, c, d, x[13], MD5_S21, MD5_T29);
	GG(d, a, b, c, x[ 2], MD5_S22, MD5_T30);
	GG(c, d, a, b, x[ 7], MD5_S23, MD5_T31);
	GG(b, c, d, a, x[12], MD5_S24, MD5_T32);

	// Perform round 3 of the transformation
	HH(a, b, c, d, x[ 5], MD5_S31, MD5_T33);
	HH(d, a, b, c, x[ 8], MD5_S32, MD5_T34);
	HH(c, d, a, b, x[11], MD5_S33, MD5_T35);
	HH(b, c, d, a, x[14], MD5_S34, MD5_T36);
	HH(a, b, c, d, x[ 1], MD5_S31, MD5_T37);
	HH(d, a, b, c, x[ 4], MD5_S32, MD5_T38);
	HH(c, d, a, b, x[ 7], MD5_S33, MD5_T39);
	HH(b, c, d, a, x[10], MD5_S34, MD5_T40);
	HH(a, b, c, d, x[13], MD5_S31, MD5_T41);
	HH(d, a, b, c, x[ 0], MD5_S32, MD5_T42);
	HH(c, d, a, b, x[ 3], MD5_S33, MD5_T43);
	HH(b, c, d, a, x[ 6], MD5_S34, MD5_T44);
	HH(a, b, c, d, x[ 9], MD5_S31, MD5_T45);
	HH(d, a, b, c, x[12], MD5_S32, MD5_T46);
	HH(c, d, a, b, x[15], MD5_S33, MD5_T47);
	HH(b, c, d, a, x[ 2], MD5_S34, MD5_T48);

	// Perform round 4 of the transformation
	II(a, b, c, d, x[ 0], MD5_S41, MD5_T49);
	II(d, a, b, c, x[ 7], MD5_S42, MD5_T50);
	II(c, d, a, b, x[14], MD5_S43, MD5_T51);
	II(b, c, d, a, x[ 5], MD5_S44, MD5_T52);
	II(a, b, c, d, x[12], MD5_S41, MD5_T53);
	II(d, a, b, c, x[ 3], MD5_S42, MD5_T54);
	II(c, d, a, b, x[10], MD5_S43, MD5_T55);
	II(b, c, d, a, x[ 1], MD5_S44, MD5_T56);
	II(a, b, c, d, x[ 8], MD5_S41, MD5_T57);
	II(d, a, b, c, x[15], MD5_S42, MD5_T58);
	II(c, d, a, b, x[ 6], MD5_S43, MD5_T59);
	II(b, c, d, a, x[13], MD5_S44, MD5_T60);
	II(a, b, c, d, x[ 4], MD5_S41, MD5_T61);
	II(d, a, b, c, x[11], MD5_S42, MD5_T62);
	II(c, d, a, b, x[ 2], MD5_S43, MD5_T63);
	II(b, c, d, a, x[ 9], MD5_S44, MD5_T64);

	// Add the transformed values to the current checksum
	curMD5[0] += a;
	curMD5[1] += b;
	curMD5[2] += c;
	curMD5[3] += d;
}



/******************************************************************************/
/* FF() is used in the round 1 transformation.                                */
/*                                                                            */
/* Input:  "a", "b", "c", "d" is the current (partial) checksum.              */
/*         "x" is the input data.                                             */
/*         "s" is the MD5_Sxx transformation constant.                        */
/*         "t" is the MD5_Txx transformation constant.                        */
/******************************************************************************/
void PMD5::FF(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t)
{
	uint32 f;

	f  = (b & c) | (~b & d);
	a += f + x + t;
	a  = RotateLeft(a, s);
	a += b;
}



/******************************************************************************/
/* GG() is used in the round 2 transformation.                                */
/*                                                                            */
/* Input:  "a", "b", "c", "d" is the current (partial) checksum.              */
/*         "x" is the input data.                                             */
/*         "s" is the MD5_Sxx transformation constant.                        */
/*         "t" is the MD5_Txx transformation constant.                        */
/******************************************************************************/
void PMD5::GG(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t)
{
	uint32 g;

	g  = (b & d) | (c & ~d);
	a += g + x + t;
	a  = RotateLeft(a, s);
	a += b;
}



/******************************************************************************/
/* HH() is used in the round 3 transformation.                                */
/*                                                                            */
/* Input:  "a", "b", "c", "d" is the current (partial) checksum.              */
/*         "x" is the input data.                                             */
/*         "s" is the MD5_Sxx transformation constant.                        */
/*         "t" is the MD5_Txx transformation constant.                        */
/******************************************************************************/
void PMD5::HH(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t)
{
	uint32 h;

	h  = (b ^ c ^ d);
	a += h + x + t;
	a  = RotateLeft(a, s);
	a += b;
}



/******************************************************************************/
/* II() is used in the round 4 transformation.                                */
/*                                                                            */
/* Input:  "a", "b", "c", "d" is the current (partial) checksum.              */
/*         "x" is the input data.                                             */
/*         "s" is the MD5_Sxx transformation constant.                        */
/*         "t" is the MD5_Txx transformation constant.                        */
/******************************************************************************/
void PMD5::II(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t)
{
	uint32 i;

	i  = (c ^ (b | ~d));
	a += i + x + t;
	a  = RotateLeft(a, s);
	a += b;
}



/******************************************************************************/
/* RotateLeft() rotates the bits in a 32-bit unsigned number left by a        */
/*      specified amount.                                                     */
/*                                                                            */
/* Input:  "x" is the value to be rotated.                                    */
/*         "n" is the number of bit to rotate.                                */
/*                                                                            */
/* Output: The new rotated number.                                            */
/******************************************************************************/
inline uint32 PMD5::RotateLeft(uint32 x, int32 n)
{
	// Rotate and return x
	return ((x << n) | (x >> (32 - n)));
}



/******************************************************************************/
/* FromByteTo32Bit() will convert a given buffer from bytes to 32-bit         */
/*      numbers.                                                              */
/*                                                                            */
/* Input:  "dest" is a pointer to store the converted data.                   */
/*         "source" is a pointer to where to read the data to convert.        */
/*         "length" is the length of the source buffer.                       */
/******************************************************************************/
void PMD5::FromByteTo32Bit(uint32 *dest, const uint8 *source, int32 length)
{
	int32 i, j;

	for (i = 0, j = 0; j < length; i++, j += 4)
		dest[i] = P_LENDIAN_TO_HOST_INT32(*((uint32 *)(source + j)));
}



/******************************************************************************/
/* From32BitToByte() will convert a given buffer from 32-bit numbers to       */
/*      bytes.                                                                */
/*                                                                            */
/* Input:  "dest" is a pointer to store the converted data.                   */
/*         "source" is a pointer to where to read the data to convert.        */
/*         "length" is the length of the source buffer.                       */
/******************************************************************************/
void PMD5::From32BitToByte(uint8 *dest, const uint32 *source, int32 length)
{
	int32 i, j;

	for (i = 0, j = 0; j < length; i++, j += 4)
	{
		dest[j]     = (source[i] & 0xff);
		dest[j + 1] = ((source[i] >> 8) & 0xff);
		dest[j + 2] = ((source[i] >> 16) & 0xff);
		dest[j + 3] = ((source[i] >> 24) & 0xff);
	}
}
