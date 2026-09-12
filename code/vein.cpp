/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "vein.h"

#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_theater.h"
#include "astar.h"
#include "ccrand.h"
#include "cell.h"
#include "draw.h"
#include "globals.h"
#include "house.h"
#include "incdec.h"
#include "inline.h"
#include "isotype.h"
#include "lightcon.h"
#include "mixfile.h"
#include "mouse.h"
#include "overtype.h"
#include "particle.h"
#include "partsys.h"
#include "ptype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "scheme.h"
#include "stimer.h"
#include "sun.h"
#include "swizzle.h"
#include "tactical.h"
#include "terrtype.h"
#include "tracker.h"
#include "vector.h"

#include "ramp.hh"

#include <cstdio>
#include <optional>

DynamicVectorClass<VeinholeMonsterClass *> VeinholeMonsterClass::VeinholeMonsters;

void const * VeinholeMonsterClass::MonsterShape = NULL;
static bool * GlobalGrowthState = NULL;


/// <summary>
/// Creates a veinhole monster in preparation for loading a saved game.
/// The monster's own state arrives from the save game stream, but the growth records
/// are allocated fresh here since the pointers that were saved are meaningless now.
/// </summary>
VeinholeMonsterClass::VeinholeMonsterClass(void) :
	BASECLASS(),
	GrowthCount(0),
	GrowthTimer(0),
	GrowthState(NULL),
	CurrentState(IDLE),
	DesiredState(IDLE),
	Control(),
	LogicTimer(0),
	CellID(0, 0),
	ShapeFrame(0),
	IsDead(false),
	IsToPuffGas(false),
	VeinCount(0)
{
	VeinholeMonsters.Add(this);
	GrowthQueue.Clear();
	GrowthQueue.Reserve(Rule->MaxVeinholeGrowth);
	if (GlobalGrowthState == NULL) {
		GlobalGrowthState = new bool [Map_Cell_Count()];
	}
	GrowthState = new bool [Map_Cell_Count()];
}


/// <summary>
/// Creates a veinhole monster at the cell specified.
/// The ground around the monster is sunk into a pit and ramped down to meet the terrain
/// beyond, and the records it needs to tend its own veins are allocated. A monster that
/// cannot be accommodated at the location is left in limbo and never joins the game.
/// </summary>
/// <param name="cell">The cell to place the center of the monster at.</param>
VeinholeMonsterClass::VeinholeMonsterClass(Cell const & cell) :
	BASECLASS(),
	GrowthCount(0),
	GrowthTimer(Rule->VeinholeGrowthRate),
	GrowthState(NULL),
	CurrentState(IDLE),
	DesiredState(IDLE),
	Control(),
	LogicTimer(0),
	CellID(cell),
	ShapeFrame(0),
	IsDead(NULL),
	IsToPuffGas(NULL),
	VeinCount(0)
{
	Create_ID();

	if (Can_Monster_Go_Here(cell)) {
		VeinholeMonsters.Add(this);

		if (!ScenarioInit) {
			static char _ramps[3][3] = {
				/// X - 1          X          X + 1
				{ RAMP_MID_SE, RAMP_SOUTH, RAMP_MID_SW }, /// Y - 1
				{ RAMP_EAST,   RAMP_NONE,  RAMP_WEST   }, // Y
				{ RAMP_MID_NE, RAMP_NORTH, RAMP_MID_NW }, /// Y + 1
			};
			for (int y = -1; y <= 1; y++) {
				for (int x = -1; x <= 1; x++) {
					CellClass * cellptr = &Map[cell + Cell(x, y)];
					cellptr->Ramp = _ramps[y + 1][x + 1];
					if (cellptr->Ramp > RAMP_NONE) {
						cellptr->ITType = IsometricTileType(cellptr->Ramp + IsometricTileTypeClass::RampStart - 1);
					} else {
						cellptr->ITType = ISOTILE_CLEAR;
					}
					cellptr->SubTile = 0;
					cellptr->Height--;
				}
			}
		}

		Control.Set_Rate(4);
		Control.Set_Step(0);
		Control.Set_Stage(-1);

		GrowthQueue.Clear();
		GrowthQueue.Reserve(Rule->MaxVeinholeGrowth);

		if (GlobalGrowthState == NULL) {
			GlobalGrowthState = new bool[Map_Cell_Count()];
		}
		GrowthState = new bool[Map_Cell_Count()];

		Coord coord = cell.As_Coord();
		coord.Z = Map.Get_Height_GL(coord);
		Unlimbo(coord);

		Strength = Rule->VeinholeTypeClass->MaxStrength;

		TargetTracker.Add_Index(Fetch_ID(), this);
	}
}


