/******************************************************************************/
/* PFile implementation file.                                                 */
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
#include "PException.h"
#include "PFile.h"
#include "PDirectory.h"
#include "PSystem.h"


/******************************************************************************/
/* PFile class                                                                */
/******************************************************************************/

/******************************************************************************/
/**	Default constructor for creating a new PFile object.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PFile::PFile(void)
{
	fileOpened = false;
	eof        = false;
	file       = NULL;
}



/******************************************************************************/
/**	Standard constructor for creating a new PFile object and open the file for
 *	this object.
 *
 *	@param fileName file name of the file to open.
 *	@param openFlags flags that specify how the file must be opened.
 *
 *	@exception PFileException
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PFile::PFile(PString fileName, uint16 openFlags)
{
	fileOpened = false;
	eof        = false;

	Open(fileName, openFlags);
}



/******************************************************************************/
/**	Destructor which closes the opened file and destroy this object.
 *//***************************************************************************/
PFile::~PFile(void)
{
	Close();
}



/******************************************************************************/
/**	Opens the file of this object in the open mode specified by the open flags.
 *
 *	@param openFlags flags used for specifying the open mode to use when
 *		opening the file.
 *
 *	@exception PFileException
 *
 *	@see #Close
 *//***************************************************************************/
void PFile::Open(uint16 openFlags)
{
	Open(name, openFlags);
}



/******************************************************************************/
/**	Opens a new file for this object in the open mode specified by the open
 *	flags.
 *
 *	@param fileName file name of the new file to open for this object.
 *	@param openFlags flags used for specifying the open mode to use when
 *		opening the file.
 *
 *	@exception PFileException
 *
 *	@see #Close
 *//***************************************************************************/
void PFile::Open(PString fileName, uint16 openFlags)
{
	char *nameStr;
	uint32 openMode;
	status_t error;

	ASSERT(fileOpened == false);

	name = fileName;

	// Store the open flags concerning desired access
	prevOpenFlags = openFlags & pModeOpenMask;

	// Find out the open flags
	switch (prevOpenFlags)
	{
		case pModeRead:
		{
			openMode = B_READ_ONLY;
			break;
		}

		case pModeWrite:
		{
			openMode = B_WRITE_ONLY;
			break;
		}

		case pModeReadWrite:
		{
			openMode = B_READ_WRITE;
			break;
		}

		default:
		{
			// Open mode not supported
			ASSERT(false);
			openMode = B_READ_ONLY;
			break;
		}
	}

	// Well, BeOS doesn't support "sharing". All files opened
	// on BeOS is shared on both read and writing
	;

	// Or action flags if any
	if (openFlags & pModeCreate)
		openMode |= (B_CREATE_FILE | B_ERASE_FILE);

	if (openFlags & pModeNoTruncate)
		openMode &= ~(B_ERASE_FILE);

	if (openFlags & pModeFailIfExists)
		openMode |= B_FAIL_IF_EXISTS;

	// Now open the file
	file = new BFile();

	error = file->SetTo((nameStr = fileName.GetString()), openMode);
	fileName.FreeBuffer(nameStr);

	if (error != B_OK)
	{
		delete file;

		if (error == B_BAD_VALUE)
			error = B_ENTRY_NOT_FOUND;

		throw PFileException(PSystem::ConvertOSError(error), fileName);
	}

	// Set the opened file flag
	fileOpened = true;
}



/******************************************************************************/
/**	Closes the file for this object.
 *
 *	@see #Open
 *//***************************************************************************/
void PFile::Close(void)
{
	if (fileOpened)
	{
		delete file;
		file = NULL;
	}

	// Cleanup class variables
	fileOpened = false;
}



/******************************************************************************/
/**	Checks if the file for this object is open.
 *
 *	@return true if the file is open; false otherwise.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
bool PFile::IsOpen(void) const
{
	return (fileOpened);
}



/******************************************************************************/
/**	Reads the specified amount of bytes from the file of this object into the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the read bytes must be
 *		stored into.
 *	@param count number of bytes to read.
 *
 *	@return the actual number of bytes that was read from the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
int32 PFile::Read(void *buffer, int32 count)
{
	int32 readBytes;

	ASSERT(fileOpened == true);

	readBytes = file->Read(buffer, count);
	if (readBytes < 0)
		throw PFileException(PSystem::ConvertOSError(readBytes), name);

	// Set the EOF flag
	eof = (readBytes == 0) ? true : false;

	return (readBytes);
}



/******************************************************************************/
/**	Writes the specified amount of bytes into the file of this object from the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the bytes to writes are
 *		stored.
 *	@param count number of bytes to write.
 *
 *	@return the actual number of bytes that was written to the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
int32 PFile::Write(const void *buffer, int32 count)
{
	int32 writtenBytes;

	ASSERT(fileOpened == true);

	// Check the write count
	if (count == 0)
		return (0);

	writtenBytes = file->Write(buffer, count);
	if (writtenBytes < 0)
		throw PFileException(PSystem::ConvertOSError(writtenBytes), name);

	return (writtenBytes);
}



/******************************************************************************/
/**	Reads a string of the limited byte size from the file of this object into
 *	the specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the read bytes must be
 *		stored into.
 *	@param maxLen maximum number of bytes to read.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::ReadString(char *buffer, int32 maxLen)
{
	uint32 bytesRead;

	bytesRead = Read(buffer, maxLen);
	buffer[bytesRead] = 0x00;
}



/******************************************************************************/
/**	Reads a string from the file of this object. The string is in Pascal
 *	format. With the Pascal format the length (number of byte characters) of
 *	the string is specified in the first word (two bytes) read. The byte
 *	characters which the string contain follows the first read word.
 *
 *	@param characterSet pointer to the character set to use when reading the
 *		string. If set to NULL the native host character set is used.
 *
 *	@return the read string.
 *
 *	@exception PFileException
 *//***************************************************************************/
PString PFile::ReadString(PCharacterSet *characterSet)
{
	PString str;
	uint16 len;
	int8 nullLen;
	const char *nullChar;
	char *buffer;

	// Read the length of the string in bytes
	len = Read_L_UINT16();

	if (len != 0)
	{
		PCharacterSet *charSet;

		// Find the character set to use
		if (characterSet == NULL)
			charSet = CreateHostCharacterSet();
		else
			charSet = characterSet;

		// Find the length of the null terminator
		nullChar = charSet->FromUnicode(0x0000, nullLen);

		// Allocate buffer to hold the string
		buffer = new char[len + nullLen];
		if (buffer == NULL)
			throw PMemoryException();

		// Read the string
		Read(buffer, len);

		// Null terminate it
		memcpy(buffer + len, nullChar, nullLen);

		// Store the string
		str.SetString(buffer, charSet);

		// Delete the host character set if no one was given
		if (characterSet == NULL)
			delete charSet;
	}

	return (str);
}



/******************************************************************************/
/**	Reads a line from the file of this object. The line is a character sequence
 *	which is terminated by nul or new-line characters.
 *
 *	@param characterSet pointer to the character set to use when reading the
 *		line. If set to NULL the native host character set is used.
 *
 *	@return the read line.
 *
 *	@exception PFileException
 *//***************************************************************************/
PString PFile::ReadLine(PCharacterSet *characterSet)
{
	PCharacterSet *charSet;
	PChar chr;
	PString retLine;
	PString appendLine;
	int8 charLen, nullLen;
	const char *nullChar;
	int32 bytesRead;
	int32 i;
	char tempBuf[80 + P_MAX_CHAR_LEN];
	bool found, newLine;

	// Clear the last bytes in the temp buffer
	memset(&tempBuf[80], 0, P_MAX_CHAR_LEN);

	// Find the character set to use
	if (characterSet == NULL)
		charSet = CreateHostCharacterSet();
	else
		charSet = characterSet;

	// Find the length of the null terminator
	nullChar = charSet->FromUnicode(0x0000, nullLen);

	// Read until we reach EOF or the line has been read
	while (!IsEOF())
	{
		// Read a bit from the file
		bytesRead = Read(tempBuf, 80);
		if (bytesRead != 0)
		{
			found   = false;
			newLine = false;
			for (i = 0; i < bytesRead; i += charLen)
			{
				// Get the length of the next character
				charLen = charSet->GetCharLength(tempBuf + i);

				// Set the check character
				chr.SetChar(tempBuf + i, charLen, charSet);

				// Ok, did we find the new line character(s)
				if (chr == '\r')
				{
					newLine = true;
					continue;
				}

				if ((chr == '\n') || (chr == 0x00))
				{
					// Found it
					i += charLen;
					found = true;
					break;
				}

				if (newLine)
				{
					// Found it
					found = true;
					break;
				}
			}

			if (i != bytesRead)
			{
				// Seek a little bit back
				Seek(-(bytesRead - i), pSeekCurrent);
			}

			// Append what we have to the return line
			memcpy(&tempBuf[i], nullChar, nullLen);
			appendLine.SetString(tempBuf, charSet);
			retLine += appendLine;

			if (found)
			{
				int32 startIndex;

				if (chr != 0x00)
				{
					// Found the new line, now remove the new line
					// character(s)
					startIndex = retLine.GetLength() - 1;

					if ((startIndex != 0) && (retLine.GetAt(startIndex - 1) == '\r'))
						retLine.Delete(startIndex - 1, 2);
					else
						retLine.Delete(startIndex, 1);
				}

				// Break the read loop
				break;
			}
		}
	}

	// If no character set was given, delete it
	if (characterSet == NULL)
		delete charSet;

	// Return the line
	return (retLine);
}



/******************************************************************************/
/**	Reads a byte (8 bit integer) from the file of this object.
 *
 *	@return the read byte (uint8).
 *
 *	@exception PFileException
 *//***************************************************************************/
uint8 PFile::Read_UINT8(void)
{
	uint8 retVal = 0;		// Return 0 in case of EOF has been reached

	// Read the byte
	Read(&retVal, 1);

	return (retVal);
}



