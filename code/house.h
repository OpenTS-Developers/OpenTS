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

/* $Header: /CounterStrike/HOUSE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : HOUSE.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 21, 1994                                                 *
 *                                                                                             *
 *                  Last Update : May 21, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "_house.h"
#include "base.h"
#include "coord.h"
#include "counter.h"
#include "credits.h"
#include "dropship.h"
#include "ftimer.h"
#include "object.h"
#include "region.h"
#include "rgb.h"
#include "storage.h"
#include "teamtype.h"
#include "timer.h"
#include "typelist.h"
#include "utracker.h"

#include "aircraft.hh"
#include "diff.hh"
#include "house.hh"
#include "infantry.hh"
#include "prodfail.hh"
#include "quarry.hh"
#include "rtti.hh"
#include "source.hh"
#include "state.hh"
#include "struct.hh"
#include "super.hh"
#include "tiberium.hh"
#include "unit.hh"
#include "urgency.hh"
#include "zone.hh"

template<class T> class DynamicVectorClass;
class TriggerClass;
class FootClass;
class FactoryClass;
class HouseTypeClass;
class ObjectClass;
class TechnoClass;
class TagClass;
class BuildingClass;
class WaypointPathClass;
class WaypointClass;
class SuperClass;
class AbstractClass;
class UnitClass;
class CCINIClass;
class ObjectTypeClass;
class SaveStreamClass;
template<class T> class DynamicVectorClass;


/****************************************************************************
**	Certain aspects of the house "country" are initially set by the scenario
**	control file. This information is static for the duration of the current
**	scenario, but is dynamic between scenarios. As such, it can't be placed in
**	the static HouseTypeClass structure, but is embedded into the house
**	class instead.
*/
class HouseStaticClass {
	public:
		HouseStaticClass(void);

		/*
		**	This value indicates the degree of smartness to assign to this house.
		**	A value is zero is presumed for human controlled houses.
		*/
		int IQ;

		/*
		**	This is the buildable tech level for this house. This value is used
		**	for when the computer is deciding what objects to build.
		*/
		int TechLevel;

		/*
		**	This is the original ally specification to use at scenario
		**	start. Various forces during play may adjust the ally state
		**	of this house.
		*/
		int Allies;

		/*
		**	This records the initial credits assigned to this house when the scenario
		**	was loaded.
		*/
		int InitialCredits;

		/*
		**	For generic (unspecified) reinforcements, they arrive by a common method. This
		**	specifies which method is to be used.
		*/
		SourceType Edge;

		/// Carries the scenario supplied house control to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(IQ);
			stream.Serialize(TechLevel);
			stream.Serialize(Allies);
			stream.Serialize(InitialCredits);
			stream.Serialize(Edge);
		}
};


