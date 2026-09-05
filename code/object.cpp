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

/* $Header: /CounterStrike/OBJECT.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OBJECT.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : October 6, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   ObjectClass::AI -- Handles generic object AI processing.                                  *
 *   ObjectClass::Active_Click_With -- Dispatches action on the object specified.              *
 *   ObjectClass::Active_Click_With -- Dispatches action on the specified cell.                *
 *   ObjectClass::Attach_Trigger -- Attach specified trigger to object.                        *
 *   ObjectClass::Can_Demolish -- Queries whether this object can be sold back.                *
 *   ObjectClass::Can_Player_Fire -- Can the player give this object an attack mission?        *
 *   ObjectClass::Can_Player_Move -- Can the player give this object a movement mission?       *
 *   ObjectClass::Can_Repair -- Queries whether this object can be repaired.                   *
 *   ObjectClass::Catch_Fire -- Called when animation is attached to this object.              *
 *   ObjectClass::Center_Coord -- Fetches the center coordinate for the object.                *
 *   ObjectClass::Clicked_As_Target -- Triggers target selection animation.                    *
 *   ObjectClass::Debug_Dump -- Displays status of the object class to the mono monitor.       *
 *   ObjectClass::Detach -- Detach the specified target from this object.                      *
 *   ObjectClass::Detach_All -- Removes the object from all tracking systems.                  *
 *   ObjectClass::Do_Shimmer -- Shimmers this object if it is cloaked.                         *
 *   ObjectClass::Docking_Coord -- Fetches the coordinate to dock at this object.              *
 *   ObjectClass::Exit_Coord -- Return with the exit coordinate for this object.               *
 *   ObjectClass::Exit_Object -- Causes the specified object to leave this object.             *
 *   ObjectClass::Fire_Coord -- Fetches the coordinate a projectile will launch from.          *
 *   ObjectClass::Fire_Out -- Informs object that attached animation has finished.             *
 *   ObjectClass::Get_Mission -- Fetches the current mission of this object.                   *
 *   ObjectClass::Get_Ownable -- Fetches the house owner legality options for this object.     *
 *   ObjectClass::Hidden -- Called when this object becomes hidden from the player.            *
 *   ObjectClass::In_Range -- Determines if the coordinate is within weapon range.             *
 *   ObjectClass::In_Which_Layer -- Fetches what layer this object is located in.              *
 *   ObjectClass::Init -- Initializes the basic object system.                                 *
 *   ObjectClass::Limbo -- Brings the object into a state of limbo.                            *
 *   ObjectClass::Look -- Called when this object needs to reveal terrain.                     *
 *   ObjectClass::Mark -- Handles basic marking logic.                                         *
 *   ObjectClass::Mark_For_Redraw -- Marks object and system for redraw.                       *
 *   ObjectClass::Move -- Moves (by force) the object in the desired direction.                *
 *   ObjectClass::Name -- Fetches the identification name of this object.                      *
 *   ObjectClass::ObjectClass -- Default constructor for objects.                              *
 *   ObjectClass::Paradrop -- Unlimbos object in paradrop mode.                                *
 *   ObjectClass::Passive_Click_With -- Right mouse button click process.                      *
 *   ObjectClass::Receive_Message -- Processes an incoming radio message.                      *
 *   ObjectClass::Record_The_Kill -- Records this object as killed by the specified object.    *
 *   ObjectClass::Render -- Displays the object onto the map.                                  *
 *   ObjectClass::Render_Coord -- Fetches the coordinate to draw this object at.               *
 *   ObjectClass::Repair -- Handles object repair control.                                     *
 *   ObjectClass::Revealed -- Reveals this object to the house specified.                      *
 *   ObjectClass::Scatter -- Tries to scatter this object.                                     *
 *   ObjectClass::Select -- Try to make this object the "selected" object.                     *
 *   ObjectClass::Sell_Back -- Sells the object -- if possible.                                *
 *   ObjectClass::Sort_Y -- Returns the coordinate used for display order sorting.             *
 *   ObjectClass::Take_Damage -- Applies damage to the object.                                 *
 *   ObjectClass::Target_Coord -- Fetches the coordinate if this object is a target.           *
 *   ObjectClass::Unlimbo -- Brings the object into the game system.                           *
 *   ObjectClass::Unselect -- This will un-select the object if it was selected.               *
 *   ObjectClass::Value -- Fetches the target value of this object.                            *
 *   ObjectClass::Weapon_Range -- Returns the weapon range for the weapon specified.           *
 *   ObjectClass::What_Action -- Determines what action to perform on specified object.        *
 *   ObjectClass::What_Action -- Returns with the action to perform for this object.           *
 *   ObjectTypeClass::Cost_Of -- Returns the cost to buy this unit.                            *
 *   ObjectTypeClass::Dimensions -- Gets the dimensions of the object in pixels.               *
 *   ObjectTypeClass::Get_Cameo_Data -- Fetches pointer to cameo data for this object type.    *
 *   ObjectTypeClass::Max_Pips -- Fetches the maximum pips allowed for this object.            *
 *   ObjectTypeClass::ObjectTypeClass -- Normal constructor for object type class objects.     *
 *   ObjectTypeClass::Occupy_List -- Returns with simple occupation list for object.           *
 *   ObjectTypeClass::One_Time -- Handles one time processing for object types.                *
 *   ObjectTypeClass::Overlap_List -- Returns a pointer to a simple overlap list.              *
 *   ObjectTypeClass::Time_To_Build -- Fetches the time to construct this object.              *
 *   ObjectTypeClass::Who_Can_Build_Me -- Determine what building can build this object type.  *
 *   ObjectTypeClass::Who_Can_Build_Me -- Finds the factory building that can build this object*
 *   ObjectClass::Get_Image_Data -- Fetches the image data to use for this object.             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "object.h"

#include "_logic.h"
#include "_map.h"
#include "_rtti.h"
#include "_rules.h"
#include "_tactica.h"
#include "alphashp.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "builtype.h"
#include "cell.h"
#include "combat.h"
#include "conquer.h"
#include "foot.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "logic.h"
#include "map.h"
#include "mono.h"
#include "objtype.h"
#include "rules.h"
#include "savestream.h"
#include "session.h"
#include "shapeset.h"
#include "swizzle.h"
#include "tactical.h"
#include "tag.h"
#include "ambient.h"
#include "tagtype.h"
#include "tracker.h"

#include <algorithm>
#include <cassert>


#define	GRAVITY	1.4


/***********************************************************************************************
 * ObjectClass::ObjectClass -- Default constructor for objects.                                *
 *                                                                                             *
 *    This is the default constructor for objects. It is called as an inherent part of the     *
 *    construction process for all the normal game objects instantiated. It serves merely to   *
 *    initialize the object values to a common (default) state.                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Objects always start in a state of limbo. They must be Unlimbo()ed before they  *
 *             can be used.                                                                    *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
ObjectClass::ObjectClass(void) :
	BASECLASS(),
	IsActive(true),
	IsDown(false),
	IsToDamage(false),
	IsToDisplay(false),
	IsInLimbo(true),
	IsSelected(false),
	IsAnimAttached(false),
	IsOnBridge(false),
	IsFalling(false),
	IsToExplode(false),
	Layer(LAYER_NONE),
	IsSubmittedToLayer(false),
	Riser(0,0,0),
	Next(NULL),
	Tag(NULL),
	Strength(255),
	Position(COORD_NONE)
{
	Objects.Add(this);
	ObjectPtrTracker.Add(this);
	AbstractTypePtrTracker.Add(this);
	TagPtrTracker.Add(this);
}


/// <summary>
/// Destroys the object and unlinks it from the game.
/// This routine will remove the object from the deferred deletion list and from every global tracker it
/// was registered with, then mark it inactive so that no stale pointer can revive it.
/// </summary>
ObjectClass::~ObjectClass(void)
{
	AmbientSounds.Detach(this);
	ObjectsToDelete.Delete(this);
	Objects.Delete(this);
	ObjectPtrTracker.Delete(this);
	AbstractTypePtrTracker.Delete(this);
	TagPtrTracker.Delete(this);

	Next = NULL;
	IsActive = false;
}


/// <summary>
/// Fetches the direction from this object to another.
/// The two objects are compared by their center points, so the answer is the facing this
/// object would have to adopt in order to look straight at the other one.
/// </summary>
/// <param name="object">The object to determine the direction to.</param>
/// <returns>Returns with the direction toward the specified object.</returns>
DirType ObjectClass::Direction(AbstractClass const * object) const
{
	return(::Direction(Center_Coord(), object->Center_Coord()));
}


/***********************************************************************************************
 * ObjectClass::Get_Image_Data -- Fetches the image data to use for this object.               *
 *                                                                                             *
 *    This routine will return with a pointer to the image data that should be used when       *
 *    this object is drawn.                                                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the shape data for this object.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void const * ObjectClass::Get_Image_Data(void) const
{
	assert(this != NULL);

	return(Class_Of()->Get_Image_Data());
}


/// <summary>
/// Determines if this object is a mobile one.
/// Use this routine before casting an object down to a FootClass, since only the mobile
/// object types carry a locomotor.
/// </summary>
/// <returns>bool; Is this object derived from FootClass?</returns>
bool ObjectClass::Is_Foot(void) const
{
	assert(this != NULL);

	return(dynamic_cast<FootClass const *>(this)!=NULL);
}


/***********************************************************************************************
 * ObjectClass::AI -- Handles generic object AI processing.                                    *
 *                                                                                             *
 *    This routine is used to handle the AI processing that occurs for all object types.       *
 *    Typically, this isn't much, but there is the concept of falling that all objects can     *
 *    be subjected to (e.g., grenades).                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::AI(void)
{
	assert(this != NULL);

	/*
	**	Falling logic is handled here.
	*/
	if (IsFalling) {
		LayerType layer = In_Which_Layer();

		Height += Riser.Z;
		if (HeightAGL <= 0) {
			HeightAGL = 0;
			IsFalling = false;
			Per_Cell_Process(PCP_END);

			Shorten_Attached_Anims(this);
		}

		if (!IsInLimbo) {
			if (IsAnimAttached) {
				Riser.Z -= 1;
				Riser.Z = std::max<int>(Riser.Z, PARACHUTE_MAX_FALL_RATE);
			} else {
				Riser.Z -= GRAVITY;
				Riser.Z = std::max<int>(Riser.Z, NO_PARACHUTE_MAX_FALL_RATE);
			}

			if (layer != In_Which_Layer()) {
				Map.Submit(this);
			}

			if (!IsFalling) {
				if (IsToExplode && Strength > 0) {
					int damage = Strength;
					Take_Damage(damage, 0, Rule->C4Warhead, NULL, true, true);
				}

				if (RTTI == RTTI_ANIM) {
					AnimClass * anim = (AnimClass *)this;
					if (anim->Class->IsFlamingGuy) {
						anim->IsFlamingGuyEnd = true;
						anim->Set_Rate(1);
						anim->Set_Stage(FACING_COUNT * anim->Class->RunningFrames + 1);
						if (Map[Get_Coord()].Land_Type() == LAND_WATER) {
							new AnimClass(Rule->SplashList[0], Get_Coord() + Coord(0, 0, 3));
						}
					}
				}
			}
		} else {
			Map.Remove(this);
		}
	}
}


