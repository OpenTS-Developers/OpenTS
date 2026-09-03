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

/* $Header: /CounterStrike/INFANTRY.CPP 2     3/03/97 10:35p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : INFANTRY.CPP                                                 *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : August 15, 1994                                              *
 *                                                                                             *
 *                  Last Update : October 28, 1996 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   InfantryClass::AI -- Handles the infantry non-graphic related AI processing.              *
 *   InfantryClass::Active_Click_With -- Handles action when clicking with infantry soldier.   *
 *   InfantryClass::Assign_Destination -- Gives the infantry a movement destination.           *
 *   InfantryClass::Assign_Target -- Gives the infantry a combat target.                       *
 *   InfantryClass::Can_Enter_Cell -- Determines if the infantry can enter the cell specified. *
 *   InfantryClass::Can_Fire -- Can the infantry fire its weapon?                              *
 *   InfantryClass::Class_Of -- Returns the class reference for this object.                   *
 *   InfantryClass::Clear_Occupy_Bit -- Clears occupy bit and given cell                       *
 *   InfantryClass::Debug_Dump -- Displays debug information about infantry unit.              *
 *   InfantryClass::Detach -- Removes the specified target from targeting computer.            *
 *   InfantryClass::Do_Action -- Launches the infantry into an animation sequence.             *
 *   InfantryClass::Doing_AI -- Handles the animation AI processing.                           *
 *   InfantryClass::Draw_It -- Draws a unit object.                                            *
 *   InfantryClass::Edge_Of_World_AI -- Detects when infantry has left the map.                *
 *   InfantryClass::Enter_Idle_Mode -- The infantry unit enters idle mode by this routine.     *
 *   InfantryClass::Fear_AI -- Process any fear related affects on this infantry.              *
 *   InfantryClass::Fire_At -- Fires projectile from infantry unit.                            *
 *   InfantryClass::Firing_AI -- Handles firing and combat AI for the infantry.                *
 *   InfantryClass::Full_Name -- Fetches the full name of the infantry unit.                   *
 *   InfantryClass::Get_Image_Data -- Fetches the image data for this infantry unit.           *
 *   InfantryClass::Greatest_Threat -- Determines greatest threat (target) for infantry unit.  *
 *   InfantryClass::InfantryClass -- The constructor for infantry objects.                     *
 *   InfantryClass::Init -- Initialize the infantry object system.                             *
 *   InfantryClass::Is_Ready_To_Random_Anima -- Checks to see if it is ready to perform an idle*
 *   InfantryClass::Limbo -- Performs cleanup operations needed when limboing.                 *
 *   InfantryClass::Mission_Attack -- Intercept attack mission for special handling.           *
 *   InfantryClass::Movement_AI -- This routine handles all infantry movement logic.           *
 *   InfantryClass::Overlap_List -- The list of cells that the infantry overlaps, but doesn't o*
 *   InfantryClass::Paradrop -- Handles paradropping infantry.                                 *
 *   InfantryClass::Per_Cell_Process -- Handles special operations that occur once per cell.   *
 *   InfantryClass::Random_Animate -- Randomly animate the infantry (maybe)                    *
 *   InfantryClass::Read_INI -- Reads units from scenario INI file.                            *
 *   InfantryClass::Response_Attack -- Plays infantry audio response to attack order.          *
 *   InfantryClass::Response_Move -- Plays infantry response to movement order.                *
 *   InfantryClass::Response_Select -- Plays infantry audio response due to being selected.    *
 *   InfantryClass::Scatter -- Causes the infantry to scatter to nearby cell.                  *
 *   InfantryClass::Set_Occupy_Bit -- Sets the occupy bit cell and bit pos                     *
 *   InfantryClass::Set_Primary_Facing -- Change infantry primary facing -- always and instantl*
 *   InfantryClass::Shape_Number -- Fetch the shape number for this infantry.                  *
 *   InfantryClass::Start_Driver -- Handles giving immediate destination and move orders.      *
 *   InfantryClass::Stop_Driver -- Stops the infantry from moving any further.                 *
 *   InfantryClass::Take_Damage -- Applies damage to the infantry unit.                        *
 *   InfantryClass::Unlimbo -- Unlimbo infantry unit in legal sub-location.                    *
 *   InfantryClass::What_Action -- Determines what action to perform for the cell specified.   *
 *   InfantryClass::What_Action -- Infantry units might be able to capture -- check.           *
 *   InfantryClass::Write_INI -- Store the infantry to the INI database.                       *
 *   InfantryClass::operator delete -- Returns the infantry object back to the free pool       *
 *   InfantryClass::operator new -- Allocates an infantry object from the free pool.           *
 *   InfantryClass::~InfantryClass -- Default destructor for infantry units.                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "infantry.h"

#include "_bench.h"
#include "_convert.h"
#include "_mixfile.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "aircraft.h"
#include "anim.h"
#include "astar.h"
#include "bench.h"
#include "building.h"
#include "builtype.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "data.h"
#include "draw.h"
#include "fly.h"
#include "fog.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "ilocos.h"
#include "incdec.h"
#include "infatype.h"
#include "inline.h"
#include "ion.h"
#include "isotype.h"
#include "language/language.h"
#include "lightcon.h"
#include "mixfile.h"
#include "mono.h"
#include "overlay.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "session.h"
#include "sun.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "tagtype.h"
#include "team.h"
#include "tiberium.h"
#include "tracker.h"
#include "tube.h"
#include "tube.hh"
#include "unit.h"
#include "vox.h"
#include "warhead.h"
#include "weapon.h"

#include "bench.hh"

#include <algorithm>


int const InfantryClass::HumanShape[32] = {7,7,6,6,6,6,5,5,5,5,4,4,4,4,3,3,3,3,2,2,2,2,1,1,1,1,0,0,0,0,7,7};

char const * const InfantryClass::INI_Name = "Infantry";


/***************************************************************************
**	This is the array of constant data associated with infantry maneuvers. It
**	specifies the frame rate as well as if the animation can be aborted.
*/
// interruptible, mobile, randomstart, rate
DoStruct const InfantryClass::MasterDoControls[DO_COUNT] = {
	{true,	false,	false,	0},	// DO_STAND_READY
	{true,	false,	false,	0},	// DO_STAND_GUARD
	{true,	false,	false,	6},	// DO_PRONE
	{true,	true,	true,	3},	// DO_WALK
	{true,	false,	false,	1},	// DO_FIRE_WEAPON
	{false,	true,	false,	1},	// DO_LIE_DOWN
	{true,	true,	true,	1},	// DO_CRAWL
	{false,	false,	false,	1},	// DO_GET_UP
	{true,	false,	false,	1},	// DO_FIRE_PRONE
	{true,	false,	false,	1},	// DO_IDLE1
	{true,	false,	false,	1},	// DO_IDLE2
	{false,	false,	false,	1},	// DO_GUN_DEATH
	{false,	false,	false,	1},	// DO_EXPLOSION_DEATH
	{false,	false,	false,	1},	// DO_EXPLOSION2_DEATH
	{false,	false,	false,	1},	// DO_GRENADE_DEATH
	{false,	false,	false,	1},	// DO_FIRE_DEATH
	{true,	true,	false,	1},	/// DO_HOVER
	{true,	true,	false,	1},	/// DO_FLY
	{true,	true,	false,	1},	/// DO_TUMBLE
	{true,	true,	false,	1},	/// DO_FIREFLY
	{false,	false,	true,	3},	/// DO_STRUGGLE
};


#ifdef _DEBUG
/***********************************************************************************************
 * InfantryClass::Debug_Dump -- Displays debug information about infantry unit.                *
 *                                                                                             *
 *    This routine is used by the debug version to display pertinent information about the     *
 *    infantry unit.                                                                           *
 *                                                                                             *
 * INPUT:   mono  -- The monochrome screen to display the debug information to.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Debug_Dump(MonoClass * mono) const
{
	mono->Set_Cursor(0, 0);

	mono->Set_Cursor(1, 11);mono->Printf("%3d", Doing);
	mono->Set_Cursor(8, 11);mono->Printf("%3d", Fear);

	mono->Fill_Attrib(66, 13, 12, 1, IsTechnician ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(66, 14, 12, 1, IsStoked ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(66, 15, 12, 1, IsProne ? MonoClass::INVERSE : MonoClass::NORMAL);

	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * InfantryClass::InfantryClass -- The constructor for infantry objects.                       *
 *                                                                                             *
 *    This is the constructor used when creating an infantry unit. All values are required     *
 *    except for facing and position. If these are absent, then the infantry is created in     *
 *    a state of limbo -- not placed upon the map.                                             *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryClass::InfantryClass(InfantryTypeClass const * type, HouseClass * house) :
	BASECLASS(house),
	Class((InfantryTypeClass *)type),
	Doing(DO_NOTHING),
	Comment(0),
	ProneStruggleTimer(0),
	LookTimer(0),
	IsTechnician(false),
	IsStoked(false),
	IsProne(false),
	IsBerzerk(false),
	IsZoneCheat(false),
	WasSelected(false),
	Fear(FEAR_NONE)
{
	Create_ID();
	Infantry.Add(this);

	Init();

	if (Class != NULL) {
		Locomotion.CreateInstance(Class->Locomotor, NULL, CLSCTX_ALL);
		Locomotion->Link_To_Object(this);
	}

	PrimaryFacing.Set_ROT(127);
	TargetTracker.Add_Index(Fetch_ID(), this);
}


/***********************************************************************************************
 * InfantryClass::Init -- Initialize the infantry object system.                               *
 *                                                                                             *
 *    This routine will force the infantry object system into its empty initial state. It      *
 *    is called when the scenario needs to be cleared in preparation for a scenario load.      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Init(void)
{
	BASECLASS::Init();

	if (House != NULL) {
		House->Tracking_Add(this);
	}

	if (Class != NULL) {

		Strength = Class->MaxStrength;

		/*
		**	Civilians carry much less ammo than soldiers do.
		*/
		Ammo = Class->MaxAmmo;

		IsCloakable = Class->IsCloakable;
	}
}


/***********************************************************************************************
 * InfantryClass::~InfantryClass -- Default destructor for infantry units.                     *
 *                                                                                             *
 *    This is the default destructor for infantry type units. It will put the infantry into    *
 *    a limbo state if it isn't already in that state and the game is still active.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/10/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
InfantryClass::~InfantryClass(void)
{
	if (GameActive && Class != NULL) {

		if (House->Can_Build(Class, false, false) == -1) {
			House->IsRecalcNeeded = true;
		}

		/*
		**	Remove this member from any team it may be associated with. This must occur at the
		**	top most level of the inheritance hierarchy because it may call virtual functions.
		*/
		if (Team != NULL) {
			Team->Remove(this);
			Team = NULL;
		}

		House->Tracking_Remove(this);
		Limbo();
	}
	Detach_This_From_All(this);
	Infantry.Delete(this);
	TargetTracker.Remove_Index(Fetch_ID());
	IsActive = false;
}


/***********************************************************************************************
 * InfantryClass::Take_Damage -- Applies damage to the infantry unit.                          *
 *                                                                                             *
 *    This routine applies the damage specified to the infantry object. It is possible that    *
 *    this routine will DESTROY the infantry unit in the process.                              *
 *                                                                                             *
 * INPUT:   damage   -- The damage points to inflict.                                          *
 *                                                                                             *
 *          distance -- The distance from the damage center point to the object's center point.*
 *                                                                                             *
 *          warhead  -- The warhead type that is inflicting the damage.                        *
 *                                                                                             *
 *          source   -- Who is responsible for inflicting the damage.                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the infantry unit destroyed by this damage?                              *
 *                                                                                             *
 * WARNINGS:   Since the infantry unit could be destroyed by this routine, be sure to check    *
 *             for this in the code that follows the call to Take_Damage().                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *   11/22/1994 JLB : Shares base damage handler for techno objects.                           *
 *   03/31/1995 JLB : Revenge factor.                                                          *
 *=============================================================================================*/
ResultType InfantryClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	ResultType res = RESULT_NONE;

	/*
	**	Prone infantry take only half damage, but never below one damage point.
	*/
	if (warhead != NULL && IsProne && damage > 0 && !forced) {
		damage = (int)(damage * warhead->ProneDamage);

		if (damage < 1) {
			damage = 1;
		}
	}

	/*
	 * If the warhead is webby and we're not web immune, paralyze us.
	 */
	if (warhead != NULL && warhead->IsWebby && !Class->IsWebImmune) {
		damage = 0;
		int variation = Random_Pick(-warhead->WebDurationVariation, warhead->WebDurationVariation);
		int duration = warhead->WebDuration + variation;
		ProneStruggleTimer = std::max((int)ProneStruggleTimer, duration);

		Do_Action(DO_STRUGGLE, true, true);

		if (Tag != NULL) {
			Tag->Spring(TEVENT_PARALYZED, this, CELL_NONE, false, source);
		}
	}

	res = BASECLASS::Take_Damage(damage, distance, warhead, source, forced, no_crew);

	if (res == RESULT_ALREADY_DESTROYED) return(res);

	/*
	** hack for dog: if you're hit by a dog, and you're the target, your
	** damage gets upped to max.
	*/

	if (res == RESULT_NONE) return(res);

	if (res == RESULT_DESTROYED) {
		Death_Announcement(source);
		Stop_Driver();
		Stun();
		Mission = MISSION_NONE;
		Assign_Mission(MISSION_GUARD);
		Commence();
		Kill_Cargo(source);

		bool delthis = false;
		if (forced && Class->IsCyborg) {
			delthis = true;
			if (IsToExplode) {
				new AnimClass(Rule->InfantryExplode, PositionCoord);
			}
		}

		if (HeightAGL <= 10 && Map[(Coord const &)PositionCoord].Land_Type() == LAND_WATER && IsToExplode) {
			new AnimClass(Rule->Wake, PositionCoord);
			new AnimClass(Rule->SplashList[0], PositionCoord + Coord(0,0,3));
			delthis = true;
		} else if (Class->IsCyborg && IsProne) {
			new AnimClass(Rule->InfantryExplode, PositionCoord);
			delthis = true;
		} else if (Class->IsJumpJet) {
			new AnimClass(Rule->InfantryExplode, PositionCoord);
			delthis = true;
		} else {

			/*
			**	The type of warhead determines the animation the infantry
			**	will perform when killed.
			*/
			int infdeath = warhead != NULL ? warhead->InfantryDeath : 0;
			if (source != NULL && source->RTTI == RTTI_BUILDING && reinterpret_cast<BuildingClass*>(source)->Class->IsLaserFence) {
				infdeath = 5;
			}

			switch (infdeath) {
				default:
				case 0:
					delthis = true;
					break;

				case 1:
					Do_Action(DO_GUN_DEATH, true);
					break;

				case 2:
					Do_Action(DO_EXPLOSION_DEATH, true);
					break;

				case 3:
					new AnimClass(Rule->InfantryExplode, PositionCoord);
					delthis = true;
					break;

				case 4:
					if (Class->IsDoggie) {
						Do_Action(DO_FIRE_DEATH, true);
					} else {
						AnimClass * anim = new AnimClass(Rule->FlamingInfantry, PositionCoord);
						anim->AlternativeDrawer = ColorSchemes[PlayerPtr->Scheme]->Converter;
						delthis = true;
					}
					break;

				case 5:
					if (Class->IsDoggie) {
						Do_Action(DO_FIRE_DEATH, true);
					} else {
						AnimClass * anim = new AnimClass(AnimTypes[ANIM_ELECT_DIE], PositionCoord);
						delthis = true;
					}
					break;
			}
		}

		if (delthis) {
			Delete_Me();
		}
		return(res);
	}

	/*
	**	When infantry gets hit, it gets scared.
	*/
	if (res != RESULT_DESTROYED) {
		Coord source_coord = (source) ? source->PositionCoord : COORD_NONE;

		/*
		**	If an engineer is damaged and it is just sitting there, then tell it
		**	to go do something since it will definitely die if it doesn't.
		*/
		if (!House->Is_Human_Player() && Class->IsEngineer && (Mission == MISSION_GUARD || Mission == MISSION_GUARD_AREA)) {
			Assign_Mission(MISSION_HUNT);
		}

		if (source != NULL) {
			Scatter(source_coord);
		}

		if (Rule->IsBerzerkAllowed && Class->IsCyborg && !IsBerzerk && res == RESULT_HALF) {
			IsBerzerk = true;
			Assign_Mission(MISSION_GUARD_AREA);
		}

		if (source != NULL && Fear < FEAR_SCARED) {
			if (Class->IsFraidyCat) {
				Fear = FEAR_PANIC;
			} else if (!Class->IsFearless && !Has_Ability(ABILITY_FEARLESS)) {
				Fear = FEAR_SCARED;
				if (Class->IsDoggie && HealthRatio <= Rule->ConditionRed) {
					Fear = FEAR_PANIC;
				}
			}
		} else {

			if (!Class->IsFearless && !Has_Ability(ABILITY_FEARLESS)) {
				/*
				**	Increase the fear of the infantry by a bit. The fear increases more
				**	quickly if the infantry is damaged.
				*/
				int morefear = FEAR_ANXIOUS;
				if (HealthRatio > Rule->ConditionRed) morefear /= 2;
				if (HealthRatio > Rule->ConditionYellow) morefear /= 2;
				Fear = FearType(std::min<int>((int)Fear + morefear, FEAR_MAXIMUM));
			}
		}
	}

	return(res);
}


