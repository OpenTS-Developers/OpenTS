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

/* $Header: /CounterStrike/AIRCRAFT.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : AIRCRAFT.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : July 22, 1994                                                *
 *                                                                                             *
 *                  Last Update : November 2, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AircraftClass::AI -- Processes the normal non-graphic AI for the aircraft.                *
 *   AircraftClass::Active_Click_With -- Handles clicking over specified cell.                 *
 *   AircraftClass::Active_Click_With -- Handles clicking over specified object.               *
 *   AircraftClass::AircraftClass -- The constructor for aircraft objects.                     *
 *   AircraftClass::Can_Enter_Cell -- Determines if the aircraft can land at this location.    *
 *   AircraftClass::Can_Fire -- Checks to see if the aircraft can fire.                        *
 *   AircraftClass::Cell_Seems_Ok -- Checks to see if a cell is good to enter.                 *
 *   AircraftClass::Desired_Load_Dir -- Determines where passengers should line up.            *
 *   AircraftClass::Draw_It -- Renders an aircraft object at the location specified.           *
 *   AircraftClass::Draw_Rotors -- Draw rotor blades on the aircraft.                          *
 *   AircraftClass::Edge_Of_World_AI -- Detect if aircraft has exited the map.                 *
 *   AircraftClass::Enter_Idle_Mode -- Gives the aircraft an appropriate mission.              *
 *   AircraftClass::Exit_Object -- Unloads passenger from aircraft.                            *
 *   AircraftClass::Fire_At -- Handles firing a projectile from an aircraft.                   *
 *   AircraftClass::Fire_Direction -- Determines the direction of fire.                        *
 *   AircraftClass::Good_Fire_Location -- Searches for and finds a good spot to fire from.     *
 *   AircraftClass::Good_LZ -- Locates a good spot ot land.                                    *
 *   AircraftClass::In_Which_Layer -- Calculates the display layer of the aircraft.            *
 *   AircraftClass::Init -- Initialize the aircraft system to an empty state.                  *
 *   AircraftClass::Is_LZ_Clear -- Determines if landing zone is free for landing.             *
 *   AircraftClass::Landing_Takeoff_AI -- Handle aircraft take off and landing processing.     *
 *   AircraftClass::Look -- Aircraft will look if they are on the ground always.               *
 *   AircraftClass::Mission_Attack -- Handles the attack mission for aircraft.                 *
 *   AircraftClass::Mission_Enter -- Control aircraft to fly to the helipad or repair center.  *
 *   AircraftClass::Mission_Guard -- Handles aircraft in guard mode.                           *
 *   AircraftClass::Mission_Guard_Area -- Handles the aircraft guard area logic.               *
 *   AircraftClass::Mission_Hunt -- Maintains hunt AI for the aircraft.                        *
 *   AircraftClass::Mission_Move -- Handles movement mission.                                  *
 *   AircraftClass::Mission_Retreat -- Handles the aircraft logic for leaving the battlefield. *
 *   AircraftClass::Mission_Unload -- Handles unloading cargo.                                 *
 *   AircraftClass::Movement_AI -- Handles aircraft physical movement logic.                   *
 *   AircraftClass::New_LZ -- Find a good landing zone.                                        *
 *   AircraftClass::Overlap_List -- Returns with list of cells the aircraft overlaps.          *
 *   AircraftClass::Paradrop_Cargo -- Drop a passenger by parachute.                           *
 *   AircraftClass::Per_Cell_Process -- Handle the aircraft per cell process.                  *
 *   AircraftClass::Pip_Count -- Returns the number of "objects" in aircraft.                  *
 *   AircraftClass::Player_Assign_Mission -- Handles player input to assign a mission.         *
 *   AircraftClass::Pose_Dir -- Fetches the natural landing facing.                            *
 *   AircraftClass::Process_Fly_To -- Handles state machine for flying to destination.         *
 *   AircraftClass::Process_Landing -- Landing process state machine handler.                  *
 *   AircraftClass::Process_Take_Off -- State machine support for taking off.                  *
 *   AircraftClass::Read_INI -- Reads aircraft object data from an INI file.                   *
 *   AircraftClass::Receive_Message -- Handles receipt of radio messages.                      *
 *   AircraftClass::Response_Attack -- Gives audio response to attack order.                   *
 *   AircraftClass::Response_Move -- Gives audio response to move request.                     *
 *   AircraftClass::Response_Select -- Gives audio response when selected.                     *
 *   AircraftClass::Rotation_AI -- Handle aircraft body and flight rotation.                   *
 *   AircraftClass::Scatter -- Causes the aircraft to move away a bit.                         *
 *   AircraftClass::Set_Speed -- Sets the speed for the aircraft.                              *
 *   AircraftClass::Shape_Number -- Fetch the shape number to use for the aircraft.            *
 *   AircraftClass::Sort_Y -- Figures the sorting coordinate.                                  *
 *   AircraftClass::Take_Damage -- Applies damage to the aircraft.                             *
 *   AircraftClass::Unlimbo -- Removes an aircraft from the limbo state.                       *
 *   AircraftClass::What_Action -- Determines what action to perform.                          *
 *   AircraftClass::What_Action -- Determines what action to perform.                          *
 *   AircraftClass::operator delete -- Deletes the aircraft object.                            *
 *   AircraftClass::operator new -- Allocates a new aircraft object from the pool              *
 *   AircraftClass::~AircraftClass -- Destructor for aircraft object.                          *
 *   _Counts_As_Civ_Evac -- Is the specified object a candidate for civilian evac logic?       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "aircraft.h"

#include "_map.h"
#include "_rtti.h"
#include "_rules.h"
#include "_tactica.h"
#include "airctype.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "ccrand.h"
#include "cell.h"
#include "dbgprint.h"
#include "findmake.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "map.h"
#include "mono.h"
#include "partsys.h"
#include "queue.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "session.h"
#include "stimer.h"
#include "sun.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "tracker.h"
#include "unit.h"
#include "unittype.h"
#include "waypoint.h"
#include "weapon.h"

#include <algorithm>


char const * const AircraftClass::INI_NAME = "Aircraft";

/***********************************************************************************************
 * _Counts_As_Civ_Evac -- Is the specified object a candidate for civilian evac logic?         *
 *                                                                                             *
 *    Examines the specified object to see if it qualifies to be a civilian evacuation. This   *
 *    can only occur if it is a civilian (or Tanya) and the special evacuation flag has been   *
 *    set in the scenario control structure.                                                   *
 *                                                                                             *
 * INPUT:   candidate   -- Candidate object to examine for civilian evacuation legality.       *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified object considered a civilian that must be auto-evacuated?   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Counts_As_Civ_Evac(ObjectClass const * candidate)
{
	/*
	**	If the candidate pointer is missing, then return with failure code.
	*/
	if (candidate == NULL) return(false);

	/*
	**	Only infantry objects can be considered for civilian evacuation action.
	*/
	InfantryClass const * inf = (candidate->RTTI == RTTI_INFANTRY) ? (InfantryClass const *)candidate : NULL;

	if (inf == NULL) return(false);

	/*
	**	If the infantry is not a civilian, then it isn't allowed to be a civilian evacuation.
	*/
	if (!inf->Class->IsCivilian) return(false);

	/*
	**	Technicians look like civilians, but are not considered a legal evacuation candidate.
	*/
	if (inf->IsTechnician) return(false);

	/*
	**	All tests pass, so return the success of the infantry as a civilian evacuation candidate.
	*/
	return(true);
}


/***********************************************************************************************
 * AircraftClass::AircraftClass -- The constructor for aircraft objects.                       *
 *                                                                                             *
 *    This routine is the constructor for aircraft objects. An aircraft object can be          *
 *    created and possibly placed into the game system by this routine.                        *
 *                                                                                             *
 * INPUT:   classid  -- The type of aircraft to create.                                        *
 *                                                                                             *
 *          house    -- The owner of this aircraft.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftClass::AircraftClass(AircraftTypeClass const * type, HouseClass * house) :
	BASECLASS(house),
	IsToSpendAmmo(false),
	Class((AircraftTypeClass *)type),
	Passenger(false),
	IsKamikaze(false),
	field_35B(false),
	IsLockedStraight(false),
	SightTimer(0),
	AttacksRemaining(1),
	IsReadyToCommence(true)
{
	Create_ID();

	if (Class != NULL) {
		Locomotion.CreateInstance(Class->Locomotor);
		Locomotion->Link_To_Object(this);
	}

	Init();

	Aircraft.Add(this);

	TargetTracker.Add_Index(Fetch_ID(), this);
}


/***********************************************************************************************
 * AircraftClass::Init -- Initialize the aircraft system to an empty state.                    *
 *                                                                                             *
 *    This routine is used to clear out the aircraft allocation system. It is called in        *
 *    preparation for a scenario load or save game load.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Init(void)
{
	BASECLASS::Init();
	if (Class != NULL) {
		PrimaryFacing.Set_ROT(Class->ROT);
		SecondaryFacing.Set_ROT(Class->ROT);
		SecondaryFacing.Set(PrimaryFacing.Current());
		HeightAGL = Class->Flight_Level();
		Ammo = Class->MaxAmmo;
		Strength = Class->MaxStrength;
	}

	if (House != NULL) {
		House->Tracking_Add(this);
	}
}


/// <summary>
/// Fetches the requested interface from this aircraft.
/// Aircraft add the fly control interface to the set that every game object supports, so
/// that the flying locomotor can interrogate them about how they wish to be flown.
/// </summary>
/// <param name="guid">The identifier of the interface being asked for.</param>
/// <param name="ppv">Pointer to the pointer to fill in with the interface.</param>
/// <returns>Returns with S_OK if the interface was supplied.</returns>
HRESULT STDMETHODCALLTYPE AircraftClass::QueryInterface(struct _GUID const &guid, void **ppv)
{
	HRESULT res = BASECLASS::QueryInterface(guid, ppv);
	if (FAILED(res)) {
		if (guid == IID_IFlyControl) {
			*ppv = (IFlyControl *)(this);
		}
		res = S_OK;
		AddRef();
	}
	return(res);
}


/// <summary>
/// Adds a reference to this aircraft.
/// </summary>
/// <returns>Returns with the new number of references outstanding.</returns>
ULONG STDMETHODCALLTYPE AircraftClass::AddRef(void)
{
	return(BASECLASS::AddRef());
}


/// <summary>
/// Releases a reference to this aircraft.
/// </summary>
/// <returns>Returns with the number of references still outstanding.</returns>
ULONG STDMETHODCALLTYPE AircraftClass::Release(void)
{
	return(BASECLASS::Release());
}


/***********************************************************************************************
 * AircraftClass::Unlimbo -- Removes an aircraft from the limbo state.                         *
 *                                                                                             *
 *    This routine is used to transition the aircraft from the limbo to the non limbo state.   *
 *    It occurs when the aircraft is placed on the map for whatever reason. When it is         *
 *    unlimboed, only then will normal game processing recognize it.                           *
 *                                                                                             *
 * INPUT:   coord -- The coordinate that the aircraft should appear at.                        *
 *                                                                                             *
 *          dir   -- The direction it should start facing.                                     *
 *                                                                                             *
 *            strength (optional) -- sets initial strength                                     *
 *                                                                                             *
 *            mission (optional) -- sets initial mission                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the aircraft unlimboed successfully?                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Unlimbo(Coord const & coord, Dir256 dir)
{
	Coord ucoord = coord;
	if (IsALoaner || !Map.In_Local_Radar(coord)) {
		ucoord.Z = Class->Flight_Level() + Map.Get_Height_GL(coord);
	} else {
		ucoord.Z = Map.Get_Height_GL(coord);
	}

	if (BASECLASS::Unlimbo(ucoord, dir)) {

		if (!Class->IsSelectable || !Class->IsLandable || (PrimaryWeapon != NULL && PrimaryWeapon->IsCamera)) {
			IsALoaner = true;
		}

		/*
		**	Hack it so that aircraft that are both passenger and cargo carrying
		**	will carry passengers at the expense of ammo.
		*/
		if (Cargo.Is_Something_Attached()) {
			Ammo = 0;
			Passenger = true;
		}

		/*
		**	Forces the body of the helicopter to face the correct direction.
		*/
		SecondaryFacing.Set(dir);

		/*
		**	Start rotor animation.
		*/
		Set_Rate(1);
		Set_Stage(0);

		/*
		**	When starting at flight level, then give it speed. When landed
		**	then it must be stationary.
		*/
		if (HeightAGL == Class->Flight_Level()) {
			Set_Speed(1);
		} else {
			Set_Speed(0);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Draw_It -- Renders an aircraft object at the location specified.             *
 *                                                                                             *
 *    This routine is used to display the aircraft object at the coordinates specified.        *
 *    The tactical map display uses this routine for all aircraft rendering.                   *
 *                                                                                             *
 * INPUT:   x,y      -- The coordinates to render the aircraft at.                             *
 *                                                                                             *
 *          window   -- The window that the coordinates are based upon.                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Draw_It(Point2D const & xpoint, Rect const & cliprect) const
{
	if (!Debug_Map && MainWindow && Scen->Special.IsFogOfWar) {
		Coord headto = (Coord)Locomotion->Head_To_Coord();
		headto.Z = PositionCoord.Z;
		if (Map.Is_Fogged(headto) && Map.Is_Fogged(PositionCoord) && !House->Is_Player_Control()) {
			return;
		}
	}

	Point2D point = xpoint;
	point += Locomotion->Draw_Point();

	if (Cargo.Is_Something_Attached() && Class->IsCarryall) {
		Cargo.Attached_Object()->Draw_It(point, cliprect);
	}

	if (Class->IsVoxel && Class->Voxel.VoxLib != NULL) {
		Coord coord = PositionCoord;
		coord.Z = Map.Get_Height_GL(coord);

		Point2D shadow;
		TacticalMap->Coord_To_Pixel(coord, shadow);
		shadow += Locomotion->Shadow_Point();

		int key = 0;
		int height = HeightAGL;

		bool occupies_cell = Occupies_Cells();
		((AircraftClass &)*this).IsOccupyingCell = false;

		CellClass * cellptr = &Map[Get_Coord()];

		bool draw_on_ground = false;
		if ((IsOnBridge || height < BRIDGE_LEPTON_HEIGHT) && (!IsOnBridge || height < 0)) {
			draw_on_ground = true;
		} else {
			if (cellptr->IsUnderBridge &&
				(cellptr->IsBridgeEastWest && cellptr->Adjacent_Cell(FACING_N).IsUnderBridge ||
				!cellptr->IsBridgeEastWest && cellptr->Adjacent_Cell(FACING_W).IsUnderBridge)) {

				((AircraftClass &)*this).HeightAGL = BRIDGE_LEPTON_HEIGHT;
				shadow.Y -= TacticalMap->Z_Lepton_To_Pixel(BRIDGE_LEPTON_HEIGHT);
			} else {
				draw_on_ground = true;
			}
		}

		if (draw_on_ground) {
			((AircraftClass &)*this).HeightAGL = 0;
		}

		/*
		**	Special manual shadow draw code.
		*/
		Matrix3D matrix;
		matrix = Locomotion->Shadow_Matrix(&key);
		Draw_Voxel_Shadow(Class->Voxel, 0, key, &Class->ShadowVoxelIndex, cliprect, shadow, Get_Isometric_View_Matrix() * matrix, true);

		((AircraftClass &)*this).HeightAGL = height;
		((AircraftClass &)*this).IsOccupyingCell = occupies_cell;

		TacticalMap->Add_To_Selectables((AircraftClass *)this, point);

		int brightness;
		if (IonStormClass::Is_Ion_Storm_Active()) {
			brightness = Scen->IonLevelLight;
		} else {
			brightness = Scen->LevelLight;
		}

		brightness *= (HeightAGL / (2 * LEVEL_LEPTON_H));
		int newbrightness = brightness + Map[coord].Brightness + Rule->ExtraAircraftLight;

		/*
		**	Actually draw the root body of the unit.
		*/
		key = -1;
		matrix = Locomotion->Draw_Matrix(&key);
		Draw_Voxel(Class->Voxel, 0, key, &Class->VoxelIndex, cliprect, point, Get_Isometric_View_Matrix() * matrix, newbrightness, SHAPE_NORMAL);
	}

	/*
	**	This draws any overlay graphics on the aircraft.
	*/
	BASECLASS::Draw_It(xpoint, cliprect);
}


/***********************************************************************************************
 * AircraftClass::Draw_Rotors -- Draw rotor blades on the aircraft.                            *
 *                                                                                             *
 *    This routine will draw rotor blades on the aircraft. It is presumed that the aircraft    *
 *    has already been drawn at the X and Y pixel coordinates specified.                       *
 *                                                                                             *
 * INPUT:   x,y   -- The X and Y pixel coordinates to draw the rotor blades.                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Draw_Rotors(Point2D const & xy, Rect const & cliprect) const
{
	ShapeFlags_Type flags = ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL);
	int shapenum;

	/*
	**	The rotor shape number depends on whether the helicopter is idling
	**	or not. A landed helicopter uses slow moving "idling" blades.
	*/
	if (HeightAGL == 0) {
		shapenum = (Fetch_Stage()%8)+4;
		flags = flags;
	} else {
		shapenum = Fetch_Stage()%4;
		flags = ShapeFlags_Type(flags|SHAPE_PREDATOR);
	}

	#if 0
	if (*this == AIRCRAFT_TRANSPORT) {
		int _stretch[FACING_COUNT] = {8, 9, 10, 9, 8, 9, 10, 9};

		/*
		**	Dual rotors offset along flight axis.
		*/
		short xx = x;
		short yy = y-LEPTON_TO_PIXEL(Height);
		FacingType face = Dir_Facing(SecondaryFacing);
		Move_Point(xx, yy, SecondaryFacing.Current(), _stretch[face]);
		Draw_Shape(AircraftTypeClass::RRotorData, shapenum, xx, yy-2, window, flags, NULL, DisplayClass::UnitShadow);

		Move_Point(xx, yy, SecondaryFacing.Current()+DIR_S, _stretch[face]*2);
		Draw_Shape(AircraftTypeClass::LRotorData, shapenum, xx, yy-2, window, flags, NULL, DisplayClass::UnitShadow);

	} else {

		/*
		**	Single rotor centered about shape.
		*/
		Draw_Shape(AircraftTypeClass::RRotorData, shapenum, x, ((y-LEPTON_TO_PIXEL(Height))-2), window, flags, NULL, DisplayClass::UnitShadow);
	}
	#endif
}