/// <summary>
/// Starts this object falling out of the sky.
/// This routine is used when an object loses whatever was holding it up. The object is
/// resubmitted to the map as an airborne one, releasing the cells it was standing on, and is
/// marked to explode when it finally hits the ground.
/// </summary>
void ObjectClass::Fall_From_Height(void)
{
	assert(this != NULL);

	IsFalling = true;
	IsToExplode = true;

	Mark(MARK_UP);
	Map.Remove(this);
	IsOnBridge = false;
	Map.Submit(this);
	Mark(MARK_DOWN);

	if (Is_Foot()) {
		FootClass * foot = (FootClass *)this;
		if (!foot->Locomotion->Is_Moving()) {
			foot->Clear_Occupy_Bit(Get_Coord());
		} else {
			foot->Locomotion->Mark_All_Occupation_Bits(MARK_UP);
		}
	}
}


/***********************************************************************************************
 * ObjectClass::What_Action -- Determines what action to perform on specified object.          *
 *                                                                                             *
 *    This routine will return that action that this object could perform if the mouse were    *
 *    clicked over the object specified.                                                       *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object to check this object against when determining    *
 *                      the action to perform.                                                 *
 *                                                                                             *
 * OUTPUT:  It returns that action that will be performed if the mouse were clicked over the   *
 *          object. Since non-derived objects cannot do anything, and cannot even be           *
 *          instantiated, this routine will always return ACTION_NONE.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType ObjectClass::What_Action(ObjectClass const *, bool) const
{
	assert(this != NULL);

	return(ACTION_NONE);
}


/***********************************************************************************************
 * ObjectClass::What_Action -- Returns with the action to perform for this object.             *
 *                                                                                             *
 *    This routine is called when information on a potential action if the mouse were clicked  *
 *    on the cell specified. This routine merely serves as a virtual placeholder so that       *
 *    object types that can actually perform some action will override this routine to provide *
 *    true functionality.                                                                      *
 *                                                                                             *
 * INPUT:   cell  -- The cell that the mouse is over and might be clicked on.                  *
 *                                                                                             *
 * OUTPUT:  Returns with the action that this object would try to perform if the mouse were    *
 *          clicked. Since objects at this level have no ability to do anything, this routine  *
 *          will always returns ACTION_NONE unless it is overridden.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
ActionType ObjectClass::What_Action(Cell const &, bool, bool) const
{
	assert(this != NULL);

	return(ACTION_NONE);
}


/***********************************************************************************************
 * ObjectClass::In_Which_Layer -- Fetches what layer this object is located in.                *
 *                                                                                             *
 *    The default layer for object location is the LAYER_GROUND. Aircraft will override this   *
 *    routine and make adjustments as necessary according to the aircraft's altitude.          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the layer that this object is located in.                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
LayerType ObjectClass::In_Which_Layer(void) const
{
	assert(this != NULL);

	if (HeightAGL < int(Rule->FlightLevel * 0.6)) {
		return(LAYER_GROUND);
	}
	return(LAYER_TOP);
}


/***********************************************************************************************
 * ObjectClass::Get_Ownable -- Fetches the house owner legality options for this object.       *
 *                                                                                             *
 *    This routine will return the ownable bits for this object. Objects at this level can't   *
 *    really be owned by anyone, but return the full spectrum of legality just to be safe.     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the ownable flags (as a combined bitfield) for this object.           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectClass::Get_Ownable(void) const
{
	assert(this != NULL);

	return(INT_MAX); /// All bits (owners), except for the highest bit, because 0xFFFFFFFF == -1 == no one
}


/***********************************************************************************************
 * ObjectClass::Can_Repair -- Queries whether this object can be repaired.                     *
 *                                                                                             *
 *    Most objects cannot be repaired. This routine defaults to returning "false", but is      *
 *    overridden by derived functions defined by object types that can support repair.         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Can this object be repaired?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Can_Repair(void) const
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Can_Demolish -- Queries whether this object can be sold back.                  *
 *                                                                                             *
 *    This routine is used to determine if this object can be sold. Most objects cannot be     *
 *    but for those objects that can, this routine will be overridden as necessary.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Can this object be sold back? Typically, the answer is no.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Can_Demolish(void) const
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Can_Player_Fire -- Can the player give this object an attack mission?          *
 *                                                                                             *
 *    This routine is used to determine if attacking is an option under player control with    *
 *    respect to this unit. This routine will be overridden as necessary for those objects     *
 *    that have the ability to attack.                                                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Can this object be given an attack order by the player?                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Can_Player_Fire(void) const
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Can_Player_Move -- Can the player give this object a movement mission?         *
 *                                                                                             *
 *    This routine is used to determine if the player has the ability to command this object   *
 *    with a movement mission. This routine will be overridden as necessary to support this    *
 *    ability.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Can this object be given a movement mission by the player?                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/13/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Can_Player_Move(void) const
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Record_The_Kill -- Records this object as killed by the specified object.      *
 *                                                                                             *
 *    This routine is called when this object is killed. If the source of the death is known,  *
 *    then a pointer to the responsible object is provided as a parameter.                     *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the cause of this unit's death.                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Record_The_Kill(TechnoClass * )
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Do_Shimmer -- Shimmers this object if it is cloaked.                           *
 *                                                                                             *
 *    When an object is cloaked, there are several conditions that would cause it to shimmer   *
 *    and thus reveal itself. When such a condition arrises, this function is called. If the   *
 *    object is cloaked, then it will shimmer. At this derivation level, cloaking is           *
 *    undefined. Objects that can cloak will override this function as necessary.              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Do_Shimmer(void)
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Exit_Object -- Causes the specified object to leave this object.               *
 *                                                                                             *
 *    This routine is called, typically, by a transport building type that requires an object  *
 *    to leave it. This routine will place the object at a suitable location or return         *
 *    a value indicating why not.                                                              *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that wishes to leave this object.                *
 *                                                                                             *
 * OUTPUT:  Returns the success value of the attempt:                                          *
 *             0: Object could not be placed -- ever                                           *
 *             1: Object placement is temporarily delayed -- try again later.                  *
 *             2: Object placement proceeded normally                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectClass::Exit_Object(TechnoClass *)
{
	assert(this != NULL);

	return(0);
}


/***********************************************************************************************
 * ObjectClass::Hidden -- Called when this object becomes hidden from the player.              *
 *                                                                                             *
 *    This routine is called when the object becomes hidden from the player. It can result in  *
 *    lost targeting and tracking abilities with respect to the hidden object.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Hidden(void)
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Look -- Called when this object needs to reveal terrain.                       *
 *                                                                                             *
 *    This routine is called when the object needs to look around the terrain. For player      *
 *    owned objects, the terrain is revealed. For non-player objects, not effect occurs.       *
 *                                                                                             *
 * INPUT:   incremental -- If true, then the looking algorithm will only examine the edges     *
 *                         of the sight range. This is more efficient and work well if the     *
 *                         object has only moved one cell since the last time it has performed *
 *                         the look operation.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This can be a time consuming operation. Call only when necessary.               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Look(bool, bool)
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Active_Click_With -- Dispatches action on the object specified.                *
 *                                                                                             *
 *    This routine is called when this object is selected and the mouse was clicked on the     *
 *    tactical map. An action is required from the object. The object that the mouse was       *
 *    over and the tentative action to perform are provided as parameters.                     *
 *                                                                                             *
 * INPUT:   action   -- The requested action to perform with the object specified.             *
 *                                                                                             *
 *          object   -- The object that the action should be performed on. This object is      *
 *                      what the mouse was over when the click occurred.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Active_Click_With(ActionType , ObjectClass *, bool)
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Active_Click_With -- Dispatches action on the specified cell.                  *
 *                                                                                             *
 *    This routine will dispatch the action requested upon the cell specified. It is called    *
 *    when the mouse is clicked over a cell while this object is selected.                     *
 *                                                                                             *
 * INPUT:   action   -- The action to perform.                                                 *
 *                                                                                             *
 *          cell     -- The location (cell) to perform this action upon.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Active_Click_With(ActionType, Cell const &, bool)
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Clicked_As_Target -- Triggers target selection animation.                      *
 *                                                                                             *
 *    This routine is called when this object is the target of some player click action.       *
 *    For more sophisticated object, this will trigger the object to begin flashing a few      *
 *    times. At this level, no action is performed.                                            *
 *                                                                                             *
 * INPUT:   flashes  -- The requested number of times to flash this object.                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Clicked_As_Target(int)
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::In_Range -- Determines if the coordinate is within weapon range.               *
 *                                                                                             *
 *    This routine will determine if the specified coordinate is within weapon range.          *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to check to see if it is within weapon range.              *
 *                                                                                             *
 *          which -- The weapon to check against.                                              *
 *                   0: primary weapon                                                         *
 *                   1: secondary weapon                                                       *
 *                                                                                             *
 * OUTPUT:  bool; Is the specified coordinate within weapon range for the weapon type          *
 *                specified?                                                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::In_Range(Coord const & , int) const
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Weapon_Range -- Returns the weapon range for the weapon specified.             *
 *                                                                                             *
 *    This routine will return the weapon range according to the type of weapon specified.     *
 *                                                                                             *
 * INPUT:   which -- The weapon to fetch the range from.                                       *
 *                   0: primary weapon                                                         *
 *                   1: secondary weapon                                                       *
 *                                                                                             *
 * OUTPUT:  Returns with the range (in leptons) of the weapon specified.                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectClass::Weapon_Range(int) const
{
	assert(this != NULL);

	return(0);
}


/***********************************************************************************************
 * ObjectClass::Scatter -- Tries to scatter this object.                                       *
 *                                                                                             *
 *    This routine is used when the object should scatter from its current location. It        *
 *    applies to units that have the ability to move.                                          *
 *                                                                                             *
 * INPUT:   coord -- The source of the threat that is causing the scatter.                     *
 *                                                                                             *
 *          forced-- Whether this scatter attempt is serious and scattering should occur       *
 *                   regardless of what is doing now.                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This may or may not cause the object to scatter. It is merely a request to the  *
 *             object that it would be good if it were to scatter.                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Scatter(Coord const &, bool, bool)
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Catch_Fire -- Called when animation is attached to this object.                *
 *                                                                                             *
 *    This routine is called when an animation is attached to this object. It might be a       *
 *    fire animation (hence the name), but it might also be smoke or any other animation       *
 *    as well.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the object caught on fire by this routine? Actually, this is really      *
 *                the answer to this question; "Is this animation attaching to this object     *
 *                that doesn't already have an animation attached?"                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Catch_Fire(void)
{
	assert(this != NULL);

	return(false);
}


/***********************************************************************************************
 * ObjectClass::Fire_Out -- Informs object that attached animation has finished.               *
 *                                                                                             *
 *    This routine is called if there is an attached animation on this object and that         *
 *    animation has finished. Typically, this is necessary for when trees are on fire.         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Fire_Out(void)
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Value -- Fetches the target value of this object.                              *
 *                                                                                             *
 *    This routine will return the target value of this object. The higher the number, the     *
 *    better the object will be as a target. This routine is called when searching for         *
 *    targets. Generic objects have no target potential, and this routine returns zero to      *
 *    reflect that. Other object types will override this routine to return the appropriate    *
 *    target value.                                                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the value of this object as a target. Higher values mean better       *
 *          target.                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectClass::Value(void) const
{
	assert(this != NULL);

	return(0);
}


/***********************************************************************************************
 * ObjectClass::Get_Mission -- Fetches the current mission of this object.                     *
 *                                                                                             *
 *    Generic objects don't have a mission, so this routine will just return MISSION_NONE.     *
 *    However, techno objects do have a mission and this routine is overloaded to handle       *
 *    those objects in order to return the correct mission value.                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current mission being followed by this object.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
MissionType ObjectClass::Get_Mission(void) const
{
	assert(this != NULL);

	return(MISSION_NONE);
}


/***********************************************************************************************
 * ObjectClass::Repair -- Handles object repair control.                                       *
 *                                                                                             *
 *    This routine will control object repair mode. At the object level, no repair is          *
 *    possible, so it is expected that any object that can repair will override this function  *
 *    as necessary.                                                                            *
 *                                                                                             *
 * INPUT:   control  -- The repair control parameter.                                          *
 *                      0  = turn repair off                                                   *
 *                      1  = turn repair on                                                    *
 *                      -1 = toggle repair state                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/24/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Repair(int )
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Sell_Back -- Sells the object -- if possible.                                  *
 *                                                                                             *
 *    This routine is called to sell back the object. Override this routine for the more       *
 *    sophisticated objects that can actually be sold back. Normal objects can't be sold and   *
 *    this routine does nothing as a consequence.                                              *
 *                                                                                             *
 * INPUT:   control  -- How to control the sell state of this object.                          *
 *                      0  = stop selling.                                                     *
 *                      1  = start selling.                                                    *
 *                      -1 = toggle selling state.                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Sell_Back(int )
{
	assert(this != NULL);
}


/***********************************************************************************************
 * ObjectClass::Move -- Moves (by force) the object in the desired direction.                  *
 *                                                                                             *
 *    This routine will instantly move the object one cell in the specified direction. It      *
 *    moves the object by force. This is typically ONLY used by the scenario editor            *
 *    process.                                                                                 *
 *                                                                                             *
 * INPUT:   facing   -- The direction to move the object.                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Naturally, this can cause illegal placement situations -- use with caution.     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Move(FacingType facing)
{
	assert(this != NULL);

	Mark(MARK_UP);
	Coord coord = Adjacent_Cell(PositionCoord, facing);
	if (Can_Enter_Cell(&Map[coord], facing) == MOVE_OK) {
		PositionCoord = coord;
	}
	Mark(MARK_DOWN);
}


/***********************************************************************************************
 * ObjectClass::Unselect -- This will un-select the object if it was selected.                 *
 *                                                                                             *
 *    This routine brings a currently selected object into an unselected state. This is        *
 *    needed when another object becomes selected as well as if the object is destroyed.       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Unselect(void)
{
	assert(this != NULL);

	if (IsSelected) {

		CurrentObject.Delete(this);

		IsSelected = false;

		if (Map.Object_To_Follow() == this) {
			Map.Set_To_Follow(NULL);
		}
	}
}


/***********************************************************************************************
 * ObjectClass::Select -- Try to make this object the "selected" object.                       *
 *                                                                                             *
 *    This routine is used to make this object into the one that is "selected". A selected     *
 *    object usually displays a floating bar graph and is available to be given orders from    *
 *    the player's I/O.                                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1994 JLB : Created.                                                                 *
 *   06/12/1995 JLB : Cannot select a loaner object.                                           *
 *   07/23/1995 JLB : Adds to head or tail depending on leader type flag.                      *
 *=============================================================================================*/
