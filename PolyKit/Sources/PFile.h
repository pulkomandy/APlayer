/******************************************************************************/
/* PFile header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PFile_h
#define __PFile_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PSocket.h"
#include "PList.h"
#include "ImportExport.h"


/******************************************************************************/
/* PFile class                                                                */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

/**
 *	This is the super class used for reading and writing files.
 */
class _IMPEXP_PKLIB PFile
{
public:
	enum POpenFlags
	{
		pModeRead =			0x0000,
		pModeWrite =		0x0001,
		pModeReadWrite =	0x0002,

		pModeShareRead =	0x0010,
		pModeShareWrite =	0x0020,

		pModeCreate =		0x0100,
		pModeNoTruncate =	0x0200,
		pModeFailIfExists =	0x0400,

		pModeOpenMask =     0x000f,
		pModeShareMask =	0x00f0,
		pModeActionMask =   0x0f00
	};

	enum PSeekFlags { pSeekBegin = 0, pSeekCurrent, pSeekEnd };

	PFile(void);
	PFile(PString fileName, uint16 openFlags);
	virtual ~PFile(void);

	virtual void Open(uint16 openFlags);
	virtual void Open(PString fileName, uint16 openFlags);
	virtual void Close(void);

	virtual bool IsOpen(void) const;

	virtual int32 Read(void *buffer, int32 count);
	virtual int32 Write(const void *buffer, int32 count);

	virtual void ReadString(char *buffer, int32 maxLen);
	virtual PString ReadString(PCharacterSet *characterSet = NULL);
	virtual PString ReadLine(PCharacterSet *characterSet = NULL);

	virtual uint8 Read_UINT8(void);

	virtual uint16 Read_L_UINT16(void);
	virtual uint32 Read_L_UINT32(void);
	virtual void ReadArray_L_UINT16s(uint16 *buffer, int32 count);
	virtual void ReadArray_L_UINT32s(uint32 *buffer, int32 count);

	virtual uint16 Read_B_UINT16(void);
	virtual uint32 Read_B_UINT32(void);
	virtual void ReadArray_B_UINT16s(uint16 *buffer, int32 count);
	virtual void ReadArray_B_UINT32s(uint32 *buffer, int32 count);

	virtual void WriteString(PString string, PCharacterSet *characterSet = NULL);
	virtual void WriteLine(PString line, PCharacterSet *characterSet = NULL);

	virtual void Write_UINT8(uint8 value);

	virtual void Write_L_UINT16(uint16 value);
	virtual void Write_L_UINT32(uint32 value);
	virtual void WriteArray_L_UINT16s(const uint16 *buffer, int32 count);
	virtual void WriteArray_L_UINT32s(const uint32 *buffer, int32 count);

	virtual void Write_B_UINT16(uint16 value);
	virtual void Write_B_UINT32(uint32 value);
	virtual void WriteArray_B_UINT16s(const uint16 *buffer, int32 count);
	virtual void WriteArray_B_UINT32s(const uint32 *buffer, int32 count);

	virtual int64 Seek(int64 offset, PSeekFlags from);
	void SeekToBegin(void);
	void SeekToEnd(void);
	virtual int64 GetPosition(void) const;
	virtual bool IsEOF(void) const;

	virtual int64 GetLength(void) const;
	virtual void SetLength(int64 newLength);

	virtual PString GetFileName(void) const;
	virtual PString GetFilePath(void) const;
	virtual PString GetFullPath(void) const;
	virtual PString GetFileType(void) const;

	virtual void SetFilePath(PString newName);
	virtual void SetFileType(PString newType);

	static void Rename(PString oldName, PString newName);
	static void Remove(PString fileName);
	static bool FileExists(PString fileName);
	static PString GetFileExtension(PString fileName);
	static PString RequestFileExtension(PString fileName, PString fileExt);

	virtual PFile *DuplicateFile(void) const;

protected:
	bool fileOpened;
	bool eof;
	uint16 prevOpenFlags;
	PString name;
	BFile *file;
};



/******************************************************************************/
/* PCacheFile class                                                           */
/******************************************************************************/
/**
 *	This is a specialized PFile class used for cached reading and writing to
 *	files. This class is recommended for reading small pieces at a time from a
 *	file. A PCacheFile object reads a big chunk from the file at once and then
 *	memory copy from this chunk into a supplied buffer where the file must be
 *	read into.
 */
class _IMPEXP_PKLIB PCacheFile : public PFile
{
public:
	PCacheFile(uint32 size = 32768);
	PCacheFile(PString fileName, uint16 openFlags, uint32 size = 32768);
	virtual ~PCacheFile(void);

	virtual void Close(void);

	virtual int32 Read(void *buffer, int32 count);
	virtual int32 Write(const void *buffer, int32 count);

	virtual int64 Seek(int64 offset, PSeekFlags from);
	virtual int64 GetPosition(void) const;

	virtual PFile *DuplicateFile(void) const;

protected:
	uint8 *cache;
	int32 cacheSize;
	int32 cacheFilled;
	int32 cachePosition;
	int64 cacheStart;
};



/******************************************************************************/
/* PMemFile class                                                             */
/******************************************************************************/
/**
 *	This is a specialized PFile class used for creating a virtual file in
 *	memory that can be used for reading and writing similar to an ordinary
 *	file.
 */
class _IMPEXP_PKLIB PMemFile : public PFile
{
public:
	PMemFile(uint32 growSize = 1024);
	PMemFile(uint8 *buffer, uint32 size, uint32 growSize = 0);
	virtual ~PMemFile(void);

	void Attach(uint8 *buffer, uint32 size, uint32 growSize = 0);
	uint8 *Detach(void);

	virtual void Close(void);

	virtual int32 Read(void *buffer, int32 count);
	virtual int32 Write(const void *buffer, int32 count);