/// <summary>
/// Destroys this veinhole monster and scrubs its veins off the map.
/// The cells around the veinhole are stripped of their vein overlay and flagged for
/// redraw, and the monster is detached from everything that was tracking it.
/// </summary>
VeinholeMonsterClass::~VeinholeMonsterClass(void)
{
	for (int x = -2; x <= 2; x++) {
		for (int y = -2; y <= 2; y++) {
			Cell cell(CellID + Cell(x,y));

			int cellid = Map_Cell_Index(cell);
			if (cellid >= 0 && cellid < Map_Cell_Count()) {
				GlobalGrowthState[cellid] = false;
				GrowthState[cellid] = false;
			}

			CellClass * cellptr = &Map[cell];
			if (cellptr->Overlay == OVERLAY_VEINS || (cellptr->Overlay != OVERLAY_NONE && OverlayTypes[cellptr->Overlay]->IsVeins)) {
				Map.Radar_Background(cellptr->CellID);
				Point2D point(TacticalRect.Top_Left());
				TacticalMap->Register_Dirty_Area(Union(cellptr->Overlay_Render_Rect(), cellptr->Overlay_Shadow_Render_Rect()) - point);
				cellptr->Overlay = OVERLAY_NONE;
				cellptr->OverlayData = 0;
			}
		}
	}

	IsInLimbo = true;
	IsDown = false;

	Detach_This_From_All(this);

	Clear_Growth();

	VeinholeMonsters.Delete(this);
	TargetTracker.Remove_Index(Fetch_ID());
}


/// <summary>
/// Finds the veinhole monster that sits at the cell specified.
/// </summary>
/// <returns>Returns with a pointer to the monster found, or NULL if there is none there.</returns>
VeinholeMonsterClass * VeinholeMonsterClass::Get_Monster_At(Cell const & cell)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		if (VeinholeMonsters[i]->CellID == cell) {
			return(VeinholeMonsters[i]);
		}
	}
	return(NULL);
}


/// <summary>
/// Finds the veinhole monster that owns the veins at the cell specified.
/// </summary>
/// <returns>Returns with a pointer to the monster that grew the veins there, or NULL if
/// no monster lays claim to that cell.</returns>
VeinholeMonsterClass * VeinholeMonsterClass::Get_Vein_Owner_At(Cell const & cell)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		if (VeinholeMonsters[i]->GrowthState[Map_Cell_Index(cell)]) {
			return(VeinholeMonsters[i]);
		}
	}
	return(NULL);
}


/// <summary>
/// Handles the per frame logic for every veinhole monster.
/// This routine is called by the main game logic loop. Monsters that have finished
/// dying are disposed of afterward.
/// </summary>
void VeinholeMonsterClass::Update_All(void)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		VeinholeMonsters[i]->AI();
	}
	Remove_Dead();
}


