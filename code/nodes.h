/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "coord.h"

struct CellNode {
	/*
	 * This is the cell that the node stands for. The map generator's spreading algorithms
	 * take nodes off the queue in score order and grow outward from this cell.
	 */
	Cell Element;

	/*
	 * This is the priority the node is queued under, the lowest scoring node always being
	 * the next one out. Every comparison operator weighs this alone, so the cell itself
	 * has no say in the ordering.
	 */
	float Score;

	CellNode(void) {}

	CellNode(Cell &cell) {
		Element = cell;
	}

	CellNode(Cell const & cell, float score) : Element(cell), Score(score) {}

	bool operator==(const CellNode & other) const { return((double)Score == (double)other.Score); }
	bool operator!=(const CellNode & other) const { return((double)Score != (double)other.Score); }
	bool operator<(const CellNode & other) const { return((double)Score < (double)other.Score); }
	bool operator>(const CellNode & other) const { return((double)Score > (double)other.Score); }
	bool operator<=(const CellNode & other) const { return((double)Score <= (double)other.Score); }
	bool operator>=(const CellNode & other) const { return((double)Score >= (double)other.Score); }

	// Carries the node to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(Element);
		stream.Serialize(Score);
	}
};



struct AStarHierarchicalNode {
	/*
	 * This is the node's own slot in the pool the search hands nodes out from, and it is
	 * what the nodes reached through this one record as their ParentIndex.
	 */
	int PoolIndex;

	/*
	 * This is the index within the node pool of the node that this one was reached from,
	 * or -1 for the node the search started at. When the destination is reached, the
	 * corridor is recovered by following these back to the start.
	 */
	int ParentIndex;

	/*
	 * This is the subzone that the node stands for. Every level of the hierarchy numbers
	 * its subzones separately, so this means nothing without the level being searched.
	 */
	int SubzoneID;

	/*
	 * This is the accumulated cost of reaching the subzone -- its passability, the threat
	 * the house sees along the way, and a small penalty for crossing a block boundary.
	 * The queue is ordered by it, cheapest first.
	 */
	float Score;

	/*
	 * This is the number of subzones between this node and the start of the search. It
	 * gives the length of the finished corridor without having to walk the parent chain
	 * to count it.
	 */
	int Depth;

	AStarHierarchicalNode(void) {}

	bool operator==(const AStarHierarchicalNode & other) const { return((double)Score == (double)other.Score); }
	bool operator!=(const AStarHierarchicalNode & other) const { return((double)Score != (double)other.Score); }
	bool operator<(const AStarHierarchicalNode & other) const { return((double)Score < (double)other.Score); }
	bool operator>(const AStarHierarchicalNode & other) const { return((double)Score > (double)other.Score); }
	bool operator<=(const AStarHierarchicalNode & other) const { return((double)Score <= (double)other.Score); }
	bool operator>=(const AStarHierarchicalNode & other) const { return((double)Score >= (double)other.Score); }

};
