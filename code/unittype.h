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

#include "face.h"
#include "techtype.h"

#include "land.hh"
#include "unit.hh"

class ConvertClass;
class Surface;

/***************************************************************************
**	The various unit types need specific data that is unique to units as
**	opposed to buildings. This derived class elaborates these additional
**	data types.
*/
class UnitTypeClass : public TechnoTypeClass
{
		typedef TechnoTypeClass BASECLASS;

		enum {
			FIRING_SYNC_FRAME_MAX = 2,
		};

	public:
		/*
		**	This value represents the unit class. It can serve as a unique
		**	identification number for this unit class.
		*/
		UnitType HeapID;

		/*
		 * This restricts the vehicle to cells of one land type -- the railroad, for a
		 * train. If LAND_NONE, then the vehicle travels anywhere its speed type allows.
		 */
		LandType MovementRestrictedTo;

		/// Unused
		TPoint3D<int> HalfDamageSmokeLocation;

		/*
		 * If this vehicle travels at a fixed pace along a fixed route, then this flag will be
		 * true. It is never brought up to speed or slowed for its destination, its path must
		 * reach the destination cell outright, and it may jump turn tracks in mid move.
		 */
		bool IsPassive;

		/*
		**	If this unit can appear out of a crate, then this flag will be true.
		*/
		bool IsCrateGoodie;

		/*
		**	Does this unit go into harvesting mode when it stops on a tiberium
		**	field?  Typically, only one unit does this and that is the harvester.
		*/
		bool IsToHarvest;

		/*
		 * Does this unit harvest veins when it stops on a weed field? Only the weed eater
		 * does this, and it unloads at a weeder building rather than a tiberium refinery.
		 */
		bool IsToVeinHarvest;

		/*
		**	If this unit has a firing animation, this flag is true. Infantry and some special
		**	vehicles are the ones with firing animations.
		*/
		bool IsFireAnim;

		/*
		**	Many vehicles have a turret with restricted motion. These vehicles must move the
		**	turret into a locked down position while travelling. Rocket launchers and artillery
		**	are good examples of this kind of unit.
		*/
		bool IsLockTurret;

		/*
		**	If this unit cannot fire while moving, then this flag will be
		**	true. Such a unit must stop and stabilize for a bit before it
		**	can fire.
		*/
		bool IsNoFireWhileMoving;

		/*
		 * If this vehicle must deploy before it can fire, then this flag will be true. It
		 * will not shoot from ground it cannot set down upon, and under a human player it
		 * never picks a target of its own, since deploying is the player's decision.
		 */
		bool IsDeployToFire;

		/*
		 * If this vehicle leans to match the slope of the ground it drives over, then this
		 * flag will be true.
		 */
		bool IsTilter;

		/*
		 * If the turret of this vehicle casts a shadow of its own rather than relying on the
		 * shadow of the body, then this flag will be true.
		 */
		bool IsUseTurretShadow;

		/*
		 * If this vehicle stands too tall to fit beneath a bridge, then this flag will be
		 * true. Its depth is fudged while it passes under a span so that the bridge deck
		 * does not appear to slice through the top of the artwork.
		 */
		bool IsTooBigToFitUnderBridge;

		/*
		 * If a carryall may pick this vehicle up, then this flag will be true. It governs the
		 * lift alone, so a vehicle refused here is left to drive itself and is otherwise
		 * unrestricted.
		 */
		bool IsTotable;

		/*
		 * If this type is the small visceroid, then this flag will be true. A small
		 * visceroid crawls where it pleases, walks through others of its own kind, and
		 * merges with the first one it reaches to become the large visceroid.
		 */
		bool IsSmallVisceroid;

		/*
		 * If this type is the large visceroid, then this flag will be true. Like the small
		 * one it is a wandering creature rather than a crewed vehicle, so it is drawn from
		 * shape artwork, aims without turning, and shrugs off an EM pulse.
		 */
		bool IsLargeVisceroid;

		/*
		 * If this vehicle may leave a wooden crate behind when it is destroyed, then this
		 * flag will be true. Whether the crate actually drops is up to the scenario, which
		 * allows truck crates and train cargo separately.
		 */
		bool IsCarriesCrate;

		/*
		 * This points to the shape set fetched from the AltImageFile, or NULL if this type
		 * has no alternate artwork. A visceroid keeps the tail of its idle animation here,
		 * past the frames its regular image data provides.
		 */
		const void * AltImageData;

		/*
		 * If this type is a creature built upon the unit class rather than a real vehicle,
		 * then this flag will be true. It is the answer the Considered_Vehicle function
		 * gives, so such a type is passed over wherever the game counts or targets
		 * vehicles as such.
		 */
		bool IsNonVehicle;

		/*
		 * If this type is the jellyfish, then this flag will be true. It swims on logic of
		 * its own instead of the usual combat and approach missions, is drawn lifted by
		 * its depth in the water, and never counts another unit as a passing blockage.
		 */
		bool IsJellyfish;

		/*
		 * If this type is the limpet drone, then this flag will be true. Like the
		 * jellyfish it is drawn from shape artwork by its animation stage, and that stage
		 * is wrapped back to the start after ten frames so the drone pulses forever.
		 */
		bool IsLimpetDrone;

		/*
		 * If this vehicle carries a mobile EM pulse cannon, then this flag will be true.
		 * Such a vehicle may be told to deploy even though it has nothing to deploy into
		 * and no cargo to unload, and deploying spends its accumulated charge as a pulse.
		 */
		bool IsMobileEMP;

