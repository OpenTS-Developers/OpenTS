/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "mech.h"

#include "_map.h"
#include "_rules.h"
#include "cell.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "inline.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "tactical.h"
#include "tube.h"

#include "layer.hh"

#include <cassert>


/// <summary>
/// Creates a mech locomotor.
/// This is the locomotor given to the units that walk from cell to cell rather than roll,
/// and it starts out with no destination and standing still.
/// </summary>
MechLocomotionClass::MechLocomotionClass(void) :
	BASECLASS(),
	DestinationCoord(COORD_NONE),
	HeadToCoord(COORD_NONE),
	IsMoving(false)
{
}


/// <summary>
/// Destroys the mech locomotor.
/// </summary>
MechLocomotionClass::~MechLocomotionClass(void)
{
	//nothing
}


/// <summary>
/// Has the mech been given somewhere to walk to?
/// </summary>
/// <returns>bool; Is the mech under movement orders?</returns>
boolean STDMETHODCALLTYPE MechLocomotionClass::Is_Moving(void)
{
	return(IsMoving);
}


/// <summary>
/// Fetches the destination the mech is ultimately walking to.
/// </summary>
/// <returns>Returns with the destination assigned, or COORD_NONE if the unit has not been
/// given one.</returns>
Coord STDMETHODCALLTYPE MechLocomotionClass::Destination(void)
{
	if (Is_Moving()) {
		return(DestinationCoord);
	}
	return(COORD_NONE);
}


