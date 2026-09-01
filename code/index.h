/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/INDEX.H                                            $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   IndexClass<T>::Add_Index -- Add element to index tracking system.                         *
 *   IndexClass<T>::Clear -- Clear index handler to empty state.                               *
 *   IndexClass<T>::Count -- Fetch the number of index entries recorded.                       *
 *   IndexClass<T>::Fetch_Index -- Fetch data from specified index.                            *
 *   IndexClass<T>::Increase_Table_Size -- Increase the internal index table capacity.         *
 *   IndexClass<T>::IndexClass -- Constructor for index handler.                               *
 *   IndexClass<T>::Invalidate_Archive -- Invalidate the archive pointer.                      *
 *   IndexClass<T>::Is_Archive_Same -- Checks to see if archive pointer is same as index.      *
 *   IndexClass<T>::Is_Present -- Checks for presense of index entry.                          *
 *   IndexClass<T>::Remove_Index -- Find matching index and remove it from system.             *
 *   IndexClass<T>::Search_For_Node -- Perform a search for the specified node ID              *
 *   IndexClass<T>::Set_Archive -- Records the node pointer into the archive.                  *
 *   IndexClass<T>::Sort_Nodes -- Sorts nodes in preparation for a binary search.              *
 *   IndexClass<T>::~IndexClass -- Destructor for index handler object.                        *
 *   compfunc -- Support function for bsearch and bsort.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "bsearch.h"

#include <algorithm>
#include <cassert>

/*
**	This class is used to create and maintain an index. It does this by assigning a unique
**	identifier number to the type objects that it is indexing. The regular binary sort and search
**	function are used for speedy index retrieval. Typical use of this would be to index pointers to
**	normal data objects, but it can be used to hold the data objects themselves. Keep in mind that
**	the data object "T" is copied around by this routine in the internal tables so not only must
**	it have a valid copy constructor, it must also be efficient. The internal algorithm will
**	create an arbitrary number of default constructed objects of type "T". Make sure it has a
**	default constructor that is efficient and trivial. The constructor need not perform any actual
**	initialization since this class has prior knowledge about the legality of these temporary
**	objects and doesn't use them until after the copy constructor is used to initialize them.
**	This means that the default constructed object of type "T" can be technically in an unusable
**	state since it won't ever be used by this routine and won't ever be returned by any of
**	this template's methods.
**
**	This should properly be called a "map container class" since it is a container with a
**	mapping of an identifier.
*/

template<class INDEX, class T>
class IndexClass
{
	public:
		IndexClass(void);
		~IndexClass(void);

		// Copying would leave the archive pointer referring into another index's table.
		IndexClass(IndexClass const &) = delete;
		IndexClass & operator = (IndexClass const &) = delete;

		/*
		**	Add element to index table.
		*/
		bool Add_Index(INDEX const & id, T const & data);

		/*
		**	Removes an index entry from the index table.
		*/
		bool Remove_Index(INDEX const & id);

		/*
		**	Check to see if index is present.
		*/
		bool Is_Present(INDEX const & id) const;

		/*
		**	Fetch number of index entries in the table.
		*/
		int Count(void) const;

		/*
		**	Actually a fetch an index data element from the table.
		*/
		T operator [] (INDEX const & id) const;

		/*
		**	Fetch a data element by position reference.
		*/
		T const & Fetch_By_Position(int id) const;
		INDEX Fetch_ID_By_Position(int pos) const;

		/*
		**	Clear out the index table to null (empty) state.
		*/
		void Clear(void);

	private:
		/*
		**	This node object is used to keep track of the connection between the data
		**	object and the index identifier number.
		*/
		struct NodeElement {
			NodeElement(void) {}		// Default constructor does nothing (by design).

			INDEX ID;		// ID number.
			T Data;			// Data element assigned to this ID number.

			bool operator == (NodeElement const & rvalue) const {return(ID == rvalue.ID);}
			bool operator < (NodeElement const & rvalue) const {return(ID < rvalue.ID);}
		};

		/*
		**	This is the pointer to the allocated index table. It contains all valid nodes in
		**	a sorted order.
		*/
		NodeElement * IndexTable;

		/*
		**	This records the number of valid nodes within the index table.
		*/
		int IndexCount;

		/*
		**	The total size (in nodes) of the index table is recorded here. If adding a node
		**	would cause the index count to exceed this value, the index table must be resized
		**	to make room.
		*/
		int IndexSize;

