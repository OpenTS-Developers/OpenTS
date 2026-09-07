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

#pragma once

#include "face.h"
#include "facing.h"
#include "ftimer.h"
#include "ipiggy.h"
#include "loco.h"
#include "matrix3d.h"
#include "timer.h"

#include "mark.hh"
#include "move.hh"

class HoverLocomotionClass : public LocomotionClass
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		HoverLocomotionClass(void);
		virtual ~HoverLocomotionClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual HRESULT Link_To_Object(void *pointer) override;
		virtual bool Is_Moving(void) override;
		virtual Coord Destination(void) override;
		virtual Coord Head_To_Coord(void) override;
		virtual Matrix3D Draw_Matrix(int *key) override;
		virtual bool Process(void) override;
		virtual void Move_To(Coord to) override;
		virtual void Stop_Moving(void) override;
		virtual void Do_Turn(DirType coord) override;
		virtual bool Power_Off(void) override;
		virtual bool Is_Powered(void) override;
		virtual bool Is_Ion_Sensitive(void) override;
		virtual bool Push(DirType dir) override;
		virtual bool Shove(DirType dir) override;
		virtual LayerType In_Which_Layer(void) override;
		virtual bool Is_Moving_Now(void) override;
		virtual void Mark_All_Occupation_Bits(int mark) override;
		virtual bool Is_Moving_Here(Coord to) override;

	private:

		/*
		 * This is the coordinate the object has been ordered to travel to, or COORD_NONE if
		 * it is under no move order. It is the far end of the journey -- the next step along
		 * the way is held in HeadToCoord.
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
		 * This is the direction the hover drive itself is pointed. The throttle stays shut
		 * until it has swung around to line up with the next stop. The body facing is kept
		 * separately, so the object may lean into a turn while the drive is already aimed at
		 * the exit.
		 */
		FacingClass Facing;

		/*
		 * This is the throttle the hover drive is being asked for, as a fraction of full
		 * speed. It is one while under way, a half while easing into the last cell, and zero
		 * while the drive is still swinging around to line up.
		 */
		double Height;

		/*
		 * This is the throttle actually being applied, as a fraction of full speed. It eases
		 * toward the demand rather than jumping to it, which is what gives a hover unit its
		 * slow pickup and its long coast.
		 */
		double Acceleration;

		/*
		 * This is the extra speed multiplier granted while the object has a straight run
		 * ahead of it -- the next two steps of its path lead the same way. It is one at all
		 * other times.
		 */
		double Boost;

		/*
		 * This is the object's vertical velocity, expressed in leptons per game frame. The
		 * hover cushion pushes it up whenever the object has sunk below its hover height,
		 * gravity pulls it back down and it is damped every frame, which together give a
		 * hover unit its bob and its settle.
		 */
		double Bounciness;

		/*
		 * If the object is still slewing about from a shove, then this flag will be true. It
		 * is cleared once the swing owed has been worked off, or at once if the hover drive
		 * loses power.
		 */
		bool WasShoved;

		/*
		 * This is the swing still owed from a shove, expressed as 360/256ths of a turn and
		 * signed for the direction to swing in. One step of it is fed into the body facing
		 * each game frame until nothing is left.
		 */
		int ShoveAccum;

		/*
		 * If the object has been pushed out of the way and has not yet reached the spot it
		 * was pushed toward, then this flag will be true. While it is set the object snaps
		 * around to its new heading instead of turning to it, and it may not be pushed again.
		 */
		bool WasPushed;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		void Gravity_AI(void);
		void Do_Shove(void);
		MoveType While_Moving(bool);
		bool Is_Moving1(void);
		void Stop(void);
		void Stop_Driver(void);
		void Motion_AI(void);
		bool Reduce_Path_Length(void);
		void Start_Of_Move(int);
		void Start(void);

};
