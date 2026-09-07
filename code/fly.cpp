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

/* $Header: /CounterStrike/FLY.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : FLY.CPP                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 24, 1994                                               *
 *                                                                                             *
 *                  Last Update : June 5, 1995 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   FlyClass::Fly_Speed -- Sets the flying object to the speed specified.                     *
 *   FlyClass::Physics -- Performs vector physics (movement).                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "fly.h"

#include "_map.h"
#include "_rand.h"
#include "_rtti.h"
#include "_rules.h"
#include "aircraft.h"
#include "airctype.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "ccrand.h"
#include "cell.h"
#include "combat.h"
#include "coord.h"
#include "face.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "houstype.h"
#include "iflyctrl.h"
#include "incdec.h"
#include "inline.h"
#include "map.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "session.h"
#include "sun.h"
#include "team.h"
#include "unit.h"
#include "unittype.h"
#include "weapon.h"

#include "layer.hh"

#include <algorithm>

static const int const1 = 16;
static const int const2 = 16;
static const int const3 = 6;
static const int const4 = 0;
//static const float const5 = 0.02;
static const int const6 = 10;

/// <summary>
/// Constructs a flying locomotor.
/// The aircraft starts out grounded, idle, and with nowhere to be. It will not stir until
/// the owning object hands it a destination.
/// </summary>
FlyLocomotionClass::FlyLocomotionClass(void) :
	BASECLASS(),
	DestinationCoord(COORD_NONE),
	HeadToCoord(COORD_NONE),
	IsMoving(false),
	FlightLevel(0),
	TargetSpeed(0.0),
	CurrentSpeed(0.0),
	IsTakingOff(false),
	IsLanding(false),
	CommencedLanding(false),
	IsTumbling(false),
	CurrentROT(0),
	Riser(0),
	IsElevating(false)
{
}


/// <summary>
/// Destroys the flying locomotor.
/// </summary>
FlyLocomotionClass::~FlyLocomotionClass(void)
{
}


/// <summary>
/// Is the aircraft under way?
/// This routine reports the aircraft's intent rather than its speed, so it stays true for
/// an aircraft that has been told to go somewhere but has yet to build up any speed.
/// </summary>
/// <returns>bool; Is the aircraft moving or trying to?</returns>
bool FlyLocomotionClass::Is_Moving(void)
{
	return(IsMoving || LinkedTo->PitchAngle > 0);
}


/// <summary>
/// Is the aircraft actually in motion at this moment?
/// Unlike Is_Moving, this routine pays no attention to intent and merely reports whether
/// the aircraft has any speed at all.
/// </summary>
/// <returns>bool; Is the aircraft moving right now?</returns>
bool FlyLocomotionClass::Is_Moving_Now(void)
{
	if (CurrentSpeed == 0) {
		return(false);
	}
	return(true);
}


/// <summary>
/// Fetches the coordinate that the aircraft is heading for.
/// </summary>
/// <returns>Returns with the destination coordinate. If the aircraft is not going
/// anywhere, COORD_NONE is returned.</returns>
Coord FlyLocomotionClass::Destination(void)
{
	if (Is_Moving()) {
		return(DestinationCoord);
	}
	return(COORD_NONE);
}


/// <summary>
/// Performs the per frame processing for the flying object.
/// This is the main entry point that the owning object calls every game frame. It gives
/// the aircraft its chance to change mission, rotate, move, land or take off, and to be
/// disposed of if it has wandered off the edge of the world.
/// </summary>
/// <returns>bool; Is the aircraft still under way?</returns>
bool FlyLocomotionClass::Process(void)
{
	if (!IsLanding && !IsTakingOff && TargetSpeed >= 1.0 && FlightLevel == 0) {
		FlightLevel = LinkedTo->TClass->Flight_Level();
	}

	if (LinkedTo->TClass->IsHunterSeeker && !LinkedTo->TarCom) {
		Acquire_Hunter_Seeker_Target();
		if (LinkedTo->TarCom) {
			IsLanding = false;
			FlightLevel = LinkedTo->TClass->Flight_Level();
			if (LinkedTo->In_Radio_Contact() != NULL) {
				LinkedTo->Transmit_Message(RADIO_OVER_OUT);
			}
			LinkedTo->Assign_Mission(MISSION_ATTACK);
			LinkedTo->Commence();
		}
	}

	/*
	**	A Mission change can always occur if the aircraft is landed or flying.
	*/
	if (LinkedTo->Strength > 0 && !IsLanding && !IsTakingOff && LinkedTo->Ready_To_Commence()) {
		LinkedTo->Commence();
	}

	/*
	**	Handle any body rotation at this time. Body rotation can occur even if the
	**	flying object is not actually moving.
	*/
	Rotation_AI();

	/*
	**	Handle any aircraft movement at this time.
	*/
	Movement_AI();

	if (!Is_Powered()) return(false);

	if (!LinkedTo->IsActive) return(false);

	if (LinkedTo->Strength > 0 &&
		DestinationCoord != COORD_NONE &&
		!IsLanding && !IsTakingOff && LinkedTo->HeightAGL > 0) {
		Nearing_Target(true, DestinationCoord);
	}

	/*
	**	Handle landing and taking off logic. Helicopters are prime users of this technique. The
	**	aircraft will either gain or lose altitude as appropriate. As the aircraft transitions
	**	between flying level and ground level, it will be moved into the appropriate render
	**	layer.
	*/
	if (LinkedTo->Strength > 0) {
		Landing_Takeoff_AI();
	}

	/*
	**	When aircraft leave the edge of the map, they might get destroyed. This occurs if the
	**	aircraft is a non-player produced unit and it has completed its mission. A transport
	**	helicopter that has already delivered reinforcements is a good example of this.
	*/
	Edge_Of_World_AI();

	return(Is_Moving());
}


/// <summary>
/// Assigns a new destination for the aircraft to fly to.
/// This routine is how the owning object steers the aircraft. A real destination starts
/// the aircraft moving, taking off first if it is still on the ground. An empty
/// destination is a request to stop, which brings a flying aircraft down to land.
/// </summary>
/// <param name="to">The coordinate to head for, or COORD_NONE to stop and land.</param>
void FlyLocomotionClass::Move_To(Coord to)
{
	if (((Coord)to).As_Cell() != DestinationCoord.As_Cell() || !IsLanding) {

		if (LinkedTo->StunDuration <= 0 && Is_Powered()) {

			if ((Coord)to == COORD_NONE) {
				int landing_altitude = 0;
				IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
				if (flyctrl != NULL) {
					landing_altitude = flyctrl->Landing_Altitude();
				}

				if ((LinkedTo->HeightAGL > landing_altitude || IsTakingOff) && !IsLanding) {
					DestinationCoord = LinkedTo->PositionCoord;
					Land();
				} else {
					DestinationCoord = COORD_NONE;
				}
				IsElevating = false;
			} else {
				DestinationCoord = to;

				if (LinkedTo->TarCom && LinkedTo->Ammo) {
					DestinationCoord.Z = LinkedTo->TClass->Flight_Level() + Map.Get_Height_GL(to);
				}

				IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
				int landing_altitude = 0;
				if (flyctrl != NULL) {
					landing_altitude = flyctrl->Landing_Altitude();
				}

				IsMoving = true;

				if (LinkedTo->Strength > 0 && !IsTakingOff && (IsLanding || LinkedTo->HeightAGL <= landing_altitude)) {
					Take_Off();
				}

				IsElevating = Is_Locked_To_Straight_Flight();
			}
		}
	}
}


