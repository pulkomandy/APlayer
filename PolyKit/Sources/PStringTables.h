/******************************************************************************/
/* PString table header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PStringTable_h
#define __PStringTable_h

// PolyKit headers
#include "POS.h"
#include "PStringCharTables.h"
#include "PStringCharSet_Amiga.h"
#include "PStringCharSet_MacRoman.h"
#include "PStringCharSet_OEM850.h"
#include "PStringCharSet_Win1250.h"
#include "PStringCharSet_Win1251.h"
#include "PStringCharSet_Win1252.h"
#include "PStringCharSet_Win1253.h"
#include "PStringCharSet_Win1254.h"
#include "PStringCharSet_Win1257.h"
#include "PStringCharSet_Win1258.h"


/******************************************************************************/
/* Decimal number table                                                       */
/******************************************************************************/
static const uint32 decimal[] =
{
	1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10
};



/******************************************************************************/
/* Positive floating power values                                             */
/******************************************************************************/
static const double posPower[] =
{
	1.0e+256, 1.0e+128, 1.0e+64, 1.0e+32, 1.0e+16, 1.0e+8, 1.0e+4, 1.0e+2, 1.0e+1
};



/******************************************************************************/
/* Negative floating power values                                             */
/******************************************************************************/
static const double negPower[] =
{
	1.0e-256, 1.0e-128, 1.0e-64, 1.0e-32, 1.0e-16, 1.0e-8, 1.0e-4, 1.0e-2, 1.0e-1
};

#endif
