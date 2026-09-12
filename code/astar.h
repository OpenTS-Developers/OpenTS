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

#pragma once

#include "_astar.h"
#include "coord.h"
#include "nodes.h"
#include "priority.h"
#include "vector.h"

#include <optional>
#include <set>
#include <utility>
#include <vector>

#include "facing.hh"
#include "move.hh"
#include "mzone.hh"
#include "zone.hh"

class CellClass;
class FootClass;
class TechnoClass;
template<class T> class DynamicVectorClass;
template<class T> class TRect;
typedef TRect<int> Rect;

class AStarClass
{
	public:
		enum ObstacleAvoidanceType {
			AVOIDANCE_NONE, /// Moving obstacles are ignored
			AVOIDANCE_SOFT, /// Moving obstacles are preferred to be routed around
			AVOIDANCE_HARD, /// Moving obstacles have to be routed around
		};

		/*-----------------------------------------------------------------------------------
		**	Constructor & destructors.
		*/
		AStarClass(void);
		~AStarClass(void);

		/* -----------------------------------------------------------------------------------
		 * Main path finding routine
		 */
		PathStruct * Find_Path(Cell const & from, Cell const & to, FootClass * foot, FacingType * moves, int max_loops, MZoneType mzone, ObstacleAvoidanceType avoidance);

		/* -----------------------------------------------------------------------------------
		 * Shared / infrastructure
		 */
		void Clear(void);
		void Reset(void);
		void Update_Map_Dimensions(Rect const & dimensions);

		/* -----------------------------------------------------------------------------------
		 * Query / estimation (non-path-building)
		 */
		int Test_Cell_Walk(Cell const & from, Cell const & to, FootClass const * foot, bool from_bridge, bool to_bridge, MZoneType mzone);

	private:
		struct RegularNode {
			CellClass ** CellSlot; /// Slot in Map.Array holding this node's cell

			/*
			 * This is the elevation this node stands at -- the cell's own height, plus the
			 * bridge deck offset when the node is on the bridge rather than under it.
			 */
			int CellHeight;

			/*
			 * Pointer to the node this one was reached from, or NULL for the starting cell.
			 * The finished route is recovered by walking the chain back from the destination.
			 */
			RegularNode * Parent;
		};

		struct RegularOpenNode {
			/*
			 * Pointer to the search node this open set entry stands for. Entries are copied
			 * by value, so the cell and its parent link live in the shared node record.
			 */
			RegularNode * Node;

			/*
			 * This is the total cost of the steps taken to reach this node from the cell the
			 * search started at.
			 */
			float MovementCost;

			/*
			 * This is the MovementCost plus a straight line estimate of the distance left to
			 * run, and it is the value the open set orders its entries by.
			 */
			float Score;

			/*
			 * This is the number of cells on the route to this node, the starting cell
			 * included. It becomes the length of the finished path.
			 */
			int PathLength;

			RegularOpenNode(void)
			{
				PathLength = 0;
				Score = 0;
				MovementCost = 0;
			}

			bool operator==(const RegularOpenNode & other) const { return((double)Score == (double)other.Score); }
			bool operator!=(const RegularOpenNode & other) const { return((double)Score != (double)other.Score); }
			bool operator<(const RegularOpenNode & other) const { return((double)Score < (double)other.Score); }
			bool operator>(const RegularOpenNode & other) const { return((double)Score > (double)other.Score); }
			bool operator<=(const RegularOpenNode & other) const { return((double)Score <= (double)other.Score); }
			bool operator>=(const RegularOpenNode & other) const { return((double)Score >= (double)other.Score); }
		};

		struct RegularNodePool {
			/*
			 * These are all the search nodes one path attempt may use. They are handed out
			 * in order and never individually released.
			 */
			RegularNode Nodes[131072];

			/*
			 * This is the number of nodes handed out so far, and so the index the next one
			 * comes from. Emptying the pool is a matter of zeroing it.
			 */
			int ActiveCount;
			RegularNodePool(void) : ActiveCount(0) {}
		};

