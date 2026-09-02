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

/* $Header: /counterstrike/HOUSE.CPP 4     3/13/97 7:11p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : HOUSE.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 21, 1994                                                 *
 *                                                                                             *
 *                  Last Update : November 4, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   HouseClass::AI -- Process house logic.                                                    *
 *   HouseClass::AI_Aircraft -- Determines what aircraft to build next.                        *
 *   HouseClass::AI_Attack -- Handles offensive attack logic.                                  *
 *   HouseClass::AI_Base_Defense -- Handles maintaining a strong base defense.                 *
 *   HouseClass::AI_Building -- Determines what building to build.                             *
 *   HouseClass::AI_Fire_Sale -- Check for and perform a fire sale.                            *
 *   HouseClass::AI_Infantry -- Determines the infantry unit to build.                         *
 *   HouseClass::AI_Money_Check -- Handles money production logic.                             *
 *   HouseClass::AI_Power_Check -- Handle the power situation.                                 *
 *   HouseClass::AI_Unit -- Determines what unit to build next.                                *
 *   HouseClass::Abandon_Production -- Abandons production of item type specified.             *
 *   HouseClass::Active_Add -- Add an object to active duty for this house.                    *
 *   HouseClass::Active_Remove -- Remove this object from active duty for this house.          *
 *   HouseClass::Adjust_Capacity -- Adjusts the house Tiberium storage capacity.               *
 *   HouseClass::Adjust_Drain -- Adjust the power drain value of the house.                    *
 *   HouseClass::Adjust_Power -- Adjust the power value of the house.                          *
 *   HouseClass::Adjust_Threat -- Adjust threat for the region specified.                      *
 *   HouseClass::As_Pointer -- Converts a house number into a house object pointer.            *
 *   HouseClass::Assign_Handicap -- Assigns the specified handicap rating to the house.        *
 *   HouseClass::Attacked -- Lets player know if base is under attack.                         *
 *   HouseClass::Available_Money -- Fetches the total credit worth of the house.               *
 *   HouseClass::Begin_Production -- Starts production of the specified object type.           *
 *   HouseClass::Blowup_All -- blows up everything                                             *
 *   HouseClass::Can_Build -- General purpose build legality checker.                          *
 *   HouseClass::Clobber_All -- removes all objects for this house                             *
 *   HouseClass::Computer_Paranoid -- Cause the computer players to becom paranoid.            *
 *   HouseClass::Debug_Dump -- Dumps the house status data to the mono screen.                 *
 *   HouseClass::Detach -- Removes specified object from house tracking systems.               *
 *   HouseClass::Do_All_To_Hunt -- Send all units to hunt.                                     *
 *   HouseClass::Does_Enemy_Building_Exist -- Checks for enemy building of specified type.     *
 *   HouseClass::Expert_AI -- Handles expert AI processing.                                    *
 *   HouseClass::Factory_Count -- Fetches the number of factories for specified type.          *
 *   HouseClass::Factory_Counter -- Fetches a pointer to the factory counter value.            *
 *   HouseClass::Fetch_Factory -- Finds the factory associated with the object type specified. *
 *   HouseClass::Find_Build_Location -- Finds a suitable building location.                    *
 *   HouseClass::Find_Building -- Finds a building of specified type.                          *
 *   HouseClass::Find_Cell_In_Zone -- Finds a legal placement cell within the zone.            *
 *   HouseClass::Find_Juicy_Target -- Finds a suitable field target.                           *
 *   HouseClass::Fire_Sale -- Cause all buildings to be sold.                                  *
 *   HouseClass::Flag_Attach -- Attach flag to specified cell (or thereabouts).                *
 *   HouseClass::Flag_Attach -- Attaches the house flag the specified unit.                    *
 *   HouseClass::Flag_Remove -- Removes the flag from the specified target.                    *
 *   HouseClass::Flag_To_Die -- Flags the house to blow up soon.                               *
 *   HouseClass::Flag_To_Lose -- Flags the house to die soon.                                  *
 *   HouseClass::Flag_To_Win -- Flags the house to win soon.                                   *
 *   HouseClass::Get_Quantity -- Fetches the total number of aircraft of the specified type.   *
 *   HouseClass::Get_Quantity -- Gets the quantity of the building type specified.             *
 *   HouseClass::Harvested -- Adds Tiberium to the harvest storage.                            *
 *   HouseClass::HouseClass -- Constructor for a house object.                                 *
 *   HouseClass::Init -- init's in preparation for new scenario                                *
 *   HouseClass::Init_Data -- Initializes the multiplayer color data.                          *
 *   HouseClass::Is_Allowed_To_Ally -- Determines if this house is allied to make allies.      *
 *   HouseClass::Is_Ally -- Checks to see if the object is an ally.                            *
 *   HouseClass::Is_Ally -- Determines if the specified house is an ally.                      *
 *   HouseClass::Is_Hack_Prevented -- Is production of the specified type and id prohibted?    *
 *   HouseClass::Is_No_YakMig -- Determines if no more yaks or migs should be allowed.         *
 *   HouseClass::MPlayer_Defeated -- multiplayer; house is defeated                            *
 *   HouseClass::Make_Ally -- Make the specified house an ally.                                *
 *   HouseClass::Make_Enemy -- Make an enemy of the house specified.                           *
 *   HouseClass::Manual_Place -- Inform display system of building placement mode.             *
 *   HouseClass::One_Time -- Handles one time initialization of the house array.               *
 *   HouseClass::Place_Object -- Places the object (building) at location specified.           *
 *   HouseClass::Place_Special_Blast -- Place a special blast effect at location specified.    *
 *   HouseClass::Power_Fraction -- Fetches the current power output rating.                    *
 *   HouseClass::Production_Begun -- Records that production has begun.                        *
 *   HouseClass::Read_INI -- Reads house specific data from INI.                               *
 *   HouseClass::Recalc_Attributes -- Recalcs all houses existence bits.                       *
 *   HouseClass::Recalc_Center -- Recalculates the center point of the base.                   *
 *   HouseClass::Refund_Money -- Refunds money to back to the house.                           *
 *   HouseClass::Remap_Table -- Fetches the remap table for this house object.                 *
 *   HouseClass::Sell_Wall -- Tries to sell the wall at the specified location.                *
 *   HouseClass::Set_Factory -- Assign specified factory to house tracking.                    *
 *   HouseClass::Silo_Redraw_Check -- Flags silos to be redrawn if necessary.                  *
 *   HouseClass::Special_Weapon_AI -- Fires special weapon.                                    *
 *   HouseClass::Spend_Money -- Removes money from the house.                                  *
 *   HouseClass::Suggest_New_Building -- Examines the situation and suggests a building.       *
 *   HouseClass::Suggest_New_Object -- Determine what would the next buildable object be.      *
 *   HouseClass::Suggested_New_Team -- Determine what team should be created.                  *
 *   HouseClass::Super_Weapon_Handler -- Handles the super weapon charge and discharge logic.  *
 *   HouseClass::Suspend_Production -- Temporarily puts production on hold.                    *
 *   HouseClass::Tally_Score -- Fills in the score system for this round                       *
 *   HouseClass::Tiberium_Fraction -- Calculates the tiberium fraction of capacity.            *
 *   HouseClass::Tracking_Add -- Informs house of new inventory item.                          *
 *   HouseClass::Tracking_Remove -- Remove object from house tracking system.                  *
 *   HouseClass::Where_To_Go -- Determines where the object should go and wait.                *
 *   HouseClass::Which_Zone -- Determines what zone a coordinate lies in.                      *
 *   HouseClass::Which_Zone -- Determines which base zone the specified cell lies in.          *
 *   HouseClass::Which_Zone -- Determines which base zone the specified object lies in.        *
 *   HouseClass::Write_INI -- Writes the house data to the INI database.                       *
 *   HouseClass::Zone_Cell -- Finds the cell closest to the center of the zone.                *
 *   HouseClass::delete -- Deallocator function for a house object.                            *
 *   HouseClass::new -- Allocator for a house class.                                           *
 *   HouseClass::operator HousesType -- Conversion to HousesType operator.                     *
 *   HouseClass::~HouseClass -- Default destructor for a house object.                         *
 *   HouseStaticClass::HouseStaticClass -- Default constructor for house static class.         *
 *   HouseClass::AI_Raise_Power -- Try to raise power levels by selling off buildings.         *
 *   HouseClass::AI_Raise_Money -- Raise emergency cash by selling buildings.                  *
 *   HouseClass::Random_Cell_In_Zone -- Find a (technically) legal cell in the zone specified. *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "house.h"

#include "_logic.h"
#include "_map.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "aircraft.h"
#include "airctype.h"
#include "building.h"
#include "builtype.h"
#include "ccrand.h"
#include "cell.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "ddist.h"
#include "dsurface.h"
#include "factory.h"
#include "globals.h"
#include "goptions.h"
#include "houstype.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "language\language.h"
#include "lightcon.h"
#include "logic.h"
#include "mono.h"
#include "mouse.h"
#include "overtype.h"
#include "quarry.h"
#include "revent.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "scheme.h"
#include "session.h"
#include "sun.h"
#include "super.h"
#include "suprtype.h"
#include "surface.h"
#include "tactical.h"
#include "tag.h"
#include "taskforc.h"
#include "team.h"
#include "techtype.h"
#include "tiberium.h"
#include "tracker.h"
#include "trigger.h"
#include "trigtype.h"
#include "unit.h"
#include "unittype.h"
#include "vector.h"
#include "vein.h"
#include "voc.h"
#include "vox.h"
#include "waypoint.h"

#include "color.hh"
#include "strategy.hh"

#include <algorithm>
#include <cassert>
#include <vector>


DynamicVectorClass<HouseClass::BuildChoiceClass *> HouseClass::BuildChoice;


/***********************************************************************************************
 * HouseClass::HouseClass -- Constructor for a house object.                                   *
 *                                                                                             *
 *    This function is the constructor and it marks the house object                           *
 *    as being allocated.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
HouseClass::HouseClass(HouseTypeClass const * type) :
	BASECLASS(),
	HeapID(HOUSE_NONE),
	Class((HouseTypeClass *)type),
	HouseTags(),
	ConYards(),
	Difficulty(Scen->CDifficulty),
	FirepowerBias(1.0),
	GroundspeedBias(1.0),
	AirspeedBias(1.0),
	ArmorBias(1.0),
	ROFBias(1.0),
	CostBias(1.0),
	BuildSpeedBias(1.0),
	RepairDelay(0.0),
	BuildDelay(0.0),
	ProductionMode(EVERYTHING),
	IsHuman(false),
	IsPlayerControl(false),
	IsStarted(false),
	IsAlerted(false),
	IsAITriggersOn(false),
	IsBaseBuilding(false),
	IsDiscovered(false),
	IsDefeated(false),
	IsObserver(false),
	IsToDie(false),
	IsToLose(false),
	IsToWin(false),
	IsCivEvacuated(false),
	FirestormDefenseActivated(false),
	IsThreatRatingNodeActive(false),
	IsRecalcNeeded(true),
	IPAddress(0),
	SquadID(0),
	LostConnection(false),
	SelectedPath(PATH_NONE),
	IsVisionary(false),
	IsTiberiumShort(false),
	IsSpied(false),
	IsThieved(false),
	IsBuiltSomething(false),
	IsResigner(false),
	IsGiverUpper(false),
	IsAllToHunt(false),
	IsParanoid(false),
	IsToLook(true),
	DidRepair(false),
	IQ(Control.IQ),
	State(STATE_BUILDUP),
	JustBuiltStructure(STRUCT_NONE),
	JustBuiltInfantry(INFANTRY_NONE),
	JustBuiltUnit(UNIT_NONE),
	JustBuiltAircraft(AIRCRAFT_NONE),
	Blockage(0),
	RepairTimer(0),
	AlertTime(0),
	BorrowedTime(0),
	CreditsSpent(0),
	HarvestedCredits(0),
	StolenBuildingsCredits(0),
	CurUnits(0),
	CurBuildings(0),
	CurInfantry(0),
	CurAircraft(0),
	Credits(0),
	Capacity(0),
	AircraftTotals(NULL),
	InfantryTotals(NULL),
	UnitTotals(NULL),
	BuildingTotals(NULL),
	DestroyedAircraft(NULL),
	DestroyedInfantry(NULL),
	DestroyedUnits(NULL),
	DestroyedBuildings(NULL),
	CapturedBuildings(NULL),
	TotalCrates(NULL),
	AircraftFactories(0),
	InfantryFactories(0),
	UnitFactories(0),
	BuildingFactories(0),
	Power(0),
	Drain(0),
	AircraftFactory(NULL),
	InfantryFactory(NULL),
	UnitFactory(NULL),
	BuildingFactory(NULL),
	FlagLocation(NULL),
	FlagHome(0,0),
	UnitsLost(0),
	BuildingsLost(0),
	WhoLastHurtMe(HOUSE_NONE),
	Center(0,0,0),
	SpawnWaypoint(-1),
	Radius(0),
	LATime(0),
	LAEnemy(HOUSE_NONE),
	ToCapture(NULL),
	RadarSpied(0),
	PointTotal(0),
	PreferredTarget(QUARRY_ANYTHING),
	Attack(),
	Enemy(HOUSE_NONE),
	AngerNodes(),
	ScoutNodes(),
	AITimer(0),
	BuildStructure(STRUCT_NONE),
	BuildUnit(UNIT_NONE),
	BuildInfantry(INFANTRY_NONE),
	BuildAircraft(AIRCRAFT_NONE),
	RatioAITriggerTeam(100),
	RatioTeamAircraft(75),
	RatioTeamInfantry(75),
	RatioTeamUnits(75),
	BaseDefenseTeamCount(0),
	CurrentDropship(0),
	HasCloakGenerator(0),
	RecalcPower(true),
	RecalcRadar(true),
	EMPDest(0,0),
	NukeDest(0,0),
	Allies(0),
	DamageTime(TICKS_PER_MINUTE * Rule->DamageDelay),
	TeamTime(1),
	TriggerTime(0),
	SpeakAttackDelay(1),
	SpeakPowerDelay(1),
	SpeakMoneyDelay(1),
	SpeakMaxedDelay(1),
	IniName(NULL),
	Scheme(0),
	BaseAreaMap(0),
	field_10E20(0),
	field_10E28(1.0),
	DefenseCostMultiplier(1.0),
	EnemyArmorForcePrediction(0.33f),
	EnemyAirForcePrediction(0.33f),
	EnemyInfantryForcePrediction(0.34f),
	PowerSurplus(0)
{
	int index;

	Create_ID();

	if (Class) {
		Scheme = Class->Scheme;
	}

	AbstractTypePtrTracker.Add(this);
	FactoryPtrTracker.Add(this);
	WaypointPathPtrTracker.Add(this);
	TagPtrTracker.Add(this);
	ObjectPtrTracker.Add(this);

	HeapID = (HousesType)Houses.Count();

	if (type != NULL) {
		for (index = 0; index < Houses.Count(); index++) {

			Houses[index]->AngerNodes.Add(this);
			AngerNodes.Add(Houses[index]);

			Houses[index]->ScoutNodes.Add(this);
			ScoutNodes.Add(Houses[index]);
		}
	}

	Houses.Add(this);

	memset((void *)&Regions[0], 0x00, sizeof(Regions));

	/*
	**	Explicit in-place construction of the super weapons is
	**	required here because the default constructor for super
	**	weapons must serve as a no-initialization constructor (save/load reasons).
	*/
	for (index = 0; index < SuperWeaponTypes.Count(); index++) {
		SuperWeapon.Add(new SuperClass(SuperWeaponTypes[index], this));
	}

	memset(UnitsKilled, 0, sizeof(UnitsKilled));
	memset(BuildingsKilled, 0, sizeof(BuildingsKilled));
	IniName = Fetch_String(TXT_COMPUTER);	// Default computer name.
	memset((void *)&Regions[0], 0x00, sizeof(Regions));
	//Allies |= (1L << HeapID);
	Control.Allies |= (1L << HeapID);

	/*
	**	Set the time of the first AI attack.
	*/
	int attack = Random_Pick(TICKS_PER_MINUTE/2, TICKS_PER_MINUTE*2);
	attack = int(attack * Rule->AttackDelay);
	Attack.Timer = attack;
	Attack.InitialAttack = attack;

	if (Session.Type == GAME_INTERNET) {
		AircraftTotals = new UnitTrackerClass(AircraftTypes.Count());
		InfantryTotals = new UnitTrackerClass(InfantryTypes.Count());
		UnitTotals = new UnitTrackerClass(UnitTypes.Count());
		BuildingTotals = new UnitTrackerClass(BuildingTypes.Count());

		DestroyedAircraft = new UnitTrackerClass(AircraftTypes.Count());
		DestroyedInfantry = new UnitTrackerClass(InfantryTypes.Count());
		DestroyedUnits = new UnitTrackerClass(UnitTypes.Count());
		DestroyedBuildings =new UnitTrackerClass(BuildingTypes.Count());

		CapturedBuildings = new UnitTrackerClass (BuildingTypes.Count());
		TotalCrates = new UnitTrackerClass (CRATE_COUNT);
	}

	for (index = 0; index < ARRAY_SIZE(Paths); index++) {
		Paths[index] = 0;
	}

	if (type != NULL) {
		IsThreatRatingNodeActive = true;
	}

	ActLike = HOUSE_NONE;
	if (Class != NULL) {
		if (strnicmp(Class->Name(), "GDI", 3) == 0) {
			ActLike = HOUSE_GOOD;
		} else if (strnicmp(Class->Name(), "Nod", 3) == 0) {
			ActLike = HOUSE_BAD;
		}
	}
}


/// <summary>
/// Can this house still bring in money?
/// This routine is used by the base building logic to decide whether the house owns, or can
/// afford to build, the refinery and harvester it needs to keep its income flowing. A house
/// that answers no must put what credits it has toward fixing that before anything else.
/// </summary>
/// <returns>bool; Is the house able to keep earning credits?</returns>
bool HouseClass::Can_Make_Money(void)
{
	int credits = Available_Money();
	int refcost = Rule->BuildRefinery[0]->Cost_Of(this);
	int harvcost = Rule->HarvesterUnit[0]->Cost_Of(this);

	bool hasref = ABQuantity.Value(Rule->BuildRefinery[0]->HeapID) > 0;
	bool hasharv = AUQuantity.Value(Rule->HarvesterUnit[0]->HeapID) > 0;

	/*
	 * If we don't have any refineries, building one is a priority.
	 */
	if (hasref) {

		/*
		 * If we have a refinery and a harvester, all's well.
		 */
		if (hasharv) {
			return(true);
		}

		bool hasfactory = false;
		for (int index = 0; index < Rule->BuildWeapons.Count(); index++) {
			if (hasfactory || ABQuantity.Value(Rule->BuildWeapons[index]->HeapID) > 0) {
				hasfactory = true;
			} else {
				hasfactory = false;
			}
		}

		int factorycost = Rule->BuildWeapons[0]->Cost_Of(this);
		if ((hasfactory && credits >= harvcost) || (credits >= harvcost + factorycost) || (credits >= refcost)) {
			return(true);
		}

		return(false);
	}

	if (credits > refcost) {
		return(true);
	}

	return(false);
}


/***********************************************************************************************
 * HouseClass::Available_Money -- Fetches the total credit worth of the house.                 *
 *                                                                                             *
 *    Use this routine to determine the total credit value of the house. This is the sum of    *
 *    the harvested Tiberium in storage and the initial unspent cash reserves.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the total credit value of the house.                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int HouseClass::Available_Money(void)
{
	return(Credits + Tiberium.Get_Total_Value());
}


/// <summary>
/// Fetches the unused tiberium storage of this house.
/// </summary>
/// <returns>Returns with the amount of tiberium the house still has room for.</returns>
int HouseClass::Available_Storage(void)
{
	return(Capacity - Tiberium.Get_Total_Amount());
}


/// <summary>
/// Fetches the power production of this house's base.
/// </summary>
/// <returns>Returns with the power that the house's buildings produce.</returns>
int HouseClass::Power_Output(void)
{
	return(Power);
}


/// <summary>
/// Fetches the power draw of this house's base.
/// </summary>
/// <returns>Returns with the power that the house's buildings consume.</returns>
int HouseClass::Power_Drain(void)
{
	return(Drain);
}


/***********************************************************************************************
 * HouseClass::Tiberium_Fraction -- Calculates the tiberium fraction of capacity.              *
 *                                                                                             *
 *    This will calculate the current tiberium (gold) load as a ratio of the maximum storage   *
 *    capacity.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the current tiberium storage situation as a ratio of load over capacity.   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
double HouseClass::Tiberium_Fraction(void) const
{
	if (Tiberium.Get_Total_Amount() == 0) {
		return(0.f);
	}
	return((double)Tiberium.Get_Total_Amount() / Capacity);
}


/***********************************************************************************************
 * HouseClass::One_Time -- Handles one time initialization of the house array.                 *
 *                                                                                             *
 *    This basically calls the constructor for each of the houses in the game. All other       *
 *    data specific to the house is initialized when the scenario is loaded.                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this ONCE at the beginning of the game.                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::One_Time(void)
{
	// empty
}


/***********************************************************************************************
 * HouseClass::Assign_Handicap -- Assigns the specified handicap rating to the house.          *
 *                                                                                             *
 *    The handicap rating will affect combat, movement, and production for the house. It can   *
 *    either make it more or less difficult for the house (controlled by the handicap value).  *
 *                                                                                             *
 * INPUT:   handicap -- The handicap value to assign to this house. The default value for      *
 *                      a house is DIFF_NORMAL.                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the old handicap value.                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *   10/22/1996 JLB : Uses act like value for multiplay only.                                  *
 *=============================================================================================*/
DiffType HouseClass::Assign_Handicap(DiffType handicap)
{
	DiffType old = Difficulty;
	Difficulty = handicap;

	if (Session.Type != GAME_NORMAL) {
		HouseTypeClass const * hptr = Class;
		FirepowerBias = hptr->FirepowerBias * Rule->Diff[handicap].FirepowerBias;
		GroundspeedBias = hptr->GroundspeedBias * Rule->Diff[handicap].GroundspeedBias * Rule->GameSpeedBias;
		AirspeedBias = hptr->AirspeedBias * Rule->Diff[handicap].AirspeedBias * Rule->GameSpeedBias;
		ArmorBias = hptr->ArmorBias * Rule->Diff[handicap].ArmorBias;
		ROFBias = hptr->ROFBias * Rule->Diff[handicap].ROFBias;
		CostBias = hptr->CostBias * Rule->Diff[handicap].CostBias;
		RepairDelay = Rule->Diff[handicap].RepairDelay;
		BuildDelay = Rule->Diff[handicap].BuildDelay;
		BuildSpeedBias = hptr->BuildSpeedBias * Rule->Diff[handicap].BuildSpeedBias * Rule->GameSpeedBias;
	} else {
		FirepowerBias = Rule->Diff[handicap].FirepowerBias;
		GroundspeedBias = Rule->Diff[handicap].GroundspeedBias * Rule->GameSpeedBias;
		AirspeedBias = Rule->Diff[handicap].AirspeedBias * Rule->GameSpeedBias;
		ArmorBias = Rule->Diff[handicap].ArmorBias;
		ROFBias = Rule->Diff[handicap].ROFBias;
		CostBias = Rule->Diff[handicap].CostBias;
		RepairDelay = Rule->Diff[handicap].RepairDelay;
		BuildDelay = Rule->Diff[handicap].BuildDelay;
		BuildSpeedBias = Rule->Diff[handicap].BuildSpeedBias * Rule->GameSpeedBias;
	}

	TeamTime = 175 * HeapID + Rule->TeamDelays[handicap];

	return(old);
}


/***********************************************************************************************
 * HouseClass::~HouseClass -- House class destructor                                           *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    8/6/96 4:48PM ST : Created                                                               *
 *=============================================================================================*/
HouseClass::~HouseClass (void)
{
	int index;

	Detach_This_From_All(this);
	Class = NULL;
	for (index = 0; index < ARRAY_SIZE(Paths); index++) {
		delete Paths[index];
		Paths[index] = NULL;
	}

	if (Session.Type == GAME_INTERNET) {
		delete AircraftTotals;
		delete InfantryTotals;
		delete UnitTotals;
		delete BuildingTotals;
		delete DestroyedAircraft;
		delete DestroyedInfantry;
		delete DestroyedUnits;
		delete DestroyedBuildings;
		delete CapturedBuildings;
		delete TotalCrates;
	}

	for (index = 0; index < SuperWeapon.Count(); index++) {
		delete SuperWeapon[index];
	}
	SuperWeapon.Clear();

	while (HouseTags.Count() > 0) {
		delete HouseTags[0];
	}

	AbstractTypePtrTracker.Delete(this);
	FactoryPtrTracker.Delete(this);
	WaypointPathPtrTracker.Delete(this);
	TagPtrTracker.Delete(this);
	ObjectPtrTracker.Delete(this);

	if (UnusedHouse == this) {
		UnusedHouse = NULL;
	}

	Houses.Delete(this);
}


#ifdef _DEBUG

/// <summary>
/// Prints the defense ratings of a zone to the debug screen.
/// This routine lays out one zone's line of statistics for the house debug dump.
/// </summary>
/// <param name="x">The mono screen column to print at.</param>
/// <param name="y">The mono screen row to print at.</param>
/// <param name="zone">The zone whose defense ratings should be printed.</param>
/// <param name="mono">The mono screen to print to.</param>
void HouseClass::Print_Zone_Stats(int x, int y, ZoneType zone, MonoClass * mono) const
{
	mono->Set_Cursor(x, y);
	mono->Printf("A:%-5d I:%-5d V:%-5d", ZoneInfo[zone].AirDefense, ZoneInfo[zone].InfantryDefense, ZoneInfo[zone].ArmorDefense);
}


/***********************************************************************************************
 * HouseClass::Debug_Dump -- Dumps the house status data to the mono screen.                   *
 *                                                                                             *
 *    This utility function will output the current status of the house class to the mono      *
 *    screen. Through this information bugs may be fixed or detected.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(0, 0);

	mono->Set_Cursor(1, 1);mono->Printf("[%d]%14.14s", Class->House, Class->Name());
	mono->Set_Cursor(20, 1);mono->Printf("[%d]%13.13s", ActLike, HouseTypes[ActLike]->Name());
	mono->Set_Cursor(39, 1);mono->Printf("%2d", Control.TechLevel);
	mono->Set_Cursor(45, 1);mono->Printf("%2d", Difficulty);
	mono->Set_Cursor(52, 1);mono->Printf("%2d", State);
	mono->Set_Cursor(58, 1);mono->Printf("%2d", Blockage);
	mono->Set_Cursor(65, 1);mono->Printf("%2d", IQ);
	mono->Set_Cursor(72, 1);mono->Printf("%5d", (int)RepairTimer);

	mono->Set_Cursor(10, 3);mono->Printf("%8.8s", (BuildAircraft == AIRCRAFT_NONE) ? " " : AircraftTypes[BuildAircraft]->Graphic_Name());
	mono->Set_Cursor(21, 3);mono->Printf("%3d", CurAircraft);
	mono->Set_Cursor(27, 3);mono->Printf("%8d", Credits);
	mono->Set_Cursor(37, 3);mono->Printf("%5d", Power);
	mono->Set_Cursor(45, 3);mono->Printf("%04X", RadarSpied);
	mono->Set_Cursor(52, 3);mono->Printf("%5d", PointTotal);
	mono->Set_Cursor(62, 3);mono->Printf("%5d", (int)TeamTime);
	mono->Set_Cursor(71, 3);mono->Printf("%5d", (int)AlertTime);

	mono->Set_Cursor(10, 5);mono->Printf("%8.8s", (BuildStructure == STRUCT_NONE) ? " " : BuildingTypes[BuildStructure]->Graphic_Name());
	mono->Set_Cursor(21, 5);mono->Printf("%3d", CurBuildings);
	mono->Set_Cursor(27, 5);mono->Printf("%8d", Tiberium);
	mono->Set_Cursor(37, 5);mono->Printf("%5d", Drain);
	mono->Set_Cursor(44, 5);mono->Printf("%16.16s", Name_From_Quarry(PreferredTarget));
	mono->Set_Cursor(62, 5);mono->Printf("%5d", (int)TriggerTime);
	mono->Set_Cursor(71, 5);mono->Printf("%5d", (int)BorrowedTime);

	mono->Set_Cursor(10, 7);mono->Printf("%8.8s", (BuildUnit == UNIT_NONE) ? " " : UnitTypes[BuildUnit]->Graphic_Name());
	mono->Set_Cursor(21, 7);mono->Printf("%3d", CurUnits);
	mono->Set_Cursor(27, 7);mono->Printf("%8d", Control.InitialCredits);
	mono->Set_Cursor(38, 7);mono->Printf("%5d", UnitsLost);
	mono->Set_Cursor(44, 7);mono->Printf("%08X", Allies);
	mono->Set_Cursor(71, 7);mono->Printf("%5d", (int)Attack.Timer);

	mono->Set_Cursor(10, 9);mono->Printf("%8.8s", (BuildInfantry == INFANTRY_NONE) ? " " : InfantryTypes[BuildInfantry]->Graphic_Name());
	mono->Set_Cursor(21, 9);mono->Printf("%3d", CurInfantry);
	mono->Set_Cursor(27, 9);mono->Printf("%8d", Capacity);
	mono->Set_Cursor(38, 9);mono->Printf("%5d", BuildingsLost);
	mono->Set_Cursor(45, 9);mono->Printf("%4d", Radius / CELL_LEPTON_W);
	mono->Set_Cursor(71, 9);mono->Printf("%5d", (int)AITimer);

	mono->Set_Cursor(54, 11);mono->Printf("%04X", Center.As_Cell());
	mono->Set_Cursor(71, 11);mono->Printf("%5d", (int)DamageTime);

	for (int index = 0; index < ARRAY_SIZE(Scen->GlobalFlags); index++) {
		mono->Set_Cursor(1+index, 15);
		if (Scen->GlobalFlags[index].Value) {
			mono->Print("1");
		} else {
			mono->Print("0");
		}
		if (index >= 24) break;
	}
	if (Enemy != HOUSE_NONE) {
		char const * name = "";
		mono->Set_Cursor(53, 15);mono->Printf("[%d]%21.21s", Enemy, HouseTypes[Enemy]->Name());
	}

	Print_Zone_Stats(27, 11, ZONE_NORTH, mono);
	Print_Zone_Stats(27, 13, ZONE_CORE, mono);
	Print_Zone_Stats(27, 15, ZONE_SOUTH, mono);
	Print_Zone_Stats(1, 13, ZONE_WEST, mono);
	Print_Zone_Stats(53, 13, ZONE_EAST, mono);

	mono->Fill_Attrib(1, 18, 12, 1, IsHuman ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 19, 12, 1, IsPlayerControl ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 20, 12, 1, IsAlerted ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 21, 12, 1, IsDiscovered ? MonoClass::INVERSE : MonoClass::NORMAL);

	mono->Fill_Attrib(14, 17, 12, 1, IsDefeated ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 18, 12, 1, IsToDie ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 19, 12, 1, IsToWin ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 20, 12, 1, IsToLose ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 21, 12, 1, IsCivEvacuated ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 22, 12, 1, IsRecalcNeeded ? MonoClass::INVERSE : MonoClass::NORMAL);

	mono->Fill_Attrib(27, 17, 12, 1, IsVisionary ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 18, 12, 1, IsTiberiumShort ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 19, 12, 1, IsSpied ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 20, 12, 1, IsThieved ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(27, 22, 12, 1, IsStarted ? MonoClass::INVERSE : MonoClass::NORMAL);

	mono->Fill_Attrib(40, 17, 12, 1, IsResigner ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 18, 12, 1, IsGiverUpper ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 19, 12, 1, IsBuiltSomething ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(40, 20, 12, 1, IsBaseBuilding ? MonoClass::INVERSE : MonoClass::NORMAL);
}
#endif


/***********************************************************************************************
 * HouseStaticClass::HouseStaticClass -- Default constructor for house static class.           *
 *                                                                                             *
 *    This is the default constructor that initializes all the values to their default         *
 *    settings.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
HouseStaticClass::HouseStaticClass(void) :
	IQ(0),
	TechLevel(1),
	Allies(0),
	InitialCredits(0),
	Edge(SOURCE_NORTH)
{
}


/***********************************************************************************************
 * HouseClass::Can_Build -- General purpose build legality checker.                            *
 *                                                                                             *
 *    This routine is called when it needs to be determined if the specified object type can   *
 *    be built by this house. Production and sidebar maintenance use this routine heavily.     *
 *                                                                                             *
 * INPUT:   type  -- Pointer to the type of object that legality is to be checked for.         *
 *                                                                                             *
 *          house -- This is the house to check for legality against. Note that this might     *
 *                   not be 'this' house since the check could be from a captured factory.     *
 *                   Captured factories build what the original owner of them could build.     *
 *                                                                                             *
 * OUTPUT:  Can the specified object be built?                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/04/1995 JLB : Created.                                                                 *
 *   08/12/1995 JLB : Updated for GDI building sandbag walls in #9.                            *
 *   10/23/1996 JLB : Hack to allow Tanya to both sides in multiplay.                          *
 *   11/04/1996 JLB : Computer uses prerequisite record.                                       *
 *=============================================================================================*/
/*
 * 1 = can build
 * 0 = can't build
 * -1 = build limit reached
 */