/// <summary>
/// Handles the per frame logic for this veinhole monster.
/// The monster dozes until something wanders close by, then rouses itself and starts
/// belching gas clouds at the intruder until it is left alone again. This routine also
/// drives the growth and the withering of the monster's patch of veins.
/// </summary>
/// <remarks>Only call this routine once per veinhole monster per game logic loop.</remarks>
void VeinholeMonsterClass::AI(void)
{
	/// Frames are stored biased by one so that a zero entry means no frame.
	static int _state_to_mouth_frame[STATE_COUNT][FRAME_COUNT + 1] = {{1, 7, 6, 5, 4, 3, 2, 1, 0, 0, 0, 0, 0}, {4, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, {1, 8, 9, 10, 11, 11, 11, 11, 11, 10, 9, 8, 1}, {12, 11, 10, 9, 8, 1, 0, 0, 0, 0, 0, 0, 0}};
	static int _state_transition_stage[STATE_COUNT] = {-1, 4, -1, -1};                                                          /// Check if we should transition to a different state as this stage
	static int _state_loop_stage[STATE_COUNT] = {8, 4, 13, 6};                                                                  /// Loop around at this stage
	static int _state_entry_step[STATE_COUNT] = {0, -1, -1, -1};                                                                /// Start the state at this stage
	static int _state_entry_stage[STATE_COUNT] = {-1, 3, 12, 5};                                                                /// Start the state with this step
	static bool _unused_bools[FRAME_COUNT] = {true, true, false, false, false, false, false, true, false, false, false, true};  /// "Mouth is closed" perhaps

	Control.Graphic_Logic();

	switch (CurrentState) {
	case IDLE:
		if (LogicTimer != 0) {
			if (DesiredState != DYING) {
				DesiredState = ATTACKING;
			}
		} else {
			if (DesiredState != DYING) {
				if (Map.Is_Something_Nearby(CellID, 2)) {
					DesiredState = ALERT;
				} else if (Control.Fetch_Stage() == _state_transition_stage[CurrentState]) {
					if (Probability_Of(0.004)) {
						Control.Set_Stage(_state_loop_stage[CurrentState] - 1);
						Control.Set_Step(-1);
					}
				}
			}
		}
		break;

	case ALERT:
		if (LogicTimer == 0 && DesiredState != DYING) {
			if (!Map.Is_Something_Nearby(CellID, 2) && DesiredState == ALERT) {
				DesiredState = IDLE;
				Control.Set_Step(1);
				if (Control.Fetch_Stage() < 0) {
					Control.Set_Stage(0);
				}
			}
		} else if (DesiredState != DYING) {
			DesiredState = ATTACKING;
			Control.Set_Step(1);
			if (Control.Fetch_Stage() < 0) {
				Control.Set_Stage(0);
			}
		} else {
			Control.Set_Step(1);
			if (Control.Fetch_Stage() < 0) {
				Control.Set_Stage(0);
			}
		}
		break;

	case ATTACKING:
		if (Control.Fetch_Stage() >= 0 && _state_to_mouth_frame[CurrentState][Control.Fetch_Stage()] == 11) {
			if (!IsToPuffGas) {
				Coord coord = Coord(CellID, Map.Get_Height_GL(CellID) + 400);
				ParticleClass * particle = GasSystem->Spawn_Particle(ParticleTypes[ParticleTypeClass::From_Name("GasCloudM1")], coord);
				particle->GasDrift.Z = 12;
				double angle = Random_Double(0.0, 1.0) * DEG_TO_RAD(360);
				particle->GasDrift.X = std::cos(angle) * 8;
				particle->GasDrift.Y = std::sin(angle) * 8;
				IsToPuffGas = true;
			}
		} else {
			IsToPuffGas = false;
		}
		if (LogicTimer != 0 || IsToPuffGas) {
			if (Control.Fetch_Stage() == _state_transition_stage[CurrentState]) {
				if (Random_Double(0.0, 1.0) < 0.1) {
					Control.Set_Stage(_state_loop_stage[CurrentState] - 1);
					Control.Set_Step(-1);
				}
			}
		} else {
			if (DesiredState != DYING) {
				DesiredState = IDLE;
			}
			Control.Set_Step(-1);
		}
		break;

	case DYING:
		if (Control.Fetch_Stage() == _state_transition_stage[CurrentState] && !IsDead) {
			Destroy_Monster();
		}
		break;
	}

	if (GrowthTimer == 0) {
		if (!IsDead) {
			Grow();
			GrowthTimer = Random_Pick(0, Rule->VeinholeGrowthRate / 2) + Rule->VeinholeGrowthRate;
		} else {
			Shrink();
			GrowthTimer = Random_Pick(0, Rule->VeinholeShrinkRate / 2) + Rule->VeinholeShrinkRate;
		}
	}

	if (!IsDead) {
		if (Control.Fetch_Stage() == _state_transition_stage[CurrentState]) {
			if (DesiredState != CurrentState) {
				CurrentState = DesiredState;
				Control.Set_Step(_state_entry_step[CurrentState]);
				Control.Set_Stage(_state_entry_stage[CurrentState]);
			} else {
				Control.Set_Step(0);
			}
		} else if (Control.Fetch_Stage() != -1) {
			ShapeFrame = _state_to_mouth_frame[CurrentState][Control.Fetch_Stage()] - 1;
		} else {
			Control.Set_Step(0);
		}
	}
}


/// <summary>
/// Draws every veinhole monster on the map.
/// </summary>
void VeinholeMonsterClass::Draw_All(void)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		VeinholeMonsters[i]->Draw_It();
	}
}


/// <summary>
/// Fetches the veinhole monster artwork for the theater specified.
/// This routine is called whenever the theater changes so that the monster is drawn
/// with artwork that suits the terrain around it.
/// </summary>
/// <param name="theater">The theater to load the monster shape for.</param>
void VeinholeMonsterClass::Init(TheaterType theater)
{
	char buffer[32];
	sprintf(buffer, "VEINHOLE.%s", TheaterClass::As_Reference(theater).Suffix.c_str());
	MonsterShape = (ShapeSet *)MixFileClass::Retrieve(buffer);
}


