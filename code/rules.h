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

/* $Header: /CounterStrike/RULES.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : RULES.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/12/96                                                     *
 *                                                                                             *
 *                  Last Update : May 12, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ccini.h"
#include "face.h"

#include "diff.hh"

class AircraftTypeClass;
class AnimTypeClass;
class BuildingTypeClass;
class BulletTypeClass;
class InfantryTypeClass;
class OverlayTypeClass;
class ParticleSystemTypeClass;
class SaveStreamClass;
class SmudgeTypeClass;
class UnitTypeClass;
class VoxelAnimTypeClass;
class WarheadTypeClass;
class TerrainTypeClass;
class WeaponTypeClass;

class DifficultyClass
{
	public:
		double FirepowerBias;
		double GroundspeedBias;
		double AirspeedBias;
		double ArmorBias;
		double ROFBias;
		double CostBias;
		double BuildSpeedBias;

		double RepairDelay;
		double BuildDelay;

		bool IsBuildSlowdown;
		bool IsWallDestroyer;
		bool IsContentScan;

		// Carries one difficulty level's settings to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(FirepowerBias);
			stream.Serialize(GroundspeedBias);
			stream.Serialize(AirspeedBias);
			stream.Serialize(ArmorBias);
			stream.Serialize(ROFBias);
			stream.Serialize(CostBias);
			stream.Serialize(BuildSpeedBias);
			stream.Serialize(RepairDelay);
			stream.Serialize(BuildDelay);
			stream.Serialize(IsBuildSlowdown);
			stream.Serialize(IsWallDestroyer);
			stream.Serialize(IsContentScan);
		}
};

class RulesClass
{
	public:
		RulesClass(void);
		~RulesClass(void);

		void Initialize(CCINIClass const & ini);

		bool Addition(CCINIClass const & ini);

		bool Special_Weapons(CCINIClass const & ini);
		bool Audio_Visual_Rules(CCINIClass const & ini);
		bool Crate_Rules(CCINIClass const & ini);
		bool Combat_Damage(CCINIClass const & ini);
		bool Color_Schemes(CCINIClass const & ini);

		bool General(CCINIClass const & ini);
		bool MPlayer(CCINIClass const & ini);
		bool Heap_Maximums(CCINIClass const & ini);

		/// These are prefixed with Do to seperate them from the globals they iterate.
		bool Do_InfantryTypes(CCINIClass const & ini);
		bool Do_HouseTypes(CCINIClass const & ini);
		bool Do_VehicleTypes(CCINIClass const & ini);
		bool Do_AircraftTypes(CCINIClass const & ini);
		bool Do_Sides(CCINIClass const & ini);
		bool Do_SuperWeaponTypes(CCINIClass const & ini);
		bool Do_BuildingTypes(CCINIClass const & ini);
		bool Do_TerrainTypes(CCINIClass const & ini);
		bool Do_TeamTypes(CCINIClass const & ini);
		bool Do_SmudgeTypes(CCINIClass const & ini);
		bool Do_OverlayTypes(CCINIClass const & ini);
		bool Do_AnimTypes(CCINIClass const & ini);
		bool Do_VoxelAnimTypes(CCINIClass const & ini);
		bool Do_WarheadTypes(CCINIClass const & ini);
		bool Do_ParticleTypes(CCINIClass const & ini);
		bool Do_ParticleSystemTypes(CCINIClass const & ini);

		bool AI(CCINIClass const & ini);
		bool Powerups(CCINIClass const & ini);
		bool Land_Types(CCINIClass const & ini);
		bool IQ(CCINIClass const & ini);
		bool Jumpjet_Controls(CCINIClass const & ini);
		bool Difficulty(CCINIClass const & ini);
		bool Do_Movies(CCINIClass const & ini);
		bool Objects(CCINIClass const & ini);

		void Save(IStream * stream);
		void Load(IStream * stream);

		void Serialize(SaveStreamClass & stream);

		void Detach(AbstractClass const * target, bool all = true);

		static int Get_Rule_Unique_ID(void);
		static int Get_Art_Unique_ID(void);
		static int Get_AI_Unique_ID(void);

		static void Load_Art_INI(void);

	public:
		/*
		 * This is the damage dealt by the explosion when an ammunition barrel is set off.
		 */
		int AmmoCrateDamage;

		/*
		 * These are the two visceroid types. A small visceroid is spawned where infantry
		 * dies in tiberium, and when two of them meet one is promoted to the large type
		 * while the other is destroyed.
		 */
		UnitTypeClass const * LargeVisceroid;
		UnitTypeClass const * SmallVisceroid;

		/*
		 * This is the vehicle type whose artwork a harvester is drawn with while it is
		 * dumping its load, so the unloading frames need not live in the harvester's art.
		 */
		UnitTypeClass const * UnloadingHarvester;

		/*
		 * This is how far, expressed in cells, an aircraft reveals the shroud around
		 * itself when it opens fire into ground it cannot see.
		 */
		int AttackingAircraftSightRange;

		/*
		 * This scales the travel speed of a subterranean unit while it is underground,
		 * and divides the time it spends nosing down into the ground.
		 */
		double TunnelSpeed;

		/*
		 * This is the interval, expressed in minutes, between the healing ticks a unit
		 * that is flagged to heal in tiberium receives while it stands in a field.
		 */
		double TiberiumHeal;

		/*
		 * These are the buildings a hunter-seeker drone launches from. The super weapon
		 * picks one of them out of the owner's structures and sends the drone out beside it.
		 */
		TypeList<BuildingTypeClass const *> HSBuilding;

		/// Unused
		bool IsFreeMCV;

		/*
		 * If a cyborg damaged past half health may go berserk, then this flag will be true.
		 */
		bool IsBerzerkAllowed;

		/*
		 * This is the direction an aircraft parks facing when it is sitting idle on its
		 * helipad or airstrip.
		 */
		Dir256 PoseDir;

		/// Unused
		AnimTypeClass const * DropPodPuff;

		/*
		 * This is the number of game frames between advances of the marker animation
		 * drawn over a plotted waypoint path.
		 */
		int WaypointAnimationSpeed;

		/*
		 * These are the effects thrown off when an ammunition barrel explodes -- the blast
		 * animation, the voxel pieces flung out of it, and the smoke system left behind.
		 */
		AnimTypeClass const * BarrelExplode;
		TypeList<VoxelAnimTypeClass const *> BarrelDebris;
		ParticleSystemTypeClass const *BarrelParticle;

		/*
		 * This is how much of the way a radar event's box moves between its two colors
		 * each frame (0 - 1). The factor bounces off either end, so the event pulses.
		 */
		float RadarEventColorSpeed;

		/*
		 * This is the radius, expressed in radar pixels, that a radar event's box stops
		 * shrinking at. Once it is reached the box settles square and merely pulses.
		 */
		int RadarEventMinRadius;

		/*
		 * This is how many radar pixels a radar event's box closes in by each frame as it
		 * shrinks onto the cell it is flagging.
		 */
		float RadarEventSpeed;

		/*
		 * This is how fast a radar event's box spins, expressed in radians per frame. The
		 * spin decays to a third of this as the box settles onto its final angle.
		 */
		float RadarEventRotationSpeed;

		/*
		 * This is the number of game frames a radar blip spends in each phase of its flash,
		 * so a smaller value makes a flashing blip blink faster.
		 */
		int FlashFrameTime;

		/*
		 * This is how long, expressed in game frames, an object's radar blip flashes for
		 * after the object has been damaged. The blip is repainted every FlashFrameTime
		 * as this runs down, so the value is set to a multiple of it. Only the local
		 * player's own objects flash.
		 */
		int RadarCombatFlashTime;

		/*
		 * This is the greatest number of waypoints the player may drop into one path.
		 */
		int MaxWaypointPathLength;

		/*
		 * This is the animation of a wake spreading across water, played wherever a unit
		 * travels over water or something falls into it.
		 */
		AnimTypeClass const * Wake;

		/*
		 * This is the animation of a burning body, played when infantry that is not a dog
		 * is killed by a warhead that sets its victims alight.
		 */
		AnimTypeClass const * FlamingInfantry;

		/*
		 * These adjust an AI trigger's weight after it has run -- the first two are added
		 * on a success and on a failure, and the third scales how heavily its record of
		 * past attempts counts. The computer is thereby steered toward triggers that pay off.
		 */
		double AITriggerSuccessWeightDelta;
		double AITriggerFailureWeightDelta;
		double AITriggerTrackRecordCoefficient;

		/// Unused
		int VeinholeMonsterStrength;

		/*
		 * This is the greatest number of vein cells one veinhole monster may own. It also
		 * sizes the monster's node array and growth queue, so raising it costs memory.
		 */
		int MaxVeinholeGrowth;

		/*
		 * These are the intervals, expressed in game frames, between the passes in which a
		 * veinhole monster spreads its veins and, once dead, withdraws them. Each is
		 * jittered by up to half its length so neighboring veinholes do not pulse in step.
		 */
		int VeinholeGrowthRate;
		int VeinholeShrinkRate;

		/*
		 * This is the animation of the veins lashing out, played on a cell of solid veins
		 * that something vulnerable to them has walked onto.
		 */
		AnimTypeClass const * VeinAttack;

		/*
		 * This is the damage the veins inflict on every vulnerable object standing in them,
		 * applied every other game frame.
		 */
		int VeinDamage;

		/*
		 * This is the greatest number of objects that may be queued up behind the one a
		 * factory is currently building.
		 */
		int MaximumQueuedObjects;

		/*
		 * This is how far, expressed in cells, an aircraft lifts the fog of war beneath
		 * itself while a fog of war game is being played.
		 */
		int AircraftFogReveal;

		/*
		 * These are the overlays that the wood crate and the silver crate are drawn with.
		 * Which of the two a crate wears decides the powerup it yields in solo play, as
		 * listed by the WoodCrate and SilverCrate settings.
		 */
		OverlayTypeClass const * WoodCrateImg;
		OverlayTypeClass const * CrateImg;

		/*
		 * These are the landing animations for a drop pod, one per compass corner the pod
		 * may come down from, so the effect lines up with its approach.
		 */
		TypeList<AnimTypeClass const *> DropPod;

		/*
		 * These are the corpse animations left behind by infantry that has played out its
		 * death sequence. One is picked at random, and dogs leave none at all.
		 */
		TypeList<AnimTypeClass const *> DeadBodies;

		/*
		 * These are the animations of scrap metal thrown clear when something built of it
		 * is wrecked -- a vehicle, a bridge, or a building caught by the ion storm.
		 */
		TypeList<AnimTypeClass const *> MetallicDebris;

		/*
		 * These are the explosion animations set off along a bridge as it collapses. One is
		 * picked at random per span cell and jittered off center so the collapse does not
		 * look uniform.
		 */
		TypeList<AnimTypeClass const *> BridgeExplosions;

		/*
		 * These are the sound effect and the animation of earth being thrown up, played as
		 * a subterranean unit burrows into or back out of the ground.
		 */
		VocType DigSound;
		AnimTypeClass const * Dig;

		/*
		 * These are the animations played where an ion cannon shot lands -- the blast on the
		 * ground, which gives way to a splash over water, and the beam that comes down onto it.
		 */
		AnimTypeClass const * IonBlast;
		AnimTypeClass const * IonBeam;

		/*
		 * This is the animation played when infantry is blown apart rather than allowed to
		 * play out a death sequence, which is what becomes of cyborgs and jumpjet troops.
		 */
		AnimTypeClass const * InfantryExplode;

		/*
		 * This is the animation of a drop pod burning its way down through the atmosphere,
		 * played on the pod as it appears above the map.
		 */
		AnimTypeClass const * AtmosphereEntry;

		/*
		 * These list the buildings that satisfy each of the generic prerequisites, so a
		 * side may meet the power, factory, barracks, radar or tech requirement with
		 * whichever of its own structures fills that role.
		 */
		TypeList<int> PrerequisitePower;
		TypeList<int> PrerequisiteFactory;
		TypeList<int> PrerequisiteBarracks;
		TypeList<int> PrerequisiteRadar;
		TypeList<int> PrerequisiteTech;

		/*
		 * These are the sound effects of a gate rising to bar the way and lowering again to
		 * let a unit through.
		 */
		VocType GateUpSound;
		VocType GateDownSound;

		/*
		 * This is the rate of turn of a jumpjet unit, expressed as 360/256ths of a rotation
		 * per game tick.
		 */
		int JumpjetTurnRate;

		/*
		 * This is the top travel speed of a jumpjet unit. The slower speeds it drops to as
		 * it closes on its destination are expressed as fractions of this.
		 */
		int JumpjetSpeed;

		/*
		 * This is how many leptons a jumpjet unit gains or loses in altitude each game
		 * tick while it is climbing toward or dropping to its cruise height.
		 */
		double JumpjetClimb;

		/*
		 * This is the altitude, expressed in leptons, that a jumpjet unit levels off at
		 * and travels at once it has finished climbing.
		 */
		int JumpjetCruiseHeight;

		/*
		 * This is how much speed a jumpjet unit picks up each game tick while working up
		 * to its travel speed. It sheds speed half again as fast as it gains it.
		 */
		double JumpjetAcceleration;

		/*
		 * This is the number of complete wobble cycles a jumpjet unit performs each
		 * second. It works with JumpjetWobbleDeviation to keep a hovering unit from
		 * appearing to hang motionless in the air.
		 */
		double JumpjetWobblesPerSecond;

		/*
		 * This is how far above and below its flight level, expressed in leptons, that a
		 * jumpjet unit drifts as it wobbles.
		 */
		int JumpjetWobbleDeviation;

		/*
		 * These are the suppression distances, expressed in cells and listed per radar
		 * event type. An event raised within this distance of a live event of the same
		 * kind is swallowed rather than added to the radar.
		 */
		TypeList<int> RadarEventSuppressionDistances;

		/*
		 * These are the durations, expressed in game frames and listed per radar event
		 * type, that an event stays drawn after its box has finished shrinking.
		 */
		TypeList<int> RadarEventVisibilityDurations;

		/*
		 * These are the lifetimes, expressed in game frames and listed per radar event
		 * type, so that a combat ping can linger longer than a dropzone marker.
		 */
		TypeList<int> RadarEventDurations;

		/*
		 * This is the damage the ion cannon inflicts at its point of impact. The computer
		 * also weighs a candidate target's strength against it when judging whether the
		 * shot would be a kill.
		 */
		int IonCannonDamage;

		/*
		 * This is the distance from a railgun beam, expressed in leptons, within which
		 * every object is damaged and not just the one that was aimed at.
		 */
		int RailgunDamageRadius;

		/*
		 * This is the tactical map zoom factor that a scripted zoom-in action pushes the
		 * camera to.
		 */
		double ZoomInFactor;

		/*
		 * This is the chance each frame that an object damaged below the ConditionRed
		 * health level throws off a spark.
		 */
		double ConditionRedSparkingProbability;

		/*
		 * This is the chance each frame that an object damaged below the ConditionYellow
		 * health level, but not yet below ConditionRed, throws off a spark.
		 */
		double ConditionYellowSparkingProbability;

		/*
		 * This is the damage inflicted when an explosion sets off the tiberium growing in
		 * a cell, which is what makes a tiberium field chain react.
		 */
		int TiberiumExplosionDamage;

		/// Unused
		int TiberiumStrength;

		/*
		 * This is the floor on the power fraction that build times are divided by, so
		 * that a house with almost no power still produces at some usable rate.
		 */
		double MinLowPowerProductionSpeed;

		/*
		 * This weights the build time discount a house earns for owning more than one
		 * factory of the appropriate kind. If zero, then extra factories grant none.
		 */
		double MultipleFactory;

		/*
		 * This controls how widely a meteor impact deforms the terrain (0 - 4). Zero
		 * leaves the ground alone, one dents only the impact cell, and higher values
		 * spread the deformation out over the surrounding cells.
		 */
		int CraterLevel;

		/*
		 * This is the chance that a tree standing next to a burning one catches alight
		 * itself, which is what lets a blaze spread through woodland.
		 */
		double TreeFlammability;

		/// Unused
		double MissileSpeedVar;

		/*
		 * This is how far a homing missile's rate of turn is allowed to vary above and
		 * below its nominal value, which gives the missile its weaving flight path.
		 */
		double MissileROTVar;

		/*
		 * Pointer to the weapon that a descending drop pod strafes the ground with. If
		 * NULL, then the pod falls harmlessly.
		 */
		WeaponTypeClass const * DropPodWeapon;

		/*
		 * This is the altitude, expressed in leptons, that a drop pod is created at above
		 * the cell it is to land on.
		 */
		int DropPodHeight;

		/*
		 * This is the slowest a drop pod may fall. A pod high in the air descends faster
		 * than this, but as it nears the ground its speed is held at this floor.
		 */
		int DropPodSpeed;

		/*
		 * This is the angle of descent, expressed in radians, that a drop pod falls at.
		 * It divides the pod's speed between forward and downward travel and fixes how
		 * far to one side of the landing cell the pod is released.
		 */
		double DropPodAngle;

		/*
		 * This scales the distance the tactical map travels with each scroll step, so the
		 * overall scrolling rate can be tuned without touching the inertia ramp.
		 */
		double ScrollMultiplier;

		/*
		 * This is the chance that the crew of a destroyed vehicle bails out as an
		 * infantry survivor rather than dying with it.
		 */
		double CrewEscape;

		/*
		 * This is how much cost or strength must be destroyed to rock the screen by one
		 * shake, so a larger value means only the bigger explosions disturb the view.
		 */
		int ShakeScreen;

		/*
		 * This is the height, expressed in leptons, that a hovering unit rides above the
		 * ground. Falling below it adds lift in proportion to how far under it the unit
		 * has sunk.
		 */
		int HoverHeight;

		/*
		 * This is the period, expressed in minutes, of the slow vertical bob a hovering
		 * unit performs. Odd and even units are given slightly different periods so that
		 * a group of them does not bob in unison.
		 */
		double HoverBob;

		/*
		 * This scales the throttle of a hovering unit that is continuing in a straight
		 * line, letting it build up more speed than one that is weaving between cells.
		 */
		double HoverBoost;

		/*
		 * This is how long a hovering unit takes to work up to full throttle, expressed
		 * in minutes.
		 */
		double HoverAcceleration;

		/*
		 * This is how long a hovering unit takes to close its throttle again, expressed
		 * in minutes.
		 */
		double HoverBrake;

		/*
		 * This is the fraction of its vertical bounce that a hovering unit keeps from one
		 * game tick to the next, which settles it instead of letting it oscillate.
		 */
		double HoverDampen;

		/*
		 * This is how long a factory waits, expressed in minutes, before trying again to
		 * send out a completed object whose exit was temporarily blocked.
		 */
		double PlacementDelay;

		/// Unused
		TypeList<VoxelAnimTypeClass const *> ExplosiveVoxelDebris;
		VoxelAnimTypeClass const * TireVoxelDebris;
		VoxelAnimTypeClass const * ScrapVoxelDebris;
		int BridgeVoxelMax;

		/*
		 * This is the number of steps a cloaking device takes to fade an object fully out
		 * of sight. The stage reached is scaled against it to pick how solid the object
		 * still appears.
		 */
		int CloakingStages;

		/*
		 * This is the radius, expressed in cells, of the map that a reveal-around-waypoint
		 * trigger action uncovers.
		 */
		int RevealTriggerRadius;

		/*
		 * This is the weight a vehicle must reach before it cracks the ice it drives
		 * over. Cracked ice stays passable until it freezes solid again.
		 */
		double IceCrackingWeight;

		/*
		 * This is the weight at which a vehicle breaks clean through the ice instead of
		 * merely cracking it, sinking and taking itself out of the game.
		 */
		double IceBreakingWeight;

		/*
		 * These are the sound effects that cracking ice may play, one of which is picked
		 * at random each time a cell cracks.
		 */
		TypeList<int> IceCrackSounds;

		/*
		 * This controls whether ground lying against the back of a cliff is treated as
		 * impassable rock. Zero leaves it alone and a value of two converts its land
		 * type, which keeps units out of the unreachable strip behind a cliff face.
		 */
		signed char CliffBackImpassability;

		/*
		 * This is the multiple of its own cost that an object must destroy in order to
		 * earn one full point of experience toward promotion.
		 */
		double VeteranRatio;

		/*
		 * This is the fraction of extra firepower granted to an object that has earned
		 * the combat ability, so a value of one doubles the damage it deals.
		 */
		double VeteranCombat;

		/*
		 * This is the fraction of extra speed granted to an object that has earned the
		 * faster movement ability.
		 */
		double VeteranSpeed;

		/*
		 * This is the fraction of extra sight range granted to an object that has earned
		 * the scouting ability.
		 */
		double VeteranSight;

		/*
		 * This is the fraction by which incoming damage is divided down for an object
		 * that has earned the armor ability, so a value of one halves what it takes.
		 */
		double VeteranArmor;

		/*
		 * This is the fraction by which the reload delay is divided down for an object
		 * that has earned the rate of fire ability.
		 */
		double VeteranROF;

		/*
		 * This is the ceiling on the experience an object may accumulate, which fixes the
		 * highest rank it can ever be promoted to.
		 */
		double VeteranCap;

		/*
		 * This is the sound effect played as an object cloaks or reappears.
		 */
		VocType CloakSound;

		/*
		 * This is the sound effect played when a building or unit is sold back for a
		 * refund.
		 */
		VocType SellSound;

		/*
		 * This is the sound effect played when a game a player is watching in the lobby
		 * is closed off, either by its host or because play has started.
		 */
		VocType GameClosed;

		/*
		 * This is the sound effect played when a message arrives from another player.
		 */
		VocType IncomingMessage;

		/*
		 * This is the sound effect played when a network or system error is reported to
		 * the player.
		 */
		VocType SystemError;

		/*
		 * This is the sound effect played when the host alters the game options in a
		 * multiplayer setup dialog.
		 */
		VocType OptionsChanged;

		/*
		 * This is the sound effect played when a newly formed game appears in the lobby's
		 * list of games waiting for players.
		 */
		VocType GameForming;

		/*
		 * This is the sound effect played when another player leaves the game or channel.
		 */
		VocType PlayerLeft;

		/*
		 * This is the sound effect played when another player joins the game or channel.
		 */
		VocType PlayerJoined;

		/// Unused
		VocType Construction;

		/*
		 * These are the two sound effects the credit counter ticks with -- the first as
		 * the total climbs and the second as it falls.
		 */
		TypeList<int> CreditTicks;

		/*
		 * This is the sound effect played as a building crumbles when it is destroyed.
		 */
		VocType CrumbleSound;

		/*
		 * This is the sound effect played as a newly completed building is placed onto
		 * the map from the sidebar.
		 */
		VocType BuildingSlam;

		/*
		 * This is the sound effect played when the radar map comes online.
		 */
		VocType RadarOn;

		/*
		 * This is the sound effect played when the radar map goes offline.
		 */
		VocType RadarOff;

		/*
		 * This is the sound effect used to scold the player for an order that cannot be
		 * carried out, such as repairing a building that is already at full strength.
		 */
		VocType ScoldSound;

		/*
		 * This is the sound effect played as a defensive structure charges its turret
		 * before it fires.
		 */
		VocType TeslaCharge;

		/// Unused
		VocType TeslaZap;

		/*
		 * This is the general purpose click, used to acknowledge a button press or an
		 * order that was accepted.
		 */
		VocType GenericClick;

		/*
		 * This is the general purpose beep. The options dialog plays it through each of
		 * the volume sliders so that the player can hear the level being set.
		 */
		VocType GenericBeep;

		/*
		 * This is the sound effect played when a building takes a hit heavy enough to
		 * knock it down to half strength, and again when it takes a major one.
		 */
		VocType BlowupSound;

		/*
		 * This is the sound effect played when a crate heals up the player's whole base.
		 */
		VocType HealCrateSound;

		/*
		 * This is the sound effect played when the parachute of an air dropped object
		 * opens.
		 */
		VocType ChuteSound;

		/*
		 * This is the sound effect played when the player orders the current selection to
		 * stop.
		 */
		VocType StopSound;

		/*
		 * This is the sound effect played when the player orders the current selection to
		 * guard.
		 */
		VocType GuardSound;

		/*
		 * This is the sound effect played when the player orders the current selection to
		 * scatter.
		 */
		VocType ScatterSound;

		/*
		 * This is the sound effect played when the player orders the current selection to
		 * deploy.
		 */
		VocType DeploySound;

		/*
		 * This is the sound effect played when an ion storm throws down a lightning bolt.
		 */
		VocType LightningSound;

		/*
		 * These are the rates at which a house short of power was meant to build, at its
		 * worst and at its best, expressed as a fraction of its full build speed.
		 */
		double WorstLowPowerBuildRateCoefficient;
		double BestLowPowerBuildRateCoefficient;

		/*
		 * This scales the time taken to build a wall section, letting walls go up faster
		 * than their cost alone would allow for.
		 */
		double WallBuildSpeedCoefficient;

		/*
		 * This is how much longer a charge and drain superweapon runs for than it took to
		 * charge. The time remaining is scaled up by it when the weapon is switched on
		 * and scaled back down again when it is switched off.
		 */
		double ChargeToDrainRatio;

		/*
		 * This converts damage soaked up by an active firestorm wall into drain time
		 * taken off the firestorm superweapon, so holding the shield up under heavy fire
		 * exhausts it sooner.
		 */
		double DamageToFirestormDamageCoefficient;

		/*
		 * This scales the speed of a tracked vehicle that is climbing to a higher cell.
		 */
		double TrackedUphill;

		/*
		 * This scales the speed of a tracked vehicle that is descending to a lower cell.
		 */
		double TrackedDownhill;

		/*
		 * This scales the speed of a wheeled vehicle that is climbing to a higher cell.
		 */
		double WheeledUphill;

		/*
		 * This scales the speed of a wheeled vehicle that is descending to a lower cell.
		 */
		double WheeledDownhill;

		/*
		 * This is the limit, expressed in leptons, on how far from its building a
		 * spotlight beam may roam. It anchors the arc the beam rotates about, and the
		 * beam's travel out from SpotlightLocationRadius is graded into ten sweep stages.
		 */
		int SpotlightMovementRadius;

		/*
		 * This is the distance ahead of its building, expressed in leptons, that a
		 * spotlight aims at. A target that strays beyond this range is given up on.
		 */
		int SpotlightLocationRadius;

		/*
		 * This is the fastest a spotlight may sweep. Its acceleration is clamped so that
		 * the sweep never builds up beyond this rate.
		 */
		double SpotlightSpeed;

		/*
		 * This is how quickly a sweeping spotlight gathers and loses speed, which keeps
		 * it from snapping to a stop at the end of each sweep.
		 */
		double SpotlightAcceleration;

		/*
		 * This is the width of the arc a sweeping spotlight covers before it reverses
		 * and sweeps back the other way.
		 */
		double SpotlightAngle;

		/*
		 * This is added to a spotlight's own radius to get the size of the pool of light
		 * it casts on the ground.
		 */
		int SpotlightRadius;

		/*
		 * This is the direction the wind blows, which drifts smoke and gas particles
		 * sideways as they rise.
		 */
		FacingType WindDirection;

		/// Unused
		LEPTON CameraRange;

		/*
		 * This is the cruising altitude, expressed in leptons, used by anything that
		 * flies without an altitude of its own.
		 */
		int FlightLevel;

		/*
		 * This is the sound of a building slamming down into place as it is deployed.
		 */
		VocType BuildingDrop;

		/// Unused
		TypeList<SmudgeTypeClass const *> Scorches;
		TypeList<SmudgeTypeClass const *> Scorches1;
		TypeList<SmudgeTypeClass const *> Scorches2;
		TypeList<SmudgeTypeClass const *> Scorches3;
		TypeList<SmudgeTypeClass const *> Scorches4;
		TypeList<SmudgeTypeClass const *> Craters;

		/*
		 * This is the service depot that units and aircraft dock with in order to be
		 * repaired or sold back.
		 */
		BuildingTypeClass const * RepairBay;

		/*
		 * These are the gate buildings for each side. The first of each pair closes an
		 * east-west wall and the second a north-south one.
		 */
		BuildingTypeClass const * GDIGateOne;
		BuildingTypeClass const * GDIGateTwo;
		BuildingTypeClass const * NodGateOne;
		BuildingTypeClass const * NodGateTwo;

		/*
		 * This is the wall tower, recognized specially so that it stitches itself into
		 * the neighboring wall segments and so that upgrading one rotates its turret.
		 */
		BuildingTypeClass const * WallTower;

		/*
		 * These are the power structures the computer chooses between when it needs more
		 * power, and the set its base rebuilding logic recognizes as a power plant.
		 */
		BuildingTypeClass const * GDIPowerPlant;
		BuildingTypeClass const * GDIPowerTurbine;
		BuildingTypeClass const * NodRegularPower;
		BuildingTypeClass const * NodAdvancedPower;

		/*
		 * This is the building that raises the firestorm wall, recognized specially so
		 * that the wall logic can find every generator a house owns.
		 */
		BuildingTypeClass const * GDIFirestormGenerator;

		/*
		 * These are the hunter seeker units each side sends out when its hunter seeker
		 * control is fired.
		 */
		UnitTypeClass const * GDIHunterSeeker;
		UnitTypeClass const * NodHunterSeeker;

		/*
		 * These name the buildings that fill each role in the computer's base, in its
		 * order of preference. The first entry it may own is the one it queues up.
		 * BuildTech is the exception -- it is never built from, and serves only to name
		 * the building that satisfies a generic tech prerequisite.
		 */
		TypeList<BuildingTypeClass const *> BuildConst;
		TypeList<BuildingTypeClass const *> BuildPower;
		TypeList<BuildingTypeClass const *> BuildRefinery;
		TypeList<BuildingTypeClass const *> BuildBarracks;
		TypeList<BuildingTypeClass const *> BuildTech;
		TypeList<BuildingTypeClass const *> BuildWeapons;

		/// Unused
		TypeList<BuildingTypeClass const *> BuildDefense;
		TypeList<BuildingTypeClass const *> BuildPDefense;
		TypeList<BuildingTypeClass const *> BuildAA;
		TypeList<BuildingTypeClass const *> BuildHelipad;

		/*
		 * These are the buildings that give the computer a radar, in its order of
		 * preference. The first one it may own is the one it queues up.
		 */
		TypeList<BuildingTypeClass const *> BuildRadar;

		/*
		 * These are the wall buildings the computer uses when it walls in its base. The
		 * first one it is allowed to own is the one it builds.
		 */
		TypeList<BuildingTypeClass const *> ConcreteWalls;

		/*
		 * These are the gate buildings the computer will fit into a wall it is laying,
		 * one list for each orientation.
		 */
		TypeList<BuildingTypeClass const *> NSGates;
		TypeList<BuildingTypeClass const *> EWGates;

		/*
		 * These give the number of wall segments a GDI computer house will lay -- the
		 * base count plus the coefficient scaled by the difficulty level.
		 */
		double GDIWallDefense;
		double GDIWallDefenseCoefficient;

		/*
		 * These are the fractions of its base's cost that a Nod or a GDI computer house
		 * wants to have invested in base defenses.
		 */
		double NodBaseDefenseCoefficient;
		double GDIBaseDefenseCoefficient;

		/*
		 * This scales how much defense the computer wants in answer to a threat -- an
		 * enemy's risk value is multiplied by it to get the defense worth building.
		 */
		int ComputerBaseDefenseResponse;

		/*
		 * If a computer house is allowed to see through a disguise when it scans for a
		 * target, then this flag will be true. A house a player controls never does.
		 */
		bool AIDetectDisguise;

		/*
		 * This caps the anti-air, anti-armor and anti-infantry ratings worked out for a
		 * base defense, so one exceptional weapon cannot dominate the computer's choice.
		 */
		int MaximumBaseDefenseValue;

		/*
		 * This is the unit that deploys into a construction yard -- the MCV. A house
		 * with neither a base nor one of these has lost.
		 */
		UnitTypeClass const * BaseUnit;

		/*
		 * These are the unit types that count as harvesters, listed so that the game can
		 * recognize one without asking every type whether it harvests. The whole list is
		 * searched when recognizing a harvester, but only the first entry is ever built,
		 * so the computer will never produce any of the others.
		 */
		TypeList<UnitTypeClass const *> HarvesterUnit;

		/*
		 * These are the aircraft that come with the helipad and the airfield that dock
		 * them, unless the IsSeparate flag says they must be bought separately.
		 */
		TypeList<AircraftTypeClass const *> PadAircraft;

		/*
		 * These are the animations of a burning building, ordered from the smallest
		 * flame to the largest. The smaller ones are chosen far more often, and the
		 * largest only for a building big enough to warrant it. Three are expected, since
		 * the first three are picked by index and any beyond them are never used.
		 */
		TypeList<AnimTypeClass const *> OnFire;

		/*
		 * These are the animations of a burning tree. One of the first two is picked at
		 * random whenever a wooden terrain object catches fire.
		 */
		TypeList<AnimTypeClass const *> TreeFire;

		/// Unused
		AnimTypeClass const * Smoke1;
		AnimTypeClass const * Smoke2;

		/*
		 * These are the animations drawn on the firestorm generator itself -- one while
		 * its wall is up, the other while the generator merely sits charged and idle.
		 */
		AnimTypeClass const * FirestormActiveAnim;
		AnimTypeClass const * FirestormIdleAnim;

		/*
		 * These are the animations played over whatever a rising firestorm wall
		 * destroys -- one for an aircraft, the other for a ground object.
		 */
		AnimTypeClass const * FirestormAirAnim;
		AnimTypeClass const * FirestormGroundAnim;

		/*
		 * This is the animation flashed on the destination cell when the player orders
		 * a unit to move there.
		 */
		AnimTypeClass const * MoveFlash;

		/*
		 * These are the parachutes used when something is dropped from the air -- one
		 * for falling ordnance, the other for a parachuting object.
		 */
		AnimTypeClass const * BombParachute;
		AnimTypeClass const * Parachute;

		/*
		 * These are the water splash animations, ordered from the smallest to the
		 * largest. Which one appears depends on how much damage struck the water.
		 */
		TypeList<AnimTypeClass const *> SplashList;

		/*
		 * These are the fires left burning where something has been destroyed. The
		 * larger one is used for the fiercer blazes.
		 */
		AnimTypeClass const * SmallFire;
		AnimTypeClass const * LargeFire;

		/// Unused
		InfantryTypeClass const * Paratrooper;

		/*
		 * This is the infantry type a disguised infantryman masquerades as, supplying
		 * both the name and the artwork an enemy player sees.
		 */
		InfantryTypeClass const * Disguise;

		/*
		 * This is the survivor that emerges from a neutral or civilian object, and
		 * occasionally from an armed one as well.
		 */
		InfantryTypeClass const * Technician;

		/*
		 * This is the survivor that sometimes emerges from a building which itself
		 * produces buildings, in place of the usual crew.
		 */
		InfantryTypeClass const * Engineer;

		/// Unused
		InfantryTypeClass const * Pilot;

		/*
		 * This is the infantry type that emerges as the survivor of a destroyed object,
		 * unless something more specific applies.
		 */
		InfantryTypeClass const * Crew;

		/*
		 * This is the warhead a napalm crate burns its surroundings with.
		 */
		WarheadTypeClass const * FlameDamage;

		/*
		 * This is the warhead an attached fire animation damages the object it is
		 * burning with.
		 */
		WarheadTypeClass const * FlameDamage2;

		/*
		 * This is the warhead the atom bomb's blast damages with.
		 */
		WarheadTypeClass const * NukeWarhead;

		/// Unused
		BulletTypeClass const * NukeProjectile;
		BulletTypeClass const * NukeDown;
		WarheadTypeClass const * EMPulseWarhead;
		BulletTypeClass const * EMPulseProjectile;

		/*
		 * This is the warhead for demolition damage -- a planted charge, an exploding
		 * Tiberium crystal, and the like.
		 */
		WarheadTypeClass const * C4Warhead;

		/*
		 * This is the warhead an Ion Cannon strike damages with. It always destroys a
		 * wall or a bridge outright rather than rolling for it.
		 */
		WarheadTypeClass const * IonCannonWarhead;

		/*
		 * This is the warhead that destroys whatever is caught in a firestorm wall as
		 * the wall comes up.
		 */
		WarheadTypeClass const * FirestormWarhead;

		/*
		 * This is the warhead the vein tendrils growing out of a veinhole attack with.
		 */
		WarheadTypeClass const * VeinholeWarhead;

		/*
		 * This is the warhead an ion storm's lightning bolts damage with.
		 */
		WarheadTypeClass const * IonStormWarhead;

		/*
		 * This is the terrain type that supplies a veinhole monster's artwork and
		 * strength.
		 */
		TerrainTypeClass const * VeinholeTypeClass;

		/// Unused
		ParticleSystemTypeClass const * DefaultLargeGreySmokeSystem;
		ParticleSystemTypeClass const * DefaultSmallGreySmokeSystem;
		ParticleSystemTypeClass const * DefaultSparkSystem;
		ParticleSystemTypeClass const * DefaultLargeRedSmokeSystem;
		ParticleSystemTypeClass const * DefaultSmallRedSmokeSystem;
		ParticleSystemTypeClass const * DefaultDebrisSmokeSystem;
		ParticleSystemTypeClass const * DefaultFireStreamSystem;

		/*
		 * This is the particle system spawned where a firestorm wall consumes a unit
		 * or an aircraft.
		 */
		ParticleSystemTypeClass const * DefaultFirestormExplosionSystem;

		/// Unused
		ParticleSystemTypeClass const * DefaultTestParticleSystem;
		ParticleSystemTypeClass const * DefaultRepairParticleSystem;

		/*
		 * These are the threat weights a techno type falls back on when it declares no
		 * target-selection coefficients of its own.
		 */
		double MyEffectivenessCoefficientDefault;
		double TargetEffectivenessCoefficientDefault;
		double TargetSpecialThreatCoefficientDefault;
		double TargetStrengthCoefficientDefault;
		double TargetDistanceCoefficientDefault;

		/*
		 * These are the threat weights used by a house whose threat rating node is not
		 * active, standing in for the per-type coefficients a smarter house reads.
		 */
		double DumbMyEffectivenessCoefficient;
		double DumbTargetEffectivenessCoefficient;
		double DumbTargetSpecialThreatCoefficient;
		double DumbTargetStrengthCoefficient;
		double DumbTargetDistanceCoefficient;

		/*
		 * This is added to a target's threat rating when it belongs to the house this
		 * one has singled out as its enemy, biasing target selection toward that foe.
		 */
		double EnemyHouseThreatBonus;

		/*
		 * These are the distances at which a hunter seeker detonates on its target and
		 * at which it begins the dive toward it.
		 */
		int HunterSeekerDetonateProximity;
		int HunterSeekerDescendProximity;

		/*
		 * These are the rates a hunter seeker dives and climbs at, expressed in leptons
		 * per frame. The emerge speed applies while it is still leaving its building.
		 */
		int HunterSeekerDescentSpeed;
		int HunterSeekerAscentSpeed;
		int HunterSeekerEmergeSpeed;

		/*
		**	This specifies the turbo boost speed for missiles when they are fired upon
		**	aircraft and the weapon is specified as having a turbo boost bonus.
		*/
		double TurboBoost;

		/*
		**	This specifies the average number of minutes between each computer attack.
		*/
		double AttackInterval;

		/*
		**	This specifies the average minutes delay before the computer will begin
		**	its first attack upon the player. The duration is also modified by the
		**	difficulty level.
		*/
		double AttackDelay;

		/*
		**	If the power ratio falls below this percentage, then a power emergency is
		**	in effect. At such times, the computer might decide to sell off some
		**	power hungry buildings in order to alleviate the situation.
		*/
		double PowerEmergencyFraction;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of airstrips.
		*/
		double AirstripRatio;

		/*
		**	Limit the number of airstrips to this amount.
		*/
		int AirstripLimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of helipads.
		*/
		double HelipadRatio;

		/*
		**	Limit the number of helipads to this amount.
		*/
		int HelipadLimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of Tesla Coils.
		*/
		double TeslaRatio;

		/*
		**	Limit tesla coil production to this maximum.
		*/
		int TeslaLimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of anti-aircraft defense.
		*/
		double AARatio;

		/*
		**	Limit anti-aircraft building quantity to this amount.
		*/
		int AALimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of defensive structures.
		*/
		double DefenseRatio;

		/*
		**	This is the limit to the number of defensive building that can be built.
		*/
		int DefenseLimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of war factories.
		*/
		double WarRatio;

		/*
		**	War factories are limited to this quantity for the computer controlled player.
		*/
		int WarLimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of infantry producing structures.
		*/
		double BarracksRatio;

		/*
		**	No more than this many barracks can be built.
		*/
		int BarracksLimit;

		/*
		**	Refinery building is limited to this many refineries.
		*/
		int RefineryLimit;

		/*
		**	This specifies the percentage of the base (by building quantity) that should
		**	be composed of refineries.
		*/
		double RefineryRatio;

		/*
		**	The computer is limited in the size of the base it can build. It is limited to the
		**	size of the largest human opponent base plus this surplus count.
		*/
		int BaseSizeAdd;

		/*
		**	If the power surplus is less than this amount, then the computer will
		**	build power plants.
		*/
		int PowerSurplus;

		/*
		**	The computer will build infantry if their cash reserve is greater than this amount.
		*/
		int InfantryReserve;

		/*
		**	This factor is multiplied by the number of buildings in the computer's base and infantry
		**	are always built until it matches that number.
		*/
		int InfantryBaseMult;

		/*
		**	Solo play has money crate amount fixed according to this rule value.
		*/
		int SoloCrateMoney;

		/*
		 * This is the strength given to a terrain object that declares none of its own
		 * -- trees, mostly.
		 */
		int TreeStrength;

		/*
		**	If a unit type is specified here, then the unit crate will generate
		**	a unit of this type (always).
		*/
		UnitTypeClass const * UnitCrateType;

		/*
		**	This is the time to delay between patrol-to-waypoint target scanning.
		*/
		double PatrolTime;

		/*
		**	This is the time interval that checking to create teams will span. The
		**	smaller this number, the more often checking for team creation will occur.
		*/
		TypeList<int> TeamDelays;

		/*
		 * This is the delay, expressed in game frames, before a computer house first
		 * picks an enemy to hate, with one entry per difficulty level. It is only ever
		 * imposed outside of solo play, where an enemy is settled upon immediately.
		 */
		TypeList<int> AIHateDelays;

		/*
		 * This is how long a team may wait, expressed in game frames, without ever
		 * receiving a member before it gives up and dissolves itself. Only teams outside
		 * of solo play are abandoned this way, since a scenario may well raise a team
		 * long before the reinforcement that fills it arrives.
		 */
		int DissolveUnfilledTeamDelay;

		/*
		 * These are the scores, one entry per difficulty level, that the computer gives
		 * each kind of enemy object while hunting for the best Ion Cannon target.
		 */
		TypeList<int> AIIonCannonConYardValue;
		TypeList<int> AIIonCannonWarFactoryValue;
		TypeList<int> AIIonCannonPowerValue;
		TypeList<int> AIIonCannonEngineerValue;
		TypeList<int> AIIonCannonThiefValue;
		TypeList<int> AIIonCannonHarvesterValue;
		TypeList<int> AIIonCannonMCVValue;
		TypeList<int> AIIonCannonAPCValue;
		TypeList<int> AIIonCannonBaseDefenseValue;
		TypeList<int> AIIonCannonPlugValue;
		TypeList<int> AIIonCannonHelipadValue;
		TypeList<int> AIIonCannonTempleValue;

		/*
		 * If the computer's spare cash falls below this amount, it switches to its
		 * alternate production priorities instead of its preferred build order.
		 */
		int AIAlternateProductionCreditCutoff;

		/*
		 * This is the percentage of the normal starting credits a computer house is
		 * given in a multiplayer game, with one entry per difficulty level.
		 */
		TypeList<int> MultiplayerAICreditMultipliers;

		/*
		 * These bound the number of base defense teams the computer keeps, one entry
		 * per difficulty level, hardest game setting first. The minimum applies only if
		 * UseMinDefenseRule is true.
		 */
		TypeList<int> MinimumAIDefensiveTeams;
		TypeList<int> MaximumAIDefensiveTeams;

		/*
		 * This is the greatest number of teams the computer will keep in the field,
		 * with one entry per difficulty level, hardest game setting first.
		 */
		TypeList<int> TotalAITeamCap;

		/*
		 * This is the chance that the computer answers a power shortage by adding a
		 * turbine upgrade to a plant it already owns rather than building another.
		 */
		double AIUseTurbineUpgradeChance;

		/*
		 * This is the chance, one entry per difficulty level and hardest game setting
		 * first, that a newly built unit is sent to the oldest team still waiting for
		 * members.
		 */
		TypeList<int> FillEarliestTeamProbability;

		/*
		**	This is the arbitrary delay to make all cloaking objects remain uncloaked
		**	before having it recloak.
		*/
		double CloakDelay;

		/*
		**	This is an overall game apparent speed bias to use for object
		**	movement purposes.
		*/
		double GameSpeedBias;

		/*
		**	If a potential target is close to the base then increase
		**	the likelyhood of attacking it by this bias factor.
		*/
		double NervousBias;

		/*
		**	When an explosive object explodes, the damage will spread out
		**	by this factor. The value represents the number of cells radius
		**	that the damage will spread for every 100 points of raw damage at
		**	the explosion center point.
		*/
		double ExplosionSpread;

		/*
		**	For weapons specially marked to check for nearby friendly buildings
		**	when scanning for good targets, this indicates the scan radius. Such
		**	weapons will supress firing on enemies if they are in close proximity
		**	to allied buildings.
		*/
		LEPTON SupressRadius;

		/*
		**	This is the maximum number of IQ settings available. The human player is
		**	presumed to be at IQ level zero.
		*/
		int MaxIQ;

		/*
		**	The IQ level at which super weapons will be automatically fired by the computer.
		*/
		int IQSuperWeapons;

		/*
		**	The IQ level at which production is automatically controlled by the computer.
		*/
		int IQProduction;

		/*
		**	The IQ level at which newly produced units start out in guard area mode instead
		**	of normal guard mode.
		*/
		int IQGuardArea;

		/*
		**	The IQ level at which the computer will be able to decide what gets repaired
		**	or sold.
		*/
		int IQRepairSell;

		/*
		**	At this IQ level or higher, a unit is allowed to automatically try to crush
		**	an atagonist if possible.
		*/
		int IQCrush;

		/*
		**	The unit/infantry will try to scatter if an incoming threat
		**	is detected.
		*/
		int IQScatter;

		/*
		**	Tech level at which the computer will scan the contents of a transport
		**	in order to pick the best target to fire upon.
		*/
		int IQContentScan;

		/*
		**	Aircraft replacement production occurs at this IQ level or higher.
		*/
		int IQAircraft;

		/*
		**	Checks for and replaces lost harvesters.
		*/
		int IQHarvester;

		/*
		**	Is allowed to sell a structure being damaged.
		*/
		int IQSellBack;

		/*
		 * This is the number of cells of clearance the computer leaves between the
		 * buildings of its base when it lays out base nodes.
		 */
		int AIBaseSpacing;

		/*
		**	The silver and wood crates in solo play will have these powerups.
		*/
		CrateType SilverCrate;
		CrateType WoodCrate;

		/*
		**	This specifies the minimum number of crates to place on the map in spite
		**	of the number of actual human players.
		*/
		int CrateMinimum;

		/*
		**	This specifies the crate maximum quantity to use.
		*/
		int CrateMaximum;

		/*
		**	Landing zone maximum alternate zone scan radius.
		*/
		LEPTON LZScanRadius;

		/*
		 * This is the flare dropped to mark a drop zone. While it burns it reveals the
		 * map around itself, and it vanishes if a building is placed over it.
		 */
		AnimTypeClass const * FlareAnim;

		/*
		**	Multiplayer default settings.
		*/
		int MPMoney;
		int MPMaxMoney;
		int MPUnitCount;
		int MPBuildLevel;

		/*
		**	Drop zone reveal radius.
		*/
		LEPTON DropZoneRadius;

		/*
		**	This is the delay that multiplayer messages will remain on the screen.
		*/
		double MessageDelay;

		/*
		**	Savour delay between when scenario detects end and the actual
		**	end of the play.
		*/
		double SavourDelay;

		/*
		**	This is the maximum number of multiplayers allowed.
		*/
		int MaxPlayers;

		/*
		**	This is the delay between 'panic attacks' when the computer's base is under
		**	attack. This delay gives the previously assigned units a chance to affect the
		**	attacker before the computer sends more.
		*/
		double BaseDefenseDelay;

		/*
		**	These values control the team suspension logic for dealing with immedate base threats.
		**	When the base is attacked, all teams with less than the specified priority will be
		**	temporarily put on hold for the number of minutes specified.
		*/
		int SuspendPriority;
		double SuspendDelay;

		/*
		**	This serves as the fraction of a building's original cost that is converted
		**	into survivors (of some fashion). There are rounding and other marginal
		**	fudge effects, but this value is the greatest control over the survivor rate.
		*/
		double SurvivorFraction;

		/*
		 * This divides a destroyed building's cost when working out how many survivors
		 * emerge from it. The divisor is doubled for a building that was captured.
		 */
		int SurvivorDivisor;

		/*
		**	This is the aircraft reload rate expressed in minutes per ammo load.
		*/
		double ReloadRate;

		/*
		**	The average time (in minutes) between the computer autocreating a team
		**	from the team's autocreate list.
		*/
		double AutocreateTime;

		/*
		**	Build up time for buildings (minutes).
		*/
		double BuildupTime;

		/*
		 * These control the harvester's docking cycle -- the load rate is the frame
		 * delay of each harvesting step, the dump rate the minutes an unload takes.
		 */
		int HarvesterLoadRate;
		double HarvesterDumpRate;

		/*
		**	This is the amount of damage done by the atom bomb in solo missions. The
		**	damage done during multiplay will be 1/5th this value.
		*/
		int AtomDamage;

		/*
		**	This array controls the difficulty affects on the game. There is one
		**	difficulty class object for each difficulty level.
		*/
		DifficultyClass Diff[DIFF_COUNT];

		/*
		 * The time quake will do this percentage of damage to all units and buildings
		 * in the game. The number is expressed as a floating point percentage.
		 */
		double QuakeDamagePercent;

		/*
		**	Percentage chance that a time quake will occur with each chronoshift use.
		*/
		double QuakeChance;

		/*
		**	Ore (Tiberium) growth rate. The value is the number of minutes between
		**	growth steps.
		*/
		double GrowthRate;

		/*
		**	This specifies the number of minutes between each shroud regrowth process.
		*/
		double ShroudRate;

		/*
		 * This specifies the number of minutes between each fog of war regrowth pass.
		 */
		double FogRate;

		/*
		 * This specifies the number of minutes between each ice growth pass. Only snow
		 * theaters grow ice.
		 */
		double IceGrowthRate;

		/// Unused
		double VeinGrowthRate;

		/*
		 * This is how long cracked ice takes to heal back to solid, in game frames.
		 */
		int IceSolidifyDelay;

		/*
		 * These control how the ambient light drifts toward its desired level -- the
		 * rate is the delay in minutes between steps, the step how far each one moves.
		 */
		double AmbientLightChangeRate;
		double AmbientLightChangeStep;

		/*
		**	This is the average minutes between each generation of a random crate
		**	to be placed on the map if generating of random crates is indicated.
		*/
		double CrateTime;

		/*
		**	This specifies the number of minutes remaining before that if the mission timer
		**	gets to this level or below, it will be displayed in red.
		*/
		double TimerWarning;

		/// Unused
		int TiberiumTransmogrify;

		/*
		**	This specifies the minutes of delay between recharges for these
		**	special weapon types.
		*/
		double NukeTime;
		double EMPulseTime;
		double IonCannonTime;
		double FirestormTime;

		/*
		**	Other miscellaneous delay times.
		*/
		double SpeakDelay;
		double DamageDelay;

		/*
		**	This is the gravity constant used to control the arcing and descent of ballistic
		**	object such as grenades and artillery.
		*/
		int Gravity;

		/*
		 * This is the height, expressed in leptons, that an object must gain before its
		 * sight range widens by another increment.
		 */
		LEPTON LeptonsPerSightIncrease;

		/*
		**	The speed at which a projectile that travels at or slower will cause
		**	objects in the target location to scatter. This simulates the ability
		**	of targets to run for cover if the projectile gives them enough time
		**	to react.
		*/
		MPHType Incoming;

		/*
		**	Minimum and maximum damage allowed per shot.
		*/
		int MinDamage;
		int MaxDamage;

		/*
		 * This is the rate of repair for units and buildings. The rate is the
		 * number of strength points repaired per repair clock tick. The cost of
		 * repair is the (floating point) fractional cost to repair the object based
		 * on the full price of the object. Example; a value of 50% means that to
		 * repair the object from 1 damage point to full strength would cost 50% of
		 * the cost to build it from scratch.
		 */
		int RepairStep;
		double RepairPercent;
		int IRepairStep;

		/*
		**	This is the rate that objects with self healing will heal. They will repair a bit
		**	every 'this' number of minutes.
		*/
		double RepairRate;
		double URepairRate;
		double IRepairRate;

		/*
		 * These floating point values are used to determine the status (health bar
		 * color) of the game objects. Objects in the 'yellow' are in a cautionary
		 * state. Object in the 'red' are in a danger state.
		 */
		double ConditionGreen;
		double ConditionYellow;
		double ConditionRed;

		/*
		**	Average number of minutes between infantry random idle animations.
		*/
		double RandomAnimateTime;

		/*
		**	Close enough distance that is used to determine if the object should
		**	stop movement when blocked. If the distance to the desired destination
		**	is equal to this distance or less, but the path is blocked, then consider
		**	the object to have gotten "close enough" to the destination to stop.
		*/
		LEPTON CloseEnoughDistance;

		/*
		**	Stray distance to group team members within. The larger the distance,
		**	the looser the teams will move.
		*/
		LEPTON StrayDistance;

		/*
		**	If a vehicle is closer than this range to a target that it can crush
		**	by driving over it, then it will try to drive over it instead of firing
		**	upon it. The larger the value, the greater the 'bigfoot crush syndrome' is
		**	has.
		*/
		LEPTON CrushDistance;

		/*
		**	For area effect crate bonus items will affect all objects within this radius.
		*/
		LEPTON CrateRadius;

		/*
		**	Maximum scatter distances for homing and non-homing projectiles.
		*/
		LEPTON HomingScatter;
		LEPTON BallisticScatter;

		/*
		**	This is the refund percentage when selling off buildings and units
		**	on the repair pad (service depot).
		*/
		double RefundPercent;

		/*
		**	The strength of bridges is held here. By corollary, the strength of the
		**	demolition charge carried by Tanya is equal to this value as well.
		*/
		int BridgeStrength;

		/*
		**	This is the overall build speed bias. Multiply this value by the normal build
		**	delay to get the effective build delay.
		*/
		double BuildSpeedBias;

		/*
		**	This is the delay between the time a C4 bomb is planted and the time it will
		**	explode. The longer the delay, the greater safety margin for a demolitioner
		**	type. The short the delay, the less time the victim has to sell the building
		**	off.
		*/
		double C4Delay;

		/*
		**	The computer will only repair a structure if it has spare money greater than this
		**	amount. The thinking is that this will prevent the computer from frittering away
		**	all it's cash on repairing and thus leaving nothing for production of defenses.
		*/
		int RepairThreshhold;

		/*
		**	This is the delay (in minutes) between retries of a failed path. The longer the
		**	delay the faster the system, but the longer the units take to react to a blocked
		**	terrain event.
		*/
		double PathDelay;

		/*
		 * This is how long a unit waits, expressed in game frames, before it gives up
		 * on a blocked path and tries to route around the obstruction.
		 */
		int BlockagePathDelay;

		/*
		**	This is the special (debug version only) movie recorder timeout value. Each second
		**	results in about 2-3 megabytes.
		*/
		double MovieTime;

		/*
		**	These are the Tiberium scan distances. The short range scan is used to determine if the
		**	current field has been exhausted. The long range scan is used when finding a Tiberium
		**	field to harvest. Keep these ranges as small as possible.
		*/
		LEPTON TiberiumShortScan;
		LEPTON TiberiumLongScan;

		/*
		 * This is how often an ion storm throws a lightning bolt, expressed as a chance
		 * in 1000 per game frame.
		 */
		int LightningFrequency;

		/*
		 * This is the percentage chance that a lightning bolt strikes a random cell
		 * instead of hunting for a lightning rod or some other worthwhile target.
		 */
		int LightningRandomness;

		/*
		 * This is the damage a lightning bolt inflicts where it strikes.
		 */
		int LightningDamage;

		/// Unused
		int LightningDuration;

		/*
		 * This is the warning period between an ion storm being triggered and the storm
		 * actually breaking, expressed in seconds.
		 */
		int LightningDeferment;

		/*
		 * This is the percentage chance that a destroyable cliff tile collapses when
		 * something damages the cell it occupies.
		 */
		int CollapseChance;

		/*
		 * This is the total amount of weed a house can store, and the divisor the weed
		 * gauge on the sidebar is drawn against. A house must fill this store before its
		 * chemical missile will recharge, so it doubles as the price of that weapon.
		 */
		int WeedCapacity;

		/*
		 * These brighten a unit, an infantryman or an aircraft beyond the light of the
		 * cell it stands in, so that objects read clearly against the terrain.
		 */
		int ExtraUnitLight;
		int ExtraInfantryLight;
		int ExtraAircraftLight;

		/*
		**	Is the computer paranoid? If so, then it will band together with other computer
		**	paranoid players when the situation looks rough.
		*/
		bool IsComputerParanoid;

		/*
		**	Should helicopters shuffle their position between firing on their
		**	target?
		*/
		bool IsCurleyShuffle;

		/*
		 * If the fog is to be blended evenly into the terrain beneath it, then this flag
		 * will be true. Otherwise the fog is dithered on, covering every other pixel in a
		 * checkerboard so that half the terrain still shows through.
		 */
		bool IsBlendedFog;

		/*
		**	If the computer players will go to easy mode if there is more
		**	than one human player, this flag will be true.
		*/
		bool IsCompEasyBonus;

		/*
		**	If fine control of difficulty settings is desired, then set this value to true.
		**	Fine control allows 5 settings. The coarse control only allows three settings.
		*/
		bool IsFineDifficulty;

		/*
		**	If the harvester is to explode more violently than normal
		**	if it is carrying cargo, then this flag will be true.
		*/
		bool IsExplosiveHarvester;

		/*
		**	Show the health bar on the enemy units?
		*/
		bool IsHealthBar;

		/*
		**	If the base is to be revealed to a new ally, then this
		**	flag will be true.
		*/
		bool IsAllyReveal;

		/*
		**	Can the helipad (and airfield) be purchased separately from the associated
		**	aircraft.
		*/
		bool IsSeparate;

		/*
		**	Give target cursor for trees? Doing this will make targetting of trees easier.
		*/
		bool IsTreeTarget;

		/*
		**	Should civilan buildings and civilians display their true name rather than
		**	the generic "Civilian Building" and "Civilain"?
		*/
		bool IsNamed;

		/*
		**	Should player controlled vehicles automatically try to crush nearby infantry
		**	instead of required the player to manually direct them to crush.
		*/
		bool IsAutoCrush;

		/*
		**	Should the player controlled buildings and units automatically return fire when
		**	fired upon?
		*/
		bool IsSmartDefense;

		/*
		**	Should player controlled units try to scatter more easily in order to
		**	avoid damage or threats?
		*/
		bool IsScatter;

		/*
		 * If an object's height above the ground is allowed to widen the area it
		 * reveals, then this flag will be true.
		 */
		bool IsRevealByHeight;

		/*
		 * If subterranean units may be ordered to travel into shrouded territory, then
		 * this flag will be true. Aircraft are barred from it either way.
		 */
		bool IsShroudedSubteranneanMovesAllowed;

		/*
		 * If the shroud is allowed to creep back over ground the player has already
		 * explored, then this flag will be true. The interval is the ShroudRate.
		 */
		bool IsShroudGrow;

		/*
		**	Multiplayer default settings.
		*/
		bool IsMPShadowGrow;
		bool IsMPBasesOn;
		bool IsMPTiberiumGrow;
		bool IsMPCrates;
		bool IsMPAIPlayers;
		bool IsMPCaptureTheFlag;
		bool IsMPBridgeDestruction;

		/*
		 * If the computer is allowed to wall in its base, then these flags will be
		 * true. A Nod house needs both; any other side needs only the general one.
		 */
		bool NodAIBuildsWalls;
		bool AIBuildsWalls;

		/*
		 * If the computer must field at least MinimumAIDefensiveTeams defensive teams
		 * before it will consider any other kind of team, then this flag will be true.
		 */
		bool UseMinDefenseRule;

		/*
		 * This is the animation played over an object that an EM pulse has knocked out.
		 */
		AnimTypeClass const * EMPulseSparkles;

		/*
		 * This is the animation whose artwork stands in for an infantryman while he
		 * struggles inside a spider web.
		 */
		AnimTypeClass const * WebbedInfantry;

		/*
		 * This is the radius, in cells, within which a passing jumpjet shimmers any
		 * cloaked object below it, giving the position away.
		 */
		int JumpjetCloakDetectionRadius;

		/*
		 * These bound the number of infantry that arrive in a drop pod delivery. The
		 * count is picked at random between them each time the pods are called in.
		 */
		int DropPodInfantryMinimum;
		int DropPodInfantryMaximum;

		/*
		 * This is how long a unit's talk bubble stays on screen, in game frames.
		 */
		unsigned int TalkBubbleTime;

		/*
		 * These list the buildings that satisfy the GDI and the Nod factory
		 * prerequisite, so a side can meet it with whichever structure it owns.
		 */
		TypeList<int> PrerequisiteGDIFactory;
		TypeList<int> PrerequisiteNodFactory;

		/*
		 * This is the health ratio at or below which an engineer captures a building
		 * rather than merely damaging it. Above it, the engineer gets a damage cursor.
		 */
		float EngineerCaptureLevel;

		/// Unused
		float EngineerDamage;
};
