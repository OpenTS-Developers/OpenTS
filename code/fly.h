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

/* $Header: /CounterStrike/FLY.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FLY.H                                                        *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 24, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 24, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ipiggy.h"
#include "loco.h"

enum ImpactType {
	IMPACT_NONE,		// No movement (of significance) occurred.
	IMPACT_NORMAL,		// Some (non eventful) movement occurred.
	IMPACT_EDGE			// The edge of the world was reached.
};

class DirType;

/****************************************************************************
**	Flying objects are handled by this class definition.
*/
class FlyLocomotionClass : public LocomotionClass
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		FlyLocomotionClass(void);
		virtual ~FlyLocomotionClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual bool Is_Moving(void) override;
		virtual bool Is_Moving_Now(void) override;
		virtual Coord Destination(void) override;
		virtual void Move_To(Coord to) override;
		virtual void Stop_Moving(void) override;
		virtual bool Process(void) override;
		virtual Matrix3D Draw_Matrix(int *key) override;
		virtual Point2D Draw_Point(void) override;
		virtual Point2D Shadow_Point(void) override;
		virtual Matrix3D Shadow_Matrix(int *key) override;
		virtual void Do_Turn(DirType coord) override;
		virtual LayerType In_Which_Layer(void) override;
		virtual bool Power_Off(void) override;
		virtual bool Is_Powered(void) override;
		virtual bool Is_Ion_Sensitive(void) override;
		virtual int Apparent_Speed(void) override;
		virtual int Get_Status(void) override;
		virtual void Acquire_Hunter_Seeker_Target(void) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		bool Landing_Takeoff_AI(void);
		bool Edge_Of_World_AI(void);
		void Movement_AI(void);
		ImpactType Physics(Coord & coord, DirType facing);
		void Rotation_AI(void);

		/*
		**	State machine support routines.
		*/
		bool Process_Take_Off(void);
		bool Process_Landing(void);

		int Nearing_Target(bool stage_approach, Coord coord);

		void Take_Off(void);
		void Land(void);
		bool Is_In_Flight(void);
		void Tumble(void);
		bool Needs_To_Land(void);
		bool Is_Locked_To_Straight_Flight(void);

	private:
		/*
		 * This is the coordinate that the aircraft is flying toward. It is COORD_NONE when
		 * the aircraft has nowhere to go, and it is set to the aircraft's own position when
		 * it is told to stop while still airborne, so that it lands where it stands.
		 */
		Coord DestinationCoord;

		/// Unused
		Coord HeadToCoord;

		/*
		 * If the aircraft has been given somewhere to go, then this flag will be true. It
		 * records intent rather than motion, so it stays true while the aircraft is still
		 * building up speed, and it is cleared once the destination cell is reached.
		 */
		bool IsMoving;

		/*
		 * This is the altitude, expressed in leptons above ground level, that the aircraft
		 * is trying to hold. It is normally the flight level of the aircraft type, but it is
		 * raised to clear high ground along the route and eased down as a dropship nears
		 * its destination.
		 */
		int FlightLevel;

		/*
		 * These are the speed the aircraft is trying to travel at and the speed it is
		 * actually traveling at, both expressed as a fraction of the aircraft type's top
		 * speed. The current speed eases toward the target by a tenth each frame, which is
		 * what gives the aircraft its acceleration and its braking.
		 */
		double TargetSpeed;
		double CurrentSpeed;

		/*
		**	Aircraft can be in either state of landing, taking off, or in steady altitude.
		**	These flags are used to control transition between flying and landing. It is
		**	necessary to handle the transition in this manner so that it occurs smoothly
		**	during the graphic processing section.
		*/
		bool IsTakingOff;
		bool IsLanding;
		bool CommencedLanding;

		/*
		 * If the aircraft has been knocked out of the sky and is spinning as it falls, then
		 * this flag will be true. Restoring the aircraft's power calls the tumble off.
		 */
		bool IsTumbling;

		/*
		 * This is the rate the body spins at while the aircraft is tumbling, expressed as
		 * 360/256ths of a rotation per game frame. It is picked at random and may be
		 * negative, so that wrecks spin either way, and it winds back toward zero as the
		 * tumble plays out.
		 */
		int CurrentROT;

		/*
		 * This is the accumulated fall rate of an aircraft that has lost its power or been
		 * destroyed, expressed in leptons per game frame. It grows with every frame spent
		 * falling, so that the wreck picks up speed on the way down.
		 */
		int Riser;

		/*
		 * If the aircraft must hold its altitude instead of settling toward the ground as it
		 * nears its destination, then this flag will be true. It is worked out when a
		 * destination is assigned, and cleared when the aircraft is sent into a building.
		 */
		bool IsElevating;
};