/// <summary>
/// Draws this veinhole monster onto the tactical map.
/// The monster is drawn with whichever mouth frame its logic has settled on, and it
/// registers itself as a selectable object so that the player can click on it. A
/// monster hidden under fog keeps its mouth shut.
/// </summary>
void VeinholeMonsterClass::Draw_It(void)
{
	if (!IsDead) {
		ShapeSet const * shape = (ShapeSet const *)MonsterShape;
		int frame = ShapeFrame;

		Coord coord = CellID.As_Coord();
		if (Map[coord].IsFogged) {
			frame = 0;
		}

		Point2D drawpoint;
		TacticalMap->Coord_To_Pixel(coord, drawpoint);
		int height = LEVEL_PIXEL_H_1 * Map[CellID].Height;
		drawpoint.Y += -49 - height;
		TacticalMap->Add_To_Selectables(this, Point2D(drawpoint.X, TacticalRect.Y + drawpoint.Y + 49));
		ConvertClass * drawer = ColorSchemes[PlayerPtr->Scheme]->Converter;
		Draw_Shape(*LogicalSurface, *drawer, shape, frame, drawpoint, TacticalRect, ShapeFlags_Type(SHAPE_CENTER|SHAPE_WIN_REL|SHAPE_ALPHA|SHAPE_ZGRAD), 0, -2 - LEVEL_PIXEL_H_1 - height, ZGRAD_GROUND, Map[coord].TileBrightness);
	}
}


/// <summary>
/// Applies damage to this veinhole monster.
/// A monster that survives being hurt rouses itself and starts belching gas at whatever
/// disturbed it. One that does not begins its death throes and is detached from
/// everything that was tracking it.
/// </summary>
/// <param name="damage">The damage to inflict; adjusted to the amount actually taken.</param>
/// <param name="forced">Should the damage be applied regardless of the usual immunities?</param>
/// <returns>Returns with the result of the damage, such as RESULT_DESTROYED.</returns>
ResultType VeinholeMonsterClass::Take_Damage(int & damage, int distance, WarheadTypeClass const * warhead, TechnoClass * source, bool forced, bool no_crew)
{
	ResultType result = BASECLASS::Take_Damage(damage, distance, warhead, source, forced, no_crew);

	switch (result) {
		case RESULT_NONE:
			break;

		case RESULT_DESTROYED:
			DesiredState = DYING;
			Detach_This_From_All(this, false);
			break;

		case RESULT_ALREADY_DESTROYED:
			return(RESULT_ALREADY_DESTROYED);

		default:
			DesiredState = ATTACKING;
			LogicTimer = 8 * TICKS_PER_SECOND;
			break;
	}
	return(result);
}


/// <summary>
/// Can a veinhole monster be placed at the cell specified?
/// A monster demands a flat patch of clear ground to itself and must keep its distance
/// from any other veinhole. The check is waived while the scenario is still being set
/// up, since the map author is trusted to have placed them sensibly.
/// </summary>
/// <param name="cell">The cell to consider as the center of the monster.</param>
/// <returns>bool; Can the monster go here?</returns>
bool VeinholeMonsterClass::Can_Monster_Go_Here(Cell const & cell)
{
	if (ScenarioInit > 0) {
		return(true);
	}

	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		VeinholeMonsterClass * monster = VeinholeMonsters[i];
		if (abs(cell.X - monster->CellID.X) < 3 && abs(cell.Y - monster->CellID.Y) < 3) {
			return(false);
		}
	}

	int cell_height = Map[cell].Height;
	if (cell_height < 1) {
		return(false);
	}

	for (int y = -1; y <= 1; y++) {
		for (int x = -1; x <= 1; x++) {
			CellClass & cptr = Map[cell + Cell(x, y)];
			if (cptr.Ramp != RAMP_NONE || cptr.Land_Type() != LAND_CLEAR || cptr.Height != cell_height) {
				return(false);
			}
		}
	}

	return(true);
}


/// <summary>
/// Destroys every veinhole monster in the game.
/// This routine is used when the scenario is being torn down, or when a saved game is
/// about to be loaded over the top of it.
/// </summary>
void VeinholeMonsterClass::Reset(void)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		delete VeinholeMonsters[i];
	}
	VeinholeMonsters.Clear();
}