/***********************************************************************************************
 * AircraftClass::Mission_Hunt -- Maintains hunt AI for the aircraft.                          *
 *                                                                                             *
 *    Hunt AI consists of finding a target and attacking it. If there is no target assigned    *
 *    and this unit doesn't automatically hunt for more targets, then it will change           *
 *    mission to a more passive (land and await further orders) type.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of ticks before calling this routine again.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_HUNT(void)
{
	if (!Ammo) {
		if (Team) Team->Remove(this);
		Enter_Idle_Mode();
	} else {
		if (TarCom == NULL) {
			if (Session.Type != GAME_NORMAL) {
				Assign_Target(Greatest_Threat(THREAT_TIBERIUM, PositionCoord, false));
			}
			if (TarCom == NULL) {
				Assign_Target(Greatest_Threat(THREAT_NORMAL, PositionCoord, false));
			}
			if (TarCom == NULL) {
				Enter_Idle_Mode();
				return(1);
			}
		}

		Assign_Mission(MISSION_ATTACK);
		return(1);
	}
	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * AircraftClass::AI -- Processes the normal non-graphic AI for the aircraft.                  *
 *                                                                                             *
 *    This handles the non-graphic AI processing for the aircraft. This usually entails        *
 *    maintenance and other AI functions.                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::AI(void)
{
	if (Mission == MISSION_SLEEP && HeightAGL > 0) {
		Assign_Mission(MISSION_GUARD);
	}

	if (CurrentMission != MISSION_ATTACK) {
		IsLockedStraight = false;
	}

	if (TarCom != NULL) {
		if (TarCom->In_Air()) {
			Assign_Target(NULL);
		} else if (TarCom->Is_Techno() && !House->Is_Ally(TarCom) && ((TechnoClass*)TarCom)->Cloak == CLOAKED && !((TechnoClass*)TarCom)->Is_Sensed_By_House(House)) {
			Assign_Target(NULL);
		}
	}

	/*
	**	Perform any base class AI processing. If during this process, the aircraft was
	**	destroyed, then detect this and bail from this AI routine early.
	*/
	BASECLASS::AI();
	if (!IsActive) {
		return;
	}

	if (!Map.In_Local_Radar(PositionCell) && Should_Delete_Off_Map()) {
		Delete_Me();
		return;
	}

	if (NavCom == &BlubCell) {
		Assign_Destination(NULL);
		Assign_Target(NULL);
		Enter_Idle_Mode();
	}

	if (TarCom == &BlubCell) {
		Assign_Destination(NULL);
		Assign_Target(NULL);
		Enter_Idle_Mode();
	}

	if (Ready_To_Commence()) {
		Commence();
	}

	if (IsToSpendAmmo) {
		if (CurrentMission != MISSION_ATTACK) {
			IsToSpendAmmo = false;
			Ammo--;
		}
	}

	if (House->Is_Ally(PlayerPtr) && SightTimer == 0) {
		Look();
		SightTimer = TICKS_PER_SECOND;
	}

	if (HealthRatio < Rule->ConditionRed && HeightAGL > 0) {
		if (Percent_Chance(Strength != 0 ? 10 : 80)) {
			new AnimClass(AnimTypes[AnimTypeClass::From_Name("SGRYSMK1")], PositionCoord);
		}
	}

	if (Cargo.Is_Something_Attached() && Class->IsCarryall) {
		Cargo.Attached_Object()->PrimaryFacing = SecondaryFacing;
		Cargo.Attached_Object()->SecondaryFacing = SecondaryFacing;
		Cargo.Attached_Object()->PositionCoord = PositionCoord;
	}
}


/***********************************************************************************************
 * AircraftClass::Mission_Unload -- Handles unloading cargo.                                   *
 *                                                                                             *
 *    This function is used to handle finding, heading toward, landing, and unloading the      *
 *    cargo from the aircraft. Once unloading of cargo has occurred, then the aircraft follows *
 *    a different mission.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns the number of game ticks to delay before calling this function again.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/31/94   JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_UNLOAD(void)
{
	enum {
		SEARCH_FOR_LZ,
		FLY_TO_LZ,
		LAND_ON_LZ,
		UNLOAD_PASSENGERS,
		TAKE_OFF
	};

	switch (Status) {

		/*
		**	Search for an appropriate destination spot if one isn't already assigned.
		*/
		case SEARCH_FOR_LZ:
			if (HeightAGL == 0 && (double)PitchAngle == 0 && (NavCom == NULL || (PositionCoord == NavCom->Center_Coord()))) {
				if (Cargo.Is_Something_Attached() && Map[(Coord const &)PositionCoord].Cell_Building() != NULL) {
					if (House->Is_Human_Player()) {
						Assign_Destination(NULL);
						Assign_Mission(MISSION_GUARD);
						if (Ready_To_Commence()) {
							Commence();
						}
					} else {
						Assign_Destination(Good_LZ());
						Status = TAKE_OFF;
					}
					return(1);
				}
				Status = UNLOAD_PASSENGERS;
			} else {
				if (NavCom == NULL && Class->IsDropship && HeightAGL > 0) {
					BuildingClass * building = NULL;
					for (int index = 0; index < Class->Dock.Count(); index++) {
						building = Find_Docking_Bay(Class->Dock[index], false);
						if (building != NULL) {
							break;
						}
					}

					if (building != NULL) {
						Assign_Destination(building);
						break;
					} else {
						Assign_Destination(Good_LZ());
						break;
					}
				} else if (NavCom == NULL) {
					Status = LAND_ON_LZ;
				} else if (Is_LZ_Clear(NavCom)) {

					if (Class->IsDropship) {

						Cell cell = CELL_NONE;
						if (NavCom->RTTI != RTTI_CELL) {
							cell = Dynamic_Cast<TechnoClass *>(NavCom)->PositionCoord.As_Cell();
						}

						if (cell != CELL_NONE) {
							ObjectClass * occupier = Map[cell].Cell_Occupier();
							while (occupier != NULL) {
								if (occupier->RTTI != RTTI_BUILDING) {
									occupier->Scatter(PositionCoord, true, true);
									occupier = occupier->Next;
								} else {
									Assign_Destination(Good_LZ());
								}
							}
						}

					} else {

						FootClass * foot = Cargo.Attached_Object();
						if (foot != NULL && foot->Team && foot->Team->Class->Get_Origin() != CELL_NONE) {
							Assign_Destination(New_LZ(&Map[foot->Team->Class->Get_Origin()]));
						} else {
							Assign_Destination(New_LZ(&Map[Scen->Get_Waypoint_Cell(WAYPT_REINF)]));
							if (Team != NULL) {
								Team->Assign_Mission_Target(NavCom);
							}
						}
					}

				} else {

					if (HeightAGL != Class->Flight_Level()) {
						Status = TAKE_OFF;
					} else {
						Status = FLY_TO_LZ;
					}
				}

			}
			break;

		/*
		**	Fly to destination.
		*/
		case FLY_TO_LZ:
			if (!Locomotion->Is_Moving()) {
				Status = LAND_ON_LZ;
			}
			if (!Is_LZ_Clear(NavCom)){
				Status = SEARCH_FOR_LZ;
			}
			break;

		/*
		**	Landing phase. Just delay until landing is complete. At that time,
		**	transition to the unloading phase.
		*/
		case LAND_ON_LZ:
			if (!Locomotion->Is_Moving()) {
				Status = UNLOAD_PASSENGERS;
			}
			return(1);

		/*
		**	Hold while unloading passengers. When passengers are unloaded the order for this
		**	transport gets changed to MISSION_RETREAT.
		*/
		case UNLOAD_PASSENGERS:
			if (Cargo.Is_Something_Attached() && Map[(Coord const &)PositionCoord].Cell_Building() != NULL) {
				if (House->Is_Human_Player()) {
					Assign_Destination(NULL);
					Assign_Mission(MISSION_GUARD);
					if (Ready_To_Commence()) {
						Commence();
					}
				} else {
					Assign_Destination(Good_LZ());
					Status = TAKE_OFF;
				}
				return(1);
			}

			if (!IsTethered) {
				if (Cargo.Is_Something_Attached()) {
					if (Class->IsCarryall) {
						Mark(MARK_UP);
						Drop_Off_Cargo();
						Mark(MARK_DOWN);

					} else {
						FootClass * unit = (FootClass *)Cargo.Detach_Object();

						/*
						**	First thing is to lift the transport off of the map so that the unlimbo
						**	process for the passengers is more likely to succeed.
						*/
						Map.Pick_Up(PositionCell, this);

						if (Exit_Object(unit)) {
							unit->IsInTransport = false;
						} else {
							Cargo.Attach(unit);
						}

						/*
						**	Restore the transport back down on the map.
						*/
						Map.Place_Down(PositionCell, this);

						if (!unit->IsInTransport) {
							if (unit->Team != NULL) {
								Team->Add(unit);
							}
						}

						if (!Cargo.Is_Something_Attached()) {
							Enter_Idle_Mode();
						}
					}
				} else {

					Enter_Idle_Mode();
				}
			}
			break;

		/*
		**	Aircraft is now taking off. Once the aircraft reaches flying altitude then it
		**	will either take off or look for another landing spot to try again.
		*/
		case TAKE_OFF:
			Status = SEARCH_FOR_LZ;
			return(1);

		default:
			break;
	}
	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * AircraftClass::Mission_Retreat -- Handles the aircraft logic for leaving the battlefield.   *
 *                                                                                             *
 *    This mission will be followed when the aircraft decides that it is time to leave the     *
 *    battle. Typically, this occurs when a loaner transport has dropped off its load or when  *
 *    an attack air vehicle has expended its ordinance.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game ticks to delay before calling this routine again.  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *   08/13/1995 JLB : Handles aircraft altitude gain after takeoff logic.                      *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_RETREAT(void)
{
	return(0);
#if NEVER
	//assert(IsActive);

	if (Class->IsFixedWing) {
		if (Class->IsFixedWing && Height < FLIGHT_LEVEL) {
			Height += 1;
			return(3);
		}
		return(TICKS_PER_SECOND*10);
	}

	enum {
		TAKE_OFF,
		FACE_MAP_EDGE,
		KEEP_FLYING
	};
	switch (Status) {

		/*
		**	Take off if landed.
		*/
		case TAKE_OFF:
			if (Process_Take_Off()) {
				Status = FACE_MAP_EDGE;
			}
			return(1);

		/*
		**	Set facing and speed toward the friendly map edge.
		*/
		case FACE_MAP_EDGE:
			Set_Speed(MPH_LIGHT_SPEED);

			/*
			**	Take advantage of the fact that the source map edge enumerations happen to
			**	occur in a clockwise order and are the first four enumerations of the map
			**	edge default for the house. If this value is masked and then shifted, a
			**	normalized direction value results. Use this value to head the aircraft
			**	toward the "friendly" map edge.
			*/
			PrimaryFacing.Set_Desired((Dir256)((House->Control.Edge & 0x03) << 6));
			SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
			Status = KEEP_FLYING;
			break;

		/*
		**	Just do nothing since we are headed toward the map edge. When the edge is
		**	reached, the aircraft should be automatically eliminated.
		*/
		case KEEP_FLYING:
			break;

		default:
			break;
	}
	return(MissionControl[Mission].Normal_Delay() + Random_Pick(0, 2));
#endif
}


