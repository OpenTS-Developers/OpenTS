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

/* $Header: /CounterStrike/UNIT.H 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : UNIT.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 14, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 14, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "foot.h"

class Surface;

/****************************************************************************
**	For each instance of a unit (vehicle) in the game, there is one of
**	these structures. This structure holds information that is specific
**	and dynamic for a particular unit.
*/
class UnitClass : public FootClass
{
		typedef FootClass BASECLASS;

	public:

		/*
		 * This is the countdown that runs the firing animation of a shape based vehicle. It
		 * rests at -1 between shots, and a burst weapon may only loose its next round on the
		 * FiringSyncFrame of its type, so the recoil artwork and the shots stay in step.
		 */
		int FiringSyncDelay;

		/*
		**	This is the timer that controls the reload rate. The MSAM rocket
		**	launcher is the primary user of this.
		*/
		CDTimerClass<FrameTimerClass> Reload;

		/*
		**	This points to the static control data that gives 'this' unit its characteristics.
		*/
		UnitTypeClass * Class;

		/*
		 * This points to the next car back in a train -- the vehicle that follows this one.
		 * The cars are chained together through it so that the head of the train can drag
		 * the whole line along its path and halt it as one piece. If NULL, nothing follows.
		 */
		UnitClass * FollowingMe;

		/*
		**	This records the house flag that this object is currently carrying.
		*/
		HousesType Flagged;

		/*
		 * If this vehicle is a train car that is being pulled along by another, then this
		 * flag will be true. Only the head of a train hands out movement orders to the
		 * cars behind it, so a follower must not drag its own followers around in turn.
		 */
		bool IsFollowing;

		/*
		**	This flag is used for when the harvester dumps ore, to track its
		**	special animation.
		*/
		bool IsDumping;

		/*
		 * If this harvester is actively working a patch of Tiberium or veins, then this
		 * flag will be true. It is cleared by any mission that takes the harvester away
		 * from its patch, and it is what lays the digging animation over the vehicle.
		 */
		bool IsHarvesting;

		/*
		 * If this unit's artwork is being layered onto the shared eight bit scratch surface
		 * rather than drawn straight to the screen, then this flag will be true. It tells
		 * the drawing code to remap through the EightBitDrawer instead of the house scheme.
		 */
		mutable bool IsCompositingToEightBitSurface;

		/*
		 * This is the direction that a wandering visceroid prefers to keep crawling in. It
		 * stands a good chance of being reused for the next idle step, so the creature
		 * wanders in a line, and it is reset to FACING_NONE when its way is blocked.
		 */
		FacingType VisceroidFacing;

		/*
		 * This is the accumulated charge of a mobile EM pulse cannon. It climbs by one for
		 * every game frame that the vehicle is not immobilized, up to the MaxCharge of its
		 * type. The cannon refuses to deploy until it is full, and firing spends all of it.
		 */
		unsigned Charge;

		/*
		 * This is the frame counter that runs the death animation of a vehicle whose artwork
		 * provides one, and it holds -1 until the vehicle is destroyed. The wreck then
		 * lingers at one point of strength until it passes the MaxDeathCounter and explodes.
		 */
		int DeathCounter;

		/// Unused
		int Unused1;

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		UnitClass(UnitTypeClass const * type = NULL, HouseClass * house = NULL);
		virtual ~UnitClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;
		virtual HRESULT STDMETHODCALLTYPE Load(IStream * stream) override;
		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual char const * Full_Name(void) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;

		bool Goto_Clear_Spot(void);
		bool Try_To_Deploy(void);
		virtual void Scatter(Coord const &, bool forced=false, bool nokidding=false) override;

		bool Flag_Attach(HousesType house);
		bool Flag_Remove(void);
		bool Harvesting(void);
		void APC_Close_Door(void);
		void APC_Open_Door(void);

