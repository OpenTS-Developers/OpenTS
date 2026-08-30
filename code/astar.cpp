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

#include "astar.h"

#include "_astar.h"
#include "_map.h"
#include "cell.h"
#include "dbgprint.h"
#include "foot.h"
#include "globals.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "inline.h"
#include "mouse.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vector.h"

#include "tube.hh"

#include <algorithm>
#include <utility>


/*
**	This is the marker to signify the end of the path list.
*/
#define	END			FACING_NONE

/*
 * EMPTY moves are used to hold the place of eliminated commands.
 */
#define	EMPTY		(FacingType)-2

/*
 * A TUNNEL step is not a direction at all -- it marks the point in the path where
 * the unit passes through a tunnel, so it takes the value just past the last
 * real facing.
 */
#define	TUNNEL		FACING_COUNT



/*=========================================================================*/
/* Define a couple of variables which are private to the module they are   */
/*      declared in.                                                       */
/*=========================================================================*/
static unsigned int CellHeights[2000];

unsigned int MapCellStride;

int AStarFacingToOffset[FACING_COUNT];


/// <summary>
/// Fetches the cell that lies one step away in the given direction.
/// This routine understands the tunnel pseudo facing, which steps straight through to the
/// far mouth of the tunnel rather than to a neighboring cell.
/// </summary>
/// <param name="facing">The direction to step in, or TUNNEL to pass through a tunnel.</param>
/// <returns>Returns with the cell stepped into. If a tunnel step is asked for at a cell that
/// has no tunnel, Cell(0,0) is returned.</returns>
inline Cell Next_Cell(Cell const & cell, FacingType facing)
{
	if (facing == TUNNEL) {
		TubeType tube = (TubeType)Map[cell].Tube;
		if (tube != TUBE_NONE) {
			return(Cell(Tubes[tube]->Exit));
		}
		return(Cell(0,0));
	}
	/// Raw index on purpose. The tunnel case is handled above, so the facing is known good
	/// here; routing this through an Adjacent_Cell helper would add a redundant mask.
	return(cell + (AdjacentCell[facing]));
}


/// <summary>
/// Fetches the cell arrived at after following a run of moves.
/// This routine is used by the corner straightening code to work out where a stretch of the
/// move list actually leads. Tunnel moves are followed through to their far mouth.
/// </summary>
/// <param name="count">The number of moves to follow.</param>
/// <returns>Returns with the cell reached at the end of the walk.</returns>
Cell Follow_Path(Cell const & cell, int count, FacingType const * path)
{
	Cell result = cell;
	for (int i = 0; i < count; i++) {
		result = Next_Cell(result, path[i]);
	}
	return(result);
}

/// <summary>
/// Fetches the cell arrived at after following a run of moves.
/// </summary>
/// <param name="count">The number of moves to follow.</param>
/// <returns>Returns with the cell reached at the end of the walk.</returns>
static Cell Follow_Path2(Cell const & cell, int count, FacingType const * path)
{
	Cell result = cell;
	for (int i = 0; i < count; i++) {
		result = Next_Cell(result, path[i]);
	}
	return(result);
}


/// <summary>
/// Determines the cost of stepping from one cell into the next.
/// The base cost comes from why the destination cell is passable or not. Cells that another
/// object is already about to walk through are priced up, traffic that is itself moving out
/// of the way costs less than a hard blockage, and bridges are made expensive when the
/// search has been asked to keep off them.
/// </summary>
/// <param name="from">The map array slot of the cell being left.</param>
/// <param name="to">The map array slot of the cell being entered.</param>
/// <param name="bridge">Is the step taking place on a bridge deck?</param>
/// <param name="move">The verdict of the object's cell entry test.</param>
/// <returns>Returns with the cost of the step, in nominal cell units.</returns>
double AStarClass::Get_Movement_Cost(CellClass **from, CellClass **to, bool bridge, MoveType move, FootClass * foot)
{
	static float _costs[MOVE_COUNT] = {
		1.0,		// MOVE_OK
		1000.0,		// MOVE_CLOAK
		1.0,		// MOVE_MOVING_BLOCK
		1.0,		/// MOVE_CLOSED_GATE
		60.0,		/// MOVE_FRIENDLY_DESTROYABLE
		20.0,		// MOVE_DESTROYABLE
		8.0,		// MOVE_TEMP
		10000.0		// MOVE_NO
	};

	CellClass * from_cell = from[0];
	CellClass * to_cell = to[0];

	float cost = _costs[move];

	FootClass *next_occupier;
	FootClass * occupier;
	CellClass * cellptr = to_cell;

	bool on_bridge = bridge;

	if (move == MOVE_MOVING_BLOCK) {
		occupier = (FootClass *)cellptr->Cell_Occupier(on_bridge);
		bool clear = false;
		int depth = 0;
		if (Avoidance == AVOIDANCE_NONE) {
			while (depth < 10) {
				if (occupier == NULL) {
					clear = true;
					break;
				}

				if (!occupier->Is_Foot()) {
					clear = false;
					break;
				}

				FacingType facing;

				if (occupier->Speed == 0.0) {
					facing = occupier->Path[0];
					if (facing == FACING_NONE) {
						clear = true;
						break;
					}
				} else {
					facing = occupier->PrimaryFacing.Current().As_Dir8();
				}

				cellptr = &Map[::Adjacent_Cell(occupier->Get_Cell(), facing)];
				on_bridge = cellptr->IsUnderBridge && (occupier->IsOnBridge || occupier->Get_Cell_Ptr()->Height - cellptr->Height > 2);
				next_occupier = (FootClass *)cellptr->Cell_Occupier(on_bridge);
				occupier = (FootClass *)next_occupier;
				depth++;
			}
		}

		if (!clear || Avoidance != AVOIDANCE_NONE) {
			cost = 4.0f;
		}
		if (Avoidance == AVOIDANCE_HARD) {
			cost = 1000.0f;
		}
	}

	if (to_cell->IsPredictedPath) {
		cost = cost * 4.0f;
	}

	if (bridge && IsAvoidBridges) {

		static const int _bridge_cell_offset1[FACING_COUNT] =  { -2,    -2,     0,     1, 1,   1,   0,  -2 };
		static const int _bridge_cell_offset2[FACING_COUNT] =  {  0, -2 * MAP_CELL_W, -2 * MAP_CELL_W, -2 * MAP_CELL_W, 0, MAP_CELL_W, MAP_CELL_W, MAP_CELL_W };

		static const FacingType _bridge_facings[3][3] = {
			{ FACING_NW, FACING_N, FACING_NE },
			{ FACING_W, FACING_NONE, FACING_E },
			{ FACING_SW, FACING_S, FACING_SE }
		};

		float multiplier = 1.0f;

		Cell step(to_cell->CellID - from_cell->CellID);

		unsigned int dir = _bridge_facings[step.Y + 1][step.X + 1];

		CellClass *cell1;
		CellClass *cell2;
		int cell2_offset;

		if (to_cell->IsBridgeEastWest) {
			cell2_offset = _bridge_cell_offset2[(dir - FACING_S) % FACING_COUNT];
			cell1 = to[_bridge_cell_offset2[dir]];
			cell2 = to[cell2_offset];
		} else {
			cell2_offset = _bridge_cell_offset1[(dir - FACING_S) % FACING_COUNT];
			cell1 = to[_bridge_cell_offset1[dir]];
			cell2 = to[cell2_offset];
		}

		if (!cell1->IsUnderBridge) {
			multiplier = 10.0f;
		} else if (cell2->IsUnderBridge) {
			multiplier = 2.0f;
		}

		return(cost * multiplier);

	}
	return(cost);
}


