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

#include "always.h"

#include "hover.h"

#include "_map.h"
#include "_rules.h"
#include "anim.h"
#include "building.h"
#include "builtype.h"
#include "ccrand.h"
#include "cell.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "infantry.h"
#include "inline.h"
#include "ion.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "tube.h"
#include "unit.h"

#include "tube.hh"

#include <algorithm>


/// <summary>
/// Creates a hover locomotor.
/// The locomotor comes into being grounded and idle. It is of no use until it has been
/// linked to the object that it is to drive.
/// </summary>
HoverLocomotionClass::HoverLocomotionClass(void) :
	DestinationCoord(COORD_NONE),
	HeadToCoord(COORD_NONE),
	Facing(0),
	Height(0),
	Acceleration(0),
	Boost(1),
	Bounciness(0),
	WasShoved(false),
	ShoveAccum(0),
	WasPushed(false)
{
}


/// <summary>
/// Attaches this locomotor to the object that it will drive.
/// The facing tracker is primed from the object's rate of turn, since the hover drive
/// slews the hull about far more freely than a tracked one would.
/// </summary>
/// <param name="pointer">Pointer to the object this locomotor will drive.</param>
/// <returns>Returns with the result of the attach operation.</returns>
HRESULT STDMETHODCALLTYPE HoverLocomotionClass::Link_To_Object(void *pointer)
{
	HRESULT res = BASECLASS::Link_To_Object(pointer);
	FacingClass face(2 * LinkedTo->TClass->ROT);
	Facing = face;
	return(res);
}


/// <summary>
/// Destroys the hover locomotor.
/// </summary>
HoverLocomotionClass::~HoverLocomotionClass(void)
{
	//nothing
}


/// <summary>
/// Handles the rise and fall of the hover cushion.
/// This routine is called every frame to keep the object floating at its rest height. It
/// adds the gentle bobbing that gives hover units their character, pushes back when the
/// object has sunk too low, and lets an unpowered one sag onto the ground.
/// </summary>
void HoverLocomotionClass::Gravity_AI(void)
{
	int height = LinkedTo->HeightAGL;
	int clearance = height;

	if (LinkedTo->Path[0] != FACING_NONE) {
		int mheight = Map.Get_Height_GL(LinkedTo->PositionCoord);
		if (Map.Get_Height_GL(Adjacent_Cell(LinkedTo->PositionCoord, LinkedTo->Path[0])) > mheight) {
			clearance -= Rule->HoverHeight;
		}
	}

	height = (int)(height + Bounciness);
	int id = LinkedTo->Fetch_ID();

	double bob_time_span = (id & 1 ? 1.0 : 1.1) * Rule->HoverBob * TICKS_PER_MINUTE;

	double bob_angle = std::sin((double)((Frame + 2 * id) % (int)bob_time_span) * DEG_TO_RAD(360) / bob_time_span);

	height = int(2 * bob_angle + height);
	if (height < 0) {
		Bounciness = 0;
		height = 0;
	}

	bool wasdown = LinkedTo->IsDown;
	LinkedTo->IsDown = false;
	LinkedTo->HeightAGL = height;
	LinkedTo->IsDown = wasdown;

	if (clearance < Rule->HoverHeight) {
		if (BASECLASS::Is_Powered()) {
			Bounciness += ((double)Rule->HoverHeight + (double)Rule->HoverHeight - (double)clearance) / (double)Rule->HoverHeight * (double)Rule->Gravity;
		}
		if (clearance < Rule->HoverHeight / 4) {
			Bounciness += (double)(Rule->Gravity / 3);
		}
	}
	Bounciness -= Rule->Gravity;
	Bounciness *= Rule->HoverDampen;
}


/// <summary>
/// Fetches the transformation matrix to draw the object with.
/// A powered hover unit rides level regardless of the ground, but one that has settled
/// takes on the slope of the cell beneath it. The key, when one is supplied, is updated so
/// that the render cache can recognize this pose again.
/// </summary>
/// <param name="key">Pointer to the render cache key to update; may be NULL.</param>
/// <returns>Returns with the matrix to render the object with.</returns>
Matrix3D STDMETHODCALLTYPE HoverLocomotionClass::Draw_Matrix(int *key)
{
	if (!Is_Powered()) {
		int ramp = Map[(Coord const &)(LinkedTo->PositionCoord)].Ramp;
		Matrix3D mtx = Get_Slope_Matrix(ramp);
		mtx.Rotate_Z(LinkedTo->PrimaryFacing.Current().As_Radian32());
		if (key != NULL && *key != -1) {
			*key = 32 * (ramp + (*key << 6));
			*key |= LinkedTo->PrimaryFacing.Current().As_Dir32();
		}
		return(mtx);
	}
	return(BASECLASS::Draw_Matrix(key));
}