/// <summary>
/// Fetches the location the mech is stepping toward.
/// </summary>
/// <returns>Returns with the location being stepped into, or the unit's own location if it
/// is not part way between cells.</returns>
Coord STDMETHODCALLTYPE MechLocomotionClass::Head_To_Coord(void)
{
	if (HeadToCoord != COORD_NONE) {
		return(HeadToCoord);
	}
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Performs one game frame's worth of movement for the mech.
/// This is the locomotor's entry point from the owning unit's AI.
/// </summary>
/// <returns>bool; Does the mech still have somewhere to walk to?</returns>
boolean STDMETHODCALLTYPE MechLocomotionClass::Process(void)
{
	Movement_AI(true);
	return(Is_Moving());
}


/// <summary>
/// Assigns a destination for the mech to walk to.
/// A stunned unit will ignore the order outright. A destination that lies under a bridge is
/// raised to the deck above it, since that is where a walking unit can actually get to.
/// </summary>
/// <param name="to">The location to walk to.</param>
void STDMETHODCALLTYPE MechLocomotionClass::Move_To(Coord to)
{
	if (LinkedTo->StunDuration <= 0) {
		Coord coord = to;
		DestinationCoord = coord;
		if (Map[(Coord)coord].IsUnderBridge) {
			DestinationCoord.Z += Tactical::Pixel_To_Z_Lepton(4 * ISO_TILE_PIXEL_H / 2);
		}
		IsMoving = true;
	}
}


/// <summary>
/// Cancels the destination the mech was walking to.
/// A unit caught part way between cells is left in motion so that it finishes the step it
/// is taking before coming to rest.
/// </summary>
void STDMETHODCALLTYPE MechLocomotionClass::Stop_Moving(void)
{
	DestinationCoord = COORD_NONE;
	if (HeadToCoord == COORD_NONE) {
		IsMoving = false;
	}
}


/// <summary>
/// Starts the mech turning toward the direction specified.
/// This merely sets the desired facing. The rotation itself is performed over the game
/// frames that follow by the unit's facing tracker.
/// </summary>
/// <param name="coord">The direction the mech should come to face.</param>
void MechLocomotionClass::Do_Turn(DirType coord)
{
	assert(LinkedTo->IsActive);

	LinkedTo->PrimaryFacing.Set_Desired(coord);
}


/// <summary>
/// Overrides the location the mech is currently stepping toward.
/// Use this routine to shove a unit into a particular spot without disturbing its overall
/// destination -- it will walk there and then pick its path up again.
/// </summary>
/// <param name="coord">The location to step into immediately.</param>
void STDMETHODCALLTYPE MechLocomotionClass::Force_Immediate_Destination(Coord coord)
{
	HeadToCoord = coord;
}


/// <summary>
/// Handles the movement logic for a walking unit.
/// This routine is the heart of the mech locomotor. It builds a path toward the assigned
/// destination, dispatches the next leg of that path, copes with blocked, destroyable and
/// tunnel cells encountered along the way, and carries the unit between cells until it
/// arrives.
/// </summary>
/// <param name="continue_moving">Should the routine pick a fresh step immediately when the
/// one it chose turns out to be unusable?</param>
void MechLocomotionClass::Movement_AI(bool continue_moving)
{
	if (LinkedTo->PrimaryFacing.Is_Rotating()) {
		return;
	}

	if (HeadToCoord == COORD_NONE) {
		if (DestinationCoord != COORD_NONE) {
			/*
			 * A destination exists. If there is no current path then try to build
			 * one. If a path already exists, fall through to dispatch the next leg.
			 */
			if (LinkedTo->Path[0] == FACING_NONE) {

				if (LinkedTo->PathDelay != 0) {
					LinkedTo->Stop_Movement_Animation();
					return;
				}

				LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

				if (!LinkedTo->Basic_Path(DestinationCoord.As_Cell(), 0, 0)) {

					if (LinkedTo->Is_In_Same_Zone(DestinationCoord)) {

						int dist = Distance(LinkedTo->Center_Coord(), DestinationCoord);

						if (dist < (int)Rule->CloseEnoughDistance && !LinkedTo->IsTethered) {
							LinkedTo->Assign_Destination(NULL);
						} else {

							if (LinkedTo->TryTryAgain != 0) {
								LinkedTo->TryTryAgain--;
							} else {

								if (LinkedTo->IsNewNavCom) {
									Sound_Effect(Rule->ScoldSound);
								}
								LinkedTo->IsNewNavCom = false;

								Cell dest_cell = DestinationCoord;
								if (LinkedTo->IsLocked) {
									Cell src_cell = LinkedTo->Destination_Coord().As_Cell();
									if (!Map.Is_Same_Cell_Zone(src_cell, dest_cell, LinkedTo->TClass->MZone, LinkedTo->Is_Moving_Onto_Bridge(), Map[dest_cell].IsUnderBridge, LinkedTo->Is_Allowed_To_Leave_Map())) {
										LinkedTo->Assign_Destination(NULL);
									}
								}

								if (LinkedTo->TarCom != NULL) {
									Cell tar_cell = LinkedTo->TarCom->Destination_Coord().As_Cell();
									if (LinkedTo->IsLocked) {
										Cell src_cell = LinkedTo->Destination_Coord().As_Cell();
										if (!Map.Is_Same_Cell_Zone(src_cell, tar_cell, LinkedTo->TClass->MZone, LinkedTo->Is_Moving_Onto_Bridge(), Map[tar_cell].IsUnderBridge, LinkedTo->Is_Allowed_To_Leave_Map())) {
											LinkedTo->Assign_Target(NULL);
										}
									}
								}
							}
						}

						LinkedTo->Set_Speed(0);
						Stop_Moving();
						IsMoving = false;
						return;
					}

					LinkedTo->Assign_Destination(NULL);
					return;
				}

				LinkedTo->TryTryAgain = FootClass::PATH_RETRY;
			}

			/*
			 * Dispatch the next leg of the path. A path entry of FACING_COUNT is
			 * a special tunnel transition.
			 */
			FacingType dir = LinkedTo->Path[0];
			if (dir == FACING_COUNT) {

				int tube = Map[LinkedTo->Get_Coord()].Tube;
				if (tube >= 0 && tube < Tubes.Count()) {
					LinkedTo->Mark(MARK_UP);
					LinkedTo->Clear_Occupy_Bit(LinkedTo->Get_Coord());

					TubeClass * tubeptr = Tubes[tube];

					Cell cell = tubeptr->Exit;
					HeadToCoord = Coord(cell, 0);

					LinkedTo->Advance_Path(1);

					LinkedTo->CurrentTube = tube;
					LinkedTo->CurrentTubeDir = FACING_FIRST;

					cell = tubeptr->Enter;
					Cell visual_cell = Adjacent_Cell(cell, tubeptr->Dirs[0]);
					Coord enter_coord(cell);
					Coord visual_coord = Map[visual_cell].Cell_Coord();
					LinkedTo->LastTubeCoord = LinkedTo->Get_Coord() + (visual_coord - enter_coord);

					int start_h = Map.Get_Height_GL(LinkedTo->Get_Coord());
					cell = tubeptr->Exit;
					int count = tubeptr->Count;
					LinkedTo->LastTubeCoord.Z = start_h + (Map.Get_Height_GL(Coord(cell, 0)) - start_h) / count;
					return;
				}

				LinkedTo->Path[0] = FACING_NONE;
				HeadToCoord = COORD_NONE;
				Stop_Moving();
				LinkedTo->Assign_Destination(NULL);
				return;
			}

			Cell what = Adjacent_Cell(LinkedTo->Get_Coord().As_Cell(), dir);

			Coord head_coord = Coord(what, 0);
			head_coord.Z = Map.Get_Height_GL(head_coord);

			CellClass * what_cell = &Map[what];

			/*
			 * If the bridge status of the cell being moved into differs from this
			 * unit's bridge status, force a fresh look scan next time.
			 */
			if (LinkedTo->IsOnBridge ^ what_cell->IsUnderBridge) {
				LinkedTo->IsPlanningToLook = true;
			}

			MoveType can_enter = LinkedTo->Can_Enter_Cell(&Map[what], dir, LinkedTo->Get_Cell_Height(), 0, true);

			/*
			 * A crusher driving into an occupied destroyable cell that has crushable
			 * overlay just crushes through it -- treat as a normal (clear) move.
			 */
			if (((can_enter != MOVE_DESTROYABLE && can_enter != MOVE_FRIENDLY_DESTROYABLE) || !LinkedTo->TClass->IsCrusher || what_cell->Overlay != OVERLAY_FIRST) && can_enter != MOVE_OK) {

				LinkedTo->Stop_Movement_Animation();

				if (can_enter == MOVE_TEMP) {

					CellClass * cellptr = &Map[what];

					if (continue_moving) {
						LinkedTo->Path[0] = FACING_NONE;
						HeadToCoord = COORD_NONE;
						LinkedTo->PathDelay = 0;
						Movement_AI(false);
						return;
					}

					int dist = LinkedTo->Distance(DestinationCoord);

					if (dist < (int)Rule->CloseEnoughDistance && !LinkedTo->In_Radio_Contact()
						&& abs(DestinationCoord.Z - LinkedTo->Get_Coord().Z) < 2 * LEVEL_LEPTON_H
						&& Map[LinkedTo->Get_Coord()].Land_Type() != LAND_TUNNEL) {

						LinkedTo->Set_Speed(0);
						LinkedTo->Assign_Destination(NULL);
						LinkedTo->Path[0] = FACING_NONE;
						Stop_Moving();
						IsMoving = false;
					} else {

						bool bridge;
						if (cellptr->IsUnderBridge) {
							int z = LinkedTo->Get_Coord().Z;
							bridge = true;
							if (abs(z / LEVEL_LEPTON_H - cellptr->Height) <= 2) {
								bridge = false;
							}
						} else {
							bridge = false;
						}
						cellptr->Incoming(COORD_NONE, true, true, bridge);
					}
					return;
				}

				if (can_enter == MOVE_MOVING_BLOCK) {

					if (!LinkedTo->IsToPathAroundBlockage) {
						LinkedTo->IsToPathAroundBlockage = true;
						LinkedTo->BlockagePathDelay = Rule->BlockagePathDelay;
					}

					if (LinkedTo->PathDelay == 0) {

						bool blockage_expired = LinkedTo->IsToPathAroundBlockage && LinkedTo->BlockagePathDelay == 0;

						bool path_ok = LinkedTo->Basic_Path(DestinationCoord.As_Cell(), 0, blockage_expired ? 2 : 1);
						LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

						if (!path_ok) {
							if (!LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
								LinkedTo->Assign_Destination(NULL);
							} else {
								LinkedTo->On_Movement_Blocked();
							}
						} else {
							LinkedTo->JumpJet_To_Walk();
						}
						return;
					}

					DirType face = ::Direction(LinkedTo->Center_Coord(), head_coord);
					Do_Turn(face);
					return;
				}

				if (can_enter == MOVE_CLOSED_GATE) {
					Map.Try_Open_Gate(LinkedTo, what);
					LinkedTo->TryTryAgain = FootClass::PATH_RETRY;
					return;
				}

				if (can_enter != MOVE_CLOAK) {
					if (can_enter != MOVE_NO) {
						if (can_enter == MOVE_DESTROYABLE || can_enter == MOVE_FRIENDLY_DESTROYABLE) {

							ObjectClass * obj = Map[what].Cell_Object(Point2D(0, 0), false);

							if (continue_moving) {
								LinkedTo->Path[0] = FACING_NONE;
								HeadToCoord = COORD_NONE;
								LinkedTo->PathDelay = 0;
								Movement_AI(false);
								return;
							}

							if (obj != NULL) {
								if (!LinkedTo->House->Is_Ally(obj)) {
									LinkedTo->Override_Mission(MISSION_ATTACK, obj, 0);
								}
							} else {
								if (Map[what].Overlay != OVERLAY_NONE && OverlayTypes[Map[what].Overlay]->IsWall) {
									LinkedTo->Override_Mission(MISSION_ATTACK, &Map[what], 0);
								}
							}
						}

						LinkedTo->Path[0] = FACING_NONE;
						LinkedTo->Set_Speed(0);
						Stop_Moving();
						LinkedTo->IsNewNavCom = false;
						LinkedTo->IsNewNavCom = false;
						return;
					}
				} else {
					Map[what].Shimmer();
				}

				if (!continue_moving) return;

				LinkedTo->Path[0] = FACING_NONE;
				HeadToCoord = COORD_NONE;
				LinkedTo->PathDelay = 0;
				Movement_AI(false);
				return;
			}

			if (Mark_Head_To(head_coord)) {

				FacingType next_dir = LinkedTo->Path[0];
				if (next_dir != FACING_NONE && next_dir != FACING_COUNT) {
					Cell cell = Adjacent_Cell(LinkedTo->Get_Cell(), next_dir);
					OverlayType overlay = Map[cell].Overlay;
					if (overlay != OVERLAY_NONE) {
						OverlayTypeClass * otype = OverlayTypes[overlay];
						if ((LinkedTo->TClass->IsCrusher || LinkedTo->Has_Ability(ABILITY_CRUSHER)) && otype->HeapID == OVERLAY_SANDBAG_WALL) {
							LinkedTo->IsCrushing = true;
							if (LinkedTo->TClass->IsTiltsWhenCrushes) {
								LinkedTo->RockingForwardsPerFrame = -0.01f;
							}
						}
					}
				}

				if (LinkedTo->IsActive) {

					DirType face = ::Direction(LinkedTo->Center_Coord(), HeadToCoord);
					Do_Turn(face);

					if (!LinkedTo->PrimaryFacing.Is_Rotating()) {
						LinkedTo->Set_Speed(1.0);
						LinkedTo->IsNewNavCom = false;
					}
				}
			} else {
				LinkedTo->Set_Speed(0);
				LinkedTo->IsNewNavCom = false;
			}
			return;

		} else {

			/*
			 * With no immediate destination and no overall destination, just stop
			 * moving (if the unit is still rolling) and bail out.
			 */
			if (LinkedTo->Speed > 0.0) {
				LinkedTo->Set_Speed(0.0);
				LinkedTo->IsNewNavCom = false;
				return;
			}
			LinkedTo->IsNewNavCom = false;
			return;
		}
	} else {

		if (Distance(Point2D(LinkedTo->Center_Coord()), Point2D(HeadToCoord)) < 16) {

			LinkedTo->Mark(MARK_UP);

			if (LinkedTo->Path[0] == FACING_NONE) {
				LinkedTo->Path[1] = FACING_NONE;
			}

			LinkedTo->Advance_Path(1);

			LinkedTo->Set_Coord(HeadToCoord);
			LinkedTo->LastPathingCell = HeadToCoord.As_Cell();
			LinkedTo->HeightAGL = 0;

			Mark_Head_To(COORD_NONE);

			if (LinkedTo->Path[0] == FACING_NONE) {
				LinkedTo->Set_Speed(0);
			}

			LinkedTo->IsOccupyingCell = true;

			LinkedTo->IsToPathAroundBlockage = false;

			/*
			 * The walk is over when there is no destination left to reach, or when
			 * this cell is the destination cell and the two are close in height.
			 */
			if (DestinationCoord == COORD_NONE
				|| (LinkedTo->Get_Cell() == DestinationCoord.As_Cell()
					&& abs(LinkedTo->Destination_Coord().Z - DestinationCoord.Z) < 2 * LEVEL_LEPTON_H)) {

				LinkedTo->Assign_Destination(NULL);
				LinkedTo->Set_Speed(0.0);
				Stop_Moving();
				IsMoving = false;
			}

			LinkedTo->Per_Cell_Process(PCP_END);

			if (LinkedTo && LinkedTo->IsActive) {
				if (!LinkedTo->IsInLimbo && !LinkedTo->IsFalling) {
					FacingType next_move = LinkedTo->Path[0];
					if (next_move != FACING_NONE && next_move != FACING_COUNT) {
						Cell check_coord = Adjacent_Cell(LinkedTo->Get_Cell(), next_move);

						OverlayType overlay = Map[check_coord].Overlay;
						if (overlay != OVERLAY_NONE) {
							OverlayTypeClass * otype = OverlayTypes[overlay];
							if ((LinkedTo->TClass->IsCrusher || LinkedTo->Has_Ability(ABILITY_CRUSHER)) && otype->HeapID == OVERLAY_SANDBAG_WALL) {
								LinkedTo->IsCrushing = true;

								if (LinkedTo->TClass->IsTiltsWhenCrushes) {
									LinkedTo->RockingForwardsPerFrame = -0.01f;
								}
							}
						}
					}

					if (LinkedTo->IsActive && !LinkedTo->IsInLimbo) {
						LinkedTo->Mark(MARK_DOWN);
						LinkedTo->IsNewNavCom = false;
					}
				}
			}
			return;
		}

		LinkedTo->Set_Speed(1.0);

		int speed = LinkedTo->Current_Speed();

		DirType desired_facing = ::Direction(LinkedTo->Center_Coord(), HeadToCoord);
		Do_Turn(desired_facing);

		if (LinkedTo->PrimaryFacing.Is_Rotating()) {
			return;
		}

		if (LinkedTo->Occupies_Cells()) {
			LinkedTo->Clear_Occupy_Bit(LinkedTo->Get_Coord());
			LinkedTo->IsOccupyingCell = false;

			LinkedTo->IsToPathAroundBlockage = false;

		}

		Cell old_cell = LinkedTo->Get_Cell();

		Coord new_coord = Move_Coord(LinkedTo->Get_Coord(), desired_facing, speed);
		Cell new_cell = new_coord;

		if (old_cell != new_cell) {

			LinkedTo->Mark(MARK_UP);
			LinkedTo->Set_Coord(new_coord);

			CellClass * old_cellptr = &Map[old_cell];
			CellClass * new_cellptr = &Map[new_cell];

			if (new_cellptr->Height == old_cellptr->Height - BRIDGE_CELL_HEIGHT && new_cellptr->IsUnderBridge) {
				LinkedTo->IsOnBridge = true;
			}
			if (!new_cellptr->IsUnderBridge && old_cellptr->IsUnderBridge) {
				LinkedTo->IsOnBridge = false;
			}

			LinkedTo->HeightAGL = 0;
			LinkedTo->Mark(MARK_DOWN);

			Map[LinkedTo->Get_Coord()].Trigger_Veins();

			LinkedTo->IsNewNavCom = false;
			return;
		} else {
			bool was_down = LinkedTo->IsDown;
			LinkedTo->IsDown = false;
			LinkedTo->Set_Coord(new_coord);
			LinkedTo->HeightAGL = 0;
			LinkedTo->IsDown = was_down;
			LinkedTo->IsNewNavCom = false;
			return;
		}
	}
}


/// <summary>
/// Moves the mech's cell reservation to the cell it is about to step into.
/// The reservation on the previous location is released and one is claimed on the new
/// location. Any crate lying in the destination is collected on the way, and that can
/// destroy or otherwise remove the unit -- hence the return value.
/// </summary>
/// <param name="coord">The location the mech intends to step into. Pass COORD_NONE to
/// merely release the reservation it holds.</param>
/// <returns>bool; Is the mech still able to proceed to the location specified?</returns>
bool MechLocomotionClass::Mark_Head_To(Coord const & coord)
{
	Coord crd = coord;

	if (crd != COORD_NONE) {
		crd.Z = Map.Get_Height_GL(crd);
		if (Map[coord].IsUnderBridge && LinkedTo->Get_Coord().Z > crd.Z + 2 * LEVEL_LEPTON_H + LEVEL_LEPTON_H) {
			crd.Z += BRIDGE_LEPTON_HEIGHT;
		}
	}

	if (HeadToCoord != COORD_NONE) {
		LinkedTo->Clear_Occupy_Bit(HeadToCoord);
	}

	if (Map[crd].Goodie_Check(LinkedTo) && !LinkedTo->IsInLimbo) {
		HeadToCoord = crd;
	} else {
		HeadToCoord = COORD_NONE;
		if (!LinkedTo->IsActive) {
			return(false);
		}
	}

	if (HeadToCoord != COORD_NONE) {
		LinkedTo->Set_Occupy_Bit(HeadToCoord);
		return(true);
	}

	LinkedTo->Set_Occupy_Bit(LinkedTo->PositionCoord);
	return(false);
}


/// <summary>
/// Fetches the class identifier of this locomotor.
/// The persistence layer uses this identifier to create a locomotor of the right kind
/// when a saved game is loaded.
/// </summary>
/// <param name="retval">Pointer to the buffer to fill in with the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no buffer was supplied.</returns>
HRESULT STDMETHODCALLTYPE MechLocomotionClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_MechLocomotion;
	return(S_OK);
}