		/*
		**	Query functions.
		*/
		virtual bool Considered_Vehicle(void) const override;
		virtual bool Is_Ready_To_Move(void) const override;
		bool Should_Crush_It(TechnoClass const * it) const;
		int Credit_Load(void) const;
		virtual DirType Turret_Facing(void) const override;
		virtual int Pip_Count(void) const override;
		virtual InfantryTypeClass const * Crew_Type(void) const override;
		virtual DirType Fire_Direction(void) const override;
		virtual FireErrorType Can_Fire(AbstractClass * target, int which) const override;
		virtual double Tiberium_Load(void) const override;
		virtual double Weed_Load(void) const override;
		virtual bool Is_Immobilized(void) const override;
		virtual int Get_Max_Speed(void) const override;
		bool Is_Route_Broken(Cell const & from, Cell const & to) const;
		AbstractClass * Plan_Route(AbstractClass const & object) const;

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Limbo(void) override;
		virtual bool Unlimbo(Coord const & , Dir256 facing=DIR_N) override;
		virtual void Record_The_Kill(TechnoClass * ) override;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual void Set_Occupy_Bit(Coord const & coord) override;
		virtual void Clear_Occupy_Bit(Coord const & coord) override;
		virtual bool Render(Rect &, bool forced, bool extras_only) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual void Unit_Draw_Voxel(Point2D xyoff, Rect rect, int intensity) const;
		virtual void Unit_Draw_Shape(Point2D xyoff, Rect rect, int intensity) const;
		virtual void Unit_Blit_Voxel(Surface& surface, Point2D xyoff, Rect rect, int alpha) const;

		/*
		**	User I/O.
		*/
		virtual ActionType What_Action(Cell const &, bool check_fog = false, bool disallow_force = false) const override;
		virtual ActionType What_Action(ObjectClass const *, bool disallow_force = false) const override;
		virtual bool Active_Click_With(ActionType action, ObjectClass * object, bool) override;
		virtual bool Active_Click_With(ActionType , Cell const &, bool) override;

		/*
		**	Combat related.
		*/
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source=0, bool forced=false, bool=false) override;
		virtual BulletClass * Fire_At(AbstractClass * target, int which=0) override;
		virtual bool Captured(HouseClass * newowner) override;
		virtual bool Deploy_To_Fire(void) const override;
		void EMPulse_Blast(void);
		void Explode(void);

		/*
		**	AI.
		*/
		virtual AbstractClass * Greatest_Threat(ThreatType threat, Coord const & coord, bool) const override;
		virtual FacingType Desired_Load_Dir(ObjectClass * passenger, Cell & moveto) const override;
		virtual RadioMessageType Receive_Message(RadioClass * from, RadioMessageType message, intptr_t & param) override;
		virtual void AI(void) override;
		virtual bool Ready_To_Commence(void) override;
		virtual int Do_MISSION_ATTACK(void) override;
		virtual int Do_MISSION_GUARD_AREA(void) override;
		virtual int Do_MISSION_UNLOAD(void) override;
		virtual int Do_MISSION_GUARD(void) override;
		virtual int Do_MISSION_HARVEST(void) override;
		virtual int Do_MISSION_HUNT(void) override;
		virtual int Do_MISSION_REPAIR(void) override;
		virtual int Do_MISSION_MOVE(void) override;
		virtual int Do_MISSION_PATROL(void) override;
		void Rotation_AI(void);
		void Firing_AI(void);
		void Reload_AI(void);
		bool Edge_Of_World_AI(void);
		void Tunnel_AI(void);
		void Visceroid_AI(void);
		void Jellyfish_AI(void);

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif

		/*
		**	Movement and animation.
		*/
		virtual void Assign_Destination(AbstractClass * target, bool = true) override;
		virtual void Overrun_Square(Cell const & cell, bool threaten=true) override;
		virtual void Approach_Target(void) override;
		virtual bool Enter_Idle_Mode(bool initial=false, bool=true) override;
		virtual MoveType Can_Enter_Cell(CellClass const * cell, FacingType dir = FACING_NONE, int cell_height = -1, CellClass const * = 0, bool = true) const override;
		virtual void Per_Cell_Process(PCPType why) override;

		/*
		**	File I/O.
		*/
		static void Read_INI(CCINIClass const & ini);
		static void Write_INI(CCINIClass & ini);

	private:

		/*
		 * This is the list of animation stages to use when a visceroid attacks, indexed by
		 * the direction of its target.
		 */
		static int Visceroid_Fire_List[FACING_COUNT];

		/*
		**	The animation stage list for harvester loading up on ore.
		*/
		static int Harvester_Load_List[FACING_COUNT];

		/*
		 * This is the name of the scenario INI section that lists the vehicles to be
		 * placed upon the map when the scenario begins.
		 */
		static char const * const INI_NAME;
};

inline UnitClass * AbstractClass::As_UnitClass(void)
{
	return(dynamic_cast<UnitClass *>(this));
}


inline UnitClass const * AbstractClass::As_UnitClass(void) const
{
	return(dynamic_cast<UnitClass const *>(this));
}


extern Rect UnitCompositeDirtyRect;