/// <summary>
/// Handles the per frame processing for a hovering object.
/// This is the main driver routine for the locomotor. It steers and accelerates the
/// object, walks it along its path a cell at a time, kicks up a wake when it passes over
/// water, and applies the bob and sag of the hover cushion.
/// </summary>
/// <returns>bool; Is the object still moving?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Process(void)
{
	if (Is_Moving() && Is_Moving1()) {
		Motion_AI();

		int height = LinkedTo->Height;
		int speed = LinkedTo->Current_Speed() * Acceleration;
		DirType direction = Direction(LinkedTo->Center_Coord(), HeadToCoord);

		if (speed == 0 && Facing.Current().Is_Complete_Turn(direction, 0)) {
			Start();
			speed = LinkedTo->Current_Speed() * Acceleration;
		}

		if (speed >= Point2D(LinkedTo->PositionCoord).Distance_To(HeadToCoord)) {
			LinkedTo->IsOccupyingCell = true;
			LinkedTo->IsToPathAroundBlockage = false;
			WasPushed = false;

			Start_Of_Move(0);

			if (LinkedTo->Path[0] == FACING_NONE && DestinationCoord == COORD_NONE) {
				HeadToCoord = COORD_NONE;
			}

			if (DestinationCoord != COORD_NONE) {
				MoveType ok = While_Moving(true);
				if (!LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(MOVE_NO);
				if (ok == MOVE_OK && HeadToCoord == COORD_NONE) {
					/*
					**	Perform "per cell" activities.
					*/
					LinkedTo->Per_Cell_Process(PCP_END);
					if (LinkedTo == NULL || !LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(false);
				}
				if (ok == MOVE_NO) {
					if (LinkedTo->CurrentTube >= TUBE_FIRST) return(false);
					Stop();
				} else if (ok != MOVE_OK) {
					Stop_Driver();
					speed = 0;
				}
			}
		}

		if (speed > 0) {
			Coord coord = LinkedTo->PositionCoord;

			if (LinkedTo->Occupies_Cells() && HeadToCoord != COORD_NONE) {
				LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
				LinkedTo->IsOccupyingCell = false;
				LinkedTo->IsToPathAroundBlockage = false;
			}

			coord = Move_Coord(coord, Facing.Current(), speed);
			if (coord.As_Cell() != LinkedTo->Get_Coord().As_Cell()) {
				LinkedTo->Mark(MARK_UP);
				LinkedTo->PositionCoord = coord;
				LinkedTo->Height = height;
				CellClass * cellptr = &Map[coord];
				if (!LinkedTo->IsOnBridge && cellptr->IsUnderBridge && LinkedTo->HeightAGL >= BRIDGE_LEPTON_HEIGHT) {
					LinkedTo->IsOnBridge = true;
				}
				if (LinkedTo->IsOnBridge == true && !cellptr->IsUnderBridge) {
					LinkedTo->IsOnBridge = false;
				}
				LinkedTo->Mark(MARK_DOWN);
			} else {
				bool wasdown = LinkedTo->IsDown;
				LinkedTo->IsDown = false;
				LinkedTo->PositionCoord = coord;
				LinkedTo->Height = height;
				LinkedTo->IsDown = wasdown;
			}
		}
	}

	if (Is_Moving_Now() && (Frame % 10) == 0) {
		if (!LinkedTo->IsOnBridge && LinkedTo->Get_Cell_Ptr()->Land_Type() == LAND_WATER) {
			if (Rule->Wake != NULL) {
				new AnimClass(Rule->Wake, LinkedTo->Get_Coord());
			}
		}
	}

	Gravity_AI();

	if (WasShoved) {
		if (Is_Powered()) {
			DirType dir = LinkedTo->PrimaryFacing.Current();
			LinkedTo->PrimaryFacing.Set(dir + Dir256(ShoveAccum));
			int delta = abs(ShoveAccum);
			if (delta >= 1) {
				delta = 1;
			}
			if (ShoveAccum < 0) {
				ShoveAccum += delta;
			} else {
				ShoveAccum -= delta;
			}
			if (ShoveAccum == 0) {
				WasShoved = false;
			}
		} else {
			WasShoved = false;
		}
		if (!WasShoved) {
			Map[LinkedTo->Get_Coord()].Trigger_Veins();
			if (Map[LinkedTo->Get_Coord()].Land_Type() == LAND_WATER) {
				if (LinkedTo->Get_Coord().Z < Map.Get_Height_GL(LinkedTo->Get_Coord()) + LEVEL_LEPTON_H) {
					LinkedTo->Fall_From_Height();
				}
			}
		}
	}

	return(Is_Moving());
}


/// <summary>
/// Does the object have a move order outstanding?
/// </summary>
/// <returns>bool; Is the object either headed somewhere or bound for a destination?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Is_Moving(void)
{
	return(DestinationCoord != COORD_NONE || HeadToCoord != COORD_NONE);
}


/// <summary>
/// Is the object actually under way?
/// An object that has been given a move order but has not yet lifted onto its cushion is
/// moving, but it is not moving now.
/// </summary>
/// <returns>bool; Is the object traveling at this moment?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Is_Moving_Now(void)
{
	return(Is_Moving() && Height != 0.0);
}


/// <summary>
/// Fetches the final destination of the hovering object.
/// </summary>
/// <returns>Returns with the destination coordinate, or COORD_NONE if the object has no
/// move order outstanding.</returns>
Coord STDMETHODCALLTYPE HoverLocomotionClass::Destination(void)
{
	if (DestinationCoord != COORD_NONE) {
		return(DestinationCoord);
	}
	return(COORD_NONE);
}


/// <summary>
/// Fetches the spot the object is immediately headed for.
/// </summary>
/// <returns>Returns with the intermediate destination, or with the object's current
/// position if it is not headed anywhere.</returns>
Coord STDMETHODCALLTYPE HoverLocomotionClass::Head_To_Coord(void)
{
	if (HeadToCoord != COORD_NONE) {
		return(HeadToCoord);
	}
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Assigns a new destination to the hovering object.
/// This is the entry point the owning object uses to hand a move order to the locomotor.
/// The destination is dropped onto the terrain -- or onto the bridge deck above it -- and
/// the drive is started if the object is not already under way.
/// </summary>
/// <param name="to">The coordinate to move to.</param>
void STDMETHODCALLTYPE HoverLocomotionClass::Move_To(Coord to)
{
	DestinationCoord = to;
	if (Is_Powered() && Is_Ion_Sensitive() && IonStormClass::Is_Ion_Storm_Active()) {
		Power_Off();
	}
	DestinationCoord.Z = Map.Get_Height_GL(DestinationCoord);

	if (Map[to].IsUnderBridge) {
		DestinationCoord.Z += BRIDGE_LEPTON_HEIGHT;
	}

	if (to != COORD_NONE && Is_Powered()) {
		if (!Is_Moving_Now()) {
			LinkedTo->Set_Speed(1.0);
			Start_Of_Move(0);
			While_Moving(true);
		} else if (HeadToCoord == COORD_NONE) {
			Start();
		}
	}
}


/***********************************************************************************************
 * DriveClass::While_Moving -- Processes unit movement.                                        *
 *                                                                                             *
 *    This routine is used to process movement for the units as they move.                     *
 *    It is called many times for each cell's worth of movement.   This                        *
 *    routine only applies after the next cell HeadTo has been determined.                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  true/false; Should this routine be called again?                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/1992 JLB : Created.                                                                 *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
MoveType HoverLocomotionClass::While_Moving(bool first_pass)
{
	assert(LinkedTo->IsActive);

	/*
	**	See if "per cell" processing is necessary.
	*/
	if (HeadToCoord != COORD_NONE) {
		LinkedTo->Clear_Occupy_Bit(HeadToCoord);
		HeadToCoord = COORD_NONE;
	}

	/*
	**	Perform quick legality checks.
	*/
	if (!Is_Moving() || DestinationCoord == COORD_NONE) {
		return(MOVE_OK);
	}

	FacingType		nextface;		// Next facing queued in path.
	nextface = LinkedTo->Path[0];

	if (nextface == FACING_COUNT) {
		TubeType tubenum = (TubeType)Map[LinkedTo->Get_Coord()].Tube;
		if (tubenum >= TUBE_FIRST && tubenum < Tubes.Count()) {
			LinkedTo->Mark(MARK_UP);
			LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
			TubeClass * tube = Tubes[tubenum];
			Cell exit = tube->Exit;
			HeadToCoord = exit.As_Coord();
			LinkedTo->Advance_Path(1);
			LinkedTo->LastPathingCell = HeadToCoord.As_Cell();
			LinkedTo->CurrentTube = tubenum;
			LinkedTo->CurrentTubeDir = FACING_FIRST;
			LinkedTo->LastTubeCoord = Map[Adjacent_Cell((Cell)tube->Enter, tube->Dirs[0])].Cell_Coord() - Coord((Cell)tube->Enter) + LinkedTo->PositionCoord;
			int height = LinkedTo->Height;
			int current_height = Map.Get_Height_GL(LinkedTo->PositionCoord);
			int count = tube->Count;
			exit = tube->Exit;
			LinkedTo->LastTubeCoord.Z = height + (Map.Get_Height_GL(exit) - current_height) / count;
		} else {
			LinkedTo->Path[0] = FACING_NONE;
			HeadToCoord = COORD_NONE;
			Stop();
			LinkedTo->Assign_Destination(NULL);
		}
		return(MOVE_NO);
	}

	if (nextface != FACING_NONE) {
		HeadToCoord = Adjacent_Cell(LinkedTo->PositionCell, nextface).As_Coord();
		HeadToCoord.Z = Map.Get_Height_GL(HeadToCoord);
		if (LinkedTo->PositionCoord.Z >= HeadToCoord.Z + 2 * LEVEL_LEPTON_H + LEVEL_LEPTON_H) {
			HeadToCoord.Z += BRIDGE_LEPTON_HEIGHT;
		}

		/*
		**	Check for crate goodie finder here.
		*/
		if (!Map[HeadToCoord].Goodie_Check(LinkedTo) && !LinkedTo->IsInLimbo) {
			if (LinkedTo->IsActive && !LinkedTo->IsFalling) {
				LinkedTo->Path[0] = FACING_NONE;
				HeadToCoord = COORD_NONE;
				Stop();
			}
			return(MOVE_NO);
		}

		if (!LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(MOVE_NO);

		if (LinkedTo->IsOnBridge != (int)Map[Adjacent_Cell(LinkedTo->PositionCoord, nextface)].IsUnderBridge) {
			LinkedTo->IsPlanningToLook = true;
		}

		Cell c = HeadToCoord;
		MoveType ok = LinkedTo->Can_Enter_Cell(&Map[c], nextface, LinkedTo->Get_Cell_Height());

		switch (ok) {
		case MOVE_CLOAK:
		case MOVE_NO:
			Map[HeadToCoord].Shimmer();
			if (first_pass) {
				Stop_Driver();
				LinkedTo->PathDelay = 0;
				LinkedTo->Path[0] = FACING_NONE;
				Start_Of_Move(0);
				ok = While_Moving(false);
			}
			if (ok != MOVE_OK) {
				HeadToCoord = COORD_NONE;
				Stop_Driver();
			}
			return(ok);

		case MOVE_CLOSED_GATE:
			HeadToCoord = COORD_NONE;
			Map.Try_Open_Gate(LinkedTo, c);
			Stop_Driver();
			return(ok);

		case MOVE_TEMP:
			if (first_pass) {
				Stop_Driver();
				LinkedTo->PathDelay = 0;
				LinkedTo->Path[0] = FACING_NONE;
				Start_Of_Move(0);
				ok = While_Moving(false);
			} else {
				if (LinkedTo->Center_Coord().Distance_To(DestinationCoord) < Rule->CloseEnoughDistance && abs(DestinationCoord.Z - LinkedTo->PositionCoord.Z) < 2 * LEVEL_LEPTON_H && Map[LinkedTo->Get_Coord()].Land_Type() != LAND_TUNNEL) {
					Stop_Moving();
					LinkedTo->Assign_Destination(NULL);
					return(MOVE_NO);
				}
				bool bridge;
				if (!Map[HeadToCoord].IsUnderBridge || abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - Map[HeadToCoord].Height) <= 2) {
					bridge = false;
				} else {
					bridge = true;
				}
				Map[HeadToCoord].Incoming(COORD_NONE, true, true, bridge);
				HeadToCoord = COORD_NONE;
				Stop_Driver();
				return(ok);
			}
			break;

		case MOVE_OK:
			LinkedTo->Per_Cell_Process(PCP_END);
			if (LinkedTo == NULL || !LinkedTo->IsActive || LinkedTo->IsInLimbo || LinkedTo->IsFalling) return(MOVE_NO);
			if (LinkedTo->IsLocked) {
				LinkedTo->Set_Occupy_Bit(HeadToCoord);
			}
			LinkedTo->Advance_Path(1);
			LinkedTo->LastPathingCell = HeadToCoord.As_Cell();
			return(ok);

		case MOVE_FRIENDLY_DESTROYABLE:
		case MOVE_DESTROYABLE:
			HeadToCoord = COORD_NONE;
			if (first_pass) {
				Stop_Driver();
				LinkedTo->PathDelay = 0;
				LinkedTo->Path[0] = FACING_NONE;
				Start_Of_Move(0);
				ok = While_Moving(false);
				return(ok);
			}
			if (Map[c].Cell_Object() != NULL) {
				if (!LinkedTo->House->Is_Ally(Map[c].Cell_Object())) {
					LinkedTo->Override_Mission(MISSION_ATTACK, Map[c].Cell_Object(), NULL);
				}
			} else {
				if (Map[c].Overlay != OVERLAY_NONE && OverlayTypes[Map[c].Overlay]->IsWall) {
					LinkedTo->Override_Mission(MISSION_ATTACK, &Map[c], NULL);
				}
			}
			return(ok);

		case MOVE_MOVING_BLOCK:
			HeadToCoord = COORD_NONE;
			Stop_Driver();
			if (!LinkedTo->IsToPathAroundBlockage) {
				LinkedTo->IsToPathAroundBlockage = true;
				LinkedTo->BlockagePathDelay = Rule->BlockagePathDelay;
			}
			if (LinkedTo->PathDelay == 0) {
				LinkedTo->Path[0] = FACING_NONE;
			}
			bool blocked = LinkedTo->IsToPathAroundBlockage && LinkedTo->BlockagePathDelay == 0;
			Start_Of_Move((blocked != 0) + 1);
			return(ok);
		}
		return(ok);
	}

	Start_Of_Move(0);
	return(MOVE_OK);
}


/// <summary>
/// Brings the hovering object to a complete stop.
/// The destination is thrown away and the drive is cut, so the object settles where it
/// stands rather than coasting on to finish the move.
/// </summary>
void HoverLocomotionClass::Stop(void)
{
	LinkedTo->Assign_Destination(NULL);
	Acceleration = 0;
	Height = 0;
	LinkedTo->Set_Speed(0);
}


/***********************************************************************************************
 * DriveClass::Stop_Driver -- Handles removing occupation bits when driving stops.             *
 *                                                                                             *
 *    This routine will remove the "reservation" flag (if present) when the vehicle is         *
 *    required to stop movement.                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the vehicle stopped?                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/22/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void HoverLocomotionClass::Stop_Driver(void)
{
	Acceleration = 0;
	Height = 0;
	LinkedTo->Set_Speed(0);
	if (HeadToCoord != COORD_NONE) {
		LinkedTo->Clear_Occupy_Bit(HeadToCoord);
		HeadToCoord = COORD_NONE;
	}
}


/// <summary>
/// Handles the steering and the throttle for a hovering object.
/// This routine is called from the locomotor's main process. It swings the drive around
/// toward the next stop, eases the throttle up to cruising speed or back off as the
/// destination nears, and points the body toward the cell after next so that the object
/// leans into its turns.
/// </summary>
void HoverLocomotionClass::Motion_AI(void)
{
	if (Is_Moving() && HeadToCoord != COORD_NONE) {
		DirType newdir = DirType().Direction(LinkedTo->Center_Coord(), HeadToCoord);
		if (!WasPushed) {
			Facing.Set_Desired(newdir);
		} else {
			Facing.Set(newdir);
		}

		if (Is_Powered() && Facing.Current().Is_Complete_Turn(newdir, DIR_STEP_8 << 8) || WasPushed) {
			if ((DestinationCoord == COORD_NONE && LinkedTo->Center_Coord().Distance_To(HeadToCoord) < CELL_LEPTON) ||
				 DestinationCoord != COORD_NONE && LinkedTo->Center_Coord().Distance_To(DestinationCoord) < CELL_LEPTON) {

				Height = 0.5;
			} else {
				Height = 1;
				if (WasPushed) {
					Acceleration = 1;
				}
			}
		} else {
			Height = 0;
		}

		if (Height > 0) {
			Boost = 1;
			if (LinkedTo->Path[0] != FACING_NONE && LinkedTo->Path[0] == LinkedTo->Path[1]) {
				Boost = Rule->HoverBoost;
			}
		}

		double accel = std::min(1.0, Boost * Height);

		if (accel > Acceleration) {
			Acceleration += 1.0 / (Rule->HoverAcceleration * TICKS_PER_MINUTE);
			Acceleration = std::min(Acceleration, accel);
		}

		if (accel < Acceleration) {
			Acceleration -= 1.0 / (Rule->HoverBrake * TICKS_PER_MINUTE);
			Acceleration = std::max(Acceleration, 0.0);
		}

		if (Height > 0 && !WasPushed) {
			Coord coord = HeadToCoord;
			if (LinkedTo->Path[0] != FACING_NONE && LinkedTo->Path[0] != FACING_COUNT) {
				coord = Adjacent_Cell(coord, LinkedTo->Path[0]);
			}
			DirType dir = Direction(LinkedTo->Center_Coord(), coord);
			LinkedTo->PrimaryFacing.Set_Desired(dir);
		}
		return;
	}

	if (Is_Moving() && HeadToCoord == COORD_NONE) {
		LinkedTo->Set_Speed(1);
		Start_Of_Move(0);
		While_Moving(true);
	}
}


/// <summary>
/// Cancels the current move order.
/// The object will still coast into the spot it has already reserved, but it will not
/// carry on toward its former destination once it arrives.
/// </summary>
void STDMETHODCALLTYPE HoverLocomotionClass::Stop_Moving(void)
{
	if (DestinationCoord != HeadToCoord) {
		DestinationCoord = COORD_NONE;
		LinkedTo->Path[0] = FACING_NONE;
	}
}


/// <summary>
/// Turns the hovering object to face the direction specified.
/// The turn is a request rather than a snap. The object will swing around toward the
/// direction given over the following game frames.
/// </summary>
/// <param name="coord">The direction the object should come to face.</param>
void HoverLocomotionClass::Do_Turn(DirType coord)
{
	assert(LinkedTo->IsActive);

	DirType dir = coord;

	LinkedTo->PrimaryFacing.Set_Desired(dir);
}


/// <summary>
/// Is the object under a move order?
/// </summary>
/// <returns>bool; Does the object have somewhere it is trying to get to?</returns>
bool HoverLocomotionClass::Is_Moving1(void)
{
	if (Is_Moving()) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Trims the movement path short when the objective is already close enough.
/// A path plotted toward another unit, or toward something that has since come within
/// weapon range, is cut back so that the object stops instead of chasing pointlessly.
/// </summary>
/// <returns>bool; Was the path shortened?</returns>
bool HoverLocomotionClass::Reduce_Path_Length(void)
{
	if (Is_Moving()) {

		/*
		**	Reduce the path length if the target is a unit and the
		**	range to the unit is less than the precalculated path steps.
		*/
		if (LinkedTo->Path[0] != FACING_NONE) {
			int dist;

			if (dynamic_cast<UnitClass *>(LinkedTo->NavCom) || dynamic_cast<InfantryClass *>(LinkedTo->NavCom)) {
				dist = Lepton_To_Cell((LEPTON)LinkedTo->Distance(DestinationCoord));

				if (dist < ARRAY_SIZE(LinkedTo->Path)) {
					LinkedTo->Path[dist] = FACING_NONE;
					return(true);
				}
			}

			if (LinkedTo->TarCom != NULL && LinkedTo->In_Range(LinkedTo->TarCom)) {
				LinkedTo->Path[0] = FACING_NONE;
				return(true);
			}
		}
	}
	return(false);
}


