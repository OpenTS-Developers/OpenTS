/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "levitate.h"

#include "_map.h"
#include "_rules.h"
#include "anim.h"
#include "cell.h"
#include "foot.h"
#include "inline.h"
#include "mouse.h"
#include "particle.h"
#include "partsys.h"
#include "psystype.h"
#include "rules.h"
#include "savestream.h"

#include "layer.hh"


LevitateLocomotionClass::GlobalControlsStruct LevitateLocomotionClass::GlobalControls = {
	0.05,	/// Drag
	4.0,	/// MaxVelocityWhenHappy
	5.0,	/// MaxVelocityWhenFollowing
	6.5,	/// MaxVelocityWhenPissedOff
	0.01,	/// AccelerationProbability
	20,		/// AccelerationDuration
	0.5,	/// Acceleration
	1.5,	/// InitialBoost
	4,		/// MaxBlockCount
	0.15,	/// IntentionalDeacceleration
	0.3,	/// IntentionalDriftVelocity
	1.5,	/// ProximityDistance
};

TypeList<int> LevitateLocomotionClass::PropulsionSoundEffects;
int PropulsionSoundEffectIndex;

char const * LevitateLocomotionClass::INI_NAME = "LEVITATION";


/// <summary>
/// Creates a levitation locomotor.
/// The locomotor comes into being idle and motionless. It is of no use until it has been
/// linked to the object that it is to carry about.
/// </summary>
LevitateLocomotionClass::LevitateLocomotionClass(void) :
	BASECLASS(),
	State(STATE_IDLE),
	Speed(0),
	MoveX(0),
	MoveY(0),
	AccelerationX(0),
	AccelerationY(0),
	AccelerationsRemaining(0),
	BlockTriesRemaining(4),
	MoveRate(0),
	Dampen(0)
{

}


/// <summary>
/// Attaches this locomotor to the object that it will drift about.
/// The levitation drive has nothing of its own to prime, so the base class does all of
/// the work.
/// </summary>
/// <param name="pointer">Pointer to the object this locomotor will drive.</param>
/// <returns>Returns with the result of the attach operation.</returns>
void LevitateLocomotionClass::Link_To_Object(void *pointer)
{
	BASECLASS::Link_To_Object(pointer);
}


/// <summary>
/// Destroys the levitation locomotor.
/// </summary>
LevitateLocomotionClass::~LevitateLocomotionClass(void)
{

}


/// <summary>
/// Per-frame vertical/hover update for the levitation locomotor. Computes a sine-based
/// "bob" offset from Rule->HoverBob, applies gravity and hover damping via the Dampen
/// field, clamps negative heights, and writes the result to the LinkedTo unit's height.
/// Also lowers the target hover height when the next path cell's terrain is higher.
/// </summary>
void LevitateLocomotionClass::Hover_AI(void)
{
	int height = LinkedTo->HeightAGL;
	int target_height = height;

	if (LinkedTo->Path[0] != FACING_NONE) {
		int here_terrain = Map.Get_Height_GL(LinkedTo->PositionCoord);
		if (Map.Get_Height_GL(Adjacent_Cell(LinkedTo->PositionCoord, (FacingType)LinkedTo->Path[0])) > here_terrain) {
			target_height = height - Rule->HoverHeight;
		}
	}

	height = (int)((double)height + Dampen);

	int id = LinkedTo->Fetch_ID();
	double bob_scale;
	if ((id & 1) != 0) {
		bob_scale = 1.0;
	} else {
		bob_scale = 1.1;
	}

	double bob = std::sinf((double)((Frame + 2 * id) % (int)(bob_scale * Rule->HoverBob * TICKS_PER_MINUTE)) * DEG_TO_RAD(360) / (bob_scale * Rule->HoverBob * TICKS_PER_MINUTE));
	int final_height = (int)(bob + bob + (double)height);
	if (final_height < 0) {
		Dampen = 0;
		final_height = 0;
	}

	bool was_down = LinkedTo->IsDown;
	LinkedTo->IsDown = false;
	LinkedTo->Set_Height_AGL(final_height);
	LinkedTo->IsDown = was_down;

	if (target_height < Rule->HoverHeight) {
		if (BASECLASS::Is_Powered()) {
			Dampen = ((double)Rule->HoverHeight + (double)Rule->HoverHeight - (double)target_height) / (double)Rule->HoverHeight * (double)Rule->Gravity + Dampen;
		}
		if (target_height < Rule->HoverHeight / 4) {
			Dampen = (double)(Rule->Gravity / 3) + Dampen;
		}
	}

	Dampen = Dampen - (double)Rule->Gravity;
	Dampen = Dampen * Rule->HoverDampen;
}