/// <summary>
/// Spreads this monster's vein patch by a few cells.
/// This routine is called by the monster's own logic whenever its growth timer expires.
/// Growth stops once the monster has reached the vein limit imposed by the rules, or if
/// vein growth has been switched off for the scenario.
/// </summary>
void VeinholeMonsterClass::Grow(void)
{
	static const int _mod = 5;

	if (GrowthCount <= Rule->MaxVeinholeGrowth - 40 && VeinCount <= Rule->MaxVeinholeGrowth - 100 && Scen->IsVeinGrowth) {

		int index = 0;
		int count = (abs(Scen->RandomNumber) % _mod) + 1;

		std::optional<CellNode> node = GrowthQueue.Extract_Min();

		while (index < count && node) {
			CellClass & cell = Map[node->Element];

			if (cell.OverlayData < OVERLAYDATA_FIRST_SOLID_VEIN) {
				cell.Place_Veins();
				VeinCount++;
			}

			if (cell.Overlay == OVERLAY_VEINS) {
				int cindex = Map_Cell_Index(node->Element);
				if (cindex >= 0 && cindex < Map_Cell_Count()) {
					GrowthState[cindex] = true;
				}
			}

			if (cell.OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {
				/// Not Facing_Add -- its mask would wrap the last facing and never end.
				for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; facing = (FacingType)(facing + FACING_90)) {
					Cell adjacent = Adjacent_Cell(node->Element, facing);
					if (Map.In_Local_Radar(adjacent)) {
						CellClass & adj_cell = Map[adjacent];
						if (abs(adj_cell.Height - cell.Height) < 2) {
							int cindex = Map_Cell_Index(adjacent);
							if (cindex >= 0 && cindex < Map_Cell_Count()) {
								if (adj_cell.Can_Place_Veins() && !GlobalGrowthState[cindex] && GrowthCount < Rule->MaxVeinholeGrowth) {
									float score = float(Frame / 50 + abs(Scen->RandomNumber() % 50) + 1);
									GlobalGrowthState[cindex] = true;
									GrowthQueue.Insert(CellNode(adjacent, score));
									GrowthCount++;
								}
								GrowthState[cindex] = true;
							}
						}
					}
				}
			}

			index++;
			if (index < count) {
				node = GrowthQueue.Extract_Min();
			}
		}
	}
}


/// <summary>
/// Withers a few cells of this dying monster's vein patch.
/// This routine is called by the monster's own logic whenever its shrink timer expires.
/// </summary>
void VeinholeMonsterClass::Shrink(void)
{
	static const int _mod = 4;

	int index = 0;
	int count = abs(Scen->RandomNumber) % _mod + 1;
	std::optional<CellNode> node = GrowthQueue.Extract_Min();

	while (index < count && node) {
		Reduce_Veins_At(&Map[Map[node->Element].CellID]);
		index++;
		if (index < count) {
			node = GrowthQueue.Extract_Min();
		}
	}
}


/// <summary>
/// Prepares the vein growth system for play.
/// This routine is called once the map and its veinholes are in place. Every monster is
/// given the records it needs to tend its own patch, and any vein on the map that no
/// monster ends up claiming is quietly removed.
/// </summary>
/// <param name="clear">Should the existing growth records be thrown away and rebuilt?</param>
void VeinholeMonsterClass::Init_Vein_Growth_System(bool clear)
{
	int i;
	int cell_count = Map_Cell_Count();

	if (clear) {
		Deinit_Vein_Growth_System();

		for (i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
			VeinholeMonsters[i]->GrowthQueue.Reserve(Rule->MaxVeinholeGrowth);
			VeinholeMonsters[i]->GrowthState = new bool[cell_count];
			memset(VeinholeMonsters[i]->GrowthState, 0, cell_count);
			VeinholeMonsters[i]->GrowthQueue.Clear();
		}
	}

	if (GlobalGrowthState == NULL) {
		GlobalGrowthState = new bool[cell_count];
	}

	Map.Reset_Iterator();
	CellClass * iter = Map.Iterate();
	while (iter != NULL) {
		iter->IsToGrowVeins = false;
		iter = Map.Iterate();
	}

	memset(GlobalGrowthState, 0, cell_count);

	for (i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		if (VeinholeMonsters[i]->IsDead) {
			VeinholeMonsters[i]->Build_Shrinking_Queue();
		} else {
			VeinholeMonsters[i]->Build_Growth_Queue();
		}
	}

	Map.Reset_Iterator();
	iter = Map.Iterate();
	while (iter != NULL) {
		if (iter->Overlay == OVERLAY_VEINS && !GlobalGrowthState[Map_Cell_Index(iter->CellID)]) {
			iter->Overlay = OVERLAY_NONE;
		}
		iter->IsToGrowVeins = false;
		iter = Map.Iterate();
	}
}


