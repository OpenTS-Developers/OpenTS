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

#include "banim.hh"
#include "bsize.hh"
#include "bstate.hh"
#include "struct.hh"
#include "super.hh"

class OverlayTypeClass;
class IsometricTileTypeClass;
class BSurface;
class ShapeSet;

/***************************************************************************
**	Building types need some special information custom to buildings. This
**	is a derived class that elaborates these additional data elements.
*/
class BuildingTypeClass : public TechnoTypeClass
{
		typedef TechnoTypeClass BASECLASS;
	public:

		/*
		**	This is the structure type identifier. It can serve as a unique
		**	identification number for building types.
		*/
		StructType HeapID;

		/*
		**	This is a pointer to a list of offsets (from the upper left corner) that
		**	are used to indicate the building's "footprint". This footprint is used
		**	to determine building placement legality and terrain passibility.
		*/
		Cell const * OccupyList;

		/*
		**	The construction animation graphic data pointer is
		**	pointed to by this element.
		*/
		void const * BuildupData;

		/// Unused
		Point3D HalfDamageSmokeLocation1;
		Point3D HalfDamageSmokeLocation2;

		/*
		 * This is the time, expressed in minutes, that a gate stays open after the last thing
		 * has passed through it. When it expires the gate closes again of its own accord.
		 */
		double GateCloseDelay;

		/*
		 * This is the radius, expressed in leptons, of the light this building casts over the
		 * terrain around it. It only matters when the LightIntensity is non-zero.
		 */
		int LightVisibility;

		/*
		 * This is the brightness of the light this building casts, as a fixed point value
		 * scaled by 1000. If zero, then the building carries no light source at all.
		 */
		int LightIntensity;

		/*
		 * These are the color tints of the light this building casts, each a fixed point value
		 * scaled by 1000, so that a structure may glow in a color of its own.
		 */
		int LightRedTint;
		int LightGreenTint;
		int LightBlueTint;

		/*
		 * These are the pixel offsets from the building's center at which its primary and
		 * secondary weapons appear to fire from. If Point2D(0xFFFF, 0xFFFF), then the shot
		 * leaves the center instead.
		 */
		Point2D PrimaryFirePixelOffset;
		Point2D SecondaryFirePixelOffset;

		/*
		 * This is the overlay type a wall building turns into when it is placed. The building
		 * destroys itself on placement and leaves this overlay behind.
		 */
		OverlayTypeClass const * ToOverlay;

		/*
		 * This is the isometric tile a pavement building turns into when it is placed. Each
		 * cell of the footprint gets one and the building then destroys itself.
		 */
		IsometricTileTypeClass const * ToTile;

		/*
		 * This is the name of the shape file holding the construction animation, for the
		 * buildings whose buildup artwork is not named after the building itself.
		 */
		TStringID<15> BuildupFilename;

		/*
		 * This is the name of the building type this one upgrades. Such a type may only be
		 * placed onto an existing building of that name, where it becomes an upgrade.
		 */
		TStringID<23> PowersUpBuilding;

		/*
		 * This is the unit given away free when this building is first completed -- the
		 * harvester a refinery arrives with. Its cost is refunded if it cannot be placed.
		 */
		UnitTypeClass const * FreeUnit;

		/*
		**	This is the direction (from the center cell) of the building in order to find a
		**	legitimate foundation square. This location will be used for targeting and capture
		**	move destination purposes.
		*/
		FacingType FoundationFace;

		/*
		**	Adjacent distance for building next to.
		*/
		int Adjacent;

		/*
		**	This flag specifies the type of object this factory building can "produce". For non
		**	factory buildings, this value will be RTTI_NONE.
		*/
		RTTIType ToBuild;

		/*
		**	For building that produce ground units (infantry and vehicles), there is a default
		**	exit point defined. This point is where the object is first placed on the map.
		**	Typically, this is located next to a door. The unit will then travel on to a clear
		**	terrain area and enter normal game processing.
		*/
		Coord ExitCoordinate;

		/*
		**	When determine which cell to head toward when exiting a building, use the
		**	list elaborated by this variable. There are directions of exit that are
		**	more suitable than others. This list is here to inform the system which
		**	directions those are.
		*/
		Cell const * ExitList;