/// <summary>
/// Finds a path between two cells at the individual cell level.
/// This is the real A* search. Cells are expanded outward from the start until the
/// destination is reached or the effort allowance runs out, and the resulting move list is
/// then straightened and optimized. When the hierarchical search has already run, the
/// expansion is confined to the corridor it marked out.
/// </summary>
/// <param name="moves">Buffer that receives the facing commands making up the path.</param>
/// <param name="max_loops">The most cells the search may expand, or a negative value to let
/// it run until the queue is exhausted.</param>
/// <param name="with_hs">Should the search be confined to the hierarchical corridor?</param>
/// <returns>Returns with a pointer to the path found, or NULL if there is no route.</returns>
PathStruct * AStarClass::Find_Path_Regular(Cell const & from, Cell const & to, FootClass *foot, FacingType *moves, int max_loops, bool with_hs)
{
	FacingType face;

	static float _facing_costs[FACING_COUNT] = {
		0.001f, /// FACING_N
		0.005f, /// FACING_NE
		0.002f, /// FACING_E
		0.006f, /// FACING_SE
		0.003f, /// FACING_S
		0.007f, /// FACING_SW
		0.004f, /// FACING_W
		0.008f, /// FACING_NW
	};

	int tries = 0;

	CellClass ** to_pptr = &Map.Array[MAP_CELL_W * to.Y + to.X];
	CellClass ** from_pptr = &Map.Array[MAP_CELL_W * from.Y + from.X];

	CellClass * to_ptr = *to_pptr;
	CellClass * from_ptr = *from_pptr;

	if (!from_ptr || !to_ptr) {
		return(NULL);
	}

	DestCellHeight = (foot->RTTI == RTTI_AIRCRAFT || !to_ptr->IsUnderBridge) ? to_ptr->Height : to_ptr->Height + BRIDGE_CELL_HEIGHT;

	if (foot->RTTI == RTTI_AIRCRAFT || !foot->IsOnBridge) {
		CurrentCellHeight = from_ptr->Height;
	} else {
		CurrentCellHeight = from_ptr->Height + BRIDGE_CELL_HEIGHT;
	}

	if (foot->TClass->IsTrain) {
		if (from_ptr->IsUnderBridge) {
			if (abs(foot->PositionCoord.Z / LEVEL_LEPTON_H - CurrentCellHeight) > 2) {
				CurrentCellHeight += BRIDGE_CELL_HEIGHT;
			}
		}
	}

	ObjectSpeed = foot->TClass->Speed;
	HierNodeIndex = 0;
	HierLastNodeCell = from;
	int * fine_final_ids = HierOnPath[0];
	RegularOpenNode * working_node = Create_Node(0, from_pptr, to, 0.0);

	if (from == to && CurrentCellHeight == DestCellHeight) {
		return(NULL);
	}

	if (Avoidance != AVOIDANCE_NONE) {
		Apply_Path_Collision_Avoidance(foot);
	}
	int start_index = from_ptr->CellID.X + MapCellStride * from_ptr->CellID.Y;
	if (CurrentCellHeight > from_ptr->Height) {
		RegularBridgeVisited[start_index] = UniqueID;
		RegularBridgeMovementCosts[start_index] = 0;
	} else {
		RegularVisited[start_index] = UniqueID;
		RegularMovementCosts[start_index] = 0;
	}

	static const int _offsets[FACING_COUNT] = {
		-MAP_CELL_W,
		-MAP_CELL_W + 1,
		1,
		MAP_CELL_W + 1,
		MAP_CELL_W,
		MAP_CELL_W - 1,
		-1,
		-MAP_CELL_W - 1
	};

	bool is_train = false;
	if (foot->TClass->IsTrain) {
		is_train = true;
		for (face = FACING_FIRST; face <= FACING_COUNT; face++) {
			int facing_diff = foot->PrimaryFacing.Current().As_Dir8() - face;
			int abs_diff = abs(facing_diff);
			if (abs_diff > FACING_E && abs_diff < FACING_W && face != FACING_COUNT) {
				CellClass * neighbor = from_pptr[_offsets[face]];
				int neighbor_index = neighbor->CellID.X + MapCellStride * neighbor->CellID.Y;
				if (CurrentCellHeight > neighbor->Height + 1) {
					RegularBridgeVisited[neighbor_index] = UniqueID;
					RegularBridgeMovementCosts[neighbor_index] = 0;
				} else {
					RegularVisited[neighbor_index] = UniqueID;
					RegularMovementCosts[neighbor_index] = 0;
				}
			}
		}
	}

	bool is_passive = foot->RTTI == RTTI_UNIT && ((UnitClass *)foot)->Class->IsPassive;

	if (max_loops < 0) {
		max_loops = (USHRT_MAX - 8);
	}

	while (working_node && tries < max_loops) {
		CellClass ** working_from = working_node->Node->CellSlot;
		if (working_from == to_pptr && working_node->Node->CellHeight == DestCellHeight) {
			break;
		}
		Cell & from_id = working_from[0]->CellID;
		RegularOpenNode * temp_node = NULL;
		int from_index = from_id.X + MapCellStride * from_id.Y;

		for (face = FACING_FIRST; face <= FACING_COUNT; face++) {

			CellClass ** neighbor_slot;
			if (face == TUNNEL) {
				int tube = (*working_from)->Tube;
				if (tube != -1) {
					Cell exit_cell = Tubes[tube]->Exit;
					neighbor_slot = &Map.Array[MAP_CELL_W * exit_cell.Y + exit_cell.X];
				} else {
					static CellClass * NoneCell = NULL;
					neighbor_slot = &NoneCell;
				}
			} else {
				neighbor_slot = &working_from[_offsets[face]];
			}

			CellClass * neighbor_cell = *neighbor_slot;
			CellClass ** working_to = neighbor_slot;

			if (neighbor_cell == NULL) {
				continue;
			}

			Cell & neighbor_id = neighbor_cell->CellID;
			int node_index;
			if (face != TUNNEL) {
				node_index = from_index + AStarFacingToOffset[face];
			} else {
				node_index = neighbor_cell->CellID.X + MapCellStride * neighbor_cell->CellID.Y;
			}

			bool base_level = !neighbor_cell->IsUnderBridge || abs(CurrentCellHeight - neighbor_cell->Height) <= 1;

			int zone_index = Map.Get_Cell_Zone_Index(neighbor_id);
			CellSubzoneStruct * subzones = Map.CellSubzones;

			short subzone_id = subzones[zone_index].SubzoneID[SUBZONE_FINE];
			if (fine_final_ids[subzone_id] != UniqueID && base_level && !neighbor_cell->AdjacentObjectCount && with_hs) {
				continue;
			}

			if (base_level && Is_Visited(0, true, node_index) && (working_node->MovementCost + 1.009) > RegularMovementCosts[node_index]) {
				continue;
			}

			if (!base_level && Is_Visited(0, false, node_index) && (working_node->MovementCost + 1.009) > RegularBridgeMovementCosts[node_index]) {
				continue;
			}
			MoveType move = foot->Can_Enter_Cell(neighbor_cell, face, CurrentCellHeight, *working_from, UseLocomotorEnterCheck);
			if (is_train && move < MOVE_NO) {
				move = MOVE_OK;
			}

			float movement_cost;
			if (face != TUNNEL) {
				movement_cost = Get_Movement_Cost(working_from, working_to, base_level == 0, move, foot) * MovementCostMultiplier + _facing_costs[face];
			} else {
				CellClass & from_cell = *working_from[0];
				CellClass & to_cell = *neighbor_cell;
				int dx = ((int)from_cell.CellID.X - (int)to_cell.CellID.X);
				int dy = ((int)from_cell.CellID.Y - (int)to_cell.CellID.Y);
				movement_cost = std::max(abs(dy), abs(dx));
			}

			if (move < MOVE_NO) {
				if (Is_Visited(0, base_level, node_index)) {
					continue;
				}

				RegularOpenNode * new_node = Create_Node(working_node, working_to, to, movement_cost);
				if (temp_node == NULL) {
					temp_node = new_node;
				} else {
					if (new_node->Score < temp_node->Score) {
						RegularQueue->Insert(*temp_node);
						temp_node = new_node;
					} else {
						RegularQueue->Insert(*new_node);
					}
				}

				if (base_level) {
					RegularVisited[node_index] = UniqueID;
					RegularMovementCosts[node_index] = new_node->MovementCost;
				} else {
					RegularBridgeVisited[node_index] = UniqueID;
					RegularBridgeMovementCosts[node_index] = new_node->MovementCost;
				}
				if (subzone_id == HierSubzonePath[SUBZONE_FINE][HierNodeIndex + 1]) {
					HierNodeIndex++;
					HierLastNodeCell = neighbor_cell->CellID;
				}
			} else if (working_to == to_pptr && !is_passive) {
				if (abs(CurrentCellHeight - DestCellHeight) <= 1) {
					goto breakout;
				}
			}
		}

		if (temp_node) {
			working_node = RegularQueue->Replace_Root(*temp_node);
		} else {
			working_node = RegularQueue->Extract_Min();
		}

		if (working_node) {
			CurrentCellHeight = working_node->Node->CellHeight;
		}
		tries++;
	}

breakout:

	if (tries == 10000 || working_node == NULL || tries == max_loops || working_node->PathLength < 2) {
		if (Avoidance != AVOIDANCE_NONE) {
			Apply_Path_Collision_Avoidance(foot);
		}
		return(NULL);
	}

	PathStruct * final_path = Build_Final_Path(working_node, moves);
	Cut_Corners(final_path, foot);

	/*
	**	Optimize the move list but only necessary if
	**	diagonal moves are allowed.
	*/
	Optimize_Moves(final_path, foot);

	if (Avoidance != AVOIDANCE_NONE) {
		Apply_Path_Collision_Avoidance(foot);
	}
	return(final_path);
}


/// <summary>
/// Creates a search node for a cell reached from a parent node.
/// The node carries the running movement cost plus an estimate of what remains, and those
/// together decide its priority in the open queue. Cell height is carried over from the
/// parent so that stepping onto and off of bridge decks is tracked.
/// </summary>
/// <param name="parent">The node this cell was reached from, or NULL for the start cell.</param>
/// <param name="cell">The map array slot of the cell the node stands for.</param>
/// <param name="to">The destination cell, used to estimate the remaining cost.</param>
/// <param name="movement_cost">The cost of the step from the parent into this cell.</param>
/// <returns>Returns with a pointer to the new node, taken from the search's node pool.</returns>
AStarClass::RegularOpenNode * AStarClass::Create_Node(RegularOpenNode * parent, CellClass ** cell, Cell const & to, float movement_cost)
{
	RegularOpenNode * open_node = &RegularOpenNodes->Nodes[RegularOpenNodes->ActiveCount++];
	RegularNode * node_data = &RegularNodes->Nodes[RegularNodes->ActiveCount++];
	node_data->CellSlot = cell;

	if (parent != NULL) {
		node_data->Parent = parent->Node;
		CellClass * current_cell = *node_data->CellSlot;
		CellClass * parent_cell = *parent->Node->CellSlot;
		node_data->CellHeight = current_cell->Height;
		RegularNode * parent_node_data = parent->Node;
		if (current_cell->IsUnderBridge) {
			if (parent_cell->IsUnderBridge && (parent_node_data->CellHeight == parent_cell->Height + BRIDGE_CELL_HEIGHT)) {
				node_data->CellHeight += BRIDGE_CELL_HEIGHT;
			} else if (!parent_cell->IsUnderBridge && abs((current_cell->Height - parent->Node->CellHeight) + BRIDGE_CELL_HEIGHT - 1) <= 1) {
				node_data->CellHeight += BRIDGE_CELL_HEIGHT;
			}
		}
	} else {
		node_data->Parent = NULL;
		node_data->CellHeight = CurrentCellHeight;
	}

	open_node->Node = node_data;
	if (parent != NULL) {
		open_node->MovementCost = movement_cost + parent->MovementCost;
		open_node->PathLength = parent->PathLength + 1;
	} else {
		open_node->MovementCost = 0.0;
		open_node->PathLength = 1;
	}

	Cell & cell_id = (*cell)->CellID;
	int dx = abs(cell_id.X - to.X);
	int dy = abs(cell_id.Y - to.Y);

	open_node->Score = open_node->MovementCost + std::sqrt(dx * dx + dy * dy);
	return(open_node);
}


