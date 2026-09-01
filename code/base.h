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

/* $Header: /CounterStrike/BASE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : BASE.H                                                       *
 *                                                                                             *
 *                   Programmer : Bill Randolph                                                *
 *                                                                                             *
 *                   Start Date : 03/27/95                                                     *
 *                                                                                             *
 *                  Last Update : March 27, 1995                                               *
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "rect.h"
#include "vector.h"

#include "house.hh"
#include "struct.hh"

#include <unknwn.h>


class CCINIClass;
class CRCEngine;
class BuildingClass;
class HouseClass;
class SaveStreamClass;

/****************************************************************************
**	This class defines one "node" in the pre-built base list.  Each node
**	contains a type of building to build, and the COORDINATE to build it at.
*/
class BaseNodeClass
{
	public:
		BaseNodeClass(void) {};
		BaseNodeClass(StructType building, Cell cell) : Type(building), CellID(cell) {};
		bool operator == (BaseNodeClass const & node);
		bool operator != (BaseNodeClass const & node);
		bool operator > (BaseNodeClass const & node);

		StructType Type;

		/*
		 * This is the cell the building is to be built at. A base defense node is added with
		 * Cell(0, 0) here, which leaves the base planner to pick the spot for it when the
		 * time comes to build it.
		 */
		Cell CellID;

		// Carries this base node to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Type);
			stream.Serialize(CellID);
		}
};


/****************************************************************************
**	This is the class that defines a pre-built base for the computer AI.
** (Despite its name, this is NOT the "base" class for C&C's class hierarchy!)
*/
class BaseClass
{
	public:

		/*
		**	Constructor/Destructor
		*/
		BaseClass(void);
		~BaseClass(void) {Nodes.Clear();}

		/*
		**	Initialization
		*/
		//void Init(void) {Nodes.Clear(); InnerCells.Clear(); OuterCells.Clear();}
		void Init(void) {Nodes.Clear();}

		/*
		**	The standard suite of load/save support routines
		*/
		void Read_INI(CCINIClass const & ini, char const * hname);
		void Write_INI(CCINIClass & ini, char const * hname);
		virtual HRESULT STDMETHODCALLTYPE Load(IStream * stream);
		virtual HRESULT STDMETHODCALLTYPE Save(IStream * stream);
		void Serialize(SaveStreamClass & stream);
		virtual void Compute_CRC(CRCEngine &) const;

		/*
		**	Tells if the given node has been built or not
		*/
		bool Is_Built(int index) const;

		/*
		**	Returns a pointer to the object for the given node
		*/
		BuildingClass * Get_Building(int index) const;

		/*
		**	Tells if the given building ptr is a node in this base's list.
		*/
		bool Is_Node(BuildingClass const * obj);

		/*
		**	Returns a pointer to the requested node.
		*/
		BaseNodeClass * Get_Node(BuildingClass const * obj);
		BaseNodeClass * Get_Node(int index) { return(&Nodes[index]); }
		BaseNodeClass * Get_Node(Cell const & cell);

		/*
		**	Returns a pointer to the next "hole" in the Nodes list.
		*/
		BaseNodeClass * Next_Buildable(StructType type = STRUCT_NONE);
		int Next_Buildable_Index(StructType type = STRUCT_NONE);

		/*
		**	This is the list of "nodes" that define the base.  Portions of this
		**	list can be pre-built by simply saving those buildings in the INI
		**	along with non-base buildings, so Is_Built will return true for them.
		*/
		DynamicVectorClass<BaseNodeClass> Nodes;

		/*
		 * This is how much of the base plan the scenario starts with already built, expressed
		 * as a percentage. It is carried in the house's own section of the scenario INI.
		 */
		int PercentBuilt;

		/*
		 * These are the cells the base occupies. The inner list grows as structures are
		 * placed and is where the computer looks for room to put the next one; the outer list
		 * is the ring of wall cells around the base, which its defenses are placed along.
		 */
		DynamicVectorClass<Cell> InnerCells;
		DynamicVectorClass<Cell> OuterCells;

		/*
		 * This is the cell the base is considered to be centered on, which is wherever the
		 * construction yard was deployed. Candidate build cells are ranked by their distance
		 * from it, and the ground beneath it sets the height the base is built at.
		 */
		Cell PlacementCenter;

		/*
		 * This is the rectangle of cells the base covers, grown to take in each structure as
		 * it is placed. The threat and defense maps the computer works from are sized and
		 * indexed by it.
		 */
		Rect BaseAreaRect;

		/*
		 * This is the base rectangle grown by one cell on every side, which is the ring the
		 * perimeter wall is laid out along. It becomes the BaseAreaRect once the wall has
		 * been planned, so the next wall is built one ring further out.
		 */
		Rect LastBaseAreaRect;

		/*
		**	This is the house this base belongs to.
		*/
		HouseClass * House;

		/// Unused
		static char const * const INI_NAME;
};

extern HouseClass *UnusedHouse;