/// <summary>
/// Dispatches one frame of the horizontal state machine: calls the Process_* handler
/// matching the current State (STATE_IDLE -> Process_Idle, and so on).
/// </summary>
void LevitateLocomotionClass::State_AI(void)
{
	switch (State) {
		case STATE_IDLE:
			Process_Idle();
			break;
		case STATE_ACCELERATING:
			Process_Accelerating();
			break;
		case STATE_CRUISING:
			Process_Cruising();
			break;
		case STATE_DECELERATING:
			Process_Decelerating();
			break;
		case STATE_DRIFTING:
			Process_Drifting();
			break;
		case STATE_ARRIVED:
			Process_Arrived();
			break;
		case STATE_RECENTERING:
			Process_Recentering();
			break;
		case STATE_DEPARTING:
			Process_Departing();
			break;
	}
}


/// <summary>
/// STATE_IDLE handler. At rest on its own cell: steers toward a valid TarCom/NavCom if
/// one exists, otherwise fires an occasional random idle thrust (unless on a sticky or
/// sleep mission). Releases its cell occupation if it has left the idle state.
/// </summary>
void LevitateLocomotionClass::Process_Idle(void)
{
	if (Validate_TarCom()) {
		Steer_Towards(LinkedTo->TarCom->Center_Coord());
	} else if (Validate_NavCom()) {
		Steer_Towards(LinkedTo->NavCom->Center_Coord());
	} else {
		if (LinkedTo->Mission != MISSION_STICKY) {
			if (LinkedTo->Mission != MISSION_SLEEP) {
				if (Random_Double(0.0, 1.0) < GlobalControls.AccelerationProbability) {
					double angle = DEG_TO_RAD(360) * (Random_Double(0.0, 1.0));
					Accelerate(angle);
				}
			}
		}
	}

	if (State != STATE_IDLE) {
		if (LinkedTo->IsOccupyingCell) {
			LinkedTo->IsOccupyingCell = false;
			LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
		}
	}
}


/// <summary>
/// STATE_ACCELERATING handler. Rides out a powered acceleration burst: decelerates if the
/// target is now within proximity, drops to STATE_CRUISING once the burst is spent.
/// </summary>
void LevitateLocomotionClass::Process_Accelerating(void)
{
	if (Validate_TarCom() && Is_In_Proximity(LinkedTo->TarCom->Center_Coord())) {
		Decelerate();
	} else if (Validate_NavCom() && Is_In_Proximity(LinkedTo->NavCom->Center_Coord())) {
		AccelerationsRemaining = 0;
		State = STATE_DECELERATING;
		MoveRate = GlobalControls.IntentionalDeacceleration;
	} else if (AccelerationsRemaining == 0) {
		MoveRate = GlobalControls.Drag;
		State = STATE_CRUISING;
	}
}


/// <summary>
/// STATE_CRUISING handler. Main coasting/travel state: decelerates on reaching the target or
/// exceeding the mood-velocity cap; comes to rest (STATE_IDLE) when nearly stopped with no
/// target; re-thrusts at random while wandering below MaxVelocityWhenHappy.
/// </summary>
void LevitateLocomotionClass::Process_Cruising(void)
{
	if (Validate_TarCom()) {
		if (Speed < GlobalControls.MaxVelocityWhenPissedOff || Is_In_Proximity(LinkedTo->TarCom->Center_Coord())) {
			Decelerate();
		}
	} else if (Validate_NavCom()) {
		if (Speed < GlobalControls.MaxVelocityWhenFollowing || Is_In_Proximity(LinkedTo->NavCom->Center_Coord())) {
			Decelerate();
		}
	} else {
		if (Speed < 0.01) {
			MoveRate = GlobalControls.Drag;
			State = STATE_IDLE;
			MoveY = 0;
			MoveX = 0;
			if (!LinkedTo->IsOccupyingCell) {
				LinkedTo->IsOccupyingCell = true;
				LinkedTo->Set_Occupy_Bit(LinkedTo->PositionCoord);
			}
		} else {
			if (AccelerationsRemaining == 0) {
				if (Speed < GlobalControls.MaxVelocityWhenHappy) {
					if (Random_Double(0.0, 1.0) < GlobalControls.AccelerationProbability) {
						double angle = DEG_TO_RAD(360) * (Random_Double(0.0, 1.0));
						Accelerate(angle);
					}
				}
			}
		}
	}

}


