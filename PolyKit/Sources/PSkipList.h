/******************************************************************************/
/* PSkipList header file.                                                     */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PSkipList_h
#define __PSkipList_h

// PolyKit headers
#include "POS.h"
#include "PException.h"
#include "PList.h"
#include "PSystem.h"


/******************************************************************************/
/* PSkipList class                                                            */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
class PSkipList
{
public:
	// Constructor and destructor
	PSkipList(uint8 maxLevel = 32);
	~PSkipList(void);				// Cannot be virtual!

	// Insertion and removal of items
	bool InsertItem(const KEY_TYPE key, const TYPE item);
	bool RemoveItem(const KEY_TYPE key);
	bool GetAndRemoveItem(const KEY_TYPE key, TYPE &item);

	// Getting and setting items
	bool GetItem(const KEY_TYPE key, TYPE &item) const;
	bool GetItem(int32 index, TYPE &item) const;
	bool SetItem(const KEY_TYPE key, const TYPE newItem);
	bool GetAndSetItem(const KEY_TYPE key, const TYPE newItem, TYPE &oldItem);

	// Getting both the key and value of an item
	bool GetKeyAndItem(int32 index, KEY_TYPE &key, TYPE &item) const;

	// Searching for a key
	bool HasKey(const KEY_TYPE key) const;

	// Getting the key of an item
	bool GetKey(int32 index, KEY_TYPE &key) const;

	// Emptying the list
	void MakeEmpty(void);
	bool IsEmpty(void) const;

	// Counting the items
	int32 CountItems(void) const;

	// Assignment
	const PSkipList<KEY_TYPE, TYPE> & operator = (const PSkipList<KEY_TYPE, TYPE> &skiplist);

protected:
	class PNode;	// Predeclaration

	// PHeader helper class
	class PHeader
	{
	public:
		// Constructor
		PHeader(uint8 level)
		{
			// Initialize member variables
			numLevels = level + 1;

			// Create vector of forward node pointers
			next = new PNode *[numLevels];
			if (next == NULL)
				throw PMemoryException();

			// Set all the pointers to forward nodes to NULL
			Clean();
		}

		// Destructor deletes the next node in the list
		virtual ~PHeader(void) { delete[] next; }

		// Clean() sets all the pointers to forward nodes to NULL
		void Clean(void) { for (uint32 i = 0; i < numLevels; i++) next[i] = NULL; }

		// Attributes
		PNode **next;		// Vector of pointers for forward nodes
		uint8 numLevels;	// Number of levels for vector of forward pointers
	};

	// PNode helper class
	class PNode : public PHeader
	{
	public:
		// Constructor
		PNode(uint8 level, KEY_TYPE key, TYPE item) : PHeader(level)
		{
			// Initialize member variables
			this->key = key;
			this->item = item;
		}

		// Attributes
		KEY_TYPE key;	// Search key
		TYPE item;		// The item
	};

	// Helper functions
	PNode *FindNode(const KEY_TYPE key) const;
	PNode *FindNodeAndUpdate(const KEY_TYPE key) const;
	PNode *RemoveNode(const KEY_TYPE key);