/// <summary>
/// Frees the vein growth records of every veinhole monster.
/// </summary>
void VeinholeMonsterClass::Deinit_Vein_Growth_System(void)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		VeinholeMonsters[i]->Clear_Growth();
	}
	Clear_Global_Data();
}


/// <summary>
/// Builds the queue of cells this monster can grow its veins from.
/// This routine spreads outward from the veinhole across the connected patch of veins,
/// claiming those cells for this monster and queueing the ones that still have room to
/// thicken or push further out.
/// </summary>
void VeinholeMonsterClass::Build_Growth_Queue(void)
{
	DynamicVectorClass<Cell> cells;
	cells.Set_Growth_Step(5000);

	GrowthCount = 0;
	VeinCount = 0;
	GrowthQueue.Clear();

	for (int i = Map_Cell_Count() - 1; i >= 0; i--) {
		GrowthState[i] = false;
	}

	for (int y = -2; y <= 2; y++) {
		for (int x = -2; x <= 2; x++) {
			CellClass * cellptr = &Map[CellID + Cell(x, y)];
			if (cellptr->Overlay == OVERLAY_VEINS) {
				int index = Map_Cell_Index(cellptr->CellID);
				cells.Add(CellID + Cell(x, y));
				if (index >= 0 && index < Map_Cell_Count()) {
					GlobalGrowthState[index] = true;
					GrowthState[index] = true;
					cellptr->IsToGrowVeins = true;
					if (cellptr->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN && cellptr->OverlayData <= OVERLAYDATA_FIRST_SOLID_VEIN + 3) {
						VeinCount++;
					} else {
						if (GrowthCount < Rule->MaxVeinholeGrowth) {
							GrowthQueue.Insert(CellNode(cellptr->CellID, 0.0f));
							GrowthCount++;
						}
					}
				}
			}
		}
	}

	while (cells.Count() > 0) {
		int last = cells.Count() - 1;
		CellClass * cellptr = &Map[cells[last]];
		cells.Delete_Index(last);

		for (int facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
			CellClass * adjacent = &cellptr->Adjacent_Cell((FacingType)facing);
			int index = Map_Cell_Index(adjacent->CellID);
			if (adjacent->Overlay == OVERLAY_VEINS && !adjacent->IsToGrowVeins) {
				if ((adjacent->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN && adjacent->OverlayData < (OVERLAYDATA_FIRST_SOLID_VEIN + 5)) || !adjacent->Can_Place_Veins()) {
					cells.Add(adjacent->CellID);
					VeinCount++;
				} else {
					if (GrowthCount < Rule->MaxVeinholeGrowth) {
						GrowthQueue.Insert(CellNode(adjacent->CellID, 0.0f));
						GrowthCount++;
						if (adjacent->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {
							cells.Add(adjacent->CellID);
						}
					}
				}
				adjacent->IsToGrowVeins = true;
				if (index >= 0 && index < Map_Cell_Count()) {
					GlobalGrowthState[index] = true;
					GrowthState[index] = true;
				}
				VeinCount++;
			}
		}
	}

}


/// <summary>
/// Builds the queue of vein cells for this dying monster to wither away.
/// This routine is used once a veinhole has been destroyed. Only the mature vein cells
/// this monster is recorded as owning are gathered, and the ones farthest from the
/// veinhole are the first to go.
/// </summary>
void VeinholeMonsterClass::Build_Shrinking_Queue(void)
{
	GrowthCount = 0;
	GrowthQueue.Clear();
	VeinCount = 0;

	Map.Reset_Iterator();
	CellClass * iter = Map.Iterate();
	while (iter != NULL) {
		if (iter->Overlay == OVERLAY_VEINS && iter->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {
			int index = Map_Cell_Index(iter->CellID);
			if (index >= 0 && index < Map_Cell_Count() && GrowthState[index]) {
				if (GrowthCount < Rule->MaxVeinholeGrowth) {
					float score = 1000 - Point2D(iter->CellID.X, iter->CellID.Y).Distance_To(Point2D(CellID.X, CellID.Y));
					GrowthQueue.Insert(CellNode(iter->CellID, score));
					GrowthCount++;
					VeinCount++;
				}
			}
		}
		iter = Map.Iterate();
	}
}


/// <summary>
/// Frees the growth state that is shared between all the veinhole monsters.
/// </summary>
void VeinholeMonsterClass::Clear_Global_Data(void)
{
	if (GlobalGrowthState) {
		delete [] GlobalGrowthState;
		GlobalGrowthState = NULL;
	}
}


/// <summary>
/// Frees the vein growth records held by this monster.
/// </summary>
void VeinholeMonsterClass::Clear_Growth(void)
{
	GrowthQueue.Clear();

	if (GrowthState) {
		delete [] GrowthState;
		GrowthState = NULL;
	}

	GrowthCount = 0;
}


/// <summary>
/// Kills this veinhole monster and leaves a patch of veins behind.
/// The cells the monster occupied are turned over to plain vein overlay, the map zones
/// are brought up to date, and a shrinking queue is built so that the leftover patch
/// will wither away over time.
/// </summary>
void VeinholeMonsterClass::Destroy_Monster(void)
{
	CellClass & cptr = Map[CellID];
	cptr.Overlay = OVERLAY_VEINS;
	cptr.OverlayData = 0;
	cptr.Recalc_Attributes();

	for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
		CellClass & adj = cptr.Adjacent_Cell(dir);
		if (dir % 2 != 0) {
			adj.Overlay = OVERLAY_NONE;
			adj.OverlayData = 0;
		} else {
			adj.Overlay = OVERLAY_VEINS;
			adj.OverlayData = 0;
		}
	}

	for (int y = -2; y <= 2; y++) {
		for (int x = -2; x <= 2; x++) {
			CellClass & cptr2 = Map[Cell(x, y) + CellID];
			cptr2.Place_Veins();
			cptr2.Recalc_Attributes();
		}
	}

	Build_Shrinking_Queue();
	Map.Update_Cell_Zone(CellID);
	Map.Update_Cell_Subzones(CellID);
	IsDead = true;
}


/// <summary>
/// Disposes of any veinhole monster that has finished dying.
/// A destroyed monster lingers on so that its patch of veins can wither away, so this
/// routine only deletes one once it has no shrinking work left to do.
/// </summary>
void VeinholeMonsterClass::Remove_Dead(void)
{
	for (int i = VeinholeMonsters.Count() - 1; i >= 0; i--) {
		if (VeinholeMonsters[i]->IsDead && VeinholeMonsters[i]->GrowthQueue.Count() == 0) {
			delete VeinholeMonsters[i];
		}
	}
}


/// <summary>
/// Loads every veinhole monster from the save game stream.
/// This routine is called by the scenario load process. Any monsters presently in
/// existence are destroyed first, then each saved monster is recreated with its vein
/// growth records and handed to the swizzler and the target tracker.
/// </summary>
/// <returns>bool; Were all the monsters read successfully?</returns>
bool VeinholeMonsterClass::Load_All(SaveStreamClass & stream)
{
	Reset();

	int cell_count = Map_Cell_Count();

	int monster_count;
	stream.Serialize(monster_count);
	if (stream.Was_Error()) {
		return(false);
	}

	GlobalGrowthState = new bool[cell_count];
	stream.Serialize_Bytes(GlobalGrowthState, (int)(cell_count));
	if (stream.Was_Error()) {
		return(false);
	}

	for (int i = 0; i < monster_count; i++) {

		/*
		 * The constructor allocates this monster's vein records and adds it to the list;
		 * the members that describe its state arrive from the stream afterwards.
		 */
		VeinholeMonsterClass * monster = new VeinholeMonsterClass();

		SwizzleIDType id;
		stream.Serialize(id);
		if (stream.Was_Error()) {
			return(false);
		}

		Swizzler.Here_I_Am(id, monster);

		stream.Set_Context(typeid(*monster).name(), id);
		monster->Serialize(stream);
		if (stream.Was_Error()) {
			return(false);
		}

		stream.Serialize_Bytes(monster->GrowthState, (int)(cell_count));
		if (stream.Was_Error()) {
			return(false);
		}

		TargetTracker.Add_Index(monster->Fetch_ID(), monster);
	}

	return(true);
}


/// <summary>
/// Lists the members this veinhole monster carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void VeinholeMonsterClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(GrowthCount);
	GrowthQueue.Serialize(stream);
	stream.Serialize(GrowthTimer);
	// GrowthState -- one flag per map cell, carried beside this record by Load_All and Save_All.
	stream.Serialize(CurrentState);
	stream.Serialize(DesiredState);
	stream.Serialize(Control);
	stream.Serialize(LogicTimer);
	stream.Serialize(CellID);
	stream.Serialize(ShapeFrame);
	stream.Serialize(IsDead);
	stream.Serialize(IsToPuffGas);
	stream.Serialize(VeinCount);
}