bool ObjectClass::Select(void)
{
	assert(this != NULL);

	if (!Debug_Map && (IsInLimbo || IsSelected || !Class_Of()->IsSelectable)) {
		return(false);
	}

	TechnoClass * techno = Dynamic_Cast<TechnoClass *>(this);
	if (!Debug_Map && Can_Player_Move() && techno && techno->IsALoaner) {
		return(false);
	}

	/*
	**	Don't allow selection of object when in building placement mode.
	*/
	if (Map.PendingObject) {
		return(false);
	}

	/*
	**	If selecting an object of a different house than the player's, make sure that
	**	the entire selection list is cleared.
	*/
	if (CurrentObject.Count() > 0) {
		HouseClass * tryhptr = Owner_HouseClass();
		HouseClass * oldhptr = CurrentObject[0]->Owner_HouseClass();
		if (oldhptr->Is_Player_Control() != tryhptr->Is_Player_Control() || !oldhptr->Is_Player_Control()) {
			Unselect_All();
		}
	}

	if (Is_Techno() && ((TechnoTypeClass *)Class_Of())->IsLeader) {
		CurrentObject.Add_Head(this);
	} else {
		CurrentObject.Add(this);
	}

	IsSelected = true;

	return(true);
}


/// <summary>
/// Fetches the screen area that this object currently covers.
/// Unlike the render rectangle, this describes only the artwork as it stands right now, and
/// an object that has scrolled out of the tactical view is reported as covering nothing.
/// </summary>
/// <returns>Returns with the visible rectangle, relative to the tactical map. If the object
/// is off screen or has no artwork, RECT_NONE is returned.</returns>
Rect ObjectClass::Get_Visual_Rect(void) const
{
	assert(this != NULL);

	Point2D point;
	TacticalMap->Coord_To_Pixel(Render_Coord(), point);

	ShapeSet const * shape = (ShapeSet const *)Get_Image_Data();
	if (shape == NULL) {
		return(RECT_NONE);
	}

	Rect shape_rect = shape->Get_Rect(0);
	Rect visual_rect(
		point.X + shape_rect.X - shape->Get_Width()/2,
		point.Y + shape_rect.Y - shape->Get_Height()/2,
		shape_rect.Width,
		shape_rect.Height);

	if (TacticalRect.Is_Overlapping(visual_rect + TacticalRect.TopLeft)) {
		return(visual_rect);
	}

	return(RECT_NONE);
}