/****************************************************************************
**	Player control structure. Each player (computer or human) has one of
**	these structures associated. These are located in a global array.
*/
class HouseClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		/*
		 * This is this house's own index into the Houses heap, assigned as the house is
		 * created. It doubles as the house's identifier in ally bit fields and cell ownership.
		 */
		HousesType HeapID;

		/*
		**	Pointer to the HouseTypeClass that this house is "owned" by.
		**	All constant data for a house type is stored in that class.
		*/
		HouseTypeClass * Class;

		/*
		 * These are the triggers attached to this house rather than to an object or a cell. The
		 * house springs them all once per AI pass and lets each decide if it has really fired.
		 */
		DynamicVectorClass<TagClass *> HouseTags;

		/*
		 * These are the construction yards this house owns. Which sides' objects the house is
		 * able to build is decided from the yards on this list.
		 */
		DynamicVectorClass<BuildingClass *> ConYards;

		/*
		**	This is the handicap (difficulty level) assigned to this house.
		*/
		DiffType Difficulty;

		/*
		**	Override handicap control values.
		*/
		double FirepowerBias;
		double GroundspeedBias;
		double AirspeedBias;
		double ArmorBias;
		double ROFBias;
		double CostBias;
		double BuildSpeedBias;
		double RepairDelay;
		double BuildDelay;

		/*
		**	The initial house data as loaded from the scenario control file is
		**	stored here. Although this data changes for each scenario, it remains
		**	static for the duration of the current scenario.
		*/
		HouseStaticClass Control;

		/*
		 * This records what the computer is currently willing to spend money on. A house that
		 * is hard up alternates between structures and units rather than paying for both.
		 */
		enum {
			EVERYTHING,
			BUILDINGS,
			UNITS,
			NOTHING
		} ProductionMode;

		/*
		**	This is the house type that this house object should act like. This
		**	value controls production choices and radar cover plate imagery.
		*/
		HousesType ActLike;

		/*
		**	If this house is controlled by the player, then this flag will be true. The
		**	computer controls all other active houses.
		*/
		bool IsHuman;

		/*
		**	If the player can control units of this house even if the player doesn't
		**	own units of this house, then this flag will be true.
		*/
		bool IsPlayerControl;

		/*
		**	This flag enables production. If the flag is false, production is disabled.
		**	By timing when this flag gets set, the player can be given some breathing room.
		*/
		bool IsStarted;

		/*
		**	When alerted, the house will create teams of the special "auto" type and
		**	will generate appropriate units to fill those team types.
		*/
		bool IsAlerted;

		/*
		 * If this house may raise teams from its AI triggers, then this flag will be true. A
		 * scenario can hold an opponent's team building back until it is ready for it.
		 */
		bool IsAITriggersOn;

		/*
		**	If automatic base building is on, then this flag will be set to true.
		*/
		bool IsBaseBuilding;

		/*
		**	If the house has been discovered, then this flag will be set
		**	to true. However, the trigger even associated with discovery
		**	will only be executed during the next house AI process.
		*/
		bool IsDiscovered;

		/*
		**	If this house is played by a human in a multiplayer game, this flag
		**	keeps track of whether this house has been defeated or not.
		*/
		bool IsDefeated;

		/*
		**	These flags are used in conjunction with the BorrowedTime timer. When
		**	that timer expires and one of these flags are set, then that event is
		**	applied to the house. This allows a dramatic pause between the event
		**	trigger and the result.
		*/
		bool IsToDie;
		bool IsToWin;
		bool IsToLose;

		/*
		**	This flag is set when a transport carrying a civilian has been
		**	successfully evacuated. It is presumed that a possible trigger
		**	event will be sprung by this event.
		*/
		bool IsCivEvacuated;

		/*
		 * If this house's firestorm defense is raised, then this flag will be true. Every
		 * firestorm wall the house owns is live while it is set, and bars movement through it.
		 */
		bool FirestormDefenseActivated;

		/*
		 * If this house owns a threat rating structure, then this flag will be true. Its units
		 * then weigh targets by their own coefficients instead of by the blunt default rules.
		 */
		bool IsThreatRatingNodeActive;

		/*
		**	If potentially something changed that might affect the sidebar list of
		**	buildable objects, then this flag indicates that at the first LEGAL opportunity,
		**	the sidebar will be recalculated.
		*/
		bool IsRecalcNeeded;

		/*
		 * This is the network address of the player controlling this house, recorded for the
		 * end of game statistics.
		 */
		int IPAddress;

		/*
		 * This is the identifier of the online clan the player belongs to, recorded for the
		 * end of game statistics.
		 */
		int SquadID;

		/*
		 * If contact with this player has been lost, then this flag will be true. Such a house
		 * is reported as having dropped out rather than as having resigned.
		 */
		bool LostConnection;

		/*
		 * This is the waypoint path that the player is currently plotting into. If PATH_NONE,
		 * then no path is being edited.
		 */
		PathType SelectedPath;

		/*
		 * These are the waypoint paths the player has plotted for this house. A path is only
		 * created once something is placed into it.
		 */
		WaypointPathClass * Paths[PATH_COUNT];

		/*
		**	If the map has been completely revealed to the player, then this flag
		**	will be set to true. By examining this flag, a second "reveal all map"
		**	crate won't be given to the player.
		*/
		bool IsVisionary;

		/*
		**	This flag is set to true when the house has determined that
		**	there is insufficient Tiberium to keep the harvesters busy.
		**	In such a case, the further refinery/harvester production
		**	should cease. This is one of the first signs that the endgame
		**	has begun.
		*/
		bool IsTiberiumShort;

		/*
		**	These flags are used for the general house trigger events of being
		**	spied and thieved. The appropriate flag will be set when the event
		**	occurs.
		*/
		bool IsSpied;
		bool IsThieved;

		/*
		**	This flag is used to control non-human repairing of buildings.  Each
		**	house gets to repair one building per loop, and this flag controls
		**	whether this house has 'spent' its repair option this time through.
		*/
		bool DidRepair;

		/*
		**	If the JustBuilt??? variable has changed, then this flag will
		**	be set to true.
		*/
		bool IsBuiltSomething;

		/*
		**	Did this house lose via resignation?
		*/
		bool IsResigner;

		/*
		**	Did this house lose because the player quit?
		*/
		bool IsGiverUpper;

		/*
		 * If this house has been told to send everything hunting, then this flag will be true.
		 * Targets outside the designated Enemy are valued lowest, so the hunt converges on it.
		 */
		bool IsAllToHunt;

		/*
		**	If this computer controlled house has reason to be mad at humans,
		**	then this flag will be true. Such a condition prevents alliances with
		**	a human and encourages the computers players to ally amongst themselves.
		*/
		bool IsParanoid;

		/*
		**	A gap generator shrouded cells and all units of this house must perform
		**	a look just in case their look radius intersects the shroud area.
		*/
		bool IsToLook;

		/*
		**	This value indicates the degree of smartness to assign to this house.
		**	A value of zero indicates that the player controls everything.
		*/
		int IQ;

		/*
		**	This records the current state of the base. This state is used to control
		**	what action the base will perform and directly affects production and
		**	unit disposition. The state will change according to time and combat
		**	events.
		*/
		StateType State;

		/*
		**	These super weapon control objects are used to control the recharge
		**	and availability of these special weapons to this house.
		*/
		DynamicVectorClass<SuperClass *> SuperWeapon;

		/*
		**	This is a record of the last building that was built. For buildings that
		**	were built as a part of scenario creation, it will be the last one
		**	discovered.
		*/
		StructType JustBuiltStructure;
		InfantryType JustBuiltInfantry;
		UnitType JustBuiltUnit;
		AircraftType JustBuiltAircraft;

		/*
		**	This records the number of triggers associated with this house that are
		**	blocking a win condition. A win will only occur if all the blocking
		**	triggers have been deleted.
		*/
		int Blockage;

		/*
		**	For computer controlled houses, there is an artificial delay between
		**	performing repair actions. This timer regulates that delay. If the
		**	timer has not expired, then no repair initiation is allowed.
		*/
		CDTimerClass<FrameTimerClass> RepairTimer;

		/*
		**	This timer controls the computer auto-attack logic. When this timer expires
		**	and the house has been alerted, then it will create a set of attack
		**	teams.
		*/
		CDTimerClass<FrameTimerClass> AlertTime;

		/*
		**	This timer is used to handle the delay between some catastrophic
		**	event trigger and when it is actually carried out.
		*/
		CDTimerClass<FrameTimerClass> BorrowedTime;

		/*
		**	Record of gains and losses for this house during the course of the
		**	scenario.
		*/
		unsigned CreditsSpent;
		unsigned HarvestedCredits;
		int StolenBuildingsCredits;

		/*
		**	This is the running count of the number of units owned by this house. This
		**	value is used to keep track of ownership limits.
		*/
		int CurUnits;
		int CurBuildings;
		int CurInfantry;
		int CurAircraft;

		/*
		**	This is the running total of the number of credits this house has accumulated.
		*/
		StorageClass Tiberium;
		int Credits;
		int Capacity;
		StorageClass Weed;

		/// Unused
		int field_1BC;

		/*
		**	Stuff to keep track of the total number of units built by this house.
		*/
		UnitTrackerClass * AircraftTotals;
		UnitTrackerClass * InfantryTotals;
		UnitTrackerClass * UnitTotals;
		UnitTrackerClass * BuildingTotals;

		/*
		**	Total number of units destroyed by this house
		*/
		UnitTrackerClass * DestroyedAircraft;
		UnitTrackerClass * DestroyedInfantry;
		UnitTrackerClass * DestroyedUnits;
		UnitTrackerClass * DestroyedBuildings;

		/*
		**	Total number of enemy buildings captured by this house
		*/
		UnitTrackerClass * CapturedBuildings;

		/*
		**	Total number of crates found by this house
		*/
		UnitTrackerClass * TotalCrates;

		/*
		**	Records the number of infantry and vehicle factories active. This value is
		**	used to regulate the speed of production.
		*/
		int AircraftFactories;
		int InfantryFactories;
		int UnitFactories;
		int BuildingFactories;

		/*
		**	This is the accumulation of the total power and drain factors. From these
		**	values a ratio can be derived. This ratio is used to control the rate
		**	of building decay.
		*/
		int Power;					// Current power output.
		int Drain;					// Power consumption.

		/*
		**	For human controlled houses, only one type of unit can be produced
		**	at any one instant. These factory objects control this production.
		*/
		FactoryClass * AircraftFactory;
		FactoryClass * InfantryFactory;
		FactoryClass * UnitFactory;
		FactoryClass * BuildingFactory;

		/*
		**	This target value specifies where the flag is located. It might be a cell
		**	or it might be an object.
		*/
		AbstractClass * FlagLocation;

		/*
		**	This is the flag-home-cell for this house.  This is where we must bring
		**	another house's flag back to, to defeat that house.
		*/
		Cell FlagHome;

		/*
		**	For multiplayer games, each house needs to keep track of how many
		**	objects of each other house they've killed.
		*/
		int UnitsKilled[20];
		int UnitsLost;
		int BuildingsKilled[20];
		int BuildingsLost;

		/*
		**	This keeps track of the last house to destroy one of my units.
		**	It's used for scoring multiplayer games.
		*/
		HousesType WhoLastHurtMe;

		/*
		**	This records information about the location and size of
		**	the base.
		*/
		Coord Center;			// Center of the base.
		int SpawnWaypoint;		// starting waypoint this house was placed at; -1 = never placed
		int Radius;				// Average building distance from center (leptons).
		struct {
			int AirDefense;
			int ArmorDefense;
			int InfantryDefense;

			/// Carries one zone's defense ratings to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(AirDefense);
				stream.Serialize(ArmorDefense);
				stream.Serialize(InfantryDefense);
			}
		} ZoneInfo[ZONE_COUNT];

		/*
		**	This records information about the last time a building of this
		**	side was attacked. This information is used to determine proper
		**	response.
		*/
		int LATime;					// Time of attack.
		HousesType LAEnemy;			// Owner of attacker.

		/*
		**	This target value is the building that must be captured as soon as possible.
		**	Typically, this will be one of the buildings of this house that has been
		**	captured and needs to be recaptured.
		*/
		AbstractClass * ToCapture;

		/*
		**	This value shows who is spying on this house's radar facilities.
		**	This is used for the other side to be able to update their radar
		**	map based on the cells that this house's units reveal.
		*/
		int RadarSpied;

		/*
		**	Running score, based on units destroyed and units lost.
		*/
		int PointTotal;

		/*
		**	This is the targeting directions for when this house gets a
		**	special weapon.
		*/
		QuarryType PreferredTarget;

	//private:

		/*
		**	Tracks number of each building type owned by this house. Even if the
		**	building is in construction, it will be reflected in this total.
		*/
		CounterClass BQuantity;
		CounterClass UQuantity;
		CounterClass IQuantity;
		CounterClass AQuantity;

		/*
		 * These track the number of each type actually in this house's service. Unlike the
		 * BQuantity totals, nothing is counted until it is out of limbo and under way.
		 */
		CounterClass ABQuantity;
		CounterClass AUQuantity;
		CounterClass AIQuantity;
		CounterClass AAQuantity;

		/*
		 * These track how many of each type this house has ever produced. The tally is never
		 * decremented, so a negative build limit caps the number ever made rather than owned.
		 */
		CounterClass PBQuantity;
		CounterClass PUQuantity;
		CounterClass PIQuantity;
		CounterClass PAQuantity;

		/*
		**	This timer keeps track of when an all out attack should be performed.
		**	When this timer expires, send most of this house's units in an
		**	attack.
		*/
		struct AttackStruct {
			CDTimerClass<FrameTimerClass> Timer;

			/// Unused
			unsigned InitialAttack;
			AttackStruct(void) { InitialAttack = 0; }

			/// Carries the all out attack schedule to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(Timer);
				stream.Serialize(InitialAttack);
			}
		} Attack;

	public:
		/*
		**	This records the overriding enemy that the computer will try to
		**	destroy. Typically, this is the last house to attack, but can be
		**	influenced by nearness.
		*/
		HousesType Enemy;

		/*
		 * These are the anger records this house keeps -- one for every other house in the
		 * game, wired up as the houses are created.
		 */
		DynamicVectorClass<AngerStruct> AngerNodes;

		/*
		 * These are the scouting records this house keeps on every other house, so that the
		 * computer knows which of its rivals it has actually laid eyes on.
		 */
		DynamicVectorClass<ScoutStruct> ScoutNodes;

		/*
		**	The house expert system is regulated by this timer. Each computer controlled
		**	house will process the Expert System AI at intermittent intervals. Not only will
		**	this distribute the overhead more evenly, but will add variety to play.
		*/
		CDTimerClass<FrameTimerClass> AITimer;
		CDTimerClass<FrameTimerClass> PickEnemyTimer;

		/*
		**	This elaborates the suggested objects to construct. When the specified object
		**	is constructed, then this corresponding value will be reset to nill state. The
		**	expert system decides what should be produced, and then records the
		**	recommendation in these variables.
		*/
		StructType BuildStructure;
		UnitType BuildUnit;
		InfantryType BuildInfantry;
		AircraftType BuildAircraft;

		/*
		 * This is the percent chance (0 - 100) that the house will consider its AI triggers
		 * when it goes looking for another team to raise.
		 */
		int RatioAITriggerTeam;

		/*
		 * These are the shares (0 - 100) of the house's team building effort that are meant to
		 * go toward aircraft, infantry and vehicles.
		 */
		int RatioTeamAircraft;
		int RatioTeamInfantry;
		int RatioTeamUnits;

		/*
		 * This is the number of base defense teams this house has in the field. While it is
		 * short of its minimum, the AI triggers will raise nothing but further defense teams.
		 */
		int BaseDefenseTeamCount;

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		HouseClass(HouseTypeClass const * type = NULL);
		virtual ~HouseClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;
		virtual HRESULT STDMETHODCALLTYPE Load(IStream * stream) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
		virtual ULONG STDMETHODCALLTYPE Release(void) override;

		int Available_Money(void);
		int Available_Storage(void);
		int Power_Output(void);
		int Power_Drain(void);
		bool Fire_Sale(void);
		void All_To_Hunt(void);

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual int Fetch_Heap_ID(void) const override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		Cell Random_Cell_In_Zone(ZoneType zone) const;
		static void Computer_Paranoid(void);
		bool Is_Allowed_To_Ally(HouseClass * house) const;
		void Super_Weapon_Handler(void);
		int * Factory_Counter(RTTIType rtti);
		int Factory_Count(RTTIType rtti) const;
		DiffType Assign_Handicap(DiffType handicap);
		AbstractClass * Find_Juicy_Target(Coord const & coord) const;
		Cell Where_To_Go(FootClass const * object) const;
		Cell Zone_Cell(ZoneType zone) const;
		ZoneType Which_Zone(Coord const & coord) const;
		ZoneType Which_Zone(ObjectClass const * object) const;
		ZoneType Which_Zone(Cell const & cell) const;
		Cell Find_Cell_In_Zone(TechnoClass const * techno, ZoneType zone) const;
		ProdFailType Begin_Production(RTTIType type, int id, bool resume=false);
		ProdFailType Suspend_Production(RTTIType type);
		ProdFailType Abandon_Production(RTTIType type, int id);
		bool Place_Object(RTTIType type, Cell const & cell);
		bool Manual_Place(BuildingClass * builder, BuildingClass * object);
		void Just_Built(TechnoClass * product);
		void Special_Weapon_AI(SuperWeaponType id);
		bool Place_Special_Blast(SuperWeaponType id, Cell const & cell);
		bool Flag_Attach(Cell const & cell, bool set_home = false);
		bool Flag_Attach(UnitClass * object, bool set_home = false);
		bool Flag_Remove(AbstractClass * target, bool set_home = false);
		void Init_Data(int color, HousesType house, int credits);
		Coord Find_Build_Location(BuildingClass * building) const;
		BuildingClass * Find_Building(StructType type, ZoneType zone=ZONE_NONE) const;
		int Expert_AI(void);
		void Production_Begun(TechnoClass const * rtti);
		void Sell_Wall(Cell const & cell, bool quiet = true);
		bool Flag_To_Die(void);
		bool Flag_To_Win(bool silent = false);
		bool Flag_To_Lose(bool silent = false);
		void Flag_To_End(void);
		void Make_Ally(HouseClass * house);
		void Make_Ally(HousesType house);
		void Make_Ally(ObjectClass * object);
		void Make_Enemy(HouseClass * house);
		void Make_Enemy(HousesType house);
		void Make_Enemy(ObjectClass * object);
		bool Is_Ally(HousesType house) const;
		bool Is_Ally(HouseClass const * house) const;
		bool Is_Ally(ObjectClass const * object) const;
		bool Is_Ally(AbstractClass const * target) const;