/******************************************************************************/
/**	Reads a 16 bit integer in little endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 16 bit integer (uint16).
 *
 *	@exception PFileException
 *//***************************************************************************/
uint16 PFile::Read_L_UINT16(void)
{
	uint16 retVal = 0;		// Return 0 in case of EOF has been reached

	// Read the number
	Read(&retVal, 2);

	return (P_LENDIAN_TO_HOST_INT16(retVal));
}



/******************************************************************************/
/**	Reads a 32 bit integer in little endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 32 bit integer (uint32).
 *
 *	@exception PFileException
 *//***************************************************************************/
uint32 PFile::Read_L_UINT32(void)
{
	uint32 retVal = 0;		// Return 0 in case of EOF has been reached

	// Read the number
	Read(&retVal, 4);

	return (P_LENDIAN_TO_HOST_INT32(retVal));
}



/******************************************************************************/
/**	Reads an array of 16 bit integers in little endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 16 bit integers.
 *	@param count the number of 16 bit integers to read.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::ReadArray_L_UINT16s(uint16 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		*buffer++ = Read_L_UINT16();
}



/******************************************************************************/
/**	Reads an array of 32 bit integers in little endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 32 bit integers.
 *	@param count the number of 32 bit integers to read.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::ReadArray_L_UINT32s(uint32 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		*buffer++ = Read_L_UINT32();
}



/******************************************************************************/
/**	Reads a 16 bit integer in big endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 16 bit integer (uint16).
 *
 *	@exception PFileException
 *//***************************************************************************/
uint16 PFile::Read_B_UINT16(void)
{
	uint16 retVal = 0;

	// Read the number
	Read(&retVal, 2);

	return (P_BENDIAN_TO_HOST_INT16(retVal));
}



/******************************************************************************/
/**	Reads a 32 bit integer in big endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 32 bit integer (uint32).
 *
 *	@exception PFileException
 *//***************************************************************************/
uint32 PFile::Read_B_UINT32(void)
{
	uint32 retVal = 0;

	// Read the number
	Read(&retVal, 4);

	return (P_BENDIAN_TO_HOST_INT32(retVal));
}



/******************************************************************************/
/**	Reads an array of 16 bit integers in big endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 16 bit integers.
 *	@param count the number of 16 bit integers to read.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::ReadArray_B_UINT16s(uint16 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		*buffer++ = Read_B_UINT16();
}



/******************************************************************************/
/**	Reads an array of 32 bit integers in big endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 32 bit integers.
 *	@param count the number of 32 bit integers to read.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::ReadArray_B_UINT32s(uint32 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		*buffer++ = Read_B_UINT32();
}



/******************************************************************************/
/**	Writes a string to the file of this object. The string will be written in
 *	Pascal format. With the Pascal format the length (number of byte
 *	characters) of the string is specified in the first word (two bytes)
 *	written. The byte characters which the string contain follows immediately
 *	after the length.
 *
 *	@param string the string to write.
 *	@param characterSet pointer to the character set to use when writing the
 *		string. If set to NULL the native host character set is used.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::WriteString(PString string, PCharacterSet *characterSet)
{
	int32 len;
	char *str;

	// First make sure the string is in the character set given
	if (characterSet == NULL)
		string.SwitchToHostCharacterSet();
	else
		string.SwitchCharacterSet(characterSet);

	// Get the character string and length in bytes
	str = string.GetString(&len);

	// Write the length
	Write_L_UINT16((uint16)len);

	if (len != 0)
	{
		// Write the string
		Write(str, len);
	}

	// Free the buffer
	string.FreeBuffer(str);
}



/******************************************************************************/
/**	Writes a line to the file of this object. A new-line character is appended
 *	to the end of the line before it is written to the file.
 *
 *	@param line is the line to write.
 *	@param characterSet pointer to the character set to use when writing the
 *		line. If set to NULL the native host character set is used.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::WriteLine(PString line, PCharacterSet *characterSet)
{
	int32 len;
	char *str;

	// First make sure the string is in the character set given
	if (characterSet == NULL)
		line.SwitchToHostCharacterSet();
	else
		line.SwitchCharacterSet(characterSet);

	// Now append the new line string
	line += P_NEWLINE_STR;

	// Get the character string and length in bytes
	str = line.GetString(&len);

	// Write the line
	Write(str, len);

	// Free the buffer
	line.FreeBuffer(str);
}



/******************************************************************************/
/**	Writes a byte (8 bit integer) to the file of this object.
 *
 *	@param value the byte (uint8) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Write_UINT8(uint8 value)
{
	Write(&value, 1);
}



/******************************************************************************/
/**	Writes a 16 bit integer in little endian format to the file of this object.
 *
 *	@param value the 16 bit integer (uint16) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Write_L_UINT16(uint16 value)
{
	uint16 newValue;

	newValue = P_HOST_TO_LENDIAN_INT16(value);
	Write(&newValue, 2);
}



/******************************************************************************/
/**	Writes a 32 bit integer in little endian format to the file of this object.
 *
 *	@param value the 32 bit integer (uint32) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Write_L_UINT32(uint32 value)
{
	uint32 newValue;

	newValue = P_HOST_TO_LENDIAN_INT32(value);
	Write(&newValue, 4);
}



/******************************************************************************/
/**	Writes an array of 16 bit integers in little endian format to the file of
 *	this object.
 *
 *	@param buffer pointer to the array of 16 bit integers to write.
 *	@param count the number of 16 bit integers to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::WriteArray_L_UINT16s(const uint16 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		Write_L_UINT16(*buffer++);
}



/******************************************************************************/
/**	Writes an array of 32 bit integers in little endian format to the file of
 *	this object.
 *
 *	@param buffer pointer to the array of 32 bit integers to write.
 *	@param count the number of 32 bit integers to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::WriteArray_L_UINT32s(const uint32 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		Write_L_UINT32(*buffer++);
}



/******************************************************************************/
/**	Writes a 16 bit integer in big endian format to the file of this object.
 *
 *	@param value the 16 bit integer (uint16) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Write_B_UINT16(uint16 value)
{
	uint16 newValue;

	newValue = P_HOST_TO_BENDIAN_INT16(value);
	Write(&newValue, 2);
}



/******************************************************************************/
/**	Writes a 32 bit integer in big endian format to the file of this object.
 *
 *	@param value the 32 bit integer (uint32) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Write_B_UINT32(uint32 value)
{
	uint32 newValue;

	newValue = P_HOST_TO_BENDIAN_INT32(value);
	Write(&newValue, 4);
}



/******************************************************************************/
/**	Writes an array of 16 bit integers in big endian format to the file of this
 *	object.
 *
 *	@param buffer pointer to the array of 16 bit integers to write.
 *	@param count the number of 16 bit integers to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::WriteArray_B_UINT16s(const uint16 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		Write_B_UINT16(*buffer++);
}



/******************************************************************************/
/**	Writes an array of 32 bit integers in big endian format to the file of this
 *	object.
 *
 *	@param buffer pointer to the array of 32 bit integers to write.
 *	@param count the number of 32 bit integers to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::WriteArray_B_UINT32s(const uint32 *buffer, int32 count)
{
	for (int32 i = 0; i < count; i++)
		Write_B_UINT32(*buffer++);
}



/******************************************************************************/
/**	Seeks to a new position in the file of this object.
 *
 *	@param offset number of bytes to move in the seek.
 *	@param from is the starting position of the seek.
 *
 *	@return the new position after the seek.
 *
 *	@exception PFileException
 *
 *	@see #SeekToBegin, #SeekToEnd
 *//***************************************************************************/
int64 PFile::Seek(int64 offset, PSeekFlags from)
{
	int64 newPos;
	int32 seekMode;

	ASSERT(fileOpened == true);

	// Convert the from flag to a seek mode flag
	switch (from)
	{
		case pSeekBegin:
		{
			seekMode = SEEK_SET;
			break;
		}

		case pSeekCurrent:
		{
			seekMode = SEEK_CUR;
			break;
		}

		case pSeekEnd:
		{
			seekMode = SEEK_END;
			break;
		}

		default:
		{
			// Unknown seek flag
			ASSERT(false);
			seekMode = SEEK_SET;
			break;
		}
	}

	// Seek the file position
	newPos = file->Seek(offset, seekMode);
	if (newPos < 0)
		throw PFileException(PSystem::ConvertOSError(newPos), name);

	// Clear eof flag
	eof = false;

	return (newPos);
}



/******************************************************************************/
/**	Seeks to the beginning of the file of this object.
 *
 *	@exception PFileException
 *
 *	@see #Seek, #SeekToEnd
 *//***************************************************************************/
void PFile::SeekToBegin(void)
{
	Seek(0, PFile::pSeekBegin);
}



/******************************************************************************/
/**	Seeks to the end of the file of this object.
 *
 *	@exception PFileException
 *
 *	@see #Seek, #SeekToBegin
 *//***************************************************************************/
void PFile::SeekToEnd(void)
{
	Seek(0, PFile::pSeekEnd);
}



/******************************************************************************/
/**	Return the current position in the file of this object.
 *
 *	@return the current file position.
 *
 *	@exception PFileException
 *//***************************************************************************/
int64 PFile::GetPosition(void) const
{
	int64 pos;

	ASSERT(fileOpened == true);

	pos = file->Position();
	if (pos < 0)
		throw PFileException(PSystem::ConvertOSError(pos), name);

	return (pos);
}



/******************************************************************************/
/**	Checks if the end-of-file (EOF) in the file of this object has been
 *	reached.
 *
 *	@return true if the end-of-file (EOF) has been reached; false otherwise.
 *//***************************************************************************/
bool PFile::IsEOF(void) const
{
	return (eof);
}



/******************************************************************************/
/**	Returns the length of the file of this object. That is the number of bytes
 *	that the file contain.
 *
 *	@return the number of bytes the file contain.
 *
 *	@exception PFileException
 *//***************************************************************************/
int64 PFile::GetLength(void) const
{
	int64 length;
	status_t error;

	ASSERT(fileOpened == true);

	error = file->GetSize(&length);
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	return (length);
}



