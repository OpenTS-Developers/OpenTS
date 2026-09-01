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
 *                     $Archive:: /Commando/Code/wwlib/listnode.h                             $*
 *                                                                                             *
 *                      $Author:: Ian_l                                                       $*
 *                                                                                             *
 *                     $Modtime:: 9/20/01 9:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


/*
**	Includes
*/
#include <cassert>


/*
**	This is a doubly linked list node. Typical use of this node is to derive
**	objects from this node. The interface class for this node can be used for
**	added convenience.
*/
class GenericList;
class GenericNode {
	public:
		GenericNode(void) : NextNode(0), PrevNode(0) {}
		virtual ~GenericNode(void) {Unlink();}
		GenericNode(GenericNode & node) {node.Link(this);}
		GenericNode & operator = (GenericNode & node) {
			if (&node != this) {
				node.Link(this);
			}
			return(*this);
		}

		void Unlink(void) {
			// note that this means that the special generic node at the head
			// and tail of the list can not be unlinked.  This is done because
			// the user should never unlink them -- it will destroy the list in
			// an evil way.
			if (Is_Valid()) {
				PrevNode->NextNode = NextNode;
				NextNode->PrevNode = PrevNode;
				PrevNode = 0;
				NextNode = 0;
			}
		}

		void Link(GenericNode * node) {
			assert(node != (GenericNode *)0);
			node->Unlink();
			node->NextNode = NextNode;
			node->PrevNode = this;
			if (NextNode) NextNode->PrevNode = node;
			NextNode = node;
		}

		GenericNode * Next(void) const {return(NextNode);}
		GenericNode * Next_Valid(void) const {
			return ((NextNode && NextNode->NextNode) ? NextNode : (GenericNode *)0);
		}
		GenericNode * Prev(void) const {return(PrevNode);}
		GenericNode * Prev_Valid(void) const {
			return ((PrevNode && PrevNode->PrevNode) ? PrevNode : (GenericNode *)0);
		}
		bool Is_Valid(void) const {return(NextNode != (GenericNode *)0 && PrevNode != (GenericNode *)0);}

	protected:
		GenericNode * NextNode;
		GenericNode * PrevNode;
};


/*
**	This is a generic list handler. It manages N generic nodes. Use the interface class
**	to the generic list for added convenience.
*/
class GenericList {
	public:
		GenericList(void) {
			FirstNode.Link(&LastNode);
		}

		virtual ~GenericList(void) {
			while (FirstNode.Next()->Is_Valid()) {
				FirstNode.Next()->Unlink();
			}
		}

		GenericNode * First(void) const {return(FirstNode.Next());}
		GenericNode * First_Valid(void) const
		{
			GenericNode *node = FirstNode.Next();
			return (node->Next() ? node : (GenericNode *)0);
		}

		GenericNode * Last(void) const {return(LastNode.Prev());}
		GenericNode * Last_Valid(void) const
		{
			GenericNode *node = LastNode.Prev();
			return (node->Prev() ? node : (GenericNode *)0);
		}

		bool Is_Empty(void) const {return(!FirstNode.Next()->Is_Valid());}
		void Add_Head(GenericNode * node) {FirstNode.Link(node);}
		void Add_Tail(GenericNode * node) {LastNode.Prev()->Link(node);}
//		void Delete(void) {while (FirstNode.Next()->Is_Valid()) delete FirstNode.Next();}

		int Get_Valid_Count(void) const
		{
			GenericNode * node = First_Valid();
			int counter = 0;
			while(node) {
				counter++;
				node = node->Next_Valid();
			}
			return counter;
		}

	protected:
		GenericNode FirstNode;
		GenericNode LastNode;

	private:
		GenericList(GenericList & list) = delete;
		GenericList & operator = (GenericList const &) = delete;
};


/*
**	This node class serves only as an "interface class" for the normal node
**	object. In order to use this interface class you absolutely must be sure
**	that the node is the root base object of the "class T". If it is true that the
**	address of the node is the same as the address of the "class T", then this
**	interface class will work. You can usually ensure this by deriving the
**	class T object from this node.
*/
template<class T> class List;
template<class T>
class Node : public GenericNode {
	public:
		T Next(void) const {return((T)GenericNode::Next());}
		T Next_Valid(void) const {return((T)GenericNode::Next_Valid());}
		T Prev(void) const {return((T)GenericNode::Prev());}
		T Prev_Valid(void) const {return((T)GenericNode::Prev_Valid());}
		bool Is_Valid(void) const {return(GenericNode::Is_Valid());}
};


/*
**	This is an "interface class" for a list of nodes. The rules for the class T object
**	are the same as the requirements required of the node class.
*/
template<class T>
class List : public GenericList {
	public:
		// Walks the real nodes of the list. The end of the list is the first node
		// that is not linked on both sides, which is the tail sentinel.
		class Iterator {
			public:
				Iterator(T node) : Node(node) {}

				T operator * (void) const {return(Node);}
				Iterator & operator ++ (void) {Node = Node->Next(); return(*this);}
				bool operator != (Iterator const &) const {return(Node->Is_Valid());}

			private:
				T Node;
		};

		List(void) {};

		T First(void) const {return((T)GenericList::First());}
		T First_Valid(void) const {return((T)GenericList::First_Valid());}
		T Last(void) const {return((T)GenericList::Last());}
		T Last_Valid(void) const {return((T)GenericList::Last_Valid());}
		void Delete(void) {while (First()->Is_Valid()) delete First();}

		Iterator begin(void) const {return(Iterator(First()));}
		Iterator end(void) const {return(Iterator(nullptr));}

	private:
		List(List<T> const & rvalue) = delete;
		List<T> operator = (List<T> const & rvalue) = delete;
};
