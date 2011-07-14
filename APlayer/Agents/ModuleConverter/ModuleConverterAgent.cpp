/******************************************************************************/
/* APlayer Module converter agent class.                                      */
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
#include "ModuleConverterAgent.h"
#include "ModuleConverter.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.02f



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ModuleConverterAgent::ModuleConverterAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global)
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
ModuleConverterAgent::~ModuleConverterAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float ModuleConverterAgent::GetVersion(void)
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
uint32 ModuleConverterAgent::GetSupportFlags(int32 index)
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
PString ModuleConverterAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_MODC_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString ModuleConverterAgent::GetDescription(int32 index)
{
	PString description;

	description.LoadString(res, IDS_MODC_DESCRIPTION);
	return (description);
}



/******************************************************************************/
/* GetPluginPriority() returns the priority the given plug-in place should    */
/*      have.                                                                 */
/*                                                                            */
/* Input:  "pluginFlag" is one of the plug-in flags to return the priority    */
/*         for.                                                               */
/*                                                                            */
/* Output: The priority.                                                      */
/******************************************************************************/
int8 ModuleConverterAgent::GetPluginPriority(uint32 pluginFlag)
{
	// Make sure we come before the MikModConverter, so
	// the converted UMX modules will be converted once more
	// by the MikModConverter
	if (pluginFlag == apaConverter)
		return (25);

	return (0);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool ModuleConverterAgent::InitAgent(int32 index)
{
	// Add all the converters supported
	converters.AddTail(new ModConv_FC13(res));
	converters.AddTail(new ModConv_SFX13(res));
	converters.AddTail(new ModConv_UMX(res));

	return (true);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void ModuleConverterAgent::EndAgent(int32 index)
{
	int32 i, count;
	ModuleConverter *convItem;

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
ap_result ModuleConverterAgent::Run(int32 index, uint32 command, void *args)
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
ap_result ModuleConverterAgent::ConvertModule(APAgent_ConvertModule *convInfo)
{
	int32 i, count;
	ModuleConverter *convItem;
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
				// Found the converter
				convInfo->moduleFile->SeekToBegin();

				// Allocate a buffer to store the memory file in.
				// The only reason we do this, is to save some time. The
				// buffer won't be reallocated all the time.
				//
				// Create the buffer with the same size as the original file
				size      = convInfo->moduleFile->GetLength();
				memBuffer = new uint8[size];
				if (memBuffer == NULL)
					throw PMemoryException();

				// Create a memory file object to store the converted module in
				memFile = new PMemFile(memBuffer, size, 4 * 1024);
//				memFile = new PFile("/boot/home/module", PFile::pModeReadWrite | PFile::pModeCreate);
				if (memFile == NULL)
				{
					delete[] memBuffer;
					throw PMemoryException();
				}

				if (convItem->ConvertModule(convInfo, memFile) == AP_OK)
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

				// Stop the loop
				break;
			}
		}
	}
	catch(PMemoryException e)
	{
		PString title, msg;

		title.LoadString(res, IDS_MODC_WIN_TITLE);
		msg.LoadString(res, IDS_MODC_ERR_MEMORY);

		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();

		retVal = AP_ERROR;
	}

	return (retVal);
}
