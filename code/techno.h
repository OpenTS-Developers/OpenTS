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

/* $Header: /CounterStrike/TECHNO.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : TECHNO.H                                                     *
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

#include "_voxel.h"
#include "cargo.h"
#include "door.h"
#include "facing.h"
#include "flasher.h"
#include "ftimer.h"
#include "globals.h"
#include "house.h"
#include "matrix3d.h"
#include "point.h"
#include "radio.h"
#include "stage.h"
#include "stbuffer.h"
#include "storage.h"
#include "techtype.h" /// This class uses TechnoTypeClass members inline, so the full definition is needed.
#include "timer.h"
#include "veteran.h"

#include "cloak.hh"
#include "detected.hh"
#include "draw.hh"
#include "fire.hh"
#include "infantry.hh"
#include "particle.hh"
#include "talk.hh"
#include "threat.hh"
#include "zgrad.hh"

#include <cstdint>


class ParticleSystemClass;
class WaveClass;
class WeaponTypeClass;
class ShapeSet;
struct WeaponDataStruct;


/****************************************************************************
**	This is the common data between building and units.
*/
class TechnoClass :	public RadioClass,
					public FlasherClass,
					public StageClass
{
		typedef RadioClass BASECLASS;

	public:
		enum {
			RAILGUN_STEPS = 50,
		};

		/*
		**	This is the house that originally owned this factory. Objects buildable
		**	by this house type will be produced from this factory regardless of who
		**	the current owner is.
		*/
		HousesType ActLike;
		CargoClass Cargo;

		VeterancyClass Veterancy;

		/*
		 * These are this object's own armor and firepower multipliers. They are 1.0 unless a
		 * crate powerup has changed them, and they scale on top of the house's biases of the
		 * same name, so a crated unit is tougher or hits harder than others of its type.
		 */
		double ArmorBias;
		double FirepowerBias;

		/*
		**	Idle animations (if any are supported by the object type) are regulated by
		**	this timer. When the timer expires an idle animation occurs. Then the
		**	timer is reinitialized to some random (bounded) setting.
		*/
		CDTimerClass<FrameTimerClass> IdleTimer;

		/*
		 * This is the countdown that blinks this object on the radar after it takes damage, so
		 * the player can see at a glance where he is being attacked.
		 */
		CDTimerClass<FrameTimerClass> RadarFlashTimer;

		/*
		 * This is where this object last plotted on the radar, in radar pixels. The radar's
		 * tracking table is keyed by it, so a moved object untracks here before plotting anew.
		 */
		Point2D RadarPos;

		/*
		**	This is a list of bits of which houses are spying on this building,
		**	if in fact this is a building.
		*/
		unsigned SpiedBy;

		/*
		**	If this object is part of a pseudo-team that the player is managing, then
		**	this will be set to the team number (0 - 9). If it is not part of any
		**	pseudo-team, then the number will be -1.
		*/
		int Group;

		/*
		**	For units in area guard mode, this is the recorded home position. The guarding
		**	unit will try to stay near this location in the course of it's maneuvers. This is
		**	also used to record a pending transport for those passengers that are waiting for
		**	the transport to become available. It is also used by harvesters so that they know
		**	where to head back to after unloading.
		*/
		AbstractClass * ArchivedTarget;

		/*
		**	This is the house that the unit belongs to.
		*/
		HouseClass * House;

		/*
		**	This records the current cloak state for this vehicle.
		*/
		CloakType Cloak;
		StageClass CloakingDevice;
		CDTimerClass<FrameTimerClass> CloakDelay;

		/*
		 * This shifts where this object samples the "predator" distortion pattern it is drawn
		 * with while cloaked, so that two cloaked objects do not shimmer in step.
		 */
		float PredatorOffset;

		/* (Targeting Computer)
		**	This is the target value for the item that this vehicle should ATTACK. If this
		**	is a vehicle with a turret, then it may differ from its movement destination.
		*/
		AbstractClass * TarCom;
		AbstractClass * SuspendedTarCom;

		/*
		 * This is the nose-up pitch of this aircraft, expressed in radians. A dropship pitches
		 * up as it flares to a stop, and counts as still moving until it eases level again.
		 */
		float PitchAngle;

		/*
		**	This is the arming countdown. It represents the time necessary
		**	to reload the weapon.
		*/
		CDTimerClass<FrameTimerClass> Arm;

		/*
		**	The number of shot this object can fire before running out of ammo. If this
		**	value is zero, then firing is not allowed. If -1, then there is no ammunition
		**	limit.
		*/
		int Ammo;

		/*
		**	This is the amount of money spent to produce this object. This value really
		**	only comes into play for the case of buildings that have special "free"
		**	objects available when purchased at the more expensive rate.
		*/
		int PurchasePrice;

		/*
		 * These are the particle systems attached to this object -- one slot per kind, such as
		 * its damage smoke or the trail of its railgun. An occupied slot blocks another system
		 * of that kind, and for a weapon effect it also holds off the weapon that spawned it.
		 */
		ParticleSystemClass * ParticleSystems[ATTACHED_PARTICLE_COUNT];

		/*
		 * This is the sonic wave this object has in flight. A sonic weapon cannot fire again
		 * while its wave lives, so an object only ever has one of them in the air at a time.
		 */
		WaveClass * Wave;

		/*
		 * These are the angles this object is presently tipped at, expressed in radians --
		 * sideways rolls it about its length, forwards pitches it nose down. The rocking rates
		 * advance them each frame, and the object is drawn tipped to match.
		 */
		float AngleRotatedSideways;
		float AngleRotatedForwards;

		/*
		 * These are the rates at which the object is presently tipping, expressed in radians of
		 * rotation per game frame. Something jolting the object sets them, and each frame they
		 * are eased back toward zero so that the rocking dies away of its own accord.
		 */
		float RockingSidewaysPerFrame;
		float RockingForwardsPerFrame;

		/*
		 * If infantry has taken this object over, then this records the type of infantry that
		 * entered it. The same soldier steps back out if the object is later destroyed, and a
		 * hijacked vehicle counts against that infantry type's build limit while it lives.
		 */
		InfantryType EnteredByInfType;

		/*
		**	This is a count of the # of loads of the various minerals that the
		**	unit has harvested.
		*/
		StorageClass Storage;

		/*
		 * This is the door, hatch, or bay of this object. It swings open and shut at the rate
		 * the object's type specifies, and whatever is trying to get out or in waits until this
		 * reports that the door has finished moving.
		 */
		DoorClass Door;

		/*
		 * This is the elevation of this object's gun barrel, which swings toward a desired
		 * pitch at its own rate just as a turret turns. Level is DIR_E, and artillery lowers
		 * the barrel back to its type's StartPitch once it has stopped firing.
		 */
		FacingClass BarrelPitch;

		/*
		**	This is the visible facing for the unit or building.
		*/
		FacingClass PrimaryFacing;

		/*
		**	This is the facing of the turret. It can be, and usually is,
		**	rotated independently of the body it is attached to.
		*/
		FacingClass SecondaryFacing;

		/*
		 * This is the index of the next shot within a burst, for those units or buildings that
		 * fire several shots in quick succession. It counts up as each shot leaves and wraps
		 * back to zero at the weapon's burst count. While it is part way through a burst the
		 * short burst rearm timing is used rather than the regular rate of fire, and alternate
		 * shots are fired from the opposite side of the barrel.
		 */
		int BurstIndex;

		/*
		 * Losing a target partway through a burst starts this countdown. The burst keeps its
		 * next-shot index until a new target is acquired or the full rearm interval expires.
		 */
		bool IsBurstResetPending;
		CDTimerClass<FrameTimerClass> BurstResetTimer;

		/*
		 * This is the countdown that keeps the targeting laser drawn from this object to
		 * whatever it is shooting at. It is restarted with every shot, but only for the object
		 * types that sport such a laser and only while the player is in control of them.
		 */
		CDTimerClass<FrameTimerClass> TargetingLaserTimer;

		/*
		 * This is the index used to pick which of a weapon's several firing sounds this object
		 * plays. It is rolled once when the object is created and never rerolled, so each
		 * object keeps one voice out of the selection instead of every one sounding alike.
		 */
		unsigned short SoundRandomSeed;

		/*
		 * This is the screen row below which a sinking object is clipped away, recalculated as
		 * it is drawn. Cutting the image off rather than shrinking it is what makes the object
		 * appear to slide beneath the surface.
		 */
		short SinkingYOffset;

		/*
		 * If this object is sinking out of sight, as a vehicle does when it breaks through the
		 * ice, then this flag will be true. A sinking object keels forward, is clipped off at
		 * the waterline, and is stunned so that it takes no further orders on the way down.
		 */
		bool IsSinking;

		/*
		 * If this object should call the rest of its base to its aid when attacked, then this flag
		 * will be true. It is the per-object counterpart of the type's IsToProtect.
		 */
		bool IsNeedingRescue;

		/*
		**	If this techno object has detected that it has outlived its
		**	purpose, then this flag will be true. Such object will either
		**	be sold or sacrificed at the first opportunity.
		*/
		bool IsUseless;

		/*
		**	This flag will be true if the object has been damaged with malice.
		**	Damage received due to friendly fire or wear and tear does not count.
		**	The computer is not allowed to sell a building unless it has been
		**	damaged with malice.
		*/
		bool IsTickedOff;

		/*
		**	If this object has inherited the ability to cloak, then this bit will
		**	be set to true.
		*/
		bool IsCloakable;

		/*
		**	If this object is designated as special then this flag will be true. For
		**	buildings, this means that it is the primary factory. For units, it means
		**	that the unit is the team leader.
		*/
		bool IsLeader;

		/*
		**	Certain units are flagged as "loaners".  These units are typically transports that
		**	are created solely for the purpose of delivering reinforcements.  Such "loaner"
		**	units are not owned by the player and thus cannot be directly controlled.  These
		**	units will leave the game as soon as they have fulfilled their purpose.
		*/
		bool IsALoaner;

		/*
		**	Once a unit enters the map, then this flag is set. This flag is used to make
		**	sure that a unit doesn't leave the map once it enters the map.
		*/
		bool IsLocked;

		/*
		**	Buildings and units with turrets usually have a recoil animation when they
		**	fire. If this flag is true, then the next rendering of the object will be
		**	in the "recoil state". The flag will then be cleared pending the next
		**	firing event.
		*/
		bool IsInRecoilState;

		/*
		**	If this unit is "loosely attached" to another unit it is given special
		**	processing. A unit is in such a condition when it is in the process of
		**	unloading from a transport type object. During the unloading process
		**	the transport object must stay still until the unit is free and clear.
		**	At that time it radios the transport object and the "tether" is broken -
		**	freeing both the unit and the transport object.
		*/
		bool IsTethered;

		/*
		**	Is this object owned by the player?  If not, then it is owned by the computer
		**	or remote opponent. This flag facilitates the many logic differences when dealing
		**	with player's or computer's units or buildings.
		*/
		bool IsOwnedByPlayer;

		/*
		**	The more sophisticated game objects must keep track of whether they are discovered
		**	or not. This is because the state of discovery can often control how the object
		**	behaves. In addition, this fact is used in radar and user I/O processing.
		*/
		bool IsDiscoveredByPlayer;

		/*
		**	This is used to control the computer recognizing this object.
		*/
		bool IsDiscoveredByComputer;

		/*
		**	Some game objects can be of the "lemon" variety. This means that they take damage
		**	even when everything is ok. This adds a little variety to the game.
		*/
		bool IsALemon;

		/// Unused
		unsigned char UnusedCooldown;

		/// Unused
		unsigned char Unused1;

		/*
		 * This is the bonus added to this object's sight range, expressed as a percentage of
		 * the range its type normally has. An object standing high up can see further, so the
		 * bonus is derived from its height each time it looks, and a bonus that has grown since
		 * the last look forces a full reveal rather than an incremental one.
		 */
		char SightIncrease;

		/*
		 * These flags control whether this object may be drafted into a team. Both are true by
		 * default and both travel with the object in the scenario, so a map can hold a
		 * particular unit back from teams without changing its type.
		 */
		bool IsTeamRecruitable;			/// For teams built from a team type.
		bool IsAutocreateRecruitable;	/// For teams the computer autocreates.

		/*
		 * If this object is currently listed in the radar's tracking table, then this flag will
		 * be true. That table is what lets the radar tell what is standing at a given radar
		 * pixel, and this flag keeps an object from being entered or removed from it twice.
		 */
		bool IsRadarTracked;

		/*
		 * If this unit is being carried by a transport, then this flag will be true. The unit
		 * is in "limbo" while it rides, so the flag serves to lift its shadow clear of the
		 * transport carrying it.
		 */
		bool IsInTransport;

		/*
		 * If this object is tumbling freely, as a shot down aircraft does on its way to the
		 * ground, then this flag will be true. While it is set the rocking angles are advanced
		 * without the clamping and damping that would otherwise settle the object level again.
		 */
		bool IsRocking;

		/*
		 * If this object is carrying out the patrol mission, then this flag will be true. It
		 * is cleared when the player hands the object an ordinary waypoint path, which is how
		 * a patrol is broken off in favor of a one way trip.
		 */
		bool IsOnPatrol;

		/*
		 * If the waypoint path this object is following is a patrol loop rather than a one
		 * shot route, then this flag will be true. It keeps the patrol cursor offered over the
		 * map so that further legs can be added without holding the modifier keys down.
		 */
		bool IsOnWaypointPatrol;

		/*
		 * This is the building this object still means to enter, remembered while it moves to
		 * a spot nearby because the bay was busy when it asked. The object tries again once it
		 * has stopped moving, and the reference is dropped once it gets in.
		 */
		ObjectClass * NearbyObject;

		/*
		 * This is the number of game frames this object remains stunned, counted down one per
		 * frame. A stunned object cannot move, fire, or produce, which is how an EM pulse and
		 * a plunge through the ice take their victims out of the fight.
		 */
		int StunDuration;

		/*
		 * This is a list of bits of which houses have attached a "limpet" drone to this
		 * object. A house that has limpeted an object sees whatever that object sees, so the
		 * drone serves as a spy that travels along with its victim.
		 */
		unsigned LimpetType;

		/*
		 * This is the speed penalty imposed by an attached limpet drone, expressed as a
		 * fraction of the object's normal speed. It slows the rate at which the body and
		 * turret turn as well, so a limpeted vehicle is sluggish as well as watched.
		 */
		float LimpetSpeedFactor;

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		TechnoClass(HouseClass* house=0);
		virtual ~TechnoClass(void) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		/*
		**	Query functions.
		*/
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual TechnoTypeClass const * Techno_Type_Class(void) const override;

		virtual bool Is_Move_Override(void) const;
		virtual bool Is_Allowed_To_Recloak(void) const;
		virtual bool Can_Scatter(void) const;
		virtual bool Is_In_Team(void) const;
		virtual bool Should_Self_Heal_Now(void) const;
		virtual bool Is_Voxel_Loaded(void) const;
		virtual bool Is_Ready_To_Move(void) const;
		virtual bool Is_Ready_To_Cloak(void) const;
		virtual bool Should_Uncloak(void) const;
		virtual DirType Turret_Facing(void) const;
		virtual bool Is_Weapon_Equipped(void) const;
		virtual bool Is_On_Elevation(void) const;
		virtual double Tiberium_Load(void) const;
		virtual double Weed_Load(void) const;
		virtual int Pip_Count(void) const;
		virtual int Refund_Amount(void) const;
		virtual int Risk(void) const;
		virtual bool Is_In_Same_Zone_As(ObjectClass const * object) const;
		virtual DirType Barrel_Pitch(AbstractClass * target) const;
		virtual bool Is_In_Same_Zone(Coord const & coord) const;

		virtual int How_Many_Survivors(void) const;
		virtual void Scatter_Incoming_Infantry(void) const;
		int What_Weapon_Should_I_Use(AbstractClass * target) const;
		virtual int Get_Collateral_Damage(void) const;
		virtual int Get_Z_Adjust(void) const;
		bool Is_Z_Fudge_Bridge(void) const;
		bool Was_Z_Fudge_Bridge(void) const;
		int Get_Z_Fudge_Column(void) const;
		int Get_Z_Fudge_Tunnel(void) const;
		int Get_Z_Fudge_Cliff(void) const;
		virtual ZGradientType Get_Z_Gradient(void) const {return(ZGRAD_90DEG);}

		virtual BuildingClass * Find_Docking_Bay(BuildingTypeClass const * b, bool friendly = false, bool evenoccupied = false) const;
		virtual Cell Find_Exit_Cell(TechnoClass const * techno) const;
		virtual Coord Turret_Coord(int which=0) const;
		virtual FacingType Desired_Load_Dir(ObjectClass * , Cell & moveto) const;
		virtual DirType Fire_Direction(void) const;
		virtual InfantryTypeClass const * Crew_Type(void) const;

		virtual bool Can_Attack_Now(void) const;
		virtual bool Can_Deploy_Now(void) const;
		virtual bool Is_Immobilized(void) const;
		virtual int Get_Max_Speed(void) const;
		virtual int Rearm_Delay(int which=0) const;
		virtual int Threat_Range(int control) const;
		virtual bool Is_Allowed_To_Leave_Map(void) const;

		virtual bool Is_Radar_Visible(DetectedType & detected) const;
		virtual bool Is_Sensed_By_Player(void) const;
		virtual bool Is_Sensed_By_House(HouseClass const * house) const;
		virtual bool Is_Renovator(void) const;
		virtual void Advance_Waypoint_Path(void);

		bool Is_Allowed_To_Retaliate(TechnoClass const * source, WarheadTypeClass const * warhead) const;
		virtual bool Is_Players_Army(void) const override;
		int Combat_Damage(int which=-1) const;
		ThreatType Heal_Threats(void) const;
		bool Can_Heal(ObjectClass const * object) const;

		Cell Nearby_Location(TechnoClass const * from=NULL) const;
		//bool Is_Visible_On_Radar(void) const;
		int Anti_Air(void) const;
		int Anti_Armor(void) const;
		int Anti_Infantry(void) const;
		int Time_To_Build(void) const;
		virtual ActionType What_Action(ObjectClass const *, bool disallow_force = false) const override;
		virtual ActionType What_Action(Cell const &, bool check_fog = false, bool disallow_force = false) const override;
		virtual Coord Fire_Coord(int which) const override;

		virtual HousesType Owner(void) const override;
		virtual HouseClass * Owner_HouseClass(void) const override;
		virtual bool Can_Player_Fire(void) const override;
		virtual bool Can_Player_Move(void) const override;
		virtual bool Can_Repair(void) const override;
		virtual int Value(void) const override;
		virtual int Get_Ownable(void) const override;
		virtual VisualType Visual_Character(bool raw = false, HouseClass const * = NULL) const override;

		/*
		**	User I/O.
		*/
		virtual void Clicked_As_Target(int count = 7) override;
		virtual bool Select(void) override;
		virtual void Response_Select(void);
		virtual void Response_Move(void);
		virtual void Response_Attack(void);
		virtual void Player_Assign_Mission(MissionType order, AbstractClass * target=NULL, AbstractClass * destination=NULL);
		Coord Predict_Target_Coord(void) const;

		/*
		**	Combat related.
		*/
		double Area_Modify(Cell const & cell) const;
		void Base_Is_Attacked(TechnoClass const * enemy);
		void Kill_Cargo(TechnoClass * source);
		virtual void Record_The_Kill(TechnoClass * source) override;
		virtual void Reduce_Ammunition(void);
		virtual bool Target_Something_Nearby(Coord const & coord, ThreatType threat=THREAT_NORMAL);
		virtual void Stun(void);
		virtual bool In_Range(Coord const & coord, int which=0) const override;
		virtual bool In_Range(AbstractClass * target, int which=0) const;
		virtual void Death_Announcement(TechnoClass const * source=0) const = 0;
		virtual FireErrorType Can_Fire(AbstractClass * target, int which=0) const;
		virtual AbstractClass * Greatest_Threat(ThreatType threat, Coord const & coord, bool) const;
		virtual void Assign_Target(AbstractClass * target);
		virtual void Override_Mission(MissionType mission, AbstractClass * tarcom, AbstractClass * navcom) override;
		virtual bool Restore_Mission(void) override;
		virtual BulletClass * Fire_At(AbstractClass * target, int which=0);
		virtual int Weapon_Range(int which) const override;
		virtual bool Captured(HouseClass * newowner);
		void Set_Owner(HouseClass * newowner);
		virtual void Laser_Zap(AbstractClass * target, int which, WeaponTypeClass const * weapon, Coord const & source_coord);
		virtual void Rock(Coord const & coord, float force);
		virtual WeaponDataStruct const * Get_Class_Weapon_Data(int which=0) const;
		virtual bool Is_Turret_Equipped(void) const;
		int Get_Sight_Bonus(Coord const & coord);
		static void Remove_Target(AbstractClass * target);
		bool Has_Ability(AbilityType ability) const;
		bool Should_Use_High_Arc(int which) const;
		double Target_Threat(TechnoClass * target, Coord const & firing_coord = COORD_NONE) const;

		virtual ResultType Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source=0, bool forced=false, bool=false) override;
		bool Evaluate_Cell(ThreatType method, int mask, Cell const & cell, int range, TechnoClass const ** object, int & value, int zone=0) const;
		bool Evaluate_Object(ThreatType method, int mask, int range, TechnoClass const * object, int & value, int zone=-1, Coord const & coord=COORD_NONE) const;
		int Evaluate_Just_Cell(Cell const & cell) const;

		void Assign_Archive_Target(AbstractClass * target);
		AbstractClass * Fetch_Archive_Target(void) const { return(ArchivedTarget); }
		__declspec( property( get=Fetch_Archive_Target, put=Assign_Archive_Target) ) AbstractClass * ArchiveTarget;

		Coord Railgun_Beam_Damage(Coord & coord, AbstractClass *abstract, WeaponTypeClass *weapon);

		inline WeaponTypeClass * Get_Primary_Weapon(void) const { const WeaponDataStruct * wdata = Get_Class_Weapon_Data(0); return(wdata->Weapon); }
		inline WeaponTypeClass * Get_Secondary_Weapon(void) const { const WeaponDataStruct * wdata = Get_Class_Weapon_Data(1); return(wdata->Weapon); }
		__declspec( property( get=Get_Primary_Weapon) ) WeaponTypeClass * PrimaryWeapon;
		__declspec( property( get=Get_Secondary_Weapon) ) WeaponTypeClass * SecondaryWeapon;

		/*
		**	AI.
		*/
		virtual void Renovate(void);
		virtual void AI(void) override;
		virtual bool Revealed(HouseClass * house) override;
		virtual RadioMessageType Receive_Message(RadioClass * from, RadioMessageType message, intptr_t & param) override;
		virtual void Cloaking_AI(bool=false);
		virtual void Rocking_AI(void);
		virtual void Try_To_Cloak(void);

		/*
		**	Scenario and debug support.
		*/