/***********************************************************************************************
 * AircraftClass::Exit_Object -- Unloads passenger from aircraft.                              *
 *                                                                                             *
 *    This routine is called when the aircraft is to unload a passenger. The passenger must    *
 *    be able to move under its own power. Typical situation is when a transport helicopter    *
 *    is to unload an infantry unit.                                                           *
 *                                                                                             *
 * INPUT:   unit  -- Pointer to the unit that is to be unloaded from this aircraft.            *
 *                                                                                             *
 * OUTPUT:  bool; Was the unit unloaded successfully?                                          *
 *                                                                                             *
 * WARNINGS:   The unload process is merely started by this routine. Radio contact is          *
 *             established with the unloading unit and when the unit is clear of the aircraft  *
 *             the radio contact will be broken and then the aircraft is free to pursue        *
 *             other.                                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Exit_Object(TechnoClass * unit)
{
	static FacingType _toface[FACING_COUNT] = {FACING_S, FACING_SW, FACING_SE, FACING_NW, FACING_NE, FACING_N, FACING_W, FACING_E};
	Cell	cell(0,0);

	/*
	**	Find a free cell to drop the unit off at.
	*/
	FacingType face;
	for (face = FACING_N; face < FACING_COUNT; face++) {
		cell = Adjacent_Cell(PositionCell, _toface[face]);
		if (unit->Can_Enter_Cell(&Map[cell]) == MOVE_OK) break;
	}

	if (face == FACING_COUNT) {
		return(false);
	}

	/*
	**	If the passenger can be placed on the map, then start it moving toward the
	**	destination cell and establish radio contact with the transport. This is used
	**	to make sure that the transport waits until the passenger is clear before
	**	unloading the next passenger or taking off.
	*/
	if (unit->Unlimbo(PositionCoord, Facing_Dir(_toface[face]))) {
		unit->Assign_Mission(MISSION_MOVE);
		unit->Assign_Destination(&Map[cell]);
		if (Transmit_Message(RADIO_HELLO, unit) == RADIO_ROGER) {
			Transmit_Message(RADIO_UNLOAD);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Paradrop_Cargo -- Drop a passenger by parachute.                             *
 *                                                                                             *
 *    Call this routine when a passenger needs to be dropped off by parachute. One passenger   *
 *    is offloaded by a call to this routine.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay time that it is safe to wait before processing any further  *
 *          paradrop actions.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Paradrop_Cargo(void)
{
	FootClass * passenger = Cargo.Detach_Object();
	if (passenger) {
		if (!passenger->Paradrop(Center_Coord())) {
			Cargo.Attach(passenger);
			passenger->Hidden();
		} else {

			/*
			**	Play a sound effect of the parachute opening.
			*/
			Sound_Effect(Rule->ChuteSound, PositionCoord);

			if (Team != NULL) {
				Team->Remove(passenger);
				if (passenger->House->Is_Human_Player()) {
					Assign_Mission(MISSION_GUARD);
				} else {
					Assign_Mission(MISSION_HUNT);
				}
			}
//			Arm = Rearm_Delay(IsSecondShot);
			Arm = 0;
		}
	}
	return(Arm);
}


/***********************************************************************************************
 * AircraftClass::Fire_At -- Handles firing a projectile from an aircraft.                     *
 *                                                                                             *
 *    Sometimes, aircraft firing needs special handling. Example: for napalm bombs, the        *
 *    bomb travels forward at nearly the speed of the delivery aircraft, not necessarily the   *
 *    default speed defined in the BulletTypeClass structure.                                  *
 *                                                                                             *
 * INPUT:   target   -- The target that the projectile is heading for.                         *
 *                                                                                             *
 *          which    -- Which weapon to use in the attack. 0=primary, 1=secondary.             *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the bullet that was created as a result of this attack.  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
static inline bool Aircraft_Fire_Shrouded(Coord const & coord)
{
	return(Map.Is_Shrouded(coord + Coord(2 * CELL_LEPTON_W, -2 * CELL_LEPTON_H)));
}


/// <summary>
/// Fires this aircraft's weapon at the target specified.
/// An aircraft with passengers aboard paradrops them instead of firing. The projectile
/// is launched with a velocity that suits the aircraft's own motion, and a human owner
/// is shown the ground around the attack if any of it lies under shroud.
/// </summary>
/// <param name="target">The object or cell to fire upon.</param>
/// <param name="which">Which of the weapons to fire.</param>
/// <returns>Returns with a pointer to the projectile created, or NULL if the aircraft
/// did not fire.</returns>
BulletClass * AircraftClass::Fire_At(AbstractClass * target, int which)
{
	/*
	**	Passenger aircraft will actually paradrop their cargo instead of
	**	firing their weapon.
	*/
	if (Cargo.Is_Something_Attached()) {
		Paradrop_Cargo();
		return(0);
	}

#if OBSOLETE
	/*
	**	If the weapon is actually a camera, then perform the "snapshot" of the
	**	ground instead of normal weapon fire.
	*/
	if (PrimaryWeapon != NULL && PrimaryWeapon->IsCamera) {
		if (House->Is_Ally(PlayerPtr)) {
			Map.Sight_From(Center_Coord().As_Cell(), 9, House, false);
		}
		Ammo = 0;
		Arm = Rearm_Delay(IsSecondShot);
		return(0);
	}
#endif

	BulletClass * bullet = BASECLASS::Fire_At(target, which);

	if (bullet) {

		if (bullet->Class->ROT == 0) {
			bullet->Velocity.Set_Speed(Locomotion->Apparent_Speed());
			bullet->Velocity.Set_Pitch(DIR_E);
			bullet->Velocity.Set_Yaw(SecondaryFacing.Current());
		}

		if (bullet->Class->ROT == 1) {
			Coord diff = target->Center_Coord() - Center_Coord();
			TVelocity3D<double> dir(diff.X, diff.Y, diff.Z);
			bullet->Velocity.Set_Yaw(dir.Get_Yaw());
			bullet->Velocity.Set_Pitch(dir.Get_Pitch());
			bullet->Velocity.Set_Speed(PrimaryWeapon->MaxSpeed);
		}

		if (House->Is_Player_Control()) {
			if (Map.Is_Shrouded(PositionCoord) ||
				Map.Is_Shrouded(PositionCoord + Coord(2 * CELL_LEPTON_W, 2 * CELL_LEPTON_H)) ||
				Map.Is_Shrouded(PositionCoord + Coord(-2 * CELL_LEPTON_W, -2 * CELL_LEPTON_H)) ||
				Map.Is_Shrouded(PositionCoord + Coord(2 * CELL_LEPTON_W, -2 * CELL_LEPTON_H)) ||
				Aircraft_Fire_Shrouded(PositionCoord) ||
				Map.Is_Shrouded(target->Center_Coord())) {
				Map.Sight_From(PositionCoord, Rule->AttackingAircraftSightRange, House);
			}
		}
	}
	return(bullet);
}


/***********************************************************************************************
 * AircraftClass::Take_Damage -- Applies damage to the aircraft.                               *
 *                                                                                             *
 *    This routine is used to apply damage to the specified aircraft. This is where any        *
 *    special crash animation will be initiated.                                               *
 *                                                                                             *
 * INPUT:   damage   -- Reference to the damage that will be applied to the aircraft.          *
 *                      This value will be filled in with the actual damage that was           *
 *                      applied.                                                               *
 *                                                                                             *
 *          distance -- Distance from the source of the explosion to this aircraft.            *
 *                                                                                             *
 *          warhead  -- The warhead type that the damage occurs from.                          *
 *                                                                                             *
 *          source   -- Pointer to the originator of the damage. This can be used so that      *
 *                      proper "thank you" can be delivered.                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the result of the damage as it affects this aircraft.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ResultType AircraftClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	ResultType res = RESULT_NONE;

	/*
	**	Apply the damage to the aircraft.
	*/
	res = BASECLASS::Take_Damage(damage, distance, warhead, source, forced, no_crew);

	/*
	**	Special action is performed if the aircraft is killed -- the cargo is destroyed
	**	as well.
	*/
	switch (res) {
		case RESULT_ALREADY_DESTROYED:
			res = RESULT_ALREADY_DESTROYED;
			break;

		case RESULT_DESTROYED:
			Death_Announcement();
			if (warhead == Rule->FirestormWarhead) {
				int count = abs(Scen->RandomNumber % 3) + 7;
				for (int i = 0; i < count; i++) {
					ParticleSystemClass * psys = new ParticleSystemClass(Rule->DefaultFirestormExplosionSystem, Center_Coord(), NULL, this);
					psys->Sparks_To_Use_Random_Direction();
				}
			} else if (Class->Explosion_Set().Count() > 0) {
				new AnimClass(Class->Explosion_Set().Pick(Scen->RandomNumber), Target_Coord());
			}

#if OBSOLETE
			/*
			**	Parachute a survivor if possible.
			*/
			if (Class->IsCrew && Percent_Chance(90) && Map[Center_Coord()].Is_Clear_To_Move(SPEED_FOOT, true, false)) {
				InfantryClass * infantry = new InfantryClass(INFANTRY_E1, House->Class->House);
				if (infantry != NULL) {
					if (!infantry->Paradrop(Center_Coord())) {
						delete infantry;
					}
				}
			}
#endif

			if (!Crash(source)){
				Delete_Me();
			}

			break;

		default:
		case RESULT_HALF:
			break;
	}

	return(res);
}


/***********************************************************************************************
 * AircraftClass::Mission_Move -- Handles movement mission.                                    *
 *                                                                                             *
 *    This state machine routine is used when an aircraft (usually helicopter) is to move      *
 *    from one location to another. It will handle any necessary take off and landing this     *
 *    may require.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames that should elapse before this routine      *
 *          is called again.                                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_MOVE(void)
{
	if (Class->IsCarryall) {
		return(Do_MISSION_MOVE_Carryall());
	}

	return(Do_MISSION_MOVE_Normal());
}


/// <summary>
/// Handles the move mission for an aircraft.
/// This routine picks a suitable landing zone, gets the aircraft airborne, flies it to
/// the destination -- by way of the assigned waypoint path if there is one -- and then
/// either lands it or drops it back into idle mode.
/// </summary>
/// <returns>Returns with the delay in game frames before this mission should be
/// processed again.</returns>
int AircraftClass::Do_MISSION_MOVE_Normal(void)
{
	enum {
		VALIDATE_LZ,
		TAKE_OFF,
		FLY_TO_LZ,
		IDLE,
		LAND
	};
	switch (Status) {

		/*
		**	Double check and change LZ if necessary.
		*/
		case VALIDATE_LZ:
			if (NavCom == NULL) {
				Enter_Idle_Mode();
			} else {
				Assign_Destination(New_LZ(NavCom));
				if (Team != NULL) {
					Team->Assign_Mission_Target(NavCom);
				}
				Status = TAKE_OFF;
			}
			break;

		/*
		**	Take off if necessary.
		*/
		case TAKE_OFF:
			if (NavCom == NULL) {
				Enter_Idle_Mode();
				return(1);
			} else {
				Locomotion->Move_To(NavCom->Destination_Coord());
				Status = FLY_TO_LZ;
				return(1);
			}
			break;

		/*
		**	Fly toward target.
		*/
		case FLY_TO_LZ:
			if (CurrentPath != PATH_NONE) {
				bool proceed_to_wp = false;
				if (NavCom != NULL) {
					Coord tar_coord = NavCom->Center_Coord();
					Coord here_coord = PositionCoord;
					proceed_to_wp = Coord(here_coord.X, here_coord.Y, 0).Distance_To(Coord(tar_coord.X, tar_coord.Y, 0)) < CELL_LEPTON;
				} else {
					proceed_to_wp = true;
				}
				if (proceed_to_wp) {
					WaypointClass * wp = PlayerPtr->Paths[CurrentPath]->Get_Waypoint(NextWaypoint);
					WaypointClass * nwp = PlayerPtr->Paths[CurrentPath]->Get_Next_Waypoint(wp);
					Execute_Waypoint_Path(nwp);
				}
			}

			if (Locomotion->Is_Moving()) {
				if (NavCom != NULL && !Cell_Seems_Ok(NavCom->Destination_Coord().As_Cell(), false)) {
					Status = VALIDATE_LZ;
				} else {
					Status = LAND;
				}
			} else {
				Status = IDLE;
			}
			return(1);

		case IDLE:
			Enter_Idle_Mode();
			return(1);

		/*
		**	Land on target.
		*/
		case LAND:
			if (Locomotion->Is_Moving()) {
				if (NavCom != NULL && !Cell_Seems_Ok(NavCom->Destination_Coord().As_Cell(), true)) {
					Status = VALIDATE_LZ;
				}
			} else {
				Status = IDLE;
			}
			return(1);

		default:
			break;
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/// <summary>
/// Sets the carried unit down on the ground below.
/// This routine is used by the carryall move mission once the aircraft has arrived over
/// its destination. Should the unit prove impossible to place, it stays aboard and the
/// carryall goes looking for somewhere else to put it.
/// </summary>
void AircraftClass::Drop_Off_Cargo(void)
{
	DebugString("Do_MISSION_MOVE_Carryall - LAND - Dropping off cargo\n");

	FootClass * unit = Cargo.Detach_Object();

	Coord coord = PositionCoord;
	coord.Z = Map.Get_Height_GL(coord);
	if (Map[coord].IsUnderBridge) {
		coord.Z += BRIDGE_LEPTON_HEIGHT;
		unit->IsOnBridge = true;
	} else {
		unit->IsOnBridge = false;
	}

	unit->Locomotion.Release();
	unit->Locomotion = ILocomotionPtr(unit->TClass->Locomotor);
	unit->Locomotion->Link_To_Object(unit);

	if (!unit->Unlimbo(coord)) {
		Cargo.Attach(unit);
	} else {
		unit->IsInTransport = false;
		unit->Look();
		Cell nearby = Nearby_Location();
		if (Contact_With_Whom() == unit) {
			Transmit_Message(RADIO_UNTETHER);
			Transmit_Message(RADIO_OVER_OUT);
		}
		if (nearby != CELL_NONE) {
			Assign_Destination(&Map[nearby]);
		} else {
			Assign_Destination(NULL);
		}
	}
}


