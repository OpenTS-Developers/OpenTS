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

#include "coord.h"
#include "vector.h"

#include "passblty.hh"

template<typename K, typename V>
class HashTableClass;

/**********************************************************************
**	A base is broken up into several zones. This type enumerates the
**	various zones.
*/
enum ZoneType {
	ZONE_NONE=-1,

	ZONE_CORE,			// Center of base.
	ZONE_NORTH,			// North section.
	ZONE_EAST,			// East section.
	ZONE_SOUTH,			// South section.
	ZONE_WEST,			// West section.

	ZONE_COUNT,
	ZONE_FIRST=0,
};


enum SubzoneLevelType {
	SUBZONE_FINE,
	SUBZONE_ROUGH,
	SUBZONE_COARSE,
	SUBZONE_COUNT
};


/*
 * TERRAIN ZONES, SUBZONES AND CONNECTIONS
 *
 * The map keeps a three layer model of which parts of the terrain can reach which other
 * parts. It is there to answer "can this object ever get there?" without running a path,
 * and to hand the hierarchical pathfinder a small graph to search in place of the tens of
 * thousands of cells the playfield actually holds. Do not confuse any of this with the
 * ZoneType above, which carves up a player's base and has nothing to do with terrain.
 *
 * 1> ZONES. Zone_Reset flood fills the playfield into zones, recording each cell's zone in
 *    CellZones. A zone is a maximal 8 connected region of cells that share one passability
 *    (PassabilityType, mirrored from the terrain by CellClass::Recalc_Passability) and lie
 *    within a single height step of each other, so a zone stops at a shoreline, at a wall,
 *    and at the foot of a cliff. Zone 0 means the cell lies off the playfield.
 *
 *    By itself a zone only says "these cells are alike". What makes it useful is the pass
 *    that follows: touching zones are noted in ZoneAdjacency, then for each style of
 *    movement (MZoneType) the zones that style can cross are unioned across that adjacency
 *    into movement zones, one per connected component, kept in Zones[mzone]. Two cells are
 *    mutually reachable by a movement style only when their zones map to the same movement
 *    zone, which is the whole of what Get_Cell_Zone and Is_Same_Cell_Zone look up. This is
 *    how a stretch of land and the water beside it remain two zones for a tank while
 *    merging into one movement zone for a hovercraft.
 *
 * 2> CONNECTIONS. Bridges and tunnels join zones that the terrain alone does not.
 *    Compute_Zone_Connections records one ZoneConnectionClass per bridge span and per
 *    tunnel pair in ZoneConnections, and each one is fed into the union above as though its
 *    two ends were neighboring cells -- but only while the connection is passable.
 *    Destroying or repairing a span toggles that flag along with the connection's subzone
 *    edges, so dropping a bridge really does cut the two banks apart for anything that has
 *    to walk between them.
 *
 * 3> SUBZONES. A movement zone can cover half the map, which is far too coarse to steer by,
 *    so the playfield is divided again at three levels (SubzoneLevelType). A subzone is a
 *    run of same zone, smooth height cells penned into one aligned block of 2x2 (fine),
 *    4x4 (rough) or 8x8 (coarse) cells. The pen is the point -- it puts a ceiling on how
 *    much ground one graph node may stand for. Each cell's subzone at each level lives in
 *    CellSubzones, and each subzone's own record -- its neighbors, its parent subzone at
 *    the next coarser level, its passability and its threat region -- lives in
 *    SubzoneTracking.
 *
 *    AStarClass walks this graph from coarse to fine, each level permitted to expand only
 *    those subzones whose parent lay on the route the level above settled for. What falls
 *    out is a corridor of fine subzones that the cell level search must then stay inside,
 *    and that is what keeps a path from one corner of the map to the other from having to
 *    consider every cell in between.
 *
 * Both layers are built from scratch by Fresh_Map and are patched in place whenever terrain
 * passability changes (Update_Cell_Zone, Update_Cell_Subzones), falling back to a full
 * rebuild when a change leaves the local topology too tangled to patch. The pathfinder
 * keeps its own bookkeeping sized to these tables, so it is reset whenever they are.
 */


struct ZoneConnectionClass
{
	public:
		ZoneConnectionClass(void) : From(0,0), To(0,0), IsPassable(false), Type(-1) {}
		ZoneConnectionClass(ZoneConnectionClass &that) : From(that.From), To(that.To), IsPassable(that.IsPassable), Type(that.Type) {}

		bool operator==(const ZoneConnectionClass & that) const { return(From == that.From && To == that.To); }
		bool operator!=(const ZoneConnectionClass & that) const { return(From != that.From || To != that.To); }

		// Carries the connection to or from a save game.
		template<typename S>
		void Serialize(S & stream)
		{
			stream.Serialize(From);
			stream.Serialize(To);
			stream.Serialize(IsPassable);
			stream.Serialize(Type);
		}

	public:
		/*
		 * These are the two cells the connection joins -- the deck cells at either end of
		 * a bridge span, or a tunnel's two entrances with the lower numbered cell first.
		 */
		Cell From;
		Cell To;