/// <summary>
/// Brings the aircraft's travels to an end.
/// This routine looks for somewhere legal near the aircraft for it to make for, and
/// assigns that as the new destination. An aircraft with nowhere at all to go is destroyed
/// rather than left loitering in an illegal spot.
/// </summary>
void FlyLocomotionClass::Stop_Moving(void)
{
	if (Is_Moving()) {

		Cell cell = LinkedTo->PositionCoord.As_Cell();
		if (!LinkedTo->IsALoaner && !Map.In_Local_Radar(cell) && !LinkedTo->Should_Delete_Off_Map()) {
			cell = Map.Closest_Edge_Cell(cell, true);
		}

		Cell nearby_cell = Map.Nearby_Location(cell, SPEED_TRACK, -1, MZONE_FLYER);
		if (nearby_cell != CELL_NONE) {

			Coord dest = nearby_cell.As_Coord();
			dest.Z = Map.Get_Height_GL(dest);
			if (Map[nearby_cell].IsUnderBridge) {
				dest.Z += BRIDGE_LEPTON_HEIGHT;
			}

			if (!IsLanding) {
				LinkedTo->Assign_Destination(&Map[dest]);
			}
		} else {
			if (LinkedTo->Strength > 0) {
				LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, NULL, true, true);
				DestinationCoord = COORD_NONE;
			}
		}
	}
}


/***********************************************************************************************
 * AircraftClass::Landing_Takeoff_AI -- Handle aircraft take off and landing processing.       *
 *                                                                                             *
 *    This routine handles the tricky maneuver of taking off and landing. The process of       *
 *    landing is not entirely safe and thus the aircraft may be destroyed as a consequence.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the aircraft destroyed by this process?                                  *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per aircraft per game logic loop. Be sure to        *
 *             examine the return value and if true, abort all further processing of this      *
 *             aircraft since it is now dead.                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FlyLocomotionClass::Landing_Takeoff_AI(void)
{
	if (LinkedTo->Strength > 0) {

		/*
		**	Handle landing and taking off logic. Helicopters are prime users of this technique. The
		**	aircraft will either gain or lose altitude as appropriate. As the aircraft transitions
		**	between flying level and ground level, it will be moved into the appropriate render
		**	layer.
		*/
		if (IsLanding || IsTakingOff) {
			LayerType layer = LinkedTo->In_Which_Layer();

			LinkedTo->Mark(MARK_UP);
			Map.Remove(LinkedTo);

			if (IsLanding) {
				Process_Landing();
			}
			if (IsTakingOff) {
				Process_Take_Off();
			}

			/*
			**	Make adjustments for altitude by moving from one layer to another as
			**	necessary.
			*/
			if (layer != LinkedTo->In_Which_Layer()) {

				/*
				**	When the aircraft is about to enter the ground layer, perform on last
				**	check to see if it is legal to enter that location. If not, then
				**	start the take off process. Let the normal logic handle this
				**	change of plans.
				*/
				bool ok = true;
				if (LinkedTo->In_Which_Layer() == LAYER_GROUND && !IsTakingOff) {
					if (!LinkedTo->Is_LZ_Clear(&Map[(Coord const &)LinkedTo->PositionCoord])) {
						IsTakingOff = true;
						LinkedTo->IsOnBridge = false;
						LinkedTo->Mark(MARK_UP);
						LinkedTo->HeightAGL += 10;
						LinkedTo->Mark(MARK_DOWN);
						ok = false;
					}
				}

				if (ok) {

					/*
					**	When the aircraft is close to the ground, it should exist as a ground object.
					**	This aspect is controlled by the Place_Down and Pick_Up functions.
					*/
					if (LinkedTo->In_Which_Layer() == LAYER_GROUND) {
						LinkedTo->Assign_Destination(NULL);		// Clear the navcom.
						LinkedTo->Transmit_Message(RADIO_TETHER);
						LinkedTo->Look();
					} else  {
						LinkedTo->Transmit_Message(RADIO_UNTETHER);

						/*
						**	If the navigation computer is not attached to the object this
						**	aircraft is in radio contact with, then assume that radio
						**	contact is now superfluous. Break radio contact.
						*/
						if (LinkedTo->In_Radio_Contact() && LinkedTo->NavCom && LinkedTo->NavCom != LinkedTo->Contact_With_Whom()) {
							LinkedTo->Transmit_Message(RADIO_OVER_OUT);
						}
					}
				}
			}

			Map.Submit(LinkedTo);
			LinkedTo->Mark(MARK_DOWN);
		}
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Edge_Of_World_AI -- Detect if aircraft has exited the map.                   *
 *                                                                                             *
 *    Certain aircraft will be eliminated when they leave the edge of the world presumably     *
 *    after completing their mission. An exception is for aircraft that have been newly        *
 *    created as reinforcements and have not yet completed their mission.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the aircraft deleted by this routine?                                    *
 *                                                                                             *
 * WARNINGS:   Be sure to call this routine only once per aircraft per game logic loop. If     *
 *             the return value is true, then abort any further processing of this aircraft    *
 *             since it has been eliminated.                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FlyLocomotionClass::Edge_Of_World_AI(void)
{
	if (!Map.In_Local_Radar((Coord const &)LinkedTo->PositionCoord)) {
		if (LinkedTo->Mission == MISSION_RETREAT) {

			/*
			**	Check to see if there are any civilians aboard. If so, then flag the house
			**	that the civilian evacuation trigger event has been fulfilled.
			*/
			while (LinkedTo->Cargo.Is_Something_Attached()) {
				FootClass * obj = LinkedTo->Cargo.Detach_Object();

				/*
				**	Flag the owning house that civ evacuation has occurred.
				*/
				if (Counts_As_Civ_Evac(obj)) {
					obj->House->IsCivEvacuated = true;
				}

				if (obj->Team != NULL) obj->Team->IsLeaveMap = true;

				delete obj;
			}
			if (LinkedTo->Team != NULL) {
				LinkedTo->Team->IsLeaveMap = true;
			}
			LinkedTo->Stun();
			LinkedTo->Delete_Me();
			return(true);
		}
	} else {
		LinkedTo->IsLocked = true;
	}
	return(false);
}