/// <summary>
/// Fetches the screen area that this object could draw into.
/// The area covers every frame the object might present, including its shadow and any
/// building bib, so it is large enough to be registered as a dirty area whenever the object
/// needs redrawing.
/// </summary>
/// <returns>Returns with the redraw rectangle, relative to the tactical map. If the object
/// has no artwork, RECT_NONE is returned.</returns>
Rect ObjectClass::Get_Render_Rect(void)
{
	assert(this != NULL);

	Point2D drawpoint;
	TacticalMap->Coord_To_Pixel(Render_Coord(), drawpoint);

	ShapeSet const * sdata = (ShapeSet const *)Class_Of()->Get_Image_Data();
	if (sdata == NULL) {
		return(RECT_NONE);
	}
	int frame = 0;
	BuildingClass * bptr = (BuildingClass *)this;
	if (RTTI == RTTI_BUILDING && bptr->Class->IsFirestormWall) {
		frame = 15;
	}

	Rect rect1 = sdata->Get_Rect(frame);
	Rect rect2 = sdata->Get_Rect(frame + (sdata->Get_Count() / 2));

	int width = sdata->Get_Width();
	int height = sdata->Get_Height();

	Rect a = Union(rect1, rect2);

	if (RTTI == RTTI_BUILDING && bptr->Class->BibShape) {
		Rect rect2 = bptr->Class->BibShape->Get_Rect(0);
		a = Union(a, rect2);
	}

	int x = drawpoint.X + a.X - width / 2;
	int y = drawpoint.Y + a.Y - height / 2;
	return(Rect(x, y, a.Width, a.Height));
}


/***********************************************************************************************
 * ObjectClass::Render -- Displays the object onto the map.                                    *
 *                                                                                             *
 *    This routine will determine the location of the object and if it is roughly on the       *
 *    visible screen, it will display it. Not displaying objects that are not on the screen    *
 *    will save valuable time.                                                                 *
 *                                                                                             *
 * INPUT:   bool; Should the render be forced regardless of whether the object is flagged to   *
 *                be redrawn?                                                                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the draw code called for this object?                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Render(Rect & cliprect, bool forced, bool extras_only) const
{
	assert(this != NULL);

	Point2D point;

	if (Debug_Map || !MainWindow || (forced || IsToDisplay) && !IsInLimbo) {
		IsToDisplay = false;

		if (TacticalMap->Coord_To_Pixel(Render_Coord(), point) || RTTI == RTTI_PARTICLESYSTEM) {

			cliprect = Intersect(cliprect, TacticalRect);

			if (cliprect.X > TacticalRect.X) {
				point.X += TacticalRect.X - cliprect.X;
			}

			if (cliprect.Y > TacticalRect.Y) {
				point.Y += TacticalRect.Y - cliprect.Y;
			}

			/*
			**	Draw the object itself
			*/
			Draw_It(point, cliprect);

			return(true);
		}
	}
	return(false);
}


#ifdef _DEBUG
/***********************************************************************************************
 * ObjectClass::Debug_Dump -- Displays status of the object class to the mono monitor.         *
 *                                                                                             *
 *    This routine is used to display the current status of the object class to the mono       *
 *    monitor.                                                                                 *
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
void ObjectClass::Debug_Dump(MonoClass * mono) const
{
	assert(this != NULL);

	mono->Set_Cursor(1, 1);mono->Printf("%-18.18s", Full_Name());
	if (Next != NULL) {
		mono->Set_Cursor(20, 5);mono->Printf("%08X", Next);
	}
	if (Tag != NULL) {
		mono->Text_Print((char const *)Tag->Class->IniName, 11, 3);
	}
	mono->Set_Cursor(34, 1);mono->Printf("%3d", Strength);

	mono->Fill_Attrib(1, 13, 12, 1, IsDown ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 14, 12, 1, IsToDamage ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 15, 12, 1, IsToDisplay ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 16, 12, 1, IsInLimbo ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(1, 17, 12, 1, IsSelected ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Fill_Attrib(14, 13, 12, 1, IsAnimAttached ? MonoClass::INVERSE : MonoClass::NORMAL);
	mono->Set_Cursor(23, 14);mono->Printf("%d", Riser);
	mono->Fill_Attrib(14, 12, 14, 1, IsFalling ? MonoClass::INVERSE : MonoClass::NORMAL);

	BASECLASS::Debug_Dump(mono);
}
#endif


/***********************************************************************************************
 * ObjectClass::Mark_For_Redraw -- Marks object and system for redraw.                         *
 *                                                                                             *
 *    This routine will mark the object and inform the display system                          *
 *    that appropriate rendering is needed. Whenever it is determined                          *
 *    that an object needs to be redrawn, call this routine.                                   *
 *                                                                                             *
 * INPUT:      none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   This is a subordinate function to the function Mark(). If an object needs to    *
 *             be redrawn it is probably better to call the function Mark(MARK_CHANGE) rather  *
 *             than this function. This function does not inform the map system that           *
 *             overlapping objects are to be redrawn and thus unless you are really sure that  *
 *             this routine should be called, don't.                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/08/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Flags map and flags unit only.                                           *
 *=============================================================================================*/
void ObjectClass::Mark_For_Redraw(void)
{
	assert(this != NULL);

	if (!IsToDisplay) {
		IsToDisplay = true;

		/*
		**	This tells the map rendering logic to "go through the motions" and call the
		**	rendering function. In the rendering function, it will sort out what gets
		**	rendered and what doesn't.
		*/
		Map.Flag_To_Redraw();
	}
}


/***********************************************************************************************
 * ObjectClass::Limbo -- Brings the object into a state of limbo.                              *
 *                                                                                             *
 *    An object brought into a state of limbo by this routine can be safely deleted. This      *
 *    routine will remove the object from all game lists and tracking systems. It is called    *
 *    prior to deleting the object or placing the object "on ice".                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the object successfully placed in limbo?                                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Limbo(void)
{
	assert(this != NULL);

	if (GameActive && !IsInLimbo) {

		Unselect();

		Detach_All();
		AmbientSounds.Detach(this);

		if (RTTI != RTTI_BUILDING || ((BuildingClass *)this)->Class->ToTile == NULL) {
			Mark(MARK_UP);
		}

		/*
		**	Remove the object from the appropriate display list.
		*/
		Map.Remove(this);

		/*
		**	Remove the object from the logic processing list.
		*/
		if (Class_Of() != NULL && Class_Of()->IsSentient && (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH || Fetch_ID() != -2)) {
			Logic.Remove(this);
		}

		if (Class_Of() != NULL && Class_Of()->AlphaImageData != NULL) {

			Point2D point;
			Coord coord = Center_Coord();
			TacticalMap->Coord_To_Pixel(coord, point);

			ShapeSet const * alpha = (ShapeSet const *)Class_Of()->AlphaImageData;
			point -= Point2D(alpha->Get_Width()/2, alpha->Get_Height()/2);

			TacticalMap->Register_Dirty_Area(Rect(point, alpha->Get_Width(), alpha->Get_Height()), true);
		}

		Hidden();
		IsInLimbo = true;
		IsToDisplay = false;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * ObjectClass::Unlimbo -- Brings the object into the game system.                             *
 *                                                                                             *
 *    This routine will place the object into the game tracking and display systems. It is     *
 *    called as a consequence of creating the object. Every game object must be unlimboed at   *
 *    some point.                                                                              *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to place the object into the game system.                  *
 *                                                                                             *
 *          dir (optional) -- initial facing direction for this object                         *
 *                                                                                             *
 * OUTPUT:  bool; Was the game object successfully unlimboed?                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *   12/23/1994 JLB : Sets object strength.                                                    *
 *=============================================================================================*/
bool ObjectClass::Unlimbo(Coord const & coord, Dir256 )
{
	assert(this != NULL);

	if (GameActive && IsInLimbo && !IsDown) {
		if (ScenarioInit || Can_Enter_Cell(&Map[coord], FACING_NONE, -1, NULL, false) == MOVE_OK) {
			IsInLimbo = false;
			IsToDisplay = false;

			ObjectTypeClass const * objclass = Class_Of();
			Coord ucoord = coord;
			if (objclass != NULL) {
				ucoord = objclass->Coord_Fixup(coord);
			}
			PositionCoord = ucoord;

			if (Mark(MARK_DOWN)) {
				if (IsActive) {

					/*
					**	Add the object to the appropriate map layer. This layer is used
					**	for rendering purposes.
					*/
					if (In_Which_Layer() != LAYER_NONE) {
						Map.Submit(this);
					}

					if (objclass != NULL) {

						if (objclass->IsSentient && (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH || Fetch_ID() != -2)) {
							Logic.Submit(this);
						}

						if (objclass->AlphaImageData != NULL) {

							Point2D point;
							Coord coord = Center_Coord();
							TacticalMap->Coord_To_Pixel(coord, point);
							point += Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);

							ShapeSet const * alpha = (ShapeSet const *)Class_Of()->AlphaImageData;
							point -= Point2D(alpha->Get_Width()/2, alpha->Get_Height()/2);

							new AlphaShapeClass(this, point.X, point.Y);

							if (!ScenarioInit) {
								Point2D dpoint = point - Point2D(TacticalMap->TacPixelX, TacticalMap->TacPixelY);
								TacticalMap->Register_Dirty_Area(Rect(dpoint, alpha->Get_Width(), alpha->Get_Height()), true);
							}
						}
					}

					if (dynamic_cast<TechnoClass *>(this) != NULL && objclass != NULL) {
						Cell const * list = Class_Of()->Occupy_List();
						if (list != NULL) {
							while (*list != REFRESH_EOL) {
								Map[*list + coord.As_Cell()].Trigger_Veins();
								list++;
							}
						}
					}
				}
				return(true);
			} else {
				IsInLimbo = true;
			}
		}
	}
	return(false);
}