		/*
		 * If the connection can be crossed -- for a bridge span, that its deck is intact --
		 * then this flag will be true; it is always true for a tunnel. A connection links
		 * its zones only while this is set, so a broken span cuts the two sides apart.
		 */
		bool IsPassable;

		/*
		 * This specifies the kind of connection: 0 = bridge span, 1 = tunnel, -1 = none.
		 */
		int Type;
};


/*
 * This struct defines the zone that the cell belongs to.
 */
struct CellZoneStruct
{
	/*
	 * This is the passability of the cell, mirrored here by CellClass::Recalc_Attributes.
	 * Cells only join a zone when their passability matches, so one value serves the zone.
	 */
	unsigned char Passability;

	/*
	 * This is the ground level height of the cell. A zone spreads only across a height
	 * step of less than two, so a zone stops at the foot of a cliff.
	 */
	unsigned char Height;

	/*
	 * This is the base terrain zone the cell was flood filled into, or 0 for a cell that
	 * lies outside the playfield. It indexes the Zones layers to give the movement zone.
	 */
	unsigned short ZoneID;

	CellZoneStruct(void) : Passability(PASSABLE_OUTSIDE), Height(0) {}
};


/*
 * This struct defines the subzone that the cell belongs to.
 */
struct CellSubzoneStruct
{
	/*
	 * This is the subzone the cell belongs to at each level of pathfinding coarseness
	 * (SubzoneLevelType), or 0 where the cell has none at that level.
	 */
	signed short SubzoneID[SUBZONE_COUNT];

	/*
	 * This is the base terrain zone of the cell, cached from CellZones on each rebuild. A
	 * subzone never spans two zones, so the fill spreads only while this value matches.
	 */
	signed short ZoneID;

	/*
	 * This is the ground level height of the cell, cached alongside its zone. A subzone
	 * spreads only across a height step of less than two, just as a zone does.
	 */
	unsigned char Height;

	/// Unused
	char Unused1;
};


/*
 * This struct defines the connection to a subzone.
 */
struct SubzoneConnectionStruct
{
	SubzoneConnectionStruct(void) : SubzoneID(0), IsCrossBlock(false) {}
	SubzoneConnectionStruct(int subzone_id) : SubzoneID(subzone_id), IsCrossBlock(false) {}
	SubzoneConnectionStruct(SubzoneConnectionStruct const &that) : SubzoneID(that.SubzoneID), IsCrossBlock(that.IsCrossBlock) {}

	bool operator==(const SubzoneConnectionStruct & that) const { return(SubzoneID == that.SubzoneID); }
	bool operator!=(const SubzoneConnectionStruct & that) const { return(SubzoneID != that.SubzoneID); }

	/*
	 * This is the subzone that the owning subzone connects to. While a connection is still
	 * staged in the SubzoneConnectionHashTable this holds both IDs packed together instead, as
	 * (neighbor << 16) | subzone, so that duplicate pairs fall out of the staging set before
	 * they are unpacked into the two subzones' adjacency lists.
	 */
	int SubzoneID;

	/*
	 * If the two subzones lie in different fill blocks, then this flag will be true. The
	 * hierarchical pathfinder charges such an edge a slight extra cost, which breaks ties in
	 * favor of a route that stays within a block.
	 */
	bool IsCrossBlock;
};


/*
 * This struct tracks a subzone's relations.
 */
struct SubzoneTrackingStruct
{
	SubzoneTrackingStruct(void)
	{
		Connections.Clear();
		ParentSubzoneID = 0;
		Passability = PASSABLE_LAND;
		ThreatRegion = 0;
	}

	bool operator==(const SubzoneTrackingStruct & that) const { return(ParentSubzoneID == that.ParentSubzoneID && Passability == that.Passability); }
	bool operator!=(const SubzoneTrackingStruct & that) const { return(ParentSubzoneID != that.ParentSubzoneID || Passability != that.Passability); }

	/*
	 * These are the subzones adjacent to this one at its own level of coarseness. Every
	 * link appears in both lists, so the graph the hierarchical search walks is undirected.
	 */
	DynamicVectorClass<SubzoneConnectionStruct> Connections;

	/*
	 * This is the subzone at the next coarser level that contains this one, or 0 at the
	 * coarsest level of all. The hierarchical search runs from coarse to fine and will only
	 * expand a subzone whose parent lay on the route the coarser level settled on, which is
	 * what keeps a long path cheap to find.
	 */
	unsigned short ParentSubzoneID;

	/*
	 * This is the passability shared by all of this subzone's cells -- a subzone never spans
	 * more than one zone, and a zone has uniform passability. This one value therefore
	 * decides whether a given style of movement may enter the subzone at all.
	 */
	PassabilityType Passability;

	/*
	 * This is the threat map region (an index into HouseClass::Regions) that this subzone
	 * sits in, taken from the cell it was grown from. The hierarchical pathfinder charges
	 * that region's threat against the cost of entering the subzone, which is how a route
	 * comes to steer around dangerous ground.
	 */
	int ThreatRegion;
};

typedef HashTableClass<unsigned int, unsigned int> ZONE_PAIR_HASH_SET;
typedef HashTableClass<unsigned int, SubzoneConnectionStruct> SUBZONE_CONNECTION_HASH_SET;