	private:
		/* ----------------------------------------------------------------------------------
		 * Regular pathfinder (cell-level A*)
		 */
		PathStruct * Find_Path_Regular(Cell const & from, Cell const & to, FootClass * foot, FacingType * moves, int max_loops, bool with_hs);
		RegularOpenNode Create_Node(std::optional<RegularOpenNode> const & parent, CellClass ** cell, Cell const & to, float movement_cost);
		bool Is_Visited(int, bool base_level, int index);
		double Get_Movement_Cost(CellClass ** from, CellClass ** to, bool bridge, MoveType move, FootClass * foot);
		void Apply_Path_Collision_Avoidance(FootClass * foot);
		FootClass * Find_Moving_Blocker(Cell const & cell, int cell_height);
		PathStruct * Build_Final_Path(RegularOpenNode const & final_node, FacingType * moves);
		void Cut_Corners(PathStruct * path, FootClass * foot);
		int Try_Diagonal_Shortcut(FootClass * foot, FacingType * moves, unsigned int * heights, int initial_first_leg_length, int initial_second_leg_length, Cell & cell);
		void Optimize_Moves(PathStruct * path, FootClass * foot);
		void Splice_Path(FacingType * moves, int start_index, int end_index, int & splice_index, Cell & cell);
		bool Plot_Straight_Line(FacingType * moves, int move_count, Cell const & from, Cell const & to, FootClass * foot, int cell_height, bool fearless);

		/* ----------------------------------------------------------------------------------
		 * Hierarchical pathfinder (subzone-level A*)
		 */
		bool Find_Path_Hierarchical(Cell const & from, Cell const & to, MZoneType mzone, FootClass const * foot);
		void Ban_Blocked_Subzone_Edges(FootClass const * foot);
		bool Subzone_Edge_Banned(int subzone1, int subzone2, int subzone_level);
		void Ban_Subzone_Edge(int subzone1, int subzone2, int subzone_level);
		void Ban_Neighborhood_Subzone_Edges(int subzone, int subzone_level);

	private:
		/* -----------------------------------------------------------------------------------
		 * Regular pathfinding state.
		 */

		/// Unused
		bool field_0;

		/*
		 * If bridges are to be shunned, then this flag will be true. A step onto one costs
		 * several times the usual amount, so a longer route along the ground wins out.
		 */
		bool IsAvoidBridges;

		/// Unused
		bool field_2;

		/*
		 * If paths are to be kept from crossing one another, then this flag will be true. The
		 * cells that slower objects in the way are about to walk through are priced up, so a
		 * faster object routes around them instead of queueing behind.
		 */
		bool IsAvoidPathCollision;

		/*
		 * This scales the cost of every step the search prices, before the small per facing
		 * tie-breaker is added on. Raising it makes terrain weigh heavier than distance.
		 */
		float MovementCostMultiplier;

		/*
		 * If the object's locomotor is to have a say in whether a cell may be entered, then
		 * this flag will be true. With it false only the terrain is consulted, so a route may
		 * be plotted through cells the object could not actually travel into.
		 */
		bool UseLocomotorEnterCheck;

		/*
		 * This is the pool of cell and parent records the finished route is recovered from.
		 * The open set entries handed to the RegularQueue name these rather than holding a
		 * cell of their own.
		 */
		RegularNodePool * RegularNodes;

		/*
		 * This is the open set for the cell level search, ordered by score so that the most
		 * promising cell reached so far is always the next one expanded.
		 */
		PriorityQueueClass<RegularOpenNode> * RegularQueue;

		/*
		 * This holds the UniqueID of the search that last visited each cell at ground level.
		 * Stamping the cells rather than clearing the table is what makes a search cheap to
		 * start.
		 */
		int * RegularVisited;

		/*
		 * This is the RegularVisited table's counterpart for cells reached up on a bridge
		 * deck. A cell may therefore be visited twice, once at each elevation.
		 */
		int * RegularBridgeVisited;

		/*
		 * This is the RegularMovementCosts table's counterpart for cells reached up on a
		 * bridge deck, which have to be costed separately from the ground beneath them.
		 */
		float * RegularBridgeMovementCosts;