/// <summary>
/// Handles the move mission for a carryall aircraft.
/// This routine arranges the ride by radio, flies the carryall over the unit or landing
/// zone it was sent to, and then either lifts the waiting unit aboard or sets down
/// whatever it is already carrying.
/// </summary>
/// <returns>Returns with the delay in game frames before this mission should be
/// processed again.</returns>
int AircraftClass::Do_MISSION_MOVE_Carryall(void)
{
	enum {
		VALIDATE_LZ,
		TAKE_OFF,
		FLY_TO_LZ,
		LAND
	};

	TechnoClass * target;

	switch (Status) {

		/*
		**	Double check and change LZ if necessary.
		*/
		case VALIDATE_LZ:
			DebugString("Do_MISSION_MOVE_Carryall - VALIDATE_LZ\n");
			if (NavCom == NULL) {
				DebugString("Do_MISSION_MOVE_Carryall - VALIDATE_LZ - NavCom == NULL\n");
				Enter_Idle_Mode();
				break;
			}

			target = Dynamic_Cast<TechnoClass *>(NavCom);
			if (target != NULL &&
				!Cargo.Is_Something_Attached() &&
				House->Is_Ally(target) &&
				(!target->Is_Techno() || target->Owner_HouseClass()->Is_Ally(this))
				&& target->RTTI == RTTI_UNIT
				&& ((UnitClass *)target)->Class->IsTotable) {
				DebugString("Do_MISSION_MOVE_Carryall - VALIDATE_LZ - target != NULL\n");
				if (Contact_With_Whom() != target) {
					Transmit_Message(RADIO_OVER_OUT);
				}
				if (Transmit_Message(RADIO_HELLO, target) == RADIO_ROGER) {
					DebugString("Do_MISSION_MOVE_Carryall - VALIDATE_LZ - RADIO_HELLO got RADIO_ROGER\n");
					if (Transmit_Message(RADIO_WANT_RIDE) != RADIO_ROGER) {
						DebugString("Do_MISSION_MOVE_Carryall - VALIDATE_LZ - RADIO_WANT_RIDE did not get RADIO_ROGER\n");
						Transmit_Message(RADIO_OVER_OUT);
						Enter_Idle_Mode();
						break;
					} else {
						Transmit_Message(RADIO_HOLD_STILL);
					}
				} else {
					Assign_Destination(NULL);
					Enter_Idle_Mode();
					break;
				}
			}
			Assign_Destination(New_LZ(NavCom));
			if (Team != NULL) {
				Team->Assign_Mission_Target(NavCom);
			}
			Status = TAKE_OFF;
			DebugString("Do_MISSION_MOVE_Carryall - VALIDATE_LZ - Status = TAKE_OFF\n");
			break;

		/*
		**	Take off if necessary.
		*/
		case TAKE_OFF:
			if (NavCom == NULL) {
				Enter_Idle_Mode();
				return(1);
			}
			Locomotion->Move_To(NavCom->Center_Coord());
			Status = FLY_TO_LZ;
			return(1);

		/*
		**	Fly toward target.
		*/
		case FLY_TO_LZ:
			if (NavCom != NULL && !Is_Target_Cell(NavCom) && Contact_With_Whom() != NavCom) {
				DebugString("Do_MISSION_MOVE_Carryall - FLY_TO_LZ - Lost contact\n");
				Status = VALIDATE_LZ;
				return(1);
			}

			if (Is_Target_Cell(NavCom) && !Is_LZ_Clear(NavCom)) {
				Status = VALIDATE_LZ;
				return(1);
			}

			if (Locomotion->Get_Status() == 1) {
				DebugString("Do_MISSION_MOVE_Carryall - FLY_TO_LZ - Begin landing\n");
				if (NavCom != NULL && NavCom->RTTI == RTTI_UNIT) {
					UnitClass *unit = ((UnitClass *)NavCom);
					if (unit->PositionCoord.As_Cell() != PositionCoord.As_Cell()) {
						Status = VALIDATE_LZ;
						DebugString("Do_MISSION_MOVE_Carryall - FLY_TO_LZ - Target moved\n");
						return(1);
					} else {
						IsReadyToCommence = false;
					}
				} else {
					IsReadyToCommence = false;
				}

			}
			if (!Locomotion->Is_Moving()) {
				DebugString("Do_MISSION_MOVE_Carryall - FLY_TO_LZ - Stopped moving\n");
				Status = LAND;
			}
			return(1);

		/*
		**	Land on target.
		*/
		case LAND:
			DebugString("Do_MISSION_MOVE_Carryall - LAND\n");
			if (Cargo.Is_Something_Attached() && NearbyObject != NULL) {
				IsReadyToCommence = true;
			} else if (Cargo.Is_Something_Attached()) {
				Mark(MARK_UP);
				Drop_Off_Cargo();
				Mark(MARK_DOWN);
				Status = VALIDATE_LZ;
			} else {
				DebugString("Do_MISSION_MOVE_Carryall - LAND - Picking up cargo\n");
				Mark(MARK_UP);
				UnitClass * unit = Map[(Coord const &)PositionCoord].Cell_Unit(Map[(Coord const &)PositionCoord].IsUnderBridge);
				if (unit != NULL && unit == Contact_With_Whom()) {
					DebugString("Do_MISSION_MOVE_Carryall - LAND - Got Cell_Unit\n");
					if (Transmit_Message(RADIO_NEED_TO_MOVE) == RADIO_ROGER) {
						DebugString("Do_MISSION_MOVE_Carryall - LAND - RADIO_NEED_TO_MOVE got RADIO_ROGER\n");
						unit->Limbo();
						unit->IsOnBridge = false;
						unit->IsInTransport = true;
						Cargo.Attach(unit);
					} else {
						Status = VALIDATE_LZ;
					}
				} else {
					Status = VALIDATE_LZ;
				}
				Transmit_Message(RADIO_OVER_OUT);
				Mark(MARK_DOWN);
				IsReadyToCommence = true;
				Enter_Idle_Mode();
			}
			return(1);
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/// <summary>
/// Handles the patrol mission for an aircraft.
/// This routine walks the aircraft along its assigned waypoint path, breaking off to
/// engage any threat it spots on the way and heading for a docking bay to rearm when it
/// runs dry.
/// </summary>
/// <returns>Returns with the delay in game frames before this mission should be
/// processed again.</returns>
int AircraftClass::Do_MISSION_PATROL(void)
{
	enum {
		VALIDATE_LZ,
		TAKE_OFF,
		FLY_TO_LZ,
		IDLE,
		LAND,
	};

	int i;

	IsOnPatrol = true;

	switch (Status) {

		/*
		**	Double check and change LZ if necessary.
		*/
		case VALIDATE_LZ:
			if (NavCom == NULL) {
				Enter_Idle_Mode();
			} else {
				Assign_Destination(New_LZ(NavCom));
				if (Team != NULL) {
					Team->Assign_Mission_Target(NavCom);
				}
				Status = TAKE_OFF;
			}
			break;

		/*
		**	Take off if necessary.
		*/
		case TAKE_OFF:
			if (NavCom == NULL) {
				Enter_Idle_Mode();
				return(1);
			} else {
				Locomotion->Move_To(NavCom->Destination_Coord());
				Status = FLY_TO_LZ;
				return(1);
			}
			break;

		/*
		**	Fly toward target.
		*/
		case FLY_TO_LZ:
			if (Ammo == 0) {
				BuildingClass * building = NULL;
				for (i = 0; i < Class->Dock.Count(); i++) {
					building = Find_Docking_Bay(Class->Dock[i], false);
					if (building != NULL) {
						break;
					}
				}

				if (building != NULL) {
					Override_Mission(MISSION_ENTER, NULL, building);
					Status = VALIDATE_LZ;
					return(1);
				}
			}

			if (Ammo != 0) {
				ObjectClass *threat = Greatest_Threat(THREAT_AREA, PositionCoord, false)->As_ObjectClass();
				if (threat != NULL) {
					Override_Mission(MISSION_ATTACK, threat, NULL);
					Status = VALIDATE_LZ;
					return(1);
				}
			}

			if (CurrentPath != PATH_NONE) {
				bool proceed_to_wp = false;
				if (NavCom != NULL) {
					Coord tar_coord = NavCom->Center_Coord();
					Coord here_coord = PositionCoord;
					proceed_to_wp = Coord(here_coord.X, here_coord.Y, 0).Distance_To(Coord(tar_coord.X, tar_coord.Y, 0)) < CELL_LEPTON;
				} else {
					proceed_to_wp = true;
				}
				if (proceed_to_wp) {
					WaypointClass * wp = PlayerPtr->Paths[CurrentPath]->Get_Waypoint(NextWaypoint);
					WaypointClass * nwp = PlayerPtr->Paths[CurrentPath]->Get_Next_Waypoint(wp);
					Execute_Waypoint_Path(nwp);
				}
			}

			if (!Locomotion->Is_Moving()) {
				Status = IDLE;
			} else {
				if (NavCom != NULL && !Cell_Seems_Ok(NavCom->Destination_Coord(), true)) {
					Status = VALIDATE_LZ;
				} else {
					Status = LAND;
				}
			}
			return(1);

		case IDLE:
			Enter_Idle_Mode();
			return(1);

		/*
		**	Land on target.
		*/
		case LAND:
			if (!Locomotion->Is_Moving()) {
				Status = IDLE;
			} else {
				if (NavCom != NULL) {
					if (!Cell_Seems_Ok(NavCom->Destination_Coord(), true)) {
						Status = VALIDATE_LZ;
					}
				}
			}
			return(1);

		default:
			break;
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * AircraftClass::Enter_Idle_Mode -- Gives the aircraft an appropriate mission.                *
 *                                                                                             *
 *    Use this routine when the mission for the aircraft is in doubt. This routine will find   *
 *    an appropriate mission for the aircraft and dispatch it.                                 *
 *                                                                                             *
 * INPUT:   initial  -- Is this called when the unit just leaves a factory or is initially     *
 *                      or is initially placed on the map?                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/05/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Enter_Idle_Mode(bool initial, bool resume_waypoint)
{
	if (Has_Suspended_Mission()) {
		Restore_Mission();
		if (CurrentMission == MISSION_PATROL) {
			Status = 0;
			IsLockedStraight = false;
		}
		return(false);
	}

	bool result = BASECLASS::Enter_Idle_Mode(initial, resume_waypoint);

	MissionType mission = (House->Is_Human_Player() || Team || !Is_Weapon_Equipped()) ? MISSION_GUARD : MISSION_GUARD_AREA;

	int landingalt = Landing_Altitude();
	if (In_Which_Layer() == LAYER_GROUND || HeightAGL <= landingalt) {
		if (IsALoaner) {
			if (Cargo.Is_Something_Attached()) {

				/*
				**	In the case of a computer controlled helicopter that hold passengers,
				**	don't unload when landing. Wait for specific instructions from the
				**	controlling team.
				*/
				if (Team != NULL) {
					mission = MISSION_GUARD;
				} else {
					mission = MISSION_UNLOAD;
				}
			} else if (Team == NULL) {
				mission = MISSION_RETREAT;
			}
		} else {
			Assign_Destination(NULL);
			Assign_Target(NULL);
			if (!House->Is_Human_Player() && Team == NULL && Is_Weapon_Equipped()) {
				mission = MISSION_GUARD_AREA;
			} else {
				mission = MISSION_GUARD;
			}
		}
	} else {
		bool assign_move = false;
		MissionType move_mission;
		if (Cargo.Is_Something_Attached()) {
			if (IsALoaner) {
				if (Team != NULL) {
					mission = MISSION_GUARD;
				} else {
					mission = MISSION_UNLOAD;
					Assign_Destination(Good_LZ());
				}
			} else {
				move_mission = MISSION_MOVE;
				assign_move = true;
			}
		} else {

			/*
			**	If this transport is a loaner and part of a team, then remove it from
			**	the team it is attached to.
			*/
			if ((IsALoaner && House->Is_Human_Player()) || (!House->Is_Human_Player() && !Class->MaxAmmo)) {
				if (Team != NULL && Team->Has_Entered_Map()) {
					Team->Remove(this);
				}
			}

			if (PrimaryWeapon != NULL) {

				/*
				**	Weapon equipped helicopters that run out of ammo and were
				**	brought in as reinforcements will leave the map.
				*/
				if (IsALoaner) {

					/*
					**	If it has no ammo, then break off of the team and leave the map.
					**	If it can fight, then give it fighting orders.
					*/
					if (Ammo == 0) {
						if (Team != NULL) Team->Remove(this);
						mission = MISSION_RETREAT;
					} else {
						if (Team == NULL) {
							mission = MISSION_HUNT;
						}
					}

				} else if (Ammo && TarCom != NULL && Mission == MISSION_ATTACK || MissionQueue == MISSION_ATTACK) {
					mission = MISSION_ATTACK;
				} else if (In_Air()) {
					if (NavCom == NULL || (CurrentMission != MISSION_MOVE && CurrentMission != MISSION_ENTER)) {
						if (Class->Dock.Count() > 0 && (IsLocked || Team == NULL)) {

							/*
							**	Normal aircraft try to find a good landing spot to rest.
							*/
							BuildingClass * building = NULL;
							for (int i = 0; i < Class->Dock.Count(); i++) {
								building = Find_Docking_Bay(Class->Dock[i], false, false);
								if (building) break;
							}
							Assign_Destination(NULL);
							if (building && Transmit_Message(RADIO_HELLO, building) == RADIO_ROGER) {
								Assign_Destination(building);
								mission = MISSION_ENTER;
							} else {
								move_mission = MISSION_MOVE;
								assign_move = true;
							}
						}
					}
				}
			} else {
				if (Team != NULL) return(false);

				move_mission = MISSION_MOVE;
				assign_move = true;
			}
		}

		if (assign_move) {
			Assign_Destination(Good_LZ());
			mission = move_mission;
		}
	}

	Assign_Mission(mission);
	if (Ready_To_Commence()) {
		Commence();
	}

	return(result);
}


#ifdef _DEBUG
/***********************************************************************************************
 * AircraftClass::Debug_Dump -- Displays the status of the aircraft to the mono monitor.       *
 *                                                                                             *
 *    This displays the current status of the aircraft class to the mono monitor. By this      *
 *    display bugs may be tracked down or prevented.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(0, 0);
	mono->Set_Cursor(1, 11);mono->Printf("%3d", AttacksRemaining);

	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * AircraftClass::Active_Click_With -- Handles clicking over specified object.                 *
 *                                                                                             *
 *    This routine is used when the player clicks over the speicifed object. It will assign    *
 *    the appropriate mission to the aircraft.                                                 *
 *                                                                                             *
 * INPUT:   action   -- The action that was nominally determined by the What_Action function.  *
 *                                                                                             *
 *          object   -- The object over which the mouse was clicked.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will alter the game sequence and causes an event packet to be      *
 *             propagated to all connected machines.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Active_Click_With(ActionType action, ObjectClass * object, bool is_waypoint)
{
	action = What_Action(object, is_waypoint);

	switch (action) {

		case ACTION_TOTE:
			Player_Assign_Mission(MISSION_MOVE, NULL, object);
			return(true);

		case ACTION_NOMOVE:
			return(false);

		case ACTION_ENTER:
			Player_Assign_Mission(MISSION_ENTER, NULL, object);
			return(true);

		case ACTION_SELF:
			Player_Assign_Mission(MISSION_UNLOAD, NULL, NULL);
			return(true);

		default:
			break;
	}

	return(BASECLASS::Active_Click_With(action, object, is_waypoint));
}


/***********************************************************************************************
 * AircraftClass::Active_Click_With -- Handles clicking over specified cell.                   *
 *                                                                                             *
 *    This routine is used when the player clicks the mouse of the specified cell. It will     *
 *    assign the appropriate mission to the aircraft.                                          *
 *                                                                                             *
 * INPUT:   action   -- The action nominally determined by What_Action().                      *
 *                                                                                             *
 *          cell     -- The cell over which the mouse was clicked.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine will affect the game sequence and causes an event object to be     *
 *             propagated to all connected machines.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Active_Click_With(ActionType action, Cell const & cell, bool is_waypoint)
{
	return(BASECLASS::Active_Click_With(action, cell, is_waypoint));
}


/***********************************************************************************************
 * AircraftClass::Player_Assign_Mission -- Handles player input to assign a mission.           *
 *                                                                                             *
 *    This routine is called as a result of player input with the intent to change the         *
 *    mission of the aircraft.                                                                 *
 *                                                                                             *
 * INPUT:   mission  -- The mission requested of the aircraft.                                 *
 *                                                                                             *
 *          target   -- The value to assign to the aircraft's targeting computer.              *
 *                                                                                             *
 *          dest.    -- The value to assign to the aircraft's navigation computer.             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The mission specified will be executed at an indeterminate future game frame.   *
 *             This is controlled by net/modem propagation delay.                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Player_Assign_Mission(MissionType mission, AbstractClass * target, AbstractClass * destination)
{
	if (AllowVoice) {
		if (mission == MISSION_ATTACK) {
			Response_Attack();
		} else {
			Response_Move();
		}
	}
	TargetClass dest(destination);
	TargetClass targ(target);
	Queue_Mission(TargetClass(this), mission, targ, dest);
}


/***********************************************************************************************
 * AircraftClass::What_Action -- Determines what action to perform.                            *
 *                                                                                             *
 *    This routine is used to determine what action will likely be performed if the mouse      *
 *    were clicked over the object specified. The display system calls this routine to         *
 *    control the mouse shape.                                                                 *
 *                                                                                             *
 * INPUT:   target   -- Pointer to the object that the mouse is currently over.                *
 *                                                                                             *
 * OUTPUT:  Returns with the action that will occur if the mouse were clicked over the         *
 *          object specified.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType AircraftClass::What_Action(ObjectClass const * target, bool disallow_force) const
{
	ActionType action = BASECLASS::What_Action(target, disallow_force);

	if (Class->IsCarryall && House->Is_Player_Control()) {
		if (action == ACTION_SELECT || action == ACTION_NONE) {
			if (House->Is_Ally(target)) {
				if (!target->Is_Techno() || target->Owner_HouseClass()->Is_Ally(this)) {
					if (!Cargo.Is_Something_Attached() && target->RTTI == RTTI_UNIT && ((UnitClass const *)target)->Class->IsTotable) {
						action = ACTION_TOTE;
					}
				}
			}
		}
	}

	if (action == ACTION_NONE) {
		action = What_Action(target->Center_Coord().As_Cell(), false, disallow_force);
	}

	if (action == ACTION_SELF) {
		if (!Cargo.How_Many()) {
		action = ACTION_NONE;
		} else if (Map[(Coord const &)PositionCoord].Cell_Building() != NULL) {
			action = ACTION_NO_DEPLOY;
		}
	}

	if (action == ACTION_ATTACK && PrimaryWeapon == NULL) {
		action = ACTION_NONE;
	}

	/*
	**	Special return to friendly repair factory action.
	*/
	if (House->Is_Player_Control() && (action == ACTION_SELECT || action == ACTION_MOVE) && target->RTTI == RTTI_BUILDING) {
		BuildingClass * building = (BuildingClass *)target;
		if ((building->Class->IsCanUnitRepair || building->Class->IsHelipad) && !building->In_Radio_Contact() && !building->Cargo.Is_Something_Attached()) {
			if (((AircraftClass *)this)->Transmit_Message(RADIO_CAN_LOAD, building) == RADIO_ROGER) {
				action = ACTION_ENTER;
			}
		}
	}

	if (Class->IsCarryall && action == ACTION_TOTE) {
		Cell cell = target->PositionCell;
		if (cell != CELL_NONE) {
			BuildingClass * building = (BuildingClass *)Map[cell].Cell_Building();
			if (building != NULL && building->Class->IsWeaponsFactory) {
				action = ACTION_NONE;
			}
		}

	}

	return(action);
}


/***********************************************************************************************
 * AircraftClass::What_Action -- Determines what action to perform.                            *
 *                                                                                             *
 *    This routine will determine what action would occur if the mouse were clicked over the   *
 *    cell specified. The display system calls this routine to determine what mouse shape      *
 *    to use.                                                                                  *
 *                                                                                             *
 * INPUT:   cell  -- The cell over which the mouse is currently positioned.                    *
 *                                                                                             *
 * OUTPUT:  Returns with the action that will be performed if the mouse were clicked at the    *
 *          specified cell location.                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType AircraftClass::What_Action(Cell const & cell, bool check_fog, bool disallow_force) const
{
	if (!House->Is_Player_Control()) {
		return(ACTION_NONE);
	}

	ActionType action = BASECLASS::What_Action(cell, check_fog, disallow_force);

	if (action == ACTION_ATTACK && PrimaryWeapon == NULL) {
		action = ACTION_NONE;
	}

	return(action);
}


/***********************************************************************************************
 * AircraftClass::Pose_Dir -- Fetches the natural landing facing.                              *
 *                                                                                             *
 *    Use this routine to get the desired facing the aircraft should assume when landing.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the normal default facing the aircraft should have when landed.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *   03/04/1996 JLB : Fixed wing aircraft always face down the runway.                         *
 *=============================================================================================*/
Dir256 AircraftClass::Pose_Dir(void) const
{
	return(Rule->PoseDir);
}


/***********************************************************************************************
 * AircraftClass::Mission_Attack -- Handles the attack mission for aircraft.                   *
 *                                                                                             *
 *    This routine is the state machine that handles the attack mission for aircraft. It will  *
 *    handling homing in on and firing on the target in the aircraft's targeting computer.     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game ticks to pass before this routine must be called   *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *   09/22/1995 JLB : Fixes brain dead helicopter for Nod scen #7.                             *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_ATTACK(void)
{
	enum {
		VALIDATE_AZ,
		PICK_ATTACK_LOCATION,
		TAKE_OFF,
		FLY_TO_POSITION,
		FIRE_AT_TARGET,
		FIRE_AT_TARGET2,

		/*
		 * A strafing run is five shots -- the first is fired by FIRE_AT_TARGET, then one
		 * by each of these states, spaced by the weapon's rate of fire.
		 */
		STRAFE_SHOT2,
		STRAFE_SHOT3,
		STRAFE_SHOT4,
		STRAFE_LAST_SHOT,

		RETURN_TO_BASE
	};
	switch (Status) {

		/*
		**	Double check target and validate the attack zone.
		*/
		case VALIDATE_AZ:
			IsLockedStraight = false;

			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
			} else {
				Status = PICK_ATTACK_LOCATION;
			}
			return(1);

		/*
		**	Pick a good location to attack from.
		*/
		case PICK_ATTACK_LOCATION:
			IsLockedStraight = false;

			if (IsToSpendAmmo) {
				IsToSpendAmmo = false;
				Ammo--;
			}

			if (TarCom == NULL || Ammo == 0) {
				Status = RETURN_TO_BASE;
			} else {
				Assign_Destination(Good_Fire_Location(TarCom));
				if (NavCom != NULL) {
					Status = FLY_TO_POSITION;
				} else {
					Status = RETURN_TO_BASE;
				}
			}
			break;

#if OBSOLETE
		/*
		**	Take off (if necessary).
		*/
		case TAKE_OFF:
			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
			} else {
				if (Process_Take_Off()) {
					Status = FLY_TO_POSITION;

					/*
					**	Break off radio contact with the helipad it is taking off from.
					*/
					if (In_Radio_Contact() && Map[Coord].Cell_Building() == Contact_With_Whom()) {
						Transmit_Message(RADIO_OVER_OUT);
					}

					/*
					**	Start flying toward the destination by skewing at first.
					**	As the flight progresses, the body will rotate to face
					**	the direction of travel.
					*/
					int diff = SecondaryFacing.Difference(Direction(NavCom));
					diff = Bound(diff, -DIR_STEP_2, DIR_STEP_2);
					PrimaryFacing = Dir256((int)SecondaryFacing.Current()+diff);
				}
				return(1);
			}
			break;
#endif

		/*
		**	Fly to attack location.
		*/
		case FLY_TO_POSITION:

			if (IsToSpendAmmo) {
				IsToSpendAmmo = false;
				Ammo--;
			}
			IsLockedStraight = false;

			if (TarCom != NULL && Ammo) {

				if (Is_Strafe()) {
					if (Planar_Distance(TarCom) < PrimaryWeapon->Range) {
						Status = FIRE_AT_TARGET;
						return(1);
					}
					Assign_Destination(TarCom);
				} else {
					if (!Locomotion->Is_Moving_Now()) {
						Status = FIRE_AT_TARGET;
						return(1);
					}
				}

				/*
				**	If the navcom was cleared mysteriously, then try to pick
				**	a new attack location. This is a likely event if the player
				**	clicks on a new target while in flight to an existing target.
				*/
				if (NavCom == NULL) {
					Status = PICK_ATTACK_LOCATION;
					return(1);
				}

				int distance = Planar_Distance(NavCom);// = Process_Fly_To(true, NavCom);

				if (distance < 2 * CELL_LEPTON) {
					SecondaryFacing.Set_Desired(Direction(TarCom));

					if (distance < 0x0010) {
						Status = FIRE_AT_TARGET;
						Assign_Destination(NULL);
					}
				} else {
					SecondaryFacing.Set_Desired(DirType().Direction(Fire_Coord(0), NavCom->Center_Coord()));
					return(1);
				}
			} else {
				Status = RETURN_TO_BASE;
			}
			return(1);

		/*
		**	Fire at the target.
		*/
		case FIRE_AT_TARGET:
			if (TarCom == NULL || Ammo == 0) {
				Status = RETURN_TO_BASE;
				return(1);
			}

			if (!Is_Strafe()) {
				PrimaryFacing.Set_Desired(Direction(TarCom));
				SecondaryFacing.Set_Desired(Direction(TarCom));
			}
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_CLOAKED:
					Do_Uncloak();
					break;

				case FIRE_REARM:
					break;

				case FIRE_FACING:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
					} else {
						if (In_Range(TarCom) && !Is_Strafe()) {
							Status = Rule->IsCurleyShuffle ? PICK_ATTACK_LOCATION : FIRE_AT_TARGET;
						} else {
							Status = PICK_ATTACK_LOCATION;
						}
						if (Is_Strafe()) {
							return(45);
						}
					}
					break;

				case FIRE_OK:
					IsToSpendAmmo = true;
					if (In_Range(TarCom)) {
						Fire_At(TarCom, 0);
					}
					Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);
					if (Is_Strafe()) {
						Status = STRAFE_SHOT2;
						IsLockedStraight = true;
					} else {
						Status = FIRE_AT_TARGET2;
						break;
					}
					return(PrimaryWeapon->ROF);

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
					} else {
						if (!Is_Strafe()) {
							Status = FIRE_AT_TARGET2;
						}
					}
					break;
			}
			return(1);

		/*
		**	Fire at the target.
		*/
		case FIRE_AT_TARGET2:
			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
				return(1);
			}

			PrimaryFacing.Set_Desired(Direction(TarCom));
			SecondaryFacing.Set_Desired(Direction(TarCom));
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_CLOAKED:
					Do_Uncloak();
					break;

				case FIRE_REARM:
					break;

				case FIRE_FACING:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
					} else {
						if (In_Range(TarCom) && !Is_Strafe()) {
							Status = Rule->IsCurleyShuffle ? PICK_ATTACK_LOCATION : FIRE_AT_TARGET;
						} else {
							Status = PICK_ATTACK_LOCATION;
						}
						if (Is_Strafe()) {
							return(45);
						}
					}
					break;

				case FIRE_OK:
					if (In_Range(TarCom)) {
						Fire_At(TarCom, 0);
					}
					Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);

					if (Ammo) {
						Status = Rule->IsCurleyShuffle ? PICK_ATTACK_LOCATION : FIRE_AT_TARGET;
					} else {
						Status = RETURN_TO_BASE;
					}
					break;

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
					} else {
						if (!In_Range(TarCom)) {
							Status = PICK_ATTACK_LOCATION;
						} else {
							Status = Rule->IsCurleyShuffle ? PICK_ATTACK_LOCATION : FIRE_AT_TARGET;
						}
					}
					break;
			}
			break;

		case STRAFE_SHOT2:
			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
				return(1);
			}
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_OK:
				case FIRE_FACING:
				case FIRE_CLOAKED:
					break;

				case FIRE_RANGE:
					Assign_Destination(TarCom);
					break;

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
						IsLockedStraight = false;
					}
					return(1);
			}
			if (In_Range(TarCom)) {
				Fire_At(TarCom, 0);
			}
			Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);
			Assign_Destination(TarCom);
			Status = STRAFE_SHOT3;
			return(PrimaryWeapon->ROF);

		case STRAFE_SHOT3:
			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
				return(1);
			}
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_OK:
				case FIRE_FACING:
				case FIRE_CLOAKED:
					break;

				case FIRE_RANGE:
					Assign_Destination(TarCom);
					break;

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
						IsLockedStraight = false;
					}
					return(1);
			}
			if (In_Range(TarCom)) {
				Fire_At(TarCom, 0);
			}
			Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);
			Assign_Destination(TarCom);
			Status = STRAFE_SHOT4;
			return(PrimaryWeapon->ROF);

		case STRAFE_SHOT4:
			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
				return(1);
			}
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_OK:
				case FIRE_FACING:
				case FIRE_CLOAKED:
					break;

				case FIRE_RANGE:
					Assign_Destination(TarCom);
					break;

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
						IsLockedStraight = false;
					}
					return(1);
			}
			if (In_Range(TarCom)) {
				Fire_At(TarCom, 0);
			}
			Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);
			Assign_Destination(TarCom);
			Status = STRAFE_LAST_SHOT;
			return(PrimaryWeapon->ROF);

		case STRAFE_LAST_SHOT:
			if (TarCom == NULL) {
				Status = RETURN_TO_BASE;
				return(1);
			}
			switch (Can_Fire(TarCom, 0)) {
				case FIRE_OK:
				case FIRE_FACING:
				case FIRE_RANGE:
				case FIRE_CLOAKED:
					if (In_Range(TarCom)) {
						Fire_At(TarCom, 0);
					}
					Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);
					Status = FLY_TO_POSITION;
					return((PrimaryWeapon->Range + 4 * CELL_LEPTON) / Class->MaxSpeed);

				default:
					if (!Ammo) {
						Status = RETURN_TO_BASE;
						IsLockedStraight = false;
					}
					break;
			}
			return(1);

		/*
		**	Fly back to landing spot.
		*/
		case RETURN_TO_BASE:

			IsLockedStraight = false;

			if (IsToSpendAmmo) {
				IsToSpendAmmo = false;
				Ammo--;
			}
			/*
			**	Break off of firing at the target if there is no more
			**	point in attacking it this mission. The player will
			**	reassign a target for the next mission.
			*/
			if (!Ammo) {
				if (IsALoaner || House->Is_Human_Player()) {
					Assign_Target(NULL);
				}
			} else if (TarCom != NULL) {
				Status = PICK_ATTACK_LOCATION;
				return(1);
			}

			IsLockedStraight = false;
			Assign_Destination(NULL);
			Enter_Idle_Mode();
			return(1);

		default:
			break;
	}

	return(Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2));
}


