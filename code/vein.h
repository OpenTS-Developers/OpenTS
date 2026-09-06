/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


#include "ftimer.h"
#include "nodes.h"
#include "object.h"
#include "priority.h"
#include "timer.h"
#include "stage.h"

#include "theater.hh"

template<class T>
class DynamicVectorClass;

class VeinholeMonsterClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		VeinholeMonsterClass(void);
		VeinholeMonsterClass(Cell const & cell);
		~VeinholeMonsterClass(void);

		virtual HRESULT GetClassID(CLSID * retval) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/

		static VeinholeMonsterClass * Get_Monster_At(Cell const & cell);
		static VeinholeMonsterClass * Get_Vein_Owner_At(Cell const & cell);

		static void Update_All(void);
		virtual void AI(void) override;
		static void Draw_All(void);
		static void Init(TheaterType theater);
		void Draw_It(void);
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced=false, bool=false) override;
		static bool Can_Monster_Go_Here(Cell const & cell);
		static void Reset(void);
		void Grow(void);
		void Shrink(void);
		static void Init_Vein_Growth_System(bool clear);
		static void Deinit_Vein_Growth_System(void);
		void Build_Growth_Queue(void);
		void Build_Shrinking_Queue(void);
		static void Clear_Global_Data(void);
		void Clear_Growth(void);
		void Destroy_Monster(void);
		static void Remove_Dead(void);
		static bool Load_All(SaveStreamClass & stream);
		static bool Save_All(SaveStreamClass & stream);
		void Reduce_Veins_At(CellClass * cellptr);

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_VEINHOLEMONSTER);}
		virtual LayerType In_Which_Layer(void) const override {return(LAYER_NONE);}

		virtual ObjectTypeClass const * Class_Of(void) const override;

	public:
		/*
		 * This is the number of nodes handed out of the GrowthNodes pool. It never falls
		 * until the queue is rebuilt, so it also serves as the check that stops a monster
		 * spreading past the vein limit in the rules.
		 */
		int GrowthCount;

		/*
		 * This is the frontier of the monster's patch -- the cells next in line to be
		 * covered, lowest score first. Once the monster is dying the same queue is refilled
		 * with the veins it already owns, farthest out first, so the patch withers back from
		 * its edges.
		 */
		PriorityQueueClass<CellNode> * GrowthQueue;

		/*
		 * This is the block of nodes the GrowthQueue's entries live in, sized to the vein
		 * limit in the rules. Holding them in one array is what lets the queue be saved as
		 * indices into it.
		 */
		CellNode * GrowthNodes;

		/*
		 * This is the countdown to the next growth step, restarted with a jittered copy of
		 * the growth rate from the rules -- or of the shrink rate, once the monster is dead.
		 */
		CDTimerClass<FrameTimerClass> GrowthTimer;

		/*
		 * This is a flag for every cell of the map saying whether the cell belongs to this
		 * monster's patch of veins. It is what lets the game work out which monster to
		 * charge when veins are destroyed somewhere.
		 */
		bool * GrowthState;

		/*
		 * This is the state the monster is currently playing -- IDLE, ALERT, ATTACKING or
		 * DYING.
		 */
		int CurrentState;

		/*
		 * This is the state the monster wants to be in. The change is held off until the
		 * animation reaches the stage it is allowed to transition at, so that the monster is
		 * never seen to jump from one pose to another.
		 */
		int DesiredState;

		/*
		 * This is the animation tracker that steps the monster through the frames of
		 * whichever state it is playing.
		 */
		StageClass Control;

		/*
		 * This is how long the monster stays roused after something has hurt it. While it is
		 * running the monster keeps belching gas instead of settling back down.
		 */
		CDTimerClass<FrameTimerClass> LogicTimer;

		/*
		 * This is the cell the monster sits in, and the point its veins spread out from.
		 */
		Cell CellID;

		/*
		 * This is the frame of the monster artwork to draw, taken from the animation stage
		 * each time the logic runs.
		 */
		int ShapeFrame;

		/*
		 * If the monster has been killed, then this flag will be true. A dead monster stops
		 * drawing and its veins wither away, and it is disposed of once the last of them is
		 * gone.
		 */
		bool IsDead;

		/*
		 * If the monster has already coughed a gas cloud out at this point of its attack
		 * animation, then this flag will be true. It keeps an open mouth from spawning a
		 * fresh cloud on every frame it is held for.
		 */
		bool IsToPuffGas;

		/*
		 * This is the number of cells this monster has covered with veins, weighed against
		 * the vein limit in the rules before it is allowed to spread any further.
		 */
		int VeinCount;

		/*
		 * This is the list of every veinhole monster in the game. The monsters are given
		 * their logic and drawn as a group off this list rather than through the usual
		 * object layers.
		 */
		static DynamicVectorClass<VeinholeMonsterClass *> VeinholeMonsters;

		enum {
			IDLE,       /// Closed, nothing is happening
			ALERT,      /// Something is close by, prepare
			ATTACKING,  /// Something is really close, or got attacked, attacking
			DYING,      /// Closing because it got killed
			STATE_COUNT,

			FRAME_COUNT = 12,
		};

	private:
		/*
		 * This is the artwork every veinhole monster is drawn with, refetched whenever the
		 * theater changes so that the monster suits the terrain around it.
		 */
		static void const * MonsterShape;
};
