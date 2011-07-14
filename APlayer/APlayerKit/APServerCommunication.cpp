/******************************************************************************/
/* APlayer server communication class.                                        */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of APlayer is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by The APlayer-Team.                               */
/* All rights reserved.                                                       */
/******************************************************************************/


// PolyKit headers
#include "POS.h"

// APlayerKit headers
#include "APServerCommunication.h"


/******************************************************************************/
/*                                                                            */
/*                         APServerCommunication class                        */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APServerCommunication::APServerCommunication(void)
{
	// Initialize member variables
	serverLooper = NULL;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APServerCommunication::~APServerCommunication(void)
{
}



/******************************************************************************/
/* ConnectToServer() will try to make a connection to the APlayer server.     */
/*                                                                            */
/* Input:  "clientName" is the name of your client. Only used for debugging.  */
/*         "dataFunc" is a pointer to a function that will be called when the */
/*         server sends you some data.                                        */
/*         "userData" is anything the caller wants.                           */
/*                                                                            */
/* Output: A handle you need to use in all the other communication calls.     */
/******************************************************************************/
void *APServerCommunication::ConnectToServer(PString clientName, ClientDataFunc dataFunc, void *userData)
{
	BMessenger messenger(NULL, serverLooper);
	BMessage message(APSERVER_MSG_CONNECT);
	BMessage reply;
	APClient *newClient;

	// Allocate a new client object
	newClient = new APClient(clientName);
	if (newClient == NULL)
		throw PMemoryException();

	// Tell the new client object about the data function
	newClient->SetDataFunction(dataFunc, userData);

	// Connect to the server
	message.AddPointer("ClientLooper", newClient);
	messenger.SendMessage(&message, &reply);

	// Check the reply for error
	if (reply.what == APSERVER_MSG_ERROR)
	{
		// An error occurred!
		delete newClient;
		return (NULL);
	}

	// Store the new client in the list
	clients.LockList();
	clients.AddTail(newClient);
	clients.UnlockList();

	// Start the BLooper
	newClient->Run();

	return (newClient);
}



/******************************************************************************/
/* DisconnectFromServer() disconnects the client from the server.             */
/*                                                                            */
/* Input:  "handle" is the handle you got from the ConnectToServer() function.*/
/******************************************************************************/
void APServerCommunication::DisconnectFromServer(void *handle)
{
	BMessenger messenger(NULL, serverLooper);
	BMessage message(APSERVER_MSG_DISCONNECT);
	int32 i, count;

	// Lock client list
	clients.LockList();

	// Find handle in client list
	count = clients.CountItems();
	for (i = 0; i < count; i++)
	{
		// Did we found the handle
		if (clients.GetItem(i) == handle)
		{
			// Yup, disconnect from the server
			message.AddPointer("ClientLooper", handle);
			messenger.SendMessage(&message, &message);

			// Check reply
			ASSERT(message.what == APSERVER_MSG_OK);

			// Remove the item from the list
			clients.RemoveItem(i);

			// Delete the handle
			((APClient *)handle)->Lock();
			((APClient *)handle)->Quit();
			break;
		}
	}

	// Unlock client list again
	clients.UnlockList();
}



/******************************************************************************/
/* SendCommand() sends the command given to the server. The reply from the    */
/*      server is returned.                                                   */
/*                                                                            */
/* Input:  "handle" is the handle you got from the ConnectToServer() function.*/
/*         "command" is the command to send.                                  */
/*                                                                            */
/* Output: The reply command from the server.                                 */
/******************************************************************************/
PString APServerCommunication::SendCommand(void *handle, PString command)
{
	BMessenger messenger(NULL, serverLooper);
	BMessage message(APSERVER_MSG_DATA);
	BMessage reply;
	const char *replyCmd;
	char *cmdStr;

	// Just add the looper and command to the message
	message.AddPointer("ClientLooper", handle);
	message.AddString("Command", (cmdStr = command.GetString()));
	command.FreeBuffer(cmdStr);

	// Send the command and wait for the reply
	messenger.SendMessage(&message, &reply);

	if (reply.what != APSERVER_MSG_OK)
	{
		// The server couldn't understand or run the command
		//
		// Find the error string
		if (reply.FindString("Result", &replyCmd) == B_OK)
			return (PString("ERR=") + replyCmd);

		return ("ERR=0");
	}

	// Extract the answer from the reply message
	if (reply.FindString("Result", &replyCmd) != B_OK)
		return ("");

	return (replyCmd);
}



/******************************************************************************/
/* AddArgument() will add the argument to the command given and return the    */
/*      new command.                                                          */
/*                                                                            */
/* Input:  "command" is the command to add an argument to.                    */
/*         "arg" is the argument to add.                                      */
/*                                                                            */
/* Output: The new command with the argument.                                 */
/******************************************************************************/
PString APServerCommunication::AddArgument(PString command, PString arg)
{
	PString newCommand;

	// First replace special characters
	arg.Replace("/", "//");
	arg.Replace("\"", "/\"");

	// Is this the first argument?
	if (command.Right(1) == "\"")
		newCommand = command + ",\"";
	else
		newCommand = command + "\"";

	// Add the argument
	newCommand += arg + "\"";

	// And return it
	return (newCommand);
}



/******************************************************************************/
/*                              Server functions                              */
/******************************************************************************/

/******************************************************************************/
/* SetServerLooper() sets the looper assigned to the server.                  */
/*                                                                            */
/* Input:  "looper" is a pointer to the servers BLooper object.               */
/******************************************************************************/
void APServerCommunication::SetServerLooper(BLooper *looper)
{
	serverLooper = looper;
}





/******************************************************************************/
/*                                                                            */
/*                               APClient class                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
APClient::APClient(PString clientName) : BLooper()
{
	PString name("Client looper: " + clientName);
	char *nameStr;

	SetName((nameStr = name.GetString()));
	name.FreeBuffer(nameStr);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
APClient::~APClient(void)
{
}



/******************************************************************************/
/* SetDataFunction() will remember the data function pointer given.           */
/*                                                                            */
/* Input:  "dataFunc" is a pointer to the data function.                      */
/*         "userData" is anything the caller wants.                           */
/******************************************************************************/
void APClient::SetDataFunction(ClientDataFunc dataFunc, void *userData)
{
	clientDataFunc = dataFunc;
	clientUserData = userData;
}



/******************************************************************************/
/* MessageReceived() is called for each message sent to the looper.           */
/*                                                                            */
/* Input:  "message" is a pointer to the message that has been sent.          */
/******************************************************************************/
void APClient::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		//
		// Data
		//
		case APSERVER_MSG_DATA:
		{
			const char *command;

			// Extract the command sent from the server
			if (message->FindString("Command", &command) == B_OK)
			{
				PString cmdTemp, cmd, temp;
				PList<PString> argList;
				int32 index;

				// Start to extract the command
				cmdTemp = command;
				index = cmdTemp.Find('=');
				if (index == -1)
					break;

				cmd = cmdTemp.Left(index);
				cmdTemp.Delete(0, index + 1);

				// Now take all the arguments
				while ((index = cmdTemp.Find("\",")) != -1)
				{
					// Extract a single argument
					temp = cmdTemp.Mid(1, index - 1);
					cmdTemp.Delete(0, index + 2);

					// Fix it
					temp.Replace("/\"", "\"");
					temp.Replace("//", "/");

					// Add the argument to the list
					argList.AddTail(temp);
				}

				// Did we have any arguments left?
				if (!cmdTemp.IsEmpty())
				{
					// Yup, fix it
					temp = cmdTemp.Mid(1, cmdTemp.GetLength() - 2);
					temp.Replace("/\"", "\"");
					temp.Replace("//", "/");

					// And add it to the list
					argList.AddTail(temp);
				}

				// Call the data function
				clientDataFunc(clientUserData, cmd, argList);
			}
			break;
		}

		//
		// Unknown message. Call the base class
		//
		default:
			BLooper::MessageReceived(message);
			break;
	}
}