/***********************************************************************************************
 * ObjectClass::Detach -- Detach the specified target from this object.                        *
 *                                                                                             *
 *    This routine is called when the object (as specified) is to be removed from the game     *
 *    engine and thus, all references to it must be severed. Typically, the only thing         *
 *    checked for at this level is the attached trigger.                                       *
 *                                                                                             *
 * INPUT:   target   -- The target that will be removed from the game system.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void ObjectClass::Detach(AbstractClass const * target, bool all)
{
	if (Tag == target) {
		if (Tag) {
			Tag->AttachCount--;
			Tag = NULL;
		}
	}

	if (all) {
		if (target == Next) {
			if (target) {
				Next = Next->Next;
			}
		}
	}
}


/***********************************************************************************************
 * ObjectClass::Detach_All -- Removes the object from all tracking systems.                    *
 *                                                                                             *
 *    This routine will take the object and see that it is removed from all miscellaneous      *
 *    tracking systems in the game. This operation is vital when deleting an object. It is     *
 *    necessary so that when the object is removed from the game, existing game objects won't  *
 *    be referencing a now invalid game object. This typically affects the targeting           *
 *    and navigation computers of other game objects.                                          *
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
void ObjectClass::Detach_All(bool all)
{
	assert(this != NULL);

	HouseClass * owner = Owner_HouseClass();
	bool sensed = false;

	/*
	**	Unselect this object if it was selected.
	*/
	bool unselect = false;

	if (all) {
		unselect = true;
	} else {
		if (Is_Techno()) {
			if (((TechnoClass *)this)->Is_Sensed_By_Player()) {
				sensed = true;
			}
		}
		if (!(owner != NULL && (owner->Is_Player_Control() || sensed))) {
			unselect = true;
		}
	}

	if (unselect) {
		Unselect();
	}

	if (Map.Object_To_Follow() == this) {
		Map.Set_To_Follow(NULL);
	}

	/*
	**	Remove from targeting computers.
	*/
	Detach_This_From_All(this, all);
}


/***********************************************************************************************
 * ObjectClass::Receive_Message -- Processes an incoming radio message.                        *
 *                                                                                             *
 *    Any radio message received that applies to objects in general are handled by this        *
 *    routine. Typically, this is the "redraw" message, which occurs when another object is    *
 *    loading or unloading and thus overlapping.                                               *
 *                                                                                             *
 * INPUT:   message  -- The message received.                                                  *
 *                                                                                             *
 * OUTPUT:  Returns with the appropriate radio response. If the message was recognized, then   *
 *          RADIO_ROGER is returned, otherwise, just RADIO_STATIC is returned.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
RadioMessageType ObjectClass::Receive_Message(RadioClass *, RadioMessageType message, int & )
{
	assert(this != NULL);

	switch (message) {

		/*
		**	This message serves as a rendering convenience. It lets the system
		**	know that there might be a visual conflict and the unit in radio
		**	contact should be redrawn. This typically occurs when a vehicle
		**	is being unloaded from a hover lander.
		*/
		case RADIO_REDRAW:
			Mark(MARK_CHANGE);
			return(RADIO_ROGER);

		default:
			break;
	}
	return(RADIO_STATIC);
}


/***********************************************************************************************
 * ObjectClass::Take_Damage -- Applies damage to the object.                                   *
 *                                                                                             *
 *    This routine applies damage to the object according to the damage parameters. It handles *
 *    reducing the strength of the object and also returns the result of that damage. The      *
 *    result value can be examined to determine if the object was destroyed, greatly damaged,  *
 *    or other results.                                                                        *
 *                                                                                             *
 * INPUT:   damage   -- Reference to the damage number to apply. This number will be adjusted  *
 *                      according to defensive armor and distance. Examine this value after    *
 *                      the call to determine the actual amount of damage applied.             *
 *                                                                                             *
 *          distance -- The distance (in leptons) from the center of the damage causing        *
 *                      explosion to the object itself.                                        *
 *                                                                                             *
 *          warhead  -- The warhead type that is causing the damage.                           *
 *                                                                                             *
 *          source   -- The perpetrator of this damage.                                        *
 *                                                                                             *
 *          forced   -- Is the damage forced upon the object regardless of whether it          *
 *                      is normally immune?                                                    *
 *                                                                                             *
 * OUTPUT:  Returns the ResultType that indicates what the affect of the damage was.           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 JLB : Created.                                                                 *
 *   12/27/1994 JLB : Trigger event processing for attacked or destroyed.                      *
 *   01/01/1995 JLB : Reduces damage greatly depending on range.                               *
 *=============================================================================================*/
ResultType ObjectClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool)
{
	assert(this != NULL);

	ResultType result = RESULT_NONE;
	int oldstrength = Strength;

	if (oldstrength > 0 && damage != 0 && (forced || !Class_Of()->IsImmune)) {
		int maxstrength = Class_Of()->MaxStrength;

		/*
		**	Modify damage based on the warhead type and the armor of the object. This results
		**	in a reduced damage value, but never below 1 damage point.  Unless
		**	it's forced damage, in which case we want full damage.
		*/
		if (!forced) {
			damage = Modify_Damage(damage, warhead, Class_Of()->Armor, distance);
		}
		if (damage == 0) return(RESULT_NONE);

		/*
		**	Are we healing/repairing?  If so, add strength, but in
		**	any case, return that no damage was done.
		*/
		if (damage < 0) {
			int oldstrength = Strength;
			Strength -= damage;
			if (Strength > maxstrength) {
				Strength = maxstrength;
			}
			if (oldstrength != Strength) {
				Clicked_As_Target(7);
			}
			return(RESULT_NONE);
		}

		/*
		**	At this point, we KNOW that at least light damage has occurred.
		*/
		result = RESULT_LIGHT;

		/*
		**	A non-fatal blow has occurred. Check to see if the object transitioned to below
		**	half strength or if it is now down to one hit point.
		*/
		if (oldstrength > damage) {

			if (oldstrength >= (maxstrength >> 1) && (oldstrength-damage) < (maxstrength >> 1)) {
				result = RESULT_HALF;
			}
		} else {

			/*
			**	When an object is damaged to destruction, it will instead stop at one
			**	damage point. This will prolong the damage state as well as
			**	give greater satisfaction when it is finally destroyed.
			*/
			damage = oldstrength;
		}

		if (oldstrength > maxstrength * Rule->ConditionRed && (oldstrength - damage) < maxstrength * Rule->ConditionRed) {
			result = RESULT_MAJOR;
		}

		/*
		**	Apply the damage to the object.
		*/
		Strength = oldstrength - damage;

		if (Strength <= 0 && RTTI == RTTI_INFANTRY && !forced) {
			InfantryClass * inf = (InfantryClass*)this;
			if (inf->Class->IsCyborg && !inf->IsProne) {
				new AnimClass(Rule->InfantryExplode, Get_Coord());
				int strength = inf->Class->MaxStrength;
				strength *= 0.25;
				Strength = strength;
				if (strength <= 1)	{
					strength = 1;
				}
				Strength = strength;
				inf->IsProne = true;
				inf->Do_Action(DO_CRAWL, true);
				result = RESULT_MAJOR;
			}
		}

		int pretag_strength = Strength;

		/*
		**	Handle any trigger event associated with this object.
		*/
		if (result == RESULT_HALF) {
			if (source && Tag != NULL) Tag->Spring(TEVENT_ENTER_YELLOW, this);

			if (IsActive) {
				if (Tag != NULL) Tag->Spring(TEVENT_ENTER_YELLOW_ANY, this);
			} else {
				return(RESULT_ALREADY_DESTROYED);
			}
		}

		if (IsActive) {
			if (result == RESULT_MAJOR) {
				if (source && Tag != NULL) Tag->Spring(TEVENT_ENTER_RED, this);

				if (IsActive) {
					if (Tag != NULL) Tag->Spring(TEVENT_ENTER_RED_ANY, this);
				} else {
					return(RESULT_ALREADY_DESTROYED);
				}
			}
		}

		if (IsActive) {
			if (Strength != oldstrength && Class_Of() != NULL && oldstrength == Class_Of()->MaxStrength) {
				if (source && Tag != NULL) Tag->Spring(TEVENT_FIRST_DAMAGED, this);
				if (Tag != NULL && IsActive) Tag->Spring(TEVENT_FIRST_DAMAGED_ANY, this);

				if (Tag != NULL) {
					if (IsActive) {
						if (source) Tag->Spring(TEVENT_FIRST_DAMAGED_ANY, this, CELL_NONE, false, source);
					} else {
						return(RESULT_ALREADY_DESTROYED);
					}
				}
			}
		} else {
			return(RESULT_ALREADY_DESTROYED);
		}

		if (IsActive && (pretag_strength <= 0 || Strength > 0)) {

			/*
			**	Check to see if the object is majorly damaged or destroyed.
			*/
			if (IsActive) {
				if (Strength == 0) {
					Record_The_Kill(source);
					result = RESULT_DESTROYED;
					Detach_All(true);
				}
			}

			if (IsActive) {
				if (source) {
					if (Tag && result != RESULT_DESTROYED) {
						Tag->Spring(TEVENT_ATTACKED, this, CELL_NONE, 0, source);
					}
				}
			}

			if (IsActive) {
				if (source) {
					if (Tag && result != RESULT_DESTROYED) {
						Tag->Spring(TEVENT_ATTACKED_BY, this, CELL_NONE, 0, source);
					}
				}
			}

			/*
			**	If any damage was assessed and this object is selected, then flag
			**	the object to be redrawn so that the health bar will be updated.
			*/
			if (IsActive) {
				if (result != RESULT_NONE && IsSelected) {
					Mark(MARK_CHANGE);
				}
			}
		} else {
			return(RESULT_ALREADY_DESTROYED);
		}
	}

	/*
	**	Return with the result of the damage taken.
	*/
	return(result);
}


