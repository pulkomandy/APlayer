/******************************************************************************/
/* APlayer ProWizard Agent class.                                             */
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
#include "PBinary.h"
#include "PAlert.h"

// Agent headers
#include "ProWizardAgent.h"
#include "ResourceIDs.h"


/******************************************************************************/
/* Version                                                                    */
/******************************************************************************/
#define AgentVersion		2.01f



/******************************************************************************/
/* Useful constants                                                           */
/******************************************************************************/
#define MINIMAL_FILE_LENGTH		1084
#define MAXIMAL_FILE_LENGTH		(2 * 1024 * 1024)



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
ProWizardAgent::ProWizardAgent(APGlobalData *global, PString fileName) : APAddOnAgent(global)
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
ProWizardAgent::~ProWizardAgent(void)
{
	// Delete the resource object
	delete res;
}



/******************************************************************************/
/* GetVersion() returns the version of the add-on.                            */
/*                                                                            */
/* Output: The add-on version.                                                */
/******************************************************************************/
float ProWizardAgent::GetVersion(void)
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
uint32 ProWizardAgent::GetSupportFlags(int32 index)
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
PString ProWizardAgent::GetName(int32 index)
{
	PString name;

	name.LoadString(res, IDS_PROZ_NAME);
	return (name);
}



/******************************************************************************/
/* GetDescription() returns the description of the current add-on.            */
/*                                                                            */
/* Input:  "index" is the add-on index starting from 0.                       */
/*                                                                            */
/* Output: The add-on description.                                            */
/******************************************************************************/
PString ProWizardAgent::GetDescription(int32 index)
{
	PString description, convName;
	int32 num;

	// Get description text
	description.LoadString(res, IDS_PROZ_DESCRIPTION);

	// Build list of converters
	for (num = PROZ_TYPE_START; num <= PROZ_TYPE_END; num++)
	{
		convName.LoadString(res, num);
		description += convName + "\n";
	}

	return (description);
}



/******************************************************************************/
/* InitAgent() will initialize the agent.                                     */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/*                                                                            */
/* Output: True for success, false for an error.                              */
/******************************************************************************/
bool ProWizardAgent::InitAgent(int32 index)
{
	// Add all the converters. Do NOT change this order
	converters.AddTail(new PROZ_KRIS);				// 00 Kris Tracker
	converters.AddTail(new PROZ_NP2);				// 01 NoisePacker v1 - v2
	converters.AddTail(new PROZ_NP3);				// 02 NoisePacker v3
	converters.AddTail(new PROZ_DI);				// 03 Digital Illusions
	converters.AddTail(new PROZ_PHA);				// 04 PhaPacker
	converters.AddTail(new PROZ_UNIC);				// 05 Unic Tracker
	converters.AddTail(new PROZ_LAX);				// 06 Laxity Packer
	converters.AddTail(new PROZ_WP);				// 07 Wanton Packer
	converters.AddTail(new PROZ_NRU);				// 08 NoiseRunner
	converters.AddTail(new PROZ_EUREKA);			// 09 Eureka Packer
	converters.AddTail(new PROZ_P41A);				// 10 The Player v4.0A + v4.0B + v4.1A
	converters.AddTail(new PROZ_P61A);				// 10b The Player v5.0A + v6.0A + v6.1A
	converters.AddTail(new PROZ_PRU1);				// 11 ProRunner v1
	converters.AddTail(new PROZ_PRU2);				// 12 ProRunner v2
	converters.AddTail(new PROZ_PP10);				// 13 ProPacker v1.0
	converters.AddTail(new PROZ_PP30);				// 14 ProPacker v2.1 + v3.0
	converters.AddTail(new PROZ_PM18);				// 15 Promizer v1.0c + v1.8a
	converters.AddTail(new PROZ_PM20);				// 16 Promizer v2.0
	converters.AddTail(new PROZ_PM40);				// 17 Promizer v4.0
	converters.AddTail(new PROZ_FC_M);				// 18 FC-M Packer
	converters.AddTail(new PROZ_HEATSEEK);			// 19 Heatseeker mc1.0
	converters.AddTail(new PROZ_XANN);				// 20 Xann Packer
	converters.AddTail(new PROZ_SKYT);				// 21 SKYT Packer
	converters.AddTail(new PROZ_MP);				// 22 Module Protector
	converters.AddTail(new PROZ_GMC);				// 23 Game Music Creator
	converters.AddTail(new PROZ_PM01);				// 24 Promizer v0.1
	converters.AddTail(new PROZ_AC1D);				// 25 AC1D Packer
	converters.AddTail(new PROZ_PYGMY);				// 26 Pygmy Packer
	converters.AddTail(new PROZ_CHAN);				// 27 Channel Player v1 - v3
	converters.AddTail(new PROZ_STAR);				// 28 StarTrekker Packer
	converters.AddTail(new PROZ_FUZZAC);			// 29 Fuzzac Packer
	converters.AddTail(new PROZ_KSM);				// 30 Kefrens Sound Machine
	converters.AddTail(new PROZ_ST26);				// 31 SoundTracker v2.6 + IceTracker v1.0
	converters.AddTail(new PROZ_TP1);				// 32 Tracker Packer v1
	converters.AddTail(new PROZ_TP3);				// 32b Tracker Packer v2 - v3
	converters.AddTail(new PROZ_NTPK);				// 33 NoiseTracker Pak
	converters.AddTail(new PROZ_POWER);				// 35 Power Music
	converters.AddTail(new PROZ_ZEN);				// 37 Zen Packer
	converters.AddTail(new PROZ_HRT);				// 38 Hornet Packer

	// Additional converters
	converters.AddTail(new PROZ_FUCHS);				// Fuchs Tracker
	converters.AddTail(new PROZ_STIM);				// SlamTilt
	converters.AddTail(new PROZ_TDD);				// The Dark Demon

	return (true);
}