		/*
		**	If the index table is sorted and ready for searching, this flag will be true. Sorting
		**	of the table only occurs when absolutely necessary.
		*/
		mutable bool IsSorted;

		/*
		**	This records a pointer to the last element found by the Is_Present() function. Using
		**	this last recorded value can allow quick fetches of data whenever possible.
		*/
		mutable NodeElement const * Archive;

		/*
		**	Increase size of internal index table by amount specified.
		*/
		bool Increase_Table_Size(int amount);

		/*
		**	Check if archive pointer is the same as that requested.
		*/
		bool Is_Archive_Same(INDEX const & id) const;

		/*
		**	Invalidate the archive pointer.
		*/
		void Invalidate_Archive(void) const;

		/*
		**	Set archive to specified value.
		*/
		void Set_Archive(NodeElement const * node) const;

		/*
		**	Search for the node in the index table.
		*/
		NodeElement const * Search_For_Node(INDEX const & id) const;

		// Put the table in ID order, which searching and position fetching both rely on.
		void Sort_Table(void) const;
};


/***********************************************************************************************
 * IndexClass<T>::IndexClass -- Constructor for index handler.                                 *
 *                                                                                             *
 *    This constructs an empty index handler.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
IndexClass<INDEX, T>::IndexClass(void) :
	IndexTable(0),
	IndexCount(0),
	IndexSize(0),
	IsSorted(false),
	Archive(0)
{
	Invalidate_Archive();
}


/***********************************************************************************************
 * IndexClass<T>::~IndexClass -- Destructor for index handler object.                          *
 *                                                                                             *
 *    This will destruct and free any resources managed by this index handler.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
IndexClass<INDEX, T>::~IndexClass(void)
{
	Clear();
}


/***********************************************************************************************
 * IndexClass<T>::Clear -- Clear index handler to empty state.                                 *
 *                                                                                             *
 *    This routine will free all internal resources and bring the index handler into a         *
 *    known empty state. After this routine, the index handler is free to be reused.           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
void IndexClass<INDEX, T>::Clear(void)
{
	delete [] IndexTable;
	IndexTable = 0;
	IndexCount = 0;
	IndexSize = 0;
	IsSorted = false;
	Invalidate_Archive();
}


/***********************************************************************************************
 * IndexClass<T>::Increase_Table_Size -- Increase the internal index table capacity.           *
 *                                                                                             *
 *    This helper function will increase the capacity of the internal index table without      *
 *    performing any alterations to the index data. Use this routine prior to adding a new     *
 *    element if the existing capacity is insufficient.                                        *
 *                                                                                             *
 * INPUT:   amount   -- The number of index element spaces to add to its capacity.             *
 *                                                                                             *
 * OUTPUT:  bool; Was the internal capacity increased without error?                           *
 *                                                                                             *
 * WARNINGS:   If there is insufficient RAM, then this routine will fail.                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
bool IndexClass<INDEX, T>::Increase_Table_Size(int amount)
{
	/*
	**	Check size increase parameter for legality.
	*/
	if (amount < 0) return(false);

	NodeElement * table = new NodeElement[IndexSize + amount];

	/*
	**	Copy all valid nodes into the new table.
	*/
	for (int index = 0; index < IndexCount; index++) {
		table[index] = IndexTable[index];
	}

	/*
	**	Make the new table the current one (and delete the old one).
	*/
	delete [] IndexTable;
	IndexTable = table;
	IndexSize += amount;
	Invalidate_Archive();

	return(true);
}


/***********************************************************************************************
 * IndexClass<T>::Count -- Fetch the number of index entries recorded.                         *
 *                                                                                             *
 *    This will return the quantity of index entries that have been recored by this index      *
 *    handler.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with number of recored indecies present.                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
int IndexClass<INDEX, T>::Count(void) const
{
	return(IndexCount);
}


/***********************************************************************************************
 * IndexClass<T>::Is_Present -- Checks for presense of index entry.                            *
 *                                                                                             *
 *    This routine will scan for the specified index entry. If it was found, then 'true' is    *
 *    returned.                                                                                *
 *                                                                                             *
 * INPUT:   id -- The index ID to search for.                                                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the index entry found in this index handler object?                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
bool IndexClass<INDEX, T>::Is_Present(INDEX const & id) const
{
	/*
	**	If there are no data elements in the index table, then it can
	**	never find the specified index. Check for and return failure
	**	in this case.
	*/
	if (IndexCount == 0) {
		return(false);
	}

	/*
	**	Check to see if this same index element was previously searched for. If
	**	so and it was previously found, then there is no need to search for it
	**	again -- just return true.
	*/
	if (Is_Archive_Same(id)) {
		return(true);
	}

	/*
	**	Perform a binary search on the index nodes in order to look for a
	**	matching index value.
	*/
	NodeElement const * nodeptr = Search_For_Node(id);

	/*
	**	If a matching index was found, then record it for future reference and return success.
	*/
	if (nodeptr != 0) {
		Set_Archive(nodeptr);
		return(true);
	}

	/*
	**	Could not find element so return failure condition.
	*/
	return(false);
}


