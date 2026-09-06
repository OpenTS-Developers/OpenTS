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

#include "_weapon.h"
#include "objtype.h"
#include "typelist.h"

#include "category.hh"
#include "mission.hh"
#include "mph.hh"
#include "mzone.hh"
#include "pip.hh"
#include "speed.hh"

class VoxelAnimTypeClass;
class UnitTypeClass;
class ParticleSystemTypeClass;
class AnimTypeClass;
class WeaponTypeClass;

/***************************************************************************
**	This class is the common data for all objects that can be owned, produced,
**	or delivered as reinforcements. These are objects that typically contain
**	crews and weapons -- the fighting objects of the game.
*/
class TechnoTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:

		enum {
			WEAPON_SLOT_COUNT = 3
		};

		/*
		 * This is the fraction of its own toughness that an object of this type does as damage
		 * to its surroundings when destroyed, so a heavier object makes a bigger mess.
		 */
		float CollateralDamageCoefficient;

		/// Unused
		float Unused1;

		/*
		 * This is the number of game frames that each step of this object's walk cycle holds
		 * for. It is what paces the TotalFramesWalked counter the animations are indexed from.
		 */
		int WalkRate;

		/*
		 * These are the abilities an object of this type gains as its crew is promoted. A
		 * veteran has the veteran set; an elite has both, so nothing is lost on promotion.
		 */
		AbilityFlagsType VeteranAbilities;
		AbilityFlagsType EliteAbilities;

		/*
		 * This is the extra threat an object of this type poses regardless of its weapons. It
		 * makes a target worth shooting that its firepower alone would not justify.
		 */
		double SpecialThreatValue;

		/*
		 * These are the weights an object of this type scores a candidate target by -- the
		 * harm each side's weapon would do to the other, the target's special threat and
		 * remaining health, and how far out of range the shot would be. A house without a
		 * threat rating node uses the cruder Dumb equivalents from the rules instead.
		 */
		double MyEffectivenessCoefficient;
		double TargetEffectivenessCoefficient;
		double TargetSpecialThreatCoefficient;
		double TargetStrengthCoefficient;
		double TargetDistanceCoefficient;

		/*
		 * This specifies how strongly the path finder steers an object of this type clear of
		 * dangerous ground. Zero ignores threat entirely; larger values buy a wider detour.
		 */
		double ThreatAvoidanceCoefficient;

		/*
		 * This is how far short of its destination an object of this type begins slowing
		 * down, expressed in leptons. An aircraft also judges when to begin its descent by it.
		 */
		int SlowdownDistance;

		/*
		 * These control how quickly an object of this type gets under way and how quickly it
		 * brakes as it nears its destination. Only a type flagged IsAccelerates uses them.
		 */
		double DeaccelerationFactor;
		double AccelerationFactor;

		/*
		 * This is the number of game frames that each stage of the cloaking fade holds for. A
		 * larger value makes the object shimmer into and out of view more slowly.
		 */
		int CloakingSpeed;

		/*
		 * These are the voxel debris types this object throws off as it dies, and the most of
		 * each that may appear. They are drawn from in order until MaxDebris is used up.
		 */
		TypeList<VoxelAnimTypeClass const *> DebrisTypes;
		TypeList<int> DebrisMaximums;

		/*
		 * This is the class ID of the locomotion object that moves an object of this type
		 * about. It is what decides whether the object drives, walks, hovers, flies or
		 * tunnels, and an instance of it is created for every object as it is unlimboed.
		 */
		CLSID Locomotor;

		/*
		 * These are the half extents of this object's voxel model, measured off the artwork
		 * as it is loaded. The tilting locomotors swing the model about them so that a
		 * vehicle appears to pivot on the ground it rests on rather than about its center.
		 */
		double VoxelCenterY;
		double VoxelCenterX;

		/*
		 * This is how strongly this object resists being rocked about, used as a divisor of
		 * the force applied. A heavy object also cracks and breaks the ice it drives over.
		 */
		double Weight;

		/// Unused
		double PhysicalSize;

		/*
		 * This is the mission an object of this type takes up when it arrives with no orders
		 * of its own. Harvesting vehicles begin harvesting; everything else begins hunting.
		 */
		MissionType InitialMission;

		/*
		 * These describe the attitude a flying object of this type strikes in the air. It
		 * banks by the roll angle as it turns and drops its nose by the pitch angle, but
		 * only while travelling faster than the pitch speed. Both angles are in radians.
		 */
		double RollAngle;
		double PitchSpeed;
		double PitchAngle;

		/*
		 * This is the number of objects of this type that one house may have at a time. A
		 * positive limit counts what the house currently owns; a negative one counts every
		 * such object it has ever built, so that limit can never be recovered by losing one.
		 */
		int BuildLimit;

		/*
		 * This is the tactical role this object is classified as -- soldier, artillery,
		 * transport, and so on. The computer's general tallies its forces by category to
		 * judge what it is short of.
		 */
		CategoryType Category;

		/// Unused
		int Unused2;

		/*
		 * This is how long this object's door takes to swing fully open or shut, expressed in
		 * game minutes. It paces a factory door as a newly built unit files out, a gate as it
		 * raises and lowers, and a transport's door as it takes on or discharges passengers.
		 */
		double DeployTime;

		/*
		 * This is the barrel elevation this object's turret rests at when it has no target
		 * to aim at, expressed as a Dir256 angle. It is also the pitch fallen back on when
		 * no ballistic arc will reach the target.
		 */
		int FireAngle;

		/*
		 * This specifies what the row of pips drawn over this object measures -- ammunition,
		 * tiberium load, passengers, or charge. If PIPSCALE_NONE, then no pips are drawn.
		 */
		PipScaleType PipScale;

		/*
		 * These are the building types this object docks with for service, listed in order of
		 * preference. An aircraft rearms at one, and a harvester unloads its tiberium at one.
		 */
		TypeList<BuildingTypeClass const *> Dock;

		/*
		 * These name this object's counterpart on the other side of a deploy -- a vehicle
		 * becomes the building it deploys into, and a building becomes the vehicle it
		 * undeploys into. The mobile construction vehicle is the canonical pair.
		 */
		BuildingTypeClass *DeploysInto;
		UnitTypeClass *UndeploysInto;

		/*
		 * These are the voice responses this object gives, one list per occasion. The
		 * response is picked at random from the applicable list, so a type given several
		 * alternatives will not repeat itself.
		 */
		TypeList<int> VoiceSelect;
		TypeList<int> VoiceMove;
		TypeList<int> VoiceAttack;
		TypeList<int> VoiceDie;
		TypeList<int> VoiceFeedback;	/// Given when the object is first badly hurt.

		/*
		 * These are the two auxiliary sounds this object type owns. What each one means is
		 * left to the object -- a building plays them as it deploys and undeploys, and an
		 * aircraft as it takes off and lands.
		 */
		VocType AuxSound1;
		VocType AuxSound2;

		/*
		**	This specifies the zone that an object of this type should recognize. Zones
		**	of this type or lower will be considered "possible to travel to".
		*/
		MZoneType MZone;

		/*
		**	When determining threat, the range can be overridden to be the value
		**	specified here. Otherwise, the range for enemy scan is equal to the
		**	longest weapon range the object has. If the value is zero, then the
		**	weapon range is used.
		*/
		LEPTON ThreatRange;

		/*
		 * This is the greatest number of pieces of debris this object may throw off as it is
		 * destroyed. If zero, then it leaves no debris behind at all.
		 */
		int MaxDebris;

		/*
		**	If this is a transporter object (e.g., hovercraft, chinook, APC), then this
		**	value specifies the maximum number of passengers it may carry.
		*/
		int MaxPassengers;

		/*
		**	Most objects have the ability to reveal the terrain around themselves.
		**	This sight range (expressed in cell distance) is specified here. If
		**	this value is 0, then this unit never reveals terrain. Bullets are
		**	typically of this nature.
		*/
		int SightRange;

		/*
		**	This is the credit cost to produce this object (presuming production is
		**	allowed).
		*/
		int Cost;

		/*
		 * This is the height above ground that an object of this type cruises at, expressed
		 * in leptons. If -1, then the global FlightLevel rule is used instead.
		 */
		int FlightLevel;

		/*
		**	The tech level that this object can be produced at.
		*/
		int Level;

		/*
		**	This specifies the building prerequisites required before an object
		**	of this type can be produced.
		*/
		TypeList<int> Prerequisite;

		/*
		**	The risk and reward values are used to determine targets and paths
		**	toward targets. When heading toward a target, a path of least
		**	risk will be followed. When picking a target, the object of
		**	greatest reward will be selected. The values assigned are
		**	arbitrary.
		*/
		int Risk;
		int Reward;

		/*
		**	This value indicates the maximum speed that this object can achieve.
		*/
		MPHType MaxSpeed;

		/*
		**	This indicates the speed (locomotion) type for this unit. Through this
		**	value the movement capabilities are deduced.
		*/
		SpeedType Speed;

		/*
		**	This is the maximum number of ammo shots this object can hold. If
		**	this number is -1, then this indicates unlimited ammo.
		*/
		int MaxAmmo;

		/*
		**	This is a bit field representing the houses that are allowed to
		**	own (by normal means) this particular object type. This value is
		**	typically used in production contexts. It is possible for a side
		**	to take possession of an object type otherwise not normally allowed.
		**	This event usually occurs as a result of capture.
		*/
		int Ownable;

		/*
		 * If a multiplayer game may include this type in a player's starting force, then this
		 * flag will be true. Eligible types are drawn from at random to fill the unit count.
		 */
		bool IsAllowedToStartInMultiplayer;

		/*
		**	This is the small icon image that is used to display the object in
		**	the sidebar for construction selection purposes.
		*/
		TStringID<24> CameoFilename;
		const void * CameoData;

		/*
		 * Where this type's cameo sorts among the others of its kind on the sidebar, lowest
		 * first.
		 */
		int CameoSortOrder;

		/*
		**	The number of animation frames allotted to rotation is specified here.
		**	For an object that has no rotation, this value will be 1. For normal
		**	vehicles this value will be 32. There are some special case units that
		**	have intermediate rotation frames.
		*/
		int Rotation;

		/*
		**	This is the rotational speed of the object. This value represents the
		**	turret or body rotation speed expresses as 360/256ths rotation steps per
		**	game tick.
		*/
		int ROT;

		/*
		**	This is the distance along the centerline heading in the direction the body
		**	is facing used to reach the center point of the turret. This distance is
		**	in leptons.
		*/
		LEPTON TurretOffset;

		/*
		**	Points you're awarded for destroying an object of this type, and
		**	points you lose if you lose an object of this type.
		*/
		int Points;

		/*
		**	This is the default explosion to use when this vehicle is destroyed.
		*/
		TypeList<AnimTypeClass const *> Explosion;

		/*
		 * This is the particle system that a building of this type runs continuously --
		 * smokestack exhaust, refinery steam -- anchored at the given offset from its center.
		 */
		ParticleSystemTypeClass const * NaturalParticleSystem;
		TPoint3D<int> NaturalParticleLocation;

		/*
		 * These are the particle systems this object gives off as it takes damage, along with
		 * the offset from its center that they are spawned at. The list is sifted at spawn
		 * time -- a spark behaving system supplies the damage sparks and a smoke behaving one
		 * supplies the damage smoke.
		 */
		TypeList<ParticleSystemTypeClass const *> DamageParticleSystems;
		TPoint3D<int> DamageSmokeOffset;

		/*
		 * This is the voxel layer that this object's shadow is cast from. A model whose
		 * silhouette is dominated by one part can throw a shadow shaped like that part instead
		 * of like the whole body.
		 */
		int ShadowIndex;

		/*
		**	This is the Tiberium storage capacity of the building. The sum of all
		**	building's storage capacity is used to determine how much Tiberium can
		**	be accumulated.
		*/
		int Capacity;

		/*
		 * If this building's turret voxel was built without its pivot resting on the ground,
		 * then this flag will be true. The barrel is then swung about the negated firing
		 * offset rather than the positive one, which puts the pivot back where the artwork
		 * assumed it would be.
		 */
		bool TurretNotExportedOnGround;

		/*
		**	These are the weapons that this techno object is armed with.
		*/
		WeaponDataStruct Weapons[WEAPON_SLOT_COUNT];

		/*
		 * If this object cannot be harmed by another object of its own type and house, then
		 * this flag will be true. It stops a mass of identical units from destroying itself
		 * with its own splash damage.
		 */
		bool IsTypeImmune;

		/*
		 * If this object sees a disguised object for what it really is when it scans for a
		 * target, then this flag will be true.
		 */
		bool IsDetectDisguise;

		/*
		 * If this object may be ordered into shrouded territory, then this flag will be true.
		 * Otherwise a shrouded destination is refused outright and the cursor shows the order
		 * as illegal.
		 */
		bool IsMoveToShroud;

		/*
		 * If this object earns veterancy from its kills, then this flag will be true. Every
		 * kill it scores is credited to its Crew, which is what promotes it toward veteran
		 * and elite rank.
		 */
		bool IsTrainable;

		/*
		 * If this object throws sparks once it has been hurt, then this flag will be true. The
		 * spark system is picked from the DamageParticleSystems list and appears once the
		 * object drops below the condition yellow health level.
		 */
		bool IsDamageSparks;

		/*
		 * If this object paints a targeting laser onto whatever it is shooting, then this flag
		 * will be true. The laser is a brief cosmetic flourish and is only drawn for a house
		 * the local player controls.
		 */
		bool IsTargetLaser;

		/*
		 * If the veins can neither hurt nor devour this object, then this flag will be true.
		 * The ABILITY_VEIN_PROOF veteran ability grants the same protection to an object whose
		 * type lacks the flag.
		 */
		bool IsImmuneToVeins;

		/*
		 * If this object mends itself while it stands on tiberium, then this flag will be
		 * true. It recovers a repair step at the interval given by the TiberiumHeal rule, and
		 * when it is destroyed it spills tiberium back into the cells around it.
		 */
		bool IsTiberiumHeal;

		/*
		 * If this object must come to a halt before it may cloak again, then this flag will be
		 * true. It is what keeps a cloaking unit exposed for as long as it keeps moving.
		 */
		bool IsCloakStop;

		/*
		 * If this vehicle is a train, then this flag will be true. A train is pathed along
		 * track instead of over open ground, refuses to be nudged aside by an ally, and is
		 * excused from the scatter and leave-the-map rules that ordinary vehicles follow.
		 */
		bool IsTrain;

		/*
		 * If this aircraft is a dropship, then this flag will be true. A dropship makes for a
		 * docking bay of its own accord, levels its pitch as it settles onto the ground, and
		 * flies neither the approach nor the takeoff profile that other aircraft use.
		 */
		bool IsDropship;

		/*
		 * If an attack on this object should alert a computer controlled owner to defend its
		 * base, then this flag will be true. It is how the AI comes to the rescue of the
		 * objects it cannot afford to lose.
		 */
		bool IsToProtect;

		/// Unused
		bool IsDisableable;

		/*
		 * If this object can never be produced, then this flag will be true. It keeps special
		 * case and scenario-only objects out of the sidebar without having to give them an
		 * unreachable tech level.
		 */
		bool IsUnbuildable;

		/*
		**	Is this object ownable by all sides in a multiplayer game? There are some
		**	special case objects that need this override ability.
		*/
		bool IsDoubleOwned;

		/*
		**	If this object should be completely and always invisible to the enemy, then
		**	this flag will be true.
		*/
		bool IsInvisible;

		/*
		 * If this object is plotted on the radar even for a house that has never discovered
		 * it, then this flag will be true. Only the IsInvisible flag can keep such an object
		 * off the radar.
		 */
		bool IsRadarVisible;

		/*
		**	If this object can serve as a good leader for a group selected
		**	series of objects, then this flag will be true. Unarmed or
		**	ability challenged units do not make good leaders. This flag is
		**	also used to indicate the primary factory when dealing with
		**	buildings.
		*/
		bool IsLeader;

		/*
		**	Does this object have the ability to detect the presence of a nearby
		**	cloaked object?
		*/
		bool IsScanner;

		/*
		**	If this object is always given its proper name rather than a generic
		**	name, then this flag will be true. Typically, civilians and Dr. Moebius
		**	fall under this category.
		*/
		bool IsNominal;

		/*
		**	Does this object type contain a rotating turret?  Gun emplacements, SAM launchers,
		**	and many vehicles contain a turret. If a turret is present, special rendering and
		**	combat logic must be performed.
		*/
		bool IsTurretEquipped;

		/*
		**	Certain objects can be repaired. For buildings, they repair "in place". For units,
		**	they must travel to a repair center to be repaired. If this flag is true, then
		**	allow the player or computer AI to repair the object.
		*/
		bool IsRepairable;

		/*
		**	Does this object contain a crew?  If it does, then when the object is destroyed, there
		**	is a distinct possibility that infantry will "pop out". Only units with crews can
		**	become "heros".
		*/
		bool IsCrew;

		/*
		**	This tells whether this unit should EVER be remapped when it is displayed
		**	on the tactical map. Normally, the unit is remapped, but for certain civilian
		**	object, remapping is not to be performed, regardless of owner.
		*/
		bool IsRemappable;

		/*
		**	Is the unit capable of cloaking?  Only Stealth Tank can do so now.
		*/
		bool IsCloakable;

		/*
		**	Can this object self heal up to half strength? Mammoth tanks from C&C had this
		**	feature.
		*/
		bool IsSelfHealing;

		/*
		**	If this object explodes violently when destroyed, then this flag will be true.
		**	The type of explosion is based on the warhead type and the damage generated
		**	corresponds to the full strength of the object.
		*/
		bool IsExploding;

		/*
		 * If this object never chooses targets for itself while a human player owns it, then
		 * this flag will be true. The threat scan gives up immediately, so the object fires
		 * only where it is told to. A computer controlled house targets with it normally.
		 */
		bool IsNoAutoFire;

		/*
		**	Some units are equipped with a rotating radar dish. These units have special
		**	animation processing. The rotating radar dish is similar to a turret, but
		**	always rotates and does not affect combat.
		*/
		bool IsRadarEquipped;

		/*
		**	If this building really only has cosmetic idle animation, then this flag will be
		**	true if this animation should run at a relatively constant rate regardless of game
		**	speed setting.
		*/
		bool IsRegulated;

		/*
		 * If this object is reloaded by hand instead of on a timer, then this flag will be
		 * true. Such an object never starts a reload delay of its own; it refills its
		 * ammunition only by visiting a service building, which is how the mine layer works.
		 */
		bool IsManualReload;

		/// Unused
		bool IsVisibleLoad;

		/*
		 * If this object attracts ion storm lightning, then this flag will be true. A powered
		 * lightning rod is struck far more often than anything else nearby, and being one
		 * overrides the ion immunity its team would otherwise grant it.
		 */
		bool IsLightningRod;

		/*
		 * If this object is a hunter seeker drone, then this flag will be true. A hunter
		 * seeker acquires a target of its own the moment it has none, never lands, and flies
		 * through an ion storm unhindered -- it exists only to reach something and die on it.
		 */
		bool IsHunterSeeker;

		/*
		**	Can this unit squash infantry?  If it can then if the player selects
		**	an (enemy) infantry unit as the movement target, it will ride over and
		**	squish the infantry unit.
		*/
		bool IsCrusher;

		/*
		 * If this vehicle pitches forward as it rolls over whatever it is crushing, then this
		 * flag will be true. The tilt is purely cosmetic, but it lends a heavy vehicle some
		 * weight as it flattens infantry and sandbag walls.
		 */
		bool IsTiltsWhenCrushes;

		/*
		 * If this object travels below the ground rather than across it, then this flag will
		 * be true. It is derived from the movement zone rather than specified on its own, and
		 * it is what lets the movement code route the object through shroud and through
		 * terrain that no surface object could cross.
		 */
		bool IsSubterranean;

		/*
		**	Should player controlled vehicles automatically try to crush nearby infantry
		**	instead of required the player to manually direct them to crush.
		*/
		bool IsAutoCrush;

		/*
		 * If this vehicle eases up to speed and brakes as it nears its destination, then this
		 * flag will be true. The curves are taken from the AccelerationFactor and
		 * DeaccelerationFactor values, and the braking begins at the SlowdownDistance.
		 * Vehicles without this flag simply travel at their target speed throughout.
		 */
		bool IsAccelerates;

		/*
		 * These are the amounts to bias this object's draw depth by while it is passing the
		 * terrain feature each one names, expressed in depth buffer units. Only the largest
		 * bias that currently applies is used, so that the object sorts behind a bridge
		 * column or in front of a cliff face the way the artwork expects.
		 */
		int ZFudgeCliff;
		int ZFudgeColumn;
		int ZFudgeTunnel;
		int ZFudgeBridge;

		//--------------------------------------------------------------------
		TechnoTypeClass(char const * ininame, SpeedType speed);
		virtual ~TechnoTypeClass() override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual void Compute_CRC(CRCEngine & crc) const override;
		bool Is_Two_Shooter(void) const;
		virtual bool Legal_Placement(Cell const & pos, HouseClass * house) const;
		virtual int Raw_Cost(void) const;
		int Max_Passengers(void) const {return(MaxPassengers);}
		virtual int Repair_Cost(void) const;
		virtual int Repair_Step(void) const;
		virtual int Flight_Level(void) const;
		virtual void const * Get_Cameo_Data(void) const override;
		virtual int Cost_Of(HouseClass * house = NULL) const override;
		virtual int Time_To_Build(void) const override;
		virtual int Get_Ownable(void) const override;
		virtual int Max_Pips(void) const override;
		bool In_Range(Coord const & coord, AbstractClass * target, WeaponTypeClass * weapon) const;
		virtual bool Read_INI(CCINIClass const & ini) override;

		WeaponDataStruct const * Get_Weapon(int which) const;
		void Set_Weapon(WeaponDataStruct const & weapon, int which);

		/*
		**	This is a pointer to the wake shape (as needed by the gunboat).
		*/
		static void const * WakeShapes;
};