		/*
		**	This is the starting facing to give this building when it first
		**	gets constructed. The facing matches the final stage of the
		**	construction animation.
		*/
		Dir256 StartFace;

		/*
		**	Each building type produces and consumes power. These values tell how
		**	much.
		*/
		int Power;
		int Drain;

		/*
		**	This is the size of the building. This size value is a rough indication
		**	of the building's "footprint".
		*/
		BSizeType Size;

		/*
		 * This is the height of the building, expressed in cells of elevation. It sets the
		 * building's lepton dimensions and how far above it a missile is launched.
		 */
		int ZHeight;

		/// Unused
		int MidPoint;

		/*
		 * This is the number of frames in this building's door animation, across which the
		 * door timer is interpolated. The damaged frames follow immediately after them.
		 */
		int DoorStages;

		/**********************************************************************
		**	For each stage that a building may be in, its animation is controlled
		**	by this structure. It dictates the starting and length of the animation
		**	frames needed for the specified state. In addition it specifies how long
		**	to delay between changes in animation. With this data it is possible to
		**	control the appearance of all normal buildings. Turrets and SAM sites are
		**	an exception since their animation is not merely cosmetic.
		*/
		struct AnimControlType {
			int	Start;			// Starting frame of animation.
			int	Count;			// Number of frames in this animation.
			int	Rate;			// Number of ticks to delay between each frame.