/******************************************************************************/
/**	Sets a new length of the file of this object. This is the new number of
 *	bytes that the file of this object will contain. If the new length is
 *	lesser than the current length, the file will be truncated.
 *
 *	@param newLength the number of bytes the file must contain.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::SetLength(int64 newLength)
{
	status_t error;

	ASSERT(fileOpened == true);

	error = file->SetSize(newLength);
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);
}



/******************************************************************************/
/**	Returns the file name of the file of this object without the file path.
 *
 *	@return the file name of the file without the file path.
 *
 *	@exception PFileException
 *
 *	@see #GetFilePath, #GetFullPath
 *//***************************************************************************/
PString PFile::GetFileName(void) const
{
	PString fileName;
	BEntry entry;
	char *nameStr;
	char nameBuf[B_FILE_NAME_LENGTH];
	status_t error;

	ASSERT(fileOpened == true);

	// Set the entry to the file
	error = entry.SetTo((nameStr = name.GetString()));
	name.FreeBuffer(nameStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Get the name of the entry
	error = entry.GetName(nameBuf);
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Return the name
	fileName = nameBuf;

	return (fileName);
}



/******************************************************************************/
/**	Returns the expanded file path of the file of this object without the file
 *	name.
 *
 *	@return the expanded file path of the file without the file name.
 *
 *	@exception PFileException
 *
 *	@see #GetFileName, #GetFullPath
 *//***************************************************************************/
PString PFile::GetFilePath(void) const
{
	PString path;
	BPath filePath;
	PString fullPath, fileName;
	char *nameStr;
	status_t error;

	ASSERT(fileOpened == true);

	// Set the entry to the file
	error = filePath.SetTo((nameStr = name.GetString()));
	name.FreeBuffer(nameStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Get the path
	fullPath = filePath.Path();
	fileName = filePath.Leaf();

	// Return the path
	path = fullPath.Left(fullPath.GetLength() - fileName.GetLength());

	return (path);
}



/******************************************************************************/
/**	Return the full path of the file of this object containing both the file
 *	path and file name. The returned path is the full expanded path of the
 *	file, regardless if the path of the file is a relative path.
 *
 *	@return the full expanded path of the file.
 *
 *	@exception PFileException
 *
 *	@see #GetFileName, #GetFilePath
 *//***************************************************************************/
PString PFile::GetFullPath(void) const
{
	PString path;
	BPath filePath;
	char *nameStr;
	status_t error;

	ASSERT(fileOpened == true);

	// Set the entry to the file
	error = filePath.SetTo((nameStr = name.GetString()));
	name.FreeBuffer(nameStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Get the path
	path = filePath.Path();

	return (path);
}



/******************************************************************************/
/**	Returns the MIME type string for the file of this object.
 *
 *	@return a string specifying the MIME type of the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
PString PFile::GetFileType(void) const
{
	PString type;
	BNodeInfo nodeInfo;
	char buffer[B_MIME_TYPE_LENGTH];
	status_t error;

	ASSERT(fileOpened == true);

	// Set the node info to the node
	error = nodeInfo.SetTo(file);
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Get the mime string
	error = nodeInfo.GetType(buffer);
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Return the type
	type = buffer;

	return (type);
}



/******************************************************************************/
/**	Sets the file path/name of the file of this object. It does not open the
 *	file, it just associates the object with the name.
 *
 *	@param newName the new file path/name for the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::SetFilePath(PString newName)
{
	ASSERT(fileOpened == false);

	name = newName;
}



/******************************************************************************/
/**	Sets the MIME type of the file of this object.
 *
 *	@param newType the string specifying the new MIME type for the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::SetFileType(PString newType)
{
	BNodeInfo nodeInfo;
	char *typeStr;
	status_t error;

	ASSERT(fileOpened == true);

	// Set the node info to the node
	error = nodeInfo.SetTo(file);
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);

	// Change the mime string
	error = nodeInfo.SetType((typeStr = newType.GetString()));
	newType.FreeBuffer(typeStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), name);
}



/******************************************************************************/
/**	Renames the specified file.
 *
 *	@param oldName the file name of the file to rename.
 *	@param newName the new file name of the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Rename(PString oldName, PString newName)
{
	BEntry entry;
	char *nameStr;
	status_t error;

	// Initialize the entry with the old name
	error = entry.SetTo((nameStr = oldName.GetString()));
	oldName.FreeBuffer(nameStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), oldName);

	// Now rename the file
	error = entry.Rename((nameStr = newName.GetString()));
	newName.FreeBuffer(nameStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), oldName);
}



/******************************************************************************/
/**	Removes the specified file.
 *
 *	@param fileName the file name of the file to remove.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PFile::Remove(PString fileName)
{
	BEntry entry;
	char *nameStr;
	status_t error;

	// Initialize the entry with the name
	error = entry.SetTo((nameStr = fileName.GetString()));
	fileName.FreeBuffer(nameStr);

	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), fileName);

	// Now remove the file
	error = entry.Remove();
	if (error != B_OK)
		throw PFileException(PSystem::ConvertOSError(error), fileName);
}



/******************************************************************************/
/**	Checks if the specified file exists.
 *
 *	@param fileName the file name of the file to check.
 *
 *	@return true if the file exists; false otherwise.
 *
 *	@exception PFileException
 *//***************************************************************************/
bool PFile::FileExists(PString fileName)
{
	BFile file;
	char *nameStr;
	status_t error;

	// Try to open the file
	error = file.SetTo((nameStr = fileName.GetString()), B_READ_ONLY);
	fileName.FreeBuffer(nameStr);

	if (error == B_OK)
		return (true);

	return (false);
}



/******************************************************************************/
/**	Returns the file extension of the specified file.
 *
 *	@param fileName the file name of the file.
 *
 *	@return a string containing the file extension. If no file extension exists
 *		for the file, then an empty string is returned.
 *//***************************************************************************/
PString PFile::GetFileExtension(PString fileName)
{
	PString fileExt;
	int32 pos;

	// Find the position of the dot that marks the start of the file extension
	pos = fileName.ReverseFind('.');

	// Return an empty string if no file extension could be found
	if (pos == -1)
		return ("");

	// Return the file extension not containing the beginning dot
	return (fileName.Mid(pos + 1));
}



/******************************************************************************/
/**	Gets a new file name based on the specified file name and file extension.
 *	If the specified file name does not contain a file extension, the specified
 *	file extension will be added to the file name to return. Else, the file
 *	extension of the specified file name is replaced with the specified file
 *	extension in the returned file name.
 *
 *	@param fileName the file name of the file to request a new file name for.
 *	@param fileExt the desired new file extension for the specified file name.
 *
 *	@return a file name containing the specified file extension.
 *//***************************************************************************/
PString PFile::RequestFileExtension(PString fileName, PString fileExt)
{
	PString newName;
	int32 pos;

	// The file extension must not contain a dot
	ASSERT(fileExt.Find('.') == -1);

	// Find the position of the dot that marks the start of the file extension
	pos = fileName.ReverseFind('.');

	// If no extension exist, then add the file extension to the new file name.
	// Else change the existing file extension for the new file name.
	if (pos == -1)
		newName = fileName + "." + fileExt;
	else
		newName = fileName.Left(pos + 1) + fileExt;

	// Return the new file name
	return (newName);
}



/******************************************************************************/
/**	Duplicates the file contained in this object by creating a new PFile object
 *	containing the same file of this object. The file name and permissions of
 *	the file in the duplicated object will be the same as the original one.
 *
 *	@return a pointer to the new PFile file object. Use the delete operator to
 *		delete/destroy it.
 *
 *	@exception PFileException
 *//***************************************************************************/
PFile *PFile::DuplicateFile(void) const
{
	PFile *newFile;

	// Create new file object
	newFile = new PFile();
	if (newFile == NULL)
		throw PMemoryException();

	try
	{
		if (fileOpened)
		{
			// Open the file
			newFile->Open(name, prevOpenFlags);

			// Set the file position
			newFile->Seek(GetPosition(), pSeekBegin);

			// Copy other variables
			newFile->eof  = eof;
			newFile->name = name;
		}
	}
	catch(PException e)
	{
		delete newFile;
		throw;
	}

	return (newFile);
}





/******************************************************************************/
/* PCacheFile class                                                           */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PCacheFile object.
 *
 *	@param size the number of bytes for the memory cache.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PCacheFile::PCacheFile(uint32 size) : PFile()
{
	cacheSize     = size;
	cacheFilled   = 0;
	cachePosition = 0;
	cacheStart    = 0;

	cache = new uint8[size];
	if (cache == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/**	Standard constructor for creating a new PCacheFile object and open the file
 *	for this object.
 *
 *	@param fileName file name of the file to open.
 *	@param openFlags flags that specify how the file must be opened.
 *	@param size the number of bytes for the memory cache.
 *
 *	@exception PFileException
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PCacheFile::PCacheFile(PString fileName, uint16 openFlags, uint32 size) : PFile(fileName, openFlags)
{
	cacheSize     = size;
	cacheFilled   = 0;
	cachePosition = 0;
	cacheStart    = 0;

	cache = new uint8[size];
	if (cache == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PCacheFile::~PCacheFile(void)
{
	delete[] cache;
}



/******************************************************************************/
/**	Closes the file for this object.
 *
 *	@see #Open
 *//***************************************************************************/
void PCacheFile::Close(void)
{
	// Don't use the cache anymore
	cacheFilled   = 0;
	cachePosition = 0;
	cacheStart    = 0;

	// Call base class
	PFile::Close();
}



/******************************************************************************/
/**	Reads the specified amount of bytes from the file of this object into the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the read bytes must be
 *		stored into.
 *	@param count number of bytes to read.
 *
 *	@return the actual number of bytes that was read from the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
int32 PCacheFile::Read(void *buffer, int32 count)
{
	int32 readBytes = 0;
	int32 left, todo;

	while (count > 0)
	{
		// Calculate how many bytes left in the cache
		left = cacheFilled - cachePosition;

		// Find the number of bytes we can copy
		todo = min(left, count);

		if (todo != 0)
		{
			// Copy the bytes from the cache we can
			memcpy((uint8 *)buffer + readBytes, cache + cachePosition, todo);

			readBytes     += todo;
			cachePosition += todo;
			count         -= todo;
		}
		else
		{
			// Fill the cache up again
			cacheStart   += cacheFilled;
			cacheFilled   = PFile::Read(cache, cacheSize);
			cachePosition = 0;

			// Check to see if we reached EOF
			if (cacheFilled == 0)
				break;
		}
	}

	return (readBytes);
}



/******************************************************************************/
/**	Writes the specified amount of bytes into the file of this object from the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the bytes to write are
 *		stored.
 *	@param count number of bytes to write.
 *
 *	@return the actual number of bytes that was written to the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
int32 PCacheFile::Write(const void *buffer, int32 count)
{
	// Flush the cache
	cacheFilled   = 0;
	cachePosition = 0;
	cacheStart    = 0;

	return (PFile::Write(buffer, count));
}



/******************************************************************************/
/**	Seeks to a new position in the file of this object.
 *
 *	@param offset number of bytes to move in the seek.
 *	@param from is the starting position of the seek.
 *
 *	@return the new position after the seek.
 *
 *	@exception PFileException
 *
 *	@see #SeekToBegin, #SeekToEnd
 *//***************************************************************************/
int64 PCacheFile::Seek(int64 offset, PSeekFlags from)
{
	int64 newPos, length;

	// Get the file length
	length = GetLength();

	// Calculate the new real position
	switch (from)
	{
		case pSeekBegin:
		{
			newPos = offset;
			break;
		}

		case pSeekEnd:
		{
			newPos = length + offset;
			break;
		}

		case pSeekCurrent:
		{
			newPos = cacheStart + cachePosition + offset;
			break;
		}

		default:
		{
			// Unknown seek flag
			ASSERT(false);
			newPos = offset;
			break;
		}
	}

	// Check for out of range values
	if (newPos < 0)
		throw PFileException(P_FILE_ERR_SEEK, name);

	if (newPos > length)
		newPos = length;

	// Check to see if we need to flush the cache
	if ((newPos < cacheStart) || (newPos >= (cacheStart + cacheFilled)))
	{
		// Flush the cache
		cacheFilled   = 0;
		cachePosition = 0;
		cacheStart    = newPos;

		// Seek to the new position
		PFile::Seek(newPos, pSeekBegin);
	}
	else
	{
		// Calculate new cache position
		cachePosition = newPos - cacheStart;
	}

	// Clear eof flag
	eof = false;

	return (newPos);
}



/******************************************************************************/
/**	Returns the current position in the file of this object.
 *
 *	@return the current file position.
 *
 *	@exception PFileException
 *//***************************************************************************/
int64 PCacheFile::GetPosition(void) const
{
	ASSERT(fileOpened == true);

	if (cacheFilled == 0)
		return (PFile::GetPosition());

	return (cacheStart + cachePosition);
}



/******************************************************************************/
/**	Duplicates the file contained in this object by creating a new PCacheFile
 *	object containing the same file of this object. The file name and
 *	permissions of the file in the duplicated object will be the same as the
 *	original one.
 *
 *	@return a pointer to the new PCacheFile file object. Use the delete
 *		operator to delete/destroy it.
 *
 *	@exception PFileException
 *//***************************************************************************/
PFile *PCacheFile::DuplicateFile(void) const
{
	PCacheFile *newFile;

	// Create new file object
	newFile = new PCacheFile(cacheSize);
	if (newFile == NULL)
		throw PMemoryException();

	try
	{
		if (fileOpened)
		{
			// Open the file
			newFile->Open(name, prevOpenFlags);

			// Set the file position
			newFile->Seek(GetPosition(), pSeekBegin);

			// Set the eof flag
			newFile->eof = eof;
		}
	}
	catch(PException e)
	{
		delete newFile;
		throw;
	}

	return (newFile);
}





/******************************************************************************/
/* PMemFile class                                                             */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PMemFile object.
 *
 *	@param growSize the number of bytes to extend the internal memory buffer
 *		with, when more memory is needed.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PMemFile::PMemFile(uint32 growSize) : PFile()
{
	name       = "Memory File";
	memBuffer  = NULL;
	bufferSize = 0;
	fileSize   = 0;
	position   = 0;
	grow       = growSize;
	fileOpened = true;
}



/******************************************************************************/
/**	Standard constructor for creating a new PMemFile object which uses the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer with the data the file should
 *		contain.
 *	@param size the size of the memory buffer in bytes.
 *	@param growSize the number of bytes to extend the internal memory buffer
 *		with, when more memory is needed.
 *//***************************************************************************/
PMemFile::PMemFile(uint8 *buffer, uint32 size, uint32 growSize) : PFile()
{
	name      = "Memory File";
	memBuffer = NULL;

	Attach(buffer, size, growSize);
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PMemFile::~PMemFile(void)
{
	Close();
}



/******************************************************************************/
/**	Attaches a memory buffer to be used as the internal memory buffer for
 *	storing/reading the file of this object. When Attach() is used, the
 *	Detach() method can be used in order to detach the memory buffer from this
 *	object. If you don't call Detach(), the buffer will automatically be freed
 *	when you call Close() or destroy the object.
 *	The specified memory buffer must have been allocated with the new uint8[]
 *	operator and not with malloc() or similary functions.
 *
 *	@param buffer pointer to the memory buffer to attach.
 *	@param size the size of the memory buffer in bytes.
 *	@param growSize the number of bytes to extend the internal memory buffer
 *		with, when more memory is needed.
 *
 *	@see #Detach
 *//***************************************************************************/
void PMemFile::Attach(uint8 *buffer, uint32 size, uint32 growSize)
{
	ASSERT(memBuffer == NULL);

	memBuffer  = buffer;
	bufferSize = size;
	fileSize   = size;
	position   = 0;
	grow       = growSize;

	fileOpened = true;
	eof        = false;
}



/******************************************************************************/
/**	Detaches the memory buffer and returns the pointer to it. The returned
 *	pointer to the memory buffer must be freed by using the
 *	'delete[] operator'.
 *
 *	@return a pointer to the detached memory buffer.
 *//***************************************************************************/
uint8 *PMemFile::Detach(void)
{
	uint8 *buffer;

	ASSERT(memBuffer != NULL);

	// Remember to pointer
	buffer = memBuffer;

	// Detach the buffer from the class
	memBuffer  = NULL;
	bufferSize = 0;
	fileSize   = 0;
	position   = 0;
	grow       = 0;

	return (buffer);
}



/******************************************************************************/
/**	Closes the file for this object.
 *
 *	@see #Open
 *//***************************************************************************/
void PMemFile::Close(void)
{
	delete[] memBuffer;

	memBuffer  = NULL;
	bufferSize = 0;
	fileSize   = 0;
	position   = 0;
	grow       = 0;
}



/******************************************************************************/
/**	Reads the specified amount of bytes from the file of this object into the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the read bytes must be
 *		stored into.
 *	@param count number of bytes to read.
 *
 *	@return the actual number of bytes that was read from the file.
 *//***************************************************************************/
int32 PMemFile::Read(void *buffer, int32 count)
{
	int32 readBytes;

	ASSERT(fileOpened == true);

	// Calculate how many bytes to read
	readBytes = min(fileSize - position, count);
	if (readBytes <= 0)
	{
		eof = true;
		return (0);
	}

	// Copy the data into the buffer
	memcpy(buffer, memBuffer + position, readBytes);

	// Move the file pointer
	position += readBytes;
	eof = false;

	return (readBytes);
}



/******************************************************************************/
/**	Writes the specified amount of bytes into the file of this object from the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the bytes to write are
 *		stored.
 *	@param count number of bytes that was written to the file.
 *//***************************************************************************/
int32 PMemFile::Write(const void *buffer, int32 count)
{
	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + count) > bufferSize)
		GrowBuffer(position + count);

	if (count != 0)
	{
		// Write the buffer into the file
		memcpy(memBuffer + position, buffer, count);

		// Calculate new position and file size
		position += count;
		fileSize  = max(fileSize, position);
	}

	return (count);
}



/******************************************************************************/
/**	Reads a byte (8 bit integer) from the file of this object.
 *
 *	@return the read byte (uint8).
 *//***************************************************************************/
uint8 PMemFile::Read_UINT8(void)
{
	ASSERT(fileOpened == true);

	// Check for end of file is reached
	if (position >= fileSize)
	{
		eof = true;
		return (0);
	}

	eof = false;
	return (memBuffer[position++]);
}



/******************************************************************************/
/**	Reads a 16 bit integer in little endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 16 bit integer (uint16).
 *//***************************************************************************/
uint16 PMemFile::Read_L_UINT16(void)
{
	uint16 retVal;

	ASSERT(fileOpened == true);

	// Check for end of file is reached
	if ((position + 1) >= fileSize)
	{
		eof = true;
		return (0);
	}

	retVal    = P_LENDIAN_TO_HOST_INT16(*((uint16 *)(memBuffer + position)));
	position += 2;
	eof       = false;

	return (retVal);
}



/******************************************************************************/
/**	Reads a 32 bit integer in little endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 32 bit integer (uint32).
 *//***************************************************************************/
uint32 PMemFile::Read_L_UINT32(void)
{
	uint32 retVal;

	ASSERT(fileOpened == true);

	// Check for end of file is reached
	if ((position + 3) >= fileSize)
	{
		eof = true;
		return (0);
	}

	retVal    = P_LENDIAN_TO_HOST_INT32(*((uint32 *)(memBuffer + position)));
	position += 4;
	eof       = false;

	return (retVal);
}



/******************************************************************************/
/**	Reads an array of 16 bit integers in little endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 16 bit integers.
 *	@param count the number of 16 bit integers to read.
 *//***************************************************************************/
void PMemFile::ReadArray_L_UINT16s(uint16 *buffer, int32 count)
{
	uint16 *tempBuffer;

	ASSERT(fileOpened == true);

	// Initialize temp variables
	tempBuffer = (uint16 *)(memBuffer + position);

	for (; ((count > 0) && ((position + 1) < fileSize)); count--, position += 2)
		*buffer++ = P_LENDIAN_TO_HOST_INT16(*tempBuffer++);
}



/******************************************************************************/
/**	Reads an array of 32 bit integers in little endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 32 bit integers.
 *	@param count the number of 32 bit integers to read.
 *//***************************************************************************/
void PMemFile::ReadArray_L_UINT32s(uint32 *buffer, int32 count)
{
	uint32 *tempBuffer;

	ASSERT(fileOpened == true);

	// Initialize temp variables
	tempBuffer = (uint32 *)(memBuffer + position);

	for (; ((count > 0) && ((position + 3) < fileSize)); count--, position += 4)
		*buffer++ = P_LENDIAN_TO_HOST_INT32(*tempBuffer++);
}



/******************************************************************************/
/**	Reads a 16 bit integer in big endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 16 bit integer (uint16).
 *//***************************************************************************/
uint16 PMemFile::Read_B_UINT16(void)
{
	uint16 retVal;

	ASSERT(fileOpened == true);

	// Check for end of file is reached
	if ((position + 1) >= fileSize)
	{
		eof = true;
		return (0);
	}

	retVal    = P_BENDIAN_TO_HOST_INT16(*((uint16 *)(memBuffer + position)));
	position += 2;
	eof       = false;

	return (retVal);
}



/******************************************************************************/
/**	Reads a 32 bit integer in big endian format from the file of this object
 *	and return it in the native host format.
 *
 *	@return the read 32 bit integer (uint32).
 *//***************************************************************************/
uint32 PMemFile::Read_B_UINT32(void)
{
	uint32 retVal;

	ASSERT(fileOpened == true);

	// Check for end of file is reached
	if ((position + 3) >= fileSize)
	{
		eof = true;
		return (0);
	}

	retVal    = P_BENDIAN_TO_HOST_INT32(*((uint32 *)(memBuffer + position)));
	position += 4;
	eof       = false;

	return (retVal);
}



/******************************************************************************/
/**	Reads an array of 16 bit integers in big endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 16 bit integers.
 *	@param count the number of 16 bit integers to read.
 *//***************************************************************************/
void PMemFile::ReadArray_B_UINT16s(uint16 *buffer, int32 count)
{
	uint16 *tempBuffer;

	ASSERT(fileOpened == true);

	// Initialize temp variables
	tempBuffer = (uint16 *)(memBuffer + position);

	for (; ((count > 0) && ((position + 1) < fileSize)); count--, position += 2)
	{
		*buffer++ = P_BENDIAN_TO_HOST_INT16(*tempBuffer++);
		position += 2;
	}
}



/******************************************************************************/
/**	Reads an array of 32 bit integers in big endian format from the file of
 *	this object and convert the integers to the native host format.
 *
 *	@param buffer pointer to where to store the array of 32 bit integers.
 *	@param count the number of 32 bit integers to read.
 *//***************************************************************************/
void PMemFile::ReadArray_B_UINT32s(uint32 *buffer, int32 count)
{
	uint32 *tempBuffer;

	ASSERT(fileOpened == true);

	// Initialize temp variables
	tempBuffer = (uint32 *)(memBuffer + position);

	for (; ((count > 0) && ((position + 3) < fileSize)); count--, position += 4)
		*buffer++ = P_BENDIAN_TO_HOST_INT32(*tempBuffer++);
}



/******************************************************************************/
/**	Writes a byte (8 bit integer) to the file of this object.
 *
 *	@param value the byte (uint8) value to write.
 *//***************************************************************************/
void PMemFile::Write_UINT8(uint8 value)
{
	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + 1) > bufferSize)
		GrowBuffer(position + 1);

	memBuffer[position++] = value;
	fileSize = max(fileSize, position);
}



/******************************************************************************/
/**	Writes a 16 bit integer in little endian format to the file of this object.
 *
 *	@param value the 16 bit integer (uint16) value to write.
 *//***************************************************************************/
void PMemFile::Write_L_UINT16(uint16 value)
{
	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + 2) > bufferSize)
		GrowBuffer(position + 2);

	*((uint16 *)(memBuffer + position)) = P_HOST_TO_LENDIAN_INT16(value);
	position += 2;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes a 32 bit integer in little endian format to the file of this object.
 *
 *	@param value the 32 bit integer (uint32) value to write.
 *//***************************************************************************/
void PMemFile::Write_L_UINT32(uint32 value)
{
	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + 4) > bufferSize)
		GrowBuffer(position + 4);

	*((uint32 *)(memBuffer + position)) = P_HOST_TO_LENDIAN_INT32(value);
	position += 4;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes an array of 16 bit integers in little endian format to the file of
 *	this object.
 *
 *	@param buffer pointer to the array of 16 bit integers to write.
 *	@param count the number of 16 bit integers to write.
 *//***************************************************************************/