/// <summary>
/// Prepares the pathfinder for a fresh search.
/// The node pools and queues are emptied and the search identifier is bumped, which
/// invalidates every visit stamp at a stroke. Only when that identifier wraps around does
/// the routine have to clear the tables for real.
/// </summary>
void AStarClass::Clear(void)
{
	int i;
	RegularNodes->ActiveCount = 0;
	RegularOpenNodes->ActiveCount = 0;
	RegularQueue->Clear();

	HierQueue->Clear();

	UniqueID++;

	if (UniqueID == 0) {
		for (i = MapCellStride * MapCellStride - 1; i >= 0; i--) {
			RegularVisited[i] = NULL;
			RegularBridgeVisited[i] = NULL;
		}

		for (i = 0; i < ARRAY_SIZE(HierOnPath); i++) {
			int * final_ids = HierOnPath[i];
			int * working_ids = HierOpened[i];
			float * costs = HierCosts[i];
			int count = Map.SubzoneTrackingEntryCount[i];
			for (int j = count - 1; j >= 0; j--) {
				final_ids[j] = 0;
				working_ids[j] = 0;
				costs[j] = 0;
			}
		}

		UniqueID++;
	}
}


/// <summary>
/// Has this cell already been examined by the current search?
/// Each search stamps the cells it visits with its own identifier rather than clearing the
/// whole table, so a stamp left over from an earlier search reads as unvisited.
/// </summary>
/// <param name="base_level">Should the ground level record be tested rather than the bridge
/// deck one?</param>
/// <param name="index">The playfield index of the cell to test.</param>
/// <returns>bool; Has the cell been visited?</returns>
bool AStarClass::Is_Visited(int, bool base_level, int index)
{
	if (base_level) {
		if (RegularVisited[index] == UniqueID) {
			return(true);
		}
		return(false);
	}
	if (RegularBridgeVisited[index] == UniqueID) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Creates the pathfinder and allocates its node pools.
/// The per cell and per subzone tables are not built here -- they depend on the size of the
/// map, so Update_Map_Dimensions and Reset supply them once a map exists.
/// </summary>
AStarClass::AStarClass(void) :
	field_0(false),
	IsAvoidBridges(false),
	field_2(false),
	IsAvoidPathCollision(true),
	MovementCostMultiplier(1),
	UseLocomotorEnterCheck(true),
	RegularVisited(NULL),
	RegularBridgeVisited(NULL),
	RegularBridgeMovementCosts(NULL),
	RegularMovementCosts(NULL),
	UniqueID(-1),
	ObjectSpeed(-1),
	IsHSEnabled(true),
	Avoidance(AVOIDANCE_NONE),
	HierNodeIndex(-1),
	HierLastNodeCell(0,0)
{
	RegularQueue = new PriorityQueueClass<RegularOpenNode>(65536);
	HierQueue = new PriorityQueueClass<AStarHierarchicalNode>(10000);
	RegularOpenNodes = new RegularOpenNodePool;
	RegularNodes = new RegularNodePool;

	RegularNodes->ActiveCount = 0;
	RegularOpenNodes->ActiveCount = 0;
	RegularQueue->Clear();
	HierQueue->Clear();

	for (int i = 0; i < 3; i++) {
		HierOnPath[i] = NULL;
		HierOpened[i] = NULL;
		HierCosts[i] = NULL;
		HierBannedEdges[i].Clear();
		memset(HierSubzonePath[i], 0, sizeof(HierSubzonePath[i]));
		HierSubzonePathCount[i] = 0;
	}
	HierNodePool = new AStarHierarchicalNode[10000];
}


/// <summary>
/// Destroys the pathfinder, releasing its node pools and search tables.
/// </summary>
AStarClass::~AStarClass(void)
{
	delete RegularQueue;
	RegularQueue = NULL;

	delete HierQueue;
	HierQueue = NULL;

	if (RegularOpenNodes != NULL) {
		delete RegularOpenNodes;
	}
	RegularOpenNodes = NULL;

	if (RegularNodes != NULL) {
		delete RegularNodes;
	}
	RegularNodes = NULL;

	if (RegularVisited != NULL) {
		delete RegularVisited;
		RegularVisited = NULL;
	}

	if (RegularBridgeVisited != NULL) {
		delete RegularBridgeVisited;
		RegularBridgeVisited = NULL;
	}

	if (RegularMovementCosts != NULL) {
		delete RegularMovementCosts;
		RegularMovementCosts = NULL;
	}

	if (RegularBridgeMovementCosts != NULL) {
		delete RegularBridgeMovementCosts;
		RegularBridgeMovementCosts = NULL;
	}

	for (int i = 0; i < 3; i++) {
		if (HierOnPath[i]) {
			delete HierOnPath[i];
			HierOnPath[i] = NULL;
		}

		if (HierOpened[i]) {
			delete HierOpened[i];
			HierOpened[i] = NULL;
		}

		if (HierCosts[i]) {
			delete HierCosts[i];
			HierCosts[i] = NULL;
		}
	}

	delete [] HierNodePool;
}


/// <summary>
/// Determines the facing that leads from one cell to a neighboring one.
/// </summary>
/// <returns>Returns with the facing to step from the first cell to the second. If the cells
/// are not neighbors, FACING_COUNT is returned.</returns>
FacingType Facing_Between(Cell const & cell1, Cell const & cell2)
{
	static FacingType _table[3][3] = {
		{ FACING_SE, FACING_S, FACING_SW },
		{ FACING_E, FACING_NONE, FACING_W },
		{ FACING_NE, FACING_N, FACING_NW }
	};

	if (abs(cell2.Y - cell1.Y) <= 1 && abs(cell2.X - cell1.X) <= 1) {
		return(_table[cell2.Y - cell1.Y + 1][cell2.X - cell1.X + 1]);
	}
	return(FACING_COUNT);
}


/// <summary>
/// Builds the final move list from a completed search.
/// This routine walks the chain of search nodes back to the start, filling in the facing
/// commands and the cell heights that the locomotors will follow.
/// </summary>
/// <param name="final_node">The search node the destination was reached on.</param>
/// <param name="moves">Buffer that receives the facing commands.</param>
/// <returns>Returns with a pointer to the completed path control structure.</returns>
/// <remarks>There is only one path control structure, so building a new path invalidates
/// the last one this routine handed out.</remarks>
PathStruct * AStarClass::Build_Final_Path(RegularOpenNode *final_node, FacingType *moves)
{
	static PathStruct path; // Main path control.

	path.Cost			= int(final_node->Score);
	path.Length 		= final_node->PathLength;
	path.Overlap		= 0;
	path.LastOverlap.X	= 0;
	path.LastOverlap.Y	= 0;
	path.Command 		= moves;
	path.Height			= CellHeights;

	RegularNode * current = final_node->Node;
	RegularNode * parent = current->Parent;

	for (int i = (final_node->PathLength - 2); i >= 0; i--) {
		if (parent != NULL) {
			CellHeights[i] = parent->CellHeight;
			moves[i] = Facing_Between((*current->CellSlot)->CellID, (*parent->CellSlot)->CellID);
		}
		current = current->Parent;
		parent = parent->Parent;
	}

	moves[final_node->PathLength - 1] = FACING_NONE;

	path.Start = (*current->CellSlot)->CellID;

	if (path.Cost == 0) {
		path.Cost = 1;
	}
	return(&path);
}


/// <summary>
/// Resizes the search tables to suit a new map size.
/// The regular search keeps a visit stamp and a running cost for every cell, and the facing
/// to index offsets depend on the width of the playfield, so all of them are rebuilt here.
/// </summary>
/// <param name="dimensions">The new playable area of the map.</param>
/// <remarks>Call this routine whenever the map dimensions change, before any path is
/// requested.</remarks>
void AStarClass::Update_Map_Dimensions(Rect const & dimensions)
{
	if (RegularVisited != NULL) {
		delete RegularVisited;
		RegularVisited = NULL;
	}
	if (RegularBridgeVisited != NULL) {
		delete RegularBridgeVisited;
		RegularBridgeVisited = NULL;
	}
	if (RegularMovementCosts != NULL) {
		delete RegularMovementCosts;
		RegularMovementCosts = NULL;
	}
	if (RegularBridgeMovementCosts != NULL) {
		delete RegularBridgeMovementCosts;
		RegularBridgeMovementCosts = NULL;
	}

	MapCellStride = dimensions.Height + dimensions.Width + 1;
	int count = MapCellStride * MapCellStride;
	RegularBridgeVisited = new int[count];
	RegularVisited = new int[count];
	RegularMovementCosts = new float[count];
	RegularBridgeMovementCosts = new float[count];

	AStarFacingToOffset[FACING_N]	= -MapCellStride;
	AStarFacingToOffset[FACING_NE]	= 1 - MapCellStride;
	AStarFacingToOffset[FACING_E]	= +1;
	AStarFacingToOffset[FACING_SE]	= MapCellStride + 1;
	AStarFacingToOffset[FACING_S]	= +MapCellStride;
	AStarFacingToOffset[FACING_SW]	= MapCellStride - 1;
	AStarFacingToOffset[FACING_W]	= -1;
	AStarFacingToOffset[FACING_NW]	= -1 - MapCellStride;
}


/// <summary>
/// Toggles the predicted path marks used for collision avoidance.
/// This routine flags the cells that objects in the way are themselves about to walk
/// through, so the search prices those cells up and steers around them. Find_Path_Regular
/// calls it once before searching and once afterwards, the second call taking down the
/// marks the first put up.
/// </summary>
/// <param name="foot">The object about to be pathed.</param>
/// <remarks>Every call flips the marks, so calls must come in pairs or the map is left
/// with stale predicted path flags on it.</remarks>
void AStarClass::Apply_Path_Collision_Avoidance(FootClass * foot)
{
	if (IsAvoidPathCollision) {
		Cell current_cell = foot->Get_Cell();
		CellClass * current_cell_ptr = &Map[current_cell];
		CellClass * next_cell_ptr = &Map[::Adjacent_Cell(current_cell, foot->PrimaryFacing.Current().As_Dir8())];

		FootClass * blocker;
		bool on_bridge;
		if (next_cell_ptr->IsUnderBridge && (abs(current_cell_ptr->Height - next_cell_ptr->Height) > 3 || foot->IsOnBridge)) {
			blocker = (FootClass *)next_cell_ptr->Cell_Bridge_Occupier();
			on_bridge = true;
		} else {
			blocker = (FootClass *)next_cell_ptr->Cell_Occupier();
			on_bridge = false;
		}
		if (blocker == NULL) {
			blocker = Find_Moving_Blocker(next_cell_ptr->CellID, next_cell_ptr->Height + (on_bridge ? BRIDGE_CELL_HEIGHT : 0));
		}

		bool marked_path = false;
		const TechnoTypeClass * foot_type = foot->TClass;

		while (blocker != NULL) {
			if (blocker->RTTI == RTTI_UNIT || blocker->RTTI == RTTI_INFANTRY) {

				int index = 0;
				Cell blocker_cell = blocker->LastPathingCell;
				const TechnoTypeClass * blocker_type = blocker->TClass;

				if (Avoidance == AVOIDANCE_HARD || (foot_type != blocker_type && foot_type->MaxSpeed > blocker_type->MaxSpeed && Map.In_Local_Radar(blocker_cell, true))) {
					if (blocker->RTTI == RTTI_UNIT) {
						if (blocker->Path[0] == FACING_NONE || blocker->Path[1] == FACING_NONE) {
							blocker = (FootClass *)blocker->Next;
							continue;
						}
					} else {
						if (blocker->Path[0] == FACING_NONE || blocker->Path[1] == FACING_NONE || blocker->Path[2] == FACING_NONE) {
							blocker = (FootClass *)blocker->Next;
							continue;
						}
					}
				} else {
					blocker = (FootClass *)blocker->Next;
					continue;
				}

				marked_path = true;
				while (index < ARRAY_SIZE(blocker->Path) && blocker->Path[index] != FACING_NONE) {
					blocker_cell = Next_Cell(blocker_cell, blocker->Path[index]);
					Map[blocker_cell].IsPredictedPath = !Map[blocker_cell].IsPredictedPath;
					index++;
				}
			}
			blocker = (FootClass *)blocker->Next;
		}

		if (!marked_path && Avoidance == AVOIDANCE_SOFT) {
			Avoidance = AVOIDANCE_NONE;
			return;
		}

		for (int x = -2; x < 3; x++) {
			for (int y = -2; y < 3; y++) {
				CellClass * cellptr = &Map[next_cell_ptr->Fetch_CellID() + Cell(x, y)];
				if (cellptr->Flag.Composite && cellptr->Fetch_CellID() != current_cell) {
					cellptr->IsPredictedPath = !cellptr->IsPredictedPath;
				}
			}
		}

		next_cell_ptr->IsPredictedPath = !next_cell_ptr->IsPredictedPath;
	}
}


/// <summary>
/// Finds an object that is heading for the given cell.
/// This routine is used by the collision avoidance code when a cell looks empty but
/// something nearby has already laid claim to it. The neighborhood is swept and each foot
/// object asked whether it is moving to the spot.
/// </summary>
/// <param name="cell">The cell being contested.</param>
/// <param name="cell_height">The height of that cell, so bridge decks are told apart from
/// the ground beneath them.</param>
/// <returns>Returns with a pointer to the object heading there, or NULL if none is.</returns>
FootClass * AStarClass::Find_Moving_Blocker(Cell const & cell, int cell_height)
{
	Coord coord = Coord(cell, 0);
	coord.Z = cell_height * LEVEL_LEPTON_H;
	for (int y = -2; y < 3; y++) {
		for (int x = -2; x < 3; x++) {
			CellClass & neighbor = Map[Cell(x,y) + cell];
			FootClass * occupier = (FootClass *)neighbor.Cell_Occupier(neighbor.IsUnderBridge && abs(neighbor.Height - cell_height) > 2);
			while (occupier != NULL) {
				if (occupier->Is_Foot()) {
					if (occupier->Locomotion->Is_Moving_Here(coord)) {
						return(occupier);
					}
				}
				occupier = (FootClass *)occupier->Next;
			}
		}
	}
	return(NULL);
}


/// <summary>
/// Converts a map cell into a rotated playfield index.
/// This routine flattens the diamond shaped playable area onto a plain array, so that code
/// keeping one entry per playable cell can index it directly.
/// </summary>
/// <returns>Returns with the index of the cell within the playfield array.</returns>
int Map_Cell_Index(Cell const & cell)
{
	return(((cell.X - cell.Y + Map.PlayRect.Width - 1) >> 1) + Map.PlayRect.Width * (cell.X - Map.PlayRect.Width + cell.Y - 1));
}


/// <summary>
/// Fetches the number of entries a rotated playfield array needs.
/// </summary>
/// <returns>Returns with the number of cell slots the playfield index covers.</returns>
int Map_Cell_Count(void)
{
	return((2 * Map.PlayRect.Width) * (Map.PlayRect.Height + 4));
}


/// <summary>
/// Straightens the right angle corners out of a finished path.
/// This routine scans the completed move list for a pair of perpendicular runs and hands
/// each one it finds to Try_Diagonal_Shortcut, which cuts the corner with a diagonal where
/// the terrain allows. Find_Path_Regular calls this before the move list is optimized.
/// </summary>
/// <param name="path">The finished path to straighten.</param>
/// <param name="foot">The object the path was built for.</param>
void AStarClass::Cut_Corners(PathStruct * path, FootClass * foot)
{
	int max_len = path->Length;

	/*
	**	Account for trailing end of list command, so reduce the maximum
	**	allowed legal commands to reflect this.
	*/
	max_len--;

	FacingType leg2_dir = FACING_NONE; /// The direction of the second part of the corner

	unsigned int * heights = path->Height;
	FacingType * moves = path->Command;
	Cell source = path->Start;

	int leg1_start = 0;
	int leg2_start = 0;

	bool found_corner = false;

	int leg1_len = 0;
	int leg2_len = 0;

	Cell start_cell = source;  /// Where does the corner we're straightening start
	Cell cursor_cell = source; /// Where we are currently

	FacingType leg1_dir = FACING_NONE; /// The direction of the first part of the corner

	/*
	 * Scan through the finalized move list looking for a turn pattern
	 * that can be replaced by a diagonal shortcut.
	 */
	while (leg1_start + leg1_len < max_len && leg2_start + leg2_len < max_len) {

		/*
		 * Normal scanning state - accumulate a straight segment until
		 * a turn suitable for diagonal fixup is detected.
		 */
		if (!found_corner) {
			FacingType move_dir = moves[leg1_start + leg1_len];
			FacingType turn = Facing_Sub(move_dir, leg1_dir);

			/*
			 * If direction continues unchanged, extend the straight segment.
			 */
			if (move_dir == leg1_dir) {
				leg1_len++;

			/*
			 * If this is a perpendicular turn, begin evaluating a diagonal shortcut.
			 */
			} else if ((turn == FACING_90 || turn == FACING_270) && leg1_dir != FACING_NONE && leg1_dir != TUNNEL && move_dir != TUNNEL) {
				found_corner = true;
				leg2_dir = move_dir;
				leg2_len = 1;
				leg2_start = leg1_start + leg1_len;

			/*
			 * Otherwise this turn cannot be optimized; advance the segment
			 * start and begin tracking a new straight run.
			 */
			} else {
				leg1_start += leg1_len;
				leg1_len = 1;

				/*
				 * Consider starting a new corner to track.
				 * We only try straightening diagonal corners.
				 */
				leg1_dir = (move_dir & 1) ? move_dir : FACING_NONE;
				start_cell = cursor_cell;
			}

			cursor_cell = Next_Cell(cursor_cell, move_dir);

		/*
		 * Fixup evaluation state - continue measuring the candidate
		 * segment while direction remains constant.
		 */
		} else if (moves[leg2_len + leg2_start] == leg2_dir) {
			leg2_len++;

		/*
		 * Candidate segment ended; attempt diagonal shortcut splice.
		 * If successful, restart scanning from the modified position.
		 */
		} else {
			leg1_start += Try_Diagonal_Shortcut(foot, &moves[leg1_start], &heights[leg1_start], leg1_len, leg2_len, start_cell);
			leg1_len = 1;
			found_corner = false;

			/*
			 * Reinitialize traversal state after modification.
			 */
			cursor_cell = ::Adjacent_Cell(start_cell, moves[leg1_start]);
			leg1_dir = moves[leg1_start];
		}
	}

	/*
	 * If loop exited while still evaluating a fixup candidate,
	 * perform one final shortcut attempt on the trailing segment.
	 */
	if (found_corner) {
		Try_Diagonal_Shortcut(foot, &moves[leg1_start], &heights[leg1_start], leg1_len, leg2_len, start_cell);
	}
}


/// <summary>
/// Replaces a corner in the move list with a diagonal run.
/// The diagonal that would cut the corner is tested cell by cell for blockage and threat,
/// and is shortened until it fits or is given up on. Tunnel moves are never cut.
/// </summary>
/// <param name="foot">The object the path belongs to.</param>
/// <param name="moves">The move list, positioned at the start of the first leg.</param>
/// <param name="heights">The cell heights that go with those moves.</param>
/// <param name="initial_first_leg_length">Length of the leg leading into the corner.</param>
/// <param name="initial_second_leg_length">Length of the leg leading out of the corner.</param>
/// <param name="cell">The cell the first leg starts at; advanced past the moves used up.</param>
/// <returns>Returns with the number of moves used up, which the caller adds to its scanning
/// position.</returns>
int AStarClass::Try_Diagonal_Shortcut(FootClass * foot, FacingType * moves, unsigned int * heights, int initial_first_leg_length, int initial_second_leg_length, Cell & cell)
{
	int first_leg_length = initial_first_leg_length;
	int second_leg_length = initial_second_leg_length;
	FacingType first_leg_direction = moves[0];
	FacingType second_leg_direction = moves[first_leg_length];
	FacingType diagonal_direction = FacingType((first_leg_direction + second_leg_direction) >> 1);

	if (diagonal_direction + FACING_45 != second_leg_direction
	&& diagonal_direction + FACING_45 != first_leg_direction) {
		diagonal_direction = FACING_N;
	}

	if (first_leg_direction == TUNNEL || second_leg_direction == TUNNEL) {
		cell = Follow_Path2(cell, first_leg_length + second_leg_length, moves);
		return(first_leg_length + second_leg_length);
	}

	Cell diagonal_start_cell = cell;

	int max_diagonal_length = second_leg_length;
	second_leg_length = first_leg_length < max_diagonal_length ? first_leg_length : max_diagonal_length;
	if (second_leg_length < first_leg_length) {
		diagonal_start_cell = Follow_Path(diagonal_start_cell, first_leg_length - second_leg_length, moves);
	}

	double threat_avoidance = foot->Threat_Avoidance_Value();
	HouseClass * house = foot->House;

	while (second_leg_length > 0) {
		int moves_remaining = second_leg_length * 2;
		bool is_blocked = false;
		Cell scan_cell = Adjacent_Cell(diagonal_start_cell, diagonal_direction);
		int scan_height = heights[first_leg_length + second_leg_length - moves_remaining];
		CellClass * scan_cell_ptr = &Map[scan_cell];

		while (!is_blocked && moves_remaining > 0) {
			is_blocked = foot->Can_Enter_Cell(scan_cell_ptr, diagonal_direction, scan_height) != MOVE_OK
			|| scan_cell_ptr->IsPredictedPath
			|| Map.Cell_Threat(diagonal_start_cell, *house) * threat_avoidance >= 1.0;

			moves_remaining--;

			scan_cell = Adjacent_Cell(scan_cell, diagonal_direction);
			scan_cell_ptr = &Map[scan_cell];

			scan_height = (scan_height - scan_cell_ptr->Height == BRIDGE_CELL_HEIGHT && scan_cell_ptr->IsUnderBridge)
			? scan_cell_ptr->Height + BRIDGE_CELL_HEIGHT
			: scan_cell_ptr->Height;

		}

		if (!is_blocked) {
			FacingType * diagonal_moves = &moves[first_leg_length - second_leg_length];
			for (int move_index = 0; move_index < 2 * second_leg_length; move_index++) {
				diagonal_moves[move_index] = diagonal_direction;
			}

			cell = Follow_Path(cell, first_leg_length - second_leg_length, moves);
			return(first_leg_length - second_leg_length);
		}

		diagonal_start_cell = Adjacent_Cell(diagonal_start_cell, first_leg_direction);
		second_leg_length--;
	}

	cell = Follow_Path2(cell, first_leg_length, moves);
	return(first_leg_length);
}


/***********************************************************************************************
 * Optimize_Moves -- Optimize the move list.                                                   *
 *                                                                                             *
 * INPUT:      char *moves to optimize                                                         *
 *                                                                                             *
 * OUTPUT:     none (list is optimized)                                                        *
 *                                                                                             *
 * WARNINGS:   EMPTY moves are used to hold the place of eliminated                            *
 *             commands. Also, NEVER call this routine with a list that                        *
 *             contains illegal commands. The list MUST be terminated                          *
 *             with a EOL command                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/08/1991  CY : Created.                                                                 *
 *   06/01/1992  JLB : Optimized and commented.                                                *
 *=============================================================================================*/
void AStarClass::Optimize_Moves(PathStruct * path, FootClass * foot)
{
	int max_len = path->Length;

	/*
	**	Account for trailing end of list command, so reduce the maximum
	**	allowed legal commands to reflect this.
	*/
	max_len--;

	Cell cursor_cell = path->Start;
	FacingType * moves = path->Command;
	unsigned int * heights = path->Height;

	int index;
	int scan_index = 0;
	int prev_turn_index = 0;
	int last_turn_index = 0;
	int max_axis;
	int max_axis_extent = 0;
	Cell path_offset(0, 0);
	Cell last_turn_cell(0, 0);
	int max_dev_x = 0;
	int max_dev_y = 0;
	Cell candidate_offset(0, 0);

	/*
	 * Scan the early portion of the move list, looking for a turn pattern
	 * that can be replaced by a straighter segment without increasing risk.
	 */
	while (scan_index < max_len && scan_index < 20) {
		FacingType move = moves[scan_index];
		FacingType step = moves[scan_index];

		if (step == TUNNEL) {

			/*
			 * Tunnels break continuity; reset the straightening state.
			 */
			cursor_cell = Adjacent_Cell(cursor_cell, FACING_N);
			candidate_offset = Cell(0, 0);
			scan_index++;
			prev_turn_index = scan_index;
			max_axis_extent = 0;
			max_dev_x = 0;
			max_dev_y = 0;
			path_offset = Cell(0, 0);
			last_turn_index = scan_index;
			last_turn_cell = Cell(0, 0);
			continue;
		}

		if (move == EMPTY) {
			scan_index++;
			continue;
		}

		Cell new_offset = Adjacent_Cell(path_offset, move);
		Cell new_candidate_offset = Adjacent_Cell(candidate_offset, move);

		/*
		 * Track the envelope of displacement; if it stops expanding, try to
		 * straighten the segment by splicing a new line.
		 */
		if (abs(new_candidate_offset.X) >= max_dev_x && abs(new_candidate_offset.Y) >= max_dev_y) {

			candidate_offset = new_candidate_offset;

			max_dev_x = abs(new_candidate_offset.X);
			max_dev_y = abs(new_candidate_offset.Y);

			max_axis = std::max(abs(new_offset.X), abs(new_offset.Y));

			cursor_cell = Adjacent_Cell(cursor_cell, step);
			if (max_axis_extent < max_axis) {
				max_axis_extent = max_axis;
			} else {
				int splice_index;
				Cell splice_cell = cursor_cell;
				Splice_Path(moves, scan_index, prev_turn_index, splice_index, splice_cell);
				Cell to_vector = cursor_cell - splice_cell;
				Plot_Straight_Line(&moves[splice_index], scan_index - splice_index + 1, splice_cell, to_vector, foot, heights[splice_index], false);
			}

			path_offset = new_offset;
			++scan_index;
		} else {
			if (last_turn_cell == Cell(0, 0)) {
				max_dev_x = 0;
				max_dev_y = 0;
				last_turn_cell = cursor_cell;
				last_turn_index = scan_index;
				candidate_offset = Cell(0, 0);
			} else {
				prev_turn_index = last_turn_index;
				path_offset = last_turn_cell - cursor_cell;
				max_dev_x = 0;
				max_dev_y = 0;
				candidate_offset = Cell(0, 0);
				max_axis_extent = std::max(abs(path_offset.X), abs(path_offset.Y));
				last_turn_cell = cursor_cell;
				last_turn_index = scan_index;
			}
		}
	}

	/*
	 * Final tail cleanup: if the last segment is longer than its straight-line
	 * extent, attempt one more splice and replot.
	 */
	if (last_turn_cell != Cell(0, 0)) {
		Cell diff = cursor_cell - last_turn_cell;
		max_axis = std::max(abs(diff.X), abs(diff.Y));
		if ((scan_index - last_turn_index - 1) > max_axis) {
			scan_index--;
			int splice_index;
			Cell splice_cell = cursor_cell;
			Splice_Path(moves, scan_index, last_turn_index, splice_index, splice_cell);
			Cell to_vector = cursor_cell - splice_cell;
			Plot_Straight_Line(&moves[splice_index], scan_index - splice_index + 1, splice_cell, to_vector, foot, heights[splice_index], true);
		}
	}

	/*
	**	Pack the command list to remove any EMPTY command entries.
	*/
	index = 0;
	int new_len = 0;
	while (moves[index] != END && index < max_len) {
		if (moves[index] != EMPTY) {
			moves[new_len++] = moves[index];
		}
		index++;
	}

	int fill = new_len;
	while (fill < path->Length + 1) {
		moves[fill++] = END;
	}

	path->Length = new_len + 1;
}


/// <summary>
/// Finds the point in a move list where a wandering leg can be straightened.
/// This routine is used by Optimize_Moves to decide how much of a leg's tail is worth
/// throwing away. The index and cell handed back are passed straight to Plot_Straight_Line,
/// which replots that stretch as a direct run.
/// </summary>
/// <param name="moves">The move list to scan.</param>
/// <param name="start_index">Index to begin scanning backwards from.</param>
/// <param name="end_index">Index to stop scanning at.</param>
/// <param name="splice_index">Receives the index the tail should be replaced from.</param>
/// <param name="cell">The cell the scan starts at; receives the cell at the splice point.</param>
void AStarClass::Splice_Path(FacingType * moves, int start_index, int end_index, int & splice_index, Cell & cell)
{
	int max_radius = 0;

	Cell displacement;
	displacement.X = 0;
	displacement.Y = 0;

	int cur_index = start_index;
	Cell base = cell;
	bool found = false;

	while (cur_index >= end_index) {
		if (moves[cur_index] == EMPTY) {
			cur_index--;
		} else {
			FacingType back = Facing_Sub(moves[cur_index], 4);
			displacement = ::Adjacent_Cell(displacement, back);
			base = ::Adjacent_Cell(base, back);
			int abs_y = abs(displacement.Y);
			int abs_x = abs(displacement.X);
			int radius = std::max(abs_x, abs_y);
			if (radius > max_radius) {
				if (found) {
					splice_index = cur_index + 1;
					cell = ::Adjacent_Cell(base, Facing_Sub(back, 4)); /// reverse, then reverse again: recovers the base from before this backward step
					return;
				}
				max_radius = radius;
			} else {
				found = true;
			}
			cur_index--;
		}
	}

	splice_index = end_index;
	cell = base;
	return;
}


/// <summary>
/// Plots a simple two legged route toward a destination.
/// This routine tries to get there with one diagonal run and one straight run, checking
/// every cell along the way for blockage, predicted traffic and threat. Both leg orders are
/// tried. Use this routine as a cheap stand in for the full search where the route is
/// likely to be trivial.
/// </summary>
/// <param name="moves">Buffer that receives the facing commands, padded out with EMPTY.</param>
/// <param name="move_count">The size of the move buffer.</param>
/// <param name="from">The cell the walk starts at.</param>
/// <param name="to">The destination, expressed as an offset from the starting cell.</param>
/// <param name="foot">The object the route is being plotted for.</param>
/// <param name="cell_height">The height the object starts at, so bridges are tracked.</param>
/// <param name="fearless">Should the object walk through threatened cells?</param>
/// <returns>bool; Was a clear route plotted?</returns>
bool AStarClass::Plot_Straight_Line(FacingType * moves, int move_count, Cell const & from, Cell const & to, FootClass * foot, int cell_height, bool fearless)
{
	int tries = 0;

	const int x = to.X;
	const int y = to.Y;

	FacingType first_dir;
	if (to.X < 0) {
		first_dir = to.Y < 0 ? FACING_NW : FACING_SW;
	} else {
		first_dir = to.Y < 0 ? FACING_NE : FACING_SE;
	}

	const int sub_xy = (int)to.X - (int)to.Y;
	const int sum_xy = (int)to.X + (int)to.Y;

	FacingType second_dir;
	if (sub_xy > 0) {
		second_dir = sum_xy > 0 ? FACING_E : FACING_N;
	} else {
		second_dir = sum_xy > 0 ? FACING_S : FACING_W;
	}

	int y_dist = abs(y);
	int x_dist = abs(x);

	int min_dist = std::min(x_dist, y_dist);
	int max_dist = std::max(x_dist, y_dist);

	int first_dist = min_dist;
	int second_dist = max_dist - min_dist;

	double avoidance = foot->Threat_Avoidance_Value();
	int threats = 0;
	HouseClass * house = foot->House;
	bool avoid_threats = avoidance > 0.00001;

	bool blocked = false;

	while (tries < 2) {

		threats = 0;

		if (first_dist != 0) {
			int height = cell_height;

			blocked = false;
			Cell current = from;
			int count1 = first_dist;
			int count2 = second_dist;

			while (count1 > 0 && !blocked) {

				current = ::Adjacent_Cell(current, first_dir);
				CellClass * cellptr = &Map[current];

				if (avoid_threats && Map.Cell_Threat(current, *house) * avoidance >= 0.01) {
					threats++;
				}

				blocked = foot->Can_Enter_Cell(cellptr, first_dir, height) != MOVE_OK || cellptr->IsPredictedPath || threats > 3 || (!fearless && threats > 0);
				count1--;

				int step_height = cellptr->Height;

				if (height - step_height == BRIDGE_CELL_HEIGHT) {
					height = step_height + BRIDGE_CELL_HEIGHT;
					if (cellptr->IsUnderBridge) {
						continue;
					}
				}

				height = step_height;
			}

			while (count2 > 0 && !blocked) {

				current = Adjacent_Cell(current, second_dir);
				CellClass * cellptr = &Map[current];

				if (avoid_threats && Map.Cell_Threat(current, *house) * avoidance >= 0.01) {
					threats++;
				}

				blocked = foot->Can_Enter_Cell(cellptr, second_dir, height) != MOVE_OK || cellptr->IsPredictedPath || threats > 3 || (!fearless && threats > 0);
				count2--;

				int step_height = cellptr->Height;

				if (height - step_height == BRIDGE_CELL_HEIGHT) {
					height = step_height + BRIDGE_CELL_HEIGHT;
					if (cellptr->IsUnderBridge) {
						continue;
					}
				}

				height = step_height;
			}
		}

		if (first_dist == 0 || blocked) {
			std::swap(second_dir, first_dir);
			std::swap(second_dist, first_dist);
		} else {
			int i;
			for (i = 0; i < first_dist; i++) {
				moves[i] = first_dir;
			}
			for (i = 0; i < second_dist; i++) {
				moves[first_dist + i] = second_dir;
			}
			for (i = 0; i < move_count - first_dist - second_dist; i++) {
				moves[first_dist + second_dist + i] = EMPTY;
			}
			return(true);
		}
		tries++;
	}
	return(false);
}


/// <summary>
/// Reallocates the per subzone search tables.
/// The hierarchical search keeps a visit stamp and a running cost for every subzone, so
/// these tables have to be rebuilt whenever the map's subzone graph changes size.
/// </summary>
/// <remarks>Call this routine after any zone or subzone rebuild, or the search will index
/// off the end of its tables.</remarks>
void AStarClass::Reset(void)
{
	int j;

	for (int i = 0; i < ARRAY_SIZE(HierOnPath); i++) {
		if (HierOnPath[i] != NULL) {
			delete[] HierOnPath[i];
		}
		int * final_ids = new int[Map.SubzoneTrackingEntryCount[i]];
		HierOnPath[i] = final_ids;
		for (j = Map.SubzoneTrackingEntryCount[i] - 1; j >= 0; j--) {
			final_ids[j] = 0;
		}

		if (HierOpened[i] != NULL) {
			delete[] HierOpened[i];
		}
		int * working_ids = new int[Map.SubzoneTrackingEntryCount[i]];
		HierOpened[i] = working_ids;
		for (j = Map.SubzoneTrackingEntryCount[i] - 1; j >= 0; j--) {
			working_ids[j] = 0;
		}

		if (HierCosts[i] != NULL) {
			delete[] HierCosts[i];
		}
		float * costs = new float[Map.SubzoneTrackingEntryCount[i]];
		HierCosts[i] = costs;
		for (j = Map.SubzoneTrackingEntryCount[i] - 1; j >= 0; j--) {
			costs[j] = 0;
		}
	}
}


/// <summary>
/// Finds a coarse route through the map's subzone graph.
/// This routine searches the subzone connection graph at each level of coarseness in turn,
/// every level confined to the corridor the coarser one came up with. The corridor that
/// falls out of it is what Find_Path_Regular is then restricted to, and that is what keeps
/// long paths affordable.
/// </summary>
/// <param name="mzone">The movement zone that decides which subzones count as passable.</param>
/// <param name="foot">The object being pathed for, consulted for threat avoidance. May be
/// NULL.</param>
/// <returns>bool; Was a corridor found at every level?</returns>
bool AStarClass::Find_Path_Hierarchical(Cell const & from, Cell const & to, MZoneType mzone, FootClass const * foot)
{
	int *pass_table = (int *)MZonePassability[mzone];

	double threat_avoidance = foot != NULL ? foot->Threat_Avoidance_Value() : threat_avoidance = 0.0;

	HouseClass * house = foot != NULL ? foot->House : NULL;

	bool avoid_threats = threat_avoidance > 0.00001;

	for (int subzone_level = SUBZONE_COARSE; subzone_level >= SUBZONE_FINE; subzone_level--) {
		HierQueue->Clear();

		CellSubzoneStruct & start_cell_subzones = Map.CellSubzones[Map.Get_Cell_Zone_Index(from)];
		int start_subzone = start_cell_subzones.SubzoneID[subzone_level];
		CellSubzoneStruct & end_cell_subzones = Map.CellSubzones[Map.Get_Cell_Zone_Index(to)];
		int end_subzone = end_cell_subzones.SubzoneID[subzone_level];

		bool is_coarse = subzone_level == SUBZONE_COARSE;
		int * coarser_final_ids;
		if (!is_coarse) {
			coarser_final_ids = HierOnPath[subzone_level + 1];
		} else {
			coarser_final_ids = NULL;
		}

		int * final_ids = HierOnPath[subzone_level];
		int * working_ids = HierOpened[subzone_level];
		float * costs = HierCosts[subzone_level];
		final_ids[start_subzone] = UniqueID;
		final_ids[end_subzone] = UniqueID;

		if (start_subzone == end_subzone) {
			if (subzone_level == SUBZONE_FINE) {
				AStarHierarchicalNode * node = &HierNodePool[0];
				node->Depth = 0;
				node->SubzoneID = start_subzone;
			}
			HierSubzonePath[subzone_level][0] = start_subzone;
			HierSubzonePathCount[subzone_level] = 1;
		} else {
			int node_count = 0;
			AStarHierarchicalNode * start_node = &HierNodePool[0];
			start_node->ParentIndex = -1;
			start_node->SubzoneID = start_subzone;
			start_node->Score = 0.0;
			start_node->Depth = 0;

			HierQueue->Insert(*start_node);
			node_count++;
			working_ids[start_subzone] = UniqueID;
			costs[start_subzone] = 0.0;

			AStarHierarchicalNode * best_node = HierQueue->Extract_Min();
			bool no_banned_edges = HierBannedEdges[subzone_level].Count() == 0;

			while (best_node != NULL) {
				int from_subzone = best_node->SubzoneID;
				if (from_subzone == end_subzone) {
					break;
				}

				SubzoneConnectionStruct * connection = &Map.SubzoneTracking[subzone_level][from_subzone].Connections[0];

				int conn_count = Map.SubzoneTracking[subzone_level][from_subzone].Connections.Count();
				for (int conn_idx = 0; conn_idx < conn_count; conn_idx++) {
					int threat = 0;
					int to_subzone = connection[conn_idx].SubzoneID;
					bool is_cross_block = connection[conn_idx].IsCrossBlock;
					PassabilityType passability = Map.SubzoneTracking[subzone_level][to_subzone].Passability;
					int to_coarser_subzone = Map.SubzoneTracking[subzone_level][to_subzone].ParentSubzoneID;

					if (avoid_threats) {
						threat = Map.Region_Threat(house, subzone_level, from_subzone, to_subzone) * threat_avoidance;
					}

					double extra_score = is_cross_block ? 0.001 : 0.0;

					static const float _passability_scores[PASSABLE_COUNT] = {1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0};
					float score = _passability_scores[passability] + best_node->Score + (double)threat + extra_score;

					if ((working_ids[to_subzone] != UniqueID || costs[to_subzone] > score) && (is_coarse || coarser_final_ids[to_coarser_subzone] == UniqueID || passability == PASSABLE_CRUSH) && pass_table[passability] == TRAVERSAL_PASSABLE) {
						if (no_banned_edges || !Subzone_Edge_Banned(from_subzone, to_subzone, subzone_level)) {
							AStarHierarchicalNode * new_node = &HierNodePool[node_count];
							new_node->ParentIndex = best_node - HierNodePool;
							new_node->SubzoneID = to_subzone;
							new_node->Score = score;
							new_node->Depth = best_node->Depth + 1;
							HierQueue->Insert(*new_node);
							working_ids[to_subzone] = UniqueID;
							costs[to_subzone] = score;
							node_count++;
						}
					}
				}
				best_node = HierQueue->Extract_Min();
			}

			if (best_node == NULL) {
				return(false);
			}

			AStarHierarchicalNode * tail_node = best_node;
			while (best_node->ParentIndex != -1) {
				final_ids[best_node->SubzoneID] = UniqueID;
				best_node = &HierNodePool[best_node->ParentIndex];
			}

			int path_count = tail_node->Depth + 1;
			HierSubzonePathCount[subzone_level] = path_count;
			path_count--;
			while (path_count > 0) {
				HierSubzonePath[subzone_level][path_count] = tail_node->SubzoneID;
				tail_node = &HierNodePool[tail_node->ParentIndex];
				path_count--;
			}

			HierSubzonePath[subzone_level][0] = tail_node->SubzoneID;
		}
	}
	return(true);
}


/// <summary>
/// Finds a path between two cells for the specified object.
/// This is the way into the pathfinder. A hierarchical subzone search runs first to narrow
/// the search down to a corridor, then the regular cell level search walks it. Should the
/// regular search fail, the corridor links it choked on are banned and the pair is retried
/// before the request is given up on.
/// </summary>
/// <param name="moves">Buffer that receives the facing commands making up the path.</param>
/// <param name="max_loops">The most cells the search may expand, or -1 to leave it to the
/// pathfinder and allow retries.</param>
/// <param name="mzone">The movement zone to path within, or MZONE_NONE to use the object's
/// own.</param>
/// <param name="avoidance">How hard the path should try to dodge other moving objects.</param>
/// <returns>Returns with a pointer to the path found, or NULL if there is no route.</returns>
PathStruct * AStarClass::Find_Path(Cell const & from, Cell const & to, FootClass * foot, FacingType * moves, int max_loops, MZoneType mzone, ObstacleAvoidanceType avoidance)
{
	int i;

	IsHSEnabled = true;

	Clear();

	for (i = 0; i < ARRAY_SIZE(HierBannedEdges); i++) {
		HierBannedEdges[i].Clear();
	}

	Avoidance = avoidance;

	CellClass * from_ptr = &Map[from];
	CellClass * to_ptr = &Map[to];

	int from_zone = Map.Get_Cell_Zone(from, mzone != MZONE_NONE ? mzone : foot->TClass->MZone, foot->IsOnBridge);
	int to_zone = Map.Get_Cell_Zone(to,  mzone != MZONE_NONE ? mzone : foot->TClass->MZone, Map[to].IsUnderBridge);

	Cell hs_from = Map.Get_Bridge_Zone_Connection_Cell(from_ptr, foot->IsOnBridge);
	Cell hs_to = Map.Get_Bridge_Zone_Connection_Cell(to_ptr, to_ptr->IsUnderBridge);

	MZoneType move_zone = mzone != MZONE_NONE ? mzone : foot->TClass->MZone;

	if (foot->RTTI == RTTI_INFANTRY) {
		if (((InfantryClass *)foot)->Class->IsJumpJet) {
			move_zone = MZONE_INFANTRY;
		}
	}

	bool with_hs;
	if (foot->TClass->IsTrain) {
		/// Hierarchical search doesn't work on trains
		with_hs = false;
	} else if (!foot->IsLocked || foot->Is_Allowed_To_Leave_Map()) {
		/// Hierarchical search requires foot to be in the map and not allowed to leave it
		with_hs = false;
	} else if (!Map.In_Local_Radar(hs_from) || !Map.In_Local_Radar(hs_to)) {
		/// Hierarchical search isn't possible if cells are not inside the playable map
		with_hs = false;
	} else {
		with_hs = true;
	}

	if (from_zone != to_zone) {
		if (with_hs) {
			/// Hierarchical search requires the from and to zones to match
			return(NULL);
		}
	}

	if (with_hs) {
		if (!Find_Path_Hierarchical(hs_from, hs_to, move_zone, foot)) {
#ifdef _DEBUG
			DebugString("Hierarchical findpath failure: (%d,%d) to (%d, %d)\n", hs_from.X, hs_from.Y, hs_to.X, hs_to.Y);
#endif
			with_hs = false;
		}
	}

	PathStruct * path = NULL;

	i = 0;
	int tries = (max_loops == -1) ? 5 : 1;

	while (true) {

		if (!with_hs) {
			DebugString("Warning.  A* without HS: (%d,%d) to (%d, %d)\n", hs_from.X, hs_from.Y, hs_to.X, hs_to.Y);
		}

		path = Find_Path_Regular(from, to, foot, moves, max_loops, with_hs);
		if (path != NULL) {
			break;
		}

		/// Regular failed, if HS isn't enabled bail
		if (!with_hs) {
			break;
		}

		int dy = abs(from.Y - to.Y);
		int dx = abs(from.X - to.X);
		if (dx <= dy) {
			dx = dy;
		}

		if (dx > 1) {
			DebugString("Regular findpath failure: (%d,%d) to (%d, %d)\n", hs_from.X, hs_from.Y, hs_to.X, hs_to.Y);
		}

		i++;

		Ban_Blocked_Subzone_Edges(foot);
		Clear();

		with_hs = IsHSEnabled ? true : false;

		if (i >= tries) {
			break;
		}

		if (with_hs) {
			if (!Find_Path_Hierarchical(hs_from, hs_to, move_zone, foot)) {
				break;
			}
		}

	}

	return(path);
}


/// <summary>
/// Bans the subzone links that the regular search could not walk.
/// This routine is called after a regular search failure. The cell the search got furthest
/// to along the corridor is examined at every subzone level, and whichever links out of it
/// turned out to be unwalkable are banned so that the next attempt routes elsewhere.
/// </summary>
/// <param name="foot">The object the reachability test should be made for.</param>
void AStarClass::Ban_Blocked_Subzone_Edges(FootClass const * foot)
{
	CellSubzoneStruct & subzone = Map.CellSubzones[Map.Get_Cell_Zone_Index(HierLastNodeCell)];

	for (int subzone_level = 0; subzone_level < SUBZONE_COUNT; subzone_level++) {
		unsigned short from_subzone = subzone.SubzoneID[subzone_level];
		DynamicVectorClass<unsigned short> to_subzones;
		to_subzones.Clear();

		if (Map.Build_Reachable_Subzones(&Map[HierLastNodeCell], subzone_level, to_subzones, foot)) {
			CellSubzoneStruct & last_subzone = Map.CellSubzones[Map.Get_Cell_Zone_Index(HierLastNodeCell)];
			unsigned int subzone_id = (unsigned short)last_subzone.SubzoneID[subzone_level];
			Ban_Neighborhood_Subzone_Edges(subzone_id, subzone_level);
			continue;
		}

		for (int j = to_subzones.Count() - 1; j >= 0; j--) {
			Ban_Subzone_Edge(from_subzone, to_subzones[j], subzone_level);
		}
	}
}


/// <summary>
/// Has this subzone link been banned?
/// The hierarchical search consults this routine before expanding a link, so that a corridor
/// the regular search has already failed on is not offered up a second time.
/// </summary>
/// <param name="subzone_level">The coarseness level the link belongs to.</param>
/// <returns>bool; Is the link banned?</returns>
bool AStarClass::Subzone_Edge_Banned(unsigned short subzone1, unsigned short subzone2, int subzone_level)
{
	if (subzone2 < subzone1) {
		std::swap(subzone1, subzone2);
	}
	unsigned int edge = subzone2 | (subzone1 << 16);

	DynamicVectorClass<unsigned int> & edges = HierBannedEdges[subzone_level];
	for (int i = edges.Count() - 1; i >= 0; i--) {
		if (edges[i] == edge) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Adds a subzone link to the banned list.
/// This routine is used by the retry logic to keep the hierarchical search from proposing a
/// corridor that the regular search has already proven it cannot walk.
/// </summary>
/// <param name="subzone_level">The coarseness level the link belongs to.</param>
void AStarClass::Ban_Subzone_Edge(unsigned int subzone1, unsigned int subzone2, int subzone_level)
{
	if (subzone1 != subzone2) {
		if (subzone2 < subzone1) {
			std::swap(subzone2, subzone1);
		}
		unsigned int edge = subzone2 | (subzone1 << 16);
		HierBannedEdges[subzone_level].Add(edge);
	}
}


/// <summary>
/// Bans the corridor links around a subzone that proved impassable.
/// This routine is used when the regular search stalls inside a subzone rather than at a
/// link between two of them. The neighboring links are banned as well, so that the retry is
/// pushed onto a genuinely different route. Where no alternative is left, the hierarchical
/// search is switched off for the rest of the request.
/// </summary>
/// <param name="subzone">The subzone the search became stuck in.</param>
/// <param name="subzone_level">The coarseness level to work at.</param>
void AStarClass::Ban_Neighborhood_Subzone_Edges(unsigned int subzone, int subzone_level)
{
	int i;
	int j;

	int path_count = HierSubzonePathCount[subzone_level];
	if (path_count <= 1) {
		IsHSEnabled = false;
		return;
	}

	int path_index = -1;
	for (i = 0; i < path_count; i++) {
		if (HierSubzonePath[subzone_level][i] == subzone) {
			path_index = i;
			break;
		}
	}

	if (path_index == -1) {
		IsHSEnabled = false;
	} else {
		unsigned short path_node;
		unsigned short path_neighbor;
		if (path_index == path_count - 1) {
			path_node = HierSubzonePath[subzone_level][path_index];
			path_neighbor = HierSubzonePath[subzone_level][path_index - 1];
		} else {
			path_node = HierSubzonePath[subzone_level][path_index + 1];
			path_neighbor = HierSubzonePath[subzone_level][path_index];
		}

		Ban_Subzone_Edge(path_node, path_neighbor, subzone_level);

		DynamicVectorClass<SubzoneConnectionStruct> & node_conn = Map.SubzoneTracking[subzone_level][path_node].Connections;
		DynamicVectorClass<SubzoneConnectionStruct> & neighbor_conn = Map.SubzoneTracking[subzone_level][path_neighbor].Connections;

		for (j = node_conn.Count() - 1; j >= 0; j--) {
			unsigned short common_subzone = node_conn[j].SubzoneID;
			if (common_subzone != path_neighbor) {
				for (i = neighbor_conn.Count() - 1; i >= 0; i--) {
					if (neighbor_conn[i].SubzoneID == common_subzone) {
						Ban_Subzone_Edge(path_neighbor, common_subzone, subzone_level);
					}
				}
			}
		}
	}
}


/// <summary>
/// Determines how far an object would have to walk between two cells.
/// This routine runs the hierarchical subzone search alone and measures the corridor it
/// comes up with, which is a good deal cheaper than building a real path. Use it where the
/// caller only needs to compare distances, or to learn whether the destination can be
/// reached at all.
/// </summary>
/// <param name="foot">The object that would do the walking. May be NULL.</param>
/// <param name="from_bridge">Is the starting cell on a bridge deck?</param>
/// <param name="to_bridge">Is the destination cell on a bridge deck?</param>
/// <param name="mzone">The movement zone to test with, or MZONE_NONE to use the object's
/// own.</param>
/// <returns>Returns with the estimated walking distance in cells. If there is no route,
/// INT_MAX is returned.</returns>
int AStarClass::Test_Cell_Walk(Cell const & from, Cell const & to, FootClass const * foot, bool from_bridge, bool to_bridge, MZoneType mzone)
{
	IsHSEnabled = true;
	Clear();

	for (int i = 0; i < ARRAY_SIZE(HierBannedEdges); i++) {
		HierBannedEdges[i].Clear();
	}

	CellClass * from_ptr = &Map[from];
	CellClass * to_ptr = &Map[to];

	Cell hs_from = Map.Get_Bridge_Zone_Connection_Cell(from_ptr, from_bridge);
	Cell hs_to = Map.Get_Bridge_Zone_Connection_Cell(to_ptr, to_bridge);

	if (mzone == MZONE_NONE) {
		if (foot) {
			mzone = foot->TClass->MZone;
		} else {
			mzone = MZONE_NORMAL;
		}
	}

	if (Find_Path_Hierarchical(hs_from, hs_to, mzone, foot)) {
		int radius = std::max(abs(from.X - to.X), abs(from.Y - to.Y));	/// The Chebyshev distance between the cells
		int node_count = HierSubzonePathCount[SUBZONE_FINE];		/// Number of nodes in the finest level path graph
		int distance = 2 * node_count - 2;							/// Number of edges between these nodes

		bool add_bridge_dist = true;
		if (to_bridge && from_bridge) {
			int to_conn = Map.Zone_Connection_Index(to, 3, 0);
			int from_conn = Map.Zone_Connection_Index(from, 3, 0);
			if (to_conn == from_conn && to_conn != -1) {
				add_bridge_dist = false;
			}
		}

		if (add_bridge_dist) {
			if (to_bridge) {
				Cell bridge_end = CELL_NONE;
				if (node_count >= 4) {
					bridge_end = Map.Find_Bridge_End_Cell_For_Subzone(to, SUBZONE_FINE, HierSubzonePath[SUBZONE_FINE][node_count - 2]);
				}
				if (node_count < 4 || bridge_end == CELL_NONE) {
					bridge_end = Map.Find_Bridge_End_Cell_For_Subzone(to, SUBZONE_FINE, HierSubzonePath[SUBZONE_FINE][node_count - 1]);
				}
				if (bridge_end == CELL_NONE) {
					bridge_end = hs_to;
				}
				if (bridge_end != CELL_NONE) {
					distance += std::max(abs(to.X - bridge_end.X), abs(to.Y - bridge_end.Y));
				}
			}

			if (from_bridge) {
				Cell bridge_end = CELL_NONE;
				if (node_count >= 4) {
					bridge_end = Map.Find_Bridge_End_Cell_For_Subzone(from, 0, HierSubzonePath[SUBZONE_FINE][1]);
				}
				if (node_count < 4 || bridge_end == CELL_NONE) {
					bridge_end = Map.Find_Bridge_End_Cell_For_Subzone(from, 0, HierSubzonePath[SUBZONE_FINE][0]);
				}
				if (bridge_end == CELL_NONE) {
					bridge_end = hs_from;
				}
				if (bridge_end != CELL_NONE) {
					distance += std::max(abs(from.X - bridge_end.X), abs(from.Y - bridge_end.Y));
				}
			}

			if (distance > radius) {
				return(distance);
			}
		}

		return(radius);
	}
	return(INT_MAX);
}