int HouseClass::Can_Build(ObjectTypeClass const * type, bool forced, bool include_in_progress) const
{
	assert(type != NULL);

	if (!forced) {

		if (((TechnoTypeClass const *)type)->IsUnbuildable) return(0);

		/*
		**	An object with a prohibited tech level availability will never be allowed, regardless
		**	of who requests it.
		*/
		if (((TechnoTypeClass const *)type)->Level == -1) return(0);

		TypeList<int> pre = ((TechnoTypeClass const *)type)->Prerequisite;

		int level = Control.TechLevel;
#ifdef _DEBUG
		if (Debug_Cheat) {
			level = 98;
			pre.Clear();
		}
#endif

		if (((TechnoTypeClass const *)type)->Level > level) return(0);

		/*
		**	The computer can always build everything.
		*/
		if (!Is_Human_Player()) return(1);

		/*
		**	See if the prerequisite requirements have been met.
		*/
		for (int i = 0; i < pre.Count(); i++) {
			switch (pre[i]) {
				case STRUCT_G_POWER: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisitePower.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisitePower[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				case STRUCT_G_FACTORY: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisiteFactory.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisiteFactory[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				case STRUCT_G_BARRACKS: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisiteBarracks.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisiteBarracks[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				case STRUCT_G_RADAR: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisiteRadar.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisiteRadar[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				case STRUCT_G_TECH: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisiteTech.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisiteTech[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				case STRUCT_G_GDIFACTORY: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisiteGDIFactory.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisiteGDIFactory[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				case STRUCT_G_NODFACTORY: {
					bool found = false;
					for (int j = 0; j < Rule->PrerequisiteNodFactory.Count(); j++) {
						if (ABQuantity.Value(Rule->PrerequisiteNodFactory[j]) > 0) {
							found = true;
							break;
						}
					}
					if (!found) return(0);
				}
				break;

				default: {
					BuildingTypeClass * btype = BuildingTypes[pre[i]];
					if (!btype->PowersUpBuilding.empty()) {
						BuildingClass * bptr;
						bool found = false;
						for (int j = Buildings.Count() - 1; j >= 0; j--) {
							bptr = Buildings[j];
							if (!bptr->IsInLimbo && bptr->House == this && bptr->IsOn) {
								if (bptr->Mission != MISSION_DECONSTRUCTION && bptr->MissionQueue != MISSION_DECONSTRUCTION) {
									found = true;
									break;
								}
							}
						}
						if (found) {
							for (int k = 0; k < ARRAY_SIZE(bptr->Upgrades); k++) {
								if (bptr->Upgrades[k] == btype) {
									goto breakout;
								}
							}
						}
						return(0);
					}
					if (ABQuantity.Value(pre[i]) == 0) return(0);
					breakout:;
				}
				break;
			}
		}

		if (type->RTTI == RTTI_BUILDINGTYPE) {

			/*
			**	Special hack to get certain objects to exist for both sides in the game.
			*/
			int own = type->Get_Ownable();

			/*
			**	Check to see if this owner can build the object type specified.
			*/
			if (own == 0) {
				return(0);
			}

			/// Checks if there's exactly one owner
			if (((own - 1) & own) == 0) {
				bool found = false;
				for (int i = 0; i < ConYards.Count(); i++) {
					BuildingClass * conyard = ConYards[i];
					if (!conyard->IsInLimbo && conyard->IsOn) {
						if (conyard->Mission != MISSION_DECONSTRUCTION && conyard->MissionQueue != MISSION_DECONSTRUCTION) {
							if (conyard->ActLike != HOUSE_NONE && ((1 << conyard->ActLike) & own) != 0) {
								found = true;
								break;
							}
						}
					}
				}
				if (!found) return(0);
			}
		}
	}

	switch ((RTTIType)type->RTTI) {
		case RTTI_UNITTYPE: {
			UnitTypeClass const * utype = (UnitTypeClass const *)type;
			if (utype->BuildLimit <= 0) {
				return(PUQuantity.Value(type->Fetch_Heap_ID()) < abs(utype->BuildLimit));
			}
			int count = UQuantity.Value(type->Fetch_Heap_ID());
			if (utype->DeploysInto != NULL) {
				count += BQuantity.Value(utype->DeploysInto->Fetch_Heap_ID());
			}
			if (count < utype->BuildLimit) {
				return(1);
			}
			if (include_in_progress) {
				for (int i = 0; i < Factories.Count(); i++) {
					FactoryClass * fptr = Factories[i];
					if (fptr->House == this && fptr->Get_Object() != NULL && fptr->Get_Object()->TClass == type) {
						if (Factories[i] != NULL) {
							return(1);
						}
						break;
					}
				}
			}
			return(-1);
		}
		break;

		case RTTI_INFANTRYTYPE: {
			InfantryTypeClass const * itype = (InfantryTypeClass const *)type;
			if (itype->BuildLimit <= 0) {
				return(PIQuantity.Value(type->Fetch_Heap_ID()) < abs(itype->BuildLimit));
			}
			int count = IQuantity.Value(type->Fetch_Heap_ID());
			if (itype->IsVehicleThief) {
				for (int i = 0; i < Units.Count(); i++) {
					UnitClass * unit = Units[i];
					if (unit->House == this && unit->EnteredByInfType == type->Fetch_Heap_ID()) {
						count++;
					}
				}
			}
			if (itype->BuildLimit <= 0 || count < itype->BuildLimit) {
				return(1);
			}
			if (include_in_progress && count == itype->BuildLimit) {
				for (int i = 0; i < Factories.Count(); i++) {
					FactoryClass * fptr = Factories[i];
					if (fptr->House == this && fptr->Get_Object() != NULL && fptr->Get_Object()->TClass == type) {
						if (Factories[i] != NULL) {
							return(1);
						}
						break;
					}
				}
			}
			return(-1);
		}
		break;

		case RTTI_BUILDINGTYPE: {
			BuildingTypeClass const * btype = (BuildingTypeClass const *)type;
			if (btype->BuildLimit <= 0) {
				return(PBQuantity.Value(type->Fetch_Heap_ID()) < abs(btype->BuildLimit));
			}
			int count = BQuantity.Value(type->Fetch_Heap_ID());
			if (count < btype->BuildLimit) {
				return(1);
			}
			if (include_in_progress) {
				for (int i = 0; i < Factories.Count(); i++) {
					FactoryClass * fptr = Factories[i];
					if (fptr->House == this && fptr->Get_Object() != NULL && fptr->Get_Object()->TClass == type) {
						if (Factories[i] != NULL) {
							return(1);
						}
						break;
					}
				}
			}
			return(-1);
		}
		break;

		case RTTI_AIRCRAFTTYPE: {
			AircraftTypeClass const * atype = (AircraftTypeClass const *)type;
			if (atype->BuildLimit <= 0) {
				return(PAQuantity.Value(type->Fetch_Heap_ID()) < abs(atype->BuildLimit));
			}
			int count = AQuantity.Value(type->Fetch_Heap_ID());
			if (count < atype->BuildLimit) {
				return(1);
			}
			if (include_in_progress) {
				for (int i = 0; i < Factories.Count(); i++) {
					FactoryClass * fptr = Factories[i];
					if (fptr->House == this && fptr->Get_Object() != NULL && fptr->Get_Object()->TClass == type) {
						if (Factories[i] != NULL) {
							return(1);
						}
						break;
					}
				}
			}
			return(-1);
		}
		break;
	}
	return(1);
}


/// <summary>
/// Finds the factory that is producing the object type specified.
/// This routine is used by the sidebar so that it can show the progress of an item that is
/// already under construction.
/// </summary>
/// <param name="object">The object type to search this house's factories for.</param>
/// <returns>Returns with a pointer to the factory building it. Otherwise, NULL is
/// returned.</returns>
FactoryClass *HouseClass::Factory_Producing_This(ObjectTypeClass const * object) const
{
	FactoryClass *fptr = NULL;

	for (int i = 0; i < Factories.Count(); i++) {
		if (Factories[i]->Get_House() == this && Factories[i]->Get_Object() != NULL) {
			if (Factories[i]->Get_Object()->TClass == object) {
				fptr = Factories[i];
				break;
			}
		}
	}
	return(fptr);
}


/***********************************************************************************************
 * HouseClass::AI -- Process house logic.                                                      *
 *                                                                                             *
 *    This handles the AI for the house object. It should be called once per house per game    *
 *    tick. It processes all house global tasks such as low power damage accumulation and      *
 *    house specific trigger events.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *   07/17/1995 JLB : Limits EVA speaking unless the player can do something.                  *
 *=============================================================================================*/
void HouseClass::AI(void)
{
	//assert(Houses.ID(this) == ID);

	if (RecalcPower) {
		Recalc_Power_Drain();
		RecalcRadar = true;
	}
	if (RecalcRadar) {
		Recalc_Radar_Availability();
	}

	if ((Frame % 100) == 0) {
		for (int a = 0; a < AngerNodes.Count(); a++) {
			if (AngerNodes[a].Level > 1) {
				AngerNodes[a].Level--;
			}
		}
	}

	/*
	**	If base building has been turned on by a trigger, then force the house to begin
	**	production and team creation as well. This is also true if the IQ is high enough to
	**	being base building.
	*/
	if (!Is_Human_Player() && (IsBaseBuilding || IQ >= Rule->IQProduction)) {
		IsBaseBuilding = true;
		IsStarted = true;
		IsAlerted = true;
	}

	/*
	**	Check to see if the house wins.
	*/
	if (IsToWin && BorrowedTime == 0 && (Session.Type != GAME_NORMAL || Blockage <= 0)) {
		CDTimerClass<SystemTimerClass> _timer = TIMER_SECOND * 2;
		while (Is_Speaking() && _timer) {
			Call_Back();
		}
		IsToWin = false;
		if (Is_Player_Control()) {
			PlayerWins = true;
		} else {
			PlayerLoses = true;
		}
	}

	/*
	**	Check to see if the house loses.
	*/
	if (/*Session.Type == GAME_NORMAL &&*/ IsToLose && BorrowedTime == 0) {
		CDTimerClass<SystemTimerClass> _timer = TIMER_SECOND * 2;
		while (Is_Speaking() && _timer) {
			Call_Back();
		}
		IsToLose = false;
		if (Is_Player_Control()) {
			PlayerLoses = true;
		} else {
			PlayerWins = true;
		}
	}

	/*
	**	Check to see if all objects of this house should be blown up.
	*/
	if (IsToDie && BorrowedTime == 0) {
		IsToDie = false;
		Blowup_All();
	}

	/*
	**	Double check power values to correct illegal conditions. It is possible to
	**	get a power output of negative (one usually) as a result of damage sustained
	**	and the fixed point fractional math involved with power adjustments. If the
	**	power rating drops below zero, then make it zero.
	*/
	Power = std::max(Power, 0);
	Drain = std::max(Drain, 0);

#if NEVER
	/*
	**	If the base has been alerted to the enemy and should be attacking, then
	**	see if the attack timer has expired. If it has, then create the attack
	**	teams.
	*/
	if (IsAlerted && AlertTime == 0) {

		/*
		**	Adjusted to reduce maximum number of teams created.
		*/
		int maxteams = Random_Pick(2, (int)(((Control.TechLevel-1)/3)+1));
		for (int index = 0; index < maxteams; index++) {
			TeamTypeClass const * ttype = Suggested_New_Team(true);
			if (ttype != NULL) {
				ScenarioInit++;
				ttype->Create_One_Of();
				ScenarioInit--;
			}
		}
		AlertTime = Rule->AutocreateTime * Random_Pick(TICKS_PER_MINUTE/2, TICKS_PER_MINUTE*2);
//		int mintime = Rule->AutocreateTime * (TICKS_PER_MINUTE/2);
//		int maxtime = Rule->AutocreateTime * (TICKS_PER_MINUTE*2);
//		AlertTime = Random_Pick(mintime, maxtime);
	}
#endif

	/*
	**	If this house's flag waypoint is a valid cell, see if there's
	**	someone sitting on it.  If so, make the scatter.  If they refuse,
	**	blow them up.
	*/
	if (FlagHome != CELL_NONE && (Frame % TICKS_PER_SECOND) == 0) {

		TechnoClass * techno = Map[FlagHome].Cell_Techno();
		if (techno != NULL) {
			bool moving = false;
			FootClass * foot = dynamic_cast<FootClass *>(techno);
			if (foot != NULL) {
				if (foot->NavCom != NULL) {
					moving = true;
				}
			}

			if (!moving) {
				techno->Scatter(COORD_NONE, true, true);
			}

			/*
			**	If the techno doesn't have a valid NavCom, he's not moving,
			**	so blow him up.
			*/
			if (foot != NULL) {
				if (foot->NavCom != NULL) {
					moving = true;
				}
			}

			/*
			**	If the techno wasn't an infantry or unit (ie he's a building),
			**	or he refuses to move, blow him up
			*/
			if (!moving) {
				int count = 0;
				while (!(techno->IsInLimbo) && count++ < 5) {
					int damage = techno->Strength;
					techno->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
				}
			}
		}
	}

	/*
	**	Create teams for this house if necessary.
	** (Use the same timer for some extra capture-the-flag logic.)
	*/
	if (/*!IsAlerted &&*/ !TeamTime) {

		SUGGESTED_TEAM_LIST team = Suggested_New_Team(false);
		if (team.Count() > 0) {
			for (int i = 0; i < team.Count(); i++) {
				team[i]->Create_One_Of(this);
			}
		}

		TeamTime = Rule->TeamDelays[Difficulty];
	}

	/*
	**	If there is insufficient power, then all buildings that are above
	**	half strength take a little bit of damage.
	*/
	if (DamageTime == 0) {

		/*
		**	When the power is below required, then the buildings will take damage over
		**	time.
		*/
		if (Power_Fraction() < 1) {
			for (int index = 0; index < Buildings.Count(); index++) {
				BuildingClass & b = *Buildings[index];

				if (b.House == this && b.HealthRatio > Rule->ConditionYellow) {
					// BG: Only damage buildings that require power, to keep the
					//     land mines from blowing up under low-power conditions
					if (b.Class->Drain) {
						int damage = 1;
						b.Take_Damage(damage, 0, Rule->C4Warhead, 0);
					}
				}
			}
		}
		DamageTime = int(TICKS_PER_MINUTE * Rule->DamageDelay);
	}

	/*
	**	If there are no more buildings to sell, then automatically cancel the
	**	sell mode.
	*/
	if (PlayerPtr == this && !CurBuildings && Map.IsSellMode) {
		Map.Sell_Mode_Control(0);
	}

	/*
	**	Various base conditions may be announced to the player. Typically, this would be
	**	low tiberium capacity or low power.
	*/
	if (PlayerPtr == this) {

		if (SpeakMaxedDelay == 0 && Available_Money() < 100 && UnitFactories+BuildingFactories+InfantryFactories > 0) {
			Speak(VOX_NO_CASH);
			Map.Flash_Money();
			SpeakMaxedDelay = Options.Normalize_Delay(int(TICKS_PER_MINUTE * Rule->SpeakDelay));
		}

		if (SpeakMaxedDelay == 0/* && IsMaxedOut*/) {
			//IsMaxedOut = false;
			if ((Capacity - Tiberium.Get_Total_Amount()) < 30/*0*/ && Capacity > 50/*0*/ /*&& (ActiveBScan & (STRUCTF_REFINERY | STRUCTF_CONST))*/) {
				Speak(VOX_NEED_MO_CAPACITY);
				SpeakMaxedDelay = Options.Normalize_Delay(int(TICKS_PER_MINUTE * Rule->SpeakDelay));
			}
		}
		if (SpeakPowerDelay == 0 && Power_Fraction() < 1) {
			if (ABQuantity.Value(Rule->BuildConst[0]->HeapID) > 0) {
				Speak(VOX_LOW_POWER);
				SpeakPowerDelay = Options.Normalize_Delay(int(TICKS_PER_MINUTE * Rule->SpeakDelay));
//				Map.Flash_Power();
				Session.Messages.Add_Message(NULL, 0, Fetch_String(TXT_LOW_POWER), Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), int(Rule->MessageDelay * TICKS_PER_MINUTE));
			}
		}
	}

	/*
	**	If there is a flag associated with this house, then mark it to be
	**	redrawn.
	*/
	if (FlagLocation != NULL) {
		UnitClass * unit = FlagLocation->As_UnitClass();
		if (unit) {
			unit->Mark(MARK_CHANGE);
		} else {
			//CELL cell = As_Cell(FlagLocation);
			//Map[cell].Redraw_Objects();
		}
	}

	bool is_time = false;

	/*
	**	Triggers are only checked every so often. If the trigger timer has expired,
	**	then set the trigger processing flag.
	*/
	if (TriggerTime == 0 || IsBuiltSomething) {
		is_time = true;
		TriggerTime = TICKS_PER_MINUTE/10;
		IsBuiltSomething = false;
	}

	/*
	**	Process any super weapon logic required.
	*/
	Super_Weapon_Handler();

	/*
	**	Special win/lose check for multiplayer games; by-passes the
	**	trigger system.  We must wait for non-zero frame, because init
	**	may not properly set IScan etc for each house; you have to go
	**	through each object's AI before it will be properly set.
	*/
	if (Session.Type != GAME_NORMAL && !IsDefeated && Frame > 0 && !Class->IsMultiplayPassive) {
		bool defeated = false;
		if (Session.Options.ShortGame) {
			if (!CurBuildings && !UQuantity.Value(Rule->BaseUnit->HeapID)) {
				defeated = true;
			}
		} else {
			if (!CurBuildings && !CurAircraft && !CurInfantry) {
				int units = CurUnits;
				if (units == 0) {
					defeated = true;
				}
				if (units && Scen->Special.IsHarvesterImmune) {
					units -= UQuantity.Value(Rule->HarvesterUnit[0]->HeapID);
				}
				if (units <= 0) {
					defeated = true;
				}
			}
		}

		if (defeated) {
			Blowup_All();
			MPlayer_Defeated();
		}
	}

	/*
	**	Try to spring all events attached to this house. The triggers will check
	**	for themselves if they actually need to be sprung or not.
	*/
	for (int index = 0; index < HouseTags.Count(); index++) {
		if (HouseTags[index]->Spring() && index > 0) {
			index--;
			continue;
		}
	}

#if NEVER
	/*
	**	If a radar facility is not present, but the radar is active, then turn the radar off.
	**	The radar also is turned off when the power gets below 100% capacity.
	*/
	if (PlayerPtr == this) {
		bool jammed = Map.Is_Radar_Active();

		/*
		**	Find if there are any radar facilities, and if they're jammed or not
		*/

		if (IsGPSActive) {
			jammed = false;
		} else {
			for (int index = 0; index < Buildings.Count(); index++) {
				BuildingClass * building = Buildings[index];
				if (building && building->House == PlayerPtr) {
					if (*building == STRUCT_RADAR /* || *building == STRUCT_EYE */) {
						if (!building->IsJammed) {
							jammed = false;
							break;
						}
					}
				}
			}
		}

		if (Map.Get_Jammed() != jammed) {
			Map.RadarClass::Flag_To_Redraw(true);
		}
		Map.Set_Jammed(jammed);
// Need to add in here where we activate it when only GPS is active.
		if (Map.Is_Radar_Active()) {
			if (ActiveBScan & STRUCTF_RADAR) {
				if (Power_Fraction() < 1 && !IsGPSActive) {
					Map.Radar_Activate(0);
				}
			} else {
				if (!IsGPSActive) {
					Map.Radar_Activate(0);
				}
			}

		} else {
			if (IsGPSActive || (ActiveBScan & STRUCTF_RADAR)) {
				if (Power_Fraction() >= 1 || IsGPSActive) {
					Map.Radar_Activate(1);
				}
			} else {
				if (Map.Is_Radar_Existing()) {
					Map.Radar_Activate(4);
				}
			}
		}
	}
#endif

	/*
	**	Perform any expert system AI processing.
	*/
	if (/*IsBaseBuilding &&*/ AITimer == 0 && !Is_Human_Player() && !Class->IsMultiplayPassive) {
		AITimer = Expert_AI();
	}

//	if (!IsBaseBuilding && State == STATE_ENDGAME) {
//		Fire_Sale();
//		Do_All_To_Hunt();
//	}

	if (!Is_Human_Player() && !Class->IsMultiplayPassive) {
		if (ProductionMode == EVERYTHING || Session.Type == GAME_NORMAL) {
			AI_Building();
			AI_Unit();
			AI_Infantry();
			AI_Aircraft();
		} else if (ProductionMode == BUILDINGS) {
			AI_Building();
			if (BuildStructure == STRUCT_NONE || !BuildingTypes[BuildStructure]->Who_Can_Build_Me(true, true, true, this)) {
				AI_Unit();
				AI_Infantry();
				AI_Aircraft();
			}
		} else if (ProductionMode == UNITS) {
			AI_Unit();
			if (BuildUnit != Rule->HarvesterUnit[0]->HeapID) {
				AI_Infantry();
				AI_Aircraft();
			}

			bool build_buildings = BuildUnit == UNIT_NONE && BuildInfantry == INFANTRY_NONE && BuildAircraft == AIRCRAFT_NONE;

			if (BuildUnit != UNIT_NONE && !UnitTypes[BuildUnit]->Who_Can_Build_Me(true, true, true, this)) {
				build_buildings = true;
			}
			if (BuildInfantry != INFANTRY_NONE && !InfantryTypes[BuildInfantry]->Who_Can_Build_Me(true, true, true, this)) {
				build_buildings = true;
			}
			if (BuildAircraft != AIRCRAFT_NONE && !AircraftTypes[BuildAircraft]->Who_Can_Build_Me(true, true, true, this)) {
				build_buildings = true;
			}

			if (build_buildings) {
				AI_Building();
			}
		}
	}

	/*
	**	If the production possibilities need to be recalculated, then do so now. This must
	**	occur after the scan bits have been properly updated.
	*/
	bool recalc_supers = false;
	if (PlayerPtr == this) {
		if (IsRecalcNeeded) {
			recalc_supers = true;
			IsRecalcNeeded = false;
			Map.Recalc();
			Map.SidebarClass::IsToRedraw = true;
			Map.IsToBlitSidebar = true;
			Map.Column[0].Flag_To_Redraw();
			Map.Column[1].Flag_To_Redraw();

			/*
			**	This placement might affect any prerequisite requirements for construction
			**	lists. Update the buildable options accordingly.
			*/
			for (int index = 0; index < Buildings.Count(); index++) {
				BuildingClass * building = Buildings[index];
				if (building && building->IsActive && building->Strength > 0 && building->House == this && building->Mission != MISSION_DECONSTRUCTION && building->MissionQueue != MISSION_DECONSTRUCTION) {

					if (PlayerPtr == building->House) {
						building->Update_Buildables();
					}
				}
			}
		}
	} else {
		if (IsRecalcNeeded) {
			IsRecalcNeeded = false;
			recalc_supers = true;
		}
	}

	if (recalc_supers) {
		Update_Present_Super_Weapons();
		Enable_Available_Super_Weapons();
	}

	/*
	**	See if it's time to re-set the can-repair flag
	*/
	if (DidRepair && RepairTimer == 0) {
		DidRepair = false;
	}

	if (this == PlayerPtr && IsToLook) {
		IsToLook = false;
		Map.All_To_Look();
	}
}


/***********************************************************************************************
 * HouseClass::Super_Weapon_Handler -- Handles the super weapon charge and discharge logic.    *
 *                                                                                             *
 *    This handles any super weapons assigned to this house. It also performs any necessary    *
 *    maintenance that the super weapons require.                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/17/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Super_Weapon_Handler(void)
{
	/*
	**	Perform all super weapon AI processing. This just checks to see if
	**	the graphic needs changing for the special weapon and updates the
	**	sidebar as necessary.
	*/
	for (SuperWeaponType special = SUPER_FIRST; special < SuperWeapon.Count(); special++) {
		SuperClass * super = SuperWeapon[special];

		if (super->Is_Present()) {

			/*
			**	Perform any charge-up logic for the super weapon. If the super
			**	weapon is owned by the player and a graphic change is detected, then
			**	flag the sidebar to be redrawn so the player will see the change.
			*/
			if (super->AI(this == PlayerPtr)) {
				if (this == PlayerPtr) Map.Column[1].Flag_To_Redraw();
			}

			if (super->Class->Type == SUPER_CHEM_MISSILE) {
				if (Weed.Get_Total_Amount() && ((double)Weed.Get_Total_Amount() / (double)Rule->WeedCapacity) == 1.0) {
					if (!super->Is_Ready()) {
						super->Recharge(this == PlayerPtr);
						Weed.Decrease_Amount(Weed.Get_Amount(0), 0);
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Attacked -- Lets player know if base is under attack.                           *
 *                                                                                             *
 *    Call this function whenever a building is attacked (with malice). This function will     *
 *    then announce to the player that his base is under attack. It checks to make sure that   *
 *    this is referring to the player's house rather than the enemy's.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/27/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Attacked(BuildingClass * source)
{
	if (source != NULL && source->Considered_Vehicle()) return;

	if (SpeakAttackDelay == 0) {
		if (Is_Player_Control()) {
			Submit_Radar_Event(RADAREVENT_BASE_ATTACKED, source->Center_Coord().As_Cell());
			Speak(VOX_BASE_UNDER_ATTACK);
			SpeakAttackDelay = Options.Normalize_Delay(int(TICKS_PER_MINUTE * Rule->SpeakDelay));
		} else if (Is_Ally(PlayerPtr) && (Session.Type == GAME_NORMAL || !source->House->Class->IsMultiplayPassive)) {
			Speak(VOX_ALLY_ATTACK);
			SpeakAttackDelay = Options.Normalize_Delay(int(TICKS_PER_MINUTE * Rule->SpeakDelay));
		}
	}

	/*
	**	If there is a trigger event associated with being attacked, process it
	**	now.
	*/
	for (int index = 0; index < HouseTags.Count(); index++) {
		HouseTags[index]->Spring(TEVENT_ATTACKED);
	}
}


/***********************************************************************************************
 * HouseClass::Harvested -- Adds Tiberium to the harvest storage.                              *
 *                                                                                             *
 *    Use this routine whenever Tiberium is harvested. The Tiberium is stored equally between  *
 *    all storage capable buildings for the house. Harvested Tiberium adds to the credit       *
 *    value of the house, but only up to the maximum storage capacity that the house can       *
 *    currently maintain.                                                                      *
 *                                                                                             *
 * INPUT:   tiberium -- The number of Tiberium credits to add to the House's total.            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/25/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Harvested(int tiberium, TiberiumType slot)
{
	PointTotal += tiberium * 5;

	if (Session.Type != GAME_NORMAL && !IsHuman) {
		Credits += tiberium * Tiberiums[slot]->CreditValue;
	} else {
		int oldcap = Capacity;
		int oldtib = Tiberium.Get_Total_Amount();

		if (tiberium + Tiberium.Get_Total_Amount() > Capacity) {
			tiberium = Capacity - Tiberium.Get_Total_Amount();
		}

		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * b = Buildings[index];
			if (b && b->IsDown && b->House == this) {
				if (b->Class->Capacity > 0) {
					while (tiberium > 0 && b->Class->Capacity > b->Storage.Get_Total_Amount()) {
						b->Storage.Increase_Amount(1, slot);
						Tiberium.Increase_Amount(1, slot);
						tiberium--;
					}
				}
			}
		}

		Silo_Redraw_Check(oldtib, oldcap);
	}
}


/// <summary>
/// Adds harvested weed to this house's storage.
/// This routine is called when a weed eater unloads. Anything that will not fit within the
/// house's weed capacity is simply thrown away.
/// </summary>
/// <param name="weed">The amount of weed harvested.</param>
/// <param name="slot">The storage slot to credit the weed to.</param>
void HouseClass::Harvested_Weed(int weed, int slot)
{
	int i = weed;
	while (i > 0) {
		if (Rule->WeedCapacity <= Weed.Get_Total_Amount()) {
			break;
		}
		Weed.Increase_Amount(1, slot);
		i--;
	}
}


/// <summary>
/// Calculates how full this house's weed storage is.
/// This routine is used when the weed storage gauge is drawn.
/// </summary>
/// <returns>Returns with the fraction of the weed storage in use, where 1.0 means
/// full.</returns>
double HouseClass::Weed_Fraction(void) const
{
	if (Weed.Get_Total_Amount() == 0) {
		return(0.f);
	}
	return((double)Weed.Get_Total_Amount() / Rule->WeedCapacity);
}


/***********************************************************************************************
 * HouseClass::Spend_Money -- Removes money from the house.                                    *
 *                                                                                             *
 *    Use this routine to extract money from the house. Typically, this is a result of         *
 *    production spending. The money is extracted from available cash reserves first. When     *
 *    cash reserves are exhausted, then Tiberium is consumed.                                  *
 *                                                                                             *
 * INPUT:   money -- The amount of money to spend.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/25/1995 JLB : Created.                                                                 *
 *   06/20/1995 JLB : Spends Tiberium before spending cash.                                    *
 *=============================================================================================*/
void HouseClass::Spend_Money(int money)
{
	int oldtib = Tiberium.Get_Total_Amount();
	int oldcapacity = Capacity;
	if (money > Credits) {
		int deficit = money - Credits;
		money = Credits;
		Credits = 0;
		if (deficit > 0 && Tiberium.Get_Total_Amount() > 0) {
			for (int index = 0; index < Buildings.Count(); index++) {
				BuildingClass * b = Buildings[index];
				if (b && b->House == this) {
					while (b->Storage.Get_Total_Amount() > 0 && deficit > 0) {
						int slot = b->Storage.First_Used_Slot();
						while (true) {
							if (b->Storage.Get_Amount(slot) <= 0) {
								break;
							}
							int amount = b->Storage.Decrease_Amount(1, slot);
							Tiberium.Decrease_Amount(amount, slot);
							int value = amount * Tiberiums[slot]->CreditValue;
							deficit -= value;
							money += value;
							if (deficit < 0) {
								Credits -= deficit;
								money += deficit;
								deficit = 0;
								break;
							}
							if (deficit <= 0) {
								break;
							}
						}
					}
				}
				if (!deficit) break;
			}
		}
	} else {
		Credits -= money;
	}
	Silo_Redraw_Check(oldtib, oldcapacity);
	CreditsSpent += money;
}


/***********************************************************************************************
 * HouseClass::Refund_Money -- Refunds money to back to the house.                             *
 *                                                                                             *
 *    Use this routine when money needs to be refunded back to the house. This can occur when  *
 *    construction is aborted. At this point, the exact breakdown of Tiberium or initial       *
 *    credits used for the orignal purchase is lost. Presume as much of the money is in the    *
 *    form of Tiberium as storage capacity will allow.                                         *
 *                                                                                             *
 * INPUT:   money -- The number of credits to refund back to the house.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/25/1995 JLB : Created.                                                                 *
 *   06/01/1995 JLB : Refunded money is never lost                                             *
 *=============================================================================================*/
void HouseClass::Refund_Money(int money)
{
	Credits += money;
}


/***********************************************************************************************
 * HouseClass::Silo_Redraw_Check -- Flags silos to be redrawn if necessary.                    *
 *                                                                                             *
 *    Call this routine when either the capacity or tiberium levels change for a house. This   *
 *    routine will determine if the aggregate tiberium storage level will result in the        *
 *    silos changing their imagery. If this is detected, then all the silos for this house     *
 *    are flagged to be redrawn.                                                               *
 *                                                                                             *
 * INPUT:   oldtib   -- Pre-change tiberium level.                                             *
 *                                                                                             *
 *          oldcap   -- Pre-change tiberium storage capacity.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Silo_Redraw_Check(int oldtib, int oldcap)
{
	int oldratio = 0;
	if (oldcap) oldratio = ((double)oldtib / oldcap * 4.0) + 0.5;
	int newratio = 0;
	if (Capacity) newratio = ((double)Tiberium.Get_Total_Amount() / Capacity * 4.0)  + 0.5;

	if (oldratio != newratio) {
		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * b = Buildings[index];
			if (b && !b->IsInLimbo && b->House == this && b->Class->IsSiloDamage) {
				b->Mark(MARK_CHANGE);
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Is_Ally -- Determines if the specified house is an ally.                        *
 *                                                                                             *
 *    This routine will determine if the house number specified is a ally to this house.       *
 *                                                                                             *
 * INPUT:   house -- The house number to check to see if it is an ally.                        *
 *                                                                                             *
 * OUTPUT:  Is the house an ally?                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Is_Ally(HousesType house) const
{
	if (house == HeapID) return(true);
	if (house != HOUSE_NONE) {
		return(((1<<house) & Allies) != 0);
	}
	return(false);
}


/***********************************************************************************************
 * HouseClass::Is_Ally -- Determines if the specified house is an ally.                        *
 *                                                                                             *
 *    This routine will examine the specified house and determine if it is an ally.            *
 *                                                                                             *
 * INPUT:   house -- Pointer to the house object to check for ally relationship.               *
 *                                                                                             *
 * OUTPUT:  Is the specified house an ally?                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Is_Ally(HouseClass const * house) const
{
	if (house) {
		if (house == this) {
			return(true);
		}
		return(Is_Ally(house->HeapID));
	}

	return(false);
}


/***********************************************************************************************
 * HouseClass::Is_Ally -- Checks to see if the object is an ally.                              *
 *                                                                                             *
 *    This routine will examine the specified object and return whether it is an ally or not.  *
 *                                                                                             *
 * INPUT:   object   -- The object to examine to see if it is an ally.                         *
 *                                                                                             *
 * OUTPUT:  Is the specified object an ally?                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Is_Ally(ObjectClass const * object) const
{
	if (object) {
		return(Is_Ally(object->Owner_HouseClass()));
	}
	return(false);
}


/// <summary>
/// Is the target specified an ally of this house?
/// This overload is the catch-all for targets whose kind is not known at the call site.
/// Anything that is not an object cannot be allied with, and so is treated as hostile.
/// </summary>
/// <returns>bool; Is the target friendly toward this house?</returns>
bool HouseClass::Is_Ally(AbstractClass const * target) const
{
	ObjectClass const * object = dynamic_cast<ObjectClass const *>(target);
	if (object) {
		return(Is_Ally(object));
	}

	return(false);
}


/// <summary>
/// Does this house see the other as its owner does? True for an ally, and either way round
/// once the local player has the whole map. Display only; never a simulation rule.
/// </summary>
bool HouseClass::Shares_View_With(HouseClass const * house) const
{
	return(Is_Ally(house) || (Session.ObiWan && (this == PlayerPtr || house == PlayerPtr)));
}


/***********************************************************************************************
 * HouseClass::Make_Ally -- Make the specified house an ally.                                  *
 *                                                                                             *
 *    This routine will make the specified house an ally to this house. An allied house will   *
 *    not be considered a threat or potential target.                                          *
 *                                                                                             *
 * INPUT:   house -- The house to make an ally of this house.                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   08/08/1995 JLB : Breaks off combat when ally commences.                                   *
 *   10/17/1995 JLB : Added reveal base when allied.                                           *
 *=============================================================================================*/
void HouseClass::Make_Ally(HousesType house)
{
	Make_Ally(Houses[house]);
}


/// <summary>
/// Makes an ally of the house specified.
/// An allied house is no longer a threat or a legal target, so any fighting already under way
/// between the two is broken off as the alliance forms. Allying with the local player may
/// also lay this house's structures bare to them.
/// </summary>
/// <param name="house">The house to make an ally of this house.</param>
void HouseClass::Make_Ally(HouseClass * house)
{
	if (Is_Allowed_To_Ally(house)) {

		Allies |= (1L << house->HeapID);

		Recalc_Threat_Regions();
		Clear_Anger(house);

		/*
		**	Don't consider the newfound ally to be an enemy -- of course.
		*/
		if (Enemy == house->HeapID) {
			Enemy = HOUSE_NONE;
		}

		if (ScenarioInit) {
			Control.Allies |= (1L << house->HeapID);
		}

		if (!ScenarioInit) {

			/*
			**	An alliance with another human player will cause the computer
			**	players (if present) to become paranoid.
			*/
			if (Is_Human_Player() && Rule->IsComputerParanoid && !house->Class->IsMultiplayPassive) {
				Computer_Paranoid();
			}

			char buffer[80];

			/*
			**	Sweep through all techno objects and perform a cheeseball tarcom clear to ensure
			**	that fighting will most likely stop when the cease fire begins.
			*/
			for (int index = 0; index < Logic.Count(); index++) {
				ObjectClass * object = Logic[index];

				if (object != NULL && Dynamic_Cast<TechnoClass *>(object) && !object->IsInLimbo && object->Owner() == HeapID) {
					TargetClass target = ((TechnoClass *)object)->TarCom;
					if (target.Is_Valid() && target.As_Techno() != NULL) {
						if (Is_Ally(target.As_Techno())) {
							((TechnoClass *)object)->Assign_Target(NULL);
						}
					}
				}
			}

			/*
			**	Cause all structures to be revealed to the house that has been
			**	allied with.
			*/
			if (Rule->IsAllyReveal && house == PlayerPtr) {
				for (int index = 0; index < Technos.Count(); index++) {
					TechnoClass * t = Technos[index];

					if (!t->IsInLimbo && t->House == this) {
						t->Look();
					}
				}
			}

			if (Is_Human_Player() && Session.Type != GAME_NORMAL && !house->Class->IsMultiplayPassive) {
				wsprintf(buffer, Fetch_String(TXT_HAS_ALLIED), (char const *)IniName, (char const *)house->IniName);
				Session.Messages.Add_Message(NULL, 0, buffer, Class->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), int(TICKS_PER_MINUTE * Rule->MessageDelay));

				if (Is_Player_Control()) {
					Speak(VOX_ALLIANCE_FORMED);
				}
			}

			Map.Flag_To_Redraw();
		}
	}
}


/***********************************************************************************************
 * HouseClass::Make_Enemy -- Make an enemy of the house specified.                             *
 *                                                                                             *
 *    This routine will flag the house specified so that it will be an enemy to this house.    *
 *    Enemy houses are legal targets for attack.                                               *
 *                                                                                             *
 * INPUT:   house -- The house to make an enemy of this house.                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   07/27/1995 JLB : Making war is a bilateral action.                                        *
 *=============================================================================================*/
void HouseClass::Make_Enemy(HousesType house)
{
	Make_Enemy(Houses[house]);
}


/// <summary>
/// Makes an enemy of the house specified.
/// This routine breaks any alliance between the two houses and stokes this house's anger
/// toward the other. Breaking an alliance is a bilateral event, so the other house is drawn
/// out of it as well.
/// </summary>
/// <param name="house">The house to make an enemy of this house.</param>
void HouseClass::Make_Enemy(HouseClass * house)
{
	if (Session.Type == GAME_NORMAL || (!house->Class->IsMultiplayPassive && !Class->IsMultiplayPassive)) {
		Add_Anger(1, house);

		if (house != NULL && Is_Ally(house)) {
			Allies &= ~(1L << house->HeapID);

			if (ScenarioInit) {
				Control.Allies &= ~(1L << house->HeapID);
			}

			Recalc_Threat_Regions();

			/*
			**	Breaking an alliance is a bilateral event.
			*/
			if (house->Is_Ally(this)) {
				house->Allies &= ~(1L << HeapID);

				if (ScenarioInit) {
					house->Control.Allies &= ~(1L << HeapID);
				}
				house->Recalc_Threat_Regions();
				house->Add_Anger(1, house);
			}

			if (Session.Type != GAME_NORMAL && !ScenarioInit && IsHuman) {
				char buffer[80];

				wsprintf(buffer, Fetch_String(TXT_AT_WAR), (char const *)IniName, (char const *)house->IniName);
				Session.Messages.Add_Message(NULL, 0, buffer, Class->Scheme, TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), int(TICKS_PER_MINUTE * Rule->MessageDelay));
				Map.Flag_To_Redraw();
				if (Is_Player_Control()) {
					Speak(VOX_ALLIANCE_BROKEN);
				}
			}
		}
	}
}


/// <summary>
/// Gives the local player the whole map for the rest of the match: shroud and fog are lifted
/// everywhere, the radar stays up, and messages can only go to everyone.
/// </summary>
void HouseClass::Become_ObiWan(void)
{
	Session.ObiWan = 1;
	Map.Reveal_The_Map(true);
	HiddenSurface->Fill(TBLACK);
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	RecalcRadar = true;
}


/***********************************************************************************************
 * HouseClass::Suggested_New_Team -- Determine what team should be created.                    *
 *                                                                                             *
 *    This routine examines the house condition and returns with the team that it thinks       *
 *    should be created. The units that are not currently a member of a team are examined      *
 *    to determine the team needed.                                                            *
 *                                                                                             *
 * INPUT:   alertcheck  -- Select from the auto-create team list.                              *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the team type that should be created. If no team should  *
 *          be created, then NULL is returned.                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
SUGGESTED_TEAM_LIST HouseClass::Suggested_New_Team(bool alertcheck)
{
	SUGGESTED_TEAM_LIST teams = TeamTypeClass::Suggested_New_Team(this, alertcheck);
	return(teams);
}


/***********************************************************************************************
 * HouseClass::Adjust_Threat -- Adjust threat for the region specified.                        *
 *                                                                                             *
 *    This routine is called when the threat rating for a region needs to change. The region   *
 *    and threat adjustment are provided.                                                      *
 *                                                                                             *
 * INPUT:   region   -- The region that adjustment is to occur on.                             *
 *                                                                                             *
 *          threat   -- The threat adjustment to perform.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Adjust_Threat(int region, int threat)
{
	static int _val[] = {
		-MAP_REGION_WIDTH - 1,	-MAP_REGION_WIDTH, -MAP_REGION_WIDTH + 1,
		-1,							0,						 1,
		MAP_REGION_WIDTH -1,		MAP_REGION_WIDTH,	 MAP_REGION_WIDTH +1
	};
	static int _thr[] = {
		2, 1, 2,
		1, 0, 1,
		2, 1,	2
	};
	int neg;
	int * val = &_val[0];
	int * thr = &_thr[0];

	if (threat < 0) {
		threat = -threat;
		neg = true;
	} else {
		neg = false;
	}

	for (int lp = 0; lp < 9; lp ++) {
		Regions[region + *val].Adjust_Threat(threat >> *thr, neg);
		val++;
		thr++;
	}
}


/***********************************************************************************************
 * HouseClass::Begin_Production -- Starts production of the specified object type.             *
 *                                                                                             *
 *    This routine is called from the event system. It will start production for the object    *
 *    type specified. This will be reflected in the sidebar as well as the house factory       *
 *    tracking variables.                                                                      *
 *                                                                                             *
 * INPUT:   type  -- The type of object to begin production on.                                *
 *                                                                                             *
 *          id    -- The subtype of object.                                                    *
 *                                                                                             *
 * OUTPUT:  Returns with the reason why, or why not, production was started.                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *   10/21/1996 JLB : Handles max object case.                                                 *
 *=============================================================================================*/
ProdFailType HouseClass::Begin_Production(RTTIType type, int id, bool resume)
{
	int result = true;
	FactoryClass * fptr;
	TechnoTypeClass const * tech = Fetch_Techno_Type(type, id);
	if (tech == NULL) {
		DebugString("Request to Begin_Production of type %d, ID %d was rejected. No such object type.\n", type, id);
		return(PROD_CANT);
	}

	BuildingClass *who = tech->Who_Can_Build_Me(false, true, true, this);
	bool onhold = false;

	if (who == NULL) {
		if (resume) {
			who = tech->Who_Can_Build_Me(true, false, true, this);
		}
		if (who != NULL) {
			onhold = true;
		} else {
			DebugString("Request to Begin_Production of '%s' was rejected. No-one can build.\n", (const char *)tech->GivenName);
			return(PROD_CANT);
		}
	}

	fptr = Fetch_Factory(type);

	if (fptr == NULL) {
		fptr = new FactoryClass;

		if (fptr == NULL) {
			DebugString("Request to Begin_Production of '%s' was rejected. Unable to create factory\n", (const char *)tech->GivenName);
			return(PROD_CANT);
		}
	}

	/*
	**	If the house is already busy producing the requested object, then
	**	return with this failure code, unless we are restarting production.
	*/
	if (fptr != NULL) {
		if (fptr->Is_Building() && type == RTTI_BUILDINGTYPE) {
			DebugString("Request to Begin_Production of '%s' was rejected. Cannot queue buildings.\n", (const char *)tech->GivenName);
			return(PROD_CANT);
		}
	}

	Set_Factory(type, fptr);

	bool has_suspended = false;
	if (fptr->IsSuspended) {
		TechnoClass *object = fptr->Object;
		if (object != NULL) {
			if (object->TClass == tech) {
				has_suspended = true;
			}
		}
	}

	if (!has_suspended) {
		result = fptr->Set(*tech, *this, resume);
	}

	if (result) {
		if (fptr->QueuedObjects.Count() && !resume && !has_suspended) {
			Map.Column[1].Flag_To_Redraw();
		} else {
			fptr->Start(onhold);

			/*
			**	Link this factory to the sidebar so that proper graphic feedback
			**	can take place.
			*/
			if (PlayerPtr == this) {
				Map.Factory_Link(fptr, type, id);
			}
		}

		return(PROD_OK);
	}

	DebugString("Request to Begin_Production of '%s' was rejected. Factory was unable to create the requested object\n", (char const *)tech->GivenName);

	if (fptr->QueuedObjects.Count() == 0 && fptr->Object == NULL) {
		DebugString("type=%d\n", type);
		DebugString("Frame == %d\n", Frame);
		DebugString("fptr->QueuedObjects.Count() == %d\n", fptr->QueuedObjects.Count());
		DebugString("Object->RTTI == %d\n", fptr->Object != NULL ? fptr->Object->Fetch_RTTI() : -1);
		DebugString("Object->HeapID == %d\n", fptr->Object != NULL ? fptr->Object->Fetch_Heap_ID() : -1);
		DebugString("IsSuspended\t= %d\n", fptr->IsSuspended);

		delete fptr;
		Set_Factory(type, NULL);
	}

	return(PROD_CANT);
}


/***********************************************************************************************
 * HouseClass::Suspend_Production -- Temporarily puts production on hold.                      *
 *                                                                                             *
 *    This routine is called from the event system whenever the production of the specified    *
 *    type needs to be suspended. The suspended production will be reflected in the sidebar    *
 *    as well as in the house control structure.                                               *
 *                                                                                             *
 * INPUT:   type  -- The type of object that production is being suspended for.                *
 *                                                                                             *
 * OUTPUT:  Returns why, or why not, production was suspended.                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ProdFailType HouseClass::Suspend_Production(RTTIType type)
{
	FactoryClass * fptr = Fetch_Factory(type);

	/*
	**	If the house is already busy producing the requested object, then
	**	return with this failure code.
	*/
	if (fptr == NULL) return(PROD_CANT);

	/*
	**	Actually suspend the production.
	*/
	fptr->Suspend();

	/*
	**	Tell the sidebar that it needs to be redrawn because of this.
	*/
	if (PlayerPtr == this) {
		Map.SidebarClass::IsToRedraw = true;
		Map.IsToBlitSidebar = true;
		Map.Flag_To_Redraw();
		Map.Column[0].Flag_To_Redraw();
		Map.Column[1].Flag_To_Redraw();
	}

	return(PROD_OK);
}


/***********************************************************************************************
 * HouseClass::Abandon_Production -- Abandons production of item type specified.               *
 *                                                                                             *
 *    This routine is called from the event system whenever production must be abandoned for   *
 *    the type specified. This will remove the factory and pending object from the sidebar as  *
 *    well as from the house factory record.                                                   *
 *                                                                                             *
 * INPUT:   type  -- The object type that production is being suspended for.                   *
 *                                                                                             *
 * OUTPUT:  Returns the reason why or why not, production was suspended.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ProdFailType HouseClass::Abandon_Production(RTTIType type, int id)
{
	FactoryClass * fptr = Fetch_Factory(type);

	/*
	**	If there is no factory to abandon, then return with a failure code.
	*/
	if (fptr == NULL) {
		return(PROD_CANT);
	}

	if (fptr->QueuedObjects.Count() > 0 && id >= 0) {
		TechnoTypeClass const * tech = Fetch_Techno_Type(type, id);
		if (fptr->Remove_From_Queue(tech)) {
			Map.Column[1].Flag_To_Redraw();
			return(PROD_OK);
		}
	}

	bool abandon = false;
	if (id == -1) {
		abandon = true;
	}
	if (!abandon) {
		TechnoClass *obj = fptr->Object;
		if (obj != NULL && id == obj->Class_Of()->Fetch_Heap_ID()) {
			abandon = true;
		}
	}

	if (!abandon) {
		return(PROD_OK);
	}

	/*
	**	Tell the sidebar that it needs to be redrawn because of this.
	*/
	if (PlayerPtr == this) {
		Map.Abandon_Production(type, fptr);

		if (type == RTTI_BUILDINGTYPE || type == RTTI_BUILDING) {
			Map.PendingObjectPtr = 0;
			Map.PendingObject = 0;
			Map.PendingHouse = HOUSE_NONE;
			Map.Set_Cursor_Shape(0);
		}
	}

	/*
	**	Abandon production of the object.
	*/
	fptr->Abandon();
	if (fptr->QueuedObjects.Count() == 0) {
		Set_Factory(type, NULL);
		delete fptr;
	} else {
		fptr->Resume_Queue();
	}

	return(PROD_OK);
}


/***********************************************************************************************
 * HouseClass::Special_Weapon_AI -- Fires special weapon.                                      *
 *                                                                                             *
 *    This routine will pick a good target to fire the special weapon specified.               *
 *                                                                                             *
 * INPUT:   id -- The special weapon id to fire.                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Special_Weapon_AI(SuperWeaponType id)
{
	/*
	**	Loop through all of the building objects on the map
	**	and see which ones are available.
	*/
	BuildingClass * bestptr = NULL;
	int best = -1;

	for (int index = 0; index < Buildings.Count(); index++) {
		BuildingClass * b = Buildings[index];

		/*
		**	If the building is valid, not in limbo, not in the process of
		**	being destroyed and not our ally, then we can consider it.
		*/
		if (b != NULL && !b->IsInLimbo && b->Strength && !Is_Ally(b)) {
			if (Percent_Chance(90) && (b->Value() > best || best == -1)) {
				best = b->Value();
				bestptr = b;
			}
		}
	}

	if (bestptr) {
		Cell cell = bestptr->Center_Coord().As_Cell();
		Place_Special_Blast(id, cell);
	}
}


/***********************************************************************************************
 * HouseClass::Place_Special_Blast -- Place a special blast effect at location specified.      *
 *                                                                                             *
 *    This routine will create a blast effect at the cell specified. This is the result of     *
 *    the special weapons.                                                                     *
 *                                                                                             *
 * INPUT:   id    -- The special weapon id number.                                             *
 *                                                                                             *
 *          cell  -- The location where the special weapon attack is to occur.                 *
 *                                                                                             *
 * OUTPUT:  Was the special weapon successfully fired at the location specified?               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : commented.                                                               *
 *   07/25/1995 JLB : Added scatter effect for nuclear bomb.                                   *
 *   07/28/1995 JLB : Revamped to use super weapon class controller.                           *
 *=============================================================================================*/
bool HouseClass::Place_Special_Blast(SuperWeaponType id, Cell const & cell)
{
	SuperWeapon[id]->Discharged(this == PlayerPtr, cell);

	return(true);
}


/***********************************************************************************************
 * HouseClass::Place_Object -- Places the object (building) at location specified.             *
 *                                                                                             *
 *    This routine is called when a building has been produced and now must be placed on       *
 *    the map. When the player clicks on the map, this routine is ultimately called when the   *
 *    event passes through the event queue system.                                             *
 *                                                                                             *
 * INPUT:   type  -- The object type to place. The actual object is lifted from the sidebar.   *
 *                                                                                             *
 *                                                                                             *
 *          cell  -- The location to place the object on the map.                              *
 *                                                                                             *
 * OUTPUT:  Was the placement successful?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Place_Object(RTTIType type, Cell const & cell)
{
	bool placed = false;
	TechnoClass * tech = 0;
	FactoryClass * factory = Fetch_Factory(type);

	/*
	**	Only if there is a factory active for this type, can it be "placed".
	**	In the case of a missing factory, then this request is completely bogus --
	**	ignore it. This might occur if, between two events to exit the same
	**	object, the mouse was clicked on the sidebar to start building again.
	**	The second placement event should NOT try to place the object that is
	**	just starting construction.
	*/
	if (factory && factory->Has_Completed()) {
		tech = factory->Get_Object();

		if (cell == CELL_NONE || cell == Cell(-1, -1)) {
			if (tech != NULL) {

				/*
				**	Try to find a place for the object to appear from. For helicopters, it has the
				**	option of finding a nearby helipad if no helipads are free.
				*/
				TechnoClass * builder = tech->Who_Can_Build_Me(false, true);
				if (builder == NULL && tech->RTTI == RTTI_AIRCRAFT) {
					builder = tech->Who_Can_Build_Me(true, true);

				}

				if (builder != NULL) {
					int exit = builder->Exit_Object(tech);
					if (exit == 2 || (exit == 1 && builder->RTTI == RTTI_BUILDING && ((BuildingClass *)builder)->Factory != NULL)) {

						/*
						**	Since the object has left the factory under its own power, delete
						**	the production manager tied to this slot in the sidebar. Its job
						**	has been completed.
						*/
						LastRadarEventCell = builder->Center_Coord().As_Cell();
						factory->Completed();
						Abandon_Production(type, -1);
						placed = true;
					} else {
						/*
						**	The object could not leave under it's own power. Just wait
						**	until the player tries to place the object again.
						*/
						if (tech->RTTI != RTTI_BUILDING) {
							DebugString("Failed to exit object from factory - refunding money\n");
							Abandon_Production(type, -1);
						}
						return(placed);
					}
				} else {
					return(placed);
				}
			}

		} else {
			if (tech) {
				TechnoClass * builder = tech->Who_Can_Build_Me(false, false);
				if (builder) {

					builder->Transmit_Message(RADIO_HELLO, tech);
					if (tech->Unlimbo((Cell)cell)) {
						if (tech->RTTI == RTTI_BUILDING) {
							if (((BuildingClass *)tech)->Class->IsFirestormWall) {
								Map.Place_Firestorm_Wall(cell, this, ((BuildingClass *)tech)->Class);
							} else if (((BuildingClass *)tech)->Class->ToOverlay != NULL && ((BuildingClass *)tech)->Class->ToOverlay->IsWall) {
								Map.Place_Wall(cell, this, ((BuildingClass *)tech)->Class);
							}
						}
						factory->Completed();
						tech->Transmit_Message(RADIO_COMPLETE, builder);
						Abandon_Production(type, -1);
						placed = true;

						if (PlayerPtr == this) {
							if (tech->IsActive && !tech->IsDiscoveredByPlayer) {
								tech->Revealed(this);
							}
							Sound_Effect(Rule->BuildingSlam);
							Map.Set_Cursor_Shape(0);
							Map.PendingObjectPtr = 0;
							Map.PendingObject = 0;
							Map.PendingHouse = HOUSE_NONE;
						}
					} else {
						placed = false;
						if (this == PlayerPtr) {
							Speak(VOX_DEPLOY);
						}
					}
					builder->Transmit_Message(RADIO_OVER_OUT);
				}
			} else {

				// Play a bad sound here?
				return(placed);
			}
		}

		if (placed) Just_Built(tech);
	}

	return(placed);
}


/// <summary>
/// Records that this house has just finished building an object.
/// This routine is called by the production code. The computer consults the record when it
/// decides what to build next, and the produced quantity trackers use it to keep a tally of
/// everything this house has turned out.
/// </summary>
/// <param name="product">The object that just rolled off the production line.</param>
void HouseClass::Just_Built(TechnoClass * product)
{
	IsBuiltSomething = true;

	TechnoTypeClass const * ttype = product->TClass;

	switch (product->Fetch_RTTI()) {
		case RTTI_UNIT:
			JustBuiltUnit = (UnitType)ttype->Fetch_Heap_ID();
			PUQuantity.Increment(ttype->Fetch_Heap_ID());
			break;

		case RTTI_INFANTRY:
			JustBuiltInfantry = (InfantryType)ttype->Fetch_Heap_ID();
			PIQuantity.Increment(ttype->Fetch_Heap_ID());
			break;

		case RTTI_BUILDING:
			JustBuiltStructure = (StructType)ttype->Fetch_Heap_ID();
			PBQuantity.Increment(ttype->Fetch_Heap_ID());
			break;

		case RTTI_AIRCRAFT:
			JustBuiltAircraft = (AircraftType)ttype->Fetch_Heap_ID();
			PAQuantity.Increment(ttype->Fetch_Heap_ID());
			break;
	}
	IsRecalcNeeded = true;
}


/***********************************************************************************************
 * HouseClass::Manual_Place -- Inform display system of building placement mode.               *
 *                                                                                             *
 *    This routine will inform the display system that building placement mode has begun.      *
 *    The cursor will be created that matches the layout of the building shape.                *
 *                                                                                             *
 * INPUT:   builder  -- The factory that is building this object.                              *
 *                                                                                             *
 *          object   -- The building that is going to be placed down on the map.               *
 *                                                                                             *
 * OUTPUT:  Was the building placement mode successfully initiated?                            *
 *                                                                                             *
 * WARNINGS:   This merely adjusts the cursor shape. Nothing that affects networked games      *
 *             is affected.                                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/04/1995 JLB : Created.                                                                 *
 *   05/30/1995 JLB : Uses the Bib_And_Offset() function to determine bib size.                *
 *=============================================================================================*/
bool HouseClass::Manual_Place(BuildingClass * builder,  BuildingClass * object)
{
	if (this == PlayerPtr && !Map.PendingObject && builder && object) {
		/*
		**	Ensures that object selection doesn't remain when
		**	building placement takes place.
		*/
		Unselect_All();

		Map.Repair_Mode_Control(0);
		Map.Sell_Mode_Control(0);

		Map.PendingObject = object->Class;
		Map.PendingObjectPtr = object;
		Map.PendingHouse = HeapID;

		Map.Set_Cursor_Shape(object->Occupy_List(true));
		Map.Set_Cursor_Pos(builder->Get_Cell());
		builder->Mark(MARK_CHANGE);
		return(true);
	}
	return(false);
}


/***************************************************************************
 * HouseClass::Clobber_All -- removes all objects for this house           *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      This routine removes the house itself, so the multiplayer code     *
 *        must not rely on there being "empty" houses lying around.        *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1995 BRR : Created.                                             *
 *   06/09/1995 JLB : Handles aircraft.                                    *
 *=========================================================================*/
void HouseClass::Clobber_All(void)
{
	int i;

	for (i = 0; i < Technos.Count(); i++) {
		if (Technos[i]->House == this) {
			delete Technos[i];
			i--;
		}
	}
	for (i = 0; i < Triggers.Count(); i++) {
		if (Triggers[i]->Class->House == Class) {
			delete Triggers[i];
			i--;
		}
	}

	delete this;
}


/***********************************************************************************************
 * HouseClass::Detach -- Removes specified object from house tracking systems.                 *
 *                                                                                             *
 *    This routine is called when an object is to be removed from the game system. If the      *
 *    specified object is part of the house tracking system, then it will be removed.          *
 *                                                                                             *
 * INPUT:   target   -- The target value of the object that is to be removed from the game.    *
 *                                                                                             *
 *          all      -- Is the target going away for good as opposed to just cloaking/hiding?  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : commented                                                                *
 *=============================================================================================*/
void HouseClass::Detach(AbstractClass const * target, bool all)
{
	if (ToCapture == target) {
		ToCapture = NULL;
	}

	TagClass * tag = ((AbstractClass *)target)->As_TagClass();
	if (tag != NULL) {
		HouseTags.Delete(tag);
	}

	if (target->RTTI == RTTI_BUILDING) {
		if (all) {
			ConYards.Delete(((BuildingClass *)target));
		}
	}

	for (int i = 0; i < ARRAY_SIZE(Paths); i++) {
		if (Paths[i] == target) {
			Paths[i] = NULL;
		}
	}

	if (AircraftFactory == target) {
		AircraftFactory = NULL;
	}

	if (InfantryFactory == target) {
		InfantryFactory = NULL;
	}

	if (UnitFactory == target) {
		UnitFactory = NULL;
	}

	if (BuildingFactory == target) {
		BuildingFactory = NULL;
	}
}


/***********************************************************************************************
 * HouseClass::Does_Enemy_Building_Exist -- Checks for enemy building of specified type.       *
 *                                                                                             *
 *    This routine will examine the enemy houses and if there is a building owned by one       *
 *    of those house, true will be returned.                                                   *
 *                                                                                             *
 * INPUT:   btype -- The building type to check for.                                           *
 *                                                                                             *
 * OUTPUT:  Does a building of the specified type exist for one of the enemy houses?           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Does_Enemy_Building_Exist(StructType btype) const
{
	for (HousesType index = HOUSE_FIRST; index < Houses.Count(); index++) {
		HouseClass * house = Houses[index];

		if (house && !Is_Ally(house) && house->ABQuantity.Value(btype) > 0) {
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * HouseClass::Suggest_New_Object -- Determine what would the next buildable object be.        *
 *                                                                                             *
 *    This routine will examine the house status and return with a techno type pointer to      *
 *    the object type that it thinks should be created. The type is restricted to match the    *
 *    type specified. Typical use of this routine is by computer controlled factories.         *
 *                                                                                             *
 * INPUT:   objecttype  -- The type of object to restrict the scan for.                        *
 *                                                                                             *
 *          kennel      -- Is this from a kennel? There are special hacks to ensure that only  *
 *                         dogs can be produced from a kennel.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to a techno type for the object type that should be         *
 *          created. If no object should be created, then NULL is returned.                    *
 *                                                                                             *
 * WARNINGS:   This is a time consuming routine. Only call when necessary.                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
TechnoTypeClass const * HouseClass::Suggest_New_Object(RTTIType objecttype, bool kennel) const
{
	TechnoTypeClass const * techno = NULL;

	switch (objecttype) {
		case RTTI_AIRCRAFT:
		case RTTI_AIRCRAFTTYPE:
			if (BuildAircraft != AIRCRAFT_NONE) {
				return(AircraftTypes[BuildAircraft]);
			}
			return(NULL);

		/*
		**	Unit construction is based on the rule that up to twice the number required
		**	to fill all teams will be created.
		*/
		case RTTI_UNIT:
		case RTTI_UNITTYPE:
			if (BuildUnit != UNIT_NONE) {
				return(UnitTypes[BuildUnit]);
			}
			return(NULL);

		/*
		**	Infantry construction is based on the rule that up to twice the number required
		**	to fill all teams will be created.
		*/
		case RTTI_INFANTRY:
		case RTTI_INFANTRYTYPE:
			if (BuildInfantry != INFANTRY_NONE) {
				return(InfantryTypes[BuildInfantry]);
			}
			return(NULL);

		/*
		**	Building construction is based upon the preconstruction list.
		*/
		case RTTI_BUILDING:
		case RTTI_BUILDINGTYPE:
			if (BuildStructure != STRUCT_NONE) {
				return(BuildingTypes[BuildStructure]);
			}
			return(NULL);
	}
	return(techno);
}


/***********************************************************************************************
 * HouseClass::Flag_Remove -- Removes the flag from the specified target.                      *
 *                                                                                             *
 *    This routine will remove the flag attached to the specified target object or cell.       *
 *    Call this routine before placing the object down. This is called inherently by the       *
 *    the Flag_Attach() functions.                                                             *
 *                                                                                             *
 * INPUT:   target   -- The target that the flag was attached to but will be removed from.     *
 *                                                                                             *
 *          set_home -- if true, clears the flag's waypoint designation                        *
 *                                                                                             *
 * OUTPUT:  Was the flag successfully removed from the specified target?                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Flag_Remove(AbstractClass * target, bool set_home)
{
	bool rc = false;

	if (target) {

		/*
		**	Remove the flag from a unit
		*/
		UnitClass * object = target->As_UnitClass();
		if (object) {
			rc = object->Flag_Remove();
			if (rc && FlagLocation == target) {
				FlagLocation = NULL;
			}

		} else {

			/*
			**	Remove the flag from a cell
			*/
			Cell cell = target->Center_Coord().As_Cell();
			if (Map.In_Radar(cell)) {
				rc = Map[cell].Flag_Remove();
				if (rc && FlagLocation == target) {
					FlagLocation = NULL;
				}
			}
		}

		/*
		**	Handle the flag home cell:
		**	If 'set_home' is set, clear the home value & the cell's overlay
		*/
		if (set_home) {
			if (FlagHome != CELL_NONE) {
				Map[FlagHome].Overlay = OVERLAY_NONE;
				FlagHome = CELL_NONE;
			}
		}
		return(rc);
	}
	return(false);
}