/***********************************************************************************************
 * InfantryClass::Shape_Number -- Fetch the shape number for this infantry.                    *
 *                                                                                             *
 *    This will determine the shape number to use for this infantry soldier. The shape number  *
 *    is relative to the shape file associated with this infantry unit.                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the shape number for this infantry object to be used when drawing.    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int InfantryClass::Shape_Number(void) const
{
	/*
	**	Fetch the shape pointer to use for the infantry. This is controlled by what
	**	choreograph sequence the infantry is performing, it's facing, and whether it
	**	is prone.
	*/
	DoType doit = Doing;
	if (doit == DO_NOTHING) doit = DO_STAND_READY;

	/*
	**	The infantry shape is always modulo the number of animation frames
	**	of the action stage that the infantry is doing.
	*/
	int shapenum = Fetch_Stage() % std::max(Class->DoControls[doit].Count, 1);

	if (Is_JumpJet() && TarCom != NULL) {

		/*
		**	If facing makes a difference, then the shape number will be incremented
		**	by the facing accordingly.
		*/
		if (Class->DoControls[doit].Jump > 0) {
			shapenum += HumanShape[Direction(TarCom).As_Dir32()] * Class->DoControls[doit].Jump;
		}

	} else {

		/*
		**	If facing makes a difference, then the shape number will be incremented
		**	by the facing accordingly.
		*/
		if (Class->DoControls[doit].Jump > 0) {
			shapenum += HumanShape[DirType(PrimaryFacing.Current()).As_Dir32()] * Class->DoControls[doit].Jump;
		}
	}

	/*
	**	Finally, the shape number is biased according to the starting frame number for
	**	that action in the infantry shape file.
	*/
	shapenum += Class->DoControls[doit].Frame;

	/*
	**	Return with the final infantry shape number.
	*/
	return(shapenum);
}


/***********************************************************************************************
 * InfantryClass::Draw_It -- Draws a unit object.                                              *
 *                                                                                             *
 *    This routine is the one that actually draws a unit object. It displays the unit          *
 *    according to its current state flags and centered at the location specified.             *
 *                                                                                             *
 * INPUT:   x,y   -- The X and Y coordinate of where to draw the unit.                         *
 *                                                                                             *
 *          window   -- The clipping window to use.                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/20/1994 JLB : Created.                                                                 *
 *   06/27/1994 JLB : Takes a window parameter.                                                *
 *   08/15/1994 JLB : Converted to infantry support.                                           *
 *   08/14/1996 JLB : Simplified.                                                              *
 *=============================================================================================*/
void InfantryClass::Draw_It(Point2D const & xpoint, Rect const & cliprect) const
{
	static int _zadj = -10;

	Point2D point = xpoint;
	Cell cell = Get_Target_Cell();

	if (CurrentTube == -1) {
		IPersistPtr persist = Locomotion;
		CLSID clsid;
		persist->GetClassID(&clsid);

		if (HeightAGL > 0 && clsid == CLSID_BallisticLocomotion) {
			ShapeSet const * shapefile = (ShapeSet const *)MFCD::Retrieve("POD.SHP");
			Point2D spoint = xpoint + Point2D(Locomotion->Shadow_Point());
			Draw_Shape(
				*LogicalSurface,
				*NormalDrawer,
				shapefile,
				Locomotion->Drawing_Code(),
				spoint,
				cliprect,
				ShapeFlags_Type(SHAPE_ZGRAD|SHAPE_ALPHA|SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_DARKEN),
				NULL,
				Get_Z_Adjust() - 2
			);

			Map[cell];
			Map[cell];

			Draw_Object(shapefile, Locomotion->Drawing_Code(), xpoint, cliprect, DIR_N, 256, 0, ZGRAD_90DEG, false, NORMAL_LIGHT);
		} else {

			/*
			**	Verify the legality of the unit class by seeing if there is shape imagery for it. If
			**	there is no shape image, then it certainly can't be drawn -- bail.
			*/
			ShapeSet const * shapefile = (ShapeSet const *)Get_Image_Data();

			if (shapefile == NULL) return;

			Cell tcell = Get_Target_Cell();

			TacticalMap->Add_To_Selectables((ObjectClass *)this, point);

			int brightness;
			if (IsOnBridge || (Map[(Coord const &)PositionCoord].IsOvershadowed && PositionCoord.Z > Map.Get_Height_GL(PositionCoord) + (BRIDGE_LEPTON_HEIGHT / 2))) {
				int light = (IonStormClass::Is_Ion_Storm_Active() ? Scen->IonLevelLight : Scen->LevelLight) * (HeightAGL / (2 * LEVEL_LEPTON_H));
				brightness = Map[tcell].Brightness + light;
			} else {
				brightness = Map[tcell].Brightness + (Map[(Coord const &)PositionCoord].IsOvershadowed ? -500 : 0);
			}
			brightness += Rule->ExtraInfantryLight;

			int height = HeightAGL;
			CellClass *cellptr = &Map[(Coord const &)PositionCoord];

			if (cellptr->IsUnderBridge && height >= BRIDGE_LEPTON_HEIGHT) {
				if ((cellptr->IsBridgeEastWest && cellptr->Adjacent_Cell(FACING_N).IsUnderBridge) ||
					(!cellptr->IsBridgeEastWest && cellptr->Adjacent_Cell(FACING_W).IsUnderBridge)) {
					height -= BRIDGE_LEPTON_HEIGHT;
				}
			}

			if (height > 0) {
				Draw_Shape(
					*LogicalSurface,
					*NormalDrawer,
					shapefile,
					Shape_Number(),
					point + Point2D(0, -IonBlastYDrawOffset) + Point2D(0, TacticalMap->Z_Lepton_To_Pixel(height)),
					cliprect,
					ShapeFlags_Type(SHAPE_ZGRAD|SHAPE_WIN_REL|SHAPE_CENTER|SHAPE_DARKEN),
					NULL,
					-5 - TacticalMap->Z_Lepton_To_Pixel(PositionCoord.Z - height)
				);
			}

			Draw_Object(shapefile, Shape_Number(), point + Point2D(0, -IonBlastYDrawOffset), cliprect, DIR_N, 256, _zadj, ZGRAD_90DEG, false, brightness);
		}
	} else {
		TacticalMap->Add_To_Selectables((ObjectClass *)this, point);
	}

	BASECLASS::Draw_It(point, cliprect);
}


/***********************************************************************************************
 * InfantryClass::Per_Cell_Process -- Handles special operations that occur once per cell.     *
 *                                                                                             *
 *    This routine will handle any special operations that need to be performed once each      *
 *    cell travelled. This includes radioing a transport that it is now clear and the          *
 *    transport is free to leave.                                                              *
 *                                                                                             *
 * INPUT:   why   -- Specifies the circumstances under which this routine was called.          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *   03/01/1995 JLB : Capture building options.                                                *
 *   05/31/1995 JLB : Capture is always successful now.                                        *
 *=============================================================================================*/
void InfantryClass::Per_Cell_Process(PCPType why)
{
	BStart(BENCH_PCP);
	CellClass * cellptr = &Map[Get_Coord()];

	if (why == PCP_END) {

		/*
		**	If the infantry unit is entering a cell that contains the building it is trying to
		**	capture, then capture it.
		*/
		if (Mission == MISSION_CAPTURE || Mission == MISSION_GUARD_AREA || Mission == MISSION_PATROL) {
			if (NavCom != NULL && NavCom->Is_Techno() && ((TechnoClass*)NavCom)->Considered_Vehicle()) {
				TechnoClass * tech = (TechnoClass *)NavCom;
				if (tech != NULL) {
					if (PositionCell == tech->PositionCell) {
						if (tech->Tag) {
							tech->Tag->Spring(TEVENT_PLAYER_ENTERED, this);
						}
						tech->Detach_All(false);
						tech->Captured(House);
						tech->EnteredByInfType = Class->HeapID;
						tech->Scatter_Incoming_Infantry();
						if (Tag && Tag->Is_To_Inherit()) {
							tech->Attach_Tag(Tag);
						}
						if (Tag) {
							Tag->Spring(TEVENT_DESTROYED_ANY, this);
						}
						Delete_Me();
						BEnd(BENCH_PCP);
						return;
					} else {
						Coord destination = Locomotion->Destination();
						if (destination == COORD_NONE || PositionCell == Cell(destination)) {
							Assign_Destination(NavCom);
							BEnd(BENCH_PCP);
							return;
						}
					}
				}
			}

			TechnoClass * tech = cellptr->Cell_Building();

			if (tech != NULL && (tech == NavCom || tech == TarCom)) {
				if (tech->Tag) {
					tech->Tag->Spring(TEVENT_PLAYER_ENTERED, this);
				}

				if (Class->IsEngineer) {
					// are we trying to repair a bridge?
					if (tech->RTTI == RTTI_BUILDING && ((BuildingClass*)tech)->Class->IsBridgeRepairHut) {
						if (House->Is_Player_Control()) Speak(VOX_BRIDGE_REPAIRED);

						bool train = false;
						for (int y = -2; y < 3; y++) {
							for (int x = -2; x < 3; x++) {
								Cell cell = (Cell)Cell(PositionCell + Cell(x, y));
								IsometricTileType ittype = Map[cell].ITType;
								if (ittype >= IsometricTileTypeClass::TrainBridgeSet && ittype < IsometricTileTypeClass::TrainBridgeSet + TRAIN_BRIDGE_COUNT) {
									train = true;
								}
							}
						}
						if (train) {
							Map.Repair_Train_Bridge(PositionCell);
						} else {
							Map.Repair_Bridge(PositionCell);
						}
						for (int i = Infantry.Count() - 1; i >= 0; i--) {
							Infantry[i]->Detach(tech, false);
						}
						tech->Scatter_Incoming_Infantry();
					}

					/*
					**	An engineer will either mega-repair a friendly or allied
					**	building or it will damage/capture an enemy building. Whether
					**	it damages or captures depends on how badly damaged the
					**	enemy building is.
					*/
					else if (House->Is_Ally(tech)) {
						tech->Renovate();
					} else {
						bool iscapturable = false;
						if (tech->RTTI == RTTI_BUILDING) {
							iscapturable = ((BuildingClass *)tech)->Class->IsCaptureable;
						}

						if (Session.Type != GAME_NORMAL && Session.Options.CrapEngineers && tech->HealthRatio > Rule->ConditionRed) {
							int maxdamage = tech->Strength - int(tech->TClass->MaxStrength * Rule->ConditionRed / 2);
							int damage = std::min<double>((tech->TClass->MaxStrength) * ((1 - Rule->ConditionRed / 2) / 2), maxdamage);
							tech->Take_Damage(damage, 0, Rule->C4Warhead, this, true);
						} else if (iscapturable) {
							if (tech->Tag) {
								tech->Tag->Spring(TEVENT_PLAYER_ENTERED, this);
							}
							tech->House->IsThieved = true;
							if (Tag && Tag->Is_To_Inherit()) {
								tech->Attach_Tag(Tag);
							}
							tech->Captured(House);
							tech->EnteredByInfType = Class->HeapID;
							tech->Scatter_Incoming_Infantry();
						}
					}
				} else {
					if (Class->IsAgent) {
						if (House->Is_Player_Control()) Speak(VOX_BUILDING_INFILTRATED);
						((BuildingClass *)tech)->Spied_By(House);
					}
				}
				if (Tag) {
					Tag->Spring(TEVENT_DESTROYED_ANY, this);
				}
				Delete_Me();
				BEnd(BENCH_PCP);
				return;

			} else {
				if (NavCom == NULL) {
					if (CurrentMission != MISSION_GUARD_AREA) {
						Enter_Idle_Mode();
					}
					if (Map[Get_Coord()].Cell_Building()) {
						Scatter(COORD_NONE, true);
					}
				}
			}
		}

		/*
		**	Infantry entering a transport vehicle will break radio contact
		**	at attach itself to the transporter.
		*/
		TechnoClass * techno;
		if (Class->IsVehicleThief) {
			techno = Get_Cell_Ptr()->Cell_Unit(IsOnBridge);
			if (techno == NULL) techno = Get_Cell_Ptr()->Cell_Aircraft(IsOnBridge);
			if (techno == NULL) techno = Get_Cell_Ptr()->Cell_Building();
		} else {
			techno = Get_Cell_Ptr()->Cell_Building();
			if (techno == NULL) techno = Get_Cell_Ptr()->Cell_Aircraft(IsOnBridge);
			if (techno == NULL) techno = Get_Cell_Ptr()->Cell_Unit(IsOnBridge);
		}

		TechnoClass * on_target_cell = NULL;
		if (NavCom != NULL && NavCom->RTTI == RTTI_CELL) {
			CellClass * cellptr = &Map[NavCom->Center_Coord()];
			if (Class->IsVehicleThief) {
				on_target_cell = cellptr->Cell_Unit(true);
				if (on_target_cell == NULL) on_target_cell = cellptr->Cell_Aircraft(true);
				if (on_target_cell == NULL) on_target_cell = cellptr->Cell_Building();
			} else {
				on_target_cell = cellptr->Cell_Building();
				if (on_target_cell == NULL) on_target_cell = cellptr->Cell_Aircraft(true);
				if (on_target_cell == NULL) on_target_cell = cellptr->Cell_Unit(true);
			}
		}

		if (Mission == MISSION_ENTER && techno != NULL && (NavCom == techno || TarCom == techno || on_target_cell != NULL && techno == on_target_cell)) {
			if (techno->Tag) {
				techno->Tag->Spring(TEVENT_PLAYER_ENTERED, this);
			}
			if (techno->RTTI == RTTI_BUILDING) {
				if (techno == Get_Cell_Ptr()->Cell_Building() && Transmit_Message(RADIO_IM_IN) == RADIO_ROGER) {
					Limbo();
					Transmit_Message(RADIO_HELLO, techno);
					techno->Cargo.Attach(this);
					NavCom = NULL;
					BEnd(BENCH_PCP);
					return;
				}
			} else {
				if (Get_Cell() == techno->Get_Cell() && techno == NavCom) {
					if (Transmit_Message(RADIO_CAN_LOAD, techno) == RADIO_ROGER) {
						ArchiveTarget = NULL;
						Limbo();
						techno->Cargo.Attach(this);
						Hidden();
					} else {
						Assign_Destination(NULL);
						Assign_Mission(MISSION_GUARD);
						Scatter(COORD_NONE, true, true);
					}
					BEnd(BENCH_PCP);
					return;
				}
			}
		}

		/*
		**	If the infantry unit is entering a cell that contains the building it is trying to
		**	sabotage, then sabotage it.
		*/
		if (Mission == MISSION_SABOTAGE) {
			BuildingClass * building = cellptr->Cell_Building();
			if (building != NULL && building == NavCom) {
				if (!building->Class->IsRepairable) {
					Assign_Target(NULL);
					Assign_Destination(NULL);
					Enter_Idle_Mode();
					BEnd(BENCH_PCP);
					return;
				}
				if (building->Tag) {
					building->Tag->Spring(TEVENT_PLAYER_ENTERED, this);
				}
				if (building->Mission != MISSION_DECONSTRUCTION) {
					building->IsGoingToBlow = true;
					building->Clicked_As_Target((Rule->C4Delay * TICKS_PER_MINUTE) / 2);
					building->CountDown = Rule->C4Delay * TICKS_PER_MINUTE;
					building->WhomToRepay = this;
				}
				NavCom = NULL;
				Do_Uncloak();
				Arm = Rearm_Delay(true);
				Scatter(building->Center_Coord(), true, true);	// RUN AWAY!
				BEnd(BENCH_PCP);
				return;
			} else {
				if (&Map[Center_Coord()] == NavCom) {
					Explosion_Damage(PositionCoord, Rule->BridgeStrength, this, Rule->C4Warhead);

					Stop_Driver();
					Scatter(Adjacent_Cell(PositionCoord, (FacingType)PrimaryFacing.Current().As_Dir8()), true, true);
					Assign_Mission(MISSION_MOVE);

					if (NavCom == NULL || Map[NavCom->Center_Coord()].Land_Type() == LAND_WATER) {
						Mark(MARK_DOWN);		// Needed only so that Tanya will get destroyed by the explosion.
					}
					Explosion_Damage(PositionCoord, Rule->BridgeStrength, NULL, Rule->C4Warhead);
					Explosion_Damage(PositionCoord, Rule->BridgeStrength, NULL, Rule->C4Warhead);
					if (!IsActive) {
						BEnd(BENCH_PCP);
						return;
					}

					Mark(MARK_DOWN);
				}
			}
		}

		/*
		**	If this unit is on a teather, then cut it at this time so that
		**	the "parent" unit is free to proceed. Note that the parent
		**	unit might actually be a building.
		*/
		if (IsTethered) {
			Transmit_Message(RADIO_UNLOADED);

			/*
			**	Special voice play.
			*/
			if (Class->VoiceComment.Count() > 1) {
				Sound_Effect((VocType)Class->VoiceComment[1], PositionCoord);
			}

			/*
			**	If the cell is now full of infantry, tell them all to scatter
			**	in order to make room for more.
			*/
			if (IsOnBridge) {
				if ((cellptr->BridgeFlag.Composite & 0x01C) == 0x01C) {
					cellptr->Incoming(COORD_NONE, true, true, true);
				}
			} else {
				if ((cellptr->Flag.Composite & 0x01C) == 0x01C) {
					cellptr->Incoming(COORD_NONE, true, true, false);
				}
			}
		}

		/*
		**	When the infantry reaches the center of the cell, it may begin a new mission.
		*/
		if (MissionQueue == MISSION_NONE && NavCom == NULL && TarCom == NULL && !In_Radio_Contact()) {
			Enter_Idle_Mode();
		}
		Commence();

		if (IsPlanningToLook) {
			IsPlanningToLook = false;
			Look(false);
		} else {
			Look(true);
		}

		/*
		**	If after all is said and done, the unit finishes its move on an impassable cell, then
		**	it must presume that it is in the case of a unit driving onto a bridge that blows up
		**	before the unit completes it's move. In such a case the unit should have been destroyed
		**	anyway, so blow it up now.
		*/
		CellClass * cellptr = &Map[Get_Coord()];
		LandType land = cellptr->Land_Type();
		if (!Locomotion->Is_Moving() && !Class->IsBomber && !Has_Ability(ABILITY_C4) && (land == LAND_ROCK || land == LAND_WATER) && (!IsOnBridge || !cellptr->IsUnderBridge)) {
			int damage = Strength;
			Take_Damage(damage, 0, Rule->C4Warhead, NULL, true);
			BEnd(BENCH_PCP);
			return;
		}

		if (!Class->IsTiberiumProof && !Has_Ability(ABILITY_TIBERIUM_PROOF) && Strength > 0) {
			TiberiumType tiberium = Map[Get_Coord()].Tiberium_Type_Here();
			if (tiberium != TIBERIUM_NONE) {
				int damage = Tiberiums[tiberium]->Power / 10;
				if (damage <= 1) damage = 1;
				Coord coord = Get_Coord();
				if (Take_Damage(damage, 0, Rule->C4Warhead, NULL, true) == RESULT_DESTROYED) {
					if (Scen->IsTiberiumDeathToVisceroid) {
						UnitClass * visceroid = new UnitClass(Rule->SmallVisceroid, House_From_HousesType(HouseTypeClass::From_Name("Neutral")));
						if (visceroid) {
							Coord cell_coord = Map[coord].Cell_Coord();
							if ((Map[cell_coord].Flag.Composite & 0x020) == 0) {
								ScenarioInit++;
								visceroid->Unlimbo(coord);
								ScenarioInit--;
							}
						}
					}
				}
			}
		}
	}

	if (IsActive) {
		BASECLASS::Per_Cell_Process(why);
	}
	BEnd(BENCH_PCP);
}