/***********************************************************************************************
 * AircraftClass::New_LZ -- Find a good landing zone.                                          *
 *                                                                                             *
 *    Use this routine to locate a good landing zone that is nearby the location specified.    *
 *    By using this routine it is possible to assign the same landing zone to several          *
 *    aircraft and they will land nearby without conflict.                                     *
 *                                                                                             *
 * INPUT:   oldlz -- Target value of desired landing zone (usually a cell target value).       *
 *                                                                                             *
 * OUTPUT:  Returns with the new good landing zone. It might be the same value passed in.      *
 *                                                                                             *
 * WARNINGS:   The landing zone might be a goodly distance away from the ideal if there is     *
 *             extensive blocking terrain in the vicinity.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AbstractClass * AircraftClass::New_LZ(AbstractClass * oldlz) const
{
	if (oldlz != NULL && (Team == NULL || !Team->Is_Leaving_Map()) && (!Is_LZ_Clear(oldlz) || !Cell_Seems_Ok(oldlz->Destination_Coord().As_Cell()))) {
		Coord coord = oldlz->Center_Coord();

		/*
		**	Scan outward in a series of concentric rings up to certain distance
		**	in cells.
		*/
		for (int radius = 0; radius < (2 * Rule->LZScanRadius) / CELL_LEPTON_W; radius++) {
			FacingType modifier = Random_Pick(FACING_N, FACING_NW);
			Cell lastcell(0,0);

			/*
			**	Perform a radius scan out from the original center location. Try to
			**	find a cell that is allowed to be a legal LZ.
			*/
			for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
				Cell newcell = Move_Cell(Cell(coord), DirType(Facing_Dir(FacingType(facing+modifier))).As_Int() & -(DIR_STEP_8 << 8), radius/* * CELL_LEPTON_W*/);
				if (Map.In_Local_Radar(newcell)) {
					AbstractClass * newtarget = &Map[newcell];

					if (newcell != lastcell && Is_LZ_Clear(newtarget) && Cell_Seems_Ok(newcell)) {
						return(newtarget);
					}
					lastcell = newcell;
				}
			}
		}
	}
	return(oldlz);
}