/***********************************************************************************************
 * AircraftClass::Movement_AI -- Handles aircraft physical movement logic.                     *
 *                                                                                             *
 *    This routine manages the aircraft movement across the map. If any movement occurred, the *
 *    aircraft will be flagged to be redrawn.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per aircraft per game logic loop.                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void FlyLocomotionClass::Movement_AI(void)
{
	#define FORCED_DESTRUCTION_DAMAGE 1000

	if (LinkedTo->Mission == MISSION_ENTER && IsElevating) {
		IsElevating = false;
	}

	Coord coord;

	if ((!Is_Powered() || LinkedTo->Strength == 0) && LinkedTo->HeightAGL != 0) {
		if (LinkedTo->Strength == 0) {
			Riser += 1;
		} else {
			Riser += 3;
		}

		Coord newcoord = LinkedTo->Get_Coord();
		coord = newcoord;
		newcoord.Z -= Riser;

		Map.Get_Height_GL(newcoord);
		Map[newcoord];

		if (Map.In_Radar(newcoord.As_Cell())) {
			Map.Remove(LinkedTo);
			LinkedTo->Mark(MARK_UP);
			LinkedTo->Set_Coord(newcoord);
			LinkedTo->Mark(MARK_DOWN);
			Map.Submit(LinkedTo);
		}

		if (LinkedTo->HeightAGL <= 0) {
			LinkedTo->Set_Height_AGL(0);

			int damage = LinkedTo->Strength;
			if (damage == 0) {
				Cell cell = newcoord.As_Cell();
				new AnimClass(Combat_Anim(FORCED_DESTRUCTION_DAMAGE, Rule->C4Warhead, Map[cell].Land_Type(), newcoord), newcoord, 0, 1, ShapeFlags_Type(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ZGRAD), Get_Explosion_Z(newcoord));
				Combat_Lighting(newcoord, FORCED_DESTRUCTION_DAMAGE, Rule->C4Warhead, false);
				Explosion_Damage(newcoord, FORCED_DESTRUCTION_DAMAGE, NULL, Rule->C4Warhead, true);
				LinkedTo->Delete_Me();
			} else {
				LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, NULL, true, true);
				Cell cell = newcoord.As_Cell();

				new AnimClass(Combat_Anim(damage, Rule->C4Warhead, Map[cell].Land_Type(), newcoord), newcoord, 0, 1, ShapeFlags_Type(SHAPE_CENTER | SHAPE_WIN_REL | SHAPE_ZGRAD), Get_Explosion_Z(newcoord));
				Combat_Lighting(newcoord, damage, Rule->C4Warhead, false);
				Explosion_Damage(newcoord, damage, NULL, Rule->C4Warhead, true);
			}
			return;
		}
	} else {
		if (LinkedTo->TClass->IsHunterSeeker && LinkedTo->TarCom != NULL) {
			AbstractClass * target = LinkedTo->TarCom;
			coord = target->Center_Coord();
			if ((target->RTTI == RTTI_UNIT || target->RTTI == RTTI_INFANTRY) && ((FootClass*)target)->CurrentTube >= 0) {
				coord = ((FootClass*)target)->LastTubeCoord;
			}
			int height = Map.Get_Height_GL(coord);
			if (coord.Z < height) {
				coord.Z = height;
			}
			DestinationCoord = coord;
			DirType dir = Direction(LinkedTo->Center_Coord(), coord);
			LinkedTo->PrimaryFacing.Set(dir);
			LinkedTo->SecondaryFacing.Set(dir);
		}

		if (LinkedTo->Strength > 0) {

			/*
			**	If for some strange reason, there is a valid NavCom, but this aircraft is not
			**	in a movement order, then give it a movement order.
			*/
			if (LinkedTo->NavCom != NULL && LinkedTo->Mission == MISSION_GUARD && LinkedTo->MissionQueue == MISSION_NONE) {
				LinkedTo->Assign_Mission(MISSION_MOVE);
			}
		}
	}

	if (!Is_Moving()) {
		return;
	}

	const TechnoTypeClass *ttype = LinkedTo->TClass;
	bool is_dropship = ttype->IsDropship;
	bool is_hunter_seeker = ttype->IsHunterSeeker;

	LinkedTo->Mark(MARK_UP);

	coord = LinkedTo->Get_Coord();
	Physics(coord, LinkedTo->PrimaryFacing.Current());

	if (!Map.In_Radar(coord.As_Cell())) {
		Cell edge = Map.Cell_To_LocalRect(coord.As_Cell());
		if (edge.X < Map.MapRect.Width / 2) {
			coord.X += CELL_LEPTON_W / 2;
		} else {
			coord.X -= CELL_LEPTON_W / 2;
		}

		if (!Map.In_Radar(coord.As_Cell())) {
			coord = Coord_Scatter(coord, CELL_LEPTON / 4, false);
		}
	}

	if (Map.In_Radar(coord.As_Cell())) {
		LinkedTo->Set_Coord(coord);
	}

	int bridge_height = 0;
	int current_height = LinkedTo->HeightAGL;
	if (!LinkedTo->IsOnBridge && current_height >= BRIDGE_LEPTON_HEIGHT && Map[LinkedTo->Get_Coord()].IsUnderBridge) {
		current_height -= BRIDGE_LEPTON_HEIGHT;
		bridge_height = BRIDGE_LEPTON_HEIGHT;
	}

	Coord dest = DestinationCoord;
	Coord cur = LinkedTo->Get_Coord();
	int dist = Distance(Point2D(dest), Point2D(cur));

	if (current_height < FlightLevel && LinkedTo->Strength > 0) {
		bool is_loaded = false;
		IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
		if (flyctrl != NULL) {
			is_loaded = flyctrl->Is_Loaded() != 0;
		}

		if (is_dropship) {
			int climb = FlightLevel - current_height;
			if (climb > 16) {
				climb = 16;
			}
			LinkedTo->Set_Height_AGL(bridge_height + current_height + climb);
		} else if (is_hunter_seeker) {
			if (IsTakingOff) {
				int climb = FlightLevel - current_height;
				climb = std::min(Rule->HunterSeekerEmergeSpeed, climb);
				LinkedTo->Set_Height_AGL(bridge_height + current_height + climb);
			} else {
				int climb = FlightLevel - current_height;
				climb = std::min(Rule->HunterSeekerAscentSpeed, climb);
				LinkedTo->Set_Height_AGL(bridge_height + current_height + climb);
			}
		} else {
			int climb = FlightLevel - current_height;
			climb = std::min(is_loaded ? 10 : 20, climb);
			LinkedTo->Set_Height_AGL(bridge_height + current_height + climb);
		}

		LinkedTo->IsOnBridge = false;
	}

	if (current_height > FlightLevel || LinkedTo->Strength == 0) {
		int descent = current_height - FlightLevel;

		if (is_dropship) {
			if (IsLanding) {
				int limit = descent / 20 + 10;
				descent = std::min(std::min(limit, 48), descent);
			} else if (FlightLevel == LinkedTo->TClass->Flight_Level()) {
				if (current_height > 16) {
					descent = 16;
				} else {
					descent = current_height;
				}
			} else {
				if (current_height > 6) {
					descent = 6;
				} else {
					descent = current_height;
				}
			}
		} else if (is_hunter_seeker) {
			descent = std::min(Rule->HunterSeekerDescentSpeed, current_height);
		} else {
			if (descent / 20 < descent) {
				descent /= 20;
			}
			if (descent >= 50) {
				descent = 50;
			} else if (descent <= 20) {
				descent = 20;
			}
		}

		current_height -= descent;
		if (LinkedTo->Strength == 0 && current_height <= 1) {
			current_height = 1;
		} else if (current_height < 0) {
			current_height = 0;
		}

		if (current_height == 0 && bridge_height > 0) {
			LinkedTo->IsOnBridge = true;
			bridge_height = 0;
		}

		LinkedTo->Set_Height_AGL(bridge_height + current_height);

		if (LinkedTo->Strength > 0 && DestinationCoord != COORD_NONE) {
			if (LinkedTo->Mission != MISSION_ENTER || LinkedTo->NavCom == NULL ||
				LinkedTo->NavCom->RTTI != RTTI_BUILDING ||
				!((BuildingClass *)LinkedTo->NavCom)->Class->IsHelipad) {

				Coord center = Coord(Cell(DestinationCoord), 0);
				Coord current = LinkedTo->Get_Coord();
				LinkedTo->Set_Coord(Move_Coord(current, Direction(current, center), 5));
			}
		}
	}

	if (LinkedTo->Strength > 0 && Is_In_Flight() && DestinationCoord != COORD_NONE) {
		IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);

		if (!Needs_To_Land()) {
			TargetSpeed = 1.0;
		} else {
			if (LinkedTo->TClass->IsHunterSeeker) {
				if (!IsTakingOff && LinkedTo->TarCom != NULL) {
					TargetSpeed = 1.0;
				} else {
					TargetSpeed = 0.0;
				}
			} else {
				double speed = (double)dist / (double)ttype->SlowdownDistance;
				if (1.0 < speed) {
					speed = 1.0;
				}
				TargetSpeed = speed;

				if (TargetSpeed < 0.1) {
					if (dist > CELL_LEPTON / 3) {
						TargetSpeed = 0.1;
					} else {
						TargetSpeed = 0.0;
						CurrentSpeed *= 0.5;
					}
				}

				if ((double)dist < CurrentSpeed) {
					CurrentSpeed = (double)dist;
				}

				if (TargetSpeed == 0.0 && CurrentSpeed == 0.0 && dist > 0) {
					CurrentSpeed = 0.05;
				}
			}
		}
	}

	if (is_dropship && LinkedTo->Strength > 0 && CurrentSpeed > 0.0 && !IsTakingOff) {
		int slowdown = ttype->SlowdownDistance;
		if (dist < slowdown) {
			double factor = 1.0 - ((double)dist - ((double)slowdown * 0.6)) / ((double)slowdown * 0.4);
			FootClass * linked = LinkedTo;
			if (factor * linked->TClass->PitchAngle > linked->TClass->PitchAngle) {
				LinkedTo->PitchAngle = LinkedTo->TClass->PitchAngle;
			} else {
				LinkedTo->PitchAngle = factor * LinkedTo->TClass->PitchAngle;
			}
		}
	}

	if (!IsElevating && LinkedTo->Strength > 0 && !IsLanding && !IsTakingOff) {
		if (DestinationCoord.As_Cell() == LinkedTo->Get_Cell() && TargetSpeed == 0.0 && (!LinkedTo->TarCom || !LinkedTo->Ammo)) {
			Land();
		}
	}

	if (LinkedTo->Strength > 0) {
		if (CurrentSpeed < TargetSpeed) {
			CurrentSpeed = std::min(CurrentSpeed + 0.1, TargetSpeed);
		} else if (CurrentSpeed > TargetSpeed) {
			CurrentSpeed = std::max(CurrentSpeed - 0.1, TargetSpeed);
		}
	}

	LinkedTo->Mark(MARK_DOWN);

	BuildingClass * building = Map[LinkedTo->Get_Coord()].Cell_Building();
	if (building != NULL && building->Class->IsFirestormWall && building->House->FirestormDefenseActivated && !LinkedTo->TClass->IsIgnoresFirestorm) {
		building->Crossing_Firestorm(LinkedTo, true);
	}
}


