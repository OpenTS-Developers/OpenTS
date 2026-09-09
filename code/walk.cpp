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

#include "walk.h"

#include "_map.h"
#include "_rules.h"
#include "_tactica.h"
#include "aircraft.h"
#include "building.h"
#include "cell.h"
#include "foot.h"
#include "globals.h"
#include "house.h"
#include "inline.h"
#include "overtype.h"
#include "rules.h"
#include "saveload.h"
#include "savestream.h"
#include "tactical.h"
#include "tube.h"
#include "unit.h"

#include "layer.hh"


/// <summary>
/// Constructs a walking locomotor.
/// This is the locomotor used by infantry, who travel on foot between the sub-cell
/// spots of the map rather than from cell center to cell center.
/// </summary>
WalkLocomotionClass::WalkLocomotionClass(void) :
	BASECLASS(),
	DestinationCoord(COORD_NONE),
	HeadToCoord(COORD_NONE),
	IsMoving(false),
	IsProcessingMovement(false),
	IsReallyMoving(false)
{
}


/// <summary>
/// Destroys the walking locomotor.
/// </summary>
WalkLocomotionClass::~WalkLocomotionClass(void)
{
}


/// <summary>
/// Is the infantry traveling somewhere?
/// </summary>
/// <returns>bool; Does the infantry have somewhere it is trying to get to?</returns>
bool WalkLocomotionClass::Is_Moving(void)
{
	return(IsMoving);
}