/***********************************************************************************************
 * InfantryClass::Detach -- Removes the specified target from targeting computer.              *
 *                                                                                             *
 *    This is a support routine that removes the target specified from any targeting or        *
 *    navigation computers. When a target is destroyed or removed from the game system,        *
 *    the target must be removed from any tracking systems of the other units. This routine    *
 *    handles removal for infantry units.                                                      *
 *                                                                                             *
 * INPUT:   target   -- The target to remove from the infantry unit's tracking systems.        *
 *                                                                                             *
 *          all      -- Is the target going away for good as opposed to just cloaking/hiding?  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Detach(AbstractClass const * target, bool all)
{
	BASECLASS::Detach(target, all);
	if (Class == target) {
		Class = NULL;
	}
}


/***********************************************************************************************
 * InfantryClass::Assign_Destination -- Gives the infantry a movement destination.             *
 *                                                                                             *
 *    This routine updates the infantry's navigation computer so that the infantry will        *
 *    travel to the destination target specified.                                              *
 *                                                                                             *
 * INPUT:   target   -- The target to have the infantry unit move to.                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Assign_Destination(AbstractClass * target, bool immediate)
{
	/*
	**	Special flag so that infantry will start heading in the right direction immediately.
	*/
	if (Locomotion->Is_Moving() && target != NULL && Map[Center_Coord()].Is_Clear_To_Move(Class->Speed, true, false)) {
		if (Class->IsJumpJet || CurrentMission != MISSION_ATTACK || NavCom != target) {
			Stop_Driver();
		}
	}

	/*
	**	When telling an infantry soldier to move to a location twice, then this
	**	means that movement is more important than safety. Get up and run!
	*/
	if (House->Is_Human_Player() && target != NULL && NavCom == target && IsProne && !Class->IsFraidyCat && !Class->IsCyborg) {
		Do_Action(DO_GET_UP);
	} else if (Class->IsDoggie && IsProne && target != NULL) {
		Do_Action(DO_GET_UP);
	}

	/*
	**	Handle entry logic here.
	*/
	if (Mission == MISSION_ENTER || MissionQueue == MISSION_ENTER) {

		/*
		**	If not already in radio contact (presumed with the transport), then
		**	either try to establish contact if allowed, or just move close and
		**	wait until radio contact can be established.
		*/
		if (!In_Radio_Contact()) {
			TechnoClass * techno = Dynamic_Cast<TechnoClass *>(target);
			if (techno != NULL) {

				/*
				**	Determine if the transport is already in radio contact. If so, then just move
				**	toward the transport and try to establish contact at a later time.
				*/
				if (techno->In_Radio_Contact()) {
// TCTCTC -- call for an update from the transport to get a good rendezvous position.

					if (techno->RTTI == RTTI_BUILDING) {
						ArchiveTarget = NULL;
						target = NULL;
					} else {
						ArchiveTarget = target;
					}
				} else {
					if (Transmit_Message(RADIO_HELLO, techno) == RADIO_ROGER) {
						if (Transmit_Message(RADIO_DOCKING) != RADIO_ROGER) {
							Transmit_Message(RADIO_OVER_OUT);
						} //else {
							//BG: keep retransmitted navcom from radio-move-here.
							//return;
						//}
					}
				}
			}
		} else {
			Path[0] = FACING_NONE;
		}
	} else {
		Path[0] = FACING_NONE;
	}

	if (target != NULL && Class->IsJumpJet && Locomotion->Is_Moving()) {
		IPersistPtr persist(Locomotion);
		CLSID clsid;
		persist->GetClassID(&clsid);
		if (clsid == CLSID_WalkLocomotion) {
			NavQueue.Add_Head(target);
			target = Get_Target_Cell_Ptr();
			if (target != NULL && ((CellClass *)target)->IsUnderBridge) {
				target = NULL;
			}
		}
	}

	if (Class->IsJumpJet && target != NULL && !Locomotion->Is_Moving()) {
		bool should_fly = Should_JumpJet_Fly(Destination_Coord().As_Cell(), target->Center_Coord().As_Cell());
		if (Is_JumpJet()) {
			if (!should_fly) {
				IPiggybackPtr piggy(Locomotion);
				if (piggy != NULL) {
					if (piggy->Is_Piggybacking() && piggy->Is_Ok_To_End()) {
						piggy->End_Piggyback(&Locomotion);
					}
				}
				ILocomotionPtr walk(CLSID_WalkLocomotion);
				walk->Link_To_Object(this);
				piggy = IPiggybackPtr(walk);
				if (piggy != NULL) {
					piggy->Begin_Piggyback(Locomotion);
					Locomotion = walk;
				}
			}
		} else {
			if (should_fly) {
				IPiggybackPtr piggy(Locomotion);
				if (piggy != NULL) {
					if (piggy->Is_Piggybacking() && piggy->Is_Ok_To_End()) {
						piggy->End_Piggyback(&Locomotion);
					}
				}
			}
		}
	}

	BASECLASS::Assign_Destination(target, immediate);
}


/***********************************************************************************************
 * InfantryClass::Assign_Target -- Gives the infantry a combat target.                         *
 *                                                                                             *
 *    This routine will update the infantry's targeting computer so that it will try to        *
 *    attack the target specified. This might result in it moving to be within range and thus  *
 *    also cause adjustment of the navigation computer.                                        *
 *                                                                                             *
 * INPUT:   target   -- The target that this infantry should attack.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *   06/30/1995 JLB : Tries to capture target if possible.                                     *
 *=============================================================================================*/
void InfantryClass::Assign_Target(AbstractClass * target)
{
	if (target != TarCom && Strength > 0) {
		IsFiring = false;
		if (IsProne) {
			Do_Action(DO_PRONE);
		} else {
			Do_Action(DO_STAND_READY);
		}
	}

	Path[0] = FACING_NONE;
	BASECLASS::Assign_Target(target);

	/*
	**	If this is an infantry that can only capture, then also assign its destination to the
	**	target specified.
	*/
	if (NavCom == NULL && Class->IsCapture && !Is_Weapon_Equipped()) {
		if (target != NULL && target->RTTI == RTTI_BUILDING && ((BuildingClass *)target)->Class->IsCaptureable) {
			Assign_Destination(target);
		}
	}
}


/// <summary>
/// Handles the infantry traveling through a subterranean tunnel.
/// This routine walks the soldier along the tube it entered, following the tube's own
/// path and sinking or rising with it along the way. When the far mouth is reached the
/// infantry is set back down on the map, marked down into its cell, and given a per
/// cell process so that it rejoins the world properly. Anything standing in the way of
/// the exit is told to scatter first.
/// </summary>
void InfantryClass::Tunnel_AI(void)
{
	bool finished = Tubes[CurrentTube]->Dirs[CurrentTubeDir] == FACING_NONE;
	if (!finished) {
		int distance = Distance(LastTubeCoord);
		TubeClass * tube = Tubes[CurrentTube];
		DirType facing;
		DirType direction = facing.Direction(Center_Coord(), LastTubeCoord);
		int speed = Class->MaxSpeed;

		int height_step;
		{
			Cell tube_cell = tube->Exit;
			Coord exit(tube_cell);
			tube_cell = tube->Enter;
			Coord enter(tube_cell);
			height_step = tube->Count;
			height_step = (Map.Get_Height_GL(exit) - Map.Get_Height_GL(enter)) / height_step;
		}

		if (distance <= speed) {
			CurrentTubeDir++;
			FacingType tube_dir = tube->Dirs[CurrentTubeDir];
			finished = tube_dir == FACING_NONE;
			PositionCoord = LastTubeCoord;

			if (!finished) {
				Coord coord = Adjacent_Cell(LastTubeCoord, tube->Dirs[CurrentTubeDir]);
				LastTubeCoord = LastTubeCoord + ((TPoint2D<int> const &)Map[coord].Cell_Coord() - (TPoint2D<int> const &)Map[LastTubeCoord].Cell_Coord());
				LastTubeCoord.Z += height_step;

				DirType dir = DirType().Direction(Center_Coord(), LastTubeCoord);
				Coord new_coord = Move_Coord(PositionCoord, dir, speed - distance);
				double divisor;
				if (tube_dir % 2) {
					divisor = CELL_LEPTON_H * M_SQRT2;
				} else {
					divisor = CELL_LEPTON_H;
				}
				new_coord.Z += (speed - distance) / divisor * height_step;
				PositionCoord = new_coord;
				return;
			}
		} else {
			FacingType tube_dir = tube->Dirs[CurrentTubeDir];
			Coord new_coord = Move_Coord(Get_Coord(), direction, speed);
			double divisor;
			if (tube_dir % 2) {
				divisor = CELL_LEPTON_H * M_SQRT2;
			} else {
				divisor = CELL_LEPTON_H;
			}
			new_coord.Z += speed / divisor * height_step;
			PositionCoord = new_coord;
			return;
		}
	}

	if (finished) {
		CellClass * cellptr = &Map[Get_Coord()];
		ObjectClass * occupier = cellptr->Cell_Occupier();
		if (Can_Enter_Cell(cellptr, FACING_NONE, 0) != MOVE_OK) {
			while (occupier != NULL) {
				if (occupier->RTTI == RTTI_UNIT || occupier->RTTI == RTTI_INFANTRY) {
					if (!((FootClass *)occupier)->Locomotion->Is_Moving()) {
						occupier->Scatter(Coord(0, 0, 0), true, true);
					}
				}
				occupier = occupier->Next;
			}
			Set_Speed(0);
		} else {
			Coord spot = cellptr->Closest_Free_Spot(Get_Coord());
			spot.Z = Map.Get_Height_GL(spot);
			PositionCoord = spot;

			Locomotion->Force_Immediate_Destination(COORD_NONE);
			Locomotion->Stop_Movement_Animation();
			Mark(MARK_DOWN);
			Set_Occupy_Bit(Get_Coord());

			if (NavCom == cellptr) {
				Scatter(Coord(0, 0, 0), true, true);
			} else {
				Set_Speed(1);
			}
			CurrentTube = TUBE_NONE;
			IsPlanningToLook = true;
			Per_Cell_Process(PCP_END);
		}
	}
}


/***********************************************************************************************
 * InfantryClass::AI -- Handles the infantry non-graphic related AI processing.                *
 *                                                                                             *
 *    This routine is used to handle the non-graphic AI processing the infantry requires.      *
 *    Call this routine ONCE per game frame.                                                   *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/08/1994 JLB : Created.                                                                 *
 *   08/14/1996 JLB : Simplified.                                                              *
 *=============================================================================================*/
void InfantryClass::AI(void)
{
	if (CurrentTube >= 0) {
		Tunnel_AI();
		Update_Radar_Position();
		return;
	}

	/*
	**	Act on new orders if the unit is at a good position to do so.
	*/
	if (Ready_To_Commence()) {
		if (Mission == MISSION_NONE && MissionQueue == MISSION_NONE) Enter_Idle_Mode();
		Commence();
	}

	if (Strength <= 0) {
		if (Doing != DO_GUN_DEATH &&
			Doing != DO_EXPLOSION_DEATH &&
			Doing != DO_EXPLOSION2_DEATH &&
			Doing != DO_GRENADE_DEATH &&
			Doing != DO_FIRE_DEATH &&
			Doing != DO_TUMBLE) {
			Strength = 1;
		}
	}

	BASECLASS::AI();

	if (!IsActive) {
		return;
	}

	if (Mission == MISSION_GUARD) {
		BuildingClass *building = Map[(Coord const &)PositionCoord].Cell_Building();
		if (building != NULL) {

			if (!building->Class->IsInvisibleInGame) {
				bool scatter = false;
				if (building->Class->IsLaserFence) {
					scatter = !(building->LaserFenceFrame == 12 || building->LaserFenceFrame == 8);
				} else if (building->Class->IsFirestormWall) {
					if (building->House->FirestormDefenseActivated) {
						scatter = true;
					}
				} else {
					if (!(building->Class->IsGate && building->Is_Gate_Open())) {
						scatter = true;
					}
				}

				if (scatter) {
					Scatter(COORD_NONE, true, true);
				}
			}
		}
	}

	if (Locomotion->Is_Moving_Now() && HeightAGL > 0 && IsOwnedByPlayer && Class->SightRange > 0) {
		if (!LookTimer) {
			Look();
			LookTimer = TICKS_PER_SECOND;
		}
	}

	/*
	**	Infantry that are not on the ground should always be redrawn. Such is
	**	the case when they are parachuting to the ground.
	*/
	if (In_Which_Layer() != LAYER_GROUND) {
		Mark(MARK_CHANGE);
	}

	/*
	**	Special hack to make sure that if this infantry is in firing animation, but the
	**	stage class isn't set, then abort the firing flag.
	*/
	if (IsFiring && Fetch_Rate() == 0) {
		IsFiring = false;
		Do_Action(DO_STAND_READY);
	}

	/*
	**	Delete this unit if it finds itself off the edge of the map and it is in
	**	guard or other static mission mode.
	*/
	if (Edge_Of_World_AI()) {
		return;
	}

	if (Theft_AI()){
		return;
	}

	/*
	**	Act on new orders if the unit is at a good position to do so.
	*/
	if (Ready_To_Commence()) {
		if (Mission == MISSION_NONE && MissionQueue == MISSION_NONE) Enter_Idle_Mode();
		Commence();
	}

	/*
	**	Handle any infantry fear logic or related actions.
	*/
	Fear_AI();

	/*
	**	Special victory dance action.
	*/
	if (NavCom == NULL && !IsProne && IsStoked && Comment == 0) {
		IsStoked = false;
		//Do_Action(Percent_Chance(50) ? DO_GESTURE1 : DO_GESTURE2);
	}

	/*
	**	Determine if this infantry unit should fire off an
	**	attack or not.
	*/
	Firing_AI();

	if (!IsActive) {
		return;
	}

	/*
	**	Handle the completion of the animation sequence.
	*/
	Doing_AI();

	if (!IsActive) {
		return;
	}

	/*
	**	Perform movement operations at this time.
	*/
	Movement_AI();
}