/***********************************************************************************************
 * ObjectClass::Mark -- Handles basic marking logic.                                           *
 *                                                                                             *
 *    This routine handles the base logic for marking an object up or down on the map. It      *
 *    manages the IsDown flag as well as flagging the object to be redrawn if necessary.       *
 *    Whenever an object is to be marked, it should call this base class function first. If    *
 *    this function returns true, then the higher level function should proceed with its own   *
 *    logic.                                                                                   *
 *                                                                                             *
 * INPUT:   mark  -- The marking method to use for this object. It can be either MARK_DOWN,    *
 *                   MARK_UP, or MARK_CHANGE.                                                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the object marked successfully?                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/23/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Mark(MarkType mark)
{
	assert(this != NULL);

	if (!IsInLimbo) {

		/*
		**	A mark for change is always successful UNLESS the object
		**	is not placed down or has already been flagged as changed
		**	this game frame.
		*/
		if (mark == MARK_CHANGE) {
			if (IsToDisplay) return(false);
			if (IsDown) {
				Mark_For_Redraw();
				return(true);
			}
			return(false);
		}

		/// These values are gathered and then never used.

		/*
		**	It is important to know whether the object is a techno class
		**	or not to see if we have to adjust the regional threat ratings
		*/
		int threat = 0;
		HousesType house = HOUSE_NONE;
		Cell cell(0,0);
		TechnoClass * tech = Dynamic_Cast<TechnoClass *>(this);
		if (tech != NULL) {
			threat = tech->Risk();
			house  = tech->Owner();
			cell   = PositionCell;
		} else {
			tech = NULL;
		}

		/*
		**	Marking down is only successful if the object isn't already
		**	placed down.
		*/
		if ((mark == MARK_DOWN || mark == MARK_DOWN_FORCED) && !IsDown) {
			IsDown = true;
			Mark_For_Redraw();
			return(true);
		}

		/*
		**	Lifting up is only successful if the object isn't already
		**	lifted up from the map.
		*/
		if (mark == MARK_UP && IsDown) {
			IsDown = false;
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * ObjectClass::Revealed -- Reveals this object to the house specified.                        *
 *                                                                                             *
 *    This routine is called when this object gets revealed to the house specified.            *
 *                                                                                             *
 * INPUT:   house -- Pointer to the house that this object is being revealed to.               *
 *                                                                                             *
 * OUTPUT:  Was this object revealed for the first time to this house? Generic objects always  *
 *          return true unless an invalid house pointer was specified.                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Revealed(HouseClass * house)
{
	assert(this != NULL);

	return(house != NULL);
}


/***********************************************************************************************
 * ObjectClass::Paradrop -- Unlimbos object in paradrop mode.                                  *
 *                                                                                             *
 *    Call this routine as a replacement for Unlimbo() if the object is to be paradropped onto *
 *    the playing field.                                                                       *
 *                                                                                             *
 * INPUT:   coord -- The desired landing coordinate to give the dropping unit.                 *
 *                                                                                             *
 * OUTPUT:  bool; Was the object successfully unlimboed and has begun paradropping?            *
 *                                                                                             *
 * WARNINGS:   The unit may not be successful in paradropping if the desired destination       *
 *             location cannot be occupied by the object.                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Paradrop(Coord const & coord)
{
	assert(this != NULL);

	IsFalling = true;
	if (Unlimbo(coord, DIR_S)) {
		AnimClass * anim = NULL;

		PositionCoord = coord;

		if (RTTI == RTTI_BULLET) {
			anim = new AnimClass(Rule->BombParachute, coord);
		} else {
			anim = new AnimClass(Rule->Parachute, coord);
		}

		/*
		**	If the animation was created, then attach it to this object.
		*/
		if (anim != NULL) {
			anim->Attach_To(this);
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * ObjectClass::Attach_Tag -- Attach specified tag to object.                                  *
 *                                                                                             *
 *    This routine is used to attach the specified tag to the object.                          *
 *                                                                                             *
 * INPUT:   trigger  -- Pointer to the tag to attach. If any existing tag is desired           *
 *                      to be detached, then pass NULL to this routine.                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the tag attached?                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool ObjectClass::Attach_Tag(TagClass * tag)
{
	assert(this != NULL);

	if (Tag != NULL) {
		TagClass * tptr = Tag;
		tptr->AttachCount--;
		Tag = NULL;
	}

	if (tag) {
		Tag = tag;
		tag->AttachCount++;
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the list of cells that this object occupies.
/// The list comes from the object's type class, so that the map knows which cells to mark as
/// occupied. An object with no type class to speak of occupies nothing at all.
/// </summary>
/// <param name="placement">Is this for placement legality checking only? The normal condition
/// is for marking occupation flags.</param>
/// <returns>Returns with a pointer to a cell offset list, terminated by REFRESH_EOL.</returns>
Cell const * ObjectClass::Occupy_List(bool placement) const
{
	static Cell const _list[] = {REFRESH_EOL};

	assert(this != NULL);

	if (Class_Of() == NULL) {
		return(_list);
	}

	return(Class_Of()->Occupy_List(placement));
}


/// <summary>
/// Finds the factory that could produce this object.
/// This routine asks the object's type class to scan the owning house for a suitable factory
/// building. Use it to discover where a replacement for this object would come from.
/// </summary>
/// <param name="intheory">Should a factory that is currently occupied still be considered?</param>
/// <param name="legal">Should the prerequisite and technology level rules be enforced?</param>
/// <returns>Returns with a pointer to the factory building found. Otherwise, NULL is
/// returned.</returns>
BuildingClass * ObjectClass::Who_Can_Build_Me(bool intheory, bool legal) const
{
	assert(this != NULL);

	return(Class_Of()->Who_Can_Build_Me(intheory, legal, false, Owner_HouseClass()));
}


/// <summary>
/// Fetches the strength of this object as a fraction of its maximum.
/// </summary>
/// <returns>Returns with the health ratio, where 1 is undamaged and 0 is destroyed.</returns>
double ObjectClass::Get_Health_Ratio(void) const
{
	assert(this != NULL);

	return((double)Strength / Class_Of()->MaxStrength);
}


/// <summary>
/// Sets the strength of this object as a fraction of its maximum.
/// An object given any health at all is left with at least one strength point, so that
/// rounding cannot quietly kill something that was meant to survive.
/// </summary>
/// <param name="health">The fraction of maximum strength to set, where 1 is undamaged.</param>
void ObjectClass::Set_Health_Ratio(double health)
{
	assert(this != NULL);

	if (health > 0) {
		Strength = (int)(Class_Of()->MaxStrength * health);
		if (Strength == 0) Strength = 1;
	} else {
		Strength = 0;
	}
}


/// <summary>
/// Lists the members every game object carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void ObjectClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Riser);
	stream.Serialize(Next);
	stream.Serialize(Tag);
	stream.Serialize(Strength);
	stream.Serialize(IsDown);
	stream.Serialize(IsToDamage);
	stream.Serialize(IsToDisplay);
	stream.Serialize(IsInLimbo);
	stream.Serialize(IsSelected);
	stream.Serialize(IsAnimAttached);
	stream.Serialize(IsOnBridge);
	stream.Serialize(IsFalling);
	stream.Serialize(IsToExplode);
	stream.Serialize(IsActive);
	stream.Serialize(Layer);
	stream.Serialize(IsSubmittedToLayer);
	stream.Serialize(Position);
}


/// <summary>
/// Fetches the ground elevation that this object is standing at.
/// This is the height of the cell underneath, raised by a bridge span when the object is
/// traveling over the bridge rather than beneath it.
/// </summary>
/// <returns>Returns with the elevation of the ground beneath the object.</returns>
int ObjectClass::Get_Cell_Height(void) const
{
	assert(this != NULL);

	CellClass *cell = Get_Cell_Ptr();
	return(cell->Height + (IsOnBridge ? BRIDGE_CELL_HEIGHT : 0));
}


/// <summary>
/// Fetches the absolute height of the object.
/// </summary>
/// <returns>Returns with the object's height in world coordinates, in leptons.</returns>
int ObjectClass::Get_Height(void) const
{
	assert(this != NULL);

	return(Position.Z);
}


/// <summary>
/// Fetches the height of the object above the ground.
/// This is the counterpart of Set_Height_AGL. The clearance is measured from the
/// terrain beneath the object, or from the bridge deck when it is riding one.
/// </summary>
/// <returns>Returns with the object's clearance above ground level, in leptons.</returns>
int ObjectClass::Get_Height_AGL(void) const
{
	assert(this != NULL);

	int height = Position.Z - Map.Get_Height_GL(PositionCoord);
	if (IsOnBridge) {
		height -= BRIDGE_LEPTON_HEIGHT;
	}
	return(height);
}


/// <summary>
/// Sets the height of the object above the ground.
/// Flying and falling objects reason in terms of their clearance rather than an
/// absolute height, so this routine adds back the ground level of the cell below --
/// or that of the bridge deck, when the object is riding one.
/// </summary>
/// <param name="height">The clearance above ground to place the object at, in
/// leptons.</param>
void ObjectClass::Set_Height_AGL(int height)
{
	assert(this != NULL);

	if (IsOnBridge) {
		height += BRIDGE_LEPTON_HEIGHT;
	}
	if (IsDown) {
		Mark(MARK_UP);
		Position.Z = height + Map.Get_Height_GL(PositionCoord);
		Mark(MARK_DOWN);
	} else {
		Position.Z = height + Map.Get_Height_GL(PositionCoord);
	}
}


/// <summary>
/// Sets the absolute height of the object.
/// This routine moves the object vertically in world coordinates. An object that is
/// down on the map is lifted off and put back down again, so that the cells it
/// occupies stay correct across the move.
/// </summary>
/// <param name="z">The absolute height to place the object at, in leptons.</param>
void ObjectClass::Set_Height(int z)
{
	assert(this != NULL);

	if (IsDown) {
		Mark(MARK_UP);
		Position.Z = z;
		Mark(MARK_DOWN);
	} else {
		Position.Z = z;
	}
}


/// <summary>
/// Marks the object as occupying a cell.
/// This routine will set the monolith occupy flag on the cell the coordinate falls in.
/// Where the coordinate rides at bridge level over an underpass, the bridge surface is
/// occupied rather than the ground below it.
/// </summary>
/// <param name="coord">The coordinate of the cell to occupy.</param>
void ObjectClass::Set_Occupy_Bit(Coord const & coord)
{
	assert(this != NULL);

	if (Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT <= coord.Z && Map[coord].IsUnderBridge) {
		Map[coord].BridgeFlag.Occupy.Monolith = true;
	} else {
		Map[coord].Flag.Occupy.Monolith = true;
	}
}


/// <summary>
/// Releases the object's occupation of a cell.
/// This routine will clear the monolith occupy flag from the cell the coordinate falls
/// in. Where the coordinate rides at bridge level over an underpass, the bridge
/// surface is released rather than the ground below it.
/// </summary>
/// <param name="coord">The coordinate of the cell to release.</param>
void ObjectClass::Clear_Occupy_Bit(Coord const & coord)
{
	assert(this != NULL);

	if (Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT <= coord.Z && Map[coord].IsUnderBridge) {
		Map[coord].BridgeFlag.Occupy.Monolith = false;
	} else {
		Map[coord].Flag.Occupy.Monolith = false;
	}
}


/// <summary>
/// Releases the voxel and motion libraries this record owns.
/// </summary>
VoxelDataStruct::~VoxelDataStruct(void)
{
	delete VoxLib;
	VoxLib = NULL;
	delete MotLib;
	MotLib = NULL;
}


/// <summary>
/// Does this object sort before the specified one?
/// Objects compare by their display sorting value, so that a list of them can be kept
/// in the back to front order the renderer wants.
/// </summary>
/// <returns>bool; Does this object sort earlier than the other one?</returns>
bool ObjectClass::operator < (ObjectClass const & object) const
{
	return(Sort_Y() < object.Sort_Y());
}


/// <summary>
/// Does this object sort after the specified one?
/// Objects compare by their display sorting value, so that a list of them can be kept
/// in the back to front order the renderer wants.
/// </summary>
/// <returns>bool; Does this object sort later than the other one?</returns>
bool ObjectClass::operator > (ObjectClass const & object) const
{
	return((Sort_Y() > object.Sort_Y()) ? true : false);
}


/// <summary>
/// Adds this object's state to the multiplayer sync check.
/// This routine will submit every part of the object that must stay identical on all
/// machines. State that legitimately differs from player to player, such as the
/// selection, is only submitted for single player and skirmish games.
/// </summary>
/// <param name="crc">The engine to submit this object's state to.</param>
void ObjectClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);

	if (Next != NULL) {
		crc(Next->Fetch_ID());
	}

	if (Tag != NULL) {
		crc(Tag->Fetch_ID());
	}

	crc(Strength);
	crc(IsDown);
	crc(IsToDamage);
	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		crc(IsToDisplay);
		crc(IsSelected);
	}
	crc(IsInLimbo);
	crc(IsAnimAttached);
	crc(IsOnBridge);
	crc(IsFalling);
	crc(IsToExplode);
	crc(IsActive);
	crc(Position.X);
	crc(Position.Y);
	crc(Position.Z);
}


