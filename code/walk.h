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

#include "ipiggy.h"
#include "loco.h"

#include <memory>


class WalkLocomotionClass : public LocomotionClass, public IPiggyback
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		WalkLocomotionClass(void);
		virtual ~WalkLocomotionClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;


		virtual bool Begin_Piggyback(std::unique_ptr<ILocomotion> & carried) override;
		virtual std::unique_ptr<ILocomotion> End_Piggyback(void) override;
		virtual bool Is_Ok_To_End(void) override;
		virtual bool Is_Piggybacking(void) override {return(Piggybacker != nullptr);}

		virtual bool Is_Moving(void) override;
		virtual Coord Destination(void) override;
		virtual Coord Head_To_Coord(void) override;
		virtual bool Process(void) override;
		virtual void Move_To(Coord to) override;
		virtual void Stop_Moving(void) override;
		virtual void Do_Turn(DirType dir) override;
		virtual LayerType In_Which_Layer(void) override;
		virtual void Force_Immediate_Destination(Coord coord) override;
		virtual bool Is_Moving_Now(void) override;
		virtual void Mark_All_Occupation_Bits(int mark) override;
		virtual bool Is_Moving_Here(Coord to) override;
		virtual bool Is_Really_Moving_Now(void) override;
		virtual void Stop_Movement_Animation(void) override {IsReallyMoving = false;};

		void Movement_AI(bool first_pass);
		bool Mark_Head_To(Coord const & coord);

	private:
		/*
		 * This is the ultimate coordinate that the infantry is walking to. However many cells
		 * of path it takes to get there, this is where the unit will end up.
		 */
		Coord DestinationCoord;

		/*
		 * This is the coordinate that the infantry is heading to as an immediate destination.
		 * It is never further away than one cell, and when it is reached the next location in
		 * the path list becomes the new head to coordinate.
		 */
		Coord HeadToCoord;

		/*
		 * If the infantry has somewhere to walk to, then this flag will be true. It is cleared
		 * once both the destination and the immediate head to coordinate have been given up.
		 */
		bool IsMoving;

		/*
		 * This flag is true for as long as a movement pass is being processed. A piggyback
		 * session refuses to end while it is set, so that control never changes hands part way
		 * through a step.
		 */
		bool IsProcessingMovement;

		/*
		 * If the infantry is actually taking steps rather than merely holding a destination,
		 * then this flag will be true. It drives the walk animation, so it is cleared whenever
		 * the unit is immobilized or stalled against a blockage.
		 */
		bool IsReallyMoving;

		/*
		 * Pointer to the locomotor that this one was stacked on top of. Walking takes over
		 * temporarily -- a jump jet coming down to cover the last few cells on foot, say --
		 * and the suspended locomotor is handed back when the walk is finished.
		 */
		std::unique_ptr<ILocomotion> Piggybacker;
};