/***********************************************************************************************
 * FlyClass::Physics -- Performs vector physics (movement).                                    *
 *                                                                                             *
 *    This routine performs movement (vector) physics. It takes the                            *
 *    specified location and moves it according to the facing and speed                        *
 *    of the vector. It returns the status of the move.                                        *
 *                                                                                             *
 * INPUT:   coord -- Reference to the coordinate that the vector will                          *
 *                   be applied to.                                                            *
 *                                                                                             *
 * OUTPUT:  Returns with the status of the vector physics. This could                          *
 *          range from no effect, to exiting the edge of the world.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   04/24/1994 JLB : Created.                                                                 *
 *   06/05/1995 JLB : Simplified to just do movement.                                          *
 *=============================================================================================*/
ImpactType FlyLocomotionClass::Physics(Coord & coord, DirType facing)
{
	int actual = Apparent_Speed();

	/*
	**	If movement occurred that is at least one
	**	pixel, then check update the coordinate and
	**	check for edge of world collision.
	*/
	if (actual > 0) {
		Coord		newcoord;		// New working coordinate.
		newcoord = Move_Coord(coord, facing, actual);

		/*
		**	Remember the new position.
		*/
		coord = newcoord;

		/*
		**	If the new coordinate is off the edge of the world, then report
		**	this.
		*/
		if (!Map.In_Radar(coord.As_Cell())) {
			return(IMPACT_EDGE);
		}

		return(IMPACT_NORMAL);
	}
	return(IMPACT_NONE);
}


/***********************************************************************************************
 * AircraftClass::Rotation_AI -- Handle aircraft body and flight rotation.                     *
 *                                                                                             *
 *    This will process the aircraft visible body and flight model rotation operations. If     *
 *    any rotation occurred, the aircraft will be flagged to be redrawn.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once per aircraft per game logic loop.                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/29/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void FlyLocomotionClass::Rotation_AI(void)
{
	if (IsTumbling && LinkedTo->Strength > 0) {
		if (!Is_Powered()) {
			DirType newdir = DirType(LinkedTo->PrimaryFacing.Current()) + DirType((Dir256)CurrentROT);
			LinkedTo->PrimaryFacing.Set(newdir);
			int diff = std::min(abs(CurrentROT), 1);
			if (CurrentROT < 0) {
				CurrentROT += diff;
			} else {
				CurrentROT -= diff;
			}
		} else {
			IsTumbling = false;
			Map[(Coord const &)LinkedTo->PositionCoord].Trigger_Veins();
		}
	}
}


/***********************************************************************************************
 * AircraftClass::Process_Take_Off -- State machine support for taking off.                    *
 *                                                                                             *
 *    This routine is used by the main game state machine processor. This utility routine      *
 *    handles a helicopter as it transitions from landed to flying state.                      *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Has the helicopter reached flight level now?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool FlyLocomotionClass::Process_Take_Off(void)
{
	bool took_off = false;
	int height = LinkedTo->HeightAGL;
	if (!LinkedTo->IsOnBridge && Map[(Coord const &)LinkedTo->PositionCoord].IsUnderBridge && height >= BRIDGE_LEPTON_HEIGHT) {
		height -= BRIDGE_LEPTON_HEIGHT;
	}

	IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
	int landing_altitude = 0;
	if (flyctrl) {
		landing_altitude = flyctrl->Landing_Altitude();
	}

	if (height >= FlightLevel) {
		IsTakingOff = false;
		IsLanding = false;
		took_off = true;
	}

	int relative_height = height - landing_altitude;
	int relative_flight_level = FlightLevel - landing_altitude;

	if (relative_height > relative_flight_level - (relative_flight_level / 3)) {
		LinkedTo->SecondaryFacing.Set_Desired(LinkedTo->PrimaryFacing.Desired());
	} else if (relative_height > relative_flight_level / 2) {
		DirType dir = Direction(LinkedTo->Center_Coord(), DestinationCoord);
		LinkedTo->PrimaryFacing.Set_Desired(dir);
		TargetSpeed = 1;
	}

	return(took_off);
}


/***********************************************************************************************
 * AircraftClass::Process_Landing -- Landing process state machine handler.                    *
 *                                                                                             *
 *    This is a support routine that is called by the main state machine routines. This        *
 *    routine is responsible for handling the helicopter as it transitions from flight to      *
 *    landing.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Has the helicopter completely landed now?                                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/1995 JLB : Created.                                                                 *
 *   03/04/1996 JLB : Handles fixed wing aircraft.                                             *
 *=============================================================================================*/
