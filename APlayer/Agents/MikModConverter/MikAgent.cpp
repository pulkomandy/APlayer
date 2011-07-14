/******************************************************************************/
/* APlayer MikAgent class.                                                    */
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
#include "PList.h"
#include "PAlert.h"

// Agent headers
#include "MikAgent.h"
#include "MikConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.13f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
MikAgent::MikAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global)
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
MikAgent::~MikAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float MikAgent::GetVersion(void)
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
uint32 MikAgent::GetSupportFlags(int32 index)
{
	return (apaConverter);
}



/******************************************************************************/
/* GetName() returns the name of the current add-on.                          */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on name.                                                   */
/******************************************************************************/
PString MikAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MIKC_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString MikAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MIKC_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool MikAgent::InitAgent(int32 index)
{
	// Add all the converters supported
	converters.AddTail(new Mik669(res));
	converters.AddTail(new MikAMF(res));
	converters.AddTail(new MikDSM(res));
	converters.AddTail(new MikFAR(res));
	converters.AddTail(new MikGDM(res));
	converters.AddTail(new MikIMF(res));
	converters.AddTail(new MikIT(res));
	converters.AddTail(new MikS3M(res));
	converters.AddTail(new MikSTM(res));
	converters.AddTail(new MikSTX(res));
	converters.AddTail(new MikULT(res));
	converters.AddTail(new MikUNI(res));
	converters.AddTail(new MikXM(res));

	return (true);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void MikAgent::EndAgent(int32 index)
{
	int32 i, count;
	MikConverter *convItem;

	// Remove all the converters again
	count = converters.CountItems();

	for (i = 0; i < count; i++)
	{
		convItem = converters.GetAndRemoveItem(0);
		delete convItem;
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
ap_result MikAgent::Run(int32 index, uint32 command, void *args)
{
	switch (command)
	{
		// Convert the module
		case APCA_CONVERT_MODULE:
		{
			return (ConvertModule((APAgent_ConvertModule *)args));
		}
	}

	return (AP_UNKNOWN);
}



/******************************************************************************/
/* ConvertModule() will try to recognize the module and then convert it.      */
/*                                                                            */
/* Input:  "convInfo" is a pointer to the converter structure.                */
/*                                                                            */
/* Output: The result from the conversion.                                    */
/******************************************************************************/
ap_result MikAgent::ConvertModule(APAgent_ConvertModule *convInfo)
{
	int32 i, count;
	MikConverter *convItem;
	PMemFile *memFile;
//	PFile *memFile;
	uint8 *memBuffer;
	uint32 size;
	ap_result retVal = AP_UNKNOWN;

	try
	{
		// Call the test function in the converters
		count = converters.CountItems();

		for (i = 0; i < count; i++)
		{
			convItem = converters.GetItem(i);
			if (convItem->CheckModule(convInfo))
			{
				// Found the converter, so now convert it to UniMod structures
				convInfo->moduleFile->SeekToBegin();
				if (convItem->ConvertModule(convInfo) == AP_OK)
				{
					// Allocate a buffer to store the memory file in.
					// The only reason we do this, is to save some time. The
					// buffer won't be reallocated all the time.
					//
					// Create the buffer with the same size as the original
					// file. There is a big chance that the new module won't
					// use the whole buffer, infact it can cut out 10-20 kb!
					size      = convInfo->moduleFile->GetLength();
					memBuffer = new uint8[size];
					if (memBuffer == NULL)
						throw PMemoryException();

					// Create a PMemFile object to store the converted module in
					memFile = new PMemFile(memBuffer, size, 16 * 1024);
//					memFile = new PFile("/boot/home/module", PFile::pModeReadWrite | PFile::pModeCreate);
					if (memFile == NULL)
					{
						delete[] memBuffer;
						throw PMemoryException();
					}

					// Save the UniMod structures into the new module
					if (convItem->ConvertToUniMod(convInfo->moduleFile, memFile) == AP_OK)
					{
						// Initialize the info structure
						convInfo->newModuleFile = memFile;
						retVal = AP_OK;
					}
					else
					{
						delete memFile;
						retVal = AP_ERROR;
					}
				}
				else
					retVal = AP_ERROR;

				// Free all memory used
				convItem->FreeAll();
				break;
			}
		}
	}
	catch(PMemoryException e)
	{
		PString title, msg;

		title.LoadString(res, IDS_MIKC_WIN_TITLE);
		msg.LoadString(res, IDS_MIKC_ERR_MEMORY);

		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();

		retVal = AP_ERROR;
	}

	return (retVal);
}
