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

/* $Header: /CounterStrike/AIRCRAFT.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : AIRCRAFT.H                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : July 22, 1994                                                *
 *                                                                                             *
 *                  Last Update : November 28, 1994 [JLB]                                      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "foot.h"
#include "iflyctrl.h"

class AircraftTypeClass;


/*
**	This aircraft class is used for all flying sentient objects. This includes fixed wing
**	aircraft as well as helicopters. It excludes bullets even though some bullets might
**	be considered to be "flying" in a loose interpretatin of the word.
*/
class AircraftClass : public FootClass, public IFlyControl
{
		typedef FootClass BASECLASS;

	public:
		/*
		**	This is a pointer to the class control structure for the aircraft.
		*/
		AircraftTypeClass * Class;

		//-----------------------------------------------------------------------------
		AircraftClass(AircraftTypeClass const * type = NULL, HouseClass * house = NULL);
		virtual ~AircraftClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;
		virtual HRESULT STDMETHODCALLTYPE Load(IStream * stream) override;
		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID * ppvObject) override;
		virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
		virtual ULONG STDMETHODCALLTYPE Release(void) override;

		/*
		 * IFlyControl methods.
		 */
		virtual LONG STDMETHODCALLTYPE Landing_Altitude(void) override;
		virtual LONG STDMETHODCALLTYPE Landing_Direction(void) override;
		virtual BOOL STDMETHODCALLTYPE Is_Loaded(void) override;
		virtual LONG STDMETHODCALLTYPE Is_Strafe(void) override;
		virtual LONG STDMETHODCALLTYPE Is_Locked(void) override;

		virtual void Init(void) override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual bool Commence(void) override;
		virtual bool Ready_To_Commence(void) override;

		virtual int Do_MISSION_ATTACK(void) override;
		virtual int Do_MISSION_UNLOAD(void) override;
		virtual int Do_MISSION_HUNT(void) override;
		virtual int Do_MISSION_RETREAT(void) override;
		virtual int Do_MISSION_MOVE(void) override;
		virtual int Do_MISSION_ENTER(void) override;
		virtual int Do_MISSION_GUARD(void) override;
		virtual int Do_MISSION_GUARD_AREA(void) override;
		virtual int Do_MISSION_PATROL(void) override;

		virtual void Assign_Destination(AbstractClass * target, bool = true) override;

		/*
		**	Query functions.
		*/
		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual LayerType In_Which_Layer(void) const override;
		virtual bool Considered_Vehicle(void) const override;
		virtual DirType Turret_Facing(void) const override;
		virtual MoveType Can_Enter_Cell(CellClass const * cell, FacingType facing = FACING_NONE, int cell_height = -1, CellClass const * = 0, bool = true) const override;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual char const * Full_Name(void) const override;
		virtual ActionType What_Action(ObjectClass const *, bool disallow_force = false) const override;
		virtual ActionType What_Action(Cell const &, bool check_fog = false, bool disallow_force = false) const override;
		virtual FacingType Desired_Load_Dir(ObjectClass * passenger, Cell & moveto) const override;
		AbstractClass * Good_Fire_Location(AbstractClass * target) const;
		bool Cell_Seems_Ok(Cell const & cell, bool landing=false) const;
		Dir256 Pose_Dir(void) const;
		AbstractClass * Good_LZ(void) const;
		virtual DirType Fire_Direction(void) const override;
		virtual bool Can_Player_Fire(void) const override;
		virtual FireErrorType Can_Fire(AbstractClass * target, int which) const override;
		virtual bool Should_Delete_Off_Map(void) override;

		/*
		**	Landing zone support functionality.
		*/
		AbstractClass * New_LZ(AbstractClass * oldlz) const;

		/*
		**	Object entry and exit from the game system.
		*/
		virtual bool Unlimbo(Coord const & , Dir256 facing = DIR_N) override;

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		virtual void Look(bool incremental=false, bool=false) override;
		void Draw_Rotors(Point2D const & xy, Rect const & cliprect) const;
		virtual int Exit_Object(TechnoClass *) override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;

		/*
		**	User I/O.
		*/
		virtual bool Active_Click_With(ActionType action, ObjectClass * object, bool) override;
		virtual bool Active_Click_With(ActionType action, Cell const & cell, bool) override;
		virtual void Player_Assign_Mission(MissionType mission, AbstractClass * target=NULL, AbstractClass * destination=NULL) override;

		/*
		**	Combat related.
		*/
		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced=false, bool=false) override;
		virtual BulletClass * Fire_At(AbstractClass * target, int which) override;
		virtual void Reduce_Ammunition(void) override;
		bool Crash(TechnoClass * source);

		/*
		**	AI.
		*/
		int Paradrop_Cargo(void);
		void Drop_Off_Cargo(void);
		virtual void AI(void) override;
		virtual bool Enter_Idle_Mode(bool initial = false, bool = true) override;
		virtual RadioMessageType Receive_Message(RadioClass * from, RadioMessageType message, intptr_t & param) override;
		virtual void Scatter(Coord const & threat, bool forced=false, bool nokidding=false) override;

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

	public:

		/*
		 * If this aircraft has fired during its current attack run, then this flag will be
		 * true. The ammunition is deducted when the run ends rather than at each shot, so a
		 * strafing pass costs one round however many shots it fires.
		 */
		bool IsToSpendAmmo;

		/*
		**	If this is a passenger carrying aircraft then this flag will be set. This is
		**	necessary because once the passengers are unloaded, the fact that it was a
		**	passenger carrier must still be known.
		*/
		bool Passenger;

		/// Unused
		bool IsKamikaze;

		/// Unused
		bool field_35B;

		/*
		 * If this aircraft is committed to a strafing run, then this flag will be true. It
		 * is reported to the locomotor through Is_Locked so the aircraft will hold its
		 * heading, and it keeps the aircraft from commencing a new order until the run ends.
		 */
		bool IsLockedStraight;

	private:
		/*
		**	This timer controls when the aircraft will reveal the terrain around itself.
		**	When this timer expires and this aircraft has a sight range, then the
		**	look around process will occur.
		*/
		CDTimerClass<FrameTimerClass> SightTimer;

		/*
		**	Most attack aircraft can make several attack runs. This value contains the
		**	number of attack runs the aircraft has left. When this value reaches
		**	zero then the aircraft is technically out of ammo.
		*/
		int AttacksRemaining;

		/*
		 * If the aircraft is at a good point to change orders, then this
		 * flag will be set to true.
		 */
		bool IsReadyToCommence;

		int Do_MISSION_MOVE_Carryall(void);
		int Do_MISSION_MOVE_Normal(void);

		/*
		 * This is the name of the scenario INI section that lists the aircraft to be
		 * placed upon the map when the scenario begins.
		 */
		static char const * const INI_NAME;
};


inline AircraftClass * AbstractClass::As_AircraftClass(void)
{
	return(dynamic_cast<AircraftClass *>(this));
}


inline AircraftClass const * AbstractClass::As_AircraftClass(void) const
{
	return(dynamic_cast<AircraftClass const *>(this));
}


bool Counts_As_Civ_Evac(ObjectClass const * candidate);
