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

#include "ftimer.h"
#include "loco.h"
#include "timer.h"


class TunnelLocomotionClass : public LocomotionClass
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		TunnelLocomotionClass(void);

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual bool Is_Moving(void) override;
		virtual bool Is_Moving_Now(void) override;
		virtual Coord Destination(void) override;
		virtual void Move_To(Coord to) override;
		virtual void Stop_Moving(void) override;
		virtual bool Process(void) override;
		virtual VisualType Visual_Character(bool flag) override;
		virtual Matrix3D Draw_Matrix(int *key) override;
		virtual int Z_Adjust(void) override;
		virtual ZGradientType Z_Gradient(void) override;
		virtual bool Is_To_Have_Shadow(void) override;
		virtual MoveType Can_Enter_Cell(Cell cell) override;
		virtual void Do_Turn(DirType coord) override;
		virtual LayerType In_Which_Layer(void) override;
		virtual FireErrorType Can_Fire(void) override;
		virtual bool Is_Surfacing(void) override;

		void Process_Turning(void);
		void Process_Digging_In(void);
		void Process_Descending(void);
		void Process_Tunneling(void);
		void Process_Ascending(void);
		void Process_Emerging(void);
		void Process_Aborting(void);

	private:
		/*
		 * This is the stage of the burrow cycle the unit is in. A move order starts it at
		 * STATE_TURNING and each stage advances to the next until it returns to STATE_IDLE,
		 * and it is what the locomotor answers its movement, layer and fire queries from.
		 */
		enum {
			STATE_IDLE,				/// Surfaced and at rest on the ground (Is_Moving() is false).
			STATE_TURNING,			/// Rotating to face the destination before digging in.
			STATE_DIGGING_IN,		/// Pitching nose-down into the ground (the dig-in rotation).
			STATE_DESCENDING,		/// Sinking straight down to full burrow depth.
			STATE_TUNNELING,		/// Travelling underground toward the destination.
			STATE_ASCENDING,		/// Rising straight back up beneath the destination cell.
			STATE_EMERGING,			/// Levelling out at the surface, then returns to idle.
			STATE_ABORTING,			/// Levelling out after an interrupted dig-in, then returns to idle.
		} State;

		Coord DestinationCoord;                         /// World coordinate the unit is burrowing toward (COORD_NONE when idle).
		ProgressTimerClass<FrameTimerClass> DigTimer;   /// Times (and gates) the dig-in / surfacing / levelling rotations.
		bool IsUnderground;                             /// Latched true partway through the dig-in rotation (see Z_Adjust); no other reader.
};