/***********************************************************************************************
 * HouseClass::Flag_Attach -- Attach flag to specified cell (or thereabouts).                  *
 *                                                                                             *
 *    This routine will attach the house flag to the location specified. If the location       *
 *    cannot contain the flag, then a suitable nearby location will be selected.               *
 *                                                                                             *
 * INPUT:   cell  -- The desired cell location to place the flag.                              *
 *                                                                                             *
 *          set_home -- if true, resets the flag's waypoint designation                        *
 *                                                                                             *
 * OUTPUT:  Was the flag successfully placed?                                                  *
 *                                                                                             *
 * WARNINGS:   The cell picked for the flag might very likely not be the cell requested.       *
 *             Check the FlagLocation value to determine the final cell resting spot.          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *   10/08/1996 JLB : Uses map nearby cell scanning handler.                                   *
 *=============================================================================================*/
bool HouseClass::Flag_Attach(Cell const & cell, bool set_home)
{
	bool rc;

	/*
	**	Only continue if this cell is a legal placement cell.
	*/
	if (Map.In_Radar(cell)) {

		/*
		**	If the flag already exists, then it must be removed from the object
		**	it is attached to.
		*/
		Flag_Remove(FlagLocation, set_home);

		/*
		**	Attach the flag to the cell specified. If it can't be placed, then pick
		**	a nearby cell where it can be placed.
		*/
		Cell newcell = cell;
		rc = Map[newcell].Flag_Place(Class->House);
		if (!rc) {
			newcell = Map.Nearby_Location(cell, SPEED_TRACK);
			if (newcell != CELL_NONE) {
				rc = Map[newcell].Flag_Place(Class->House);
			}
		}

		/*
		**	If we've found a spot for the flag, place the flag at the new cell.
		**	if 'set_home' is set, OR this house has no current flag home cell,
		**	mark that cell as this house's flag home cell.
		*/
		if (rc) {
			FlagLocation = &Map[newcell];
		}

		return(rc);
	}
	return(false);
}


/***********************************************************************************************
 * HouseClass::Flag_Attach -- Attaches the house flag the specified unit.                      *
 *                                                                                             *
 *    This routine will attach the house flag to the specified unit. This routine is called    *
 *    when a unit drives over a cell containing a flag.                                        *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that the house flag is to be attached to.        *
 *                                                                                             *
 *          set_home -- if true, clears the flag's waypoint designation                        *
 *                                                                                             *
 * OUTPUT:  Was the flag attached successfully?                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Flag_Attach(UnitClass * object, bool set_home)
{
	if (object && !object->IsInLimbo) {
		Flag_Remove(FlagLocation, set_home);

		/*
		**	Attach the flag to the object.
		*/
		object->Flag_Attach(Class->House);
		FlagLocation = object;
		return(true);
	}
	return(false);
}


/***************************************************************************
 * HouseClass::MPlayer_Defeated -- multiplayer; house is defeated          *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/25/1995 BRR : Created.                                             *
 *=========================================================================*/
void HouseClass::MPlayer_Defeated(void)
{
	char txt[80];
	int i,j;
	HouseClass * hptr;
	HouseClass * hptr2;
	int num_alive;
	int num_humans;
	bool all_allies;
	bool foe_fell;

	/*
	**	Set the defeat flag for this house
	*/
	IsDefeated = true;

	/*
	**	If this is a computer controlled house, then all computer controlled
	**	houses become paranoid.
	*/
	if (IQ == Rule->MaxIQ && !Is_Human_Player() && Rule->IsComputerParanoid) {
		Computer_Paranoid();
	}

	/*
	**	Remove this house's flag & flag home cell
	*/
	if (Scen->Special.IsCaptureTheFlag) {
		if (FlagLocation) {
			Flag_Remove(FlagLocation, true);
		} else {
			if (FlagHome != CELL_NONE) {
				Flag_Remove(&Map[FlagHome], true);
			}
		}
	}

	/*
	 * If harvester truce is on, remove all of this player's harvesters.
	 */
	if (Session.Type != GAME_NORMAL && Scen->Special.IsHarvesterImmune) {
		for (int i = 0; i < Units.Count(); i++) {
			UnitClass * unit = Units[i];
			if (unit->House == this && unit->IsActive) {
				unit->Delete_Me();
			}
		}
	}

	/*
	**	If this is me:
	**	- Set MPlayerObiWan, so I can only send messages to all players, and
	**	not just one (so I can't be obnoxiously omnipotent), and reveal the
	**	map -- unless coach mode leaves me with my allies' vision instead
	**	- Add my defeat message
	*/
	if (PlayerPtr == this) {
		if (!Session.Options.CoachMode) {
			Become_ObiWan();
		}

		/*
		**	Pop up a message showing that I was defeated
		*/
		wsprintf(txt, Fetch_String(TXT_PLAYER_DEFEATED), (char const *)IniName);
		Session.Messages.Add_Message(NULL, 0, txt, Session.ColorIdx,
		TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), int(Rule->MessageDelay * TICKS_PER_MINUTE));

		Speak(VOX_YOU_HAVE_LOST);
		Map.Flag_To_Redraw();
		DebugString("MPlayer_Defeated() - Player %s has been defeated (OBIWAN MODE)\n", (char const *)IniName);

	} else {

		/*
		**	If it wasn't me, find out who was defeated
		*/
		if (!Class->IsMultiplayPassive) {
			wsprintf(txt, Fetch_String(TXT_PLAYER_DEFEATED), (char const *)IniName);

			Session.Messages.Add_Message(NULL, 0, txt, Scheme,
				TextPrintType(TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW), int(Rule->MessageDelay * TICKS_PER_MINUTE));
			Speak(VOX_PLAYER_WAS_DEFEATED);
			Map.Flag_To_Redraw();
		}
		DebugString("MPlayer_Defeated() - Opponent %s has been defeated\n", (char const *)IniName);
	}

	/*
	**	Find out how many players are left alive.
	*/
	num_alive = 0;
	num_humans = 0;
	foe_fell = false;
	for (i = 0; i < Houses.Count(); i++) {
		hptr = Houses[i];
		if (hptr && !hptr->IsDefeated && !hptr->Class->IsMultiplayPassive) {
			if (hptr->Is_Human_Player()) {
				num_humans++;
			}
			if (!Is_Ally(hptr) || !hptr->Is_Ally(this)) {
				foe_fell = true;
			}
			num_alive++;
		}
	}
	DebugString("MPlayer_Defeated() - Alive = %d, Humans = %d\n", num_alive, num_humans);

	/*
	**	If all the houses left alive are allied with each other, then in reality
	**	there's only one player left:
	*/
	all_allies = true;
	for (i = 0; i < Houses.Count(); i++) {

		/*
		**	Get a pointer to this house
		*/
		hptr = Houses[i];
		if (!hptr || hptr->IsDefeated || (Session.Type != GAME_SKIRMISH && hptr->Class->IsMultiplayPassive))
			continue;

		/*
		**	Loop through all houses; if there's one left alive that this house
		**	isn't allied with, then all_allies will be false
		*/
		for (j = 0; j < Houses.Count(); j++) {
			hptr2 = Houses[j];
			if (!hptr2) {
				continue;
			}

			if (!hptr2->IsDefeated && (Session.Type == GAME_SKIRMISH || !hptr2->Class->IsMultiplayPassive) && (!hptr->Is_Ally(hptr2) || !hptr2->Is_Ally(hptr))) {
				all_allies = false;
				break;
			}
		}
		if (!all_allies) {
			break;
		}
	}

	/*
	**	If all houses left are allies, set 'num_alive' to 1; game over.
	*/
	if (all_allies) {
		Session.SawGameCompletion = true;
		DebugString("Saw game completion due to player defeat\n");
		DebugString("MPlayer_Defeated() - All remaining players are allied\n");
		num_alive = 1;
	}

	/*
	**	If there's only one human player left or no humans left, the game is over.
	**	With nobody but the computer playing, only the fall of the survivors' last
	**	enemy ends it, so a match that only ever had one side never ends. Then:
	**	- Determine whether this player wins or loses, based on the state of the
	**	player's IsDefeated flag
	**	- Find all players' indices in the Session.Score array
	**	- Tally up scores for this game
	*/
	bool game_over = Session.AIOnly ? (num_alive == 1 && foe_fell) : (num_alive == 1 || num_humans == 0);
	if (game_over) {
		IsToDie = false;

		if (PlayerPtr->IsDefeated) {
			DebugString("MPlayer_Defeated() - Flag_To_Lose\n");
			Flag_To_Lose(false);
		} else {
			DebugString("MPlayer_Defeated() - Flag_To_Win\n");
			Flag_To_Win(false);
		}
	}
}


/***************************************************************************
 * HouseClass::Blowup_All -- blows up everything                           *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/16/1995 BRR : Created.                                             *
 *   06/09/1995 JLB : Handles aircraft.                                    *
 *   05/07/1996 JLB : Handles ships.                                       *
 *=========================================================================*/