/***********************************************************************************************
 * InfantryClass::Can_Enter_Cell -- Determines if the infantry can enter the cell specified.   *
 *                                                                                             *
 *    This routine is used to examine the cell specified and determine if the infantry is      *
 *    allowed to enter it. It is used by the path finding algorithm.                           *
 *                                                                                             *
 * INPUT:   cell  -- The cell to examine.                                                      *
 *                                                                                             *
 * OUTPUT:  Returns the type of blockage in the cell.                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
MoveType InfantryClass::Can_Enter_Cell(CellClass const * cellptr, FacingType dir, int cell_height, CellClass const * adj_cellptr, bool) const
{
	if (IsInLimbo && Locomotion != NULL && Locomotion->In_Which_Layer() == LAYER_AIR) {
		return(MOVE_OK);
	}

	/*
	**	If we are moving into an illegal cell, then we can't do that.
	*/
	//if ((unsigned)cell >= MAP_CELL_TOTAL) return(MOVE_NO);

	bool isbridge = cellptr->IsUnderBridge && (cell_height == -1 || abs(cell_height - cellptr->Height) > 1);
	unsigned char flag = cellptr->Flag.Composite;
	HousesType inftype = cellptr->InfType;
	bool vehicleflag = cellptr->Flag.Occupy.Vehicle;

	TubeClass *tube = cellptr->Get_Tunnel();

	if (dir == FACING_COUNT) {
		if (tube != NULL && Cell(tube->Exit) != Cell(tube->Enter)) {
			return(MOVE_OK);
		}
		return(MOVE_NO);
	}

	if (tube != NULL) {
		int a = abs(dir - tube->EnterDir);
		if (a > 2 && a < 6 && dir != FACING_NONE) {
			return(MOVE_NO);
		}
	}

	TubeClass *tube_adj = cellptr->Adjacent_Cell(Facing_Sub(dir, FACING_180)).Get_Tunnel();
	if (tube_adj != NULL) {
		int a = abs(Facing_Sub(dir, FACING_180) - tube_adj->EnterDir);
		if (a > FACING_90 && a < FACING_270 && dir != FACING_NONE) {
			return(MOVE_NO);
		}
	}

	if (cell_height - cellptr->Height > 4) {
		return(MOVE_OK);
	}

	if (Can_Reach(cellptr, dir, cell_height, isbridge, adj_cellptr) == MOVE_NO) {
		return(MOVE_NO);
	}

	if (cell_height != -1 && cellptr->IsUnderBridge && cell_height == cellptr->Height + BRIDGE_CELL_HEIGHT) {
		flag = cellptr->BridgeFlag.Composite;
		inftype = cellptr->BridgeInfType;
		vehicleflag = cellptr->BridgeFlag.Occupy.Vehicle;
	}

	/*
	**	If moving off the edge of the map, then consider that an illegal move.
	*/
	if (IsLocked && !ScenarioInit && !Map.In_Local_Radar(cellptr) && !Is_Allowed_To_Leave_Map()) {
		return(MOVE_NO);
	}

	MoveType retval = MOVE_OK;

	/*
	**	Walls are considered impassable for infantry UNLESS the wall has a hole
	**	in it.
	*/
	if (cellptr->Overlay != OVERLAY_NONE) {
		OverlayTypeClass const & otype = *OverlayTypes[cellptr->Overlay];

		if (otype.IsCrate && !House->Is_Human_Player()) {
			return(MOVE_NO);
		}

		if (otype.IsWall) {
			if ((cellptr->OverlayData / 16) != otype.DamageLevels) {

				/*
				**	If the wall can be destroyed, then return this fact instead of
				**	a complete failure to enter.
				*/
				if (Is_Weapon_Equipped() && PrimaryWeapon->Is_Wall_Destroyer()) {
					if (House->Is_Ally(cellptr->Owner)) {
						retval = MOVE_FRIENDLY_DESTROYABLE;
					} else {
						retval = MOVE_DESTROYABLE;
					}
				} else {
					return(MOVE_NO);
				}
			}
		}

	}

	/*
	**	Loop through all of the objects in the square setting a bit
	**	for how they affect movement.
	*/
	int trynum = 0;
	ObjectClass * obj = cellptr->Cell_Occupier(isbridge);
	while (obj != NULL) {

		if (obj != (ObjectClass *)this) {

			/*
			**	Always allow movement if the cell is the object to be captured or sabotaged.
			*/
			if (Mission == MISSION_ENTER || Mission == MISSION_CAPTURE || Mission == MISSION_SABOTAGE ||
				((Mission == MISSION_GUARD_AREA || Mission == MISSION_PATROL || Mission == MISSION_GUARD) && Class->IsEngineer)) {
				if (obj == NavCom || (&Map[(Coord const &)obj->PositionCoord] == NavCom || obj == TarCom)) {
					if (!IsTethered && !isbridge && Ground[cellptr->Land_Type()].Cost[Class->Speed] == 0) {
						return(MOVE_NO);
					}
					return(MOVE_OK);
				}
				BuildingClass *building = cellptr->Cell_Building();
				if (building != NULL && obj != building) {
					if (building == NavCom) {
						obj = obj->Next;
						continue;
					}

					if (&Map[building->Center_Coord()] == NavCom || building == TarCom) {
						obj = obj->Next;
						continue;
					}
				}
			}

			/*
			**	Guard area should not allow the guarding unit to enter the cell with the
			**	guarded unit.
			*/
			//if (Mission == MISSION_GUARD_AREA && ArchiveTarget == obj && Is_Target_Unit(ArchiveTarget)) {
			if (Mission == MISSION_GUARD_AREA && ArchiveTarget == obj && ArchiveTarget->RTTI == RTTI_UNIT) {
				return(MOVE_NO);
			}

			if (Class->IsVehicleThief && obj->Considered_Vehicle() && NavCom == obj && !obj->TClass->IsTrain) {
				return(MOVE_OK);
			}

			/*
			**	If object is a land mine, allow movement
			*/
			if (obj->RTTI == RTTI_BUILDING) {
				BuildingClass *building = (BuildingClass *)obj;
				if (building->Class->IsInvisibleInGame) {
					obj = obj->Next;
					continue;
				}

				if (building->Class->IsLaserFence && (building->LaserFenceFrame == 12 || building->LaserFenceFrame == 8)) {
					obj = obj->Next;
					continue;
				}

				if (building->Class->IsFirestormWall) {
					if (building->House->FirestormDefenseActivated) {
						return(MOVE_NO);
					}
					obj = obj->Next;
					continue;
				}

				if (building->Class->IsGate) {
					if (!building->Is_Gate_Open()) {
						if (building->House->Is_Ally(House)) {
							if (retval < MOVE_CLOSED_GATE) {
								retval = MOVE_CLOSED_GATE;
							}
						} else {
							if (!Is_Weapon_Equipped()) {
								return(MOVE_NO);
							}
							if (retval < MOVE_DESTROYABLE) {
								retval = MOVE_DESTROYABLE;
							}
						}
					}
					obj = obj->Next;
					continue;
				}
			}

			/*
			**	Special case check so that a landed aircraft that is in radio contact, will not block
			**	a capture attempt. It is presumed that this case happens when a helicopter is landed
			**	at a helipad.
			*/
//			if ((Mission != MISSION_CAPTURE && Mission != MISSION_SABOTAGE) || obj->What_Am_I() != RTTI_AIRCRAFT || !((AircraftClass *)obj)->In_Radio_Contact()) {

				/*
				**	Special check to always allow entry into the building that this infantry
				**	is trying to capture.
				*/
//				if (obj->What_Am_I() == RTTI_BUILDING || obj->What_Am_I() == RTTI_AIRCRAFT || obj->What_Am_I() == RTTI_UNIT) {
//					if ((Mission == MISSION_CAPTURE || Mission == MISSION_SABOTAGE) && (obj == NavCom || obj == TarCom)) {
//						return(MOVE_OK);
//					}
//				}

				/*
				**	Special check to always allow entry into the building that this infantry
				**	is trying to capture.
				*/
				if (Mission == MISSION_ENTER && obj == NavCom && IsTethered) {
					return(MOVE_OK);
				}

				/*
				**	Allied objects block movement using different rules than for enemy
				**	objects.
				*/
				if (House->Is_Ally(obj) || ScenarioInit) {
					switch ((RTTIType)obj->RTTI) {

						/*
						**	A unit blocks as either a moving blockage or a stationary temp blockage.
						**	This depends on whether the unit is currently moving or not.
						*/
						case RTTI_UNIT:
							if (((UnitClass *)obj)->Locomotion->Is_Moving() || ((UnitClass *)obj)->NavCom != NULL) {
								if (((UnitClass *)obj)->IsOccupyingCell || ((UnitClass *)obj)->Locomotion->Will_Jump_Tracks()) {
									if (retval < MOVE_MOVING_BLOCK) retval = MOVE_MOVING_BLOCK;
								}
							} else {
								if (retval < MOVE_TEMP) retval = MOVE_TEMP;
							}
							break;

						/*
						**	Aircraft and buildings always block movement. If for some reason there is an
						**	allied terrain object, that blocks movement as well.
						*/
						//case RTTI_TERRAIN:
						case RTTI_AIRCRAFT:
						case RTTI_BUILDING:
							return(MOVE_NO);

						case RTTI_INFANTRY:
							if (!((InfantryClass *)obj)->Locomotion->Is_Moving()) {
								trynum++;
							}
							break;

						case RTTI_TERRAIN:
						default:
							break;
					}

				} else {

					/*
					**	Cloaked enemy objects are not considered if this is a Find_Path()
					**	call.
					*/
					TechnoClass *tech = Dynamic_Cast<TechnoClass *>(obj);
					if (tech == NULL || tech->Cloak != CLOAKED) {

						RTTIType rtti = obj->RTTI;

						/*
						**	Any non-allied blockage is considered impassible if the infantry
						**	is not equipped with a weapon.
						*/
						if (Combat_Damage() <= 0 && rtti != RTTI_TERRAIN) return(MOVE_NO);

						/*
						**	Some kinds of terrain are considered destroyable if the infantry is equipped
						**	with the weapon that can destroy it. Otherwise, the terrain is considered
						**	impassable.
						*/
						switch (rtti) {
							case RTTI_TERRAIN:
#ifdef OBSOLETE
								if (((TerrainClass *)obj)->Class->Armor == ARMOR_WOOD &&
										Class->PrimaryWeapon->WarheadPtr->IsWoodDestroyer) {

									if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
								} else {
									return(MOVE_NO);
								}
								break;
#else
								break;
#endif
							case RTTI_INFANTRY:
								if (((InfantryClass *)obj)->Class->IsDisguised) {
									retval = MOVE_TEMP;
								}
								break;

							case RTTI_BUILDING:
								if (((BuildingClass *)obj)->Class->IsBridgeRepairHut) {
									return(MOVE_NO);
								}
								if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
								break;

							default:
								if (retval < MOVE_DESTROYABLE) retval = MOVE_DESTROYABLE;
								break;
						}
					} else {
						if (retval < MOVE_CLOAK) retval = MOVE_CLOAK;
					}
				}
//			}
		}

		/*
		**	Move to next object in chain.
		*/
		obj = obj->Next;
	}

	/*
	**	If foot soldiers cannot travel on the cell -- consider it impassable.
	*/
	if (/*retval == MOVE_OK &&*/ !IsTethered && !isbridge && Ground[cellptr->Land_Type()].Cost[Class->Speed] == 0) {

#ifdef OBSOLETE
		/*
		** Special case - if it's an engineer, and the cell under consideration
		** is his NavCom, and his mission is mission_capture, then he's most
		** likely moving to his final destination to repair a bridge, so we
		** should let him.
		*/
		if (*this == INFANTRY_RENOVATOR && Is_Target_Cell(TarCom) && (cell == ::As_Cell(NavCom)) && (cellptr->TType == TEMPLATE_BRIDGE1D || cellptr->TType == TEMPLATE_BRIDGE2D || (cellptr->TType >= TEMPLATE_BRIDGE_1C && cellptr->TType <= TEMPLATE_BRIDGE_3E) ) ) {
			return(MOVE_OK);
		}
#endif
		return(MOVE_NO);
	}

	/*
	**	if a unit has the cell reserved then we just can't go in there.
	*/
	if (retval == MOVE_OK && vehicleflag) {
		return(MOVE_MOVING_BLOCK);
	}

	/*
	**	if a block of infantry has the cell reserved then there are two
	**	possibilities...
	*/
	if (inftype != HOUSE_NONE) {
		if (House->Is_Ally(inftype)) {
			if ((flag & 0x1C) == 0x1C) {
				if (retval < MOVE_MOVING_BLOCK) {
					if (trynum == 3) {
						retval = MOVE_TEMP;
					} else {
						retval = MOVE_MOVING_BLOCK;
					}
				}
			}
		} else {
			if (Combat_Damage() > 0) {
				if (retval < MOVE_DESTROYABLE) {
					retval = MOVE_DESTROYABLE;
				}
			} else {
				return(MOVE_NO);
			}
		}
	}

	/*
	**	If it is still ok to move the infantry, then perform the last check
	**	to see if the cell is already full of infantry.
	*/
	if (retval == MOVE_OK && (flag & 0x1C) == 0x1C) {
		return(MOVE_NO);
	}

	/*
	**	Return with the most severe reason why this cell would be impassable.
	*/
	return(retval);
}


/***********************************************************************************************
 * InfantryClass::Can_Fire -- Can the infantry fire its weapon?                                *
 *                                                                                             *
 *    Determines if the infantry unit can fire on the target. If it can't fire, then the       *
 *    reason why is returned.                                                                  *
 *                                                                                             *
 * INPUT:   target   -- The target to determine if the infantry can fire upon.                 *
 *                                                                                             *
 * OUTPUT:  Returns the fire error type that indicates if the infantry can fire and if it      *
 *          can't, why not.                                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *   06/27/1995 JLB : Flame thrower can fire while prone now.                                  *
 *=============================================================================================*/
FireErrorType InfantryClass::Can_Fire(AbstractClass * target, int which) const
{
	/*
	**	If a medic is shooting at a healed target, let's declare the target
	**	illegal so he won't be constantly healing healed infantrymen.
	*/
	if (Combat_Damage() < 0) {
		InfantryClass * targ = target->As_InfantryClass();
		if (targ == NULL || targ->HealthRatio >= Rule->ConditionGreen) {
			return(FIRE_ILLEGAL);
		}
	}

	/*
	**	If this unit cannot fire while moving, then bail.
	*/
	if (Speed > 0.1 || (NavCom != NULL && Doing != DO_NOTHING && !MasterDoControls[Doing].Interrupt)) {
		return(FIRE_MOVING);
	}

	WeaponTypeClass const * weapon = Get_Class_Weapon_Data(which)->Weapon;
	if (weapon != NULL && weapon->UseFireParticles && NavCom != NULL) {
		return(FIRE_MOVING);
	}

	FireErrorType error = Locomotion->Can_Fire();
	if (error) {
		return(error);
	}

	return(BASECLASS::Can_Fire(target, which));
}


/// <summary>
/// Is the infantry unable to move at the moment?
/// A soldier struggling back to its feet after being knocked prone is pinned in place
/// until it has finished doing so.
/// </summary>
/// <returns>bool; Is the infantry immobilized?</returns>
bool InfantryClass::Is_Immobilized(void) const
{
	return(ProneStruggleTimer > 0 || BASECLASS::Is_Immobilized());
}


/***********************************************************************************************
 * InfantryClass::Enter_Idle_Mode -- The infantry unit enters idle mode by this routine.       *
 *                                                                                             *
 *    Use this routine when the infantry unit as accomplished its task and needs to find       *
 *    something to do. The default behavior is to enter some idle state such as guarding.      *
 *                                                                                             *
 * INPUT:   initial  -- Is this called when the unit just leaves a factory or is initially     *
 *                      or is initially placed on the map?                                     *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Enter_Idle_Mode(bool initial, bool resume_waypoint)
{
	bool result = BASECLASS::Enter_Idle_Mode(initial, resume_waypoint);

	MissionType order = MISSION_GUARD;

	if (TarCom != NULL) {
		order = MISSION_ATTACK;
		if (Mission == MISSION_SABOTAGE) {
			order = MISSION_SABOTAGE;
		}
		if (Mission == MISSION_CAPTURE) {
			order = MISSION_CAPTURE;
		}
	} else {

		Handle_Navigation_List();

		if (NavCom != NULL) {
			order = MISSION_MOVE;
			if (Mission == MISSION_CAPTURE) {
				order = MISSION_CAPTURE;
			}
			if (Mission == MISSION_SABOTAGE) {
				order = MISSION_SABOTAGE;
			}
		} else {

			if (Mission == MISSION_GUARD || Mission == MISSION_GUARD_AREA || Mission != MISSION_NONE && (Current_Mission_Control().IsZombie || Current_Mission_Control().IsParalyzed)) {
				return(false);
			}

			if (House->Is_Human_Player() || Team != NULL) {
				if (CurrentMission == MISSION_GUARD_AREA || (Has_Ability(ABILITY_GUARD_AREA) && Team == NULL)) {
					order = MISSION_GUARD_AREA;
				} else {
					order = MISSION_GUARD;
				}

			} else {
				if (House->IQ < Rule->IQGuardArea && CurrentMission != MISSION_GUARD_AREA) {
					order = MISSION_GUARD;
				} else {
					if (Is_Weapon_Equipped()) {
						order = MISSION_GUARD_AREA;
					} else {
						if (Class->IsEngineer || Class->IsVehicleThief) {
							order = MISSION_GUARD_AREA;
						} else {
							order = MISSION_GUARD;
						}
					}
				}
			}
		}
	}
	if (CurrentMission != MISSION_PATROL && CurrentMission != MISSION_GUARD_AREA && order != MISSION_NONE) {
		Assign_Mission(order);
	}
	return(result);
}


/***********************************************************************************************
 * InfantryClass::Random_Animate -- Randomly animate the infantry (maybe)                      *
 *                                                                                             *
 *    This routine is the random animator initiator for infantry units. This routine should    *
 *    be called regularly. On occasion, it will cause the infantry to go into an idle          *
 *    animation.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/01/1994 JLB : Created.                                                                 *
 *   12/13/1994 JLB : Does random facing change.                                               *
 *   07/02/1995 JLB : Nikoomba special effects.                                                *
 *=============================================================================================*/