	virtual uint8 Read_UINT8(void);

	virtual uint16 Read_L_UINT16(void);
	virtual uint32 Read_L_UINT32(void);
	virtual void ReadArray_L_UINT16s(uint16 *buffer, int32 count);
	virtual void ReadArray_L_UINT32s(uint32 *buffer, int32 count);

	virtual uint16 Read_B_UINT16(void);
	virtual uint32 Read_B_UINT32(void);
	virtual void ReadArray_B_UINT16s(uint16 *buffer, int32 count);
	virtual void ReadArray_B_UINT32s(uint32 *buffer, int32 count);

	virtual void Write_UINT8(uint8 value);

	virtual void Write_L_UINT16(uint16 value);
	virtual void Write_L_UINT32(uint32 value);
	virtual void WriteArray_L_UINT16s(const uint16 *buffer, int32 count);
	virtual void WriteArray_L_UINT32s(const uint32 *buffer, int32 count);

	virtual void Write_B_UINT16(uint16 value);
	virtual void Write_B_UINT32(uint32 value);
	virtual void WriteArray_B_UINT16s(const uint16 *buffer, int32 count);
	virtual void WriteArray_B_UINT32s(const uint32 *buffer, int32 count);

	virtual int64 Seek(int64 offset, PSeekFlags from);
	virtual int64 GetPosition(void) const;

	virtual int64 GetLength(void) const;
	virtual void SetLength(int64 newLength);

	virtual PString GetFileName(void) const;
	virtual PString GetFilePath(void) const;
	virtual PString GetFullPath(void) const;
	virtual PString GetFileType(void) const;

	virtual void SetFilePath(PString newName);
	virtual void SetFileType(PString newType);

	virtual PFile *DuplicateFile(void) const;

protected:
	void GrowBuffer(int64 endPoint);

	PString fileType;

	uint8 *memBuffer;
	int64 bufferSize;
	int64 fileSize;
	int64 position;
	int32 grow;

private:
	// Protect some of the PFile functions, so they can't be used
	virtual void Open(uint16 /*openFlags*/) { ASSERT(false); };
	virtual void Open(PString /*fileName*/, uint16 /*openFlags*/) {
		ASSERT(false);
	};
};



/******************************************************************************/
/* PSocketFile class.                                                         */
/******************************************************************************/
/**
 *	This is the super class for reading and writing to files over thru a
 *	socket. This class is not intended to be used directly.
 */
class _IMPEXP_PKLIB PSocketFile : public PFile
{
protected:
	PSocketFile(PLineSocket *socket, int32 maxBufferSize = 100);
	virtual ~PSocketFile(void);

public:
	virtual int32 Read(void *buffer, int32 count);

	virtual int64 Seek(int64 offset, PSeekFlags from);
	virtual int64 GetPosition(void) const;

	virtual int64 GetLength(void) const;

protected:
	void Cleanup(void);			// Is called from the derived class Close() function

	PLineSocket *dataSock;		// Used as the data socket
	uint16 curOpenFlags;		// Will be set in the Open() function

	int32 fileSize;

	PList<uint8 *> bufferList;
	int32 maxElements;

	int32 startPos;
	int32 endPos;
	int32 currentPos;
	int32 lastLen;

private:
	// Protect some of the PFile functions, so they can't be used
	virtual int32 Write(const void * /*buffer*/, int32 /*count*/) {
		ASSERT(false);
		return 0;
	};
	virtual void SetLength(int64 /*newLength*/) { ASSERT(false); };
	virtual void SetFilePath(PString /*newName*/) { ASSERT(false); };
	virtual void SetFileType(PString /*newType*/) { ASSERT(false); };
};



/******************************************************************************/
/* PHTTPFile class                                                            */
/******************************************************************************/
/**
 *	This is a specialized PSocketFile class for reading and writing to files
 *	over thru a HTTP connection.
 */
class _IMPEXP_PKLIB PHTTPFile : public PSocketFile
{
public:
	PHTTPFile(PLineSocket *socket, int32 maxBufferSize = 100);
	PHTTPFile(PString url, uint16 openFlags, PLineSocket *socket, int32 maxBufferSize = 100);
	virtual ~PHTTPFile(void);

	virtual void Open(uint16 openFlags);
	virtual void Open(PString url, uint16 openFlags);
	virtual void Close(void);

	virtual PString GetFileName(void) const;
	virtual PString GetFilePath(void) const;
	virtual PString GetFullPath(void) const;
	virtual PString GetFileType(void) const;

	void SetAgentName(PString agent);

	virtual PFile *DuplicateFile(void) const;

protected:
	PString fileType;
	PString agentName;
};



/******************************************************************************/
/* PFTPFile class                                                             */
/******************************************************************************/
/**
 *	This is a specialized PSocketFile class for reading and writing to files
 *	over thru a FTP connection.
 */
class _IMPEXP_PKLIB PFTPFile : public PSocketFile
{
public:
	PFTPFile(PLineSocket *socket, int32 maxBufferSize = 100);
	PFTPFile(PString url, uint16 openFlags, PLineSocket *socket, int32 maxBufferSize = 100);
	virtual ~PFTPFile(void);

	virtual void Open(uint16 openFlags);
	virtual void Open(PString url, uint16 openFlags);
	virtual void Close(void);

	virtual PString GetFileName(void) const;
	virtual PString GetFilePath(void) const;
	virtual PString GetFullPath(void) const;
	virtual PString GetFileType(void) const;

	void SetLogonName(PString user, PString password);
	void SetLogonName(PString url);

	virtual PFile *DuplicateFile(void) const;

protected:
	PLineSocket *cmdSock;		// Used as the command socket

	PString fileType;
	PString logonName;
	PString logonPassword;
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