/***********************************************************************************************
 * DriveClass::Start_Of_Move -- Tries to get a unit to advance toward cell.                    *
 *                                                                                             *
 *    This will try to start a unit advancing toward the cell it is                            *
 *    facing. It will check for and handle legality and reserving of the                       *
 *    necessary cell.                                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  true/false; Should this routine be called again because                            *
 *                      initial start operation is temporarily delayed?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/02/1992 JLB : Created.                                                                 *
 *   10/18/1993 JLB : This should be called repeatedly until HeadTo is not NULL.               *
 *   03/16/1994 JLB : Revamped for track logic.                                                *
 *   04/15/1994 JLB : Converted to member function.                                            *
 *   06/19/1995 JLB : Fixed so that it won't fire on ground unnecessarily.                     *
 *   07/13/1995 JLB : Handles bumping into cloaked objects.                                    *
 *   09/22/1995 JLB : Breaks out of hopeless hunt mode.                                        *
 *   07/10/1996 JLB : Sets scan limit if necessary.                                            *
 *=============================================================================================*/
void HoverLocomotionClass::Start_Of_Move(int num)
{
	assert(LinkedTo->IsActive);

	FacingType		facing;				// Direction movement will commence.
	Cell			destcell;			// Cell of destination.
	Coord			dest;				// Destination coordinate.

	Reduce_Path_Length();

	facing = LinkedTo->Path[0];

	if (DestinationCoord != COORD_NONE) {
		if (LinkedTo->PositionCell == DestinationCoord.As_Cell() && abs(LinkedTo->Destination_Coord().Z - DestinationCoord.Z) < 2 * LEVEL_LEPTON_H) {
			Stop_Moving();
			LinkedTo->Assign_Destination(NULL);
			return;
		}
	}

	/*
	**	If the path is invalid at this point, then generate one. If
	**	generating a new path fails, then abort NavCom.
	*/
	if (facing == FACING_NONE) {
		if (DestinationCoord == COORD_NONE) {
			return;
		}

		if (LinkedTo->PositionCell == DestinationCoord.As_Cell() && abs(LinkedTo->Destination_Coord().Z - DestinationCoord.Z) <= 2 * LEVEL_LEPTON_H) {
			return;
		}

		/*
		**	If after a path search, there is still no valid path, then set the
		**	NavCom to null and let the script take care of assigning a new
		**	navigation target.
		*/
		if (LinkedTo->PathDelay != 0) {
			return;
		}

		if (!LinkedTo->Basic_Path(DestinationCoord.As_Cell(), 0, num)) {
			LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

			if (!LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
				LinkedTo->Assign_Destination(NULL);
				return;
			}

			/*
			**	If the unit is close enough to the target then just stop
			**	driving now. This prevents the fidgeting that would occur
			**	if they mindlessly kept trying to get to the exact location
			**	desired. This is quite necessary since it is typical to move
			**	several units with the same mouse click.
			*/
			if (!LinkedTo->Is_On_Priority_Mission() && LinkedTo->Distance(DestinationCoord) < Rule->CloseEnoughDistance && (LinkedTo->Mission == MISSION_MOVE || LinkedTo->Mission == MISSION_GUARD_AREA)) {
				LinkedTo->Assign_Destination(NULL);
				if (!LinkedTo->IsActive) return;
			} else {
				if (LinkedTo->TryTryAgain > 0) {
					LinkedTo->TryTryAgain--;
				} else {
					LinkedTo->Assign_Destination(NULL);
					if (!LinkedTo->IsActive) return;
					if (LinkedTo->IsNewNavCom) Sound_Effect(Rule->ScoldSound);
					LinkedTo->IsNewNavCom = false;
				}
			}

			/*
			**	Since the path was blocked, check to make sure that it was completely
			**	blocked. If so and it has a valid TarCom and it is out of range of the
			**	TarCom, then give this unit a range limit so that it might not pick
			**	a "can't reach" target again.
			*/
			if (!Is_Moving() && LinkedTo->TarCom != NULL && !LinkedTo->In_Range(LinkedTo->TarCom)) {
				LinkedTo->IsScanLimited = true;
				if (LinkedTo->Team != NULL) LinkedTo->Team->Scan_Limit();
				LinkedTo->Assign_Target(NULL);
			}

			/*
			**	Stop the movement, for now, and let the subsequent logic in later game
			**	frames resume movement as appropriate.
			*/
			if (HeadToCoord != COORD_NONE) {
				LinkedTo->Clear_Occupy_Bit(HeadToCoord);
				HeadToCoord = COORD_NONE;
			}
			Stop_Moving();
			return;
		}

		LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

		/*
		**	If a basic path could be found, but the immediate move destination is
		**	blocked by a friendly temporary blockage, then cause that blockage
		**	to scatter.
		*/
		Cell cell = Adjacent_Cell(LinkedTo->Center_Coord().As_Cell(), LinkedTo->Path[0]);
		if (Map.In_Local_Radar(cell, true)) {
			MoveType ok = LinkedTo->Can_Enter_Cell(&Map[cell], LinkedTo->Path[0], LinkedTo->Get_Cell_Height());
			if (ok == MOVE_TEMP) {
				CellClass * cellptr = &Map[cell];
				ObjectClass * blockage = cellptr->Cell_Techno(Point2D(0, 0), LinkedTo->PositionCoord.Z > Map.Get_Height_GL(cell) + 3 * LEVEL_LEPTON_H);
				if (blockage && LinkedTo->House->Is_Ally(blockage)) {

					/*
					**	If the target can be told to get out of the way, only bother
					**	to do so if we aren't very close to the target and this
					**	object can just say "good enough" and stop here.
					*/
					if (LinkedTo->Center_Coord().Distance_To(DestinationCoord) < Rule->CloseEnoughDistance && !LinkedTo->In_Radio_Contact() &&
						abs(DestinationCoord.Z - LinkedTo->PositionCoord.Z) < 2 * LEVEL_LEPTON_H && Map[LinkedTo->Get_Coord()].Land_Type() != LAND_TUNNEL) {

						Stop_Moving();
						LinkedTo->Assign_Destination(NULL);
						return;
					} else {
						bool bridge = (cellptr->IsUnderBridge && abs(LinkedTo->PositionCoord.Z / LEVEL_LEPTON_H - cellptr->Height) > 2);
						cellptr->Incoming(COORD_NONE, true, true, bridge);
					}
				}
			}
		}

		LinkedTo->TryTryAgain = FootClass::PATH_RETRY;
		facing = LinkedTo->Path[0];
	}
}