bool FlyLocomotionClass::Process_Landing(void)
{
	if (!IsLanding) {
		return(true);
	}

	if (LinkedTo->TClass->IsHunterSeeker && LinkedTo->TarCom == NULL) {
		Acquire_Hunter_Seeker_Target();
		if (LinkedTo->TarCom) {
			IsLanding = false;
			FlightLevel = LinkedTo->TClass->Flight_Level();
			if (LinkedTo->In_Radio_Contact() != NULL) {
				LinkedTo->Transmit_Message(RADIO_OVER_OUT);
			}
			LinkedTo->Assign_Mission(MISSION_ATTACK);
			LinkedTo->Commence();
			return(false);
		}
	}

	bool has_landed = false;

	int height = LinkedTo->HeightAGL;
	if (Map[(Coord const &)LinkedTo->PositionCoord].IsUnderBridge && height >= BRIDGE_LEPTON_HEIGHT) {
		height -= BRIDGE_LEPTON_HEIGHT;
	}

	if (LinkedTo->TClass->IsDropship && height == 0) {
		if (LinkedTo->PitchAngle > 0) {
			static const double _dropship_pitch_rate = 0.02;
			LinkedTo->PitchAngle = std::max(0.0, LinkedTo->PitchAngle - _dropship_pitch_rate);
		}
	}

	TargetSpeed = 0;

	int landing_altitude = 0;
	IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
	if (flyctrl) {
		landing_altitude = flyctrl->Landing_Altitude();
	}

	bool ok = false;

	if (LinkedTo->Can_Enter_Cell(&Map[DestinationCoord]) == MOVE_OK) {
		ok = true;
	} else {
		Take_Off();
		Cell pos = LinkedTo->PositionCoord.As_Cell();
		Cell nearby = Map.Nearby_Location(pos, SPEED_TRACK, -1, MZONE_FLYER);
		if (nearby != CELL_NONE) {
			Coord nearby_coord = nearby.As_Coord();
			nearby_coord.Z = Map.Get_Height_GL(nearby_coord);
			if (Map[nearby].IsUnderBridge) {
				nearby_coord.Z += BRIDGE_LEPTON_HEIGHT;
			}
			Move_To(nearby_coord);
			ok = true;
		}
	}

	if (!ok) {
		LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, NULL, true, true);
		DestinationCoord = COORD_NONE;
		return(false);
	}

	if (!CommencedLanding && height < 300) {
		Coord coord = LinkedTo->PositionCoord;
		coord.Z = Map.Get_Height_GL(coord);
		CommencedLanding = true;

		if (LinkedTo->TClass->IsDropship) {
			new AnimClass(AnimTypes[AnimTypeClass::From_Name("DROPLAND")], coord);
		} else if (LinkedTo->RTTI == RTTI_AIRCRAFT && ((AircraftClass*)LinkedTo)->Class->IsCarryall) {
			new AnimClass(AnimTypes[AnimTypeClass::From_Name("CARYLAND")], coord);
		}

		if (LinkedTo->Strength > 0) {
			Sound_Effect(LinkedTo->TClass->AuxSound2, coord);
		}
	}

	if (height <= landing_altitude) {
		if (LinkedTo->PitchAngle <= 0) {

			Coord coord = LinkedTo->Get_Coord();
			if (Map[coord].IsUnderBridge) {
				if (coord.Z >= BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(coord)) {
					LinkedTo->IsOnBridge = true;
				}
			}

			LinkedTo->HeightAGL = landing_altitude;
			IsLanding = false;
			IsTakingOff = false;
			has_landed = true;
			LinkedTo->Set_Speed(0);
			CurrentSpeed = 0;
			TargetSpeed = 0;

			CellClass * cptr;
			FacingType face;
			if (LinkedTo->LastAdjacencyCell != Cell(0, 0)) {
				for (face = FACING_FIRST; face < FACING_COUNT; face++) {
					Cell adjacent = Adjacent_Cell(LinkedTo->LastAdjacencyCell, face);
					cptr = &Map[adjacent];
					cptr->AdjacentObjectCount--;
				}
				LinkedTo->LastAdjacencyCell = LinkedTo->PositionCell;
				for (face = FACING_FIRST; face < FACING_COUNT; face++) {
					Cell adjacent = Adjacent_Cell(LinkedTo->LastAdjacencyCell, face);
					cptr = &Map[adjacent];
					cptr->AdjacentObjectCount++;
				}
			} else {
				LinkedTo->LastAdjacencyCell = LinkedTo->PositionCell;
				for (face = FACING_FIRST; face < FACING_COUNT; face++) {
					Cell adjacent = Adjacent_Cell(LinkedTo->LastAdjacencyCell, face);
					cptr = &Map[adjacent];
					cptr->AdjacentObjectCount++;
				}
			}

			if (LinkedTo->Center_Coord().As_Cell() == DestinationCoord.As_Cell() || Map[DestinationCoord.As_Cell()].Cell_Building() == LinkedTo->Contact_With_Whom()) {
				IsMoving = false;
				Move_To(COORD_NONE);
				LinkedTo->Assign_Destination(NULL);
			}
		}
	}

	return(has_landed);
}


