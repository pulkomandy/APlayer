/******************************************************************************/
/* PBinary header file.                                                       */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PBinary_h
#define __PBinary_h

// PolyKit headers
#include "POS.h"
#include "ImportExport.h"


/******************************************************************************/
/* PBinary class                                                              */
/******************************************************************************/
struct PBinaryData;

#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

/**
 *	This is a container class used for holding binary data in a temporary
 *	memory buffer. When using this class, it is not neccessary to keep track
 *	of the length of the binary data as the class maintains the length of the
 *	buffer for you automatically.
 *
 *	The PBinary class is similar to the PString class as it keeps a reference
 *	counter on the data that it holds. This means assigning one PBinary object
 *	to another PBinary object is very fast as the two objects will point at
 *	exactly the same data. The data buffer will first be freed when the last
 *	object that points at the data is destroyed.
 */
class _IMPEXP_PKLIB PBinary
{
public:
	PBinary(void);
	PBinary(uint8 *buffer, uint32 length);
	PBinary(PBinary &source);
	virtual ~PBinary(void);

	void Attach(uint8 *buf, uint32 length);
	uint8 *Detach(void);

	uint32 GetLength(void) const;
	const uint8 *GetBufferForReadOnly(void) const;
	uint8 *GetBufferForWriting(void);
	void SetLength(uint32 newLength);

	uint8 operator [] (int32 index) const;

	const PBinary & operator = (const PBinary &newBuffer);

protected:
	void Init(void);

	void AllocBuffer(int32 length);
	void Release(void);
	void Release(PBinaryData *data);

	void CopyBeforeWrite(int32 newLength = -1);

	PBinaryData *buffer;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
