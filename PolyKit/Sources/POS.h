/******************************************************************************/
/* POS header file.                                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __POS_h
#define __POS_h


////////////////////////////////////////////////////////////////////////////////
// Set the operation system to compile under to one of these values:
//
// __p_beos     - BeOS
////////////////////////////////////////////////////////////////////////////////
#define __p_beos			1

// Define __p_os according to the native operative system
//
// BeOS
#ifdef __BEOS__
#define __p_os				__p_beos
#endif

// BeOS headers
#if __p_os == __p_beos
#include "BeOS.h"
#endif

// Debug headers
#include "PDebug.h"



////////////////////////////////////////////////////////////////////////////////
// Set the endianess of your system you compile under to one of these values:
//
// __p_little   - Little endian
// __p_big      - Big endian
////////////////////////////////////////////////////////////////////////////////
#define __p_little			1
#define __p_big				2

#if __p_os == __p_beos

#ifdef __POWERPC__
#define __p_endian			__p_big
#else
#define __p_endian			__p_little
#endif

#endif



////////////////////////////////////////////////////////////////////////////////
// Datatype definition (BeOS datatypes)
//
// Use these datatypes everywhere in your add-on. This makes sure it can be
// compiled under any platform without any troubles.
////////////////////////////////////////////////////////////////////////////////
#if __p_os != __p_beos

typedef signed char						int8;
typedef unsigned char					uint8;
typedef volatile signed char			vint8;
typedef volatile unsigned char			vuint8;

typedef short							int16;
typedef unsigned short					uint16;
typedef volatile short					vint16;
typedef volatile unsigned short			vuint16;

typedef long							int32;
typedef unsigned long					uint32;
typedef volatile long					vint32;
typedef volatile unsigned long			vuint32;

typedef __int64							int64;
typedef unsigned __int64				uint64;
typedef volatile __int64				vint64;
typedef volatile unsigned __int64		vuint64;

typedef volatile long					vlong;
typedef volatile int					vint;
typedef volatile short					vshort;
typedef volatile char					vchar;

typedef volatile unsigned long			vulong;
typedef volatile unsigned int			vuint;
typedef volatile unsigned short			vushort;
typedef volatile unsigned char			vuchar;

typedef unsigned char					uchar;
typedef unsigned short					unichar;

#endif



////////////////////////////////////////////////////////////////////////////////
// Max/min macros
////////////////////////////////////////////////////////////////////////////////
#if __p_os == __p_beos

#undef min
#undef max
#define min(x, y)						min_c(x, y)
#define max(x, y)						max_c(x, y)

#endif



////////////////////////////////////////////////////////////////////////////////
// Endian macros
////////////////////////////////////////////////////////////////////////////////
#if __p_os == __p_beos

// Well, under BeOS, we just use Be's own macros
#define P_HOST_TO_LENDIAN_INT16(arg)	B_HOST_TO_LENDIAN_INT16(arg)
#define P_HOST_TO_LENDIAN_INT32(arg)	B_HOST_TO_LENDIAN_INT32(arg)

#define P_HOST_TO_BENDIAN_INT16(arg)	B_HOST_TO_BENDIAN_INT16(arg)
#define P_HOST_TO_BENDIAN_INT32(arg)	B_HOST_TO_BENDIAN_INT32(arg)

#define P_LENDIAN_TO_HOST_INT16(arg)	B_LENDIAN_TO_HOST_INT16(arg)
#define P_LENDIAN_TO_HOST_INT32(arg)	B_LENDIAN_TO_HOST_INT32(arg)

#define P_BENDIAN_TO_HOST_INT16(arg)	B_BENDIAN_TO_HOST_INT16(arg)
#define P_BENDIAN_TO_HOST_INT32(arg)	B_BENDIAN_TO_HOST_INT32(arg)

#endif



////////////////////////////////////////////////////////////////////////////////
// Directory slashes
////////////////////////////////////////////////////////////////////////////////
#if __p_os == __p_beos

#define P_DIRSLASH_STR	"/"
#define P_DIRSLASH_CHR	'/'

#endif



////////////////////////////////////////////////////////////////////////////////
// New line strings
////////////////////////////////////////////////////////////////////////////////
#if __p_os == __p_beos
#define P_NEWLINE_STR	"\n"
#endif



////////////////////////////////////////////////////////////////////////////////
// Error codes base lines
////////////////////////////////////////////////////////////////////////////////
#define PGeneralErrorBase		(LONG_MIN)						// 0x80000000
#define PFileErrorBase			(PGeneralErrorBase + 0x1000)	// 0x80001000
#define PSecurityErrorBase		(PFileErrorBase + 0x1000)		// 0x80002000
#define PKeyErrorBase			(PSecurityErrorBase + 0x1000)	// 0x80003000
#define PSoundErrorBase			(PKeyErrorBase + 0x1000)		// 0x80004000
#define PNetworkErrorBase		(PSoundErrorBase + 0x1000)		// 0x80005000



////////////////////////////////////////////////////////////////////////////////
// General errors
////////////////////////////////////////////////////////////////////////////////
enum PGeneralError
{
	P_OK						= 0,					// No error
	P_ERR_ANY					= -1,					// Any error

	P_GEN_ERR_BAD_ARGUMENT		= PGeneralErrorBase,	// Invalid argument passed to function
	P_GEN_ERR_NO_MEMORY,								// Out of memory
	P_GEN_ERR_NO_RESOURCES,								// Out of resources
	P_GEN_ERR_NO_INIT,									// Can't perform the operation. Missing proper initialization
	P_GEN_ERR_CANT_PERFORM,								// Can't perform the operation
	P_GEN_ERR_BAD_IDENTIFIER,							// Invalid identifier
	P_GEN_ERR_BAD_INDEX,								// Invalid index value
	P_GEN_ERR_UNSUPPORTED_OS,							// Unsupported operative system
	P_GEN_ERR_INSUFFICIENT_BUFFER,						// The data buffer passed to a function is too small
	P_GEN_ERR_TIMEOUT,									// Timed out
};



////////////////////////////////////////////////////////////////////////////////
// File errors
////////////////////////////////////////////////////////////////////////////////
enum PFileError
{
	P_FILE_ERR_FILE				= PFileErrorBase,		// Illegal file operation
	P_FILE_ERR_NAME_TOO_LONG,							// File or directory name too long
	P_FILE_ERR_ENTRY_NOT_FOUND,							// Entry could not be found
	P_FILE_ERR_ENTRY_EXISTS,							// Entry does already exists
	P_FILE_ERR_DISK_FULL,								// There is not enough space on the disk
	P_FILE_ERR_NOT_A_DIRECTORY,							// The directory is invalid
	P_FILE_ERR_IS_A_DIRECTORY,							// File operation on a directory
	P_FILE_ERR_SEEK,									// An attempt was made to move the file pointer before the beginning of the file
	P_FILE_ERR_EOF,										// EOF reached
	P_FILE_ERR_LINK,									// Link error in the file system
	P_FILE_ERR_LOCKED,									// File or directory is locked
	P_FILE_ERR_FILE_CORRUPT,							// File is corrupted
	P_FILE_ERR_INVALID_ENTRY_NAME,						// Entry name is invalid
	P_FILE_ERR_BAD_ACCESS_MODE,							// Invalid access mode
};



////////////////////////////////////////////////////////////////////////////////
// Security errors
////////////////////////////////////////////////////////////////////////////////
enum PSecurityError
{
	P_SEC_ERR_ACCESS_DENIED     = PSecurityErrorBase,	// Access denied
};



////////////////////////////////////////////////////////////////////////////////
// Sound errors
////////////////////////////////////////////////////////////////////////////////
enum PSoundError
{
	P_SOUND_ERR_NO_SOUNDCARD	= PSoundErrorBase,		// No soundcard is installed in the machine
	P_SOUND_ERR_NO_SOFTWARE,							// Required sound software is not started
	P_SOUND_ERR_UNKNOWN,								// Unknown sound error occured
};



////////////////////////////////////////////////////////////////////////////////
// Network errors
////////////////////////////////////////////////////////////////////////////////
enum PNetworkError
{
	P_NETWORK_ERR_INTERRUPTED	= PNetworkErrorBase,	// Socket was interrupted
	P_NETWORK_ERR_WOULD_BLOCK,							// Socket would block
	P_NETWORK_ERR_SOCKET_CLOSED,						// Socket is closed
	P_NETWORK_ERR_INTERFACE_BUSY,						// Network interface is busy
	P_NETWORK_ERR_ALREADY_CONNECTED,					// The socket is already connected
	P_NETWORK_ERR_CONNECTION_REJECTED,					// The listener rejected the connection
};

#endif
