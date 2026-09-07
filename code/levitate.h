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
#include "typelist.h"

class CCINIClass;


class LevitateLocomotionClass : public LocomotionClass
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		LevitateLocomotionClass(void);
		virtual ~LevitateLocomotionClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual HRESULT Link_To_Object(void *pointer) override;
		virtual bool Is_Moving(void) override;
		virtual Coord Destination(void) override;
		virtual Coord Head_To_Coord(void) override;
		virtual bool Process(void) override;
		virtual LayerType In_Which_Layer(void) override;
		virtual bool Is_Moving_Now(void) override;
		virtual void Mark_All_Occupation_Bits(int mark) override;


		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		void Hover_AI(void);

		void State_AI(void);
		void Process_Idle(void);
		void Process_Accelerating(void);
		void Process_Cruising(void);
		void Process_Decelerating(void);
		void Process_Drifting(void);
		void Process_Arrived(void);
		void Process_Recentering(void);

		void Process_Departing(void);

		bool Validate_TarCom(void);
		bool Validate_NavCom(void);

		void Accelerate(double&);
		void Accelerate_Towards(Coord const & coord);
		void Drift_Towards(Coord const & coord);
		void Drift(DirType const & dir);
		void Decelerate(void);
		bool Is_In_Proximity(Coord const & coord);
		bool Has_Arrived(Coord const & coord);

		void Steer_Towards(Coord const & coord);
		void Move_AI(void);
		double Update_Speed(void);
		bool Is_Not_On_Cell(Coord const & coord);
		void Set_Coord(Coord const & coord);
		bool Can_Move_Here(Coord const & coord);
		void Update_Bridge_State(Coord const & coord);
		bool Needs_New_Target(void);
		void Stop(void);

		static void Read_INI(CCINIClass const & ini);

	private:
		/*
		 * This is the state of the levitator's horizontal drift machine. State_AI dispatches
		 * one frame of handling from it each tick, and anything but STATE_IDLE counts as
		 * movement as far as the rest of the game is concerned.
		 */
		enum {
			STATE_IDLE,				/// At rest on its cell; bobs in place and picks up new targets.
			STATE_ACCELERATING,		/// Powered acceleration burst toward a target.
			STATE_CRUISING,			/// Coasting under drag at mood-velocity; the main travel state.
			STATE_DECELERATING,		/// Braking to a halt, then re-steers or comes to rest.
			STATE_DRIFTING,			/// Intentional low-speed drift toward a nearby target.
			STATE_ARRIVED,			/// Close enough to the target to count as arrived; all motion zeroed.
			STATE_RECENTERING,		/// Blocked-move recovery: drifts back to cell center to re-path.
			STATE_DEPARTING,		/// Glides out of the recentered cell on a fresh drift, then cruises.
		} State;

		/*
		 * This is the magnitude of the current horizontal velocity, recomputed by Update_Speed
		 * whenever the velocity changes. The state handlers measure it against the cap for the
		 * unit's current mood, and treat anything under 0.01 as stopped.
		 */
		double Speed;

		/*
		 * These are the two components of the unit's horizontal velocity, expressed in leptons
		 * per frame. Move_AI bleeds them off against MoveRate, adds the current thrust in, and
		 * steps the unit's position by them once the destination cell has been approved.
		 */
		double MoveX;
		double MoveY;

		/*
		 * These are the two components of the powered thrust vector, added to the velocity
		 * once a frame for as long as AccelerationsRemaining lasts. Accelerate builds them
		 * from the thrust angle, and the drift and arrival handlers zero them again.
		 */
		double AccelerationX;
		double AccelerationY;

		/*
		 * This is the number of frames of powered thrust left in the current burst. Accelerate
		 * charges it from AccelerationDuration and Move_AI counts it down. Once it runs out,
		 * the accelerating state gives way to coasting.
		 */
		int AccelerationsRemaining;

		/*
		 * This is the number of blocked moves the unit will tolerate before it gives up on its
		 * destination. It is recharged from MaxBlockCount on every successful move, so only a
		 * persistent obstruction can run it out.
		 */
		int BlockTriesRemaining;

		/*
		 * This is the amount of speed bled off the velocity each frame -- ordinary Drag while
		 * cruising, the steeper IntentionalDeacceleration while braking, and zero while
		 * drifting, so that a deliberate drift holds its speed.
		 */
		double MoveRate;

		/*
		 * This is the levitator's vertical velocity, expressed in leptons per frame. Hover_AI
		 * steps the unit's height by it, adds lift while the unit is below its hover height,
		 * subtracts gravity, and scales it back each frame so that the hovering settles
		 * instead of oscillating.
		 */
		double Dampen;

	private:
		/*
		 * These are the tuning values that govern every levitating unit, read from the
		 * [LEVITATION] section of the rules by Read_INI. They are shared by the whole class of
		 * units, so nothing here can be varied from one unit to the next.
		 */
		static struct GlobalControlsStruct {
			/*
			 * This is the speed bled off the velocity each frame while a unit is coasting,
			 * which is what eventually brings an unthrusting levitator to a halt.
			 */
			double Drag;

			/*
			 * These are the cruising speed thresholds for the three moods a levitator travels
			 * in: wandering with nothing to chase, following a destination, and closing on a
			 * target. The mood in force decides which one the Speed is measured against.
			 */
			double MaxVelocityWhenHappy;
			double MaxVelocityWhenFollowing;
			double MaxVelocityWhenPissedOff;

			/*
			 * This is the chance per frame that a unit with nothing to chase fires off a
			 * random thrust, expressed as a fraction (0 - 1). It is what makes an idle
			 * levitator wander about instead of sitting still.
			 */
			double AccelerationProbability;

			/*
			 * This is the length of a powered thrust burst, expressed in game frames. A
			 * longer burst carries the unit further before it drops back to coasting.
			 */
			int AccelerationDuration;

			/*
			 * This is how hard a thrust pushes: the amount added to the unit's velocity on
			 * each frame of the burst.
			 */
			double Acceleration;

			/*
			 * This is the one-off velocity kick applied the instant a thrust starts, so that
			 * the unit breaks away at once rather than easing into motion.
			 */
			double InitialBoost;

			/*
			 * This is the number of blocked moves a unit will tolerate before it abandons its
			 * destination and re-centers on its own cell.
			 */
			int MaxBlockCount;

			/*
			 * This is the speed bled off each frame while a unit is deliberately braking. It
			 * is steeper than Drag, so a unit closing on a target comes to rest far sooner
			 * than one merely coasting.
			 */
			double IntentionalDeacceleration;

			/*
			 * This is the speed of a deliberate drift -- the gentle creep a unit uses to close
			 * the last stretch to a nearby target or to settle back onto its own cell.
			 */
			double IntentionalDriftVelocity;

			/*
			 * This is the range within which a target counts as near, expressed in cells.
			 * Inside it a unit drifts gently at the target or brakes rather than thrusting.
			 */
			double ProximityDistance;
		} GlobalControls;

		/*
		 * These are the engine sounds a levitating unit makes when it fires a thrust; one is
		 * picked at random each time. Only every fourth thrust is actually heard, so that a
		 * swarm of hovering units does not drown out the rest of the battle.
		 */
		static TypeList<int> PropulsionSoundEffects;

		/*
		 * This is the name of the rules section that the global controls are read from.
		 */
		static char const * INI_NAME;
};
