/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "face.h"
#include "loco.h"

#include "mark.hh"


class MechLocomotionClass : public LocomotionClass
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		MechLocomotionClass(void);
		virtual ~MechLocomotionClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual bool Is_Moving(void) override;
		virtual Coord Destination(void) override;
		virtual Coord Head_To_Coord(void) override;
		virtual bool Process(void) override;
		virtual void Move_To(Coord to) override;
		virtual void Stop_Moving(void) override;
		virtual void Do_Turn(DirType coord) override;
		virtual LayerType In_Which_Layer(void) override;
		virtual void Force_Immediate_Destination(Coord coord) override;
		virtual bool Is_Moving_Now(void) override;
		virtual void Mark_All_Occupation_Bits(int mark) override;
		virtual bool Is_Moving_Here(Coord to) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/

	private:
		/*
		 * This is the coordinate that the mech has been ordered to walk to. It may lie any
		 * distance away -- the HeadToCoord below is the next step along the way to it. When
		 * this is COORD_NONE, the mech has nowhere it must be.
		 */
		Coord DestinationCoord;

		/*
		**	This is the coordinate that the unit is heading to
		**	as an immediate destination. This coordinate is never further
		**	than once cell (or track) from the unit's location. When this coordinate
		**	is reached, then the next location in the path list becomes the
		**	next HeadTo coordinate.
		*/
		Coord HeadToCoord;

		/*
		 * If the mech has somewhere to walk to, then this flag will be true. It is cleared
		 * once both the destination and the immediate head to coordinate have been given up.
		 */
		bool IsMoving;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		void Movement_AI(bool continue_moving);
		bool Mark_Head_To(Coord const & coord);
};