/// <summary>
/// Cuts the power to the hover drive.
/// This routine is called when an ion storm or similar mishap disables the object. Any
/// move order it was following is abandoned and it is given a shove, so that it drifts
/// and slews as it sinks rather than dropping neatly in place.
/// </summary>
/// <returns>bool; Was the power turned off?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Power_Off(void)
{
	if (Is_Powered() && LinkedTo->CurrentMission != MISSION_SLEEP) {
		Do_Shove();
	}
	if (Is_Moving()) {
		Stop_Moving();
	}

	return(BASECLASS::Power_Off());
}


/// <summary>
/// Is the hover drive still running?
/// An object whose power has been cut keeps a little lift until it has settled all the
/// way onto the ground, so it is treated as powered for as long as it has height to lose.
/// </summary>
/// <returns>bool; Is the object still under power?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Is_Powered(void)
{
	if (!BASECLASS::Is_Powered() && LinkedTo->HeightAGL <= 0) {
		return(false);
	}
	return(true);
}


/// <summary>
/// Is this object vulnerable to an ion storm?
/// A hover unit that is docked with a weapons factory, or is still sitting on the cells
/// it was built onto, is exempt. Without that mercy a storm would strand freshly built
/// units across the factory doorway.
/// </summary>
/// <returns>bool; Should an ion storm cut this object's power?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Is_Ion_Sensitive(void)
{
	BuildingClass *bptr;
	if (LinkedTo->In_Radio_Contact()) {
		BuildingClass * ptr = (BuildingClass *)LinkedTo->Contact_With_Whom();

		if (ptr->RTTI == RTTI_BUILDING) {
			ptr = (BuildingClass *)LinkedTo->Contact_With_Whom();
			if (ptr->Class->IsWeaponsFactory) {
				return(false);
			}
		}
	}

	bptr = Map[(Coord const &)LinkedTo->PositionCoord].Cell_Building();

	if (bptr != NULL && bptr->Class->IsWeaponsFactory) {

		Cell diff = LinkedTo->PositionCoord.As_Cell() - bptr->PositionCoord.As_Cell();

		if (diff.X == 0 && diff.Y == 1) {
			return(false);
		} else
		if (diff.X == 2 && diff.Y == 1) {
			return(false);
		} else
		if (diff.X == 3 && diff.Y == 1) {
			return(false);
		}

	}

	return(true);
}