/// <summary>
/// Handles the approach to the destination coordinate.
/// This routine is called while the aircraft has somewhere to be. It turns the body and
/// the turret toward the destination, settles on a flight level that clears the ground in
/// the way, and stages the speed down as the aircraft closes in. A hunter seeker drone
/// gets its own approach, which ends with the drone detonating on its victim.
/// </summary>
/// <param name="stage_approach">Should the approach speed and turret aim be staged for arrival?</param>
/// <returns>Returns with the distance remaining to the destination.</returns>
int FlyLocomotionClass::Nearing_Target(bool stage_approach, Coord coord)
{
	IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);

	/*
	 * A strafing aircraft that is over an ammo-bearing attack run should ignore the
	 * "nearing target" speed staging so that it can blast straight over the target.
	 */
	if (flyctrl != NULL && LinkedTo->TarCom != NULL && LinkedTo->Ammo && flyctrl->Is_Strafe()) {
		stage_approach = false;
	}

	/*
	 * Determine the distance to the target. If there is a building sitting on the target
	 * cell (and this isn't a hunter-seeker), measure to the center of that building instead.
	 */
	Coord aim = coord;
	BuildingClass * building = Map[Coord(coord).As_Cell()].Cell_Building();
	if (building != NULL && !LinkedTo->TClass->IsHunterSeeker) {
		aim = building->Docking_Coord();
	}

	LinkedTo->Get_Height();

	Point2D diff = Point2D(LinkedTo->Center_Coord()) - Point2D(aim);
	int dist = diff.Length();

	/*
	 * Turn the body of the aircraft to face the destination coordinate.
	 */
	if (flyctrl == NULL || !flyctrl->Is_Locked()) {
		DirType dir = Direction(LinkedTo->Center_Coord(), coord);

		bool turn = true;
		if (flyctrl != NULL && flyctrl->Is_Strafe()) {
			Coord here = LinkedTo->Center_Coord();
			if (!(Point2D(here).Distance_To(Coord(coord)) > CELL_LEPTON * 3 ||
				dir.Is_Complete_Turn(LinkedTo->PrimaryFacing.Current(), DIR_STEP_8))) {
				turn = false;
			}
		}

		if (turn) {
			LinkedTo->PrimaryFacing.Set_Desired(Direction(LinkedTo->Center_Coord(), coord));
		}
	}

	/*
	 * Aim the turret. If close to the destination and allowed, aim the turret at the
	 * current target (or the landing direction); otherwise keep it aimed at the destination.
	 */
	if (dist < CELL_LEPTON && stage_approach && !LinkedTo->TClass->IsHunterSeeker) {

		if (LinkedTo->TarCom != NULL && LinkedTo->Ammo &&
			(flyctrl == NULL || !flyctrl->Is_Strafe())) {

			if (flyctrl == NULL || !flyctrl->Is_Locked()) {
				LinkedTo->SecondaryFacing.Set_Desired(LinkedTo->Direction(LinkedTo->TarCom));
			}

		} else {
			DirType land(DIR_N);
			if (flyctrl != NULL) {
				land = DirType(FacingType(flyctrl->Landing_Direction()));
			}
			if (flyctrl == NULL || !flyctrl->Is_Locked()) {
				LinkedTo->SecondaryFacing.Set_Desired(land);
			}
		}

	} else {
		if (flyctrl == NULL || !flyctrl->Is_Locked()) {
			LinkedTo->SecondaryFacing.Set_Desired(Direction(LinkedTo->Center_Coord(), coord));
		}
	}

	/*
	 * A hunter-seeker has special logic as it approaches its target -- it detonates when
	 * very close, descends smoothly when somewhat close, and otherwise scans ahead to find
	 * a flight level that clears the terrain in front of it.
	 */
	bool flight_level_set = false;
	if (LinkedTo->TClass->IsHunterSeeker && LinkedTo->TarCom != NULL) {
		AbstractClass * tarcom = LinkedTo->TarCom;
		Coord here = LinkedTo->Get_Coord();
		Coord dst = DestinationCoord;
		int proximity = (Point2D(here) - Point2D(dst)).Length();

		if (proximity < Rule->HunterSeekerDetonateProximity) {

			/*
			 * Close enough to detonate. Take the target with it.
			 */
			TechnoClass * techno = Dynamic_Cast<TechnoClass *>(tarcom);
			if (techno != NULL) {
				WeaponTypeClass * weapon = LinkedTo->Get_Class_Weapon_Data(0)->Weapon;
				WarheadTypeClass const * warhead = weapon->WarheadPtr;
				int damage = weapon->Attack;

				techno->Take_Damage(damage, 0, warhead, LinkedTo, true, true);
				int damage2 = weapon->Attack;
				LinkedTo->Take_Damage(damage2, 0, warhead, NULL, true, true);

				int attack = weapon->Attack;
				Combat_Lighting(LinkedTo->Get_Coord(), attack, Rule->C4Warhead, false);
				Explosion_Damage(LinkedTo->Get_Coord(), attack, NULL, warhead, true);
				return(0);
			}

		} else if (proximity < Rule->HunterSeekerDescendProximity) {

			/*
			 * Descend smoothly toward the target as it gets closer.
			 */
			int targetz = tarcom->Center_Coord().Z;
			int floor = Map.Get_Height_GL(LinkedTo->Get_Coord());
			int level = floor + LinkedTo->TClass->Flight_Level();
			float ratio = (float)proximity / Rule->HunterSeekerDescendProximity;
			int newlevel = (int)std::lerp((double)targetz, (double)level, (double)ratio) - floor;
			if (newlevel < 10) {
				newlevel = 10;
			}
			FlightLevel = newlevel;
			flight_level_set = true;

		} else {

			/*
			 * Look ahead along the current heading to find the highest terrain that needs
			 * to be cleared, and raise the flight level if necessary.
			 */
			Coord scan = LinkedTo->Get_Coord();
			int floor = Map.Get_Height_GL(scan);
			if (Map[scan].IsUnderBridge) {
				floor += BRIDGE_LEPTON_HEIGHT;
			}
			int peak = floor;

			for (int look = 10; look != 0; look--) {
				DirType facing = LinkedTo->PrimaryFacing.Current();
				int speed = Apparent_Speed();
				if (speed > 0) {
					scan = Move_Coord(scan, facing, speed);
					Map.In_Radar(scan.As_Cell());
				}

				int height = Map.Get_Height_GL(scan);
				if (Map[scan].IsUnderBridge) {
					height += BRIDGE_LEPTON_HEIGHT;
				}
				if (height > peak) {
					peak = height;
				}
			}

			if (peak > floor) {
				FlightLevel = peak + LinkedTo->TClass->Flight_Level();
				flight_level_set = true;
			}
		}
	}

	/*
	 * Pick the flight level to head for. An elevating aircraft heads for the destination's
	 * ground level; dropships ease down as they near their slowdown distance.
	 */
	if (!flight_level_set) {
		if (IsElevating && dist < 3 * CELL_LEPTON) {
			FlightLevel = DestinationCoord.Z - Map.Get_Height_GL(DestinationCoord);
		} else {
			if (LinkedTo->TClass->IsDropship) {
				if (dist >= LinkedTo->TClass->SlowdownDistance) {
					FlightLevel = LinkedTo->TClass->Flight_Level();
				} else {
					float frac = (float)(dist / LinkedTo->TClass->SlowdownDistance);
					TechnoTypeClass const * ttype = LinkedTo->TClass;
					FlightLevel = (int)std::lerp((double)(LinkedTo->TClass->Flight_Level() / 3),
						(double)ttype->Flight_Level(), (double)frac);
				}
			} else {
				FlightLevel = LinkedTo->TClass->Flight_Level();
			}
		}
	}

	/*
	 * Stage the target speed by distance band so the aircraft slows down as it nears the
	 * destination. The non-hunter-seeker elevate teardown releases the fly-control interface.
	 */
	if (!LinkedTo->TClass->IsHunterSeeker) {
		if (Needs_To_Land()) {
			if (dist < CELL_LEPTON / 2) {
				if (stage_approach) {
					TargetSpeed = 0.0;
				}
				if (!IsElevating && CurrentSpeed < 0.05) {
					Land();
				}
			} else if (dist < CELL_LEPTON * 2) {
				if (stage_approach) {
					TargetSpeed = 0.5;
				}
			} else if (dist < CELL_LEPTON * 3) {
				if (stage_approach) {
					TargetSpeed = 0.75;
				}
			}
		}
	}

	/*
	 * If a non-player selected this aircraft, deselect it once it slips into shroud or fog.
	 */
	if (LinkedTo->IsSelected && !LinkedTo->House->Is_Player_Control()) {
		if (Map.Is_Shrouded(LinkedTo->Get_Coord()) ||
			(Scen->Special.IsFogOfWar && Map.Is_Fogged(LinkedTo->Get_Coord()))) {
			LinkedTo->Unselect();
		}
	}

	return(dist);
}