void HouseClass::Blowup_All(void)
{
	int i;
	int damage;
	UnitClass * uptr;
	InfantryClass * iptr;
	BuildingClass * bptr;
	int count;

	/*
	**	Find everything owned by this house & blast it with a huge amount of damage
	**	at zero range.  Do units before infantry, so the units' drivers are killed
	**	too.  Using Explosion_Damage is like dropping a big bomb right on the
	**	object; it will also damage anything around it.
	*/
	for (i = 0; i < ::Units.Count(); i++) {
		if (::Units[i]->House == this && !::Units[i]->IsInLimbo) {
			uptr = ::Units[i];

			/*
			**	Some units can't be killed with one shot, so keep damaging them until
			**	they're gone.  The unit will destroy itself, and put an infantry in
			**	its place.  When the unit destroys itself, decrement 'i' since
			**	its pointer will be removed from the active pointer list.
			*/
			count = 0;
			while (::Units[i]==uptr && uptr->Strength) {
				damage = uptr->Strength;
				uptr->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
				count++;
				if (count > 5 && uptr->IsActive) {
					delete uptr;
					break;
				}
			}
			i--;
		}
	}

	/*
	**	Destroy all aircraft owned by this house.
	*/
	for (i = 0; i < ::Aircraft.Count(); i++) {
		if (::Aircraft[i]->House == this && !::Aircraft[i]->IsInLimbo) {
			AircraftClass * aptr = ::Aircraft[i];

			damage = aptr->Strength;
			aptr->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
			if (!aptr->IsActive) {
				i--;
			}
		}
	}

	/*
	**	Buildings don't delete themselves when they die; they shake the screen
	**	and begin a countdown, so don't decrement 'i' when it's destroyed.
	*/
	for (i = 0; i < Buildings.Count(); i++) {
		if (Buildings[i]->House == this && !Buildings[i]->IsInLimbo) {
			bptr = Buildings[i];

			count = 0;
			while (Buildings[i]==bptr && bptr->Strength) {
				damage = bptr->Strength;
				bptr->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
				count++;
				if (count > 5) {
					delete bptr;
					break;
				}
			}
		}
	}

	/*
	**	Infantry don't delete themselves when they die; they go into a death-
	**	animation sequence, so there's no need to decrement 'i' when they die.
	*/
	for (i = 0; i < Infantry.Count(); i++) {
		if (Infantry[i]->House == this && !Infantry[i]->IsInLimbo) {
			iptr = Infantry[i];

			count = 0;
			while (Infantry[i]==iptr && iptr->Strength) {
				damage = iptr->Strength;
				iptr->Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);

				count++;
				if (count > 5) {
					delete iptr;
					break;
				}
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Flag_To_Die -- Flags the house to blow up soon.                                 *
 *                                                                                             *
 *    When this routine is called, the house will blow up after a period of time. Typically    *
 *    this is called when the flag is captured or the HQ destroyed.                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the house flagged to blow up?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Flag_To_Die(void)
{
	if (!IsToWin && !IsToDie && !IsToLose) {
		IsToDie = true;
		BorrowedTime = TICKS_PER_MINUTE * Rule->SavourDelay;
		if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
			int time = Frame + std::max((int)BorrowedTime, (int)Session.MaxAhead);
			BorrowedTime = 10 * ((time + 9) / 10) - Frame;
		}
		DebugString("Frame %d, BorrowedTime == %d\n", Frame, (int)BorrowedTime);
	}
	return(IsToDie);
}


/***********************************************************************************************
 * HouseClass::Flag_To_Win -- Flags the house to win soon.                                     *
 *                                                                                             *
 *    When this routine is called, the house will be declared the winner after a period of     *
 *    time.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Was the house flagged to win?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Flag_To_Win(bool silent)
{
	if (!IsToWin && !IsToDie && !IsToLose) {
		IsToWin = true;
		if (!silent) {
			BorrowedTime = int(TICKS_PER_MINUTE * Rule->SavourDelay);
			if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
				int time = Frame + std::max((int)BorrowedTime, (int)Session.MaxAhead);
				BorrowedTime = 10 * ((time + 9) / 10) - Frame;
			}
			DebugString("Frame %d, BorrowedTime == %d\n", Frame, (int)BorrowedTime);
		}
		if (Session.Type == GAME_NORMAL && Is_Player_Control()) {
			Lock_Scenario_Input();
			TacticalMap->Set_Caption_Text(TXT_SCENARIO_WON);
			Speak(VOX_ACCOMPLISHED);
			Map.Flag_To_Redraw();
		} else if (Is_Player_Control() && !IsObserver) {
			TacticalMap->Set_Caption_Text(TXT_VICTORIOUS);
			Speak(VOX_YOU_ARE_VICTORIOUS);
			Map.Flag_To_Redraw();
		}
	}
	return(IsToWin);
}


/***********************************************************************************************
 * HouseClass::Flag_To_Lose -- Flags the house to die soon.                                    *
 *                                                                                             *
 *    When this routine is called, it will spell the doom of this house. In a short while      *
 *    all of the object owned by this house will explode. Typical use of this routine is when  *
 *    the flag has been captured or the command vehicle has been destroyed.                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Has the doom been initiated?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Flag_To_Lose(bool silent)
{
	IsToWin = false;
	if (!IsToDie && !IsToLose) {
		IsToLose = true;
		if (!silent) {
			BorrowedTime = int(TICKS_PER_MINUTE * Rule->SavourDelay);
			if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
				int time = Frame + std::max((int)BorrowedTime, (int)Session.MaxAhead);
				BorrowedTime = 10 * ((time + 9) / 10) - Frame;
			}
			DebugString("Frame %d, BorrowedTime == %d\n", Frame, (int)BorrowedTime);
		}
		if (Session.Type == GAME_NORMAL && Is_Player_Control()) {
			Lock_Scenario_Input();
			TacticalMap->Set_Caption_Text(TXT_SCENARIO_LOST);
			Speak(VOX_FAIL);
			Map.Flag_To_Redraw();
		} else if (Is_Player_Control() && !IsObserver) {
			TacticalMap->Set_Caption_Text(TXT_LOST);
			Speak(VOX_YOU_HAVE_LOST);
			Map.Flag_To_Redraw();
		}
	}
	return(IsToLose);
}


/// <summary>
/// Flags this house to bring its game to an end.
/// Use this routine to wind the scenario up regardless of who has earned the victory. A
/// house with no verdict yet is awarded the win; one already flagged merely gets the grace
/// period it needs to play the ending out.
/// </summary>
void HouseClass::Flag_To_End(void)
{
	if (!IsToLose && !IsToWin) {
		Flag_To_Win();
	} else {
		BorrowedTime.Start();
	}
}


/***********************************************************************************************
 * HouseClass::Init_Data -- Initializes the multiplayer color data.                            *
 *                                                                                             *
 *    This routine is called when initializing the color and remap data for this house. The    *
 *    primary user of this routine is the multiplayer version of the game, especially for      *
 *    saving & loading multiplayer games.                                                      *
 *                                                                                             *
 * INPUT:   color    -- The color of this house.                                               *
 *                                                                                             *
 *          house    -- The house that this should act like.                                   *
 *                                                                                             *
 *          credits  -- The initial credits to assign to this house.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Init_Data(int color, HousesType house, int credits)
{
	Credits = Control.InitialCredits = credits;
	Credits = credits;
	Class->Scheme = color;
	Scheme = color;
}


/***********************************************************************************************
 * HouseClass::Power_Fraction -- Fetches the current power output rating.                      *
 *                                                                                             *
 *    Use this routine to fetch the current power output as a fixed point fraction. The        *
 *    value 0x0100 is 100% power.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with power rating as a fixed pointer number.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/22/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
double HouseClass::Power_Fraction(void) const
{
	if (Power >= Drain || Drain == 0) return(1);

	if (Power) {
		return((float)Power / (float)Drain);
	}
	return(0);
}


/***********************************************************************************************
 * HouseClass::Sell_Wall -- Tries to sell the wall at the specified location.                  *
 *                                                                                             *
 *    This routine will try to sell the wall at the specified location. If there is a wall     *
 *    present and it is owned by this house, then it can be sold.                              *
 *                                                                                             *
 * INPUT:   cell  -- The cell that wall selling is desired.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/05/1995 JLB : Created.                                                                 *
 *   11/02/1996 JLB : Checks unsellable bit for wall type.                                     *
 *=============================================================================================*/
void HouseClass::Sell_Wall(Cell const & cell, bool quiet)
{
	if (cell != CELL_NONE) {
		CellClass const & cptr = Map[cell];
		OverlayType overlay = cptr.Overlay;

		if (overlay != OVERLAY_NONE) {
			HousesType owner = cptr.Owner;

			if (owner != HOUSE_NONE && Houses[owner]->Is_Human_Player()) {
				OverlayTypeClass const * optr = OverlayTypes[overlay];

				if (optr->IsWall) {
					BuildingTypeClass const * btype = NULL;
					for (int i = 0; i < BuildingTypes.Count(); i++) {
						if (BuildingTypes[i]->ToOverlay == optr) {
							btype = BuildingTypes[i];
							break;
						}
					}

					if (btype != NULL && !btype->IsUnsellable) {

						if (PlayerPtr == this && !quiet) {
							Sound_Effect(Rule->SellSound);
						}

						btype->Cost_Of(this);
						Map[cell].Overlay = OVERLAY_NONE;
						Map[cell].OverlayData = 0;
						Map[cell].Owner = HOUSE_NONE;
						Map[cell].Recalc_Attributes();
						Map[cell].Wall_Update();
						Map.Radar_Background(cell);
						Map.Update_Cell_Zone(cell);
						Map.Update_Cell_Subzones(cell);
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Suggest_New_Building -- Examines the situation and suggests a building.         *
 *                                                                                             *
 *    This routine is called when a construction yard needs to know what to build next. It     *
 *    will either examine the prebuilt base list or try to figure out what to build next       *
 *    based on the current game situation.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building type class to build.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
BuildingTypeClass const * HouseClass::Suggest_New_Building(void) const
{
	if (BuildStructure != STRUCT_NONE) {
		return(BuildingTypes[BuildStructure]);
	}
	return(NULL);
}


/***********************************************************************************************
 * HouseClass::Find_Building -- Finds a building of specified type.                            *
 *                                                                                             *
 *    This routine is used to find a building of the specified type. This is particularly      *
 *    useful for when some event requires a specific building instance. The nuclear missile    *
 *    launch is a good example.                                                                *
 *                                                                                             *
 * INPUT:   type  -- The building type to scan for.                                            *
 *                                                                                             *
 *          zone  -- The zone that the building must be located in. If no zone specific search *
 *                   is desired, then pass ZONE_NONE.                                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the building type requested. If there is no building     *
 *          of the type requested, then NULL is returned.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1995 JLB : Created.                                                                 *
 *   10/02/1995 JLB : Allows for zone specifics.                                               *
 *=============================================================================================*/
BuildingClass * HouseClass::Find_Building(StructType type, ZoneType zone) const
{
	/*
	**	Only scan if we KNOW there is at least one building of the type
	**	requested.
	*/
	if (BQuantity.Value(type) > 0) {

		/*
		**	Search for a suitable launch site for this missile.
		*/
		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * b = Buildings[index];
			if (b && !b->IsInLimbo && this == b->House && BuildingTypes.ID(b->Class) == type) {
				if (zone == ZONE_NONE || Which_Zone(b) == zone) {
					return(b);
				}
			}
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * HouseClass::Find_Build_Location -- Finds a suitable building location.                      *
 *                                                                                             *
 *    This routine is used to find a suitable building location for the building specified.    *
 *    The auto base building logic uses this when building the base for the computer.          *
 *                                                                                             *
 * INPUT:   building -- Pointer to the building that needs to be placed down.                  *
 *                                                                                             *
 * OUTPUT:  Returns with the coordinate to place the building at. If there are no suitable     *
 *          locations, then NULL is returned.                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/27/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Coord HouseClass::Find_Build_Location(BuildingClass * building) const
{
	return(COORD_NONE);
}


/***********************************************************************************************
 * HouseClass::Recalc_Center -- Recalculates the center point of the base.                     *
 *                                                                                             *
 *    This routine will average the location of the base and record the center point. The      *
 *    recorded center point is used to determine such things as how far the base is spread     *
 *    out and where to protect the most. This routine should be called whenever a building     *
 *    is created or destroyed.                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Recalc_Center(void)
{
	/*
	**	First presume that there is no base. If there is a base, then these values will be
	**	properly filled in below.
	*/
	Center = COORD_NONE;
	Radius = 0;
	for (ZoneType zone = ZONE_FIRST; zone < ZONE_COUNT; zone++) {
		ZoneInfo[zone].AirDefense = 0;
		ZoneInfo[zone].ArmorDefense = 0;
		ZoneInfo[zone].InfantryDefense = 0;
	}

	/*
	**	Only process the center base size/position calculation if there are buildings to
	**	consider. When no buildings for this house are present, then no processing need
	**	occur.
	*/
	if (CurBuildings > 0) {
		int x = 0;
		int y = 0;
		int count = 0;
		int index;

		for (index = 0; index < Buildings.Count(); index++) {
			BuildingClass const * b = Buildings[index];

			if (b != NULL && !b->IsInLimbo && b->House == this && b->Strength > 0) {

				/*
				**	Give more "weight" to buildings that cost more. The presumption is that cheap
				**	buildings don't affect the base disposition as much as the more expensive
				**	buildings do.
				*/
				int weight = (b->Class->Cost_Of(this) / 1000)+1;
				for (int i = 0; i < weight; i++) {
					x += b->Center_Coord().X;
					y += b->Center_Coord().Y;
					count++;
				}
			}
		}

		/*
		**	This second check for quantity of buildings is necessary because the first
		**	check against CurBuildings doesn't take into account if the building is in
		**	limbo, but for base calculation, the limbo state disqualifies a building
		**	from being processed. Thus, CurBuildings may indicate a base, but count may
		**	not match.
		*/
		if (count > 0) {
			x /= count;
			y /= count;
			Center = Coord(x, y);
		}

		/*
		**	If there were any buildings discovered as legal to consider as part of the base,
		**	then figure out the general average radius of the building disposition as it
		**	relates to the center of the base.
		*/
		if (count > 1) {
			int radius = 0;

			for (index = 0; index < Buildings.Count(); index++) {
				BuildingClass const * b = Buildings[index];

				if (b != NULL && !b->IsInLimbo && (HouseClass *)b->House == this && b->Strength > 0) {
					radius += Distance(Center, b->Center_Coord());
				}
			}
			Radius = std::max(radius / count, 2 * CELL_LEPTON_W);

			/*
			**	Determine the relative strength of each base defense zone.
			*/
			for (index = 0; index < Buildings.Count(); index++) {
				BuildingClass const * b = Buildings[index];

				if (b != NULL && !b->IsInLimbo && (HouseClass *)b->House == this && b->Strength > 0) {
					ZoneType z = Which_Zone(b);

					if (z != ZONE_NONE) {
						ZoneInfo[z].ArmorDefense += b->Anti_Armor();
						ZoneInfo[z].AirDefense += b->Anti_Air();
						ZoneInfo[z].InfantryDefense += b->Anti_Infantry();
					}
				}
			}

		} else {
			Radius = 2 * CELL_LEPTON;
		}
	}
}


/***********************************************************************************************
 * HouseClass::Expert_AI -- Handles expert AI processing.                                      *
 *                                                                                             *
 *    This routine is called when the computer should perform expert AI processing. This       *
 *    method of AI is categorized as an "Expert System" process.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of game frames to delay before calling this routine again.      *
 *                                                                                             *
 * WARNINGS:   This is relatively time consuming -- call periodically.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int HouseClass::Expert_AI(void)
{
	BuildingClass * b = NULL;
	bool stop = false;
	int time = TICKS_PER_SECOND * 10;

	/*
	**	If there is no enemy assigned to this house, then assign one now. The
	**	enemy that is closest is picked. However, don't pick an enemy if the
	**	base has not been established yet.
	*/
	if (PickEnemyTimer == 0) {
		if (Enemy == HOUSE_NONE && Session.Type != GAME_NORMAL && !Class->IsMultiplayPassive) {
			Coord center = Center;
			if (center != COORD_NONE) {
				int close = INT_MAX;
				HouseClass * enemy = NULL;

				for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {
					HouseClass * h = Houses[house];
					if (h != this && !h->Class->IsMultiplayPassive && !h->IsDefeated) {

						/*
						**	Determine a priority value based on distance to the center of the
						**	candidate base. The higher the value, the better the candidate house
						**	is to becoming the preferred enemy for this house.
						*/
						int value = (h->Center - center).Length();

						/*
						**	Compare the calculated value for this candidate house and if it is
						**	greater than the previously recorded maximum, record this house as
						**	the prime candidate for enemy.
						*/
						if (value < close) {
							enemy = h;
							close = value;
						}
					}
				}

				/*
				**	Record this closest enemy base as the first enemy to attack.
				*/
				if (enemy) {
					Add_Anger(1, enemy);
				}
			}
		}
	}

	/*
	**	If the current enemy no longer has a base or is defeated, then don't consider
	**	that house a threat anymore. Clear out the enemy record and then try
	**	to find a new enemy.
	*/
	if (Enemy != HOUSE_NONE) {
		HouseClass * h = Houses[Enemy];

		if (h->IsDefeated) {
			Clear_Anger(h);
			Enemy = HOUSE_NONE;
		}
	}

	if (Session.Type != GAME_NORMAL || IQ >= Rule->IQSuperWeapons) {
		AI_Super_Weapons();
	}

	/*
	**	House state transition check occurs here. Transitions that occur here are ones
	**	that relate to general base condition rather than specific combat events.
	**	Typically, this is limited to transitions between normal buildup mode and
	**	broke mode.
	*/
	if (State == STATE_ENDGAME) {
		Fire_Sale();
		All_To_Hunt();
	} else {
		if (State == STATE_BUILDUP) {
			if (Available_Money() < 25) {
				State = STATE_BROKE;
			}
		}
		if (State == STATE_BROKE) {
			if (Available_Money() >= 25) {
				State = STATE_BUILDUP;
			}
		}
		if (State == STATE_ATTACKED && LATime + TICKS_PER_MINUTE < Frame) {
			State = STATE_BUILDUP;
		}
		if (State != STATE_ATTACKED && LATime + TICKS_PER_MINUTE > Frame) {
			State = STATE_ATTACKED;
		}
	}

	if (Session.Type != GAME_NORMAL) {
		/*
		**	Records the urgency of all actions possible.
		*/
		UrgencyType urgency[STRATEGY_COUNT];
		StrategyType strat;
		for (strat = STRATEGY_FIRST; strat < STRATEGY_COUNT; strat++) {
			urgency[strat] = URGENCY_NONE;

			switch (strat) {
				case STRATEGY_FIRE_SALE:
					urgency[strat] = Check_Fire_Sale();
					break;

				case STRATEGY_RAISE_MONEY:
					urgency[strat] = Check_Raise_Money();
					break;

				default:
					urgency[strat] = URGENCY_NONE;
					break;
			}
		}

		/*
		**	Performs the action required for each of the strategies that share
		**	the most urgent category. Stop processing if any strategy at the
		**	highest urgency performed any action. This is because higher urgency
		**	actions tend to greatly affect the lower urgency actions.
		*/
		for (UrgencyType u = URGENCY_CRITICAL; u >= URGENCY_LOW; u--) {
			//bool acted = false;

			for (strat = STRATEGY_FIRST; strat < STRATEGY_COUNT; strat++) {
				if (urgency[strat] == u) {
					switch (strat) {
						case STRATEGY_FIRE_SALE:
							//acted |= AI_Fire_Sale(u);
							AI_Fire_Sale(u);
							break;

						case STRATEGY_RAISE_MONEY:
							//acted |= AI_Raise_Money(u);
							AI_Raise_Money(u);
							break;

						default:
							break;
					}
				}
			}
		}
	}

	return(TICKS_PER_SECOND*7 + Random_Pick(1, TICKS_PER_SECOND/2));
}


/// <summary>
/// Determines how urgently this house should sell off its base.
/// This routine is part of the computer's emergency handling. A house that still holds
/// buildings but has nothing left that can produce is finished, and there is no reason to
/// leave the base standing.
/// </summary>
/// <returns>Returns with the urgency of selling the base off.</returns>
UrgencyType HouseClass::Check_Fire_Sale(void)
{
	/*
	**	If there are no more factories at all, then sell everything off because the game
	**	is basically over at this point.
	*/
	if (State != STATE_ATTACKED && CurBuildings > 0) {
		for (int i = 0; i < Buildings.Count(); i++) {
			BuildingClass * bptr = Buildings[i];
			if (bptr != NULL && bptr->IsActive && !bptr->IsInLimbo && this == bptr->House && bptr->Class->ToBuild != RTTI_NONE) {
				return(URGENCY_NONE);
			}
		}
		return(URGENCY_CRITICAL);
	}
	return(URGENCY_NONE);
}


/*
**	Checks to see if money is critically low and something must be done
**	to immediately raise cash.
*/

/// <summary>
/// Determines how urgently this house must raise cash.
/// This routine is part of the computer's emergency handling. It weighs how close the house
/// already is to restoring its income -- a refinery or a harvester already on order and
/// affordable counts as the problem being dealt with. Human controlled houses are left to
/// fend for themselves.
/// </summary>
/// <returns>Returns with the urgency of this house's money troubles.</returns>
UrgencyType HouseClass::Check_Raise_Money(void)
{
	int i;
	int j;

	UrgencyType urgency = URGENCY_NONE;

	/*
	 * Humans look after their own finances.
	 */
	if (Is_Human_Player()) {
		return(URGENCY_NONE);
	}

	if (!Can_Make_Money()) {
		if (ABQuantity.Value(Rule->BuildRefinery[0]->HeapID) > 0) {

			/*
			 * A refinery already going up means the situation is being taken care of.
			 */
			for (i = 0; i < Buildings.Count(); i++) {
				if (Buildings[i]->House == this) {
					if (Buildings[i]->Class == Rule->BuildRefinery[0] && Buildings[i]->CurrentMission == MISSION_CONSTRUCTION) {
						return(URGENCY_NONE);
					}
					urgency = URGENCY_NONE;
				}
			}

			/*
			 * There is a refinery, so a harvester is what this house is short of. If one is
			 * not on order, or is on order but cannot be paid for, then cash must be raised.
			 */
			if (BuildUnit != Rule->HarvesterUnit[0]->HeapID) {
				if (Available_Money() < Rule->HarvesterUnit[0]->Cost_Of(this)) {
					urgency++;
				}
			} else {
				for (j = 0; j < Factories.Count(); j++) {
					FactoryClass * fptr = Factories[j];
					if (fptr->House == this && Factories[j]->Get_Object() != NULL) {
						UnitClass * unit = (UnitClass *)Factories[j]->Get_Object();
						if (unit->RTTI == RTTI_UNIT && unit->Class == Rule->HarvesterUnit[0]) {
							fptr = Factories[j];
							if (fptr != NULL) {
								int owed = fptr->Balance;
								if (Available_Money() >= owed) {
									return(urgency);
								}
							}
							break;
						}
					}
				}
				urgency++;
			}
		} else {

			/*
			 * No refinery at all. The same reasoning applies to getting one built.
			 */
			if (BuildStructure != Rule->BuildRefinery[0]->HeapID) {
				if (Available_Money() < Rule->BuildRefinery[0]->Cost_Of(this)) {
					urgency++;
				}
			} else {
				for (j = 0; j < Factories.Count(); j++) {
					FactoryClass * fptr = Factories[j];
					if (fptr->House == this && Factories[j]->Get_Object() != NULL) {
						BuildingClass * building = (BuildingClass *)Factories[j]->Get_Object();
						if (building->RTTI == RTTI_BUILDING && building->Class == Rule->BuildRefinery[0]) {
							fptr = Factories[j];
							if (fptr != NULL) {
								int owed = fptr->Balance;
								if (Available_Money() >= owed) {
									return(urgency);
								}
							}
							break;
						}
					}
				}
				urgency++;
			}
		}
	}
	return(urgency);
}


/// <summary>
/// Sells off the base and sends everything on a suicide run.
/// This is the last resort of the computer's emergency handling, and is acted upon only when
/// the situation has been judged critical.
/// </summary>
/// <param name="urgency">The urgency that the fire sale check reported.</param>
/// <returns>bool; Was the fire sale ordered?</returns>
bool HouseClass::AI_Fire_Sale(UrgencyType urgency)
{
	if (CurBuildings > 0 && urgency == URGENCY_CRITICAL) {
		Fire_Sale();
		All_To_Hunt();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * HouseClass::AI_Raise_Money -- Raise emergency cash by selling buildings.                    *
 *                                                                                             *
 *    This routine handles the situation where the computer desperately needs cash but cannot  *
 *    wait for normal harvesting to raise it. Buildings must be sold.                          *
 *                                                                                             *
 * INPUT:   urgency  -- The urgency level that cash must be raised. The greater the urgency,   *
 *                      the more important the buildings that can be sold become.              *
 *                                                                                             *
 * OUTPUT:  bool; Was a building sold to raise cash?                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/02/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::AI_Raise_Money(UrgencyType urgency)
{
	int i;
	int j;

	int refund = 0;
	int needed = 0;

	/*
	 * A refinery plus a war factory means a harvester is the cheaper way back into business.
	 */
	bool can_build_harvester = ABQuantity.Value(Rule->BuildRefinery[0]->HeapID) > 0 &&
		(ABQuantity.Value(Rule->BuildWeapons[0]->HeapID) > 0 || ABQuantity.Value(Rule->BuildWeapons[1]->HeapID) > 0);
	if (can_build_harvester) {
		needed = Rule->HarvesterUnit[0]->Cost_Of(this);
	} else {
		needed = Rule->BuildRefinery[0]->Cost_Of(this);
	}

	/*
	 * Sell the base off from the back of the build list forward, stopping as soon as enough
	 * has been raised to pay for the replacement.
	 */
	i = Base.Nodes.Count() - 1;
	for (; i >= 0; i--) {

		BuildingClass * bptr = Base.Get_Building(i);
		if (bptr != NULL) {
			char level = bptr->UpgradeLevel;
			if (level != 0) {
				int cost = bptr->Upgrades[level - 1]->Cost_Of(this);
				Credits += cost;
				bptr->Remove_Upgrade();
				RecalcPower = true;
				RecalcRadar = true;
				Adjust_House_Power(this);
			} else {
				bptr->Sell_Back(1);
				if (bptr->Class->IsGate) {
					Base.Nodes.Delete_Index(i);
				}
				refund += bptr->Refund_Amount();
			}
		} else if (Base.Is_Built(i)) {
			Sell_Wall(Base.Nodes[i].CellID, false);
			Base.Nodes.Delete_Index(i);
		}

		if (refund + Credits > needed) {

			/*
			 * Enough money is in hand, so stop everything that is still on order and put
			 * the proceeds towards the replacement instead.
			 */
			for (j = Factories.Count() - 1; j >= 0; j--) {
				FactoryClass * fptr = Factories[j];
				if (fptr->House == this) {
					Factories[j]->Suspend();
					Factories[j]->Abandon();
					delete Factories[j];
				}
			}
			BuildUnit = UNIT_NONE;
			BuildInfantry = INFANTRY_NONE;
			BuildAircraft = AIRCRAFT_NONE;
			BuildStructure = STRUCT_NONE;
			if (can_build_harvester) {
				BuildUnit = Rule->HarvesterUnit[0]->HeapID;
				ProductionMode = UNITS;
			} else {
				int next = Base.Next_Buildable_Index();
				if (next != 0) {
					Base.Nodes.Insert_After(next - 1, BaseNodeClass(Rule->BuildRefinery[0]->HeapID, Cell(0, 0)));
					for (j = Base.Nodes.Count() - 1; j > next; j--) {
						if (Base.Nodes[j].Type == Rule->BuildRefinery[0]->HeapID) {
							Base.Nodes.Delete_Index(j);
						}
					}
					ProductionMode = BUILDINGS;
					AI_Building();
				} else {
					goto GO_HUNT;
				}
			}
			break;
		}
	}

	if (i <= 0) {
		GO_HUNT:
		All_To_Hunt();
	}
	return(false);
}


/***********************************************************************************************
 * HouseClass::AI_Building -- Determines what building to build.                               *
 *                                                                                             *
 *    This routine handles the general case of determining what building to build next.        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine again. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *   11/03/1996 JLB : Tries to match aircraft of enemy                                         *
 *=============================================================================================*/
int HouseClass::AI_Building(void)
{
	enum {
		WALL = -3,
		STOP = -2,
		DEFENSE = -1
	};

	if (BuildStructure != STRUCT_NONE) return(TICKS_PER_SECOND);

	if (ConYards.Count() == 0) return(TICKS_PER_SECOND);

	BaseNodeClass * node = Base.Next_Buildable();

	if (node == NULL) return(TICKS_PER_SECOND);

	/*
	 * Build some walls.
	 */
	if (node->Type == WALL) {
		Base.Nodes.Delete_Index(Base.Nodes.ID(node));
		AI_Build_Wall();
		return(1);
	}

	/*
	**	Always build up some base defense.
	*/
	if (node->Type == DEFENSE || BuildingTypes[node->Type] == Rule->WallTower && node->CellID == Cell(0, 0)) {

		int nodeid = Base.Nodes.ID(node);
		DynamicVectorClass<Cell> * cells = NULL;
		if (Base.OuterCells.Count() > 0) {
			cells = &Base.OuterCells;
		}

		if (!AI_Build_Defense(nodeid, cells)) {

			/*
			 * A wall tower node is deleted twice, so the node that follows it goes
			 * with it.
			 */
			if (node->Type == Rule->WallTower->HeapID) {
				Base.Nodes.Delete_Index(nodeid);
			}

			/*
			 * Remove the node from the list.
			 */
			Base.Nodes.Delete_Index(nodeid);
			return(1);
		}

		node = Base.Next_Buildable();
	}

	if (node == NULL || node->Type == STOP) return(TICKS_PER_SECOND);

	BuildingTypeClass * b = BuildingTypes[node->Type];

	/*
	**	Try to build a power plant if there is insufficient power.
	*/
	if (Session.Type != GAME_NORMAL && b->Drain + Drain > Power - PowerSurplus && b != Rule->BuildConst[0] && b->Drain > 0) {

		BuildingTypeClass const * choice;

		if (stricmp(Class->IniName, "GDI") == 0) {

			bool can_build_turbine = false;
			for (int i = 0; i < Buildings.Count(); i++) {

				BuildingClass * owned_b = Buildings[i];
				if (owned_b->House == this) {
					if (owned_b->Class == Rule->GDIPowerPlant && owned_b->UpgradeLevel < owned_b->Class->Upgrades) {
						can_build_turbine = true;
						break;
					}
				}
			}

			if (can_build_turbine && (Random_Pick(0, INT_MAX-1) / (double)(INT_MAX-1)) < Rule->AIUseTurbineUpgradeChance) {
				choice = Rule->GDIPowerTurbine;
			} else {
				choice = Rule->GDIPowerPlant;
			}

		} else {

			DynamicVectorClass<BuildingTypeClass const *> owned_buildings;

			for (int i = 0; i < Buildings.Count(); i++) {
				BuildingClass * b2 = Buildings[i];
				if (b2->House == this) {
					owned_buildings.Add(b2->Class);
				}
			}

			if (AI_Has_Prerequisites(Rule->NodAdvancedPower, owned_buildings, owned_buildings.Count())) {
				choice = Rule->NodAdvancedPower;
			} else {
				choice = Rule->NodRegularPower;
			}
		}

		/*
		 * Build our chosen power structure before building whatever else we're trying to build.
		 */
		int id = Base.Nodes.ID(node);
		Base.Nodes.Insert_After(id - 1, BaseNodeClass(choice->HeapID, Cell(0, 0)));

		return(1);
	}

	/*
	 * If this is a building upgrade, check that it can actually be placed where it is
	 * scheduled to go.
	 */
	if (b->PowersUpToLevel <= 0 && b->PowersUpToLevel == -1 && node->CellID != CELL_NONE && !b->PowersUpBuilding.empty()) {

		BuildingClass * existing_building = Map[node->CellID].Cell_Building();
		BuildingTypeClass * powerup = BuildingTypes[BuildingTypeClass::From_Name(b->PowersUpBuilding)];

		if (existing_building == NULL) {
			node->CellID = Cell(0, 0);
		} else if (existing_building->Class != powerup) {
			node->CellID = Cell(0, 0);
		} else if (existing_building->Class->PowersUpToLevel == -1 && existing_building->UpgradeLevel >= existing_building->Class->Upgrades || existing_building->Class->PowersUpToLevel > 0 && existing_building->UpgradeLevel > 0) {
			node->CellID = Cell(0, 0);
		}
	}

	BuildStructure = node->Type;

	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * HouseClass::AI_Unit -- Determines what unit to build next.                                  *
 *                                                                                             *
 *    This routine handles the general case of determining what units to build next.           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of games frames to delay before calling this routine again.*
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int HouseClass::AI_Unit(void)
{
	if (BuildUnit != UNIT_NONE) return(TICKS_PER_SECOND);

	int harv = AUQuantity.Value(Rule->HarvesterUnit[0]->HeapID);
	int ref = ABQuantity.Value(Rule->BuildRefinery[0]->HeapID);
	int mult;
	if (Session.Type == GAME_NORMAL || Difficulty == DIFF_HARD) {
		mult = 1;
	} else {
		mult = 2;
	}

	/*
	**	A computer controlled house will try to build a replacement
	**	harvester if possible.
	*/
	if (IQ >= Rule->IQHarvester && !IsTiberiumShort && !Is_Human_Player() && ref * mult > harv) {
		if ((unsigned int)Rule->HarvesterUnit[0]->Level <= (unsigned int)Control.TechLevel) {
			BuildUnit = Rule->HarvesterUnit[0]->HeapID;
			return(TICKS_PER_SECOND);
		}
	}

	std::vector<int> counter(UnitTypes.Count(), 0);
	std::vector<int> value(UnitTypes.Count(), 0x7FFFFFFF);

	/*
	**	Build a list of the maximum of each type we wish to produce. This will be
	**	twice the number required to fill all teams.
	*/
	for (int i = 0; i < Teams.Count(); i++) {
		TeamClass * tptr = Teams[i];
		if (tptr != NULL) {

			int val = tptr->CreationFrame;

			if (((tptr->Class->IsReinforcable && !tptr->IsFullStrength) || (!tptr->IsForcedActive && !tptr->IsHasBeen)) && tptr->House == this) {
				TEAM_MEMBER_LIST _members;
				tptr->Team_Members(_members);

				for (int subindex = 0; subindex < _members.Count(); subindex++) {

					UnitTypeClass const * memtype = (UnitTypeClass const *)_members[subindex];

					if (memtype->RTTI == RTTI_UNITTYPE
						&& static_cast<unsigned>(memtype->HeapID) < counter.size()) {
						counter[memtype->HeapID]++;
						if (val < value[memtype->HeapID]) {
							value[memtype->HeapID] = val;
						}
					}
				}
			}
		}
	}

	/*
	**	Reduce the theoretical maximum by the actual number of objects currently
	**	in play.
	*/
	for (int oindex = 0; oindex < Units.Count(); oindex++) {
		UnitClass * obj = Units[oindex];
		if (obj != NULL && obj->Is_Recruitable(this)
			&& static_cast<unsigned>(obj->Class->HeapID) < counter.size()
			&& counter[obj->Class->HeapID] > 0) {
			counter[obj->Class->HeapID]--;
		}
	}

	/*
	**	Pick to build the most needed object but don't consider those object that
	**	can't be built because of scenario restrictions or insufficient cash.
	*/
	int bestval = -1;
	UnitType lasttype = UNIT_NONE;
	int lastval = 0x7FFFFFFF;
	std::vector<UnitType> bestlist;
	bestlist.reserve(UnitTypes.Count());
	for (UnitType type = UnitType(0); type < UnitTypes.Count(); type++) {
		if (counter[type] > 0 && Can_Build(UnitTypes[type], false, false) && UnitTypes[type]->Cost_Of(this) <= Available_Money()) {
			if (bestval == -1 || bestval < counter[type]) {
				bestval = counter[type];
				bestlist.clear();
			}
			bestlist.push_back(type);

			if (lasttype == UNIT_NONE || value[type] < lastval) {
				lasttype = type;
				lastval = value[type];
			}
		}
	}

	if (Random_Double(0, 0x7FFFFFFE) < Rule->FillEarliestTeamProbability[Difficulty] / 100.0) {
		BuildUnit = lasttype;
	} else {
		/*
		**	The object type to build is now known. Fetch a pointer to the techno type class.
		*/
		if (!bestlist.empty()) {
			BuildUnit = bestlist[Random_Pick(0, static_cast<int>(bestlist.size()) - 1)];
		}
	}

	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * HouseClass::AI_Infantry -- Determines the infantry unit to build.                           *
 *                                                                                             *
 *    This routine handles the general case of determining what infantry unit to build         *
 *    next.                                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before being called again.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int HouseClass::AI_Infantry(void)
{
	if (BuildInfantry != INFANTRY_NONE) return(TICKS_PER_SECOND);

	std::vector<int> counter(InfantryTypes.Count(), 0);
	std::vector<int> value(InfantryTypes.Count(), 0x7FFFFFFF);

	/*
	**	Build a list of the maximum of each type we wish to produce. This will be
	**	twice the number required to fill all teams.
	*/
	for (int i = 0; i < Teams.Count(); i++) {
		TeamClass * tptr = Teams[i];
		if (tptr != NULL) {

			int val = tptr->CreationFrame;

			if (((tptr->Class->IsReinforcable && !tptr->IsFullStrength) || (!tptr->IsForcedActive && !tptr->IsHasBeen)) && tptr->House == this) {
				TEAM_MEMBER_LIST _members;
				tptr->Team_Members(_members);

				for (int subindex = 0; subindex < _members.Count(); subindex++) {

					InfantryTypeClass const * memtype = (InfantryTypeClass const *)_members[subindex];

					if (memtype->RTTI == RTTI_INFANTRYTYPE
						&& static_cast<unsigned>(memtype->HeapID) < counter.size()) {
						counter[memtype->HeapID]++;
						if (val < value[memtype->HeapID]) {
							value[memtype->HeapID] = val;
						}
					}
				}
			}
		}
	}

	/*
	**	Reduce the theoretical maximum by the actual number of objects currently
	**	in play.
	*/
	for (int oindex = 0; oindex < Infantry.Count(); oindex++) {
		InfantryClass * obj = Infantry[oindex];
		if (obj != NULL && obj->Is_Recruitable(this)
			&& static_cast<unsigned>(obj->Class->HeapID) < counter.size()
			&& counter[obj->Class->HeapID] > 0) {
			counter[obj->Class->HeapID]--;
		}
	}

	/*
	**	Pick to build the most needed object but don't consider those object that
	**	can't be built because of scenario restrictions or insufficient cash.
	*/
	int bestval = -1;
	InfantryType lasttype = INFANTRY_NONE;
	int lastval = 0x7FFFFFFF;
	std::vector<InfantryType> bestlist;
	bestlist.reserve(InfantryTypes.Count());
	for (InfantryType type = InfantryType(0); type < InfantryTypes.Count(); type++) {
		if (counter[type] > 0 && Can_Build(InfantryTypes[type], false, false) && InfantryTypes[type]->Cost_Of(this) <= Available_Money()) {
			if (bestval == -1 || bestval < counter[type]) {
				bestval = counter[type];
				bestlist.clear();
			}
			bestlist.push_back(type);

			if (lasttype == INFANTRY_NONE || value[type] < lastval) {
				lasttype = type;
				lastval = value[type];
			}
		}
	}

	if (Random_Double(0, 0x7FFFFFFE) < Rule->FillEarliestTeamProbability[Difficulty] / 100.0) {
		BuildInfantry = lasttype;
	} else {
		/*
		**	The object type to build is now known. Fetch a pointer to the techno type class.
		*/
		if (!bestlist.empty()) {
			BuildInfantry = bestlist[Random_Pick(0, static_cast<int>(bestlist.size()) - 1)];
		}
	}

	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * HouseClass::AI_Aircraft -- Determines what aircraft to build next.                          *
 *                                                                                             *
 *    This routine is used to determine the general case of what aircraft to build next.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of frame to delay before calling this routine again.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int HouseClass::AI_Aircraft(void)
{
	if (BuildAircraft != AIRCRAFT_NONE) return(TICKS_PER_SECOND);

	std::vector<int> counter(AircraftTypes.Count(), 0);
	std::vector<int> value(AircraftTypes.Count(), 0x7FFFFFFF);

	/*
	**	Build a list of the maximum of each type we wish to produce. This will be
	**	twice the number required to fill all teams.
	*/
	for (int i = 0; i < Teams.Count(); i++) {
		TeamClass * tptr = Teams[i];
		if (tptr != NULL) {

			int val = tptr->CreationFrame;

			if (((tptr->Class->IsReinforcable && !tptr->IsFullStrength) || (!tptr->IsForcedActive && !tptr->IsHasBeen)) && tptr->House == this) {
				TEAM_MEMBER_LIST _members;
				tptr->Team_Members(_members);

				for (int subindex = 0; subindex < _members.Count(); subindex++) {

					AircraftTypeClass const * memtype = (AircraftTypeClass const *)_members[subindex];

					if (memtype->RTTI == RTTI_AIRCRAFTTYPE
						&& static_cast<unsigned>(memtype->HeapID) < counter.size()) {
						counter[memtype->HeapID]++;
						if (val < value[memtype->HeapID]) {
							value[memtype->HeapID] = val;
						}
					}
				}
			}
		}
	}

	/*
	**	Reduce the theoretical maximum by the actual number of objects currently
	**	in play.
	*/
	for (int oindex = 0; oindex < Aircraft.Count(); oindex++) {
		AircraftClass * obj = Aircraft[oindex];
		if (obj != NULL && obj->Is_Recruitable(this)
			&& static_cast<unsigned>(obj->Class->HeapID) < counter.size()
			&& counter[obj->Class->HeapID] > 0) {
			counter[obj->Class->HeapID]--;
		}
	}

	/*
	**	Pick to build the most needed object but don't consider those object that
	**	can't be built because of scenario restrictions or insufficient cash.
	*/
	int bestval = -1;
	AircraftType lasttype = AIRCRAFT_NONE;
	int lastval = 0x7FFFFFFF;
	std::vector<AircraftType> bestlist;
	bestlist.reserve(AircraftTypes.Count());
	for (AircraftType type = AircraftType(0); type < AircraftTypes.Count(); type++) {
		if (counter[type] > 0 && Can_Build(AircraftTypes[type], false, false) && AircraftTypes[type]->Cost_Of(this) <= Available_Money()) {
			if (bestval == -1 || bestval < counter[type]) {
				bestval = counter[type];
				bestlist.clear();
			}
			bestlist.push_back(type);

			if (lasttype == AIRCRAFT_NONE || value[type] < lastval) {
				lasttype = type;
				lastval = value[type];
			}
		}
	}

	if (Random_Double(0, 0x7FFFFFFE) < Rule->FillEarliestTeamProbability[Difficulty] / (100.0)) {
		BuildAircraft = lasttype;
	} else {
		/*
		**	The object type to build is now known. Fetch a pointer to the techno type class.
		*/
		if (!bestlist.empty()) {
			BuildAircraft = bestlist[Random_Pick(0, static_cast<int>(bestlist.size()) - 1)];
		}
	}

	return(TICKS_PER_SECOND);
}


/***********************************************************************************************
 * HouseClass::Production_Begun -- Records that production has begun.                          *
 *                                                                                             *
 *    This routine is used to inform the Expert System that production of the specified object *
 *    has begun. This allows the AI to proceed with picking another object to begin production *
 *    on.                                                                                      *
 *                                                                                             *
 * INPUT:   product  -- Pointer to the object that production has just begun on.               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Production_Begun(TechnoClass const * product)
{
	// nothing
}


/***********************************************************************************************
 * HouseClass::Tracking_Remove -- Remove object from house tracking system.                    *
 *                                                                                             *
 *    This routine informs the Expert System that the specified object is no longer part of    *
 *    this house's inventory. This occurs when the object is destroyed or captured.            *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object to remove from the tracking systems of this      *
 *                      house.                                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Tracking_Remove(TechnoClass const * techno)
{
	if (techno->TClass->IsInsignificant) return;

	switch ((RTTIType)techno->RTTI) {
		case RTTI_BUILDING:
			if (techno->Considered_Vehicle()) {
				CurUnits--;
			} else {
				CurBuildings--;
			}
			BQuantity.Decrement(techno->TClass->Fetch_Heap_ID());
			break;

		case RTTI_AIRCRAFT:
			CurAircraft--;
			AQuantity.Decrement(techno->TClass->Fetch_Heap_ID());
			break;

		case RTTI_INFANTRY:
			CurInfantry--;
			IQuantity.Decrement(techno->TClass->Fetch_Heap_ID());
			break;

		case RTTI_UNIT:
			CurUnits--;
			UQuantity.Decrement(techno->TClass->Fetch_Heap_ID());
			break;

		default:
			break;
	}
}


/***********************************************************************************************
 * HouseClass::Tracking_Add -- Informs house of new inventory item.                            *
 *                                                                                             *
 *    This function is called when the specified object is now available as part of the house's*
 *    inventory. This occurs when the object is newly produced and also when it is captured    *
 *    by this house.                                                                           *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object that is now part of the house inventory.         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Tracking_Add(TechnoClass const * techno)
{
	StructType building;
	AircraftType aircraft;
	InfantryType infantry;
	UnitType unit;

	if (techno->TClass->IsInsignificant) return;

	switch ((RTTIType)techno->RTTI) {
		case RTTI_BUILDING:
			if (techno->Considered_Vehicle()) {
				CurUnits++;
			} else {
				CurBuildings++;
			}
			building = (StructType)techno->TClass->Fetch_Heap_ID();
			BQuantity.Increment(building);
			if (Session.Type == GAME_INTERNET) {
				BuildingTotals->Increment_Unit_Total(building);
			}
			break;

		case RTTI_AIRCRAFT:
			CurAircraft++;
			aircraft = (AircraftType)techno->TClass->Fetch_Heap_ID();
			AQuantity.Increment(aircraft);
			if (Session.Type == GAME_INTERNET) {
				AircraftTotals->Increment_Unit_Total(aircraft);
			}
			break;

		case RTTI_INFANTRY:
			infantry = (InfantryType)techno->TClass->Fetch_Heap_ID();
			CurInfantry++;
			IQuantity.Increment(infantry);
			if (Session.Type == GAME_INTERNET) {
				InfantryTotals->Increment_Unit_Total(infantry);
			}
			break;

		case RTTI_UNIT:
			CurUnits++;
			unit = (UnitType)techno->TClass->Fetch_Heap_ID();
			UQuantity.Increment(unit);
			if (Session.Type == GAME_INTERNET) {
				UnitTotals->Increment_Unit_Total(unit);
			}
			break;

		default:
			break;
	}
}


/***********************************************************************************************
 * HouseClass::Factory_Counter -- Fetches a pointer to the factory counter value.              *
 *                                                                                             *
 *    Use this routine to fetch a pointer to the variable that holds the number of factories   *
 *    that can produce the specified object type. This is a helper routine used when           *
 *    examining the number of factories as well as adjusting their number.                     *
 *                                                                                             *
 * INPUT:   rtti  -- The RTTI of the object that could be produced.                            *
 *                                                                                             *
 * OUTPUT:  Returns with the number of factories owned by this house that could produce the    *
 *          object of the type specified.                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int * HouseClass::Factory_Counter(RTTIType rtti)
{
	switch (rtti) {
		case RTTI_UNITTYPE:
		case RTTI_UNIT:
			return(&UnitFactories);

		case RTTI_AIRCRAFTTYPE:
		case RTTI_AIRCRAFT:
			return(&AircraftFactories);

		case RTTI_INFANTRYTYPE:
		case RTTI_INFANTRY:
			return(&InfantryFactories);

		case RTTI_BUILDINGTYPE:
		case RTTI_BUILDING:
			return(&BuildingFactories);

		default:
			break;
	}
	return(NULL);
}


/***********************************************************************************************
 * HouseClass::Active_Remove -- Remove this object from active duty for this house.            *
 *                                                                                             *
 *    This routine will recognize the specified object as having been removed from active      *
 *    duty.                                                                                    *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object to remove from active duty.                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/16/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Active_Remove(TechnoClass const * techno)
{
	if (techno->RTTI == RTTI_BUILDING) {
		int * fptr = Factory_Counter(((BuildingClass *)techno)->Class->ToBuild);
		if (fptr != NULL) {
			*fptr = *fptr - 1;
		}
	}
}


/***********************************************************************************************
 * HouseClass::Active_Add -- Add an object to active duty for this house.                      *
 *                                                                                             *
 *    This routine will recognize the specified object as having entered active duty. Any      *
 *    abilities granted to the house by that object are now available.                         *
 *                                                                                             *
 * INPUT:   techno   -- Pointer to the object that is entering active duty.                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/16/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Active_Add(TechnoClass const * techno)
{
	if (techno->RTTI == RTTI_BUILDING) {
		int * fptr = Factory_Counter(((BuildingClass *)techno)->Class->ToBuild);
		if (fptr != NULL) {
			*fptr = *fptr + 1;
		}
	}
}


/***********************************************************************************************
 * HouseClass::Which_Zone -- Determines what zone a coordinate lies in.                        *
 *                                                                                             *
 *    This routine will determine what zone the specified coordinate lies in with respect to   *
 *    this house's base. A location that is too distant from the base, even though it might    *
 *    be a building, is not considered part of the base and returns ZONE_NONE.                 *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to examine.                                                *
 *                                                                                             *
 * OUTPUT:  Returns with the base zone that the specified coordinate lies in.                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ZoneType HouseClass::Which_Zone(Coord const & coord) const
{
	if (coord == COORD_NONE) return(ZONE_NONE);

	int distance = Distance(Center, coord);
	if (distance <= Radius) return(ZONE_CORE);
	if (distance > Radius*4) return(ZONE_NONE);

	Dir256 facing = Direction(Center, coord).As_Dir256();
	/// Both tests are inverted. ZONE_NORTH claims everything except the north wedge, and
	/// the ZONE_SOUTH arm can never be true at all.
	if (facing > DIR_NE && facing < DIR_NW) return(ZONE_NORTH);
	if (facing > DIR_SW && facing < DIR_SE) return(ZONE_SOUTH);
	if (facing > DIR_NE || facing >= DIR_SE) return(ZONE_WEST);
	return(ZONE_EAST);
}


/***********************************************************************************************
 * HouseClass::Which_Zone -- Determines which base zone the specified object lies in.          *
 *                                                                                             *
 *    Use this routine to determine what zone the specified object lies in.                    *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that will be checked for zone occupation.        *
 *                                                                                             *
 * OUTPUT:  Returns with the base zone that the object lies in. For objects that are too       *
 *          distant from the center of the base, ZONE_NONE is returned.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ZoneType HouseClass::Which_Zone(ObjectClass const * object) const
{
	if (!object) return(ZONE_NONE);
	return(Which_Zone(object->Center_Coord()));
}


/***********************************************************************************************
 * HouseClass::Which_Zone -- Determines which base zone the specified cell lies in.            *
 *                                                                                             *
 *    This routine is used to determine what base zone the specified cell is in.               *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine.                                                      *
 *                                                                                             *
 * OUTPUT:  Returns the base zone that the cell lies in or ZONE_NONE if the cell is too far    *
 *          away.                                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ZoneType HouseClass::Which_Zone(Cell const & cell) const
{
	return(Which_Zone((Coord)cell));
}


/***********************************************************************************************
 * HouseClass::Zone_Cell -- Finds the cell closest to the center of the zone.                  *
 *                                                                                             *
 *    This routine is used to find the cell that is closest to the center point of the         *
 *    zone specified. Typical use of this routine is for building and unit placement so that   *
 *    they can "cover" the specified zone.                                                     *
 *                                                                                             *
 * INPUT:   zone  -- The zone that the center point is to be returned.                         *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that is closest to the center point of the zone specified.   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell HouseClass::Zone_Cell(ZoneType zone) const
{
	switch (zone) {
		case ZONE_CORE:
			return(Center.As_Cell());

		case ZONE_NORTH:
			return(Move_Coord(Center, DIR_N, Radius*3).As_Cell());

		case ZONE_EAST:
			return(Move_Coord(Center, DIR_E, Radius*3).As_Cell());

		case ZONE_WEST:
			return(Move_Coord(Center, DIR_W, Radius*3).As_Cell());

		case ZONE_SOUTH:
			return(Move_Coord(Center, DIR_S, Radius*3).As_Cell());

		default:
			break;
	}
	return(CELL_NONE);
}


/***********************************************************************************************
 * HouseClass::Where_To_Go -- Determines where the object should go and wait.                  *
 *                                                                                             *
 *    This function is called for every new unit produced or delivered in order to determine   *
 *    where the unit should "hang out" to await further orders. The best area for the          *
 *    unit to loiter is returned as a cell location.                                           *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that needs to know where to go.                  *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that the unit should move to.                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/02/1995 JLB : Created.                                                                 *
 *   11/04/1996 JLB : Simplified to use helper functions                                       *
 *=============================================================================================*/
Cell HouseClass::Where_To_Go(FootClass const * object) const
{
	assert(object != NULL);

	ZoneType zone;			// The zone that the object should go to.
	if (object->Anti_Air() + object->Anti_Armor() + object->Anti_Infantry() == 0) {
		zone = ZONE_CORE;
	} else {
		zone = Random_Pick(ZONE_NORTH, ZONE_WEST);
	}

	Cell cell = Random_Cell_In_Zone(zone);
	assert(cell != CELL_NONE);

	return(Map.Nearby_Location(cell, SPEED_TRACK, Map.Get_Cell_Zone(object->PositionCoord.As_Cell())));
}


/***********************************************************************************************
 * HouseClass::Find_Juicy_Target -- Finds a suitable field target.                             *
 *                                                                                             *
 *    This routine is used to find targets out in the field and away from base defense.        *
 *    Typical of this would be the attack helicopters and the roving attack bands of           *
 *    hunter killers.                                                                          *
 *                                                                                             *
 * INPUT:   coord -- The coordinate of the attacker. Closer targets are given preference.      *
 *                                                                                             *
 * OUTPUT:  Returns with a suitable target to attack.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AbstractClass * HouseClass::Find_Juicy_Target(Coord const & coord) const
{
	UnitClass * best = 0;
	int value = 0;

	for (int index = 0; index < Units.Count(); index++) {
		UnitClass * unit = Units[index];

		if (unit && !unit->IsInLimbo && !Is_Ally(unit) && unit->House->Which_Zone(unit) == ZONE_NONE && unit->HeightAGL >= -20 && unit->Cloak != CLOAKED) {
			int val = coord.Distance_To(unit->Center_Coord());

			if (unit->Anti_Air()) val *= 2;

			if (value == 0 || val < value) {
				value = val;
				best = unit;
			}
		}
	}
	if (best) {
		return(best);
	}
	return(NULL);
}


/***********************************************************************************************
 * HouseClass::Fetch_Factory -- Finds the factory associated with the object type specified.   *
 *                                                                                             *
 *    This is the counterpart to the Set_Factory function. It will return with a factory       *
 *    pointer that is associated with the object type specified.                               *
 *                                                                                             *
 * INPUT:   rtti  -- The RTTI of the object type to find the factory for.                      *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the factory (if present) that can manufacture the        *
 *          object type specified.                                                             *
 *                                                                                             *
 * WARNINGS:   If this returns a non-NULL pointer, then the factory is probably already busy   *
 *             producing another unit of that category.                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
FactoryClass * HouseClass::Fetch_Factory(RTTIType rtti) const
{
	FactoryClass * factory;

	switch (rtti) {
		case RTTI_INFANTRY:
		case RTTI_INFANTRYTYPE:
			factory = InfantryFactory;
			break;

		case RTTI_UNIT:
		case RTTI_UNITTYPE:
			factory = UnitFactory;
			break;

		case RTTI_BUILDING:
		case RTTI_BUILDINGTYPE:
			factory = BuildingFactory;
			break;

		case RTTI_AIRCRAFT:
		case RTTI_AIRCRAFTTYPE:
			factory = AircraftFactory;
			break;

		default:
			factory = NULL;
			break;
	}
	return(factory);
}


/***********************************************************************************************
 * HouseClass::Set_Factory -- Assign specified factory to house tracking.                      *
 *                                                                                             *
 *    Call this routine when a factory has been created and it now must be passed on to the    *
 *    house for tracking purposes. The house maintains several factory pointers and this       *
 *    routine will ensure that the factory pointer gets stored correctly.                      *
 *                                                                                             *
 * INPUT:   rtti  -- The RTTI of the object the factory it to manufacture.                     *
 *                                                                                             *
 *          factory  -- The factory object pointer.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Set_Factory(RTTIType rtti, FactoryClass * factory)
{
	assert(rtti != RTTI_NONE);

	switch (rtti) {
		case RTTI_UNIT:
		case RTTI_UNITTYPE:
			UnitFactory = factory;
			break;

		case RTTI_INFANTRY:
		case RTTI_INFANTRYTYPE:
			InfantryFactory = factory;
			break;

		case RTTI_BUILDING:
		case RTTI_BUILDINGTYPE:
			BuildingFactory = factory;
			break;

		case RTTI_AIRCRAFT:
		case RTTI_AIRCRAFTTYPE:
			AircraftFactory = factory;
			break;
	}
}


/***********************************************************************************************
 * HouseClass::Factory_Count -- Fetches the number of factories for specified type.            *
 *                                                                                             *
 *    This routine will count the number of factories owned by this house that can build       *
 *    objects of the specified type.                                                           *
 *                                                                                             *
 * INPUT:   rtti  -- The type of object (RTTI) that the factories are to be counted for.       *
 *                                                                                             *
 * OUTPUT:  Returns with the number of factories that can build the object type specified.     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/30/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int HouseClass::Factory_Count(RTTIType rtti) const
{
	int const * ptr = ((HouseClass *)this)->Factory_Counter(rtti);
	if (ptr != NULL) {
		return(*ptr);
	}
	return(0);
}


/// <summary>
/// Creates and initializes every house listed in the INI database.
/// This routine is called as a scenario starts up. Each house is created, given its scenario
/// data, and -- in a solo mission -- handicapped according to the chosen difficulty.
/// </summary>
/// <param name="ini">The INI database to read the houses from.</param>
void HouseClass::Read_All(CCINIClass const & ini)
{
	HouseClass 	* p;				// Pointer to current player data.
	HousesType index;

	int count = ini.Entry_Count("Houses");

	for (index = HOUSE_FIRST; index < count; index++) {
		/// Reading the entry is what forces the house type to be created. The house below is
		/// built from the type at the same index, which need not be the one just read.
		ini.Get_HousesType("Houses", ini.Get_Entry("Houses", index), HOUSE_NONE);
		new HouseClass(HouseTypes[index]);
	}

	for (index = HOUSE_FIRST; index < Houses.Count(); index++) {
		p = Houses[index];
		p->Read_INI(ini);
		if (Session.Type == GAME_NORMAL) {
			if (p->Is_Human_Player()) {
				p->Assign_Handicap(Scen->Difficulty);
			} else {
				p->Assign_Handicap(Scen->CDifficulty);
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Read_INI -- Reads house specific data from INI.                                 *
 *                                                                                             *
 *    This routine reads the house specific data for a particular                              *
 *    scenario from the scenario INI file. Typical data includes starting                      *
 *    credits, maximum unit count, etc.                                                        *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to loaded scenario INI file.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/24/1994 JLB : Created.                                                                 *
 *   05/18/1995 JLB : Creates all houses.                                                      *
 *=============================================================================================*/
void HouseClass::Read_INI(CCINIClass const & ini)
{
	char const	* hname;			// Pointer to house name.

	hname = Class->Name();

	Control.TechLevel = ini.Get_Int(hname, "TechLevel", Scen->Scenario);
	Control.InitialCredits = ini.Get_Int(hname, "Credits", 0) * 100;
	Credits = Control.InitialCredits;

	RatioAITriggerTeam = ini.Get_Int(hname, "RatioAITriggerTeam", RatioAITriggerTeam);
	RatioTeamAircraft = ini.Get_Int(hname, "RatioTeamAircraft", 75);
	RatioTeamInfantry = ini.Get_Int(hname, "RatioTeamInfantry", 75);
	RatioTeamUnits = ini.Get_Int(hname, "RatioTeamUnits", 75);

	ActLike = (HousesType)ini.Get_Int(hname, "ActsLike", ActLike);
	if (ActLike == HOUSE_NONE) {
		ActLike = HOUSE_FIRST;
	}

	int iq = ini.Get_Int(hname, "IQ", 0);
	if (iq > Rule->MaxIQ) iq = 1;
	IQ = Control.IQ = iq;

	Control.Edge = ini.Get_SourceType(hname, "Edge", SOURCE_NORTH);
	IsPlayerControl = ini.Get_Bool(hname, "PlayerControl", false);

	int owners = ini.Get_Owners(hname, "Allies", Allies);
	Make_Ally(Houses[HeapID]);

	Scheme = ini.Get_Scheme_Index(hname, "Color", Scheme);

	Initialize_Radar_Color();

	Base.Read_INI(ini, hname);
	Base.House = this;

	//Make_Ally(HOUSE_NEUTRAL);
	for (HousesType h = HOUSE_FIRST; h < Houses.Count(); h++) {
		HouseClass * hptr = Houses[h];
		if ((owners & (1 << hptr->Class->House)) != 0) {
			Make_Ally(hptr);
		}
	}

	TeamTime = 175 * HeapID + Rule->TeamDelays[Difficulty];
}


/// <summary>
/// Writes every house out to the INI database.
/// This routine records the roster of houses and then lets each one write its own scenario
/// data. It is the counterpart of Read_All.
/// </summary>
/// <param name="ini">The INI database to write the houses to.</param>
void HouseClass::Write_All(CCINIClass & ini)
{
	char buffer[32];
	HousesType index;

	ini.Clear("Houses");

	for (index = HOUSE_FIRST; index < Houses.Count(); index++) {
		sprintf(buffer, "%d", index);
		ini.Put_HousesType("Houses", buffer, Houses[index]->Class->House);
	}

	for (index = HOUSE_FIRST; index < Houses.Count(); index++) {
		Houses[index]->Write_INI(ini);
	}
}


/***********************************************************************************************
 * HouseClass::Write_INI -- Writes the house data to the INI database.                         *
 *                                                                                             *
 *    This routine will write out all data necessary to recreate it in anticipation of a       *
 *    new scenario. All houses (that are active) will have their scenario type data written    *
 *    out.                                                                                     *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to write the data to.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/09/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Write_INI(CCINIClass & ini)
{
	char const * name = Class->IniName;

	ini.Clear(name);

	ini.Put_Int(name, "Credits", (int)(Control.InitialCredits / 100));
	ini.Put_SourceType(name, "Edge", Control.Edge);
	ini.Put_Int(name, "ActsLike", ActLike);
	ini.Put_Int(name, "TechLevel", Control.TechLevel);
	ini.Put_Int(name, "IQ", Control.IQ);
	ini.Put_Bool(name, "PlayerControl", IsPlayerControl);

	unsigned allies = 0;
	for (HousesType index = HOUSE_FIRST; index < Houses.Count(); index++) {
		if ((Control.Allies & (1 << Houses[index]->HeapID)) != 0) {
			allies |= (1 << Houses[index]->Class->House);
		}
	}
	ini.Put_Owners(name, "Allies", allies);

	ini.Put_Scheme_Index(name, "Color", Scheme);
	if (Class->Side != SIDE_NONE) {
		ini.Put_Side(name, "Side", Class->Side);
	}
	Base.Write_INI(ini, name);
}


/***********************************************************************************************
 * HouseClass::Fire_Sale -- Cause all buildings to be sold.                                    *
 *                                                                                             *
 *    This routine will sell back all buildings owned by this house.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was a fire sale performed?                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Fire_Sale(void)
{
	if (CurBuildings > 0) {
		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * b = Buildings[index];

			if (b != NULL && !b->IsInLimbo && b->House == this && b->Strength > 0) {
				b->Sell_Back(1);
			}
		}
		return(false);
	}
	return(true);
}


/***********************************************************************************************
 * HouseClass::Do_All_To_Hunt -- Send all units to hunt.                                       *
 *                                                                                             *
 *    This routine will cause all combatants of this house to go into hunt mode. The effect of *
 *    this is to throw everything this house has to muster at the enemies of this house.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/23/1996 JLB : Created.                                                                 *
 *   10/02/1996 JLB : Handles aircraft too.                                                    *
 *=============================================================================================*/
void HouseClass::All_To_Hunt(void)
{
	int index;

	for (index = 0; index < Technos.Count(); index++) {
		TechnoClass * techno = Technos[index];

		if (techno->Is_Foot() && this == techno->House && techno->IsDown && !techno->IsInLimbo) {
			if (((FootClass *)techno)->Team) ((FootClass *)techno)->Team->Remove(((FootClass *)techno));
			techno->Assign_Mission(MISSION_HUNT);
		}
	}

	IsAllToHunt = true;
}


/***********************************************************************************************
 * HouseClass::Is_Allowed_To_Ally -- Determines if this house is allied to make allies.        *
 *                                                                                             *
 *    Use this routine to determine if this house is legally allowed to ally with the          *
 *    house specified. There are many reason why an alliance is not allowed. Typically, this   *
 *    is when there would be no more opponents left to fight or if this house has been         *
 *    defeated.                                                                                *
 *                                                                                             *
 * INPUT:   house -- The house that alliance with is desired.                                  *
 *                                                                                             *
 * OUTPUT:  bool; Is alliance with the house specified prohibited?                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool HouseClass::Is_Allowed_To_Ally(HouseClass * house) const
{
	if (house != NULL) {
		/*
		**	One cannot ally twice with the same house.
		*/
		if (Is_Ally(house)) {
			return(false);
		}
	}

	/*
	**	If the scenario is being set up, then alliances are always
	**	allowed. No further checking is required.
	*/
	if (ScenarioInit) {
		return(true);
	}

	/*
	**	When the house is defeated, it can no longer make alliances.
	*/
	if (IsDefeated) {
		return(false);
	}

	// An observer takes no side.
	if (house != NULL && house->IsObserver) {
		return(false);
	}

	/*
	**	Count the number of active houses in the game as well as the
	**	number of existing allies with this house.
	*/
	int housecount = 0;
	int allycount = 0;
	for (HousesType house2 = HOUSE_FIRST; house2 < Houses.Count(); house2++) {
		HouseClass * hptr = Houses[house2];
		if (!hptr->IsDefeated && !hptr->Class->IsMultiplayPassive) {
			housecount++;
			if (Is_Ally(hptr)) {
				allycount++;
			}
		}
	}

	/*
	**	Alliance is not allowed if there wouldn't be any enemies left to
	**	fight.
	*/
	if (housecount == allycount+1) {
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * HouseClass::Computer_Paranoid -- Cause the computer players to becom paranoid.              *
 *                                                                                             *
 *    This routine will cause the computer players to become suspicious of the human           *
 *    players and thus the computer players will band together in order to defeat the          *
 *    human players.                                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Computer_Paranoid(void)
{
	/*
	**	Loop through every computer controlled house and make allies with all other computer
	**	controlled houses and then make enemies with all other human controlled houses.
	*/
	for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {
		HouseClass * hptr = Houses[house];
		if (hptr != NULL && !hptr->IsDefeated && !hptr->Is_Human_Player()) {
			hptr->IsParanoid = true;

			/*
			**	Break alliance with every human it is allied with and make friends with
			**	any other computer players.
			*/
			for (HousesType house2 = HOUSE_FIRST; house2 < Houses.Count(); house2++) {
				HouseClass * hptr2 = Houses[house2];
				if (hptr2 != NULL && !hptr2->IsDefeated) {
					if (hptr2->Is_Human_Player()) {
						hptr->Make_Enemy(hptr2);
					} else {
						hptr->Make_Ally(hptr2);
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Adjust_Power -- Adjust the power value of the house.                            *
 *                                                                                             *
 *    This routine will update the power output value of the house. It will cause any buildgins*
 *    that need to be redrawn to do so.                                                        *
 *                                                                                             *
 * INPUT:   adjust   -- The amount to adjust the power output value.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Adjust_Power(int adjust)
{
	if (!GameActive) return;

	Power += adjust;
	Update_Spied_Power_Plants();
}


/***********************************************************************************************
 * HouseClass::Adjust_Drain -- Adjust the power drain value of the house.                      *
 *                                                                                             *
 *    This routine will update the drain value of the house. It will cause any buildings that  *
 *    need to be redraw to do so.                                                              *
 *                                                                                             *
 * INPUT:   adjust   -- The amount to adjust the drain (positive means more drain).            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Adjust_Drain(int adjust)
{
	Drain += adjust;
	Update_Spied_Power_Plants();
}


/***********************************************************************************************
 * HouseClass::Update_Spied_Power_Plants -- Redraw power graphs on spied-upon power plants.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/11/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
void HouseClass::Update_Spied_Power_Plants(void)
{
	int count = CurrentObject.Count();
	if (count) {
		for (int index = 0; index < count; index++) {
			ObjectClass const * tech = CurrentObject[index];
			if (tech && tech->RTTI==RTTI_BUILDING) {
				BuildingClass *bldg = (BuildingClass *)tech;
				if (!bldg->IsOwnedByPlayer && bldg->Class->Power > 0) {
					if ( bldg->SpiedBy & (1<<(PlayerPtr->Class->House)) ) {
						bldg->Mark(MARK_CHANGE);
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * HouseClass::Find_Cell_In_Zone -- Finds a legal placement cell within the zone.              *
 *                                                                                             *
 *    Use this routine to determine where the specified object should go if it were to go      *
 *    some random (but legal) location within the zone specified.                              *
 *                                                                                             *
 * INPUT:   techno   -- The object that is desirous of going into the zone specified.          *
 *                                                                                             *
 *          zone     -- The zone to find a location within.                                    *
 *                                                                                             *
 * OUTPUT:  Returns with the cell that the specified object could be placed in the zone. If    *
 *          no valid location could be found, then 0 is returned.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/01/1996 JLB : Created.                                                                 *
 *   11/04/1996 JLB : Not so strict on zone requirement.                                       *
 *=============================================================================================*/
Cell HouseClass::Find_Cell_In_Zone(TechnoClass const * techno, ZoneType zone) const
{
	return(Cell(0, 0));
}


/// <summary>
/// Rotates a direction back by one 45 degree facing.
/// The position within the facing is preserved, so a direction picked anywhere inside a
/// compass quadrant comes back shifted such that the quadrant is centered upon its
/// compass direction.
/// </summary>
/// <param name="dir">The direction to rotate.</param>
/// <returns>Returns with the direction rotated back one facing.</returns>
static DirType Rotate_Back_One_Facing(DirType const & dir)
{
	int fraction = dir.Facing & ((DIR_STEP_8 << 8) - 1);
	DirType sector(dir);
	DirType back(sector.Snap_To_8().Raw - 1);
	return(DirType(back.Snap_To_8().Facing ^ fraction));
}


/***********************************************************************************************
 * HouseClass::Random_Cell_In_Zone -- Find a (technically) legal cell in the zone specified.   *
 *                                                                                             *
 *    This routine will pick a random cell within the zone specified. The pick will be         *
 *    clipped to the map edge when necessary.                                                  *
 *                                                                                             *
 * INPUT:   zone  -- The zone to pick a cell from.                                             *
 *                                                                                             *
 * OUTPUT:  Returns with a picked cell within the zone. If the entire zone lies outside of the *
 *          map, then a cell in the core zone is returned instead.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/04/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
Cell HouseClass::Random_Cell_In_Zone(ZoneType zone) const
{
	Coord coord;
	int maxdist = 0;

	int radius = std::max(3 * CELL_LEPTON, std::min(Radius, 8 * CELL_LEPTON));
	maxdist = radius*2;

	switch (zone) {
		case ZONE_CORE:
			coord = Coord_Scatter(Center, Random_Pick(0, radius), true);
			break;

		/*
		 * Each edge zone picks a raw 16 bit facing, forces it into the matching
		 * compass quadrant, then rotates back 45 degrees so that the quadrant is
		 * centered upon the compass direction (e.g., north covers NW..NE). Each
		 * quadrant clears the top bits that must be zero and sets the ones that
		 * must be one, so north needs no set and west needs no clear.
		 */
		case ZONE_NORTH: {
			int distance = Random_Pick(radius, maxdist);
			DirType dir(Scen->RandomNumber());
			dir.Raw &= ~(DIR_W << 8);
			coord = Move_Coord(Center, Rotate_Back_One_Facing(dir), distance);
			break;
		}

		case ZONE_EAST: {
			int distance = Random_Pick(radius, maxdist);
			DirType dir(Scen->RandomNumber());
			dir.Raw &= ~(DIR_S << 8);
			dir.Raw |= DIR_E << 8;
			coord = Move_Coord(Center, Rotate_Back_One_Facing(dir), distance);
			break;
		}

		case ZONE_SOUTH: {
			int distance = Random_Pick(radius, maxdist);
			DirType dir(Scen->RandomNumber());
			dir.Raw &= ~(DIR_E << 8);
			dir.Raw |= DIR_S << 8;
			coord = Move_Coord(Center, Rotate_Back_One_Facing(dir), distance);
			break;
		}

		case ZONE_WEST: {
			int distance = Random_Pick(radius, maxdist);
			DirType dir(Scen->RandomNumber());
			dir.Raw |= DIR_W << 8;
			coord = Move_Coord(Center, Rotate_Back_One_Facing(dir), distance);
			break;
		}
	}

	/*
	**	Double check that the location is valid and if so, convert it into a cell
	**	number.
	*/
	Cell cell;
	if (coord == COORD_NONE /*|| !Map.In_Radar(coord.As_Cell())*/) {
		if (zone == ZONE_CORE) {

			/*
			**	Finding a cell within the core failed, so just pick the center
			**	cell. This cell is guaranteed to be valid.
			*/
			cell = Center.As_Cell();
		} else {

			/*
			**	If the edge fails, then try to find a cell within the core.
			*/
			cell = Random_Cell_In_Zone(ZONE_CORE);
		}
	} else {
		cell = coord.As_Cell();
	}

	/*
	**	If the randomly picked location is not in the legal map area, then clip it to
	**	the legal map area.
	*/
	if (!Map.In_Local_Radar(cell)) {
		cell = Map.Clip_To_Map(cell);
	}
	return(cell);
}


/// <summary>
/// Makes an ally of the object's owner.
/// This is a convenience for the cases where the house to befriend is known only through one
/// of the objects that belong to it.
/// </summary>
void HouseClass::Make_Ally(ObjectClass * object)
{
	if (object) {
		Make_Ally(object->Owner_HouseClass());
	}
}


/// <summary>
/// Makes an enemy of the object's owner.
/// This is a convenience for the cases where the offending house is known only through one
/// of the objects that belong to it.
/// </summary>
void HouseClass::Make_Enemy(ObjectClass * object)
{
	if (object) {
		Make_Enemy(object->Owner_HouseClass());
	}
}


/// <summary>
/// Places a waypoint at the coordinate specified.
/// The waypoint is appended to the path this house is currently editing. Nothing happens if
/// the cell has already been claimed by another path.
/// </summary>
void HouseClass::Place_Waypoint(Coord const & coord)
{
	if (Can_Place_Waypoint(coord.As_Cell())) {
		Ensure_Path(SelectedPath);
		Paths[SelectedPath]->Add_Waypoint(coord);
	}
}


/// <summary>
/// Selects the waypoint at the coordinate specified.
/// This routine is used by the waypoint plotting controls, and only considers the path this
/// house is currently editing.
/// </summary>
/// <returns>bool; Was a waypoint selected?</returns>
bool HouseClass::Select_Waypoint(Coord const & coord)
{
	Ensure_Path(SelectedPath);
	return(Paths[SelectedPath]->Select_Waypoint(coord));
}


/// <summary>
/// May a waypoint be placed at the cell specified?
/// A cell may hold only one waypoint, although the currently selected path is free to
/// revisit a cell of its own.
/// </summary>
/// <returns>bool; Is the cell free for a waypoint?</returns>
bool HouseClass::Can_Place_Waypoint(Cell const & cell)
{
	for (PathType path = PATH_FIRST; path < PATH_COUNT; path++) {
		Ensure_Path(path);
		for (int index = 0; index < Paths[path]->Waypoint_Count(); index++) {
			WaypointClass * wp = Paths[path]->Get_Waypoint(index);
			if (wp->Location.As_Cell() == cell && path != SelectedPath) {
				return(false);
			}
		}
	}
	return(true);
}


/// <summary>
/// Fetches the waypoint located at the cell specified.
/// This routine is used by the waypoint plotting controls to determine what it is that the
/// player just clicked on.
/// </summary>
/// <returns>Returns with a pointer to the waypoint found. Otherwise, NULL is
/// returned.</returns>
WaypointClass * HouseClass::Waypoint_At(Cell const & cell)
{
	for (PathType path = PATH_FIRST; path < PATH_COUNT; path++) {
		Ensure_Path(path);
		for (int index = 0; index < Paths[path]->Waypoint_Count(); index++) {
			WaypointClass * waypt = Paths[path]->Get_Waypoint(index);
			if (waypt->Location.As_Cell() == cell) {
				return(waypt);
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the path that a waypoint belongs to.
/// This routine is used when a waypoint object is in hand but its place among the house's
/// waypoint paths is not known -- the display code needs both in order to label it.
/// </summary>
/// <param name="waypt">The waypoint to look up.</param>
/// <param name="xpath">Reference to fill in with the path that owns the waypoint.</param>
/// <param name="waypt_id">Reference to fill in with the waypoint's position along that
/// path.</param>
/// <returns>bool; Was the waypoint found on one of the paths?</returns>
bool HouseClass::Fetch_Waypoint_Data(WaypointClass * waypt, PathType & xpath, char & waypt_id)
{
	for (PathType path = PATH_FIRST; path < PATH_COUNT; path++) {
		Ensure_Path(path);
		for (int index = 0; index < Paths[path]->Waypoint_Count(); index++) {
			if (Paths[path]->Get_Waypoint(index) == waypt) {
				xpath = path;
				waypt_id = index;
				return(true);
			}
		}
	}
	xpath = PATH_NONE;
	waypt_id = 0;
	return(false);
}


/// <summary>
/// Stops tracking an object as one of this house's active possessions.
/// This routine is called when an object leaves this house's control. A building also takes
/// its power drain and storage capacity with it, and the tiberium it was holding is banked
/// rather than lost when the building was taken by capture.
/// </summary>
/// <param name="techno">The object to stop tracking.</param>
/// <param name="bycapture">Was this object lost to a capture?</param>
void HouseClass::Tracking_Active_Remove(TechnoClass * techno, bool bycapture)
{
	TechnoTypeClass const * ttype;

	switch (techno->Fetch_RTTI()) {
		case RTTI_BUILDING:
			ttype = techno->TClass;
			ABQuantity.Decrement(ttype->Fetch_Heap_ID());

			if (techno != NULL) {
				BuildingClass * bptr = (BuildingClass *)techno;

				RecalcPower = true;
				Capacity -= bptr->Class->Capacity;
				if (bycapture) {
					StorageClass *storage = &bptr->Storage;
					int i = storage->First_Used_Slot();
					while (i != -1) {
						int amount = storage->Decrease_Amount(0x7FFFFFFF, i);
						Harvested(amount, TiberiumType(i));
						i = storage->First_Used_Slot();
					}
				} else {
					Tiberium -= bptr->Storage;
				}
			}
			break;

		case RTTI_AIRCRAFT:
			ttype = techno->TClass;
			AAQuantity.Decrement(ttype->Fetch_Heap_ID());
			break;

		case RTTI_INFANTRY:
			ttype = techno->TClass;
			AIQuantity.Decrement(ttype->Fetch_Heap_ID());
			break;

		case RTTI_UNIT:
			ttype = techno->TClass;
			AUQuantity.Decrement(ttype->Fetch_Heap_ID());
			break;
	}
}


/// <summary>
/// Begins tracking an object as one of this house's active possessions.
/// This routine is called when an object comes under this house's control. A building also
/// hands over its power drain, its storage capacity, and whatever tiberium it is holding.
/// </summary>
/// <param name="techno">The object to begin tracking.</param>
void HouseClass::Tracking_Active_Add(TechnoClass * techno, bool bycapture)
{
	TechnoTypeClass const * ttype;
	BuildingClass * bptr;

	switch (techno->Fetch_RTTI()) {
		case RTTI_BUILDING:
			ttype = techno->TClass;
			ABQuantity.Increment(ttype->Fetch_Heap_ID());
			bptr = dynamic_cast<BuildingClass *>(techno);
			if (bptr != NULL) {
				Adjust_Drain(bptr->Power_Drain());
				Capacity += bptr->Class->Capacity;
				Tiberium += techno->Storage;
			}
			break;

		case RTTI_AIRCRAFT:
			ttype = techno->TClass;
			AAQuantity.Increment(ttype->Fetch_Heap_ID());
			break;

		case RTTI_INFANTRY:
			if (!((InfantryClass *)techno)->IsTechnician) {
				ttype = techno->TClass;
				AIQuantity.Increment(ttype->Fetch_Heap_ID());
			}
			break;

		case RTTI_UNIT:
			ttype = techno->TClass;
			AUQuantity.Increment(ttype->Fetch_Heap_ID());
			break;
	}
}


/***********************************************************************************************
 * HouseClass::As_Pointer -- Converts a house number into a house object pointer.              *
 *                                                                                             *
 *    Use this routine to convert a house number into the house pointer that it represents.    *
 *    A simple index into the Houses template array is not sufficient, since the array order   *
 *    is arbitrary. An actual scan through the house object is required in order to find the   *
 *    house object desired.                                                                    *
 *                                                                                             *
 * INPUT:   house -- The house type number to look up.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the house object that the house number represents.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
HouseClass * House_From_HousesType(HousesType house)
{
	for (int index = 0; index < Houses.Count(); index++) {
		HouseClass * housep = Houses[index];
		if (housep->Class->House == house) {
			return(housep);
		}
	}
	return(NULL);
}


/// <summary>
/// Adds this house's state to the game checksum.
/// This routine serves the multiplayer sync checker, which compares each machine's checksum
/// in order to detect when the simulations have drifted apart.
/// </summary>
/// <param name="crc">The checksum engine to submit the house data to.</param>
void HouseClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Difficulty);
	crc(Class->House);
	crc(FirepowerBias);
	crc(GroundspeedBias);
	crc(AirspeedBias);
	crc(ArmorBias);
	crc(ROFBias);
	crc(CostBias);
	crc(BuildSpeedBias);
	crc(RepairDelay);
	crc(BuildDelay);
	crc(IsHuman);
	crc(IsStarted);
	crc(IsAlerted);
	crc(IsAITriggersOn);
	crc(IsDefeated);
	crc(IsObserver);
	crc(IQ);
	crc(State);
	crc(Blockage);
	crc((int)RepairTimer);
	crc((int)AlertTime);
	crc((int)BorrowedTime);
	crc(CurUnits);
	crc(CurBuildings);
	crc(CurInfantry);
	crc(CurAircraft);
	crc((int)Credits);
	crc((int)Capacity);
	crc(AircraftFactories);
	crc(InfantryFactories);
	crc(UnitFactories);
	crc(BuildingFactories);
	crc(Power);
	crc(Drain);
	crc(WhoLastHurtMe);
	crc(Enemy);
	Base.Compute_CRC(crc);
}


/// <summary>
/// Loads this house from the data stream.
/// The super weapons belong to the session that created this house rather than to the
/// record, so they are disposed of before the saved members are read over the top of them.
/// </summary>
/// <param name="stream">The stream to read the house from.</param>
/// <returns>Returns with S_OK, or the failure code reported by the stream.</returns>
HRESULT STDMETHODCALLTYPE HouseClass::Load(IStream *stream)
{
	while (SuperWeapon.Count()) {
		delete SuperWeapon[0];
		SuperWeapon.Delete_Index(0);
	}

	return(Load_Members(stream));
}


/// <summary>
/// Lists the members this house carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void HouseClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Class);
	stream.Serialize(HouseTags);
	stream.Serialize(ConYards);
	stream.Serialize(Difficulty);
	stream.Serialize(FirepowerBias);
	stream.Serialize(GroundspeedBias);
	stream.Serialize(AirspeedBias);
	stream.Serialize(ArmorBias);
	stream.Serialize(ROFBias);
	stream.Serialize(CostBias);
	stream.Serialize(BuildSpeedBias);
	stream.Serialize(RepairDelay);
	stream.Serialize(BuildDelay);
	stream.Serialize(Control);
	stream.Serialize(ProductionMode);
	stream.Serialize(ActLike);
	stream.Serialize(IsHuman);
	stream.Serialize(IsPlayerControl);
	stream.Serialize(IsStarted);
	stream.Serialize(IsAlerted);
	stream.Serialize(IsAITriggersOn);
	stream.Serialize(IsBaseBuilding);
	stream.Serialize(IsDiscovered);
	stream.Serialize(IsDefeated);
	stream.Serialize(IsObserver);
	stream.Serialize(IsToDie);
	stream.Serialize(IsToWin);
	stream.Serialize(IsToLose);
	stream.Serialize(IsCivEvacuated);
	stream.Serialize(FirestormDefenseActivated);
	stream.Serialize(IsThreatRatingNodeActive);
	stream.Serialize(IsRecalcNeeded);
	stream.Serialize(IPAddress);
	stream.Serialize(SquadID);
	stream.Serialize(LostConnection);
	stream.Serialize(SelectedPath);
	stream.Serialize(Paths);
	stream.Serialize(IsVisionary);
	stream.Serialize(IsTiberiumShort);
	stream.Serialize(IsSpied);
	stream.Serialize(IsThieved);
	stream.Serialize(DidRepair);
	stream.Serialize(IsBuiltSomething);
	stream.Serialize(IsResigner);
	stream.Serialize(IsGiverUpper);
	stream.Serialize(IsAllToHunt);
	stream.Serialize(IsParanoid);
	stream.Serialize(IsToLook);
	stream.Serialize(IQ);
	stream.Serialize(State);
	stream.Serialize(SuperWeapon);
	stream.Serialize(JustBuiltStructure);
	stream.Serialize(JustBuiltInfantry);
	stream.Serialize(JustBuiltUnit);
	stream.Serialize(JustBuiltAircraft);
	stream.Serialize(Blockage);
	stream.Serialize(RepairTimer);
	stream.Serialize(AlertTime);
	stream.Serialize(BorrowedTime);
	stream.Serialize(CreditsSpent);
	stream.Serialize(HarvestedCredits);
	stream.Serialize(StolenBuildingsCredits);
	stream.Serialize(CurUnits);
	stream.Serialize(CurBuildings);
	stream.Serialize(CurInfantry);
	stream.Serialize(CurAircraft);
	stream.Serialize(Tiberium);
	stream.Serialize(Credits);
	stream.Serialize(Capacity);
	stream.Serialize(Weed);
	stream.Serialize(field_1BC);
	// AircraftTotals -- internet game tallies owned by this house, built by the constructor.
	// InfantryTotals
	// UnitTotals
	// BuildingTotals
	// DestroyedAircraft
	// DestroyedInfantry
	// DestroyedUnits
	// DestroyedBuildings
	// CapturedBuildings
	// TotalCrates
	stream.Serialize(AircraftFactories);
	stream.Serialize(InfantryFactories);
	stream.Serialize(UnitFactories);
	stream.Serialize(BuildingFactories);
	stream.Serialize(Power);
	stream.Serialize(Drain);
	stream.Serialize(AircraftFactory);
	stream.Serialize(InfantryFactory);
	stream.Serialize(UnitFactory);
	stream.Serialize(BuildingFactory);
	stream.Serialize(FlagLocation);
	stream.Serialize(FlagHome);
	stream.Serialize(UnitsKilled);
	stream.Serialize(UnitsLost);
	stream.Serialize(BuildingsKilled);
	stream.Serialize(BuildingsLost);
	stream.Serialize(WhoLastHurtMe);
	stream.Serialize(Center);
	stream.Serialize(Radius);
	stream.Serialize(ZoneInfo);
	stream.Serialize(LATime);
	stream.Serialize(LAEnemy);
	stream.Serialize(ToCapture);
	stream.Serialize(RadarSpied);
	stream.Serialize(PointTotal);
	stream.Serialize(PreferredTarget);
	stream.Serialize(BQuantity);
	stream.Serialize(UQuantity);
	stream.Serialize(IQuantity);
	stream.Serialize(AQuantity);
	stream.Serialize(ABQuantity);
	stream.Serialize(AUQuantity);
	stream.Serialize(AIQuantity);
	stream.Serialize(AAQuantity);
	stream.Serialize(PBQuantity);
	stream.Serialize(PUQuantity);
	stream.Serialize(PIQuantity);
	stream.Serialize(PAQuantity);
	stream.Serialize(Attack);
	stream.Serialize(Enemy);
	stream.Serialize(AngerNodes);
	stream.Serialize(ScoutNodes);
	stream.Serialize(AITimer);
	stream.Serialize(PickEnemyTimer);
	stream.Serialize(BuildStructure);
	stream.Serialize(BuildUnit);
	stream.Serialize(BuildInfantry);
	stream.Serialize(BuildAircraft);
	stream.Serialize(RatioAITriggerTeam);
	stream.Serialize(RatioTeamAircraft);
	stream.Serialize(RatioTeamInfantry);
	stream.Serialize(RatioTeamUnits);
	stream.Serialize(BaseDefenseTeamCount);
	stream.Serialize(DropshipLoadouts);
	stream.Serialize(CurrentDropship);
	stream.Serialize(HasCloakGenerator);
	stream.Serialize(RemapColorRGB);
	stream.Serialize(Base);
	stream.Serialize(RecalcPower);
	stream.Serialize(RecalcRadar);
	stream.Serialize(EMPDest);
	stream.Serialize(NukeDest);
	stream.Serialize(Allies);
	stream.Serialize(DamageTime);
	stream.Serialize(TeamTime);
	stream.Serialize(TriggerTime);
	stream.Serialize(SpeakAttackDelay);
	stream.Serialize(SpeakPowerDelay);
	stream.Serialize(SpeakMoneyDelay);
	stream.Serialize(SpeakMaxedDelay);

	// BuildChoice -- a scratch list shared by every house rather than owned by one.
	stream.Serialize(Regions);
	stream.Serialize(IniName);
	stream.Serialize(Scheme);
	// BaseAreaMap -- points at a defense grid that only exists while a base defense is being
	// placed.
	stream.Serialize(field_10E20);
	stream.Serialize(field_10E28);
	stream.Serialize(DefenseCostMultiplier);
	stream.Serialize(field_10E38);
	stream.Serialize(EnemyArmorForcePrediction);
	stream.Serialize(EnemyAirForcePrediction);
	stream.Serialize(EnemyInfantryForcePrediction);
	stream.Serialize(PowerSurplus);
	stream.Serialize(SpawnWaypoint);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence contract. The load system uses the identifier to
/// discover which class to build when the object is read back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE HouseClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_HouseClass;
	return(S_OK);
}


/// <summary>
/// Fetches a waypoint path, creating it if need be.
/// This routine is used by the waypoint plotting code, which should never have to trouble
/// itself over whether the path object exists yet.
/// </summary>
/// <param name="path">The path to fetch.</param>
/// <returns>Returns with a pointer to the waypoint path. It is never NULL.</returns>
WaypointPathClass * HouseClass::Ensure_Path(PathType path)
{
	WaypointPathClass *wp = Paths[path];
	if (wp == NULL) {
		wp = new WaypointPathClass(path);
		Paths[path] = wp;
	}
	return(wp);
}


/// <summary>
/// Raises this house's firestorm wall.
/// Every firestorm wall this house owns is switched on and the map zones are recalculated,
/// since the raised wall now bars movement through the cells it stands in.
/// </summary>
void HouseClass::Activate_Firestorm(void)
{
	if (!FirestormDefenseActivated) {
		DynamicVectorClass<Cell> cells;
		FirestormDefenseActivated = true;
		for (int i = Buildings.Count() - 1; i >= 0; i--) {
			BuildingClass * b = Buildings[i];
			if (b->Class->IsFirestormWall && b->House == this) {
				b->Update_FS_Wall_State();
				cells.Add(b->Center_Coord().As_Cell());
			}
		}
		Map.Zone_Reset();
		Map.Recalc_Cells_In_List(cells);
	}
}


/// <summary>
/// Lowers this house's firestorm wall.
/// Every firestorm wall this house owns is switched off and the map zones are recalculated,
/// since those cells may be walked through once more. The player is told that the defense
/// has gone offline.
/// </summary>
void HouseClass::Deactivate_Firestorm(void)
{
	if (FirestormDefenseActivated) {
		FirestormDefenseActivated = false;
		DynamicVectorClass<Cell> cells;
		for (int i = Buildings.Count() - 1; i >= 0; i--) {
			BuildingClass * b = Buildings[i];
			if (b->Class->IsFirestormWall && b->House == this) {
				b->Update_FS_Wall_State();
				cells.Add(b->Center_Coord().As_Cell());
			}
		}
		Map.Zone_Reset();
		Map.Recalc_Cells_In_List(cells);
		if (Is_Player_Control()) {
			Speak(VOX_FIRESTORM_DEFENSE_OFFLINE);
		}
	}
}


/// <summary>
/// Handles the loss of a firestorm generator.
/// This routine is called when one of the house's firestorm generators goes away. Should it
/// have been the last working one, the firestorm super weapon is discharged, so that the
/// wall cannot be raised again until a generator is rebuilt and the weapon recharged.
/// </summary>
void HouseClass::Lost_Firestorm_Generator(void)
{
	if (FirestormDefenseActivated) {
		for (int i = Buildings.Count() - 1; i >= 0; i--) {
			BuildingClass * b = Buildings[i];
			if (b->Class == Rule->GDIFirestormGenerator && b->House == this && b->IsOn && !b->IsInLimbo &&
				b->Mission != MISSION_DECONSTRUCTION && b->Mission != MISSION_CONSTRUCTION) {

				return;
			}
		}
		for (int super = 0; super < SuperWeapon.Count(); super++) {
			if (SuperWeapon[super]->Class->Type == SUPER_FIRESTORM) {
				SuperWeapon[super]->Discharged(this == PlayerPtr, Cell(0, 0));
			}
		}
	}
}


/// <summary>
/// Adds to the anger this house feels toward another house.
/// Anger builds up as a rival makes a nuisance of itself, and the house declares whichever
/// surviving non-ally it is angriest at to be its enemy. Pass a negative level to take some
/// of that anger back off again.
/// </summary>
/// <param name="level">The amount of anger to add, which may be negative.</param>
/// <param name="house">The house that the anger is directed at.</param>
void HouseClass::Add_Anger(int level, HouseClass const * house)
{
	int i;

	for (i = 0; i < AngerNodes.Count(); i++) {
		if (AngerNodes[i].House == house) {
			AngerNodes[i].Level += level;
		}
	}

	int maxanger = 0;
	HouseClass * mostangry = NULL;

	for (i = 0; i < AngerNodes.Count(); i++) {
		HouseClass * enemy = AngerNodes[i].House;
		if (AngerNodes[i].Level > maxanger && !enemy->IsDefeated) {
			if (!Is_Ally(enemy)) {
				maxanger = AngerNodes[i].Level;
				mostangry = AngerNodes[i].House;
			}
		}
	}

	if (mostangry != NULL) {
		Enemy = mostangry->HeapID;
	} else {
		Enemy = HOUSE_NONE;
	}

}


/// <summary>
/// Marks another house as having been scouted.
/// The computer keeps a scouting record so that it knows which of its rivals it has actually
/// found and may therefore start plotting against.
/// </summary>
/// <param name="house">The house that has now been scouted.</param>
void HouseClass::Mark_Scouted(HouseClass const * house)
{
	for (int i = 0; i < ScoutNodes.Count(); i++) {
		if (ScoutNodes[i].House == house) {
			ScoutNodes[i].IsScouted = true;
		}
	}
}


/// <summary>
/// Starts the computer building a base.
/// This routine lays out a base plan for the house, unless the scenario has already supplied
/// one of its own.
/// </summary>
void HouseClass::Begin_Construction(void)
{
	if (Base.Nodes.Count() == 0) {
		int save_init = ScenarioInit;
		ScenarioInit = false;
		Make_Base_Nodes();
		ScenarioInit = save_init;
	}
}


/// <summary>
/// Fetches the first structure in a list that this house may own.
/// This routine is used to resolve the generic structure lists in the rules -- the barracks
/// or the power plant, say -- down to the particular one that this house builds.
/// </summary>
/// <param name="vector">The candidate structures, in order of preference.</param>
/// <returns>Returns with the first structure this house may own, or NULL if it may own
/// none.</returns>
BuildingTypeClass const * HouseClass::Get_First_Ownable(DynamicVectorClass<BuildingTypeClass const *> const & vector) const
{
	int owners = 1 << HouseTypes.ID(Class);
	for (int i = 0; i < vector.Count(); i++) {
		if (owners & vector[i]->Ownable) {
			return(vector[i]);
		}
	}

	return(NULL);
}


/// <summary>
/// Are the prerequisites for this object accounted for?
/// This routine is used while the computer plans its base, where the question must be asked
/// of the structures the house intends to own rather than of those it owns at this moment.
/// A generic prerequisite is resolved to whichever structure of that kind this house builds.
/// </summary>
/// <param name="type">The object whose prerequisites are being tested.</param>
/// <param name="owned">The structures to be counted as owned for the purposes of the test.</param>
/// <param name="ownedcount">How many entries of the owned list are to be considered.</param>
/// <returns>bool; Are all of the prerequisites satisfied?</returns>
bool HouseClass::AI_Has_Prerequisites(TechnoTypeClass const * type, DynamicVectorClass<BuildingTypeClass const *> & owned, int ownedcount) const
{
	BuildingTypeClass const * own_building;

	for (int i = 0; i < type->Prerequisite.Count(); i++) {

		if (type->Prerequisite[i] >= 0) {

			BuildingTypeClass const * b = BuildingTypes[type->Prerequisite[i]];
			if (b != Rule->BuildConst[0]) {

				bool found = false;
				for (int j = 0; j < ownedcount; j++) {
					if (owned[j] == b) {
						found = true;
						break;
					}
				}

				if (!found) {
					return(false);
				}
			}

		} else {

			if (type->Prerequisite[i] == -1) {
				continue;
			}

			own_building = NULL;

			switch (type->Prerequisite[i]) {

				case STRUCT_G_FACTORY:
					own_building = Get_First_Ownable(Rule->BuildWeapons);
					break;

				case STRUCT_G_BARRACKS:
					own_building = Get_First_Ownable(Rule->BuildBarracks);
					break;

				case STRUCT_G_RADAR:
					own_building = Get_First_Ownable(Rule->BuildRadar);
					break;

				case STRUCT_G_TECH:
					own_building = Get_First_Ownable(Rule->BuildTech);
					break;

				default:
					break;
			}

			bool found = false;
			for (int j = 0; j < ownedcount; j++) {
				if (owned[j] == own_building) {
					found = true;
					break;
				}
			}

			if (!found) {
				return(false);
			}
		}
	}

	return(true);
}


/// <summary>
/// Builds the computer's base construction plan.
/// This routine fills the base node list with everything this house is allowed and able to
/// build, ordered so that a prerequisite is always queued ahead of whatever needs it. Extra
/// refineries and placeholders for base defenses are woven in along the way, according to
/// the difficulty level and the side's appetite for fortification.
/// </summary>
void HouseClass::Make_Base_Nodes(void)
{
	int index;

	bool is_gdi = stricmp(Class->IniName, "GDI") == 0;
	bool is_nod = stricmp(Class->IniName, "NOD") == 0;

	Base.Init();

	BuildingTypeClass * plug = NULL;
	if (Addon_Enabled(ADDON_FIRESTORM)) {
		int plugnum = Random_Pick(0, 2);
		if (plugnum == 0) {
			plug = BuildingTypes[BuildingTypeClass::From_Name("GAPLUG2")];
		} else if (plugnum == 1) {
			plug = BuildingTypes[BuildingTypeClass::From_Name("GAPLUG3")];
		} else {
			plug = BuildingTypes[BuildingTypeClass::From_Name("GAPLUG4")];
		}
	}

	int ownable = 1 << HouseTypes.ID(Class);

	DynamicVectorClass<BuildingTypeClass const *> buildables;
	DynamicVectorClass<bool> isadded;

	for (index = 0; index < BuildingTypes.Count(); index++) {
		BuildingTypeClass const * builtype = BuildingTypes[index];
		if (ownable & builtype->Ownable &&
			builtype->CanAIBuildThis &&
			builtype->Level <= Control.TechLevel &&
			(!builtype->IsWeeder || VeinholeMonsterClass::VeinholeMonsters.Count() > 0) &&
			builtype != plug) {

			buildables.Add(builtype);
			isadded.Add(false);
		}
	}

	int buildable_count = buildables.Count();
	DynamicVectorClass<BuildingTypeClass const *> startingqueue;

	BuildingTypeClass const * conyard = Rule->BuildConst[0];
	for (index = 0; index < buildable_count; index++) {
		if (conyard == buildables[index]) {
			isadded[index] = true;
			startingqueue.Add(conyard);
			break;
		}
	}

	startingqueue.Add(Get_First_Ownable(Rule->BuildPower));

	BuildingTypeClass const * barracks = Get_First_Ownable(Rule->BuildBarracks);
	for (index = 0; index < buildables.Count(); index++) {
		if (buildables[index] == barracks) {
			BuildingTypeClass const * temp = buildables[0];
			buildables[0] = barracks;
			isadded[index] = isadded[0];
			isadded[0] = false;
			buildables[index] = temp;
			break;
		}
	}

	BuildingTypeClass const * weapons = Get_First_Ownable(Rule->BuildWeapons);
	for (index = 0; index < buildables.Count(); index++) {
		if (buildables[index] == weapons) {
			BuildingTypeClass const * temp = buildables[1];
			buildables[1] = weapons;
			isadded[index] = isadded[1];
			isadded[1] = false;
			buildables[index] = temp;
			break;
		}
	}

	int unprocessed = buildable_count - 1;
	while (unprocessed > 0) {

		int plugid = -1;
		bool added = false;
		int queued = startingqueue.Count();
		for (index = 0; index < buildable_count; index++) {

			if (isadded[index]) continue;

			BuildingTypeClass const * b = buildables[index];
			if (stricmp(b->IniName, "GAPLUG") == 0) {
				plugid = index;
				continue;
			}

			if (AI_Has_Prerequisites(b, startingqueue, queued)) {
				isadded[index] = true;
				startingqueue.Add(b);
				if (b->IsHelipad) {
					int num = Random_Pick(1, 3);
					for (int i = 0; i < num; i++) {
						startingqueue.Add(b);
					}
				}
				added = true;
				unprocessed--;
			}
		}

		if (!added) {
			if (plugid != -1) {
				isadded[plugid] = true;
				startingqueue.Add(buildables[plugid]);
				unprocessed--;
			} else {
				break;
			}
		}
	}

	int refcount = 2 - Difficulty;

	BuildingTypeClass const * ref = Get_First_Ownable(Rule->BuildRefinery);
	int refpos = 0;
	for (index = 0; index < startingqueue.Count() - 1; index++) {
		if (startingqueue[index] == ref) {
			if (refpos != -1 && refcount > 0) {
				for (int i = 0; i < refcount; i++) {
					startingqueue.Insert_After(Random_Pick(refpos, startingqueue.Count() - 1), ref);
				}
			}
			break;
		}
		refpos++;
	}

	DynamicVectorClass<BuildingTypeClass const *> finalqueue = {startingqueue[0], startingqueue[1], startingqueue[2]};

	int defensecount = 0;
	int buildcost = startingqueue[1]->Cost_Of(this) + startingqueue[2]->Cost_Of(this);

	static const double _cost_sub = 2000;
	static const double _cost_div = 1500;

	for (index = 3; index < startingqueue.Count(); index++) {
		int cost = (int)((buildcost - _cost_sub) * DefenseCostMultiplier / _cost_div);
		double wanted_defenses = is_nod ? cost * Rule->NodBaseDefenseCoefficient : cost * Rule->GDIBaseDefenseCoefficient;

		if (defensecount < (int)wanted_defenses) {
			int deficiency = (int)wanted_defenses - defensecount;
			defensecount += (int)deficiency;
			do {
				if (is_gdi) {
					finalqueue.Add(Rule->WallTower);
				}
				finalqueue.Add((BuildingTypeClass const *)-1);
				deficiency--;
			} while (deficiency);
		}

		finalqueue.Add(startingqueue[index]);
		buildcost += startingqueue[index]->Cost_Of(this);
	}

	if (is_nod || !Rule->AIBuildsWalls) {
		for (int count = (3 - Difficulty) * (is_gdi ? 3 : 2); count > 0; count--) {
			if (is_gdi) {
				finalqueue.Add(Rule->WallTower);
			}
			finalqueue.Add((BuildingTypeClass const *)-1);
		}
	}

	for (index = 0; index < finalqueue.Count(); index++) {
		BuildingTypeClass const * b = finalqueue[index];
		if ((int)b < 0 && (int)b >= -3) {
			Base.Nodes.Add(BaseNodeClass((StructType)(int)b, Cell(0, 0)));
		} else {
			Base.Nodes.Add(BaseNodeClass(b->HeapID, Cell(0, 0)));
		}
	}

	if ((!is_nod || Rule->NodAIBuildsWalls) && Rule->AIBuildsWalls) {
		Base.Nodes.Add(BaseNodeClass((StructType)-3, Cell(0, 0)));
	}
}


/// <summary>
/// Scores a base cell by its distance from the base center.
/// This routine is the weight function used when a structure should simply go as near to the
/// middle of the base as it can be got. A cell close to the center scores lowest and is
/// therefore the one preferred.
/// </summary>
/// <param name="house">The house whose base center the distance is measured from.</param>
/// <param name="cell">The cell being scored.</param>
/// <param name="tie_breaker">Tie breaking bias to be added to the score.</param>
/// <returns>Returns with the weight of the cell; the lower the better.</returns>
int HouseClass::Base_Cell_Weight_By_Distance(HouseClass const & house, Cell const & cell, int tie_breaker, int context)
{
	int x = cell.X - house.Base.PlacementCenter.X;
	int y = cell.Y - house.Base.PlacementCenter.Y;

	int size = std::max(abs(x), abs(y));
	return(tie_breaker + size * 1000);
}


/// <summary>
/// Scores a base cell by how well it is already defended.
/// This routine is the weight function that the base defense planner hands to
/// Where_To_Place_Building. A cell that is thinly covered, and that lies toward the quadrant
/// under threat, scores lowest and is therefore the one preferred.
/// </summary>
/// <param name="house">The house whose base defense map is consulted.</param>
/// <param name="cell">The cell being scored.</param>
/// <param name="tie_breaker">Tie breaking bias to be added to the score.</param>
/// <param name="context">The quadrant to favor, or -1 to disregard direction entirely.</param>
/// <returns>Returns with the weight of the cell; the lower the better.</returns>
int HouseClass::Base_Cell_Weight_By_Defense_Coverage(HouseClass const & house, Cell const & cell, int tie_breaker, int context)
{
	int value = house.BaseAreaMap[(cell.X - house.Base.BaseAreaRect.X) + (cell.Y - house.Base.BaseAreaRect.Y) * house.Base.BaseAreaRect.Width];
	int offset = 0;
	if (context != FACING_NONE) {
		int x = cell.X - house.Base.PlacementCenter.X;
		int y = cell.Y - house.Base.PlacementCenter.Y;
		DirType dir = DirType(std::atan2((double)-y, (double)x));
		offset = abs(DIR_STEP_8 * context - dir.As_Dir256());
		if (offset >= DIR_STEP_2) {
			offset = DIR_MAX - offset;
		}
	}
	return(tie_breaker + 1000 * (offset + (value * 128)));
}


/// <summary>
/// Determines where a structure should be placed within the computer's base.
/// This routine ranks the cells the base occupies with a weight function supplied by the
/// caller, then works outward from the best of them looking for a spot that is clear, level
/// with the base, and legal for the structure. The house's base spacing is honored on the
/// first pass and relaxed on the second, so that a cramped base still gets built up.
/// </summary>
/// <param name="buildingtype">The structure that needs somewhere to go.</param>
/// <param name="func">The weight function used to rank the candidate cells; lower wins.</param>
/// <param name="context">Extra context handed through to the weight function.</param>
/// <returns>Returns with the cell to build upon, or Cell(0,0) if the base has no room.</returns>
Cell HouseClass::Where_To_Place_Building(BuildingTypeClass *buildingtype, int (*func)(HouseClass const &, Cell const &, int, int), int context)
{
	int i;
	DynamicVectorClass<Cell> sorted_cells;

	if (Base.PlacementCenter == Cell(0, 0)) {
		return(Center.As_Cell());
	}

	/*
	 * Rank the cells the base occupies by the weight the caller's function gives them.
	 */
	IndexClass<int, Cell> ranking;
	sorted_cells.Clear();

	for (i = 0; i < Base.InnerCells.Count(); i++) {
		Cell cell = Base.InnerCells[i];
		int score;
		if (!buildingtype->IsCloakGenerator) {
			score = func(*this, cell, i, context);
		} else {
			score = i + 1000 * (Center - Coord(cell)).Length();
		}
		ranking.Add_Index(score, cell);
	}

	/// Pull the ranked cells out in ascending weight order.
	for (i = 0; i < ranking.Count(); i++) {
		sorted_cells.Add(ranking.Fetch_By_Position(i));
	}

	/// Height at base placement center.
	int base_height = Map[Base.PlacementCenter].Height;
	int house_mask  = 1 << HeapID;

	/// Declared out here deliberately -- scoped to the loop, MSVC6 pools its frame slot with
	/// the cloak generator distance temporary and the whole slot map shifts.
	Cell base_cell;

	for (int pass = 0; pass < 2; pass++) {
		for (int index = 0; index < sorted_cells.Count(); index++) {
			base_cell = sorted_cells[index];
			Cell occupied_dir(0, 0);

			/// Sum the directions of the neighbors this house already occupies.
			for (int face = 0; face < FACING_COUNT; face++) {
				CellClass *cptr = &Map[Adjacent_Cell(base_cell, (FacingType)face)];
				if (cptr->OccupiedBy & house_mask) {
					occupied_dir = Adjacent_Cell(occupied_dir, (FacingType)face);
				}
			}

			if (occupied_dir == Cell(0, 0)) {
				continue;
			}

			/// Face away from the mass of the base, so the structure lands on its outside.
			int x = -(int)occupied_dir.X;
			int y = -(int)occupied_dir.Y;
			double angle = std::atan2((double)-y, (double)x);
			DirType dir = DirType(angle);
			FacingType facing = dir.As_Facing();

			/*
			 * Offset the placement so the structure's own footprint and the base
			 * spacing fall on the side the facing points away from.
			 */
			Cell place_offset(0, 0);
			if (facing >= FACING_NE && facing <= FACING_SE) {
				place_offset.X = (short)Rule->AIBaseSpacing;
			} else if (facing >= FACING_SW && facing <= FACING_NW) {
				place_offset.X = (short)(1 - Rule->AIBaseSpacing - buildingtype->Width());
			}
			if (facing >= FACING_SE && facing <= FACING_SW) {
				place_offset.Y = (short)Rule->AIBaseSpacing;
			} else if (facing >= FACING_NW || facing <= FACING_NE) {
				place_offset.Y = (short)(1 - Rule->AIBaseSpacing - buildingtype->Height(false));
			}

			static Cell scan_offsets[FACING_COUNT] = {
				Cell(-1,  0), Cell(-1, -1), Cell( 0, -1), Cell( 1, -1),
				Cell( 1,  0), Cell( 1,  1), Cell( 0,  1), Cell(-1,  1)
			};
			static Cell step_offsets[FACING_COUNT] = {
				Cell( 1,  0), Cell( 1,  1), Cell( 0,  1), Cell(-1,  1),
				Cell(-1,  0), Cell(-1, -1), Cell( 0, -1), Cell( 1, -1)
			};

			int trial = 0;

			do {
				/*
				 * Compute start position for placement search.
				 */
				Cell pos = base_cell + place_offset + scan_offsets[facing];

				/*
				 * On the second placement try, start one cell away from the
				 * occupied mass (opposite facing).
				 */
				if (trial == 1) {
					pos = Adjacent_Cell(pos, Facing_Sub(facing, FACING_COUNT / 2));
				}

				/// The spacing border applies only on the first placement try.
				int use_spacing = (trial != 1);
				int attempt = 0;

				while (attempt < 3) {
					int total_height = buildingtype->Height(false);
					total_height += 2 * (Rule->AIBaseSpacing * use_spacing);
					int spacing      = Rule->AIBaseSpacing * use_spacing;
					int total_width  = buildingtype->Width() + 2 * spacing;

					Rect rect;
					rect.X      = pos.X - spacing;
					rect.Y      = pos.Y - spacing;
					rect.Width  = total_width;
					rect.Height = total_height;

					/// The second pass relaxes the area-availability requirement.
					if (pass == 1 || Map.Is_Area_Available(rect, HeapID)) {
						if (buildingtype->Legal_Placement(pos, this)) {
							if (abs(base_height - Map[pos].Height) < 3 && Can_Build_Here(buildingtype, pos)) {
								return(pos);
							}
						}
					}

					pos += step_offsets[facing];
					attempt++;
				}
				trial++;
			} while (trial < 2);
		}
	}

	return(Cell(0, 0));
}


/// <summary>
/// Determines where an upgrade should be placed.
/// This routine finds the structure this house owns that the upgrade attaches to and that
/// still has room for it, preferring the least upgraded one.
/// </summary>
/// <param name="upgrade">The upgrade that needs a home.</param>
/// <returns>Returns with the cell of the structure to upgrade, or Cell(0,0) if there is
/// none.</returns>
Cell HouseClass::Where_To_Place_Upgrade(BuildingTypeClass const * upgrade) const
{
	StructType upgradee_id = BuildingTypeClass::From_Name(upgrade->PowersUpBuilding);
	if (upgradee_id == STRUCT_NONE) {
		return(Cell(0, 0));
	}

	BuildingTypeClass const * upgradee_type = BuildingTypes[upgradee_id];
	BuildingClass * upgradee = NULL;
	int upgrade_level = upgradee_type->Upgrades + 10;
	bool anyslot = upgrade->PowersUpToLevel == -1;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * b = Buildings[i];
		if (b->House == this && b->Class == upgradee_type && b->Can_Upgrade(upgrade, this)) {
			if (anyslot && upgrade_level > b->UpgradeLevel || !anyslot && b->UpgradeLevel == 0) {
				upgrade_level = b->UpgradeLevel;
				upgradee = b;
			}
		}
	}

	if (upgradee != NULL) {
		return(upgradee->PositionCoord.As_Cell());
	}

	return(Cell(0, 0));
}


/// <summary>
/// Clamps a cell so that it lies within a rectangle.
/// </summary>
/// <param name="bounds">The rectangle to confine the cell to.</param>
/// <param name="cell">The cell to be clamped.</param>
/// <returns>Returns with the nearest cell that lies inside the rectangle.</returns>
Cell Clamp_Cell(Rect const & bounds, Cell const & cell)
{
	Cell result = cell;
	if (result.X < bounds.X) {
		result.X = bounds.X;
	}
	if (result.X >= bounds.Width + bounds.X) {
		result.X = bounds.Width + bounds.X - 1;
	}
	if (result.Y < bounds.Y) {
		result.Y = bounds.Y;
	}
	if (result.Y >= bounds.Height + bounds.Y) {
		result.Y = bounds.Height + bounds.Y - 1;
	}
	return(result);
}


/// <summary>
/// Stamps a structure's defense value into the base defense map.
/// The value is spread across the cells about the structure and falls away with distance, so
/// that the base defense planner can see which parts of the base a given structure really
/// protects.
/// </summary>
/// <param name="building">The defending structure to stamp into the map.</param>
/// <param name="value">The defense value of the structure against the category being
/// mapped.</param>
/// <param name="values">The defense map to accumulate into.</param>
/// <param name="area">The area of the world that the defense map covers.</param>
void HouseClass::Calculate_Defense_Values(BuildingClass const * building, int value, int * values, Rect const & area) const
{
	if (value <= 0) value = 1;

	Cell center = building->PositionCoord.As_Cell();

	int x_min = std::max(center.X - 6, area.X);
	int x_max = std::min(area.X + area.Width, center.X + 6);
	int y_min = std::max(center.Y - 6, area.Y);
	int y_max = std::min(area.Y + area.Height, center.Y + 6);

	for (int y = y_min; y < y_max; ++y) {
		for (int x = x_min; x < x_max; ++x) {
			int y_offset = y - center.Y;
			double distance = std::sqrt((double)((x - center.X) * (x - center.X) + y_offset * y_offset));
			if (distance < 6.0) {
				double adjusted_distance = distance;
				int x_offset = x - area.X;
				int area_y_offset = y - area.Y;
				if (adjusted_distance < 1.0) {
					adjusted_distance = 1.0;
				}
				int * cell_value = &values[x_offset + area.Width * area_y_offset];
				*cell_value += ((double)value / ((adjusted_distance - 1.0) * 0.1 + 1.0));
			}
		}
	}
}


/// <summary>
/// Settles a base defense structure onto a base node.
/// This routine is called by the base building logic when the next node of the plan calls
/// for a defense. It finds the weakest defended quadrant of the base, decides whether air,
/// armor or infantry cover is most lacking there, favors a cheap structure that answers that
/// need, and gives it a cell -- either one the enemy has been attacking through, or a fresh
/// spot within the base. Any wall node made redundant by the choice is dropped.
/// </summary>
/// <param name="nodeindex">The base node to fill in.</param>
/// <param name="cells">The threat cells to defend against, or NULL to simply defend the
/// base.</param>
/// <returns>bool; Was a defense settled upon the node?</returns>
bool HouseClass::AI_Build_Defense(int nodeindex, DynamicVectorClass<Cell> * cells)
{
	int i;

	int bestidx = -1;
	int bestval = 999999;

	int aair_arr[4];
	aair_arr[0] = 0;
	aair_arr[1] = 0;
	aair_arr[2] = 0;
	aair_arr[3] = 0;

	int aarr_arr[4];
	aarr_arr[0] = 0;
	aarr_arr[1] = 0;
	aarr_arr[2] = 0;
	aarr_arr[3] = 0;

	int agrnd_arr[4];
	agrnd_arr[0] = 0;
	agrnd_arr[1] = 0;
	agrnd_arr[2] = 0;
	agrnd_arr[3] = 0;

	int types[4];

	DynamicVectorClass<Cell> cells2[4];

	int size = Base.BaseAreaRect.Width * Base.BaseAreaRect.Height;
	int *aair = new int[size];
	int *aarmor = new int[size];
	int *aground = new int[size];
	memset(aair, 0, size * sizeof(int));
	memset(aarmor, 0, size * sizeof(int));
	memset(aground, 0, size * sizeof(int));

	bool is_gdi = stricmp(Class->IniName, "GDI") == 0;

	/*
	 * Bucket the incoming threat cells into the four quadrants around the
	 * base placement center.
	 */
	if (cells != NULL) {
		for (i = 0; i < cells->Count(); i++) {
			Cell offset = (*cells)[i] - Base.PlacementCenter;
			DirType dir = DirType(std::atan2((double)-offset.Y, (double)offset.X));
			cells2[dir.As_Dir4()].Add((*cells)[i]);
		}
	}

	/*
	 * Sum the defense values of our existing buildings per quadrant and
	 * accumulate the per-cell defense maps.
	 */
	for (i = 0; i < Buildings.Count(); i++) {
		BuildingClass * building = Buildings[i];
		if (building->House == this) {
			Cell offset = building->PositionCell - Base.PlacementCenter;
			if (offset.X != 0 || offset.Y != 0) {
				DirType dir = DirType(std::atan2((double)-offset.Y, (double)offset.X));
				int quadrant = dir.As_Dir4();
				int air = building->Anti_Air_Defense_Value();
				int armor = building->Anti_Armor_Defense_Value();
				int infantry = building->Anti_Infantry_Defense_Value();
				if (air + armor + infantry > 0) {
					aair_arr[quadrant] += air;
					aarr_arr[quadrant] += armor;
					agrnd_arr[quadrant] += infantry;
					Calculate_Defense_Values(building, air, aair, Base.BaseAreaRect);
					Calculate_Defense_Values(building, armor, aarmor, Base.BaseAreaRect);
					Calculate_Defense_Values(building, infantry, aground, Base.BaseAreaRect);
				}
			}
		}
	}

	/*
	 * Pick the weakest defended quadrant.
	 */
	for (i = 0; i < 4; i++) {
		types[i] = aair_arr[i] + aarr_arr[i] + agrnd_arr[i];
		if (types[i] < bestval && (cells == NULL || cells2[i].Count() > 0)) {
			bestval = types[i];
			bestidx = i;
		}
	}

	int air_value = aair_arr[bestidx];
	int armor_value = aarr_arr[bestidx];
	int ground_value = agrnd_arr[bestidx];

	Calculate_Enemy_Predictions();

	/*
	 * Build the list of non-defense building types we own. This serves as the
	 * prerequisite basis for the defense candidates.
	 */
	DynamicVectorClass<BuildingTypeClass const *> owned;
	for (i = 0; i < Buildings.Count(); i++) {
		BuildingClass * building = Buildings[i];
		if (building->House == this) {
			BuildingTypeClass * type = building->Class;
			if (!type->IsBaseDefense && type != Rule->WallTower) {
				owned.Add(building->Class);
			}
		}
	}
	if (is_gdi) {
		owned.Add(Rule->WallTower);
	}

	StructType type_id = Base.Nodes[nodeindex].Type;
	BuildingTypeClass * pending = NULL;

	/*
	 * Compare the share each defense category has in the chosen quadrant
	 * against the predicted enemy force composition.
	 */
	float air_ratio = bestval != 0 ? air_value / (float)bestval : 0.0f;
	float armor_ratio = bestval != 0 ? armor_value / (float)bestval : 0.0f;
	float ground_ratio = bestval != 0 ? ground_value / (float)bestval : 0.0f;
	float air_deficit = EnemyAirForcePrediction - air_ratio;
	float armor_deficit = EnemyArmorForcePrediction - armor_ratio;
	float ground_deficit = EnemyInfantryForcePrediction - ground_ratio;

	DynamicVectorClass<BuildingTypeClass *> air_defenses = Get_Anti_Air_Defense_Buildings(owned);
	DynamicVectorClass<BuildingTypeClass *> armor_defenses = Get_Anti_Armor_Defense_Buildings(owned);
	DynamicVectorClass<BuildingTypeClass *> ground_defenses = Get_Anti_Ground_Defense_Buildings(owned);

	/*
	 * Address the category with the largest deficit first.
	 */
	DynamicVectorClass<BuildingTypeClass *> * defenses;
	int category;
	if (air_deficit <= armor_deficit || air_deficit <= ground_deficit) {
		if (armor_deficit > ground_deficit) {
			defenses = &armor_defenses;
			BaseAreaMap = aarmor;
			category = 1;
		} else {
			defenses = &ground_defenses;
			BaseAreaMap = aground;
			category = 2;
		}
	} else {
		defenses = &air_defenses;
		BaseAreaMap = aair;
		category = 0;
	}

	/*
	 * If there are no candidates for the chosen category, fall back to any
	 * category that has them.
	 */
	bool success = true;
	if (defenses->Count() == 0) {
		if (armor_defenses.Count() > 0) {
			defenses = &armor_defenses;
			category = 1;
			BaseAreaMap = aarmor;
		} else if (ground_defenses.Count() > 0) {
			defenses = &ground_defenses;
			category = 2;
			BaseAreaMap = aground;
		} else if (air_defenses.Count() > 0) {
			defenses = &air_defenses;
			category = 0;
			BaseAreaMap = aair;
		} else {
			success = false;
		}
	}

	if (success) {

		/*
		 * Pick the defense to build: cheap buildings and buildings with a high
		 * value against the chosen category are favored.
		 */
		BuildingTypeClass * choice;
		if (defenses->Count() == 1) {
			choice = (*defenses)[0];
		} else {
			DiscreteDistributionClass<BuildingTypeClass> distribution;
			for (i = 0; i < defenses->Count(); i++) {
				BuildingTypeClass * type = (*defenses)[i];
				int weight = 10000 / type->Cost_Of(this);
				if (category == 2) {
					weight += type->AntiInfantryValue;
				} else if (category == 1) {
					weight += type->AntiArmorValue;
				} else {
					weight += type->AntiAirValue;
				}
				distribution.Add(type, weight);
			}
			choice = distribution.Sample();
		}

		/*
		 * If the node already has a type assigned, build that type and queue the
		 * picked defense at the following node instead.
		 */
		BuildingTypeClass * build_type;
		build_type = choice;
		if (type_id >= 0) {
			pending = choice;
			build_type = BuildingTypes[type_id];
		}

		Cell cell(0, 0);
		if (cells != NULL) {
			IndexClass<int, Cell> cellidx;
			DynamicVectorClass<Cell> cellvec;
			cellvec.Clear();
			for (i = 0; i < cells2[bestidx].Count(); i++) {
				Cell threat_cell = cells2[bestidx][i];
				cellidx.Add_Index(Base_Cell_Weight_By_Defense_Coverage(*this, threat_cell, i, -1), threat_cell);
			}
			for (i = 0; i < cellidx.Count(); i++) {
				cellvec.Add(cellidx.Fetch_By_Position(i));
			}
			cell = cellvec[0];
			cells->Delete_Index(cells->ID(cell));
		} else {
			cell = Where_To_Place_Building(build_type, Base_Cell_Weight_By_Defense_Coverage, 2 * bestidx);
		}

		if (cell == Cell(0, 0)) {
			success = false;
		} else if (pending != NULL) {
			Base.Nodes[nodeindex].CellID = cell;
			if (Base.Nodes[nodeindex + 1].Type == STRUCT_NONE) {
				Base.Nodes[nodeindex + 1].Type = (StructType)pending->HeapID;
				Base.Nodes[nodeindex + 1].CellID = cell;

				/*
				 * A wall node at this cell is now redundant -- remove it.
				 */
				if (cells != NULL) {
					BuildingTypeClass const * wall = Get_First_Ownable(Rule->ConcreteWalls);
					for (i = Base.Nodes.Count() - 1; i >= 0; i--) {
						if (Base.Nodes[i].CellID == cell && Base.Nodes[i].Type == wall->HeapID) {
							Base.Nodes.Delete_Index(i);
							break;
						}
					}
				}
			}
		} else {
			Base.Nodes[nodeindex].Type = (StructType)build_type->HeapID;
			Base.Nodes[nodeindex].CellID = cell;
		}
	}

	delete[] aair;
	delete[] aarmor;
	delete[] aground;
	BaseAreaMap = NULL;

	return(success);
}


/// <summary>
/// Fetches the anti-aircraft defenses this house could build.
/// This routine gathers the candidates that the base defense planner will choose between.
/// Only structures this house may own, that are within its tech level, and whose
/// prerequisites are already accounted for, are listed.
/// </summary>
/// <param name="owned">The structures to count as owned when testing prerequisites.</param>
/// <returns>Returns with the list of candidates, which may well be empty.</returns>
DynamicVectorClass<BuildingTypeClass *> HouseClass::Get_Anti_Air_Defense_Buildings(DynamicVectorClass<BuildingTypeClass const *> & owned) const
{
	unsigned ownable = 1 << HouseTypes.ID(Class);
	DynamicVectorClass<BuildingTypeClass *> defenses;

	for (int i = 0; i < BuildingTypes.Count(); i++) {
		BuildingTypeClass * b = BuildingTypes[i];
		if (ownable & b->Ownable && b->AntiAirValue > 0 && b->Level <= Control.TechLevel && AI_Has_Prerequisites(b, owned, owned.Count())) {
			defenses.Add(b);
		}
	}

	return(defenses);
}


/// <summary>
/// Fetches the anti-armor defenses this house could build.
/// This routine gathers the candidates that the base defense planner will choose between.
/// Only structures this house may own, that are within its tech level, and whose
/// prerequisites are already accounted for, are listed.
/// </summary>
/// <param name="owned">The structures to count as owned when testing prerequisites.</param>
/// <returns>Returns with the list of candidates, which may well be empty.</returns>
DynamicVectorClass<BuildingTypeClass *> HouseClass::Get_Anti_Armor_Defense_Buildings(DynamicVectorClass<BuildingTypeClass const *> & owned) const
{
	unsigned ownable = 1 << HouseTypes.ID(Class);
	DynamicVectorClass<BuildingTypeClass *> defenses;

	for (int i = 0; i < BuildingTypes.Count(); i++) {
		BuildingTypeClass * b = BuildingTypes[i];
		if (ownable & b->Ownable && b->AntiArmorValue > 0 && b->Level <= Control.TechLevel && AI_Has_Prerequisites(b, owned, owned.Count())) {
			defenses.Add(b);
		}
	}

	return(defenses);
}


/// <summary>
/// Fetches the anti-infantry defenses this house could build.
/// This routine gathers the candidates that the base defense planner will choose between.
/// Only structures this house may own, that are within its tech level, and whose
/// prerequisites are already accounted for, are listed.
/// </summary>
/// <param name="owned">The structures to count as owned when testing prerequisites.</param>
/// <returns>Returns with the list of candidates, which may well be empty.</returns>
DynamicVectorClass<BuildingTypeClass *> HouseClass::Get_Anti_Ground_Defense_Buildings(DynamicVectorClass<BuildingTypeClass const *> & owned) const
{
	unsigned ownable = 1 << HouseTypes.ID(Class);
	DynamicVectorClass<BuildingTypeClass *> defenses;

	for (int i = 0; i < BuildingTypes.Count(); i++) {
		BuildingTypeClass * b = BuildingTypes[i];
		if (ownable & b->Ownable && b->AntiInfantryValue > 0 && b->Level <= Control.TechLevel && AI_Has_Prerequisites(b, owned, owned.Count())) {
			defenses.Add(b);
		}
	}

	return(defenses);
}


/// <summary>
/// Predicts the makeup of the enemy's forces.
/// The base defense planner consults these predictions to judge whether it is short of
/// anti-air, anti-armor or anti-infantry cover.
/// </summary>
void HouseClass::Calculate_Enemy_Predictions(void)
{
	EnemyInfantryForcePrediction = 0.33f;
	EnemyAirForcePrediction = 0.33f;
	EnemyArmorForcePrediction = 0.33f;
}


/// <summary>
/// Plans a perimeter wall around the computer's base.
/// This routine surveys the edges of the base area and adds wall nodes to the base plan
/// wherever the ground will take them, setting a gate into the middle of any run long enough
/// to need one. GDI additionally gets a share of wall towers to man the finished line.
/// </summary>
void HouseClass::AI_Build_Wall(void)
{
	int index;

	Base.LastBaseAreaRect = Rect(Base.BaseAreaRect.X - 1, Base.BaseAreaRect.Y - 1, Base.BaseAreaRect.Width + 2, Base.BaseAreaRect.Height + 2);
	DynamicVectorClass<BaseNodeClass> wall_nodes;
	DynamicVectorClass<BaseNodeClass> gate_nodes;

	Cell start_cells[4] = {
		Cell(Base.LastBaseAreaRect.X, Base.LastBaseAreaRect.Y),
		Cell(Base.LastBaseAreaRect.X, Base.LastBaseAreaRect.Y + Base.LastBaseAreaRect.Height - 1),
		Cell(Base.LastBaseAreaRect.X, Base.LastBaseAreaRect.Y + 1),
		Cell(Base.LastBaseAreaRect.X + Base.LastBaseAreaRect.Width - 1, Base.LastBaseAreaRect.Y + 1)
	};

	Cell end_cells[4] = {
		Cell(Base.LastBaseAreaRect.X + Base.LastBaseAreaRect.Width, Base.LastBaseAreaRect.Y),
		Cell(Base.LastBaseAreaRect.X + Base.LastBaseAreaRect.Width, Base.LastBaseAreaRect.Y + Base.LastBaseAreaRect.Height - 1),
		Cell(Base.LastBaseAreaRect.X, Base.LastBaseAreaRect.Y + Base.LastBaseAreaRect.Height - 1),
		Cell(Base.LastBaseAreaRect.X + Base.LastBaseAreaRect.Width - 1, Base.LastBaseAreaRect.Y + Base.LastBaseAreaRect.Height - 1)
	};

	BuildingTypeClass const * wall = Get_First_Ownable(Rule->ConcreteWalls);
	BuildingTypeClass const * ewgate = Get_First_Ownable(Rule->EWGates);
	BuildingTypeClass const * nsgate = Get_First_Ownable(Rule->NSGates);

	BuildingTypeClass const * gates[] = { ewgate, ewgate, nsgate, nsgate };
	static const FacingType _step_facings[] = { FACING_E, FACING_E, FACING_S, FACING_S };
	static const FacingType _adjacent_facings[] = { FACING_N, FACING_S, FACING_W, FACING_E };
	Cell run_start(0, 0);
	int base_height = Map[Base.PlacementCenter].Height;

	for (index = 0; index < 4; index++) {
		Cell cell = start_cells[index];
		FacingType dir = _step_facings[index];

		int run_length = 0;
		bool in_run = false;
		int soft_blocked = 0;

		while (cell != end_cells[index]) {
			CellClass * cellptr = &Map[cell];
			CellClass * adjptr = &Map[Adjacent_Cell(cell, _adjacent_facings[index])];
			LandType land = cellptr->Land_Type();
			LandType adjland = adjptr->Land_Type();

			bool height_check = abs(cellptr->Height - base_height) <= 2;
			bool land_check = land != LAND_ROCK && land != LAND_WATER && land != LAND_ICE && adjland != LAND_ROCK && adjland != LAND_WATER && adjland != LAND_ICE;
			bool overlay_check = cellptr->Overlay == OVERLAY_NONE;
			bool building_check = cellptr->Cell_Building() == NULL && adjptr->Cell_Building() == NULL;
			bool terrain_check = cellptr->Cell_Terrain() == NULL && adjptr->Cell_Terrain() == NULL;
			bool ramp_check = cellptr->Ramp == 0;
			bool radar_check = Map.In_Local_Radar(cell);

			if (height_check && land_check && overlay_check && building_check && terrain_check && ramp_check && radar_check) {
				if (!in_run) {
					in_run = true;
					run_length = 1;
					run_start = cell;
				} else {
					run_length++;
				}
			} else {
				bool hard_blocked;
				if (height_check && land_check && building_check && terrain_check && radar_check) {
					hard_blocked = false;
					soft_blocked++;
				} else {
					hard_blocked = true;
					if (soft_blocked > 0) {
						hard_blocked = false;
						soft_blocked = 0;
					}
				}

				if (run_length >= 5 || !hard_blocked) {
					Cell node_cell = run_start;
					int placed = 0;
					Cell prev_cell = run_start;
					for (int gate_index = run_length / 2 - 1; placed < run_length; prev_cell = node_cell) {
						if (placed == gate_index && hard_blocked) {
							gate_nodes.Add(BaseNodeClass(gates[index]->HeapID, node_cell));
							placed += 3;
							Cell step1 = Adjacent_Cell(prev_cell, dir);
							Cell step2 = Adjacent_Cell(step1, dir);
							node_cell = Adjacent_Cell(step2, dir);
						} else {
							wall_nodes.Add(BaseNodeClass(wall->HeapID, node_cell));
							placed++;
							node_cell = Adjacent_Cell(prev_cell, dir);
						}
					}
				}

				run_length = 0;
				in_run = false;
			}

			cell = Adjacent_Cell(cell, dir);
		}

		if (run_length >= 5) {
			Cell node_cell = run_start;
			int placed = 0;
			Cell prev_cell = run_start;
			for (int gate_index = run_length / 2 - 1; placed < run_length; prev_cell = node_cell) {
				if (placed == gate_index) {
					gate_nodes.Add(BaseNodeClass(gates[index]->HeapID, node_cell));
					placed += 3;
					Cell step1 = Adjacent_Cell(prev_cell, dir);
					Cell step2 = Adjacent_Cell(step1, dir);
					node_cell = Adjacent_Cell(step2, dir);
				} else {
					wall_nodes.Add(BaseNodeClass(wall->HeapID, node_cell));
					placed++;
					node_cell = Adjacent_Cell(prev_cell, dir);
				}
			}
		}
	}

	for (index = 0; index < wall_nodes.Count(); index++) {
		Base.Nodes.Add(wall_nodes[index]);
	}

	for (index = 0; index < gate_nodes.Count(); index++) {
		Base.Nodes.Add(gate_nodes[index]);
	}

	if (stricmp(Class->IniName, "GDI") == 0) {
		for (index = 0; index < wall_nodes.Count(); index++) {
			Base.OuterCells.Add(wall_nodes[index].CellID);
		}

		double count = wall_nodes.Count() * 0.2;
		double max_count = (3 - Difficulty) * Rule->GDIWallDefenseCoefficient + Rule->GDIWallDefense;
		if (count >= max_count) {
			count = max_count;
		}

		int wall_defenses = (int)count;
		for (index = 0; index < wall_defenses; index++) {
			Base.Nodes.Add(BaseNodeClass(Rule->WallTower->HeapID, Cell(0, 0)));
			Base.Nodes.Add(BaseNodeClass((StructType)-1, Cell(0, 0)));
		}
	}

	Base.BaseAreaRect = Base.LastBaseAreaRect;
}


/// <summary>
/// Recalculates the power output and drain of this house.
/// This routine tallies every structure the house has running and then refreshes whatever
/// depends on the result -- factory speed and super weapon availability among them. An
/// undiscovered structure contributes nothing while the player is still to find it.
/// </summary>
void HouseClass::Recalc_Power_Drain(void)
{
	bool was_low_power = Power_Fraction() < 1.0;

	RecalcPower = false;
	Power = 0;
	Drain = 0;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * b = Buildings[i];
		if (b && b->House == this && !b->IsInLimbo && b->IsDown) {
			if (Is_Player_Control() && !b->IsDiscoveredByPlayer && Session.Type == GAME_NORMAL) continue;
			Power += Buildings[i]->Power_Output();
			Drain += b->Power_Drain();
		}
	}

	Adjust_House_Power(this);
	Recalc_House_Factories(this);

	if (was_low_power != (Power_Fraction() < 1.0)) {
		Update_Present_Super_Weapons();
	}

	RecalcRadar = true;
}


/// <summary>
/// Determines whether the radar map should be available.
/// This routine is called after anything that might switch the radar on or off -- a change
/// in power, an ion storm, or the loss of the radar structure itself. The tactical map is
/// told to raise or lower the radar to suit. Only the player's house has a radar to manage.
/// </summary>
void HouseClass::Recalc_Radar_Availability(void)
{
	RecalcRadar = false;

	if (this == PlayerPtr) {
		// A player given the whole map keeps the radar through storms, blackouts and losses.
		bool radar_on = Session.ObiWan != 0;

		if (!radar_on && !IonStormClass::Is_Ion_Storm_Active() && Power >= Drain) {
			if (Scen->IsFreeRadar) {
				radar_on = true;
			} else {
				for (int i = 0; i < Buildings.Count(); i++) {
					BuildingClass * b = Buildings[i];
					if (b && b->House == this && b->IsOn && b->Class->IsRadar && !b->IsInLimbo && b->IsDown) {
						if (Is_Player_Control() && !b->IsDiscoveredByPlayer && Session.Type == GAME_NORMAL) continue;
						if (b->CurrentMission != MISSION_DECONSTRUCTION && b->MissionQueue != MISSION_DECONSTRUCTION) {
							if (b->StunDuration == 0) {
								radar_on = true;
							}
							break;
						}
					}
				}
			}
		}

		if (Map.Is_Radar_Existing() != radar_on) {
			Map.Toggle_Radar(radar_on);
		}
	}
}


/// <summary>
/// Fetches an unused waypoint path for this house.
/// This routine is used when the player starts plotting a fresh patrol or move path and
/// needs somewhere to record it.
/// </summary>
/// <returns>Returns with the path found, or PATH_NONE if every path is already in use.</returns>
PathType HouseClass::New_Waypoint_Path(void) const
{
	for (int i = 0; i < PATH_COUNT; i++) {
		WaypointPathClass * path = Paths[i];
		if (path == NULL) {
			path = new WaypointPathClass();
		}
		if (path->Waypoint_Count() == 0) {
			return((PathType)i);
		}
	}
	return(PATH_NONE);
}


/// <summary>
/// May another waypoint be added to the selected path?
/// This routine is used by the waypoint plotting mode to decide whether the player is
/// allowed to drop another marker. A path that has reached its length limit, or that is
/// already being traveled, is closed to additions.
/// </summary>
/// <returns>bool; Can the currently selected path take another waypoint?</returns>
bool HouseClass::Can_Add_Waypoint_To_Path(void) const
{
	if (SelectedPath < 0 || SelectedPath >= PATH_COUNT) return(false);
	WaypointPathClass * path = Paths[SelectedPath];
	return(path->Waypoint_Count() < Rule->MaxWaypointPathLength && path->Current_Waypoint() == -1);
}


/// <summary>
/// Enables threat rating for this house.
/// A threat rating structure lets the house weigh targets by the coefficients of the unit
/// doing the choosing rather than by the blunt default rules. This routine is called when
/// such a structure comes into the house's possession.
/// </summary>
void HouseClass::Activate_Threat_Node(void)
{
	IsThreatRatingNodeActive = true;
}


/// <summary>
/// Brings a production factory back in line with what may still be built.
/// This routine is called whenever the house gains or loses something that changes its build
/// options. Objects that can no longer be built are dropped from the queue, production of a
/// now illegal object is abandoned, and an object that is merely unavailable for the moment
/// is suspended until it may proceed again.
/// </summary>
/// <param name="rtti">Which of the house's factories to bring up to date.</param>
void HouseClass::Update_Factories(RTTIType rtti)
{
	FactoryClass * factory = NULL;

	switch (rtti) {
		case RTTI_BUILDINGTYPE:
			factory = BuildingFactory;
			break;

		case RTTI_UNITTYPE:
			factory = UnitFactory;
			break;

		case RTTI_INFANTRYTYPE:
			factory = InfantryFactory;
			break;

		case RTTI_AIRCRAFTTYPE:
			factory = AircraftFactory;
			break;
	}

	if (factory != NULL) {
		for (int i = factory->QueuedObjects.Count() - 1; i >= 0; i--) {
			TechnoTypeClass const * ttype = factory->QueuedObjects[i];
			if (ttype->Who_Can_Build_Me(true, false, true, this) == NULL) {
				factory->QueuedObjects.Delete_Index(i);
			}
		}
		if (factory->Object != NULL) {
			if (factory->Object->TClass->Who_Can_Build_Me(true, false, true, this) == NULL) {
				factory->Abandon();
				factory->Resume_Queue();
			} else {
				if (factory->Object->TClass->Who_Can_Build_Me(true, true, true, this) == NULL) {
					factory->Suspend(false);
					if (PlayerPtr == this) {
						Map.SidebarClass::IsToRedraw = true;
						Map.IsToBlitSidebar = true;
						Map.Flag_To_Redraw();
						Map.Column[0].Flag_To_Redraw();
						Map.Column[1].Flag_To_Redraw();
					}
				} else {
					if (factory->IsSuspended && !factory->IsOnHold) {
						factory->Start(false);
					}
				}
			}
		}
		if (factory->Object == NULL && factory->QueuedObjects.Count() == 0) {
			delete factory;
		}
	}
}


/// <summary>
/// Records that another house has spied upon this house's radar.
/// When it is the player doing the spying, this routine also reveals the map around
/// everything this house owns, since a spied radar hands its owner a look at the entire
/// force.
/// </summary>
/// <param name="house">The house that performed the spying.</param>
void HouseClass::Update_Spied_Radar(HouseClass * house)
{
	RadarSpied |= 1 << house->Class->House;
	if (house == PlayerPtr) {
		for (int index = 0; index < Technos.Count(); index++) {
			TechnoClass * obj = Technos[index];
			if (obj && !obj->IsInLimbo && obj->House == this) {
				obj->Look();
			}
		}
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
	}
}


/// <summary>
/// Clears the anger this house holds against another house.
/// This routine is used when a grudge should be dropped -- when an alliance is formed, for
/// example. The house reconsiders which enemy it hates most as a result.
/// </summary>
/// <param name="house">The house to be forgiven.</param>
void HouseClass::Clear_Anger(HouseClass const * house)
{
	for (int i = 0; i < AngerNodes.Count(); i++) {
		if (AngerNodes[i].House == house) {
			Add_Anger(-AngerNodes[i].Level, house);
			break;
		}
	}
}


/// <summary>
/// Rebuilds this house's threat map.
/// Every object that this house has reason to fear adds its risk rating to the map region
/// it stands in. The computer consults the resulting map whenever it needs to know which
/// parts of the world are dangerous -- for target selection, base planning and team
/// routing alike.
/// </summary>
void HouseClass::Recalc_Threat_Regions(void)
{
	int i;

	for (i = 0; i < MAP_TOTAL_REGIONS; i++) {
		Regions[i].Reset_Threat();
	}

	for (i = Technos.Count() - 1; i >= 0; i--) {
		TechnoClass * techno = Technos[i];
		int risk = techno->Risk();

		if (risk > 0) {
			if ((techno->RTTI == RTTI_INFANTRY || techno->RTTI == RTTI_UNIT || techno->RTTI == RTTI_AIRCRAFT) && techno != NULL) {
				FootClass * foot = (FootClass *)techno;
				if (foot->LastAdjacencyCell != CELL_NONE) {
					int region = Map.Cell_Region(foot->LastAdjacencyCell);
					if (foot->House != this && (!Is_Human_Player() || !Is_Ally(foot->House))) {
						Adjust_Threat(region, risk);
					}
				}
			} else {
				int region = Map.Cell_Region(techno->Get_Cell_Ptr()->CellID);
				Adjust_Threat(region, risk);
			}
		}
	}
}


/// <summary>
/// Counts the teams of a given type that this house owns.
/// This routine is used by the team creation logic so that a team type's allowed count is
/// not exceeded.
/// </summary>
/// <returns>Returns with the number of teams of that type belonging to this house.</returns>
int HouseClass::Owned_Team_Count(TeamTypeClass const * teamtype) const
{
	int count = 0;
	for (int i = Teams.Count() - 1; i >= 0; i--) {
		TeamClass * team = Teams[i];
		if (team && team->House == this && teamtype == team->Class) {
			count++;
		}
	}
	return(count);
}


/// <summary>
/// Can this house field the specified team type?
/// Every member of the team's task force must be something this house is able to build.
/// Outside of campaign games an existing recruitable unit of that type will serve just as
/// well, so that a house can still assemble teams from what it happens to own.
/// </summary>
/// <returns>bool; Can the team be created by this house?</returns>
bool HouseClass::Can_Create_Team(TeamTypeClass const * teamtype)
{
	if (teamtype != NULL && teamtype->TaskForce != NULL) {
		for (int i = 0; i < teamtype->TaskForce->ClassCount; i++) {
			TechnoTypeClass const * ttype = teamtype->TaskForce->Members[i].Class;
			if (ttype == NULL || ttype->Who_Can_Build_Me(true, true, true, this) == NULL) {
				if (Session.Type != GAME_NORMAL) return(false);
				bool found = false;
				for (int j = Feet.Count() - 1; j >= 0; j--) {
					FootClass * foot = Feet[j];
					if (foot->House == this && ttype == foot->TClass && teamtype->Can_Recruit(foot, this)) {
						found = true;
						break;
					}
				}
				if (!found) return(false);
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Updates what the computer is willing to spend its money on.
/// A hard up computer house alternates between structures and units rather than trying to
/// pay for both at once, favoring whichever it did not just finish, and it will insist
/// on structures while it lacks a barracks or a war factory or is running short of power.
/// Once the treasury recovers it goes back to building everything.
/// </summary>
/// <param name="type">The RTTI type of the object production has just dealt with.</param>
void HouseClass::Update_Production_Mode(RTTIType type)
{
	if (Is_Human_Player() || Session.Type == GAME_NORMAL) return;

	switch (ProductionMode) {
		case BUILDINGS:
			if (Available_Money() < Rule->AIAlternateProductionCreditCutoff) {
				if (type == RTTI_BUILDING) {
					ProductionMode = UNITS;
				}
			} else {
				ProductionMode = EVERYTHING;
			}
			break;

		case UNITS:
			if (Available_Money() < Rule->AIAlternateProductionCreditCutoff) {
				bool hasbarracks = false;
				for (int i = 0; i < Rule->BuildBarracks.Count(); i++) {
					if (!hasbarracks && ABQuantity.Value(Rule->BuildBarracks[i]->HeapID) <= 0) {
						hasbarracks = false;
					} else {
						hasbarracks = true;
					}
				}

				bool hasweapons = false;
				for (int j = 0; j < Rule->BuildWeapons.Count(); j++) {
					if (!hasweapons && ABQuantity.Value(Rule->BuildWeapons[j]->HeapID) <= 0) {
						hasweapons = false;
					} else {
						hasweapons = true;
					}
				}

				if (!hasweapons || !hasbarracks || Drain > Power || Random_Pick(0, 1) == 0) {
					if (type != RTTI_BUILDING) {
						ProductionMode = BUILDINGS;
					}
				}
			} else {
				ProductionMode = EVERYTHING;
			}
			break;

		case NOTHING:
			if (Available_Money() > Rule->AIAlternateProductionCreditCutoff) {
				ProductionMode = EVERYTHING;
			}
			break;

		case EVERYTHING:
			if (Available_Money() < Rule->AIAlternateProductionCreditCutoff) {
				ProductionMode = (type == RTTI_BUILDING) ? UNITS : BUILDINGS;
			}
			break;
	}
}


/// <summary>
/// Handles the computer's use of its charged super weapons.
/// This routine is called from the house AI. Every super weapon that has finished
/// charging is handed to the handler for its type, which picks a target and fires it.
/// Human houses are left to fire their own.
/// </summary>
void HouseClass::AI_Super_Weapons(void)
{
	if (!Is_Human_Player()) {
		for (int i = 0; i < SuperWeapon.Count(); i++) {
			SuperClass * super = SuperWeapon[i];

			if (super != NULL && super->Is_Ready()) {
				switch (super->Class->Type) {

					case SUPER_MULTI_MISSILE:
						AI_Multi_Missile(super);
						break;

					case SUPER_ION_CANNON:
						AI_Ion_Cannon(super);
						break;

					case SUPER_HUNTER_SEEKER:
						AI_Hunter_Seeker(super);
						break;

					case SUPER_CHEM_MISSILE:
						AI_Chem_Missile(super);
						break;

					case SUPER_DROP_PODS:
						AI_Drop_Pods(super);
						break;

					default:
						break;
				}
			}
		}
	}
}


/// <summary>
/// Handles the computer's use of the ion cannon super weapon.
/// This routine rates the enemy's objects by how much their loss would hurt -- engineers,
/// vehicle thieves, construction yards, war factories and power plants are all prized
/// well above ordinary troops -- and strikes one of the best rated. Only objects that
/// would actually die to the blast earn their premium rating, and cloaked objects are
/// rated at random so the computer does not appear to see through them.
/// </summary>
void HouseClass::AI_Ion_Cannon(SuperClass * super)
{
	if (Enemy == HOUSE_NONE) return;
	HouseClass * enemy = Houses[Enemy];

	DynamicVectorClass<AbstractClass *> targets;
	int best = 0;

	for (int i = 0; i < Technos.Count(); i++) {
		int value = 0;
		bool valid = false;
		TechnoClass * techno = Technos[i];

		if (techno->House == enemy) {
			value = 1;
			if (techno->In_Which_Layer() == LAYER_GROUND && techno->IsActive && !techno->IsInLimbo) {
				valid = true;
			} else if (Difficulty == DIFF_EASY) {
				for (int j = 0; j < Factories.Count(); j++) {
					FactoryClass * factory = Factories[j];
					if (factory->Get_Object() == techno && factory->Fetch_Rate() != 0 && !factory->IsSuspended) {
						valid = true;
					}
				}
			}

			switch (techno->Fetch_RTTI()) {
				case RTTI_INFANTRY:
					if (techno->Strength <= Rule->IonCannonDamage) {
						InfantryTypeClass const * inftype = ((InfantryClass *)techno)->Class;
						if (inftype->IsEngineer) {
							value = Rule->AIIonCannonEngineerValue[Difficulty];
						} else if (inftype->IsVehicleThief) {
							value = Rule->AIIonCannonThiefValue[Difficulty];
						} else {
							value = 2;
						}
					}
					break;

				case RTTI_BUILDING:
					value = 3;
					if (techno->Strength <= Rule->IonCannonDamage) {
						BuildingTypeClass const * builtype = ((BuildingClass *)techno)->Class;
						if (builtype->ToBuild == RTTI_BUILDINGTYPE) {
							value = Rule->AIIonCannonConYardValue[Difficulty];
						} else if (builtype->ToBuild == RTTI_UNITTYPE) {
							value = Rule->AIIonCannonWarFactoryValue[Difficulty];
						} else if (builtype->Power > builtype->Drain) {
							value = Rule->AIIonCannonPowerValue[Difficulty];
						} else if (builtype->IsBaseDefense) {
							value = Rule->AIIonCannonBaseDefenseValue[Difficulty];
						} else if (builtype->IsPlug) {
							value = Rule->AIIonCannonPlugValue[Difficulty];
						} else if (builtype->IsTemple) {
							value = Rule->AIIonCannonTempleValue[Difficulty];
						} else if (builtype->IsHoverPad) {
							value = Rule->AIIonCannonHelipadValue[Difficulty];
						} else {
							value = 4;
						}
					}
					break;

				case RTTI_UNIT:
					if (techno->Strength <= Rule->IonCannonDamage) {
						UnitTypeClass const * unittype = ((UnitClass *)techno)->Class;
						if (unittype->IsToHarvest) {
							value = Rule->AIIonCannonHarvesterValue[Difficulty];
						} else if (unittype->DeploysInto == Rule->BuildConst[0]) {
							value = Rule->AIIonCannonMCVValue[Difficulty];
						} else if (unittype->MaxPassengers > 0) {
							value = Rule->AIIonCannonAPCValue[Difficulty];
						} else {
							value = 2;
						}
					}
					break;
			}
		}

		if (techno->Cloak == CLOAKED || techno->RTTI == RTTI_BUILDING && ((BuildingClass*)techno)->TranslucencyLevel == 15) {
			value = Random_Pick(0, best + 10);
		}

		if (valid) {
			if (value > best) {
				targets.Clear();
				targets.Add(techno);
				best = value;
			} else if (value == best) {
				targets.Add(techno);
			}
		}
	}

	if (targets.Count() > 0) {
		AbstractClass * target = targets[Random_Pick(0, targets.Count() - 1)];
		if (target != NULL) {
			Cell cell = target->Center_Coord().As_Cell();
			if (cell != CELL_NONE) {
				Place_Special_Blast((SuperWeaponType)SuperWeapon.ID(super), cell);
			}
		}
	}
}


/// <summary>
/// Handles the computer's use of the hunter seeker super weapon.
/// The drone hunts down a victim of its own choosing, so this routine merely turns it
/// loose once the house has an enemy to send it after.
/// </summary>
void HouseClass::AI_Hunter_Seeker(SuperClass * super)
{
	if (Enemy != HOUSE_NONE) {
		Place_Special_Blast((SuperWeaponType)SuperWeapon.ID(super), CELL_NONE);
	}
}


/// <summary>
/// Handles the computer's use of the multi-missile super weapon.
/// The enemy structure standing in the most threatening spot is chosen as the target. A
/// building hidden by a cloaking field is rated at random instead, so that the computer
/// does not appear to see through it.
/// </summary>
void HouseClass::AI_Multi_Missile(SuperClass * super)
{
	if (Enemy != HOUSE_NONE) {

		/*
		**	Loop through all of the building objects on the map
		**	and see which ones are available.
		*/
		BuildingClass * bestptr = NULL;
		int best = -1;
		HouseClass * enemy = Houses[Enemy];

		for (int index = Buildings.Count() - 1; index >= 0; index--) {
			BuildingClass * b = Buildings[index];

			/*
			**	If the building is valid, not in limbo, not in the process of
			**	being destroyed and not our ally, then we can consider it.
			*/
			if (b->House == enemy) {
				int value = Map.Cell_Threat(b->Center_Coord().As_Cell(), *this);
				if (b->TranslucencyLevel == 15) {
					value = Random_Pick(0, 100);
				}
				if (value > best) {
					best = value;
					bestptr = b;
				}
			}
		}

		if (bestptr) {
			Cell cell = bestptr->Center_Coord().As_Cell();
			Place_Special_Blast((SuperWeaponType)SuperWeapon.ID(super), cell);
		}
	}
}


/// <summary>
/// Handles the computer's use of the chemical missile super weapon.
/// The enemy structure standing in the most threatening spot is chosen as the target. A
/// building hidden by a cloaking field is rated at random instead, so that the computer
/// does not appear to see through it.
/// </summary>
void HouseClass::AI_Chem_Missile(SuperClass * super)
{
	if (Enemy != HOUSE_NONE) {

		/*
		**	Loop through all of the building objects on the map
		**	and see which ones are available.
		*/
		BuildingClass * bestptr = NULL;
		int best = -1;
		HouseClass * enemy = Houses[Enemy];

		for (int index = Buildings.Count() - 1; index >= 0; index--) {
			BuildingClass * b = Buildings[index];

			/*
			**	If the building is valid, not in limbo, not in the process of
			**	being destroyed and not our ally, then we can consider it.
			*/
			if (b->House == enemy) {
				int value = Map.Cell_Threat(b->Center_Coord().As_Cell(), *this);
				if (b->TranslucencyLevel == 15) {
					value = Random_Pick(0, 100);
				}
				if (value > best) {
					best = value;
					bestptr = b;
				}
			}
		}

		if (bestptr) {
			Cell cell = bestptr->Center_Coord().As_Cell();
			Place_Special_Blast((SuperWeaponType)SuperWeapon.ID(super), cell);
		}
	}
}


/// <summary>
/// Invalidates the base node that a building occupies.
/// This routine is called when a building leaves the map. Any other node laying claim to
/// the same spot is released so the computer may build there again, and base defense
/// nodes are retired outright in multiplayer games, since the AI picks those spots for
/// itself rather than following the pre-built base list.
/// </summary>
/// <param name="building">The building whose base node location is to be invalidated.</param>
void HouseClass::Invalidate_Base_Node_Position(BuildingClass * building)
{
	if (!Debug_Map) {
		Cell cell = building->PositionCoord.As_Cell();
		for (int i = 0; i < Base.Nodes.Count(); i++) {
			if (Base.Nodes[i].Type == building->Class->HeapID && Base.Nodes[i].CellID == cell) {
				Cell node_cell = Base.Nodes[i].CellID;
				for (int j = 0; j < Base.Nodes.Count(); j++) {
					if (i != j && Base.Nodes[j].CellID == node_cell) {
						Base.Nodes[j].CellID = CELL_NONE;
					}
				}
				if (building->Class->IsBaseDefense && Session.Type != GAME_NORMAL) {
					Base.Nodes[i].Type = STRUCT_NONE;
					Base.Nodes[i].CellID = CELL_NONE;
				}
				return;
			}
		}
	}
}


/// <summary>
/// Converts a (formerly human) house into a computer-controlled house. Disowns the
/// player's factories, applies an AI handicap, re-centers the pre-built base list on
/// the existing construction yard, and inserts base-defense, wall-tower and power
/// nodes so the AI can maintain and expand the captured base.
/// </summary>
void HouseClass::AI_Takeover(void)
{
	if (!Is_Human_Player()) {
		return;
	}

	IsHuman = false;
	IsPlayerControl = false;
	IQ = Rule->MaxIQ;

	/*
	 * Rename the house to the generic computer name.
	 */
	IniName = Fetch_String(TXT_COMPUTER);

	/*
	 * Disown and disband any factories this house controls.
	 */
	int index;
	for (index = Factories.Count() - 1; index >= 0; index--) {
		FactoryClass * factory = Factories[index];
		if (factory->House == this) {
			Factories[index]->Suspend(true);
			Factories[index]->Abandon();
			delete Factories[index];
		}
	}

	Assign_Handicap((DiffType)(2 - Difficulty));

	/*
	 * Find the construction yard to center the rebuilt base around.
	 */
	BuildingClass * conyard = NULL;
	for (index = Buildings.Count() - 1; index >= 0; index--) {
		BuildingClass * bptr = Buildings[index];
		if (bptr->Class == Rule->BuildConst[0] && bptr->House == this && !bptr->IsInLimbo) {
			conyard = bptr;
			break;
		}
	}
	if (conyard == NULL) {
		return;
	}

	Cell center = conyard->PositionCoord.As_Cell();
	Center = Coord(center, 0);
	if (Base.Nodes.Count() == 0) {
		int save = ScenarioInit;
		ScenarioInit = 0;
		Make_Base_Nodes();
		ScenarioInit = save;
	}
	Base.Nodes[0].CellID = center;
	Base.PlacementCenter = center;
	IsStarted = true;
	IsAITriggersOn = true;
	IsBaseBuilding = true;

	/*
	 * Snap each pre-built base node onto a matching building that already exists.
	 */
	for (int j = Base.Nodes.Count() - 1; j >= 0; j--) {
		int type = Base.Nodes[j].Type;
		if (type >= STRUCT_FIRST && ABQuantity.Value(type) > 0) {
			BuildingTypeClass * btype = BuildingTypes[type];
			for (int k = Buildings.Count() - 1; k >= 0; k--) {
				BuildingClass * building = Buildings[k];
				if (building->House == this && !building->IsInLimbo
					&& (building->Class == btype || building->Upgrades[0] == btype
						|| building->Upgrades[1] == btype || building->Upgrades[2] == btype)) {
					Base.Nodes[j].CellID = building->PositionCoord.As_Cell();
				}
			}
		}
	}

	/*
	 * Sell off any walls this house owns.
	 */
	Map.Reset_Iterator();
	CellClass * cell = Map.Iterate();
	int l = 0;
	while (cell != NULL) {
		if (cell->Owner == HeapID) {
			OverlayType overlay = cell->Overlay;
			if (overlay != OVERLAY_NONE && OverlayTypes[overlay]->IsWall) {
				Sell_Wall(cell->CellID, true);
			}
		}
		cell = Map.Iterate();
	}
	/*
	 * Sell any wall-tower upgrades that are not real upgrades.
	 */
	for (index = Buildings.Count() - 1; index >= 0; index--) {
		BuildingClass * building = Buildings[index];
		if (building->House == this && building->Class == Rule->WallTower
			&& building->UpgradeLevel <= 0 && !building->IsInLimbo) {
			building->Sell_All_Upgrades();
		}
	}

	for (index = 0; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings[index];
		if (building->House != this || building->IsInLimbo) {
			continue;
		}
		BuildingTypeClass * btype = building->Class;
		if (!btype->IsBaseDefense && !(btype == Rule->WallTower && building->UpgradeLevel > 0)) {
			continue;
		}
		if (!btype->Who_Can_Build_Me(true, false, true, this)) {
			continue;
		}
		int count = Base.Nodes.Count();
		while (l < count) {
			if (Base.Nodes[l].Type == STRUCT_NONE) {
				if (building->Class == Rule->WallTower) {
					Base.Nodes.Insert_After(l, BaseNodeClass(building->Upgrades[building->UpgradeLevel - 1]->HeapID, building->PositionCoord.As_Cell()));
					Base.Nodes[l].Type = building->Class->HeapID;
					Base.Nodes[l].CellID = building->PositionCoord.As_Cell();
					l++;
				} else {
					Base.Nodes[l].Type = btype->HeapID;
					Base.Nodes[l].CellID = building->PositionCoord.As_Cell();
				}
				l++;
				break;
			}
			l++;
		}
	}

	int node_index = 0;
	int power_add = 0;
	index = 0;
	int drain_add = 0;
	for (; index < Buildings.Count(); index++) {
		BuildingClass * building = Buildings[index];
		if (building->House != this || building->IsInLimbo) {
			continue;
		}
		BuildingTypeClass * btype = building->Class;
		if (btype != Rule->GDIPowerPlant && btype != Rule->NodRegularPower && btype != Rule->NodAdvancedPower) {
			continue;
		}
		if (building->PositionCoord.As_Cell() == Base.Nodes[1].CellID) {
			continue;
		}
		if (!building->Class->Who_Can_Build_Me(1, 0, 1, this)) {
			continue;
		}
		int count = Base.Nodes.Count();
		while (node_index < count) {
			if (Base.Nodes[node_index].Type >= STRUCT_FIRST) {
				BuildingTypeClass * ntype = BuildingTypes[Base.Nodes[node_index].Type];
				power_add += ntype->Power;
				drain_add += ntype->Drain;
				if (node_index > 0 && power_add < drain_add + PowerSurplus) {
					Base.Nodes.Insert_After(node_index - 1, BaseNodeClass(building->Class->HeapID, building->PositionCoord.As_Cell()));
					power_add += building->Class->Power;
					drain_add -= ntype->Drain;
					node_index++;
					break;
				}
			}
			node_index++;
		}
	}

	/*
	 * GDI bases gain power-turbine upgrade nodes for each upgradeable power plant.
	 */
	if (stricmp(Class->IniName, "GDI") == 0) {
		int bindex = 0;
		int power = 0;
		int drain = 0;
		for (index = 0; index < Buildings.Count(); index++) {
			BuildingClass * building = Buildings[index];
			if (building->House == this && !building->IsInLimbo && building->Class == Rule->GDIPowerPlant && building->UpgradeLevel > 0) {
				for (int upgrade = 0; upgrade < building->UpgradeLevel; upgrade++) {
					while (bindex < Base.Nodes.Count()) {
						if (Base.Nodes[bindex].Type >= STRUCT_FIRST) {
							BuildingTypeClass * ntype = BuildingTypes[Base.Nodes[bindex].Type];
							power += ntype->Power;
							drain += ntype->Drain;
							if (bindex > 0 && power < drain + PowerSurplus) {
								Base.Nodes.Insert_After(bindex - 1, BaseNodeClass(Rule->GDIPowerTurbine->HeapID, building->PositionCoord.As_Cell()));
								power += Rule->GDIPowerTurbine->Power;
								drain -= ntype->Drain;
								bindex++;
								break;
							}
						}
						bindex++;
					}
				}
			}
		}
	}
}


/// <summary>
/// Updates the availability of this house's super weapons.
/// A super weapon that depends on a building is suspended while that building is shut
/// down or the base is short of power, and taken away entirely once the building is gone
/// or the house has been defeated. The sidebar is flagged for redraw and any targeting
/// mode cancelled when the local player is the one affected.
/// </summary>
void HouseClass::Update_Present_Super_Weapons(void)
{
	if (GameActive) {
		for (int s = 0; s < SuperWeapon.Count(); s++) {
			if (SuperWeapon[s]->Is_Present() && ((SuperWeapon[s]->NeedsBuilding && !SuperWeapon[s]->Is_One_Time()) || IsDefeated)) {
				bool present = false;
				bool enabled = false;
				if (!IsDefeated) {
					for (int j = 0; j < Buildings.Count(); j++) {
						BuildingClass * b = Buildings[j];
						if (!b->IsInLimbo && b->IsActive && b->House->Fetch_ID() == Fetch_ID()) {
							for (int k = 0; k < 3; k++) {
								BuildingTypeClass const * upgrade = b->Upgrades[k];
								if (upgrade != NULL && (upgrade->SuperWeapon == s || upgrade->SuperWeapon2 == s)) {
									present = true;
									if (!enabled) enabled = b->IsOn;
								}
							}
							if (b->Fetch_Super_Weapon() == s || b->Fetch_Super_Weapon2() == s) {
								present = true;
								if (enabled) break;
								enabled = b->IsOn;
							}
							if (enabled && present) break;
						}
					}
				}

				int power = Power;
				int drain = Drain;
				if (power < drain && drain != 0) {
					if (power == 0 || (double)power / (double)drain < 1.0) {
						enabled = false;
					}
				}

				if (present && !IsDefeated) {
					if (!enabled && SuperWeapon[s]->Is_Powered() && SuperWeapon[s]->Suspend(true)) {
						if (PlayerPtr != NULL && Fetch_ID() == PlayerPtr->Fetch_ID()) {
							if (Map.IsTargettingMode == s) {
								Map.IsTargettingMode = SUPER_NONE;
							}
							Map.Column[1].Flag_To_Redraw();
						}
						IsRecalcNeeded = true;
					}

					if (enabled && SuperWeapon[s]->Suspend(false)) {
						if (PlayerPtr != NULL && Fetch_ID() == PlayerPtr->Fetch_ID()) {
							if (s == Map.IsTargettingMode) {
								Map.IsTargettingMode = SUPER_NONE;
							}
							Map.Column[1].Flag_To_Redraw();
						}
						IsRecalcNeeded = true;
					}
				} else {
					if (SuperWeapon[s]->Remove() && PlayerPtr != NULL) {
						if (Fetch_ID() == PlayerPtr->Fetch_ID()) {
							if (Map.IsTargettingMode == s) {
								Map.IsTargettingMode = SUPER_NONE;
							}
							Map.Column[1].Flag_To_Redraw();
						}
						IsRecalcNeeded = true;
					}
				}
			}
		}
	}
}


/// <summary>
/// Enables every super weapon this house has the buildings for.
/// This routine looks over the house's structures, upgrades included, and switches on any
/// super weapon they grant that is not already available. The local player also gains a
/// sidebar button for each one turned on.
/// </summary>
void HouseClass::Enable_Available_Super_Weapons(void)
{
	if (!IsDefeated) {
		for (int s = 0; s < SuperWeapon.Count(); s++) {
			if (!SuperWeapon[s]->Is_Present() || SuperWeapon[s]->Is_One_Time()) {
				bool from_upgrade = false;
				int j = Buildings.Count() - 1;
				if (j >= 0) {
					while (true) {
						BuildingClass * b = Buildings[j];
						if (b->IsActive && b->House == this && !b->IsInLimbo) {
							for (int k = 0; k < BUILDING_UPGRADE_MAX; k++) {
								BuildingTypeClass const * upgrade = b->Upgrades[k];
								if (upgrade != NULL && (upgrade->SuperWeapon == s || upgrade->SuperWeapon2 == s)) {
									from_upgrade = true;
									break;
								}
							}
							if (b->Fetch_Super_Weapon() == s || b->Fetch_Super_Weapon2() == s || from_upgrade) {
								SuperWeapon[s]->Enable(false, this == PlayerPtr, Power_Fraction() < 1.0);
								if (this == PlayerPtr) {
									Map.Add(RTTI_SPECIAL, s);
									Map.Column[1].Flag_To_Redraw();
								}
								break;
							}
						}
						j--;
						if (j < 0) {
							break;
						}
					}
				}
			}
		}
	}
}


/// <summary>
/// Has this house used up the build limit for an object type?
/// This routine is consulted by the production and sidebar logic so that a limited type
/// stops being offered once the house owns, or has queued, its allotment. A negative
/// limit tallies every one ever produced; a positive limit tallies only the survivors.
/// Vehicle thieves also count the vehicles they are currently sitting in.
/// </summary>
/// <param name="type">The object type to check the build limit of.</param>
/// <returns>bool; Has the limit been used up? An invalid type is always exhausted.</returns>
bool HouseClass::Is_Build_Limited(TechnoTypeClass const * type) const
{
	if (type == NULL) {
		return(true);
	} else {
		int queued = 0;
		FactoryClass * factory = Fetch_Factory(type->RTTI);
		if (factory != NULL) {
			queued = factory->Total(type);
		}

		switch ((RTTIType)type->RTTI) {
		case RTTI_UNITTYPE:
			if (type->BuildLimit <= 0) {
				if (queued + PUQuantity.Value(type->Fetch_Heap_ID()) >= abs(type->BuildLimit)) {
					return(true);
				}
			}
			if (type->BuildLimit > 0) {
				if (queued + UQuantity.Value(type->Fetch_Heap_ID()) >= type->BuildLimit) {
					return(true);
				}
			}
			break;

		case RTTI_INFANTRYTYPE: {
			if (type->BuildLimit <= 0) {
				if (queued + PIQuantity.Value(type->Fetch_Heap_ID()) >= abs(type->BuildLimit)) {
					return(true);
				}
			}
			InfantryTypeClass const * inft = (InfantryTypeClass const *)type;
			int owned = IQuantity.Value(type->Fetch_Heap_ID());
			if (inft->IsVehicleThief) {
				for (int i = 0; i < Units.Count(); i++) {
					UnitClass * unit = Units[i];
					if (unit->House == this && unit->EnteredByInfType == type->Fetch_Heap_ID()) {
						owned++;
					}
				}
			}
			if (type->BuildLimit > 0) {
				if (queued + owned >= type->BuildLimit) {
					return(true);
				}
			}
			break;
		}

		case RTTI_AIRCRAFTTYPE:
			if (type->BuildLimit <= 0) {
				if (queued + PAQuantity.Value(type->Fetch_Heap_ID()) >= abs(type->BuildLimit)) {
					return(true);
				}
			}
			if (type->BuildLimit > 0) {
				if (type != NULL) {
					if (queued + AQuantity.Value(type->Fetch_Heap_ID()) >= type->BuildLimit) {
						return(true);
					}
				}
			}
			break;

		default:
			break;
		}
	}
	return(false);
}


/// <summary>
/// Is this house driven from the local machine?
/// A campaign mission may hand the player more than one side, so any house flagged as
/// human or as player controlled counts. Network games recognize only the local player's
/// own house.
/// </summary>
/// <returns>bool; Is this house under local player control?</returns>
bool HouseClass::Is_Player_Control(void) const
{
	if (Session.Type == GAME_NORMAL) {
		return(IsHuman || IsPlayerControl);
	}
	return(this == PlayerPtr) == true;
}


/// <summary>
/// Is this house run by a human?
/// In campaign games the question collapses to whether the house answers to the local
/// player, since the local player is the only human in the game.
/// </summary>
/// <returns>bool; Is this house controlled by a human player?</returns>
bool HouseClass::Is_Human_Player(void) const
{
	if (Session.Type == GAME_NORMAL) {
		return(Is_Player_Control());
	}
	return(IsHuman);
}


/// <summary>
/// Can this house place a building at the specified location?
/// This routine keeps a computer base compact by requiring a candidate site to touch
/// something the house already occupies. Campaign games impose no such restriction.
/// </summary>
/// <param name="cell">The upper left cell of the proposed building location.</param>
/// <returns>bool; Is the location acceptable to build at?</returns>
bool HouseClass::Can_Build_Here(BuildingTypeClass *building, Cell const & cell)
{
	if (Session.Type == GAME_NORMAL) {
		return(true);
	}

	int spacing = Rule->AIBaseSpacing;
	int width = (2 * spacing) + building->Width();
	int height = (2 * spacing) + building->Height();

	int mask = 1 << HeapID;

	for (int x = cell.X - spacing - 1; x < cell.X + width + 1; x++) {
		for (int y = cell.Y - spacing - 1; y < cell.Y + height + 1; y++) {
			if (Map[Cell(x, y)].OccupiedBy & mask) {
				return(true);
			}
		}
	}

	return(false);
}


/// <summary>
/// Sets up the radar remap color for this house.
/// This routine derives the house's remap color from its assigned color scheme, in the
/// pixel format the display is currently running in.
/// </summary>
/// <remarks>Call this routine again if the house color scheme or the display format
/// changes.</remarks>
void HouseClass::Initialize_Radar_Color(void)
{
	ColorScheme * scheme = ColorSchemes[Scheme];
	unsigned short c = scheme->Converter->Convert_Pixel(scheme->Bright);
	int r = (c >> DSurface::RedRight) << DSurface::RedLeft;
	int g = (c >> DSurface::GreenRight) << DSurface::GreenLeft;
	int b = (c >> DSurface::BlueRight) << DSurface::BlueLeft;
	RemapColorRGB = RGBClass(r,g,b);
}


/// <summary>
/// Handles the computer's use of the drop pod super weapon.
/// This routine picks a landing site out along a randomly chosen edge of the map and
/// fires the super weapon at it, provided the house has an enemy worth harassing.
/// </summary>
void HouseClass::AI_Drop_Pods(SuperClass * super)
{
	if (Enemy != HOUSE_NONE) {
		Cell cell = CELL_NONE;
		Cell random_cell = Random_Cell_In_Zone(Random_Pick(ZONE_NORTH, ZONE_WEST));
		cell = Map.Nearby_Location(random_cell, SPEED_FOOT);
		if (cell != CELL_NONE) {
			Place_Special_Blast((SuperWeaponType)SuperWeapon.ID(super), cell);
		}
	}
}


/// <summary>
/// Adds a reference to this house.
/// Houses are permanent heap objects rather than reference counted ones, so this routine
/// exists only to satisfy the IUnknown contract.
/// </summary>
/// <returns>Returns with the reference count, which is always one.</returns>
ULONG STDMETHODCALLTYPE HouseClass::AddRef(void)
{
	return(1);
}


/// <summary>
/// Releases a reference to this house.
/// Houses are permanent heap objects rather than reference counted ones, so this routine
/// exists only to satisfy the IUnknown contract. It never destroys the house.
/// </summary>
/// <returns>Returns with the reference count, which is always one.</returns>
ULONG STDMETHODCALLTYPE HouseClass::Release(void)
{
	return(1);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_HOUSE.</returns>
RTTIType HouseClass::Fetch_RTTI(void) const
{
	return(RTTI_HOUSE);
}


/// <summary>
/// Fetches the heap index of this house.
/// </summary>
/// <returns>Returns with the index of this house within the house heap.</returns>
int HouseClass::Fetch_Heap_ID(void) const
{
	return(HeapID);
}
