/******************************************************************************/
/* PException header file.                                                    */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PException_h
#define __PException_h

// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "ImportExport.h"


/******************************************************************************/
/* PException class                                                           */
/******************************************************************************/
#if __p_os == __p_beos && __POWERPC__
#pragma export on
#endif

/**
 *	This is the super class of all exception types in PolyKit. All exceptions
 *	in PolyKit are derived from the PException class.
 */
class _IMPEXP_PKLIB PException
{
public:
	PException(int32 error = P_ERR_ANY);
	virtual ~PException(void);

	/// The error number for the exception
	int32 errorNum;
};



/******************************************************************************/
/* PBoundsException class                                                     */
/******************************************************************************/
/**
 *	This is the super class of all bounds exceptions.
 */
class _IMPEXP_PKLIB PBoundsException : public PException
{
public:
	PBoundsException(int32 error = P_ERR_ANY, double val = 0.0);
	virtual ~PBoundsException(void);

	/// The value of the bound causing the exception
	double value;
};



/******************************************************************************/
/* PEventException class                                                      */
/******************************************************************************/
/**
 *	This is the super class of all event exceptions.
 */
class PSync;

class _IMPEXP_PKLIB PEventException : public PException
{
public:
	PEventException(PSync *object);
	virtual ~PEventException(void);

	/// The event causing the exception
	PSync *event;
};



/******************************************************************************/
/* PFileException class                                                       */
/******************************************************************************/
/**
 *	This is the super class of all file exceptions.
 */
class _IMPEXP_PKLIB PFileException : public PException
{
public:
	PFileException(int32 error = P_FILE_ERR_FILE, PString file = "");
	virtual ~PFileException(void);

	/// The file name of the file causing the exception
	PString fileName;
};



/******************************************************************************/
/* PFTPException class                                                        */
/******************************************************************************/
/**
 *	This is the super class of all FTP (File Transfer Protocol) exceptions.
 */
class _IMPEXP_PKLIB PFTPException : public PException
{
public:
	PFTPException(PString respond);
	virtual ~PFTPException(void);

	/// The respond header causing the exception
	PString respondHeader;
};



/******************************************************************************/
/* PHTTPException class                                                       */
/******************************************************************************/
/**
 *	This is the super class of all HTTP (HyperText Transfer Protocol)
 *	exceptions.
 */
class _IMPEXP_PKLIB PHTTPException : public PException
{
public:
	PHTTPException(PString respond);
	virtual ~PHTTPException(void);

	/// The respond header causing the exception
	PString respondHeader;
};



/******************************************************************************/
/* PMemoryException class                                                     */
/******************************************************************************/
/**
 *	This is the super class of all memory exceptions.
 */
class _IMPEXP_PKLIB PMemoryException : public PException
{
public:
	PMemoryException(void);
	virtual ~PMemoryException(void);
};



/******************************************************************************/
/* PNetworkException class                                                    */
/******************************************************************************/
/**
 *	This is the super class of all network exceptions.
 */
class _IMPEXP_PKLIB PNetworkException : public PException
{
public:
	PNetworkException(int32 error = P_ERR_ANY);
	virtual ~PNetworkException(void);
};



/******************************************************************************/
/* PSoundException class                                                      */
/******************************************************************************/
/**
 *	This is the super class of all sound/audio exceptions.
 */
class _IMPEXP_PKLIB PSoundException : public PException
{
public:
	PSoundException(int32 error = P_ERR_ANY);
	virtual ~PSoundException(void);
};



/******************************************************************************/
/* PSystemException class                                                     */
/******************************************************************************/
/**
 *	This is the super class of all operative system specific exceptions.
 */
class _IMPEXP_PKLIB PSystemException : public PException
{
public:
	PSystemException(int32 error = P_ERR_ANY);
	virtual ~PSystemException(void);
};



/******************************************************************************/
/* PUserException class                                                       */
/******************************************************************************/
/**
 *	This is the super class of user utilizing exceptions. Use this class to
 *	create new exception types of your own.
 */
class _IMPEXP_PKLIB PUserException : public PException
{
public:
	PUserException(int32 error = P_OK);
	virtual ~PUserException(void);
};

#if __p_os == __p_beos && __POWERPC__
#pragma export off
#endif

#endif