/***********************************************************************************************
 * IndexClass<T>::Fetch_By_Index -- Fetch data from specified index.                           *
 *                                                                                             *
 *    This routine will find the specified index and return the data value associated with it. *
 *                                                                                             *
 * INPUT:   id -- The index ID to search for.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the data value associated with the index value.                       *
 *                                                                                             *
 * WARNINGS:   This routine presumes that the index exists. If it doesn't exist, then the      *
 *             default constructed object "T" is returned instead. To avoid this problem,      *
 *             always verfiy the existance of the index by calling Is_Present() first.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
T IndexClass<INDEX, T>::operator [] (INDEX const & id) const
{
	if (Is_Present(id)) {

		/*
		**	Count on the fact that the archive pointer is always valid after a call to Is_Present
		**	that returns "true".
		*/
		return(Archive->Data);
	}
	return(T());
}



template<class INDEX, class T>
T const & IndexClass<INDEX, T>::Fetch_By_Position(int pos) const
{
	assert(pos < IndexCount);

	Sort_Table();

	return(IndexTable[pos].Data);
}


/// <summary>
/// Fetches the identifier of the entry at the position specified.
/// </summary>
/// <remarks>Positions run in ID order, matching Fetch_By_Position, so the two may be
/// paired to walk the index.</remarks>
template<class INDEX, class T>
INDEX IndexClass<INDEX, T>::Fetch_ID_By_Position(int pos) const
{
	assert(pos < IndexCount);

	Sort_Table();

	return(IndexTable[pos].ID);
}


/***********************************************************************************************
 * IndexClass<T>::Is_Archive_Same -- Checks to see if archive pointer is same as index.        *
 *                                                                                             *
 *    This routine compares the specified index ID with the previously recorded index archive  *
 *    pointer in order to determine if they match.                                             *
 *                                                                                             *
 * INPUT:   id -- The index ID to compare to the archive index object pointer.                 *
 *                                                                                             *
 * OUTPUT:  bool; Does the specified index match the archive pointer?                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
bool IndexClass<INDEX, T>::Is_Archive_Same(INDEX const & id) const
{
	if (Archive != 0 && Archive->ID == id) {
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * IndexClass<T>::Invalidate_Archive -- Invalidate the archive pointer.                        *
 *                                                                                             *
 *    This routine will set the archive pointer to an invalid state. This must be performed    *
 *    if ever the archive pointer would become illegal (such as when the element it refers to  *
 *    is deleted).                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
void IndexClass<INDEX, T>::Invalidate_Archive(void) const
{
	Archive = 0;
}


/***********************************************************************************************
 * IndexClass<T>::Set_Archive -- Records the node pointer into the archive.                    *
 *                                                                                             *
 *    This routine records the specified node pointer. Use this routine when there is a        *
 *    good chance that the specified node will be requested in the immediate future.           *
 *                                                                                             *
 * INPUT:   node  -- Pointer to the node to assign to the archive.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
void IndexClass<INDEX, T>::Set_Archive(NodeElement const * node) const
{
	Archive = node;
}


/***********************************************************************************************
 * IndexClass<T>::Add_Index -- Add element to index tracking system.                           *
 *                                                                                             *
 *    This routine will record the index information into this index manager object. It        *
 *    associates the index number with the data specified. The data is copied to an internal   *
 *    storage location.                                                                        *
 *                                                                                             *
 * INPUT:   id    -- The ID number to associate with the data.                                 *
 *                                                                                             *
 *          data  -- The data to store.                                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the element added without error? Failure indicates that RAM has been     *
 *                exhausted.                                                                   *
 *                                                                                             *
 * WARNINGS:   The data is COPIED to internal storage. This means that the data object must    *
 *             have a functional and efficient copy constructor and assignment operator.       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
bool IndexClass<INDEX, T>::Add_Index(INDEX const & id, T const & data)
{
#ifdef _DEBUG
	/*
	**	Ensure that two elements with the same index are not added to the
	**	array.
	*/
	for (int index = 0; index < IndexCount; index++) {
		assert(IndexTable[index].ID != id);
	}