/// <summary>
/// Pushes the hovering object one cell in the direction specified.
/// This routine is used to get an object out of the way when something else needs the
/// spot it is loitering in. The push is refused if the object cannot enter the adjacent
/// cell, or if it has already been pushed and has not yet come to rest.
/// </summary>
/// <param name="dir">The direction to push the object toward.</param>
/// <returns>bool; Was the object pushed?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Push(DirType dir)
{
	if (Is_Powered() && !WasPushed) {

		FacingType face = dir.As_Facing();
		Cell cell = Adjacent_Cell(LinkedTo->PositionCell, face);

		FootClass * link = LinkedTo;

		if (link->Can_Enter_Cell(&Map[cell]) == MOVE_OK) {
			WasPushed = true;

			if (Is_Moving()) {

				LinkedTo->Path[0] = FACING_NONE;
				if (HeadToCoord != COORD_NONE) {
					LinkedTo->Clear_Occupy_Bit(HeadToCoord);
				}

				HeadToCoord = cell;
				HeadToCoord.Z = Map.Get_Height_GL(HeadToCoord);

				if (LinkedTo->PositionCoord.Z >= HeadToCoord.Z + 2 * LEVEL_LEPTON_H + LEVEL_LEPTON_H) {
					HeadToCoord.Z += BRIDGE_LEPTON_HEIGHT;
				}

				LinkedTo->Set_Occupy_Bit(HeadToCoord);

			} else {
				Move_To((Coord)cell);
			}
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Shoves the hovering object aside in the direction specified.
/// This is a push with attitude -- the object is displaced exactly as a push would do it,
/// but it slews about while it goes.
/// </summary>
/// <param name="dir">The direction to shove the object toward.</param>
/// <returns>bool; Was the object shoved?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Shove(DirType dir)
{
	if (Push(dir)) {
		Do_Shove();
		return(true);
	}
	return(false);
}


/// <summary>
/// Sets the hovering object slewing about from a shove.
/// This routine gives a jostled object the drunken wobble that hover units are known for.
/// Which way it swings is left to chance.
/// </summary>
void HoverLocomotionClass::Do_Shove(void)
{
	WasShoved = true;
	ShoveAccum = Random_Pick(20, 30);
	if (Percent_Chance(50)) {
		ShoveAccum = -ShoveAccum;
	}
}


/// <summary>
/// Fetches the class identifier of this locomotor.
/// The persistence system uses this identifier to create a locomotor of the right kind
/// when the object it drives is loaded back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE HoverLocomotionClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_HoverLocomotion;
	return(S_OK);
}


/// <summary>
/// Lists the members this hover locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void HoverLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(DestinationCoord);
	stream.Serialize(HeadToCoord);
	stream.Serialize(Facing);
	stream.Serialize(Height);
	stream.Serialize(Acceleration);
	stream.Serialize(Boost);
	stream.Serialize(Bounciness);
	stream.Serialize(WasShoved);
	stream.Serialize(ShoveAccum);
	stream.Serialize(WasPushed);
}


/// <summary>
/// Fetches the display layer that the object should be rendered in.
/// A hovering object skims the ground rather than flying over it, so it shares the ground
/// layer with ordinary vehicles.
/// </summary>
/// <returns>Returns with the layer this object belongs in.</returns>
LayerType STDMETHODCALLTYPE HoverLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_GROUND);
}


