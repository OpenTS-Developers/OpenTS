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

/* $Header: /CounterStrike/CARGO.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CARGO.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 23, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 23, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


class FootClass;
class MonoClass;

/****************************************************************************
**	This class handles the basic cargo logic.
*/
class CargoClass {
		friend class TechnoClass;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		CargoClass(void) : Quantity(0), CargoHold(0) {};
		~CargoClass(void) {/*CargoHold=0;*/};

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/

#ifdef _DEBUG
		void Debug_Dump(MonoClass *mono) const;
#endif
		void AI(void) {};

		int How_Many(void) const {return(Quantity);};
		bool Is_Something_Attached(void) const {return(CargoHold != 0);};
		FootClass * Attached_Object(void) const;
		FootClass * Detach_Object(void);
		void Detach(FootClass * object);
		void Attach(FootClass * object);

		// Carries the cargo hold to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(Quantity);
			stream.Serialize(CargoHold);
		}

	private:

		/*
		**	This is the number of objects attached to this cargo hold. For transporter
		**	objects, they might contain more than one object.
		*/
		unsigned Quantity;

		/*
		**	This is the target value of any attached object. A value of zero indicates
		**	that no object is attached.
		*/
		FootClass * CargoHold;
};