void PMemFile::WriteArray_L_UINT16s(const uint16 *buffer, int32 count)
{
	uint16 *tempBuf;

	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + count * 2) > bufferSize)
		GrowBuffer(position + count * 2);

	tempBuf = (uint16 *)(memBuffer + position);

	for (int32 i = 0; i < count; i++)
		*(tempBuf + i) = P_HOST_TO_LENDIAN_INT16(*buffer++);

	position += count * 2;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes an array of 32 bit integers in little endian format to the file of
 *	this object.
 *
 *	@param buffer pointer to the array of 32 bit integers to write.
 *	@param count the number of 32 bit integers to write.
 *//***************************************************************************/
void PMemFile::WriteArray_L_UINT32s(const uint32 *buffer, int32 count)
{
	uint32 *tempBuf;

	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + count * 4) > bufferSize)
		GrowBuffer(position + count * 4);

	tempBuf = (uint32 *)(memBuffer + position);

	for (int32 i = 0; i < count; i++)
		*(tempBuf + i) = P_HOST_TO_LENDIAN_INT32(*buffer++);

	position += count * 4;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes a 16 bit integer in big endian format to the file of this object.
 *
 *	@param value the 16 bit integer (uint16) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PMemFile::Write_B_UINT16(uint16 value)
{
	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + 2) > bufferSize)
		GrowBuffer(position + 2);

	*((uint16 *)(memBuffer + position)) = P_HOST_TO_BENDIAN_INT16(value);
	position += 2;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes a 32 bit integer in big endian format to the file of this object.
 *
 *	@param value the 32 bit integer (uint32) value to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PMemFile::Write_B_UINT32(uint32 value)
{
	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + 4) > bufferSize)
		GrowBuffer(position + 4);

	*((uint32 *)(memBuffer + position)) = P_HOST_TO_BENDIAN_INT32(value);
	position += 4;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes an array of 16 bit integers in big endian format to the file of this
 *	object.
 *
 *	@param buffer pointer to the array of 16 bit integers to write.
 *	@param count the number of 16 bit integers to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PMemFile::WriteArray_B_UINT16s(const uint16 *buffer, int32 count)
{
	uint16 *tempBuf;

	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + count * 2) > bufferSize)
		GrowBuffer(position + count * 2);

	tempBuf = (uint16 *)(memBuffer + position);

	for (int32 i = 0; i < count; i++)
		*(tempBuf + i) = P_HOST_TO_BENDIAN_INT16(*buffer++);

	position += count * 2;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Writes an array of 32 bit integers in big endian format to the file of this
 *	object.
 *
 *	@param buffer pointer to the array of 32 bit integers to write.
 *	@param count the number of 32 bit integers to write.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PMemFile::WriteArray_B_UINT32s(const uint32 *buffer, int32 count)
{
	uint32 *tempBuf;

	ASSERT(fileOpened == true);

	// Check to see if we need to grow the buffer
	if ((position + count * 4) > bufferSize)
		GrowBuffer(position + count * 4);

	tempBuf = (uint32 *)(memBuffer + position);

	for (int32 i = 0; i < count; i++)
		*(tempBuf + i) = P_HOST_TO_BENDIAN_INT32(*buffer++);

	position += count * 4;
	fileSize  = max(fileSize, position);
}



