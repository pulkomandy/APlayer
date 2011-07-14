/******************************************************************************/
/* PSocket implementation file.                                               */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#define _BUILDING_POLYKIT_LIBRARY_

#include <arpa/inet.h>
#include <sys/time.h>

// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PSystem.h"
#include "PString.h"
#include "PSynchronize.h"
#include "PDirectory.h"
#include "PList.h"
#include "PSocket.h"


/******************************************************************************/
/* PSocket class                                                              */
/******************************************************************************/

/******************************************************************************/
/* Internal defines                                                           */
/******************************************************************************/
#define PSW_ACCEPT				0x0001
#define PSW_READ				0x0002
#define PSW_WRITE				0x0003



/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PSocket::PSocket(void)
{
	// Initialize member variables
	clone             = false;
	listenInitialized = false;
	connected         = false;
	socketClosed      = true;
	useSendTo         = false;
	timeoutValue      = 30000;
	useSocket         = -1;
	sockLen           = 0;
	connectedSocket   = -1;
}



/******************************************************************************/
/* Copy constructor                                                           */
/******************************************************************************/
PSocket::PSocket(const PSocket &source)
{
	CopyIt((PSocket &)source);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PSocket::~PSocket(void)
{
	// Make sure the socket is closed
	CloseConnection();
}



/******************************************************************************/
/* UseProtocol() initializes the class to use the protocol given.             */
/*                                                                            */
/* Input:  "protocol" is the protocol you want to use.                        */
/*         "address" is the address you want to use.                          */
/*         "type" is the type of the communication.                           */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
void PSocket::UseProtocol(PProtocol protocol, PString address, PCommunication type)
{
	// Close the socket if it's open
	CloseConnection();

	// Remember the arguments
	useProtocol     = protocol;
	useType         = type;
	useAddress      = address;

	// Initialize the protocol
	switch (protocol)
	{
		case pTCP_IP:
		{
			InitializeTCPIP(address);
			break;
		}

		default:
		{
			// Not supported
			ASSERT(false);
			throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
		}
	}
}



/******************************************************************************/
/* SetTimeout() will change the timeout value.                                */
/*                                                                            */
/* Input:  "timeout" is the new timeout value in milliseconds.                */
/******************************************************************************/
void PSocket::SetTimeout(uint32 timeout)
{
	timeoutValue = timeout;
}



/******************************************************************************/
/* ListenForConnection() waits on a connection from another machine.          */
/*                                                                            */
/* Except: PNetworkException, PEeventException.                               */
/******************************************************************************/
void PSocket::ListenForConnection(void)
{
	int error;

	// Clear the socket close flag
	socketClosed = false;

	// Do we need to do a bind and listen?
	if (!listenInitialized)
	{
		error = bind(useSocket, (const struct sockaddr *)&sockAddr, sockLen);
		if (error < 0)
			throw PNetworkException(PSystem::ConvertOSError(errno));
	}

	if (useType == pStream)
	{
		if (!listenInitialized)
		{
			// Now listen for a connection
			error = listen(useSocket, 32);
			if (error < 0)
				throw PNetworkException(PSystem::ConvertOSError(errno));

			listenInitialized = true;
		}

		// Wait for the connection
		EventWait(useSocket, PSW_ACCEPT, PSYNC_INFINITE);
	}
	else
	{
		// Datagram socket
		listenInitialized = true;

		// Wait for something to arrive
		EventWait(useSocket, PSW_READ, PSYNC_INFINITE);

		// Got it, now link the socket
		connectedSocket = useSocket;
	}

	// Got a listen socket
	connected = false;
}



/******************************************************************************/
/* CreateConnection() tries to connect to another machine.                    */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
void PSocket::CreateConnection(void)
{
	int error;

	// Clear the socket close flag
	socketClosed = false;

	// Try to connect with the socket
	error = connect(useSocket, (const struct sockaddr *)&sockAddr, sockLen);
	if (error < 0)
		throw PNetworkException(PSystem::ConvertOSError(errno));

	// We got a connection
	connected       = true;
	connectedSocket = useSocket;
}



/******************************************************************************/
/* CloseConnection() closes the connection and clean up.                      */
/******************************************************************************/
void PSocket::CloseConnection(void)
{
	if ((useType == pStream) && (connectedSocket != -1) && (!connected))
	{
		// Close the "accept" socket
		close(connectedSocket);
	}

	// Close the main socket
	if ((useSocket != -1) && !clone)
		close(useSocket);

	// Reset the member variables
	connectedSocket   = -1;
	useSocket         = -1;
	listenInitialized = false;
	connected         = false;
	useSendTo         = false;
}



/******************************************************************************/
/* ReceiveData() will receive a block of data.                                */
/*                                                                            */
/* Input:  "data" is a pointer to a buffer to receive the data.               */
/*         "length" is a variable the function will store the length of the   */
/*         data received. Store the length of the buffer here before calling. */
/*                                                                            */
/* Except: PNetworkException, PEeventException.                               */
/******************************************************************************/
void PSocket::ReceiveData(void *data, uint32 *length)
{
	int error;

	if ((useType == pStream) || connected)
	{
		// Stream or connected socket
		for (;;)
		{
			// First wait for a read signal
			EventWait(connectedSocket, PSW_READ, timeoutValue);

			// Receive the data
			error = recv(connectedSocket, data, *length, 0);
			if (error == 0)
				socketClosed = true;
			else
			{
				if (error < 0)
				{
					if (errno != EWOULDBLOCK)
						throw PNetworkException(PSystem::ConvertOSError(errno));
				}
				else
					break;		// Stop the loop, we got the data
			}
		}

		// Store the number of bytes read
		*length = error;
	}
	else
	{
		// Datagram socket
		int clientSockLen;

		for (;;)
		{
			// First wait until some data arrive
			EventWait(connectedSocket, PSW_READ, timeoutValue);

			// Receive the data
			clientSockLen = sizeof(clientSockAddr);
			// error = recvfrom(connectedSocket, data, *length, 0, (struct sockaddr *)&clientSockAddr, &clientSockLen);
			error = recv(connectedSocket, data, *length, 0);
			if (error == 0)
				socketClosed = true;
			else
			{
				if (error < 0)
				{
					if (errno != EWOULDBLOCK)
						throw PNetworkException(PSystem::ConvertOSError(errno));
				}
				else
					break;		// Stop the loop, we got the data
			}
		}

		// Store the length
		*length = error;

		// If we want to send data back, use sendto
		useSendTo = true;
	}
}



/******************************************************************************/
/* SendData() will send a block of data to the receiver.                      */
/*                                                                            */
/* Input:  "data" is a pointer to the data you want to send.                  */
/*         "length" is the length of the data.                                */
/*                                                                            */
/* Except: PNetworkException, PEeventException.                               */
/******************************************************************************/
void PSocket::SendData(const void *data, uint32 length)
{
	int error;

	if (useSendTo)
	{
		// Send the data directly to the other side
		error = sendto(connectedSocket, data, length, 0, (struct sockaddr *)&clientSockAddr, sizeof(clientSockAddr));
		if (error < 0)
			throw PNetworkException(PSystem::ConvertOSError(errno));
	}
	else
	{
		for (;;)
		{
			// First wait for a write signal
			EventWait(connectedSocket, PSW_WRITE, timeoutValue);

			// Send the data
			error = send(connectedSocket, data, length, 0);
			if (error == 0)
				socketClosed = true;
			else
			{
				if (error < 0)
				{
					if (errno != EWOULDBLOCK)
						throw PNetworkException(PSystem::ConvertOSError(errno));
				}
				else
					break;
			}
		}
	}
}



/******************************************************************************/
/* AddWaitObjects() will add x number of objects to wait for. The class will  */
/*      in all blocking states wait for these objects too.                    */
/*                                                                            */
/* Input:  "..." is the objects you want to add. End with NULL.               */
/******************************************************************************/
void PSocket::AddWaitObjects(PSync *first, ...)
{
	va_list objectList;
	PSync *nextObject = first;

	// Find the first argument
	va_start(objectList, first);

	// Add all objects to the array
	while (nextObject != NULL)
	{
		// Add the object to the array
		waitObjects.AddTail(nextObject);

		// Get next argument
		nextObject = va_arg(objectList, PSync *);
	}

	// Reset variable arguments
	va_end(objectList);
}



/******************************************************************************/
/* RemoveWaitObjects() will remove x number of objects from the wait list.    */
/*                                                                            */
/* Input:  "..." is the objects you want to remove. End with NULL.            */
/******************************************************************************/
void PSocket::RemoveWaitObjects(PSync *first, ...)
{
	va_list objectList;
	PSync *nextObject = first;

	// Find the first argument
	va_start(objectList, first);

	while (nextObject != NULL)
	{
		// Remove the object from the list
		waitObjects.FindAndRemoveItem(nextObject);

		// Get next argument
		nextObject = va_arg(objectList, PSync *);
	}

	// Reset variable arguments
	va_end(objectList);
}



/******************************************************************************/
/* SetWaitObjects() will remove all the current wait objects and then copy    */
/*      the objects from the source given.                                    */
/*                                                                            */
/* Input:  "source" is where to copy the new wait objects from.               */
/******************************************************************************/
void PSocket::SetWaitObjects(const PSocket *source)
{
	int32 i, count;

	// Remove all the current objects
	waitObjects.MakeEmpty();

	// Now copy the ones from the source socket
	count = source->waitObjects.CountItems();
	for (i = 0; i < count; i++)
		waitObjects.AddTail(source->waitObjects.GetItem(i));
}



/******************************************************************************/
/* GetObjectAddress() will return the address of the current object.          */
/*                                                                            */
/* Input:  "withPort" indicates if you want the port returned or not.         */
/*                                                                            */
/* Output: The address of the object.                                         */
/******************************************************************************/
PString PSocket::GetObjectAddress(bool withPort)
{
	PString addr;

	switch (useProtocol)
	{
		//
		// TCP/IP
		//
		case pTCP_IP:
		{
			int32 index;

			addr = useAddress;

			if (!withPort)
			{
				// Find the split colon between the address and port
				index = useAddress.Find(':');
				if (index >= 0)
					addr = addr.Left(index);
			}
			break;
		}

		default:
		{
			// Not supported
			ASSERT(false);
			throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
		}
	}

	return (addr);
}



/******************************************************************************/
/* GetAddressFromName() will return the address of the name.                  */
/*                                                                            */
/* Input:  "name" is the name you want to convert.                            */
/*                                                                            */
/* Output: The address of the name.                                           */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
PString PSocket::GetAddressFromName(PString name)
{
	PString addr;

	switch (useProtocol)
	{
		//
		// TCP/IP
		//
		case pTCP_IP:
		{
			struct hostent *hostInfo;
			struct in_addr inAddr;
			unsigned int ipAddr;
			char *nameStr;

			if (name == "*")
				ipAddr = INADDR_ANY;
			else
			{
				// Check for dotted IP address string
				ipAddr = inet_addr((nameStr = name.GetString()));

				// If not an address, then try to resolve it as a host name
				if ((ipAddr == INADDR_BROADCAST) && (name != "255.255.255.255"))
				{
					hostInfo = gethostbyname(nameStr);
					if (hostInfo == NULL)
					{
						name.FreeBuffer(nameStr);
						throw PNetworkException(PSystem::ConvertOSError(errno));
					}

					// Retrieve the IP address
					ipAddr = *((unsigned int *)hostInfo->h_addr);
				}

				name.FreeBuffer(nameStr);
			}

			// Convert the IP address to a string
			inAddr.s_addr = ipAddr;
			addr = inet_ntoa(inAddr);
			break;
		}

		default:
		{
			// Not supported
			ASSERT(false);
			throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
		}
	}

	return (addr);
}



/******************************************************************************/
/* GetNameFromAddress() will return the name from an address.                 */
/*                                                                            */
/* Input:  "address" is the address you want to convert.                      */
/*                                                                            */
/* Output: The name.                                                          */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
PString PSocket::GetNameFromAddress(PString address)
{
	PString name;

	switch (useProtocol)
	{
		//
		// TCP/IP
		//
		case pTCP_IP:
		{
			struct hostent *hostInfo;
			unsigned int ipAddr;
			char *adrStr;

			// Convert the address to a number
			ipAddr = inet_addr((adrStr = address.GetString()));
			address.FreeBuffer(adrStr);

			// Find the name
			hostInfo = gethostbyaddr((const char *)&ipAddr, sizeof(ipAddr), AF_INET);
			if (hostInfo == NULL)
				throw PNetworkException(PSystem::ConvertOSError(errno));

			// Return the name from the hostent structure
			name = hostInfo->h_name;
			break;
		}

		default:
		{
			// Not supported
			ASSERT(false);
			throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
		}
	}

	return (name);
}



/******************************************************************************/
/* GetConnectedAddress() will return the address of the machine connected to  */
/*      the socket.                                                           */
/*                                                                            */
/* Input:  "withPort" indicates if you want the port number returned with     */
/*         the address.                                                       */
/*                                                                            */
/* Output: The connected address.                                             */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
PString PSocket::GetConnectedAddress(bool withPort)
{
	SocketAddr peerAddr;
	unsigned int peerLen = sizeof(peerAddr);
	PString addr;
	int error;

	if ((useType == pStream) || connected)
	{
		// Get the peer address
		error = getpeername(connectedSocket, (struct sockaddr *)&peerAddr, &peerLen);
		if (error < 0)
			throw PNetworkException(PSystem::ConvertOSError(errno));
	}
	else
		peerAddr = clientSockAddr;

	// Convert the address to ascii
	switch (useProtocol)
	{
		//
		// TCP/IP
		//
		case pTCP_IP:
		{
			addr = inet_ntoa(peerAddr.sockIn.sin_addr);

			if (withPort)
				addr += ":" + PString::CreateUNumber(peerAddr.sockIn.sin_port);

			break;
		}

		default:
		{
			// Not supported
			ASSERT(false);
			throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
		}
	}

	return (addr);
}



/******************************************************************************/
/* operator = (PSocket &) will copy the socket given.                         */
/*                                                                            */
/* Input:  "source" is the socket to copy.                                    */
/*                                                                            */
/* Output: A reference to the new object.                                     */
/******************************************************************************/
PSocket & PSocket::operator = (const PSocket &source)
{
	CopyIt((PSocket &)source);

	return (*this);
}



/******************************************************************************/
/* CopyIt() will copy the socket given into the current object.               */
/*                                                                            */
/* Input:  "source" is the socket to copy.                                    */
/******************************************************************************/
void PSocket::CopyIt(PSocket &source)
{
	// Cleanup the object if it has been used before
	CloseConnection();

	// Copy all the variables
	clone             = true;
	listenInitialized = source.listenInitialized;
	connected         = source.connected;
	socketClosed      = source.socketClosed;
	useSendTo         = source.useSendTo;

	timeoutValue      = source.timeoutValue;

	useProtocol       = source.useProtocol;
	useType           = source.useType;
	useAddress        = source.useAddress;
	usePort           = source.usePort;
	useSocket         = source.useSocket;

	sockLen           = source.sockLen;
	sockAddr          = source.sockAddr;

	connectedSocket   = source.connectedSocket;
	clientSockAddr    = source.clientSockAddr;

	waitObjects       = source.waitObjects;

	// It's not a good thing, but we need to tell the
	// source object not to close the connected socket.
	// It's the new objects responsibility now
	source.connectedSocket = -1;
}



/******************************************************************************/
/* InitializeTCPIP() initialize the class to use the TCP/IP protocol.         */
/*                                                                            */
/* Input:  "address" is the address and port you want to use.                 */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
void PSocket::InitializeTCPIP(PString address)
{
	int sockType;
	PString addr;
	char *addrStr;
	int32 index;

	// Convert the communication type
	switch (useType)
	{
		case pStream:
		{
			sockType = SOCK_STREAM;
			break;
		}

		case pDatagram:
		{
			sockType = SOCK_DGRAM;
			break;
		}

		default:
		{
			// Default to stream sockets
			sockType = SOCK_STREAM;
			break;
		}
	}

	// Create the socket
	useSocket = socket(AF_INET, sockType, 0);
	if (useSocket < 0)
		throw PNetworkException(PSystem::ConvertOSError(errno));

	// Find the split colon between the address and port
	index = address.Find(':');
	if (index < 0)
	{
		close(useSocket);
		useSocket = -1;

		throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
	}

	// Find the IP address and port number
	addr    = GetAddressFromName(address.Left(index));
	usePort = address.GetUNumber(index + 1);

	// Initialize the SOCKADDR structure
	memset(&sockAddr, 0, sizeof(sockAddr));
	sockAddr.sockIn.sin_family      = AF_INET;
	sockAddr.sockIn.sin_port        = htons(usePort);
	sockAddr.sockIn.sin_addr.s_addr = inet_addr((addrStr = addr.GetString()));
	addr.FreeBuffer(addrStr);

	// Set the length of the socket address
	sockLen = sizeof(sockAddr.sockIn);
}



/******************************************************************************/
/* EventWait() will wait to the socket has trigged the event or an user event */
/*      has occured.                                                          */
/*                                                                            */
/* Input:  "waitSocket" is the socket to wait on.                             */
/*         "waitFlag" is what to wait on.                                     */
/*         "timeout" is how long time you want to wait.                       */
/*                                                                            */
/* Except: PNetworkException, PEventException.                                */
/******************************************************************************/
void PSocket::EventWait(int waitSocket, uint32 waitFlag, uint32 timeout)
{
	PSync **waitArray;
	int32 i, waitCount;
	uint32 toWait;
	int error;

	// If the socket is closed, notify the caller
	if (socketClosed)
		throw PNetworkException(P_NETWORK_ERR_SOCKET_CLOSED);

	// Create the array with the wait objects
	waitCount = waitObjects.CountItems();
	waitArray = new PSync *[waitCount];
	if (waitArray == NULL)
		throw PMemoryException();

	// Copy the objects
	for (i = 0; i < waitCount; i++)
		waitArray[i] = waitObjects.GetItem(i);

	try
	{
		switch (waitFlag)
		{
			//
			// Accept
			//
			case PSW_ACCEPT:
			{
				unsigned int clientSockLen;
				int flag;

				// Set the socket in non-block mode
				flag  = true;
				error = setsockopt(waitSocket, SOL_SOCKET, SO_NONBLOCK, &flag, sizeof(int));
				if (error < 0)
					throw PNetworkException(PSystem::ConvertOSError(errno));

				// Did we get an accept?
				for (;;)
				{
					clientSockLen = sizeof(clientSockAddr);
					error = accept(waitSocket, (struct sockaddr *)&clientSockAddr, &clientSockLen);
					if (error >= 0)
					{
						// Got a connection
						connectedSocket = error;
						break;
					}

					if (errno != EWOULDBLOCK)
					{
						flag = false;
						setsockopt(waitSocket, SOL_SOCKET, SO_NONBLOCK, &flag, sizeof(int));

						throw PNetworkException(PSystem::ConvertOSError(errno));
					}

					// No clients connected yet, so we wait for the user events
					if (timeout == PSYNC_INFINITE)
					{
						i = MultipleObjectsWait(waitArray, waitCount, false, 500);
						if ((i != pSyncError) && (i != pSyncTimeout))
							throw PEventException(waitArray[i]);
					}
					else
					{
						toWait = min(500, timeout);
						if (toWait == 0)
							throw PNetworkException(P_GEN_ERR_TIMEOUT);

						i = MultipleObjectsWait(waitArray, waitCount, false, toWait);
						if ((i != pSyncError) && (i != pSyncTimeout))
							throw PEventException(waitArray[i]);

						timeout -= toWait;
					}
				}

				// At this point, we got a connection. Set the socket to block mode again
				flag  = false;
				error = setsockopt(waitSocket, SOL_SOCKET, SO_NONBLOCK, &flag, sizeof(int));
				if (error < 0)
					throw PNetworkException(PSystem::ConvertOSError(errno));

				break;
			}

			//
			// Read
			//
			case PSW_READ:
			{
				struct timeval tv;
				struct fd_set fds;

				for (;;)
				{
					// Check for user events
					i = MultipleObjectsWait(waitArray, waitCount, false, 0);
					if ((i != pSyncError) && (i != pSyncTimeout))
						throw PEventException(waitArray[i]);

					// Initialize the time to wait
					toWait     = min(500, timeout);
					tv.tv_sec  = 0;
					tv.tv_usec = toWait * 1000;

					// Initialize the wait structure
					FD_ZERO(&fds);
					FD_SET(waitSocket, &fds);

					// Wait for some data to read
					error = select(32, &fds, NULL, NULL, &tv);
					if (error < 0)
						throw PNetworkException(PSystem::ConvertOSError(errno));

					if (error > 0)
						break;		// Got some data to read

					// Subtract the time occured from the timeout
					if (timeout != PSYNC_INFINITE)
					{
						timeout -= toWait;
						if (timeout == 0)
							throw PNetworkException(P_GEN_ERR_TIMEOUT);
					}
				}
				break;
			}

			//
			// Write
			//
			case PSW_WRITE:
			{
				// No waiting here, just check the user events
				i = MultipleObjectsWait(waitArray, waitCount, false, 0);
				if ((i != pSyncError) && (i != pSyncTimeout))
					throw PEventException(waitArray[i]);

				break;
			}

			//
			// All others
			//
			default:
			{
				ASSERT(false);
				break;
			}
		}
	}
	catch(...)
	{
		// Delete the array again
		delete[] waitArray;
		throw;
	}

	// Delete the array again
	delete[] waitArray;
}




/******************************************************************************/
/* PLineSocket class                                                          */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PLineSocket::PLineSocket(void)
{
	// Initialize member variables
	eolCode       = "\r\n";
	maxBufferSize = 1 * 1024 * 1024;		// 1 Mb.

	buffer        = NULL;
	bufferLen     = 0;
	bufferFilled  = 0;
}



/******************************************************************************/
/* Copy constructor                                                           */
/******************************************************************************/
PLineSocket::PLineSocket(const PLineSocket &source)
{
	buffer = NULL;
	CopyIt(source);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PLineSocket::~PLineSocket(void)
{
	// Delete the buffer
	delete[] buffer;
}



/******************************************************************************/
/* SetEOLCode() will change the EOL code. This is the string that is appended */
/*      after each line you send.                                             */
/*                                                                            */
/* Input:  "eol" is the new EOL code.                                         */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
void PLineSocket::SetEOLCode(PString eol)
{
	// The EOL may not be empty
	if (eol.IsEmpty())
		throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);

	eolCode = eol;
}



/******************************************************************************/
/* SetMaxLineLength() will set the maximum length a line much be before a     */
/*      failure. 0 means no limit.                                            */
/*                                                                            */
/* Input:  "length" is the maximum length in bytes.                           */
/******************************************************************************/
void PLineSocket::SetMaxLineLength(uint32 length)
{
	maxBufferSize = length;
}



/******************************************************************************/
/* GetEOLCode() returns the EOL code used.                                    */
/*                                                                            */
/* Output: The EOL code.                                                      */
/******************************************************************************/
PString PLineSocket::GetEOLCode(void) const
{
	return (eolCode);
}



/******************************************************************************/
/* GetMaxLineLength() returns the maximum number of bytes a line can be.      */
/*                                                                            */
/* Output: The maximum line length.                                           */
/******************************************************************************/
int32 PLineSocket::GetMaxLineLength(void) const
{
	return (maxBufferSize);
}



/******************************************************************************/
/* ReceiveData() will receive a block of data. If it can't read all the data  */
/*      it will block.                                                        */
/*                                                                            */
/* Input:  "data" is a pointer to a buffer to receive the data.               */
/*         "length" is a variable the function will store the length of the   */
/*         data received. Store the length of the buffer here before calling. */
/*                                                                            */
/* Except: PNetworkException, PEeventException.                               */
/******************************************************************************/
void PLineSocket::ReceiveData(void *data, uint32 *length)
{
	uint8 *destBuf = (uint8 *)data;
	int32 toCopy;
	uint32 read = 0;

	// Start to check if there is any data in the line buffer
	if (bufferFilled != 0)
	{
		// There is, now copy as many bytes as possible
		toCopy = min(*length, (uint32)bufferFilled);
		memcpy(destBuf, buffer, toCopy);

		// Remove the data copied from the buffer
		memcpy(buffer, buffer + toCopy, bufferFilled - toCopy);

		// Adjust the buffer counters
		bufferFilled -= toCopy;
		read         += toCopy;
	}

	if (read != *length)
	{
		// Read the rest from the socket
		*length -= read;
		PSocket::ReceiveData(destBuf + read, length);
		*length += read;
	}
}



/******************************************************************************/
/* ReceiveLine() will receive a string.                                       */
/*                                                                            */
/* Input:  "characterSet" is the character set to decode the string in.       */
/*                                                                            */
/* Except: PNetworkException, PEventException.                                */
/******************************************************************************/
PString PLineSocket::ReceiveLine(PCharacterSet *characterSet)
{
	PString line;
	PCharacterSet *charSet;
	char *eolBuf;
	int32 eolLen;
	uint8 *tempBuf;
	int32 tempBufLen, receivedLen;
	int32 i, j, startSearch;
	bool gotLine = false;

	// This function do only work on stream sockets
	ASSERT(useType == pStream);

	// Use default character set if none is specified
	if (characterSet == NULL)
		charSet = CreateHostCharacterSet();
	else
		charSet = characterSet;

	// Get the EOL code buffer in the character set to use
	eolBuf = eolCode.GetString(charSet, &eolLen);

	// Allocate temp buffer
	tempBufLen = 256;
	tempBuf    = new uint8[tempBufLen];
	if (tempBuf == NULL)
		throw PMemoryException();

	try
	{
		// Initialize the start index of the search
		startSearch = 0;

		do
		{
			// Search for the EOL code
			for (i = startSearch, j = 0; i < bufferFilled; i++)
			{
				if (buffer[i] == eolBuf[j])
				{
					j++;
					if (j == eolLen)
					{
						char *lineBuf;
						int32 size;

						// We found the EOL mark, so clip out the string
						size = i - j + 1;

						lineBuf = new char[size + 1];

						memcpy(lineBuf, buffer, size);
						lineBuf[size] = 0x00;
						line.SetString(lineBuf, charSet);

						delete[] lineBuf;

						// Remove the line from the buffer
						memcpy(buffer, buffer + i + 1, bufferFilled - (i + 1));
						bufferFilled -= (i + 1);

						gotLine = true;
						break;
					}
				}
				else
				{
					// Begin at the start of the EOL code, because it wasn't
					// the whole code we found
					j = 0;
				}
			}

			// Remember the new start index
			startSearch = i - j;

			if (!gotLine)
			{
				// Fill out a buffer with some data
				receivedLen = tempBufLen;
				PSocket::ReceiveData(tempBuf, (uint32 *)&receivedLen);

				// Check to see if the read data can be filled in the buffer
				if (receivedLen > (bufferLen - bufferFilled))
				{
					// Allocate a bigger buffer
					uint8 *newBuf;

					// First check if we has an overflow
					if ((maxBufferSize != 0) && ((bufferLen + receivedLen) > maxBufferSize))
						throw PNetworkException(P_GEN_ERR_INSUFFICIENT_BUFFER);

					newBuf = new uint8[bufferLen + receivedLen];
					if (newBuf == NULL)
						throw PMemoryException();

					// Okay, copy the old buffer to the new buffer
					if (buffer != NULL)
						memcpy(newBuf, buffer, bufferFilled);

					// Delete the old buffer and use the new one
					delete[] buffer;
					buffer = newBuf;
					bufferLen += receivedLen;
				}

				// Copy the received data to the buffer
				memcpy(buffer + bufferFilled, tempBuf, receivedLen);
				bufferFilled += receivedLen;
			}
		}
		while (!gotLine);
	}
	catch(...)
	{
		// Cleanup
		delete[] tempBuf;
		eolCode.FreeBuffer(eolBuf);

		if (characterSet == NULL)
			delete charSet;

		throw;
	}

	// Delete the buffers
	delete[] tempBuf;
	eolCode.FreeBuffer(eolBuf);

	// Delete the character set if the default is used
	if (characterSet == NULL)
		delete charSet;

	return (line);
}



/******************************************************************************/
/* SendLine() will send a string.                                             */
/*                                                                            */
/* Input:  "line" is the string to send.                                      */
/*         "characterSet" is the character set to send the string in.         */
/*                                                                            */
/* Except: PNetworkException, PEventException.                                */
/******************************************************************************/
void PLineSocket::SendLine(PString line, PCharacterSet *characterSet)
{
	char *buffer;
	int32 bufLen;

	// Add the EOL code
	line += eolCode;

	// Switch to the character set to use
	if (characterSet != NULL)
		line.SwitchCharacterSet(characterSet);

	// Get the buffer and length
	buffer = line.GetString(&bufLen);

	try
	{
		// Send the line
		SendData(buffer, bufLen);
	}
	catch(...)
	{
		line.FreeBuffer(buffer);
		throw;
	}

	// Free the buffer
	line.FreeBuffer(buffer);
}



/******************************************************************************/
/* operator = (PLineSocket &) will copy the socket given.                     */
/*                                                                            */
/* Input:  "source" is the socket to copy.                                    */
/*                                                                            */
/* Output: A reference to the new object.                                     */
/******************************************************************************/
PLineSocket & PLineSocket::operator = (const PLineSocket &source)
{
	CopyIt(source);

	return (*this);
}



/******************************************************************************/
/* CopyIt() will copy the socket given into the current object.               */
/*                                                                            */
/* Input:  "source" is the socket to copy.                                    */
/******************************************************************************/
void PLineSocket::CopyIt(const PLineSocket &source)
{
	// Call the base class
	*(PSocket *)this = source;

	// Cleanup the object if it has been used before
	delete[] buffer;

	// Copy the member variables
	eolCode       = source.eolCode;
	maxBufferSize = source.maxBufferSize;

	bufferLen     = source.bufferLen;
	bufferFilled  = source.bufferFilled;

	// Allocate a new buffer and copy the contents to it
	if (source.buffer != NULL)
	{
		buffer = new uint8[bufferLen];
		if (buffer == NULL)
			throw PMemoryException();

		memcpy(buffer, source.buffer, bufferLen);
	}
	else
		buffer = NULL;
}




/******************************************************************************/
/* PProxySocket class                                                         */
/******************************************************************************/

/******************************************************************************/
/* Constructor                                                                */
/******************************************************************************/
PProxySocket::PProxySocket(void)
{
	// Initialize member variables
	proxyEnabled = true;
}



/******************************************************************************/
/* Copy constructor                                                           */
/******************************************************************************/
PProxySocket::PProxySocket(const PProxySocket &source)
{
	CopyIt(source);
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
PProxySocket::~PProxySocket(void)
{
}



/******************************************************************************/
/* EnableProxy() enable or disable the proxy usage.                           */
/*                                                                            */
/* Input:  "enable" indicate if you want to enable or disable the proxy.      */
/******************************************************************************/
void PProxySocket::EnableProxy(bool enable)
{
	proxyEnabled = enable;
}



/******************************************************************************/
/* SetProxyServer() sets the proxy server address and type.                   */
/*                                                                            */
/* Input:  "type" is the type of the proxy server.                            */
/*         "address" is the address to the proxy server.                      */
/******************************************************************************/
void PProxySocket::SetProxyServer(PProxyType type, PString address)
{
	// Remember the arguments
	proxyType    = type;
	proxyAddress = address;
}



/******************************************************************************/
/* IsEnable() returns the enable state.                                       */
/*                                                                            */
/* Output: True if proxy is enabled, false if not.                            */
/******************************************************************************/
bool PProxySocket::IsEnabled(void) const
{
	return (proxyEnabled);
}



/******************************************************************************/
/* GetProxyType() returns the type of the proxy protocol used.                */
/*                                                                            */
/* Output: The proxy protocol in use.                                         */
/******************************************************************************/
PProxySocket::PProxyType PProxySocket::GetProxyType(void) const
{
	return (proxyType);
}



/******************************************************************************/
/* GetProxyAddress() returns the address to the proxy server.                 */
/*                                                                            */
/* Output: The proxy server address.                                          */
/******************************************************************************/
PString PProxySocket::GetProxyAddress(void) const
{
	return (proxyAddress);
}



/******************************************************************************/
/* CreateConnection() tries to connect to another machine using the proxy     */
/*      server.                                                               */
/*                                                                            */
/* Except: PNetworkException.                                                 */
/******************************************************************************/
void PProxySocket::CreateConnection(void)
{
	if (proxyEnabled)
	{
		// Start to swap the sockets
		connectionSocket = *this;

		// Initialize the proxy socket
		UseProtocol(pTCP_IP, proxyAddress);

		// Try to connect to the proxy server
		PLineSocket::CreateConnection();

		// Tell the proxy server what it should connect to
		switch (proxyType)
		{
			//
			// HTTP
			//
			case pHTTP:
			{
				PString addr, line;
				PCharSet_UTF8 charSet;
				bool valid = false;

				addr = connectionSocket.GetObjectAddress(true);
				SendLine("CONNECT " + addr + " HTTP/1.0", &charSet);
				SendLine("", &charSet);

				// Now check to see if we got a valid connection
				do
				{
					line = ReceiveLine(&charSet);

					if (line.Left(13) == "HTTP/1.0 200 ")
						valid = true;
				}
				while (!line.IsEmpty());

				if (!valid)
					throw PNetworkException(P_NETWORK_ERR_CONNECTION_REJECTED);

				break;
			}

			// Unknown proxy server type
			default:
			{
				ASSERT(false);
				throw PNetworkException(P_GEN_ERR_BAD_ARGUMENT);
			}
		}
	}
	else
	{
		// Proxy server disabled, so just make a normal connection
		PLineSocket::CreateConnection();
	}
}



/******************************************************************************/
/* CloseConnection() closes the connection and clean up.                      */
/******************************************************************************/
void PProxySocket::CloseConnection(void)
{
	// First close the proxy server connection
	PLineSocket::CloseConnection();

	// Now close the connection socket.
	// In fact, the socket isn't bound to anything,
	// so it will only close the socket
	connectionSocket.CloseConnection();
}



/******************************************************************************/
/* operator = (PProxySocket &) will copy the socket given.                    */
/*                                                                            */
/* Input:  "source" is the socket to copy.                                    */
/*                                                                            */
/* Output: A reference to the new object.                                     */
/******************************************************************************/
PProxySocket & PProxySocket::operator = (const PProxySocket &source)
{
	CopyIt(source);

	return (*this);
}



/******************************************************************************/
/* CopyIt() will copy the socket given into the current object.               */
/*                                                                            */
/* Input:  "source" is the socket to copy.                                    */
/******************************************************************************/
void PProxySocket::CopyIt(const PProxySocket &source)
{
	// Call the base class
	*(PLineSocket *)this = source;

	// Copy the member variables
	proxyEnabled     = source.proxyEnabled;
	proxyType        = source.proxyType;
	proxyAddress     = source.proxyAddress;

	connectionSocket = source.connectionSocket;
}