/***********************************************************************************************
 * ObjectClass::Distance -- Determines distance to target.                                     *
 *                                                                                             *
 *    This will determine the distance (direct line) to the target. The distance is in         *
 *    'leptons'. This routine is typically used for weapon range checks.                       *
 *                                                                                             *
 * INPUT:   target   -- The target to determine range to.                                      *
 *                                                                                             *
 * OUTPUT:  Returns with the range to the specified target (in leptons).                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectClass::Distance(AbstractClass const * target) const
{
	assert(this != NULL);

	if (target == NULL) {
		return(0);
	}
	BuildingClass * obj;
	int dist = Distance(target->Center_Coord());

	/*
	**	If the object is a building the adjust it by the average radius
	**	of the object.
	*/
	obj = target->RTTI == RTTI_BUILDING ? (BuildingClass *)target : NULL;
	if (obj) {
		dist -= ((obj->Class->Width() + obj->Class->Height()) * (CELL_LEPTON / 4));
		if (dist < 0) dist = 0;
	}

	/*
	**	Return the distance to the target
	*/
	return(dist);
}


/// <summary>
/// Determines the flat distance to a target.
/// This is the ground plan counterpart of Distance -- any height difference between
/// the two objects is ignored. A building target is credited with its average radius
/// so that the range is measured to the edge of the structure and not to its middle.
/// </summary>
/// <param name="target">The target to determine range to.</param>
/// <returns>Returns with the range to the specified target, in leptons.</returns>
int ObjectClass::Planar_Distance(AbstractClass const * target) const
{
	assert(this != NULL);

	if (target == NULL) {
		return(0);
	}
	BuildingClass * obj;
	int dist = Point2D(Center_Coord()).Distance_To(Point2D(target->Center_Coord()));

	/*
	**	If the object is a building the adjust it by the average radius
	**	of the object.
	*/
	obj = target->RTTI == RTTI_BUILDING ? (BuildingClass *)(target) : NULL;
	if (obj) {
		dist -= ((obj->Class->Width() + obj->Class->Height()) * (CELL_LEPTON / 4));
		if (dist < 0) dist = 0;
	}

	/*
	**	Return the distance to the target
	*/
	return(dist);
}


/// <summary>
/// Determines the squared flat distance to a target.
/// This routine is used where only the ordering of distances matters, since squared
/// values sort the same way and the square root can be spared. Height is ignored.
/// </summary>
/// <param name="target">The target to measure to.</param>
/// <returns>Returns with the square of the horizontal distance, in leptons. A NULL
/// target yields zero.</returns>
int ObjectClass::Relative_Distance(AbstractClass const * target) const
{
	assert(this != NULL);

	if (target == NULL) {
		return(0);
	}

	Coord center = Center_Coord();
	Coord target_center = target->Center_Coord();
	Point2D pt(center.X - target_center.X, center.Y - target_center.Y);
	return((pt.X * pt.X) + (pt.Y * pt.Y));
}


/// <summary>
/// Determines the squared flat distance to a coordinate.
/// This routine is used where only the ordering of distances matters, since squared
/// values sort the same way and the square root can be spared. Height is ignored.
/// </summary>
/// <param name="coord">The coordinate to measure to.</param>
/// <returns>Returns with the square of the horizontal distance, in leptons.</returns>
int ObjectClass::Relative_Distance(Coord const & coord) const
{
	assert(this != NULL);

	Coord center = Center_Coord();
	Point2D pt(center.X - coord.X, center.Y - coord.Y);
	return((pt.X * pt.X) + (pt.Y * pt.Y));
}


/// <summary>
/// Fetches the center coordinate of the object.
/// Derived objects whose middle lies away from their position -- buildings, most
/// notably -- override this routine so that ranging and targeting work from the
/// center of the object rather than from a corner of it.
/// </summary>
/// <returns>Returns with the coordinate at the center of the object.</returns>
Coord ObjectClass::Center_Coord(void) const
{
	assert(this != NULL);

	return(PositionCoord);
}