/// <summary>
/// Builds the transformation matrix for drawing the aircraft body.
/// This routine turns the body to its facing and then leans it into whatever attitude the
/// flight calls for -- nose down at speed, rolled into a turn, or following the rocking of
/// an aircraft that has been knocked about.
/// </summary>
/// <param name="key">Optional cache key for the resulting orientation. It may be NULL, and
/// is set to -1 for an attitude that is not worth caching.</param>
/// <returns>Returns with the matrix to draw the aircraft with.</returns>
Matrix3D FlyLocomotionClass::Draw_Matrix(int * key)
{
	Matrix3D mtx;
	mtx.Make_Identity();

	if (key && *key != -1) {
		*key <<= 5;
		*key |= LinkedTo->SecondaryFacing.Current().As_Dir32();
	}

	mtx.Rotate_Z(LinkedTo->SecondaryFacing.Current().As_Radian32());

	if (LinkedTo->HeightAGL > 0 || LinkedTo->TClass->IsDropship) {

		if (LinkedTo->IsRocking) {
			mtx.Rotate_X(LinkedTo->AngleRotatedSideways);

			if (CurrentSpeed > LinkedTo->TClass->PitchSpeed) {
				mtx.Rotate_Y(float(LinkedTo->AngleRotatedForwards + LinkedTo->TClass->PitchAngle));
			} else {
				mtx.Rotate_Y(LinkedTo->AngleRotatedForwards);
			}
			if (key) {
				*key = -1;
			}
			return(mtx);
		}

		if (!LinkedTo->TClass->IsDropship) {
			if (key && *key != -1) {
				*key <<= 1;
			}
			if (CurrentSpeed > LinkedTo->TClass->PitchSpeed && !LinkedTo->TClass->IsHunterSeeker) {
				if (key && *key != -1) {
					*key |= 1;
				}
				mtx.Rotate_Y(float(LinkedTo->TClass->PitchAngle));
			}
			if (key && *key != -1) {
				*key <<= 2;
			}

			if (CurrentSpeed > LinkedTo->TClass->PitchSpeed) {

				if (LinkedTo->SecondaryFacing.Is_Rotating_CW()) {
					if (key && *key != -1) {
						*key |= 1;
					}
					mtx.Rotate_X(float(LinkedTo->TClass->RollAngle));
				} else if (LinkedTo->SecondaryFacing.Is_Rotating_CCW()) {
					if (key && *key != -1) {
						*key |= 2;
					}
					mtx.Rotate_X(float(-LinkedTo->TClass->RollAngle));
				}
			}
			return(mtx);
		}

		if (LinkedTo->TClass->IsDropship) {
			if (key) {
				*key = -1;
			}
			int dist = (Point2D(LinkedTo->PositionCoord) - Point2D(DestinationCoord)).Length();
			if (dist < LinkedTo->TClass->SlowdownDistance && !IsTakingOff && LinkedTo->Mission != MISSION_RETREAT) {
				mtx.Rotate_Y(-LinkedTo->PitchAngle);
			}
		} else {
			if (key) {
				*key <<= 3;
			}
			if (LinkedTo->SecondaryFacing.Is_Rotating_CW()) {
				if (key) {
					*key |= 1;
				}
				mtx.Rotate_X(float(LinkedTo->TClass->RollAngle));
			} else if (LinkedTo->SecondaryFacing.Is_Rotating_CCW()) {
				if (key) {
					*key |= 2;
				}
				mtx.Rotate_X(float(-LinkedTo->TClass->RollAngle));
			}
		}
	}

	return(mtx);
}


/// <summary>
/// Fetches the drawing offset for the aircraft body.
/// This routine gives an airborne aircraft a gentle bobbing motion so that it does not
/// look pinned in place while it hovers. Dropships and grounded aircraft do not bob.
/// </summary>
/// <returns>Returns with the pixel offset to shift the aircraft by.</returns>
Point2D FlyLocomotionClass::Draw_Point(void)
{
	int y = 0;
	IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);

	int landing_altitude = 0;
	if (flyctrl) {
		landing_altitude = flyctrl->Landing_Altitude();
	}

	if (!LinkedTo->TClass->IsDropship && !IsLanding && !IsTakingOff && LinkedTo->HeightAGL > landing_altitude) {
		y = (std::sin((Frame % 20) * M_PI / 10)) * 1.5 + 0.5;
	}

	return(Point2D(0, y));
}


/// <summary>
/// Fetches the drawing offset for the aircraft's shadow.
/// The shadow is drawn where the aircraft's position puts it, so no adjustment is needed.
/// </summary>
/// <returns>Returns with the pixel offset to shift the shadow by.</returns>
Point2D FlyLocomotionClass::Shadow_Point(void)
{
	return(Point2D(0, 0));
}


/// <summary>
/// Tells the aircraft to lift off the ground.
/// This routine records the intention to climb and starts the engine sound, leaving the
/// climb itself to the landing and takeoff logic. A stunned aircraft stays put.
/// </summary>
void FlyLocomotionClass::Take_Off(void)
{
	if (LinkedTo->StunDuration <= 0) {
		IsLanding = false;
		IsTakingOff = true;
		FlightLevel = LinkedTo->TClass->Flight_Level();

		if (LinkedTo->HeightAGL == 0) {
			LinkedTo->PrimaryFacing.Set(LinkedTo->SecondaryFacing.Desired());
		}

		Sound_Effect(LinkedTo->TClass->AuxSound1, LinkedTo->PositionCoord);
	}
}


/// <summary>
/// Tells the aircraft to come down and land.
/// This routine only records the intention to land. The descent itself is performed over
/// time by the landing and takeoff logic.
/// </summary>
void FlyLocomotionClass::Land(void)
{
	IsTakingOff = false;
	IsLanding = true;
	CommencedLanding = false;
	FlightLevel = 0;
}


/// <summary>
/// Builds the transformation matrix for the aircraft's shadow.
/// This routine lays the shadow over the slope of the ground beneath the aircraft and
/// turns it to match the body facing. A dropship always casts a flat shadow.
/// </summary>
/// <param name="key">Optional cache key for the shadow orientation. It may be NULL, and a
/// value of -1 marks the shadow as not worth caching.</param>
/// <returns>Returns with the matrix to draw the shadow with.</returns>
Matrix3D FlyLocomotionClass::Shadow_Matrix(int * key)
{
	int ramp = Map[(Coord const &)LinkedTo->PositionCoord].Ramp;
	if (LinkedTo->TClass->IsDropship) {
		ramp = 0;
	}

	Matrix3D matrix = Get_Slope_Matrix(ramp);
	matrix.Rotate_Z(LinkedTo->SecondaryFacing.Current().As_Radian32());

	if (key != NULL && *key != -1) {
		*key = 32 * (ramp + (*key << 6));
		*key |= LinkedTo->SecondaryFacing.Current().As_Dir32();
	}
	return(matrix);
}


/// <summary>
/// Turns the aircraft body to the specified facing.
/// This routine snaps the body around immediately rather than rotating it over time.
/// </summary>
/// <param name="coord">The facing to set the aircraft body to.</param>
void FlyLocomotionClass::Do_Turn(DirType coord)
{
	LinkedTo->SecondaryFacing.Set(coord);
}


/// <summary>
/// Is the aircraft airborne enough to be treated as flying?
/// This routine tells the difference between an aircraft that is truly flying and one that
/// is still hugging the ground on its way up or down. An aircraft climbing away is not
/// counted as in flight until it is well clear of the deck.
/// </summary>
/// <returns>bool; Is the aircraft in flight?</returns>
bool FlyLocomotionClass::Is_In_Flight(void)
{
	if (!IsLanding && (!IsTakingOff || LinkedTo->HeightAGL >= FlightLevel / 2)) {
		return(true);
	}
	return(false);
}


ClassID FlyLocomotionClass::Class_ID(void) const
{
	return(ClassID_FlyerLocomotion);
}