/******************************************************************************/
/**	Seeks to a new position in the file of this object.
 *
 *	@param offset number of bytes to move in the seek.
 *	@param from is the starting position of the seek.
 *
 *	@return the new position after the seek.
 *
 *	@exception PFileException
 *
 *	@see #SeekToBegin, #SeekToEnd
 *//***************************************************************************/
int64 PMemFile::Seek(int64 offset, PSeekFlags from)
{
	int64 newPos;

	ASSERT(fileOpened == true);

	// Find out where to seek
	switch (from)
	{
		case pSeekBegin:
		{
			newPos = offset;
			break;
		}

		case pSeekCurrent:
		{
			newPos = position + offset;
			break;
		}

		case pSeekEnd:
		{
			newPos = fileSize + offset;
			break;
		}

		default:
		{
			// Unknown seek flag
			ASSERT(false);
			newPos = offset;
			break;
		}
	}

	// Check for a negative position
	if (newPos < 0)
		throw PFileException(P_FILE_ERR_SEEK, name);

	if (newPos > fileSize)
		newPos = fileSize;

	// Use the new position
	position = newPos;

	// Clear eof flag
	eof = false;

	return (newPos);
}



/******************************************************************************/
/**	Returns the current position in the file of this object.
 *
 *	@return the current file position.
 *//***************************************************************************/
int64 PMemFile::GetPosition(void) const
{
	ASSERT(fileOpened == true);

	return (position);
}



/******************************************************************************/
/**	Returns the length of the file of this object. That is the number of bytes
 *	that the file contain.
 *
 *	@return the number of bytes the file contain.
 *//***************************************************************************/
int64 PMemFile::GetLength(void) const
{
	ASSERT(fileOpened == true);

	return (fileSize);
}