/// <summary>
/// Draws the object without writing to the depth buffer.
/// This routine is used when the object must appear over the scene rather than take
/// part in depth sorting. The base object has nothing special to arrange, so it just
/// draws itself in the usual fashion.
/// </summary>
/// <param name="point">The pixel position to draw the object at.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
void ObjectClass::Editor_Draw_It(Point2D const & point, Rect const & cliprect) const
{
	assert(this != NULL);

	Draw_It(point, cliprect);
}


/// <summary>
/// Marks the object for destruction.
/// This routine will detach the object from everything that refers to it, remove it
/// from the map, and queue it on the deferred deletion list so that the actual delete happens at a
/// safe point in the game logic.
/// </summary>
/// <remarks>The object must not be referred to after this routine returns.</remarks>
void ObjectClass::Delete_Me(void)
{
	assert(this != NULL);
//	//assert(IsActive);

	Detach_This_From_All(this);
	Limbo();
	IsActive = false;
	ObjectsToDelete.Add(this);
}


/// <summary>
/// Has this object been deactivated?
/// An object is deactivated the moment it is queued for deletion, so anything that
/// walks the object lists must skip it from then on.
/// </summary>
/// <returns>bool; Is the object no longer active?</returns>
bool ObjectClass::Is_Inactive(void) const
{
	return(IsActive==false);
}


/// <summary>
/// Determines the bounding rectangle of a list of objects.
/// This routine will build the smallest lepton rectangle that encloses the coordinates
/// of every object in the list. It is used to size the region a group covers.
/// </summary>
/// <param name="list">The list of objects to enclose.</param>
/// <returns>Returns with the bounding rectangle. An empty list yields an empty
/// rectangle.</returns>
Rect Vector_Rect(DynamicVectorClass<ObjectClass *> const & list)
{
	Rect bounds(0, 0, 0, 0);

	if (list.Count() > 0) {

		/// Start with the position of the first object
		bounds.X = list[0]->PositionCoord.X;
		bounds.Y = list[0]->PositionCoord.Y;

		for (int i = list.Count() - 1; i >= 0; i--) {
			Coord coord = list[i]->PositionCoord;
			int x = coord.X;
			int y = coord.Y;

			if (x < bounds.X) {
				bounds.Width += bounds.X - x;
				bounds.X = x;
			}
			if (y < bounds.Y) {
				bounds.Height += bounds.Y - y;
				bounds.Y = y;
			}
			if (x > bounds.X + bounds.Width) {
				bounds.Width = x - bounds.X;
			}
			if (y > bounds.Y + bounds.Height) {
				bounds.Height = y - bounds.Y;
			}
		}
	}
	return(bounds);
}


/// <summary>
/// Determines the average coordinate of a list of objects.
/// This routine is used to find the center of a group so that formation moves and
/// group orders have a point of reference to work from.
/// </summary>
/// <param name="list">The list of objects to average together.</param>
/// <returns>Returns with the coordinate at the center of the group.</returns>
/// <remarks>The list must not be empty.</remarks>
Coord Vector_Center(DynamicVectorClass<ObjectClass *> const & list)
{
	Coord center = Coord(0, 0, 0);
	int count = list.Count();
	for (int i = count - 1; i >= 0; i--) {
		center += list[i]->PositionCoord;
	}
	center.X /= count;
	center.Y /= count;
	center.Z /= count;
	return(center);
}


/// <summary>
/// Finds the object in the list that lies nearest a coordinate.
/// This routine is used where a group of objects must nominate one of its members --
/// the unit that will lead a formation move, for example.
/// </summary>
/// <param name="list">The list of objects to search.</param>
/// <param name="coord">The coordinate to measure each object against.</param>
/// <returns>Returns with a pointer to the closest object. Otherwise, NULL is
/// returned.</returns>
ObjectClass * Vector_Closest_Object(DynamicVectorClass<ObjectClass *> const & list, Coord const & coord)
{
	ObjectClass * closest = NULL;
	int mindist = 99999999;

	for (int i = list.Count() - 1; i >= 0; i--) {
		if (closest == NULL) {
			closest = list[i];
			mindist = coord.Distance_To(closest->PositionCoord);
		} else {
			int dist = Distance(coord, list[i]->PositionCoord);
			if (dist < mindist) {
				closest = list[i];
				mindist = dist;
			}
		}
	}
	return(closest);
}


/// <summary>
/// Sets the location of the object.
/// This is the raw coordinate assignment that backs the PositionCoord property.
/// </summary>
/// <param name="coord">The coordinate to place the object at.</param>
/// <remarks>No map bookkeeping is performed here. The caller must mark the object off
/// the map and back down again around the move.</remarks>
void ObjectClass::Set_Coord(Coord const & coord)
{
	assert(this != NULL);

	Position = coord;
}


/// <summary>
/// Fetches the cell the object currently sits in.
/// </summary>
/// <returns>Returns with a pointer to the cell the object's coordinate falls in.</returns>
CellClass * ObjectClass::Get_Cell_Ptr(void) const
{
	assert(this != NULL);

	return(&Map[Get_Coord()]);
}


/// <summary>
/// Fetches the cell the object is headed for.
/// </summary>
/// <returns>Returns with the cell that holds the object's destination.</returns>
Cell ObjectClass::Get_Target_Cell(void) const
{
	assert(this != NULL);

	return(Destination_Coord().As_Cell());
}


/// <summary>
/// Fetches the cell the object is headed for.
/// </summary>
/// <returns>Returns with a pointer to the cell that holds the object's destination.</returns>
CellClass * ObjectClass::Get_Target_Cell_Ptr(void) const
{
	assert(this != NULL);

	return(&Map[Destination_Coord().As_Cell()]);
}


/// <summary>
/// Will the object be standing on a bridge when it arrives?
/// This routine is used by the movement code, which must know whether the object is
/// bound for a bridge deck or for the ground beneath it so that height and cell
/// occupation are tracked against the right surface.
/// </summary>
/// <returns>bool; Will the object be on a bridge once it reaches its destination?</returns>
bool ObjectClass::Is_Moving_Onto_Bridge(void) const
{
	assert(this != NULL);

	bool bridge = IsOnBridge;

	Coord destination = Destination_Coord();
	int dest_height = Map.Get_Height_GL(destination);
	int current_height = Map.Get_Height_GL(PositionCoord);

	/*
	 * We're not on a bridge, but the destination contains a bridge and is
	 * lower than our GL - so we're going onto a bridge.
	 */
	if (!IsOnBridge) {
		if (current_height - dest_height > 3 * LEVEL_LEPTON_H) {
			if (Map[destination].IsUnderBridge) {
				return(true);
			}
		}
	}

	/*
	 * We are currently on a bridge, but the destination is higher than our GL
	 * so we're going off a bridge.
	 */
	if (IsOnBridge) {
		if (dest_height - current_height > 3 * LEVEL_LEPTON_H) {
			return(false);
		}
	}

	/*
	 * We're not moving on or off a bridge, so return if we're currently on one.
	 */
	return(bridge);
}


/// <summary>
/// Assigns a waypoint path for the object to follow.
/// The base object has no notion of a waypoint path, so this routine does nothing.
/// Mobile derived objects override it to record the path and begin traveling it.
/// </summary>
void ObjectClass::Set_Waypoint_Path(PathType path, char index)
{
	assert(this != NULL);

	// empty
}


/// <summary>
/// Is the object resting on the terrain?
/// This routine is used wherever ground contact matters -- crushing, cell occupation
/// and the like. An object still in limbo is never considered to be on the ground.
/// </summary>
/// <returns>bool; Is the object placed down and at ground level?</returns>
bool ObjectClass::On_Ground(void) const
{
	assert(this != NULL);

	if (IsDown && HeightAGL < LEVEL_LEPTON_H) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Is the object flying above the terrain?
/// This routine is the counterpart of On_Ground and is used wherever airborne objects
/// must be told apart from those resting on the map. An object still in limbo is never
/// considered to be in the air.
/// </summary>
/// <returns>bool; Is the object placed down and clear of the ground?</returns>
bool ObjectClass::In_Air(void) const
{
	assert(this != NULL);

	if (IsDown && HeightAGL >= LEVEL_LEPTON_H) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the techno type class for this object.
/// The base object carries no shared type data of its own. Derived techno objects
/// override this routine so that the TClass property resolves to their type.
/// </summary>
/// <returns>Returns with a pointer to the object's techno type class, or NULL if it
/// has none.</returns>
TechnoTypeClass const * ObjectClass::Techno_Type_Class(void) const
{
	assert(this != NULL);

	return(NULL);
}


/***********************************************************************************************
 * ObjectClass::Sort_Y -- Returns the coordinate used for display order sorting.               *
 *                                                                                             *
 *    This routine will return the value to be used for object sorting. The sorting ensures    *
 *    that the object are rendered from a top to bottom order. Certain object use a sorting    *
 *    value different from their center coordinate. This is true if the object "touches the    *
 *    ground" at a point that is different from the object's center point.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the value to use as the Y sorting value.                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int ObjectClass::Sort_Y(void) const
{
	assert(this != NULL);

	return(Render_Coord().Y);
}


/// <summary>
/// Is the object above the ground rather than buried beneath it?
/// This routine is used to weed out objects that are currently underground. A
/// subterranean unit that has burrowed is no target and cannot be interacted with.
/// </summary>
/// <returns>bool; Is the object at or above ground level?</returns>
bool ObjectClass::Not_Underground(void) const
{
	assert(this != NULL);

	return(HeightAGL > -20);
}