/***********************************************************************************************
 * AircraftClass::Receive_Message -- Handles receipt of radio messages.                        *
 *                                                                                             *
 *    This routine receives all radio messages directed at this aircraft. It is used to handle *
 *    all inter-object coordination. Typically, this would be for transport helicopters and    *
 *    other complex landing operations required of helicopters.                                *
 *                                                                                             *
 * INPUT:   from     -- The source of this radio message.                                      *
 *                                                                                             *
 *          message  -- The message itself.                                                    *
 *                                                                                             *
 *          param    -- An optional parameter that may be used to transfer additional          *
 *                      data.                                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the radio response from the aircraft.                                 *
 *                                                                                             *
 * WARNINGS:   Some radio messages are handled by the base classes.                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType AircraftClass::Receive_Message(RadioClass * from, RadioMessageType message, int & param)
{
	AbstractClass * target;

	switch (message) {

		case RADIO_RELOAD:
			if (Ammo >= (Class->MaxAmmo / 2) && TarCom != NULL) {
				return(RADIO_ROGER);
			}
			return(BASECLASS::Receive_Message(from, message, param));

		case RADIO_PREPARED:
			if (TarCom != NULL) return(RADIO_NEGATIVE);
			if ((HeightAGL == 0 && Ammo == Class->MaxAmmo) || (HeightAGL > 0 && Ammo > 0)) return(RADIO_ROGER);
			return(RADIO_NEGATIVE);

		case RADIO_ALL_DONE:
			if (Ammo == Class->MaxAmmo) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		/*
		**	Something disastrous has happened to the object in contact with. Fall back
		**	and regroup. This means that any landing process is immediately aborted.
		*/
		case RADIO_RUN_AWAY:
			Scatter(COORD_NONE, true);
			break;

		/*
		**	The ground control requests that this specified landing spot be used.
		*/
		case RADIO_MOVE_HERE:
			BASECLASS::Receive_Message(from, message, param);
			target = (AbstractClass *)param;
			if (target->As_BuildingClass() != NULL) {
				if (Transmit_Message(RADIO_CAN_LOAD, Dynamic_Cast<TechnoClass *>(target)) != RADIO_ROGER) {
					return(RADIO_NEGATIVE);
				}
				Assign_Mission(MISSION_ENTER);
				Assign_Destination(target);
			} else {
				Assign_Mission(MISSION_MOVE);
				Assign_Destination(target);
			}
			Commence();
			return(RADIO_ROGER);

		/*
		**	Ground control is requesting if the aircraft requires navigation direction.
		*/
		case RADIO_NEED_TO_MOVE:
			BASECLASS::Receive_Message(from, message, param);
			if (!Locomotion->Is_Moving() || NavCom == NULL) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		/*
		**	This message is sent by the passenger when it determines that it has
		**	entered the transport.
		*/
		case RADIO_IM_IN:
			if (Cargo.Total_Size() >= Class->Max_Passengers()) {
				Door.Close_Door(Class->DeployTime);
			}

			/*
			**	If a civilian has entered the transport, then the transport will immediately
			**	fly off the map.
			*/
			if (Counts_As_Civ_Evac(from)) {
				Assign_Mission(MISSION_RETREAT);
			}
			return(RADIO_ATTACH);

		/*
		**	Docking maintenance message received. Check to see if new orders should be given
		**	to the impatient unit.
		*/
		case RADIO_DOCKING:
			if (Class->Max_Passengers() > 0 && Can_Fit_Passenger(from)) {
				BASECLASS::Receive_Message(from, message, param);

				if (!Locomotion->Is_Moving()) {

					Door.Open_Door(Class->DeployTime);

					/*
					**	If the potential passenger needs someplace to go, then figure out a good
					**	spot and tell it to go.
					*/
					if (Transmit_Message(RADIO_NEED_TO_MOVE, from) == RADIO_ROGER) {

						/*
						**	Tell the potential passenger where it should go. If the passenger is
						**	already at the staging location, then tell it to move onto the transport
						**	directly.
						*/
						param = (int)this;
						if (Transmit_Message(RADIO_MOVE_HERE, param, from) != RADIO_ROGER) {
							Transmit_Message(RADIO_OVER_OUT, from);
						} else {
							Contact_With_Whom()->Unselect();
						}
					}
				}
				return(RADIO_ROGER);
			}
			break;

		/*
		**	Asks if the passenger can load on this transport.
		*/
		case RADIO_CAN_LOAD:
			if (Class->Max_Passengers() == 0 || from == NULL || !House->Is_Ally(from)) return(RADIO_STATIC);
			if (from->RTTI == RTTI_UNIT && !Class->IsVehicleTransport) return(RADIO_STATIC);
			if (Can_Fit_Passenger(from)) {
				return(RADIO_ROGER);
			}
			return(RADIO_NEGATIVE);

		case RADIO_UNLOADED:
			if (Class->IsCarryall && Mission == MISSION_MOVE && IsTethered) {
				if ((Cargo.Is_Something_Attached() && Cargo.Attached_Object() == from) || NavCom == from) {
					return(RADIO_NEGATIVE);
				}
			}
			break;

		default:
			break;
	}

	/*
	**	Let the base class take over processing this message.
	*/
	return(BASECLASS::Receive_Message(from, message, param));
}


/***********************************************************************************************
 * AircraftClass::Desired_Load_Dir -- Determines where passengers should line up.              *
 *                                                                                             *
 *    This routine is used by the transport helicopter to determine the location where the     *
 *    infantry passengers should line up before loading.                                       *
 *                                                                                             *
 * INPUT:   object   -- The object that is trying to load up on this transport.                *
 *                                                                                             *
 *                   -- Reference to the cell that the passengers should move to before the    *
 *                      actual load process may begin.                                         *
 *                                                                                             *
 * OUTPUT:  Returns with the direction that the helicopter should face for the load operation. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   07/30/1995 JLB : Revamped to scan all adjacent cells.                                     *
 *=============================================================================================*/
FacingType AircraftClass::Desired_Load_Dir(ObjectClass * object, Cell & moveto) const
{
	static FacingType _toface[FACING_COUNT] = {FACING_S, FACING_SW, FACING_SE, FACING_W, FACING_E, FACING_NW, FACING_NE, FACING_N};

	moveto = CELL_NONE;

	Cell center = Center_Coord().As_Cell();
	for (int face = FACING_FIRST; face < FACING_COUNT; face++) {
		moveto = Adjacent_Cell(center, _toface[face]);
		if (Map.In_Radar(moveto) && (object->Center_Coord().As_Cell() == moveto || Map[moveto].Is_Clear_To_Move(SPEED_FOOT, false, false))) return(FACING_N);
	}
	return(FACING_N);
}


/***********************************************************************************************
 * AircraftClass::Can_Enter_Cell -- Determines if the aircraft can land at this location.      *
 *                                                                                             *
 *    This routine is used when the passability of a cell needs to be determined. This is      *
 *    necessary when scanning for a location that the aircraft can land.                       *
 *                                                                                             *
 * INPUT:   cell  -- The cell location to check for landing.                                   *
 *                                                                                             *
 * OUTPUT:  Returns a value indicating if the cell is a legal landing spot or not.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
MoveType AircraftClass::Can_Enter_Cell(CellClass const * cell, FacingType, int cell_height, CellClass const *, bool) const
{
	if (Team && Team->Is_Leaving_Map() && !Map.In_Local_Radar(cell)) return(MOVE_OK);

	ObjectClass const * occupier = cell->Cell_Occupier();

	if (occupier == NULL ||
		!occupier->Is_Techno() ||
		((TechnoClass *)occupier)->House->Is_Ally(House) ||
		(((TechnoClass *)occupier)->Cloak != CLOAKED &&
			(ScenarioInit == 0 && (occupier->RTTI != RTTI_BUILDING || !((BuildingClass*)occupier)->Class->IsInvisible)) )
		) {

		if (!cell->Is_Clear_To_Move(SPEED_WINGED, false, false)) return(MOVE_NO);
	}

	if (Session.Type == GAME_NORMAL && IsOwnedByPlayer && !IsALoaner && Map.Is_Shrouded(cell->Center_Coord())) {
		return(MOVE_NO);
	}

	return(MOVE_OK);
}


/***********************************************************************************************
 * AircraftClass::Good_Fire_Location -- Searches for and finds a good spot to fire from.       *
 *                                                                                             *
 *    Given the specified target, this routine will locate a good spot for the aircraft to     *
 *    fire at the target.                                                                      *
 *                                                                                             *
 * INPUT:   target   -- The target that is desired to be attacked.                             *
 *                                                                                             *
 * OUTPUT:  Returns with the target location of the place that firing should be made from.     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   06/14/1995 JLB : Finer resolution on ring scan.                                           *
 *   11/02/1996 JLB : Bias fire position to get closer to moving objects.                      *
 *=============================================================================================*/
AbstractClass * AircraftClass::Good_Fire_Location(AbstractClass * target) const
{
	if (target != NULL) {
		if (((AircraftClass *)this)->Is_Strafe()) {
			return(target);
		}

		int range = Weapon_Range(0);
		Coord tcoord = target->Center_Coord();
		Cell bestcell = CELL_NONE;
		Cell best2cell = CELL_NONE;
		int bestval = -1;
		int best2val = -1;

		/*
		**	Try to get closer to a target that is moving.
		*/
		Coord altcoord = COORD_NONE;
		FootClass * foot = target->As_FootClass();
		if (foot != NULL) {
			AbstractClass * alttarg = foot->NavCom;
			if (alttarg != NULL) {
				altcoord = alttarg->Center_Coord();
			}
		}

		for (int r = range-CELL_LEPTON; r > CELL_LEPTON; r -= CELL_LEPTON) {
			for (int face = 0; face < DIR_MAX; face += DIR_STEP_16) {
				Coord newcoord = Move_Coord(tcoord, (Dir256)face, r);
				Cell newcell = newcoord.As_Cell();

				if (Map.In_Local_Radar(newcell) && (Session.Type != GAME_NORMAL || Map[newcell].IsVisible) && Cell_Seems_Ok(newcell, true)) {
					int dist;
					if (altcoord != COORD_NONE) {
						dist = Point2D(newcoord).Distance_To(Point2D(altcoord));
					} else {
						dist = Point2D(Center_Coord()).Distance_To(Point2D(newcoord));
					}
					if (bestval == -1 || dist < bestval) {
						best2val = bestval;
						best2cell = bestcell;
						bestval = dist;
						bestcell = newcell;
					}
				}
			}
			if (bestval != -1) break;
		}

		if (best2val == -1) {
			best2cell = bestcell;
		}

		/*
		**	If it found a good firing location, then return this location as
		**	a target value.
		*/
		if (bestval != -1) {
			if (Percent_Chance(50)) {
				return(&Map[bestcell]);
			} else {
				return(&Map[best2cell]);
			}
		}
	}
	return(NULL);
}


/***********************************************************************************************
 * AircraftClass::Cell_Seems_Ok -- Checks to see if a cell is good to enter.                   *
 *                                                                                             *
 *    This routine examines the navigation computers of other aircraft in order to see if the  *
 *    specified cell is safe to fly to. The intent of this routine is to avoid unnecessary     *
 *    mid-air collisions.                                                                      *
 *                                                                                             *
 * INPUT:   cell     -- The cell to examine for clear airspace.                                *
 *                                                                                             *
 *          strict   -- Should the scan consider the aircraft, that is making this check, a    *
 *                      blocking aircraft. Typically, the aircraft itself is not considered    *
 *                      a blockage -- an aircraft can always exist where it is currently       *
 *                      located. A strict check is useful for helicopters that need to move    *
 *                      around at the slightest provocation.                                   *
 *                                                                                             *
 * OUTPUT:  Is the specified cell free from airspace conflicts?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool AircraftClass::Cell_Seems_Ok(Cell const & cell, bool strict) const
{
	if (!Map.In_Local_Radar(cell)) {
		return(true);
	}

	bool is_toting = (Class->IsCarryall && NavCom != NULL && NavCom->RTTI == RTTI_UNIT && ((UnitClass const *)NavCom)->Class->IsTotable);

	/*
	**	Make sure that no other aircraft are heading to the selected location. If they
	**	are, then don't consider the location as valid.
	*/
	AbstractClass * astarget = &Map[cell];
	for (int index = 0; index < Feet.Count(); index++) {
		FootClass * foot = Feet[index];
		if (foot && (!is_toting || NavCom != foot) && (strict || foot != this) && !foot->IsInLimbo && foot->IsDown) {
			if (foot->PositionCell == cell) {
				return(false);
			}

			if (foot->IsActive && foot->RTTI == RTTI_AIRCRAFT && foot->NavCom == astarget &&
				(foot->House == House || (House->Is_Ally(foot) && foot->House->Is_Ally(this)))) {
				return(false);
			}
		}
	}
	return(true);
}


/***********************************************************************************************
 * AircraftClass::Mission_Enter -- Control aircraft to fly to the helipad or repair center.    *
 *                                                                                             *
 *    This routine is used when the aircraft needs to fly for either rearming or repairing.    *
 *    It tries to establish contact with the support building. Once contact is established     *
 *    the ground controller takes care of commanding the aircraft.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the delay before this routine should be called again.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   07/04/1995 JLB : Ground controller gives orders.                                          *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_ENTER(void)
{
	enum {
		INITIAL,
		TAKEOFF,
		ALTITUDE,
		STACK,
		DOWNWIND,
		CROSSWIND,
		TRAVEL,
		LANDING
	};

	/*
	**	Verify that it has a valid NavCom. If it doesn't then request one from the
	**	building this building is trying to land upon. If that fails, then enter
	**	idle mode.
	*/
	if (NavCom == NULL && In_Which_Layer() != LAYER_GROUND) {
		if (Transmit_Message(RADIO_DOCKING) != RADIO_ROGER && !Move_To_Object_Nearby()) {
			Enter_Idle_Mode();
			return(1);
		}
	}

	switch (Status) {
		case INITIAL:
			Status = TRAVEL;
			IsReadyToCommence = true;
			break;

		case TAKEOFF:
		case ALTITUDE:
		case STACK:
		case DOWNWIND:
		case CROSSWIND:
			IsReadyToCommence = true;
			break;

		case TRAVEL:
			Transmit_Message(RADIO_DOCKING);
			if (!In_Radio_Contact()) {
				if (!Move_To_Object_Nearby()) {
					if (Locomotion->Get_Status() == 1) {
						Status = LANDING;
						IsReadyToCommence = true;
					}
					if (MissionQueue == MISSION_NONE) {
						Assign_Destination(NULL);
						Enter_Idle_Mode();
						IsReadyToCommence = true;
						break;
					} else {
						IsReadyToCommence = true;
					}
				}
				IsReadyToCommence = true;
				break;
			} else {
				if (Locomotion->Get_Status() == 1) {
					Status = LANDING;
					IsReadyToCommence = false;
				}
				return(3);
			}
			break;

		case LANDING:
			if (Locomotion->Get_Status() == 1) {
				if (NavCom != NULL) {
					Coord nav = NavCom->Center_Coord();
					Coord pos = PositionCoord;
					int x = nav.X - pos.X;
					int y = nav.Y - pos.Y;
					x = x > 0 ? std::min(5, x) : std::max(-5, x);
					y = y > 0 ? std::min(5, y) : std::max(-5, y);
					pos += Coord(x, y, 0);
					PositionCoord = pos;
				}
				IsReadyToCommence = true;
			} else {
				IsReadyToCommence = true;
				switch (Transmit_Message(RADIO_IM_IN)) {
					case RADIO_ROGER:
						if (Class->IsCarryall && Cargo.Is_Something_Attached()) {
							TechnoClass * contact = Contact_With_Whom();
							TechnoClass * cargo = Cargo.Detach_Object();
							Transmit_Message(RADIO_OVER_OUT);
							if (cargo->Transmit_Message(RADIO_HELLO, contact) == RADIO_ROGER) {
								Cargo.Attach((FootClass *)cargo);
								Mark(MARK_UP);
								Drop_Off_Cargo();
								Mark(MARK_DOWN);
							} else {
								Assign_Destination(Good_LZ());
							}
							Assign_Mission(MISSION_MOVE);
						} else {
							Assign_Mission(MISSION_GUARD);
						}
						break;

					case RADIO_ATTACH:
						Limbo();
						Contact_With_Whom()->Cargo.Attach(this);
						break;

					default:
						Enter_Idle_Mode();
				}
			}
			break;

		default:
			break;
	}
	return(1);
}