	// Attributes
	uint8 maxLevel;		// The max level limit
	uint8 level;		// Current level
	int32 count;		// Number of items
	PHeader *header;	// The header for the forward nodes
	PNode **update;		// Update vector with forward node pointers
};



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "maxLevel" is the maximal level for the list.                      */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
PSkipList<KEY_TYPE, TYPE>::PSkipList(uint8 maxLevel)
{
	// Initialize member variables
	this->maxLevel = maxLevel;
	level = 0;
	count = 0;

	// Allocate and initialize the header
	header = new PHeader(maxLevel);
	if (header == NULL)
		throw PMemoryException();

	// Allocate update vector with forward node pointers
	update = new PNode *[maxLevel];
	if (update == NULL)
	{
		delete header;				// Delete the header
		throw PMemoryException();
	}
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
PSkipList<KEY_TYPE, TYPE>::~PSkipList(void)
{
	// Make the list empty
	MakeEmpty();

	// Delete the update vector
	delete[] update;

	// Delete the header
	delete header;
}



/******************************************************************************/
/* InsertItem() inserts an item into the list.                                */
/*                                                                            */
/* Input:  "key" is the search key for the item.                              */
/*         "item" is the item to insert.                                      */
/*                                                                            */
/* Output: True if the item was inserted, false if not.                       */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::InsertItem(const KEY_TYPE key, const TYPE item)
{
	// Find nearest node and update the vector with pointers to this node
	PNode *node = FindNodeAndUpdate(key);

	// Return false if a node already exist with the specified search key
	if ((node != NULL) && (node->key == key))
		return (false);			// The node does already exists in the list

	// Generate a new level for the node using the randomized probabilistic function
	uint8 newLevel = 0;
	while ((PSystem::Random(0x7fffffff) < 0x3fffffff) && (newLevel < (maxLevel - 1)))
		newLevel++;

	// Update the current level, header, and update vector,
	// if the new level is higher then the current level
	if (newLevel > level)
	{
		// Update the update vector to point at the header forward node pointers
		for (uint32 i = level + 1; i <= newLevel; i++)
			update[i] = (PNode *)header;

		// Set the current level to the new level
		level = newLevel;
	}

	// Create and initialize the new node
	node = new PNode(level, key, item);
	if (node == NULL)
		throw PMemoryException();

	// Update the new node's forward node pointers for each level, and update
	// update vector's forward node pointers to this node for each level
	for (uint32 i = 0; i <= level; i++)
	{
		// Update the node's forward pointer
		node->next[i] = update[i]->next[i];

		// Update the update vector's forward pointer to the new node
		update[i]->next[i] = node;
	}

	// Increment the item counter
	count++;

	return (true);
}



/******************************************************************************/
/* RemoveItem() removes an item from the list.                                */
/*                                                                            */
/* Input:  "key" is the search key for the item.                              */
/*                                                                            */
/* Output: True if the item was removed, false if not.                        */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::RemoveItem(const KEY_TYPE key)
{
	// Remove the node containing the item from the list
	PNode *node = RemoveNode(key);
	if (node == NULL)
		return (false);

	// Delete the node
	delete node;

	// Adjust the current level to reflect the removal of the node
	while ((level > 0) && (header->next[level] == NULL))
		level--;

	// Decrement the item counter
	count--;

	return (true);
}



/******************************************************************************/
/* GetAndRemoveItem() gets an item and removes it from the list.              */
/*                                                                            */
/* Input:  "key" is the search key for the item.                              */
/*         "item" is a reference where to store the item value.               */
/*                                                                            */
/* Output: True if the item is returned, false if not.                        */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::GetAndRemoveItem(const KEY_TYPE key, TYPE &item)
{
	// Remove the node containing the item from the list
	PNode *node = RemoveNode(key);
	if (node == NULL)
		return (false);

	// Save the item before deleting the node
	item = node->item;

	// Delete the node
	delete node;

	// Adjust the current level to reflect the removal of the node
	while ((level > 0) && (header->next[level] == NULL))
		level--;

	// Decrement the item counter
	count--;

	return (true);
}



/******************************************************************************/
/* GetItem() gets an item from the list.                                      */
/*                                                                            */
/* Input:  "key" is the search key for the item.                              */
/*         "item" is a reference where to store the item value.               */
/*                                                                            */
/* Output: True if the item is returned, false if not.                        */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::GetItem(const KEY_TYPE key, TYPE &item) const
{
	// Find the node with the item
	PNode *node = FindNode(key);
	if (node == NULL)
		return (false);

	// Return the item
	item = node->item;

	return (true);
}



/******************************************************************************/
/* GetItem() gets an item from the list.                                      */
/*                                                                            */
/* Input:  "index" is the index of the item.                                  */
/*         "item" is a reference where to store the item value.               */
/*                                                                            */
/* Output: True if the item is returned, false if not.                        */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::GetItem(int32 index, TYPE &item) const
{
	// Check to see if the index is out of range
	if (index >= count)
		return (false);

	// Go to the node at the specified index
	PNode *node = (PNode *)header;
	for (int32 i = 0; i <= index; i++)
		node = node->next[0];

	// Return the item of the node
	item = node->item;

	return (true);
}



/******************************************************************************/
/* SetItem() replaces an item in the list with a new item.                    */
/*                                                                            */
/* Input:  "key" is the search key for the old item.                          */
/*         "newItem" is the new item.                                         */
/*                                                                            */
/* Output: True if the item was set, false if not.                            */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::SetItem(const KEY_TYPE key, const TYPE newItem)
{
	// Find the node with the item
	PNode *node = FindNode(key);
	if (node == NULL)
		return (false);

	// Set this item to the new item
	node->item = newItem;

	return (true);
}



/******************************************************************************/
/* GetAndSetItem() gets an item in the list and replaces it with a new item.  */
/*                                                                            */
/* Input:  "key" is the search key for the item.                              */
/*         "newItem" is the new item.                                         */
/*         "oldItem" is where the old item will be stored.                    */
/*                                                                            */
/* Output: True if the item has been changed, false if not.                   */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::GetAndSetItem(const KEY_TYPE key, const TYPE newItem, TYPE &oldItem)
{
	// Find the node with the item
	PNode *node = FindNode(key);
	if (node == NULL)
		return (false);

	// Save the old item
	oldItem = node->item;

	// Set the item of the node to the new item
	node->item = newItem;

	return (true);
}



/******************************************************************************/
/* HasKey() tests if the list has a specific key.                             */
/*                                                                            */
/* Input:  "key" is the search key.                                           */
/*                                                                            */
/* Output: True if the list has the key, or false otherwise.                  */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::HasKey(const KEY_TYPE key) const
{
	// Search through all the nodes from the start of the list
	PNode *node = (PNode *)header;
	while ((node = node->next[0]) != NULL)
	{
		// Return true, if the key was found
		if (node->key == key)
			return (true);
	}

	// The key could not be found
	return (false);
}



/******************************************************************************/
/* GetKey() returns the key of an item at a specific index.                   */
/*                                                                            */
/* Input:  "index" is the index of the item.                                  */
/*         "key" is a reference where to store the key.                       */
/*                                                                            */
/* Output: True if the key is returned, false if not.                         */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::GetKey(int32 index, KEY_TYPE &key) const
{
	// Check the index for out of range
	if (index >= count)
		return (false);

	// Go to the node at the specified index
	PNode *node = (PNode *)header;
	for (int32 i = 0; i <= index; i++)
		node = node->next[0];

	// Return the key of the node
	key = node->key;

	return (true);
}



/******************************************************************************/
/* GetKeyAndItem() returns both the key and item of an item at a specific     */
/*      index.                                                                */
/*                                                                            */
/* Input:  "index" is the index of the item.                                  */
/*         "key" is a reference to where to store the key.                    */
/*         "item" is a reference to where to store the item.                  */
/*                                                                            */
/* Output: True if the item is returned, false if not.                        */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::GetKeyAndItem(int32 index, KEY_TYPE &key, TYPE &item) const
{
	// Check the index for out of range
	if (index >= count)
		return (false);

	// Go to the node at the specified index
	PNode *node = (PNode *)header;
	for (int32 i = 0; i <= index; i++)
		node = node->next[0];

	// Return the key and item of the node
	key  = node->key;
	item = node->item;

	return (true);
}



/******************************************************************************/
/* MakeEmpty() empties the list.                                              */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
void PSkipList<KEY_TYPE, TYPE>::MakeEmpty(void)
{
	PNode *node, *next;

	// Delete all the nodes in the list
	node = header->next[0];
	while (node != NULL)
	{
		next = node->next[0];
		delete node;
		node = next;
	}

	// Reset the member variables
	header->Clean();
	count = 0;
	level = 0;
}



/******************************************************************************/
/* IsEmpty() tests if the list is empty.                                      */
/*                                                                            */
/* Output: True if the list is empty, or false otherwise.                     */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
bool PSkipList<KEY_TYPE, TYPE>::IsEmpty(void) const
{
	// Test if the bottom forward pointer of the header is NULL
	return (header->next[0] == NULL ? true : false);
}



/******************************************************************************/
/* CountItems() counts and returns the number of items in the list.           */
/*                                                                            */
/* Output: The number of items in the list.                                   */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
int32 PSkipList<KEY_TYPE, TYPE>::CountItems(void) const
{
	// Return item count
	return (count);
}



/******************************************************************************/
/* operator = (const PSkipList<KEY_TYPE, TYPE> &skiplist) assigns this list   */
/*      to the specified skip list.                                           */
/*                                                                            */
/* Input:  "skiplist" is the skip list to assign to this list.                */
/*                                                                            */
/* Output: The address of the changed skip list.                              */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
const PSkipList<KEY_TYPE, TYPE> & PSkipList<KEY_TYPE, TYPE>::operator = (const PSkipList<KEY_TYPE, TYPE> &skiplist)
{
	// Empty the list
	MakeEmpty();

	// For each node in the specified list, insert the contents of the node
	// into this list
	PNode *next = skiplist.header->next[0];
	while (next != NULL)
	{
		InsertItem(next->key, next->item);
		next = next->next[0];
	}

	// Return the address of this list
	return (*this);
}



/******************************************************************************/
/* FindNode() finds a node in the list.                                       */
/*                                                                            */
/* Input:  "key" is the search key for the node.                              */
/*                                                                            */
/* Output: The pointer to the node or NULL if it couldn't be found.           */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
typename PSkipList<KEY_TYPE, TYPE>::PNode* PSkipList<KEY_TYPE, TYPE>::FindNode(const KEY_TYPE key) const
{
	PNode *node, *next;

	// Trace the node topdown from the current level to the lowest level, until
	// we find the node with the key closest to, and less or equal to the search key
	node = (PNode *)header;
	for (int32 i = level; i >= 0; i--)
	{
		// Trace the node using the forward pointers until we reach NIL or
		// the node with the key closest to, and less or equal to the search key
		while (((next = node->next[i]) != NULL) && (next->key < key))
			node = next;
	}

	// Set the node to the closest node
	node = node->next[0];

	// Return NULL if a node could not be found with the specified search key
	if ((node == NULL) || (node->key != key))
		return (NULL);

	// Return the node
	return (node);
}



/******************************************************************************/
/* FindNodeAndUpdate() finds a node in the list and updates the update vector.*/
/*                                                                            */
/* Input:  "key" is the search key for the node.                              */
/*                                                                            */
/* Output: The pointer to the node or NULL if it couldn't be found.           */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
typename PSkipList<KEY_TYPE, TYPE>::PNode* PSkipList<KEY_TYPE, TYPE>::FindNodeAndUpdate(const KEY_TYPE key) const
{
	PNode *node, *next;

	// Trace the node topdown from the current level to the lowest level, until
	// we find the node with the key closest to, and less or equal to the search key
	node = (PNode *)header;
	for (int32 i = level; i >= 0; i--)
	{
		// Trace the node using the forward pointers until we reach NIL or
		// or the node with the key closest to, and less or equal to the search key
		while (((next = node->next[i]) != NULL) && (next->key < key))
			node = next;

		// Update the update vector's forward pointer to point at the closest node
		update[i] = node;
	}

	// Return the closest node
	return (node->next[0]);
}



/******************************************************************************/
/* RemoveNode() removes a node from the list.                                 */
/*                                                                            */
/* Input:  "key" is the search key for the node.                              */
/*                                                                            */
/* Output: The pointer to the node or NULL if it couldn't be found.           */
/******************************************************************************/
template<class KEY_TYPE, class TYPE>
typename PSkipList<KEY_TYPE, TYPE>::PNode* PSkipList<KEY_TYPE, TYPE>::RemoveNode(const KEY_TYPE key)
{
	// Find nearest node and update the vector with pointers to this node
	PNode *node = FindNodeAndUpdate(key);

	// Return NULL if a node does not exist with the specified search key
	if ((node == NULL) || (node->key != key))
		return (NULL);

	// Update the update vector's forward pointers to point to the next node
	// from the node to remove
	for (uint32 i = 0; i <= level; i++)
	{
		// Continue as long as the update vector's forward pointer points at
		// the node to remove
		if (update[i]->next[i] != node)
			break;

		// Set the update vector's forward pointer to point to the next node
		// from the node to remove
		update[i]->next[i] = node->next[i];
	}

	// Return the node
	return (node);
}

#endif