/// <summary>
/// STATE_DECELERATING handler. Brakes toward a stop; once nearly halted, re-steers toward the
/// TarCom/NavCom, or comes to rest (STATE_IDLE) and re-occupies its cell if there is none.
/// </summary>
void LevitateLocomotionClass::Process_Decelerating(void)
{
	if (Speed < 0.01) {
		if (Validate_TarCom()) {
			Steer_Towards(LinkedTo->TarCom->Center_Coord());
		} else if (Validate_NavCom()) {
			Steer_Towards(LinkedTo->NavCom->Center_Coord());
		} else {
			MoveRate = GlobalControls.Drag;
			State = STATE_IDLE;
			MoveY = 0;
			MoveX = 0;
			if (!LinkedTo->IsOccupyingCell) {
				LinkedTo->IsOccupyingCell = true;
				LinkedTo->Set_Occupy_Bit(LinkedTo->PositionCoord);
			}
		}
	}
}


/// <summary>
/// STATE_DRIFTING handler. While drifting toward a nearby target, keeps re-steering at it;
/// returns to STATE_CRUISING if the target is lost.
/// </summary>
void LevitateLocomotionClass::Process_Drifting(void)
{
	if (Validate_TarCom()) {
		Steer_Towards(LinkedTo->TarCom->Center_Coord());
	} else if (Validate_NavCom()) {
		Steer_Towards(LinkedTo->NavCom->Center_Coord());
	} else {
		MoveRate = GlobalControls.Drag;
		State = STATE_CRUISING;
	}
}


/// <summary>
/// STATE_ARRIVED handler. Reached the target (within half a cell, motion zeroed): re-steers if
/// the target has moved, otherwise comes to rest (STATE_IDLE) and re-occupies its cell.
/// </summary>
void LevitateLocomotionClass::Process_Arrived(void)
{
	if (Validate_TarCom()) {
		Steer_Towards(LinkedTo->TarCom->Center_Coord());
	} else if (Validate_NavCom()) {
		if (Has_Arrived(LinkedTo->NavCom->Center_Coord())) {
			if (!LinkedTo->NavCom->Is_Techno()) {
				LinkedTo->NavCom = NULL;
			}
		} else {
			Steer_Towards(LinkedTo->NavCom->Center_Coord());
		}
	} else {
		MoveRate = GlobalControls.Drag;
		State = STATE_IDLE;
		MoveY = 0;
		MoveX = 0;
		if (!LinkedTo->IsOccupyingCell) {
			LinkedTo->IsOccupyingCell = true;
			LinkedTo->Set_Occupy_Bit(LinkedTo->PositionCoord);
		}
	}
}


/// <summary>
/// Levitator state step that re-pins the unit onto its cell center and seeds an
/// intentional drift toward its TarCom/NavCom. If the unit is essentially on its cell
/// center, it requests a path toward the target and starts an intentional drift
/// (STATE_DEPARTING); otherwise it drifts back toward the cell center (STATE_RECENTERING).
/// </summary>
void LevitateLocomotionClass::Process_Recentering(void)
{
	Coord home = Coord_Snap(LinkedTo->PositionCoord);

	Point2D pt(home.X, home.Y);
	Point2D p(LinkedTo->Center_Coord());
	if (pt.Distance_To(p) < 5) {
		Coord center;
		if (Validate_TarCom()) {
			center = LinkedTo->TarCom->Center_Coord();
		} else if (Validate_NavCom()) {
			center = LinkedTo->NavCom->Center_Coord();
		} else {
			MoveRate = GlobalControls.Drag;
			State = STATE_CRUISING;
			return;
		}

		LinkedTo->Basic_Path(center.As_Cell(), 0, 0);
		Set_Coord(home);

		DirType dir;
		if (LinkedTo->Path[0] == FACING_NONE) {
			dir = Direction(LinkedTo->PositionCoord, center);
		} else {
			dir = DirType(LinkedTo->Path[0]);
		}

		double angle = dir.As_Radian();
		double sine = std::sinf(angle);
		double cosine = std::cosf(angle);

		AccelerationsRemaining = 0;
		MoveRate = 0;
		AccelerationY = 0;
		AccelerationX = 0;

		MoveX = GlobalControls.IntentionalDriftVelocity * cosine;
		MoveY = -(GlobalControls.IntentionalDriftVelocity * sine);
		Speed = GlobalControls.IntentionalDriftVelocity;
		State = STATE_DEPARTING;
		return;
	}

	Drift_Towards(home);
	State = STATE_RECENTERING;
}


