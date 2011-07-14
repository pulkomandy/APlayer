/******************************************************************************/
/* APlayer decruncher agent class.                                            */
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
#include "PBinary.h"
#include "PFile.h"
#include "PAlert.h"

// APlayerKit headers
#include "APGlobalData.h"

// Agent headers
#include "DecruncherAgent.h"
#include "Decruncher.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.0f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
DecruncherAgent::DecruncherAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global)
{
	// Fill out the version variable
	aplayerVersion = APLAYER_CURRENT_VERSION;

	// Allocate the resource object
	res = new PResource(fileName);
	if (res == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
DecruncherAgent::~DecruncherAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float DecruncherAgent::GetVersion(void)
{
	return (AgentVersion);
}



/******************************************************************************/
/* GetSupportFlags() returns some flags telling what the add-on supports.     */
/*                                                                            */
/* Input:  "index" is the add-on index number.                                */
/*                                                                            */
/* Output: Is the flags.                                                      */
/******************************************************************************/
uint32 DecruncherAgent::GetSupportFlags(int32 index)
{
	return (apaDecruncher);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString DecruncherAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_DECRUNCH_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString DecruncherAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_DECRUNCH_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool DecruncherAgent::InitAgent(int32 index)
{
	// Add all the decrunchers supported
	decrunchers.AddTail(new Decrunch_PowerPacker);
	decrunchers.AddTail(new Decrunch_XPK_SQSH);

	return (true);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void DecruncherAgent::EndAgent(int32 index)
{
	int32 i, count;
	Decruncher *item;

	// Remove all the converters again
	count = decrunchers.CountItems();

	for (i = 0; i < count; i++)
	{
		item = decrunchers.GetAndRemoveItem(0);
		delete item;
	}
}



/******************************************************************************/
/* Run() will run one command in the agent.                                   */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*         "command" is the command to run.                                   */
/*         "args" is a pointer to a structure containing the arguments. The   */
/*         structure is different for each command.                           */
/*                                                                            */
/* Output: The result from the command.                                       */
/******************************************************************************/
ap_result DecruncherAgent::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Decrunch the file
		case APDA_DECRUNCH_FILE:
		{
			return (DecrunchFile((APAgent_DecrunchFile *)args));
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* DecrunchFile() will decrunch the file if it's packed with one of the known */
/*      packers.                                                              */
/*                                                                            */
/* Input:  "decrunchInfo" is a pointer to the decruncher structure.           */
/*                                                                            */
/* Output: The result from the decrunching.                                   */
/******************************************************************************/
ap_result DecruncherAgent::DecrunchFile(APAgent_DecrunchFile *decrunchInfo)
{
	int32 i, count;
	Decruncher *item;
	PBinary sourceBuf, destBuf;
	PMemFile *memFile;
	uint32 length;
	ap_result retVal = AP_UNKNOWN;

	// Call the test function in the decrunchers
	count = decrunchers.CountItems();

	for (i = 0; i < count; i++)
	{
		item = decrunchers.GetItem(i);
		if (item->Determine(decrunchInfo))
		{
			// Found the decruncher, now read the whole file into memory
			decrunchInfo->file->SeekToBegin();
			length = decrunchInfo->file->GetLength();
			sourceBuf.SetLength(length);
			decrunchInfo->file->Read(sourceBuf.GetBufferForWriting(), length);

			// Allocate a buffer to hold the decrunched data in
			length = item->GetUnpackedSize(decrunchInfo);
			destBuf.SetLength(length);

			// Unpack the module
			if (item->Unpack(sourceBuf, destBuf) == AP_OK)
			{
				// Create a memory file to hold the decrunched data
				memFile = new PMemFile();
				if (memFile == NULL)
					retVal = AP_ERROR;
				else
				{
					// Attach the allocated buffer to the memory file
					memFile->Attach(destBuf.Detach(), length);

					// Initialize the info structure
					decrunchInfo->decrunchedFile = memFile;
					retVal = AP_OK;
				}
			}
			else
			{
				// Show error
				PString title, msg;

				title.LoadString(res, IDS_DECRUNCH_WIN_TITLE);
				msg.LoadString(res, IDS_DECRUNCH_ERR_CORRUPT);

				PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
				alert.Show();
			}

			// Stop the loop
			break;
		}
	}

	return (retVal);
}
