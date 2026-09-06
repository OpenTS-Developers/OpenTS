/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "facing.h"
#include "loco.h"


class JumpjetLocomotionClass : public LocomotionClass
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		JumpjetLocomotionClass(void);
		virtual ~JumpjetLocomotionClass(void) override;

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
		virtual bool Is_Moving_Now(void) override;
		virtual void Mark_All_Occupation_Bits(int mark) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		void Process_Grounded(void);
		void Process_Ascent(void);
		void Process_Hover(void);
		void Process_Cruise(void);
		void Process_Descent(void);
		void Process_Unknown(void);
		void Movement_AI(void);
		Coord Closest_Free_Spot(Coord const & to) const;
		int Desired_Flight_Level(void) const;

	private:
		/*
		 * This is the coordinate the jumpjet is flying to, or COORD_NONE if it has been
		 * given nowhere to go. A jumpjet pays no attention to the path list and travels
		 * straight there, so this serves as both the immediate and the final destination.
		 */
		Coord HeadToCoord;

		/*
		 * If the jumpjet has somewhere to be, then this flag will be true. It says nothing
		 * about whether the object is off the ground -- one still climbing away and one
		 * settling onto its landing spot are both moving.
		 */
		bool IsMoving;

		/*
		 * This is the stage of flight the jumpjet is in. Each game frame is handed to the
		 * processing routine belonging to this stage, which flies the object and decides
		 * when it is time to pass on to the next one.
		 */
		enum ProcessStateType {
			GROUNDED,
			ASCENDING,
			HOVERING,
			CRUISING,
			DESCENDING,
			UNKNOWN,
		} CurrentState;

		/*
		 * This is the direction the jumpjet is traveling in. It swings around toward the
		 * destination while the object flies, and the object is carried along it by the
		 * current speed each game frame.
		 */
		FacingClass Facing;

		/*
		 * These are the jumpjet's present and wanted speeds, expressed in leptons per game
		 * frame. Each stage of flight sets the target it wants, and the speed eases toward
		 * it at the jumpjet acceleration rate -- braking half again as hard as it picks up.
		 */
		double CurrentSpeed;
		double TargetSpeed;

		/*
		 * This is the altitude the jumpjet is trying to hold, expressed in leptons above the
		 * ground. It is the cruising height while under way, a fraction of that while
		 * turning or slowing, and zero once the unit has committed to a landing.
		 */
		int FlightLevel;

		/*
		 * This is the phase of the jumpjet's wobble, expressed in radians. It winds on while
		 * the unit hovers or cruises and is reset whenever it does neither, so that an
		 * object always begins to wobble from a level attitude.
		 */
		double CurrentWobble;

		/*
		 * If the jumpjet has claimed the spot it means to touch down on, then this flag will
		 * be true. The claim keeps anything else from taking the spot while the unit
		 * descends, and it is given up if the unit is ordered away again.
		 */
		bool IsLanding;
};
