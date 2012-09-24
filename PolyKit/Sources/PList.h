/******************************************************************************/
/* PList header file.                                                         */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PList_h
#define __PList_h

// PolyKit headers
#include "POS.h"
#include "ImportExport.h"
#include "PException.h"


/******************************************************************************/
/* PList class                                                                */
/******************************************************************************/
template<class TYPE>
class PList
{
public:
	// Constructor and destructor
	PList(int32 blockSize = 10);
	~PList(void);		// Do not make virtual

	// Insertion of items
	void AddHead(TYPE item);
	int32 AddTail(TYPE item);
	void InsertItem(TYPE item, int32 index);
	void InsertList(const PList<TYPE> &list, int32 index = -1);

	// Removing of items
	TYPE GetAndRemoveItem(int32 index);
	int32 FindAndRemoveItem(TYPE item);
	void RemoveItem(int32 index);
	void RemoveItems(int32 index, int32 count);

	// Getting and setting items
	TYPE GetHead(void) const;
	TYPE GetTail(void) const;
	TYPE GetItem(int32 index) const;
	void SetItem(TYPE item, int32 index);
	TYPE GetAndSetItem(TYPE item, int32 index);
	TYPE &GetElement(int32 index) const;

	// Searching
	bool HasItem(TYPE item, int32 startPos = 0) const;
	int32 GetItemIndex(TYPE item, int32 startPos = 0) const;

	// List counts
	int32 CountItems(void) const;
	bool IsEmpty(void) const;
	void MakeEmpty(void);

	// Assignment
	const PList<TYPE> & operator = (const PList<TYPE> &newList);

protected:
	typedef struct PNode
	{
		PNode *nextNode;
		PNode *prevNode;
		TYPE *data;
		int32 dataCount;
	} PNode;

	typedef struct PListOptimize
	{
		// These variables are used to optimize all the get functions
		PNode *lastUsedNode;
		int32 lastUsedNodeIndex;
	} PListOptimize;

	PNode *AllocateNode(void) const;
	PNode *FindNode(int32 index, int32 *pos) const;
	void InsertNewItem(TYPE item, PNode *node, int32 index);
	TYPE RemoveSomeItems(int32 index, int32 count);

	PNode *firstNode;
	PNode *lastNode;
	int32 addSize;
	int32 itemCount;