bool InfantryClass::Random_Animate(void)
{
	if (Is_Ready_To_Random_Animate()) {
		IdleTimer = (int)Random_Double(Rule->RandomAnimateTime * (TICKS_PER_MINUTE/2), Rule->RandomAnimateTime * (TICKS_PER_MINUTE*2));

		/*
		**	Scared infantry will always follow the golden rule of civilians;
		**		"When in darkness or in doubt, run in circles, scream, and shout!"
		*/
		if (Class->IsFraidyCat && !House->Is_Human_Player() && Fear > FEAR_ANXIOUS) {
			Scatter(COORD_NONE, true);
			return(true);
		}

		switch (Random_Pick(0, 10)) {
			case 0:
				break;

			case 3:
			case 4:
			case 5:
				Do_Action(DO_IDLE1);
				break;

			case 6:
				PrimaryFacing.Set(Facing_Dir(Random_Pick(FACING_N, FACING_NW)));
				break;

			case 1:
			case 2:
			case 7:
				Do_Action(DO_IDLE2);
				PrimaryFacing.Set(Facing_Dir(Random_Pick(FACING_N, FACING_NW)));
				if (!IsSelected && IsOwnedByPlayer && Class->VoiceComment.Count() > 0 && Sim_Random_Pick(0, 2) == 0) {
					Sound_Effect((VocType)Class->VoiceComment[0], PositionCoord);
				}
				break;

			/*
			**	On occasion, civilian types will wander about.
			*/
			case 8:
				PrimaryFacing.Set(Facing_Dir(Random_Pick(FACING_N, FACING_NW)));
				if (!House->Is_Human_Player() && Class->IsFraidyCat) {
					Scatter(COORD_NONE, true);
				}
				break;

			case 9:
			case 10:
				PrimaryFacing.Set(Facing_Dir(Random_Pick(FACING_N, FACING_NW)));
				break;

		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * InfantryClass::Scatter -- Causes the infantry to scatter to nearby cell.                    *
 *                                                                                             *
 *    This routine is used when the infantry should scatter to a nearby cell. Scattering       *
 *    occurs as an occasional consequence of being fired upon. It is one of the features       *
 *    that makes infantry so "charming".                                                       *
 *                                                                                             *
 * INPUT:   threat   -- The coordinate source of the threat that is causing the infantry to    *
 *                      scatter. If the threat isn't from a particular direction, then this    *
 *                      parameter will be NULL.                                                *
 *                                                                                             *
 *          forced   -- The threat is real and a serious effort to scatter should be made.     *
 *                                                                                             *
 *          nokidding-- The scatter should affect the player's infantry even if it otherwise   *
 *                      wouldn't have.                                                         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *   12/12/1994 JLB : Flame thrower infantry always scatter.                                   *
 *   08/02/1996 JLB : Added the nokidding parameter                                            *
 *=============================================================================================*/
void InfantryClass::Scatter(Coord const & threat, bool forced, bool nokidding)
{
	/*
	**	A unit that is in the process of going somewhere will never scatter.
	*/
	if (Locomotion->Is_Moving()) forced = false;

	/*
	**	Certain missions prevent scattering regardless of whether it would be
	**	a good idea or not.
	*/
	if (!Current_Mission_Control().IsScatter && !forced) return;

	/*
	**	If the infantry is currently engaged in legitimate combat, then don't
	**	scatter unless forced to.
	*/
	if (!Class->IsFraidyCat && TarCom != NULL && !forced) return;

	/*
	**	Don't scatter if performing an action that can't be interrupted.
	*/
	if (Doing != DO_NOTHING && !MasterDoControls[Doing].Interrupt) return;

	/*
	**	For human players, don't scatter the infantry, if the special
	**	flag has not been enabled that allows infantry scatter.
	*/
	if (!Rule->IsScatter && !Has_Ability(ABILITY_SCATTER) && !nokidding && House->Is_Human_Player() && !forced && Team == NULL) return;

	if (forced || Class->IsFraidyCat) {
		FacingType	toface;

		if (threat != COORD_NONE) {
			toface = Dir_Facing(::Direction(threat, PositionCoord));
			toface = (FacingType)(toface + Random_Pick(FACING_FIRST, FACING_180) - FACING_90);
		} else {
			Point2D coord(Center_Coord().X, Center_Coord().Y);
			coord = Point2D(coord.X & (CELL_LEPTON_W-1), coord.Y & (CELL_LEPTON_H-1));

			if (coord != Point2D(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2)) {
				toface = ::Direction(Point2D(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2), coord).As_Dir8();
			} else {
				toface = PrimaryFacing.Current().As_Dir8();
			}
			toface = (FacingType)(toface + FacingType(Random_Pick(FACING_FIRST, FACING_180) - FACING_90));
			Cell nearbyloc = Map.Nearby_Location(Destination_Coord().As_Cell(), Class->Speed, -1, MZONE_NORMAL, IsOnBridge, Point2D(1, 1), false, true);

			if (nearbyloc != CELL_NONE) {
				Assign_Destination(&Map[nearbyloc]);
				return;
			}
		}

		Cell altcell(0, 0);
		Cell newcell(0, 0);
		Cell _cell(Destination_Coord());

		CellClass *cellptr = &Map[_cell];

		/// The parentheses are missing on purpose: the whole sum is the ternary
		/// condition, not just the bridge term.
		int z = cellptr->Height + Is_Moving_Onto_Bridge() ? BRIDGE_CELL_HEIGHT : 0;
		Coord destcoord = Destination_Coord();
		destcoord.Z = z * LEVEL_LEPTON_H;

		FacingType face;
		for (face = FACING_N; face < FACING_COUNT; face++) {
			FacingType newface = Facing_Add(toface, face);
			Cell checkcell = Adjacent_Cell(_cell, newface);
			CellClass *cptr = &Map[checkcell];

			if (Map.In_Local_Radar(checkcell) && Can_Enter_Cell(cptr, newface, Get_Cell_Height()) == MOVE_OK) {
				if (altcell == CELL_NONE) altcell = checkcell;
				if (CELL_NONE == Cell(0,0)) {
					Coord checkcoord = checkcell.As_Coord();
					checkcoord.Z = destcoord.Z;
					if (checkcell == TacticalMap->Coord_To_Cell(checkcoord) && !Map[checkcell].IsUnderBridge) {
						newcell = checkcell;
						break;
					}
				}
			}
		}

		Cell destination = newcell;
		if (destination == CELL_NONE) {
			destination = altcell;
		}

		if (destination != CELL_NONE) {
			Assign_Mission(MISSION_MOVE);
			Assign_Destination(&Map[destination]);
		}
	}
}


/***********************************************************************************************
 * InfantryClass::Do_Action -- Launches the infantry into an animation sequence.               *
 *                                                                                             *
 *    This starts the infantry into a choreographed animation sequence. These sequences can    *
 *    be as simple as standing up or lying down, but can also be complex, such as dying or     *
 *    performing some idle animation.                                                          *
 *                                                                                             *
 * INPUT:   todo     -- The choreographed sequence to start.                                   *
 *                                                                                             *
 *          force    -- Force starting this animation even if the current animation is flagged *
 *                      as uninterruptible. This is necessary for death animations.            *
 *                                                                                             *
 * OUTPUT:  bool; Was the animation started?                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Do_Action(DoType todo, bool force, bool randomize)
{
	if (todo == DO_NOTHING || Class->DoControls[todo].Count == 0) {
		return(false);
	}

	if (todo != Doing && (Doing == DO_NOTHING || force || MasterDoControls[Doing].Interrupt)) {
		Doing = todo;
		if (todo == DO_IDLE1 || todo == DO_IDLE2) {
			Set_Rate(Options.Normalize_Delay(MasterDoControls[Doing].Rate));
		} else {
			Set_Rate(MasterDoControls[Doing].Rate);
		}
		if (randomize) {
			Set_Stage(Sim_Random_Pick(0,std::max((int)Class->DoControls[todo].Count, 1) - 1));
		} else {
			Set_Stage(0);
		}

		/*
		**	Kludge to make sure that if infantry is in the dying animation, it isn't still
		**	moving as well.
		*/
		if (Strength == 0) {
			Stop_Driver();
		}

		/*
		**	Since the animation sequence might be interrupted. Set any flags
		**	necessary so that if interrupted, the affect on the infantry is
		**	still accomplished.
		*/
		switch (todo) {
			case DO_LIE_DOWN:
				IsProne = true;
				break;

			case DO_GET_UP:
				IsProne = false;
				break;

			default:
				break;
		}

		return(true);
	}

	return(false);
}


/***********************************************************************************************
 * InfantryClass::Stop_Driver -- Stops the infantry from moving any further.                   *
 *                                                                                             *
 *    This is used to stop the infantry from animating in movement. This function will stop    *
 *    the infantry moving and revert it to either a prone or standing.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the driving stopped?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Stop_Driver(void)
{
	if (IsProne) {
		Do_Action(DO_PRONE);
	} else {
		Do_Action(DO_STAND_READY);
	}

	if (Can_Enter_Cell(&Map[(Coord const &)PositionCoord], PrimaryFacing.Current().As_Dir8(), Get_Cell_Height()) == MOVE_OK) {
		IsZoneCheat = false;
	} else {
		IsZoneCheat = true;
	}

	return(BASECLASS::Stop_Driver());
}


/***********************************************************************************************
 * InfantryClass::Start_Driver -- Handles giving immediate destination and move orders.        *
 *                                                                                             *
 *    Use this routine to being the infantry moving toward the destination specified. The      *
 *    destination is first checked to see if there is a free spot available. Then the infantry *
 *    reserves that spot and begins movement toward it.                                        *
 *                                                                                             *
 * INPUT:   headto   -- The coordinate location desired for the infantry to head to.           *
 *                                                                                             *
 * OUTPUT:  bool; Was the infantry successfully started on its journey? Failure may be because *
 *                the specified destination could not contain the infantry unit.               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/21/1994 JLB : Created.                                                                 *
 *   05/14/1995 JLB : Tries to move to closest spot possible.                                  *
 *   05/15/1995 JLB : Uses closest spot if moving onto transport.                              *
 *=============================================================================================*/
bool InfantryClass::Start_Driver(Coord & headto)
{
	Coord old = headto;

	/*
	**	Convert the head to coordinate to a legal sub-position location.
	*/
	headto = Map[headto].Closest_Free_Spot(Move_Coord(headto, DirType().Direction(Center_Coord(), headto)+((DIR_S << 8)-1), CELL_LEPTON / 2 - 4), false, Map[headto].IsUnderBridge);
	if (headto == COORD_NONE && Can_Enter_Cell(&Map[old]) == MOVE_OK) {
		headto = Map[old].Closest_Free_Spot(Move_Coord(old, DirType().Direction(Center_Coord(), headto)+((DIR_S << 8)-1), CELL_LEPTON / 2), true, Map[old].IsUnderBridge);
	}

	/*
	**	If the infantry started moving, then fixup the occupation bits.
	*/
	if (headto != COORD_NONE && BASECLASS::Start_Driver(headto)) {
		if (!IsActive) return(false);

		/*
		**	Remove the occupation bit from the infantry's current location.
		*/
		Clear_Occupy_Bit(PositionCoord);

		/*
		**	Set the occupation bit for the new headto location.
		*/
		Set_Occupy_Bit(headto);
		return(true);
	}

	return(false);
}


/***********************************************************************************************
 * InfantryClass::Limbo -- Performs cleanup operations needed when limboing.                   *
 *                                                                                             *
 *    This routine will clean up the infantry occupation bits (as necessary) as well as stop   *
 *    the infantry movement process when it gets limboed.                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the infantry unit limboed?                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Limbo(void)
{
	Locomotion->Stop_Movement_Animation();
	return(BASECLASS::Limbo());
}


/***********************************************************************************************
 * InfantryClass::Fire_At -- Fires projectile from infantry unit.                              *
 *                                                                                             *
 *    Use this routine when the infantry unit wishes to fire a projectile. This routine        *
 *    will launch the projectile and perform any other necessary infantry specific operations. *
 *                                                                                             *
 * INPUT:   target   -- The target of the attack.                                              *
 *                                                                                             *
 *          which    -- Which weapon to use for firing. 0=primary, 1=secondary.                *
 *                                                                                             *
 * OUTPUT:  Returns with pointer to the projectile launched. If none could be launched, then   *
 *          NULL is returned. If there is already the maximum bullet objects in play, then     *
 *          this could happen.                                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
BulletClass * InfantryClass::Fire_At(AbstractClass * target, int which)
{
	IsFiring = false;

	BulletClass * bullet = BASECLASS::Fire_At(target, which);
	if (bullet != NULL && !IsInLimbo) {

		/*
		**	For fraidycat infantry that run out of ammo, always go into
		**	a maximum fear state at that time.
		*/
		if (Class->IsFraidyCat && !Ammo) {
			Fear = FEAR_MAXIMUM;
			if (Mission == MISSION_ATTACK || Mission == MISSION_HUNT) {
				Assign_Mission(MISSION_GUARD);
			}
		}
	}
	return(bullet);
}


