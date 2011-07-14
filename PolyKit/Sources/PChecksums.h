/******************************************************************************/
/* PChecksums header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PChecksums_h
#define __PChecksums_h

// PolyKit headers
#include "POS.h"
#include "ImportExport.h"


/******************************************************************************/
/* PMD5 class                                                                 */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

class _IMPEXP_PKLIB PMD5
{
public:
	PMD5(void);
	virtual ~PMD5(void);

	void AddBuffer(const uint8 *buffer, int32 length);
	const uint8 *CalculateChecksum(void);

protected:
	void Transform(const uint8 *block);
	void FF(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t);
	void GG(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t);
	void HH(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t);
	void II(uint32 &a, uint32 b, uint32 c, uint32 d, uint32 x, uint32 s, uint32 t);

	inline uint32 RotateLeft(uint32 x, int32 n);

	void FromByteTo32Bit(uint32 *dest, const uint8 *source, int32 length);
	void From32BitToByte(uint8 *dest, const uint32 *source, int32 length);

	uint8 tempBuffer[64];	// Temporary buffer
	uint32 count[2];		// Number of bits, modulo 2^64 (lsb first)
	uint32 curMD5[4];		// Current MD5 checksum

	uint8 checksum[16];		// Holding the last returned MD5 checksum
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