/***********************************************************************************************
 * AircraftClass::Good_LZ -- Locates a good spot to land.                                      *
 *                                                                                             *
 *    This routine is used when helicopters need a place to land, but there are no obvious     *
 *    spots (i.e., helipad) available. It will try to land near a friendly helipad or friendly *
 *    building if there are no helipads anywhere. In the event that there are no friendly      *
 *    buildings anywhere on the map, then just land right where it is flying.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the target location where this aircraft should land. This value may   *
 *          not be a clear cell, but the normal landing logic will resolve that problem.       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AbstractClass * AircraftClass::Good_LZ(void) const
{
	/*
	**	Scan through all of the buildings and try to land near
	**	the helipad (if there is one) or the nearest friendly building.
	*/
	Cell bestcell = Cell(0, 0);
	int bestdist = -1;

	if (Class->Dock.Count() > 0) {
		for (int index = 0; index < Buildings.Count(); index++) {
			BuildingClass * building = Buildings[index];

			if (building && !building->IsInLimbo && building->House == House) {
				int dist = Distance_To(building);
				if (building->Class == Class->Dock[0]) {
					dist /= 4;
				}
				if (bestdist == -1 || dist < bestdist) {
					Cell cell = building->PositionCell;
					cell = Map.Nearby_Location(cell, SPEED_FOOT, Map.Get_Cell_Zone(cell), MZONE_NORMAL, false, Point2D(3, 3));
					if (cell != CELL_NONE) {
						bestdist = dist;
						bestcell = cell;
					}
				}
			}
		}
	}

	if (bestdist != -1) {
		if (bestdist < CELL_LEPTON) {
			bestcell = Map.Nearby_Location(PositionCell, SPEED_FOOT, Map.Get_Cell_Zone(PositionCell));
			return(&Map[bestcell]);
		}
		return(&Map[bestcell]);
	}

	if (bestdist == -1) {
		for (int index = 0; index < Objects.Count(); index++) {
			TechnoClass * techno = Dynamic_Cast<TechnoClass *>(Objects[index]);
			if (techno && !techno->IsInLimbo && techno->House == House && techno != (AircraftClass *)this) {
				int dist = Distance_To(techno);
				if (bestdist == -1 || dist < bestdist) {
					Cell cell = techno->PositionCell;
					cell = Map.Nearby_Location(cell, SPEED_FOOT, Map.Get_Cell_Zone(cell));
					if (cell != CELL_NONE) {
						bestdist = dist;
						bestcell = cell;
					}
				}
			}
		}
	}

	/*
	**	Return with the suitable location if one was found.
	*/
	if (bestdist != -1) {
		return(&Map[bestcell]);
	}

	/*
	**	No good location was found. Just try to land here.
	*/
	return(&Map[(Coord const &)PositionCoord]);
}


/***********************************************************************************************
 * AircraftClass::Fire_Direction -- Determines the direction of fire.                          *
 *                                                                                             *
 *    This routine will determine what direction a projectile would take if it were fired      *
 *    from the aircraft. This is the direction that the aircraft's body is facing.             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the direction of projectile fire.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
DirType AircraftClass::Fire_Direction(void) const
{
	return(SecondaryFacing.Current());
}


/***********************************************************************************************
 * AircraftClass::~AircraftClass -- Destructor for aircraft object.                            *
 *                                                                                             *
 *    This is the destructor for aircraft. It will limbo the aircraft if it isn't already      *
 *    and also removes the aircraft from any team it may be attached to.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
AircraftClass::~AircraftClass(void)
{
	if (GameActive && Class) {

		if (House->Can_Build(Class, false, false) == -1) {
			House->IsRecalcNeeded = true;
		}

		/*
		**	If there are any cargo members, delete them.
		*/
		Kill_Cargo(NULL);

		/*
		**	Remove this member from any team it may be associated with. This must occur at the
		**	top most level of the inheritance hierarchy because it may call virtual functions.
		*/
		if (Team) {
			Team->Remove(this);
			Team = NULL;
		}

		House->Tracking_Remove(this);

		AircraftClass::Limbo();
		Class = NULL;
	}

	Detach_This_From_All(this);
	Aircraft.Delete(this);
	TargetTracker.Remove_Index(Fetch_ID());
	IsActive = false;
}


/***********************************************************************************************
 * AircraftClass::Scatter -- Causes the aircraft to move away a bit.                           *
 *                                                                                             *
 *    This routine will cause the aircraft to move away from its current location and then     *
 *    enter some idle mode. Typically this is called when the aircraft is attacked while on    *
 *    the ground.                                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Scatter(Coord const & , bool, bool )
{
	/*
	**	Certain missions prevent scattering regardless of whether it would be
	**	a good idea or not.
	*/
	if (!Current_Mission_Control().IsScatter) return;

	Enter_Idle_Mode();
}


/***********************************************************************************************
 * AircraftClass::Mission_Guard -- Handles aircraft in guard mode.                             *
 *                                                                                             *
 *    Aircraft don't like to be in guard mode if in flight. If this situation is detected,     *
 *    then figure out what the aircraft should be doing and go do it.                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine again. *
 *                                                                                             *
 * WARNINGS:   This routine typically calls the normal guard logic for ground units.           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1995 JLB : Created.                                                                 *
 *   10/10/1995 JLB : Hunts for harvesters that are unescorted.                                *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_GUARD(void)
{
	if (HeightAGL == Class->Flight_Level()) {

		/*
		**	If part of a team, then do nothing, since the team
		**	handler will take care of giving this aircraft a
		**	mission.
		*/
		if (Team) {
			if (NavCom != NULL) {
				Assign_Mission(MISSION_MOVE);
			}
			return(Current_Mission_Control().Normal_Delay());
		}

		if (PrimaryWeapon == NULL) {
			Assign_Destination(&Map[(Coord const &)PositionCoord]);
			Assign_Mission(MISSION_MOVE);
		} else {
			if (Team == NULL) Enter_Idle_Mode();
		}
		return(1);
	}
	//if (House->IsHuman) return(MissionControl[Mission].Normal_Delay());

	/*
	**	If the aircraft is very badly damaged, then it will search for a
	**	repair bay first.
	*/
	if (!House->Is_Human_Player() && House->Available_Money() >= 100 && HealthRatio <= Rule->ConditionYellow) {
		if (!In_Radio_Contact() ||
			(HeightAGL == 0 &&
				(Contact_With_Whom()->RTTI != RTTI_BUILDING || ((BuildingClass *)Contact_With_Whom())->Class_Of() != Rule->RepairBay))) {

			BuildingClass * building = Find_Docking_Bay(Rule->RepairBay, true);
			if (building != NULL) {
				Assign_Mission(MISSION_ENTER);
				Assign_Destination(building);
				Assign_Target(NULL);
				return(1);
			}
		}
	}

	/*
	**	If the aircraft cannot attack anything because of lack of ammo,
	**	abort any normal guard logic in order to look for a helipad
	**	to rearm.
	*/
	if (!House->Is_Human_Player() && Ammo == 0 && Is_Weapon_Equipped()) {
		if (!In_Radio_Contact()) {
			BuildingClass * building = NULL;
			for (int index = 0; index < Class->Dock.Count(); index++) {
				building = Find_Docking_Bay(Class->Dock[index], false);
				if (building != NULL) {
					break;
				}
			}

			if (building != NULL) {
				Assign_Mission(MISSION_ENTER);
				Assign_Destination(building);
				Assign_Target(NULL);
				return(1);
			}
		}
	}

	if (Ammo != -1 && Ammo < (Class->MaxAmmo / 2) && In_Radio_Contact()) {
		if (Contact_With_Whom()->RTTI == RTTI_BUILDING && ((BuildingClass *)Contact_With_Whom())->Class->IsCanUnitReload) {
			return(1);
		}
	}

	/*
	**	If the aircraft already has a target, then attack it if possible.
	*/
	if (TarCom != NULL) {
		Assign_Mission(MISSION_ATTACK);
		return(1);
	}

	/*
	**	Transport helicopters don't really do anything but just sit there.
	*/
	if (!Is_Weapon_Equipped()) {
		return(TICKS_PER_SECOND*3);
	}

	/*
	**	Computer controlled helicopters will defend themselves by bouncing around
	**	and looking for a free helipad.
	*/
	if (HeightAGL == 0 && !In_Radio_Contact()) {
		//Scatter(COORD_NONE, true);
		return(TICKS_PER_SECOND*3);
	}

	/*
	**	Perform a special check to hunt for harvesters that are outside of the protective
	**	shield of their base.
	*/
	if (!House->Is_Human_Player() && House->State != STATE_ATTACKED) {
		AbstractClass * target = House->Find_Juicy_Target(PositionCoord);

		if (target != NULL) {
			Assign_Target(target);
			Assign_Mission(MISSION_ATTACK);
		}
	}

	if (House->Is_Human_Player() && !In_Air()) {
		return(MISSION_GUARD);
	}

	return(BASECLASS::Do_MISSION_GUARD());
}


/***********************************************************************************************
 * AircraftClass::Mission_Guard_Area -- Handles the aircraft guard area logic.                 *
 *                                                                                             *
 *    This routine handles area guard logic for aircraft. Aircraft require special handling    *
 *    for this mode since they are to guard area only if they are in a position to do so.      *
 *    Otherwise they just defend themselves.                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine        *
 *          again.                                                                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int AircraftClass::Do_MISSION_GUARD_AREA(void)
{
	if (HeightAGL == Class->Flight_Level()) {
		if (Team == NULL) Enter_Idle_Mode();
		return(1);
	}
	if (TarCom != NULL) {
		Assign_Mission(MISSION_ATTACK);
		return(1);
	}
	return(BASECLASS::Do_MISSION_GUARD_AREA());
}


/// <summary>
/// Determines whether the player may give this aircraft a fire order.
/// A grounded aircraft with no ammunition must remain available for reloading instead of
/// accepting an attack that it cannot begin.
/// </summary>
/// <returns>Can the player give this aircraft a fire order?</returns>
bool AircraftClass::Can_Player_Fire(void) const
{
	if (Ammo == 0 && !In_Air()) {
		return(false);
	}
	return(BASECLASS::Can_Player_Fire());
}


/***********************************************************************************************
 * AircraftClass::Can_Fire -- Checks to see if the aircraft can fire.                          *
 *                                                                                             *
 *    This routine is used to determine if the aircraft can fire its weapon at the target      *
 *    specified. If it cannot, then the reason why is returned.                                *
 *                                                                                             *
 * INPUT:   target   -- The target that the aircraft might fire upon.                          *
 *                                                                                             *
 *          which    -- The weapon that will be used to fire.                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the reason why it can't fire or with FIRE_OK.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/05/1996 JLB : Created.                                                                 *
 *   07/11/1996 JLB : Fixed for camera carrying aircraft.                                      *
 *=============================================================================================*/
FireErrorType AircraftClass::Can_Fire(AbstractClass * target, int which) const
{
	if (IonStormClass::Is_Ion_Storm_Active()) {
		return(FIRE_CANT);
	}

	if (Passenger && !Cargo.Is_Something_Attached()) {
		return(FIRE_AMMO);
	}

	/*
	**	Double check to make sure that the facing is roughly toward
	**	the target. If the difference is too great, then firing is
	**	temporarily postponed.
	*/
	DirType dir = SecondaryFacing.Current();
	dir -= Direction(target);
	if (DIR_STEP_32 < dir) {
		return(FIRE_FACING);
	}

	return(BASECLASS::Can_Fire(target, which));
}


/***********************************************************************************************
 * AircraftClass::Assign_Destination -- Assigns movement destination to the object.            *
 *                                                                                             *
 *    This routine is called when the object needs to have a new movement destination          *
 *    assigned.  Aircraft have their own version of this routine because a fixed-wing plane    *
 *    trying to land will behave poorly if given a new destination while it's landing.         *
 *                                                                                             *
 * INPUT:   destination -- The destination to assign to this object.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Assign_Destination(AbstractClass * dest, bool immediate)
{
	if (dest != NULL) {

		/*
		 * A flying aircraft that is told to go somewhere must abort any landing
		 * approach it might be in the middle of.
		 */
		if (dest->In_Air()) {
			dest = NULL;
		} else {

			/*
			 * Special docking logic applies when this aircraft has been ordered to
			 * enter a building (a helipad or a repair facility).
			 */
			if (dest->What_Am_I() == RTTI_BUILDING && (Mission == MISSION_ENTER || MissionQueue == MISSION_ENTER)) {
				BuildingClass * destination = (BuildingClass *)dest;

				if (!In_Radio_Contact()) {

					/*
					 * The destination building is already occupied (in radio contact).
					 * Try to find an alternate docking bay to head for instead.
					 */
					if (((TechnoClass *)dest)->In_Radio_Contact()) {
						ArchiveTarget = dest;

						if (destination->Class->IsHelipad) {
							dest = NULL;
							for (int index = 0; index < Class->Dock.Count(); index++) {
								dest = Find_Docking_Bay(Class->Dock[index], false, true);
								if (dest != NULL) {
									break;
								}
							}

							Assign_Destination(NULL);

							MissionType mission = MISSION_MOVE;
							if (dest != NULL && Transmit_Message(RADIO_CAN_LOAD, (TechnoClass *)dest) == RADIO_ROGER) {
								Transmit_Message(RADIO_HELLO, (TechnoClass *)dest);
								mission = MISSION_ENTER;
							} else {
								dest = Good_LZ();
							}
							Assign_Mission(mission);
							if (Ready_To_Commence()) {
								Commence();
							}
						}

						if (destination->Class->IsCanUnitRepair) {
							NearbyObject = destination;
							dest = NULL;
						}

					} else {

						/*
						 * The destination is free. Ask permission to dock; if refused,
						 * break contact and fall back on the archive target.
						 */
						if (Transmit_Message(RADIO_DOCKING, (TechnoClass *)dest) != RADIO_ROGER) {
							Transmit_Message(RADIO_OVER_OUT, LParam, NULL);
							if (((BuildingClass *)dest)->Class->IsCanUnitRepair || ((BuildingClass *)dest)->Class->IsCanUnitReload) {
								ArchiveTarget = dest;
								dest = NULL;
							}
						}

						if (destination->Class->IsCanUnitRepair || destination->Class->IsCanUnitReload) {
							if (NavCom != NULL) {
								ArchiveTarget = NavCom;
							}
						} else {
							ArchiveTarget = dest;
						}
					}

				} else {

					/*
					 * This aircraft is already in radio contact. If the destination
					 * building is not yet in contact, perform the docking handshake. If
					 * it is already our own contact there is nothing to do; if it is in
					 * contact with someone else, just stash it as the archive target.
					 */
					TechnoClass * building = Dynamic_Cast<TechnoClass *>(dest);
					if (building != NULL) {
						if (building->In_Radio_Contact()) {
							if (Contact_With_Whom() != building) {
								ArchiveTarget = dest;
							}
						} else {
							if (Transmit_Message(RADIO_HELLO, building) == RADIO_ROGER) {
								if (Transmit_Message(RADIO_DOCKING, LParam, NULL) == RADIO_ROGER) {
									return;
								}
								Transmit_Message(RADIO_OVER_OUT, LParam, NULL);
								if (((BuildingClass *)dest)->Class->IsCanUnitRepair || ((BuildingClass *)dest)->Class->IsCanUnitReload) {
									ArchiveTarget = dest;
									dest = NULL;
								}
							}
						}
					}
				}
			}

			/*
			 * If sitting on top of a repair or reload building, make sure power is
			 * restored to the locomotion and break radio contact with it if this
			 * aircraft is being sent somewhere other than that building.
			 */
			CellClass * cellptr = Get_Cell_Ptr();
			if (!cellptr->IsUnderBridge) {
				BuildingClass * occupier = (BuildingClass *)cellptr->Cell_Occupier();

				while (occupier != NULL) {
					if (occupier != (BuildingClass *)this && occupier->RTTI == RTTI_BUILDING) {
						break;
					}
					occupier = (BuildingClass *)occupier->Next;
				}

				if (occupier != NULL && (occupier->Class->IsCanUnitRepair || occupier->Class->IsCanUnitReload)) {
					if (!Locomotion->Is_Powered() && !IonStormClass::Is_Ion_Storm_Active()) {
						Locomotion->Power_On();
					}

					BuildingClass * contact = (BuildingClass *)Contact_With_Whom();
					if ((FootClass *)contact == (FootClass *)occupier && (FootClass *)dest != (FootClass *)contact) {
						Transmit_Message(RADIO_OVER_OUT, LParam, NULL);
					}
				}
			}
		}
	}

	BASECLASS::Assign_Destination(dest, immediate);
}