			// Carries the animation control to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(Start);
				stream.Serialize(Count);
				stream.Serialize(Rate);
			}
		};
		AnimControlType Anims[BSTATE_COUNT] = {};

		struct AnimDataType {
			/*
			 * This is the name of the animation type to run in this slot, or an empty string
			 * if this building has no animation for it.
			 */
			char Anim[16];

			/*
			 * This is the name of the animation type to run in this slot once the building is
			 * damaged. If the artwork provides none, then the healthy animation is used.
			 */
			char AnimDamaged[16];

			/*
			 * This is the pixel offset from the building's center at which this animation is
			 * placed, so that a plume of smoke sits on the right chimney.
			 */
			Point2D Location;

			/*
			 * This is the depth bias given to this animation, which decides whether it draws
			 * in front of or behind the building it belongs to.
			 */
			char ZAdjust;

			/*
			 * This is the bias added to this animation's sorting position, which moves it
			 * earlier or later among the objects it shares its layer with.
			 */
			char YSort;

			/*
			 * If this animation only runs while the building has power, then this flag will be
			 * true. Losing power pauses it rather than stopping it.
			 */
			bool Powered;

			/*
			 * If this animation is created and destroyed along with the building's power, then
			 * this flag will be true. It is not merely paused the way a Powered one is.
			 */
			bool PoweredLight;

			// Carries the animation entry to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(Anim);
				stream.Serialize(AnimDamaged);
				stream.Serialize(Location);
				stream.Serialize(ZAdjust);
				stream.Serialize(YSort);
				stream.Serialize(Powered);
				stream.Serialize(PoweredLight);
			}
		};

		/*
		 * These are the attached animations this building carries, one per BAnimType slot. The
		 * slots at the head of the array are filled in by whatever upgrades get installed.
		 */
		AnimDataType AnimData[BANIM_COUNT];

		/*
		 * This is the number of upgrades that may be installed on this building. It also
		 * bounds the UpgradeLevel a building of this type can reach.
		 */
		int Upgrades;

		/*
		 * This is the shape data drawn in place of the building's normal artwork while it is
		 * deploying or packing itself back up.
		 */
		ShapeSet const * DeployingAnim;

		/*
		 * This is the shape data drawn over a factory's doorway while it unloads, so that
		 * whatever is passing through appears to be moving through the door.
		 */
		ShapeSet const * UnderDoorAnim;

		/*
		 * This is the shape data for a factory's door, animated open and shut as the building
		 * unloads. Its frame is interpolated across the DoorStages by the door timer.
		 */
		ShapeSet const * DoorAnim;

		/// Unused
		ShapeSet const * SpecialZOverlay;

		/// Unused
		int SpecialZOverlayZAdjust;

		/*
		 * This is the shape data for the overlay laid flat on the ground beneath a finished
		 * building, such as the concrete apron of a weapons factory.
		 */
		ShapeSet const * BibShape;

		/*
		 * This is the depth bias applied when the building is drawn against its Z shape, so
		 * that its artwork masks the objects around it at the right depth.
		 */
		int NormalZAdjust;

		/*
		 * These are this building's ratings as a defense against aircraft, armor and infantry,
		 * derived from its weapons. The computer plans and weighs its base defenses by them.
		 */
		int AntiAirValue;
		int AntiArmorValue;
		int AntiInfantryValue;

		/*
		 * This is the pixel offset applied to the Z shape as the building is drawn, for the
		 * structures whose artwork does not sit squarely on their footprint.
		 */
		Point2D ZShapePointMove;

		/*
		 * This is the screen area the building's artwork covers, relative to its center. It is
		 * computed once from every frame of every shape the type owns and then cached here.
		 */
		Rect DrawRect;

		/*
		 * This is the brightness added to the cell's own lighting as this building is drawn,
		 * so that a structure may appear lit from within.
		 */
		short ExtraLight;

		/*
		 * If the owner may switch this building off to save power, then this flag will be
		 * true. Only a building that actually drains power is worth switching off.
		 */
		bool IsCanTogglePower;

		/*
		 * If this building carries a sweeping spotlight, then this flag will be true. A
		 * BuildingLightClass is attached when it is placed in order to drive the beam.
		 */
		bool HasSpotlight;

		/*
		 * If this building is the Temple of Nod, then this flag will be true. The computer
		 * rates one as a class of ion cannon target in its own right.
		 */
		bool IsTemple;

		/*
		 * If this building is a plug, then this flag will be true. The computer rates one
		 * above an ordinary structure when it is choosing an ion cannon target.
		 */
		bool IsPlug;

		/*
		 * If this building is a helipad that arrives with its own aircraft, then this flag
		 * will be true. Completing one delivers the first of the Rule->PadAircraft free.
		 */
		bool IsHoverPad;

		/*
		**	Is this building allowed to be considered for building adjacency
		**	checking? If false, then building off of (or adjacent to) this building
		**	is not considered.
		*/
		bool IsBase;

		/*
		**	This flag controls whether the building is equiped with a dirt
		**	bib or not. A building with a bib has a dirt patch automatically
		**	attached to the structure when it is placed.
		*/
		bool IsBibbed;

		/*
		**	If this building is a special wall type, such that it exists as a building
		**	for purposes of construction but transforms into an overlay wall object when
		**	it is placed on the map, then this flag will be true.
		*/
		bool IsWall;

		/*
		**	Certain building types can be captures by enemy infantry. For those
		**	building types, this flag will be true. Typically, military or hardened
		**	structures such as turrets cannot be captured.
		*/
		bool IsCaptureable;

		/*
		**	Does this building require power to function? Usually, this isn't the case. The building
		**	normally either has no effect by power level or is gradually reduced in effectiveness. This
		**	flag is for those buildings that completely cease to function when the power drops below
		**	full.
		*/
		bool IsPowered;

		/*
		**	If this flag is true, then the building cannot be sold even if it could have been built. This
		**	is especially useful for mines which can be built but cannot be sold.
		*/
		bool IsUnsellable;

		/*
		 * If this building provides its owner with a radar map, then this flag will be true.
		 * Spying on one hands the spy that radar, so the RadarSpied tally tracks it.
		 */
		bool IsRadar;

		/*
		 * If this building's weapon charges up visibly before firing, then this flag will be
		 * true. Such a building animates the charge whether or not it has a turret to aim.
		 */
		bool IsHasChargeAnim;

		/*
		 * If this building shows how full it is, then this flag will be true. Its special
		 * animation is held at one of four stages according to how much is in store.
		 */
		bool IsSiloDamage;

		/*
		 * If vehicles may drive onto this building to be repaired, then this flag will be
		 * true. A unit that docks is put to sleep while the repair mission runs.
		 */
		bool IsCanUnitRepair;

		/*
		 * If aircraft may dock with this building to rearm, then this flag will be true. A
		 * helicopter out of ammunition must return to one before it can attack again.
		 */
		bool IsCanUnitReload;

		/// Unused
		bool IsFlat;

		/*
		 * If harvesters may dock with this building to unload tiberium, then this flag will
		 * be true. The harvester is sent into its unload mission once it has attached.
		 */
		bool IsDockUnload;

		/// Unused
		bool IsRecoilless;

		/*
		 * If this building is to sit idle rather than look for something to do, then this
		 * flag will be true. Its guard mission does nothing and simply waits out a delay.
		 */
		bool IsHasStupidGuardMode;

		/*
		 * If this building is a bridge repair hut, then this flag will be true. An engineer
		 * who enters one rebuilds the bridge it belongs to.
		 */
		bool IsBridgeRepairHut;

		/*
		 * If this building is a gate, then this flag will be true. A gate opens as something
		 * friendly approaches and closes again after the GateCloseDelay.
		 */
		bool IsGate;

		/*
		 * If this building is a surface to air missile launcher, then this flag will be true.
		 * It runs an attack state machine of its own and only holds airborne targets.
		 */
		bool IsSAM;

		/*
		 * If this building is a construction yard, then this flag will be true. In a
		 * multiplayer game it may be told to pack itself back into a vehicle.
		 */
		bool IsConstructionYard;

		/*
		 * If this building launches its super weapon from a silo, then this flag will be
		 * true. Its missile mission opens the doors and raises the missile before firing.
		 */
		bool IsNukeSilo;

		/*
		 * If this building refines tiberium into credits, then this flag will be true. A
		 * harvester docks with one to unload and returns to it whenever it fills up.
		 */
		bool IsRefinery;

		/*
		 * If this building processes veins rather than tiberium, then this flag will be true.
		 * It is the weed harvester's counterpart to a refinery.
		 */
		bool IsWeeder;

		/*
		 * If this building produces vehicles, then this flag will be true. A unit completed
		 * there is driven out through the doors before it is handed to its owner.
		 */
		bool IsWeaponsFactory;

		/*
		 * If this building is a laser fence post, then this flag will be true. A post links
		 * itself to its neighbors and raises the fence spans between them while powered.
		 */
		bool IsLaserFencePost;

		/*
		 * If this building is a span of laser fence, then this flag will be true. A span is
		 * raised by the posts either side of it and blocks the ground between them.
		 */
		bool IsLaserFence;

		/*
		 * If this building is a segment of firestorm wall, then this flag will be true. A
		 * segment links to its neighbors and is only solid while the defense is switched on.
		 */
		bool IsFirestormWall;

		/*
		 * If infantry may enter this building to be healed, then this flag will be true.
		 * Whoever walks in is put to sleep while the repair mission tends to them.
		 */
		bool IsHospital;

		/*
		 * If infantry may enter this building to be re-equipped, then this flag will be true.
		 * It treats its visitors exactly as a hospital does.
		 */
		bool IsArmory;

		/*
		 * If this building is an EM pulse cannon, then this flag will be true. It aims at a
		 * point on the ground but never fires a weapon of its own.
		 */
		bool IsEMPulseCannon;

		/*
		 * If this building is the deployed form of a Tick Tank, then this flag will be true.
		 * It faces east when it deploys and may pack itself up again.
		 */
		bool IsTickTank;

		/*
		 * If this building's turret is a voxel model rather than a shape animation, then this
		 * flag will be true. It may then aim freely instead of snapping to 32 facings.
		 */
		bool IsTurretAnimAVoxel;

		/*
		 * If this building cloaks its owner's objects within CloakRadiusInCells, then this
		 * flag will be true. The field collapses when it loses power or is destroyed.
		 */
		bool IsCloakGenerator;

		/*
		 * If this building reveals cloaked and burrowed objects out to CloakRadiusInCells,
		 * then this flag will be true. It is also a mobile deployer, and faces east.
		 */
		bool IsSensorArray;

		/*
		 * If this building is the deployed form of a mobile missile launcher, then this flag
		 * will be true. It faces east when it deploys and may pack itself up again.
		 */
		bool IsICBMLauncher;

		/*
		 * If this building is the deployed form of an artillery piece, then this flag will be
		 * true. It faces north when deployed and idles its barrel back to the StartPitch.
		 */
		bool IsArtillary;

		/*
		 * If aircraft may land on this building to rearm and repair, then this flag will be
		 * true. An aircraft out of ammunition goes looking for one of its owner's.
		 */
		bool IsHelipad;

		/*
		 * If this building is the GDI barracks, then this flag will be true. Infantry leaving
		 * it prefer the cell one in and two down, shifted by the ExitCoordinate.
		 */
		bool IsGDIBarracks;

		/*
		 * If this building is the Nod barracks, then this flag will be true. Infantry leaving
		 * it prefer the cell two in and two down, shifted by the ExitCoordinate.
		 */
		bool IsNODBarracks;

		/*
		 * This is the super weapon this building grants its owner while it stands, or
		 * SUPER_NONE if it grants none.
		 */
		SuperWeaponType SuperWeapon;

		/*
		 * This is the second super weapon this building grants its owner, for the structures
		 * that offer a choice of two. Otherwise it is SUPER_NONE.
		 */
		SuperWeaponType SuperWeapon2;

		/*
		 * This is the number of frames in this gate's opening animation, across which the
		 * door timer is interpolated. The damaged frames follow immediately after them.
		 */
		int GateStages;

		/*
		 * This is the number of upgrade levels this building adds to the structure named by
		 * PowersUpBuilding (1 - 3). If -1, then it adds one and needs a free slot to do so.
		 */
		int PowersUpToLevel;

		/*
		 * This is the name of the barrel's voxel file, needed only when it cannot be
		 * derived from the turret's. A turret named "...TUR..." implies "...BARL...".
		 */
		char VoxelBarrelFile[15 + 1];

		/*
		 * This is the uniform scale the barrel voxel is drawn at, applied as the last step
		 * of the barrel matrix. It defaults to 1.0, the size the model was built at.
		 */
		double VoxelBarrelScale;

		/*
		 * This is the offset from the barrel's rotation pivot out to the point it pitches
		 * about, applied once the barrel's pitch has been rotated into its matrix.
		 */
		Point3D VoxelBarrelOffsetToPitchPivotPoint;

		/*
		 * This is the offset from the barrel's base out to the point it rotates about,
		 * applied once the turret's facing has been rotated into the barrel matrix.
		 */
		Point3D VoxelBarrelOffsetToRotatePivotPoint;

		/*
		 * This is the offset from the building's own origin out to the base of the barrel
		 * assembly. It is the first translation the barrel matrix applies.
		 */
		Point3D VoxelBarrelOffsetToBuildingPivotPoint;

		/*
		 * This is the offset out to the barrel's muzzle, and thus where a shot appears to
		 * leave from. The Y component is mirrored for the second shot of a burst.
		 */
		Point3D VoxelBarrelOffsetToBarrelEnd;

		/*
		 * This is the animation rate, expressed in ticks between frames, for this
		 * building's turret while its weapon charges up. It lets a charging weapon wind up
		 * visibly faster or slower than the turret animates at rest.
		 */
		int TurretChargeAnimRate;

		/*
		 * This is the barrel pitch to give this building when it is first constructed,
		 * expressed on the compass dial where DIR_E is level. A deployed artillery piece
		 * returns its barrel to this pitch whenever it has nothing to shoot at.
		 */
		Dir256 StartPitch;

		/*
		 * If this building is the deployed form of a limpet mine, then this flag will be
		 * true. It may always pack itself up again, refuses a manual attack order, and an
		 * EM pulse destroys it outright rather than stunning it.
		 */
		bool IsLimpetMine;

		/*
		 * If this building is the deployed form of a mobile war factory, then this flag
		 * will be true. Such a structure may always pack itself up again, becomes the
		 * house's primary factory as it opens, and a vehicle thief may target it.
		 */
		bool IsMobileWar;

		/*
		 * If this building is the deployed form of a mobile stealth generator, then this
		 * flag will be true. Its only effect is to mark the type as a mobile deployer so
		 * that the undeploy machinery recognizes it.
		 */
		bool IsMobileStealth;

		/*
		 * If this building is the deployed form of a Juggernaut, then this flag will be
		 * true. Under computer control it packs itself up once its target moves out of
		 * range, and it returns its barrel to the StartPitch while idle.
		 */
		bool IsJuggernaut;

		/*
		 * If this building is the deployed form of a core defender, then this flag will be
		 * true. An EM pulse leaves such a structure running where it would shut down any
		 * other building.
		 */
		bool IsCoreDefender;

		/*
		 * If this building's gun barrel is a voxel model rather than a shape animation,
		 * then this flag will be true. The barrel is loaded as a voxel of its own, separate
		 * from the turret, and drawn pitched and rotated over it.
		 */
		bool IsBarrelAnimAVoxel;

		/*
		 * If this building's turret animation may not run alongside its active animation,
		 * then this flag will be true. Such a turret only animates while the weapon is
		 * charging or charged, and the active animation is stopped for the duration.
		 */
		bool IsTurretAnimExclusive;

		/// Unused
		bool UnusedBTypeBool1;

		/*
		 * If this building's door animation carries a second set of frames for when the
		 * structure is damaged, then this flag will be true. The damaged frames follow the
		 * healthy ones in the same shape file.
		 */
		bool IsDamagedDoor;

		/*
		 * If this building exists purely as a game mechanic and is never meant to be seen,
		 * then this flag will be true. Such a structure is skipped by the renderer, the
		 * radar and the mouse, and explosions pass it by entirely.
		 */
		bool IsInvisibleInGame;

		/*
		 * If this building is drawn through the color converter of the cell it stands on
		 * rather than the usual object palette, then this flag will be true. It lets a
		 * structure that is really scenery pick up the lighting of the terrain.
		 */
		bool IsTerrainPalette;

		/*
		 * If this building ignores the usual placement restrictions, then this flag will be
		 * true. Such a type may be dropped on any cell regardless of terrain, adjacency or
		 * what already occupies the ground.
		 */
		bool IsCanPlaceAnywhere;

		/*
		 * If this building's artwork provides an extra, more heavily damaged stage beyond the
		 * normal damaged frames, then this flag will be true. Most do, so it defaults true.
		 */
		bool IsExtraDamageStage;

		/*
		 * If the computer is allowed to work this building into the base it plans, then
		 * this flag will be true. A structure the player may build is not automatically one
		 * the computer knows what to do with.
		 */
		bool CanAIBuildThis;

		/*
		 * If this building counts as a base defense, then this flag will be true. The
		 * computer plans its defenses in a pass of their own and rates each one against
		 * air, armor and infantry so it can weigh them against the enemy it expects.
		 */
		bool IsBaseDefense;

		/*
		 * This is the radius, expressed in cells, of the field a cloak generator projects
		 * or a sensor array watches. The largest value among all building types decides how
		 * big the shared cloaking surface must be.
		 */
		char CloakRadiusInCells;

		/*
		 * If this building's shape data is to be loaded the first time it is asked for
		 * rather than up front with the rest of the theater art, then this flag will be
		 * true. It exists to keep seldom seen structures out of memory.
		 */
		bool IsDemandLoad;

		/*
		 * If the construction animation is to be loaded only when a building of this type
		 * is actually placed, then this flag will be true. It trades a load hitch at
		 * placement time for a smaller memory footprint.
		 */
		bool IsDemandLoadBuildup;

		/*
		 * If the construction animation may be thrown away once the building is up, then
		 * this flag will be true. It keeps rarely rebuilt structures from holding their
		 * buildup shapes for the whole mission.
		 */
		bool IsFreeBuildup;

		/*
		 * If owning this building teaches the house to weigh its targets intelligently,
		 * then this flag will be true. The structure or upgrade that carries it activates
		 * the house's threat node for good -- there is no way back to the "dumb" values.
		 */
		bool IsThreatRatingNode;

		/*
		 * This is the theater qualified name of the building's shape file, recorded as the
		 * art is resolved. A type that defers loading its image uses this to find the file
		 * the first time the shape is really needed.
		 */
		char TheaterImageFile[19 + 1];

		/*---------------------------------------------------------------------------
		**	This is the building type explicit constructor.
		*/
		BuildingTypeClass(char const * ininame = NULL);
		virtual ~BuildingTypeClass() override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		static StructType From_Name(char const * name);
		static StructType From_Given_Name(char const * name);
		static void Init(TheaterType theater);
		static void Fetch_Z_Data(void);
		static void Post_Load_Game(void);
		static void One_Time(void);
		static BuildingTypeClass * Find_Or_Make(char const * name);

		int Width(void) const;
		int Height(bool bib=false) const;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override;

		virtual bool Read_INI(CCINIClass const & ini) override;
		int Flush_For_Placement(Cell const & cell, HouseClass * house) const;
		virtual int Cost_Of(HouseClass * house = NULL) const override;
		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual int Max_Pips(void) const override;
		virtual Point3D Pixel_Dimensions(void) const override;
		virtual Point3D Lepton_Dimensions(void) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house = NULL) const override;
		virtual ObjectClass * Create_One_Of(HouseClass * house) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;
		virtual void const * Get_Image_Data(void) const override;
		void Fetch_Building_Normal_Image(TheaterType theater);
		void Fetch_Building_Voxel_Image(void);
		virtual void const * Get_Buildup_Data(void) const;
		void Free_Buildup_Data(void);
		virtual bool Legal_Placement(Cell const & pos, HouseClass * house = NULL) const override;

		bool Is_Factory(void) const {return(ToBuild != RTTI_NONE);}
		virtual int Raw_Cost(void) const override;

		Cell const * Exit_List(void) const;
		Rect Get_Draw_Rect(void);

		static int Get_Max_Drain(DynamicVectorClass<BuildingTypeClass *> const & list);
		static void Post_Read_Tile_Fixup(void);
		bool Can_Always_Undeploy(void) const;
		bool Is_Mobile_Deployer(void) const;
		Dir256 Deploy_Facing(void) const;
		void Calculate_Base_Defense_Values(void);

	public:

		/*
		 * This is the shared Z buffer shape that gives every building its depth profile as
		 * it is drawn, so that objects behind it are correctly hidden. Buildings six or
		 * more cells wide are too large for it and must go without.
		 */
		static void const * BuildingZShape;

		/*
		 * This is the animating symbol drawn over a building that is not running -- either
		 * switched off by its owner or starved of power. It exists as feedback to the
		 * owner, so it appears only on buildings under local control.
		 */
		static void const * PowerOffShapes;

		/*
		 * This is the animating wrench symbol drawn over a building that is undergoing
		 * repair. It gives the player a visible sign that credits are being spent even
		 * while the damage itself is not obvious.
		 */
		static void const * WrenchShapes;

		/*
		 * These are the footprint dimensions, expressed in cells, of every building size
		 * indexed by BSizeType. Any query of a building's width or height resolves through
		 * these two tables rather than decoding the size enumeration itself.
		 */
		static int SizeWidth[BSIZE_COUNT];
		static int SizeHeight[BSIZE_COUNT];

	private:

		/*
		 * These are the footprint cell lists for every building size, indexed by
		 * BSizeType. A building type adopts the list that matches its Size, so the
		 * ordinary rectangular structures never spell their occupied cells out.
		 */
		static Cell const OccupyLists[BSIZE_COUNT][24];

		/*
		 * These are the exit cell lists for every building size, indexed by BSizeType.
		 * Each one rings its footprint in the order the exit cells should be tried, so a
		 * newly produced unit leaves by the nearest clear square.
		 */
		static Cell const ExitLists[BSIZE_COUNT][30];

		void Init_Anim(BStateType state, int start, int count, int rate) const;
};

inline BuildingTypeClass * AbstractClass::As_BuildingTypeClass(void) { return(dynamic_cast<BuildingTypeClass *>(this)); }
inline BuildingTypeClass const * AbstractClass::As_BuildingTypeClass(void) const { return(dynamic_cast<BuildingTypeClass const *>(this)); }

extern BSurface * CloakingSurface;