		/*
		 * This is the cost of reaching each cell at ground level as of its last visit. An
		 * already visited cell is expanded again only if it can be reached for less.
		 */
		float * RegularMovementCosts;

		/*
		 * This is the stamp written into the visit and cost tables by the current search.
		 * Bumping it invalidates every table at a stroke, so they only have to be cleared
		 * for real on the rare occasion that it wraps around.
		 */
		int UniqueID;

		/*
		 * This records the speed of the object the current path is being built for, taken
		 * from its type as the search starts.
		 */
		int ObjectSpeed;

		/*
		 * This is the elevation the node being expanded stands at. It decides whether a
		 * neighboring cell is entered at ground level or up on its bridge deck.
		 */
		int CurrentCellHeight;

		/*
		 * This is the elevation the destination must be reached at -- the deck height when the
		 * cell is spanned by a bridge. Arriving at any other elevation is not arriving.
		 */
		int DestCellHeight;

		/*
		 * If the regular search is to be confined to the hierarchical corridor, then this flag
		 * will be true. It is cleared when the corridor proves unwalkable and no alternative
		 * route is left, which lets the retry search the map unrestricted.
		 */
		bool IsHSEnabled;

		/*
		 * This specifies how hard the current request should try to route around objects that
		 * are themselves on the move. A blocking object costs four times the normal amount
		 * under AVOIDANCE_SOFT and a thousand times that under AVOIDANCE_HARD.
		 */
		ObstacleAvoidanceType Avoidance;

		/* -----------------------------------------------------------------------------------
		 * Hierarchical pathfinding state.
		 */

		/*
		 * These are the stamps marking the subzones that make up the finished corridor at
		 * each level. The next finer level and the regular search may expand only within it.
		 */
		int * HierOnPath[SUBZONE_COUNT];

		/*
		 * These are the stamps marking which subzones the current search has reached, one
		 * table per level of coarseness. An unstamped subzone has no cost recorded yet.
		 */
		int * HierOpened[SUBZONE_COUNT];

		/*
		 * These are the cheapest scores reached so far for every subzone, one table per
		 * level of coarseness. A subzone is reopened when it is reached below this cost.
		 */
		float * HierCosts[SUBZONE_COUNT];

		/*
		 * The nodes one level of the hierarchical search reaches, in the order it reached
		 * them. A node names its parent by its position here, so this outlives the queue
		 * entries taken from it and is emptied for every level.
		 */
		std::vector<AStarHierarchicalNode> HierNodePool;

		/*
		 * This is the open set for the hierarchical search, ordered by score so that the
		 * cheapest subzone reached so far is always the next one expanded.
		 */
		PriorityQueueClass<AStarHierarchicalNode> * HierQueue;

		/*
		 * This is how far the regular search has progressed along the fine subzone corridor,
		 * expressed as an index into HierSubzonePath. It advances a step each time the search
		 * enters the next subzone the corridor calls for.
		 */
		int HierNodeIndex;

		/*
		 * This is the cell where the regular search last advanced into the next subzone of
		 * the fine corridor. A failed search is presumed to have choked here, so this is
		 * where the retry logic looks for subzone links to ban.
		 */
		Cell HierLastNodeCell;

		/*
		 * Subzone graph edges found to be blocked during the current path attempt,
		 * per hierarchy level; the hierarchical search refuses to expand across
		 * these when it retries after a regular pathfinding failure
		 */
		std::set<std::pair<int, int>> HierBannedEdges[SUBZONE_COUNT];

		/*
		 * Ordered list of subzone IDs forming the hierarchical path per level
		 */
		int HierSubzonePath[SUBZONE_COUNT][500];

		/*
		 * Number of valid entries in each hierarchical subzone path
		 */
		int HierSubzonePathCount[SUBZONE_COUNT];
};

/* -----------------------------------------------------------------------------------
 * File-scope helpers
 */
Cell Follow_Path(Cell const & cell, int count, FacingType const * path);

int Map_Cell_Index(Cell const & cell);
int Map_Cell_Count(void);

extern int AStarFacingToOffset[FACING_COUNT];