/***********************************************************************************************
 * InfantryClass::Unlimbo -- Unlimbo infantry unit in legal sub-location.                      *
 *                                                                                             *
 *    This will attempt to unlimbo the infantry unit at the designated coordinate, but will    *
 *    ensure that the coordinate is a legal subposition.                                       *
 *                                                                                             *
 * INPUT:   coord    -- The coordinate to unlimbo the infantry at.                             *
 *                                                                                             *
 *          facing   -- The desired initial facing for the infantry unit.                      *
 *                                                                                             *
 *          strength -- The desired initial strength for the infantry unit.                    *
 *                                                                                             *
 *          mission  -- The desired initial mission for the infantry unit.                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the infantry unlimboed successfully?                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/26/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Unlimbo(Coord const & xcoord, Dir256 facing)
{
	int height = Map.Get_Height_GL(xcoord);
	Coord coord = xcoord;

	if (coord.Z == height) {

		/*
		**	Make sure that the infantry start in a legal position on the map.
		*/
		coord = Map[xcoord].Closest_Free_Spot(xcoord, ScenarioInit || !Map.In_Local_Radar(xcoord.As_Cell()));
		if (coord == COORD_NONE) {
			return(false);
		}
		coord.Z = height;
	}

	if (BASECLASS::Unlimbo(coord, facing)) {

		/*
		**	If there is no sight range, then this object isn't discovered by the player unless
		**	it actually appears in a cell mapped by the player.
		*/
		if (Class->SightRange == 0) {
			IsDiscoveredByPlayer = false;
		}

		if (coord.Z <= height + BRIDGE_LEPTON_HEIGHT) {
			Set_Occupy_Bit(coord);
		}

		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * InfantryClass::Greatest_Threat -- Determines greatest threat (target) for infantry unit.    *
 *                                                                                             *
 *    This routine intercepts the Greatest_Threat request and adds the appropriate target      *
 *    types to search for. For regular infantry, this consists of all the ground types. For    *
 *    rocket launching infantry, this also includes aircraft.                                  *
 *                                                                                             *
 * INPUT:   threat   -- The basic threat control value.                                        *
 *                                                                                             *
 * OUTPUT:  Returns with the best target for this infantry unit to attack. If no suitable      *
 *          target could be found, then TARGET_NONE is returned.                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/01/1995 JLB : Created.                                                                 *
 *   09/28/1995 JLB : Engineers try to recapture buildings first.                              *
 *=============================================================================================*/
AbstractClass * InfantryClass::Greatest_Threat(ThreatType threat, Coord const & coord, bool onlyenemy) const
{
	/*
	**	Engineers consider only buildings that can be captured as being a threat. All others
	**	are ignored. If there is a building that needs to be recaptured and it is nearby
	**	then automatically head toward it to recapture it.
	*/
	if (!House->Is_Human_Player() && Class->IsCapture && !Is_Weapon_Equipped()) {
		if (House->ToCapture != NULL && Distance_To(House->ToCapture) < 15 * CELL_LEPTON) {
			return(House->ToCapture);
		}
		threat = ThreatType(threat | THREAT_CAPTURE);
	}

	if (Class->IsVehicleThief) {
		if (NavCom != NULL && NavCom->Is_Techno() && ((TechnoClass *)NavCom)->Considered_Vehicle() && !((TechnoClass *)NavCom)->TClass->IsTrain && Distance(NavCom) < 15 * CELL_LEPTON) {
			return(NavCom);
		}
	}

	if (!Is_Weapon_Equipped()) {
		if (!Class->IsCapture && !Class->IsVehicleThief) {
			return(NULL);
		}
		if (Class->IsVehicleThief) {
			threat = ThreatType((threat & ~(THREAT_BUILDINGS|THREAT_VEHICLES|THREAT_BOATS|THREAT_AIR)) | THREAT_VEHICLES);
		}
	}

	if ((threat & (THREAT_INFANTRY|THREAT_VEHICLES|THREAT_BUILDINGS|THREAT_TIBERIUM|THREAT_CIVILIANS|THREAT_POWER|THREAT_FACTORIES|THREAT_BASE_DEFENSE)) == 0) {
		if (PrimaryWeapon != NULL) {
			threat = ThreatType(threat | PrimaryWeapon->Allowed_Threats());
		}
		if (SecondaryWeapon != NULL) {
			threat = ThreatType(threat | SecondaryWeapon->Allowed_Threats());
		}
	}

	/*
	**	Organic weapon types don't consider anything but infantry to be a threat. Such
	**	weapon types would be the dog jaw and the medic first aid kit.
	*/
	if (Is_Weapon_Equipped() && PrimaryWeapon->WarheadPtr->IsOrganic) {
		threat = ThreatType(threat & ~(THREAT_BUILDINGS|THREAT_VEHICLES|THREAT_BOATS|THREAT_AIR));
	}

	/*
	**	Human controlled infantry don't automatically fire upon buildings.
	*/
	if (Is_Weapon_Equipped() && House->Is_Human_Player()) {
		threat = ThreatType(threat & ~THREAT_BUILDINGS);
	}

	/*
	**	If this is a bomber type, then allow buildings to be considered a threat.
	*/
	if ((Class->IsBomber || Has_Ability(ABILITY_C4)) && !House->Is_Human_Player()) {
		threat = ThreatType(threat | THREAT_BUILDINGS);
	}

	/*
	**	Special hack: if it's a thief, then the only possible objects to
	**	consider are tiberium-processing objects (silos & refineries).
	*/
	if (Class->IsThief) {
		threat = ThreatType(threat | THREAT_CAPTURE | THREAT_TIBERIUM);
	}
	return(BASECLASS::Greatest_Threat(threat, coord, onlyenemy));
}


/***********************************************************************************************
 * InfantryClass::What_Action -- Infantry units might be able to capture -- check.             *
 *                                                                                             *
 *    This routine checks to see if the infantry unit can capture the specified object rather  *
 *    than merely attacking it. If this is the case, then ACTION_CAPTURE will be returned.     *
 *                                                                                             *
 * INPUT:   object   -- The object that the mouse is currently over.                           *
 *                                                                                             *
 * OUTPUT:  Returns the action that will be performed if the mouse were clicked over the       *
 *          object specified.                                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/01/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType InfantryClass::What_Action(ObjectClass const * object, bool disallow_force) const
{
	assert(object != NULL);

	ActionType action = BASECLASS::What_Action(object, disallow_force);

	/*
	**	If this is an engineer/renovator, we have to make some adjustments.
	**	If the cursor is over an enemy building, return action-none.  If it's
	**	over a friendly building, we have to return action-capture so he can
	**	renovate it.
	**	However, abort the whole thing if the building is a barrel or mine.
	*/
	if (Class->IsEngineer && object->RTTI == RTTI_BUILDING && House->Is_Player_Control()) {
		BuildingClass const * bldg = (BuildingClass *)object;
		if (!bldg->Considered_Vehicle() || bldg->Class->IsMobileWar) {
			if (bldg->Class->IsRepairable || bldg->Class->IsMobileWar) {
				if (bldg->Class->IsBridgeRepairHut) {
					return(Map.Can_Repair_Bridge(bldg->Center_Coord()) ? ACTION_GREPAIR : ACTION_NO_GREPAIR);
				}
				if (House->Is_Ally(bldg)) {
					if (bldg->HealthRatio == Rule->ConditionGreen) {
						return(ACTION_NO_GREPAIR);
					}
					return(ACTION_GREPAIR);
				} else {

					if (bldg->Class->IsCaptureable) {
						if (bldg->HealthRatio > Rule->EngineerCaptureLevel) {
							return(ACTION_DAMAGE);
						}
						return(ACTION_CAPTURE);
					}
				}
			}
		}

	}

	/*
	**	If this is a medic, and the cursor's over a friendly infantryman,
	**	execute an action-attack.  In CSII, if this is a mechanic and the
	**	cursor's over a friendly vehicle, execute an action-attack.
	*/
	if (Combat_Damage() < 0 && House->Is_Player_Control()) {
		if (House->Is_Ally(object)) {
			if (object->RTTI == RTTI_INFANTRY) {
				if (object != this) {
					if (object->HealthRatio < Rule->ConditionGreen) {
						return(ACTION_HEAL);
					}
				} else {
					return(ACTION_GUARD_AREA);
				}
			}
			const TechnoClass *tech = ::Dynamic_Cast<TechnoClass const *>(object);
			if (tech == NULL || !tech->TClass->Max_Passengers()) {
				if (action == ACTION_GUARD_AREA || action == ACTION_MOVE) {
					return(action);
				}
				return(ACTION_SELECT);
			}
		} else {
			//return(ACTION_NOMOVE);
			return(ACTION_ATTACK_SUPPORT);
		}
	}

	/*
	**	See if it's a thief attacking an enemy vehicle, let him CAPTURE it.
	*/
	if (House->Is_Player_Control() && Class->IsVehicleThief && object->RTTI != RTTI_BUILDING && object->Considered_Vehicle()) {
		if (object->TClass->IsTrain) {
			return(ACTION_SELECT);
		}
		if (((UnitClass *)object)->House != House) {
			if (Scen->Special.IsHarvesterImmune && Rule->HarvesterUnit.Is_In_List((UnitTypeClass const *)object)) {
				return(ACTION_SELECT);
			}
			return(ACTION_CAPTURE);
		}
	}

	if (object->RTTI == RTTI_BUILDING && House->Is_Ally(object) && House->Is_Player_Control()) {
		BuildingClass const * bldg = (BuildingClass *)object;
		if (bldg->Class->IsHospital) {
			if (Strength < Class->MaxStrength) {
				switch (((InfantryClass *)this)->Transmit_Message(RADIO_CAN_LOAD, (RadioClass *)object)) {
					case RADIO_ROGER:
						action = ACTION_ENTER;
						break;

					case RADIO_NEGATIVE:
						action = ACTION_NO_ENTER;
						break;
				}
			} else {
				action = ACTION_NO_ENTER;
			}
		}
	}

	if (object->RTTI == RTTI_BUILDING && House->Is_Ally(object) && House->Is_Player_Control()) {
		BuildingClass const * bldg = (BuildingClass *)object;
		if (bldg->Class->IsArmory) {
			if (!Veterancy.Is_Elite()) {
				switch (((InfantryClass *)this)->Transmit_Message(RADIO_CAN_LOAD, (RadioClass *)object)) {
					case RADIO_ROGER:
						action = ACTION_ENTER;
						break;

					case RADIO_NEGATIVE:
						action = ACTION_NO_ENTER;
						break;
				}
			} else {
				action = ACTION_NO_ENTER;
			}
		}
	}

	/*
	**	See if it's a commando, and if he's attacking a building,
	**	have him return ACTION_SABOTAGE instead
	*/
	if (House->Is_Player_Control() && (Class->IsBomber || Has_Ability(ABILITY_C4)) && action == ACTION_ATTACK && object->RTTI == RTTI_BUILDING && !object->Considered_Vehicle()) {
		BuildingClass const * obj = (BuildingClass *)object;
		/*
		**	Hack: Tanya should shoot barrels, bomb other structures.
		*/
		if (obj->Class->IsRepairable) {
			return(ACTION_SABOTAGE);
		} else {
			return(ACTION_ATTACK);
		}
	}

	/*
	**	There is no self-select action available for infantry types.
	*/
	if (action == ACTION_SELF) {
		action = ACTION_NONE;
	}

	/*
	**	Check to see if it can enter a transporter.
	*/
	if (action != ACTION_NO_ENTER &&
		House->Is_Ally(object) &&
		House->Is_Player_Control() && ::Dynamic_Cast<TechnoClass const *>(object) != NULL &&
		action != ACTION_ATTACK) {

		TechnoTypeClass const * tclass = object->TClass;
		if (tclass != NULL && tclass->Max_Passengers() > 0) {
			bool try_enter = true;
			if (object->Is_Foot()) {
				FootClass * foot = (FootClass *)object;
				if ((foot->Team != NULL && !foot->Team->Class->IsLoadable) || foot->Locomotion->Is_Moving()) {
					action = ACTION_NO_ENTER;
					try_enter = false;
				}
			}

			if (try_enter) {
				switch (((InfantryClass *)this)->Transmit_Message(RADIO_CAN_LOAD, (TechnoClass*)object)) {
					case RADIO_ROGER:
						action = ACTION_ENTER;
						break;

					case RADIO_NEGATIVE:
						action = ACTION_NO_ENTER;
						break;

					default:
						break;
				}
			}
		}
	}

	if (House->Is_Player_Control() && Class->IsCapture) {
		if (action == ACTION_ATTACK) {
			if (!House->Is_Ally(object) && (
				(object->RTTI == RTTI_BUILDING && ((BuildingClass *)object)->Class->IsCaptureable) )
				) {

					if (Class->IsBomber && object->Considered_Vehicle() == true) {
						return(ACTION_SABOTAGE);
					}

					/*
					**	If we're trying to capture a building, make sure we can get
					**	to it.  Find an adjacent cell that's the same zone as us.
					**	The target circumstance is a naval yard that doesn't touch
					**	the shore - a total island.  In that case, we can't capture
					**	it, so we shouldn't show the action-capture cursor.
					*/
					action = ACTION_CAPTURE;
					if (object->RTTI == RTTI_BUILDING) {
						Cell cell = object->Center_Coord().As_Cell();
						int targzone = Map.Get_Cell_Zone(Get_Target_Cell(), Class->MZone, IsOnBridge);
						Cell const *list = ((BuildingClass *)object)->Class->Occupy_List(false);
						bool found = false;
						while (*list != REFRESH_EOL && !found) {
							Cell newcell = cell + *list++;
							for (FacingType i=FACING_N; i < FACING_COUNT; i++) {
								Cell adjcell = Adjacent_Cell(newcell, i);
								if (Map.Get_Cell_Zone(adjcell, Class->MZone, false) == targzone) {
									found = true;
									break;
								}
							}
						}
						if (!found) {
							action = ACTION_NONE;
						}
					}
			} else {
				if (!Is_Weapon_Equipped()) {
					//action = ACTION_NONE;
				}
			}
		}
	}

	if (action == ACTION_ATTACK) {
		if (!Is_Weapon_Equipped()) {
			action = ACTION_ATTACK_SUPPORT;
		}
	}
	/*
	**	If it doesn't know what to do with the object, then just
	**	say it can't move there.
	*/
	if (House->Is_Player_Control() && action == ACTION_NONE) action = ACTION_NOMOVE;

	return(action);
}


/***********************************************************************************************
 * InfantryClass::Active_Click_With -- Handles action when clicking with infantry soldier.     *
 *                                                                                             *
 *    This routine is called when the player clicks over an object while this infantry soldier *
 *    is selected. Capture attempts are prohibited if the infantry cannot capture. The         *
 *    command might respond if told to sabotage something.                                     *
 *                                                                                             *
 * INPUT:   action   -- The action that is nominally to be performed.                          *
 *                                                                                             *
 *          object   -- The object over which the mouse was clicked.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Active_Click_With(ActionType action, ObjectClass * object, bool is_waypoint)
{
	action = What_Action(object, is_waypoint);

	switch (action) {
		case ACTION_GREPAIR:
		case ACTION_DAMAGE:
		case ACTION_CAPTURE:
			action = ACTION_CAPTURE;
			break;

		case ACTION_HEAL:
			action = ACTION_ATTACK;
			break;

		case ACTION_SABOTAGE:
		case ACTION_ATTACK:
		case ACTION_GUARD_AREA:
		case ACTION_MOVE:
			action = action;
			break;

		default:
			break;
	}

	return(BASECLASS::Active_Click_With(action, object, is_waypoint));
}


/***********************************************************************************************
 * InfantryClass::Full_Name -- Fetches the full name of the infantry unit.                     *
 *                                                                                             *
 *    This routine will return with the full name (as a text number) for this infantry         *
 *    unit. Typically, this is the normal name, but in cases of civilian type survivors from   *
 *    a building explosion, it might be a technician instead. In such a case, the special      *
 *    technician name number is returned instead.                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the full name to use for this infantry unit.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/30/1995 JLB : Created.                                                                 *
 *   10/28/1996 JLB : Spy returns "enemy soldier" text name.                                   *
 *=============================================================================================*/
const char * InfantryClass::Full_Name(void) const
{
	if (IsTechnician) {
		return(Fetch_String(TXT_TECHNICIAN));
	}

	if (Class->IsDisguised && !House->Is_Player_Control() && Rule->Disguise != NULL) {
		return(Rule->Disguise->GivenName);
	}

	return(Class->GivenName);
}


/***********************************************************************************************
 * InfantryClass::Mission_Attack -- Intercept attack mission for special handling.             *
 *                                                                                             *
 *    This routine intercepts the normal attack mission and if an engineer is detected and the *
 *    target is a building, then the engineer will be automatically assigned the capture       *
 *    mission. In other cases, the normal attack logic will proceed.                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the number of game frames to delay before calling this routine again. *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/07/1995 JLB : Created.                                                                 *
 *   04/15/1996 BWG : Engineers can only attack their own house's buildings now.               *
 *   05/29/1996 JLB : Engineers can now damage/capture enemy buildings.                        *
 *=============================================================================================*/
int InfantryClass::Do_MISSION_ATTACK(void)
{
	BuildingClass * building = dynamic_cast<BuildingClass *>(TarCom);
	if ((Class->IsBomber || Has_Ability(ABILITY_C4)) && building != NULL && building->Class->IsRepairable) {
		Assign_Destination(TarCom);
		Assign_Mission(MISSION_SABOTAGE);
		return(1);
	}

	if (Class->IsCapture && TarCom != NULL && TarCom->RTTI == RTTI_BUILDING) {
		Assign_Destination(TarCom);
		Assign_Mission(MISSION_CAPTURE);
		return(1);
	}

	return(BASECLASS::Do_MISSION_ATTACK());
}


/***********************************************************************************************
 * InfantryClass::What_Action -- Determines what action to perform for the cell specified.     *
 *                                                                                             *
 *    This routine will determine what action to perform if the mouse was clicked on the cell  *
 *    specified. This is just a courier function since the lower level classes actually        *
 *    perform the work. The need for this routine at this level is due to the existence of     *
 *    a similarly named function at this level as well.  C++ namespace rules require this      *
 *    function courier to be in place or an error will result.                                 *
 *                                                                                             *
 * INPUT:   cell  -- The cell that the mouse might be clicked upon.                            *
 *                                                                                             *
 * OUTPUT:  Returns with the action that would be given to this infantry unit if the mouse     *
 *          were clicked at the cell specified.                                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType InfantryClass::What_Action(Cell const & cell, bool check_fog, bool disallow_force) const
{
	if (!House->Is_Player_Control()) {
		return(ACTION_NONE);
	}

	ActionType action = BASECLASS::What_Action(cell, check_fog, disallow_force);

	/*
	**	If this is a medic, and the cursor's over a friendly infantryman,
	**	execute an action-attack.
	*/
	if (Combat_Damage() < 0 && House->Is_Player_Control()) {
		if (action == ACTION_ATTACK) {
			action = ACTION_GUARD_AREA;
		}
	}

	if (Class->IsJumpJet && (action == ACTION_MOVE || action == ACTION_NOMOVE) && (Map[cell].Is_Near_Tunnel_NW() || Map[cell].Is_Near_Tunnel_ES())) {
		action = ACTION_NONE;
	} else {
		if (action == ACTION_MOVE && Map[cell].Has_Tunnel()) {
			return(Map[cell].Can_Enter_Tunnel(this) ? ACTION_ENTER_TUNNEL : ACTION_NO_ENTER_TUNNEL);
		}
	}

	CellClass * cellptr = &Map[cell];
	BuildingTypeClass * fogged_building = NULL;
	FoggedObjectClass * fogged_object = NULL;

	if (Scen->Special.IsFogOfWar && check_fog && cellptr->FoggedObjects != NULL) {
		for (int i = 0; i < cellptr->FoggedObjects->Count(); i++) {
			fogged_object = (*cellptr->FoggedObjects)[i];
			if (fogged_object->CanDraw && fogged_object->RTTI == RTTI_BUILDING) {
				if (fogged_object->RTTI == RTTI_BUILDING) {
					fogged_building = (BuildingTypeClass *)fogged_object->Records[0].TypeClass;
				} else {
					fogged_building = NULL;
				}
				break;
			}
		}
	}

	if (Class->IsEngineer && fogged_building != NULL && House->Is_Player_Control()) {

		/*
		**	Engineers may repair a destroyed bridge.
		*/
		if (fogged_building->IsBridgeRepairHut) {
			return(Map.Can_Repair_Bridge(fogged_object->Center_Coord()) ? ACTION_GREPAIR : ACTION_NO_GREPAIR);
		}

		if (fogged_building->IsRepairable) {
			return(ACTION_CAPTURE);
		}
	}

	if (action == ACTION_ATTACK && !Is_Weapon_Equipped()) {
		action = ACTION_ATTACK_SUPPORT;
	}

	if (House->Is_Player_Control() && action == ACTION_NONE && !Class->IsJumpJet) {
		action = ACTION_NOMOVE;
	}

	return(action);
}


