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

#include "abstype.h"
#include "coord.h"
#include "ftimer.h"
#include "nodes.h"
#include "priority.h"
#include "timer.h"
#include "typelist.h"

#include "tiberium.hh"

class CCINIClass;
class OverlayTypeClass;
class AnimTypeClass;


class TiberiumClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:

		TiberiumClass(char const * ininame = NULL);
		virtual ~TiberiumClass() override;

		virtual ClassID Class_ID(void) const override;
		virtual bool Load(SaveStreamClass & stream) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual RTTIType Fetch_RTTI(void) const override { return(RTTI_TIBERIUM); }
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override { return(HeapID); }
		virtual bool Read_INI(CCINIClass const & ini) override;

		static bool Process(CCINIClass const & ini);

		static void Tiberium_Spread(void);
		static void Init_Tiberium_Spread_System(void);
		static void Deinit_Tiberium_Spread_System(void);

		void Spread_AI(void);
		void Init_Spread(void);
		void Recalc_Spread(void);
		void Clear_Spread(void);
		void Queue_Spread(Cell const & cell);

		static void Clear_Spread_State(Cell const & cell);

		static void Tiberium_Growth(void);
		static void Init_Tiberium_Growth_System(void);
		static void Deinit_Tiberium_Growth_System(void);

		void Growth_AI(void);
		void Init_Growth(void);
		void Recalc_Growth(void);
		void Clear_Growth(void);
		void Queue_Growth(Cell const & cell);

		static void Post_Load_Game(void);

		/*
		 * This is this tiberium type's own index into the Tiberiums heap, assigned as the type
		 * is created. It is the value a cell records to say which tiberium grows there.
		 */
		TiberiumType HeapID;

		/*
		 * This is the number of game frames that must elapse between spread passes for this
		 * tiberium. It is reloaded into the SpreadTimer at the end of every pass.
		 */
		int SpreadDelay;

		/*
		 * This is the share of the enqueued cells that a single spread pass will seed from.
		 * If it is zero, then this tiberium never spreads at all.
		 */
		double SpreadPercentage;

		/*
		 * This is the number of game frames that must elapse between growth passes for this
		 * tiberium. It is reloaded into the GrowthTimer at the end of every pass, shortened
		 * when the scenario has the accelerated tiberium growth option set.
		 */
		int GrowthDelay;

		/*
		 * This is the share of the enqueued cells that a single growth pass will ripen. If it
		 * is zero, then this tiberium never grows past the stage it was placed at.
		 */
		double GrowthPercentage;

		/*
		 * This is the number of credits that one unit of this tiberium is worth once refined.
		 * A cell is worth this much for every growth stage it carries.
		 */
		int CreditValue;

		/*
		 * This is the damage that one unit of this tiberium does when it is set off -- by a
		 * chain reaction in the field, by poisoning an infantryman standing in it, or by going
		 * up along with the refinery it was stored in.
		 */
		int Power;

		/*
		 * This is the color scheme that the tiberium overlay and its debris animations are
		 * remapped through, which is what gives each tiberium type its own hue.
		 */
		int Color;

		/*
		 * These are the animations thrown off when this tiberium is blasted out of a cell. One
		 * of them is picked at random and drawn in the tiberium's own color.
		 */
		TypeList<AnimTypeClass const *> Debris;

		/*
		 * This points to the first of the run of overlay types that this tiberium is drawn
		 * with. The Variety and RampVariety counts say how many follow it.
		 */
		OverlayTypeClass const * Overlay;

		/*
		 * This is the number of growth stages the tiberium artwork carries. A cell stops
		 * ripening once it reaches the last one.
		 */
		int FrameCount;

		/*
		 * This is the number of overlay types, starting at the Overlay, that hold the flat
		 * ground artwork. One is picked per cell so that a field does not look uniform.
		 */
		int Variety;

		/*
		 * This is the number of overlay types that follow the flat ones and hold the sloped
		 * ground artwork, four ramp directions' worth of them. If it is zero, then this
		 * tiberium cannot take root on a slope at all.
		 */
		int RampVariety;

		/*
		 * This is the number of records handed out of the SpreadNodes pool so far. The pool is
		 * never recycled, so the queue is rebuilt from the map once the pool runs low.
		 */
		int SpreadCount;

		/*
		 * This is the queue of cells waiting to seed their neighbors, ordered by the game
		 * frame at which each becomes due.
		 */
		PriorityQueueClass<CellNode> * SpreadQueue;

		/*
		 * This is one flag per map cell, true while the cell is sitting in the SpreadQueue. It
		 * keeps a cell from being enqueued twice over.
		 */
		bool * SpreadState;

		/*
		 * This is the pool of queue records the SpreadQueue is built out of -- one per map
		 * cell, handed out in order by the SpreadCount cursor.
		 */
		CellNode * SpreadNodes;

		/*
		 * This counts down the frames remaining until this tiberium's next spread pass.
		 */
		CDTimerClass<FrameTimerClass> SpreadTimer;

		/*
		 * This is the number of records handed out of the GrowthNodes pool so far. The pool is
		 * never recycled, so the queue is rebuilt from the map once the pool runs low.
		 */
		int GrowthCount;

		/*
		 * This is the queue of cells waiting to ripen, ordered by the game frame at which each
		 * becomes due.
		 */
		PriorityQueueClass<CellNode> * GrowthQueue;

		/*
		 * This is one flag per map cell, true while the cell is sitting in the GrowthQueue. It
		 * keeps a cell from being enqueued twice over.
		 */
		bool * GrowthState;

		/*
		 * This is the pool of queue records the GrowthQueue is built out of -- one per map
		 * cell, handed out in order by the GrowthCount cursor.
		 */
		CellNode * GrowthNodes;

		/*
		 * This counts down the frames remaining until this tiberium's next growth pass.
		 */
		CDTimerClass<FrameTimerClass> GrowthTimer;
};

extern DynamicVectorClass<TiberiumClass *> Tiberiums;