/// <summary>
/// Lists the members this flying locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void FlyLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(DestinationCoord);
	stream.Serialize(HeadToCoord);
	stream.Serialize(IsMoving);
	stream.Serialize(FlightLevel);
	stream.Serialize(TargetSpeed);
	stream.Serialize(CurrentSpeed);
	stream.Serialize(IsTakingOff);
	stream.Serialize(IsLanding);
	stream.Serialize(CommencedLanding);
	stream.Serialize(IsTumbling);
	stream.Serialize(CurrentROT);
	stream.Serialize(Riser);
	stream.Serialize(IsElevating);
}


/// <summary>
/// Determines which render layer the aircraft belongs in.
/// </summary>
/// <returns>Returns with LAYER_GROUND while the aircraft is on the deck, or LAYER_TOP once
/// it is above it.</returns>
LayerType FlyLocomotionClass::In_Which_Layer(void)
{
	return(LinkedTo->HeightAGL <= 0 ? LAYER_GROUND : LAYER_TOP);
}


/// <summary>
/// Cuts the power to the aircraft.
/// This routine is called when the aircraft is drained of power, such as by an ion storm
/// or an EMP pulse. An aircraft that was under way is thrown into a tumble and told to
/// stop, so that it falls out of the sky rather than coasting on to its objective.
/// </summary>
/// <returns>bool; Was the power successfully cut?</returns>
bool FlyLocomotionClass::Power_Off(void)
{
	if (Is_Moving()) {
		Tumble();
		Stop_Moving();
		Riser = 0;
	}
	return(BASECLASS::Power_Off());
}


/// <summary>
/// Is the aircraft still under power?
/// </summary>
/// <returns>bool; Does the aircraft still have power?</returns>
bool FlyLocomotionClass::Is_Powered(void)
{
	return(BASECLASS::Is_Powered());
}


/// <summary>
/// Is this aircraft bothered by an ion storm?
/// Hunter seeker drones fly through a storm unhindered; every other aircraft is grounded
/// by one.
/// </summary>
/// <returns>bool; Does an ion storm affect this aircraft?</returns>
bool FlyLocomotionClass::Is_Ion_Sensitive(void)
{
	return(!LinkedTo->TClass->IsHunterSeeker);
}


/// <summary>
/// Sends the aircraft into an out of control tumble.
/// This routine is used when the aircraft is knocked out of the sky. It sets the body
/// spinning in a randomly chosen direction so that the wreck looks suitably doomed on the
/// way down.
/// </summary>
void FlyLocomotionClass::Tumble(void)
{
	IsTumbling = true;
	CurrentROT = Random_Pick(20, 30);
	if (Percent_Chance(50)) {
		CurrentROT = -CurrentROT;
	}
}


/// <summary>
/// Fetches the speed the aircraft is currently traveling at.
/// </summary>
/// <returns>Returns with the distance the aircraft will cover in one game frame.</returns>
int FlyLocomotionClass::Apparent_Speed(void)
{
	return(LinkedTo->TClass->MaxSpeed * CurrentSpeed);
}


/// <summary>
/// Fetches the broad flight status of the aircraft.
/// This routine is used by outside logic that needs to know what the aircraft is busy
/// doing without having to examine the locomotor itself.
/// </summary>
/// <returns>Returns with the status code for taking off, landing, moving, or sitting
/// idle.</returns>
int FlyLocomotionClass::Get_Status(void)
{
	if (IsLanding) {
		return(1);
	}
	if (IsTakingOff) {
		return(0);
	}
	if (Is_Moving()) {
		return(2);
	}
	return(3);
}


/// <summary>
/// Picks a victim for the hunter seeker drone.
/// This routine is called while the drone is loose and has nothing to chase. It gathers
/// every legal enemy object on the map and picks one at random, favoring human owned
/// targets in a multiplay game so that the drone makes a nuisance of itself where it will
/// be noticed.
/// </summary>
void FlyLocomotionClass::Acquire_Hunter_Seeker_Target(void)
{
	if (LinkedTo->TarCom == NULL) {

		DynamicVectorClass<ObjectClass *> aitargets;
		DynamicVectorClass<ObjectClass *> humantargets;

		bool atenemy = Session.Type != GAME_NORMAL && LinkedTo->House->Enemy != HOUSE_NONE && !LinkedTo->House->Is_Human_Player();

		for (int i = Technos.Count() - 1; i >= 0; i--) {
			TechnoClass * techno = Technos[i];

			bool enemy = true;
			if (!atenemy) {
				if (LinkedTo->House->Is_Ally(techno->House)) {
					enemy = false;
				}
			} else {
				if (techno->House != Houses[LinkedTo->House->Enemy]) {
					enemy = false;
				}
			}

			if (enemy && techno->Strength > 0 && !techno->IsInLimbo && techno->IsActive &&
				!techno->TClass->IsInvisible && techno->TClass->IsLegalTarget &&
				!(Scen->Special.IsHarvesterImmune && techno->RTTI == RTTI_UNIT && ((UnitClass*)techno)->Class->IsToHarvest)) {

				if (Session.Type != GAME_NORMAL && LinkedTo->House->Is_Human_Player() && !techno->House->Class->IsMultiplayPassive &&
					(techno->RTTI == RTTI_UNIT || techno->RTTI == RTTI_BUILDING || techno->RTTI == RTTI_INFANTRY || techno->RTTI == RTTI_AIRCRAFT)) {
					humantargets.Add(techno);
				} else {
					aitargets.Add(techno);
				}
			}
		}

		if (humantargets.Count() > 0) {
			LinkedTo->Assign_Target(humantargets[Scen->RandomNumber(0, humantargets.Count() - 1)]);
		} else if (aitargets.Count() > 0) {
			LinkedTo->Assign_Target(aitargets[Scen->RandomNumber(0, aitargets.Count() - 1)]);
		}
	}
}


/// <summary>
/// Does the aircraft need to settle down toward the ground?
/// This routine is used while nearing a destination to decide whether the approach should
/// be staged down into a landing. A strafing aircraft with ammunition left is exempt, since
/// it is making an attack run rather than an approach.
/// </summary>
/// <returns>bool; Should the aircraft slow down and land?</returns>
bool FlyLocomotionClass::Needs_To_Land(void)
{
	if (IsLanding) {
		return(true);
	}

	if (!IsElevating) {
		return(true);
	}

	IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
	if (flyctrl != NULL && !flyctrl->Is_Strafe()) {
		return(true);
	}

	if (LinkedTo->Ammo == 0) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Is the aircraft locked into level straight flight?
/// This routine is consulted when a destination is assigned in order to decide whether the
/// aircraft should hold its altitude instead of easing down toward the ground. A high
/// destination, a loaded weapon aimed at a target, or a locked fly control will all keep
/// the aircraft flying straight.
/// </summary>
/// <returns>bool; Must the aircraft hold a level straight course?</returns>
bool FlyLocomotionClass::Is_Locked_To_Straight_Flight(void)
{
	if (DestinationCoord.Z > Map.Get_Height_GL(DestinationCoord) + 120) {
		return(true);
	}
	if (LinkedTo->TarCom != NULL && LinkedTo->Ammo) {
		return(true);
	}

	IFlyControl * const flyctrl = dynamic_cast<IFlyControl *>(LinkedTo);
	if (flyctrl) {
		if (flyctrl->Is_Locked()) {
			return(true);
		}
	}
	return(false);
}