/// <summary>
/// STATE_DEPARTING handler. No-op: the unit has been re-centered and seeded with a fresh drift;
/// Move_AI carries it out of the cell and flips it to STATE_CRUISING on the first good move.
/// </summary>
void LevitateLocomotionClass::Process_Departing(void)
{
	//nothing
}


/// <summary>
/// Tests whether the unit's TarCom is a live, active, deployed Techno worth steering at,
/// clearing a stale/invalid target on the way.
/// </summary>
bool LevitateLocomotionClass::Validate_TarCom(void)
{
	if (LinkedTo->TarCom != NULL) {
		if (!LinkedTo->TarCom->Is_Techno()) return(false);
		TechnoClass * techno = dynamic_cast<TechnoClass *>(LinkedTo->TarCom);
		if (techno != NULL && techno->IsActive && techno->IsDown) {
			return(true);
		}
		LinkedTo->Assign_Target(NULL);
	}
	return(false);
}


/// <summary>
/// Tests whether the unit's NavCom is a valid move destination (a non-Techno cell target, or a
/// live active Techno), clearing a stale NavCom on the way.
/// </summary>
bool LevitateLocomotionClass::Validate_NavCom(void)
{
	if (LinkedTo->NavCom != NULL) {
		if (!LinkedTo->NavCom->Is_Techno()) return(true);
		TechnoClass * techno = dynamic_cast<TechnoClass *>(LinkedTo->NavCom);
		if (techno != NULL && techno->IsActive && techno->IsDown) {
			return(true);
		}
		LinkedTo->NavCom = NULL;
	}
	return(false);
}


/// <summary>
/// Applies a powered thrust at the given angle: spawns a gas-puff particle (and occasional engine
/// sound), sets the acceleration vector plus an initial velocity boost, recomputes Speed, and
/// enters STATE_ACCELERATING.
/// </summary>
/// <param name="angle">Thrust direction, in radians.</param>
void LevitateLocomotionClass::Accelerate(double & angle)
{
	if (PropulsionSoundEffects.Count() > 0 && (PropulsionSoundEffectIndex++ & 3) == 0) {
		Sound_Effect((VocType)PropulsionSoundEffects.Pick(NonCriticalRandomNumber));
	}

	AccelerationsRemaining = GlobalControls.AccelerationDuration;

	double sine = std::sinf(angle);
	double cosine = std::cosf(angle);

	AccelerationX = +(GlobalControls.Acceleration * cosine);
	AccelerationY = -(GlobalControls.Acceleration * sine);

	MoveX += GlobalControls.InitialBoost * cosine;
	MoveY -= GlobalControls.InitialBoost * sine;
	Update_Speed();

	ParticleSystemClass *psys = new ParticleSystemClass(ParticleSystemTypes[ParticleSystemTypeClass::From_Name("GasPuffSys")], LinkedTo->PositionCoord);
	ParticleClass *part = psys->Spawn_Held_Particle(LinkedTo->PositionCoord, LinkedTo->PositionCoord);
	part->GasDrift.Z = -12;
	part->GasDrift.X = (int)(cosine * -16.0);
	part->GasDrift.Y = (int)(sine * 16.0);
	MoveRate = GlobalControls.Drag;
	State = STATE_ACCELERATING;
}


