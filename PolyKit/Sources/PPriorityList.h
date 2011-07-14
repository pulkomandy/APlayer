/******************************************************************************/
/* PPriorityList header file.                                                 */
/******************************************************************************/
/* This source, or parts thereof, may be used in any software as long the     */
/* license of PolyKit is keep. See the LICENSE file for more information.     */
/*                                                                            */
/* Copyright (C) 1998-2002 by Polycode.                                       */
/* All rights reserved.                                                       */
/******************************************************************************/


#ifndef __PPriorityList_h
#define __PPriorityList_h

// PolyKit headers
#include "POS.h"
#include "ImportExport.h"
#include "PList.h"


/******************************************************************************/
/* PPriorityList class                                                        */
/******************************************************************************/
template<class TYPE>
class PPriorityList
{
public:
	// Constructor and destructor
	PPriorityList(int32 blockSize = 10);
	~PPriorityList(void);		// Do not make virtual

	// Insertion of items
	int32 InsertItem(TYPE item, int8 priority);

	// Removing of items
	TYPE GetAndRemoveItem(int32 index);
	int32 FindAndRemoveItem(TYPE item, int8 priority);
	void RemoveItem(int32 index);
	void RemoveItems(int32 index, int32 count);

	// Getting items
	TYPE GetHead(void) const;
	TYPE GetTail(void) const;
	TYPE GetItem(int32 index) const;

	// Searching
	bool HasItem(TYPE item, int8 priority, int32 startPos = 0) const;
	int32 GetItemIndex(TYPE item, int8 priority, int32 startPos = 0) const;

	// List counts
	int32 CountItems(void) const;
	bool IsEmpty(void) const;
	void MakeEmpty(void);

	// Assignment
	const PPriorityList<TYPE> & operator = (const PPriorityList<TYPE> &list);

protected:
	struct PItem
	{
		TYPE item;
		int8 priority;

		friend inline bool operator == (const PItem &item1, const PItem &item2) { return ((item1.item == item2.item) && (item1.priority == item2.priority)); };
		friend inline bool operator != (const PItem &item1, const PItem &item2) { return ((item1.item != item2.item) || (item1.priority != item2.priority)); };
	};

	PList<PItem> *list;
};



/******************************************************************************/
/* Constructor                                                                */
/*                                                                            */
/* Input:  "blockSize" is the number of items to allocate at once.            */
/******************************************************************************/
template<class TYPE>
PPriorityList<TYPE>::PPriorityList(int32 blockSize)
{
	// Allocate the list
	list = new PList<PItem>(blockSize);
	if (list == NULL)
		throw PMemoryException();
}



/******************************************************************************/
/* Destructor                                                                 */
/******************************************************************************/
template<class TYPE>
PPriorityList<TYPE>::~PPriorityList(void)
{
	// Delete the list object
	delete list;
}



/******************************************************************************/
/* InsertItem() will add a new item with the priority given in the list.      */
/*                                                                            */
/* Input:  "item" is the new item to add.                                     */
/*         "priority" is the priority the item should have.                   */
/*                                                                            */
/* Output: The index the item is inserted at.                                 */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
int32 PPriorityList<TYPE>::InsertItem(TYPE item, int8 priority)
{
	int32 index, count;
	PItem listItem;

	// Get the number of items in the list
	count = list->CountItems();

	// Find the place in the list to insert the item
	for (index = 0; index < count; index++)
	{
		// Get the current item from the list
		listItem = list->GetItem(index);

		// Did we found the right place
		if (priority > listItem.priority)
			break;
	}

	// Create and insert the item
	listItem.item     = item;
	listItem.priority = priority;
	list->InsertItem(listItem, index);

	return (index);
}



