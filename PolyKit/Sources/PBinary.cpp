/******************************************************************************/
/* PBinary implementation file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

#include <algorithm>
#include <cstring>

#include <cassert>

// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PSynchronize.h"
#include "PBinary.h"


/******************************************************************************/
/* Data structure                                                             */
/******************************************************************************/
typedef struct PBinaryData
{
	PMRSWLock lock;
	int32 refCount;
	int32 bufferLen;
	uint8 *buffer;
} PBinaryData;



/******************************************************************************/
/* Empty buffer                                                               */
/******************************************************************************/
static PBinaryData emptyBuffer;



/******************************************************************************/
/* PBinary class                                                              */
/******************************************************************************/

/******************************************************************************/
/**	Default constructor for creating a PBinary object that holds no data.
 *//***************************************************************************/
PBinary::PBinary(void)
{
	Init();
}



/******************************************************************************/
/**	Standard constructor for creating a PBinary object that is initialized
 *	with a specified memory buffer.
 *
 *	@param buf pointer to the buffer to attach to this object.
 *	@param length the number of bytes of the buffer to attach to this object.
 *//***************************************************************************/
PBinary::PBinary(uint8 *buf, uint32 length)
{
	Init();
	Attach(buf, length);
}



/******************************************************************************/
/**	Copy constructor for creating a PBinary object that is a copy of another
 *	PBinary object.
 *
 *	@param source the PBinary object to copy into this object.
 *//***************************************************************************/
