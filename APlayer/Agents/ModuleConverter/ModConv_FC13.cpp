/******************************************************************************/
/* ModuleConverter Future Composer 1.0-1.3 class.                             */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"

// Agent headers
#include "ModuleConverter.h"
#include "ModuleConverterAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Wave table lengths                                                         */
/******************************************************************************/
static uint8 waveLength[80] =
{
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
	0x10, 0x10, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x10, 0x08, 0x10, 0x10, 0x08, 0x08, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



/******************************************************************************/
/* Wave tables                                                                */
/******************************************************************************/
static uint8 waveTables[] =
{
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 0
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0x3f, 0x37, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 1
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0x37, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 2
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0x2f, 0x27, 0x1f, 0x17, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 3
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0x27, 0x1f, 0x17, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 4
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0x1f, 0x17, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 5
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x17, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 6
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x0f, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 7
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x07,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 8
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0xff, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 9
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x07, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 10
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x88, 0x0f, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 11
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x88, 0x90, 0x17, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 12
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x88, 0x90, 0x98, 0x1f, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 13
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x88, 0x90, 0x98, 0xa0, 0x27, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 14
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0x2f, 0x37,
	0xc0, 0xc0, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,			// 15
	0x00, 0xf8, 0xf0, 0xe8, 0xe0, 0xd8, 0xd0, 0xc8,
	0xc0, 0xb8, 0xb0, 0xa8, 0xa0, 0x98, 0x90, 0x88,
	0x80, 0x88, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0x37,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 16
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 17
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 18
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 19
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 20
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 21
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 22
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 23
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7f,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 24
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 25
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 26
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 27
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 28
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f, 0x7f,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,			// 29
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81,
	0x81, 0x81, 0x81, 0x81, 0x81, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,			// 30
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,			// 31
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,			// 32
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f,			// 33
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f,			// 34
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f,			// 35
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f,			// 36
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,			// 37
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,			// 38
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,			// 39
	0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f,
	0x80, 0x80, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8,			// 40
	0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
	0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x7f,
	0x80, 0x80, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,			// 41
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
	0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66,			// 42
	0x61, 0x58, 0x53, 0x4d, 0x2c, 0x20, 0x18, 0x12,
	0x04, 0xdb, 0xd3, 0xcd, 0xc6, 0xbc, 0xb5, 0xae,
	0xa8, 0xa3, 0x9d, 0x99, 0x93, 0x8e, 0x8b, 0x8a,
	0x45, 0x45, 0x79, 0x7d, 0x7a, 0x77, 0x70, 0x66,			// 43
	0x5b, 0x4b, 0x43, 0x37, 0x2c, 0x20, 0x18, 0x12,
	0x04, 0xf8, 0xe8, 0xdb, 0xcf, 0xc6, 0xbe, 0xb0,
	0xa8, 0xa4, 0x9e, 0x9a, 0x95, 0x94, 0x8d, 0x83,
	0x00, 0x00, 0x40, 0x60, 0x7f, 0x60, 0x40, 0x20, 		// 44
	0x00, 0xe0, 0xc0, 0xa0, 0x80, 0xa0, 0xc0, 0xe0,
	0x00, 0x00, 0x40, 0x60, 0x7f, 0x60, 0x40, 0x20,			// 45
	0x00, 0xe0, 0xc0, 0xa0, 0x80, 0xa0, 0xc0, 0xe0,
	0x80, 0x80, 0x90, 0x98, 0xa0, 0xa8, 0xb0, 0xb8,			// 46
	0xc0, 0xc8, 0xd0, 0xd8, 0xe0, 0xe8, 0xf0, 0xf8,
	0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38,
	0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x7f,
	0x80, 0x80, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
	0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
};



/******************************************************************************/
/* CheckModule() will be check the module to see if it's a FC13 module.       */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a converter info structure where to     */
/*         read and store information needed.                                 */
/******************************************************************************/
bool ModConv_FC13::CheckModule(APAgent_ConvertModule *convInfo)
{
	uint32 fileSize;
	PFile *file = convInfo->moduleFile;

	// Check the module size
	fileSize = file->GetLength();
	if (fileSize < 100)
		return (false);

	// Check the mark
	file->SeekToBegin();

	if (file->Read_B_UINT32() != 'SMOD')
		return (false);

	// Skip the song length
	file->Seek(4, PFile::pSeekCurrent);

	// Check the offset pointers
	for (int8 i = 0; i < 8; i++)
	{
		if (file->Read_B_UINT32() > fileSize)
			return (false);
	}

	return (true);
}



/******************************************************************************/
/* ModuleConverter() will be convert the module from FC13 to FC14.            */
/*                                                                            */
/* Input:  "convInfo" is a pointer to a converter info structure where to     */
/*         read and store information needed.                                 */
/*         "destFile" is where to write the converted data.                   */
/*                                                                            */
/* Output: Is an APlayer return code.                                         */
/******************************************************************************/
ap_result ModConv_FC13::ConvertModule(APAgent_ConvertModule *convInfo, PFile *destFile)
{
	PFile *file = convInfo->moduleFile;
	ap_result retVal = AP_OK;
	uint8 *pattBuf = NULL;

	try
	{
		uint32 i;
		uint32 seqLength;
		uint16 sampleLengths[10];
		uint32 offsets[8];
		uint32 newOffsets[8];

		// Start to write the ID mark
		file->Read_B_UINT32();
		destFile->Write_B_UINT32('FC14');

		// Copy the sequence length
		seqLength = file->Read_B_UINT32();

		// Make the sequence even
		if (seqLength & 0x01)
			destFile->Write_B_UINT32(seqLength + 1);
		else
			destFile->Write_B_UINT32(seqLength);

		// Read the offsets
		file->ReadArray_B_UINT32s(offsets, 8);
		destFile->Seek(8 * 4, PFile::pSeekCurrent);

		// Copy the sample information
		for (i = 0; i < 10; i++)
		{
			sampleLengths[i] = file->Read_B_UINT16();
			destFile->Write_B_UINT16(sampleLengths[i]);
			destFile->Write_B_UINT16(file->Read_B_UINT16());
			destFile->Write_B_UINT16(file->Read_B_UINT16());
		}

		if (file->IsEOF())
		{
			ShowError(IDS_MODC_ERR_LOADING_HEADER);
			throw PUserException();
		}

		// Write the wave table lengths
		destFile->Write(waveLength, 80);

		// Copy the sequences
		CopyData(file, destFile, seqLength);

		// Copy the patterns
		newOffsets[0] = destFile->GetPosition();
		if (newOffsets[0] & 0x1)
		{
			// Odd offset, make it event
			newOffsets[0]++;
			destFile->Write_UINT8(0x00);
		}

		newOffsets[1] = offsets[1];

		// Allocate buffer to hold the patterns
		pattBuf = new uint8[offsets[1]];
		if (pattBuf == NULL)
			throw PMemoryException();

		// Load the pattern data into the buffer
		file->Seek(offsets[0], PFile::pSeekBegin);
		file->Read(pattBuf, offsets[1]);

		// Scan the pattern data after the portamento flags and double it's data
		for (i = 1; i < offsets[1]; i += 2)
		{
			if (pattBuf[i] & 0x80)
				pattBuf[i + 2] = (((pattBuf[i + 2] & 0x1f) * 2) & 0x1f) | (pattBuf[i + 2] & 0x20);
		}

		// Write the patterns
		destFile->Write(pattBuf, offsets[1]);

		// Free the pattern buffer
		delete[] pattBuf;
		pattBuf = NULL;

		// Copy the frequency sequences
		newOffsets[2] = destFile->GetPosition();
		newOffsets[3] = offsets[3];
		file->Seek(offsets[2], PFile::pSeekBegin);
		CopyData(file, destFile, offsets[3]);

		// Copy the volume sequences
		newOffsets[4] = destFile->GetPosition();
		newOffsets[5] = offsets[5];
		file->Seek(offsets[4], PFile::pSeekBegin);
		CopyData(file, destFile, offsets[5]);

		if (file->IsEOF())
		{
			ShowError(IDS_MODC_ERR_LOADING_PATTERNS);
			throw PUserException();
		}

		// Copy the sample data
		newOffsets[6] = destFile->GetPosition();
		file->Seek(offsets[6], PFile::pSeekBegin);

		for (i = 0; i < 10; i++)
		{
			if (sampleLengths[i] != 0)
				CopyData(file, destFile, sampleLengths[i] * 2);

			if (file->IsEOF())
			{
				ShowError(IDS_MODC_ERR_LOADING_SAMPLES);
				throw PUserException();
			}

			// Write pad bytes
			destFile->Write_B_UINT16(0);
		}

		// Write the wavetables
		newOffsets[7] = destFile->GetPosition();
		destFile->Write(waveTables, sizeof(waveTables));

		// Seek back and write the offsets
		destFile->Seek(8, PFile::pSeekBegin);
		destFile->WriteArray_B_UINT32s(newOffsets, 8);

		// Set the module type
		convInfo->modKind.LoadString(res, IDS_MODC_NAME_FC13);
	}
	catch(PUserException e)
	{
		delete[] pattBuf;

		retVal = AP_ERROR;
	}

	return (retVal);
}