/******************************************************************************/
/**	Sets a new length of the file of this object. This is the new number of
 *	bytes that the file of this object will contain. If the new length is
 *	lesser than the current length, the file will be truncated.
 *
 *	@param newLength the number of bytes the file must contain.
 *//***************************************************************************/
void PMemFile::SetLength(int64 newLength)
{
	ASSERT(fileOpened == true);

	if (newLength > bufferSize)
		GrowBuffer(newLength);

	fileSize = newLength;
}



/******************************************************************************/
/**	Returns the file name of the file of this object without the file path.
 *
 *	@return the file name of the file without the file path.
 *
 *	@see #GetFilePath, #GetFullPath
 *//***************************************************************************/
PString PMemFile::GetFileName(void) const
{
	ASSERT(fileOpened == true);

	return (name);
}



/******************************************************************************/
/**	Returns the expanded file path of the file of this object without the file
 *	name.
 *
 *	@return the expanded file path of the file without the file name.
 *
 *	@see #GetFileName, #GetFullPath
 *//***************************************************************************/
PString PMemFile::GetFilePath(void) const
{
	ASSERT(fileOpened == true);

	return (name);
}



/******************************************************************************/
/**	Return the full path of the file of this object containing both the file
 *	path and file name. The returned path is the full expanded path of the
 *	file, regardless if the path of the file is a relative path.
 *
 *	@return the full expanded path of the file.
 *
 *	@see #GetFileName, #GetFilePath
 *//***************************************************************************/
PString PMemFile::GetFullPath(void) const
{
	ASSERT(fileOpened == true);

	return (name);
}



/******************************************************************************/
/**	Returns the MIME type string for the file of this object.
 *
 *	@return a string specifying the MIME type of the file.
 *//***************************************************************************/
PString PMemFile::GetFileType(void) const
{
	return (fileType);
}



/******************************************************************************/
/**	Sets the file path/name of the file of this object. It does not open the
 *	file, it just associates the object with the name.
 *
 *	@param newName the new file path/name for the file.
 *//***************************************************************************/
void PMemFile::SetFilePath(PString newName)
{
	// Well, memory files don't use the name to anything,
	// but you can set it if you call functions that
	// doesn't differ between PFile and PMemFile objects and
	// need to get the filename
	name = newName;
}



/******************************************************************************/
/**	Sets the MIME type of the file of this object.
 *
 *	@param newType the string specifying the new MIME type for the file.
 *//***************************************************************************/
void PMemFile::SetFileType(PString newType)
{
	fileType = newType;
}



/******************************************************************************/
/**	Duplicates the file contained in this object by creating a new PMemFile
 *	object containing the same file of this object. The file name and
 *	permissions of the file in the duplicated object will be the same as the
 *	original one.
 *
 *	@return a pointer to the new PMemFile file object. Use the delete operator
 *		to delete/destroy it.
 *
 *	@exception PFileException
 *//***************************************************************************/
PFile *PMemFile::DuplicateFile(void) const
{
	PMemFile *newFile;

	// Create new file object
	newFile = new PMemFile(grow);
	if (newFile == NULL)
		throw PMemoryException();

	try
	{
		if (bufferSize != 0)
		{
			// Allocate a new buffer
			newFile->GrowBuffer(bufferSize);

			// Copy the contents
			memcpy(newFile->memBuffer, memBuffer, bufferSize);

			// Set the file position
			newFile->Seek(GetPosition(), pSeekBegin);

			// Copy other variables
			newFile->eof      = eof;
			newFile->name     = name;
			newFile->fileType = fileType;
		}
	}
	catch(PException e)
	{
		delete newFile;
		throw;
	}

	return (newFile);
}



/******************************************************************************/
/**	Extends the internal memory buffer to contain at least the specified number
 *	of bytes.
 *
 *	@param endPoint the minimum amount of bytes that the memory buffer must be
 *		extended to.
 *
 *	@exception PFileException
 *//***************************************************************************/
void PMemFile::GrowBuffer(int64 endPoint)
{
	uint8 *newBuffer;
	int64 newSize;

	// Check to see if we can grow the buffer
	if (grow == 0)
		throw PFileException(P_FILE_ERR_DISK_FULL, name);

	// Calculate the new size of the buffer
	newSize = bufferSize + grow;
	while (newSize < endPoint)
		newSize += grow;

	// Allocate new buffer
	newBuffer = new uint8[newSize];

	if (fileSize != 0)
	{
		// Copy the old buffer into the new one
		memcpy(newBuffer, memBuffer, fileSize);
	}

	// Free the old buffer and use the new one
	delete[] memBuffer;

	memBuffer  = newBuffer;
	bufferSize = newSize;
}