/***********************************************************************************************
 * InfantryClass::Class_Of -- Returns the class reference for this object.                     *
 *                                                                                             *
 *    This routine will return a reference to the infantry type class object that describes    *
 *    this infantry's characteristics.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the InfantryTypeClass object associated with this      *
 *          infantry object.                                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectTypeClass const * InfantryClass::Class_Of(void) const
{
	return(Class);
}


/***********************************************************************************************
 * InfantryClass::Read_INI -- Reads units from scenario INI file.                              *
 *                                                                                             *
 *    This routine is used to read all the starting units from the                             *
 *    scenario control INI file. The units are created and placed on the                       *
 *    map by this routine.                                                                     *
 *                                                                                             *
 *    INI entry format:                                                                        *
 *      Housename, Typename, Strength, Cellnum, CellSublocation, Missionname,                  *
 *         Facingnum, Triggername                                                              *
 *                                                                                             *
 * INPUT:   buffer   -- Pointer to the loaded scenario INI file.                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Read_INI(CCINIClass const & ini)
{
	InfantryClass	* infantry;			// Working infantry pointer.
	HousesType		inhouse;			// Infantry house.
	InfantryType	classid;			// Infantry class.
	char			buf[128];
	char			* validation;
	Dir256 			dir;
	TagTypeClass	* tp;

	int len = ini.Entry_Count(INI_Name);
	for (int index = 0; index < len; index++) {
		char const * entry = ini.Get_Entry(INI_Name, index);

		/*
		**	Get an infantry entry
		*/
		ini.Get_String(INI_Name, entry, NULL, buf, sizeof(buf));

		/*
		**	1st token: house name.
		*/
		inhouse = HouseTypeClass::From_Name(strtok(buf, ","));
		if (inhouse != HOUSE_NONE) {
			HouseClass * inhousep = House_From_HousesType(inhouse);
			if (inhousep == NULL) {
				continue;
			}

			/*
			**	2nd token: infantry type name.
			*/
			classid = InfantryTypeClass::From_Name(strtok(NULL, ","));

			if (classid != INFANTRY_NONE) {

				infantry = new InfantryClass(InfantryTypes[classid], inhousep);
				if (infantry != NULL) {

					/*
					**	3rd token: strength.
					*/
					int strength = atoi(strtok(NULL, ","));

					/*
					**	4th token: cell #.
					*/
					int x, y;
					if (NewINIFormat >= 4) {
						x = atoi(strtok(NULL, ","));
						y = atoi(strtok(NULL, ","));
						x = (short)x;
					} else {
						int cellnum = atoi(strtok(NULL, ","));
						x = cellnum % 128;
						y = cellnum / 128;
					}
					Cell cell(x, y);
					Coord coord = cell.As_Coord();

					/*
					**	5th token: cell sub-location.
					*/
					int sub = atoi(strtok(NULL, ","));
					coord = Coord_Whole(coord) + StoppingCoordAbs[sub];

					/*
					**	Fetch the mission and facing.
					*/
					MissionType mission = MissionClass::Mission_From_Name(strtok(NULL, ","));
					validation = strtok(NULL, ",");
					if (validation) {
						dir = (Dir256)atoi(validation);
					} else {
						dir = DIR_N;
					}

					validation = strtok(NULL, ",");
					if (validation) {
						tp = TagTypeClass::From_Name(validation);
						if (tp != NULL) {
							TagClass * tt = Find_Or_Make(tp);
							if (tt != NULL) {
								infantry->Attach_Tag(tt);
							}
						}
					}

					validation = strtok(NULL, ",");
					if (validation) {
						infantry->Veterancy.From_Integer(atoi(validation));
					}

					validation = strtok(NULL, ",");
					if (validation) {
						infantry->Group = atoi(validation);
					}

					validation = strtok(NULL, ",");
					if (validation) {
						infantry->IsOnBridge = atoi(validation) != 0;
						if (infantry->IsOnBridge) {
							coord.Z = Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT;
						} else {
							coord.Z = Map.Get_Height_GL(coord);
						}
					}

					validation = strtok(NULL, ",");
					if (validation) {
						infantry->IsTeamRecruitable = atoi(validation) != 0;
					}

					validation = strtok(NULL, ",");
					if (validation) {
						infantry->IsAutocreateRecruitable = atoi(validation) != 0;
					}

					if (&Map[coord] != &BlubCell && infantry->Unlimbo(coord, dir)) {
						infantry->Strength = infantry->Class->MaxStrength * (strength / 256.0);
						if (infantry->Strength > infantry->Class->MaxStrength-3) infantry->Strength = infantry->Class->MaxStrength;
						if (infantry->Strength < 1) infantry->Strength = 1;
						if (Session.Type == GAME_NORMAL || infantry->House->Is_Human_Player()) {
							infantry->Assign_Mission(mission);
							infantry->Commence();
						} else {
							infantry->Enter_Idle_Mode();
						}
					} else {

						/*
						**	If the infantry could not be unlimboed, then this is a big error.
						**	Delete the infantry.
						*/
						delete infantry;
					}
				}
			}
		}
	}
}


/***********************************************************************************************
 * InfantryClass::Write_INI -- Store the infantry to the INI database.                         *
 *                                                                                             *
 *    This will store all the infantry objects to the INI database specified.                  *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to store the infantry data to.              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/03/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Write_INI(CCINIClass & ini)
{
	/*
	**	First, clear out all existing infantry data from the ini file.
	*/
	ini.Clear(INI_Name);

	/*
	**	Write the infantry data out.
	*/
	for (int index = 0; index < Infantry.Count(); index++) {
		InfantryClass * infantry = Infantry[index];
		if (!infantry->IsInLimbo) {
			char	uname[10];
			char	buf[128];

			sprintf(uname, "%d", index);
			sprintf(buf, "%s,%s,%d,%d,%d,%d,%s,%d,%s,%d,%d,%d,%d,%d",
					(char const *)infantry->House->Class->IniName,
					(char const *)infantry->Class->IniName,
					int(infantry->HealthRatio*256),
					infantry->PositionCell.X,
					infantry->PositionCell.Y,
					CellClass::Spot_Index(infantry->PositionCoord),
					MissionClass::Mission_Name((infantry->Mission == MISSION_NONE) ?
						infantry->MissionQueue : infantry->Mission),
					infantry->PrimaryFacing.Current().As_Dir256(),
					infantry->Tag != NULL ? (char const *)infantry->Tag->Class->IniName : "None",
					infantry->Veterancy.To_Integer(),
					infantry->Group,
					infantry->IsOnBridge,
					infantry->IsTeamRecruitable,
					infantry->IsAutocreateRecruitable
				);
			ini.Put_String(INI_Name, uname, buf);
		}
	}
}


/***********************************************************************************************
 * InfantryClass::Fear_AI -- Process any fear related affects on this infantry.                *
 *                                                                                             *
 *    Use this routine to handle the fear logic for this infantry. It will slowly increase     *
 *    the bravery of the infantry as well as cause it to stand up or lie down as appropriate.  *
 *    It will even handle the special fraidy cat logic for civilian infantry.                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this once per game logic loop per infantry unit.                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Fear_AI(void)
{
	/*
	**	After a time, the infantry will gain courage.
	*/
	if (Fear > 0) {

		if (Class->IsDoggie) {
			if (Fear >= FEAR_PANIC) {
				if (!Locomotion->Is_Moving() && NavCom == NULL) {
					if (Map[(Coord const &)PositionCoord].Land_Type() == LAND_TIBERIUM) {
						Do_Action(DO_LIE_DOWN);
					} else {
						Goto_Tiberium(16, false);
						if (NavCom != NULL) {
							Assign_Target(NULL);
							Assign_Mission(MISSION_MOVE);
							Commence();
						}
					}
				}
			}
			if (!Class->IsFearless) {
				Fear--;
			}
			return;
		}

		if (!Class->IsFearless) {
			Fear--;
		}

		/*
		**	When an armed civilian becomes unafraid, he will then reload
		**	another clip into his pistol.
		*/
		if (Fear == 0 && Ammo == 0 && Is_Weapon_Equipped()) {
			Ammo = Class->MaxAmmo;
		}

		/*
		**	Stand up if brave and lie down if afraid.
		*/
		if (IsProne) {
			if (Fear < FEAR_ANXIOUS) {
				Do_Action(DO_GET_UP);
			}
		} else  {

			/*
			**	Drop to the ground if anxious. Don't drop to the ground while moving
			**	and the special elite flag is active.
			*/
			if (Fear >= FEAR_ANXIOUS && (!House->Is_Human_Player() || ((NavCom == NULL && !Locomotion->Is_Moving())))) {
				Do_Action(DO_LIE_DOWN);
			}
		}
	}

	/*
	**	When in darkness or in doubt,
	**	run in circles, scream, and shout.
	*/
	if (Class->IsFraidyCat && Fear > FEAR_ANXIOUS && !IsFalling && !Locomotion->Is_Moving() && NavCom == NULL) {
		Scatter(COORD_NONE, true);
	}
}


/***********************************************************************************************
 * InfantryClass::Edge_Of_World_AI -- Detects when infantry has left the map.                  *
 *                                                                                             *
 *    This routine will detect when the infantry has left the edge of the world and will       *
 *    delete it as necessary.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the infantry unit deleted by this routine?                               *
 *                                                                                             *
 * WARNINGS:   Be sure the check the return value and if true, abort any further processing    *
 *             for this infantry unit.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Edge_Of_World_AI(void)
{
	/*
	**	Delete this unit if it finds itself off the edge of the map and it is in
	**	guard or other static mission mode.
	*/
	if (Team != NULL && IsLocked) Team->IsLeaveMap = true;

	if (Team == NULL && Mission == MISSION_GUARD && !Map.In_Radar(PositionCoord)) {
		Stun();
		Delete_Me();
		return(true);
	}
	return(false);
}


/// <summary>
/// Handles a thief stealing the vehicle it has been sent after.
/// A thief that gets close enough to an enemy vehicle on its own level captures the
/// vehicle outright and is consumed in the process. Failing that, the thief keeps
/// chasing, re-aiming itself whenever the vehicle wanders away from where it was
/// headed.
/// </summary>
/// <returns>bool; Was the vehicle stolen, and this infantry consumed doing it?</returns>
bool InfantryClass::Theft_AI(void)
{
	if (!Class->IsThief) return(false);
	if (NavCom == NULL) return(false);
	if (NavCom->RTTI != RTTI_UNIT) return(false);

	UnitClass * unit = (UnitClass *)NavCom;
	if (House->Is_Ally(unit)) return(false);

	Coord coord = unit->Destination_Coord();
	int z = coord.Z;
	coord = Center_Coord();

	/*
	 * If the thief is close enough to the vehicle (and on the same level),
	 * then capture the vehicle outright.
	 */
	if (abs(z - coord.Z) < LEVEL_LEPTON_H) {
		if (Distance_To(unit) < CELL_LEPTON / 2) {
			if (unit->Tag != NULL) {
				unit->Tag->Spring(TEVENT_PLAYER_ENTERED, this);
			}
			unit->Transmit_Message(RADIO_OVER_OUT);
			unit->Detach_All(false);
			if (Tag != NULL && Tag->Is_To_Inherit()) {
				unit->Attach_Tag(Tag);
			}
			unit->Captured(House);
			unit->EnteredByInfType = Class->HeapID;
			Delete_Me();
			return(true);
		}
	}

	/*
	 * The vehicle is too far away to capture. Keep chasing it down unless
	 * the thief is already heading toward it.
	 */
	if (Distance_To(unit) < 2 * CELL_LEPTON) {
		if (Coord(Locomotion->Destination()) != COORD_NONE) {
			if (unit->Distance(Coord(Locomotion->Destination())) <= CELL_LEPTON / 2) {
				return(false);
			}
		}
		Assign_Destination(NavCom);
	} else {
		if (Coord(Locomotion->Destination()) != COORD_NONE) {
			if (unit->Destination_Coord().As_Cell() == Coord(Locomotion->Destination()).As_Cell()) {
				return(false);
			}
		}
	}
	Assign_Destination(NavCom);
	return(false);
}


/***********************************************************************************************
 * InfantryClass::Firing_AI -- Handles firing and combat AI for the infantry.                  *
 *                                                                                             *
 *    This will examine the infantry and determine what firing action is required. It will     *
 *    search for targets, starting firing animations, and launch bullets as necessary.         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per infantry per game logic loop.                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Firing_AI(void)
{
	if (TarCom != NULL) {
		int primary = What_Weapon_Should_I_Use(TarCom);

		if (!IsFiring) {
			switch (Can_Fire(TarCom, primary)) {
				case FIRE_ILLEGAL:
					if (Combat_Damage(primary) < 0) {
						ObjectClass * targ = dynamic_cast<ObjectClass *>(TarCom);
						if (Is_Target_Infantry(targ)) {
							if (targ->HealthRatio >= Rule->ConditionGreen) {
								Assign_Target(NULL);
							}
						} else {
							Assign_Target(NULL);
						}
					}
					break;

				case FIRE_CLOAKED:
					Do_Uncloak();
					break;

				case FIRE_OK:
					/*
					**	Start firing animation.
					*/
					if (Is_JumpJet()) {
						Do_Action(DO_FIREFLY);
					} else {
						if (IsProne) {
							Do_Action(DO_FIRE_PRONE);
						} else {
							Do_Action(DO_FIRE_WEAPON);
						}
					}

					IsFiring = true;
					PrimaryFacing.Set(Direction(TarCom));

					/*
					**	If the target is in range, and the NavCom is the same, then just
					**	stop and keep firing.
					*/
					if (TarCom == NavCom) {
						NavCom = NULL;
						Path[0] = FACING_NONE;
					}
					break;
			}
		}

		/*
		**	If in the middle of firing animation, then only
		**	process that. Infantry cannot fire and move simultaneously.
		**	At some point in the firing animation process, a projectile
		**	will be launched. When the required animation frames have
		**	been completed, the firing animation stops.
		*/
		int firestage = Class->FireLaunch;
		if (IsProne) firestage = Class->ProneLaunch;

		if (IsFiring && Fetch_Stage() == firestage) {

			primary = What_Weapon_Should_I_Use(TarCom);
			if (Can_Fire(TarCom, primary) == FIRE_OK) {
				Fire_At(TarCom, primary);
			} else {
				IsFiring = false;
				if (IsProne) {
					Do_Action(DO_PRONE);
				} else {
					Do_Action(DO_STAND_READY);
				}
			}

			const WeaponDataStruct * wdata = Get_Class_Weapon_Data(0);
			if (wdata->Weapon->MaxSpeed < Rule->Incoming) {
				Map[TarCom->Center_Coord()].Incoming(PositionCoord, true);
			}
		}
	} else {
		IsFiring = false;
	}
}


/***********************************************************************************************
 * InfantryClass::Doing_AI -- Handles the animation AI processing.                             *
 *                                                                                             *
 *    Infantry can be in one of many different animation sequences. At the conclusion of each  *
 *    sequence, the infantry will quite likely transition to a new animation state. This       *
 *    routine handles detecting when that trasition should occur and starting the infantry     *
 *    into its new state.                                                                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per infantry unit per game logic loop.              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Doing_AI(void)
{
	if (Doing == DO_NOTHING || Fetch_Stage() >= Class->DoControls[Doing].Count) {
		switch (Doing) {
			default:
				if (ProneStruggleTimer == 0) {
					if (Doing != DO_NOTHING && Class->DoControls[Doing].Facing != FACING_NONE) {
						PrimaryFacing.Set(Class->DoControls[Doing].Facing);
					}

					if (Locomotion->Is_Moving() && Speed > 0.1) {
						if (IsProne) {
							Do_Action(DO_CRAWL, true);
						} else {
							Do_Action(DO_WALK, true);
						}
					} else {
						if (IsProne) {
							Do_Action(DO_PRONE, true);
						} else {
							Do_Action(DO_STAND_READY, true);
						}
					}
				}
				break;

			case DO_STRUGGLE:
				if (ProneStruggleTimer == 0) {
					Do_Action(DO_PRONE, true);
				}
				break;

			case DO_GUN_DEATH:
			case DO_EXPLOSION_DEATH:
			case DO_EXPLOSION2_DEATH:
			case DO_GRENADE_DEATH:
			case DO_FIRE_DEATH:
				if (Fetch_Stage() >= Class->DoControls[Doing].Count) {
					if (!Class->IsDoggie) {
						new AnimClass(Rule->DeadBodies.Pick(Scen->RandomNumber), Center_Coord());
					}
					Delete_Me();
					return;
				}
		}
	}
}


/***********************************************************************************************
 * InfantryClass::Movement_AI -- This routine handles all infantry movement logic.             *
 *                                                                                             *
 *    It examines the infantry state and determines what movement action should be initiated   *
 *    or processed. It handles the actual movement of the infantry as well as any path finding *
 *    or infantry startup logic.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per infantry unit per game logic loop.              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Movement_AI(void)
{
	/*
	**	Special hack check to ensure that infantry will never get stuck in a movement order if
	**	there is no place to go.
	*/
	if (Mission == MISSION_MOVE && NavCom == NULL && !Locomotion->Is_Moving()) {
		Enter_Idle_Mode();
	}

	if (Mission == MISSION_MOVE && NavCom != NULL && !Locomotion->Is_Moving()) {
		Assign_Destination(NavCom);
		Set_Speed(1.0);
	}

	if (NavCom != NULL) {
		Cell cell = NavCom->Destination_Coord().As_Cell();

		/*
		**	Double check to make sure it doesn't have a movement destination into a zone
		**	that it can't travel to. In such a case, abort the movement process by clearing
		**	the navigation computer.
		*/
		if ((!IsZoneCheat || Can_Enter_Cell(Get_Target_Cell_Ptr()) != MOVE_NO) && !Locomotion->Is_Moving() && !IsTethered && NavCom != NULL && IsLocked && !Map.Is_Same_Cell_Zone(Destination_Coord().As_Cell(), cell, Class->MZone, Is_Moving_Onto_Bridge(), Map[cell].IsUnderBridge, Is_Allowed_To_Leave_Map())) {
// hack: if it's tanya, spy, or engineer, let 'em move there anyway.
			if (!Class->IsCapture && Mission != MISSION_ENTER) {
				Assign_Destination(NULL);
			}
		}
	}

	if (Locomotion->Is_Really_Moving_Now()) {
		if (Is_JumpJet()) {
			if (!IsFiring) {
				if (Speed > 0.8) {
					Do_Action(DO_FLY);
				} else {
					Do_Action(DO_HOVER);
				}
			}
		} else {
			if (IsProne) {
				Do_Action(DO_CRAWL);
			} else {
				Do_Action(DO_WALK);
			}
		}
	} else {
		if (Doing == DO_WALK || Doing == DO_FLY || Doing == DO_HOVER) {
			Do_Action(DO_STAND_READY);
		}
		if (Doing == DO_CRAWL) {
			Do_Action(DO_PRONE);
		}
	}
}