#ifdef _DEBUG
		void Debug_Dump(MonoClass *mono) const;
		void Print_Zone_Stats(int x, int y, ZoneType zone, MonoClass * mono) const;
#endif
		virtual void AI(void) override;

		void Add_Anger(int level, HouseClass const * house);
		void Mark_Scouted(HouseClass const * house);

		void Begin_Construction(void);
		BuildingTypeClass const * Get_First_Ownable(DynamicVectorClass<BuildingTypeClass const *> const & owned) const;
		bool AI_Has_Prerequisites(TechnoTypeClass const * type, DynamicVectorClass<BuildingTypeClass const *> & owned, int ownedcount) const;
		void Make_Base_Nodes(void);
		static int Base_Cell_Weight_By_Distance(HouseClass const & house, Cell const & cell, int tie_breaker, int context);
		static int Base_Cell_Weight_By_Defense_Coverage(HouseClass const & house, Cell const & cell, int tie_breaker, int context);
		Cell Where_To_Place_Upgrade(BuildingTypeClass const * upgrade) const;
		Cell Where_To_Place_Building(BuildingTypeClass *buildingtype, int (*func)(HouseClass const &, Cell const &, int, int), int context);
		void Calculate_Defense_Values(BuildingClass const * building, int value, int * values, Rect const & area) const;
		DynamicVectorClass<BuildingTypeClass *> Get_Anti_Air_Defense_Buildings(DynamicVectorClass<BuildingTypeClass const *> & owned) const;
		DynamicVectorClass<BuildingTypeClass *> Get_Anti_Armor_Defense_Buildings(DynamicVectorClass<BuildingTypeClass const *> & owned) const;
		DynamicVectorClass<BuildingTypeClass *> Get_Anti_Ground_Defense_Buildings(DynamicVectorClass<BuildingTypeClass const *> & owned) const;

		void Recalc_Power_Drain(void);
		void Recalc_Radar_Availability(void);

		void Update_Present_Super_Weapons(void);
		void Enable_Available_Super_Weapons(void);

		void Calculate_Enemy_Predictions(void);
		bool AI_Build_Defense(int nodeid, DynamicVectorClass<Cell> * cells);
		void AI_Build_Wall(void);

		void Clear_Anger(HouseClass const * house);
		void Recalc_Threat_Regions(void);
		void Activate_Threat_Node(void);

		int Owned_Team_Count(TeamTypeClass const * teamtype) const;
		bool Can_Create_Team(TeamTypeClass const * teamtype);

		WaypointPathClass * Ensure_Path(PathType path);
		void Place_Waypoint(Coord const & coord);
		bool Select_Waypoint(Coord const & coord);
		bool Can_Place_Waypoint(Cell const & cell);
		bool Fetch_Waypoint_Data(WaypointClass * waypt, PathType & path, char & waypt_id);
		WaypointClass * Waypoint_At(Cell const & cell);
		PathType New_Waypoint_Path(void) const;
		bool Can_Add_Waypoint_To_Path(void) const;

		// Factory controls.
		FactoryClass * Fetch_Factory(RTTIType rtti) const;
		void Set_Factory(RTTIType rtti, FactoryClass * factory);
		void Update_Production_Mode(RTTIType type);
		void Production_Status_Changed(void) {IsRecalcNeeded = true;}

		int Can_Build(ObjectTypeClass const * type, bool illegal, bool nofactory) const;
		FactoryClass * Factory_Producing_This(ObjectTypeClass const * object) const;

		TechnoTypeClass const * Suggest_New_Object(RTTIType objectype, bool kennel=false) const;
		BuildingTypeClass const * Suggest_New_Building(void) const;
		void Recalc_Center(void);
		bool Does_Enemy_Building_Exist(StructType) const;
		void Harvested(int tiberium, TiberiumType slot);
		void Harvested_Weed(int weed, int slot);
		void Spend_Money(int money);
		void Refund_Money(int money);
		void Attacked(BuildingClass * source);
		void Adjust_Power(int adjust);
		void Adjust_Drain(int adjust);
		void Update_Spied_Radar(HouseClass * house);
		void Update_Spied_Power_Plants(void);
		void Update_Factories(RTTIType rtti);
		double Power_Fraction(void) const;
		double Tiberium_Fraction(void) const;
		double Weed_Fraction(void) const;
		void Begin_Production(void) {IsStarted = true;};
		SUGGESTED_TEAM_LIST Suggested_New_Team(bool alertcheck = false);
		void Adjust_Threat(int region, int threat);
		void Tracking_Remove(TechnoClass const * techno);
		void Tracking_Add(TechnoClass const * techno);
		void Tracking_Active_Remove(TechnoClass * techno, bool bycapture);
		void Tracking_Active_Add(TechnoClass * techno, bool bycapture);
		void Active_Remove(TechnoClass const * techno);
		void Active_Add(TechnoClass const * techno);

		UrgencyType Check_Fire_Sale(void);
		UrgencyType Check_Raise_Money(void);

		bool AI_Fire_Sale(UrgencyType urgency);
		bool AI_Raise_Money(UrgencyType urgency);

		void AI_Super_Weapons(void);
		void AI_Ion_Cannon(SuperClass * super);
		void AI_Hunter_Seeker(SuperClass * super);
		void AI_Multi_Missile(SuperClass * super);
		void AI_Chem_Missile(SuperClass * super);
		void AI_Drop_Pods(SuperClass * super);

		void AI_Takeover(void);

		void Activate_Firestorm(void);
		void Deactivate_Firestorm(void);
		void Lost_Firestorm_Generator(void);

		bool Can_Make_Money(void);

		static void One_Time(void);

		void Invalidate_Base_Node_Position(BuildingClass * building);
		bool Is_Build_Limited(TechnoTypeClass const * type) const;

		bool Is_Player_Control(void) const;
		bool Is_Human_Player(void) const;

		bool Can_Build_Here(BuildingTypeClass *building,  Cell const & cell);

		/*
		**	File I/O.
		*/
		static void Read_All(CCINIClass const & ini);
		void Read_INI(CCINIClass const & ini);
		static void Write_All(CCINIClass & ini);
		void Write_INI(CCINIClass & ini);

		/*
		**	Special house actions.
		*/
		void Detach(AbstractClass const * target, bool all);

		/*
		 * These are the three loadouts the player fills in on the dropship screen; a dropship
		 * reinforcement delivers whatever is listed in the one CurrentDropship names.
		 */
		DropshipLoadoutClass DropshipLoadouts[3];

		/*
		 * This is the loadout that the next dropship reinforcement will arrive with. It
		 * advances after each delivery, so the three loadouts are spent in turn.
		 */
		int CurrentDropship;

		/// Unused
		bool HasCloakGenerator;

		/*
		 * This is the color that identifies this house on the radar, derived from its color
		 * scheme in the pixel format the display is currently running in.
		 */
		RGBClass RemapColorRGB;
		void Initialize_Radar_Color(void);

		/*
		 * This is the base plan for this house -- the structures the computer means to have
		 * and the cells it intends to put them in.
		 */
		BaseClass Base;

		/*
		 * These flags are raised whenever something might have changed this house's power
		 * output or radar availability; the next house AI pass recalculates and clears them.
		 */
		bool RecalcPower;
		bool RecalcRadar;

		/*
		 * This is the cell that this house's EM pulse cannon has been ordered to fire at. The
		 * cannon aims itself here when it is told to launch.
		 */
		Cell EMPDest;

		/*
		**	This count down timer class decrements and then changes
		**	the Atomic Bomb state.
		*/
		Cell NukeDest;

		/*
		**	This routine completely removes this house & all its objects from the game.
		*/
		void Clobber_All(void);

		/*
		**	This routine blows up everything in this house.  Fun!
		*/
		void Blowup_All(void);

		/*
		**	This routine gets called in multiplayer games when every unit, building,
		**	and infantry for a house is destroyed.
		*/
		void MPlayer_Defeated(void);

		/*
		**	When the game's over, this routine assigns everyone their score.
		*/
		///void Tally_Score(void); // Moved to MPScore

		friend class MapEditClass;

	private:
		void Silo_Redraw_Check(int oldtib, int oldcap);
		int AI_Building(void);
		int AI_Unit(void);
		int AI_Infantry(void);
		int AI_Aircraft(void);

		/*
		**	This is a bit field record of all the other houses that are allies with
		**	this house. It is presumed that any house that isn't an ally, is therefore
		**	an enemy. A house is always considered allied with itself.
		*/
		unsigned Allies;

		/*
		**	General low-power related damaged is doled out whenever this timer
		**	expires.
		*/
		CDTimerClass<FrameTimerClass> DamageTime;

		/*
		**	Team creation is done whenever this timer expires.
		*/
		CDTimerClass<FrameTimerClass> TeamTime;

		/*
		**	This controls the rate that the trigger time logic is processed.
		*/
		CDTimerClass<FrameTimerClass> TriggerTime;

		/*
		**	At various times, the computer may announce the player's condition. The following
		**	variables are used as countdown timers so that these announcements are paced
		**	far enough apart to reduce annoyance.
		*/
		CDTimerClass<FrameTimerClass> SpeakAttackDelay;
		CDTimerClass<FrameTimerClass> SpeakPowerDelay;
		CDTimerClass<FrameTimerClass> SpeakMoneyDelay;
		CDTimerClass<FrameTimerClass> SpeakMaxedDelay;

		/*
		**	This structure is used to record a build request as determined by
		**	the house AI processing. Higher priority build requests take precidence.
		*/
		struct BuildChoiceClass {
			UrgencyType	Urgency;    // The urgency of the build request.
			StructType	Structure;  // The type of building to produce.

			BuildChoiceClass(UrgencyType urgency=URGENCY_NONE, StructType structure=STRUCT_NONE) : Urgency(urgency), Structure(structure) {};
			bool operator==(BuildChoiceClass const & ) const {return(false);}
			bool operator!=(BuildChoiceClass const & ) const {return(true);}
			HRESULT Save(IStream *) const {return(S_OK);};
			HRESULT Load(IStream *) {return(S_OK);};
		};

		static DynamicVectorClass<BuildChoiceClass *> BuildChoice;

	public:
		/*
		**	This vector holds the recorded status of the map regions. It is through
		**	this region information that team paths are calculated.
		*/
		RegionClass Regions[MAP_TOTAL_REGIONS];

	/*
	**	These values are for multiplay only.
	*/
	public:
		/*
		**	This is the name ("handle") the player has chosen for himself.
		*/
		TStringID<HOUSE_NAME_MAX> IniName;

		/*
		**	For multiplayer games, each house instance has a remap table; the table
		**	in the HousesTypeClass isn't used.  This variable is set to the remap
		**	table for the color the player wants to play.
		*/
		int Scheme;

		int QuantityB(int index) {return(BQuantity.Value(index));}
		int QuantityU(int index) {return(UQuantity.Value(index));}
		int QuantityI(int index) {return(IQuantity.Value(index));}
		int QuantityA(int index) {return(AQuantity.Value(index));}

		/*
		 * Pointer to the coverage grid, over Base.BaseAreaRect, for the kind of defense being
		 * planned. The cell weighing routine is static, so this is how it reaches the grid.
		 */
		int *BaseAreaMap;

		/// Unused
		int field_10E20;

		/// Unused
		double field_10E28;

		/*
		 * This scales the number of base defenses the computer plans against the value of the
		 * base it is protecting. It is left at 1.0 throughout, so cost alone decides.
		 */
		double DefenseCostMultiplier;

		/// Unused
		int field_10E38;

		/*
		 * These are the shares of armor, air and infantry the house expects the enemy to
		 * field. The base defense planner builds toward whichever cover it is shortest of.
		 */
		float EnemyArmorForcePrediction;
		float EnemyAirForcePrediction;
		float EnemyInfantryForcePrediction;

		/*
		 * This is the power headroom the computer keeps in hand when it judges whether it can
		 * afford another power hungry structure, so that it stops before the base browns out.
		 */
		int PowerSurplus;
};

HouseClass * House_From_HousesType(HousesType house);