/******************************************************************************/
/* PSocketFile class                                                          */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PSocketFile object.
 *
 *	@param socket pointer to the socket to use for this object.
 *	@param maxBufferSize the maximum number of kilobytes for the back-data
 *		buffer.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PSocketFile::PSocketFile(PLineSocket *socket, int32 maxBufferSize) : PFile()
{
	// Remember the arguments
	dataSock    = socket;
	maxElements = maxBufferSize;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PSocketFile::~PSocketFile(void)
{
}



/******************************************************************************/
/**	Reads the specified amount of bytes from the file of this object into the
 *	specified memory buffer.
 *
 *	@param buffer pointer to the memory buffer where the read bytes must be
 *		stored into.
 *	@param count number of bytes to read.
 *
 *	@return the actual number of bytes that was read from the file.
 *
 *	@exception PFileException
 *//***************************************************************************/
int32 PSocketFile::Read(void *buffer, int32 count)
{
	uint8 *buf = NULL;
	int32 bufNum;
	int32 startIndex, toCopy;
	uint8 *destBuf = (uint8 *)buffer;
	int32 readBytes = 0;

	ASSERT(fileOpened == true);

	// Did we reach EOF?
	if (eof)
		return (0);

	while (count > 0)
	{
		// Time to read another block of data
		if (currentPos == endPos)
		{
			uint32 sockRead;
			uint32 toDo;
			uint8 *tempBuf;

			try
			{
				// Yap do it, but first allocate a new buffer
				buf = new uint8[1024];
				if (buf == NULL)
					throw PMemoryException();

				tempBuf = buf;
				toDo    = 1024;
				lastLen = 0;

				do
				{
					// Read the data
					sockRead = toDo;
					dataSock->ReceiveData(tempBuf, &sockRead);

					toDo    -= sockRead;
					tempBuf += sockRead;
					lastLen += sockRead;
				}
				while (toDo > 0);
			}
			catch(PNetworkException e)
			{
				// Some error occurred, probably socket closed
				;
			}

			// Did we read anything at all?
			if (lastLen == 0)
			{
				delete[] buf;
				eof = true;
				break;
			}

			// Add the buffer to the list
			bufferList.AddTail(buf);
			endPos += 1024;
		}

		// Did we reach the maximum number of elements in the list?
		if (bufferList.CountItems() > maxElements)
		{
			// Yip, remove the first one
			buf = bufferList.GetAndRemoveItem(0);
			delete[] buf;
			startPos += 1024;
		}

		// Find the buffer to read from
		bufNum = (currentPos - startPos) / 1024;
		buf    = bufferList.GetItem(bufNum);

		// Find the start index and number of bytes to copy
		startIndex = (currentPos - startPos) % 1024;
		toCopy     = min(1024 - startIndex, 1024);
		toCopy     = min(toCopy, count);

		// If we read from the last buffer, adjust
		// the number of bytes to what has been
		// filled in the last buffer
		if (bufNum == (bufferList.CountItems() - 1))
		{
			// First check to see if we have read all the bytes
			if ((currentPos - startPos) == (bufNum * 1024 + lastLen))
			{
				eof = true;
				break;
			}

			toCopy = min(toCopy, lastLen - startIndex);
		}

		// Copy the bytes
		memcpy(destBuf, &buf[startIndex], toCopy);

		// Adjust the counters
		destBuf    += toCopy;
		currentPos += toCopy;
		count      -= toCopy;
		readBytes  += toCopy;
	}

	// Adjust the file size if needed
	if (currentPos > fileSize)
		fileSize = currentPos;

	return (readBytes);
}



/******************************************************************************/
/**	Seeks to a new position in the file of this object.
 *
 *	@param offset number of bytes to move in the seek.
 *	@param from is the starting position of the seek.
 *
 *	@return the new position after the seek.
 *
 *	@exception PFileException
 *
 *	@see #SeekToBegin, #SeekToEnd
 *//***************************************************************************/
int64 PSocketFile::Seek(int64 offset, PSeekFlags from)
{
	int64 newPos, length;

	// Get the file length
	length = GetLength();

	// Calculate the new real position
	switch (from)
	{
		case pSeekBegin:
		{
			newPos = offset;
			break;
		}

		case pSeekEnd:
		{
			newPos = length + offset;
			break;
		}

		case pSeekCurrent:
		{
			newPos = currentPos + offset;
			break;
		}

		default:
		{
			// Unknown seek flag
			ASSERT(false);
			newPos = offset;
			break;
		}
	}

	// Check for out of range
	if (newPos < 0)
		throw PFileException(P_FILE_ERR_SEEK, name);

	// Do we need to re-open the file?
	if (newPos < startPos)
	{
		// Yes, do it
		Close();
		Open(curOpenFlags);

		// Set the length again in case it couldn't be
		// determined in the open
		fileSize = length;
	}

	// Should we set the position in the current buffer
	if ((newPos >= startPos) && (newPos < endPos))
		currentPos = newPos;
	else
	{
		uint8 *buf;
		int32 toRead;

		// Find out how many bytes to read
		toRead = newPos - currentPos;

		// Allocate temporary buffer
		buf = new uint8[toRead];
		if (buf == NULL)
			throw PMemoryException();

		try
		{
			// Read the bytes to skip
			Read(buf, toRead);

			// Delete the buffer
			delete[] buf;
		}
		catch(...)
		{
			delete[] buf;
			throw;
		}
	}

	// Clear eof flag
	eof = false;

	return (currentPos);
}



/******************************************************************************/
/**	Returns the current position in the file of this object.
 *
 *	@return the current file position.
 *//***************************************************************************/
int64 PSocketFile::GetPosition(void) const
{
	ASSERT(fileOpened == true);

	return (currentPos);
}



/******************************************************************************/
/**	Returns the length of the file of this object. That is the number of bytes
 *	that the file contain.
 *
 *	@return the number of bytes the file contain or 0 if it couldn't be
 *		determined.
 *//***************************************************************************/
int64 PSocketFile::GetLength(void) const
{
	ASSERT(fileOpened == true);

	return (fileSize);
}



/******************************************************************************/
/**	Cleans up the connection on the socket of this object.
 *//***************************************************************************/
void PSocketFile::Cleanup(void)
{
	int32 i, count;

	// Free the buffers
	count = bufferList.CountItems();
	for (i = 0; i < count; i++)
		delete[] bufferList.GetItem(i);

	bufferList.MakeEmpty();

	// Close the socket
	if (dataSock != NULL)
		dataSock->CloseConnection();

	fileOpened = false;
}





/******************************************************************************/
/* PHTTPFile class                                                            */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PHTTPFile object.
 *
 *	@param socket pointer to the socket to use for this object.
 *	@param maxBufferSize the maximum number of kilobytes for the back-data
 *		buffer.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PHTTPFile::PHTTPFile(PLineSocket *socket, int32 maxBufferSize) : PSocketFile(socket, maxBufferSize)
{
	// Initialize member variables
	agentName = "PolyKit";
}



/******************************************************************************/
/**	Standard constructor for creating a new PHTTPFile object and open the file
 *	for this object.
 *
 *	@param url the URL of the file to open.
 *	@param openFlags flags that specify how the file must be opened.
 *	@param socket pointer to the socket to use for this object.
 *	@param maxBufferSize the maximum number of kilobytes for the back-data
 *		buffer.
 *
 *	@exception PFileException, PHTTPException
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PHTTPFile::PHTTPFile(PString url, uint16 openFlags, PLineSocket *socket, int32 maxBufferSize) : PSocketFile(socket, maxBufferSize)
{
	// Initialize member variables
	agentName = "PolyKit";

	// Open the file
	Open(url, openFlags);
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PHTTPFile::~PHTTPFile(void)
{
}



/******************************************************************************/
/**	Opens the file of this object in the open mode specified by the open flags.
 *
 *	@param openFlags flags used for specifying the open mode to use when
 *		opening the file.
 *
 *	@exception PFileException, PHTTPException
 *
 *	@see #Close
 *//***************************************************************************/
void PHTTPFile::Open(uint16 openFlags)
{
	Open(name, openFlags);
}



/******************************************************************************/
/**	Opens the file of this object in the open mode specified by the open flags
 *	and from the specified URL.
 *
 *	@param url the URL of the file to open.
 *	@param openFlags flags used for specifying the open mode to use when
 *		opening the file.
 *
 *	@exception PFileException, PHTTPException
 *
 *	@see #Close
 *//***************************************************************************/
void PHTTPFile::Open(PString url, uint16 openFlags)
{
	PString serverAdr, urlName;
	int32 index;

	ASSERT(fileOpened == false);
	ASSERT((openFlags & pModeOpenMask) != pModeWrite);

	// Remember the open flags
	curOpenFlags = openFlags;

	// Check and remember the URL
	if (url.Left(7).CompareNoCase("HTTP://") != 0)
		throw PFileException(P_GEN_ERR_BAD_ARGUMENT, url);

	name = url;

	// Extract the server name and add a port if missing
	index = url.Find('/', 7);
	if (index == -1)
		throw PFileException(P_GEN_ERR_BAD_ARGUMENT, url);

	serverAdr = url.Mid(7, index - 7);
	urlName   = url.Mid(index);

	if (serverAdr.Find(':') == -1)
		serverAdr += ":80";

	try
	{
		PString line;
		PCharSet_UTF8 charSet;

		// Initialize the socket
		dataSock->UseProtocol(PSocket::pTCP_IP, serverAdr);

		// Try to connect to the server
		dataSock->CreateConnection();

		// Send the HTTP commands to the server so we
		// can retrieve the file
		dataSock->SendLine("GET " + urlName + " HTTP/1.0", &charSet);
		dataSock->SendLine("Host: " + serverAdr, &charSet);
		dataSock->SendLine("User-Agent: " + agentName, &charSet);
		dataSock->SendLine("Accept: */*", &charSet);
		dataSock->SendLine("Connection: close", &charSet);
		dataSock->SendLine("", &charSet);

		// Check the first respond header
		line = dataSock->ReceiveLine(&charSet);
		index = line.Find(' ');
		if (line.GetUNumber(index + 1) != 200)
		{
			// We didn't get an OK from the server, so fail
			throw PHTTPException(line);
		}

		// Initialize member variables so they are ready
		// to receive the file
		fileOpened = true;
		fileSize   = 0;
		fileType   = "application/octet-stream";

		startPos   = 0;
		endPos     = 0;
		currentPos = 0;
		lastLen    = 0;

		// Now read all the respond headers and parse them
		for (;;)
		{
			line = dataSock->ReceiveLine(&charSet);
			if (line.IsEmpty())
				break;

			// Got the file size?
			if (line.Left(16).CompareNoCase("Content-Length: ") == 0)
				fileSize = line.Mid(16).GetUNumber();

			// Got the file type?
			if (line.Left(14).CompareNoCase("Content-Type: ") == 0)
			{
				index = line.Find(';', 14);
				if (index != -1)
					fileType = line.Mid(14, index - 14);
				else
					fileType = line.Mid(14);
			}
		}
	}
	catch(PNetworkException e)
	{
		// Convert the exception
		throw PFileException(e.errorNum, name);
	}
}



/******************************************************************************/
/**	Closes the file for this object.
 *
 *	@see #Open
 *//***************************************************************************/
void PHTTPFile::Close(void)
{
	// Clean up the object
	Cleanup();
}



/******************************************************************************/
/**	Returns the file name of the file of this object without the file path.
 *
 *	@return the file name of the file without the file path.
 *
 *	@see #GetFilePath, #GetFullPath
 *//***************************************************************************/
PString PHTTPFile::GetFileName(void) const
{
	int32 index;

	ASSERT(fileOpened == true);

	index = name.ReverseFind('/');
	if (index == -1)
		return ("");

	return (name.Mid(index + 1));
}



/******************************************************************************/
/**	Returns the file path of the file of this object without the file name.
 *
 *	@return the file path of the file without the file name.
 *
 *	@see #GetFileName, #GetFullPath
 *//***************************************************************************/
PString PHTTPFile::GetFilePath(void) const
{
	int32 index;

	ASSERT(fileOpened == true);

	index = name.ReverseFind('/');
	if (index == -1)
		return ("");

	return (name.Left(index + 1));
}



/******************************************************************************/
/**	Returns the full path of the file of this object containing both the file
 *	path and file name. The returned path is the full expanded path of the
 *	file, regardless if the path of the file is a relative path.
 *
 *	@return the full expanded path of the file.
 *
 *	@see #GetFileName, #GetFilePath
 *//***************************************************************************/
PString PHTTPFile::GetFullPath(void) const
{
	ASSERT(fileOpened == true);

	return (name);
}



/******************************************************************************/
/**	Returns the MIME type string for the file of this object.
 *
 *	@return a string specifying the MIME type of the file.
 *//***************************************************************************/
PString PHTTPFile::GetFileType(void) const
{
	ASSERT(fileOpened == true);

	return (fileType);
}



/******************************************************************************/
/**	Sets the agent name which is sent to the web server as id (browser name).
 *
 *	@param agent the name if the agent to use as id for at web server.
 *//***************************************************************************/
void PHTTPFile::SetAgentName(PString agent)
{
	agentName = agent;
}



/******************************************************************************/
/**	Duplicates the file contained in this object by creating a new PHTTPFile
 *	object containing the same file of this object. The file name and
 *	permissions of the file in the duplicated object will be the same as the
 *	original one.
 *
 *	@return a pointer to the new PHTTPFile file object. Use the delete operator
 *		to delete/destroy it.
 *
 *	@exception PFileException
 *//***************************************************************************/
PFile *PHTTPFile::DuplicateFile(void) const
{
	PHTTPFile *newFile;

	// Create new file object
	newFile = new PHTTPFile(dataSock, maxElements);
	if (newFile == NULL)
		throw PMemoryException();

	try
	{
		if (fileOpened)
		{
			// Open the file
			newFile->Open(name, prevOpenFlags);

			// Set the file position
			newFile->Seek(GetPosition(), pSeekBegin);

			// Set the eof flag
			newFile->eof = eof;
		}
	}
	catch(PException e)
	{
		delete newFile;
		throw;
	}

	return (newFile);
}





/******************************************************************************/
/* PFTPFile class                                                             */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PFTPFile object.
 *
 *	@param socket pointer to the socket to use for this object.
 *	@param maxBufferSize the maximum number of kilobytes for the back-data
 *		buffer.
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PFTPFile::PFTPFile(PLineSocket *socket, int32 maxBufferSize) : PSocketFile(NULL, maxBufferSize)
{
	// Remember the arguments
	cmdSock       = socket;

	// Initialize member variables
	logonName     = "anonymous";
	logonPassword = "polykit@polycode.dk";
}



/******************************************************************************/
/**	Standard constructor for creating a new PFTPFile object and open the file
 *	for this object.
 *
 *	@param url the URL of the file to open.
 *	@param openFlags flags that specify how the file must be opened.
 *	@param socket pointer to the socket to use for this object.
 *	@param maxBufferSize the maximum number of kilobytes for the back-data
 *		buffer.
 *
 *	@exception PFileException, PFTPException
 *
 *	@see #Open, #Close
 *//***************************************************************************/
PFTPFile::PFTPFile(PString url, uint16 openFlags, PLineSocket *socket, int32 maxBufferSize) : PSocketFile(NULL, maxBufferSize)
{
	// Remember the arguments
	cmdSock       = socket;

	// Initialize member variables
	logonName     = "anonymous";
	logonPassword = "polykit@polycode.dk";

	// Open the file
	Open(url, openFlags);
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PFTPFile::~PFTPFile(void)
{
}



/******************************************************************************/
/**	Opens the file of this object in the open mode specified by the open flags.
 *
 *	@param openFlags flags used for specifying the open mode to use when
 *		opening the file.
 *
 *	@exception PFileException, PFTPException
 *
 *	@see #Close
 *//***************************************************************************/
void PFTPFile::Open(uint16 openFlags)
{
	Open(name, openFlags);
}



/******************************************************************************/
/**	Opens the file of this object in the open mode specified by the open flags
 *	and from the specified URL.
 *
 *	@param url the URL of the file to open.
 *	@param openFlags flags used for specifying the open mode to use when
 *		opening the file.
 *
 *	@exception PFileException, PFTPException
 *
 *	@see #Close
 *//***************************************************************************/
void PFTPFile::Open(PString url, uint16 openFlags)
{
	PString serverAdr, urlName;
	PString pasvAdr;
	int32 index, index1, index2;
	uint32 port;

	ASSERT(fileOpened == false);
	ASSERT((openFlags & pModeOpenMask) != pModeWrite);

	// Remember the open flags
	curOpenFlags = openFlags;

	// Check and remember the URL
	if (url.Left(6).CompareNoCase("FTP://") != 0)
		throw PFileException(P_GEN_ERR_BAD_ARGUMENT, url);

	// Extract any user name and password from the url
	index = url.Find('@');
	if (index != -1)
		url.Delete(6, index - 6 + 1);

	name = url;

	// Extract the server name and add a port if missing
	index = url.Find('/', 6);
	if (index == -1)
		throw PFileException(P_GEN_ERR_BAD_ARGUMENT, url);

	serverAdr = url.Mid(6, index - 6);
	urlName   = url.Mid(index);

	if (serverAdr.Find(':') == -1)
		serverAdr += ":21";

	try
	{
		PString line;
		PCharSet_UTF8 charSet;

		// Initialize the socket
		cmdSock->UseProtocol(PSocket::pTCP_IP, serverAdr);

		// Try to connect to the server
		cmdSock->CreateConnection();

		// Read the respond header
		line = cmdSock->ReceiveLine(&charSet);
		if (line.Left(3) != "220")
			throw PFTPException(line);

		// Now logon to the server
		cmdSock->SendLine("USER " + logonName, &charSet);
		line = cmdSock->ReceiveLine(&charSet);
		if (line.Left(3) != "331")
			throw PFTPException(line);

		cmdSock->SendLine("PASS " + logonPassword, &charSet);
		line = cmdSock->ReceiveLine(&charSet);
		if (line.Left(3) != "230")
			throw PFTPException(line);

		// Read welcome message if any
		while (line.Left(4) != "230 ")
			line = cmdSock->ReceiveLine(&charSet);

		// Initialize the server to passive connection
		cmdSock->SendLine("TYPE I", &charSet);
		line = cmdSock->ReceiveLine(&charSet);
		if (line.Left(3) != "200")
			throw PFTPException(line);

		cmdSock->SendLine("PASV", &charSet);
		line = cmdSock->ReceiveLine(&charSet);
		if (line.Left(3) != "227")
			throw PFTPException(line);

		// Parse the address and port to connect to
		index = line.ReverseFind('(');
		if (index == -1)
			throw PFileException(P_GEN_ERR_CANT_PERFORM, url);

		// Take each number and build the address string
		index1 = line.ReverseFind(',');
		if (index1 == -1)
			throw PFileException(P_GEN_ERR_CANT_PERFORM, url);

		index2 = line.ReverseFind(',', index1 - 1);
		if (index2 == -1)
			throw PFileException(P_GEN_ERR_CANT_PERFORM, url);

		pasvAdr = line.Mid(index + 1, index2 - index - 1);
		pasvAdr.Replace(',', '.');

		port = (line.GetUNumber(index2 + 1) * 256) + line.GetUNumber(index1 + 1);
		pasvAdr += ":" + PString::CreateUNumber(port);

		// Create the data socket
		if (is_instance_of(cmdSock, PProxySocket))
		{
			dataSock = new PProxySocket();
			if (dataSock == NULL)
				throw PMemoryException();

			((PProxySocket *)dataSock)->EnableProxy(((PProxySocket *)cmdSock)->IsEnabled());
			((PProxySocket *)dataSock)->SetProxyServer(((PProxySocket *)cmdSock)->GetProxyType(), ((PProxySocket *)cmdSock)->GetProxyAddress());
		}
		else
		{
			dataSock = new PLineSocket();
			if (dataSock == NULL)
				throw PMemoryException();
		}

		dataSock->SetEOLCode(cmdSock->GetEOLCode());
		dataSock->SetMaxLineLength(cmdSock->GetMaxLineLength());
		dataSock->SetWaitObjects(cmdSock);

		dataSock->UseProtocol(PSocket::pTCP_IP, pasvAdr);
		dataSock->CreateConnection();

		// Now tell the server to send the file
		cmdSock->SendLine("RETR " + urlName, &charSet);
		line = cmdSock->ReceiveLine(&charSet);
		if (line.Left(3) != "150")
			throw PFTPException(line);

		// Initialize member variables so they are ready
		// to receive the file
		fileOpened = true;
		fileSize   = 0;
		fileType   = "application/octet-stream";

		startPos   = 0;
		endPos     = 0;
		currentPos = 0;
		lastLen    = 0;
	}
	catch(PNetworkException e)
	{
		// Convert the exception
		throw PFileException(e.errorNum, name);
	}
}



/******************************************************************************/
/**	Closes the file for this object.
 *
 *	@see #Open
 *//***************************************************************************/
void PFTPFile::Close(void)
{
	// Clean up the object
	Cleanup();

	// Close the command socket
	cmdSock->CloseConnection();

	// Delete the data socket
	delete dataSock;
	dataSock = NULL;
}



/******************************************************************************/
/**	Returns the file name of the file of this object without the file path.
 *
 *	@return the file name of the file without the file path.
 *
 *	@see #GetFilePath, #GetFullPath
 *//***************************************************************************/
PString PFTPFile::GetFileName(void) const
{
	int32 index;

	ASSERT(fileOpened == true);

	index = name.ReverseFind('/');
	if (index == -1)
		return ("");

	return (name.Mid(index + 1));
}



/******************************************************************************/
/**	Returns the file path of the file of this object without the file name.
 *
 *	@return the file path of the file without the file name.
 *
 *	@see #GetFileName, #GetFullPath
 *//***************************************************************************/
PString PFTPFile::GetFilePath(void) const
{
	int32 index;

	ASSERT(fileOpened == true);

	index = name.ReverseFind('/');
	if (index == -1)
		return ("");

	return (name.Left(index + 1));
}



/******************************************************************************/
/**	Returns the full path of the file of this object containing both the file
 *	path and file name. The returned path is the full expanded path of the
 *	file, regardless if the path of the file is a relative path.
 *
 *	@return the full expanded path of the file.
 *
 *	@see #GetFileName, #GetFilePath
 *//***************************************************************************/
PString PFTPFile::GetFullPath(void) const
{
	ASSERT(fileOpened == true);

	return (name);
}



/******************************************************************************/
/**	Returns the MIME type string for the file of this object.
 *
 *	@return a string specifying the MIME type of the file.
 *//***************************************************************************/
PString PFTPFile::GetFileType(void) const
{
	ASSERT(fileOpened == true);

	return (fileType);
}



/******************************************************************************/
/**	Sets the user name and password needed in order to logon.
 *
 *	@param user the user name used for login.
 *	@param password the password used for login.
 *//***************************************************************************/
void PFTPFile::SetLogonName(PString user, PString password)
{
	logonName     = user;
	logonPassword = password;
}



/******************************************************************************/
/* SetLogonName() will set the logon name and password extracted from a URL.  */
/*      If no user are stored in the URL, it will set the logon name to       */
/*      anonymous.                                                            */
/*                                                                            */
/* Input:  "url" is the URL to parse in this syntax:                          */
/*         ftp://user:password@ftp.polycode.dk.                               */
/******************************************************************************/
void PFTPFile::SetLogonName(PString url)
{
	int32 index, index1;
	bool anonymous = true;

	// Is the URL valid
	if (url.Left(6).CompareNoCase("FTP://") == 0)
	{
		// Does the URL have a user?
		index = url.Find(':', 6);
		if (index != -1)
		{
			logonName = url.Mid(6, index - 6);

			// Now find the password
			index1 = url.Find('@', index);
			if (index1 != -1)
			{
				logonPassword = url.Mid(index + 1, index1 - index - 1);
				anonymous     = false;
			}
		}
	}

	// Set the logon name to anonymous?
	if (anonymous)
	{
		logonName     = "anonymous";
		logonPassword = "polykit@polycode.dk";
	}
}



/******************************************************************************/
/**	Duplicates the file contained in this object by creating a new PFTPFile
 *	object containing the same file of this object. The file name and
 *	permissions of the file in the duplicated object will be the same as the
 *	original one.
 *
 *	@return a pointer to the new PFTPFile file object. Use the delete operator
 *		to delete/destroy it.
 *
 *	@exception PFileException
 *//***************************************************************************/
PFile *PFTPFile::DuplicateFile(void) const
{
	PFTPFile *newFile;

	// Create new file object
	newFile = new PFTPFile(dataSock, maxElements);
	if (newFile == NULL)
		throw PMemoryException();

	try
	{
		if (fileOpened)
		{
			// Open the file
			newFile->Open(name, prevOpenFlags);

			// Set the file position
			newFile->Seek(GetPosition(), pSeekBegin);

			// Set the eof flag
			newFile->eof = eof;
		}
	}
	catch(PException e)
	{
		delete newFile;
		throw;
	}

	return (newFile);
}