/// <summary>
/// Saves every veinhole monster to the save game stream.
/// This routine is called by the scenario save process. Each monster is written along
/// with its vein growth records so that growth can pick up where it left off.
/// </summary>
/// <returns>bool; Were all the monsters written successfully?</returns>
bool VeinholeMonsterClass::Save_All(SaveStreamClass & stream)
{
	int monster_count = VeinholeMonsters.Count();
	stream.Serialize(monster_count);
	if (stream.Was_Error()) {
		return(false);
	}

	int cell_count = Map_Cell_Count();
	stream.Serialize_Bytes(GlobalGrowthState, (int)(cell_count));
	if (stream.Was_Error()) {
		return(false);
	}

	for (int i = 0; i < monster_count; i++) {
		SwizzleIDType id = Swizzler.ID_Of(VeinholeMonsters[i]);
		stream.Serialize(id);
		if (stream.Was_Error()) {
			return(false);
		}

		VeinholeMonsters[i]->Serialize(stream);
		if (stream.Was_Error()) {
			return(false);
		}

		stream.Serialize_Bytes(VeinholeMonsters[i]->GrowthState, (int)(cell_count));
		if (stream.Was_Error()) {
			return(false);
		}

	}

	return(true);
}


/// <summary>
/// Fetches the object type class for this veinhole monster.
/// </summary>
/// <returns>Returns with a pointer to the veinhole type as specified by the rules.</returns>
ObjectTypeClass const * VeinholeMonsterClass::Class_Of(void) const
{
	return(Rule->VeinholeTypeClass);
}