/******************************************************************************/
/* EndAgent() will clean up the agent.                                        */
/*                                                                            */
/* Input:  "index" is the agent index number.                                 */
/******************************************************************************/
void ProWizardAgent::EndAgent(int32 index)
{
	int32 i, count;
	ProWizard *convItem;

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
ap_result ProWizardAgent::Run(int32 index, uint32 command, void *args)
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
ap_result ProWizardAgent::ConvertModule(APAgent_ConvertModule *convInfo)
{
	PBinary module;
	uint8 *buffer;
	uint32 size;
	ap_result retVal;

	// First check the size of the file
	size = convInfo->moduleFile->GetLength();
	if ((size < MINIMAL_FILE_LENGTH) || (size > MAXIMAL_FILE_LENGTH))
		return (AP_UNKNOWN);

	// Allocate buffer to hold the file
	module.SetLength(size + 256);		// Add safety buffer
	buffer = module.GetBufferForWriting();

	// Clear the safety part of the buffer
	memset(&buffer[size], 0, 256);

	// Load the file into the memory
	convInfo->moduleFile->SeekToBegin();
	convInfo->moduleFile->Read(buffer, size);

	// Call the test function in the converters
	retVal = CheckModule(module, convInfo);

	return (retVal);
}



/******************************************************************************/
/* CheckModule() check a converter list and check the module against these    */
/*      converters.                                                           */
/*                                                                            */
/* Input:  "module" is a reference to the module to check.                    */
/*         "convInfo" is a pointer to the converter structure.                */
/*                                                                            */
/* Output: AP_OK if the module has been converted, else AP_UNKNOWN.           */
/******************************************************************************/
ap_result ProWizardAgent::CheckModule(const PBinary &module, APAgent_ConvertModule *convInfo)
{
	PString msg;
	PMemFile *memFile;
//	PFile *memFile;
	int32 i, count;
	ProWizard *convItem;
	uint32 calcSize;
	uint8 *memBuffer;
	ap_result retVal = AP_UNKNOWN;

	try
	{
		// Get the number of converters
		count = converters.CountItems();

		for (i = 0; i < count; i++)
		{
			convItem = converters.GetItem(i);
			calcSize = convItem->CheckModule(module);
			if (calcSize != 0)
			{
				// Found the converter
				//
				// Allocate a buffer to store the file in.
				// The only reason we do this, is to save some time. The
				// buffer won't be reallocated all the time.
				//
				// Create the buffer with the calculated size
				memBuffer = new uint8[calcSize];
				if (memBuffer == NULL)
					throw PMemoryException();

				// Create a memory file object to store the converted module in
				memFile = new PMemFile(memBuffer, calcSize, 4 * 1024);
//				memFile = new PFile("/boot/home/module", PFile::pModeReadWrite | PFile::pModeCreate);
				if (memFile == NULL)
				{
					delete[] memBuffer;
					throw PMemoryException();
				}

				if (convItem->ConvertModule(module, memFile) == AP_OK)
				{
					// Initialize the info structure
					convInfo->newModuleFile = memFile;
					convInfo->modKind.LoadString(res, convItem->GetModuleType());
					retVal = AP_OK;
				}
				else
				{
					// Show an error
					PString title, message, type;

					// Get the alert strings
					title.LoadString(res, IDS_PROZ_WIN_TITLE);
					type.LoadString(res, convItem->GetModuleType());
					message.Format_S1(res, IDS_PROZ_ERR_CANT_CONVERT, type);

					PAlert alert(title, message, PAlert::pStop, PAlert::pOk);
					alert.Show();

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

		title.LoadString(res, IDS_PROZ_WIN_TITLE);
		msg.LoadString(res, IDS_PROZ_ERR_MEMORY);

		PAlert alert(title, msg, PAlert::pStop, PAlert::pOk);
		alert.Show();

		retVal = AP_ERROR;
	}

	return (retVal);
}