	PListOptimize *optimize;
};



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "blockSize" is the number of items to allocate at once.            */
/******************************************************************************/
template<class TYPE>
PList<TYPE>::PList(int32 blockSize)
{
	// Initialize member variables
	firstNode = NULL;
	lastNode  = NULL;
	addSize   = blockSize;
	itemCount = 0;

	// Initialize optimization variables
	optimize = new PListOptimize;
	if (optimize == NULL)
		throw PMemoryException();

	optimize->lastUsedNode      = NULL;
	optimize->lastUsedNodeIndex = 0;
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
template<class TYPE>
PList<TYPE>::~PList(void)
{
	MakeEmpty();

	delete optimize;
}



/******************************************************************************/
/* AddHead() will add a new item at the head of the list.                     */
/*                                                                            */
/* Input:  "item" is the new item to add.                                     */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::AddHead(TYPE item)
{
	if (firstNode == NULL)
	{
		PNode *workNode;

		// First item in the list we are about to add
		workNode = AllocateNode();
		workNode->data[0] = item;

		// Link the new node to the list
		firstNode = workNode;
		lastNode  = workNode;
		itemCount = 1;
	}
	else
	{
		// Insert the new item at the first position in the node
		InsertNewItem(item, firstNode, 0);
	}
}



/******************************************************************************/
/* AddTail() will add a new item at the tail of the list.                     */
/*                                                                            */
/* Input:  "item" is the new item to add.                                     */
/*                                                                            */
/* Output: The index the item was added at.                                   */
/******************************************************************************/
template<class TYPE>
int32 PList<TYPE>::AddTail(TYPE item)
{
	PNode *workNode;

	if (lastNode == NULL)
	{
		// First item in the list we are about to add
		workNode = AllocateNode();
		workNode->data[0] = item;

		// Link the new node to the list
		firstNode = workNode;
		lastNode  = workNode;
	}
	else
	{
		// Find an empty space at the tail
		if (lastNode->dataCount < addSize)
		{
			// There are some space in the last node
			lastNode->data[lastNode->dataCount] = item;
			lastNode->dataCount++;
		}
		else
		{
			// We need to allocate a new node
			workNode = AllocateNode();
			workNode->data[0] = item;

			// Link the new node to the list
			lastNode->nextNode = workNode;
			workNode->prevNode = lastNode;
			lastNode           = workNode;
		}

		// Reset the get state variable
		optimize->lastUsedNode = NULL;
	}

	// Increment the number of items
	itemCount++;

	return (itemCount - 1);
}



/******************************************************************************/
/* InsertItem() will add a new item at the index given in the list.           */
/*                                                                            */
/* Input:  "item" is the new item to add.                                     */
/*         "index" is the position in the list to insert the item.            */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::InsertItem(TYPE item, int32 index)
{
	PNode *insertNode;
	int32 pos;

	// First check for out of range
	if ((index < 0) || (index > CountItems()))
		throw PBoundsException(P_ERR_ANY, index);

	// Add at the head of the list?
	if (index == 0)
	{
		AddHead(item);
		return;
	}

	// Add at the tail of the list?
	if (index == CountItems())
	{
		AddTail(item);
		return;
	}

	// Find the node to insert the item in
	insertNode = FindNode(index, &pos);
	InsertNewItem(item, insertNode, index - pos);
}



/******************************************************************************/
/* InsertList (PList &) will insert the list into the current list at the     */
/*      given position.                                                       */
/*                                                                            */
/* Input:  "list" is the list to copy.                                        */
/*         "index" is the index to insert the list.                           */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::InsertList(const PList<TYPE> &list, int32 index)
{
	PNode *curNode;
	int32 i;

	// Copy the list
	curNode = list.firstNode;
	while (curNode != NULL)
	{
		if (index == -1)
		{
			for (i = 0; i < curNode->dataCount; i++)
				AddTail(curNode->data[i]);
		}
		else
		{
			for (i = 0; i < curNode->dataCount; i++)
				InsertItem(curNode->data[i], index++);
		}

		curNode = curNode->nextNode;
	}

	// Reset the get state variable
	optimize->lastUsedNode = NULL;
}



/******************************************************************************/
/* GetAndRemoveItem() will remove one item from the list and return it.       */
/*                                                                            */
/* Input:  "index" is the index of the item to remove.                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
inline TYPE PList<TYPE>::GetAndRemoveItem(int32 index)
{
	return (RemoveSomeItems(index, 1));
}



/******************************************************************************/
/* FindAndRemoveItem() will remove one item from the list.                    */
/*                                                                            */
/* Input:  "item" is the item to remove.                                      */
/*                                                                            */
/* Output: The index where the item was removed at or -1 if it couldn't be    */
/*         found.                                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
int32 PList<TYPE>::FindAndRemoveItem(TYPE item)
{
	int32 index;

	index = GetItemIndex(item);
	if (index != -1)
		RemoveItem(index);

	return (index);
}



/******************************************************************************/
/* RemoveItem() will remove one item from the list at the index given.        */
/*                                                                            */
/* Input:  "index" is the index to the item to remove.                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::RemoveItem(int32 index)
{
	RemoveSomeItems(index, 1);
}



/******************************************************************************/
/* RemoveItems() will remove a number of items from the list.                 */
/*                                                                            */
/* Input:  "index" is the index of the first item to remove.                  */
/*         "count" is the number of items to remove.                          */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::RemoveItems(int32 index, int32 count)
{
	RemoveSomeItems(index, count);
}



/******************************************************************************/
/* GetHead() will return the first item in the list.                          */
/*                                                                            */
/* Output: The first item in the list.                                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PList<TYPE>::GetHead(void) const
{
	if (firstNode == NULL)
		throw PBoundsException();

	return (firstNode->data[0]);
}



/******************************************************************************/
/* GetTail() will return the last item in the list.                           */
/*                                                                            */
/* Output: The last item in the list.                                         */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PList<TYPE>::GetTail(void) const
{
	if (lastNode == NULL)
		throw PBoundsException();

	return (lastNode->data[lastNode->dataCount - 1]);
}



/******************************************************************************/
/* GetItem() will return the item at the index given.                         */
/*                                                                            */
/* Input:  "index" is the index in the list of the item you want.             */
/*                                                                            */
/* Output: The item at the index in the list.                                 */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PList<TYPE>::GetItem(int32 index) const
{
	PNode *itemNode;
	int32 pos;

	// First check for out of range
	if ((index < 0) || (index >= CountItems()))
		throw PBoundsException(P_ERR_ANY, index);

	itemNode = FindNode(index, &pos);
	return (itemNode->data[index - pos]);
}



/******************************************************************************/
/* SetItem() will change an item in the list.                                 */
/*                                                                            */
/* Input:  "item" is the new item.                                            */
/*         "index" is where to set the item.                                  */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::SetItem(TYPE item, int32 index)
{
	PNode *setNode;
	int32 pos;

	// First check for out of range
	if ((index < 0) || (index >= CountItems()))
		throw PBoundsException(P_ERR_ANY, index);

	// Find the node to change
	setNode = FindNode(index, &pos);
	setNode->data[index - pos] = item;
}



/******************************************************************************/
/* GetAndSetItem() will change an item in the list and return the old value.  */
/*                                                                            */
/* Input:  "item" is the new item.                                            */
/*         "index" is where to set the item.                                  */
/*                                                                            */
/* Output: Is the replaced item value.                                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PList<TYPE>::GetAndSetItem(TYPE item, int32 index)
{
	PNode *setNode;
	int32 pos;
	TYPE oldValue;

	// First check for out of range
	if ((index < 0) || (index >= CountItems()))
		throw PBoundsException(P_ERR_ANY, index);

	// Find the node to change
	setNode = FindNode(index, &pos);
	oldValue = setNode->data[index - pos];
	setNode->data[index - pos] = item;

	return (oldValue);
}



/******************************************************************************/
/* GetElement() will return a reference to the item at the index given.       */
/*                                                                            */
/* Input:  "index" is the index in the list of the item you want.             */
/*                                                                            */
/* Output: A reference to the item at the index in the list.                  */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE & PList<TYPE>::GetElement(int32 index) const
{
	PNode *itemNode;
	int32 pos;

	// First check for out of range
	if ((index < 0) || (index >= CountItems()))
		throw PBoundsException(P_ERR_ANY, index);

	itemNode = FindNode(index, &pos);
	return (itemNode->data[index - pos]);
}



/******************************************************************************/
/* HasItem() will search for the item to see if it is in the list.            */
/*                                                                            */
/* Input:  "item" is the item to search for.                                  */
/*         "startPos" is the start index in the list to begin the search.     */
/*                                                                            */
/* Output: True if the item was found, else false.                            */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
bool PList<TYPE>::HasItem(TYPE item, int32 startPos) const
{
	if (IsEmpty())
		return (false);

	return (GetItemIndex(item, startPos) == -1 ? false : true);
}



/******************************************************************************/
/* GetItemIndex() will search for the item and return the index.              */
/*                                                                            */
/* Input:  "item" is the item to search for.                                  */
/*         "startPos" is the start index in the list to begin the search.     */
/*                                                                            */
/* Output: The index in the list where it found the item or -1.               */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
int32 PList<TYPE>::GetItemIndex(TYPE item, int32 startPos) const
{
	PNode *curNode;
	int32 pos;
	int32 i;

	// First check for out of range
	if ((startPos < 0) || (startPos >= CountItems()))
		throw PBoundsException(P_ERR_ANY, startPos);

	try
	{
		// First find the start node
		curNode = FindNode(startPos, &pos);
	}
	catch(PBoundsException e)
	{
		// Well, we couldn't find the node, so return -1
		return (-1);
	}

	// Now begin to search for the item
	startPos -= pos;
	while (curNode != NULL)
	{
		for (i = startPos; i < curNode->dataCount; i++)
		{
			if (curNode->data[i] == item)
				return (pos + i);			// Return the index
		}

		// Go to the next node
		pos     += curNode->dataCount;
		curNode  = curNode->nextNode;
		startPos = 0;
	}

	// Item not found
	return (-1);
}



/******************************************************************************/
/* CountItems() will return the number of items in the list.                  */
/*                                                                            */
/* Output: The number of items in the list.                                   */
/******************************************************************************/
template<class TYPE>
inline int32 PList<TYPE>::CountItems(void) const
{
	return (itemCount);
}



/******************************************************************************/
/* IsEmpty() will check the list to see if it's empty.                        */
/*                                                                            */
/* Output: True if the list is empty, else false.                             */
/******************************************************************************/
template<class TYPE>
inline bool PList<TYPE>::IsEmpty(void) const
{
	return (itemCount == 0);
}



/******************************************************************************/
/* MakeEmpty() will clear the list and delete all the elements in it.         */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::MakeEmpty(void)
{
	PNode *curNode, *nextNode;

	// Delete all the items and nodes
	curNode = firstNode;
	while (curNode != NULL)
	{
		nextNode = curNode->nextNode;

		delete[] curNode->data;
		delete curNode;

		curNode = nextNode;
	}

	// Clear the list variables
	firstNode              = NULL;
	lastNode               = NULL;
	itemCount              = 0;
	optimize->lastUsedNode = NULL;
}



/******************************************************************************/
/* operator = (PList &) will empty the current list and copy all the elements */
/*      from the list given.                                                  */
/*                                                                            */
/* Input:  "newList" is the new list.                                            */
/*                                                                            */
/* Output: The pointer to the current list.                                   */
/******************************************************************************/
template<class TYPE>
const PList<TYPE> & PList<TYPE>::operator = (const PList<TYPE> &newList)
{
	// Clear the list
	MakeEmpty();

	// Copy all the elements
	InsertList(newList);

	return (*this);
}



/******************************************************************************/
/* AllocateNode() will allocate a new node and return it.                     */
/*                                                                            */
/* Output: A pointer to the new node.                                         */
/******************************************************************************/
template<class TYPE>
typename PList<TYPE>::PNode *PList<TYPE>::AllocateNode(void) const
{
	PNode *newNode;

	// Allocate the node
	newNode = new PNode;
	if (newNode == NULL)
		throw PMemoryException();

	// Fill out the new node
	newNode->nextNode  = NULL;
	newNode->prevNode  = NULL;
	newNode->dataCount = 1;	// We know that there will be at least added one item in the list
	newNode->data      = new TYPE[addSize];
	if (newNode->data == NULL)
	{
		delete newNode;
		throw PMemoryException();
	}

	return (newNode);
}



/******************************************************************************/
/* FindNode() will try to find the node with the index given.                 */
/*                                                                            */
/* Input:  "index" is the index to find the node at.                          */
/*         "pos" is a pointer to store the position the first item in the     */
/*         node have.                                                         */
/*                                                                            */
/* Output: A pointer to the new node.                                         */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
typename PList<TYPE>::PNode *PList<TYPE>::FindNode(int32 index, int32 *pos) const
{
	PNode *workNode;
	int32 workPos;

	if (optimize->lastUsedNode != NULL)
	{
		// If the new index is bigger than the old one, continue the search
		if (index >= optimize->lastUsedNodeIndex)
		{
			workNode = optimize->lastUsedNode;
			workPos  = optimize->lastUsedNodeIndex;
		}
		else
		{
			// Ok, the new index is lower than the old one.
			// Now find out if it is faster to make a backward
			// search or we should begin from the start
			//
			// If the new index is in the upper half of the
			// list going from the start to the old index,
			// a backward search is faster
			if (index < (optimize->lastUsedNodeIndex / 2))
			{
				// The new index is in the lower half, so begin over
				workNode = firstNode;
				workPos  = 0;
			}
			else
			{
				// Make backward search
				workNode = optimize->lastUsedNode->prevNode;
				workPos  = optimize->lastUsedNodeIndex - workNode->dataCount;

				while (workNode != NULL)
				{
					if (index >= workPos)
					{
						// Found the node, now return it
						*pos                        = workPos;
						optimize->lastUsedNodeIndex = workPos;
						optimize->lastUsedNode      = workNode;

						return (workNode);
					}

					// Go to the previous node
					workNode = workNode->prevNode;

					if (workNode != NULL)
						workPos -= workNode->dataCount;
				}

				// Something is wrong, we should never get here!
				throw PBoundsException(P_ERR_ANY, index);
			}
		}
	}
	else
	{
		// Start over from the beginning of the list
		workNode = firstNode;
		workPos  = 0;
	}

	// Search for the index using forward search
	while (workNode != NULL)
	{
		if (index < (workPos + workNode->dataCount))
		{
			// Found the node, now return it
			*pos                        = workPos;
			optimize->lastUsedNodeIndex = workPos;
			optimize->lastUsedNode      = workNode;

			return (workNode);
		}

		// Go to the next node
		workPos += workNode->dataCount;
		workNode = workNode->nextNode;
	}

	// Something is wrong, we should never get here!
	throw PBoundsException(P_ERR_ANY, index);

	// Just to make the compiler happy
	return (NULL);
}



/******************************************************************************/
/* InsertNewItem() will insert the item in the node and index given.          */
/*                                                                            */
/* Input:  "item" is the item to insert.                                      */
/*         "node" is a pointer to the node.                                   */
/*         "index" is the index in the node.                                  */
/******************************************************************************/
template<class TYPE>
void PList<TYPE>::InsertNewItem(TYPE item, PNode *node, int32 index)
{
	int32 i, j;

	// Start to check to see if the data array is full
	if (node->dataCount < addSize)
	{
		// It isn't, so move the items in the array to
		// make space for the new item
		for (i = node->dataCount; i > index; i--)
			node->data[i] = node->data[i - 1];

		// Now insert the new item
		node->data[index] = item;
		node->dataCount++;
	}
	else
	{
		// Well, the array is full. Check to see if there is any
		// space in the next array
		if ((node->nextNode != NULL) && (node->nextNode->dataCount < addSize))
		{
			// Yeah, now copy as many items as possible, but
			// first move the items to make room
			PNode *workNode = node->nextNode;
			int32 toCopy;

			toCopy = min(addSize - workNode->dataCount, node->dataCount - index);

			for (i = workNode->dataCount + toCopy - 1; i >= toCopy; i--)
				workNode->data[i] = workNode->data[i - toCopy];

			// Now copy the items into the room we have created
			for (i = node->dataCount - toCopy, j = 0; j < toCopy; i++, j++)
				workNode->data[j] = node->data[i];

			// Now move the items in the first node to make room
			// for the new item to insert if needed
			if ((node->dataCount - index) != toCopy)
			{
				for (i = node->dataCount - toCopy; i > index; i--)
					node->data[i] = node->data[i - 1];
			}

			// Insert the new item
			node->data[index] = item;

			// Adjust the data counters
			node->dataCount     -= (toCopy - 1);
			workNode->dataCount += toCopy;
		}
		else
		{
			// There isn't enough space, so we need to allocate
			// a new node and array
			PNode *newNode = AllocateNode();

			// Move the items from the insert position to the end
			// into the new array
			for (i = index, j = 0; i < node->dataCount; i++, j++)
				newNode->data[j] = node->data[i];

			// Insert the new item
			node->data[index] = item;

			// Adjust the data count in the node structures
			newNode->dataCount = node->dataCount - index;
			node->dataCount    = index + 1;

			// Link the new node into the list
			newNode->nextNode = node->nextNode;
			node->nextNode    = newNode;
			newNode->prevNode = node;

			if (newNode->nextNode != NULL)
				newNode->nextNode->prevNode = newNode;
			else
				lastNode = newNode;
		}
	}

	// Increment the list counter
	itemCount++;

	// Reset the get state variable
	optimize->lastUsedNode = NULL;
}



/******************************************************************************/
/* RemoveSomeItems() will remove a number of items from the list.             */
/*                                                                            */
/* Input:  "index" is the index of the first item to remove.                  */
/*         "count" is the number of items to remove.                          */
/*                                                                            */
/* Output: The first item removed.                                            */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PList<TYPE>::RemoveSomeItems(int32 index, int32 count)
{
	PNode *workNode, *prevNode;
	int32 tempCount = count;
	int32 i, pos, oldDataCount;
	TYPE firstItem;

	// First check for out of range
	if ((index < 0) || (index >= CountItems()))
		throw PBoundsException(P_ERR_ANY, index);

	// Find the node where the first item to remove is
	workNode = FindNode(index, &pos);

	// Remember the first item
	firstItem = workNode->data[index - pos];

	// Decrement the data counters in the nodes where we remove items
	oldDataCount = workNode->dataCount;
	pos          = index - pos;

	while ((workNode != NULL) && (tempCount > 0))
	{
		workNode->dataCount--;
		tempCount--;
		pos++;
		itemCount--;

		if (pos == oldDataCount)
		{
			pos      = 0;
			workNode = workNode->nextNode;

			if (workNode != NULL)
				oldDataCount = workNode->dataCount;
		}
	}

	// Move the items in the last node
	if (pos != 0)
	{
		if (pos <= workNode->dataCount)
		{
			for (i = max(pos - count, 0); pos < oldDataCount; i++, pos++)
				workNode->data[i] = workNode->data[pos];
		}
		else
		{
			for (i = pos - workNode->dataCount; pos < oldDataCount; i++, pos++)
				workNode->data[i] = workNode->data[pos];
		}
	}

	// Remove empty nodes
	prevNode = NULL;
	workNode = firstNode;

	while (workNode != NULL)
	{
		if (workNode->dataCount == 0)
		{
			// Found a node that is empty, so deallocate the memory
			PNode *removeNode;

			if (prevNode == NULL)
				firstNode = workNode->nextNode;
			else
				prevNode->nextNode = workNode->nextNode;

			removeNode = workNode;
			workNode   = workNode->nextNode;

			delete[] removeNode->data;
			delete removeNode;

			if (workNode == NULL)
				lastNode = prevNode;
		}
		else
		{
			prevNode = workNode;
			workNode = workNode->nextNode;
		}
	}

	// Compact the list
	workNode = firstNode;

	while (workNode != NULL)
	{
		if ((workNode->dataCount < (addSize / 2)) && (workNode->nextNode != NULL))
		{
			int32 toCopy = min(addSize - workNode->dataCount, workNode->nextNode->dataCount);
			int32 j;

			for (i = workNode->dataCount, j = 0; j < toCopy; i++, j++)
			{
				workNode->data[i] = workNode->nextNode->data[j];
				workNode->dataCount++;
				workNode->nextNode->dataCount--;
			}

			// Move the rest of the item back
			for (i = 0; i < workNode->nextNode->dataCount; i++)
				workNode->nextNode->data[i] = workNode->nextNode->data[i + toCopy];

			// Have we removed all the items so we can delete the node?
			if (workNode->nextNode->dataCount == 0)
			{
				// Yes, so we remove it
				PNode *removeNode;

				removeNode = workNode->nextNode;

				workNode->nextNode = workNode->nextNode->nextNode;
				delete[] removeNode->data;
				delete removeNode;

				if (workNode->nextNode == NULL)
					lastNode = workNode;
			}
		}
		else
			workNode = workNode->nextNode;
	}

	// Reset the get state variable
	optimize->lastUsedNode = NULL;

	return (firstItem);
}

#endif