#endif

	/*
	**	Ensure that there is enough room to add this index. If not, then increase the
	**	capacity of the internal index table.
	*/
	if (IndexCount + 1 > IndexSize) {
		if (!Increase_Table_Size(IndexSize == 0 ? 10 : IndexSize)) {

			/*
			**	Failure to increase the size of the index table means failure to add
			**	the index element.
			*/
			return(false);
		}
	}

	/*
	**	Add the data to the end of the index data and then sort the index table.
	*/
	IndexTable[IndexCount].ID = id;
	IndexTable[IndexCount].Data = data;
	IndexCount++;
	IsSorted = false;

	return(true);
}


/***********************************************************************************************
 * IndexClass<T>::Remove_Index -- Find matching index and remove it from system.               *
 *                                                                                             *
 *    This will scan through the previously added index elements and if a match was found, it  *
 *    will be removed.                                                                         *
 *                                                                                             *
 * INPUT:   id -- The index ID to search for and remove.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the index element found and removed?                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
bool IndexClass<INDEX, T>::Remove_Index(INDEX const & id)
{
	/*
	**	Find the array index into the table that matches the specified id value.
	*/
#if 0
	/*
	int found_index = -1;
	for (int index = 0; index < IndexCount; index++) {
		if (IndexTable[index].ID == id) {
			found_index = index;
			break;
		}
	}
	*/
#else

	/*
	**	Find the array index into the table that matches the specified id value.
	*/
	NodeElement const * nodeptr = Search_For_Node(id);
	if (nodeptr == NULL) {
		return(false);
	}

	int found_index = nodeptr - IndexTable;
#endif

	/*
	**	Trying to remove something that isn't there could be an
	**	error in the calling routine?
	*/
//	assert(found_index);

	/*
	**	If the array index was found, then copy all higher index entries
	**	downward to fill the vacated location. We cannot use memcpy here because the type
	**	object may not support raw copies. C++ defines the assignment operator to deal
	**	with this, so that is what we use.
	*/
	if (found_index != -1) {

		for (int index = found_index+1; index < IndexCount; index++) {
			IndexTable[index-1] = IndexTable[index];
		}
		IndexCount--;

		NodeElement fake;
		fake.ID = 0;
		fake.Data = T();
		IndexTable[IndexCount] = fake;		// zap last (now unused) element

		Invalidate_Archive();
		return(true);
	}

	return(false);
}


/// <summary>
/// Puts the index table into ID order, if it is not in that order already.
/// </summary>
/// <remarks>Sorting only happens when a search or a position fetch needs it, so an index
/// that is only being added to never pays for it.</remarks>
template<class INDEX, class T>
void IndexClass<INDEX, T>::Sort_Table(void) const
{
	if (!IsSorted) {
		std::sort(IndexTable, IndexTable + IndexCount);
		Invalidate_Archive();
		IsSorted = true;
	}
}


/***********************************************************************************************
 * IndexClass<T>::Search_For_Node -- Perform a search for the specified node ID                *
 *                                                                                             *
 *    This routine will perform a binary search on the previously recorded index values and    *
 *    if a match was found, it will return a pointer to the NodeElement.                       *
 *                                                                                             *
 * INPUT:   id -- The index ID to search for.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the NodeElement that matches the index ID specified. If  *
 *          no matching index could be found, then NULL is returned.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
template<class INDEX, class T>
typename IndexClass<INDEX, T>::NodeElement const * IndexClass<INDEX, T>::Search_For_Node(INDEX const & id) const
{
	/*
	**	If there are no elements in the list, then it certainly can't find any matches.
	*/
	if (IndexCount == 0) {
		return(0);
	}

	Sort_Table();

	/*
	**	This list is sorted and ready to perform a binary search upon it.
	*/
	NodeElement node;
	node.ID = id;
	return(Binary_Search(IndexTable, IndexCount, node));
}