/// <summary>
/// Thrusts toward the given coord by computing the direction to it and calling Accelerate().
/// </summary>
/// <param name="coord">World coordinate to accelerate toward.</param>
void LevitateLocomotionClass::Accelerate_Towards(Coord const & coord)
{
	DirType dir;
	dir.Direction(LinkedTo->PositionCoord, coord);
	double angle = dir.As_Radian();
	//double angle = Direction(LinkedTo->PositionCoord, coord).As_Radian(); // this gives wrong stack
	Accelerate(angle);
}


/// <summary>
/// Starts an intentional low-speed drift toward the given coord, clamping each velocity axis so
/// it never overshoots the remaining distance.
/// </summary>
/// <param name="coord">World coordinate to drift toward.</param>
void LevitateLocomotionClass::Drift_Towards(Coord const & coord)
{
	Coord position = LinkedTo->PositionCoord;
	Drift(DirType().Direction(position, coord));
	int dx = coord.X - position.X;
	int dy = coord.Y - position.Y;
	if (abs(MoveX) > abs(dx)) {
		MoveX = dx;
	}
	if (abs(MoveY) > abs(dy)) {
		MoveY = dy;
	}
}


/// <summary>
/// Seeds an intentional drift in the given direction at IntentionalDriftVelocity (clearing any
/// queued acceleration) and enters STATE_DRIFTING.
/// </summary>
/// <param name="dir">Direction to drift in.</param>
void LevitateLocomotionClass::Drift(DirType const & dir)
{
	double angle = dir.As_Radian();
	double sine = std::sinf(angle);
	double cosine = std::cosf(angle);

	AccelerationsRemaining = 0;
	MoveRate = 0;
	AccelerationY = 0;
	AccelerationX = 0;

	MoveX = GlobalControls.IntentionalDriftVelocity * cosine;
	MoveY = -GlobalControls.IntentionalDriftVelocity * sine;
	Speed = GlobalControls.IntentionalDriftVelocity;

	State = STATE_DRIFTING;
}


/// <summary>
/// Begins braking toward a stop (IntentionalDeacceleration) and enters STATE_DECELERATING.
/// </summary>
void LevitateLocomotionClass::Decelerate(void)
{
	AccelerationsRemaining = 0;
	State = STATE_DECELERATING;
	MoveRate = GlobalControls.IntentionalDeacceleration;
}


/// <summary>
/// Tests whether the coord is within ProximityDistance cells of the unit's center.
/// </summary>
/// <param name="coord">World coordinate to test against.</param>
/// <returns>True if within proximity range.</returns>
bool LevitateLocomotionClass::Is_In_Proximity(Coord const & coord)
{
	Point2D pt(coord.X, coord.Y);
	Point2D p(LinkedTo->Center_Coord());
	return(pt.Distance_To(p) < GlobalControls.ProximityDistance * CELL_LEPTON);
}


/// <summary>
/// Tests whether the unit has effectively arrived at coord -- close enough to its cell center
/// to stop. Crossing this is what flips Steer_Towards to STATE_ARRIVED and zeroes all velocity.
/// </summary>
/// <param name="coord">World coordinate to test against.</param>
/// <returns>True once the unit has effectively arrived.</returns>
bool LevitateLocomotionClass::Has_Arrived(Coord const & coord)
{
	Point2D pt(coord.X, coord.Y);
	Point2D p(LinkedTo->Center_Coord());
	return(pt.Distance_To(p) < CELL_LEPTON / 2);
}


/// <summary>
/// Steers toward the target coord: once it has arrived it snaps to STATE_ARRIVED and zeroes all
/// motion; within drift range it drifts toward the coord; otherwise it accelerates toward it.
/// </summary>
/// <param name="coord">World coordinate to steer toward.</param>
void LevitateLocomotionClass::Steer_Towards(Coord const & coord)
{
	if (Has_Arrived(coord)) {
		State = STATE_ARRIVED;
		AccelerationsRemaining = 0;
		AccelerationY = 0;
		AccelerationX = 0;
		Speed = 0;
		MoveY = 0;
		MoveX = 0;
	} else {
		if (Is_In_Proximity(coord)) {
			Drift_Towards(coord);
		} else {
			Accelerate_Towards(coord);
		}
	}
}