/***********************************************************************************************
 * AircraftClass::In_Which_Layer -- Calculates the display layer of the aircraft.              *
 *                                                                                             *
 *    This examines the aircraft to determine what display layer it should be located          *
 *    in. Fixed wing aircraft must always be in the top layer if they are flying even though   *
 *    they may be low to the ground.                                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the layer that this aircraft resides in.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
LayerType AircraftClass::In_Which_Layer(void) const
{
	return(Locomotion->In_Which_Layer());
}


/***********************************************************************************************
 * AircraftClass::Look -- Aircraft will look if they are on the ground always.                 *
 *                                                                                             *
 *    Aircraft perform a look operation according to their sight range. If the aircraft is     *
 *    on the ground, then it will look a distance of one cell regardless of what its           *
 *    specified sight range is.                                                                *
 *                                                                                             *
 * INPUT:   incremental -- Is this an incremental look?                                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/23/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Look(bool incremental, bool dont_map)
{
	assert(!IsInLimbo);

	int sight_range = Class->SightRange;
	if (HeightAGL == 0) {
		sight_range = 1;
	}

	if (sight_range) {
		Map.Sight_From(PositionCoord, sight_range, House, incremental, dont_map);
	} else {
		if (Scen->Special.IsFogOfWar) {
			sight_range = Rule->AircraftFogReveal;
			Map.Sight_From(PositionCoord, sight_range, House, false, false, true, HeightAGL < (Rule->FlightLevel / 2));
		}
	}
}


/***********************************************************************************************
 * AircraftClass::Write_INI -- Store the units to the INI database.                            *
 *                                                                                             *
 *    This routine will store all the unit data to the INI database.                           *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database object to store to.                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Write_INI(CCINIClass & ini)
{
	/*
	 * First, clear out all existing aircraft data from the ini file.
	 */
	ini.Clear(INI_NAME);

	/*
	 * Write the aircraft data out.
	 */
	for (int index = 0; index < Aircraft.Count(); index++) {
		AircraftClass * air = Aircraft[index];
		if (air != NULL && !air->IsInLimbo && air->IsActive) {
			char	uname[10];
			char	buf[128];

			sprintf(uname, "%d", index);

			sprintf(buf, "%s,%s,%d,%d,%d,%d,%s,%s,%d,%d,%d,%d",
				(char const *)air->House->Class->IniName,
				(char const *)air->Class->IniName,
				(int)(air->HealthRatio*256),
				air->PositionCell.X,
				air->PositionCell.Y,
				air->PrimaryFacing.Current().As_Dir256(),
				MissionClass::Mission_Name(air->Mission),
				(air->Tag != NULL) ? (char const *)air->Tag->Class->IniName : "None",
				air->Veterancy.To_Integer(),
				air->Group,
				air->IsTeamRecruitable,
				air->IsAutocreateRecruitable
				);
			ini.Put_String(INI_NAME, uname, buf);
		}
	}
}


/***********************************************************************************************
 * AircraftClass::Read_INI -- Reads aircraft object data from an INI file.                     *
 *                                                                                             *
 *    This routine is used to read the aircraft object data from the INI file buffer           *
 *    specified. This is used by the scenario loader code to interpret the INI file and        *
 *    create the specified objects therein.                                                    *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the INI buffer.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void AircraftClass::Read_INI(CCINIClass const & ini)
{
	AircraftClass	* air;      // Working unit pointer.
	AircraftType	classid;    // Unit class.
	char				buf[128];

	int counter = ini.Entry_Count(INI_NAME);

	for (int index = 0; index < counter; index++) {
		char const * entry = ini.Get_Entry(INI_NAME, index);

		ini.Get_String(INI_NAME, entry, NULL, buf, sizeof(buf));

		HouseClass * inhousep = House_From_Name(strtok(buf, ","));
		if (inhousep != NULL) {
			classid = AircraftTypeClass::From_Name(strtok(NULL, ","));

			if (classid != AIRCRAFT_NONE) {
				air = new AircraftClass(AircraftTypes[classid], inhousep);
				if (air) {

					/*
					**	Read the raw data.
					*/
					int strength = atoi(strtok(NULL, ","));

					Cell cell;
					Coord coord;
					if (NewINIFormat >= 4) {
						unsigned short x = atoi(strtok(NULL, ","));
						unsigned short y = atoi(strtok(NULL, ","));
						cell = Cell(x,y);
					} else {
						int c = atoi(strtok(NULL, ","));
						cell = Cell(c);
					}

					coord = cell;

					Dir256 dir = (Dir256)atoi(strtok(NULL, ","));

					MissionType mission = MissionClass::Mission_From_Name(strtok(NULL, ","));

					TagTypeClass * tp = TagTypeClass::From_Name(strtok(NULL,","));
					if (tp != NULL) {
						TagClass * tt = Find_Or_Make(tp);
						if (tt != NULL) {
							air->Attach_Tag(tt);
						}
					}

					char * token = strtok(NULL, ",");
					if (token != NULL) {
						air->Veterancy.From_Integer(atoi(token));
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						air->Group = atoi(token);
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						air->IsTeamRecruitable = atoi(token) != 0;
					}

					token = strtok(NULL, ",");
					if (token != NULL) {
						air->IsAutocreateRecruitable = atoi(token) != 0;
					}

					if (air->Unlimbo(coord, dir)) {
						air->Strength = ((double)air->Class->MaxStrength) * strength / 256.0;
						if (air->Strength > air->Class->MaxStrength-3) air->Strength = air->Class->MaxStrength;
						if (air->Strength == 0) air->Strength = 1;
						if (Session.Type == GAME_NORMAL || air->House->Is_Human_Player()) {
							air->Assign_Mission(mission);
							air->Commence();
						} else {
							air->Enter_Idle_Mode();
						}
					} else {
						/*
						**	If the aircraft could not be unlimboed, then this is a catastrophic error
						**	condition. Delete the aircraft.
						*/
						delete air;
					}
				}
			}
		}
	}
}


/// <summary>
/// Reads this aircraft back in from the save game stream.
/// The aircraft is withdrawn from the target tracker under the identity it is carrying now,
/// since the one it is about to be given is the one it was saved with. Post_Load enters it
/// again once that identity has arrived.
/// </summary>
/// <param name="stream">The stream to read this object from.</param>
/// <returns>Returns with S_OK if the aircraft was loaded successfully.</returns>
HRESULT STDMETHODCALLTYPE AircraftClass::Load(IStream * stream)
{
	TargetTracker.Remove_Index(Fetch_ID());
	return(BASECLASS::Load(stream));
}


/// <summary>
/// Lists the members this aircraft carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AircraftClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(IsToSpendAmmo);
	stream.Serialize(Passenger);
	stream.Serialize(IsKamikaze);
	stream.Serialize(field_35B);
	stream.Serialize(IsLockedStraight);
	stream.Serialize(SightTimer);
	stream.Serialize(AttacksRemaining);
	stream.Serialize(IsReadyToCommence);
}


/// <summary>
/// Enters this aircraft in the target tracker under the identity it was saved with.
/// </summary>
void AircraftClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	TargetTracker.Add_Index(Fetch_ID(), this);
}


/// <summary>
/// Is this aircraft ready to move on to its next mission step?
/// An aircraft that is holding a locked heading, or that is on a sticky or rescue
/// mission, is left to finish what it is doing.
/// </summary>
/// <returns>bool; Is the aircraft ready to commence?</returns>
bool AircraftClass::Ready_To_Commence(void)
{
	if (CurrentMission != MISSION_STICKY && CurrentMission != MISSION_RESCUE && !IsLockedStraight) {
		if (IsReadyToCommence) {
			return(true);
		}
		return(false);
	}
	return(false);
}


/// <summary>
/// Adds this aircraft's state to the running checksum.
/// This routine is used by the network synchronization check to prove that every machine
/// agrees about the condition of this aircraft.
/// </summary>
/// <param name="crc">The checksum engine to submit this object's state to.</param>
void AircraftClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Passenger);
	crc(IsKamikaze);
	crc(field_35B);
	crc((int)SightTimer);
	crc(AttacksRemaining);
}


/// <summary>
/// Removes all references to the specified object.
/// This routine is called when an object is about to disappear, so that this aircraft is
/// not left holding a pointer to it.
/// </summary>
/// <param name="target">The object that is going away.</param>
/// <param name="all">Should even the loosest of references be severed?</param>
void AircraftClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	if (Class == target) {
		Class = NULL;
	}
}


/// <summary>
/// Fetches the altitude that this aircraft should settle at.
/// An empty carryall touches right down when it has been sent to enter a helipad or
/// repair bay, but otherwise a carryall holds station just above the ground so that it
/// can pick up or set down its cargo.
/// </summary>
/// <returns>Returns with the height above ground level to settle at.</returns>
LONG STDMETHODCALLTYPE AircraftClass::Landing_Altitude(void)
{
	if (Class->IsCarryall && !Cargo.Is_Something_Attached() && In_Radio_Contact()) {
		BuildingClass * bptr = (BuildingClass *)Contact_With_Whom();
		if (Mission == MISSION_ENTER) {
			if (bptr != NULL) {
				if (bptr->Class->IsCanUnitRepair || bptr->Class->IsHelipad) {
					return(0);
				}
			}
		}
	}

	if ((In_Radio_Contact() && Class->IsCarryall) || (Cargo.Is_Something_Attached() && Class->IsCarryall)) {
		return(100);
	}

	return(0);
}


/// <summary>
/// Fetches the facing that this aircraft should land at.
/// The aircraft lines itself up with whatever it is in radio contact with, since that is
/// the helipad or unit it has come to service. Failing that it holds its present heading
/// while loaded, or settles into the default parked pose.
/// </summary>
/// <returns>Returns with the facing to land at.</returns>
LONG STDMETHODCALLTYPE AircraftClass::Landing_Direction(void)
{
	TechnoClass * tptr = Contact_With_Whom();
	if (tptr != NULL) {
		return(tptr->PrimaryFacing.Current().As_Dir8());
	}
	if (Cargo.Is_Something_Attached()) {
		return(SecondaryFacing.Current().As_Dir8());
	}
	return(Rule->PoseDir);
}


/// <summary>
/// Is this aircraft carrying anything?
/// The locomotor asks this so that it can fly a loaded aircraft differently from an
/// empty one.
/// </summary>
/// <returns>Returns with true if there is cargo aboard this aircraft.</returns>
BOOL STDMETHODCALLTYPE AircraftClass::Is_Loaded(void)
{
	return(Cargo.Is_Something_Attached());
}


/// <summary>
/// Should this aircraft attack by strafing?
/// The locomotor asks this in order to choose between a strafing run and an attack made
/// from a hover. Only a visible and unguided projectile is suited to strafing.
/// </summary>
/// <returns>Returns with true if the aircraft should make strafing attack runs.</returns>
LONG STDMETHODCALLTYPE AircraftClass::Is_Strafe(void)
{
	const WeaponDataStruct * data = Get_Class_Weapon_Data(0);
	if (data == NULL) {
		return(false);
	}
	WeaponTypeClass *weapon = data->Weapon;
	if (weapon == NULL) {
		return(false);
	}
	if (weapon->Bullet->ROT > 1) {
		return(false);
	}
	if (weapon->Bullet->IsInvisible) {
		return(false);
	}
	return(true);
}


/// <summary>
/// Is this aircraft locked into straight and level flight?
/// The locomotor asks this so that it will not turn the aircraft while it is committed
/// to an attack run.
/// </summary>
/// <returns>Returns with true if the aircraft must hold its present heading.</returns>
LONG STDMETHODCALLTYPE AircraftClass::Is_Locked(void)
{
	return(IsLockedStraight);
}


/// <summary>
/// Starts this aircraft on its assigned mission.
/// The aircraft is released from any straight and level flight restriction before the
/// normal commence processing takes over.
/// </summary>
/// <returns>bool; Was the mission commenced?</returns>
bool AircraftClass::Commence(void)
{
	IsLockedStraight = false;
	return(BASECLASS::Commence());
}


/// <summary>
/// Should this aircraft be quietly removed once it leaves the map?
/// A loaner, or an aircraft belonging to a team that is on its way off the map, has no
/// further business in the scenario and is deleted rather than left to fly forever.
/// </summary>
/// <returns>bool; Should this aircraft be deleted when it flies off the map?</returns>
bool AircraftClass::Should_Delete_Off_Map(void)
{
	if (TarCom == NULL && IsLocked) {
		if ((Team == NULL && IsALoaner) || (Team != NULL && Team->Is_Leaving_Map())) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Spends a round of this aircraft's ammunition.
/// Aircraft do not lose ammunition through the normal firing sequence -- their supply is
/// accounted for by the mission code instead -- so this override does nothing at all.
/// </summary>
void AircraftClass::Reduce_Ammunition(void)
{
	//nothing
}


/// <summary>
/// Is this aircraft to be treated as a ground vehicle?
/// An aircraft sitting on the ground occupies its cell and is handled like any other
/// vehicle by the code that asks this question.
/// </summary>
/// <returns>bool; Should this aircraft be considered a vehicle?</returns>
bool AircraftClass::Considered_Vehicle(void) const
{
	return(On_Ground());
}


/// <summary>
/// Sends this aircraft plummeting out of the sky.
/// This routine is used when an airborne aircraft is destroyed. Rather than simply
/// vanishing, the aircraft is stunned and set rocking so that it tumbles to the ground.
/// The kill is credited, any attached trigger is sprung, and the cargo dies with it.
/// </summary>
/// <param name="source">The object responsible for the kill. This may be NULL.</param>
/// <returns>bool; Was the aircraft sent into a crash?</returns>
bool AircraftClass::Crash(TechnoClass * source)
{
	if (HeightAGL > 0) {
		if (Strength > 0) {
			if (source != NULL && Tag != NULL) {
				Tag->Spring(TEVENT_FIRST_DAMAGED, this);
			}
			if (Tag != NULL && IsActive) {
				Tag->Spring(TEVENT_FIRST_DAMAGED_ANY, this);
			}
			if (Tag != NULL) {
				if (IsActive && source != NULL) {
					Tag->Spring(TEVENT_FIRST_DAMAGED_ANY, this, CELL_NONE, false, source);
				}
			}
			if (!IsActive) {
				return(true);
			}
			Record_The_Kill(source);
			if (!IsActive) {
				return(true);
			}
			Strength = 0;
		}
		IsRocking = true;
		Transmit_Message(RADIO_OVER_OUT);
		Stun();
		Kill_Cargo(source);

		RockingSidewaysPerFrame = Random_Double(0.10, 0.25);
		if (Random_Pick(0, 1) == 0) {
			RockingSidewaysPerFrame = -RockingSidewaysPerFrame;
		}
		RockingForwardsPerFrame = Random_Double(0.0, 0.1);

		Detach_This_From_All(this, false);
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the run time type of this object.
/// </summary>
/// <returns>Returns with RTTI_AIRCRAFT.</returns>
RTTIType AircraftClass::Fetch_RTTI(void) const
{
	return(RTTI_AIRCRAFT);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence support. The save/load machinery uses the
/// class identifier to recreate an object of the correct type when a game is restored.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE AircraftClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_AircraftClass;
	return(S_OK);
}


/// <summary>
/// Fetches the descriptive name of this aircraft.
/// </summary>
/// <returns>Returns with a pointer to the name given to this aircraft's type.</returns>
char const * AircraftClass::Full_Name(void) const
{
	return(Class->GivenName);
}


/// <summary>
/// Fetches the direction that this aircraft's weapon is pointing.
/// An aircraft carries its weapon on the secondary facing, so this is what the targeting
/// and rendering code is told when it asks about the turret.
/// </summary>
/// <returns>Returns with the current turret facing.</returns>
DirType AircraftClass::Turret_Facing(void) const
{
	return(SecondaryFacing.Current());
}


/***********************************************************************************************
 * AircraftClass::Class_Of -- Fetches a reference to the class type for this object.           *
 *                                                                                             *
 *    This routine will fetch a reference to the TypeClass of this object.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with reference to the type class of this object.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectTypeClass const * AircraftClass::Class_Of(void) const
{
	return(Class);
}