/// <summary>
/// Removes a stage of vein growth from the cell specified.
/// This routine is used by the shrinking logic of a dying veinhole. The neighboring
/// cells are re-examined afterward so that any cell left bare is dropped from the
/// growth records, while a cell that still carries veins is queued for further work.
/// </summary>
/// <param name="cellptr">Pointer to the cell to reduce the veins at.</param>
void VeinholeMonsterClass::Reduce_Veins_At(CellClass * cellptr)
{
	if (cellptr->Overlay == OVERLAY_VEINS && cellptr->OverlayData >= OVERLAYDATA_FIRST_SOLID_VEIN) {

		bool facingbools[4];
		int facing;
		for (facing = FACING_FIRST; facing < FACING_COUNT; facing += FACING_90) {
			CellClass * adjacent = &cellptr->Adjacent_Cell((FacingType)facing);
			facingbools[facing / 2] = (adjacent->Overlay == OVERLAY_VEINS && (adjacent->OverlayData < OVERLAYDATA_FIRST_SOLID_VEIN || adjacent->OverlayData > OVERLAYDATA_FIRST_SOLID_VEIN + 3));
		}

		cellptr->Redraw_Veins();
		VeinCount--;

		for (facing = FACING_FIRST; facing < FACING_COUNT; facing += FACING_90) {
			CellClass * adjacent = &cellptr->Adjacent_Cell((FacingType)facing);
			if (adjacent->Overlay == OVERLAY_NONE) {
				int cindex = Map_Cell_Index(adjacent->CellID);
				if (cindex >= 0 && cindex < Map_Cell_Count()) {
					GrowthState[cindex] = false;
					GlobalGrowthState[cindex] = false;
				}

				if (facingbools[facing / 2] == true && !IsDead) {
					GrowthQueue.Remove_Matching(CellNode(adjacent->CellID));
				}
			}
		}

		OverlayType overlay = cellptr->Overlay;
		Cell & cell = cellptr->CellID;
		int cindex = Map_Cell_Index(cell);
		if (cindex >= 0 && cindex < Map_Cell_Count()) {
			if (overlay == OVERLAY_NONE) {
				GrowthState[cindex] = false;
				GlobalGrowthState[cindex] = false;
			} else if (!IsDead) {
				if (GrowthCount < Rule->MaxVeinholeGrowth) {
					float score = float(Frame / 50 + abs(Scen->RandomNumber() % 50) + 1);
					GrowthQueue.Insert(CellNode(cell, score));
					GrowthCount++;
					GrowthState[cindex] = true;
					GlobalGrowthState[cindex] = true;
				}
			}

		}
	}
}


ClassID VeinholeMonsterClass::Class_ID(void) const
{
	return(ClassID_VeinholeMonsterClass);
}
