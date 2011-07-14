/******************************************************************************/
/* ModuleConverter Converter class.                                           */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"
#include "PString.h"
#include "PResource.h"
#include "PFile.h"
#include "PAlert.h"

// Agent headers
#include "ModuleConverter.h"
#include "ModuleConverterAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ModuleConverter::ModuleConverter(PResource *resource)
{
	// Initialize member variables
	res = resource;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
ModuleConverter::~ModuleConverter(void)
{
}



/******************************************************************************/
/* ShowError() opens a message window and show the error.                     */
/*                                                                            */
/* Input:  "id" is the string id to show.                                     */
/******************************************************************************/
void ModuleConverter::ShowError(uint32 id)
{
	PString title, msg;

	title.LoadString(res, IDS_MODC_WIN_TITLE);
	msg.LoadString(res, id);

	PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
	alert.Show();
}



/******************************************************************************/
/* CopyData() will copy data from the source file to the destination file.    */
/*                                                                            */
/* Input:  "source" is a pointer to the source file.                          */
/*         "dest" is a pointer to the destination file.                       */
/*         "length" is the number of bytes to copy.                           */
/*                                                                            */
/* Except: PFileException.                                                    */
/******************************************************************************/
void ModuleConverter::CopyData(PFile *source, PFile *dest, uint32 length)
{
	uint8 buf[1024];

	while (length >= 1024)
	{
		source->Read(buf, 1024);
		dest->Write(buf, 1024);
		length -= 1024;
	}

	if (length > 0)
	{
		source->Read(buf, length);
		dest->Write(buf, length);
	}
}