/// <summary>
/// Per-frame horizontal movement driver of the levitation locomotor (called from Process
/// right after State_AI). Applies drag to Speed/MoveX/MoveY, consumes one queued
/// acceleration, recomputes Speed, and computes the new coord. Commits it to the LinkedTo
/// unit if the destination cell is passable (falls back to an axis-split move if blocked).
/// Handles bridge-entry/exit flagging, blocked-target recovery via BlockTriesRemaining
/// (dropping a dead TarCom/NavCom and re-centering with a new drift direction), and the
/// STATE_DEPARTING -> STATE_CRUISING transition on a successful move.
/// </summary>
void LevitateLocomotionClass::Move_AI(void)
{
	double drag = Speed - MoveRate;
	if (drag <= 0.0) {
		MoveY = 0;
		MoveX = 0;
	} else if (Speed > 0.0) {
		double scale = drag / Speed;
		MoveX *= scale;
		MoveY *= scale;
	}

	if (AccelerationsRemaining > 0) {
		MoveX += AccelerationX;
		MoveY += AccelerationY;
		AccelerationsRemaining--;
	}

	Update_Speed();

	Coord coord = LinkedTo->PositionCoord;
	int move_x = (int)MoveX;
	int move_y = (int)MoveY;
	coord.X += move_x;
	coord.Y += move_y;

	if (Is_Not_On_Cell(coord)) {
		bool move = false;
		if (Can_Move_Here(coord)) {
			move = true;
		} else {
			Coord newcoord = coord;
			if (move_x > 0) {
				coord.X++;
			} else if (move_x < 0) {
				newcoord.X--;
			}
			if (move_y > 0) {
				newcoord.Y++;
			} else if (move_y < 0) {
				newcoord.Y--;
			}
			if (Can_Move_Here(newcoord)) {
				move = true;
				coord = newcoord;
			}
		}

		if (move) {
			bool was_down = LinkedTo->IsDown;
			if (was_down) {
				LinkedTo->Mark(MARK_UP);
			}

			Set_Coord(coord);
			BlockTriesRemaining = GlobalControls.MaxBlockCount;

			Update_Bridge_State(coord);

			if (was_down) {
				LinkedTo->Mark(MARK_DOWN);
			}

			if (State == STATE_DEPARTING) {
				MoveRate = GlobalControls.Drag;
				State = STATE_CRUISING;
			}
			return;
		}

		if (BlockTriesRemaining-- <= 0) {
			BlockTriesRemaining = GlobalControls.MaxBlockCount;
			if (Needs_New_Target()) {
				LinkedTo->NavCom = NULL;
			}
		}

		Drift_Towards(Coord_Snap(LinkedTo->PositionCoord));
		State = STATE_RECENTERING;
		return;
	}

	Set_Coord(coord);
}


/// <summary>
/// Recomputes Speed as the magnitude of the (MoveX, MoveY) velocity.
/// </summary>
/// <returns>The recomputed speed.</returns>
double LevitateLocomotionClass::Update_Speed(void)
{
	Speed = std::sqrt(MoveX * MoveX + MoveY * MoveY);
	return(Speed);
}


/// <summary>
/// Tests whether the coord lies in a different cell than the unit currently occupies.
/// </summary>
/// <param name="coord">World coordinate to test.</param>
/// <returns>True if the coord is on a different cell.</returns>
bool LevitateLocomotionClass::Is_Not_On_Cell(Coord const & coord)
{
	return(coord.As_Cell() != LinkedTo->PositionCoord.As_Cell());
}


/// <summary>
/// Moves the unit to the given coordinate without toggling its display/occupation state
/// (preserves the IsDown flag across the move).
/// </summary>
/// <param name="coord">New world coordinate.</param>
void LevitateLocomotionClass::Set_Coord(Coord const & coord)
{
	bool was_down = LinkedTo->IsDown;
	LinkedTo->IsDown = false;
	LinkedTo->PositionCoord = coord;
	LinkedTo->IsDown = was_down;
}