/***********************************************************************************************
 * InfantryClass::Get_Image_Data -- Fetches the image data for this infantry unit.             *
 *                                                                                             *
 *    The image data for the infantry differs from normal if this is a spy. A spy always       *
 *    appears like a minigunner to the non-owning players.                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the image data to use for this infantry soldier.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void const * InfantryClass::Get_Image_Data(void) const
{
	if (Doing == DO_STRUGGLE && Rule->WebbedInfantry != NULL) {
		return(Rule->WebbedInfantry->Get_Image_Data());
	}
	if (!IsOwnedByPlayer && Class->IsDisguised && Rule->Disguise != NULL) {
		return(Rule->Disguise->ImageData);
	}
	return(BASECLASS::Get_Image_Data());
}


/***********************************************************************************************
 * InfantryClass::Is_Ready_To_Random_Anima -- Checks to see if it is ready to perform an idle  *
 *                                                                                             *
 *    This routine will examine this infantry and determine if it is allowed and ready to      *
 *    perform an idle animation. The conditions under which idle animations can be performed   *
 *    are restrictive. Hence this routine.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Is this infantry ready to do an idle animation?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/01/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Is_Ready_To_Random_Animate(void) const
{
	/*
	**	See if the base classes (more rudimentary checking) determines that idle animations
	**	cannot occur. If they cannot, then return with the failure code.
	*/
	if (!BASECLASS::Is_Ready_To_Random_Animate()) {
		return(false);
	}

	/*
	**	When the infantry is walking or otherwise engauged in travel, it won't idle animate.
	*/
	if (Locomotion->Is_Moving()) {
		return(false);
	}

	/*
	**	When prone, idle animations cannot occur. This is primarily because there are no prone
	**	idle animations.
	*/
	if (IsProne) {
		return(false);
	}

	/*
	**	When firing, the infantry should not perform any idle animations.
	*/
	if (IsFiring) {
		return(false);
	}

	/*
	**	Only if the infantry is in guard or ready stance is idle animations allowed. This is
	**	because the idle animations start and end with these frames.
	*/
	if (Doing != DO_STAND_GUARD && Doing != DO_STAND_READY) {
		return(false);
	}

	/*
	**	Since no reason was found to indicate it is not a good time to idle
	**	animate, then it must be a good time to do so.
	*/
	return(true);
}


/***********************************************************************************************
 * InfantryClass::Paradrop -- Handles paradropping infantry.                                   *
 *                                                                                             *
 *    This routine will paradrop this soldier at the location specified. It will cause the     *
 *    soldier to hunt if controlled by the computer and to guard if controlledy by the         *
 *    human.                                                                                   *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to paradrop the soldier to.                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the paradrop successful?                                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool InfantryClass::Paradrop(Coord const & coord)
{
	if (BASECLASS::Paradrop(coord)) {
		if (House->Is_Human_Player()) {
			Assign_Mission(MISSION_GUARD);
		} else {
			Assign_Mission(MISSION_HUNT);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * InfantryClass::Set_Occupy_Bit -- Sets the occupy bit cell and bit pos                       *
 *                                                                                             *
 * INPUT:      CELL      - the cell we are setting the bit in                                  *
 *                                                                                             *
 *               int      - the spot index we are setting the bit for                          *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/08/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void InfantryClass::Set_Occupy_Bit(Coord const & coord)
{
	int spot_index = CellClass::Spot_Index(coord);
	CellClass & cell = Map[coord];
	if (Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT <= coord.Z && cell.IsUnderBridge) {

		/*
		**	Set the occupy position for the spot that we passed in
		*/
		cell.BridgeFlag.Composite |= (1 << spot_index);

		/*
		**	Record the type of infantry that now owns the cell
		*/
		cell.BridgeInfType = Owner();
	} else {

		/*
		**	Set the occupy position for the spot that we passed in
		*/
		cell.Flag.Composite |= (1 << spot_index);

		/*
		**	Record the type of infantry that now owns the cell
		*/
		cell.InfType = Owner();
	}
}


/***************************************************************************
 * InfantryClass::Clear_Occupy_Bit -- Clears occupy bit and given cell     *
 *                                                                         *
 * INPUT:                                                                  *
 *                                                                         *
 * OUTPUT:                                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/08/1995 PWG : Created.                                             *
 *=========================================================================*/
void InfantryClass::Clear_Occupy_Bit(Coord const & coord)
{
	int spot_index = CellClass::Spot_Index(coord);
	CellClass &cell = Map[coord];
	int height = Map.Get_Height_GL(coord);

	if (height + BRIDGE_LEPTON_HEIGHT <= coord.Z && cell.IsUnderBridge) {

		/*
		**	Clear the occupy bit for the infantry in that cell
		*/
		cell.BridgeFlag.Composite &= ~(1 << spot_index);

		/*
		**	If he was the last infantry recorded in the cell then
		**	remove the infantry ownership flag.
		*/
		if (!(cell.BridgeFlag.Composite & 0x1C)) {
			cell.BridgeInfType = HOUSE_NONE;
		}

	} else {

		/*
		**	Clear the occupy bit for the infantry in that cell
		*/
		cell.Flag.Composite &= ~(1 << spot_index);

		/*
		**	If he was the last infantry recorded in the cell then
		**	remove the infantry ownership flag.
		*/
		if (!(cell.Flag.Composite & 0x1C)) {
			cell.InfType = HOUSE_NONE;
		}
	}
}


/// <summary>
/// Reads this infantry back in from the save game stream.
/// The soldier is withdrawn from the target tracker under the identity it is carrying now,
/// since the one it is about to be given is the one it was saved with. Post_Load enters it
/// again once that identity has arrived.
/// </summary>
/// <returns>Returns with S_OK if the object was read successfully.</returns>
HRESULT STDMETHODCALLTYPE InfantryClass::Load(IStream * stream)
{
	TargetTracker.Remove_Index(Fetch_ID());
	return(BASECLASS::Load(stream));
}


/// <summary>
/// Lists the members this infantry carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void InfantryClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Class);
	stream.Serialize(Doing);
	stream.Serialize(Comment);
	stream.Serialize(Fear);
	stream.Serialize(IsBerzerk);
	stream.Serialize(IsTechnician);
	stream.Serialize(IsStoked);
	stream.Serialize(IsProne);
	stream.Serialize(IsZoneCheat);
	stream.Serialize(WasSelected);
	stream.Serialize(ProneStruggleTimer);
	stream.Serialize(LookTimer);
}


/// <summary>
/// Enters this infantry in the target tracker under the identity it was saved with.
/// </summary>
void InfantryClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	TargetTracker.Add_Index(Fetch_ID(), this);
}


/// <summary>
/// Stops the walking or crawling animation.
/// The locomotion code calls this routine when the infantry comes to rest, so that the
/// soldier does not carry on marching in place.
/// </summary>
void InfantryClass::Stop_Movement_Animation(void)
{
	if (Doing == DO_CRAWL || Doing == DO_WALK) {
		Doing = DO_NOTHING;
	}
}


/// <summary>
/// Is the infantry ready to be given something new to do?
/// The mission logic uses this routine to find a safe moment to change the infantry's
/// orders. A soldier that is firing, falling, or partway through an animation that
/// must not be cut short will refuse until it is finished.
/// </summary>
/// <returns>bool; Can the infantry be handed a new order right now?</returns>
bool InfantryClass::Ready_To_Commence(void)
{
	if (CurrentMission != MISSION_STICKY && CurrentMission != MISSION_RESCUE) {
		if (!IsFiring && !IsFalling) {
			if (Locomotion->Is_Moving_Now() && Mission != MISSION_GUARD && Mission != MISSION_HUNT && (Mission != MISSION_ATTACK || TarCom)) {
				return(false);
			}
			if (Doing == DO_NOTHING || InfantryClass::MasterDoControls[Doing].Interrupt) {
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Frightens this infantry as much as it can be frightened.
/// Types flagged as fearless, and veterans who have earned the fearless ability, pay
/// this routine no attention at all.
/// </summary>
void InfantryClass::Start_Fear(void)
{
	if (!Class->IsFearless && !Has_Ability(ABILITY_FEARLESS)) {
		Fear = FEAR_MAXIMUM;
	}
}


/// <summary>
/// Clears any fear this infantry is feeling.
/// Use this routine to bring a cowering soldier back to its senses immediately, rather
/// than waiting for its nerve to recover on its own.
/// </summary>
void InfantryClass::Stop_Fear(void)
{
	if (!Class->IsFearless) {
		Fear = FEAR_NONE;
	}
}


/// <summary>
/// Performs one of the infantry's idle animations.
/// This routine is used by the random animation logic to give a soldier that has been
/// standing around too long something to do with itself.
/// </summary>
/// <param name="which">The idle animation variation to perform.</param>
void InfantryClass::Do_Idle(int which)
{
	switch (which) {
		case 0:
		default:
			Do_Action(DO_IDLE1);
			break;
		case 1:
			Do_Action(DO_IDLE2);
			break;
	}
}


/// <summary>
/// Submits this infantry's state to the game checksum.
/// The multiplayer sync check uses this routine to prove that every machine agrees
/// about the object. Only the members that steer the game logic are submitted.
/// </summary>
/// <param name="crc">The checksum engine to submit the values to.</param>
void InfantryClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(Doing);
	crc((int)Comment);
	crc(Fear);
	crc(IsTechnician);
	crc(IsStoked);
	crc(IsProne);
	crc(IsZoneCheat);
	crc(WasSelected);
}


/// <summary>
/// Fetches the coordinate that this infantry's weapon is mounted at.
/// Infantry carry their weapon rather than mount it, so this routine just hands back the
/// same spot the weapon is fired from.
/// </summary>
/// <param name="which">The weapon slot to fetch the mounting for.</param>
/// <returns>Returns with the coordinate the projectile should first appear at.</returns>
Coord InfantryClass::Turret_Coord(int which) const
{
	return(Fire_Coord(which));
}


/// <summary>
/// Fetches the speed this infantry should be moving at.
/// Going prone changes the pace. The types that crawl are slowed down by it, while the
/// types that merely duck actually cover ground faster than they do upright.
/// </summary>
/// <returns>Returns with the movement speed to use.</returns>
int InfantryClass::Current_Speed(void)
{
	int speed = BASECLASS::Current_Speed();
	if (IsProne) {
		if (Class->IsCrawling) {
			speed -= speed / 3;
		} else {
			speed += speed / 2;
		}
	}
	return(speed);
}


/// <summary>
/// Handles the infantry discovering that its way is blocked.
/// The locomotion code calls this routine when the infantry cannot proceed along its
/// path, so that the object gets a chance to react to the obstruction.
/// </summary>
void InfantryClass::On_Movement_Blocked(void)
{
	BASECLASS::On_Movement_Blocked();
	if (NavCom != NULL && Distance(NavCom) < CELL_LEPTON_W * 4 && Is_JumpJet()) {
		/// nothing
	}
}


/// <summary>
/// Switches a grounded jumpjet over to walking.
/// This routine is used when the remaining journey is too short to be worth taking off
/// for. A walk locomotor is piggybacked onto the jumpjet one and carries the infantry
/// the rest of the way to its navigation target.
/// </summary>
/// <returns>bool; Was the walk locomotor engaged?</returns>
bool InfantryClass::JumpJet_To_Walk(void)
{
	int path_length = 0;
	if (NavCom != NULL) {
		for (int i = 0; i < CONQUER_PATH_MAX; i++) {
			if (Path[i] == FACING_NONE) break;
			if (Path[i] == FACING_COUNT) break;
			path_length++;
		}
	}
	if (path_length >= 4) return(false);

	if (Is_JumpJet()) {
		IPiggybackPtr piggy(Locomotion);
		if (piggy != NULL && !piggy->Is_Piggybacking()) {
			ILocomotionPtr walk(CLSID_WalkLocomotion);
			walk->Link_To_Object(this);
			piggy = IPiggybackPtr(walk);
			if (piggy != NULL) {
				Path[0] = FACING_NONE;
				piggy->Begin_Piggyback(Locomotion);
				Locomotion = walk;
				Locomotion->Move_To(NavCom->Center_Coord());
				return(true);
			}
		}
	}

	return(false);
}


/// <summary>
/// Sends this infantry berzerk.
/// Use this routine to drive a soldier out of its mind. A berzerk soldier will attack
/// whatever happens to be nearby, with no regard for who owns it.
/// </summary>
void InfantryClass::Berzerk(void)
{
	IsBerzerk = true;
}


/// <summary>
/// Is this infantry flying under jumpjet power?
/// A jumpjet type can have a walk locomotor piggybacked onto it while it is down on
/// the ground, so the type alone is not enough -- the locomotor actually in control
/// is examined as well.
/// </summary>
/// <returns>bool; Is the jumpjet locomotor the one in control?</returns>
bool InfantryClass::Is_JumpJet(void) const
{
	if (!Class->IsJumpJet) {
		return(false);
	}

	IPersistPtr persist(Locomotion);
	CLSID clsid;
	persist->GetClassID(&clsid);
	return((clsid == CLSID_JumpjetLocomotion) ? true : false);
}


/// <summary>
/// Should the jumpjet infantry fly in order to reach the destination cell?
/// This routine is used to choose between flying and walking for the next leg of a
/// journey. A trip that ground movement can manage easily is walked, so that jumpjets
/// do not take off for every step, but anything ground movement cannot reach -- or
/// would take too long over -- is flown instead. An ion storm keeps the unit down.
/// </summary>
/// <param name="from">The cell the infantry is starting out from.</param>
/// <param name="to">The cell the infantry wants to reach.</param>
/// <returns>bool; Should the trip be made by air?</returns>
bool InfantryClass::Should_JumpJet_Fly(Cell const & from, Cell const & to)
{
	int height = HeightAGL;
	height = height; /// dead code

	if (IonStormClass::Is_Ion_Storm_Active()) {
		return(false);
	}

	if (HeightAGL > 0) {
		return(true);
	}

	if (House->Is_Human_Player()) {
		CellClass * startptr = &Map[from];
		if (startptr != NULL && startptr->Is_Near_Tunnel_NW()) {
			return(false);
		}
	}

	if (Map.Is_Same_Cell_Zone(from, to, MZONE_INFANTRY, IsOnBridge, Map[to].IsUnderBridge, false)) {
		if (from == to) {
			return(false);
		}
		int dist = std::max(abs(to.X - from.X), abs(to.Y - from.Y));
		if (dist == 1) {
			return(false);
		}
		if (dist >= 12 || !Map.In_Local_Radar(from) || !Map.In_Local_Radar(to)) {
			return(true);
		}
		return(Search.Test_Cell_Walk(from, to, this, IsOnBridge, Map[to].IsUnderBridge, MZONE_INFANTRY) > 15);
	}

	return(true);
}


/// <summary>
/// Moves the infantry closer to whatever it is attacking.
/// Engineers are the special case here -- they must physically reach the building they
/// have been aimed at, so they are given it as a movement destination rather than
/// merely closing to weapon range.
/// </summary>
void InfantryClass::Approach_Target(void)
{
	if (TarCom) {
		if (!Class->IsEngineer || NavCom == TarCom)	{
			BASECLASS::Approach_Target();
		}
		else {
			Assign_Destination(TarCom, true);
		}
	}
}


/// <summary>
/// Can this infantry repair and capture buildings?
/// </summary>
/// <returns>bool; Is this soldier an engineer?</returns>
bool InfantryClass::Is_Renovator(void) const
{
	return(Class->IsEngineer);
}


/// <summary>
/// Handles the guard mission for this infantry.
/// Attack dogs left standing about on tiberium will turn to face east and then lie
/// down, which is how they come to be found sleeping in the fields. Everything else
/// about guard duty is left to the normal FootClass handler.
/// </summary>
/// <returns>The delay in game frames before this mission should be processed again.</returns>
int InfantryClass::Do_MISSION_GUARD(void)
{
	if (!IsProne && TarCom == NULL && NavCom == NULL && !PrimaryFacing.Is_Rotating() && Class->IsDoggie && Get_Cell_Ptr()->Land_Type() == LAND_TIBERIUM) {
		if (PrimaryFacing.Current().As_Dir8() != FACING_E) {
			DirType dir(DIR_E);
			Locomotion->Do_Turn(dir);
		} else {
			Do_Action(DO_LIE_DOWN);
		}
	}

	return(BASECLASS::Do_MISSION_GUARD());
}


/// <summary>
/// Fetches the class identifier used to persist this object.
/// The save system records this identifier alongside the object data so that the
/// correct kind of object can be created again when the stream is read back.
/// </summary>
/// <param name="retval">Pointer to the buffer to fill in with the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no buffer was supplied.</returns>
HRESULT STDMETHODCALLTYPE InfantryClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_InfantryClass;
	return(S_OK);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with RTTI_INFANTRY.</returns>
RTTIType InfantryClass::Fetch_RTTI(void) const
{
	return(RTTI_INFANTRY);
}


/// <summary>
/// Handles a player click on a map cell while this infantry is selected.
/// This routine defers to the FootClass handler; infantry have no cell click
/// behavior of their own beyond the normal movement and attack orders.
/// </summary>
/// <returns>bool; Was the click acted upon?</returns>
bool InfantryClass::Active_Click_With(ActionType action, Cell const & cell, bool is_waypoint)
{
	return(BASECLASS::Active_Click_With(action, cell, is_waypoint));
}
