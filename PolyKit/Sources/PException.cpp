/******************************************************************************/
/* PException implementation file.                                            */
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
#include "PSynchronize.h"
#include "PException.h"


/******************************************************************************/
/* PException class                                                           */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PException object with the
 *	specified error number.
 *
 *	@param error the error number for the exception.
 *//***************************************************************************/
PException::PException(int32 error)
{
	errorNum = error;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PException::~PException(void)
{
}





/******************************************************************************/
/* PBoundsException class                                                     */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PBoundsException object with the
 *	specified error number and the bound value causing the exception.
 *
 *	@param error the error number for the exception.
 *	@param val the bound value causing the exception.
 *//***************************************************************************/
PBoundsException::PBoundsException(int32 error, double val) : PException(error)
{
	value = val;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PBoundsException::~PBoundsException(void)
{
}





/******************************************************************************/
/* PEventException class                                                      */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PEventException object with the
 *	event object causing the exception.
 *
 *	@param object the event object causing the exception.
 *//***************************************************************************/
PEventException::PEventException(PSync *object) : PException()
{
	event = object;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PEventException::~PEventException(void)
{
}





/******************************************************************************/
/* PFileException class                                                       */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PFileException object with the
 *	specified error number and the file name causing the exception.
 *
 *	@param error the error number for the exception.
 *	@param file the file name of the file causing the exception.
 *//***************************************************************************/
PFileException::PFileException(int32 error, PString file) : PException(error)
{
	fileName = file;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PFileException::~PFileException(void)
{
}





/******************************************************************************/
/* PFTPException class                                                        */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PFTPException object with the
 *	respond header causing the exception.
 *
 *	@param respond the respond header causing the exception.
 *//***************************************************************************/
PFTPException::PFTPException(PString respond) : PException()
{
	// Remember the full respond header
	respondHeader = respond;

	// Find the error number
	errorNum = respond.GetUNumber();
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PFTPException::~PFTPException(void)
{
}





/******************************************************************************/
/* PHTTPException class                                                       */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PHTTPException object with the
 *	respond header causing the exception.
 *
 *	@param respond the respond header causing the exception.
 *//***************************************************************************/
PHTTPException::PHTTPException(PString respond) : PException()
{
	int32 index;

	// Remember the full respond header
	respondHeader = respond;

	// Find the error number
	index = respond.Find(' ');
	if (index != -1)
		errorNum = respond.GetUNumber(index + 1);
	else
		errorNum = 400;
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PHTTPException::~PHTTPException(void)
{
}





/******************************************************************************/
/* PMemoryException class                                                     */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PMemoryException object.
 *//***************************************************************************/
PMemoryException::PMemoryException(void) : PException(P_GEN_ERR_NO_MEMORY)
{
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PMemoryException::~PMemoryException(void)
{
}





/******************************************************************************/
/* PNetworkException class                                                    */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PNetworkException object with the
 *	specified error number.
 *
 *	@param error the error number for the exception.
 *//***************************************************************************/
PNetworkException::PNetworkException(int32 error) : PException(error)
{
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PNetworkException::~PNetworkException(void)
{
}





/******************************************************************************/
/* PSoundException class                                                      */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PSoundException object with the
 *	specified error number.
 *
 *	@param error the error number for the exception.
 *//***************************************************************************/
PSoundException::PSoundException(int32 error) : PException(error)
{
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PSoundException::~PSoundException(void)
{
}





/******************************************************************************/
/* PSystemException class                                                     */
/******************************************************************************/

/******************************************************************************/
/**	Standard constructor for creating a new PSystemException object with the
 *	specified error number.
 *
 *	@param error the error number for the exception.
 *//***************************************************************************/
PSystemException::PSystemException(int32 error) : PException(error)
{
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PSystemException::~PSystemException(void)
{
}





/******************************************************************************/
/* PUserException class                                                       */
/******************************************************************************/

/******************************************************************************/
/** Standard constructor for creating a new PUserException object with the
 *	specified error number.
 *
 *	@param error the error number for the exception.
 *//***************************************************************************/
PUserException::PUserException(int32 error) : PException(error)
{
}



/******************************************************************************/
/**	Destructor which destroys this object.
 *//***************************************************************************/
PUserException::~PUserException(void)
{
}