#ifdef _DEBUG
		virtual void Debug_Dump(MonoClass *mono) const override;
#endif

		/*
		**	Display and rendering support functionality. Supports imagery and how
		**	object interacts with the map and thus indirectly controls rendering.
		*/
		void Techno_Draw_Object(ShapeSet const * shapefile, int shapenum, Point2D const & drawpoint, Rect const & rect, Dir256 rotation=DIR_N, int scale=0x0100, int zadjust=0, ZGradientType zgrad=ZGRAD_GROUND, bool=false, int brightness=0, ShapeSet const * zshapefile=0, int zshapenum=0, Point2D zoff = Point2D(0,0), ShapeFlags_Type negflags=SHAPE_NORMAL) const;

		virtual void Draw_Pre_Render(Point2D const & point, Rect const & cliprect) const override;
		virtual void Draw_Post_Render(Point2D const & point, Rect const & cliprect) const override;
		virtual void Draw_Action_Line(void) const;

		virtual void Draw_Voxel(VoxelDataStruct const & voxeldata, int frame, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, int brightness, ShapeFlags_Type negflags) const;

		void Techno_Blit_Voxel(StaticBufferClass::Entry const & cache, Point2D const & point, Rect const & cliprect, ShapeFlags_Type flags, int brightness) const;
		void Techno_Draw_Voxel_Shadow(VoxelDataStruct const & voxeldata, int layer_index, int key, VoxelIndexClass * cache, Rect const & cliprect, Point2D const & point, Matrix3D const & matrix, bool force_cache) const;
		SurfaceRegion Techno_Render_Voxel_Object(VoxelDataStruct const & voxeldata, Matrix3D const & matrix, Point2D const & point, Rect const & cliprect, int frame, ShapeFlags_Type flags, int brightness) const;
		SurfaceRegion Techno_Render_Voxel_Shadow(VoxelDataStruct const & voxeldata, Matrix3D const & matrix, Point2D const & point, Rect const & cliprect, int layer_index, ShapeFlags_Type flags, bool cached) const;

		bool Is_Decoration_Visible(void) const;
		Point2D Pip_Origin(Point2D const & point) const;
		virtual void Draw_Health_Bar_Old(Point2D const & point, Rect const & rect) const;
		virtual void Draw_Health_Bar(Point2D const & point, Rect const & rect) const;
		virtual void Draw_Pips(Point2D const & bottomleft, Point2D const & bottomright, Rect const & rect) const;
		virtual void Draw_Insignia(Point2D const & bottomleft, Point2D const & center, Rect const & rect) const;
		virtual void Draw_Text_Overlay(Point2D const & point1, Point2D const & point2, Rect const & rect) const;
		virtual void Hidden(void) override;
		virtual bool Mark(MarkType mark=MARK_CHANGE) override;
		virtual int Exit_Object(TechnoClass *) override;
		virtual void Do_Uncloak(bool silent=false);
		virtual void Do_Cloak(bool silent=false);
		virtual void Do_Shimmer(void) override;
		int Get_Predator_Offset(void) const;
		void Calculate_Sinking_Offset(short height, int y);
		void Draw_Target_Laser(void) const;
		static void Draw_Double_Selection_Bracket(Coord const & coord1, Coord const & coord2, int color);
		static void Draw_Single_Selection_Bracket(Coord const & coord1, Coord const & coord2, int color);

		/*
		**	Movement and animation.
		*/
		virtual int Apparent_Brightness(int brightness = 1000) const;
		virtual bool Is_Ready_To_Random_Animate(void) const;
		virtual bool Random_Animate(void) {return(false);}
		virtual void Assign_Destination(AbstractClass * target, bool = true);
		virtual void Per_Cell_Process(PCPType why) override;
		virtual bool Enter_Idle_Mode(bool initial=false, bool = true);
		virtual void Look(bool incremental=false, bool dontmap=false) override;
		virtual void Radar_Track(void);
		virtual void Radar_Untrack(void);
		virtual void Plot_On_Radar(void) const;
		virtual void Update_Radar_Position(bool force_update=false);
		static void Set_Talker(TechnoClass * techno, TalkType bubble=TALK_NONE);
		static void Set_Action_Lines(bool on);
		void Remove_Damage_Particle(void);
		bool Enter_Object_Nearby(void);
		bool Move_To_Object_Nearby(void);

		/*
		**	Map entry and exit logic.
		*/
		virtual bool Limbo(void) override;
		virtual bool Unlimbo(Coord const & , Dir256 facing=DIR_N) override;
		virtual void Init(void) override;
		virtual void Detach(AbstractClass const * target, bool all) override;

		/*
		 * This is the countdown that keeps the action line of a selected object on screen. It
		 * is restarted whenever the player issues an order, so the line showing where that
		 * order is headed fades away shortly after it has been read.
		 */
		static CDTimerClass<FrameTimerClass> ActionLineTimer;

		/*
		 * If the player has asked for action lines to be drawn, then this flag will be true.
		 * It follows the matching game option, so turning that option off suppresses the
		 * lines for every object at once.
		 */
		static bool ActionLines;

		static void Reset_Action_Line_Timer(void);

		/*
		 * These record the one talk bubble the game can display -- which bubble to draw, the
		 * object it hovers over, and how long it lingers. They are set together when a script
		 * puts words in an object's mouth and cleared together when it falls silent.
		 */
		static TalkType TalkBubbleType;
		static TechnoClass * TalkBubbleOwner;
		static CDTimerClass<SystemTimerClass> TalkBubbleTimer;

		/*
		**	Facing translation tables that fix the flaw with 3D studio when
		**	it renders 45 degree angles.
		*/
		static int const BodyShape[FACING_COUNT * 4];
};