/******************************************************************************/
/* GetAndRemoveItem() will remove one item from the list and return it.       */
/*                                                                            */
/* Input:  "index" is the index of the item to remove.                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PPriorityList<TYPE>::GetAndRemoveItem(int32 index)
{
	PItem listItem;

	// Get and remove the item from the list
	listItem = list->GetAndRemoveItem(index);

	// Return the item
	return (listItem.item);
}



/******************************************************************************/
/* FindAndRemoveItem() will remove one item from the list.                    */
/*                                                                            */
/* Input:  "item" is the item to remove.                                      */
/*         "priority" is the item priority.                                   */
/*                                                                            */
/* Output: The index where the item was removed at or -1 if it couldn't be    */
/*         found.                                                             */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
int32 PPriorityList<TYPE>::FindAndRemoveItem(TYPE item, int8 priority)
{
	int32 index;

	index = GetItemIndex(item, priority);
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
inline void PPriorityList<TYPE>::RemoveItem(int32 index)
{
	list->RemoveItem(index);
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
inline void PPriorityList<TYPE>::RemoveItems(int32 index, int32 count)
{
	list->RemoveItems(index, count);
}



/******************************************************************************/
/* GetHead() will return the first item in the list.                          */
/*                                                                            */
/* Output: The first item in the list.                                        */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PPriorityList<TYPE>::GetHead(void) const
{
	PItem listItem;

	// Get the item
	listItem = list->GetHead();

	// Return the real item
	return (listItem.item);
}



/******************************************************************************/
/* GetTail() will return the last item in the list.                           */
/*                                                                            */
/* Output: The last item in the list.                                         */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
TYPE PPriorityList<TYPE>::GetTail(void) const
{
	PItem listItem;

	// Get the item
	listItem = list->GetTail();

	// Return the real item
	return (listItem.item);
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
TYPE PPriorityList<TYPE>::GetItem(int32 index) const
{
	PItem listItem;

	// Get the item
	listItem = list->GetItem(index);

	// Return the real item
	return (listItem.item);
}



/******************************************************************************/
/* HasItem() will search for the item with the priority given to see if it is */
/*      in the list.                                                          */
/*                                                                            */
/* Input:  "item" is the item to search for.                                  */
/*         "priority" is the priority on the item.                            */
/*         "startPos" is the start index in the list to begin the search.     */
/*                                                                            */
/* Output: True if the item was found, else false.                            */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
inline bool PPriorityList<TYPE>::HasItem(TYPE item, int8 priority, int32 startPos) const
{
	// Search for the item
	return (GetItemIndex(item, priority, startPos) == -1 ? false : true);
}



/******************************************************************************/
/* GetItemIndex() will search for the item with the priority given and return */
/*      the index.                                                            */
/*                                                                            */
/* Input:  "item" is the item to search for.                                  */
/*         "priority" is the items priority.                                  */
/*         "startPos" is the start index in the list to begin the search.     */
/*                                                                            */
/* Output: The index in the list where it found the item or -1.               */
/*                                                                            */
/* Except: PBoundsException.                                                  */
/******************************************************************************/
template<class TYPE>
int32 PPriorityList<TYPE>::GetItemIndex(TYPE item, int8 priority, int32 startPos) const
{
	PItem listItem;

	// Build the item to search for
	listItem.item     = item;
	listItem.priority = priority;

	// Try to find the item
	return (list->GetItemIndex(listItem, startPos));
}



/******************************************************************************/
/* CountItems() will return the number of items in the list.                  */
/*                                                                            */
/* Output: The number of items in the list.                                   */
/******************************************************************************/
template<class TYPE>
inline int32 PPriorityList<TYPE>::CountItems(void) const
{
	return (list->CountItems());
}



/******************************************************************************/
/* IsEmpty() will check the list to see if it's empty.                        */
/*                                                                            */
/* Output: True if the list is empty, else false.                             */
/******************************************************************************/
template<class TYPE>
inline bool PPriorityList<TYPE>::IsEmpty(void) const
{
	return (list->IsEmpty());
}



/******************************************************************************/
/* MakeEmpty() will clear the list and delete all the elements in it.         */
/******************************************************************************/
template<class TYPE>
inline void PPriorityList<TYPE>::MakeEmpty(void)
{
	list->MakeEmpty();
}



/******************************************************************************/
/* operator = (PPriorityList &) will empty the current list and copy all the  */
/*      elements from the list given.                                         */
/*                                                                            */
/* Input:  "newList" is the new list.                                         */
/*                                                                            */
/* Output: The pointer to the current list.                                   */
/******************************************************************************/
template<class TYPE>
const PPriorityList<TYPE> & PPriorityList<TYPE>::operator = (const PPriorityList<TYPE> &newList)
{
	// Clear the list
	list->MakeEmpty();

	// Copy all the elements
	list = newList.list;

	return (*this);
}

#endif