PBinary::PBinary(PBinary &source)
{
	Init();

	*this = source;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PBinary::~PBinary(void)
{
	Release();
}



/******************************************************************************/
/**	Attaches a memory buffer to this object.
 *
 *	@param buf pointer to the buffer to attach to this object.
 *	@param length the number of bytes of the buffer to attach to this object.
 *//***************************************************************************/
void PBinary::Attach(uint8 *buf, uint32 length)
{
	PBinaryData *data;

	// First release current buffer
	Release();

	// Allocate the data structure
	data = new PBinaryData;
	if (data == NULL)
		throw PMemoryException();

	// Fill out the structure
	data->refCount  = 1;
	data->bufferLen = length;
	data->buffer    = buf;

	// Attach it
	buffer = data;
}



/******************************************************************************/
/**	Detaches the internal memory buffer from this object and returns the
 *	pointer to the detached buffer.
 *
 *	@return a pointer to the detached memory buffer.
 *//***************************************************************************/
uint8 *PBinary::Detach(void)
{
	uint8 *buf;

	// Make sure we have a single copy of the buffer
	CopyBeforeWrite();

	if (buffer == &emptyBuffer)
		return (NULL);

	// Get the buffer and detach it
	buf = buffer->buffer;
	delete buffer;
	Init();

	return (buf);
}



/******************************************************************************/
/**	Returns the length of the data contained in this object.
 *
 *	@return the number of bytes of the data contained in this object.
 *//***************************************************************************/
uint32 PBinary::GetLength(void) const
{
	uint32 len;

	// First make sure we can read the length
	buffer->lock.WaitToRead();

	// Get the length
	len = buffer->bufferLen;

	// Done and return it
	buffer->lock.DoneReading();

	return (len);
}



/******************************************************************************/
/**	Returns a pointer to a memory buffer containing the binary data of this
 *	object for read-only purpose.
 *
 *	@return a pointer to a read-only memory buffer containing the binary data
 *		hold by this object.
 *//***************************************************************************/
const uint8 *PBinary::GetBufferForReadOnly(void) const
{
	uint8 *buf;

	// First make sure we can read the length
	buffer->lock.WaitToRead();

	// Get the address
	if (buffer == &emptyBuffer)
		buf = NULL;
	else
		buf = buffer->buffer;

	// Done and return it
	buffer->lock.DoneReading();

	return (buf);
}



/******************************************************************************/
/**	Returns a pointer to a memory buffer containing the binary data of this
 *	object for writing purposes mainly.
 *
 *	@return a pointer to a memory buffer for writing to the binary data hold
 *		by this object.
 *//***************************************************************************/
uint8 *PBinary::GetBufferForWriting(void)
{
	// Make sure we have a copy by ourself
	CopyBeforeWrite();

	return ((uint8 *)GetBufferForReadOnly());
}



/******************************************************************************/
/**	Changes the length of the data contained in this object to the specified
 *	length. If the new length is lesser than the current length, the data will
 *	be truncated to the new length.
 *
 *	@param newLength the new length measures in bytes of the data contained in
 *		this object.
 *//***************************************************************************/
void PBinary::SetLength(uint32 newLength)
{
	// Allocate a new buffer
	CopyBeforeWrite(newLength);
}



/******************************************************************************/
/**	Index operator which returns the byte at the specified index.
 *
 *	@param index the index of the byte into the data contained of this object.
 *
 *	@return the byte at the specified index.
 *//***************************************************************************/
uint8 PBinary::operator [] (int32 index) const
{
	uint8 byte;

	// First make sure we can read the data
	buffer->lock.WaitToRead();

	// Check for valid index
	assert(index >= 0);
	assert(index < buffer->bufferLen);

	// Get the byte
	byte = buffer->buffer[index];

	// Done and return it
	buffer->lock.DoneReading();

	return (byte);
}



/******************************************************************************/
/**	Assignment operator which assigns this object to another object.
 *
 *	@param newBuffer another PBinary object to assign this object to.
 *
 *	@return a pointer to this new object.
 *//***************************************************************************/
const PBinary & PBinary::operator = (const PBinary &newBuffer)
{
	// If the buffers are the same, do nothing
	if (newBuffer.buffer != buffer)
	{
		// Copy the reference
		newBuffer.buffer->lock.WaitToWrite();

		Release();
		buffer = newBuffer.buffer;
		buffer->refCount++;

		newBuffer.buffer->lock.DoneWriting();
	}

	return (*this);
}



/******************************************************************************/
/**	Initializes this object.
 *//***************************************************************************/
void PBinary::Init(void)
{
	// Initialize the empty buffer
	emptyBuffer.refCount  = -1;
	emptyBuffer.bufferLen = 0;
	emptyBuffer.buffer    = NULL;
	buffer = &emptyBuffer;
}



/******************************************************************************/
/**	Allocates a new memory buffer for this object.
 *
 *	@param length the number of bytes to allocate.
 *//***************************************************************************/
void PBinary::AllocBuffer(int32 length)
{
	if (length == 0)
		Init();				// Makes the string empty
	else
	{
		PBinaryData *data;

		// Allocate the buffer structure
		data = new PBinaryData;
		if (data == NULL)
			throw PMemoryException();

		// Allocate space to the buffer
		data->buffer = new uint8[length];
		if (data->buffer == NULL)
		{
			delete data;
			throw PMemoryException();
		}

		// Initialize the structure
		data->refCount  = 1;
		data->bufferLen = length;

		// Remember the structure
		buffer = data;
	}
}



/******************************************************************************/
/**	Releases a reference to the internal memory buffer of this object. The
 *	reference count is decremented and the internal memory buffer is freed if
 *	the new reference count is zero.
 *//***************************************************************************/
void PBinary::Release(void)
{
	if (buffer != &emptyBuffer)
	{
		buffer->lock.WaitToWrite();

		// Count down reference counter
		buffer->refCount--;

		// If there isn't any references left, delete the buffer
		if (buffer->refCount == 0)
		{
			delete[] buffer->buffer;
			delete buffer;
			Init();
		}
		else
			buffer->lock.DoneWriting();
	}
}



/******************************************************************************/
/**	Releases a reference to the specified data. The reference count is
 *	decremented and the memory buffer allocated in relation to the specified
 *	data is freed if the new reference count is zero.
 *
 *	@param data is a pointer to the data structure to release.
 *//***************************************************************************/
void PBinary::Release(PBinaryData *data)
{
	if (data != &emptyBuffer)
	{
		data->lock.WaitToWrite();
	
		// Count down reference counter
		data->refCount--;

		// If there isn't any references left, delete the buffer
		if (data->refCount == 0)
		{
			delete[] data->buffer;
			delete data;
		}
		else
			data->lock.DoneWriting();
	}
}



/******************************************************************************/
/**	Creates a copy of the memory buffer which this object is refering to. This
 *	copy will be used as the new internal memory buffer of this object.
 *
 *	@param newLength the length measured in bytes of the new memory buffer
 *		copy. If newLength is equal to -1, the current length of the memory
 *		buffer is maintained.
 *//***************************************************************************/
void PBinary::CopyBeforeWrite(int32 newLength)
{
	int32 count, len;

	// Get the reference count and current length
	buffer->lock.WaitToRead();
	count = buffer->refCount;
	len   = buffer->bufferLen;
	buffer->lock.DoneReading();

	// Use the current length
	if (newLength == -1)
		newLength = len;

	if ((count > 1) || (newLength != len))
	{
		PBinaryData *oldData;

		// Remember old data pointers
		oldData = buffer;

		// Allocate new buffer
		AllocBuffer(newLength);

		// Copy the string to the new buffer
		if ((len != 0) && (newLength != 0))
			memcpy(buffer->buffer, oldData->buffer, min(newLength, len));

		// Release the reference
		Release(oldData);
	}
}