/// <summary>
/// Tests whether the unit may move into the cell containing the coord: resolves the facing from
/// its current cell and consults Can_Enter_Cell, treating destroyable occupants as passable only
/// when every one of them is a foot unit.
/// </summary>
/// <param name="coord">Destination world coordinate.</param>
/// <returns>True if the move is allowed.</returns>
bool LevitateLocomotionClass::Can_Move_Here(Coord const & coord)
{
	CellClass * here_cellptr = &Map[(Coord const &)LinkedTo->PositionCoord];
	CellClass * there_cellptr = &Map[coord];
	FacingType facing = FACING_NW;
	/// The loop test can never fire -- the step below only ever yields a facing of 0 to 7 --
	/// so a destination cell that is not adjacent spins here forever.
	while (facing != FACING_NONE) {
		if (&here_cellptr->Adjacent_Cell(facing) == there_cellptr) break;
		facing = Facing_Sub(facing, FACING_45);
	}

	switch (LinkedTo->Can_Enter_Cell(&Map[coord], facing, LinkedTo->Get_Cell_Height())) {
		case MOVE_OK:
		case MOVE_MOVING_BLOCK:
			return(true);

		case MOVE_DESTROYABLE:
			{
				bool all_foot = true;
				ObjectClass * occupier;
				CellClass * cellptr = &Map[coord];
				if (abs(coord.Z - LinkedTo->PositionCoord.Z) < 2 * CELL_LEPTON) {
					occupier = cellptr->Cell_Occupier(false);
				} else if (cellptr->IsBridgeDeck) {
					occupier = cellptr->Cell_Occupier(true);
				} else {
					occupier = NULL;
				}
				while (all_foot && occupier != NULL) {
					if (occupier->IsActive && occupier->IsDown) {
						all_foot = occupier->Is_Foot();
					}
					occupier = occupier->Next;
				}
				return(all_foot);
			}

		default:
			return(false);
	}
	return(false);
}


/// <summary>
/// Updates the unit's IsOnBridge flag for the coord's cell: sets it when entering an under-bridge
/// cell at bridge height, clears it when leaving the bridge.
/// </summary>
/// <param name="coord">World coordinate whose cell is examined.</param>
void LevitateLocomotionClass::Update_Bridge_State(Coord const & coord)
{
	CellClass * cellptr = &Map[coord];
	if (!LinkedTo->IsOnBridge && cellptr->IsUnderBridge && LinkedTo->HeightAGL >= BRIDGE_LEPTON_HEIGHT) {
		LinkedTo->IsOnBridge = true;
	}
	if (LinkedTo->IsOnBridge == true && !cellptr->IsUnderBridge) {
		LinkedTo->IsOnBridge = false;
	}
}


/// <summary>
/// Has the unit run out of anything to head for?
/// This routine is used by the movement handler once the unit has spent its retries against
/// an obstruction. With neither a target nor a destination left worth keeping, the move is
/// abandoned rather than ground out against the blockage.
/// </summary>
/// <returns>bool; Does the unit need a fresh objective?</returns>
/// <remarks>This is not a pure query -- a stale target or destination is cleared
/// in passing.</remarks>
bool LevitateLocomotionClass::Needs_New_Target(void)
{
	if (Validate_TarCom()) {
		return(false);
	}
	if (Validate_NavCom()) {
		return(false);
	}
	return(true);
}


/// <summary>
/// ILocomotion per-frame tick. Runs the horizontal state machine (State_AI), integrates horizontal
/// movement (Move_AI), spawns a water wake every tenth frame while moving over water, then updates
/// the vertical hover (Hover_AI).
/// </summary>
/// <returns>True while the unit is still moving.</returns>
bool LevitateLocomotionClass::Process(void)
{
	State_AI();

	Move_AI();
	if (Is_Moving_Now() && (Frame % 10) == 0) {
		if (!LinkedTo->IsOnBridge && LinkedTo->Get_Cell_Ptr()->Land_Type() == LAND_WATER && Rule->Wake != NULL) {
			new AnimClass(Rule->Wake, LinkedTo->PositionCoord);
		}

	}
	Hover_AI();

	return(Is_Moving());
}


/// <summary>
/// Reports whether the locomotor is in any state other than STATE_IDLE.
/// </summary>
/// <returns>True while moving.</returns>
bool LevitateLocomotionClass::Is_Moving(void)
{
	return(State != STATE_IDLE);
}


/// <summary>
/// Reports whether the locomotor is in any state other than STATE_IDLE (identical to Is_Moving).
/// </summary>
/// <returns>True while moving.</returns>
bool LevitateLocomotionClass::Is_Moving_Now(void)
{
	return(State != STATE_IDLE);
}


/// <summary>
/// ILocomotion destination query; the levitator reports none.
/// </summary>
/// <returns>COORD_NONE.</returns>
Coord LevitateLocomotionClass::Destination(void)
{
	return(COORD_NONE);
}