/// <summary>
/// Lists the members this mech locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void MechLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(DestinationCoord);
	stream.Serialize(HeadToCoord);
	stream.Serialize(IsMoving);
}


/// <summary>
/// Fetches the display layer that the mech is rendered in.
/// </summary>
/// <returns>Returns with the layer appropriate to a unit that walks on the ground.</returns>
LayerType STDMETHODCALLTYPE MechLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_GROUND);
}


/// <summary>
/// Is the mech in visible motion at this instant?
/// This differs from Is_Moving in that a unit which has been given a destination but is
/// standing still -- blocked, or waiting on a path -- is not moving now.
/// </summary>
/// <returns>bool; Is the mech turning or walking right now?</returns>
boolean STDMETHODCALLTYPE MechLocomotionClass::Is_Moving_Now(void)
{
	if (LinkedTo->PrimaryFacing.Is_Rotating()) {
		return(true);
	}
	if (Is_Moving() && HeadToCoord != COORD_NONE && LinkedTo->Current_Speed() > 0) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Marks or clears the occupation bit for the cell the mech is stepping into.
/// This routine is called when the unit is marked up or down on the map so that the cell
/// it is walking toward stays reserved for it.
/// </summary>
/// <param name="mark">The marking operation to perform; MARK_UP releases the cell.</param>
void STDMETHODCALLTYPE MechLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (mark == MARK_UP) {
		LinkedTo->Clear_Occupy_Bit((Coord)Head_To_Coord());
	} else {
		LinkedTo->Set_Occupy_Bit((Coord)Head_To_Coord());
	}
}


/// <summary>
/// Is the mech already walking into the location specified?
/// This routine is used by the cell reservation logic to tell whether this unit has
/// already claimed the spot it is being asked about.
/// </summary>
/// <param name="to">The location to test against.</param>
/// <returns>bool; Is the mech heading into that location?</returns>
boolean STDMETHODCALLTYPE MechLocomotionClass::Is_Moving_Here(Coord to)
{
	Coord coord = Head_To_Coord();

	if (Coord(coord).As_Cell() == to.As_Cell() && abs(coord.Z - to.Z) <= LEVEL_LEPTON_H) {
		return(true);
	}

	return(false);
}