/// <summary>
/// Is the infantry under way at this moment?
/// This routine separates an infantry that is walking here and now from one that is
/// merely under orders to travel but is standing still.
/// </summary>
/// <returns>bool; Is the infantry moving right now?</returns>
bool WalkLocomotionClass::Is_Moving_Now(void)
{
	if (Is_Moving() && LinkedTo->Speed > 0 && HeadToCoord != COORD_NONE) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the ultimate destination of the infantry.
/// </summary>
/// <returns>Returns with the coordinate being traveled to, or COORD_NONE if the infantry has
/// nowhere it needs to be.</returns>
Coord WalkLocomotionClass::Destination(void)
{
	if (Is_Moving()) {
		return(DestinationCoord);
	}
	return(COORD_NONE);
}


/// <summary>
/// Fetches the spot the infantry is stepping to.
/// </summary>
/// <returns>Returns with the immediate destination, or the current position if the infantry
/// is not part way between spots.</returns>
Coord WalkLocomotionClass::Head_To_Coord(void)
{
	if (HeadToCoord != COORD_NONE) {
		return(HeadToCoord);
	}
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Performs one logic pass of the walking locomotor.
/// The owning foot object calls this routine every game logic loop to advance the
/// infantry along its path.
/// </summary>
/// <returns>bool; Is the infantry still traveling somewhere?</returns>
bool WalkLocomotionClass::Process(void)
{
	IsProcessingMovement = true;
	Movement_AI(true);
	IsProcessingMovement = false;
	return(Is_Moving());
}


/// <summary>
/// Sets the ultimate destination for the infantry to walk to.
/// A stunned infantry ignores the request outright. A destination that lies under a
/// bridge is raised to the bridge deck, so that the infantry walks over it rather
/// than beneath it.
/// </summary>
/// <param name="to">The coordinate to travel to, or COORD_NONE to clear the destination.</param>
void WalkLocomotionClass::Move_To(Coord to)
{
	if (LinkedTo->StunDuration <= 0) {
		DestinationCoord = to;
		if (to != COORD_NONE) {
			if (Map[to].IsUnderBridge) {
				DestinationCoord.Z += TacticalMap->Pixel_To_Z_Lepton(4 * ISO_TILE_PIXEL_H / 2);
			}
			IsMoving = true;
		} else {
			if (HeadToCoord == COORD_NONE) {
				IsMoving = false;
			}
		}
	}
}


/// <summary>
/// Stops the infantry from traveling any further.
/// The step already under way is allowed to finish; it is the ultimate destination
/// that is forgotten.
/// </summary>
void WalkLocomotionClass::Stop_Moving(void)
{
	DestinationCoord = COORD_NONE;
	if (HeadToCoord == COORD_NONE) {
		IsMoving = false;
	}
}


/// <summary>
/// Turns the infantry to face the direction specified.
/// Infantry snap around instantly, so there is no rotation to play out over time.
/// </summary>
/// <param name="dir">The direction the infantry should face.</param>
void WalkLocomotionClass::Do_Turn(DirType dir)
{
	LinkedTo->PrimaryFacing.Set(dir);
}


/// <summary>
/// Sets the spot that the infantry is stepping to right now.
/// This routine overrides the step in progress. It is used when the infantry must be
/// redirected without waiting for the current step to finish.
/// </summary>
/// <param name="coord">The coordinate to step to, or COORD_NONE to abandon the step.</param>
void WalkLocomotionClass::Force_Immediate_Destination(Coord coord)
{
	HeadToCoord = coord;
	if (HeadToCoord == COORD_NONE && DestinationCoord == COORD_NONE) {
		IsMoving = false;
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
void WalkLocomotionClass::Movement_AI(bool first_pass)
{
	if (HeadToCoord == COORD_NONE) {
		if (DestinationCoord != COORD_NONE) {
			if (LinkedTo->Path[0] == FACING_NONE) {
				if (LinkedTo->PathDelay != 0) {
					LinkedTo->Stop_Movement_Animation();
					return;
				}

				LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

				if (!LinkedTo->Basic_Path(DestinationCoord.As_Cell(), 0, 0)) {
					IsReallyMoving = false;

					if (LinkedTo->Is_In_Same_Zone(DestinationCoord)) {
						LinkedTo->On_Movement_Blocked();

						if (Distance(LinkedTo->Center_Coord(), DestinationCoord) < Rule->CloseEnoughDistance && !LinkedTo->IsTethered) {
							LinkedTo->Assign_Destination(NULL);
						} else {
							if (LinkedTo->TryTryAgain) {
								LinkedTo->TryTryAgain--;
							} else {
								if (LinkedTo->IsNewNavCom) {
									Sound_Effect(Rule->ScoldSound);
								}
								LinkedTo->IsNewNavCom = false;

								Cell dest = DestinationCoord.As_Cell();

								if (LinkedTo->IsLocked) {
									Cell nav_dest = LinkedTo->Destination_Coord().As_Cell();
									if (!Map.Is_Same_Cell_Zone(nav_dest, dest, LinkedTo->TClass->MZone, LinkedTo->Is_Moving_Onto_Bridge(), Map[dest].IsUnderBridge, LinkedTo->Is_Allowed_To_Leave_Map())) {
										LinkedTo->Assign_Destination(NULL);
									}
								}

								if (LinkedTo->TarCom != NULL) {
									Cell tar_dest = LinkedTo->TarCom->Destination_Coord().As_Cell();
									if (LinkedTo->IsLocked) {
										if (LinkedTo->TarCom != NULL) {
											Cell nav_dest = LinkedTo->Destination_Coord().As_Cell();
											if (!Map.Is_Same_Cell_Zone(nav_dest, tar_dest, LinkedTo->TClass->MZone, LinkedTo->Is_Moving_Onto_Bridge(), Map[tar_dest].IsUnderBridge, LinkedTo->Is_Allowed_To_Leave_Map())) {
												LinkedTo->Assign_Target(NULL);
											}
										}
									}
								}
							}
						}

						LinkedTo->Set_Speed(0.0);
						Stop_Moving();
						return;
					}

					LinkedTo->Assign_Destination(NULL);
					return;
				}

				LinkedTo->TryTryAgain = FootClass::PATH_RETRY;
				if (LinkedTo->JumpJet_To_Walk()) {
					return;
				}
			}

			FacingType dir = (FacingType)LinkedTo->Path[0];
			if (dir == FACING_COUNT) {
				int tube = Map[LinkedTo->Get_Coord()].Tube;
				if (tube >= 0 && tube < Tubes.Count()) {
					LinkedTo->Mark(MARK_UP);
					LinkedTo->Clear_Occupy_Bit(LinkedTo->Get_Coord());

					TubeClass * tube_ptr = Tubes[tube];
					Cell cell = tube_ptr->Exit;
					HeadToCoord = Coord(cell, 0);

					LinkedTo->Advance_Path(1);

					LinkedTo->CurrentTube = tube;
					LinkedTo->CurrentTubeDir = FACING_FIRST;

					cell = tube_ptr->Enter;
					Cell visual_cell = Adjacent_Cell(cell, tube_ptr->Dirs[0]);
					Coord enter_coord(cell);
					Coord visual_coord = Map[visual_cell].Cell_Coord();
					LinkedTo->LastTubeCoord = LinkedTo->Get_Coord() + (visual_coord - enter_coord);

					int start_h = Map.Get_Height_GL(LinkedTo->Get_Coord());
					cell = tube_ptr->Exit;
					LinkedTo->LastTubeCoord.Z = start_h + (Map.Get_Height_GL(Coord(cell, 0)) - start_h) / tube_ptr->Count;
					return;
				}

				LinkedTo->Path[0] = FACING_NONE;
				HeadToCoord = COORD_NONE;
				Stop_Moving();
				LinkedTo->Assign_Destination(NULL);
				return;
			}

			Coord coord = Adjacent_Cell(LinkedTo->Get_Coord(), dir);
			Cell cell = coord;

			if (LinkedTo->IsOnBridge ^ Map[Adjacent_Cell(LinkedTo->Get_Coord(), dir)].IsUnderBridge) {
				LinkedTo->IsPlanningToLook = true;
			}

			MoveType move = LinkedTo->Can_Enter_Cell(&Map[cell], dir, LinkedTo->Get_Cell_Height(), NULL, true);
			if (move != MOVE_OK) {
				IsReallyMoving = false;
				LinkedTo->Stop_Movement_Animation();

				if (move == MOVE_TEMP) {
					CellClass & cellptr = Map[cell];
					if (first_pass) {
						LinkedTo->Path[0] = FACING_NONE;
						HeadToCoord = COORD_NONE;
						LinkedTo->PathDelay = 0;
						Movement_AI(false);
					} else {
						int dist = LinkedTo->Center_Coord().Distance_To(DestinationCoord);
						if (dist < Rule->CloseEnoughDistance &&
							!LinkedTo->In_Radio_Contact() &&
							abs(DestinationCoord.Z - LinkedTo->Get_Coord().Z) < 2 * LEVEL_LEPTON_H &&
							Map[LinkedTo->Get_Coord()].Land_Type() != LAND_TUNNEL) {

							HeadToCoord = COORD_NONE;
							Stop_Moving();
							LinkedTo->Set_Speed(0.0);
							LinkedTo->Assign_Destination(NULL);
							LinkedTo->Path[0] = FACING_NONE;
						} else {
							bool isbridge = cellptr.IsUnderBridge && abs(LinkedTo->Get_Coord().Z / LEVEL_LEPTON_H - cellptr.Height) > 2;
							cellptr.Incoming(COORD_NONE, true, true, isbridge);
						}
					}
					return;
				}

				if (move == MOVE_MOVING_BLOCK) {
					if (!LinkedTo->IsToPathAroundBlockage) {
						LinkedTo->IsToPathAroundBlockage = true;
						LinkedTo->BlockagePathDelay = Rule->BlockagePathDelay;
					}

					if (LinkedTo->PathDelay != 0) {
						return;
					}

					bool blocked = LinkedTo->IsToPathAroundBlockage && LinkedTo->BlockagePathDelay == 0;
					bool found = LinkedTo->Basic_Path(DestinationCoord.As_Cell(), 0, blocked ? 2 : 1);
					LinkedTo->PathDelay = Rule->PathDelay * TICKS_PER_MINUTE;

					if (!found) {
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

				if (move == MOVE_CLOSED_GATE) {
					Map.Try_Open_Gate(LinkedTo, cell);
					LinkedTo->TryTryAgain = FootClass::PATH_RETRY;
					return;
				}

				if (move == MOVE_DESTROYABLE || move == MOVE_FRIENDLY_DESTROYABLE) {
					ObjectClass * object = Map[cell].Cell_Object();
					if (first_pass) {
						LinkedTo->Path[0] = FACING_NONE;
						HeadToCoord = COORD_NONE;
						LinkedTo->PathDelay = 0;
						Movement_AI(false);
						return;
					}

					if (object != NULL) {
						if (!LinkedTo->House->Is_Ally(object)) {
							LinkedTo->Override_Mission(MISSION_ATTACK, object, NULL);
						}
					} else {
						if (Map[cell].Overlay != OVERLAY_NONE && OverlayTypes[Map[cell].Overlay]->IsWall) {
							LinkedTo->Override_Mission(MISSION_ATTACK, &Map[cell], NULL);
						}
					}
				}

				if (move != MOVE_CLOAK) {
					if (move != MOVE_NO) {
						LinkedTo->Path[0] = FACING_NONE;
						LinkedTo->Set_Speed(0.0);
						Stop_Moving();
						LinkedTo->IsNewNavCom = false;
						LinkedTo->IsNewNavCom = false;
						return;
					}
				} else {
					Map[cell].Shimmer();
				}

				if (first_pass) {
					LinkedTo->Path[0] = FACING_NONE;
					HeadToCoord = COORD_NONE;
					LinkedTo->PathDelay = 0;
					Movement_AI(false);
				}
				return;
			}

			if (Mark_Head_To(coord)) {
				IsReallyMoving = true;
				if (LinkedTo->IsActive) {
					DirType face = ::Direction(LinkedTo->Center_Coord(), HeadToCoord);
					Do_Turn(face);
					LinkedTo->Set_Speed(1.0);
					LinkedTo->IsNewNavCom = false;
				}
			} else {
				LinkedTo->Set_Speed(0.0);
				LinkedTo->IsNewNavCom = false;
			}
			return;
		} else {
			if (LinkedTo->Speed > 0.0) {
				LinkedTo->Set_Speed(0.0);
				LinkedTo->IsNewNavCom = false;
				return;
			}
			LinkedTo->IsNewNavCom = false;
			return;
		}
	} else {

		IsReallyMoving = true;

		if (Distance(Point2D(LinkedTo->Center_Coord()), Point2D(HeadToCoord)) < 17) {
			LinkedTo->Mark(MARK_UP);

			if (LinkedTo->Path[0] == FACING_NONE) {
				LinkedTo->Path[1] = FACING_NONE;
			}

			LinkedTo->Advance_Path(1);

			LinkedTo->Set_Coord(HeadToCoord);
			LinkedTo->LastPathingCell = HeadToCoord.As_Cell();
			LinkedTo->Set_Height_AGL(0);
			LinkedTo->IsToPathAroundBlockage = false;

			Mark_Head_To(COORD_NONE);

			if (LinkedTo->Path[0] == FACING_NONE) {
				LinkedTo->Set_Speed(0.0);
			}

			LinkedTo->Per_Cell_Process(PCP_END);

			if (LinkedTo && LinkedTo->IsActive && !LinkedTo->IsInLimbo && !LinkedTo->IsFalling) {
				if (DestinationCoord == COORD_NONE) {
					LinkedTo->Assign_Destination(NULL);
					LinkedTo->Set_Speed(0.0);
					HeadToCoord = COORD_NONE;
					Stop_Moving();
					IsReallyMoving = false;
				} else {
					if (LinkedTo->Destination_Coord().As_Cell() == DestinationCoord.As_Cell()) {
						if (abs(LinkedTo->Destination_Coord().Z - DestinationCoord.Z) < 2 * LEVEL_LEPTON_H) {
							LinkedTo->Assign_Destination(NULL);
							LinkedTo->Set_Speed(0.0);
							HeadToCoord = COORD_NONE;
							Stop_Moving();
							IsReallyMoving = false;
						}
					}
				}

				LinkedTo->Mark(MARK_DOWN);
				LinkedTo->IsNewNavCom = false;
			}
			return;
		}

		if (LinkedTo->Is_Immobilized()) {
			IsReallyMoving = false;
			LinkedTo->IsNewNavCom = false;
			return;
		}

		LinkedTo->Set_Speed(1.0);
		int speed = LinkedTo->Current_Speed();
		LinkedTo->IsToPathAroundBlockage = false;

		DirType face = ::Direction(LinkedTo->Center_Coord(), HeadToCoord);
		Do_Turn(face);

		Cell old_cell = LinkedTo->Get_Cell();
		Coord new_coord = Move_Coord(LinkedTo->Get_Coord(), face, speed);
		Cell new_cell = new_coord.As_Cell();

		if (old_cell != new_cell) {
			LinkedTo->Mark(MARK_UP);
			LinkedTo->Set_Coord(new_coord);

			CellClass * old_cellptr = &Map[old_cell];
			CellClass * new_cellptr = &Map[new_cell];

			if (new_cellptr->Height == old_cellptr->Height - BRIDGE_CELL_HEIGHT) {
				if (new_cellptr->IsUnderBridge) {
					LinkedTo->IsOnBridge = true;
				}
			}
			if (!new_cellptr->IsUnderBridge && old_cellptr->IsUnderBridge) {
				LinkedTo->IsOnBridge = false;
			}

			LinkedTo->Set_Height_AGL(0);
			LinkedTo->Mark(MARK_DOWN);
			Map[LinkedTo->Get_Coord()].Trigger_Veins();
			LinkedTo->IsNewNavCom = false;
		} else {
			bool was_down = LinkedTo->IsDown;
			LinkedTo->IsDown = false;
			LinkedTo->Set_Coord(new_coord);
			LinkedTo->Set_Height_AGL(0);
			LinkedTo->IsDown = was_down;
			LinkedTo->IsNewNavCom = false;
		}
	}
}


/// <summary>
/// Reserves the sub-cell spot that the infantry is about to step onto.
/// The spot currently held is released first, then the closest free spot within the
/// destination is claimed instead. Whatever crate happens to be waiting there is
/// collected as part of the move.
/// </summary>
/// <param name="coord">The coordinate to step toward, or COORD_NONE to merely release the
/// spot currently held.</param>
/// <returns>bool; Was a spot successfully reserved?</returns>
/// <remarks>Collecting a crate can destroy the infantry outright, so the caller must not
/// presume it still exists when this routine returns false.</remarks>
bool WalkLocomotionClass::Mark_Head_To(Coord const & coord)
{
	if (HeadToCoord != COORD_NONE) {
		LinkedTo->Clear_Occupy_Bit(HeadToCoord);
	} else {
		LinkedTo->Clear_Occupy_Bit(LinkedTo->PositionCoord);
	}

	Coord crd = coord;
	bool any_spot = false;

	if (coord != COORD_NONE) {
		if (LinkedTo->Mission == MISSION_CAPTURE || LinkedTo->Mission == MISSION_ENTER || LinkedTo->Mission == MISSION_GUARD_AREA || LinkedTo->Mission == MISSION_PATROL) {
			UnitClass * unit = dynamic_cast<UnitClass *>(LinkedTo->NavCom);
			if (unit != NULL && unit->PositionCell == coord.As_Cell()) {
				any_spot = true;
			}
			AircraftClass * aircraft = dynamic_cast<AircraftClass *>(LinkedTo->NavCom);
			if (aircraft != NULL && aircraft->PositionCell == coord.As_Cell()) {
				any_spot = true;
			}
			BuildingClass * building = dynamic_cast<BuildingClass *>(LinkedTo->NavCom);
			if (building != NULL && Map[coord].Cell_Building() == building) {
				any_spot = true;
			}
		}

		bool isbridge = Map[crd].IsUnderBridge && LinkedTo->PositionCoord.Z > 3 * LEVEL_LEPTON_H + Map.Get_Height_GL(crd);
		HeadToCoord = Map[crd].Closest_Free_Spot(crd, any_spot, isbridge);

		if (!Map[HeadToCoord].Goodie_Check(LinkedTo) && !LinkedTo->IsInLimbo) {
			HeadToCoord = COORD_NONE;
			if (!LinkedTo->IsActive) {
				return(false);
			}
		}
	} else {
		HeadToCoord = coord;
	}

	if (HeadToCoord != COORD_NONE) {
		LinkedTo->Set_Occupy_Bit(HeadToCoord);
		return(true);
	}

	LinkedTo->Set_Occupy_Bit(LinkedTo->PositionCoord);
	return(false);
}


ClassID WalkLocomotionClass::Class_ID(void) const
{
	return(ClassID_WalkLocomotion);
}


/// <summary>
/// Lists the members this walk locomotor carries.
/// The locomotor this one was stacked on top of is a separate persistent object rather
/// than a member, so it travels as a record of its own and is recreated as the class it was
/// saved as.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void WalkLocomotionClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(DestinationCoord);
	stream.Serialize(HeadToCoord);
	stream.Serialize(IsMoving);
	stream.Serialize(IsProcessingMovement);
	stream.Serialize(IsReallyMoving);

	bool haspiggy = (Piggybacker != NULL);
	stream.Serialize(haspiggy);

	if (haspiggy) {
		if (stream.Is_Saving()) {
			Save_Object(stream, Piggybacker.get());
		} else {
			Piggybacker = Load_Locomotor(stream);
		}
	}
}


/// <summary>
/// Fetches the display layer that walking objects belong in.
/// </summary>
/// <returns>Returns with the layer that objects using this locomotor render into.</returns>
LayerType WalkLocomotionClass::In_Which_Layer(void)
{
	return(LAYER_GROUND);
}


/// <summary>
/// Attaches a piggybacking locomotor to this one.
/// This routine is used when some temporary means of travel, such as being carried
/// along, must take over from ordinary walking.
/// </summary>
/// <param name="carried">The locomotor that is to take over the unit.</param>
/// <returns>bool; Was the locomotor taken on? One already carrying a locomotor refuses.</returns>
bool WalkLocomotionClass::Begin_Piggyback(std::unique_ptr<ILocomotion> & carried)
{
	if (carried == nullptr || Piggybacker != nullptr) {
		return(false);
	}
	Piggybacker = std::move(carried);
	return(true);
}


/// <summary>
/// Ends the piggyback session and hands back the locomotor that was riding along.
/// Ownership of the piggybacking locomotor passes to the caller.
/// </summary>
/// <returns>Returns with the locomotor that was riding, or nothing when none was.</returns>
std::unique_ptr<ILocomotion> WalkLocomotionClass::End_Piggyback(void)
{
	return(std::move(Piggybacker));
}


/// <summary>
/// Can the piggybacking locomotor hand control back now?
/// This routine is consulted before a piggyback session is ended, so that walking is
/// not resumed part way through a step.
/// </summary>
/// <returns>bool; Is it safe to end the piggyback?</returns>
bool WalkLocomotionClass::Is_Ok_To_End(void)
{
	if (!Is_Moving() && Piggybacker != NULL && !IsProcessingMovement) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Releases the sub-cell spot that this infantry has reserved.
/// This routine is called when the infantry is being lifted off the map so that the
/// spot it had claimed becomes available to others again.
/// </summary>
/// <param name="mark">The occupancy marking operation being performed.</param>
void WalkLocomotionClass::Mark_All_Occupation_Bits(int mark)
{
	if (mark == MARK_UP) {
		LinkedTo->Clear_Occupy_Bit(Head_To_Coord());
	}
}


/// <summary>
/// Is the infantry stepping to the location specified?
/// This routine is used by the occupancy logic to discover whether this infantry has
/// already laid claim to the spot in question.
/// </summary>
/// <param name="to">The coordinate to test the immediate destination against.</param>
/// <returns>bool; Is the infantry walking to that spot?</returns>
bool WalkLocomotionClass::Is_Moving_Here(Coord to)
{
	Coord headto = Head_To_Coord();
	if (headto.As_Cell() == Coord(to).As_Cell() && abs(headto.Z - to.Z) <= LEVEL_LEPTON_H) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Is the infantry actually walking right now?
/// This routine tells apart an infantry that is genuinely under way from one that
/// merely holds orders to travel but has yet to take a step.
/// </summary>
/// <returns>bool; Is the infantry really moving at this moment?</returns>
bool WalkLocomotionClass::Is_Really_Moving_Now(void)
{
	return(IsReallyMoving);
}
