/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "tunnel.h"

#include "_map.h"
#include "_rules.h"
#include "_tactica.h"
#include "anim.h"
#include "cell.h"
#include "combat.h"
#include "coord.h"
#include "face.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "inline.h"
#include "map.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "tactical.h"

#include "layer.hh"
#include "visual.hh"


/// <summary>
/// Constructor for the tunnel locomotor.
/// This locomotor is attached to units that burrow beneath the map rather than driving
/// across it. It begins idle and above ground, and stays that way until a move order
/// sets it digging.
/// </summary>
TunnelLocomotionClass::TunnelLocomotionClass(void) :
	BASECLASS(),
	State(STATE_IDLE),
	DestinationCoord(COORD_NONE),
	IsUnderground(false)
{
}


/// <summary>
/// Reports whether the unit is anywhere in the dig cycle (State != STATE_IDLE).
/// </summary>
/// <returns>True while dig-moving.</returns>
bool TunnelLocomotionClass::Is_Moving(void)
{
	if (State != STATE_IDLE) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Reports whether the unit is actively dig-moving -- in the cycle and past the initial turn
/// (not STATE_IDLE and not STATE_TURNING).
/// </summary>
/// <returns>True while actively moving.</returns>
bool TunnelLocomotionClass::Is_Moving_Now(void)
{
	if (Is_Moving() && State != STATE_TURNING) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Returns the burrow destination while moving, or the current position when idle.
/// </summary>
/// <returns>The destination coordinate.</returns>
Coord TunnelLocomotionClass::Destination(void)
{
	if (Is_Moving()) {
		return(DestinationCoord);
	}
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Orders the unit to dig its way to the location specified.
/// This routine starts the dig cycle, or retargets one that is already under way -- a unit
/// still below ground merely resumes traveling toward the new destination. A stunned unit
/// ignores the order altogether.
/// </summary>
/// <param name="to">The location to travel to.</param>
void TunnelLocomotionClass::Move_To(Coord to)
{
	if (LinkedTo->StunDuration <= 0) {
		Coord coord = to;
		DestinationCoord = LinkedTo->Class_Of()->Coord_Fixup(to);
		if (coord != COORD_NONE && LinkedTo->HeightAGL < 0) {
			if (State != STATE_DESCENDING && State != STATE_ASCENDING && State != STATE_TURNING && State != STATE_EMERGING && State != STATE_DIGGING_IN) {
				LinkedTo->Mark(MARK_UP);
				State = STATE_TUNNELING;
			}
		}
		if (State == STATE_IDLE) {
			State = STATE_TURNING;
		}
	}
}


/// <summary>
/// Cancels the unit's current dig order.
/// What the cancel costs depends on how far the dig has gotten. A unit that has not yet
/// committed merely forgets where it was going, one on its way down turns around, and one
/// already traveling underground must make for the nearest ground it can surface on. If
/// there is no such ground to be had, it stays buried for good.
/// </summary>
void TunnelLocomotionClass::Stop_Moving(void)
{
	switch (State) {
		case STATE_ABORTING:
			DestinationCoord = COORD_NONE;
			break;
		case STATE_EMERGING:
			DestinationCoord = COORD_NONE;
			break;
		case STATE_ASCENDING:
			DestinationCoord = COORD_NONE;
			break;

		case STATE_TURNING:
			DestinationCoord = COORD_NONE;
			break;

		case STATE_TUNNELING: {
			Coord pos = LinkedTo->PositionCoord;
			if (Point2D(pos - Point2D(DestinationCoord)).Length() > CELL_LEPTON_DIAG) {

				Cell cell = LinkedTo->PositionCell;
				Cell nearby = Map.Nearby_Location(cell, SPEED_TRACK, Map.Get_Cell_Zone(cell), MZONE_NORMAL, false, Point2D(1, 1), false, false, true);

				if (nearby == CELL_NONE) {
					nearby = Map.Nearby_Location(cell, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(1, 1), false, false, true);
				}

				if (nearby == CELL_NONE) {
					LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, NULL, true, true);
					DestinationCoord = COORD_NONE;
				} else {
					DestinationCoord = nearby;
				}
			}
			break;
		}

		case STATE_DESCENDING:
			State = STATE_ASCENDING;
			DestinationCoord = COORD_NONE;
			break;

		case STATE_DIGGING_IN:
			State = STATE_ABORTING;
			DestinationCoord = COORD_NONE;
			break;
	}
}


/// <summary>
/// Handles the per frame processing of the dig cycle.
/// This routine walks the unit through turning, digging in, descending, traveling and
/// surfacing again. It also drops an enemy unit from the selection once it has slipped
/// under shroud or fog, and re-submits the unit to the map whenever it crosses between
/// the ground and underground layers.
/// </summary>
/// <returns>bool; Is the unit still working through its dig cycle?</returns>
bool TunnelLocomotionClass::Process(void)
{
	if (Is_Moving()) {
		int agl = LinkedTo->HeightAGL;
		int hgt = LinkedTo->Height;
		int lyr = In_Which_Layer();
		switch (State) {

			case STATE_TURNING:
				if (agl < 0) {
					State = STATE_DESCENDING;
				} else {
					Process_Turning();
				}
				break;

			case STATE_DIGGING_IN:
				if (agl < 0) {
					State = STATE_DESCENDING;
				} else {
					Process_Digging_In();
				}
				break;

			case STATE_DESCENDING:
				if (hgt == -CELL_LEPTON) {
					LinkedTo->Mark(MARK_UP);
					State = STATE_TUNNELING;
				} else {
					Process_Descending();
				}
				break;

			case STATE_TUNNELING:
				Process_Tunneling();
				break;

			case STATE_ASCENDING:
				Process_Ascending();
				break;

			case STATE_EMERGING:
				Process_Emerging();
				break;

			case STATE_ABORTING:
				Process_Aborting();
				break;
		}

		if (LinkedTo->IsSelected) {
			if (!LinkedTo->House->Is_Player_Control()) {
				if (Map.Is_Shrouded(LinkedTo->PositionCoord) || (Scen->Special.IsFogOfWar && Map.Is_Fogged(LinkedTo->PositionCoord))) {
					LinkedTo->Unselect();
				}
			}
		}
		LinkedTo->Remove_Damage_Particle();
		if (In_Which_Layer() != lyr) {
			Map.Submit(LinkedTo);
		}
		return(true);
	}

	return(false);
}


/// <summary>
/// Determines how the unit is to be drawn.
/// A unit traveling underground is either not drawn at all or betrayed by a ripple in the
/// ground above it, according to what the caller asks for. At every other point in the
/// dig cycle it is drawn as usual.
/// </summary>
/// <param name="flag">Should the buried unit be hidden outright rather than rippled?</param>
/// <returns>Returns with the visual treatment to draw the unit with.</returns>
VisualType TunnelLocomotionClass::Visual_Character(bool flag)
{
	if (State == STATE_TUNNELING) {
		if (flag) {
			return(VISUAL_HIDDEN);
		}
		return(VISUAL_RIPPLE);
	}
	return(VISUAL_NORMAL);
}


/// <summary>
/// STATE_TURNING handler. Rotates in place to face the destination; once aligned it starts the
/// dig-in timer, plays the dig sound/animation, and advances to STATE_DIGGING_IN.
/// </summary>
void TunnelLocomotionClass::Process_Turning(void)
{
	if (!LinkedTo->PrimaryFacing.Is_Rotating()) {
		DirType dir = DirType().Direction(LinkedTo->PositionCoord, DestinationCoord);
		if (dir != LinkedTo->PrimaryFacing.Current()) {
			Do_Turn(dir);
		} else {
			DigTimer = ((64.0 / LinkedTo->TClass->ROT) / Rule->TunnelSpeed);
			State = STATE_DIGGING_IN;
			Sound_Effect(Rule->DigSound, LinkedTo->PositionCoord);
			new AnimClass(Rule->Dig, LinkedTo->PositionCoord);
			IsUnderground = false;
		}
	}
}


/// <summary>
/// STATE_DIGGING_IN handler. Waits out the nose-down dig-in rotation (DigTimer), then sets travel
/// speed, replays the dig effect, detaches passengers, and advances to STATE_DESCENDING.
/// </summary>
void TunnelLocomotionClass::Process_Digging_In(void)
{
	if (DigTimer.Progress() >= 1.0) {
		LinkedTo->Set_Speed(1.0);
		State = STATE_DESCENDING;
		Sound_Effect(Rule->DigSound, LinkedTo->PositionCoord);
		new AnimClass(Rule->Dig, LinkedTo->PositionCoord);
		LinkedTo->Detach_All(false);
	}
}


/// <summary>
/// STATE_ABORTING handler. Reached when a move is cancelled mid dig-in: waits out the levelling
/// rotation, then clears the NavCom and returns to STATE_IDLE.
/// </summary>
void TunnelLocomotionClass::Process_Aborting(void)
{
	if (DigTimer.Progress() >= 1.0) {
		State = STATE_IDLE;
		LinkedTo->NavCom = NULL;
	}
}


/// <summary>
/// STATE_DESCENDING handler. Sinks the unit straight down by the tunnel speed until it reaches full
/// burrow depth, then lifts its surface footprint and advances to STATE_TUNNELING.
/// </summary>
void TunnelLocomotionClass::Process_Descending(void)
{
	Coord coord = LinkedTo->PositionCoord;

	int mheight = -CELL_LEPTON;

	if (coord.Z > mheight) {

		int speed = int(LinkedTo->Current_Speed() * Rule->TunnelSpeed);
		if (speed <= 5) {
			speed = 5;
		}
		coord.Z -= speed;
		if (coord.Z < mheight) {
			coord.Z = mheight;
		}
		LinkedTo->PositionCoord = coord;

	} else {
		LinkedTo->Mark(MARK_UP);
		State = STATE_TUNNELING;
	}
}


/// <summary>
/// Underground travel state. The unit is below ground burrowing toward the
/// destination. When it arrives under the destination it picks a nearby valid
/// surface cell and begins surfacing (or self-destructs if none is reachable);
/// otherwise it advances its position toward the destination.
/// </summary>
void TunnelLocomotionClass::Process_Tunneling(void)
{
	Coord coord = LinkedTo->PositionCoord;

	if (Point2D(DestinationCoord).Distance_To(coord) < 20) {

		coord = DestinationCoord;
		coord.Z = -CELL_LEPTON;

		if (LinkedTo->Can_Enter_Cell(&Map[(Coord const &)(LinkedTo->PositionCoord)]) && Map.In_Local_Radar(LinkedTo->PositionCoord)) {

			Cell cell = LinkedTo->PositionCell;
			cell = Map.Nearby_Location(cell, SPEED_TRACK, Map.Get_Cell_Zone(cell), MZONE_NORMAL, false, Point2D(1, 1), false, false, true);

			if (cell == CELL_NONE) {
				cell = Map.Nearby_Location(LinkedTo->PositionCell, SPEED_TRACK, -1, MZONE_SUBTERANNEAN, false, Point2D(1, 1), false, false, true);
			}

			if (cell == CELL_NONE) {
				LinkedTo->Take_Damage(LinkedTo->Strength, 0, Rule->C4Warhead, NULL, true, true);
				return;
			}

			Move_To(Coord(cell, 0));

		} else {
			if (LinkedTo->HeightAGL > -50) {
				Sound_Effect(Rule->DigSound, LinkedTo->PositionCoord);
			}
			State = STATE_ASCENDING;
			LinkedTo->Mark(MARK_DOWN);
			IsUnderground = false;
		}

	} else {
		coord = Move_Coord(coord, DirType().Direction(coord, DestinationCoord), 19);
	}

	LinkedTo->PositionCoord = coord;
}


/// <summary>
/// STATE_ASCENDING handler. Rises straight up by the tunnel speed until reaching ground level
/// (replaying the dig effect near the surface), then runs Look(), starts the levelling timer,
/// halts speed, and advances to STATE_EMERGING.
/// </summary>
void TunnelLocomotionClass::Process_Ascending(void)
{
	Coord coord = LinkedTo->PositionCoord;

	int mheight = Map.Get_Height_GL(coord);

	if (coord.Z < mheight) {
		int height = LinkedTo->HeightAGL;

		int speed = int(LinkedTo->Current_Speed() * Rule->TunnelSpeed);
		if (speed <= 5) {
			speed = 5;
		}
		coord.Z += speed;
		if (coord.Z > mheight) {
			coord.Z = mheight;
		}
		LinkedTo->PositionCoord = coord;

		if (height <= -50 && LinkedTo->HeightAGL > -50) {
			Sound_Effect(Rule->DigSound, LinkedTo->PositionCoord);
			new AnimClass(Rule->Dig, LinkedTo->PositionCoord);
		}
	} else {
		LinkedTo->Look();
		DigTimer = int(64.0 / LinkedTo->TClass->ROT);
		LinkedTo->Set_Speed(0);
		State = STATE_EMERGING;
	}
}


/// <summary>
/// STATE_EMERGING handler. Waits out the surfacing rotation (DigTimer), runs end-of-move per-cell
/// processing, then stops moving and returns to STATE_IDLE.
/// </summary>
void TunnelLocomotionClass::Process_Emerging(void)
{
	if (DigTimer.Progress() >= 1.0) {
		LinkedTo->Per_Cell_Process(PCP_END);
		if (LinkedTo != NULL && LinkedTo->IsActive && !LinkedTo->IsInLimbo && !LinkedTo->IsFalling) {
			Stop_Moving();
			State = STATE_IDLE;
			LinkedTo->NavCom = NULL;
		}
	}
}


/// <summary>
/// Fetches the transform to draw the unit with.
/// A unit resting on the surface is simply tilted to sit on the ramp beneath it. Once the
/// dig begins it is pitched over onto its nose instead, swinging back upright again as it
/// surfaces.
/// </summary>
/// <param name="key">The shape cache key to fold this pose into. May be NULL.</param>
/// <returns>Returns with the matrix to draw the unit with.</returns>
Matrix3D TunnelLocomotionClass::Draw_Matrix(int * key)
{
	if (State == STATE_IDLE) {
		int ramp = Map[(Coord const &)(LinkedTo->PositionCoord)].Ramp;
		Matrix3D mtx;
		if (key != NULL && *key != -1) {
			*key = ramp + (*key << 6);
		}
		mtx.Make_Identity();
		if (ramp != 0) {
			mtx = Get_Slope_Matrix(ramp) * BASECLASS::Draw_Matrix(key);
		} else {
			mtx = BASECLASS::Draw_Matrix(key);
		}
		return(mtx);
	}
	if (key != NULL) {
		*key = -1;
	}
	double theta;

	switch (State) {

		case STATE_DESCENDING:
			theta = DEG_TO_RAD(90);
			break;

		case STATE_ASCENDING:
			theta = DEG_TO_RAD(-90);
			break;

		case STATE_DIGGING_IN:
			if (key != NULL) {
				*key = -1;
			}
			theta = DigTimer.Progress() * DEG_TO_RAD(90);
			break;

		case STATE_EMERGING:
			if (key != NULL) {
				*key = -1;
			}
			theta = (1.0 - DigTimer.Progress()) * -DEG_TO_RAD(90);
			break;

		case STATE_ABORTING:
			if (key != NULL) {
				*key = -1;
			}
			theta = (1.0 - DigTimer.Progress()) * DEG_TO_RAD(90);
			break;

		default:
			theta = 0.0;
			break;
	}
	Matrix3D mtx = BASECLASS::Draw_Matrix(key);
	if (key != NULL && *key != -1) {
		*key = *key << 8;
		*key |= (int(theta * 32.0) & 64-1);
	}
	mtx.Rotate_Y(theta);
	return(mtx);
}


/// <summary>
/// Returns the vertical pixel offset that sinks or raises the sprite for the current dig phase,
/// derived from the terrain-height delta and the rotation progress.
/// </summary>
/// <returns>The Z pixel adjustment.</returns>
int TunnelLocomotionClass::Z_Adjust(void)
{
	static int tunnel_Z_Adjust[] = {45, 45};

	Coord coord = LinkedTo->PositionCoord;
	Point2D point1;
	TacticalMap->Coord_To_Pixel(coord, point1);

	coord.Z = Map.Get_Height_GL(coord);

	Point2D point2;
	TacticalMap->Coord_To_Pixel(coord, point2);

	int zadjust = point1.Y - point2.Y;

	double progress = DigTimer.Progress();

	Dir256 face = LinkedTo->PrimaryFacing.Current().As_Dir256();

	switch (State) {

		case STATE_TUNNELING:
			return(tunnel_Z_Adjust[1] + zadjust);

		case STATE_EMERGING:
			return(tunnel_Z_Adjust[0] * (1.0 - progress) + zadjust);

		case STATE_DIGGING_IN:
			if (progress > 0.4 && !IsUnderground) {
				IsUnderground = true;
			}
			if (face > Dir256(70) && face < Dir256(140)) {
				zadjust = tunnel_Z_Adjust[1] + zadjust;
			} else {
				zadjust = tunnel_Z_Adjust[1] * progress + zadjust;
			}
			return(zadjust);

		case STATE_DESCENDING:
			return(tunnel_Z_Adjust[1] + zadjust);

		case STATE_ASCENDING:
			return(tunnel_Z_Adjust[0] + zadjust);
	}

	return(BASECLASS::Z_Adjust());
}


/// <summary>
/// Fetches the depth gradient to draw the unit with.
/// A unit that has pitched over onto its nose for the dig is standing on end, so it must
/// be shaded down its length rather than across the flat, as the base locomotor would.
/// </summary>
/// <returns>Returns with the Z gradient to draw the unit with.</returns>
ZGradientType TunnelLocomotionClass::Z_Gradient(void)
{
	if (State == STATE_DESCENDING || State == STATE_DIGGING_IN || State == STATE_ABORTING || State == STATE_EMERGING || State == STATE_ASCENDING) {
		return(ZGRAD_90DEG);
	}
	return(BASECLASS::Z_Gradient());
}


/// <summary>
/// Reports whether the unit casts a shadow -- true only in the surface-level phases (idle, turning,
/// dig-in, emerging, aborting), false once it is pitched down or underground.
/// </summary>
/// <returns>True if it casts a shadow.</returns>
bool TunnelLocomotionClass::Is_To_Have_Shadow(void)
{
	if (State == STATE_IDLE || State == STATE_TURNING || State == STATE_ABORTING || State == STATE_DIGGING_IN || State == STATE_EMERGING) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Returns whether the unit may burrow into the given cell (MOVE_OK if the cell allows burrowing or
/// in map-debug mode, otherwise MOVE_NO).
/// </summary>
/// <param name="cell">Cell to test.</param>
/// <returns>MOVE_OK or MOVE_NO.</returns>
MoveType TunnelLocomotionClass::Can_Enter_Cell(Cell cell)
{
	if (!Debug_Map && !Map[cell].Can_Burrow_Here()) {
		return(MOVE_NO);
	}
	return(MOVE_OK);
}


/// <summary>
/// Sets the unit's desired facing (used while turning to face the dig destination).
/// </summary>
/// <param name="coord">Desired facing.</param>
void TunnelLocomotionClass::Do_Turn(DirType coord)
{
	DirType dir = coord;
	LinkedTo->PrimaryFacing.Set_Desired(dir);
}


/// <summary>
/// Fetches the class identifier for this locomotor.
/// The save system records the identifier so that the right locomotor can be created
/// again when the game is loaded.
/// </summary>
/// <param name="retval">The location to store the class identifier in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT TunnelLocomotionClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TunnelLocomotion;
	return(S_OK);
}


/// <summary>
/// Lists the members this tunnel locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TunnelLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(State);
	stream.Serialize(DestinationCoord);
	stream.Serialize(DigTimer);
	stream.Serialize(IsUnderground);
}


/// <summary>
/// Returns the render layer: underground while travelling (STATE_TUNNELING), ground otherwise.
/// </summary>
/// <returns>The render layer.</returns>
LayerType TunnelLocomotionClass::In_Which_Layer(void)
{
	if (State != STATE_TUNNELING) {
		return(LAYER_GROUND);
	}
	return(LAYER_UNDERGROUND);
}


/// <summary>
/// Determines if the unit may fire its weapon.
/// This routine denies fire to any unit that is not sitting quietly on the surface. A
/// subterranean unit has no shot while it is lining up, digging, or under the ground.
/// </summary>
/// <returns>Returns with the fire error, or FIRE_OK if the unit is free to shoot.</returns>
FireErrorType TunnelLocomotionClass::Can_Fire(void)
{
	FireErrorType fire = BASECLASS::Can_Fire();

	if (fire == FIRE_OK && State != STATE_IDLE) {
		fire = FIRE_MOVING;
	}

	return(fire);
}


/// <summary>
/// Reports whether the unit is in the act of surfacing (ascending or emerging).
/// </summary>
/// <returns>True while surfacing.</returns>
bool TunnelLocomotionClass::Is_Surfacing(void)
{
	return(State == STATE_ASCENDING || State == STATE_EMERGING);
}