inline bool TechnoClass::Is_Allowed_To_Leave_Map(void) const { return(false); }
inline bool TechnoClass::Is_Renovator(void) const { return(false); }
inline bool TechnoClass::Is_Turret_Equipped(void) const { return(TClass->IsTurretEquipped); }
inline bool TechnoClass::Is_Move_Override(void) const { return(false); }
inline bool TechnoClass::Is_In_Team(void) const { return(false); }
inline bool TechnoClass::Is_Ready_To_Move(void) const { return(true); }


inline DirType TechnoClass::Turret_Facing(void) const
{
	return(PrimaryFacing.Current());
}


inline WeaponDataStruct const * TechnoClass::Get_Class_Weapon_Data(int which) const
{
	if (Veterancy.Is_Elite() && which == 0) {
		return(TClass->Get_Weapon(2));
	}

	return(TClass->Get_Weapon(which));
}


inline void TechnoClass::Advance_Waypoint_Path(void)
{

}


inline void TechnoClass::Draw_Action_Line(void) const
{

}


inline bool TechnoClass::Is_In_Same_Zone_As(ObjectClass const * object) const
{
	return(true);
}


inline TechnoClass * AbstractClass::As_TechnoClass(void)
{
	return(dynamic_cast<TechnoClass *>(this));
}


inline TechnoClass const * AbstractClass::As_TechnoClass(void) const
{
	return(dynamic_cast<TechnoClass const *>(this));
}