/// <summary>
/// Returns the unit's current position as its immediate heading target.
/// </summary>
/// <returns>The current position.</returns>
Coord LevitateLocomotionClass::Head_To_Coord(void)
{
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Cancels the unit's destination and zeroes its commanded speed.
/// </summary>
void LevitateLocomotionClass::Stop(void)
{
	LinkedTo->Assign_Destination(NULL);
	LinkedTo->Set_Speed(0);
}


ClassID LevitateLocomotionClass::Class_ID(void) const
{
	return(ClassID_LevitateLocomotion);
}


/// <summary>
/// Lists the members this levitation locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void LevitateLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(State);
	stream.Serialize(Speed);
	stream.Serialize(MoveX);
	stream.Serialize(MoveY);
	stream.Serialize(AccelerationX);
	stream.Serialize(AccelerationY);
	stream.Serialize(AccelerationsRemaining);
	stream.Serialize(BlockTriesRemaining);
	stream.Serialize(MoveRate);
	stream.Serialize(Dampen);
	// GlobalControls -- shared by the whole class and read from the rules rather than held per
	// unit.
	// PropulsionSoundEffects
	// INI_NAME
}


/// <summary>
/// Reports the render layer; levitating units draw in the ground layer.
/// </summary>
/// <returns>LAYER_GROUND.</returns>
LayerType LevitateLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_GROUND);
}


/// <summary>
/// Loads the [LEVITATION] global tuning values (drag, velocity caps, acceleration, block count,
/// drift, proximity, and propulsion sounds) from the rules INI into GlobalControls.
/// </summary>
/// <param name="ini">Rules INI database to read from.</param>
void LevitateLocomotionClass::Read_INI(CCINIClass const & ini)
{
	GlobalControls.Drag = ini.Get_Float(INI_NAME, "Drag", GlobalControls.Drag);
	GlobalControls.MaxVelocityWhenHappy = ini.Get_Float(INI_NAME, "MaxVelocityWhenHappy", GlobalControls.MaxVelocityWhenHappy);
	GlobalControls.MaxVelocityWhenFollowing = ini.Get_Float(INI_NAME, "MaxVelocityWhenFollowing", GlobalControls.MaxVelocityWhenFollowing);
	GlobalControls.MaxVelocityWhenPissedOff = ini.Get_Float(INI_NAME, "MaxVelocityWhenPissedOff", GlobalControls.MaxVelocityWhenPissedOff);
	GlobalControls.AccelerationProbability = ini.Get_Float(INI_NAME, "AccelerationProbability", GlobalControls.AccelerationProbability);
	GlobalControls.AccelerationDuration = ini.Get_Int(INI_NAME, "AccelerationDuration", GlobalControls.AccelerationDuration);
	GlobalControls.Acceleration = ini.Get_Float(INI_NAME, "Acceleration", GlobalControls.Acceleration);
	GlobalControls.InitialBoost = ini.Get_Float(INI_NAME, "InitialBoost", GlobalControls.InitialBoost);
	GlobalControls.MaxBlockCount = ini.Get_Int(INI_NAME, "MaxBlockCount", GlobalControls.MaxBlockCount);
	PropulsionSoundEffects = ini.Get_VocType_List(ini, INI_NAME, "PropulsionSoundEffect", PropulsionSoundEffects);
	GlobalControls.IntentionalDeacceleration = ini.Get_Float(INI_NAME, "IntentionalDeacceleration", GlobalControls.IntentionalDeacceleration);
	GlobalControls.IntentionalDriftVelocity = ini.Get_Float(INI_NAME, "IntentionalDriftVelocity", GlobalControls.IntentionalDriftVelocity);
	GlobalControls.ProximityDistance = ini.Get_Float(INI_NAME, "ProximityDistance", GlobalControls.ProximityDistance);
}


/// <summary>
/// Sets or clears the unit's cell occupation bit for its current position, per the mark request.
/// </summary>
/// <param name="mark">MARK_UP to clear the occupation bit, otherwise set it.</param>
void LevitateLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (mark == MARK_UP) {
		LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
	} else {
		LinkedTo->Set_Occupy_Bit(LinkedTo->PositionCoord);
	}
}
