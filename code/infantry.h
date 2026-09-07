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

/* $Header: /CounterStrike/INFANTRY.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : INFANTRY.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 15, 1994                                              *
 *                                                                                             *
 *                  Last Update : August 15, 1994   [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "_infantr.h"
#include "foot.h"

#include "do.hh"
#include "fear.hh"


class InfantryClass : public FootClass
{
		typedef FootClass BASECLASS;

	public:
		InfantryTypeClass * Class;

		/*
		**	If the infantry is undergoing some choreographed animation sequence, then
		**	this holds the particular sequence number. The frame of animation is kept
		**	track of by the regular frame tracking system. When performing an animation
		**	sequence, the infantry cannot perform anything else (even move).
		*/
		DoType Doing;

		/*
		**	Certain infantry will either perform some comment or say something after an
		**	amount of time has expired subsequent to an significant event. This is the
		**	timer the counts down.
		*/
		CDTimerClass<FrameTimerClass> Comment;

		/*
		**	The fear rating of this infantry unit. The more afraid the infantry, the more
		**	likely it is to panic and seek cover.
		*/
		FearType Fear;

		/*
		 * If this infantry has gone berzerk, then this flag will be true. A berzerk soldier
		 * no longer spares its allies when picking targets, so it attacks whatever is nearby.
		 */
		bool IsBerzerk;

		/*
		**	If this civilian is actually a technician, then this flag will be true.
		**	It should only be set for the civilian type infantry. Typically, the
		**	technician appears after a building is destroyed.
		*/
		bool IsTechnician;

		/*
		**	If the infantry just performed some feat, then it may respond with an action.
		**	This flag will be true if an action is to be performed when the Comment timer
		**	has expired.
		*/
		bool IsStoked;

		/*
		**	This flag indicates if the infantry unit is prone. Prone infantry become that way
		**	when they are fired upon. Infantry in the prone position are less vulnerable to
		**	combat.
		*/
		bool IsProne;

		/*
		**	If the infantry is allowed to move one cell from one zone to another, then this
		**	flag will be true. It exists only so that when a bridge is destroyed, the bomb
		**	placer is allowed to run from the destroyed bridge cell back onto a real cell.
		*/
		bool IsZoneCheat;

		/*
		**	This flag is set for the dogs, when they launch into bullet mode.
		**	it's to remember if the unit was selected, and if it was, then
		**	when the dog is re-enabled, he'll reselect himself.
		*/
		bool WasSelected;

		/*
		 * This is how long this infantry remains paralyzed by a webbing warhead, in game
		 * frames. While it counts down the infantry cannot move and struggles in place.
		 */
		CDTimerClass<FrameTimerClass> ProneStruggleTimer;

		/*
		 * This is the countdown between map reveals for a moving infantry. Revealing the
		 * shroud is expensive, so a walking soldier only looks around once a second.
		 */
		CDTimerClass<FrameTimerClass> LookTimer;

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		InfantryClass(InfantryTypeClass const * type = NULL, HouseClass * house = NULL);
		virtual ~InfantryClass(void) override;

		virtual ClassID Class_ID(void) const override;
		virtual HRESULT Load(SaveStreamClass & stream) override;
		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		virtual void Init(void) override;

		virtual void Assign_Destination(AbstractClass *, bool=true) override;

		/*
		**	Query functions.
		*/
		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual bool Is_Ready_To_Random_Animate(void) const override;
		virtual void const * Get_Image_Data(void) const override;
		int Shape_Number(void) const;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual const char * Full_Name(void) const override;

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Unlimbo(Coord const & coord, Dir256 facing) override;
		virtual bool Paradrop(Coord const & coord) override;
		virtual bool Limbo(void) override;
		virtual void Detach(AbstractClass const * target, bool all) override;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;

		/*
		**	User I/O.
		*/
		virtual bool Active_Click_With(ActionType action, ObjectClass * object, bool) override;
		virtual bool Active_Click_With(ActionType action, Cell const & cell, bool) override;

		/*
		**	Combat related.
		*/
		virtual ActionType What_Action(ObjectClass const *, bool disallow_force = false) const override;
		virtual ActionType What_Action(Cell const &, bool check_fog = false, bool disallow_force = false) const override;
		virtual BulletClass * Fire_At(AbstractClass * target, int which) override;
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source=0, bool forced=false, bool=false) override;
		virtual FireErrorType Can_Fire(AbstractClass * target, int which) const override;
		virtual void Assign_Target(AbstractClass *) override;
		virtual void Set_Occupy_Bit(Coord const & coord) override;
		virtual void Clear_Occupy_Bit(Coord const & coord) override;
		virtual bool Is_Renovator(void) const override;

		virtual void On_Movement_Blocked(void) override;
		virtual bool JumpJet_To_Walk(void) override;

		/*
		**	Driver control support functions. These are used to control cell
		**	occupation flags and driver instructions.
		*/
		virtual bool Stop_Driver(void) override;
		virtual bool Start_Driver(Coord & coord) override;

		/*
		**	AI.
		*/
		virtual void AI(void) override;
		void Fear_AI(void);
		virtual AbstractClass * Greatest_Threat(ThreatType threat, Coord const & coord, bool) const override;
		virtual bool Ready_To_Commence(void) override;
		virtual int Do_MISSION_ATTACK(void) override;
		virtual int Do_MISSION_GUARD(void) override;
		virtual void Berzerk(void) override;
		virtual void Start_Fear(void) override;
		virtual void Stop_Fear(void) override;
		virtual void Do_Idle(int which) override;
		bool Edge_Of_World_AI(void);
		void Firing_AI(void);
		void Doing_AI(void);
		void Movement_AI(void);
		void Tunnel_AI(void);
		bool Theft_AI(void);

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass const & ini);
		static void Write_INI(CCINIClass & ini);

		/*
		**	Movement and animation.
		*/
		virtual bool Do_Action(DoType todo, bool force=false, bool randomize=false);
		virtual bool Random_Animate(void) override;
		virtual MoveType Can_Enter_Cell(CellClass const * cell, FacingType dir = FACING_NONE, int cell_height = -1, CellClass const * = 0, bool = true) const override;
		virtual void Per_Cell_Process(PCPType why) override;
		virtual bool Enter_Idle_Mode(bool initial=false, bool=true) override;
		virtual void Scatter(Coord const & threat, bool forced=false, bool nokidding=false) override;
		virtual Coord Turret_Coord(int which=0) const override;
		virtual bool Is_Immobilized(void) const override;
		virtual int Current_Speed(void) override;
		virtual void Approach_Target(void) override;
		virtual void Stop_Movement_Animation(void) override;
		bool Is_JumpJet(void) const;
		bool Should_JumpJet_Fly(Cell const & from, Cell const & to);

		/*
		**	Translation table to convert facing into infantry shape number. This special
		**	table is needed since several facing stages are reused and flipped about the Y
		**	axis.
		*/
		static int const HumanShape[32];

	private:

		static DoStruct const MasterDoControls[DO_COUNT];

		/*
		 * This is the name of the scenario INI section that the preplaced infantry are
		 * read from and written back out to.
		 */
		static char const * const INI_Name;
};


inline InfantryClass * AbstractClass::As_InfantryClass(void)
{
	return(dynamic_cast<InfantryClass *>(this));
}


inline InfantryClass const * AbstractClass::As_InfantryClass(void) const
{
	return(dynamic_cast<InfantryClass const *>(this));
}