		/*
		 * If this vehicle is the core defender, then this flag will be true. It is treated
		 * as a building where its selection box and health bar are concerned, it stands
		 * taller than an ordinary vehicle, and an EM pulse cannot paralyze it.
		 */
		bool IsCoreDefender;

		/*
		 * This is the number of frames in the standing animation of a shape based
		 * vehicle, counted per facing. If it is zero, then the vehicle has no idle
		 * artwork of its own and its first walk frame is shown instead.
		 */
		char StandingFrames;

		/*
		 * This is the number of frames in the death animation of a shape based vehicle,
		 * counted per facing. If it is zero, then the vehicle simply explodes when it is
		 * destroyed rather than lingering as a wreck while an animation plays out.
		 */
		char DeathFrames;

		/*
		 * This is the number of game frames that each frame of the death animation is
		 * held for. It is never allowed to be less than one, since the frame to show is
		 * derived by dividing the vehicle's death counter by it.
		 */
		char DeathFrameRate;

		/*
		 * This is the charge that a mobile EM pulse cannon must build up before it may
		 * fire. The vehicle's Charge climbs toward this value whenever it is able, and a
		 * charging vehicle draws its pip bar as a fraction of it.
		 */
		unsigned MaxCharge;

		/*
		 * This is the charge that a mobile EM pulse cannon is built with, so that a
		 * vehicle can be given to a house ready to fire rather than having to sit out
		 * its first charge.
		 */
		int StartCharge;

		/*
		 * These are the frames of the firing animation on which the first and second
		 * rounds of a burst are actually released, so that the shots stay in step with
		 * the recoil artwork. If -1, then that round is not tied to the animation.
		 */
		int FiringSyncFrame[FIRING_SYNC_FRAME_MAX];

		/*
		 * This is the frame that the standing animation begins at within the shape file.
		 * It defaults to just past the walk frames, but the artwork may name it outright
		 * when the blocks are not laid down in the usual order.
		 */
		int StartStandFrame;

		/*
		 * This is the frame that the walking animation begins at within the shape file.
		 * The walk frames come first in the usual layout, so this defaults to zero.
		 */
		int StartWalkFrame;

		/*
		 * This is the frame that the firing animation begins at within the shape file. It
		 * defaults to just past the walk and standing frames, and falls back to the
		 * standing frames for a vehicle that has no firing animation.
		 */
		int StartFiringFrame;

		/*
		 * This is the frame that the death animation begins at within the shape file, or
		 * -1 if this vehicle has no death animation at all.
		 */
		int StartDeathFrame;

		/*
		 * This is the count that a destroyed vehicle's DeathCounter must reach before the
		 * wreck finally explodes and is removed. It is derived from the extent of the
		 * death frames, though the artwork may name it outright.
		 */
		int MaxDeathCounter;

		/*
		 * This specifies how many facings the shape artwork of this vehicle provides.
		 * Artwork cut into 8, 16, 32 or 64 facings is indexed by the vehicle's own heading;
		 * anything else is drawn from a single facing regardless of which way it points.
		 */
		int Facings;

		/*
		 * This specifies how many facings the turret artwork of this vehicle provides, which
		 * need not match the hull. Every stock turret is cut into 32.
		 */
		int TurretFacings;

		/*
		 * This is the frame the turret artwork begins at, or -1 to take the frame the
		 * layout puts it at, eight walk blocks in however many facings the hull has.
		 */
		int StartTurretFrame;

		/*
		 * This is the number of frames in the walking animation of a shape based vehicle,
		 * counted per facing. It doubles as the stride from one facing to the next within
		 * the walk frames, so it must match the artwork exactly.
		 */
		char WalkFrames;

		/*
		 * This is the number of frames in the firing animation of a shape based vehicle,
		 * counted per facing. If it is zero, then the vehicle has no firing animation and
		 * its shots are never held back to line up with one.
		 */
		char FiringFrames;

		/*
		 * This is the name of an alternate shape file for this unit, without its
		 * extension. The shape set itself is fetched into the AltImageData whenever the
		 * type is read or loaded, so only the name has to be kept.
		 */
		TStringID<24> AltImageFile;

		/*
		**	This is the explicit unit type class constructor.
		*/
		UnitTypeClass(char const * ininame = NULL);
		virtual ~UnitTypeClass() override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override;

		static UnitType From_Name(char const * name);
		static void Init(TheaterType ) {};
		static void One_Time(void);
		static UnitTypeClass * Find_Or_Make(char const * name);

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual Point3D Pixel_Dimensions(void) const override;
		virtual Point3D Lepton_Dimensions(void) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house = NULL) const override;
		virtual ObjectClass * Create_One_Of(HouseClass * house) const override;
		virtual int Repair_Step(void) const override;

		TPoint2D<int> Turret_Adjust(Dir256 dir, TPoint2D<int> const & xy) const;

		/*
		 * These point to the shape sets that the small and large visceroids are drawn
		 * from while the game is running in "SnoBee" mode, in place of the artwork named
		 * by their own types. They are fetched once at startup, since every visceroid on
		 * the map looks alike.
		 */
		static void const * SmallVisceroidShapes;
		static void const * LargeVisceroidShapes;
};

extern Surface * EightBitSurface;
extern ConvertClass * EightBitDrawer;