/// <summary>
/// Starts the hovering object moving toward its destination.
/// This routine is used to spin the drive back up when the object has been sitting still,
/// either at the beginning of a move order or after it has been stalled by a blockage.
/// </summary>
void HoverLocomotionClass::Start(void)
{
	bool anew = HeadToCoord == COORD_NONE || LinkedTo->PositionCell == HeadToCoord.As_Cell();

	Height = 1.0;
	LinkedTo->Set_Speed(1.0);
	Start_Of_Move(0);
	if (anew) {
		While_Moving(true);
	}
}


/// <summary>
/// Sets or clears the occupation bits for the hovering object.
/// This routine handles the "reservation" of the spot the object is headed for, so that
/// other objects treat it as taken while the hover is still on its way there.
/// </summary>
/// <param name="mark">The marking operation to perform; MARK_UP releases the cell,
/// anything else reserves it.</param>
void STDMETHODCALLTYPE HoverLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (mark == MARK_UP) {
		Coord coord = Head_To_Coord();
		LinkedTo->Clear_Occupy_Bit(coord);
	} else {
		Coord coord = Head_To_Coord();
		LinkedTo->Set_Occupy_Bit(coord);
	}
}


/// <summary>
/// Is the object headed for this location?
/// This routine is used to find out whether the hovering object has already reserved the
/// spot in question, so that other objects will not try to settle into it as well.
/// </summary>
/// <param name="to">The coordinate to compare the current destination against.</param>
/// <returns>bool; Is the object moving to this location?</returns>
boolean STDMETHODCALLTYPE HoverLocomotionClass::Is_Moving_Here(Coord to)
{
	Coord coord = Head_To_Coord();

	if (coord != COORD_NONE) {
		if (coord.As_Cell() == to.As_Cell() && abs(coord.Z - to.Z) <= LEVEL_LEPTON_H) {
			return(true);
		}
	}
	return(false);
}
