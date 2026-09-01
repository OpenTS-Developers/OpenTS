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

#include "mapgen.h"

#include "_map.h"
#include "_palette.h"
#include "_rules.h"
#include "_tactica.h"
#include "animtype.h"
#include "astar.h"
#include "building.h"
#include "builtype.h"
#include "ccrand.h"
#include "cell.h"
#include "conquer.h"
#include "coord.h"
#include "data.h"
#include "dbgprint.h"
#include "gamedirs.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "init.h"
#include "inline.h"
#include "isotype.h"
#include "language\language.h"
#include "netshare.h"
#include "nodes.h"
#include "overtype.h"
#include "ownrdraw.h"
#include "pcx.h"
#include "progress.h"
#include "rules.h"
#include "smartdeform.h"
#include "smudtype.h"
#include "tactical.h"
#include "tagtype.h"
#include "terrain.h"
#include "terrtype.h"
#include "tiberium.h"
#include "trigtype.h"
#include "unit.h"
#include "unittype.h"
#include "vector.h"
#include "vein.h"
#include "wdtnet.h"
#include "winfix.h"
#include "worlddom.h"

#include "ramp.hh"

#include <algorithm>
#include <deque>


bool (*RMGCallback)() = MapGen_Call_Back;

BOOL CALLBACK Map_Seed_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);


double Random_Fraction(void);
unsigned int Pick_Random_UInt(unsigned int minval, unsigned int maxval);
DynamicVectorClass<Cell> * Pick_Spread_Cells(DynamicVectorClass<Cell> & cells);

MapGeneratorClass RandomMapGen;

Random2Class RMGRandom;
#define RMG_RANDOM_DOUBLE(max_value) ((unsigned int)RMGRandom() * (double)(max_value)) * (1.0 / UINT_MAX)
#define RMG_RANDOM_DOUBLE2(max_value) ((unsigned int)RMGRandom() * (1.0 / UINT_MAX)) * (double)(max_value)

double Sample_Normal(double mean, double standard_dev);



struct DirtRoadLink {
	DirtRoadLink(void);
	DirtRoadLink(Cell const & cell, FacingType dir);
	DirtRoadLink(DirtRoadLink const & that);

	Cell Offset;
	FacingType Facing;
};


struct DirtRoadTile {
	DirtRoadTile(DirtRoadLink const & link1);
	DirtRoadTile(DirtRoadLink const & link1, DirtRoadLink const & link2);
	DirtRoadTile(DirtRoadLink const & link1, DirtRoadLink const & link2, DirtRoadLink const & link3);
	DirtRoadTile(DirtRoadLink const & link1, DirtRoadLink const & link2, DirtRoadLink const & link3, DirtRoadLink const & link4);
	DirtRoadTile(void);
	~DirtRoadTile(void);

	int Count;
	DirtRoadLink * Links;
	IsometricTileType ITType;
};


struct DirtRoadNode {
	DirtRoadNode(void);
	DirtRoadNode(IsometricTileType road_tile, Cell const & origin, IsometricTileType cap_tile, Cell const & cap_origin, int link_index);
	DirtRoadNode(DirtRoadNode const & that);

	IsometricTileType Tile;
	Cell Origin;
	IsometricTileType CapTile;
	Cell CapOrigin;
	int BackLinkIndex;
};


extern DirtRoadTile DirtRoadTiles[];


/// Gaussian (normal) random number generator using the Marsaglia polar form of the Box-Muller transform.
class NormalDistribution
{
	public:
		NormalDistribution(double (*function)(void));
		double operator() (void);

		bool HasSpare;
		double Spare;
		double (*Rand)(void);
};


NormalDistribution RMGGaussian(Random_Fraction);

int MapRegionClass::MapStartDiagonal;
int MapRegionClass::MapEndDiagonal;

MapRegionClass::CellData *RMGCellData;

int MapRegionClass::TotalCount;

DynamicVectorClass<MapRegionClass *> MapRegionClass::MapRegions;
DynamicVectorClass<Cell> * TiberiumLayoutCells;


/// <summary>
/// Creates a cleared working record for a map cell.
/// This routine puts the cell into the state the random map generator expects to find it in
/// before any region has laid claim to it -- unowned, flat, unforested and with no tile
/// placed on it yet.
/// </summary>
MapRegionClass::CellData::CellData(void) :
	CellID(0,0),
	HillHeight(0),
	HillVolatility(0),
	GreenTileChance(0),
	SandTileChance(0),
	RoughTileChance(0),
	ForestChance(0),
	RegionID(0),
	SpreadID(0),
	WaterMask(-1),
	Interior(false),
	Inviolate(false),
	TilePlaced(false),
	Forested(false),
	Iced(false),
	InFillArea(false),
	InFillReach(true),
	CanExpand(false)
{
	//nothing
}

void Init_Dirt_Roads(void);
int Dirt_Road_Tile_Index(IsometricTileType ittype);


/// <summary>
/// Fetches a random cell from anywhere on the map.
/// Slots the generator never filled in are passed over, so the cell handed back is
/// always a real one.
/// </summary>
/// <returns>Returns with the cell picked, chosen uniformly from the mapped cells.</returns>
Cell MapRegionClass::Pick_Random_Map_Cell(void)
{
	int grid_size = MapCellStride * MapCellStride;
	unsigned int index;
	do {
		index = Pick_Random_UInt(0, grid_size - 1);
	} while (RMGCellData[index].CellID == Cell(0,0));
	return(RMGCellData[index].CellID);
}


/// <summary>
/// Fetches a random clear cell from anywhere on the map.
/// This is the map wide counterpart of Pick_Random_Clear_Cell, for callers that do not
/// care which region the cell falls in. The search gives up rather than blocking if the
/// map is congested.
/// </summary>
/// <returns>Returns with the clear cell found, or Cell(0,0) if the map has none to
/// spare.</returns>
Cell MapRegionClass::Pick_Random_Clear_Map_Cell(void)
{
	int tries = 0;
	while (true) {
		Cell cell = Pick_Random_Map_Cell();
		if (++tries > 200) {
			return(Cell(0,0));
		}
		if (Map[cell].Is_Tile_Clear()) {
			return(cell);
		}
	}
}


/// <summary>
/// Constructs a map region grown from a seed cell.
/// The region takes its terrain from the seed and joins the generator's list of regions,
/// which is what the map generator later grows, merges and scores.
/// </summary>
/// <param name="cell">The seed cell the region starts out from.</param>
MapRegionClass::MapRegionClass(Cell cell) :
	NeighborRegions(0),
	ID(TotalCount),
	CellCount(0),
	CellHeight(Map[cell].Height),
	ContainsWater(Map[cell].Is_Tile_With_Water()),
	Position(cell),
	Unsplittable(false),
	HasConnections(true),
	SplitCount(0),
	Discardable(false),
	Cells(),
	RegionBounds(0,0,0,0)
{
	Cells.Set_Growth_Step(100000);

	TotalCount++;
	MapRegions.Add(this);
}


/// <summary>
/// Destructor. Removes this region from the global MapRegions list.
/// </summary>
MapRegionClass::~MapRegionClass(void)
{
	MapRegions.Delete(this);
}


/// <summary>
/// Removes every map region the generator has built.
/// This routine wipes the slate between generation attempts, so that a fresh run starts
/// with no regions at all and its region numbering back at zero.
/// </summary>
void MapRegionClass::Delete_All_Regions(void)
{
	MapRegionClass::TotalCount = 0;

	if (TiberiumLayoutCells != NULL) {
		delete TiberiumLayoutCells;
		TiberiumLayoutCells = NULL;
	}

	for (int i = MapRegionClass::MapRegions.Count() - 1; i >= 0; i--) {
		MapRegionClass * region = MapRegionClass::MapRegions[i];
		if (region != NULL) {
			delete region;
		}
	}
}


/// <summary>
/// Determines if the generator may still alter this cell.
/// </summary>
/// <returns>bool; Is the cell still unprotected?</returns>
bool MapRegionClass::CellData::Is_Not_Inviolate(Cell const &cell)
{
	return(!MapRegionClass::Get_Cell_Data(cell).Inviolate);
}


/// <summary>
/// Determines if the cell lies well in from the edge of its region.
/// The interior flag is what keeps lake seeding off the fringe of the ground it grows on.
/// </summary>
/// <returns>bool; Does the cell lie in the interior?</returns>
bool MapRegionClass::CellData::Is_Interior(Cell const &cell)
{
	return(MapRegionClass::Get_Cell_Data(cell).Interior);
}


/// <summary>
/// Protects a cell from further alteration.
/// This routine is used to pin down a cell the generator has finished with, so that a
/// later pass leaves it alone.
/// </summary>
void MapRegionClass::CellData::Set_Inviolate(Cell const &cell)
{
	MapRegionClass::Get_Cell_Data(cell).Inviolate = true;
}


/// <summary>
/// Lifts the protection from a cell.
/// This routine is the counterpart of Set_Inviolate, and hands a pinned down cell back to
/// the passes that follow.
/// </summary>
void MapRegionClass::CellData::Clear_Inviolate(Cell const &cell)
{
	MapRegionClass::Get_Cell_Data(cell).Inviolate = false;
}


/// The loop body indexes with the bound `grid_size` rather than the loop variable, so
/// nothing within the grid is cleared and the write lands one slot past the end of it.

/// <summary>
/// Lifts the protection from every cell on the map.
/// This routine is used between generator passes, when cells that an earlier pass pinned
/// down are meant to become fair game again.
/// </summary>
void MapRegionClass::CellData::Clear_Inviolates(void)
{
	int grid_size = MapCellStride * MapCellStride;
	for (int i = 0; i < grid_size; i++) {
		MapRegionClass::Get_Cell_Data(grid_size).Inviolate = false;
	}
}


/// <summary>
/// Fetches the generator's working record for a cell.
/// Every cell carries one of these records for as long as the random map generator is
/// running. It is where region ownership, the terrain chances and the protection flag
/// live, and every other routine here reaches its cell through this one.
/// </summary>
/// <returns>Returns with a reference to the cell's working record.</returns>
MapRegionClass::CellData & MapRegionClass::Get_Cell_Data(Cell const & cell)
{
	return(RMGCellData[cell.X + cell.Y * MapCellStride]);
}


/// <summary>
/// Fetches the generator's working record for a grid slot.
/// This is the raw index overload, for callers that are already walking the cell data
/// grid rather than holding a cell.
/// </summary>
/// <param name="index">The slot within the cell data grid.</param>
/// <returns>Returns with a reference to the working record at that slot.</returns>
MapRegionClass::CellData & MapRegionClass::Get_Cell_Data(int index)
{
	return(RMGCellData[index]);
}


/// <summary>
/// Brings this region's cell tally back in step with the grid.
/// Regions hand cells back and forth as they grow, merge and split, which leaves the
/// running count stale. Use this routine to re-establish it.
/// </summary>
void MapRegionClass::Recount_Cells(void)
{
	CellCount = 0;
	int grid_size = MapCellStride * MapCellStride;

	for (int i = 0; i < grid_size; i++) {
		if (RMGCellData[i].RegionID == ID && RMGCellData[i].CellID != Cell(0,0)) {
			CellCount++;
		}
	}
}


/// <summary>
/// Fetches a random cell belonging to this region.
/// </summary>
/// <returns>Returns with the cell picked, chosen uniformly from the region's cells.</returns>
Cell MapRegionClass::Pick_Random_Cell(void)
{
	int grid_size = MapCellStride * MapCellStride;
	unsigned int index = 0;
	do {
		index = Pick_Random_UInt(0, grid_size - 1);
	} while (RMGCellData[index].RegionID != ID || RMGCellData[index].CellID == Cell(0,0));

	return(RMGCellData[index].CellID);
}


/// <summary>
/// Fetches a random clear cell from this region.
/// This routine is used when the generator needs somewhere unobstructed to place
/// something. The search gives up rather than blocking if the region is congested.
/// </summary>
/// <returns>Returns with the clear cell found, or Cell(0,0) if the region has none to
/// spare.</returns>
Cell MapRegionClass::Pick_Random_Clear_Cell(void)
{
	int tries = 0;
	while (true) {
		Cell cell = Pick_Random_Cell();
		if (++tries > 200) {
			return(Cell(0,0));
		}
		if (Map[cell].Is_Tile_Clear()) {
			return(cell);
		}
	}
}


/// The candidate is assigned through a reference bound to grid entry 0, so the search
/// scribbles over that entry as it runs and the cell handed back is read from it.

/// <summary>
/// Fetches a random clear cell from the interior of this region.
/// This routine is used where the generator must keep well away from the region's edges.
/// The search gives up rather than blocking if the interior is congested.
/// </summary>
/// <returns>Returns with the clear interior cell found, or Cell(0,0) if the region has
/// none to spare.</returns>
Cell MapRegionClass::Pick_Random_Clear_Interior_Cell(void)
{
	MapRegionClass::CellData & data = RMGCellData[0];
	int grid_size = MapCellStride * MapCellStride;
	int tries = 0;
	while (true) {
		data = RMGCellData[Pick_Random_UInt(0, grid_size - 1)];
		if (++tries >= 100) {
			return(Cell(0,0));
		}
		if (data.RegionID == ID && data.CellID != Cell(0, 0) && data.Interior) {
			if (Map[data.CellID].Is_Tile_Clear()) {
				break;
			}
		}
	}
	return(data.CellID);
}


/// <summary>
/// Fetches a random clear cell lying within a movement zone.
/// This routine is used when the generator needs somewhere unobstructed that is also
/// reachable -- a starting point, or anything that would be useless marooned. The search
/// gives up rather than blocking if nothing suitable turns up.
/// </summary>
/// <param name="zone">The movement zone the cell must lie within.</param>
/// <param name="region_id">The region the cell must belong to, or -1 to accept any region.</param>
/// <returns>Returns with the cell found, or Cell(0,0) if the zone has none to spare.</returns>
Cell MapRegionClass::Pick_Random_Clear_Cell_In_Zone(int zone, int region_id)
{
	MapRegionClass::CellData & data = RMGCellData[0];
	int grid_size = MapCellStride * MapCellStride;
	int tries = 0;
	Cell c(0,0);
	while (true) {
		data = RMGCellData[Pick_Random_UInt(0, grid_size - 1)];
		if (++tries >= 100) {
			return(c);
		}
		CellClass & cellref = Map[data.CellID];
		if (data.CellID != c
				&& Map.Get_Cell_Zone(cellref.CellID, MZONE_NORMAL, false) == zone
				&& cellref.Is_Tile_Clear()
				&& (region_id == -1 || data.RegionID == region_id)) {
			return(data.CellID);
		}
	}
	return(c);
}


/// <summary>
/// Clears the spread markers on every cell, back to -1.
/// Every routine that spreads out from a seed cell stamps its own id here to remember which
/// cells it has already reached. This routine wipes the grid clean for the next one.
/// Note that Clear_Cell_Data_Spreads clears to zero rather than to -1.
/// </summary>
void MapRegionClass::CellData::Reset_Spreads(bool)
{
	int grid_size = MapCellStride * MapCellStride;
	for (int i = 0; i < grid_size; i++) {
		RMGCellData[i].SpreadID = -1;
	}
}


/// <summary>
/// Scores a step against a preferred direction.
/// This routine is used to steer the wandering boundary that Split_Region grows. A dash
/// of randomness is mixed into the score so that the boundary does not come out looking
/// ruler-drawn.
/// </summary>
/// <param name="cell1">The cell the step starts from.</param>
/// <param name="cell2">The cell the step ends at.</param>
/// <param name="ref_angle">The preferred direction, in radians.</param>
/// <returns>Returns with the score. It rises with both the length of the step and how
/// far the step strays from the preferred direction, so the lowest-scoring step is the
/// one that suits best.</returns>
double MapRegionClass::Get_Angle_Score(const Cell & cell1, const Cell & cell2, double ref_angle)
{
	static const double _angle_weight = 1.5;
	static const double _length_weight = 0.15;

	int dx = cell2.X - cell1.X;
	int dy = cell2.Y - cell1.Y;

	double length = std::sqrt((double)(dx * dx + dy * dy));

	double angle = (dx != 0) ? std::atan(-((double)dy / (double)dx)) : M_PI_2;
	if (dx < 0) {
		angle += M_PI;
	}

	double angle_diff = fabs(angle - ref_angle);
	while (angle_diff >= M_PI * 2) {
		angle_diff -= M_PI * 2;
	}
	if (angle_diff > M_PI) {
		angle_diff = M_PI * 2 - angle_diff;
	}

	return(RMG_RANDOM_DOUBLE2(2.5) + _length_weight * length + _angle_weight * angle_diff);
}


/// <summary>
/// Creates a region grown out from a seed cell.
/// This routine gathers up every cell reachable from the seed that shares its height and
/// its wet or dry character, and makes a region of them. A dry patch too small to be
/// worth keeping is dissolved into its surroundings instead of becoming a region, and
/// the ground under it is flattened to match.
/// </summary>
/// <param name="cptr">The cell to grow the region out from.</param>
/// <returns>Returns with the region created. NULL is returned when the patch was
/// dissolved rather than kept.</returns>
MapRegionClass *MapRegionClass::Create_Region_From_Cell(CellClass *cptr)
{
	int i;
	int region_id = MapRegionClass::TotalCount;

	DynamicVectorClass<Cell> cells;
	cells.Set_Growth_Step(50000);
	cells.Add(cptr->Fetch_CellID());

	bool has_water = cptr->Is_Tile_With_Water() || cptr->Is_Tile_Swamp();
	int cell_height = cptr->Height;
	int processed_count = 0;

	while (cells.Count()) {
		Cell cell = cells[cells.Count() - 1];
		cells.Delete_Index(cells.Count() - 1);

		CellData::Set_Spread(cell, region_id);
		CellData::Set_Region(cell, region_id);

		Cell ncell;
		for (i = 0; i < FACING_COUNT; i++) {
			ncell = Adjacent_Cell(cell, (FacingType)i);
			CellData & ndata = MapRegionClass::Get_Cell_Data(ncell);
			if (My_In_Radar(ncell)) {
				if (ndata.RegionID == -1 && ndata.SpreadID != region_id) {
					ndata.SpreadID = region_id;
					if (Map[ncell].Height == cell_height) {
						int n_has_water = Map[ncell].Is_Tile_With_Water() || Map[ncell].Is_Tile_Swamp();
						if (!(has_water ^ n_has_water)) {
							cells.Add(ncell);
						}
					}
				}
			}
		}

		processed_count++;
	}

	if (processed_count < 75 && !has_water) {
		bool reset_done = false;

		if (region_id == 0) {
			DynamicVectorClass<Cell> * region_cells = RandomMapGen.Build_Region_Border_Cell_List(0);
			Cell region_cell = (*region_cells)[Pick_Random_UInt(0, region_cells->Count() - 1)];

			Cell ncell;
			for (i = 0; i < FACING_COUNT; i++) {
				ncell = Adjacent_Cell(region_cell, (FacingType)i);
				if (My_In_Radar(ncell)) {
					if (Map[ncell].Height != Map[region_cell].Height && !Map[ncell].Is_Tile_With_Water()) {
						int height = Map[ncell].Height;

						Map.Reset_Iterator();
						for (CellClass *iter = Map.Iterate(); iter; iter = Map.Iterate()) {
							CellData & idata = MapRegionClass::Get_Cell_Data(iter->Fetch_CellID());
							if (idata.RegionID == 0) {
								idata.RegionID = -1;
								idata.CanExpand = false;
								iter->ITType = ISOTILE_CLEAR;
								iter->SubTile = 0;
								iter->Height = (height == -1) ? RandomMapGen.CellHeight : height;
							}
						}

						reset_done = true;
						break;
					}
				}
			}

			delete region_cells;
		} else {
			Cell seed = cptr->Fetch_CellID();
			Cell side_cell;
			side_cell = Cell(seed.X - 1, seed.Y);
			if (!My_In_Radar(side_cell)) {
				side_cell = Cell(seed.X, seed.Y - 1);
			}

			if (My_In_Radar(side_cell)) {
				int id = CellData::Get_Region(side_cell);
				int height = Map[side_cell].Height;

				Map.Reset_Iterator();
				CellClass *iter = Map.Iterate();
				while (iter != NULL) {
					CellData & idata = MapRegionClass::Get_Cell_Data(iter->Fetch_CellID());
					if (idata.RegionID == region_id) {
						idata.RegionID = id;
						idata.CanExpand = false;
						iter->ITType = ISOTILE_CLEAR;
						iter->SubTile = 0;
						iter->Height = (height == -1) ? RandomMapGen.CellHeight : height;
					}
					iter = Map.Iterate();
				}

				reset_done = true;
			}
		}

		if (reset_done) {
			return(NULL);
		}
	}

	MapRegionClass * region = new MapRegionClass(cptr->Fetch_CellID());
	region->CellHeight = cell_height;
	region->ContainsWater = has_water;
	region->CellCount = 0;
	region->Recount_Cells();
	return(region);
}


/// <summary>
/// Discards every region on the map.
/// This routine strips the region assignments out of the generator's cell grid and
/// destroys the regions themselves. Use it before rebuilding the regions from scratch.
/// </summary>
void MapRegionClass::Destroy_Cell_Regions(void)
{
	if (RMGCellData != NULL) {
		int grid_size = MapCellStride * MapCellStride;
		for (int i = 0; i < grid_size; i++) {
			RMGCellData[i].RegionID = -1;
			RMGCellData[i].SpreadID = -1;
		}
	}

	for (int i = MapRegionClass::MapRegions.Count() - 1; i >= 0; i--) {
		MapRegionClass * region = MapRegionClass::MapRegions[i];
		if (region != NULL) {
			delete region;
		}
	}

	MapRegionClass::TotalCount = 0;
}


/// <summary>
/// Creates a region for every stretch of water.
/// This routine is run ahead of the land pass so that water and swamp are gathered up
/// first, leaving Create_Land_Regions to mop up whatever is left over.
/// </summary>
void MapRegionClass::Create_Water_Regions(void)
{
	int grid_size = MapCellStride * MapCellStride;

	for (int i = 0; i < grid_size; i++) {
		Cell cell = RMGCellData[i].CellID;
		CellClass *cptr = &Map[cell];
		if (RMGCellData[i].RegionID == -1 && RMGCellData[i].CellID != Cell(0,0)) {
			if (cptr->Is_Tile_With_Water() || cptr->Is_Tile_Swamp()) {
				Create_Region_From_Cell(cptr);
			}
		}
	}
}


/// <summary>
/// Creates regions out of the remaining land cells.
/// This routine is the companion to Create_Water_Regions and is run after it, so that
/// every cell the water pass left over ends up belonging to some region of land.
/// </summary>
/// <param name="unsplittable">Should the regions created be exempt from later splitting?</param>
void MapRegionClass::Create_Land_Regions(bool unsplittable)
{
	int grid_size = MapCellStride * MapCellStride;

	for (int i = 0; i < grid_size; i++) {
		if (RMGCellData[i].RegionID == -1 && RMGCellData[i].CellID != Cell(0,0)) {
			CellClass *cptr = &Map[RMGCellData[i].CellID];
			MapRegionClass *rgn = Create_Region_From_Cell(cptr);
			if (rgn != NULL) {
				rgn->Unsplittable = unsplittable;
			}
		}
	}
}


/// <summary>
/// Assigns a cell to a region.
/// This routine quietly does nothing when the generator's cell grid has not been
/// allocated.
/// </summary>
/// <param name="region_id">The region id to store.</param>
void MapRegionClass::CellData::Set_Region(Cell const &cell, int region_id)
{
	if (RMGCellData != NULL) {
		CellData & data = MapRegionClass::Get_Cell_Data(cell);
		data.RegionID = region_id;
	}
}


/// <summary>
/// Fetches the region that a cell belongs to.
/// </summary>
/// <returns>Returns with the cell's region id. -1 is returned if the generator's cell
/// grid has not been allocated.</returns>
int MapRegionClass::CellData::Get_Region(Cell const &cell)
{
	if (RMGCellData == NULL) {
		return(-1);
	}
	return(MapRegionClass::Get_Cell_Data(cell).RegionID);
}


/// <summary>
/// Marks a cell as reached by the spread currently running.
/// </summary>
/// <param name="cell">The cell to mark.</param>
/// <param name="patch_id">The id the spreading routine knows itself by.</param>
void MapRegionClass::CellData::Set_Spread(Cell const &cell, int patch_id)
{
	MapRegionClass::Get_Cell_Data(cell).SpreadID = patch_id;
}


/// <summary>
/// Fetches the spread marker for a cell.
/// A spreading routine tests this against its own id to tell whether it has already been over
/// the cell, so that it cannot double back on itself.
/// </summary>
/// <returns>Returns with the id of the spread that last reached the cell.</returns>
int MapRegionClass::CellData::Get_Spread(Cell const &cell)
{
	return(MapRegionClass::Get_Cell_Data(cell).SpreadID);
}


/// <summary>
/// Wipes this region's cells back to bare ground.
/// This routine hands every cell of the region over to another region and flattens the
/// tile underneath it, optionally setting a new height as it goes. Use it when a region
/// is being dissolved or merged away.
/// </summary>
/// <param name="new_id">The region the freed cells are handed to.</param>
/// <param name="cell_height">Height to set on each cell, or -1 to leave heights alone.</param>
void MapRegionClass::Clear_Cells(int new_id, int cell_height)
{
	int grid_size = MapCellStride * MapCellStride;

	for (int i = 0; i < grid_size; i++) {
		if (RMGCellData[i].RegionID == ID) {
			RMGCellData[i].RegionID = new_id;
			RMGCellData[i].CanExpand = false;
			Cell cell = RMGCellData[i].CellID;
			CellClass *cptr = &Map[cell];
			cptr->ITType = ISOTILE_CLEAR;
			cptr->SubTile = 0;
			if (cell_height != -1) {
				cptr->Height = cell_height;
			}
		}
	}
	CellCount = 0;
}


/// <summary>
/// Hands this region's outer cells over to another region.
/// This routine peels the region inward, a ring at a time, stamping every cell it takes
/// with the region id given. It is used both to shave a region back and, with a scratch
/// id, to work out how far in from the border a cell lies.
/// </summary>
/// <param name="rings">How many rings deep to peel.</param>
/// <param name="region_id">The region id stamped onto each cell taken.</param>
void MapRegionClass::Reassign_Border_Cells(int rings, unsigned int region_id)
{
	DynamicVectorClass<Cell> * cells = Build_Border_Cell_List();
	if (rings > 1) {
		CellData::Reset_Spreads(false);
	}

	for (int i = 0; i < rings; i++) {

		for (int j = cells->Count() - 1; j >= 0; j--) {
			Cell c = (*cells)[j];
			CellData::Set_Region(c, region_id);
		}

		if (i < rings - 1) {
			DynamicVectorClass<Cell> * ncells = new DynamicVectorClass<Cell>;
			ncells->Set_Growth_Step(cells->Count());

			for (int k = cells->Count() - 1; k >= 0; k--) {
				Cell cell = (*cells)[k];
				for (int dir = 0; dir < FACING_COUNT; dir++) {
					Cell ncell = Adjacent_Cell(cell, (FacingType)dir);
					if (My_In_Radar(ncell)) {
						MapRegionClass::CellData * data = &MapRegionClass::Get_Cell_Data(ncell);
						if (data->RegionID == ID && data->SpreadID != i + 1) {
							ncells->Add(ncell);
							data->SpreadID = i + 1;
						}
					}
				}
			}
			delete cells;
			cells = ncells;
		}
	}

	delete cells;

	if (rings > 1) {
		CellData::Reset_Spreads(false);
	}
}


/// <summary>
/// Fetches the cells along this region's edge.
/// This routine sweeps the whole map looking for cells of this region that touch some
/// other region. Build_Border_Cell_List2 does the same job from the region's own cell
/// list and is the cheaper of the two when that list can be trusted.
/// </summary>
/// <returns>Returns with a newly allocated vector of the region's edge cells. The caller
/// owns it and must delete it.</returns>
DynamicVectorClass<Cell> *MapRegionClass::Build_Border_Cell_List(void)
{
	DynamicVectorClass<Cell> *cells = new DynamicVectorClass<Cell>;

	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();

	while (cptr != NULL) {

		if (CellData::Get_Region(cptr->Fetch_CellID()) == ID) {
			bool add = false;

			for (int facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
				Cell ncell = cptr->Adjacent_Cell((FacingType)facing).Fetch_CellID();
				if (My_In_Radar(ncell)) {
					if (CellData::Get_Region(ncell) != ID) {
						add = true;
					}
				}
			}

			if (add) {
				cells->Add(cptr->Fetch_CellID());
			}
		}

		cptr = Map.Iterate();
	}

	return(cells);
}


/// <summary>
/// Raises or lowers every cell in this region.
/// </summary>
/// <param name="cell_height">The signed amount to add to each cell's height.</param>
void MapRegionClass::Increase_Height(char cell_height)
{
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		int rid = CellData::Get_Region(cptr->Fetch_CellID());
		if (rid == ID) {
			cptr->Height += cell_height;
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Splits this region into two smaller regions.
/// This routine is how the generator turns a large flat expanse into varied ground. A
/// wandering boundary is grown out from a random cell until it has taken a fair share of
/// the region, and each half then becomes a region in its own right, one of them at a
/// new height. Fragments too small to stand on their own are folded into the largest
/// neighbor instead. A region marked unsplittable is left well alone.
/// </summary>
/// <returns>bool; Was the region split?</returns>
bool MapRegionClass::Split_Region(void)
{
	int i, j;

	if (Unsplittable) {
		return(false);
	}

	int heap_size = (2 * CellCount) + 10;
	CellNode * nodes = new CellNode[heap_size];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>((2 * CellCount) + 10);
	queue->Clear();

	CellData::Reset_Spreads(false);

	int spread_count = 0;
	for (i = Cells.Count() - 1; i >= 0; i--) {
		MapRegionClass::Get_Cell_Data(Cells[i]).RegionID = -2;
	}

	unsigned int target_count = Pick_Random_UInt(CellCount / 8, CellCount / 3);
	unsigned int seed_index = Pick_Random_UInt(0, Cells.Count() - 1);

	int node_count = 1;
	Cell seed = Cells[seed_index];
	nodes[0].Element = seed;
	nodes[0].Score = 0.0f;
	MapRegionClass::Get_Cell_Data(seed).SpreadID = -3;
	queue->Insert(nodes[0]);

	CellNode * node = queue->Extract_Min();
	double dist = (unsigned int)RMGRandom() * (DEG_TO_RAD(360) / UINT_MAX);

	while (node != NULL) {
		if (spread_count >= (int)target_count) {
			break;
		}

		CellData::Set_Region(node->Element, -3);

		int cell_index = node->Element.X + MapCellStride * node->Element.Y;
		CellNode * newnode = &nodes[node_count];

		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = (FacingType)(dir + FACING_90)) {
			Cell ncell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(ncell)) {
				if (CellData::Get_Region(ncell) == -2 && CellData::Get_Spread(ncell) != -3 && Map[ncell].Is_Tile_Clear()) {
					newnode->Element = ncell;
					newnode->Score = Get_Angle_Score(seed, ncell, dist);
					MapRegionClass::Get_Cell_Data(cell_index + AStarFacingToOffset[dir]).SpreadID = -3;
					node_count++;
					queue->Insert(*newnode++);
				}
			}
		}

		spread_count++;
		dist += Sample_Normal(0, DEG_TO_RAD(22.5));
		node = queue->Extract_Min();
	}

	CellData::Reset_Spreads(false);

	CellNode * remaining = queue->Extract_Min();
	DynamicVectorClass<Cell> ncells;

	while (remaining != NULL) {
		Cell c = remaining->Element;
		MapRegionClass::Get_Cell_Data(c).RegionID = -3;
		remaining = queue->Extract_Min();
	}

	delete[] nodes;
	delete queue;

	DynamicVectorClass<MapRegionClass *> new_regions;
	for (i = Cells.Count() - 1; i >= 0; i--) {
		Cell cell = Cells[i];
		int cell_index = cell.X + MapCellStride * cell.Y;
		if (MapRegionClass::Get_Cell_Data(cell_index).RegionID < -1) {
			int old_id = MapRegionClass::Get_Cell_Data(cell_index).RegionID;

			MapRegionClass * region = new MapRegionClass(cell);
			region->Cells.Add(cell);
			region->CellCount++;
			new_regions.Add(region);
			MapRegionClass::Get_Cell_Data(cell_index).RegionID = region->ID;

			DynamicVectorClass<Cell> open;
			open.Set_Growth_Step(100000);
			open.Add(cell);

			while (open.Count() != 0) {
				int index = open.Count() - 1;
				Cell c = open[index];
				open.Delete_Index(index);

				for (int m = 0; m < FACING_COUNT; m++) {
					Cell ncell = Adjacent_Cell(c, (FacingType)m);
					if (My_In_Radar(ncell)) {
						CellData & ndata = MapRegionClass::Get_Cell_Data(ncell);
						if (ndata.RegionID == old_id) {
							region->Cells.Add(ncell);
							region->CellCount++;
							ndata.RegionID = region->ID;
							open.Add(ncell);
						}
					}
				}
			}
		}
	}

	for (i = new_regions.Count() - 1; i >= 0; i--) {
		MapRegionClass * region = new_regions[i];
		if (region->Discardable) {
			continue;
		}

		DynamicVectorClass<Cell> * cells = region->Build_Border_Cell_List2();
		bool * adjacent = new bool[MapRegionClass::TotalCount];
		for (j = 0; j < MapRegionClass::TotalCount; j++) {
			adjacent[j] = false;
		}

		for (j = cells->Count() - 1; j >= 0; j--) {
			Cell c = (*cells)[j];
			for (int m = 0; m < FACING_COUNT; m++) {
				Cell ncell = Adjacent_Cell(c, (FacingType)m);
				if (My_In_Radar(ncell)) {
					int id = MapRegionClass::Get_Cell_Data(ncell).RegionID;
					if (id >= 0 && id != region->ID) {
						adjacent[id] = true;
					}
				}
			}
		}

		delete cells;

		MapRegionClass * merge_target = NULL;
		int min_height = -1;
		int max_height = -1;
		int merge_height = -1;
		int max_count = -1;

		for (j = 0; j < MapRegionClass::TotalCount; j++) {
			if (adjacent[j]) {
				MapRegionClass * other = NULL;
				for (int k = 0; k < MapRegionClass::MapRegions.Count(); k++) {
					if (MapRegionClass::MapRegions[k]->ID == j) {
						other = MapRegionClass::MapRegions[k];
						break;
					}
				}

				if (!other->Discardable) {
					if (other->CellCount > max_count && !other->ContainsWater) {
						max_count = other->CellCount;
						merge_height = other->CellHeight;
						merge_target = other;
					}

					if (min_height == -1) {
						min_height = other->CellHeight;
						max_height = other->CellHeight;
					} else {
						if (other->CellHeight > max_height) {
							max_height = other->CellHeight;
						}
						if (other->CellHeight < min_height) {
							min_height = other->CellHeight;
						}
					}
				}
			}
		}

		DynamicVectorClass<int> heights;
		int diff = max_height - min_height;
		if (diff != 0) {
			if (diff != 4) {
				if (diff == 8) {
					heights.Add((max_height + min_height) / 2);
				}
			} else {
				heights.Add(max_height);
				heights.Add(min_height);
			}
		} else {
			if (min_height >= 4) {
				heights.Add(min_height - 4);
			}
			if (max_height <= 7) {
				heights.Add(max_height + 4);
			}
		}

		delete[] adjacent;

		if (region->CellCount > 100) {
			int old_height = region->CellHeight;
			unsigned int max_index = (unsigned int)heights.Count() - 1;
			unsigned int index = Pick_Random_UInt(0, max_index);

			region->CellHeight = heights[index];
			int height_diff = region->CellHeight - old_height;

			Map.Reset_Iterator();
			for (CellClass * cellptr = Map.Iterate(); cellptr; cellptr = Map.Iterate()) {
				Cell c = cellptr->CellID;
				if (MapRegionClass::CellData::Get_Region(c) == region->ID) {
					cellptr->Height += height_diff;
				}
			}
		} else {
			if (merge_target != NULL) {
				int old_height = region->CellHeight;
				region->CellHeight = merge_height;
				int height_diff = region->CellHeight - old_height;

				Map.Reset_Iterator();
				for (CellClass * cellptr = Map.Iterate(); cellptr; cellptr = Map.Iterate()) {
					Cell c = cellptr->CellID;
					if (MapRegionClass::CellData::Get_Region(c) == region->ID) {
						cellptr->Height += height_diff;
					}
				}

				for (int j = region->Cells.Count() - 1; j >= 0; j--) {
					Cell c = region->Cells[j];
					merge_target->Cells.Add(c);
					merge_target->CellCount++;
					MapRegionClass::Get_Cell_Data(c).RegionID = merge_target->ID;
				}

				region->Discardable = true;
			}
		}
	}

	for (i = MapRegionClass::MapRegions.Count() - 1; i >= 0; i--) {
		MapRegionClass * region = MapRegionClass::MapRegions[i];
		if (region != this && region->Discardable) {
			delete region;
		}
	}

	SplitCount++;
	return(true);
}


/// <summary>
/// Fetches the cells along this region's edge.
/// This routine works from the region's own cell list rather than sweeping the whole
/// map, so it is the cheaper of the two border builders. Use it whenever that list can
/// be trusted to be current.
/// </summary>
/// <returns>Returns with a newly allocated vector of the region's edge cells. The caller
/// owns it and must delete it.</returns>
DynamicVectorClass<Cell> *MapRegionClass::Build_Border_Cell_List2(void)
{
	DynamicVectorClass<Cell> *cells = new DynamicVectorClass<Cell>;
	cells->Set_Growth_Step(std::max(10, CellCount / 2));

	for (int i = Cells.Count() - 1; i >= 0; i--) {
		Cell cell = Cells[i];
		bool add = false;
		for (int facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
			Cell ncell = Adjacent_Cell(cell, (FacingType)facing);
			if (My_In_Radar(ncell)) {
				if (CellData::Get_Region(ncell) != ID) {
					add = true;
					break;
				}
			}
		}

		if (add) {
			cells->Add(cell);
		}
	}

	return(cells);
}


/// <summary>
/// Grows this region outward into unclaimed ground.
/// This routine takes clear, unowned cells that lie at the region's own height, working
/// outward a ring at a time. Running into ground owned by another region abandons the
/// whole attempt, so a caller may use this to find out whether there is room to grow.
/// </summary>
/// <param name="rings">How far out to grow, in rings.</param>
/// <returns>bool; Did the region grow without running into a neighbor?</returns>
bool MapRegionClass::Grow(int rings)
{
	DynamicVectorClass<Cell> * cells = Build_Border_Cell_List();

	for (int i = 0; i < rings; i++) {
		DynamicVectorClass<Cell> * ncells = new DynamicVectorClass<Cell>;
		ncells->Set_Growth_Step(3 * cells->Count());

		for (int j = 0; j < cells->Count(); j++) {
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
				Cell ncell = Adjacent_Cell((*cells)[j], dir);
				if (My_In_Radar(ncell)) {
					if (Map[ncell].Height == CellHeight) {
						MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(ncell);
						int id = data.RegionID;
						if (id == -1 && Map[ncell].Is_Tile_Clear()) {
							ncells->Add(ncell);
							data.RegionID = ID;
							Map[ncell].Height = CellHeight;
						} else if (id != ID) {
							delete cells;
							delete ncells;
							return(false);
						}
					}
				}
			}
		}

		delete cells;
		cells = ncells;
	}

	delete cells;
	return(true);
}


/// <summary>
/// Claims every expandable cell this region can reach.
/// This routine spreads out from the region's border through ground that is still
/// unowned and marked as expandable, taking all of it for this region.
/// </summary>
/// <returns>bool; Always true.</returns>
bool MapRegionClass::Claim_Expandable_Cells(void)
{
	int i;
	int j;

	DynamicVectorClass<Cell> * cells = Build_Border_Cell_List();

	DynamicVectorClass<Cell> ncells;
	ncells.Set_Growth_Step(50000);

	for (i = 0; i < cells->Count(); i++) {
		ncells.Add((*cells)[i]);
	}

	delete cells;

	while (ncells.Count() > 0) {
		j = ncells.Count() - 1;
		Cell c = ncells[j];
		ncells.Delete_Index(j);
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell ncell = Adjacent_Cell(c, dir);
			if (My_In_Radar(ncell)) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(ncell);
				int id = data.RegionID;
				if (id == -1 && data.CanExpand) {
					data.RegionID = ID;
					ncells.Add(ncell);
				}
			}
		}
	}

	return(true);
}


/// <summary>
/// Breaks the map's oversized regions down into smaller ones.
/// This routine first brings every region's cell list and bounding rectangle back in
/// step with the cell grid, then keeps splitting any dry region larger than the map's
/// cliff setting allows until nothing oversized is left. Regions small enough to keep
/// are marked unsplittable so that they are not picked over again.
/// </summary>
void MapRegionClass::Make_Ice_Regions(void)
{
	int i;
	for (i = 0; i < MapRegionClass::MapRegions.Count(); i++) {
		MapRegionClass::MapRegions[i]->Cells.Clear();
		MapRegionClass::MapRegions[i]->CellCount = 0;
		MapRegionClass::MapRegions[i]->RegionBounds = Rect(9999, 9999, 0, 0);
	}

	int grid_size = MapCellStride * MapCellStride;
	for (i = grid_size - 1; i >= 0; i--) {
		CellData & data = MapRegionClass::Get_Cell_Data(i);
		Cell cell = data.CellID;
		int y = cell.Y;
		int x = cell.X;
		if (My_In_Radar(cell)) {
			int id = data.RegionID;
			if (id >= 0 && id < MapRegionClass::MapRegions.Count()) {
				MapRegionClass::MapRegions[id]->CellCount++;
				MapRegionClass::MapRegions[id]->Cells.Add(cell);

				Rect & rect = MapRegionClass::MapRegions[id]->RegionBounds;
				if (!rect.Width) {
					rect.X = x;
					rect.Y = y;
					rect.Width = 1;
					rect.Height = 1;
				}

				int old_x = rect.X;
				if (x >= rect.X + rect.Width) {
					rect.Width = x - old_x + 1;
				}
				if (x < old_x) {
					int old_width = rect.Width;
					rect.X = x;
					rect.Width = old_x - x + old_width;
				}

				int old_y = rect.Y;
				if (y >= rect.Y + rect.Height) {
					rect.Height = y - old_y + 1;
				}
				if (y < old_y) {
					int old_height = rect.Height;
					rect.Y = y;
					rect.Height = old_y - y + old_height;
				}
			}
		}
	}

	double size_limit = ((double)RandomMapGen.SeedData.Cliffs * 0.005 + 0.05) * (double)(int)RandomMapGen.LocalWidth * (double)(int)RandomMapGen.LocalHeight;
	int limit = (int)(size_limit + size_limit);

	while (true) {
		bool restart = false;

		for (int i = 0; i < MapRegionClass::MapRegions.Count(); i++) {
			MapRegionClass * region = MapRegionClass::MapRegions[i];
			if (!region->Unsplittable && !region->ContainsWater) {
				if (region->CellCount > limit) {
					if (region->Split_Region()) {
						delete region;
						restart = true;
						break;
					}
				} else {
					region->Unsplittable = true;
				}
			}
		}

		if (!restart) {
			break;
		}
	}
}


/// <summary>
/// Marks the cells lying deep inside this region.
/// This routine flags every cell of the region that sits well in from its border,
/// leaving the fringe unmarked. The region itself is left owning all of its cells.
/// </summary>
void MapRegionClass::Mark_Interior_Cells(void)
{
	int grid_size = MapCellStride * MapCellStride;
	for (int i = 0; i < grid_size; i++) {
		MapRegionClass::Get_Cell_Data(i).Interior = false;
	}

	Reassign_Border_Cells(8, -2);

	for (int j = 0; j < grid_size; j++) {
		CellData & data = RMGCellData[j];
		if (data.RegionID == ID) {
			RMGCellData[j].Interior = true;
		} else if (data.RegionID == -2) {
			RMGCellData[j].RegionID = ID;
		}
	}
}


/// <summary>
/// Divides the finished terrain up into regions.
/// This routine is the top level of the region pass. The height regions are expanded,
/// the map is carved up afresh, and every region then works out which regions it borders
/// and connects itself to them with bridges and ramps.
/// </summary>
/// <returns>bool; Always true.</returns>
bool MapRegionClass::Make_Regions(void)
{
	bool success = RandomMapGen.Expand_All_High_Ground();
	Destroy_Cell_Regions();
	Create_Land_Regions(false);
	if (success) {
		int i;
		for (i = 0; i < MapRegions.Count(); i++) {
			MapRegions[i]->Build_Neighbor_Regions();
		}
		for (i = 0; i < MapRegions.Count(); i++) {
			MapRegions[i]->Make_Region_Connections();
		}
		for (i = 0; i < MapRegions.Count(); i++) {
			delete MapRegions[i]->NeighborRegions;
			MapRegions[i]->NeighborRegions = NULL;
		}
	}
	return(true);
}


/// <summary>
/// Builds the list of regions bordering this one.
/// This routine is called by Make_Regions before any connections are laid out, since
/// deciding where a bridge or a ramp should go means knowing which regions touch. The
/// region's cell tally is brought back up to date on the way through.
/// </summary>
void MapRegionClass::Build_Neighbor_Regions(void)
{
	NeighborRegions = new DynamicVectorClass<int>;
	bool * adjacent = new bool[MapRegionClass::TotalCount];
	for (int i = 0; i < MapRegionClass::TotalCount; ++i) {
		adjacent[i] = false;
	}
	DynamicVectorClass<Cell> * cells = Build_Border_Cell_List();

	for (int j = 0; j < cells->Count(); j++) {
		Cell cell = (*cells)[j];
		for (int facing = 0; facing < FACING_COUNT; facing++) {
			Cell ncell = Adjacent_Cell(cell, (FacingType)facing);
			if (My_In_Radar(ncell)) {
				int id = CellData::Get_Region(ncell);
				if (id >= 0) {
					adjacent[id] = true;
				}
			}
		}
	}

	for (int k = 0; k < MapRegionClass::TotalCount; k++) {
		if (adjacent[k] && k != ID) {
			NeighborRegions->Add(k);
		}
	}

	Recount_Cells();

	delete cells;
	delete[] adjacent;
}


/// <summary>
/// Connects two regions with a bridge over the water.
/// This routine is used when a stretch of water separates two pieces of land. It hunts
/// for a straight span whose ends land in the two regions given, lays a low bridge and
/// its approach roads along it, and drops a repair hut at either end. The search gives
/// up rather than blocking if the water proves too awkward to cross.
/// </summary>
/// <param name="region1">First region to connect.</param>
/// <param name="region2">Second region to connect.</param>
/// <returns>bool; Was a bridge placed?</returns>
bool MapRegionClass::Connect_Regions_With_Bridge(MapRegionClass * region1, MapRegionClass * region2)
{
	int id1 = region1->ID;
	int id2 = region2->ID;

	int tries = 0;
	bool result = false;

	do {
		if (result) {
			break;
		}

		Cell cell = Pick_Random_Cell();

		Rect rect1(cell.X - 1, cell.Y, 3, 1);
		Rect rect2(rect1.X, rect1.Y, 3, 1);
		Rect rect4(cell.X, cell.Y - 1, 1, 3);
		Rect rect3(rect4.X, rect4.Y, 1, 3);

		bool allow_rect3 = true;
		bool allow_rect1 = true;

		while (!MapGeneratorClass::Can_Place_Paved_Road(rect1, false, false)) {
			Cell cell1;
			Cell cell2;
			cell1.X = rect1.X;
			cell2.X = rect1.X + 2;
			rect1.Y--;
			cell1.Y = rect1.Y;
			cell2.Y = rect1.Y;
			CellClass * cptr1 = &Map[cell1];
			CellClass * cptr2 = &Map[cell2];
			if (!Map.In_Local_Radar(cell1) || !Map.In_Local_Radar(cell2) || cptr1->Is_Tile_Cliff() || cptr2->Is_Tile_Cliff()) {
				allow_rect1 = false;
				break;
			}
		}

		if (allow_rect1) {
			if (MapGeneratorClass::Can_Place_Paved_Road(Rect(rect1.X, rect1.Y - 3, 3, 3), false, false)) {
				while (!MapGeneratorClass::Can_Place_Paved_Road(rect2, false, false)) {
					Cell cell1;
					Cell cell2;
					cell1.X = rect2.X;
					cell2.X = rect2.X + 2;
					rect2.Y++;
					cell1.Y = rect2.Y;
					cell2.Y = rect2.Y;
					CellClass * cptr1 = &Map[cell1];
					CellClass * cptr2 = &Map[cell2];
					if (!Map.In_Local_Radar(cell1) || !Map.In_Local_Radar(cell2) || cptr1->Is_Tile_Cliff() || cptr2->Is_Tile_Cliff()) {
						allow_rect1 = false;
						break;
					}
				}

				if (allow_rect1) {
					if (!MapGeneratorClass::Can_Place_Paved_Road(Rect(rect2.X, rect2.Y + 1, 3, 3), false, false)) {
						allow_rect1 = false;
					}
				}
			} else {
				allow_rect1 = false;
			}
		}

		while (!MapGeneratorClass::Can_Place_Paved_Road(rect3, false, false)) {
			Cell cell1;
			Cell cell2;
			cell1.Y = rect3.Y;
			cell2.Y = rect3.Y + 2;
			rect3.X--;
			cell1.X = rect3.X;
			cell2.X = rect3.X;
			CellClass * cptr1 = &Map[cell1];
			CellClass * cptr2 = &Map[cell2];
			if (!Map.In_Local_Radar(cell1) || !Map.In_Local_Radar(cell2) || cptr1->Is_Tile_Cliff() || cptr2->Is_Tile_Cliff()) {
				allow_rect3 = false;
				break;
			}
		}

		if (allow_rect3) {
			if (MapGeneratorClass::Can_Place_Paved_Road(Rect(rect3.X - 3, rect3.Y, 3, 3), false, false)) {
				while (!MapGeneratorClass::Can_Place_Paved_Road(rect4, false, false)) {
					Cell cell1;
					Cell cell2;
					cell1.Y = rect4.Y;
					cell2.Y = rect4.Y + 2;
					rect4.X++;
					cell1.X = rect4.X;
					cell2.X = rect4.X;
					CellClass * cptr1 = &Map[cell1];
					CellClass * cptr2 = &Map[cell2];
					if (!Map.In_Local_Radar(cell1) || !Map.In_Local_Radar(cell2) || cptr1->Is_Tile_Cliff() || cptr2->Is_Tile_Cliff()) {
						allow_rect3 = false;
						break;
					}
				}

				if (allow_rect3) {
					if (!MapGeneratorClass::Can_Place_Paved_Road(Rect(rect4.X + 1, rect4.Y, 3, 3), false, false)) {
						allow_rect3 = false;
					}
				}
			} else {
				allow_rect3 = false;
			}
		}

		int len1 = 999;
		int len2 = 999;
		int region_a = -1;
		int region_b = -1;
		int region_c = -1;
		int region_d = -1;

		if (allow_rect1) {
			len1 = abs(rect2.Y - rect1.Y);
			if (RMGCellData != NULL) {
				region_a = MapRegionClass::Get_Cell_Data(Cell(rect1.X, rect1.Y)).RegionID;
				region_b = MapRegionClass::Get_Cell_Data(Cell(rect2.X, rect2.Y)).RegionID;
			} else {
				region_a = -1;
				region_b = -1;
			}
		}

		if (allow_rect3) {
			len2 = abs(rect4.X - rect3.X);
			if (RMGCellData == NULL) {
				region_c = -1;
				region_d = -1;
			} else {
				region_c = MapRegionClass::Get_Cell_Data(Cell(rect4.X, rect4.Y)).RegionID;
				region_d = MapRegionClass::Get_Cell_Data(Cell(rect3.X, rect3.Y)).RegionID;
			}
		}

		if (allow_rect1) {
			if ((region_a != id1 || region_b != id2) && (region_a != id2 || region_b != id1)) {
				allow_rect1 = false;
			}
		}

		if (allow_rect3) {
			if (region_c == id1 && region_d == id2 || region_c == id2 && region_d == id1) {
				if (allow_rect1) {
					if (len1 < len2) {
						allow_rect3 = false;
					} else {
						allow_rect1 = false;
					}
				}
			} else {
				allow_rect3 = false;
			}
		}

		Rect bridge(0, 0, 0, 0);
		int max_len = tries / 25 + 8;

		if (allow_rect3 && len2 < max_len) {
			bridge = Rect(rect3.X, rect3.Y, rect4.X - rect3.X + 1, 3);
		} else if (allow_rect1 && len1 < max_len) {
			bridge = Rect(rect1.X, rect1.Y, 3, rect2.Y - rect1.Y + 1);
		}

		if (bridge.Is_Valid()) {
			if (allow_rect3) {
				if (Is_Bridge_Allowed(bridge)) {
					for (int y = bridge.Y; y < bridge.Y + bridge.Height; y++) {
						for (int x = bridge.X; x < bridge.X + bridge.Width; x++) {
							if (x == bridge.X) {
								Map[Cell(x, y)].Overlay = OVERLAY_LOWBRIDGE_21;
							} else if (x == bridge.X + bridge.Width - 1) {
								Map[Cell(x, y)].Overlay = OVERLAY_LOWBRIDGE_19;
							} else {
								Map[Cell(x, y)].Overlay = (OverlayType)(OVERLAY_LOWBRIDGE_01 + (x % 4));
							}
							Map[Cell(x, y)].OverlayData = y - bridge.Y;
						}
					}

					if (MapGeneratorClass::Can_Place_Paved_Road_End(Rect(bridge.X + bridge.Width, bridge.Y - 2, 6, 6), false) && Pick_Random_UInt(0, 1)) {
						MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 10), Cell(bridge.X + bridge.Width, bridge.Y));
					} else {
						MapGeneratorClass::Place_Tile(IsometricTileTypeClass::PavedRoadEnds, Cell(bridge.X + bridge.Width, bridge.Y));
					}

					if (MapGeneratorClass::Can_Place_Paved_Road_End(Rect(bridge.X - 6, bridge.Y - 2, 6, 6), false) && Pick_Random_UInt(0, 1)) {
						MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 9), Cell(bridge.X - 4, bridge.Y));
					} else {
						MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 2), Cell(bridge.X - 1, bridge.Y));
					}

					if (!Place_Bridge_Hut(Rect(bridge.X, bridge.Y - 1, 2, 5))) {
						Place_Bridge_Hut(Rect(bridge.X - 1, bridge.Y - 2, 3, 7));
					}

					if (!Place_Bridge_Hut(Rect(bridge.X + bridge.Width - 2, bridge.Y - 1, 2, 5))) {
						Place_Bridge_Hut(Rect(bridge.X + bridge.Width - 2, bridge.Y - 2, 3, 7));
					}

					result = true;
				}
			} else if (bridge.Height > 0 && allow_rect1 && Is_Bridge_Allowed(bridge)) {
				for (int y = bridge.Y; y < bridge.Y + bridge.Height; y++) {
					for (int x = bridge.X; x < bridge.X + bridge.Width; x++) {
						if (y == bridge.Y) {
							Map[Cell(x, y)].Overlay = OVERLAY_LOWBRIDGE_23;
						} else if (y == bridge.Y + bridge.Height - 1) {
							Map[Cell(x, y)].Overlay = OVERLAY_LOWBRIDGE_25;
						} else {
							Map[Cell(x, y)].Overlay = (OverlayType)(OVERLAY_LOWBRIDGE_10 + (y % 4));
						}
						Map[Cell(x, y)].OverlayData = x - bridge.X;
					}
				}

				if (MapGeneratorClass::Can_Place_Paved_Road_End(Rect(bridge.X - 2, bridge.Y - 6, 7, 6), false) && Pick_Random_UInt(0, 1)) {
					MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 13), Cell(bridge.X, bridge.Y - 4));
				} else {
					MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 1), Cell(bridge.X, bridge.Y - 1));
				}

				if (MapGeneratorClass::Can_Place_Paved_Road_End(Rect(bridge.X - 2, bridge.Y + bridge.Height, 7, 6), false) && Pick_Random_UInt(0, 1)) {
					MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 12), Cell(bridge.X, bridge.Y + bridge.Height));
				} else {
					MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 3), Cell(bridge.X, bridge.Y + bridge.Height));
				}

				if (!Place_Bridge_Hut(Rect(bridge.X - 1, bridge.Y, 5, 2))) {
					Place_Bridge_Hut(Rect(bridge.X - 2, bridge.Y - 1, 7, 3));
				}

				if (!Place_Bridge_Hut(Rect(bridge.X - 1, bridge.Y + bridge.Height - 2, 5, 2))) {
					Place_Bridge_Hut(Rect(bridge.X - 2, bridge.Y + bridge.Height - 2, 7, 3));
				}

				result = true;
			}
		}

	} while (++tries < 200);

	return(result);
}


/// <summary>
/// Determines if a bridge may be built here.
/// This routine is used by the bridge layer to vet a candidate span before committing to
/// it. The ground must lie on the map, and be unobstructed, level, and either clear or
/// water throughout.
/// </summary>
/// <param name="rect">The candidate bridge footprint.</param>
/// <returns>bool; May the bridge be built?</returns>
bool MapRegionClass::Is_Bridge_Allowed(Rect const & rect) const
{
	int height = Map[Cell(rect.X, rect.Y)].Height;

	if (My_In_Radar(Cell(rect.X, rect.Y)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y)) &&
		My_In_Radar(Cell(rect.X, rect.Y + rect.Height - 1)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1,rect.Y + rect.Height - 1))) {
		for (int y = rect.Y; y < rect.Y + rect.Height + 1; y++) {
			for (int x = rect.X; x < rect.X + rect.Width + 1; x++) {
				CellClass *cptr = &Map[Cell(x,y)];
				if (cptr->Overlay != OVERLAY_NONE) {
					return(false);
				}
				if (cptr->Height != height) {
					return(false);
				}
				if (!cptr->Is_Tile_Clear() && !cptr->Is_Tile_With_Water()) {
					return(false);
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Places a repair hut at the end of a bridge.
/// This routine hunts within the given area for somewhere the hut will actually fit and
/// builds it there for the neutral house. Only the one hut is placed.
/// </summary>
/// <param name="rect">The area to search for a buildable cell.</param>
/// <returns>bool; Was a hut placed?</returns>
bool MapRegionClass::Place_Bridge_Hut(Rect const & rect) const
{
	for (int y = rect.Y; y < rect.Y + rect.Height + 1; y++) {
		for (int x = rect.X; x < rect.X + rect.Width + 1; x++) {
			CellClass *cptr = &Map[Cell(x,y)];
			if (cptr->Overlay == OVERLAY_NONE && cptr->Is_Tile_Clear() && cptr->Cell_Occupier() == NULL) {
				HouseClass *hptr = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));
				StructType caba = BuildingTypeClass::From_Name("CABHUT");
				BuildingClass *thehut = new BuildingClass(BuildingTypes[caba], hptr);
				thehut->Unlimbo(cptr->CellID);
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Creates the connections from this region to its neighbors.
/// This routine is called once per region by Make_Regions. A water region spans itself
/// with bridges to link the dry land on either side; dry land instead carves ramps down
/// to each lower neighbor, as many of them as the map's accessibility setting will bear.
/// A region that ends up with nothing joining it to its neighbors is flagged as
/// unconnected.
/// </summary>
void MapRegionClass::Make_Region_Connections(void)
{
	if (ContainsWater) {
		for (int i = 0; i < NeighborRegions->Count() - 1; i++) {
			MapRegionClass * region = From_ID((*NeighborRegions)[i]);
			if (region->NeighborRegions->Count() > 1 || region->CellCount > 50) {
				for (int k = i + 1; k < NeighborRegions->Count(); k++) {
					MapRegionClass * region2 = From_ID((*NeighborRegions)[k]);
					if ((region2->NeighborRegions->Count() > 1 || region2->CellCount > 50) && !region2->ContainsWater) {
						if (region2->CellHeight == region->CellHeight && region->CellHeight == CellHeight) {
							Connect_Regions_With_Bridge(region, region2);
						}
					}
				}
			}
		}
	} else {
		for (int i = 0; i < NeighborRegions->Count(); i++) {
			MapRegionClass * region = From_ID((*NeighborRegions)[i]);
			if (region->ID > ID && region->CellHeight != CellHeight) {
				int total = 0;
				int target = ((int)Pick_Random_UInt(0, 100) < RandomMapGen.SeedData.Accessibility ? Pick_Random_UInt(1, 2) : 0) + 1;

				int tries = 0;

				int id1 = region->CellHeight > CellHeight ? region->ID : ID;
				int id2 = region->CellHeight > CellHeight ? ID : region->ID;

				MapRegionClass * region2 = From_ID(id1);
				DynamicVectorClass<Cell> * cells = region2->Build_Border_Cell_List();

				while (total < target && tries < 100) {
					Cell cell = (*cells)[Pick_Random_UInt(0, cells->Count() - 1)];
					if (Map.In_Local_Radar(cell)) {
						total += region2->Connect_Regions_With_Ramp(cell, id2, (tries > 50)) ? 1 : 0;
					}
					tries++;
				}
				if (total == 0) {
					HasConnections = false;
				}

				delete cells;
			}
		}
	}
}


/// <summary>
/// Carves a ramp joining this region to a lower neighbor.
/// This routine is called by the region connector for each candidate cell along the
/// boundary. It looks at which way the region continues around the cell, picks the ramp
/// orientation that suits -- corner ramps first, then the straight ones -- and hands the
/// digging to the matching builder. A caller that has run out of patience may ask for
/// the fallback, which tries the four straight ramps at default endpoints instead.
/// </summary>
/// <param name="cell">The cell around which to attempt the ramp.</param>
/// <param name="region_id">The region on the far side of the height change.</param>
/// <param name="allow_fallback">Should default ramp placements be tried if nothing suits?</param>
/// <returns>bool; Was a ramp carved?</returns>
bool MapRegionClass::Connect_Regions_With_Ramp(Cell const & cell, int region_id, bool allow_fallback)
{
	bool valid = true;
	unsigned mask = Build_Neighbor_Region_Mask(cell, valid, region_id);
	if (!valid) {
		return(false);
	}

	bool result = false;

	if (mask == (MG_FACINGF_N | MG_FACINGF_NE | MG_FACINGF_E | MG_FACINGF_SE | MG_FACINGF_S | MG_FACINGF_SW | MG_FACINGF_W | MG_FACINGF_NW)) {
		return(false);
	}

	{
		Cell c1;
		Cell c2;

		/*
		 * Try each of the four corner ramp orientations that matches the
		 * neighbor mask, then the four straight ramp orientations.
		 */
		if ((mask & (MG_FACINGF_N | MG_FACINGF_E)) == (MG_FACINGF_N | MG_FACINGF_E)
			&& (mask & (MG_FACINGF_S | MG_FACINGF_SW | MG_FACINGF_W)) == 0) {
			c1.X = cell.X - 1;
			c1.Y = cell.Y - 5;
			c2.X = cell.X + 5;
			c2.Y = cell.Y + 1;
			if ((mask & MG_FACINGF_NW) != 0) {
				++c1.Y;
			}
			if ((mask & MG_FACINGF_SE) != 0) {
				--c2.X;
			}
			result = Build_Corner_Ramp_SW(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_E | MG_FACINGF_S)) == (MG_FACINGF_E | MG_FACINGF_S)
			&& (mask & (MG_FACINGF_NE | MG_FACINGF_SW)) == 0) {
			c1.X = cell.X + 5;
			c1.Y = cell.Y - 1;
			c2.X = cell.X - 1;
			c2.Y = cell.Y + 5;
			if ((mask & MG_FACINGF_SW) != 0) {
				--c2.Y;
			}
			if ((mask & MG_FACINGF_NE) != 0) {
				--c1.X;
			}
			result = Build_Corner_Ramp_NW(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_S | MG_FACINGF_W)) == (MG_FACINGF_S | MG_FACINGF_W)
			&& (mask & (MG_FACINGF_N | MG_FACINGF_NE | MG_FACINGF_E)) == 0) {
			c1.X = cell.X + 1;
			c1.Y = cell.Y + 5;
			c2.X = cell.X - 5;
			c2.Y = cell.Y - 1;
			if ((mask & MG_FACINGF_NW) != 0) {
				--c2.X;
			}
			if ((mask & MG_FACINGF_SE) != 0) {
				--c1.Y;
			}
			result = Build_Corner_Ramp_NE(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_N | MG_FACINGF_W)) == (MG_FACINGF_N | MG_FACINGF_W)
			&& (mask & (MG_FACINGF_E | MG_FACINGF_SE | MG_FACINGF_S)) == 0) {
			c1.X = cell.X - 5;
			c1.Y = cell.Y + 1;
			c2.X = cell.X + 1;
			c2.Y = cell.Y - 5;
			if ((mask & MG_FACINGF_NE) != 0) {
				++c2.Y;
			}
			if ((mask & MG_FACINGF_SW) != 0) {
				++c1.X;
			}
			result = Build_Corner_Ramp_NE(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_SW | MG_FACINGF_W | MG_FACINGF_NW)) == 0
			&& (mask & (MG_FACINGF_N | MG_FACINGF_S)) != 0) {
			c1.X = cell.X - 1;
			c1.Y = cell.Y - 4;
			c2.X = c1.X;
			c2.Y = cell.Y + 4;
			if ((mask & MG_FACINGF_N) != 0) {
				++c1.Y;
				++c2.Y;
			}
			if ((mask & MG_FACINGF_S) != 0) {
				--c2.Y;
				--c1.Y;
			}

			c1.X += Pick_Random_UInt(0, 1);
			c2.X += Pick_Random_UInt(0, 1);

			result = Build_West_Ramp(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_NE | MG_FACINGF_E | MG_FACINGF_SE)) == 0
			&& (mask & (MG_FACINGF_N | MG_FACINGF_S)) != 0) {
			c1.X = cell.X + 1;
			c1.Y = cell.Y + 4;
			c2.X = c1.X;
			c2.Y = cell.Y - 4;
			if ((mask & MG_FACINGF_N) != 0) {
				++c2.Y;
				++c1.Y;
			}
			if ((mask & MG_FACINGF_S) != 0) {
				--c1.Y;
				--c2.Y;
			}

			c1.X += Pick_Random_UInt(0, 1) - 1;
			c2.X += Pick_Random_UInt(0, 1) - 1;

			result = Build_East_Ramp(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_N | MG_FACINGF_NE | MG_FACINGF_NW)) == 0
			&& (mask & (MG_FACINGF_E | MG_FACINGF_W)) != 0) {
			c1.X = cell.X + 4;
			c1.Y = cell.Y - 1;
			c2.X = cell.X - 4;
			c2.Y = c1.Y;
			if ((mask & MG_FACINGF_W) != 0) {
				++c2.X;
				++c1.X;
			}
			if ((mask & MG_FACINGF_E) != 0) {
				--c1.X;
				--c2.X;
			}

			c1.Y += Pick_Random_UInt(0, 1);
			c2.Y += Pick_Random_UInt(0, 1);

			result = Build_North_Ramp(c1, c2, region_id);
		}

		if (!result && (mask & (MG_FACINGF_SE | MG_FACINGF_S | MG_FACINGF_SW)) == 0
			&& (mask & (MG_FACINGF_E | MG_FACINGF_W)) != 0) {
			c1.X = cell.X - 4;
			c1.Y = cell.Y + 1;
			c2.X = cell.X + 4;
			c2.Y = c1.Y;
			if ((mask & MG_FACINGF_W) != 0) {
				++c1.X;
				++c2.X;
			}
			if ((mask & MG_FACINGF_E) != 0) {
				--c2.X;
				--c1.X;
			}

			c1.Y += Pick_Random_UInt(0, 1) - 1;
			c2.Y += Pick_Random_UInt(0, 1) - 1;

			result = Build_South_Ramp(c1, c2, region_id);
		}

		/*
		 * If nothing fit the mask, optionally force a default straight ramp
		 * in whatever direction isn't blocked by a diagonal neighbor.
		 */
		if (allow_fallback) {
			if (!result) {

				if ((mask & MG_FACINGF_S) == 0) {
					result = Build_South_Ramp(Cell(cell.X - 4, cell.Y + 1), Cell(cell.X + 4, cell.Y + 1), region_id);
				}

				if (!result && (mask & MG_FACINGF_E) == 0) {
					result = Build_East_Ramp(Cell(cell.X + 1, cell.Y + 4), Cell(cell.X + 1, cell.Y - 4), region_id);
				}

				if (!result && (mask & MG_FACINGF_S) == 0) {
					result = Build_North_Ramp(Cell(cell.X + 4, cell.Y - 1), Cell(cell.X - 4, cell.Y - 1), region_id);
				}

				if (!result && (mask & MG_FACINGF_W) == 0) {
					return(Build_West_Ramp(Cell(cell.X - 1, cell.Y - 4), Cell(cell.X - 1, cell.Y + 4), region_id));
				}
			}
		}
	}

	return(result);
}


/// <summary>
/// Determines which way this region continues around a cell.
/// This routine samples the eight patches of ground surrounding the cell and reports
/// which of them still belong to this region. Connect_Regions_With_Ramp uses the answer
/// to pick a ramp orientation, and the site is refused outright when the ground under
/// the cell itself is barely part of the region at all.
/// </summary>
/// <param name="cell">The cell at the center of the area to sample.</param>
/// <param name="valid">Cleared when the site is unsuitable for a ramp.</param>
/// <returns>Returns with a mask of the facings in which the region continues, or zero
/// if the site was refused.</returns>
unsigned MapRegionClass::Build_Neighbor_Region_Mask(Cell const & cell, bool & valid, int unknown)
{
	Cell c = (cell - Cell(2, 2));
	int ox = c.X;
	int oy = c.Y;

	if (!MapGeneratorClass::Region_Cell_Count_At_Least(Rect(ox, oy, 5, 5), ID, unknown, 6)) {
		valid = false;
		return(0);
	}

	unsigned mask = 0;
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			if (y != 1 || x != 1) {
				if (MapGeneratorClass::Region_Cell_Count_At_Least(Rect(ox + 5 * (x - 1), oy + 5 * (y - 1), 5, 5), ID, unknown, 6)) {
					/// One bit per neighboring direction, clockwise from the northeast, laid out
					/// as a 3x3 grid so that it can be indexed by the offsets of the scan below.
					static int _facing_bits[3][3] = {
						MG_FACINGF_NW, MG_FACINGF_N, MG_FACINGF_NE,
						MG_FACINGF_W, 0, MG_FACINGF_E,
						MG_FACINGF_SW, MG_FACINGF_S, MG_FACINGF_SE
					};
					mask |= _facing_bits[y][x];
				}
			}
		}
	}

	return(mask);
}


/// <summary>
/// Carves a south-facing transition ramp between two cells.
/// This routine cuts one of the four straight ramps the region connector uses to join
/// ground at differing heights. The ramp runs west to east with the low ground lying to
/// the south of it. The east, north and west builders are this same routine turned a
/// quarter at a time.
/// </summary>
/// <param name="cell1">Left endpoint of the ramp.</param>
/// <param name="cell2">Right endpoint of the ramp.</param>
/// <param name="region_id">The region whose height the leveled ground adopts.</param>
/// <returns>bool; Was the ramp carved?</returns>
bool MapRegionClass::Build_South_Ramp(Cell const & cell1, Cell const & cell2, int region_id)
{
	/*
	 * Compute a bounding rect around the ramp area and validate it.
	 */
	Rect rect(cell1.X - 5, std::min(cell1.Y, cell2.Y) - 3, cell2.X - cell1.X + 9, std::max(cell1.Y, cell2.Y) - std::min(cell1.Y, cell2.Y) + 11);
	if (!Is_Ramp_Area_Clear(rect, region_id)) {
		return(false);
	}

	/*
	 * The ramp body drifts sideways as it runs; reject if the drift is too
	 * large to fit within the horizontal span.
	 */
	int dy = cell2.Y - cell1.Y;
	int abs_dy = abs(dy);
	int dx = cell2.X - cell1.X;
	if (abs_dy > dx - 5) {
		return(false);
	}

	int half = (dx + 1) / 2 + 3;

	/*
	 * Level the left and right halves on either side of the ramp, the far
	 * side to the target region's height and the near side to ours.
	 */
	Flatten_Area(Rect(rect.X, cell1.Y + 1, half + 2, rect.Height - cell1.Y + rect.Y + 1), region_id);
	Flatten_Area(Rect(cell2.X - (dx + 1) / 2, cell2.Y + 1, half + 2, rect.Height - cell2.Y + rect.Y + 1), region_id);
	Flatten_Area(Rect(rect.X + 4, rect.Y + 2, half - 2, cell1.Y - rect.Y - 1));
	Flatten_Area(Rect(cell2.X - half + 2, rect.Y + 2, half - 1, cell2.Y - rect.Y - 1));

	/*
	 * Place the two slope set pieces at the ramp ends.
	 */
	MapGeneratorClass::Place_Tile(IsometricTileTypeClass::SlopeSetPieces, cell1, region_id, CellHeight);
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 1), cell2 - Cell(2, 0), region_id, CellHeight);

	/*
	 * Carve the diagonal jog that absorbs the vertical drift between the two
	 * endpoints. A random pick decides whether the jog sits at the near or the
	 * far end of the ramp; the straight remainder is offset accordingly.
	 */
	Cell cell;
	Cell tail(0, 0);

	if (dy != 0) {
		int rsel = Pick_Random_UInt(0, 1);

		Cell bias;
		bias.X = 0;
		int down = dy > 0;
		bias.Y = (dy >= 0) - 1;

		if (rsel == 1) {
			tail.X = abs_dy;
			tail.Y = dy;
		} else {
			bias.X = cell2.X - abs_dy - cell1.X - 5;
		}

		int forward = 0;
		if (abs_dy > 0) {
			int backward = 0;
			do {
				static int _heights[] = {3, 2, 1, 0, 0};
				static int _ramps[] = {11, 15, 15, 15, 7, 12, 16, 16, 16, 8};

				for (int row = 0; row < 5; row++) {
					int add_y = down ? forward : backward;

					Cell pos = Cell(cell1.X + 3, cell1.Y) + Cell(forward, row) + bias;
					cell = Cell(pos.X, pos.Y + add_y);

					CellClass * cellptr = &Map[cell];

					unsigned char height = (unsigned char)_heights[row];
					cellptr->Height = CellHeight + height - 4;

					unsigned char ramp = (unsigned char)_ramps[5 * down + row];
					cellptr->Ramp = ramp;
					cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + ramp - 1);
					cellptr->SubTile = 0;
				}

				forward++;
				backward--;
			} while (forward < abs_dy);
		}
	}

	/*
	 * Extend the straight ramp over whatever horizontal span the jog didn't use.
	 */
	for (int x = 0; x < cell2.X - cell1.X - abs_dy - 5; x++) {
		for (int i = 0; i < 4; i++) {
			cell = Cell(cell1.X + 3, cell1.Y) + Cell(x, i) + tail;
			CellData::Set_Region(cell, region_id);
			CellClass * cellptr = &Map[cell];
			cellptr->Height = CellHeight - i - 1;
			cellptr->Ramp = 4;
			cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 3);
			cellptr->SubTile = 0;
		}
	}

	return(true);
}


/// <summary>
/// Carves an east-facing transition ramp between two cells.
/// This routine cuts one of the four straight ramps the region connector uses to join
/// ground at differing heights. The ramp runs south to north with the low ground lying
/// to the east of it. See Build_South_Ramp for the rest of the family.
/// </summary>
/// <param name="cell1">Bottom endpoint of the ramp.</param>
/// <param name="cell2">Top endpoint of the ramp.</param>
/// <param name="region_id">The region whose height the leveled ground adopts.</param>
/// <returns>bool; Was the ramp carved?</returns>
bool MapRegionClass::Build_East_Ramp(Cell const & cell1, Cell const & cell2, int region_id)
{
	/*
	 * Compute a bounding rect around the ramp area and validate it.
	 */
	int x_max = std::max(cell1.X, cell2.X);
	int x_min = std::min(cell1.X, cell2.X);

	Rect rect;
	rect.X      = x_min - 3;
	rect.Y      = cell2.Y - 4;
	rect.Height = cell1.Y - cell2.Y + 9;
	rect.Width  = x_max - x_min + 11;

	if (!Is_Ramp_Area_Clear(rect, region_id)) {
		return(false);
	}

	/*
	 * The ramp body drifts sideways as it runs; reject if the drift is too
	 * large to fit within the vertical span.
	 */
	int dx = cell2.X - cell1.X;
	int dy = cell1.Y - cell2.Y;
	int abs_dx = abs(dx);
	if (abs_dx > dy - 5) {
		return(false);
	}

	int half = (dy + 1) / 2 + 2;

	/*
	 * Level the top and bottom halves on either side of the ramp, the far
	 * side to the target region's height and the near side to ours.
	 */
	Flatten_Area(Rect(cell1.X + 1, cell1.Y - (dy + 1) / 2, 7, half + 3), region_id);
	Flatten_Area(Rect(cell2.X + 1, rect.Y, 7, half + 2), region_id);
	Flatten_Area(Rect(rect.X + 2, cell1.Y - (dy + 1) / 2, cell1.X - rect.X - 1, (dy + 1) / 2 + 1));
	Flatten_Area(Rect(rect.X + 2, cell2.Y, cell2.X - rect.X - 1, (dy + 1) / 2));

	/*
	 * Place the two slope set pieces at the ramp ends.
	 */
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 2), cell1 - Cell(0, 2), region_id, CellHeight);
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 3), cell2, region_id, CellHeight);

	/*
	 * Carve the diagonal jog that absorbs the horizontal drift between the two
	 * endpoints. A random pick decides whether the jog sits at the near or the
	 * far end of the ramp; the straight remainder is offset accordingly.
	 */
	Cell cell;
	Cell tail(0, 0);

	if (dx != 0) {
		int rsel = Pick_Random_UInt(0, 1);

		int right = dx > 0;
		Cell bias((dx >= 0) - 1, 0);

		if (rsel == 1) {
			tail.X = dx;
			tail.Y = -abs_dx;
		} else {
			bias = Cell((dx >= 0) - 1, -(short)(cell1.Y - abs_dx - cell2.Y - 5));
		}

		int forward = 0;
		if (abs_dx > 0) {
			int backward = 0;
			do {
				static int _heights[] = {3, 2, 1, 0, 0};
				static int _ramps[] = {10, 14, 14, 14, 6, 11, 15, 15, 15, 7};

				for (int row = 0; row < 5; row++) {
					int add_x = right ? forward : backward;

					Cell pos = Cell(cell1.X, cell1.Y - 3) + Cell(row, -forward) + bias;
					cell = Cell(pos.X + add_x, pos.Y);

					CellClass * cellptr = &Map[cell];

					unsigned char height = (unsigned char)_heights[row];
					cellptr->Height = CellHeight + height - 4;

					unsigned char ramp = (unsigned char)_ramps[5 * right + row];
					cellptr->Ramp = ramp;
					cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + ramp - 1);
					cellptr->SubTile = 0;
				}

				forward++;
				backward--;
			} while (forward < abs_dx);
		}
	}

	/*
	 * Extend the straight ramp over whatever vertical span the jog didn't use.
	 */
	for (int y = 0; y < cell1.Y - cell2.Y - abs_dx - 5; y++) {
		for (int i = 0; i < 4; i++) {
			cell = Cell(cell1.X, cell1.Y - 3) + Cell(i, -y) + tail;
			CellData::Set_Region(cell, region_id);
			CellClass * cellptr = &Map[cell];
			cellptr->Height = CellHeight - i - 1;
			cellptr->Ramp = 3;
			cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 2);
			cellptr->SubTile = 0;
		}
	}

	return(true);
}


/// <summary>
/// Carves a north-facing transition ramp between two cells.
/// This routine cuts one of the four straight ramps the region connector uses to join
/// ground at differing heights. The ramp runs east to west with the low ground lying to
/// the north of it. See Build_South_Ramp for the rest of the family.
/// </summary>
/// <param name="cell1">East endpoint of the ramp.</param>
/// <param name="cell2">West endpoint of the ramp.</param>
/// <param name="region_id">The region whose height the leveled ground adopts.</param>
/// <returns>bool; Was the ramp carved?</returns>
bool MapRegionClass::Build_North_Ramp(Cell const & cell1, Cell const & cell2, int region_id)
{
	/*
	 * Compute a bounding rect around the ramp area and validate it.
	 */
	Rect rect(cell2.X - 4, std::min(cell1.Y, cell2.Y) - 7, cell1.X - cell2.X + 8, std::max(cell1.Y, cell2.Y) - std::min(cell1.Y, cell2.Y) + 11);
	if (!Is_Ramp_Area_Clear(rect, region_id)) {
		return(false);
	}

	/*
	 * The ramp body drifts sideways as it runs; reject if the drift is too
	 * large to fit within the horizontal span.
	 */
	int dy = cell2.Y - cell1.Y;
	int dx = cell1.X - cell2.X;
	int abs_dy = abs(dy);
	if (abs_dy > dx - 5) {
		return(false);
	}

	int half = (dx + 1) / 2 + 3;

	/*
	 * Level the left and right halves on either side of the ramp, the far
	 * side to the target region's height and the near side to ours.
	 */
	Flatten_Area(Rect(rect.X, rect.Y, half + 2, cell2.Y - rect.Y), region_id);
	Flatten_Area(Rect(cell1.X - half + 2, rect.Y, half + 3, cell1.Y - rect.Y), region_id);
	Flatten_Area(Rect(cell2.X, cell2.Y, half - 2, rect.Height + rect.Y - cell2.Y - 2));
	Flatten_Area(Rect(cell1.X - half + 2, cell1.Y, half - 1, rect.Height + rect.Y - cell1.Y - 2));

	/*
	 * Place the three slope set pieces at the ramp ends.
	 */
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 4), cell2 - Cell(0, 3), region_id, CellHeight);
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 5), cell1 - Cell(2, 3), region_id, CellHeight);
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 6), cell1 - Cell(1, 1), region_id, CellHeight);

	/*
	 * Carve the diagonal jog that absorbs the vertical drift between the two
	 * endpoints. A random pick decides whether the jog sits at the near or the
	 * far end of the ramp; the straight remainder is offset accordingly.
	 */
	Cell cell;
	Cell tail(0, 0);

	if (dy != 0) {
		int rsel = Pick_Random_UInt(0, 1);

		Cell bias;
		bias.X = 0;
		int down = dy > 0;
		bias.Y = dy < 0;

		if (rsel == 1) {
			tail.X = abs_dy;
			tail.Y = -dy;
		} else {
			bias.X = cell1.X - abs_dy - cell2.X - 5;
		}

		int forward = 0;
		if (abs_dy > 0) {
			int backward = 0;
			do {
				static int _heights[] = {3, 2, 1, 0, 0};
				static int _ramps[] = {10, 14, 14, 14, 6, 9, 13, 13, 13, 5};

				for (int row = 0; row < 5; row++) {
					int add_y = down ? backward : forward;

					Cell pos = Cell(cell2.X + 3, cell2.Y) + Cell(forward, -row) + bias;
					cell = Cell(pos.X, pos.Y + add_y);

					CellClass * cellptr = &Map[cell];

					unsigned char height = (unsigned char)_heights[row];
					cellptr->Height = CellHeight + height - 4;

					unsigned char ramp = (unsigned char)_ramps[5 * down + row];
					cellptr->Ramp = ramp;
					cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + ramp - 1);
					cellptr->SubTile = 0;
				}

				forward++;
				backward--;
			} while (forward < abs_dy);
		}
	}

	/*
	 * Extend the straight ramp over whatever horizontal span the jog didn't use.
	 */
	for (int x = 0; x < cell1.X - cell2.X - abs_dy - 5; x++) {
		for (int i = 0; i < 4; i++) {
			cell = Cell(cell2.X + 3, cell2.Y) + Cell(x, -i) + tail;
			CellData::Set_Region(cell, region_id);
			CellClass * cellptr = &Map[cell];
			cellptr->Height = CellHeight - i - 1;
			cellptr->Ramp = 2;
			cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 1);
			cellptr->SubTile = 0;
		}
	}

	return(true);
}


/// <summary>
/// Carves a west facing ramp between two regions.
/// One of the four straight ramp builders, each a duplicate of the same algorithm turned a
/// quarter circle. This routine runs the slope north to south with the low ground off to its
/// west, levels the ground either side of it, and jogs sideways as far as it must to meet
/// both endpoints.
/// </summary>
/// <param name="cell1">The northern endpoint of the ramp.</param>
/// <param name="cell2">The southern endpoint of the ramp.</param>
/// <param name="region_id">The region on the low side, whose height the leveled ground
/// takes.</param>
/// <returns>bool; Was the ramp carved?</returns>
bool MapRegionClass::Build_West_Ramp(Cell const & cell1, Cell const & cell2, int region_id)
{
	/*
	 * Compute a bounding rect around the ramp area and validate it.
	 */
	int x_max = std::max(cell1.X, cell2.X);
	int x_min = std::min(cell1.X, cell2.X);

	Rect rect;
	rect.X      = x_min - 7;
	rect.Y      = cell1.Y - 4;
	rect.Height = cell2.Y - cell1.Y + 9;
	rect.Width  = x_max - x_min + 11;

	if (!Is_Ramp_Area_Clear(rect, region_id)) {
		return(false);
	}

	/*
	 * The ramp body drifts sideways as it runs; reject if the drift is too
	 * large to fit within the vertical span.
	 */
	int dx = cell2.X - cell1.X;
	int dy = cell2.Y - cell1.Y;
	int abs_dx = abs(dx);
	if (abs_dx > dy - 5) {
		return(false);
	}

	int half = (dy + 1) / 2 + 2;

	/*
	 * Level the top and bottom halves on either side of the ramp, the far
	 * side to the target region's height and the near side to ours.
	 */
	Flatten_Area(Rect(rect.X, rect.Y, cell1.X - rect.X, half + 2), region_id);
	Flatten_Area(Rect(rect.X, cell2.Y - half + 3, cell2.X - rect.X, half + 2), region_id);
	Flatten_Area(Rect(cell1.X, cell1.Y, rect.X + rect.Width - cell1.X - 1, half - 1));
	Flatten_Area(Rect(cell2.X, cell2.Y - half + 3, rect.X + rect.Width - cell2.X - 1, half - 2));

	/*
	 * Place the three slope set pieces at the ramp ends.
	 */
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 7), cell1 - Cell(3, 0), region_id, CellHeight);
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 8), cell2 - Cell(3, 2), region_id, CellHeight);
	MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 9), cell2 - Cell(1, 1), region_id, CellHeight);

	/*
	 * Carve the diagonal jog that absorbs the horizontal drift between the two
	 * endpoints. A random pick decides whether the jog sits at the near or the
	 * far end of the ramp; the straight remainder is offset accordingly.
	 */
	Cell cell;
	Cell tail(0, 0);

	if (dx != 0) {
		int rsel = Pick_Random_UInt(0, 1);

		int right = dx > 0;
		Cell bias(dx > 0, 0);

		if (rsel == 1) {
			tail.X = dx;
			tail.Y = abs_dx;
		} else {
			bias.Y = cell2.Y - abs_dx - cell1.Y - 5;
		}

		int forward = 0;
		if (abs_dx > 0) {
			int backward = 0;
			do {
				static int _heights[] = {3, 2, 1, 0, 0};
				static int _ramps[] = {9, 13, 13, 13, 5, 12, 16, 16, 16, 8};

				for (int row = 0; row < 5; row++) {
					int add_x = right ? forward : backward;

					Cell pos = Cell(cell1.X, cell1.Y + 3) + Cell(-row, forward) + bias;
					cell = Cell(pos.X + add_x, pos.Y);

					CellClass * cellptr = &Map[cell];

					unsigned char height = (unsigned char)_heights[row];
					cellptr->Height = CellHeight + height - 4;

					unsigned char ramp = (unsigned char)_ramps[5 * right + row];
					cellptr->Ramp = ramp;
					cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + ramp - 1);
					cellptr->SubTile = 0;
				}

				forward++;
				backward--;
			} while (forward < abs_dx);
		}
	}

	/*
	 * Extend the straight ramp over whatever vertical span the jog didn't use.
	 */
	for (int y = 0; y < cell2.Y - cell1.Y - abs_dx - 5; y++) {
		for (int i = 0; i < 4; i++) {
			cell = Cell(cell1.X, cell1.Y + 3) + Cell(-i, y) + tail;
			CellData::Set_Region(cell, region_id);
			CellClass * cellptr = &Map[cell];
			cellptr->Height = CellHeight - i - 1;
			cellptr->Ramp = 1;
			cellptr->ITType = IsometricTileTypeClass::RampStart;
			cellptr->SubTile = 0;
		}
	}

	return(true);
}


/// <summary>
/// Carves a ramp around the southeast corner of the high ground.
/// One of the four corner ramp builders. This routine joins the high ground to the low
/// ground lying off to its south and east, with a diagonal slope turning the corner between
/// the two straight faces. The corner must measure at least three cells on either arm.
/// </summary>
/// <param name="cell1">The west endpoint of the corner, upon the south facing arm.</param>
/// <param name="cell2">The north endpoint of the corner, upon the east facing arm.</param>
/// <param name="region_id">The region on the low side, whose height the leveled ground
/// takes.</param>
/// <returns>bool; Was the corner ramp carved?</returns>
bool MapRegionClass::Build_Corner_Ramp_SE(Cell const & cell1, Cell const & cell2, int region_id)
{
	int dx = cell2.X - cell1.X;
	if (dx >= 3) {
		int dy = cell1.Y - cell2.Y;
		if (dy >= 3) {
			Rect rect(cell1.X - 2, cell2.Y - 2, dx + 10, dy + 10);
			if (Is_Ramp_Area_Clear(rect, region_id)) {

				Flatten_Area(Rect(rect.X, cell1.Y + 1, rect.Width, 7), region_id);
				Flatten_Area(Rect(cell2.X + 1, rect.Y, 7, rect.Height), region_id);
				Flatten_Area(Rect(cell1.X, cell1.Y - 1, 1, 2));
				Flatten_Area(Rect(cell2.X - 1, cell2.X, 2, 1));
				Flatten_Area(Rect(cell1.X + 1, cell2.Y + 1, cell2.X - cell1.X - 1, cell1.Y - cell2.Y - 1));

				MapGeneratorClass::Place_Tile(IsometricTileTypeClass::SlopeSetPieces, cell1, region_id, CellHeight);
				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 3), cell2, region_id, CellHeight);

				for (int i = 0; i < 4; ++i) {
					CellClass * cptr = &Map[Cell(cell2.X + i, cell1.Y + i)];
					cptr->Height = CellHeight - (char)i - 1;
					cptr->Ramp = 7;
					cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 6);
					cptr->SubTile = 0;
				}

				for (int j = 0; j < 4; ++j) {
					for (int k = 0; k < cell2.X - cell1.X + j - 3; ++k) {
						CellClass * cptr = &Map[Cell(cell1.X + k + 3, cell1.Y + j)];
						cptr->Height = CellHeight - (char)j - 1;
						cptr->Ramp = 4;
						cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 3);
						cptr->SubTile = 0;
					}
				}

				for (int m = 0; m < 4; ++m) {
					for (int n = 0; n < cell1.Y - cell2.Y + m - 3; ++n) {
						CellClass * cptr = &Map[Cell(cell2.X + m, cell2.Y + n + 3)];
						cptr->Height = CellHeight - (char)m - 1;
						cptr->Ramp = 3;
						cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 2);
						cptr->SubTile = 0;
					}
				}

				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Carves a ramp around the northeast corner of the high ground.
/// One of the four corner ramp builders. This routine joins the high ground to the low
/// ground lying off to its north and east, with a diagonal slope turning the corner between
/// the two straight faces. The corner must measure at least three cells on either arm.
/// </summary>
/// <param name="cell1">The lower right endpoint of the corner.</param>
/// <param name="cell2">The upper left endpoint of the corner.</param>
/// <param name="region_id">The region on the low side, whose height the leveled ground
/// takes.</param>
/// <returns>bool; Was the corner ramp carved?</returns>
bool MapRegionClass::Build_Corner_Ramp_NE(Cell const & cell1, Cell const & cell2, int region_id)
{
	int dx = cell1.X - cell2.X;
	if (dx >= 3) {
		int dy = cell1.Y - cell2.Y;
		if (dy >= 3) {
			Rect rect(cell2.X - 2, cell2.Y - 7, dx + 10, dy + 10);
			if (Is_Ramp_Area_Clear(rect, region_id)) {

				Flatten_Area(Rect(rect.X, rect.Y, rect.Width, 7), region_id);
				Flatten_Area(Rect(cell1.X + 1, rect.Y, 7, rect.Height), region_id);
				Flatten_Area(Rect(cell2.X, cell2.Y, 1, 2));
				Flatten_Area(Rect(cell1.X - 1, cell1.Y, 2, 1));
				Flatten_Area(Rect(cell2.X + 1, cell2.Y + 1, cell1.X - cell2.X - 1, cell1.Y - cell2.Y - 1));

				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 2), Cell(cell1.X, cell1.Y - 2), region_id, CellHeight);
				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 4), Cell(cell2.X, cell2.Y - 3), region_id, CellHeight);

				for (int i = 0; i < 4; ++i) {
					CellClass * cptr = &Map[Cell(cell1.X + i, cell2.Y - i)];
					cptr->Height = CellHeight - (char)i - 1;
					cptr->Ramp = 6;
					cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 5);
					cptr->SubTile = 0;
				}

				for (int j = 0; j < 4; ++j) {
					for (int k = 0; k < cell1.Y - cell2.Y + j - 3; ++k) {
						CellClass * cptr = &Map[Cell(cell1.X + j, cell1.Y - k - 3)];
						cptr->Height = CellHeight - (char)j - 1;
						cptr->Ramp = 3;
						cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 2);
						cptr->SubTile = 0;
					}
				}

				for (int m = 0; m < 4; ++m) {
					for (int n = 0; n < cell1.X - cell2.X + m - 3; ++n) {
						CellClass * cptr = &Map[Cell(cell2.X + n + 3, cell2.Y - m)];
						cptr->Height = CellHeight - (char)m - 1;
						cptr->Ramp = 2;
						cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 1);
						cptr->SubTile = 0;
					}
				}

				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Carves a ramp around the northwest corner of the high ground.
/// One of the four corner ramp builders. This routine joins the high ground to the low
/// ground lying off to its north and west, with a diagonal slope turning the corner between
/// the two straight faces. The corner must measure at least three cells on either arm.
/// </summary>
/// <param name="cell2">The upper right endpoint of the corner.</param>
/// <param name="cell1">The lower left endpoint of the corner.</param>
/// <param name="region_id">The region on the low side, whose height the leveled ground
/// takes.</param>
/// <returns>bool; Was the corner ramp carved?</returns>
bool MapRegionClass::Build_Corner_Ramp_NW(Cell const & cell2, Cell const & cell1, int region_id)
{
	int dx = cell2.X - cell1.X;
	if (dx >= 3) {
		int dy = cell1.Y - cell2.Y;
		if (dy >= 3) {
			Rect rect(cell1.X - 7, cell2.Y - 7, dx + 10, dy + 10);
			if (Is_Ramp_Area_Clear(rect, region_id)) {

				Flatten_Area(Rect(rect.X, rect.Y, rect.Width, 7), region_id);
				Flatten_Area(Rect(rect.X, rect.Y, 7, rect.Height), region_id);
				Flatten_Area(Rect(cell2.X, cell2.Y, 1, 2));
				Flatten_Area(Rect(cell1.X, cell1.Y, 2, 1));
				Flatten_Area(Rect(cell1.X + 1, cell2.Y + 1, cell2.X - cell1.X - 1, cell1.Y - cell2.Y - 1));

				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 5), cell2 - Cell(2, 3), region_id, CellHeight);
				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 6), cell2 - Cell(1, 1), region_id, CellHeight);
				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 8), cell1 - Cell(3, 2), region_id, CellHeight);
				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 9), cell1 - Cell(1, 1), region_id, CellHeight);

				for (int i = 0; i < 4; ++i) {
					CellClass * cptr = &Map[Cell(cell1.X - i, cell2.Y - i)];
					cptr->Height = CellHeight - (char)i - 1;
					cptr->Ramp = 5;
					cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 4);
					cptr->SubTile = 0;
				}

				for (int j = 0; j < 4; ++j) {
					for (int k = 0; k < cell2.X - cell1.X + j - 3; ++k) {
						CellClass * cptr = &Map[cell2 - Cell(k + 3, j)];
						cptr->Height = CellHeight - (char)j - 1;
						cptr->Ramp = 2;
						cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 1);
						cptr->SubTile = 0;
					}
				}

				for (int m = 0; m < 4; ++m) {
					for (int n = 0; n < cell1.Y - cell2.Y + m - 3; ++n) {
						CellClass * cptr = &Map[cell1 - Cell(m, n + 3)];
						cptr->Height = CellHeight - (char)m - 1;
						cptr->Ramp = 1;
						cptr->ITType = IsometricTileTypeClass::RampStart;
						cptr->SubTile = 0;
					}
				}

				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Carves a ramp around the southwest corner of the high ground.
/// One of the four corner ramp builders. This routine joins the high ground to the low
/// ground lying off to its south and west, with a diagonal slope turning the corner between
/// the two straight faces. The corner must measure at least three cells on either arm.
/// </summary>
/// <param name="cell1">The upper left endpoint of the corner.</param>
/// <param name="cell2">The lower right endpoint of the corner.</param>
/// <param name="region_id">The region on the low side, whose height the leveled ground
/// takes.</param>
/// <returns>bool; Was the corner ramp carved?</returns>
bool MapRegionClass::Build_Corner_Ramp_SW(Cell const & cell1, Cell const & cell2, int region_id)
{
	int dx = cell2.X - cell1.X;
	if (dx >= 3) {
		int dy = cell2.Y - cell1.Y;
		if (dy >= 3) {
			Rect rect(cell1.X - 7, cell1.Y - 4, dx + 10, dy + 10);
			if (Is_Ramp_Area_Clear(rect, region_id)) {
				Flatten_Area(Rect(rect.X, rect.Y, 7, rect.Height), region_id);
				Flatten_Area(Rect(rect.X, cell2.Y + 1, rect.Width, 7), region_id);
				Flatten_Area(Rect(cell1.X, cell1.Y, 2, 1));
				Flatten_Area(Rect(cell2.X, cell2.Y - 1, 1, 2));
				Flatten_Area(Rect(cell1.X + 1, cell1.Y + 1, cell2.X - cell1.X - 1, cell2.Y - cell1.Y - 1));

				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 7), Cell(cell1.X - 3, cell1.Y), region_id, CellHeight);
				MapGeneratorClass::Place_Tile((IsometricTileType)(IsometricTileTypeClass::SlopeSetPieces + 1), Cell(cell2.X - 2, cell2.Y), region_id, CellHeight);

				for (int i = 0; i < 4; ++i) {
					CellClass * cptr = &Map[Cell(cell1.X - i, cell2.Y + i)];
					cptr->Height = CellHeight - (char)i - 1;
					cptr->Ramp = 8;
					cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 7);
					cptr->SubTile = 0;
				}

				for (int j = 0; j < 4; ++j) {
					for (int k = 0; k < cell2.Y - cell1.Y + j - 3; ++k) {
						CellClass * cptr = &Map[Cell(cell1.X - j, cell1.Y + k + 3)];
						cptr->Height = CellHeight - (char)j - 1;
						cptr->Ramp = 1;
						cptr->ITType = IsometricTileTypeClass::RampStart;
						cptr->SubTile = 0;
					}
				}

				for (int m = 0; m < 4; ++m) {
					for (int n = 0; n < cell2.X - cell1.X + m - 3; ++n) {
						CellClass * cptr = &Map[Cell(cell2.X - n - 3, cell2.Y + m)];
						cptr->Height = CellHeight - (char)m - 1;
						cptr->Ramp = 4;
						cptr->ITType = (IsometricTileType)(IsometricTileTypeClass::RampStart + 3);
						cptr->SubTile = 0;
					}
				}

				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Is there room upon the map to carve a ramp here?
/// Every ramp builder asks this routine before it disturbs a single cell, so that a ramp is
/// never left half carved. The area must lie on the playable map, be clear of anything
/// already laid down, and belong to no region but the two being joined.
/// </summary>
/// <param name="rect">The area the ramp would occupy.</param>
/// <param name="id">The other region allowed to hold cells here, besides this one.</param>
/// <returns>bool; Is the area free for the ramp?</returns>
bool MapRegionClass::Is_Ramp_Area_Clear(Rect & rect, int id)
{
	if (Map.In_Local_Radar(Cell(rect.X, rect.Y)) &&
		Map.In_Local_Radar(Cell(rect.X + rect.Width - 1, rect.Y)) &&
		Map.In_Local_Radar(Cell(rect.X, rect.Y + rect.Height - 1)) &&
		Map.In_Local_Radar(Cell(rect.X + rect.Width - 1,rect.Y + rect.Height - 1))) {

		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			for (int x = rect.X; x < rect.X + rect.Width; x++) {
				Cell cell(x, y);
				int rid = CellData::Get_Region(cell);
				if (rid != ID && rid != id) {
					return(false);
				}
				CellClass * cptr = &Map[cell];
				if (!cptr->Is_Tile_Clear()) {
					return(false);
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Levels an area of ground into another region.
/// This is the counterpart of the one argument version, used by the ramp builders to prepare
/// the ground on the far side of a slope -- the side belonging to the region being joined to
/// rather than to this one.
/// </summary>
/// <param name="rect">The area of cells to level.</param>
/// <param name="region_id">The region to claim the cells for, and to take the height from.</param>
void MapRegionClass::Flatten_Area(Rect const & rect, int region_id)
{
	for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
		for (int x = rect.X; x < rect.X + rect.Width; x++) {
			Cell cell(x, y);
			if (My_In_Radar(cell)) {
				CellData::Set_Region(cell, region_id);
				CellClass * cellptr = &Map[cell];
				cellptr->Height = From_ID(region_id)->CellHeight;
			}
		}
	}
}


/// <summary>
/// Levels an area of ground into this region.
/// The cells are claimed by this region and dragged to its height. This routine is used by
/// the ramp builders to prepare the flat ground on the near side of a slope.
/// </summary>
/// <param name="rect">The area of cells to level.</param>
void MapRegionClass::Flatten_Area(Rect const & rect)
{
	for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
		for (int x = rect.X; x < rect.X + rect.Width; x++) {
			Cell cell(x, y);
			if (My_In_Radar(cell)) {
				CellData::Set_Region(cell, ID);
				Map[cell].Height = CellHeight;
			}
		}
	}
}


/// <summary>
/// Fetches a region by the number it is known by.
/// The cell data grid records nothing but region numbers, so every routine that works from
/// the grid comes back through here to reach the region itself.
/// </summary>
/// <param name="id">The region number to look up.</param>
/// <returns>Returns with a pointer to the region. Otherwise, NULL is returned if no region
/// carries that number.</returns>
MapRegionClass * MapRegionClass::From_ID(int id)
{
	for (int i = 0; i < MapRegions.Count(); i++) {
		if (MapRegions[i]->ID == id) {
			return(MapRegions[i]);
		}
	}
	return(NULL);
}


/// <summary>
/// Picks the player start locations for a random map.
/// This routine hunts for open buildable ground on the larger land masses, and only on the
/// ones the players can actually reach one another across. The starts are then spread as far
/// apart as the candidates allow, and whatever candidates are left over are handed on as the
/// layout for the tiberium fields.
/// </summary>
/// <returns>bool; Was a start location found for every player?</returns>
bool Generate_Starting_Points(void)
{
	int zone = Map.Zone_Reset();

	int region_size_threshold = std::max(600.0, ((double)RandomMapGen.LocalHeight * (double)RandomMapGen.LocalWidth * 0.06));

	DynamicVectorClass<Cell> *cells = new DynamicVectorClass<Cell>;
	cells->Set_Growth_Step(200);

	DynamicVectorClass<int> *regions = new DynamicVectorClass<int>;

	int i;
	for (i = 0; i < MapRegionClass::MapRegions.Count(); i++) {
		MapRegionClass::MapRegions[i]->Recount_Cells();
	}

	for (i = 0; i < MapRegionClass::MapRegions.Count(); i++) {
		MapRegionClass *region = MapRegionClass::MapRegions[i];
		if (region->CellCount > region_size_threshold) {
			if (Map.Get_Cell_Zone(region->Pick_Random_Clear_Cell(), MZONE_NORMAL, false) == zone) {
				regions->Add(region->ID);
			}
		}
	}

	Rect bounds(Map.LocalRect.X + 4, Map.LocalRect.Y + 4, Map.LocalRect.Width - 8, Map.LocalRect.Height - 8);

	while (cells->Count() < 15 * RandomMapGen.SeedData.NumPlayers) {
		Cell c(0,0);

		if (regions->Count() > 0) {
			c = MapRegionClass::Pick_Random_Clear_Cell_In_Zone(zone, (*regions)[Pick_Random_UInt(0, regions->Count() - 1)]);
		} else {
			c = MapRegionClass::Pick_Random_Clear_Map_Cell();
		}

		if (c != Cell(0,0)) {
			Rect rect(c.X - 5, c.Y - 5, 10, 10);
			if (MapGeneratorClass::Can_Place_Paved_Road(rect, false, false) && Map.In_Area_Radar(bounds, c, true)) {
				cells->Add(c);
			}
		}
	}

	if (cells->Count() < RandomMapGen.SeedData.NumPlayers) {
		if (cells != NULL) {
			delete cells;
		}
		if (regions != NULL) {
			delete regions;
		}

		return(false);
	}

	DynamicVectorClass<Cell> *layout = Pick_Spread_Cells(*cells);

	for (i = 0; i < RandomMapGen.SeedData.NumPlayers; i++) {
		Scen->Set_Waypoint(i, (*layout)[i]);
		Map[Scen->Get_Waypoint_Cell(i)].IsWaypoint = true;
	}

	for (i = 0; i < RandomMapGen.SeedData.NumPlayers; i++) {
		if (layout->Count() > 0) {
			layout->Delete_Index(0);
		}
	}

	delete TiberiumLayoutCells;
	TiberiumLayoutCells = layout;

	if (cells != NULL) {
		delete cells;
	}
	if (regions != NULL) {
		delete regions;
	}

	return(true);
}


/// <summary>
/// Chooses a well spread out subset of the candidate cells.
/// This routine is used to place the things that must not end up bunched together -- the
/// player start points and the tiberium fields. Cells in different regions are reckoned
/// further apart than they really are, so the picks favor separate pieces of ground.
/// </summary>
/// <param name="cells">The candidate cells to choose from.</param>
/// <returns>Returns with a newly allocated list of the cells chosen. The caller owns it.</returns>
DynamicVectorClass<Cell> * Pick_Spread_Cells(DynamicVectorClass<Cell> & cells)
{
	double max_distance = -1.0;
	int max_cell_index = -1;
	double layout_fraction = (double)RandomMapGen.SeedData.TiberiumLayout / 100.0;
	int target_count = (int)(layout_fraction * 10.0 + (double)(RandomMapGen.SeedData.NumPlayers + 3));

	DynamicVectorClass<Cell> *layout = new DynamicVectorClass<Cell>;

	int current = 0;
	int current_pos = 0;
	int first = 0;
	if (cells.Count() - 1 > 0) {
		do {
			current = first;
			Cell c1 = cells[current];
			int region1 = MapRegionClass::CellData::Get_Region(c1);

			current_pos = ++current;
			if (current < cells.Count()) {
				int c1x = c1.X;
				int c1y = c1.Y;
				do {
					Cell c2 = cells[current];
					int dx = c1x - c2.X;
					int dy = c1y - c2.Y;
					int region2 = MapRegionClass::CellData::Get_Region(c2);

					double score = std::sqrt((double)(dy * dy + dx * dx)) + (double)(region2 != region1 ? 20 : 0);

					/*
					 * The check against zero here is always true; the inner cursor
					 * starts past the first cell.
					 */
					if (current) {
						if (score > max_distance) {
							max_distance = score;
							max_cell_index = first;
						}
					}

					++current;
				} while (current < cells.Count());

				current = current_pos;
			}
			first = current;
		} while (current < cells.Count() - 1);
	}

	layout->Add(cells[max_cell_index]);

	while (layout->Count() < target_count) {
		double best_score = -1.0;
		int best_index = -1;

		for (first = 0; first < cells.Count(); first++) {
			Cell c1 = cells[first];
			int region1 = MapRegionClass::CellData::Get_Region(c1);

			double min_score = 9999999.0;
			for (int j = 0; j < layout->Count(); j++) {
				Cell c2 = (*layout)[j];
				int dx = c1.X - c2.X;
				int dy = c1.Y - c2.Y;
				int region2 = MapRegionClass::CellData::Get_Region(c2);

				double score = std::sqrt((double)(dy * dy + dx * dx)) + (double)(region2 != region1 ? 20 : 0);
				if (score < min_score) {
					min_score = score;
				}
			}

			if (min_score > best_score) {
				best_score = min_score;
				best_index = first;
			}
		}

		layout->Add(cells[best_index]);
	}

	return(layout);
}


/// <summary>
/// Creates a random map seed with the default settings.
/// This routine establishes the map parameters the random map dialog offers the player
/// before anything has been chosen, and gives the seed the file extension and description
/// that the load and save dialogs list it under.
/// </summary>
MapSeedClass::MapSeedClass(void) :
	Biome(BIOME_TUNDRA),
	Hills(0),
	Time(TIME_OF_DAY_AFTERNOON),
	WaterAmount(0),
	NumPlayers(2),
	Tiberium(0),
	TiberiumLayout(0),
	Vegetation(0),
	Cities(0),
	Width(0),
	Height(0),
	Accessibility(0),
	Cliffs(0),
	Seed(-1),
	TiberiumWildlife(0),
	VeinholeMonsters(0),
	UseIonStorms(0),
	UseBlueTiberium(0),
	UseTransitions(0)
{
	Extension = "SED";
	memset(MapDescription, 0, sizeof(MapDescription));
	strcpy(MapDescription, Fetch_String(TXT_RANDOM_MAP_DESCRIPTION));
	Description = MapDescription;
}


/// <summary>
/// Constructs the random map generator.
/// The game keeps a single generator for its whole run; this routine merely puts it into the
/// idle state it must be in before the first map is asked of it.
/// </summary>
MapGeneratorClass::MapGeneratorClass(void) :
	SeedData(),
	MapSeeder(NULL),
	MapPreview(NULL),
	LocalWidth(0),
	LocalHeight(0),
	SeededWaterAmount(0),
	PlaceWaterfall(0),
	CellHeight(4)
{
	//nothing
}


/// <summary>
/// Destroys the random map generator.
/// This routine gives back the regions, the cell data grid, and the preview and seed objects
/// the generator accumulated while it was building maps.
/// </summary>
MapGeneratorClass::~MapGeneratorClass(void)
{
	MapRegionClass::Destroy_Cell_Regions();

	if (RMGCellData != NULL) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	if (MapSeeder != NULL) {
		delete MapSeeder;
		MapSeeder = NULL;
	}

	if (MapPreview != NULL) {
		delete MapPreview;
		MapPreview = NULL;
	}
}


/// <summary>
/// Runs the random map generator dialog.
/// Use this routine to let the player lay out and preview a random battlefield before a
/// skirmish or multiplayer game begins. It does not return until the player accepts the map
/// or gives up on it, and the title screen behind is kept alive in the meantime.
/// </summary>
/// <param name="callback">Progress callback to run while the dialog is up.</param>
/// <returns>Returns with the dialog result -- 1 if the player accepted the map, 2 if the
/// dialog was canceled, and 0 if it could not be opened at all.</returns>
int Do_Random_Map_Dialog(bool (*callback)())
{
	WDTTerritory *wdt = NULL;
	LONG res = 0;

	if (Session.Type == GAME_INTERNET && Session.IsWDT) {
		wdt = WDT_Get_Territory(Session.WDTTerritory);
	}

	HWND dialog;
	if (Addon_Enabled(ADDON_FIRESTORM)) {
		dialog = OwnerDraw::Begin_Dialog(wdt != NULL ? IDD_MAPGEN_WDT : IDD_MAPGEN_FS, Map_Seed_Dialog_Proc);
	} else {
		dialog = OwnerDraw::Begin_Dialog(IDD_MAPGEN, Map_Seed_Dialog_Proc);
	}

	if (dialog) {
		RMGCallback = callback;
		RandomMapGen.SeedData.Callback = callback;
		SetWindowLongA(dialog, DWL_USER, (LONG)&res);
		OwnerDraw::Display_Dialog(dialog);
		while (res == 0) {
			if (OwnerDraw::Dialog_Message_Handler() == 1) {
				break;
			}
			if (callback != NULL) {
				callback();
			}
			Title_Screen_Restore(false);
		}
		OwnerDraw::End_Dialog(dialog);
	}

	RMGCallback = MapGen_Call_Back;
	RandomMapGen.SeedData.Callback = NULL;

	if (RandomMapGen.MapPreview != NULL) {
		if (RandomMapGen.MapPreview->Get_Preview_Surface() != NULL) {
			RawFileClass file("RandMap.img");
			Write_PCX_File(file, *RandomMapGen.MapPreview->Get_Preview_Surface(), &GamePalette);
		}
		delete RandomMapGen.MapPreview;
		RandomMapGen.MapPreview = NULL;
	}

	if (RandomMapGen.MapSeeder != NULL) {
		delete RandomMapGen.MapSeeder;
		RandomMapGen.MapSeeder = NULL;
	}

	return(res);
}


/// <summary>
/// Prunes the random map preview cache.
/// This routine is called after a fresh preview has been cached. It throws away the previews
/// that have gone longest untouched, so that the cache directory cannot grow forever.
/// </summary>
void Clean_Up_RMCache(void)
{
	WIN32_FIND_DATA *ff;

	DynamicVectorClass<WIN32_FIND_DATA *> files;

	ff = new WIN32_FIND_DATA;
	HANDLE handle = FindFirstFile("rmcache\\*.mmp", ff);

	if (handle != INVALID_HANDLE_VALUE) {
		files.Add(ff);

		ff = new WIN32_FIND_DATA;
		while (FindNextFile(handle, ff) != 0) {
			files.Add(ff);
			ff = new WIN32_FIND_DATA;
		}
		FindClose(handle);
	}

	/*
	 * The last record was allocated but never added to the list (the
	 * enumeration ended). Free it here.
	 */
	delete ff;

	/*
	 * Limit the cache to a fixed number of entries. While there are too many
	 * cached maps, repeatedly find the oldest file and delete it.
	 */
	while (files.Count() > 70) {
		FILETIME oldesttime;
		oldesttime.dwLowDateTime = 0xFFFFFFFF;
		oldesttime.dwHighDateTime = 0x7FFFFFFF;

		int oldest = -1;
		for (int index = 0; index < files.Count(); index++) {
			WIN32_FIND_DATA * file = files[index];

			if (file->ftLastAccessTime.dwHighDateTime != 0) {
				if (CompareFileTime(&file->ftLastAccessTime, &oldesttime) == -1) {
					oldest = index;
					oldesttime.dwLowDateTime = files[index]->ftLastAccessTime.dwLowDateTime;
					oldesttime.dwHighDateTime = files[index]->ftLastAccessTime.dwHighDateTime;
				}
			} else if (file->ftCreationTime.dwHighDateTime != 0) {
				if (CompareFileTime(&file->ftCreationTime, &oldesttime) == -1) {
					oldest = index;
					oldesttime.dwLowDateTime = files[index]->ftCreationTime.dwLowDateTime;
					oldesttime.dwHighDateTime = files[index]->ftCreationTime.dwHighDateTime;
				}
			} else if (file->ftLastWriteTime.dwHighDateTime != 0) {
				if (CompareFileTime(&file->ftLastWriteTime, &oldesttime) == -1) {
					oldest = index;
					oldesttime.dwLowDateTime = files[index]->ftLastWriteTime.dwLowDateTime;
					oldesttime.dwHighDateTime = files[index]->ftLastWriteTime.dwHighDateTime;
				}
			}
		}

		if (oldest == -1) {
			break;
		}

		if (!DeleteFile(files[oldest]->cFileName)) {
			break;
		}

		delete files[oldest];
		files.Delete_Index(oldest);
	}

	while (files.Count()) {
		delete files[0];
		files.Delete_Index(0);
	}
}


/// <summary>
/// Generates the random map the player has asked for.
/// This routine is what the map generator dialog's preview and generate buttons come down
/// to. A map that has been built for these exact settings before is kept in a cache and
/// merely fetched back, so flipping between two seeds costs nothing the second time. The
/// finished preview is left in RandMap.img for the lobby to show.
/// </summary>
/// <param name="dialog">The dialog to show generation progress within.</param>
/// <param name="callback">Progress callback to run while the map is being built.</param>
void Do_Random_Map(HWND dialog, bool (*callback)())
{
	if (Session.Type == GAME_INTERNET && Session.IsWDT && WDT_Get_Territory(Session.WDTTerritory) != NULL) {
		RandomMapGen.SeedData.NumPlayers = 4;
	}
	char *digest = CalcRandomMapDigest();
	char name[128];
	memset(name, 0, sizeof(name));
	strncpy(name, "rmcache\\", sizeof(name));
	strncat(name, digest, sizeof(name));
	delete digest;
	strncat(name, ".mmp", sizeof(name));
	DebugString("Cache filename is %s\n", name);
	CCFileClass cfile(name);

	if (cfile.Is_Available()) {
		if (RandomMapGen.MapPreview == NULL) {
			RandomMapGen.MapPreview = new MapPreviewClass;
		}
		if (RandomMapGen.MapPreview->Read_PCX_Preview(name)) {
			RawFileClass file("RandMap.img");
			Write_PCX_File(file, *RandomMapGen.MapPreview->Get_Preview_Surface(), &GamePalette);
			delete RandomMapGen.MapPreview;
			RandomMapGen.MapPreview = NULL;
			return;
		}

		delete RandomMapGen.MapPreview;
		RandomMapGen.MapPreview = NULL;
	}

	RMGCallback = callback;
	RandomMapGen.SeedData.Callback = callback;
	if (RandomMapGen.SeedData.Seed == -1) {
		RandomMapGen.SeedData.Seed = Sim_Random_Pick(0U, 65535U);
	}
	RandomMapGen.Generate_Random_Map(true, dialog);
	RandomMapGen.MapPreview->Create_Preview();

	if (RandomMapGen.MapSeeder != NULL) {
		delete RandomMapGen.MapSeeder;
	}

	RandomMapGen.MapSeeder = new MapSeedClass;
	memcpy(RandomMapGen.MapSeeder, &RandomMapGen.SeedData, sizeof(RandomMapGen.SeedData));

	Scen->Set_Scenario_Name(Fetch_String(TXT_RANDOM_MAP_DESCRIPTION));
	Title_Screen_Restore();

	if (RandomMapGen.MapPreview != NULL) {
		if (RandomMapGen.MapPreview->Get_Preview_Surface() != NULL) {
			RawFileClass file("RandMap.img");
			Write_PCX_File(file, *RandomMapGen.MapPreview->Get_Preview_Surface(), &GamePalette);
			WIN32_FIND_DATA ff;
			if (FindFirstFile("rmcache", &ff) == INVALID_HANDLE_VALUE) {
				CreateDirectory("rmcache", 0);
			}
			CopyFile("RandMap.img", name, FALSE);
			Clean_Up_RMCache();
		}
		delete RandomMapGen.MapPreview;
		RandomMapGen.MapPreview = NULL;
	}

	if (RandomMapGen.MapSeeder != NULL) {
		delete RandomMapGen.MapSeeder;
		RandomMapGen.MapSeeder = NULL;
	}
}


/// <summary>
/// Dialog procedure for the random map generator ("Map Seed") dialog.
/// Handles previewing, generating, saving, loading and deleting random maps, and randomizing
/// the generator settings. The dialog's result code is written through the DWL_USER pointer set
/// up by Do_Random_Map_Dialog so that writing it ends that dialog's modal message loop.
/// </summary>
/// <param name="window">Handle to the dialog window.</param>
/// <param name="message">Window message identifier.</param>
/// <param name="wparam">Message-specific first parameter.</param>
/// <param name="lparam">Message-specific second parameter.</param>
/// <returns>TRUE if the message was processed, FALSE otherwise.</returns>
BOOL CALLBACK Map_Seed_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	static int _unused = -1;

	BOOL result = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (result) {
		return(result);
	}

	LONG * state = (LONG *)GetWindowLongA(window, DWL_USER);

	switch (message) {

		/*
		 * Repaint the map preview, if one exists.
		 */
		case WM_PAINT:
			if (RandomMapGen.MapPreview != NULL) {
				RandomMapGen.MapPreview->Blit_Preview(window);
			}
			ValidateRect(window, NULL);
			return(0);

		/*
		 * Initialize the dialog controls from the current seed settings.
		 */
		case WM_INITDIALOG: {
			_unused = -1;
			HWND handle = GetDlgItem(window, IDC_MAPGEN_PREVIEW);
			if (Debug_Map) {
				EnableWindow(handle, false);
			} else {
				EnableWindow(handle, true);
			}
			if (RandomMapGen.SeedData.Seed == -1) {
				RandomMapGen.SeedData.Seed = Sim_Random_Pick(0U, 65535U);
			}
			RandomMapGen.SeedData.Set_Settings(window);

			bool enable = RandomMapGen.SeedData.Files_Present();
			handle = GetDlgItem(window, IDC_MAPGEN_LOAD_MAP);
			if (handle != NULL) {
				EnableWindow(handle, enable);
			}
			handle = GetDlgItem(window, IDC_MAPGEN_DELETE_MAP);
			if (handle == NULL) {
				return(0);
			}
			EnableWindow(handle, enable);
			return(0);
		}

		case WM_COMMAND:
			switch (LOWORD(wparam)) {

				/*
				 * Generate the map and accept the dialog.
				 */
				case IDOK:
					RandomMapGen.SeedData.Get_Settings(window);
					if (Debug_Map) {
						RandomMapGen.Generate_Random_Map(false, window);
						Scen->Set_Scenario_Name(Fetch_String(TXT_RANDOM_MAP_DESCRIPTION));
						Write_Scenario_INI("RandMap.Map", true);
					} else {
						if (RandomMapGen.MapPreview == NULL || RandomMapGen.MapPreview->Get_Preview_Surface() == NULL) {
							RandomMapGen.Generate_Random_Map(true, window);
							if (Debug_Map) {
								Scen->Set_Scenario_Name(Fetch_String(TXT_RANDOM_MAP_DESCRIPTION));
								Write_Scenario_INI("RandMap.Map", true);
							}
						}
					}
					*state = 1;
					return(1);

				/*
				 * Cancel the dialog.
				 */
				case IDCANCEL:
					*state = 2;
					return(1);

				/*
				 * Load a saved map seed.
				 */
				case IDC_MAPGEN_LOAD_MAP:
					RandomMapGen.SeedData.Get_Settings(window);
					if (RandomMapGen.SeedData.LoadOptionsClass::Load() == true) {
						PostMessageA(window, WM_COMMAND, MAKEWPARAM(IDC_MAPGEN_PREVIEW, BN_CLICKED), (LPARAM)GetDlgItem(window, IDC_MAPGEN_PREVIEW));
					}
					RandomMapGen.SeedData.Set_Settings(window);
					return(0);

				/*
				 * Save the current map seed.
				 */
				case IDC_MAPGEN_SAVE_MAP: {
					RandomMapGen.SeedData.Get_Settings(window);
					RandomMapGen.SeedData.MapDescription[0] = '\0';
					RandomMapGen.SeedData.LoadOptionsClass::Save(RandomMapGen.SeedData.MapDescription);

					bool enable = RandomMapGen.SeedData.Files_Present();
					HWND handle = GetDlgItem(window, IDC_MAPGEN_LOAD_MAP);
					if (handle != NULL) {
						EnableWindow(handle, enable);
					}
					handle = GetDlgItem(window, IDC_MAPGEN_DELETE_MAP);
					if (handle == NULL) {
						return(0);
					}
					EnableWindow(handle, enable);
					return(0);
				}

				/*
				 * Delete the saved map seed.
				 */
				case IDC_MAPGEN_DELETE_MAP: {
					RandomMapGen.SeedData.Get_Settings(window);
					RandomMapGen.SeedData.LoadOptionsClass::Delete();

					bool enable = RandomMapGen.SeedData.Files_Present();
					HWND handle = GetDlgItem(window, IDC_MAPGEN_LOAD_MAP);
					if (handle != NULL) {
						EnableWindow(handle, enable);
					}
					handle = GetDlgItem(window, IDC_MAPGEN_DELETE_MAP);
					if (handle == NULL) {
						return(0);
					}
					EnableWindow(handle, enable);
					return(0);
				}

				/*
				 * Build and display a preview of the current map seed.
				 */
				case IDC_MAPGEN_PREVIEW:
					RandomMapGen.SeedData.Get_Settings(window);
					RandomMapGen.Generate_Random_Map(true, window);
					RandomMapGen.MapPreview->Create_Preview();
					if (RandomMapGen.MapSeeder != NULL) {
						delete RandomMapGen.MapSeeder;
					}
					RandomMapGen.MapSeeder = new MapSeedClass;
					memcpy(RandomMapGen.MapSeeder, &RandomMapGen.SeedData, sizeof(MapSeedClass));
					PostMessageA(window, WM_PAINT, 0, 0);
					return(0);

				/*
				 * Randomize the generator settings.
				 */
				case IDC_MAPGEN_SURPRISE:
					RandomMapGen.SeedData.Get_Settings(window);
					RandomMapGen.SeedData.Randomize();
					RandomMapGen.SeedData.Set_Settings(window);
					return(0);

				default:
					return(0);
			}

		default:
			return(0);
	}
}


/// <summary>
/// Reads the map generator dialog into these settings.
/// This routine is called before a preview or a generate, so that whatever the player has
/// dialed in on the controls becomes the seed the generator works from. The settings taken
/// off the dialog are run through Fixup_Settings, so an impossible combination can never
/// reach the generator. The Firestorm settings are cleared away when that addon is absent.
/// </summary>
/// <param name="dialog">The map generator dialog to read.</param>
void MapSeedClass::Get_Settings(HWND dialog)
{
	WDTTerritory * wdt = NULL;
	if (Session.Type == GAME_INTERNET && Session.IsWDT) {
		wdt = WDT_Get_Territory(Session.WDTTerritory);
	}

	HWND handle;
	char str[30];

	handle = GetDlgItem(dialog, IDC_MAPGEN_ENVIRONMENT);
	Biome = ComboBox_GetItemData(handle, ComboBox_GetCurSel(handle));

	handle = GetDlgItem(dialog, IDC_MAPGEN_TIME_OF_DAY);
	Time = ComboBox_GetItemData(handle, ComboBox_GetCurSel(handle));

	handle = GetDlgItem(dialog, IDC_MAPGEN_MAP_WIDTH);
	Width = ComboBox_GetItemData(handle, ComboBox_GetCurSel(handle));

	handle = GetDlgItem(dialog, IDC_MAPGEN_MAP_HEIGHT);
	Height = ComboBox_GetItemData(handle, ComboBox_GetCurSel(handle));

	handle = GetDlgItem(dialog, IDC_MAPGEN_DIMENSION_EDIT);
	GetWindowText(handle, str, ARRAY_SIZE(str));
	Seed = atoi(str);

	handle = GetDlgItem(dialog, IDC_MAPGEN_TIBERIUM_AMOUNT);
	Tiberium = Slider_GetPos(handle);

	if (wdt != NULL) {
		NumPlayers = 4;
	} else {
		handle = GetDlgItem(dialog, IDC_MAPGEN_PLAYERS);
		NumPlayers = Slider_GetPos(handle);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_HILLS);
	Hills = Slider_GetPos(handle);

	handle = GetDlgItem(dialog, IDC_MAPGEN_WATER);
	WaterAmount = Slider_GetPos(handle);

	handle = GetDlgItem(dialog, IDC_MAPGEN_CLIFFS);
	Cliffs = Slider_GetPos(handle);

	handle = GetDlgItem(dialog, IDC_MAPGEN_VEGETATION);
	Vegetation = Slider_GetPos(handle);

	handle = GetDlgItem(dialog, IDC_MAPGEN_CITIES);
	Cities = Slider_GetPos(handle);

	handle = GetDlgItem(dialog, IDC_MAPGEN_ACCESSIBILITY);
	Accessibility = Slider_GetPos(handle);

	handle = GetDlgItem(dialog, IDC_MAPGEN_TIBERIUM_FIELDS);
	TiberiumLayout = Slider_GetPos(handle);

	TiberiumWildlife = 0;
	VeinholeMonsters = 0;
	UseIonStorms = false;
	UseTransitions = false;
	UseBlueTiberium = false;

	if (Addon_Enabled(ADDON_FIRESTORM)) {
		handle = GetDlgItem(dialog, IDC_MAPGEN_LIFEFORMS);
		if (handle != NULL) {
			TiberiumWildlife = Button_GetCheck(handle) == BST_CHECKED ? 30 : 0;
		}

		handle = GetDlgItem(dialog, IDC_MAPGEN_VEINHOLES);
		if (handle != NULL) {
			VeinholeMonsters = Slider_GetPos(handle);
		}

		handle = GetDlgItem(dialog, IDC_MAPGEN_ION_STORMS);
		if (handle != NULL) {
			UseIonStorms = Button_GetCheck(handle) == BST_CHECKED;
		}

		handle = GetDlgItem(dialog, IDC_MAPGEN_TRANSITIONS);
		if (handle != NULL) {
			UseTransitions = Button_GetCheck(handle) == BST_CHECKED;
		}

		UseBlueTiberium = (double)Tiberium > 0.75;
	}

	Fixup_Settings();
}


/// <summary>
/// Fills the map generator dialog in from these settings.
/// This routine is the counterpart of Get_Settings, and is called whenever the dialog must
/// show a different set of options -- when it first appears, after a randomize, and after a
/// load. In a tournament game the controls are further restricted, or locked outright, to
/// whatever the territory permits the player to meddle with.
/// </summary>
/// <param name="dialog">The map generator dialog to fill in.</param>
void MapSeedClass::Set_Settings(HWND dialog)
{
	static char _win_name[24];

	static int _biome_names[BIOME_COUNT] = {
		TXT_BIOME_TUNDRA,
		TXT_BIOME_TAIGA,
		TXT_BIOME_TEMPERATE,
		TXT_BIOME_DESERT,
		TXT_BIOME_MUTATED
	};

	static int _time_names[TIME_OF_DAY_COUNT] = {
		TXT_TIME_MORNING,
		TXT_TIME_AFTERNOON,
		TXT_TIME_DUSK,
		TXT_TIME_NIGHT
	};

	static int _map_size_names[MAPSIZE_COUNT] = {
		TXT_MAPSIZE_SMALL,
		TXT_MAPSIZE_MEDIUM,
		TXT_MAPSIZE_LARGE,
		TXT_MAPSIZE_VERY_LARGE
	};

	WDTTerritory *wdt = NULL;
	if (Session.Type == GAME_INTERNET && Session.IsWDT) {
		wdt = WDT_Get_Territory(Session.WDTTerritory);
	}

	Fixup_Settings();

	HWND handle;
	LRESULT item;
	int i;

	handle = GetDlgItem(dialog, IDC_MAPGEN_ENVIRONMENT);
	while (SendMessageA(handle, CB_GETCOUNT, 0, 0) > 0) {
		SendMessageA(handle, CB_DELETESTRING, 0, 0);
	}
	for (i = BIOME_FIRST; i < BIOME_COUNT; i++) {
		if (i != BIOME_MUTATED || Addon_Enabled(ADDON_FIRESTORM)) {
			item = SendMessageA(handle, CB_ADDSTRING, 0, (LPARAM)Fetch_String(_biome_names[i]));
			SendMessageA(handle, CB_SETITEMDATA, item, i);
		}
	}
	item = SendMessageA(handle, CB_FINDSTRING, 0, (LPARAM)Fetch_String(_biome_names[Biome]));
	SendMessageA(handle, CB_SETCURSEL, item, 0);

	handle = GetDlgItem(dialog, IDC_MAPGEN_TIME_OF_DAY);
	while (SendMessageA(handle, CB_GETCOUNT, 0, 0) > 0) {
		SendMessageA(handle, CB_DELETESTRING, 0, 0);
	}
	for (i = TIME_OF_DAY_FIRST; i < TIME_OF_DAY_COUNT; i++) {
		item = SendMessageA(handle, CB_ADDSTRING, 0, (LPARAM)Fetch_String(_time_names[i]));
		SendMessageA(handle, CB_SETITEMDATA, item, i);
	}
	item = SendMessageA(handle, CB_FINDSTRING, 0, (LPARAM)Fetch_String(_time_names[Time]));
	SendMessageA(handle, CB_SETCURSEL, item, 0);

	handle = GetDlgItem(dialog, IDC_MAPGEN_MAP_WIDTH);
	while (SendMessageA(handle, CB_GETCOUNT, 0, 0) > 0) {
		SendMessageA(handle, CB_DELETESTRING, 0, 0);
	}
	for (i = 0; i < MAPSIZE_COUNT; i++) {
		item = SendMessageA(handle, CB_ADDSTRING, 0, (LPARAM)Fetch_String(_map_size_names[i]));
		SendMessageA(handle, CB_SETITEMDATA, item, i);
	}
	item = SendMessageA(handle, CB_FINDSTRING, 0, (LPARAM)Fetch_String(_map_size_names[Width]));
	SendMessageA(handle, CB_SETCURSEL, item, 0);

	handle = GetDlgItem(dialog, IDC_MAPGEN_MAP_HEIGHT);
	while (SendMessageA(handle, CB_GETCOUNT, 0, 0) > 0) {
		SendMessageA(handle, CB_DELETESTRING, 0, 0);
	}
	for (i = 0; i < MAPSIZE_COUNT; i++) {
		item = SendMessageA(handle, CB_ADDSTRING, 0, (LPARAM)Fetch_String(_map_size_names[i]));
		SendMessageA(handle, CB_SETITEMDATA, item, i);
	}
	item = SendMessageA(handle, CB_FINDSTRING, 0, (LPARAM)Fetch_String(_map_size_names[Height]));
	SendMessageA(handle, CB_SETCURSEL, item, 0);

	handle = GetDlgItem(dialog, IDC_MAPGEN_DIMENSION_EDIT);
	sprintf(_win_name, "%d", Seed);
	SetWindowTextA(handle, _win_name);
	if (wdt != NULL) {
		EnableWindow(handle, wdt->UserModSeed ? TRUE : FALSE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_TIBERIUM_AMOUNT);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->TiberiumAmountMin, wdt->TiberiumAmountMax, Tiberium, wdt->UserModTiberiumAmount);

		CheckDlgButton(dialog, IDC_WDT_1ON1, FALSE);
		CheckDlgButton(dialog, IDC_WDT_2ON2, TRUE);
		EnableWindow(GetDlgItem(dialog, IDC_WDT_1ON1), FALSE);
		EnableWindow(GetDlgItem(dialog, IDC_WDT_2ON2), FALSE);
	} else {
		Set_Scroll_Bar(handle, 1, 100, Tiberium, TRUE);

		handle = GetDlgItem(dialog, IDC_MAPGEN_PLAYERS);
		Set_Scroll_Bar(handle, 2, MAX_PLAYERS, NumPlayers, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_HILLS);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->HillsMin, wdt->HillsMax, Hills, wdt->UserModHills);
	} else {
		Set_Scroll_Bar(handle, 0, 100, Hills, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_WATER);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->WaterMin, wdt->WaterMax, WaterAmount, wdt->UserModWater);
	} else {
		Set_Scroll_Bar(handle, 0, 100, WaterAmount, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_CLIFFS);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->CliffsMin, wdt->CliffsMax, Cliffs, wdt->UserModCliffs);
	} else {
		Set_Scroll_Bar(handle, 0, 100, Cliffs, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_VEGETATION);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->VegetationMin, wdt->VegetationMax, Vegetation, wdt->UserModVegetation);
	} else {
		Set_Scroll_Bar(handle, 0, 100, Vegetation, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_CITIES);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->CitiesMin, wdt->CitiesMax, Cities, wdt->UserModCities);
	} else {
		Set_Scroll_Bar(handle, 0, 100, Cities, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_TIBERIUM_FIELDS);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->TiberiumFieldsMin, wdt->TiberiumFieldsMax, TiberiumLayout, wdt->UserModTiberiumFields);
	} else {
		Set_Scroll_Bar(handle, 0, 100, TiberiumLayout, TRUE);
	}

	handle = GetDlgItem(dialog, IDC_MAPGEN_ACCESSIBILITY);
	if (wdt != NULL) {
		Set_Scroll_Bar(handle, wdt->AccessibilityMin, wdt->AccessibilityMax, Accessibility, wdt->UserModAccessability);

		EnableWindow(GetDlgItem(dialog, IDC_MAPGEN_ENVIRONMENT), wdt->UserModBiome ? TRUE : FALSE);
		EnableWindow(GetDlgItem(dialog, IDC_MAPGEN_TIME_OF_DAY), wdt->UserModTime ? TRUE : FALSE);
		EnableWindow(GetDlgItem(dialog, IDC_MAPGEN_MAP_WIDTH), wdt->UserModWidth ? TRUE : FALSE);
		EnableWindow(GetDlgItem(dialog, IDC_MAPGEN_MAP_HEIGHT), wdt->UserModHeight ? TRUE : FALSE);

		Set_Checkbox(GetDlgItem(dialog, IDC_MAPGEN_LIFEFORMS), TiberiumWildlife > 0, wdt->UserModTiberiumCreatures);

		Set_Scroll_Bar(GetDlgItem(dialog, IDC_MAPGEN_VEINHOLES), 0, 5, VeinholeMonsters, wdt->UserModVeinholeMonsters);

		Set_Checkbox(GetDlgItem(dialog, IDC_MAPGEN_TRANSITIONS), UseTransitions, wdt->UserModTimeTransitions);

		Set_Checkbox(GetDlgItem(dialog, IDC_MAPGEN_ION_STORMS), UseIonStorms, TRUE);

		if (!wdt->UserModBiome && !wdt->UserModTime && !wdt->UserModCliffs && !wdt->UserModAccessability &&
			!wdt->UserModHills && !wdt->UserModTiberiumAmount && !wdt->UserModTiberiumFields && !wdt->UserModWater &&
			!wdt->UserModVegetation && !wdt->UserModCities && !wdt->UserModWidth && !wdt->UserModHeight &&
			!wdt->UserModVeinholeMonsters) {
			EnableWindow(GetDlgItem(dialog, IDC_MAPGEN_SURPRISE), FALSE);
		}

	} else {

		Set_Scroll_Bar(handle, 0, 100, Accessibility, TRUE);

		Set_Checkbox(GetDlgItem(dialog, IDC_MAPGEN_LIFEFORMS), TiberiumWildlife > 0, TRUE);

		Set_Scroll_Bar(GetDlgItem(dialog, IDC_MAPGEN_VEINHOLES), 0, 5, VeinholeMonsters, TRUE);

		Set_Checkbox(GetDlgItem(dialog, IDC_MAPGEN_TRANSITIONS), UseTransitions, TRUE);

		Set_Checkbox(GetDlgItem(dialog, IDC_MAPGEN_ION_STORMS), UseIonStorms, TRUE);
	}

	InvalidateRect(dialog, 0, 0);
}


/// <summary>
/// Sets up one of the map generation sliders.
/// This routine is used by Set_Settings to point a slider at the span of values its setting
/// is permitted to take. A setting with nothing left to choose between is shown disabled
/// rather than hidden, so the dialog keeps its shape.
/// </summary>
/// <param name="handle">The slider control to set up.</param>
/// <param name="min">The lowest value the slider may be dragged to.</param>
/// <param name="max">The highest value the slider may be dragged to.</param>
/// <param name="position">Where the thumb should sit.</param>
/// <param name="enable">Should the player be allowed to move this slider?</param>
void MapSeedClass::Set_Scroll_Bar(HWND handle, unsigned int min, unsigned int max, int position, bool enable)
{
	if (max <= min) {
		EnableWindow(handle, FALSE);
		Slider_SetRange(handle, 0, 100);
		Slider_SetPos(handle, position);
	} else {
		EnableWindow(handle, enable);
		Slider_SetRange(handle, min, max);
		Slider_SetPos(handle, position);
	}
}


/// <summary>
/// Sets up one of the map generation checkboxes.
/// This routine is the companion of Set_Scroll_Bar, and is used by Set_Settings to show a
/// setting the dialog offers as a simple yes or no. A setting the player is not allowed to
/// touch is shown disabled rather than hidden, so the dialog keeps its shape.
/// </summary>
/// <param name="handle">The checkbox control to set up.</param>
/// <param name="state">Should the box be shown checked?</param>
/// <param name="enable">Should the player be allowed to change this setting?</param>
void MapSeedClass::Set_Checkbox(HWND handle, bool state, bool enable)
{
	Button_SetCheck(handle, state != 0);
	Button_Enable(handle, enable);
}


/// <summary>
/// Rolls a fresh set of map generation settings.
/// This routine is what the dialog's randomize button calls, handing the player a whole new
/// map to consider for the price of one click. A tournament territory may forbid the player
/// to touch some of the settings; those are left alone, and the rest are rolled within
/// whatever range the territory allows.
/// </summary>
void MapSeedClass::Randomize(void)
{
	WDTTerritory *wdt = NULL;

	if (Session.Type == GAME_INTERNET && Session.IsWDT) {

		wdt = WDT_Get_Territory(Session.WDTTerritory);
		if (wdt == NULL || wdt->UserModBiome) {
			Biome = Sim_Random_Pick<unsigned int>(BIOME_FIRST, BIOME_COUNT - 1);
		}

	} else {
		Biome = Sim_Random_Pick<unsigned int>(BIOME_FIRST, BIOME_COUNT - 1);
	}

	if (!Addon_Enabled(ADDON_FIRESTORM) && Biome == BIOME_MUTATED) {
		Biome = BIOME_TEMPERATE;
	}

	if (wdt == NULL || wdt->UserModTime) {
		Time = Sim_Random_Pick<unsigned int>(TIME_OF_DAY_FIRST, TIME_OF_DAY_COUNT - 1);
	}

	if (wdt != NULL) {
		Hills = Sim_Random_Pick(wdt->HillsMin, wdt->HillsMax);
		WaterAmount = Sim_Random_Pick(wdt->WaterMin, wdt->WaterMax);
		Tiberium = Sim_Random_Pick(wdt->TiberiumAmountMin, wdt->TiberiumAmountMax);
		TiberiumLayout = Sim_Random_Pick(wdt->TiberiumFieldsMin, wdt->TiberiumFieldsMax);
		Vegetation = Sim_Random_Pick(wdt->VegetationMin, wdt->VegetationMax);
		Cities = Sim_Random_Pick(wdt->CitiesMin, wdt->CitiesMax);
		Accessibility = Sim_Random_Pick(wdt->AccessibilityMin, wdt->AccessibilityMax);
		Cliffs = Sim_Random_Pick(wdt->CliffsMin, wdt->CliffsMax);
	} else {
		Hills = Sim_Random_Pick(0U, 100U);
		WaterAmount = Sim_Random_Pick(0U, 100U);
		Tiberium = Sim_Random_Pick(0U, 100U);
		TiberiumLayout = Sim_Random_Pick(0U, 100U);
		Vegetation = Sim_Random_Pick(0U, 100U);
		Cities = Sim_Random_Pick(0U, 100U);
		Accessibility = Sim_Random_Pick(0U, 100U);
		Cliffs = Sim_Random_Pick(0U, 100U);
	}

	if (wdt == NULL || wdt->UserModWidth) {
		Width = Sim_Random_Pick(0U, 3U);
	}

	if (wdt == NULL || wdt->UserModHeight) {
		Height = Sim_Random_Pick(0U, 3U);
	}

	if (wdt == NULL || wdt->UserModVeinholeMonsters) {
		VeinholeMonsters = Sim_Random_Pick(0U, 5U);
	}

	if (wdt == NULL || wdt->UserModTiberiumCreatures) {
		TiberiumWildlife = Sim_Random_Pick(0U, 100U) > 0.5 ? Sim_Random_Pick(0U, 100U) : 0U;
	}

	UseIonStorms = (Sim_Random_Pick(0U, 1000U) < 500 && Addon_Enabled(ADDON_FIRESTORM)) ? true : false;

	if (wdt == NULL || wdt->UserModBlueTiberium) {
		UseBlueTiberium = Sim_Random_Pick(0U, 1000U) >= 500;
	}

	if (wdt == NULL || wdt->UserModTimeTransitions) {
		UseTransitions = (Sim_Random_Pick(0U, 1000U) < 500U && Addon_Enabled(ADDON_FIRESTORM)) ? true : false;
	}

	if (wdt == NULL || wdt->UserModSeed) {
		Seed = Sim_Random_Pick(0U, 65535U);
	}
	Fixup_Settings();
}


/// <summary>
/// Imposes the tournament territory's map settings.
/// A Worldwide Domination Tournament game does not let the players design the battlefield --
/// the territory being fought over dictates what it looks like. This routine replaces the
/// generator settings with that territory's own, and does nothing at all in any other game.
/// </summary>
void MapSeedClass::Fixup_WDT_Settings(void)
{
	if (Session.Type == GAME_INTERNET && Session.IsWDT) {
		WDTTerritory *wdt = WDT_Get_Territory(Session.WDTTerritory);
		if (wdt != NULL) {

			UseTransitions = wdt->TimeTransitions;

			if (wdt->Seed == 0) {
				Seed = Sim_Random_Pick(0U, 65535U);
			} else {
				Seed = wdt->Seed;
			}

			switch (wdt->Width) {
				case 0:
					Width = 0;
					break;
				case 1:
					Width = 1;
					break;
				case 2:
					Width = 2;
					break;
				case 3:
					Width = 3;
					break;
				default:
					Width = 1;
					break;

			}

			switch (wdt->Height) {
				case 0:
					Height = 0;
					break;
				case 1:
					Height = 1;
					break;
				case 2:
					Height = 2;
					break;
				case 3:
					Height = 3;
					break;
				default:
					Height = 1;
					break;
			}

			NumPlayers = 4;

			switch (wdt->Biome) {
				case 1:
					Biome = BIOME_DESERT;
					break;
				case 3:
					Biome = BIOME_TEMPERATE;
					break;
				case 2:
					Biome = BIOME_TAIGA;
					break;
				case 0:
					Biome = BIOME_TUNDRA;
					break;
				case 4:
					Biome = BIOME_MUTATED;
					break;
				default:
					Biome = BIOME_TEMPERATE;
					break;
			}

			switch (wdt->Time) {
				case 2:
					Time = TIME_OF_DAY_MORNING;
					break;
				case 0:
					Time = TIME_OF_DAY_AFTERNOON;
					break;
				case 1:
					Time = TIME_OF_DAY_DUSK;
					break;
				case 3:
					Time = TIME_OF_DAY_NIGHT;
					break;
				default:
					Time = TIME_OF_DAY_AFTERNOON;
					break;
			}

			Cliffs = wdt->CliffsDefault;
			Accessibility = wdt->AccessibilityDefault;
			Hills = wdt->HillsDefault;
			WaterAmount = wdt->WaterDefault;
			Tiberium = wdt->TiberiumAmountDefault;
			TiberiumLayout = wdt->TiberiumFieldsDefault;
			Vegetation = wdt->VegetationDefault;
			Cities = wdt->CitiesDefault;
			TiberiumWildlife = wdt->TiberiumCreatures ? 30 : 0;
			VeinholeMonsters = wdt->Veinholes;
			UseIonStorms = false;
			UseBlueTiberium = wdt->BlueTiberium ? true : false;
		}
	}
}


/// <summary>
/// Forces every map generation setting within its legal range.
/// This routine is called whenever the settings may have been tampered with -- after the
/// dialog is read, after a randomize, and after a load -- so that the generator itself never
/// has to defend against a nonsensical value. A tournament territory gets to narrow the
/// limits further, and the mutated biome is only allowed when the Firestorm addon is here.
/// </summary>
void MapSeedClass::Fixup_Settings(void)
{
	if (Session.Type == GAME_INTERNET && Session.IsWDT) {
		WDTTerritory *wdt = WDT_Get_Territory(Session.WDTTerritory);
		if (wdt != NULL) {
			Clamp_Setting(Cliffs, wdt->CliffsMin, wdt->CliffsMax, true);
			Clamp_Setting(Accessibility, wdt->AccessibilityMin, wdt->AccessibilityMax, true);
			Clamp_Setting(Hills, wdt->HillsMin, wdt->HillsMax, true);
			Clamp_Setting(WaterAmount, wdt->WaterMin, wdt->WaterMax, true);
			Clamp_Setting(Tiberium, wdt->TiberiumAmountMin, wdt->TiberiumAmountMax, true);
			Clamp_Setting(TiberiumLayout, wdt->TiberiumFieldsMin, wdt->TiberiumFieldsMax, true);
			Clamp_Setting(Vegetation, wdt->VegetationMin, wdt->VegetationMax, true);
			Clamp_Setting(Cities, wdt->CitiesMin, wdt->CitiesMax, true);
		}
	}

	if (Biome >= BIOME_COUNT) {
		Biome = BIOME_COUNT - 1;
	} else if (Biome < BIOME_FIRST) {
		Biome = BIOME_FIRST;
	}

	if (Biome == BIOME_MUTATED && !Addon_Enabled(ADDON_FIRESTORM)) {
		Biome = BIOME_TEMPERATE;
	}

	if (Time >= TIME_OF_DAY_COUNT) {
		Time = TIME_OF_DAY_COUNT - 1;
	} else if (Time < TIME_OF_DAY_FIRST) {
		Time = TIME_OF_DAY_FIRST;
	}

	if (Hills > 100) {
		Hills = 100;
	} else if (Hills < 0) {
		Hills = 0;
	}

	if (WaterAmount > 100) {
		WaterAmount = 100;
	} else if (WaterAmount < 0) {
		WaterAmount = 0;
	}

	if (NumPlayers > MAX_PLAYERS) {
		NumPlayers = MAX_PLAYERS;
	} else if (NumPlayers < 2) {
		NumPlayers = 2;
	}

	if (Tiberium > 100) {
		Tiberium = 100;
	} else if (Tiberium < 1) {
		Tiberium = 1;
	}

	if (TiberiumLayout > 100) {
		TiberiumLayout = 100;
	} else if (TiberiumLayout < 0) {
		TiberiumLayout = 0;
	}

	if (Vegetation > 100) {
		Vegetation = 100;
	} else if (Vegetation < 0) {
		Vegetation = 0;
	}

	if (Cities > 100) {
		Cities = 100;
	} else if (Cities < 0) {
		Cities = 0;
	}

	if (Width >= 4) {
		Width = 4 - 1;
	} else if (Width < 0) {
		Width = 0;
	}

	if (Height >= 4) {
		Height = 4 - 1;
	} else if (Height < 0) {
		Height = 0;
	}

	if (Accessibility > 100) {
		Accessibility = 100;
	} else if (Accessibility < 0) {
		Accessibility = 0;
	}

	if (Cliffs > 100) {
		Cliffs = 100;
	} else if (Cliffs < 0) {
		Cliffs = 0;
	}

	if (Seed > 65535) {
		Seed = 65535;
	} else if (Seed < 0) {
		Seed = 0;
	}

	if (VeinholeMonsters < 0) {
		VeinholeMonsters = 0;
	} else if (VeinholeMonsters > 5) {
		VeinholeMonsters = 5;
	}

	if (TiberiumWildlife < 0) {
		TiberiumWildlife = 0;
	} else if (TiberiumWildlife > 100) {
		TiberiumWildlife = 100;
	}
}


/// <summary>
/// Forces a setting back within its legal range.
/// This routine is used to hold a single map generation setting to the limits imposed upon
/// it. A setting that is already legal is left exactly as it is.
/// </summary>
/// <param name="setting">The setting to police. It is only written when found out of range.</param>
/// <param name="min">The lowest legal value.</param>
/// <param name="max">The highest legal value.</param>
/// <param name="use_midpoint">Should an illegal value land in the middle of the range rather
/// than on the boundary it broke?</param>
void MapSeedClass::Clamp_Setting(int & setting, int min, int max, bool use_midpoint) const
{
	int value = setting;

	if (value > max) {
		if (use_midpoint) {
			value = (max + min) >> 1;
		} else {
			value = max;
		}
		setting = value;
	} else if (value < min) {
		if (use_midpoint) {
			value = (max + min) >> 1;
		} else {
			value = min;
		}
		setting = value;
	}
}


/// <summary>
/// Saves the current map generator settings.
/// Name a file to write it directly. Pass NULL instead and the standard save dialog asks the
/// player where to put it and what to call it.
/// </summary>
/// <param name="name">Name of the settings file, or NULL to ask the player.</param>
/// <returns>bool; Were the settings saved?</returns>
bool MapSeedClass::Save(const char * name)
{
	if (name) {
		return(Save_File(name, MapDescription));
	}

	MapDescription[0] = 0;
	if (BASECLASS::Save(MapDescription)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Is this the generator's own map rather than settings a player saved? The map travels to
/// the other machines with the match, so it is kept where the game's own files are.
/// </summary>
static bool Is_Shared_Map_File(char const * file_name)
{
	return(stricmp(file_name, RANDOM_MAP_FILE_NAME) == 0);
}


/// <summary>
/// Writes the map generator settings to a file.
/// This routine records everything the random map dialog offers, so that loading the file
/// back and generating again lays down the very same terrain.
/// </summary>
/// <param name="file_name">Name of the settings file to write.</param>
/// <param name="descr">Description to keep with the settings. This is the text the load
/// dialog lists the map under.</param>
/// <returns>bool; Were the settings written?</returns>
bool MapSeedClass::Save_File(const char * file_name, const char * descr)
{
	if (file_name != NULL) {
		DebugString("Saving random map: %s - %s\n", file_name, descr);
		CCFileClass shared(file_name);
		RawFileClass owned(Saved_Game_Name(file_name).c_str());
		FileClass & file = Is_Shared_Map_File(file_name) ? (FileClass &)shared : (FileClass &)owned;
		INIClass ini;
		ini.Put_String("RandomMap", "Description", descr);
		ini.Put_Int("RandomMap", "Width", Width, 0);
		ini.Put_Int("RandomMap", "Height", Height, 0);
		ini.Put_Int("RandomMap", "NumPlayers", NumPlayers, 0);
		ini.Put_Int("RandomMap", "Seed", Seed, 0);
		ini.Put_Int("RandomMap", "Biome", Biome, BIOME_FIRST);
		ini.Put_Int("RandomMap", "Time", Time, TIME_OF_DAY_FIRST);
		ini.Put_Int("RandomMap", "RegionSize", Cliffs, 0);
		ini.Put_Int("RandomMap", "Ruggedness", Hills, 0);
		ini.Put_Int("RandomMap", "Accessibility", Accessibility, 0);
		ini.Put_Int("RandomMap", "WaterAmount", WaterAmount, 0);
		ini.Put_Int("RandomMap", "Tiberium", Tiberium, 0);
		ini.Put_Int("RandomMap", "TiberiumLayout", TiberiumLayout, 0);
		ini.Put_Int("RandomMap", "Vegetation", Vegetation, 0);
		ini.Put_Int("RandomMap", "UrbanPresence", Cities, 0);
		ini.Put_Int("RandomMap", "VeinholeMonsters", VeinholeMonsters, 0);
		ini.Put_Int("RandomMap", "TiberiumWildlife", TiberiumWildlife, 0);
		ini.Put_Bool("RandomMap", "UseIonStorms", UseIonStorms);
		ini.Put_Bool("RandomMap", "UseBlueTiberium", UseBlueTiberium);
		ini.Put_Bool("RandomMap", "UseTransitions", UseTransitions);
		ini.Save(file);
		return(true);
	}
	return(false);
}


/// <summary>
/// Loads a saved set of map generator settings.
/// Name a file to read it directly. Pass NULL instead and the standard load dialog asks the
/// player which of the saved maps to bring back.
/// </summary>
/// <param name="name">Name of the settings file, or NULL to ask the player.</param>
/// <returns>bool; Were the settings loaded?</returns>
bool MapSeedClass::Load(const char * name)
{
	if (name) {
		return(Load_File(name));
	}

	if (BASECLASS::Load()) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Reads the map generator settings from a file.
/// This routine restores a set of options saved earlier, so that a map the player liked can
/// be generated all over again. Any setting the file leaves out keeps the value it has.
/// </summary>
/// <param name="file_name">Name of the settings file to read.</param>
/// <returns>bool; Were the settings read?</returns>
bool MapSeedClass::Load_File(const char * file_name)
{
	if (file_name != NULL) {
		DebugString("Loading random map: %s\n", file_name);
		CCFileClass shared(file_name);
		RawFileClass owned(Saved_Game_Name(file_name).c_str());
		FileClass & file = Is_Shared_Map_File(file_name) ? (FileClass &)shared : (FileClass &)owned;
		INIClass ini;

		if (ini.Load(file)) {
			memset(MapDescription, 0, sizeof(MapDescription));
			ini.Get_String("RandomMap", "Description", Fetch_String(TXT_RANDOM_MAP_DESCRIPTION), MapDescription, sizeof(MapDescription));
			Width = ini.Get_Int("RandomMap", "Width", Width);
			Height = ini.Get_Int("RandomMap", "Height", Height);
			NumPlayers = ini.Get_Int("RandomMap", "NumPlayers", NumPlayers);
			Seed = ini.Get_Int("RandomMap", "Seed", Seed);
			Biome = ini.Get_Int("RandomMap", "Biome", Biome);
			Time = ini.Get_Int("RandomMap", "Time", Time);
			Cliffs = ini.Get_Int("RandomMap", "RegionSize", Cliffs);
			Hills = ini.Get_Int("RandomMap", "Ruggedness", Hills);
			Accessibility = ini.Get_Int("RandomMap", "Accessibility", Accessibility);
			WaterAmount = ini.Get_Int("RandomMap", "WaterAmount", WaterAmount);
			Tiberium = ini.Get_Int("RandomMap", "Tiberium", Tiberium);
			TiberiumLayout = ini.Get_Int("RandomMap", "TiberiumLayout", TiberiumLayout);
			Vegetation = ini.Get_Int("RandomMap", "Vegetation", Vegetation);
			Cities = ini.Get_Int("RandomMap", "UrbanPresence", Cities);
			VeinholeMonsters = ini.Get_Int("RandomMap", "VeinholeMonsters", VeinholeMonsters);
			TiberiumWildlife = ini.Get_Int("RandomMap", "TiberiumWildlife", TiberiumWildlife);
			UseIonStorms = ini.Get_Bool("RandomMap", "UseIonStorms", UseIonStorms);
			UseBlueTiberium = ini.Get_Bool("RandomMap", "UseBlueTiberium", UseBlueTiberium);
			UseTransitions = ini.Get_Bool("RandomMap", "UseTransitions", UseTransitions);
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Discards a saved set of map generator settings.
/// The standard delete dialog asks the player which of the saved maps to throw away.
/// </summary>
/// <returns>bool; Were the settings deleted?</returns>
bool MapSeedClass::Delete(void)
{
	return(BASECLASS::Delete());
}


/// <summary>
/// Removes a saved settings file from the disk.
/// This routine is what the delete dialog calls once the player has settled on which of the
/// saved maps is to go.
/// </summary>
/// <param name="file_name">Name of the settings file to remove.</param>
/// <returns>bool; Was the file removed?</returns>
bool MapSeedClass::Delete_File(const char * file_name)
{
	return(BASECLASS::Delete_File(file_name));
}


/// <summary>
/// Fills in a list entry for a saved random map.
/// This routine is called once per candidate file as the load and save dialogs build their
/// lists. The description stored inside the map becomes the text the player picks from; a
/// map that carries no description is listed as unusable. The generator's own scratch file
/// is passed over.
/// </summary>
/// <param name="entry">The list entry to fill in.</param>
/// <param name="ff">The file the directory search turned up.</param>
/// <returns>bool; Was the entry filled in from a readable random map file?</returns>
bool MapSeedClass::Read_File(FileEntryClass * entry, WIN32_FIND_DATAA * ff)
{
	char buffer[128];

	if (entry != NULL && ff != NULL) {
		if (stricmp(ff->cFileName, RANDOM_MAP_FILE_NAME)) {
			RawFileClass file(Saved_Game_Name(ff->cFileName).c_str());
			INIClass ini;
			if (ini.Load(file)) {
				if (ini.Get_String("RandomMap", "Description", 0, buffer, sizeof(buffer)) > 0 )
				{
					strncpy(entry->Descr, buffer, sizeof(buffer));
					entry->Valid = true;
				} else {
					entry->Descr[0] = 0;
					entry->Valid = false;
				}
				entry->Scenario = 0;
				entry->House = HOUSE_FIRST;
				strncpy(entry->Filename, ff->cFileName, sizeof(entry->Filename));
				if (!strlen(entry->Filename)) {
					strncpy(entry->Filename, ff->cAlternateFileName, sizeof(entry->Filename));
				}
				entry->DateTime.dwHighDateTime = ff->ftLastWriteTime.dwHighDateTime;
				entry->DateTime.dwLowDateTime = ff->ftLastWriteTime.dwLowDateTime;
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Fetches a random fraction from the map generator.
/// This routine is the uniform source that every other random pick in the generator is
/// built upon. It draws from the generator's own stream rather than the game's, so the
/// terrain a given map seed produces never varies.
/// </summary>
/// <returns>Returns with a fraction from zero up to, but never reaching, one.</returns>
double Random_Fraction(void)
{
	return RMG_RANDOM_DOUBLE(1);
}


/// <summary>
/// Fetches a random number within the specified range.
/// This is the workhorse random pick of the map generator. Use this routine wherever a
/// count, an index, or an offset must be chosen at random. Both bounds are legal picks.
/// </summary>
/// <param name="minval">The lowest number that may be picked.</param>
/// <param name="maxval">The highest number that may be picked.</param>
/// <returns>Returns with the number picked, somewhere from minval to maxval inclusive.</returns>
unsigned int Pick_Random_UInt(unsigned int minval, unsigned int maxval)
{
	unsigned int value = 0;
	while (true) {
		value = ((Random_Fraction() * (maxval - minval + 1)) + (double)minval);
		if (value <= maxval) {
			break;
		}
	}
	return(value);
}


/// <summary>
/// Constructs a normal distribution sampler.
/// This routine builds a bell curve source on top of whatever uniform random generator the
/// caller supplies. The map generator uses it wherever a value should cluster about a mean
/// rather than land anywhere in a range with equal chance.
/// </summary>
/// <param name="function">The uniform random source to draw upon. It must return a fraction
/// from zero up to one.</param>
NormalDistribution::NormalDistribution(double (*function)(void))
{
	HasSpare = false;
	Spare = 0;
	Rand = function;
}


/// <summary>
/// Draws a value from the standard normal distribution.
/// Values come out of the polar method two at a time, so every other call is served from the one
/// held over from the call before it and consumes nothing from the random source.
/// </summary>
/// <returns>Returns with a sample of mean zero and unit standard deviation.</returns>
double NormalDistribution::operator ()(void)
{
	if (HasSpare) {
		HasSpare = false;
		/*
		 * The parentheses are deliberately left off here; adding them swaps the
		 * order of the generated instructions.
		 */
		return Spare;
	}

	double u = 0.0;
	double v = 0.0;
	double s = 0.0;

	do {
		u = 2.0 * Rand() - 1.0;
		v = 2.0 * Rand() - 1.0;
		s = v * v + u * u;
	} while (s >= 1.0 || s == 0.0);

	double l = log(s);
	s = std::sqrt((-l - l) / s);
	Spare = s * v;
	HasSpare = true;
	return(s * u);
}


/// <summary>
/// Draws a normally distributed value.
/// The map generator reaches for this routine wherever it wants a quantity that clusters about a
/// preferred value rather than being spread evenly across a range.
/// </summary>
/// <param name="mean">The center of the distribution.</param>
/// <param name="standard_dev">The spread of the distribution.</param>
/// <returns>Returns with the sample drawn.</returns>
double Sample_Normal(double mean, double standard_dev)
{
	return(standard_dev * RMGGaussian() + mean);
}


/// <summary>
/// Draws a normally distributed value that keeps within bounds.
/// Samples that stray outside the bounds are thrown away and drawn again. A distribution that
/// sits outside the bounds altogether is recentered on them first, so that the draw does have
/// something to land on.
/// </summary>
/// <param name="mean">The center of the distribution to draw from.</param>
/// <param name="scale">The spread of the distribution to draw from.</param>
/// <param name="lower_bound">The lowest value that may be returned.</param>
/// <param name="upper_bound">The highest value that may be returned.</param>
/// <returns>Returns with the sample drawn, which always lies within the bounds.</returns>
double Sample_Truncated_Normal(double mean, double scale, double lower_bound, double upper_bound)
{
	double value;

	if (mean - scale > upper_bound || mean + scale < lower_bound) {
		scale = (upper_bound - lower_bound) * 0.5;
		mean = scale + lower_bound;
	}
	do {
		do {
			//value = scale * RMGGaussian() + mean;
			value = Sample_Normal(mean, scale);
		}
		while (value < lower_bound);
	}
	while (value > upper_bound);
	return(value);
}


/// <summary>
/// Generates a complete random map.
/// This is the top level of the random map generator. Working from the seed data the player
/// settled on, it lays down the water, carves the terrain into regions, finds somewhere for each
/// player to start, and then dresses the result with veinholes, tiberium, towns, hills and
/// vegetation, reporting its progress as it goes.
/// </summary>
/// <param name="full_init">Should the scenario be rebuilt from scratch and the preview redrawn
/// between phases?</param>
/// <param name="dialog">The map generator dialog to repaint as the preview is refreshed.</param>
void MapGeneratorClass::Generate_Random_Map(bool full_init, HWND dialog)
{
	if (RMGCallback != NULL) RMGCallback();

	RMGRandom = Random2Class(SeedData.Seed);
	RMGGaussian = NormalDistribution(Random_Fraction);

	if (RMGCallback != NULL) RMGCallback();

	if (!Scen->IsReadingScenario) {
		Progress.Initialize(100, 1, true);
		Progress.Set_Graphic_Data("PROGBAR2.SHP");
		Progress.Display_Progress();
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Init random map\n");

	Init_Map(full_init);

	if (RMGCallback != NULL) RMGCallback();

	ScenarioInit++;

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Seeding water\n");

	if (SeedData.WaterAmount != 0) {
		Seed_Water();
	}

	Update_Progress(55);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	if (SeedData.Biome == BIOME_TUNDRA) {
		DebugString("RMG: Smooting ice\n");
		Smooth_Ice();
	}

	Update_Progress(60);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Init regions\n");

	Init_Regions();

	Update_Progress(65);

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Making regions\n");

	if (SeedData.Biome != BIOME_TUNDRA) {
		MapRegionClass::Make_Ice_Regions();
	}
	MapRegionClass::Make_Regions();
	Map.Build_All_Cliffs(0, -1);

	Update_Progress(70);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Recalculating cell attributes\n");
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Recalc_Attributes();
		cptr = Map.Iterate();
	}

	if (RMGCallback != NULL) RMGCallback();

	Update_Progress(75);

	DebugString("RMG: Creating starting points\n");

	bool ok = false;
	while (!ok) {
		ok = Generate_Starting_Points();
		if (ok) {
			ok = Init_Start_Points();
		}
	}

	DebugString("RMG: Adding lights\n");
	Generate_Lights();

	DebugString("RMG: Adding veinholes\n");
	if (Scen->Theater == THEATER_TEMPERATE) {
		Generate_Veinholes();
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Creating tiberium\n");
	Create_Tiberium();

	if (RMGCallback != NULL) RMGCallback();

	if (TiberiumLayoutCells != NULL) {
		delete TiberiumLayoutCells;
		TiberiumLayoutCells = NULL;
	}

	DebugString("RMG: Recalculating cell attributes\n");
	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Recalc_Attributes();
		cptr = Map.Iterate();
	}

	Update_Progress(80);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Generating urban areas\n");

	if (SeedData.Biome == BIOME_TEMPERATE || SeedData.Biome == BIOME_DESERT || SeedData.Biome == BIOME_MUTATED) {
		Generate_Urban_Areas();
	} else {
		Generate_Rural_Areas();
	}

	Update_Progress(85);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Recalculating cell attributes\n");
	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Recalc_Attributes();
		cptr = Map.Iterate();
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Creating hills\n");
	Generate_Hills();

	Update_Progress(90);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Creating LATs, rocks etc\n");

	if (SeedData.Biome == BIOME_DESERT || SeedData.Biome == BIOME_TEMPERATE || SeedData.Biome == BIOME_MUTATED) {
		Seed_Vegetation();
		Generate_Vegetation();
	} else {
		Seed_Arctic_Vegetation();
		Generate_Arctic_Vegetation();
	}

	if (RMGCallback != NULL) RMGCallback();

	Update_Progress(95);

	if (RMGCallback != NULL) RMGCallback();

	if (full_init) {
		RandomMapGen.MapPreview->Create_Preview();
		SendMessage(dialog, WM_PAINT, 0, 0);
	}

	ScenarioInit--;

	if (RMGCallback != NULL) RMGCallback();

	DebugString("RMG: Recalculating cell attributes\n");
	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Recalc_Attributes();
		cptr = Map.Iterate();
	}

	if (RMGCallback != NULL) RMGCallback();

	VeinholeMonsterClass::Init_Vein_Growth_System(true);
	TiberiumClass::Init_Tiberium_Growth_System();
	TiberiumClass::Init_Tiberium_Spread_System();

	DebugString("RMG: Cleanup\n");

	SeededWaterAmount = 0;
	WorkingRegionID = 0;

	if (RMGCellData != NULL) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	for (int i = MapRegionClass::MapRegions.Count() - 1; i >= 0; i--) {
		MapRegionClass * region = MapRegionClass::MapRegions[i];
		if (region != NULL) {
			delete region;
		}
	}

	MapRegionClass::TotalCount = 0;
	Map.Overpass();
	DebugString("RMG: Compute Radar Image\n");
	Map.Set_Local_Dimensions(Map.LocalRect);
	Map.Compute_Radar_Image();

	Update_Progress(100);

	DebugString("RMG: Done\n");

	Progress.End_Dialog();
}


/// <summary>
/// Releases the working state generation leaves behind.
/// The per-cell bookkeeping and the region objects are thrown away, and the finished map is
/// handed back to the game proper with its overpass, its dimensions and its radar image all
/// brought up to date.
/// </summary>
void MapGeneratorClass::Cleanup(void)
{
	SeededWaterAmount = 0;
	WorkingRegionID = 0;

	if (RMGCellData != NULL) {
		delete [] RMGCellData;
		RMGCellData = NULL;
	}

	for (int i = MapRegionClass::MapRegions.Count() - 1; i >= 0; i--) {
		MapRegionClass * region = MapRegionClass::MapRegions[i];
		if (region != NULL) {
			delete region;
		}
	}

	MapRegionClass::TotalCount = 0;
	Map.Overpass();
	DebugString("RMG: Compute Radar Image\n");
	Map.Set_Local_Dimensions(Map.LocalRect);
	Map.Compute_Radar_Image();
}


/// <summary>
/// Prepares a blank map for the generator to work on.
/// The biome, size and time of day the player chose are written out as a scenario the engine can
/// swallow, and the map comes back sized, cleared to its fill terrain and lit to suit the hour.
/// A full initialization goes further and tears down the previous scenario altogether -- its
/// objects, houses and theater data -- which is what the generator dialog does between previews.
/// </summary>
/// <param name="full_init">Should the whole scenario and theater be rebuilt from scratch?</param>
void MapGeneratorClass::Init_Map(bool full_init)
{
	int biome = SeedData.Biome;

	char const *_theaters[BIOME_COUNT] = {
		"SNOW",
		"SNOW",
		"TEMPERATE",
		"TEMPERATE",
		"TEMPERATE",
	};

	char const *_fills[BIOME_COUNT] = {
		"Clear",
		"Clear",
		"Clear",
		"Clear",
		"Clear"
	};

	double _tod_values[TIME_OF_DAY_COUNT] = {
		0.75,
		1.0,
		0.75,
		0.5
	};

	double tod = _tod_values[SeedData.Time];

	TheaterType _biome_to_theater[BIOME_COUNT] = {
		THEATER_SNOW,
		THEATER_SNOW,
		THEATER_TEMPERATE,
		THEATER_TEMPERATE,
		THEATER_TEMPERATE,
	};

	double _tod_scales[2] = {
		1.0,
		0.75
	};

	double scale = _tod_scales[_biome_to_theater[biome]];

	/*
	 * Interpolate the local map dimensions between the minimum and maximum size
	 * tables (indexed by player count) by the requested Width/Height fraction.
	 */
	static const int _width_min[] = {50, 65, 75, 85, 100, 120, 135};
	static const int _width_max[] = {100, 115, 128, 140, 160, 170, 175};
	static const int _height_min[] = {50, 65, 75, 85, 100, 120, 135};
	static const int _height_max[] = {100, 115, 128, 140, 160, 170, 175};

	int size_index = SeedData.NumPlayers - 2;
	float width_frac = SeedData.Width * (1.0f / 3.0f);
	float height_frac = SeedData.Height * (1.0f / 3.0f);
	LocalWidth = (int)std::lerp((double)_width_min[size_index], (double)_width_max[size_index], (double)width_frac);
	LocalHeight = (int)std::lerp((double)_height_min[size_index], (double)_height_max[size_index], (double)height_frac);

	CCINIClass ini;

	Rect size(0, 0, LocalWidth + 4, LocalHeight + 12);
	Rect lsize(2, 5, LocalWidth, LocalHeight);

	ini.Clear();

	CellHeight = 4;

	ini.Put_String("Map", "Theater", _theaters[biome]);
	ini.Put_Rect("Map", "Size", size);
	ini.Put_Rect("Map", "LocalSize", lsize);
	ini.Put_Int("Map", "Level", CellHeight, 0);
	ini.Put_String("Map", "Fill", _fills[biome]);
	ini.Put_String(HouseTypes[HOUSE_GOOD]->IniName, "TechLevel", "0");
	ini.Put_String("Basic", "Player", HouseTypes[HOUSE_GOOD]->IniName);
	ini.Put_Float("Lighting", "Ambient", tod * scale);
	ini.Put_Float("Lighting", "RedTint", 1.0);
	ini.Put_Float("Lighting", "GreenTint", 1.0);
	ini.Put_Float("Lighting", "BlueTint", 1.0);
	ini.Put_Float("Lighting", "Ground", 0.0);
	ini.Put_Float("Lighting", "Level", 0.01);

	if (!Scen->IsReadingScenario) {
		Progress.Set_Progress_Percent(0, 1);
	}

	if (RMGCallback != NULL) RMGCallback();

	TiberiumClass::Deinit_Tiberium_Spread_System();
	TiberiumClass::Deinit_Tiberium_Growth_System();
	if (!full_init) {
		ScenarioInit++;
		Scen->Set_Scenario_Name("");
		if (Debug_Map) {
			Clear_Scenario();
		}
		Read_Scenario_INI(ini, true);
		Fill_In_Data();
		Cell c;
		c.Y = Map.PlayRect.Width / 2 + Map.PlayRect.Height / 2;
		c.X = c.Y + 1;
		Scen->Set_Waypoint(WAYPT_REINF, c);
		Scen->Set_Waypoint(Scen->Home, Scen->Get_Waypoint_Cell(WAYPT_REINF));
		Map[Scen->Get_Waypoint_Cell(Scen->Home)].IsWaypoint = true;
		Map.Set_Tactical_Position(Scen->Get_Waypoint_Coord(Scen->Home));
	} else {

		Scen->Reset();
		if (MapPreview == NULL) {
			MapPreview = new MapPreviewClass;
		}

		Update_Progress(2);

		bool changed = true;
		TheaterType last = Scen->Theater;
		TheaterType theater = _biome_to_theater[SeedData.Biome];

		if (MapSeeder != NULL) {
			last = _biome_to_theater[MapSeeder->Biome];
			if (MapSeeder->Width == SeedData.Width && MapSeeder->Height == SeedData.Height && last == theater
					&& MapSeeder->NumPlayers == SeedData.NumPlayers) {
				changed = false;
			}
		}

		if (RMGCallback != NULL) RMGCallback();

		/*
		 * On a theater/seed change strip the isometric tile types from the
		 * AbstractTypes list, tear down all existing objects, then re-add the
		 * (possibly reloaded) isometric tile types.
		 */
		if (MapSeeder == NULL || theater != last || changed) {
			for (int index = AbstractTypes.Count() - 1; index >= 0; index--) {
				if (AbstractTypes[index]->What_Am_I() == RTTI_ISOTILETYPE) {
					AbstractTypes.Delete_Index(index);
				}
			}

			if (RMGCallback != NULL) RMGCallback();

			Delete_All_Objects();

			if (RMGCallback != NULL) RMGCallback();

			for (int tile_index = 0; tile_index < IsometricTileTypes.Count(); tile_index++) {
				AbstractTypes.Add(IsometricTileTypes[tile_index]);
			}
		}

		Update_Progress(5);

		if (RMGCallback != NULL) RMGCallback();

		if (changed) {
			Map.Init_Clear();
			Map.Set_Map_Dimensions(size, true, 0, true);
			Map.Set_Local_Dimensions(lsize);
			if (TacticalMap != NULL) {
				delete TacticalMap;
			}
			TacticalMap = new Tactical;
			TacticalMap->Reset_Dirty_Rectangles();
		}

		Update_Progress(10);

		while (Units.Count() > 0) {
			delete Units[Units.Count() - 1];
		}

		while (Infantry.Count() > 0) {
			delete Infantry[Infantry.Count() - 1];
		}

		while (Buildings.Count() > 0) {
			delete Buildings[Buildings.Count() - 1];
		}

		while (Terrains.Count() > 0) {
			delete Terrains[Terrains.Count() - 1];
		}

		VeinholeMonsterClass::Reset();

		if (RMGCallback != NULL) RMGCallback();

		Map.Reset_Iterator();
		CellClass *cptr = Map.Iterate();
		while (cptr != NULL) {
			cptr->Ramp = 0;
			cptr->Height = 4;
			cptr->ITType = ISOTILE_CLEAR;
			cptr->SubTile = 0;
			cptr->Overlay = OVERLAY_NONE;
			cptr->OverlayData = 0;
			cptr = Map.Iterate();
		}

		if (RMGCallback != NULL) RMGCallback();

		Update_Progress(15);

		Map.Compute_Zone_Connections();
		Map.Zone_Reset();
		Map.Reset_All_Subzones();

		Update_Progress(20);

		if (RMGCallback != NULL) RMGCallback();

		Scen->Clear_All_Waypoints();
		Cell c;
		c.Y = Map.PlayRect.Width / 2 + Map.PlayRect.Height / 2;
		c.X = c.Y + 1;
		Scen->Set_Waypoint(WAYPT_REINF, c);
		Scen->Set_Waypoint(Scen->Home, Scen->Get_Waypoint_Cell(WAYPT_REINF));
		Map[Scen->Get_Waypoint_Cell(Scen->Home)].IsWaypoint = true;
		Map.Set_Tactical_Position(Scen->Get_Waypoint_Coord(Scen->Home));

		Update_Progress(25);

		if (RMGCallback != NULL) RMGCallback();

		ScenarioInit++;

		if (theater != Scen->Theater) {
			Init_Theater(theater);
			Scen->Theater = theater;
		}

		Update_Progress(30);

		if (RMGCallback != NULL) RMGCallback();

		if (MapSeeder == NULL || theater != last || changed) {
			Rule->Initialize(*RuleINI);
			Scen->Read_Global_INI(*RuleINI);
		}

		Update_Progress(35);

		if (RMGCallback != NULL) RMGCallback();

		if (MapSeeder == NULL || theater != last || changed) {

			HouseClass *hptr;
			if (Houses.Count() == 0) {
				hptr = new HouseClass(HouseTypes[HOUSE_GOOD]);
				hptr->Read_INI(ini);
				hptr = new HouseClass(HouseTypes[HOUSE_BAD]);
				hptr->Read_INI(ini);
				hptr = new HouseClass(HouseTypes[HOUSE_NEUTRAL]);
				hptr->Read_INI(ini);
				hptr = new HouseClass(HouseTypes[HOUSE_MUTANT]);
				hptr->Read_INI(ini);
			}

			if (RMGCallback != NULL) RMGCallback();

			TerrainTypeClass::Init(Scen->Theater);
			if (Scen->Theater != LastTheater) {
				IsometricTileTypeClass::Read_Control_File(Scen->Theater, true);
			} else {
				IsometricTileTypeClass::Clear_Use_Counts();
			}

			OverlayTypeClass::Init(Scen->Theater);

			if (RMGCallback != NULL) RMGCallback();
			if (RMGCallback != NULL) RMGCallback();
			if (RMGCallback != NULL) RMGCallback();

			BuildingTypeClass::Init(Scen->Theater);

			if (RMGCallback != NULL) RMGCallback();

			AnimTypeClass::Init(Scen->Theater);

			if (RMGCallback != NULL) RMGCallback();

			SmudgeTypeClass::Init(Scen->Theater);

			if (RMGCallback != NULL) RMGCallback();

			IsometricTileTypeClass::Load_Tiles(0, 1);
		}
		LastTheater = Scen->Theater;
	}

	ScenarioInit--;
	Update_Progress(40);

	if (RMGCallback != NULL) RMGCallback();

	delete RMGCellData;

	RMGCellData = new MapRegionClass::CellData[MapCellStride * MapCellStride];

	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		Cell c = cptr->CellID;
		MapRegionClass::Get_Cell_Data(c).CellID = c;
		cptr->Height = CellHeight;
		cptr = Map.Iterate();
	}

	Update_Progress(45);

	if (RMGCallback != NULL) RMGCallback();

	MapRegionClass::Delete_All_Regions();

	SeededWaterAmount = 0;
	WorkingRegionID = 0;
	PlaceWaterfall = (float)Random_Fraction() < 0.25f;
	Map.Clear_Radar();
	Map.Set_Local_Dimensions(lsize);
	Map.Init_Radar();
	Map.Compute_Radar_Image();

	Scen->AmbientLight = tod * 100.0;
	Scen->LevelLight = 1;

	if (SeedData.UseTransitions) {
		CCINIClass ini;
		switch (SeedData.Time) {
		case TIME_OF_DAY_MORNING: {
			CCFileClass file("MORNING.INI");
			ini.Load(file, false);
		} break;
		case TIME_OF_DAY_AFTERNOON: {
			CCFileClass file("DAY.INI");
			ini.Load(file, false);
		} break;
		case TIME_OF_DAY_DUSK: {
			CCFileClass file("DUSK.INI");
			ini.Load(file, false);
		} break;
		case TIME_OF_DAY_NIGHT: {
			CCFileClass file("NIGHT.INI");
			ini.Load(file, false);
		} break;
		}
		Scen->Read_Local_INI(ini);
		TriggerTypeClass::Read_All(ini);
		TagTypeClass::Read_All(ini);
	}

	if (SeedData.UseIonStorms) {
		CCFileClass file("ION.INI");
		CCINIClass ini;
		ini.Load(file, false);
		Rule->General(ini);
		char const * const LIGHTING = "Lighting";
		Scen->IonAmbientLight= (int) (100.0 * ini.Get_Float(LIGHTING, "IonAmbient", Scen->AmbientLight / 100.0));
		Scen->IonRedTint     = (int) (100.0 * ini.Get_Float(LIGHTING, "IonRed",  Scen->RedTint / 100.0));
		Scen->IonGreenTint   = (int) (100.0 * ini.Get_Float(LIGHTING, "IonGreen",Scen->GreenTint / 100.0));
		Scen->IonBlueTint    = (int) (100.0 * ini.Get_Float(LIGHTING, "IonBlue", Scen->BlueTint / 100.0));
		Scen->IonGroundLight = (int) (NORMAL_LIGHT * ini.Get_Float(LIGHTING, "IonGround", Scen->GroundLight / NORMAL_LIGHT));
		Scen->IonLevelLight  = (int) (NORMAL_LIGHT * ini.Get_Float(LIGHTING, "IonLevel", Scen->LevelLight / NORMAL_LIGHT));
		TriggerTypeClass::Read_All(ini);
		TagTypeClass::Read_All(ini);
	}

	Update_Progress(50);
}


/// <summary>
/// Reports how far generation has come.
/// The word goes to the loading screen when a scenario is being read, and to the map generator
/// dialog's own progress bar otherwise.
/// </summary>
/// <param name="percent_progress">How far along generation is, from 0 to 100.</param>
void MapGeneratorClass::Update_Progress(int percent_progress)
{
	if (Scen->IsReadingScenario) {
		Session.Update_Progress(percent_progress + 99);
	} else {
		Progress.Set_Progress_Percent(0, percent_progress);
	}
}


/// <summary>
/// Puts the map's water onto the terrain.
/// This routine is the water phase of generation. A river is attempted first on any map wet
/// enough to carry one, then a lake, each of them retried a few times before the generator
/// gives up and settles for a drier map than was asked for.
/// </summary>
void MapGeneratorClass::Seed_Water(void)
{
	WorkingRegionID++;

	bool river_seeded;
	bool lake_seeded;
	int i;
	int j;

	if (SeedData.Biome != BIOME_DESERT && SeedData.WaterAmount > 20) {
		i = 0;
		river_seeded = false;
		while (!river_seeded && i < 10) {
			if (SeedData.Biome <= BIOME_TUNDRA) {
				river_seeded = Seed_Arctic_River(Cell(0, 0), 0.0);
			} else {
				river_seeded = Seed_River(Cell(0, 0), 0.0, 0);
			}
			i++;
		}
		if (river_seeded) {
			WorkingRegionID++;
		}
	}

	j = 0;
	lake_seeded = false;
	while (!lake_seeded && j < 10) {
		if (SeedData.Biome <= BIOME_TUNDRA) {
			lake_seeded = Seed_Arctic_Lake(Cell(0, 0));
		} else {
			lake_seeded = Seed_Lake(Cell(0, 0));
		}
		j++;
	}
	if (lake_seeded) {
		WorkingRegionID++;
	}
}


/// <summary>
/// Wipes out a region and hands its cells to another.
/// This routine is used when the generator abandons something it has already laid down. The
/// cells are flattened back to clear terrain at a single height, are barred from expanding
/// again, and become the property of the region named by the caller.
/// </summary>
/// <param name="id">The region whose cells are to be wiped out.</param>
/// <param name="new_id">The region that is to inherit the wiped cells.</param>
/// <param name="cell_height">The height to flatten the cells to, or -1 to use the
/// generator's own working height.</param>
void MapGeneratorClass::Clear_Region_Cells(int id, int new_id, int cell_height)
{
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID());
		if (data.RegionID == id) {
			data.RegionID = new_id;
			data.CanExpand = false;
			cptr->ITType = ISOTILE_CLEAR;
			cptr->SubTile = 0;
			cptr->Height = cell_height == -1 ? CellHeight : cell_height;
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Seeds a lake on a temperate map.
/// The water spreads out from a seed cell until the map's water budget is spent, and is then
/// handed to the water region builder -- on a mutated map it may be soured into swamp along the
/// way. A lake that comes out too small to be worth keeping is rolled back and the ground
/// restored.
/// </summary>
/// <param name="cell">The cell to grow the lake from, or Cell(0,0) to let the routine choose
/// one.</param>
/// <returns>bool; Was a lake of usable size seeded?</returns>
bool MapGeneratorClass::Seed_Lake(const Cell & cell)
{
	bool success = true;

	float ratios[BIOME_COUNT] = {
		0.40f,
		0.35f,
		0.35f,
		0.10f,
		0.30f,
	};

	int water_value = SeedData.WaterAmount;
	if (water_value != 0) {
		water_value = (int)(ratios[SeedData.Biome] * (double)LocalWidth * (double)LocalHeight * (double)water_value * 0.02 + 100.0);
	}

	int spread_limit = water_value - SeededWaterAmount;
	if (spread_limit <= 75) {
		return(false);
	}

	CellNode *nodes = new CellNode[std::max((2 * spread_limit) + 2, 100)];
	PriorityQueueClass<CellNode> *queue = new PriorityQueueClass<CellNode>(std::max((2 * spread_limit) + 2, 100));

	Map.Reset_Iterator();
	CellClass *iter = Map.Iterate();
	while (iter != NULL) {
		Set_Cell_Data_Spread(iter->Fetch_CellID(), 0);
		iter = Map.Iterate();
	}

	int grid_size = MapCellStride * MapCellStride;

	if (cell == Cell(0, 0)) {
		int i;
		for (i = 0; i < grid_size; i++) {
			MapRegionClass::Get_Cell_Data(i).Interior = false;
		}

		Clear_Region_Border(0, 2, -2);

		for (i = 0; i < grid_size; i++) {
			MapRegionClass::CellData &data = MapRegionClass::Get_Cell_Data(i);
			if (data.RegionID == 0 || data.RegionID == WorkingRegionID) {
				MapRegionClass::Get_Cell_Data(i).Interior = true;
			} else if (data.RegionID == -2) {
				data.RegionID = 0;
			}
		}
	} else {
		for (int i = 0; i < grid_size; i++) {
			MapRegionClass::Get_Cell_Data(i).Interior = true;
		}
	}

	int tries = 0;
	Cell seed_cell;
	if (cell == Cell(0, 0)) {
		while (true) {
			unsigned int x = Pick_Random_UInt(0, Map.PlayRect.Width - 1);
			unsigned int y = Pick_Random_UInt(0, Map.PlayRect.Height - 1);

			seed_cell = Cell(1, Map.PlayRect.Width) + Cell(x, -x) + Cell(y, y);

			if (++tries >= 200) {
				return(false);
			}

			if (MapRegionClass::CellData::Get_Region(seed_cell) == 0 && Map[seed_cell].Is_Tile_Clear() && MapRegionClass::Get_Cell_Data(seed_cell).Interior) {
				if (tries < 200) {
					break;
				}
				return(false);
			}
		}
	} else {
		seed_cell = cell;
	}

	int seeded_count = 0;

	int max_spread = 76;
	if (spread_limit >= 76) {
		max_spread = spread_limit;
	}
	double scale = (double)(spread_limit / 6);
	double mean = (double)(spread_limit / 3);
	int spread_count = (int)Sample_Truncated_Normal(mean, scale, 75.0, (double)max_spread);

	queue->Clear();

	int marker_index = 1;
	nodes[0].Element = seed_cell;
	nodes[0].Score = 0.0f;
	Set_Cell_Data_Spread(seed_cell, WorkingRegionID);
	queue->Insert(nodes[0]);

	DynamicVectorClass<Cell> cells;
	cells.Set_Growth_Step(2000);

	CellNode *node = queue->Extract_Min();
	if (spread_count > 0) {
		do {
			if (node == NULL) {
				break;
			}
			if (!success) {
				break;
			}

			Set_Cell_Data_Region(node->Element, WorkingRegionID);

			CellClass *cellptr = &Map[node->Element];
			int tile_index = Pick_Random_UInt(0, 5);
			cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::WaterSet + tile_index);

			int subtile = Pick_Random_UInt(0, 3);
			cellptr->SubTile = subtile;

			cells.Add(cellptr->CellID);

			CellNode *newnode = &nodes[marker_index];
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
				Cell newcell = Adjacent_Cell(node->Element, dir);
				if (My_In_Radar(newcell)) {
					MapRegionClass::CellData &data = MapRegionClass::Get_Cell_Data(newcell);
					if (data.RegionID || data.SpreadID == WorkingRegionID || !Map[newcell].Is_Tile_Clear() || !data.Interior) {
						if (data.RegionID && data.RegionID != WorkingRegionID) {
							success = false;
						}
					} else {
						newnode->Element = newcell;
						newnode->Score = Get_Spread_Score(seed_cell, newcell, seeded_count);
						Set_Cell_Data_Spread(newcell, WorkingRegionID);
						marker_index++;
						queue->Insert(*newnode++);
					}
				}
			}

			seeded_count++;
			node = queue->Extract_Min();
		} while (seeded_count < spread_count);
	}

	CellNode *remaining = queue->Extract_Min();
	while (remaining != NULL) {
		if (!success) {
			break;
		}

		CellClass *cellptr = &Map[remaining->Element];
		MapRegionClass::CellData &data = MapRegionClass::Get_Cell_Data(remaining->Element);
		if (!data.RegionID && cellptr->Is_Tile_Clear() && data.Interior) {
			int tile_index = Pick_Random_UInt(0, 5);
			cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::WaterSet + tile_index);

			int subtile = Pick_Random_UInt(0, 3);
			cellptr->SubTile = subtile;

			Set_Cell_Data_Region(remaining->Element, WorkingRegionID);
		} else {
			success = false;
		}

		remaining = queue->Extract_Min();
		seeded_count++;
	}

	if (success) {
		if (seeded_count <= 75 || seeded_count <= spread_count / 4) {
			success = false;
		} else {
			success = true;
			if (cell == Cell(0, 0)) {
				success = Map.Build_All_Shores(WorkingRegionID);
			}
			if (success && cell == Cell(0, 0)) {
				success = Grow_Region(WorkingRegionID, 1, Rect(0, 0, MAP_CELL_W, MAP_CELL_H), 0, 0);
				if (success) {
					if (SeedData.Biome == BIOME_MUTATED) {
						int swamp_count = Pick_Random_UInt(0, 2);

						for (int i = 0; i < swamp_count; i++) {
							Generate_Swamp(cells, seeded_count, nodes, queue);
						}
					}
				}
			}
		}
	}

	delete queue;
	delete[] nodes;

	if (success) {
		SeededWaterAmount += seeded_count;
		cells.Clear();
		return(true);
	}

	Clear_Region_Cells(WorkingRegionID, 0, -1);
	return(false);
}


/// <summary>
/// Turns part of a water body into swamp.
/// This routine is used on a mutated map to spoil an otherwise respectable lake. The water is
/// soured out from a seed cell, the transitions along the new shore are tidied, and a little
/// foliage is dropped on top to finish the job.
/// </summary>
/// <param name="cells">The water cells the swamp may claim.</param>
/// <param name="last">The size of the water body, which decides how far the swamp may
/// spread.</param>
/// <param name="nodes">Scratch storage for the spread.</param>
/// <param name="queue">The queue that orders the spread.</param>
void MapGeneratorClass::Generate_Swamp(DynamicVectorClass<Cell> &cells, int last, CellNode *nodes, PriorityQueueClass<CellNode> *queue)
{
	Cell swamp_origin = CELL_NONE;
	int tries = 0;

	DebugString("Generating swamp\n");

	while (swamp_origin == CELL_NONE) {
		Cell c = cells[Pick_Random_UInt(0, cells.Count() - 1)];
		if (!Map[c].Is_Tile_Swamp()) {
			swamp_origin = c;
		}

		tries++;
		if (tries >= 200) {
			break;
		}
	}

	if (swamp_origin == CELL_NONE) {
		return;
	}

	DynamicVectorClass<Cell> swamp_cells;
	swamp_cells.Set_Growth_Step(500);

	int max_spread = std::min(last / 2, 200);
	int min_spread = std::min(last / 8, 50);

	int spread_count = Pick_Random_UInt(min_spread, max_spread);
	queue->Clear();

	int marker_index = 1;
	nodes[0].Element = swamp_origin;
	nodes[0].Score = 0;
	queue->Insert(nodes[0]);

	CellNode * node = queue->Extract_Min();
	int spread_step = 0;
	while (spread_step < spread_count && node != NULL) {
		CellClass * cptr = &Map[node->Element];
		cptr->ITType = IsometricTileTypeClass::SwampTile;
		cptr->SubTile = 0;
		swamp_cells.Add(cptr->CellID);

		CellNode * newnode = &nodes[marker_index];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
			Cell newcell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(newcell)) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);
				CellClass * newcellptr = &Map[newcell];
				if (newcellptr->ITType >= IsometricTileTypeClass::WaterSet && newcellptr->ITType < IsometricTileTypeClass::WaterSet + WATER_COUNT) {
					newnode->Element = newcell;
					newnode->Score = Get_Spread_Score(swamp_origin, newcell, spread_step);
					data.SpreadID = WorkingRegionID;
					marker_index++;
					queue->Insert(*newnode++);
				}
			}
		}

		spread_step++;
		node = queue->Extract_Min();
	}

	int i;
	for (i = 0; i < swamp_cells.Count(); i++) {
		Map[swamp_cells[i]].Fixup_LAT();
	}

	DynamicVectorClass<unsigned short> swamp_tiles;

	for (i = 0; i < SWAMP_COUNT - 1; i++) {
		swamp_tiles.Add((unsigned short)(IsometricTileTypeClass::SwampTile + i + 1));
	}

	int patch_count = Pick_Random_UInt(1, 8);
	for (i = 0; i < patch_count; i++) {
		int patch_index = Pick_Random_UInt(0, swamp_tiles.Count() - 1);
		IsometricTileTypeClass * itype = IsometricTileTypes[swamp_tiles[patch_index]];
		int width = itype->Width;
		int height = itype->Height;

		Cell patch_cell = CELL_NONE;
		int patch_tries = 0;
		while (patch_cell == CELL_NONE) {
			Cell base = swamp_cells[Pick_Random_UInt(0, swamp_cells.Count() - 1)];
			patch_cell = base;
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					if (Map[base + Cell(x, y)].ITType != IsometricTileTypeClass::SwampTile) {
						patch_cell = CELL_NONE;
					}
				}
			}

			patch_tries++;
			if (patch_tries >= 200) {
				break;
			}
		}

		if (patch_cell == CELL_NONE) {
			break;
		}

		for (int y = 0; y < height; y++) {
			char subtile = width * y;
			for (int x = 0; x < width; x++) {
				Cell c = patch_cell + Cell(x, y);
				Map[c].ITType = (IsometricTileType)swamp_tiles[patch_index];
				Map[c].SubTile = subtile + x;
			}
		}

		swamp_tiles.Delete_Index(patch_index);
	}

	int foliage_count = Pick_Random_UInt(0, 4);
	for (i = 0; i < foliage_count; i++) {
		Cell c = swamp_cells[Pick_Random_UInt(0, swamp_cells.Count() - 1)];
		if (Map[c].Cell_Terrain(false) == NULL) {
			char terrain_name[8];
			int terrain_index = Pick_Random_UInt(1, 5);
			sprintf(terrain_name, "FONA0%d", terrain_index);
			new TerrainClass(TerrainTypes[TerrainTypeClass::From_Name(terrain_name)], c);
		}
	}

	DebugString("Swamp heap size:?? -- Marker Index:%d\n", marker_index);
}


/// <summary>
/// Seeds a frozen lake on an arctic map.
/// This is the tundra counterpart of Seed_Lake. The sheet spreads out from a seed cell until the
/// map's water budget is spent and is then given its edge variants. A lake that cannot be grown
/// to a worthwhile size is abandoned and the ground put back the way it was found.
/// </summary>
/// <param name="cell">The cell to grow the lake from, or Cell(0,0) to let the routine choose
/// one.</param>
/// <returns>bool; Was a lake of usable size seeded?</returns>
bool MapGeneratorClass::Seed_Arctic_Lake(const Cell & cell)
{
	float ratios[BIOME_COUNT] = {
		0.40f,
		0.35f,
		0.35f,
		0.10f,
		0.30f,
	};

	int water_value;
	if (SeedData.WaterAmount == 0) {
		water_value = 0;
	} else {
		water_value = (int)(ratios[SeedData.Biome] * (double)LocalWidth * (double)LocalHeight * (double)SeedData.WaterAmount * 0.02 + 100.0);
	}

	int spread_limit = water_value - SeededWaterAmount;
	IsometricTileType ice1 = IsometricTileTypeClass::Ice1Set;
	if (spread_limit <= 0) {
		return(false);
	}

	CellNode *nodes = new CellNode[std::max((2 * spread_limit) + 2, 100)];
	PriorityQueueClass<CellNode> *queue = new PriorityQueueClass<CellNode>(std::max((2 * spread_limit) + 2, 100));

	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		Set_Cell_Data_Spread(cptr->Fetch_CellID(), 0);
		cptr = Map.Iterate();
	}

	Cell seed_cell;
	if (cell == Cell(0, 0)) {
		int tries = 0;
		while (true) {
			unsigned int x = Pick_Random_UInt(0, Map.PlayRect.Width - 1);
			unsigned int y = Pick_Random_UInt(0, Map.PlayRect.Height - 1);

			seed_cell = Cell(1, Map.PlayRect.Width) + Cell(x, -x) + Cell(y, y);

			tries++;
			if (tries >= 200) {
				return(false);
			}

			if (MapRegionClass::CellData::Get_Region(seed_cell) == 0 && Map[seed_cell].Is_Tile_Clear()) {
				if (tries < 200) {
					break;
				}
				return(false);
			}
		}
	} else {
		seed_cell = cell;
	}

	bool success = true;
	int seeded_count = 0;

	int max_spread = std::max(spread_limit, 76);
	double scale = (double)(spread_limit / 6);
	double mean = (double)(spread_limit / 4);

	int spread_count = (int)Sample_Truncated_Normal(mean, scale, 75.0, (double)max_spread);

	queue->Clear();

	int marker_index = 1;
	nodes[0].Element = seed_cell;
	nodes[0].Score = 0.0f;
	Set_Cell_Data_Spread(seed_cell, WorkingRegionID);
	queue->Insert(nodes[0]);

	CellNode *node = queue->Extract_Min();

	DynamicVectorClass<Cell> *ice_cells = new DynamicVectorClass<Cell>();
	ice_cells->Set_Growth_Step(spread_count + 1);

	while (seeded_count < spread_count && node != NULL) {
		Set_Cell_Data_Region(node->Element, WorkingRegionID);

		CellClass *node_cellptr = &Map[node->Element];
		node_cellptr->ITType = ice1;
		node_cellptr->SubTile = 0;
		node_cellptr->IsIceGrowthAllowed = true;
		ice_cells->Add(node_cellptr->Fetch_CellID());

		CellNode *newnode = &nodes[marker_index];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
			Cell newcell = Adjacent_Cell(node->Element, dir);

			if (My_In_Radar(newcell)) {

				if (RMGCellData != NULL) {
					MapRegionClass::CellData &data = MapRegionClass::Get_Cell_Data(newcell);
					if (!data.RegionID && data.SpreadID != WorkingRegionID) {
						CellClass *newcellptr = &Map[newcell];
						if (newcellptr->Is_Tile_Clear()) {
							newnode->Element = newcell;
							newnode->Score = Get_Spread_Score(seed_cell, newcell, seeded_count);
							Set_Cell_Data_Spread(newcell, WorkingRegionID);
							marker_index++;
							queue->Insert(*newnode++);
						}
					}
				}
			}
		}

		seeded_count++;
		node = queue->Extract_Min();
	}

	CellNode *remaining = queue->Extract_Min();
	while (remaining != NULL) {
		if (!success) {
			break;
		}

		MapRegionClass::CellData &data = MapRegionClass::Get_Cell_Data(remaining->Element);
		if (data.RegionID || !Map[remaining->Element].Is_Tile_Clear()) {
			success = false;
		} else {
			CellClass *cellptr = &Map[remaining->Element];
			cellptr->ITType = IsometricTileTypeClass::Ice1Set;
			cellptr->SubTile = 0;
			cellptr->IsIceGrowthAllowed = true;
			data.RegionID = WorkingRegionID;
			//ice_cells->Add(cellptr->Fetch_CellID());
			ice_cells->Add(cellptr->CellID);
		}

		remaining = queue->Extract_Min();
		seeded_count++;
	}

	Seed_Ice(*ice_cells, (IsometricTileType)(IsometricTileTypeClass::Ice1Set + ICE_CRACKED));
	Seed_Ice(*ice_cells, (IsometricTileType)(IsometricTileTypeClass::Ice1Set + ICE_EDGE));

	delete queue;
	delete[] nodes;
	delete ice_cells;

	if (success) {
		SeededWaterAmount += seeded_count;
		return(true);
	}

	Clear_Region_Cells(WorkingRegionID, 0, -1);
	return(false);
}


/// <summary>
/// Scatters an edge variant over a sheet of ice.
/// This routine is run over an ice sheet once it has been laid down, dabbing blotches of one of
/// the ice variants here and there so that the sheet does not read as one flat expanse. It is
/// called once per variant.
/// </summary>
/// <param name="cells">The ice cells that may be painted.</param>
/// <param name="last">The ice variant to paint with.</param>
void MapGeneratorClass::Seed_Ice(DynamicVectorClass<Cell> & cells, IsometricTileType last)
{
	double rand_scale = last == IsometricTileTypeClass::Ice1Set + ICE_CRACKED ? ICE_CRACKED - 1 : 1;

	int seed_count = Pick_Random_UInt(0, 15);

	CellNode * nodes = new CellNode[std::max(cells.Count() * 2, 64)];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(std::max(cells.Count() * 2, 64));

	for (int i = cells.Count() - 1; i >= 0; i--) {
		MapRegionClass::Get_Cell_Data(cells[i]).Iced = false;
	}

	while (seed_count > 0) {
		int spread_step = 0;
		int spread_limit = std::max(4, cells.Count() / 20);
		int spread_count = Pick_Random_UInt(3, spread_limit);
		queue->Clear();

		int seed_index = Pick_Random_UInt(0, cells.Count() - 1);
		int node_count = 1;
		Cell seed_cell = cells[seed_index];
		nodes[0].Element = seed_cell;
		nodes[0].Score = 0.0f;

		MapRegionClass::Get_Cell_Data(seed_cell).Iced = true;
		queue->Insert(nodes[0]);

		CellNode * node = queue->Extract_Min();
		while (spread_step < spread_count && node != NULL) {
			Map[node->Element].ITType = (IsometricTileType)last;

			CellNode * newnode = &nodes[node_count];
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
				Cell newcell = Adjacent_Cell(node->Element, dir);
				if (My_In_Radar(newcell)) {
					CellClass * newcellptr = &Map[newcell];
					MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);
					if (newcellptr->Is_Tile_Ice() && !data.Iced) {
						newnode->Element = newcell;
						newnode->Score = Get_Ice_Score(seed_cell, newcell, rand_scale);
						data.Iced = true;
						node_count++;
						queue->Insert(*newnode++);
					}
				}
			}

			spread_step++;
			node = queue->Extract_Min();
		}

		seed_count--;
	}

	delete[] nodes;

	delete queue;
}


/// <summary>
/// Scores a candidate cell for a spreading patch of ice.
/// This is the ice counterpart of Get_Spread_Score. Ground near the seed is claimed first, and
/// the weight given to the random term decides how ragged the edge of the patch turns out.
/// </summary>
/// <param name="cell1">The cell the patch is spreading from.</param>
/// <param name="cell2">The candidate cell being scored.</param>
/// <param name="random_range">How much randomness to allow into the score.</param>
/// <returns>Returns with the candidate's priority. Lower is claimed first.</returns>
double MapGeneratorClass::Get_Ice_Score(const Cell & cell1, const Cell & cell2, double random_range) const
{
	int x = cell1.X - cell2.X;
	int y = cell1.Y - cell2.Y;
	double dist = std::sqrt(x * x + y * y);
	return(RMG_RANDOM_DOUBLE(random_range) + dist);
}


/// <summary>
/// Fetches how much water this map is meant to hold.
/// The figure follows from the water setting the player chose, the biome, and the size of the
/// playable area. The seeding routines treat it as a budget and stop once they have spent it.
/// </summary>
/// <returns>Returns with the wanted number of water cells, or zero for a map that is to have no
/// water at all.</returns>
int MapGeneratorClass::Get_Target_Water_Amount(void)
{
	float ratios[BIOME_COUNT] = {
		0.40,
		0.35,
		0.35,
		0.10,
		0.30,
	};

	if (SeedData.WaterAmount == 0) {
		return(0);
	}
	return(ratios[SeedData.Biome] * 0.02 * SeedData.WaterAmount * LocalWidth * LocalHeight + 100.0);
}


/// <summary>
/// Scores a candidate cell for a spreading water body.
/// The flood fill takes the lowest scoring candidate first, so ground near the seed is claimed
/// first while a dose of randomness keeps the resulting shore from coming out round. The score
/// also eases off as the spread wears on, which lets the shape reach further out than it
/// otherwise would.
/// </summary>
/// <param name="cell1">The cell the body is spreading from.</param>
/// <param name="cell2">The candidate cell being scored.</param>
/// <param name="spread_step">How far along the spread already is.</param>
/// <returns>Returns with the candidate's priority. Lower is claimed first.</returns>
double MapGeneratorClass::Get_Spread_Score(const Cell & cell1, const Cell & cell2, int spread_step) const
{
	static const float scale = 0.5;
	static const float _random_range = 10.0f;
	static const float _step_weight = 0.02f;

	int x = cell1.X - cell2.X;
	int y = cell1.Y - cell2.Y;
	float dist = std::sqrt(x * x + y * y);

	float random_value = _random_range * (float)Random_Fraction();
	float step_term = (spread_step * _step_weight);
	float dist_term = (dist * scale);

	return(random_value + dist_term - step_term);
}


/// <summary>
/// Draws a wandering river of water tiles across the map, perturbing the heading
/// and brush width at each step, occasionally spawning a branch or a waterfall.
/// On success the painted region is converted into proper water regions and, for
/// elevation-4 maps, an optional raised lake edge is built. This is the temperate
/// sibling of Seed_Arctic_River.
/// </summary>
/// <param name="cell">Seed cell, or Cell(0, 0) to pick a random edge seed.</param>
/// <param name="start_angle">Initial heading angle in radians (used when cell is given).</param>
/// <param name="is_branch">True when invoked recursively as a branch sub-call.</param>
/// <returns>True if enough water was painted, false otherwise.</returns>
bool MapGeneratorClass::Seed_River(const Cell & cell, double start_angle, bool is_branch)
{
	bool orig_subcall = is_branch;
	bool success = true;
	int waterfall_count = 0;

	Cell seed_cell;
	double angle;

	if (cell == Cell(0, 0)) {
		Cell corner(1, Map.PlayRect.Width);
		Cell far_corner = corner + Cell(Map.PlayRect.Width, -Map.PlayRect.Width) + Cell(Map.PlayRect.Height, Map.PlayRect.Height) + Cell(-2, 0);

		int dir4 = (int)Pick_Random_UInt(0, 3);
		int x = (int)Pick_Random_UInt(0, Map.PlayRect.Width - 1);
		int y = (int)Pick_Random_UInt(0, Map.PlayRect.Height - 1);

		Cell candidates[4];
		candidates[0] = corner + Cell(x, -x);
		candidates[1] = far_corner - Cell(y, y);
		candidates[2] = far_corner - Cell(x, -x);
		candidates[3] = corner + Cell(y, y);

		double base_angle = DEG_TO_RAD(315) - (double)dir4 * M_PI_2;

		seed_cell = candidates[dir4];
		angle = Sample_Truncated_Normal(base_angle, DEG_TO_RAD(30),
				base_angle - M_PI_4, base_angle + M_PI_4);
	} else {
		seed_cell = cell;
		angle = start_angle;
	}

	if (!My_In_Radar(seed_cell)) {
		return(false);
	}

	int seeded_count = 0;

	double width_d = (double)SeedData.WaterAmount * 0.07;
	if (1.0 > width_d) {
		width_d = 1.0;
	}
	int width_top = (int)width_d;
	double width = (double)Pick_Random_UInt(1, width_top);
	int half = (int)width / 2;
	double width_hi = (double)half + width;
	double width_lo = width - (double)half;

	int length = (int)Pick_Random_UInt(35, 125);

	double cur_angle = angle;
	double cur_x = (double)seed_cell.X + 0.5;
	double cur_y = (double)seed_cell.Y + 0.5;
	double angle_hi = angle + M_PI_2;
	double angle_lo = angle - M_PI_2;

	double stop_roll = 1.0;
	while (My_In_Radar(Cell((int)cur_x, (int)cur_y)) && success) {

		double step_y = std::cos(cur_angle);
		double step_x = std::sin(cur_angle);

		int span = (int)(width + 0.5);
		bool x_run = true;
		bool y_run = true;
		bool any_x = true;
		bool any_y = true;

		double offset_x = (double)(span - 1) * step_x * 0.5;
		double px = cur_x - offset_x;
		double offset_y = (double)(span - 1) * step_y * 0.5;
		double py = cur_y - offset_y;

		if (span > 0) {
			int i = span;
			Cell pcell;
			while (true) {
				if (My_In_Radar(Cell((int)px, (int)py))) {
					pcell = Cell((int)px, (int)py);
					if (Get_Cell_Data_Region(pcell) == 0
							&& Map[pcell].Is_Tile_Clear()
							|| Get_Cell_Data_Region(pcell) == WorkingRegionID) {

						Set_Cell_Data_Region(pcell, WorkingRegionID);
						CellClass * cellptr = &Map[Cell((int)px, (int)py)];
						int tile_index = Pick_Random_UInt(0, 5);
						cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::WaterSet + tile_index);
						int subtile = Pick_Random_UInt(0, 3);
						cellptr->SubTile = subtile;
					} else {
						success = false;
					}
				}

				if ((unsigned int)(int)(px + step_x) != (unsigned int)(int)px) {
					x_run = false;
					any_x = false;
				} else {
					x_run = any_x;
				}
				if ((unsigned int)(int)(py + step_y) != (unsigned int)(int)py) {
					any_y = false;
				}

				px += step_x;
				py += step_y;
				i--;
				if (i == 0) {
					y_run = any_y;
					break;
				}
			}
		}

		if (fabs(step_y) > fabs(step_x)) {
			y_run = false;
		} else {
			x_run = false;
		}

		if (!is_branch && (x_run || y_run) && waterfall_count < 1
				&& (double)abs((int)(cur_angle - angle)) < M_PI_4) {

			if (PlaceWaterfall) {
				if (seeded_count > length) {
					int type;
					if (x_run) {
						type = (step_y > 0.0) ? 2 : 6;
					} else {
						type = (step_x > 0.0) ? 0 : 4;
					}

					bool placed = false;
					Cell from((int)(px - step_x), (int)(py - step_y));
					Cell to((int)(cur_x - offset_x), (int)(cur_y - offset_y));
					Place_Waterfall(WorkingRegionID, to, from, type, placed, cur_x, cur_y);
					if (placed) {
						WorkingRegionID++;
						waterfall_count++;
					}
				}
			}
		}

		cur_x += step_y;
		cur_y -= step_x;

		if (Random_Fraction() < 0.01) {
			if (success) {
				if (!is_branch && waterfall_count == 0) {
					is_branch = true;
					double branch_base = cur_angle + M_PI_2;
					double branch_angle = Sample_Truncated_Normal(branch_base, DEG_TO_RAD(30),
							cur_angle + DEG_TO_RAD(30), cur_angle + DEG_TO_RAD(150));

					Cell branch_cell((int)px, (int)py);
					success = Seed_River(branch_cell, branch_angle, true);
				}
			}
		}

		if (seeded_count > 5) {
			double delta_angle = Sample_Truncated_Normal(0.0, DEG_TO_RAD(18),
					angle_lo - cur_angle, angle_hi - cur_angle);
			cur_angle += delta_angle;
		}

		if (half > 0) {
			double delta_width = Sample_Truncated_Normal(0.0, 0.5,
					width_lo - width, width_hi - width);
			width += delta_width;
		}

		stop_roll = Random_Fraction();
		seeded_count++;
		if (stop_roll < 0.005) {
			break;
		}
	}

	if (seeded_count < 40) {
		success = false;
	}

	if (stop_roll < 0.005) {
		if (My_In_Radar(Cell((int)cur_x, (int)cur_y)) && success) {
			if (!Seed_Lake(Cell((int)cur_x, (int)cur_y))) {
				success = false;
			}
		}
	}

	if (!orig_subcall && success) {
		success = Map.Build_All_Shores(WorkingRegionID);
	}

	bool raised = false;

	if (waterfall_count == 0 && success && CellHeight == 4 && !orig_subcall) {

		if (Random_Fraction() < 0.7 && SeedData.Biome != BIOME_TUNDRA) {

			Rect bounds(0, 0, MAP_CELL_W, MAP_CELL_H);
			if (Grow_Water_Region(WorkingRegionID, 0.01f, bounds, seed_cell, true)) {
				success = Grow_Region(WorkingRegionID, 6, Rect(0, 0, MAP_CELL_W, MAP_CELL_H), 0, 0);
				if (success) {
					int region_id = WorkingRegionID;
					Map.Reset_Iterator();
					CellClass * cptr = Map.Iterate();
					while (cptr != NULL) {
						Cell c = cptr->CellID;
						int rid = Get_Cell_Data_Region(c);
						if (rid != region_id) {
							cptr->Height += 4;
						}
						cptr = Map.Iterate();
					}
					raised = true;
				}
			} else {
				success = false;
			}
		}
	}

	if (!orig_subcall && success && waterfall_count == 0) {
		if (!raised) {
			success = Grow_Region(WorkingRegionID, 2, Rect(0, 0, MAP_CELL_W, MAP_CELL_H), 0, 0);
		}
	} else if (waterfall_count > 0) {
		if (success) {
			success = Grow_Region(WorkingRegionID, 2, Rect(0, 0, MAP_CELL_W, MAP_CELL_H), 1, CellHeight);
		}
	}

	if (success) {
		if (raised) {
			CellHeight += 4;
		}
		SeededWaterAmount += seeded_count;
		return(true);
	}

	Clear_Region_Cells(WorkingRegionID, 0, -1);

	if (waterfall_count > 0) {
		Clear_Region_Cells(WorkingRegionID - 1, 0, -1);
	}

	return(false);
}


/// <summary>
/// Draws a wandering "river of ice" polyline of Ice1Set tiles across the map.
/// Each step perturbs the heading and length, occasionally recurses to spawn a
/// branch, and finally scatters ice-edge variants via Seed_Ice. This is the
/// arctic sibling of Seed_River.
/// </summary>
/// <param name="cell">Seed cell, or Cell(0, 0) to pick a random edge seed.</param>
/// <param name="start_angle">Initial heading angle in radians (used when cell is given).</param>
/// <returns>True if enough ice was painted, false otherwise.</returns>
bool MapGeneratorClass::Seed_Arctic_River(const Cell & cell, double start_angle)
{
	bool success = true;

	Cell seed_cell;
	double angle;

	if (cell == Cell(0, 0)) {
		Cell corner(1, Map.PlayRect.Width);
		Cell far_corner = corner + Cell(Map.PlayRect.Width, -Map.PlayRect.Width) + Cell(Map.PlayRect.Height, Map.PlayRect.Height) + Cell(-2, 0);

		int dir4 = (int)Pick_Random_UInt(0, 3);
		int x = (int)Pick_Random_UInt(0, Map.PlayRect.Width - 1);
		int y = (int)Pick_Random_UInt(0, Map.PlayRect.Height - 1);

		Cell candidates[4];
		candidates[0] = corner + Cell(x, -x);
		candidates[1] = far_corner - Cell(y, y);
		candidates[2] = far_corner - Cell(x, -x);
		candidates[3] = corner + Cell(y, y);

		double base_angle = DEG_TO_RAD(315) - (double)dir4 * M_PI_2;

		seed_cell = candidates[dir4];
		angle = Sample_Truncated_Normal(base_angle, DEG_TO_RAD(60),
				base_angle - M_PI_2, base_angle + M_PI_2);
	} else {
		seed_cell = cell;
		angle = start_angle;
	}

	if (!My_In_Radar(seed_cell)) {
		return(false);
	}

	int seeded_count = 0;

	double length_d = (double)SeedData.WaterAmount * 0.09;
	if (4.0 > length_d) {
		length_d = 4.0;
	}
	int length_top = (int)length_d;
	double length = (double)Pick_Random_UInt(3, length_top);
	int half = (int)length / 2;

	double cur_angle = angle;
	double length_hi = length + (double)half;
	double length_lo = length - (double)half;
	double cur_x = (double)seed_cell.X + 0.5;
	double cur_y = (double)seed_cell.Y + 0.5;
	double angle_hi = angle + M_PI_2;
	double angle_lo = angle - M_PI_2;

	double stop_roll = 1.0;

	DynamicVectorClass<Cell> *ice_cells = new DynamicVectorClass<Cell>();
	ice_cells->Set_Growth_Step(5000);

	while (My_In_Radar(Cell((int)cur_x, (int)cur_y)) && success) {

		double step_y = std::cos(cur_angle);
		double step_x = std::sin(cur_angle);

		int span = (int)(length + 0.5);
		double px = cur_x - step_x * (double)(span - 1) * 0.5;
		double py = cur_y - step_y * (double)(span - 1) * 0.5;

		for (int i = span; i > 0; i--) {
			if (My_In_Radar(Cell((int)px, (int)py))) {
				if (RMGCellData != NULL) {
					MapRegionClass::Get_Cell_Data(Cell((int)px, (int)py)).RegionID = WorkingRegionID;
				}

				CellClass *cellptr = &Map[Cell((int)px, (int)py)];
				Cell id = cellptr->CellID;
				cellptr->ITType = IsometricTileTypeClass::Ice1Set;
				cellptr->SubTile = 0;
				ice_cells->Add(id);
			}

			px += step_x;
			py += step_y;
		}

		cur_x += step_y;
		cur_y -= step_x;

		if (Random_Fraction() < 0.01) {
			double branch_base = cur_angle + M_PI_2;
			double branch_angle = Sample_Truncated_Normal(branch_base, DEG_TO_RAD(30),
					cur_angle + DEG_TO_RAD(30), cur_angle + DEG_TO_RAD(150));

			Cell branch_cell((int)px, (int)py);
			success = Seed_Arctic_River(branch_cell, branch_angle);
		}

		double delta_angle = Sample_Truncated_Normal(0.0, DEG_TO_RAD(18),
				angle_lo - cur_angle, angle_hi - cur_angle);
		cur_angle += delta_angle;

		if (half > 0) {
			double delta_len = Sample_Truncated_Normal(0.0, 0.5,
					length_lo - length, length_hi - length);
			length += delta_len;
		}

		stop_roll = Random_Fraction();
		seeded_count++;
		if (stop_roll < 0.005) {
			break;
		}
	}

	if (seeded_count < 40) {
		success = false;
	}

	if (stop_roll < 0.005) {
		if (My_In_Radar(Cell((int)cur_x, (int)cur_y)) && success) {
			Seed_Arctic_Lake(Cell((int)cur_x, (int)cur_y));
		}
	}

	if (success) {
		Seed_Ice(*ice_cells, (IsometricTileType)(IsometricTileTypeClass::Ice1Set + ICE_CRACKED));
		Seed_Ice(*ice_cells, (IsometricTileType)(IsometricTileTypeClass::Ice1Set + ICE_EDGE));
	}

	delete ice_cells;

	if (success) {
		SeededWaterAmount += seeded_count;
		return(true);
	}

	Clear_Region_Cells(WorkingRegionID, 0, -1);
	return(false);
}


/// <summary>
/// Levels a region to a single height.
/// Every cell the region owns is forced to the height given, which is how the generator makes a
/// plateau or a basin out of ground that was grown at whatever height it happened to find.
/// </summary>
/// <param name="id">The region to level.</param>
/// <param name="cell_height">The height every one of its cells takes.</param>
void MapGeneratorClass::Set_Height_Of_ID(int id, char cell_height)
{
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		int rid = Get_Cell_Data_Region(cptr->Fetch_CellID());
		if (rid == id) {
			cptr->Height = cell_height;
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Raises the terrain belonging to one region.
/// This routine lifts a region bodily, so that the ground it owns keeps whatever shape it was
/// grown with while rising above its neighbors.
/// </summary>
/// <param name="id">The region to raise.</param>
/// <param name="cell_height">The amount each of its cells is to rise by.</param>
void MapGeneratorClass::Increase_Height_Of_ID(int id, char cell_height)
{
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		int rid = Get_Cell_Data_Region(cptr->Fetch_CellID());
		if (rid == id) {
			cptr->Height += cell_height;
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Places a waterfall where a river spills off a shelf.
/// This routine is used by the river seeding to carry water down a change in height. The ground
/// beyond the drop is examined for room; if there is any, it is painted with water, raised into
/// a shelf of its own, and dressed with the waterfall tiles for the heading given. When there is
/// no room the terrain is left exactly as it was found and the river must go elsewhere.
/// </summary>
/// <param name="region_id">The region the painted spillway cells are given to.</param>
/// <param name="cell1">One end of the river edge the waterfall hangs from.</param>
/// <param name="cell2">The other end of that river edge.</param>
/// <param name="direction">The cardinal heading the water falls toward -- 0 north, 2 east,
/// 4 south, 6 west.</param>
/// <param name="placed">Set to whether the waterfall went in. Mirrors the return value.</param>
/// <param name="head_x">The river head X, carried past the waterfall on success.</param>
/// <param name="head_y">The river head Y, carried past the waterfall on success.</param>
/// <returns>bool; Was the waterfall placed?</returns>
bool MapGeneratorClass::Place_Waterfall(int region_id, Cell const & cell1, Cell const & cell2, int direction, bool & placed, double & head_x, double & head_y)
{
	placed = false;

	int pre_x = 0;
	int pre_y = 0;
	int pre_w = 0;
	int pre_h = 0;

	int seed_x = 0;
	int seed_y = 0;
	int seed_w = 0;
	int seed_h = 0;

	int patch_x = 0;
	int patch_y = 0;
	int patch_w = 0;
	int patch_h = 0;

	Rect bounds_rect(0, 0, 0, 0);
	Cell center(0, 0);

	switch (direction) {
		case FACING_N: {
			int dx = cell2.X - cell1.X;
			int width = dx + 1;
			pre_x = cell1.X - 2;
			pre_y = cell1.Y - 12;
			pre_w = dx + 5;
			pre_h = 12;

			seed_x = cell1.X;
			seed_y = cell1.Y - 4;
			seed_w = width;
			seed_h = 4;

			patch_x = cell1.X;
			patch_y = cell1.Y - 12;
			patch_w = width;
			patch_h = 8;

			bounds_rect = Rect(0, seed_y, MAP_CELL_W, MAP_CELL_H - seed_y);
			center = Cell(cell1.X, seed_y);
			break;
		}
		case FACING_S: {
			int dx = cell1.X - cell2.X;
			int width = dx + 1;
			pre_x = cell2.X - 2;
			pre_y = cell2.Y + 1;
			pre_w = dx + 5;
			pre_h = 12;

			seed_x = cell2.X;
			seed_y = cell2.Y + 1;
			seed_w = width;
			seed_h = 4;

			patch_x = cell2.X;
			patch_y = cell2.Y + 5;
			patch_w = width;
			patch_h = 8;

			bounds_rect = Rect(0, 0, MAP_CELL_W, patch_y - 1);
			center = Cell(cell2.X, cell2.Y + 1);
			break;
		}
		case FACING_E: {
			int dy = cell2.Y - cell1.Y;
			int width = dy + 1;
			pre_x = cell1.X + 1;
			pre_y = cell1.Y - 2;
			pre_w = 12;
			pre_h = dy + 5;

			seed_x = cell1.X + 1;
			seed_y = cell1.Y;
			seed_w = 4;
			seed_h = width;

			patch_x = cell1.X + 5;
			patch_y = cell1.Y;
			patch_w = 8;
			patch_h = width;

			bounds_rect = Rect(0, 0, patch_x - 1, MAP_CELL_H);
			center = Cell(cell1.X + 5, cell1.Y);
			break;
		}
		case FACING_W: {
			int dy = cell1.Y - cell2.Y;
			int width = dy + 1;
			pre_x = cell2.X - 12;
			pre_y = cell2.Y - 2;
			pre_w = 12;
			pre_h = dy + 5;

			seed_x = cell2.X - 4;
			seed_y = cell2.Y;
			seed_w = 4;
			seed_h = width;

			patch_x = cell2.X - 12;
			patch_y = cell2.Y;
			patch_w = 8;
			patch_h = width;

			bounds_rect = Rect(seed_x, 0, MAP_CELL_W - seed_x, MAP_CELL_H);
			center = Cell(seed_x, cell2.Y);
			break;
		}
		default:
			break;
	}

	int yy;
	int xx;

	for (yy = pre_y; yy < pre_y + pre_h; yy++) {
		for (xx = pre_x; xx < pre_x + pre_w; xx++) {
			if (My_In_Radar(Cell(xx, yy))) {
				if (Get_Cell_Data_Region(Cell(xx, yy)) != 0) {
					return(true);
				}

				if (!Map[Cell(xx, yy)].Is_Tile_Clear()) {
					return(true);
				}
			}
		}
	}

	for (yy = seed_y; yy < seed_y + seed_h; yy++) {
		for (xx = seed_x; xx < seed_x + seed_w; xx++) {
			if (My_In_Radar(Cell(xx, yy))) {
				int tile_index = Pick_Random_UInt(0, 5);
				Map[Cell(xx, yy)].ITType = (IsometricTileType)(IsometricTileTypeClass::WaterSet + tile_index);

				int subtile = Pick_Random_UInt(0, 3);
				Map[Cell(xx, yy)].SubTile = subtile;

				Set_Cell_Data_Region(Cell(xx, yy), region_id);
			}
		}
	}

	bool success = Grow_Water_Region(WorkingRegionID, 0.003f, bounds_rect, center, false);
	if (success) {
		success = Map.Build_All_Shores(WorkingRegionID);
		if (success) {
			success = Grow_Region(WorkingRegionID, 2, bounds_rect, 0, 0);
			if (success) {
				int region = WorkingRegionID;
				Map.Reset_Iterator();
				CellClass * cptr = Map.Iterate();
				while (cptr != NULL) {
					Cell c = cptr->CellID;
					int other_region = Get_Cell_Data_Region(c);
					if (other_region == region) {
						cptr->Height += 4;
					}
					cptr = Map.Iterate();
				}

				for (yy = patch_y; yy < patch_y + patch_h; yy++) {
					for (xx = patch_x; xx < patch_x + patch_w; xx++) {
						if (My_In_Radar(Cell(xx, yy))) {
							int tile_index = Pick_Random_UInt(0, 5);
							Map[Cell(xx, yy)].ITType = (IsometricTileType)(IsometricTileTypeClass::WaterSet + tile_index);

							int subtile = Pick_Random_UInt(0, 3);
							Map[Cell(xx, yy)].SubTile = subtile;

							Set_Cell_Data_Region(Cell(xx, yy), region_id);
						}
					}
				}

				bool place_ok = true;
				IsometricTileType tile = ISOTILE_CLEAR;

				switch (direction) {
					case FACING_E: {
						Cell base(patch_x, patch_y + patch_h);
						int height = Map[base + Cell(2, 0)].Height;

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallEast];
						Map.Set_Cursor_Pos(base);
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallEast + WATERFALL_EAST_COUNT - 1];
						Map.Set_Cursor_Pos(base - Cell(0, patch_h + 2));
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Cell top = base + Cell(0, 2);
						Cell bottom = base - Cell(0, patch_h + 3);

						if (My_In_Radar(top)) {
							Map[top].ITType = ISOTILE_NONE;
							Map[top].Height += 4;
							Map[top].SubTile = 0;
							Set_Cell_Data_Region(top, WorkingRegionID);
						}

						if (My_In_Radar(bottom)) {
							Map[bottom].ITType = ISOTILE_NONE;
							Map[bottom].Height += 4;
							Map[bottom].SubTile = 0;
							Set_Cell_Data_Region(bottom, WorkingRegionID);
						}

						int seg = 0;
						int remaining = patch_h;
						while (seg < patch_h) {
							if (remaining % 2) {
								Map.Set_Cursor_Pos(base - Cell(0, seg + 1));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallEast + 1];
								seg++;
								remaining--;
							} else {
								Map.Set_Cursor_Pos(base - Cell(0, seg + 2));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallEast + 2];
								seg += 2;
								remaining -= 2;
							}
							Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);
						}
						break;
					}

					case FACING_W: {
						Cell base(seed_x - 2, patch_y + patch_h);
						int height = Map[base - Cell(2, 0)].Height;

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallWest];
						Map.Set_Cursor_Pos(base);
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallWest + WATERFALL_WEST_COUNT - 1];
						Map.Set_Cursor_Pos(base - Cell(0, patch_h + 2));
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Map[base - Cell(0, 1)].ITType = ISOTILE_NONE;
						Map[base + Cell(0, patch_w + 2)].ITType = ISOTILE_NONE;

						for (int i = 0; i < patch_h + 2; i++) {
							Map[base - Cell(0, i)].Height -= 4;
						}

						Cell top = base + Cell(1, 2);
						Cell bottom = base - Cell(-1, patch_h + 3);

						if (My_In_Radar(top)) {
							Map[top].ITType = ISOTILE_NONE;
							Map[top].Height += 4;
							Map[top].SubTile = 0;
							Set_Cell_Data_Region(top, WorkingRegionID);
						}

						if (My_In_Radar(bottom)) {
							Map[bottom].ITType = ISOTILE_NONE;
							Map[bottom].Height += 4;
							Map[bottom].SubTile = 0;
							Set_Cell_Data_Region(bottom, WorkingRegionID);
						}

						int seg = 0;
						int remaining = patch_h;
						while (seg < patch_h) {
							if (remaining % 2) {
								Map.Set_Cursor_Pos(base - Cell(-1, seg + 1));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallWest + 1];
								seg++;
								remaining--;
							} else {
								Map.Set_Cursor_Pos(base - Cell(-1, seg + 2));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallWest + 2];
								seg += 2;
								remaining -= 2;
							}
							Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);
						}
						break;
					}

					case FACING_S: {
						Cell base(patch_x - 2, patch_y);
						int height = Map[base + Cell(0, 2)].Height;

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallSouth];
						Map.Set_Cursor_Pos(base);
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallSouth + WATERFALL_SOUTH_COUNT - 1];
						Map.Set_Cursor_Pos(base + Cell(patch_w + 2, 0));
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Cell left = base - Cell(1, 0);
						Cell right = base + Cell(patch_w + 4, 0);

						if (My_In_Radar(left)) {
							Map[left].ITType = ISOTILE_NONE;
							Map[left].Height += 4;
							Map[left].SubTile = 0;
							Set_Cell_Data_Region(left, WorkingRegionID);
						}

						if (My_In_Radar(right)) {
							Map[right].ITType = ISOTILE_NONE;
							Map[right].Height += 4;
							Map[right].SubTile = 0;
							Set_Cell_Data_Region(right, WorkingRegionID);
						}

						int seg = 0;
						int remaining = patch_w;
						while (seg < patch_w) {
							if (remaining % 2) {
								Map.Set_Cursor_Pos(base + Cell(seg + 2, 0));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallSouth + 1];
								seg++;
								remaining--;
							} else {
								Map.Set_Cursor_Pos(base + Cell(seg + 2, 0));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallSouth + 2];
								seg += 2;
								remaining -= 2;
							}
							Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);
						}
						break;
					}

					case FACING_N: {
						Cell base(seed_x - 2, seed_y - 2);
						int height = Map[base - Cell(0, 2)].Height;

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallNorth];
						Map.Set_Cursor_Pos(base);
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallNorth + WATERFALL_NORTH_COUNT - 1];
						Map.Set_Cursor_Pos(base + Cell(patch_w + 2, 0));
						Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);

						Map[base + Cell(1, 0)].ITType = ISOTILE_NONE;
						Map[base + Cell(patch_w + 2, 0)].ITType = ISOTILE_NONE;

						for (int i = 0; i < patch_w + 2; i++) {
							Map[base + Cell(i + 1, 0)].Height -= 4;
						}

						Cell left = base + Cell(-1, 1);
						Cell right = base + Cell(patch_w + 4, 1);

						if (My_In_Radar(left)) {
							Map[left].ITType = ISOTILE_NONE;
							Map[left].Height += 4;
							Map[left].SubTile = 0;
							Set_Cell_Data_Region(left, WorkingRegionID);
						}

						if (My_In_Radar(right)) {
							Map[right].ITType = ISOTILE_NONE;
							Map[right].Height += 4;
							Map[right].SubTile = 0;
							Set_Cell_Data_Region(right, WorkingRegionID);
						}

						int seg = 0;
						int remaining = patch_w;
						while (seg < patch_w) {
							int column = seg + 2;
							if (remaining % 2) {
								Map.Set_Cursor_Pos(base + Cell(column, 1));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallNorth + 1];
								seg++;
								remaining--;
							} else {
								Map.Set_Cursor_Pos(base + Cell(column, 1));
								Map.PendingObject = IsometricTileTypes[IsometricTileTypeClass::WaterfallNorth + 2];
								seg += 2;
								remaining -= 2;
							}
							Map.Pick_Random_Tile_Variant(tile, ISOTILE_NONE + 1, height, WorkingRegionID, place_ok);
						}
						break;
					}

					default:
						break;
				}
			}
		}
	}

	int x_offsets[4] = { 0, 12, 0, -12 };
	int y_offsets[4] = { -12, 0, 12, 0 };

	if (success) {
		int index = direction / 2;
		head_x += (double)x_offsets[index];
		head_y += (double)y_offsets[index];
	}

	placed = success;
	return(success);
}


/// <summary>
/// Assigns a cell to a region.
/// This routine is the generator's shorthand for the region data hanging off each cell.
/// </summary>
void MapGeneratorClass::Set_Cell_Data_Region(Cell const &cell, int region_id)
{
	MapRegionClass::CellData::Set_Region(cell, region_id);
}


/// <summary>
/// Fetches the region a cell belongs to.
/// This routine is the generator's shorthand for the region data hanging off each cell.
/// </summary>
int MapGeneratorClass::Get_Cell_Data_Region(Cell const &cell)
{
	return(MapRegionClass::CellData::Get_Region(cell));
}


/// <summary>
/// Marks a cell as reached by the spread currently running.
/// This routine is the generator's shorthand for the region data hanging off each cell.
/// </summary>
void MapGeneratorClass::Set_Cell_Data_Spread(Cell const &cell, int patch_id)
{
	MapRegionClass::CellData::Set_Spread(cell, patch_id);
}


/// <summary>
/// Fetches the id of the spread that last reached a cell.
/// This routine is the generator's shorthand for the region data hanging off each cell.
/// </summary>
int MapGeneratorClass::Get_Cell_Data_Spread(Cell const &cell)
{
	return(MapRegionClass::CellData::Get_Spread(cell));
}


/// <summary>
/// Grows a region outward into the ground around it.
/// The region spreads from its own border, claiming unowned clear ground that lies within the
/// bounding rectangle, and gives up the moment it runs into ground another region already owns.
/// This routine is how the generator turns a seed into a region of usable size.
/// </summary>
/// <param name="id">The region to grow.</param>
/// <param name="rings">How far out from its border the region may spread.</param>
/// <param name="rect">The area the region is confined to.</param>
/// <param name="set_height">Should the claimed ground be flattened to the given height, taking
/// over the preceding region and any water it meets along the way?</param>
/// <param name="height">The height given to newly claimed ground.</param>
/// <returns>bool; Did the region grow without running into a neighbor?</returns>
bool MapGeneratorClass::Grow_Region(int id, int rings, Rect rect, char set_height, char height)
{
	DynamicVectorClass<Cell> * cells = Build_Region_Border_Cell_List(id);

	DynamicVectorClass<Cell> * ncells;
	for (int i = 0; i < rings; i++) {
		ncells = new DynamicVectorClass<Cell>;
		ncells->Set_Growth_Step(3 * cells->Count());

		for (int j = 0; j < cells->Count(); j++) {
			for (int dir = 0; dir < FACING_COUNT; dir++) {
				Cell ncell = Adjacent_Cell((*cells)[j], (FacingType)dir);
				if (My_In_Radar(ncell) && rect.Is_Point_Within(Point2D(ncell.X, ncell.Y))) {
					MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(ncell);
					int rid = data.RegionID;
					bool reclaim = set_height && rid == id - 1;
					CellClass * cptr = &Map[ncell];
					if ((rid == 0 || reclaim) && (cptr->Is_Tile_Clear() || (reclaim && cptr->Is_Tile_With_Water()))) {
						ncells->Add(ncell);
						data.RegionID = id;
						if (set_height && cptr->Is_Tile_Clear()) {
							Map[ncell].Height = height;
						}
					} else if (rid != id) {
						delete cells;
						delete ncells;
						return(false);
					}
				}
			}
		}

		delete cells;
		cells = ncells;
	}

	delete cells;
	return(true);
}


/// <summary>
/// Flattens a band of terrain around the edge of a region.
/// The cells along the border are stripped back to plain clear ground at the working height and
/// handed to another region. This routine is used to give a region a clean skirt before
/// anything is grown or placed against it.
/// </summary>
/// <param name="id">The region whose border is to be cleared.</param>
/// <param name="rings">How many cells deep the cleared band should reach.</param>
/// <param name="new_id">The region the cleared cells are handed to.</param>
/// <returns>bool; Always true.</returns>
bool MapGeneratorClass::Clear_Region_Border(int id, int rings, int new_id)
{
	int i;
	int j;

	DynamicVectorClass<Cell> * cells = Build_Region_Border_Cell_List(id);

	if (rings > 1) {
		Map.Reset_Iterator();
		CellClass *cptr = Map.Iterate();
		while (cptr != NULL) {
			MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID()).SpreadID = 0;
			cptr = Map.Iterate();
		}
	}

	for (i = 0; i < rings; i++) {

		for (j = cells->Count() - 1; j >= 0; j--) {
			Cell c = (*cells)[j];
			MapRegionClass::CellData::Set_Region(c, new_id);
			CellClass *cptr = &Map[c];
			cptr->ITType = ISOTILE_CLEAR;
			cptr->SubTile = 0;
			cptr->Height = CellHeight;
		}

		if (i < rings - 1) {
			DynamicVectorClass<Cell> * ncells = new DynamicVectorClass<Cell>;
			ncells->Set_Growth_Step(cells->Count());

			for (j = cells->Count() - 1; j >= 0; j--) {
				Cell c = (*cells)[j];
				for (int dir = 0; dir < FACING_COUNT; dir++) {
					Cell ncell = Adjacent_Cell(c, (FacingType)dir);
					if (My_In_Radar(ncell)) {
						MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(ncell);
						if (data.RegionID == id && data.SpreadID != i + 1) {
							ncells->Add(ncell);
							data.SpreadID = i + 1;
						}
					}
				}
			}

			delete cells;
			cells = ncells;
		}
	}

	delete cells;

	if (rings > 1) {
		Map.Reset_Iterator();
		CellClass *cptr = Map.Iterate();
		while (cptr != NULL) {
			int grid_size = MapCellStride * MapCellStride;
			MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID()).SpreadID = 0;
			cptr = Map.Iterate();
		}
	}

	return(true);
}


/// <summary>
/// Gathers the border cells of a region.
/// A cell counts as border when at least one of its neighbors belongs to somebody else. The
/// growing and clearing routines take this list as the frontier they work outward from.
/// </summary>
/// <param name="id">The region whose boundary is wanted.</param>
/// <returns>Returns with a pointer to a freshly allocated list of the region's border cells.
/// The caller owns the list and must delete it.</returns>
DynamicVectorClass<Cell> *MapGeneratorClass::Build_Region_Border_Cell_List(int id)
{
	DynamicVectorClass<Cell> *cells = new DynamicVectorClass<Cell>;
	cells->Set_Growth_Step(LocalWidth * LocalHeight);

	int grid_size = MapCellStride * MapCellStride;
	for (int i = 0; i < grid_size; i++) {
		Cell cell = RMGCellData[i].CellID;
		if (cell != Cell(0, 0) && MapRegionClass::CellData::Get_Region(cell) == id) {
			bool add = false;
			for (int facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
				Cell ncell = Adjacent_Cell(cell, (FacingType)facing);
				if (My_In_Radar(ncell)) {
					if (MapRegionClass::CellData::Get_Region(ncell) != id) {
						add = true;
						break;
					}
				}
			}

			if (add) {
				cells->Add(cell);
			}
		}
	}

	return(cells);
}


/// <summary>
/// Grows the region identified by region_id outward across the map as a
/// priority-queue flood fill, scoring candidate cells by their angle relative
/// to a reference cell. Border cells that abut a different region clear the
/// success flag.
/// </summary>
/// <param name="region_id">Region id written into the cell data of claimed cells.</param>
/// <param name="spread_scale">Scale factor controlling how far the region spreads.</param>
/// <param name="bounds">Bounding rectangle the spread is confined to.</param>
/// <param name="origin">Reference origin used when computing the spread angle.</param>
/// <param name="claim_frontier">When true, drain the remaining frontier into the region.</param>
/// <returns>True if the region grew without colliding with another region.</returns>
bool MapGeneratorClass::Grow_Water_Region(int region_id, float spread_scale, Rect const & bounds, Cell const & origin, bool claim_frontier)
{
	CellNode * nodes = new CellNode[std::max(2 * LocalHeight * LocalWidth, 100)];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(std::max(2 * LocalHeight * LocalWidth, 100));

	int marker_index = 0;
	bool result_flag = true;

	/*
	 * Seed the starting angle from which side of the map the region touches.
	 */
	double angle = 0.0;
	if (bounds.X != 0) {
		angle = 0.0;
	} else if (bounds.Width != MAP_CELL_W) {
		angle = M_PI;
	}
	if (bounds.Y != 0) {
		angle = 3 * M_PI_2;
	} else if (bounds.Height != MAP_CELL_H) {
		angle = M_PI_2;
	}

	Clear_Cell_Data_Spreads();

	/*
	 * Insert the region's border cells as the initial frontier.
	 */
	DynamicVectorClass<Cell> * seed_cells = Build_Region_Border_Cell_List(region_id);
	int seed_count = seed_cells->Count();

	int spread_count = 0;
	if (seed_count > 0) {
		CellNode * newnode = nodes;
		do {
			Cell seed = (*seed_cells)[spread_count];
			if (seed.X >= bounds.X && seed.X < bounds.X + bounds.Width &&
				seed.Y >= bounds.Y && seed.Y < bounds.Y + bounds.Height) {

				newnode->Element = seed;
				newnode->Score = Get_Angle_Score(origin, seed, angle);
				marker_index++;
				queue->Insert(*newnode++);
			}
		} while (++spread_count < seed_count);
	}

	delete seed_cells;

	/*
	 * Compute how far to spread.
	 */
	double scale = log((double)seed_count);
	if (1.0 > scale) {
		scale = 1.0;
	}

	spread_count = (int)(1.0f / ((float)(1.0 / scale) * spread_scale) * 0.5f);
	spread_count += Pick_Random_UInt(0, spread_count / 2);

	CellNode * node = queue->Extract_Min();

	int spread_step = 0;
	if (spread_count > 0) {
		while (true) {
			if (!result_flag || node == NULL) {
				break;
			}

			MapRegionClass::CellData & ndata = MapRegionClass::Get_Cell_Data(node->Element);
			if (!ndata.RegionID) {
				if (Map[node->Element].Is_Tile_Clear()) {
					ndata.CanExpand = true;
					ndata.RegionID = region_id;
				}
			}

			CellNode * newnode = &nodes[marker_index];
			for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
				Cell newcell = Adjacent_Cell(node->Element, dir);
				if (My_In_Radar(newcell)) {
					int other_region;
					if (RMGCellData != NULL) {
						MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);
						if (!data.RegionID && data.SpreadID != region_id &&
							newcell.X >= bounds.X && newcell.X < bounds.X + bounds.Width &&
							newcell.Y >= bounds.Y && newcell.Y < bounds.Y + bounds.Height &&
							Map[newcell].Is_Tile_Clear()) {

							newnode->Element = newcell;
							newnode->Score = Get_Angle_Score(origin, newcell, angle);
							Set_Cell_Data_Spread(newcell, region_id);
							marker_index++;
							queue->Insert(*newnode++);
							continue;
						}
						other_region = MapRegionClass::Get_Cell_Data(newcell).RegionID;
						if (!other_region) {
							continue;
						}
					} else {
						other_region = -1;
					}

					if (other_region != region_id) {
						result_flag = false;
					}
				}
			}

			spread_step++;
			angle += Sample_Normal(0, M_PI_4);
			node = queue->Extract_Min();
			if (spread_step >= spread_count) {
				break;
			}
		}
	}

	/*
	 * Optionally drain the remaining frontier and assign it to the region.
	 */
	if (claim_frontier) {
		CellNode * remaining = queue->Extract_Min();
		while (remaining != NULL) {
			if (!result_flag) {
				break;
			}

			int other_region;
			if (RMGCellData != NULL) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(remaining->Element);
				other_region = data.RegionID;
				if (!other_region) {
					data.RegionID = region_id;
				} else if (other_region != region_id) {
					result_flag = false;
				}
			} else {
				if (-1 != region_id) {
					result_flag = false;
				}
			}

			remaining = queue->Extract_Min();
		}
	}

	delete[] nodes;
	delete queue;

	return(result_flag);
}


/// <summary>
/// Scores how far a step strays from the wanted heading.
/// The generator uses this routine to bias a spread toward a preferred direction -- a low score
/// means the step points the right way. A little randomness is mixed in so that two equally
/// good candidates do not always resolve the same way.
/// </summary>
/// <param name="cell1">The cell being stepped from.</param>
/// <param name="cell2">The cell being stepped to.</param>
/// <param name="ref_angle">The preferred heading, in radians.</param>
/// <returns>Returns with the penalty for this step. Lower is better.</returns>
double MapGeneratorClass::Get_Angle_Score(const Cell & cell1, const Cell & cell2, double ref_angle)
{
	static const double _random_weight = 2.0;
	static const double _angle_weight = 1.5;

	int dx = cell2.X - cell1.X;
	int dy = cell2.Y - cell1.Y;

	double angle = (dx != 0) ? std::atan(-((double)dy / (double)dx)) : M_PI_2;
	if (dx < 0) {
		angle += M_PI;
	}

	double angle_diff = fabs(angle - ref_angle);
	while (angle_diff >= M_PI * 2) {
		angle_diff -= M_PI * 2;
	}
	if (angle_diff > M_PI) {
		angle_diff = M_PI * 2 - angle_diff;
	}

	return(RMG_RANDOM_DOUBLE2(_random_weight) + _angle_weight * angle_diff);
}


/// <summary>
/// Clears the spread marker from every cell, back to zero.
/// This routine wipes the markers so that the next routine to spread out from a seed starts
/// from a clean map. Note that Reset_Spreads clears to -1 rather than to zero.
/// </summary>
void MapGeneratorClass::Clear_Cell_Data_Spreads(void)
{
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		Set_Cell_Data_Spread(cptr->Fetch_CellID(), 0);
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Raises the terrain everywhere outside one region.
/// Use this routine to sink a region relative to the rest of the map; the named region keeps
/// the height it has while the world around it climbs.
/// </summary>
/// <param name="cell_height">The amount every other cell is to rise by.</param>
/// <param name="id">The region to leave alone.</param>
void MapGeneratorClass::Increase_Height_Except_ID(int cell_height, int id)
{
	Map.Reset_Iterator();
	CellClass *cptr = Map.Iterate();
	while (cptr != NULL) {
		int rid = Get_Cell_Data_Region(cptr->Fetch_CellID());
		if (rid != id) {
			cptr->Height += cell_height;
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Rebuilds the generator's regions from scratch.
/// Whatever regions an earlier pass left behind are thrown away and derived afresh from the
/// terrain as it now stands. The water regions are settled first and given room to breathe, so
/// that the land regions fill in around them.
/// </summary>
void MapGeneratorClass::Init_Regions(void)
{
	int i;

	MapRegionClass::Destroy_Cell_Regions();
	MapRegionClass::Create_Water_Regions();

	for (i = 0; i < MapRegionClass::MapRegions.Count(); i++) {
		MapRegionClass *ptr = MapRegionClass::MapRegions[i];
		if (ptr->ContainsWater) {
			ptr->Grow(4);
			ptr->Claim_Expandable_Cells();
		}
	}

	MapRegionClass::Create_Land_Regions(false);
}


/// <summary>
/// Builds the region map for the generated terrain.
/// This routine runs once the ground itself has settled. Ice regions are gathered first on any
/// biome that has ice, then the general regions, and finally the map's own height regions are
/// rebuilt so that placement and pathfinding agree with the new landscape.
/// </summary>
/// <returns>bool; Were the height regions built?</returns>
bool MapGeneratorClass::Make_Regions(void)
{
	if (SeedData.Biome != BIOME_TUNDRA) {
		MapRegionClass::Make_Ice_Regions();
	}
	MapRegionClass::Make_Regions();
	return(Map.Build_All_Cliffs(0, -1));
}


/// <summary>
/// Determines if a region holds enough of an area to count.
/// Build_Neighbor_Region_Mask uses this routine to decide which of the blocks around a cell
/// belong to a region, and so which directions a ramp may be run in. Counting stops as soon as
/// the answer is settled.
/// </summary>
/// <param name="rect">The area of cells to examine.</param>
/// <param name="region_id">The region being looked for.</param>
/// <param name="count">The number of cells the region must own for the test to pass.</param>
/// <returns>bool; Does the region own at least that many cells within the area?</returns>
bool MapGeneratorClass::Region_Cell_Count_At_Least(Rect const & rect, int region_id, int, int count)
{
	Cell cell(rect.X, rect.Y);
	int actual = 0;

	for (int y = 0; y < rect.Height; y++) {
		for (int x = 0; x < rect.Width; x++) {
			if (My_In_Radar(cell + Cell(x, y))) {
				if (MapRegionClass::CellData::Get_Region(cell + Cell(x, y)) == region_id) {
					actual++;
					if (actual >= count) {
						return(true);
					}
				}
			}
		}
	}

	return(false);
}


/// <summary>
/// Expands the high ground over the whole map.
/// This routine is the map wide driver for MapClass::Expand_High_Ground. It stops at the
/// first cell that will not expand, so that the caller can throw the map away and start again.
/// </summary>
/// <returns>bool; Did every cell expand successfully?</returns>
bool MapGeneratorClass::Expand_All_High_Ground(void)
{
	bool success = true;
	Map.Set_Cursor_Shape(NULL);
	Map.Reset_Iterator();
	CellClass *cptr = cptr = Map.Iterate();
	while (cptr != NULL) {
		if (!success) {
			break;
		}
		success = Map.Expand_High_Ground(cptr, -1);
		cptr = Map.Iterate();
	}
	return(success);
}


/// <summary>
/// Floods clear, in-radar cells outward from every player's start waypoint, marking each
/// visited cell Inviolate. A player passes only if the flood reaches exactly 400 cells; if any
/// player's flood runs dry first, the Inviolate flags are cleared and the routine fails so the
/// caller regenerates the map.
/// </summary>
/// <returns>True if every player's start point spreads 400 cells; false (after clearing the
/// Inviolate flags) if any player falls short.</returns>
bool MapGeneratorClass::Init_Start_Points(void)
{
	int player_index = 0;

	if (SeedData.NumPlayers > 0) {
		while (true) {
			Cell seed = Scen->Get_Waypoint_Cell((WAYPOINT)player_index);

			CellNode * nodes = new CellNode[800];
			PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(800);

			Clear_Cell_Data_Spreads();
			queue->Clear();

			int patch_id = player_index + 1;
			int node_count = 1;
			int spread_count = 0;
			nodes[0].Element = seed;
			nodes[0].Score = 0.0f;
			MapRegionClass::Get_Cell_Data(seed).SpreadID = patch_id;
			queue->Insert(nodes[0]);

			CellNode * node = queue->Extract_Min();
			if (node == NULL) {
				break;
			}

			while (true) {
				if (spread_count >= 400) {
					break;
				}

				MapRegionClass::Get_Cell_Data(node->Element).Inviolate = true;

				CellNode * newnode = &nodes[node_count];
				for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
					Cell ncell = Adjacent_Cell(node->Element, dir);
					if (My_In_Radar(ncell) && MapRegionClass::Get_Cell_Data(ncell).SpreadID == 0) {
						if (Map[ncell].Is_Tile_Clear()) {
							newnode->Element = ncell;
							int dx = ncell.X - seed.X;
							int dy = ncell.Y - seed.Y;
							newnode->Score = std::sqrt((double)(dx * dx + dy * dy));
							MapRegionClass::Get_Cell_Data(ncell).SpreadID = patch_id;
							node_count++;
							queue->Insert(*newnode++);
						}
					}
				}

				spread_count++;
				node = queue->Extract_Min();
				if (node == NULL) {
					break;
				}
			}

			if (spread_count != 400) {
				break;
			}

			delete[] nodes;
			delete queue;

			player_index = patch_id;
			if (patch_id >= SeedData.NumPlayers) {
				return(true);
			}
		}

		MapRegionClass::CellData::Clear_Inviolates();
		return(false);
	}
	return(true);
}


/// <summary>
/// Distributes tiberium across the map.
/// This routine seeds a field at every point of the map's tiberium layout, then hands out a
/// compensating field to each player's start point -- the further a player sits from the
/// tiberium already placed, the more of it is put on their doorstep, so that nobody begins the
/// game starved.
/// </summary>
void MapGeneratorClass::Create_Tiberium(void)
{
	int tiberium_count = SeedData.NumPlayers * (30 * SeedData.Tiberium + 2500) / TiberiumLayoutCells->Count();
	int tib_index = 0;
	int layout_index = 0;

	if (TiberiumLayoutCells->Count() > 0) {
		do {
			int amount = (int)((double)tiberium_count + Sample_Truncated_Normal(0.0, 50.0, -100.0, 100.0));
			bool use_blue_tiberium;
			bool use_tiberium_tree;
			if (SeedData.UseBlueTiberium) {
				use_blue_tiberium = Pick_Random_UInt(0, 100) < 70;
			} else {
				use_blue_tiberium = true;
			}
			if (use_blue_tiberium) {
				use_tiberium_tree = Pick_Random_UInt(0, 100) < 25;
			} else {
				use_tiberium_tree = false;
			}

			if (amount >= 0) {
				Create_Tiberium_Patch((*TiberiumLayoutCells)[layout_index], amount, layout_index + 1, use_blue_tiberium, use_tiberium_tree, 1);
			}
		} while (++layout_index < TiberiumLayoutCells->Count());
	}

	double *mean_distances = new double[SeedData.NumPlayers];
	double *mean_distances_start = mean_distances;
	int player_index = 0;
	if (SeedData.NumPlayers > 0) {
		double *dptr = mean_distances;
		do {
			double total_distance = 0.0;
			Cell waypoint = Scen->Get_Waypoint_Cell((WAYPOINT)player_index);
			if (TiberiumLayoutCells->Count() > 0) {
				int y = waypoint.Y;
				int x = waypoint.X;
				do {
					Cell c = (*TiberiumLayoutCells)[tib_index];
					total_distance += std::sqrt((double)((c.X - x) * (c.X - x) + (c.Y - y) * (c.Y - y)));
					tib_index++;
				} while (tib_index < TiberiumLayoutCells->Count());
				tib_index = 0;
			}
			player_index++;
			*dptr++ = total_distance / (double)TiberiumLayoutCells->Count();
		} while (player_index < SeedData.NumPlayers);
	}

	double min_mean_distance = 9999999.0;
	player_index = 0;
	while (player_index < SeedData.NumPlayers) {
		if (mean_distances[player_index] < min_mean_distance) {
			min_mean_distance = mean_distances[player_index];
		}
		player_index++;
	}

	bool use_blue_tiberium = false;
	if (!SeedData.UseBlueTiberium || Pick_Random_UInt(0, 100) < 75) {
		use_blue_tiberium = true;
	}

	static const double _distance_scale = 15.0;

	if (SeedData.NumPlayers > 0) {
		do {
			int d = (int)((*mean_distances - min_mean_distance) * _distance_scale) + 500;
			Create_Tiberium_Patch(Scen->Get_Waypoint_Cell((WAYPOINT)tib_index), d, ++tib_index, use_blue_tiberium, use_blue_tiberium, 0);
			mean_distances++;
		} while (tib_index < SeedData.NumPlayers);
		mean_distances = mean_distances_start;
	}

	delete[] mean_distances;
}


/// <summary>
/// Grows one tiberium field outward from a cell.
/// This routine spreads tiberium over open ground, thickening what is already there rather than
/// overwriting it, and will pick a fresh start if the growth stalls early. It can also plant a
/// tiberium tree at the heart of the field and, with the Firestorm addon present, turn a few
/// tiberium creatures loose in it -- which is how wildlife reaches a generated map.
/// </summary>
/// <param name="cell">The cell the field grows out from.</param>
/// <param name="count">The number of cells of tiberium to lay down.</param>
/// <param name="patch_id">Tag marking a cell as claimed by this field, so that the field cannot
/// grow back over itself.</param>
/// <param name="use_primary_tiberium">Should the primary tiberium overlays be used rather than
/// the alternate ones?</param>
/// <param name="place_tree">Should a tiberium tree be planted in the field?</param>
/// <param name="spawn_wildlife">Should tiberium creatures be turned loose in the field?</param>
void MapGeneratorClass::Create_Tiberium_Patch(Cell const &cell, int count, int patch_id, bool use_primary_tiberium, bool place_tree, bool spawn_wildlife)
{
	static char const *_tib_creature_names[] = {
		"JFISH",
		"VISC_SML",
		"VISC_LRG",
		"DOGGIE"
	};

	static char const *_tib_tree_names[] = {
		"TIBTRE01",
		"TIBTRE02",
		"TIBTRE03"
	};

	int placed_count = 0;
	CellNode *nodes = new CellNode[10 * count];
	PriorityQueueClass<CellNode> *queue = new PriorityQueueClass<CellNode>(10 * count);

	bool queue_was_reset_for_spread = false;
	Cell spread_origin = cell;
	int restart_count = 0;
	Cell first_placed_cell = CELL_NONE;

	float tib_wildlife_factor = (float)SeedData.TiberiumWildlife * 0.01f;
	int wildlife_count = (int)((double)(unsigned int)(Pick_Random_UInt(0, 4) - 2) * tib_wildlife_factor);
	if (!Addon_Enabled(ADDON_FIRESTORM)) {
		wildlife_count = 0;
	}

	int wildlife_trigger = Pick_Random_UInt(0, count);
	HouseClass *neutral = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));

	queue->Clear();
	CellNode *node = NULL;
	int marker_index = 0;
	int visited = 1;

	while (placed_count < count) {
		if (restart_count >= 10) {
			break;
		}

		if (node == NULL) {
			queue->Clear();

			Map.Reset_Iterator();
			CellClass *iter = Map.Iterate();
			while (iter != NULL) {
				Cell c = iter->CellID;
				Set_Cell_Data_Spread(c, 0);
				iter = Map.Iterate();
			}

			marker_index = 1;
			nodes[0].Element = cell;
			nodes[0].Score = 0;
			Set_Cell_Data_Spread(cell, patch_id);
			queue->Insert(nodes[0]);
			node = queue->Extract_Min();
			queue_was_reset_for_spread = false;
			restart_count++;
			spread_origin = cell;
		}

		/*
		 * Every use below reads node->Element THROUGH the node pointer, and the loop
		 * re-reads it on every pass. This is load-bearing: the neighbor writes go
		 * to nodes[marker_index], and marker_index is reset to 0 in the block below
		 * while node still points at nodes[0], so the first neighbor stored OVERWRITES
		 * node->Element and the remaining directions are taken from the new cell.
		 * Caching the cell in a local would silently scan the true 8-neighborhood and
		 * generate different tiberium fields.
		 */
		CellClass *cellptr = &Map[node->Element];
		if (MapRegionClass::CellData::Is_Not_Inviolate(node->Element)) {
			if (!queue_was_reset_for_spread) {
				spread_origin = node->Element;
				queue->Clear();
				marker_index = 0;
				queue_was_reset_for_spread = true;
			}

			if (cellptr->Overlay == OVERLAY_NONE) {
				if (use_primary_tiberium) {
					cellptr->Overlay = OverlayType(Pick_Random_UInt(0, 11) + 102);
				} else {
					cellptr->Overlay = OverlayType(Pick_Random_UInt(0, 11) + 127);
				}
				placed_count++;
				visited++;
			} else {
				if (cellptr->OverlayData < OVERLAYDATA_TIBERIUM_MAX) {
					cellptr->OverlayData++;
					placed_count++;
					visited++;
				}
			}

			if (first_placed_cell == CELL_NONE) {
				first_placed_cell = node->Element;
			}

			if (placed_count == wildlife_trigger && wildlife_count > 0 && spawn_wildlife) {
				int creature_index = Pick_Random_UInt(0, 3);
				FootClass *foot = NULL;
				if (creature_index < 3) {
					foot = new UnitClass(UnitTypes[UnitTypeClass::From_Name(_tib_creature_names[creature_index])], neutral);
				} else {
					foot = new InfantryClass(InfantryTypes[InfantryTypeClass::From_Name(_tib_creature_names[creature_index])], neutral);
				}

				if (foot != NULL) {
					if (!foot->Unlimbo(node->Element.As_Coord(), DIR_N)) {
						delete foot;
					}
				}

				wildlife_count--;
				if (visited < count) {
					wildlife_trigger = Pick_Random_UInt(visited, count);
				}
			}
		}

		CellNode *newnode = &nodes[marker_index];
		for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell newcell = Adjacent_Cell(node->Element, (FacingType)dir);
			if (Map.In_Local_Radar(newcell, true)) {
				CellClass *newcellptr = &Map[newcell];
				if (newcellptr->Is_Tile_Clear()) {
					if ((newcellptr->Overlay == OVERLAY_NONE && Get_Cell_Data_Spread(newcell) != patch_id)
							|| (newcellptr->OverlayData < OVERLAYDATA_TIBERIUM_MAX && newcellptr->Tiberium_Type_Here() != TIBERIUM_NONE)) {
						newnode->Element = newcell;
						newnode->Score = Get_Tiberium_Score(spread_origin, newcell);
						Set_Cell_Data_Spread(newcell, patch_id);
						marker_index++;
						queue->Insert(*newnode++);
					}
				}
			}
		}

		node = queue->Extract_Min();
	}

	if (place_tree) {
		if (first_placed_cell != CELL_NONE) {
			Map[first_placed_cell].Overlay = OVERLAY_NONE;
			new TerrainClass(TerrainTypes[TerrainTypeClass::From_Name(_tib_tree_names[Pick_Random_UInt(0, 2)])], first_placed_cell);
		}
	}

	delete[] nodes;
	delete queue;
}


/// <summary>
/// Fetches the spread priority of a candidate tiberium cell.
/// Create_Tiberium_Patch grows in cheapest first order, so this score is what keeps a field
/// gathered about its origin while leaving the edge irregular.
/// </summary>
/// <param name="cell1">The cell the field is growing out from.</param>
/// <param name="cell2">The candidate cell being considered.</param>
/// <returns>Returns with the score, where a lower score means the cell is reached sooner.</returns>
double MapGeneratorClass::Get_Tiberium_Score(const Cell & cell1, const Cell & cell2) const
{
	static const double _random_range = 5.0;

	int x = cell1.X - cell2.X;
	int y = cell1.Y - cell2.Y;
	double dist = std::sqrt(x * x + y * y);
	return(RMG_RANDOM_DOUBLE(_random_range) + dist);
}


/// <summary>
/// Works out how high the hills of the map should stand.
/// This routine sweeps the map giving each cell a height drawn from its already settled
/// neighbors, which is what makes the terrain roll rather than jump. The seed's hills setting
/// governs how far the heights are allowed to wander, and a flat enough setting leaves the map
/// alone entirely. Cells anchored by Seed_Hill_Anchors keep the height they were given.
/// </summary>
void MapGeneratorClass::Seed_Hill_Heights(void)
{
	static const double scale = 0.005;

	int i;
	int grid_size = MapCellStride * MapCellStride;
	double height_step = SeedData.Hills * 0.001 + 0.1;
	double min_volatility = SeedData.Hills * 0.0025;
	double max_volatility = SeedData.Hills * 0.005;

	if (min_volatility >= scale) {
		for (i = 0; i < grid_size; i++) {
			MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(i);
			Cell cell = data.CellID;
			if (cell != Cell(0, 0)) {
				data.HillVolatility = 0;
				double up_height = 0.0;
				double height_trend = 0.0;

				if (My_In_Radar(cell - Cell(1, 1))) {
					up_height = MapRegionClass::Get_Cell_Data(cell - Cell(1, 1)).HillHeight;
				}

				Cell newcell = cell - Cell(1, 0);
				if (My_In_Radar(newcell)) {
					MapRegionClass::CellData & data2 = MapRegionClass::Get_Cell_Data(newcell);
					data.HillHeight += data2.HillHeight;
					data.HillVolatility = data2.HillVolatility;
					height_trend += data2.HillHeight - up_height;
				} else {
					data.HillHeight = data.HillHeight;
					data.HillVolatility = min_volatility;
				}

				newcell = cell - Cell(0, 1);
				if (My_In_Radar(newcell)) {
					MapRegionClass::CellData & data2 = MapRegionClass::Get_Cell_Data(newcell);
					data.HillHeight += data2.HillHeight;
					data.HillVolatility += data2.HillVolatility;
					height_trend += data2.HillHeight - up_height;
				} else {
					data.HillHeight = data.HillHeight;
					data.HillVolatility += min_volatility;
				}

				data.HillHeight *= 0.5;
				data.HillVolatility *= 0.5;
				if (height_trend > 0.0) {
					height_trend = height_step;
				} else if (height_trend < 0.0) {
					height_trend = -height_step;
				}
				const double mean = 0.0;
				data.HillVolatility += Sample_Truncated_Normal(mean, scale, -data.HillVolatility, max_volatility - data.HillVolatility);

				if (!data.Inviolate) {
					double min_height = -2.0 - data.HillHeight;
					double max_height = 2.0 - data.HillHeight;
					double trend;
					if (height_trend < min_height) {
						trend = min_height;
					} else if (height_trend > max_height) {
						trend = max_height;
					} else {
						trend = height_trend;
					}
					data.HillHeight += Sample_Truncated_Normal(trend, data.HillVolatility, min_height, max_height);
				}
			}
		}

		for (i = 0; i < grid_size; i++) {
			MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(i);
			if (data.HillHeight < 0) {
				data.HillHeight = (int)(data.HillHeight - 0.5);
			}
			if (data.HillHeight > 0) {
				data.HillHeight = (int)(data.HillHeight + 0.5);
			}
		}
	}
}


/// <summary>
/// Pins down the hill heights that must not move.
/// This routine runs ahead of Seed_Hill_Heights and protects the ground beside shorelines and
/// cliffs, so that the random walk which follows cannot raise a hill into the sea or bury an
/// existing cliff face.
/// </summary>
void MapGeneratorClass::Seed_Hill_Anchors(void)
{
	Map.Reset_Iterator();

	CellClass * cptr = Map.Iterate();
	while (cptr != NULL) {
		if (cptr->Is_Tile_Shore()) {
			for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
				Cell cell = Adjacent_Cell(cptr->Fetch_CellID(), (FacingType)dir);
				if (My_In_Radar(cell) && Map[cell].Is_Tile_Clear()) {
					MapRegionClass::Get_Cell_Data(cell).HillHeight = 0.5;
					MapRegionClass::Get_Cell_Data(cell).Inviolate = true;
					break;
				}
			}
		} else if (cptr->Is_Tile_Cliff()) {
			for (int dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
				Cell cell = Adjacent_Cell(cptr->Fetch_CellID(), (FacingType)dir);
				if (My_In_Radar(cell) && Map[cell].Is_Tile_Clear()) {
					MapRegionClass::Get_Cell_Data(cell).Inviolate = true;
					break;
				}
			}
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Raises the hills of the map.
/// This routine takes the per cell hill heights worked out by the seeding passes and lifts the
/// terrain to meet them one step at a time, letting the smoothing system cut the slopes and
/// ramps in as it goes. Ramp groups that turn out to be redundant are flattened away again.
/// </summary>
void MapGeneratorClass::Generate_Hills(void)
{
	Seed_Hill_Anchors();
	Seed_Hill_Heights();

	Init_Deform_Grid_RMG();

	int grid_size = MapCellStride * MapCellStride;
	for (int i = 0; i < grid_size; i++) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(i);
		if (data.CellID != Cell(0, 0)) {
			CellClass * cellptr = &Map[data.CellID];
			int change = cellptr->Height + data.HillHeight - Get_Deformed_Cell_Height(cellptr);
			int steps = abs(change);
			int direction = change < 0 ? -1 : 1;
			while (steps > 0) {
				Deform_Cell_RMG(data.CellID, direction, 0, Get_Deformable_Cell_Corners(cellptr, direction));
				steps--;
			}
		}
	}

	Commit_Deform_Grid_RMG();

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();

	Cell offsets[] = { Cell(0, 0), Cell(1, 0), Cell(1, 1), Cell(0, 1)};
	RampType ramps[] = { RAMP_MID_SE, RAMP_MID_SW, RAMP_MID_NW, RAMP_MID_NE };

	while (cptr != NULL) {
		int height_step = 0;
		bool raise = true;
		if (cptr->Ramp == RAMP_CORNER_NW) {
			for (int i = 0; i < ARRAY_SIZE(offsets); i++) {
				if (Map[cptr->Fetch_CellID() + offsets[i]].Ramp != RAMP_CORNER_NW + i) {
					raise = false;
				}
			}
		} else if (cptr->Ramp == RAMP_MID_SE) {
			height_step += 1;
			for (int i = 0; i < ARRAY_SIZE(offsets); i++) {
				if (Map[cptr->Fetch_CellID() + offsets[i]].Ramp != ramps[i]) {
					raise = false;
				}
			}
		} else {
			raise = false;
		}

		if (raise) {
			for (int i = 0; i < ARRAY_SIZE(offsets); i++) {
				CellClass * cellptr = &Map[cptr->Fetch_CellID() + offsets[i]];
				cellptr->ITType = ISOTILE_NONE;
				cellptr->SubTile = 0;
				cellptr->Ramp = RAMP_NONE;
				cellptr->Height += height_step;
			}
		}

		cptr = Map.Iterate();
	}
}


/// <summary>
/// Seeds the ground cover chances of the map.
/// This routine decides, cell by cell, how likely each kind of ground cover is before
/// Generate_Vegetation does the actual planting. The seed's vegetation setting scales the lot,
/// and the ground along a shoreline is made far greener than the interior.
/// </summary>
void MapGeneratorClass::Seed_Vegetation(void)
{
	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();

	int index = SeedData.Biome - 2;
	double density = SeedData.Vegetation * 0.01;

	double chances[3][4] = {
		/// rough, green, sand, forest
		{0.005,   0.02,  0.0,  0.003}, /// Temperate
		{0.01,    0.0,   0.03, 0.001}, /// Desert
		{0.005,   0.02,  0.0,  0.003}  /// Mutated
	};

	while (cptr != NULL) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID());
		if (cptr->Is_Tile_Shore()) {
			for (int x = cptr->Fetch_CellID().X - 2; x < cptr->Fetch_CellID().X + 3; x++) {
				for (int y = cptr->Fetch_CellID().Y - 2; y < cptr->Fetch_CellID().Y + 3; y++) {
					if (My_In_Radar(Cell(x, y))) {
						data.RoughTileChance = chances[index][0];
						data.GreenTileChance = density * chances[index][1] * 10.0;
						data.SandTileChance = 0;
						data.ForestChance = density * chances[index][3] * 10.0;
					}
				}
			}
		} else if (data.RoughTileChance == 0.0) {
			data.RoughTileChance = chances[index][0];
			data.GreenTileChance = density * chances[index][1];
			data.SandTileChance = chances[index][2];
			data.ForestChance = density * chances[index][3];
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Seeds the ground cover chances of an arctic map.
/// This is the tundra and taiga counterpart of Seed_Vegetation, and like it, it runs ahead of
/// Generate_Arctic_Vegetation, which does the actual planting.
/// </summary>
void MapGeneratorClass::Seed_Arctic_Vegetation(void)
{
	int index = SeedData.Biome;

	double chances[2][3] = {
		/// rough, forest, sand
		{0.005,   0.0005, 0.002}, /// Tundra
		{0.005,   0.0015, 0.001}  /// Taiga
	};

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();

	double density = SeedData.Vegetation * 0.01;
	while (cptr != NULL) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID());
		data.RoughTileChance = chances[index][0];
		data.ForestChance = density * chances[index][1];
		data.SandTileChance = chances[index][2];
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Dresses a temperate, desert or mutated map with ground cover.
/// This routine walks the map planting green, rough and sand patches and the occasional wood
/// against the chances that Seed_Vegetation left on each cell, then litters the result with
/// rocks. A mutated map is given its mold and crystal growths first.
/// </summary>
void MapGeneratorClass::Generate_Vegetation(void)
{
	CellClass * cptr;

	if (SeedData.Biome == BIOME_MUTATED) {
		int map_size = ((SeedData.Width + 1) * (SeedData.Height + 1));

		int mold_count = Pick_Random_UInt(3, (map_size / 4) + 4);
		while (mold_count > 0) {
			Generate_Mold();
			mold_count--;
		}

		DynamicVectorClass<Cell> crystal_cliffs;
		Map.Reset_Iterator();
		cptr = Map.Iterate();
		while (cptr != NULL) {
			if (cptr->Is_Tile_Cliff() && cptr->SubTile == 0) {
				int cliff = cptr->ITType - IsometricTileTypeClass::CliffSet;
				if ((cliff >= 4 && cliff <= 6 || cliff >= 14 && cliff <= 16)) {
					crystal_cliffs.Add(cptr->CellID);
				}
			}
			cptr = Map.Iterate();
		}

		int crystal_count = Pick_Random_UInt(6, (SeedData.Width + 1) * (SeedData.Height + 1) + 8);
		while (crystal_count > 0) {
			if (Pick_Random_UInt(0, 1) == 0 && crystal_cliffs.Count() > 0) {
				int cell_index = Pick_Random_UInt(0, crystal_cliffs.Count() - 1);
				Generate_Crystals(crystal_cliffs[cell_index]);
				crystal_cliffs.Delete_Index(cell_index);
			} else {
				Generate_Crystals(MapRegionClass::Pick_Random_Clear_Map_Cell());
			}
			crystal_count--;
		}
	}

	if (SeedData.Biome == BIOME_MUTATED || SeedData.Biome == BIOME_DESERT) {
		int rough_count = Pick_Random_UInt(5, 30);
		int placed = 0;
		for (int tries = 0; tries < rough_count * 20 && placed < rough_count; tries++) {
			IsometricTileType rough_tile = IsometricTileType(IsometricTileTypeClass::RoughGround + Pick_Random_UInt(0, 9));
			Cell c = MapRegionClass::Pick_Random_Map_Cell();
			if (Can_Place_Tile(rough_tile, c, false, false)) {
				Place_Tile(rough_tile, c);
				placed++;
			}
		}
	}

	Clear_Cell_Data_Spreads();

	Map.Reset_Iterator();
	cptr = Map.Iterate();

	int mean_index = SeedData.Biome - 2;
	int means[3][3] = {
		{30, 20, 20}, /// Temperate
		{20, 20, 40}, /// Desert
		{30, 20, 20}  /// Mutated
	};

	while (cptr != NULL) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID());
		if (cptr->Is_Tile_Clear() && cptr->Ramp == RAMP_NONE && cptr->Overlay == OVERLAY_NONE && cptr->Cell_Occupier() == NULL && !data.Inviolate) {
			Cell cell = cptr->Fetch_CellID();
			int origin_id = cell.X + cell.Y * MAP_CELL_W;
			if (Random_Fraction() < data.GreenTileChance) {
				Place_Tile_Patch(cptr, IsometricTileTypeClass::GreenTile, Sample_Truncated_Normal(means[mean_index][0], 20, 4, 80), origin_id, false);
			} else {
				if (Random_Fraction() < data.RoughTileChance) {
					Place_Tile_Patch(cptr, IsometricTileTypeClass::RoughTile, Sample_Truncated_Normal(means[mean_index][1], 20, 4, 80), origin_id, false);
				} else {
					if (Random_Fraction() < data.SandTileChance) {
						Place_Tile_Patch(cptr, IsometricTileTypeClass::SandTile, Sample_Truncated_Normal(means[mean_index][2], 20, 4, 80), origin_id, false);
					} else {
						if (Random_Fraction() < data.ForestChance) {
							Place_Forest(cptr, Sample_Truncated_Normal(25, 10, 10, 35), Sample_Truncated_Normal(0.2, 0.1, 0.05, 0.4));
						}
					}
				}
			}
		}
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Fixup_LAT();
		cptr = Map.Iterate();
	}

	int overlay_count = Pick_Random_UInt(0, Map_Cell_Count() / 100);
	if (SeedData.Biome == BIOME_TEMPERATE) {
		overlay_count /= 2;
	}

	int placed = 0;
	for (int tries = 0; tries < overlay_count * 5 && placed < overlay_count; tries++) {
		CellClass * cellptr = &Map[MapRegionClass::Pick_Random_Map_Cell()];
		if (cellptr->Overlay == OVERLAY_NONE) {
			if (cellptr->Is_Tile_Clear_To_Sand_LAT()) {
				cellptr->Overlay = OverlayType(Pick_Random_UInt(0, 4) + OVERLAY_SAND_ROCK_01);
			} else {
				if (cellptr->Is_Tile_Clear() || cellptr->Is_Tile_Clear_To_Green_LAT()) {
					cellptr->Overlay = OverlayType(Pick_Random_UInt(0, 4) + OVERLAY_CLEAR_ROCK_01);
				}
			}
			cellptr->OverlayData = 0;
			cellptr->Recalc_Attributes();
			placed++;
		}
	}
}


/// <summary>
/// Dresses an arctic map with ground cover.
/// This is the tundra and taiga counterpart of Generate_Vegetation. It scatters rough ground,
/// then walks the map planting rough patches, woods and rock fields against the chances that
/// Seed_Arctic_Vegetation left on each cell.
/// </summary>
void MapGeneratorClass::Generate_Arctic_Vegetation(void)
{
	CellClass * cptr;

	int rough_count = Pick_Random_UInt(5, 30);
	int placed = 0;
	for (int tries = 0; tries < rough_count * 20 && placed < rough_count; tries++) {
		IsometricTileType rough_tile = IsometricTileType(IsometricTileTypeClass::RoughGround + Pick_Random_UInt(0, 9));
		Cell c = MapRegionClass::Pick_Random_Map_Cell();
		if (Can_Place_Tile(rough_tile, c, false, false)) {
			Place_Tile(rough_tile, c);
			placed++;
		}
	}

	Clear_Cell_Data_Spreads();

	Map.Reset_Iterator();
	cptr = Map.Iterate();

	int mean_index = SeedData.Biome;
	int means[3] = {
		20, /// Tundra
		20, /// Taiga
		0,  /// Temperate
	};

	while (cptr != NULL) {
		MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(cptr->Fetch_CellID());
		if (cptr->Is_Tile_Clear() && cptr->Ramp == RAMP_NONE && !data.Inviolate) {
			Cell cell = cptr->CellID;
			int origin_id = cell.X + cell.Y * MAP_CELL_W;
			if (Random_Fraction() < data.RoughTileChance) {
				Place_Tile_Patch(cptr, IsometricTileTypeClass::RoughTile, Sample_Truncated_Normal(means[mean_index], 15, 4, 60), origin_id, false);
			}
			if (Random_Fraction() < data.ForestChance && !data.Forested && !data.Inviolate) {
				Place_Forest(cptr, Sample_Truncated_Normal(30, 10, 10, 45), Sample_Truncated_Normal(0.2, 0.1, 0.05, 0.4));
			}
			if (Random_Fraction() < data.SandTileChance) {
				Place_Tile_Patch(cptr, IsometricTileTypeClass::Rocks, Sample_Truncated_Normal(means[mean_index], 11, 3, 30), origin_id, false);
			}
		}
		cptr = Map.Iterate();
	}

	Map.Reset_Iterator();
	cptr = Map.Iterate();
	while (cptr != NULL) {
		cptr->Fixup_LAT();
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Plants a wood outward from a starting cell.
/// This routine spreads over the ground around the seed cell and plants a tree on only some of
/// what it reaches, which is what gives a wood a dense heart and a thinning edge. Ground it has
/// walked over is remembered, so that woods do not pile up on one another.
/// </summary>
/// <param name="cellptr">The cell the wood grows out from.</param>
/// <param name="count">The number of cells the wood may spread over.</param>
/// <param name="density">The chance, from 0 to 1, that a cell reached is given a tree.</param>
void MapGeneratorClass::Place_Forest(CellClass const * cellptr, int count, double density)
{
	CellNode * nodes = new CellNode[10 * count];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(10 * count);
	int i = 0;
	queue->Clear();

	int node_count = 1;
	Cell cell = cellptr->Fetch_CellID();
	nodes->Element = cell;
	nodes->Score = 0;

	MapRegionClass::Get_Cell_Data(cellptr->Fetch_CellID()).Forested = true;

	queue->Insert(*nodes);

	CellNode * node = queue->Extract_Min();

	int min_tree = 1;
	int max_tree = 25;
	if (SeedData.Biome != BIOME_DESERT && SeedData.Biome != BIOME_MUTATED) {
		if (SeedData.Biome == BIOME_TEMPERATE) {
			max_tree = 20;
		}
	} else {
		min_tree = 21;
	}

	while (i < count && node != NULL) {
		CellClass * cptr = &Map[node->Element];
		if (cptr->Is_Tile_Clear() && cptr->Cell_Occupier() == NULL && cptr->Overlay == OVERLAY_NONE && cptr->Land_Type() != LAND_ROCK) {
			if (Random_Fraction() < density) {
				char tree_name[20];
				int tree_index = Pick_Random_UInt(min_tree, max_tree);
				sprintf(tree_name, "TREE%d%d", tree_index / 10, tree_index % 10);
				new TerrainClass(TerrainTypes[TerrainTypeClass::From_Name(tree_name)], cptr->Fetch_CellID());
			}
		}

		CellNode * newnode = &nodes[node_count];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell newcell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(newcell)) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);
				if (!data.Forested && !data.Inviolate) {
					newnode->Element = newcell;
					newnode->Score = Get_Forest_Score(cellptr->Fetch_CellID(), newcell);
					data.Forested = true;
					node_count++;
					queue->Insert(*newnode++);
				}
			}
		}

		i++;
		node = queue->Extract_Min();
	}

	delete[] nodes;
	delete queue;
}


/// <summary>
/// Fetches the spread priority of a candidate forest cell.
/// Place_Forest plants in cheapest first order, so this score is what gives a wood its rounded
/// shape and its uneven treeline.
/// </summary>
/// <param name="cell1">The cell the forest is growing out from.</param>
/// <param name="cell2">The candidate cell being considered.</param>
/// <returns>Returns with the score, where a lower score means the cell is reached sooner.</returns>
double MapGeneratorClass::Get_Forest_Score(const Cell & cell1, const Cell & cell2) const
{
	static const double _random_range = 5.0;

	int x = cell1.X - cell2.X;
	int y = cell1.Y - cell2.Y;
	double dist = std::sqrt(x * x + y * y);
	return(RMG_RANDOM_DOUBLE(_random_range) + dist);
}


/// <summary>
/// Grows a patch of one isometric tile type across the ground.
/// This is how the vegetation pass lays down its blotches of green, rough and sand ground. The
/// patch spreads outward from the starting cell over open ground alone, and stops once it has
/// claimed its quota of cells.
/// </summary>
/// <param name="cellptr">The cell the patch grows out from.</param>
/// <param name="ittype">The isometric tile type to lay down.</param>
/// <param name="count">The number of cells the patch may claim.</param>
/// <param name="origin_id">Tag marking a cell as claimed by this patch, so that the patch cannot
/// grow back over itself.</param>
/// <param name="on_pavement">May the patch spread over pavement as well as open ground?</param>
void MapGeneratorClass::Place_Tile_Patch(CellClass * cellptr, IsometricTileType ittype, int count, int origin_id, bool on_pavement)
{
	CellNode * nodes = new CellNode[10 * count];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(10 * count);
	int i = 0;
	queue->Clear();

	int node_count = 1;
	Cell cell = cellptr->Fetch_CellID();
	nodes->Element = cell;
	nodes->Score = 0;

	MapRegionClass::Get_Cell_Data(cellptr->Fetch_CellID()).SpreadID = origin_id;

	queue->Insert(*nodes);

	CellNode * node = queue->Extract_Min();
	while (i < count && node != NULL) {
		CellClass * cptr = &Map[node->Element];
		cptr->ITType = ittype;

		CellNode * newnode = &nodes[node_count];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell newcell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(newcell)) {
				CellClass * newcellptr = &Map[newcell];
				if (newcellptr->Is_Tile_Clear() || on_pavement && (newcellptr->Is_Tile_Pavement() || newcellptr->Is_Tile_Misc_Pavement())) {
					if (MapRegionClass::Get_Cell_Data(newcell).SpreadID != origin_id && newcellptr->Ramp == RAMP_NONE && newcellptr->Overlay == OVERLAY_NONE && newcellptr->Cell_Occupier() == NULL) {
						newnode->Element = newcell;
						newnode->Score = Get_Tile_Patch_Score(cellptr->Fetch_CellID(), newcell);
						MapRegionClass::Get_Cell_Data(newcell).SpreadID = origin_id;
						node_count++;
						queue->Insert(*newnode++);
					}
				}
			}
		}

		i++;
		node = queue->Extract_Min();
	}

	delete[] nodes;
	delete queue;
}


/// <summary>
/// Fetches the spread priority of a candidate patch cell.
/// Place_Tile_Patch fills in cheapest first order, so this score is what keeps a patch roughly
/// round while leaving its edge ragged.
/// </summary>
/// <param name="cell1">The cell the patch is growing out from.</param>
/// <param name="cell2">The candidate cell being considered.</param>
/// <returns>Returns with the score, where a lower score means the cell is filled sooner.</returns>
double MapGeneratorClass::Get_Tile_Patch_Score(const Cell & cell1, const Cell & cell2) const
{
	static const double _random_range = 5.0;

	int x = cell1.X - cell2.X;
	int y = cell1.Y - cell2.Y;
	double dist = std::sqrt(x * x + y * y);
	return(RMG_RANDOM_DOUBLE(_random_range) + dist);
}


/// <summary>
/// Grows one patch of blue mold on a mutated map.
/// This routine hunts for an unobstructed spot, spreads mold outward from it, then blends the
/// patch into the surrounding ground and dresses it with a little foliage. If nowhere suitable
/// turns up it does nothing rather than force a patch into a bad place.
/// </summary>
void MapGeneratorClass::Generate_Mold(void)
{
	Cell mold_origin = CELL_NONE;
	int tries = 0;

	DebugString("Generating mold\n");

	while (mold_origin == CELL_NONE) {
		Cell c = MapRegionClass::Pick_Random_Clear_Map_Cell();
		if (Is_Cell_Area_Clear(Rect(c.X - 1, c.Y - 1, 3, 3), true)) {
			mold_origin = c;
		}
		if (++tries >= 200) {
			break;
		}
	}

	if (mold_origin == CELL_NONE) {
		return;
	}

	DynamicVectorClass<Cell> cells;
	cells.Set_Growth_Step(500);

	int spread_count = Pick_Random_UInt(30, 150);
	int heap_size = std::max(100, 6 * spread_count);

	CellNode * nodes = new CellNode[heap_size];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(heap_size);
	queue->Clear();

	int marker_index = 1;
	nodes[0].Element = mold_origin;
	nodes[0].Score = 0;
	queue->Insert(nodes[0]);

	CellNode * node = queue->Extract_Min();
	int spread_step = 0;
	while (spread_step < spread_count && node != NULL) {
		CellClass * cptr = &Map[node->Element];
		cptr->ITType = IsometricTileTypeClass::BlueMoldTile;
		cptr->SubTile = 0;
		cells.Add(cptr->CellID);

		CellNode * newnode = &nodes[marker_index];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
			Cell newcell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(newcell)) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);

				bool can_spread = true;
				for (FacingType dir2 = FACING_FIRST; dir2 < FACING_COUNT; dir2 = FacingType(dir2 + FACING_90)) {
					Cell adj = Adjacent_Cell(newcell, dir2);
					CellClass * adjptr = &Map[adj];
					if (!adjptr->Is_Tile_Clear() && adjptr->ITType != IsometricTileTypeClass::BlueMoldTile) {
						can_spread = false;
						break;
					}
				}

				if (can_spread && !data.Inviolate && marker_index < heap_size) {
					newnode->Element = newcell;
					newnode->Score = Get_Spread_Score(mold_origin, newcell, spread_step);
					data.SpreadID = WorkingRegionID;
					marker_index++;
					queue->Insert(*newnode++);
				}
			}
		}

		spread_step++;
		node = queue->Extract_Min();
	}

	int i;
	for (i = 0; i < cells.Count(); i++) {
		Map[cells[i]].Fixup_LAT();
	}

	int foliage_count = Pick_Random_UInt(0, 4);
	for (i = 0; i < foliage_count; i++) {
		Cell c = CELL_NONE;
		int local_tries = 0;
		while (c == CELL_NONE) {
			c = cells[Pick_Random_UInt(0, cells.Count() - 1)];
			if (Map[c].ITType != IsometricTileTypeClass::BlueMoldTile) {
				c = CELL_NONE;
			}
			if (++local_tries >= 200) {
				break;
			}
		}

		if (c == CELL_NONE) {
			break;
		}

		if (Map[c].Cell_Terrain(false) == NULL && Map[c].Overlay == OVERLAY_NONE) {
			if (Random_Fraction() < 0.75) {
				char terrain_name[8];
				int terrain_index = Pick_Random_UInt(1, 5);
				sprintf(terrain_name, "FONA0%d", terrain_index);
				new TerrainClass(TerrainTypes[TerrainTypeClass::From_Name(terrain_name)], c);
			} else {
				Map[c].Overlay = OVERLAY_LARGE_TIBERIUM01;
				Map[c].OverlayData = 0;
			}
		}
	}

	DebugString("Mold heap size: %d -- Marker Index: %d\n", heap_size, marker_index);

	delete queue;
	delete[] nodes;
}


/// <summary>
/// Grows one crystal deposit on a mutated map.
/// The seed may be open ground or a cliff face; on a cliff the routine hangs a crystal
/// formation off the rock and spreads out from the foot of it. The finished patch is blended
/// into its surroundings and dressed with a little foliage.
/// </summary>
/// <param name="cell">The cell to seed the deposit at.</param>
void MapGeneratorClass::Generate_Crystals(const Cell & cell)
{
	bool on_cliff = false;

	Cell cliff_seed = CELL_NONE;
	Cell crystal_origin;

	DebugString("Generating crystal\n");

	if (!Map[cell].Is_Tile_Clear()) {
		on_cliff = true;

		int cliff = Map[cell].ITType - IsometricTileTypeClass::CliffSet;
		if (cliff >= 4 && cliff <= 6) {
			if (Is_Cell_Area_Clear(Rect(cell.X, cell.Y - 1, 2, 1), false) && Pick_Random_UInt(0, 1) == 0) {
				Place_Tile((IsometricTileType)(IsometricTileTypeClass::CrystalCliff), cell, -1, Map[Cell(cell.X, cell.Y + 1)].Height + 4);
				crystal_origin = Cell(cell.X, cell.Y - 1);
				cliff_seed = Cell(cell.X + 1, cell.Y - 1);
			} else {
				if (!Is_Cell_Area_Clear(Rect(cell.X, cell.Y + 2, 2, 1), false)) {
					return;
				}
				Place_Tile((IsometricTileType)(IsometricTileTypeClass::CrystalCliff + 1), cell, -1, Map[Cell(cell.X, cell.Y + 1)].Height + 4);
				crystal_origin = Cell(cell.X, cell.Y + 2);
				cliff_seed = Cell(cell.X + 1, cell.Y + 2);
			}
		} else if (cliff >= 14 && cliff <= 16) {
			if (Is_Cell_Area_Clear(Rect(cell.X - 1, cell.Y, 1, 2), false) && Pick_Random_UInt(0, 1) == 0) {
				Place_Tile((IsometricTileType)(IsometricTileTypeClass::CrystalCliff + 4), cell, -1, Map[Cell(cell.X + 1, cell.Y)].Height + 4);
				crystal_origin = Cell(cell.X - 1, cell.Y);
				cliff_seed = Cell(cell.X - 1, cell.Y + 1);
			} else {
				if (!Is_Cell_Area_Clear(Rect(cell.X + 2, cell.Y, 1, 2), false)) {
					return;
				}
				Place_Tile((IsometricTileType)(IsometricTileTypeClass::CrystalCliff + 5), cell, -1, Map[Cell(cell.X + 1, cell.Y)].Height + 4);
				crystal_origin = Cell(cell.X + 2, cell.Y);
				cliff_seed = Cell(cell.X + 2, cell.Y + 1);
			}
		} else {
			return;
		}
	} else {
		crystal_origin = cell;
	}

	if (crystal_origin == CELL_NONE) {
		return;
	}

	DynamicVectorClass<Cell> cells;
	cells.Set_Growth_Step(500);
	int spread_count = Pick_Random_UInt(30, 150);
	int heap_size = 6 * spread_count;
	if (heap_size < 100) {
		heap_size = 100;
	}

	CellNode *nodes = new CellNode[heap_size];
	PriorityQueueClass<CellNode> *queue = new PriorityQueueClass<CellNode>(heap_size);
	queue->Clear();

	int marker_index = 0;
	if (on_cliff) {
		nodes[0].Element = cliff_seed;
		nodes[0].Score = 0;
		marker_index = 1;
		queue->Insert(nodes[0]);
	}

	CellNode *seed = &nodes[marker_index++];
	seed->Element = crystal_origin;
	seed->Score = 0;
	queue->Insert(*seed);

	CellNode *node = queue->Extract_Min();
	int spread_step = 0;
	while (spread_step < spread_count) {
		if (node == NULL) {
			break;
		}
		CellClass *cptr = &Map[node->Element];
		cptr->ITType = IsometricTileTypeClass::CrystalTile;
		cptr->SubTile = 0;
		cells.Add(cptr->CellID);

		CellNode *newnode = &nodes[marker_index];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir = FacingType(dir + FACING_90)) {
			Cell newcell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(newcell)) {
				MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);
				if (Map[newcell].Is_Tile_Clear() && Map[newcell].Overlay == OVERLAY_NONE && marker_index < heap_size) {
					newnode->Element = newcell;
					newnode->Score = Get_Spread_Score(crystal_origin, newcell, spread_step);
					data.SpreadID = WorkingRegionID;
					marker_index++;
					queue->Insert(*newnode++);
				}
			}
		}

		spread_step++;
		node = queue->Extract_Min();
	}

	int i;
	for (i = 0; i < cells.Count(); i++) {
		Map[cells[i]].Fixup_LAT();
	}

	int foliage_count = Pick_Random_UInt(0, 5);
	for (i = 0; i < foliage_count; i++) {
		Cell c = CELL_NONE;
		int local_tries = 0;
		while (c == CELL_NONE) {
			c = cells[Pick_Random_UInt(0, cells.Count() - 1)];
			if (Map[c].ITType != IsometricTileTypeClass::CrystalTile || MapRegionClass::Get_Cell_Data(c).Inviolate) {
				c = CELL_NONE;
			}
			if (++local_tries >= 200) {
				break;
			}
		}

		if (c == CELL_NONE) {
			break;
		}

		if (Map[c].Cell_Terrain(false) == NULL) {
			char terrain_name[8];
			int terrain_index = Pick_Random_UInt(6, 15);
			sprintf(terrain_name, "FONA%d%d", terrain_index / 10, terrain_index % 10);
			new TerrainClass(TerrainTypes[TerrainTypeClass::From_Name(terrain_name)], c);
		}
	}

	DebugString("Mold heap size: %d -- Marker Index: %d\n", heap_size, marker_index);

	delete queue;
	delete[] nodes;
}

double ConditionRedChances[BIOME_COUNT] = {0.5, 0.2, 0.2, 0.6, 0.8};
double ConditionYellowChances[BIOME_COUNT] = {0.5, 0.3, 0.2, 0.5, 0.95};


/// <summary>
/// Scatters settlements across the map.
/// This routine is the civilization pass of the generator. It works out how much settlement
/// the seed asked for, then either grows a full urban area with roads, buildings and traffic,
/// or drops a smaller rural cluster instead. It gives up rather than hunt forever for
/// somewhere to build.
/// </summary>
void MapGeneratorClass::Generate_Urban_Areas(void)
{
	int count = (SeedData.Cities + 0.32) * 3;
	int tries = 0;
	int placed = 0;

	Init_Dirt_Roads();

	while (true) {
		if (placed >= count) {
			break;
		}
		Cell cell(0, 0);
		int tries_here = 0;
		while (true) {
			cell = MapRegionClass::Pick_Random_Clear_Map_Cell();
			if (++tries_here >= 20) break;
			if (MapRegionClass::CellData::Is_Not_Inviolate(cell) || Is_Area_Cliff_Free(cell)) {
				break;
			}
		}

		if (Random_Fraction() < (1.0 - SeedData.Cities)) {
			if (Generate_Rural_Roads(cell)) {
				Generate_Rural_Buildings(cell);
				Generate_Rural_Units(cell);
				placed++;
			}
		} else {
			if (tries_here != 20) {
				int size = Sample_Truncated_Normal(2 * Map.PlayRect.Width * Map.PlayRect.Height / 30, 300, 100, 2000);
				DynamicVectorClass<Cell> * cells = Create_Urban_Area(cell, size);
				if (cells != NULL) {
					Generate_Urban_Roads(*cells);
					Generate_Urban_Buildings(*cells);
					Generate_Urban_Units(*cells);
					Generate_Urban_Pavement(*cells);
					placed++;
					delete cells;
				}
			}
			tries++;
		}

		if (tries >= 10) {
			break;
		}
	}
}


/// <summary>
/// Paves an urban area by flood-filling pavement tiles outward from a seed cell
/// using a priority queue, then trims the border cells and scatters miscellaneous
/// pavement tiles across the result.
/// </summary>
/// <param name="cell">The seed cell to grow the urban area from.</param>
/// <param name="size">The maximum number of cells to process during the flood fill.</param>
/// <returns>The collected urban area cells, or NULL if the area is too small or sparse.</returns>
DynamicVectorClass<Cell> * MapGeneratorClass::Create_Urban_Area(Cell const & cell, int size)
{
	CellNode * nodes = new CellNode[10 * size];
	PriorityQueueClass<CellNode> * queue = new PriorityQueueClass<CellNode>(10 * size);
	queue->Clear();

	int processed = 0;

	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr != NULL) {
		Cell c = cptr->CellID;
		Set_Cell_Data_Spread(c, 0);
		cptr = Map.Iterate();
	}

	nodes[0].Element = (Cell &)cell;
	nodes[0].Score = 0;
	Set_Cell_Data_Spread(cell, 1);
	int node_count = 1;
	queue->Insert(nodes[0]);

	CellNode * node = queue->Extract_Min();

	DynamicVectorClass<Cell> * cells = new DynamicVectorClass<Cell>;
	cells->Set_Growth_Step(size + 2);

	while (size > processed && node != NULL) {
		Map[node->Element].ITType = IsometricTileTypeClass::PaveTile;
		cells->Add(node->Element);

		CellNode * newnode = &nodes[node_count];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell newcell = Adjacent_Cell(node->Element, dir);
			if (My_In_Radar(newcell)) {
				CellClass * newcellptr = &Map[newcell];
				if (newcellptr->Is_Tile_Clear() && newcellptr->Overlay == OVERLAY_NONE) {
					MapRegionClass::CellData & data = MapRegionClass::Get_Cell_Data(newcell);
					if (data.SpreadID != 1 && !data.Inviolate) {
						newnode->Element = newcell;
						newnode->Score = Get_Urban_Score(cell, newcell);
						Set_Cell_Data_Spread(newcell, 1);
						node_count++;
						queue->Insert(*newnode++);
					}
				}
			}
		}

		processed++;
		node = queue->Extract_Min();
	}

	delete[] nodes;
	delete queue;

	Rect bounds = Get_Cell_Bounding_Rect(*cells);

	if (processed <= 100 || (double)(bounds.Width * bounds.Height) >= (double)cells->Count() * 1.6) {
		for (int j = cells->Count() - 1; j >= 0; j--) {
			Map[(*cells)[j]].ITType = ISOTILE_NONE;
		}
		delete cells;
		return(NULL);
	}

	for (int cell_index = cells->Count() - 1; cell_index >= 0; cell_index--) {
		Cell c = (*cells)[cell_index];
		for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
			Cell newcell = Adjacent_Cell(c, dir);
			if (My_In_Radar(newcell) && Map[newcell].ITType != IsometricTileTypeClass::PaveTile) {
				cells->Delete_Index(cell_index);
			}
		}
	}

	for (int k = 0; k < cells->Count(); k++) {
		MapRegionClass::CellData::Set_Inviolate((*cells)[k]);
	}

	int patch_count = cells->Count() / 3;
	for (int i = 0; i < patch_count; i++) {
		Cell base = (*cells)[Pick_Random_UInt(0, cells->Count() - 1)];
		int tile_variant = Pick_Random_UInt(0, 6);

		bool place = true;
		if (tile_variant > 3) {
			for (int m = 0; m < 2; m++) {
				for (int n = 0; n < 2; n++) {
					Cell c(base.X + m, base.Y + n);
					if (!MapRegionClass::Get_Cell_Data(c).Inviolate || Map[c].ITType != IsometricTileTypeClass::PaveTile) {
						place = false;
					}
				}
			}
		}

		if (place) {
			Place_Tile((IsometricTileType)(IsometricTileTypeClass::MiscPaveTile + tile_variant), base, -1, -1);
		}
	}

	return(cells);
}


/// <summary>
/// Fetches the spread priority of a candidate urban cell.
/// Create_Urban_Area paves in cheapest first order, so this score is what gives a town its
/// roughly square plan and its ragged outskirts.
/// </summary>
/// <param name="cell1">The cell the area is growing out from.</param>
/// <param name="cell2">The candidate cell being considered.</param>
/// <returns>Returns with the score, where a lower score means the cell is paved sooner.</returns>
double MapGeneratorClass::Get_Urban_Score(const Cell & cell1, const Cell & cell2) const
{
	static const double _random_range = 3.0;

	int x = cell1.X - cell2.X;
	int y = cell1.Y - cell2.Y;
	double dist = std::max(abs(x), abs(y));
	return(RMG_RANDOM_DOUBLE(_random_range) + dist);
}


/// <summary>
/// Lays the road grid of an urban area.
/// This routine rules roads east to west and then north to south across the town, complete
/// with junctions, medians and end caps. It runs before the buildings go in, since
/// Generate_Urban_Buildings will only build on the pavement this leaves behind.
/// </summary>
/// <param name="cells">The cells of the urban area the road grid is fitted to.</param>
void MapGeneratorClass::Generate_Urban_Roads(const DynamicVectorClass<Cell> & cells)
{
	Rect bounds = Get_Cell_Bounding_Rect(cells);

	/// Horizontal east-west road bands
	if (bounds.Height > 5) {
		Cell start;
		start.Y = bounds.Y + 2;
		start.X = bounds.X;
		int remaining = bounds.Height - 2;

		while (remaining > 3) {

			Cell junction = Plan_Paved_Road_Junction(start, bounds, FACING_E);
			Cell end = Plan_Paved_Road(junction, bounds, FACING_E);

			if (end != Cell(0, 0) && start != Cell(0, 0) && (end.X - start.X) >= 6) {

				/// Left road cap / start piece
				if (Can_Place_Paved_Road(Rect(junction.X - 4, junction.Y, 4, 3), false, false) && Pick_Random_UInt(0, 1)) {
					Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 9), junction - Cell(3, 0));
				} else {
					Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 2), junction);
				}

				/// Fill straight horizontal road
				for (int x = 0; x < end.X - junction.X - 1; x++) {
					Place_Tile(IsometricTileTypeClass::PavedRoads, junction + Cell(x + 1, 0));
				}

				/// Right road cap / end piece
				if (Can_Place_Paved_Road(Rect(end.X, end.Y, 4, 3), false, false) && Pick_Random_UInt(0, 1)) {
					Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 10), end);
				} else {
					Place_Tile(IsometricTileTypeClass::PavedRoadEnds, end);
				}

				remaining -= 10;
				start.Y += 10;
			} else {
				start.Y += 2;
				remaining -= 2;
			}
		}
	}

	/// Vertical north-south road bands
	if (bounds.Width > 8) {

		int remaining = bounds.Height - 5;
		Cell start(bounds.X + 5, bounds.Y);

		while (remaining > 3) {

			Cell cursor = Plan_Paved_Road_Junction(start, bounds, FACING_S);
			Cell end = Plan_Paved_Road(cursor, bounds, FACING_S);

			if (end != Cell(0, 0) && start != Cell(0, 0) && (end.Y - start.Y) >= 6) {

				bool in_segment = false;
				bool initial = true;
				bool median_pending = false;
				bool median_flip = false;
				bool skip_tail = false;

				while (cursor.Y < end.Y) {

					if (!in_segment) {
						Rect road_rect;
						Rect junction_rect;

						road_rect.Height = 2;
						junction_rect.Width = 3;
						road_rect.Width = 3;
						road_rect.X = cursor.X;
						road_rect.Y = cursor.Y;
						junction_rect.X = cursor.X;
						junction_rect.Y = cursor.Y - 3;
						junction_rect.Height = 4;

						if ((end.Y - cursor.Y) < 2) {
							skip_tail = true;
							break;
						}

						if (initial && Can_Place_Paved_Road(Rect(cursor.X, cursor.Y - 3, 3, 4), false, false) && Pick_Random_UInt(0, 1)) {
							Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 13), cursor - Cell(0, 3));
							in_segment = true;
						} else if (Can_Place_Paved_Road_Junction(junction_rect)) {
							Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 5), cursor - Cell(0, 3));
							Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 1), cursor);
							in_segment = true;
						} else if (Can_Place_Paved_Road(road_rect, false, false)) {
							Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 1), cursor);
							in_segment = true;
						}
					} else {

						/// Straight vertical road segment
						if (Can_Place_Paved_Road(Rect(cursor.X, cursor.Y, 3, 1), false, false)) {

							Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 1), cursor);

							if (median_pending) {
								if (median_flip) {
									Place_Tile((IsometricTileType)(IsometricTileTypeClass::Medians + 5), cursor + Cell(1, 0));
									median_flip = false;
								} else {
									Place_Tile((IsometricTileType)(IsometricTileTypeClass::Medians + 4), cursor + Cell(1, 0));
								}
							}
						} else {

							Rect junction_rect(cursor.X, cursor.Y, 3, 4);

							if (median_pending) {
								Place_Tile((IsometricTileType)(IsometricTileTypeClass::Medians + 3), cursor + Cell(1, -1));
								median_pending = false;
							}

							if (Can_Place_Paved_Road_Junction(junction_rect)) {

								int bottom = end.Y;

								if ((bottom - cursor.Y) < 4) {
									Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 7), cursor);
									skip_tail = true;
									break;
								}

								if (Can_Place_Paved_Road(Rect(cursor.X, cursor.Y + 3, 3, 2), false, false)) {
									if ((bottom - cursor.Y) > 5) {
										median_pending = true;
										median_flip = true;
									}
								}

								Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 3), cursor);

								if (Can_Place_Paved_Road(Rect(cursor.X - 1, cursor.Y, 1, 3), true, false)) {
									Place_Tile((IsometricTileType)(IsometricTileTypeClass::Medians + 12), cursor - Cell(1, 0));
								}

								if (Can_Place_Paved_Road(Rect(cursor.X + 3, cursor.Y, 1, 3), true, false)) {
									Place_Tile((IsometricTileType)(IsometricTileTypeClass::Medians + 12), cursor + Cell(3, 0));
								}

								cursor.Y += 2;
							} else {

								Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 3), cursor - Cell(0, 1));

								CellClass * cellptr = &Map[cursor + Cell(1, -2)];

								if (cellptr->Is_Tile_Road_Median()) {
									while (cellptr->Is_Tile_Road_Median()) {
										cellptr->ITType = (IsometricTileType)(IsometricTileTypeClass::PavedRoads + 1);
										cellptr->SubTile = 1;
										cellptr = &cellptr->Adjacent_Cell(FACING_N);
									}
								}

								in_segment = false;
							}
						}
					}

					initial = false;
					cursor.Y++;
				}

				if (!skip_tail) {
					if (median_pending) {
						Place_Tile((IsometricTileType)(IsometricTileTypeClass::Medians + 3), cursor + Cell(1, -1));
					}

					/// Final vertical end cap
					if (Can_Place_Paved_Road(Rect(cursor.X, cursor.Y, 3, 4), false, false) && Pick_Random_UInt(0, 1)) {
						Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoads + 12), cursor);
					} else {
						Place_Tile((IsometricTileType)(IsometricTileTypeClass::PavedRoadEnds + 3), cursor);
					}
				}

				remaining -= 10;
				start.X += 10;
			} else {
				start.X += 2;
				remaining -= 2;
			}
		}
	}
}


/// <summary>
/// Places civilian buildings across an urban area.
/// This routine fills the pavement laid down by Generate_Urban_Roads with city blocks, giving
/// each building the worn look the biome calls for. Landmarks that should appear only once are
/// dropped from the pool as soon as they have been placed.
/// </summary>
/// <param name="cells">The cells of the urban area available to build on.</param>
void MapGeneratorClass::Generate_Urban_Buildings(const DynamicVectorClass<Cell> & cells)
{
	static char const * _names[] = {
		"CITY01",
		"CITY02",
		"CITY03",
		"CITY04",
		"CITY05",
		"CITY06",
		"CITY07",
		"CITY08",
		"CITY09",
		"CITY10",
		"CITY11",
		"CITY12",
		"CITY13",
		"CITY14",
		"CITY15",
		"CITY16",
		"CITY17",
		"CITY18",
		"CAHOSP"
	};

	int count = Pick_Random_UInt(cells.Count() / 20, cells.Count() / 10) / 2;
	int tries = 0;
	int placed = 0;

	DynamicVectorClass<char *> names;
	for (int i = 0; i < ARRAY_SIZE(_names); i++) {
		names.Add((char *&)_names[i]);
	}

	HouseClass * neutral = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));

	if (count > 0) {
		while (true) {
			if (tries >= 2 * count || names.Count() <= 1) {
				break;
			}
			int name_index = Pick_Random_UInt(0, names.Count() - 1);
			StructType bindex = BuildingTypeClass::From_Name(names[name_index]);
			BuildingClass * building = new BuildingClass(BuildingTypes[bindex], neutral);

			int tries_here = 0;
			bool was_placed = false;
			while (!was_placed) {
				if (tries_here >= 60) {
					break;
				}

				Cell building_cell = cells[Pick_Random_UInt(0, cells.Count() - 1)];

				Cell const * list = building->Class->Occupy_List(true);
				bool ok = true;
				while (*list != REFRESH_EOL) {
					CellClass * cellptr = &Map[building_cell + *list];
					if (!cellptr->Is_Tile_Pavement() && !cellptr->Is_Tile_Misc_Pavement() || cellptr->Cell_Occupier() != NULL || cellptr->Land_Type() == LAND_ROCK || !Map.In_Local_Radar(cellptr)) {
						ok = false;
					}
					list++;
					if (!ok) break;
				}

				if (ok) {
					if (building->Unlimbo(Coord(building_cell, 0))) {

						double condition_red_chance = ConditionRedChances[SeedData.Biome];
						double condition_yellow_chance = ConditionYellowChances[SeedData.Biome];

						double health_ratio = 1.0;
						if (Random_Fraction() < condition_red_chance) {
							health_ratio = Rule->ConditionRed;
						} else if (Random_Fraction() < condition_yellow_chance) {
							health_ratio = Rule->ConditionYellow;
						}
						building->Strength = building->Class->MaxStrength * health_ratio;

						building->Mark(MARK_CHANGE);

						was_placed = true;
						placed++;
						if (name_index > 17) {
							names.Delete_Index(name_index);
						}
					}
				}
				tries_here++;
			}

			if (!was_placed) {
				delete building;
			}

			tries++;
			if (placed >= count) {
				break;
			}
		}
	}
}


/// <summary>
/// Scatters civilian traffic through an urban area.
/// This routine populates a finished town with a handful of neutral pedestrians and vehicles.
/// How many turn up, and how battered they are, follows from the biome -- a mutated map keeps
/// few civilians and most of those are in poor shape.
/// </summary>
/// <param name="cells">The cells of the urban area the civilians may occupy.</param>
void MapGeneratorClass::Generate_Urban_Units(const DynamicVectorClass<Cell> & cells)
{
	static char const * _names[] = {
		"CIV1",
		"CIV2",
		"CIV3",
		"PICK",
		"WINI",
		"BUS",
		"CAR"
	};

	static int _max_counts[BIOME_COUNT] = {
		4, /// Tundra
		6, /// Taiga
		6, /// Temperate
		3, /// Desert
		2  /// Mutated
	};

	HouseClass * neutral = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));
	int count = Pick_Random_UInt(1, _max_counts[SeedData.Biome]);

	int tries = 0;
	int placed = 0;

	if (count > 0) {
		while (true) {
			if (tries >= 2 * count) {
				break;
			}

			int unit_index = Pick_Random_UInt(0, ARRAY_SIZE(_names) - 1);
			FootClass * foot = NULL;
			if (unit_index <= 2) {
				foot = new InfantryClass(InfantryTypes[InfantryTypeClass::From_Name(_names[unit_index])], neutral);
			} else {
				foot = new UnitClass(UnitTypes[UnitTypeClass::From_Name(_names[unit_index])], neutral);
			}

			int tries_here = 0;
			bool was_placed = false;
			while (!was_placed) {
				if (tries_here >= 60) {
					break;
				}
				Cell unit_cell = cells[Pick_Random_UInt(0, cells.Count() - 1)];
				if (Map.In_Local_Radar(unit_cell) && Map[unit_cell].Cell_Occupier() == NULL) {
					if (foot->Unlimbo(Coord(unit_cell), (Dir256)(Pick_Random_UInt(FACING_FIRST, FACING_COUNT - 1) * DIR_STEP_8))) {

						double condition_red_chance = ConditionRedChances[SeedData.Biome];
						double condition_yellow_chance = ConditionYellowChances[SeedData.Biome];

						double health_ratio = 1.0;
						if (Random_Fraction() < condition_red_chance) {
							health_ratio = Rule->ConditionRed;
						} else if (Random_Fraction() < condition_yellow_chance) {
							health_ratio = Rule->ConditionYellow;
						}
						foot->Strength = foot->TClass->MaxStrength * health_ratio;

						foot->Mark(MARK_CHANGE);
						was_placed = true;
						placed++;
					}
				}
				tries_here++;
			}

			if (!was_placed) {
				delete foot;
			}

			tries++;
			if (placed >= count) {
				break;
			}
		}
	}
}


/// <summary>
/// Determines if the ground around a cell is free of cliffs.
/// This routine is used when the generator is looking for somewhere to drop a settlement -- a
/// town wants a decent patch of open ground, not a cliff face through the middle of it.
/// </summary>
/// <param name="cell">The center of the area to examine.</param>
/// <returns>bool; Is the surrounding ground free of cliffs?</returns>
bool MapGeneratorClass::Is_Area_Cliff_Free(const Cell & cell)
{
	int yy = cell.Y;
	int xx = cell.X;

	int xmin = xx - 7;
	int ymin = yy - 7;
	int xmax = xx + 7;
	int ymax = yy + 7;

	for (int x = xmin; x < xmax; x++) {
		for (int y = ymin; y < ymax; y++) {
			if (Map[Cell(x, y)].Is_Tile_Cliff()) {
				return(false);
			}
		}
	}
	return(true);
}


/// <summary>
/// Stamps an isometric tile onto the map.
/// This routine covers the whole footprint of a multi-cell tile, so the generator can lay down
/// a cliff, a road piece or a block of pavement with one call. Any part of the footprint that
/// falls off the map is quietly skipped.
/// </summary>
/// <param name="ittype">The isometric tile type to stamp down.</param>
/// <param name="cell">The top left cell of the tile's footprint.</param>
/// <param name="region">The generator region the covered cells are credited to.</param>
/// <param name="height">The height to lay the tile at, or -1 to leave heights alone.</param>
void MapGeneratorClass::Place_Tile(IsometricTileType ittype, const Cell & cell, int region, int height)
{
	IsometricTileTypeClass * itype = IsometricTileTypes[ittype];
	IsoTileSet * set = (IsoTileSet *)itype->Get_Image_Data();
	if (set != NULL) {
		for (int y = 0; y < itype->Height; y++) {
			for (int x = 0; x < itype->Width; x++) {
				Cell ncell = cell + Cell(x, y);
				if (My_In_Radar(ncell)) {
					CellClass * cptr = &Map[ncell];
					int subtile = itype->SubTile_Index(x, y);
					IsoTileRecord const * record = set->Fetch_Record_Pointer_Unsafe(subtile);
					if (record != NULL) {
						cptr->ITType = itype->HeapID;
						cptr->SubTile = subtile;
						if (height != -1) {
							cptr->Height = height + record->Height - 4;
						}
						cptr->Ramp = record->RampType;
						MapRegionClass::CellData::Set_Region(ncell, region);
					}
				}
			}
		}
	}
}


/// <summary>
/// Finds the cell a paved road run should start from.
/// This routine walks outward until it reaches paved ground that will take a road, then backs
/// off by a random amount. Generate_Urban_Roads pairs it with Plan_Paved_Road to fix both ends
/// of every road in the grid.
/// </summary>
/// <param name="cell">The cell to start walking from.</param>
/// <param name="rect">The bounding rectangle of the urban area the road must stay inside.</param>
/// <param name="dir">The direction the road runs -- FACING_E or FACING_S.</param>
/// <returns>Returns with the junction cell found. Otherwise, Cell(0,0) is returned if the walk
/// left the area first.</returns>
Cell MapGeneratorClass::Plan_Paved_Road_Junction(Cell const & cell, Rect const & rect, FacingType dir)
{
	Cell wcell = cell;

	int margin_x = 0;
	int margin_y = 0;
	int margin_width = 0;
	int margin_height = 0;
	Rect tile_size(0, 0, 0, 0);

	bool south = dir == FACING_S;

	if (dir == FACING_E) {
		tile_size = Rect(0, 0, 1, 3);
		margin_y = -3;
		margin_height = 6;
	} else {
		tile_size = Rect(0, 0, 3, 1);
		margin_x = -3;
		margin_width = 6;
	}

	while (true) {
		if (!(wcell.X < rect.X + rect.Width && wcell.Y < rect.Y + rect.Height)) return(Cell(0, 0));
		if (Map[wcell].ITType == IsometricTileTypeClass::PaveTile) {
			break;
		}
		wcell = Adjacent_Cell(wcell, dir);
	}

	while (true) {
		if (!(wcell.X < rect.X + rect.Width && wcell.Y < rect.Y + rect.Height)) return(Cell(0, 0));
		if (Can_Place_Paved_Road(Rect(margin_x + wcell.X, margin_y + wcell.Y, margin_width + tile_size.Width, margin_height + tile_size.Height), south, south)) {
			break;
		}
		wcell = Adjacent_Cell(wcell, dir);
	}

	int length = std::max(wcell.Y - cell.Y, wcell.X - cell.X);
	if (length > 0) {
		int shorten = Pick_Random_UInt(0, length);
		while (shorten > 0) {
			Cell back = Adjacent_Cell(wcell, Facing_Sub(dir, FACING_180));
			if (!Can_Place_Paved_Road(Rect(margin_x + back.X, margin_y + back.Y, margin_width + tile_size.Width, margin_height + tile_size.Height), false, false)) break;
			wcell = back;
			shorten--;
		}
	}

	return(wcell);
}


/// <summary>
/// Finds the far end of a paved road run.
/// This routine walks outward from the junction cell for as long as road may legally be laid
/// and the run stays within the town, then backs off. A long road is trimmed by a random
/// amount so that the road grid does not look stamped out.
/// </summary>
/// <param name="cell">The cell the road run starts from.</param>
/// <param name="rect">The bounding rectangle of the urban area the road must stay inside.</param>
/// <param name="dir">The direction the road runs -- FACING_E or FACING_S.</param>
/// <returns>Returns with the cell the road run ends at.</returns>
Cell MapGeneratorClass::Plan_Paved_Road(Cell const & cell, Rect const & rect, FacingType dir)
{
	bool south = dir == FACING_S;
	Cell wcell = cell;

	Rect margins(0, 0, 0, 0);
	Rect tile_size(0, 0, 0, 0);

	if (dir == FACING_E) {
		tile_size = Rect(0, 0, 1, 3);
		margins.Y = -3;
		margins.Height = 6;
	} else {
		tile_size = Rect(0, 0, 3, 1);
		margins.X = -3;
		margins.Width = 6;
	}

	while (true) {
		if (!(wcell.X < rect.X + rect.Width + 1 && wcell.Y < rect.Y + rect.Height + 1)) break;
		if (!Can_Place_Paved_Road(Rect(margins.X + wcell.X, margins.Y + wcell.Y, margins.Width + tile_size.Width, margins.Height + tile_size.Height), south, south)) {
			break;
		}
		wcell = Adjacent_Cell(wcell, dir);
	}

	wcell = Adjacent_Cell(wcell, Facing_Sub(dir, FACING_180));

	int length = std::max(wcell.X - cell.X, wcell.Y - cell.Y);
	if (length > 6) {
		int shorten = Pick_Random_UInt(0, 3);
		while (shorten > 0) {
			Cell back = Adjacent_Cell(wcell, Facing_Sub(dir, FACING_180));
			shorten--;
			wcell = back;
		}
	}

	return(wcell);
}


/// <summary>
/// Determines if an area of the map is clear ground.
/// This routine is used before dropping down something that needs elbow room -- a mold seed,
/// or the ground beside a crystal cliff face. Any part of the area lying off the map counts
/// against it.
/// </summary>
/// <param name="rect">The area of cells to examine.</param>
/// <param name="check_data">Should cells the generator has protected be rejected too?</param>
/// <returns>bool; Is the whole area clear?</returns>
bool MapGeneratorClass::Is_Cell_Area_Clear(Rect const & rect, bool check_data)
{
	if (My_In_Radar(Cell(rect.X, rect.Y)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y)) &&
		My_In_Radar(Cell(rect.X, rect.Y + rect.Height - 1)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1,rect.Y + rect.Height - 1))) {
		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			for (int x = rect.X; x < rect.X + rect.Width; x++) {
				CellClass *cptr = &Map[Cell(x,y)];
				if (!cptr->Is_Tile_Clear()) {
					return(false);
				}
				if (check_data && !MapRegionClass::CellData::Is_Not_Inviolate(cptr->CellID)) {
					return(false);
				}
				if (cptr->Overlay != OVERLAY_NONE) {
					return(false);
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if a paved road may be laid over an area.
/// The urban road planner uses this routine both to walk a road out to its full length and to
/// decide whether a junction or a cap will fit, so the caller states which kinds of road tile
/// it is willing to build over.
/// </summary>
/// <param name="rect">The area of cells the road would cover.</param>
/// <param name="overlap_road">May road already laid be built over?</param>
/// <param name="overlap_road_end">May an end cap already laid be built over?</param>
/// <returns>bool; May the road be placed here?</returns>
bool MapGeneratorClass::Can_Place_Paved_Road(Rect const & rect, bool overlap_road, bool overlap_road_end)
{
	if (My_In_Radar(Cell(rect.X, rect.Y)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y)) &&
		My_In_Radar(Cell(rect.X, rect.Y + rect.Height - 1)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y + rect.Height - 1))) {

		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			for (int x = rect.X; x < rect.X + rect.Width; x++) {
				CellClass * cptr = &Map[Cell(x, y)];
				if (cptr->Is_Tile_Paved_Road()) {
					if (!overlap_road) {
						return(false);
					}
				} else if (cptr->Is_Tile_Paved_Road_End()) {
					if (!overlap_road_end) {
						return(false);
					}
				} else if (!cptr->Is_Tile_Clear() && !cptr->Is_Tile_Misc_Pavement() && !cptr->Is_Tile_Pavement()) {
					return(false);
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if a paved road end cap may be laid over an area.
/// This is the level ground variant of Can_Place_Paved_Road -- an end cap cannot straddle a
/// slope, so every cell of the area must sit at the same height.
/// </summary>
/// <param name="rect">The area of cells the end cap would cover.</param>
/// <param name="overlap_road">May road already laid be built over?</param>
/// <returns>bool; May the end cap be placed here?</returns>
bool MapGeneratorClass::Can_Place_Paved_Road_End(Rect const & rect, bool overlap_road)
{
	if (My_In_Radar(Cell(rect.X, rect.Y)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y)) &&
		My_In_Radar(Cell(rect.X, rect.Y + rect.Height - 1)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y + rect.Height - 1))) {

		int height = Map[Cell(rect.X, rect.Y)].Height;

		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			for (int x = rect.X; x < rect.X + rect.Width; x++) {
				CellClass * cptr = &Map[Cell(x, y)];
				if (cptr->Height != height) {
					return(false);
				}
				if (cptr->Is_Tile_Paved_Road() || cptr->Is_Tile_Paved_Road_End()) {
					if (!overlap_road) {
						return(false);
					}
				} else if (!cptr->Is_Tile_Clear() && !cptr->Is_Tile_Misc_Pavement() && !cptr->Is_Tile_Pavement()) {
					return(false);
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the rectangle that encloses a group of cells.
/// </summary>
/// <param name="cells">The cells to enclose.</param>
/// <returns>Returns with the smallest rectangle that holds every one of the cells.</returns>
Rect MapGeneratorClass::Get_Cell_Bounding_Rect(const DynamicVectorClass<Cell> & cells)
{
	Cell cmin;
	Cell cmax;
	cmin.X = 999;
	cmax.X = 0;
	cmin.Y = 999;
	cmax.Y = 0;
	for (int i = cells.Count() - 1; i >= 0; i--) {
		Cell cell = cells[i];
		cmin.X = std::min(cell.X, cmin.X);
		cmin.Y = std::min(cell.Y, cmin.Y);
		cmax.X = std::max(cell.X, cmax.X);
		cmax.Y = std::max(cell.Y, cmax.Y);
	}
	return(Rect(cmin.X, cmin.Y, (cmax.X - cmin.X) + 1, (cmax.Y - cmin.Y) + 1));
}


/// <summary>
/// Determines if a paved road junction will fit.
/// A junction has to grow out of road that is already there, so the ground it would take
/// over must be paved road or median, and the row it would spill onto must be free for it.
/// </summary>
/// <param name="rect">The area the junction would cover.</param>
/// <returns>bool; Will the junction fit?</returns>
bool MapGeneratorClass::Can_Place_Paved_Road_Junction(Rect & rect)
{
	if (My_In_Radar(Cell(rect.X, rect.Y)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1, rect.Y)) &&
		My_In_Radar(Cell(rect.X, rect.Y + rect.Height - 1)) &&
		My_In_Radar(Cell(rect.X + rect.Width - 1,rect.Y + rect.Height - 1))) {

		for (int y = rect.Y; y < rect.Y + 3; y++) {
			for (int x = rect.X; x < rect.X + 3; x++) {
				CellClass * cptr = &Map[Cell(x, y)];
				if ((cptr->ITType < IsometricTileTypeClass::PavedRoads || cptr->ITType > IsometricTileTypeClass::PavedRoads + 1) && !cptr->Is_Tile_Road_Median()) {
					return(false);
				}
			}
		}
		for (int x = rect.X; x < rect.X + 3; x++) {
			CellClass * cptr = &Map[Cell(x, rect.Y + 3)];
			if (!cptr->Is_Tile_Pavement() && !cptr->Is_Tile_Clear() && !cptr->Is_Tile_Misc_Pavement()) {
				return(false);
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Weathers a city's pavement with scattered patches.
/// This routine is used to break up the flat sameness of a large paved area. Patches of the
/// alternate pavement tiles are laid over it wherever one of the stock patch shapes will
/// fit, in proportion to how much pavement there is.
/// </summary>
/// <param name="cells">The paved cells that may serve as patch anchors.</param>
void MapGeneratorClass::Generate_Urban_Pavement(DynamicVectorClass<Cell> & cells)
{
	Rect sizes[5] = {
		Rect(0, 0, 6, 4),
		Rect(0, 0, 4, 6),
		Rect(0, 0, 4, 4),
		Rect(0, 0, 4, 2),
		Rect(0, 0, 2, 4)
	};

	int count = cells.Count() / 50;
	int placed = 0;

	for (int tries = 0; tries < 10 * count && placed < count; tries++) {
		Cell cell = cells[Pick_Random_UInt(0, cells.Count() - 1)];
		int size = 0;
		bool place = false;
		Rect area(cell.X, cell.Y, 0, 0);
		while (true) {
			if (size >= 5) {
				break;
			}
			area.Width = sizes[size].Width;
			area.Height = sizes[size].Height;
			if (Can_Place_Misc_Pavement(area)) {
				place = true;
			}
			size++;
			if (place) {
				break;
			}
		}
		if (place) {
			IsometricTileType tile = (IsometricTileType)(Pick_Random_UInt(0, 5) + IsometricTileTypeClass::MiscPaveTile + 8);
			for (int y = 0; y < area.Height / 2; y++) {
				for (int x = 0; x < area.Width / 2; x++) {
					Place_Tile(tile, Cell(area.X + 2 * x, area.Y + 2 * y));
				}
			}
			placed++;
		}
	}
}


/// <summary>
/// Determines if a patch of decorative pavement will fit.
/// The ground has to be paved already and clear of anything standing on it, since the patch
/// only changes how the pavement looks and not what may be built there.
/// </summary>
/// <param name="rect">The area the patch would cover.</param>
/// <returns>bool; Will the patch fit?</returns>
bool MapGeneratorClass::Can_Place_Misc_Pavement(Rect const & rect)
{
	if (My_In_Radar(Cell(rect.X, rect.Y)) &&
		My_In_Radar(Cell((rect.X - 1) + rect.Width, rect.Y)) &&
		My_In_Radar(Cell(rect.X, (rect.Y - 1) + rect.Height)) &&
		My_In_Radar(Cell((rect.X - 1) + rect.Width, (rect.Y - 1) + rect.Height))) {

		for (int y = rect.Y; y < rect.Y + rect.Height; y++) {
			for (int x = rect.X; x < rect.X + rect.Width; x++) {
				CellClass *cptr = &Map[Cell(x,y)];
				IsometricTileType itype = cptr->ITType;
				if (!cptr->Is_Tile_Pavement() && (itype < IsometricTileTypeClass::MiscPaveTile || itype > IsometricTileTypeClass::MiscPaveTile + 7)) {
					return(false);
				}
				if (cptr->Cell_Occupier() != NULL) {
					return(false);
				}
			}
		}
		return(true);
	}
	return(false);
}


/// <summary>
/// Determines if a tile will fit where it is wanted.
/// The generator asks this routine before laying anything down. The ground the tile would
/// cover must be clear and sloped the way the tile expects; the caller may also insist that
/// no other tile has spoken for it, and that the whole tile lands inside the map.
/// </summary>
/// <param name="ittype">The isometric tile type to test.</param>
/// <param name="cell">The cell the tile would be anchored at.</param>
/// <param name="collision">Should ground another tile has claimed be refused?</param>
/// <param name="in_map">Must the whole tile land inside the map?</param>
/// <returns>bool; Will the tile fit?</returns>
bool MapGeneratorClass::Can_Place_Tile(IsometricTileType ittype, Cell const & cell, bool collision, bool in_map)
{
	IsometricTileTypeClass * itype = IsometricTileTypes[ittype];
	IsoTileSet * set = (IsoTileSet *)itype->Get_Image_Data();
	if (set != NULL) {
		for (int y = 0; y < itype->Height; y++) {
			for (int x = 0; x < itype->Width; x++) {
				Cell ncell = cell + Cell(x, y);
				if (My_In_Radar(ncell)) {
					CellClass * cptr = &Map[ncell];
					int subtile = itype->SubTile_Index(x, y);
					IsoTileRecord const * record = set->Fetch_Record_Pointer_Unsafe(subtile);
					if (record != NULL) {
						if ((!cptr->Is_Tile_Clear() && !cptr->Is_Tile_Ramp()) || cptr->Ramp != record->RampType || collision && MapRegionClass::Get_Cell_Data(ncell).TilePlaced) {
							return(false);
						}
					}
				} else if (in_map) {
					return(false);
				}
			}
		}
	}
	return(true);
}


/// <summary>
/// Takes a tile back off the map.
/// This routine undoes a placement the generator has thought better of, leaving the ground
/// it covered blank for something else to claim.
/// </summary>
/// <param name="ittype">The isometric tile type to remove.</param>
/// <param name="cell">The cell the tile is anchored at.</param>
void MapGeneratorClass::Remove_Tile(IsometricTileType ittype, Cell const & cell)
{
	IsometricTileTypeClass * itype = IsometricTileTypes[ittype];
	IsoTileSet * set = (IsoTileSet *)itype->Get_Image_Data();
	if (set != NULL) {
		for (int y = 0; y < itype->Height; y++) {
			for (int x = 0; x < itype->Width; x++) {
				Cell ncell = cell + Cell(x, y);
				if (My_In_Radar(ncell)) {
					CellClass * cptr = &Map[ncell];
					int subtile = itype->SubTile_Index(x, y);
					IsoTileRecord const * record = set->Fetch_Record_Pointer_Unsafe(subtile);
					if (record != NULL) {
						cptr->ITType = TILE_NONE;
						cptr->SubTile = 0;
					}
				}
			}
		}
	}
}


/// <summary>
/// Reserves or gives back the ground a tile would cover.
/// The dirt road builder uses this routine to stop two road pieces laying claim to the same
/// ground while it is still deciding, and to release that claim when a stretch of road is
/// abandoned.
/// </summary>
/// <param name="ittype">The isometric tile type whose footprint is affected.</param>
/// <param name="cell">The cell the tile is anchored at.</param>
/// <param name="placed">Should the ground be reserved?</param>
void MapGeneratorClass::Mark_Tile(IsometricTileType ittype, Cell const & cell, bool placed)
{
	IsometricTileTypeClass * itype = IsometricTileTypes[ittype];
	IsoTileSet * set = (IsoTileSet *)itype->Get_Image_Data();
	if (set != NULL) {
		for (int y = 0; y < itype->Height; y++) {
			for (int x = 0; x < itype->Width; x++) {
				Cell ncell = cell + Cell(x, y);
				if (My_In_Radar(ncell)) {
					int subtile = itype->SubTile_Index(x, y);
					IsoTileRecord const * record = set->Fetch_Record_Pointer_Unsafe(subtile);
					if (record != NULL) {
						MapRegionClass::Get_Cell_Data(ncell).TilePlaced = placed;
					}
				}
			}
		}
	}
}


/// <summary>
/// Plants the rural settlements the map calls for.
/// This routine finds open ground with room enough for a road junction and then hands it to
/// the road, building and unit generators in turn, so that every settlement ends up with a
/// road network and something standing along it.
/// </summary>
void MapGeneratorClass::Generate_Rural_Areas(void)
{
	Init_Dirt_Roads();

	int count = (SeedData.Cities + 0.49) * 2;
	int tries = 0;
	int placed = 0;

	while (true) {
		if (placed >= count) {
			break;
		}
		Cell cell;
		cell.X = 0;
		cell.Y = 0;
		int tries_here = 0;
		while (true) {
			cell = MapRegionClass::Pick_Random_Clear_Map_Cell();
			if (++tries_here >= 20) break;
			if (MapRegionClass::CellData::Is_Not_Inviolate(cell) && Can_Place_Paved_Road(Rect(cell.X - 4, cell.Y - 4, 8, 8), false, false) && Map.In_Local_Radar(cell)) {
				break;
			}
		}

		if (tries_here != 20) {
			if (Generate_Rural_Roads(cell)) {
				Generate_Rural_Buildings(cell);
				Generate_Rural_Units(cell);
				placed++;
			}
		}

		tries++;
		if (tries >= 10) {
			break;
		}
	}
}


/// <summary>
/// Scatters civilian structures about a rural settlement.
/// This routine drops a handful of neutral buildings on the flat ground near the center,
/// taking each one out of the pool as it is used so the settlement is not built from the
/// same house over and over. The desert, temperate and mutated biomes draw on the civilian
/// structures alone and leave the abandoned ones out.
/// </summary>
/// <param name="cell">The center of the settlement to build around.</param>
void MapGeneratorClass::Generate_Rural_Buildings(const Cell & cell)
{
	static char const * _names[] = {
		"ABAN01",
		"ABAN02",
		"ABAN03",
		"ABAN04",
		"ABAN05",
		"ABAN06",
		"ABAN07",
		"ABAN08",
		"ABAN09",
		"ABAN10",
		"ABAN11",
		"ABAN12",
		"ABAN13",
		"ABAN14",
		"ABAN15",
		"CA0001",
		"CA0002",
		"CA0003",
		"CA0004",
		"CA0005",
		"CA0006",
		"CA0007",
		"CA0008",
		"CA0009",
		"CA0010",
		"CA0011",
		"CA0012",
		"CA0013",
		"CA0014",
		"CA0015",
		"CA0016",
		"CA0017",
		"CA0018",
		"CA0019",
		"CA0020",
		"CA0021"
	};

	int count = Pick_Random_UInt(8, 12);
	int tries = 0;
	int placed = 0;

	int height = Map[cell].Height;

	int min_building = 0;
	int max_building = ARRAY_SIZE(_names) - 1;
	if (SeedData.Biome == BIOME_DESERT || SeedData.Biome == BIOME_TEMPERATE || SeedData.Biome == BIOME_MUTATED) {
		min_building = 15;
	}

	DynamicVectorClass<char *> names;
	for (int i = 0; i < ARRAY_SIZE(_names); i++) {
		names.Add((char *&)_names[i]);
	}

	HouseClass * neutral = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));

	if (count > 0) {
		while (true) {
			if (tries >= 2 * count) {
				break;
			}
			int name_index = Pick_Random_UInt(min_building, max_building);
			StructType bindex = BuildingTypeClass::From_Name(names[name_index]);
			if (bindex != STRUCT_NONE) {
				BuildingClass * building = new BuildingClass(BuildingTypes[bindex], neutral);

				int tries_here = 0;
				bool was_placed = false;
				while (!was_placed) {
					if (tries_here >= 60) {
						break;
					}

					Cell building_cell = cell + Cell(Pick_Random_UInt(0, 14) - 7, Pick_Random_UInt(0, 14) - 7);

					Cell const * list = building->Class->Occupy_List(true);
					bool ok = true;
					while (*list != REFRESH_EOL) {
						CellClass * cellptr = &Map[building_cell + *list];
						if (cellptr->Cell_Occupier() != NULL || !cellptr->Is_Tile_Clear() || cellptr->Height != height || cellptr->Land_Type() == LAND_ROCK || !Map.In_Local_Radar(cellptr)) {
							ok = false;
						}
						list++;
						if (!ok) break;
					}

					if (ok) {
						if (building->Unlimbo(Coord(building_cell, 0))) {
							was_placed = true;
							placed++;
							names.Delete_Index(name_index);
							max_building--;
						}
					}
					tries_here++;
				}

				if (!was_placed) {
					delete building;
				}

				tries++;
				if (placed >= count) {
					break;
				}
			}
		}
	}
}


/// <summary>
/// Scatters civilians and their vehicles about a rural settlement.
/// This routine gives the settlement some neutral life of its own. The harsher the biome,
/// the likelier each of them is to turn up already the worse for wear.
/// </summary>
/// <param name="cell">The center of the settlement to populate.</param>
void MapGeneratorClass::Generate_Rural_Units(const Cell & cell)
{
	static char const * _names[] = {
		"CIV1",
		"CIV2",
		"CIV3",
		"PICK",
		"WINI",
		"CAR"
	};

	static int _max_counts[BIOME_COUNT] = {
		4, /// Tundra
		6, /// Taiga
		6, /// Temperate
		3, /// Desert
		1  /// Mutated
	};

	HouseClass * neutral = House_From_HousesType(HouseTypeClass::From_Name("Neutral"));
	int count = Pick_Random_UInt(1, _max_counts[SeedData.Biome]);

	int tries = 0;
	int placed = 0;

	while (placed < count) {
		if (tries >= 2 * count) {
			break;
		}

		int name_index = Pick_Random_UInt(0, ARRAY_SIZE(_names) - 1);
		FootClass * foot = NULL;
		if (name_index <= 2) {
			foot = new InfantryClass(InfantryTypes[InfantryTypeClass::From_Name(_names[name_index])], neutral);
		} else {
			foot = new UnitClass(UnitTypes[UnitTypeClass::From_Name(_names[name_index])], neutral);
		}

		int tries_here = 0;
		bool was_placed = false;
		while (!was_placed) {
			if (tries_here >= 60) {
				break;
			}
			Cell unit_cell = cell + Cell(Pick_Random_UInt(0, 16) - 8, Pick_Random_UInt(0, 16) - 8);
			if (Map.In_Local_Radar(unit_cell) && Map[unit_cell].Cell_Occupier() == NULL) {
				if (foot->Unlimbo(Coord(unit_cell), (Dir256)(Pick_Random_UInt(FACING_FIRST, FACING_COUNT - 1) * DIR_STEP_8))) {

					double condition_red_chance = ConditionRedChances[SeedData.Biome];
					double condition_yellow_chance = ConditionYellowChances[SeedData.Biome];

					double health_ratio = 1.0;
					if (Random_Fraction() < condition_red_chance) {
						health_ratio = Rule->ConditionRed;
					} else if (Random_Fraction() < condition_yellow_chance) {
						health_ratio = Rule->ConditionYellow;
					}
					foot->Strength = foot->TClass->MaxStrength * health_ratio;

					foot->Mark(MARK_CHANGE);
					was_placed = true;
					placed++;
				}
			}
			tries_here++;
		}

		if (!was_placed) {
			delete foot;
		}

		tries++;
	}
}


/// <summary>
/// Lays out the dirt roads of a rural settlement.
/// This routine starts from a junction at the cell given and grows the road outward until
/// it runs out of room. Every loose end is sealed with a cap piece, and a stretch that
/// cannot be carried onward is taken back off the map and capped instead, so the network
/// never trails off into open ground.
/// </summary>
/// <param name="cell">The cell the road network should start from.</param>
/// <returns>bool; Was anything more than a lone junction laid down?</returns>
bool MapGeneratorClass::Generate_Rural_Roads(const Cell & cell)
{
	std::deque<DirtRoadNode> queue;

	int processed = 0;

	/*
	 * Pick a junction tile that fits at the starting cell. All eleven junction
	 * shapes are tried once, beginning at a random shape so that the roads
	 * don't always start out looking the same.
	 */
	int attempt = 0;
	unsigned int random_offset = Pick_Random_UInt(0, 10);
	IsometricTileType junction;
	while (true) {
		junction = (IsometricTileType)(IsometricTileTypeClass::DirtRoadJunction + (int)(random_offset + attempt) % DIRT_ROAD_JUNCTION_COUNT);
		if (Can_Place_Tile(junction, cell, false, false)) {
			break;
		}
		if (++attempt >= 11) {
			return(false);
		}
	}

	/*
	 * Seed the work queue with the junction tile. It has no cap tile and no
	 * back link, since there is no road leading into it yet.
	 */
	DirtRoadNode root(junction, cell, ISOTILE_NONE, cell, -1);
	queue.push_back(root);

	/*
	 * Grow the road network breadth-first until the queue runs dry or the
	 * pending road count reaches its limit.
	 */
	while ((int)queue.size() < 1020) {
		bool success = true;
		DirtRoadNode node = queue.front();

		int child_count = 0;
		DirtRoadNode * children = NULL;

		if (node.Tile != ISOTILE_NONE) {

			/*
			 * Commit this tile to the map. The cap tile that provisionally
			 * terminated the road here is no longer needed.
			 */
			if (node.CapTile != ISOTILE_NONE) {
				Mark_Tile(node.CapTile, node.CapOrigin, false);
			}
			Place_Tile(node.Tile, node.Origin);

			DirtRoadTile & tile = DirtRoadTiles[Dirt_Road_Tile_Index(node.Tile)];

			/*
			 * Extend the road out of every link of this tile except the one
			 * that leads back the way it came.
			 */
			if (tile.Count > 1) {
				int link_count = tile.Count - (node.BackLinkIndex != -1);
				children = new DirtRoadNode[link_count];
				if (children != NULL) {
					DirtRoadNode * child = children;
					for (int link_index = 0; link_index < tile.Count; link_index++) {
						if (link_index != node.BackLinkIndex) {
							DirtRoadLink link = tile.Links[link_index];
							FacingType reverse = Facing_Sub(link.Facing, FACING_180);

							/*
							 * Find a road piece to continue with at the cell this
							 * link connects to, along with a cap piece in case the
							 * continuation is later rolled back. Failing to connect
							 * dooms the whole tile.
							 */
							Cell connection = Adjacent_Cell(node.Origin + link.Offset, link.Facing);
							int cap_link = 0;
							if (Find_Dirt_Road_Tile(reverse, connection, 1, child->CapTile, child->CapOrigin, cap_link)) {
								if (!Find_Dirt_Road_Tile(reverse, connection, 4, child->Tile, child->Origin, child->BackLinkIndex)) {
									child->BackLinkIndex = cap_link;
								} else {
									Mark_Tile(child->Tile, child->Origin, true);
								}
								Mark_Tile(child->CapTile, child->CapOrigin, true);
								child_count++;
								child++;
							} else {
								success = false;
								break;
							}
						}
					}
				}
			}
		}

		if (success && node.Tile != ISOTILE_NONE) {

			/*
			**	Every link found a continuation, so queue the new road pieces up
			**	for processing of their own links.
			*/
			if (children != NULL) {
				DirtRoadNode * src = children;
				for (int j = 0; j < child_count; j++) {
					queue.push_back(*src);
					src++;
				}
			}
		} else {

			/*
			 * One of the links could not be connected. Take the tile back off
			 * the map, unmark whatever continuations were already found, and
			 * terminate the incoming road with the cap tile instead.
			 */
			if (node.Tile != ISOTILE_NONE) {
				Remove_Tile(node.Tile, node.Origin);
				DirtRoadNode * undo = children;
				while (child_count > 0) {
					if (undo->Tile != ISOTILE_NONE) {
						Mark_Tile(undo->Tile, undo->Origin, false);
					}
					if (undo->CapTile != ISOTILE_NONE) {
						Mark_Tile(undo->CapTile, undo->CapOrigin, false);
					}
					undo++;
					--child_count;
				}
			}
			if (node.CapTile != ISOTILE_NONE) {
				Place_Tile(node.CapTile, node.CapOrigin);
			}
		}

		if (children != NULL) {
			delete[] children;
		}

		queue.pop_front();
		processed++;
		if (queue.empty()) {
			return(processed > 1);
		}
	}

	/*
	 * The pending road count reached its limit before the network was
	 * finished. Every pending road gets sealed off with its cap tile.
	 */
	while (!queue.empty()) {
		DirtRoadNode node = queue.front();
		Place_Tile(node.CapTile, node.CapOrigin);
		queue.pop_front();
	}
	return(processed > 1);
}

// --- FACING 0 (NORTH) ---
static int Road_N_1Way[] = {95, 96, 97};
static int Road_N_2Way[] = {0, 1, 2, 3, 4, 5, 11, 84, 85, 86, 87, 88, 89, 90, 91, 98, 99};
static int Road_N_3Way[] = {24, 26, 27};
static int Road_N_4Way[] = {28};

// --- FACING 1 (NORTHEAST) ---
static int Road_NE_1Way[] = {45, 46, 47};
static int Road_NE_2Way[] = {0, 6, 7, 8, 9, 10, 11, 35, 36, 37, 38, 39, 40, 41, 48, 49};
static int Road_NE_3Way[] = {29, 31, 32, 34};
static int Road_NE_4Way[] = {33};

// --- FACING 2 (EAST) ---
static int Road_E_1Way[] = {62, 63, 64};
static int Road_E_2Way[] = {1, 6, 12, 13, 14, 15, 51, 52, 53, 54, 55, 56, 57, 58, 65, 66};
static int Road_E_3Way[] = {24, 25, 27, 34};
static int Road_E_4Way[] = {28};

// --- FACING 3 (SOUTHEAST) ---
static int Road_SE_1Way[] = {78, 79, 80};
static int Road_SE_2Way[] = {2, 7, 12, 16, 17, 18, 68, 69, 70, 71, 72, 73, 74, 81, 82};
static int Road_SE_3Way[] = {29, 30, 32};
static int Road_SE_4Way[] = {33};

// --- FACING 4 (SOUTH) ---
static int Road_S_1Way[] = {92, 93, 94};
static int Road_S_2Way[] = {8, 13, 16, 19, 20, 21, 84, 85, 86, 87, 88, 89, 90, 91, 98, 99};
static int Road_S_3Way[] = {24, 25, 26};
static int Road_S_4Way[] = {28};

// --- FACING 5 (SOUTHWEST) ---
static int Road_SW_1Way[] = {42, 43, 44};
static int Road_SW_2Way[] = {3, 14, 17, 19, 22, 35, 36, 37, 38, 39, 40, 41, 48, 49};
static int Road_SW_3Way[] = {29, 30, 31};
static int Road_SW_4Way[] = {33};

// --- FACING 6 (WEST) ---
static int Road_W_1Way[] = {59, 60, 61};
static int Road_W_2Way[] = {4, 9, 18, 20, 23, 51, 52, 53, 54, 55, 56, 57, 58, 65, 66};
static int Road_W_3Way[] = {25, 26, 27, 34};
static int Road_W_4Way[] = {28};

// --- FACING 7 (NORTHWEST) ---
static int Road_NW_1Way[] = {75, 76, 77};
static int Road_NW_2Way[] = {5, 10, 15, 21, 22, 23, 68, 69, 70, 71, 72, 73, 74, 81, 82};
static int Road_NW_3Way[] = {30, 31, 32};
static int Road_NW_4Way[] = {33};

/*
 * --- THE MASTER LOOKUP TABLE ---
 * Organizes the sub-arrays into FACING_COUNT arrays
 */
static int * RoadTileLibrary[FACING_COUNT][4] = {
	{ Road_N_1Way,  Road_N_2Way,  Road_N_3Way,  Road_N_4Way  }, // North
	{ Road_NE_1Way, Road_NE_2Way, Road_NE_3Way, Road_NE_4Way }, // North-East
	{ Road_E_1Way,  Road_E_2Way,  Road_E_3Way,  Road_E_4Way  }, // East
	{ Road_SE_1Way, Road_SE_2Way, Road_SE_3Way, Road_SE_4Way }, // South-East
	{ Road_S_1Way,  Road_S_2Way,  Road_S_3Way,  Road_S_4Way  }, // South
	{ Road_SW_1Way, Road_SW_2Way, Road_SW_3Way, Road_SW_4Way }, // South-West
	{ Road_W_1Way,  Road_W_2Way,  Road_W_3Way,  Road_W_4Way  }, // West
	{ Road_NW_1Way, Road_NW_2Way, Road_NW_3Way, Road_NW_4Way }  // North-West
};

/*
 * --- THE COUNT TABLE ---
 * Stores the number of tile variations available for each direction/complexity pair
 */
static int RoadTileCounts[FACING_COUNT][4] = {
	{ ARRAY_SIZE(Road_N_1Way),  ARRAY_SIZE(Road_N_2Way),  ARRAY_SIZE(Road_N_3Way),  ARRAY_SIZE(Road_N_4Way)  }, // N
	{ ARRAY_SIZE(Road_NE_1Way), ARRAY_SIZE(Road_NE_2Way), ARRAY_SIZE(Road_NE_3Way), ARRAY_SIZE(Road_NE_4Way) }, // NE
	{ ARRAY_SIZE(Road_E_1Way),  ARRAY_SIZE(Road_E_2Way),  ARRAY_SIZE(Road_E_3Way),  ARRAY_SIZE(Road_E_4Way)  }, // E
	{ ARRAY_SIZE(Road_SE_1Way), ARRAY_SIZE(Road_SE_2Way), ARRAY_SIZE(Road_SE_3Way), ARRAY_SIZE(Road_SE_4Way) }, // SE
	{ ARRAY_SIZE(Road_S_1Way),  ARRAY_SIZE(Road_S_2Way),  ARRAY_SIZE(Road_S_3Way),  ARRAY_SIZE(Road_S_4Way)  }, // S
	{ ARRAY_SIZE(Road_SW_1Way), ARRAY_SIZE(Road_SW_2Way), ARRAY_SIZE(Road_SW_3Way), ARRAY_SIZE(Road_SW_4Way) }, // SW
	{ ARRAY_SIZE(Road_W_1Way),  ARRAY_SIZE(Road_W_2Way),  ARRAY_SIZE(Road_W_3Way),  ARRAY_SIZE(Road_W_4Way)  }, // W
	{ ARRAY_SIZE(Road_NW_1Way), ARRAY_SIZE(Road_NW_2Way), ARRAY_SIZE(Road_NW_3Way), ARRAY_SIZE(Road_NW_4Way) }  // NW
};


/// <summary>
/// Finds a dirt road piece that connects the way the road needs to go.
/// This routine is used by the rural road builder to carry a road onward. It hunts the road
/// library for a shape that offers a connection facing the right way and that will fit on
/// the ground where that connection would land. How many ways the piece branches is left
/// to chance within the caller's limit, so junctions turn up of their own accord.
/// </summary>
/// <param name="facing">The direction the piece must connect in.</param>
/// <param name="target_cell">The cell the matching connection must land on.</param>
/// <param name="max_connections">The most connections the chosen piece may have.</param>
/// <param name="out_ittype">Filled in with the road piece chosen.</param>
/// <param name="out_origin">Filled in with the cell to anchor the piece at.</param>
/// <param name="out_link_index">Filled in with the connection that matched.</param>
/// <returns>bool; Was a road piece found that fits? The outputs are cleared when none
/// does.</returns>
bool MapGeneratorClass::Find_Dirt_Road_Tile(FacingType facing, Cell const & target_cell, int max_connections, IsometricTileType & out_ittype, Cell & out_origin, int & out_link_index)
{
	int current_conn_count;
	int initial_conn_count;

	initial_conn_count = current_conn_count = 1;

	if (max_connections > 1) {
		double roll = Sample_Truncated_Normal(2.0, 0.5, 2.0, max_connections);
		initial_conn_count = current_conn_count = roll;
	}

	while (true) {
		int tile_count = RoadTileCounts[facing][current_conn_count - 1];
		int * tile_list = RoadTileLibrary[facing][current_conn_count - 1];

		int start_index;
		if (current_conn_count == 2 && Random_Fraction() < 0.7) {
			start_index = Pick_Random_UInt(7, tile_count - 1);
		} else {
			start_index = Pick_Random_UInt(0, tile_count - 1);
		}

		for (int i = 0; i < tile_count; i++) {
			DirtRoadTile & tile = DirtRoadTiles[tile_list[(i + start_index) % tile_count]];
			int link_index;
			for (link_index = 0; link_index < tile.Count; link_index++) {
				if (tile.Links[link_index].Facing == facing) {
					break;
				}
			}
			DirtRoadLink link;
			link.Offset = tile.Links[link_index].Offset;
			Cell origin = target_cell - link.Offset;
			if (Can_Place_Tile(tile.ITType, origin, 1, 1)) {
				DirtRoadLink link2;
				link2.Offset = tile.Links[link_index].Offset;
				out_origin = target_cell - link2.Offset;
				out_ittype = tile.ITType;
				out_link_index = link_index;
				return(true);
			}
		}

		current_conn_count = (current_conn_count + 1) % (max_connections + 1);
		if (current_conn_count == 0) {
			current_conn_count = 1;
		}

		if (current_conn_count == initial_conn_count) {
			out_ittype = ISOTILE_NONE;
			out_origin = Cell(0, 0);
			out_link_index = -1;
			return(false);
		}
	}
}


/// <summary>
/// Blends the edges of the ice laid down on the map.
/// This routine runs once the ice is in place, so that every sheet of it meets the ground
/// around it with the proper transition tiles rather than a hard edge.
/// </summary>
void MapGeneratorClass::Smooth_Ice(void)
{
	int frst = IsometricTileTypeClass::Ice1Set;
	int last = IsometricTileTypeClass::Ice3Set + ICE_COUNT-1;
	Map.Reset_Iterator();
	CellClass * cptr = Map.Iterate();
	while (cptr != NULL) {
		int ittype = cptr->ITType;
		if (ittype >= frst && ittype <= last) {
			Cell cell = cptr->CellID;
			Map.Smoothen_Ice(cell, true);
		}
		cptr = Map.Iterate();
	}
}


/// <summary>
/// Rings each player's start position with floodlights.
/// This routine is used to light the starting positions on maps set at dusk or at night.
/// The ring is turned to whatever angle clears the terrain, and the lights are tagged so
/// that they can be switched with the time of day when the map uses transitions.
/// </summary>
void MapGeneratorClass::Generate_Lights(void)
{
	TagTypeClass * onoff = NULL;
	if (SeedData.UseTransitions) {
		onoff = TagTypeClass::From_Name("Light On/Off");
	}

	StructType glite = BuildingTypeClass::From_Name("GALITE");
	if (glite != STRUCT_NONE) {
		HouseClass * hptr = House_From_HousesType(HouseTypeClass::From_Name("Special"));
		if (hptr != NULL) {
			bool old = Debug_Map;
			Debug_Map = true;
			int i;
			for (i = 0; i < SeedData.NumPlayers; i++) {
				Cell waypoint = Scen->Get_Waypoint_Cell(i);
				int tries = 20;

				static const int _light_counts[] = {0, 0, 2, 4};
				int light_count = _light_counts[SeedData.Time];
				if (SeedData.UseTransitions) {
					light_count = 4;
				}

				bool can_place = false;
				int j;
				while (tries >= 0 && !can_place) {
					double angle = Random_Fraction() * DEG_TO_RAD(360);
					can_place = true;

					for (j = 0; j < light_count; j++) {
						Cell place_cell = Move_Coord(Coord(waypoint), j * DEG_TO_RAD(360) / light_count + angle, 8 * CELL_LEPTON);
						if (!BuildingTypes[glite]->Legal_Placement(place_cell, hptr) || !Map.In_Radar(place_cell)) {
							can_place = false;
							break;
						}
					}

					if (can_place) {
						for (j = 0; j < light_count; j++) {
							Cell place_cell = Move_Coord(Coord(waypoint), j * DEG_TO_RAD(360) / light_count + angle, 8 * CELL_LEPTON);
							BuildingClass * light = new BuildingClass(BuildingTypes[glite], hptr);
							if (!light->Unlimbo(Coord(place_cell))) {
								delete light;
							} else {
								if (light != NULL && SeedData.UseTransitions) {
									light->Attach_Tag(Find_Or_Make(onoff));
								}
							}

						}
					}

					tries--;
				}
			}
			Debug_Map = old;
		}
	}
}


/// <summary>
/// Plants the veinhole monsters the map calls for.
/// This routine looks for flat open ground with room for a monster and the patch of veins
/// it starts out with. A crowded map simply gets fewer monsters rather than holding the
/// generator up.
/// </summary>
void MapGeneratorClass::Generate_Veinholes(void)
{
	int tries = 0;
	int placed = 0;
	int count = SeedData.VeinholeMonsters;

	int save_init = ScenarioInit;
	ScenarioInit = 0;

	while (tries < 200 && placed < count) {

		Cell cell = MapRegionClass::Pick_Random_Clear_Map_Cell();

		Rect area(cell.X - 2, cell.Y - 2, 5, 5);

		if (Map.In_Local_Radar(Cell(area.X - 1, area.Y - 1))) {
			if (Map.In_Local_Radar(Cell(area.X + area.Width, area.Y - 1))) {
				if (Map.In_Local_Radar(Cell(area.X - 1, area.Y + area.Height))) {
					if (Map.In_Local_Radar(Cell(area.X + area.Width, area.Y + area.Height))) {
						bool valid = true;
						int height = Map[cell].Height;
						for (int y = area.Y; y < area.Y + area.Height; y++) {
							for (int x = area.X; x < area.X + area.Width; x++) {
								CellClass * cptr = &Map[Cell(x, y)];
								if (!MapRegionClass::CellData::Is_Not_Inviolate(cptr->CellID)) {
									valid = false;
								}
								if (!cptr->Is_Tile_Clear() || cptr->Height != height || cptr->Overlay != OVERLAY_NONE) {
									valid = false;
								}
							}
						}

						if (valid && VeinholeMonsterClass::Can_Monster_Go_Here(cell) && valid) {
							CellClass * cellptr = &Map[cell];
							for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
								CellClass * adjacent = &cellptr->Adjacent_Cell(dir);
								adjacent->Overlay = OVERLAY_VEINHOLE_DUMMY;
								adjacent->OverlayData = 0;
							}
							cellptr->Overlay = OVERLAY_VEINHOLE;
							cellptr->OverlayData = 0;

							new VeinholeMonsterClass(cellptr->CellID);
							for (int y = -2; y <= 2; y++) {
								for (int x = -2; x <= 2; x++) {
									CellClass * vein_cell = &Map[cell + Cell(x, y)];
									if (abs(y) == 2 || abs(x) == 2) {
										vein_cell->Place_Veins();
									}
									vein_cell->Recalc_Attributes();
								}
							}

							placed++;
						}
					}
				}
			}
		}

		tries++;
	}

	ScenarioInit = save_init;
}


/// <summary>
/// Constructs a DirtRoadLink from a cell offset and a facing direction.
/// </summary>
/// <param name="cell">Cell offset stored in Offset.</param>
/// <param name="dir">Facing direction stored in Facing.</param>
DirtRoadLink::DirtRoadLink(Cell const & cell, FacingType dir) :
	Offset(cell),
	Facing(dir)
{

}


/// <summary>
/// Default constructor; sets Offset to (0,0) and Facing to FACING_NONE.
/// </summary>
DirtRoadLink::DirtRoadLink(void) :
	Offset(0, 0),
	Facing(FACING_NONE)
{

}


/// <summary>
/// Copy constructor; copies Offset and Facing from another DirtRoadLink.
/// </summary>
/// <param name="that">Source link to copy.</param>
DirtRoadLink::DirtRoadLink(DirtRoadLink const & that) :
	Offset(that.Offset),
	Facing(that.Facing)
{

}


/// <summary>
/// Constructs a pending stretch of dirt road.
/// A node pairs the road piece to be laid with the cap piece that seals it off should the
/// road get no further, and remembers the way the road came in so the builder does not
/// double back on itself. The rural road generator keeps these in its work queue.
/// </summary>
/// <param name="road_tile">The road piece to lay.</param>
/// <param name="origin">The cell the road piece is anchored at.</param>
/// <param name="cap_tile">The cap piece that seals this stretch off.</param>
/// <param name="cap_origin">The cell the cap piece is anchored at.</param>
/// <param name="link_index">The connection the road arrived through, or -1 for the first
/// piece.</param>
DirtRoadNode::DirtRoadNode(IsometricTileType road_tile, Cell const & origin, IsometricTileType cap_tile, Cell const & cap_origin, int link_index) :
	Tile(road_tile),
	Origin(origin),
	CapTile(cap_tile),
	CapOrigin(cap_origin),
	BackLinkIndex(link_index)
{

}


/// <summary>
/// Constructs an empty dirt road node.
/// </summary>
DirtRoadNode::DirtRoadNode(void) :
	Tile(ISOTILE_NONE),
	Origin(Cell(0, 0)),
	CapTile(ISOTILE_NONE),
	CapOrigin(Cell(0, 0)),
	BackLinkIndex(-1)
{

}


/// <summary>
/// Copy constructor; copies all five members from another DirtRoadNode.
/// </summary>
/// <param name="that">Source struct to copy.</param>
DirtRoadNode::DirtRoadNode(DirtRoadNode const & that) :
	Tile(that.Tile),
	Origin(that.Origin),
	CapTile(that.CapTile),
	CapOrigin(that.CapOrigin),
	BackLinkIndex(that.BackLinkIndex)
{

}


/// <summary>
/// Constructs a dirt road tile with a single connection.
/// This is the shape the road builder uses to cap a road off.
/// </summary>
DirtRoadTile::DirtRoadTile(DirtRoadLink const & link1) :
	ITType(ISOTILE_CLEAR)
{
	Count = 1;
	Links = new DirtRoadLink[1];
	Links[0] = link1;
}


/// <summary>
/// Constructs a dirt road tile with two connections.
/// </summary>
DirtRoadTile::DirtRoadTile(DirtRoadLink const & link1, DirtRoadLink const & link2) :
	ITType(ISOTILE_CLEAR)
{
	Count = 2;
	Links = new DirtRoadLink[2];
	Links[0] = link1;
	Links[1] = link2;
}


/// <summary>
/// Constructs a dirt road tile with three connections.
/// </summary>
DirtRoadTile::DirtRoadTile(DirtRoadLink const & link1, DirtRoadLink const & link2, DirtRoadLink const & link3) :
	ITType(ISOTILE_CLEAR)
{
	Count = 3;
	Links = new DirtRoadLink[3];
	Links[0] = link1;
	Links[1] = link2;
	Links[2] = link3;
}


/// <summary>
/// Constructs a dirt road tile with four connections.
/// </summary>
DirtRoadTile::DirtRoadTile(DirtRoadLink const & link1, DirtRoadLink const & link2, DirtRoadLink const & link3, DirtRoadLink const & link4) :
	ITType(ISOTILE_CLEAR)
{
	Count = 4;
	Links = new DirtRoadLink[4];
	Links[0] = link1;
	Links[1] = link2;
	Links[2] = link3;
	Links[3] = link4;
}


/// <summary>
/// Constructs a dirt road tile with no connections.
/// This is the placeholder for the library slots that carry no road at all.
/// </summary>
DirtRoadTile::DirtRoadTile()// :
{
	Count = 0;
	Links = NULL;
}


/// <summary>
/// Destructor; deletes the Links array and sets Links to NULL.
/// </summary>
DirtRoadTile::~DirtRoadTile(void)
{
	delete Links;
	Links = NULL;
}


DirtRoadTile DirtRoadTiles[] = {
	/* 0 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(2, 0), FACING_NE)),
	/* 1 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 0), FACING_E)),
	/* 2 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 1), FACING_SE)),
	/* 3 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 0), FACING_SW)),
	/* 4 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 5 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_N), DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 6 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_NE), DirtRoadLink(Cell(1, 1), FACING_E)),
	/* 7 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_NE), DirtRoadLink(Cell(1, 1), FACING_SE)),
	/* 8 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_NE), DirtRoadLink(Cell(0, 1), FACING_S)),
	/* 9 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_NE), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 10 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 11 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(0, 0), FACING_N)),
	/* 12 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(1, 2), FACING_SE)),
	/* 13 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(0, 1), FACING_S)),
	/* 14 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(1, 0), FACING_SW)),
	/* 15 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 16 */ DirtRoadTile(DirtRoadLink(Cell(2, 1), FACING_SE), DirtRoadLink(Cell(0, 1), FACING_S)),
	/* 17 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_SE), DirtRoadLink(Cell(1, 0), FACING_SW)),
	/* 18 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_SE), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 19 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_S), DirtRoadLink(Cell(1, 0), FACING_SW)),
	/* 20 */ DirtRoadTile(DirtRoadLink(Cell(0, 1), FACING_S), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 21 */ DirtRoadTile(DirtRoadLink(Cell(0, 1), FACING_S), DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 22 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_SW), DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 23 */ DirtRoadTile(DirtRoadLink(Cell(0, 1), FACING_W), DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 24 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(0, 1), FACING_S)),
	/* 25 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(0, 1), FACING_S), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 26 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 1), FACING_S), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 27 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 28 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 0), FACING_E), DirtRoadLink(Cell(0, 1), FACING_S), DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 29 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(2, 2), FACING_SE), DirtRoadLink(Cell(1, 1), FACING_SW)),
	/* 30 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(1, 1), FACING_SW), DirtRoadLink(Cell(2, 2), FACING_SE)),
	/* 31 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(1, 1), FACING_SW)),
	/* 32 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(2, 2), FACING_SE)),
	/* 33 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(1, 1), FACING_SW), DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(2, 2), FACING_SE)),
	/* 34 */ DirtRoadTile(DirtRoadLink(Cell(3, 0), FACING_NE), DirtRoadLink(Cell(3, 2), FACING_E), DirtRoadLink(Cell(0, 2), FACING_W)),
	/* 35 */ DirtRoadTile(DirtRoadLink(Cell(3, 0), FACING_NE), DirtRoadLink(Cell(1, 2), FACING_SW)),
	/* 36 */ DirtRoadTile(DirtRoadLink(Cell(3, 0), FACING_NE), DirtRoadLink(Cell(1, 2), FACING_SW)),
	/* 37 */ DirtRoadTile(DirtRoadLink(Cell(3, 0), FACING_NE), DirtRoadLink(Cell(1, 2), FACING_SW)),
	/* 38 */ DirtRoadTile(DirtRoadLink(Cell(3, 0), FACING_NE), DirtRoadLink(Cell(1, 2), FACING_SW)),
	/* 39 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(1, 1), FACING_SW)),
	/* 40 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_NE), DirtRoadLink(Cell(1, 0), FACING_SW)),
	/* 41 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_NE), DirtRoadLink(Cell(1, 0), FACING_SW)),
	/* 42 */ DirtRoadTile(DirtRoadLink(Cell(1, 2), FACING_SW)),
	/* 43 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_SW)),
	/* 44 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_SW)),
	/* 45 */ DirtRoadTile(DirtRoadLink(Cell(3, 0), FACING_NE)),
	/* 46 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE)),
	/* 47 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE)),
	/* 48 */ DirtRoadTile(DirtRoadLink(Cell(4, 0), FACING_NE), DirtRoadLink(Cell(1, 1), FACING_SW)),
	/* 49 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_NE), DirtRoadLink(Cell(1, 3), FACING_SW)),
	/* 50 */ DirtRoadTile(),
	/* 51 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(3, 0), FACING_E)),
	/* 52 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(3, 0), FACING_E)),
	/* 53 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(3, 0), FACING_E)),
	/* 54 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(3, 0), FACING_E)),
	/* 55 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(2, 0), FACING_E)),
	/* 56 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(1, 0), FACING_E)),
	/* 57 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(0, 0), FACING_E)),
	/* 58 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(0, 0), FACING_E)),
	/* 59 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 60 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W)),
	/* 61 */ DirtRoadTile(DirtRoadLink(Cell(0, 1), FACING_W)),
	/* 62 */ DirtRoadTile(DirtRoadLink(Cell(2, 0), FACING_E)),
	/* 63 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E)),
	/* 64 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_E)),
	/* 65 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(3, 1), FACING_E)),
	/* 66 */ DirtRoadTile(DirtRoadLink(Cell(0, 1), FACING_W), DirtRoadLink(Cell(3, 0), FACING_E)),
	/* 67 */ DirtRoadTile(),
	/* 68 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(3, 3), FACING_SE)),
	/* 69 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(3, 3), FACING_SE)),
	/* 70 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(3, 3), FACING_SE)),
	/* 71 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(3, 3), FACING_SE)),
	/* 72 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(2, 2), FACING_SE)),
	/* 73 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(1, 1), FACING_SE)),
	/* 74 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(1, 1), FACING_SE)),
	/* 75 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 76 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 77 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW)),
	/* 78 */ DirtRoadTile(DirtRoadLink(Cell(3, 3), FACING_SE)),
	/* 79 */ DirtRoadTile(DirtRoadLink(Cell(1, 2), FACING_SE)),
	/* 80 */ DirtRoadTile(DirtRoadLink(Cell(2, 2), FACING_SE)),
	/* 81 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(4, 2), FACING_SE)),
	/* 82 */ DirtRoadTile(DirtRoadLink(Cell(1, 1), FACING_NW), DirtRoadLink(Cell(2, 4), FACING_SE)),
	/* 83 */ DirtRoadTile(),
	/* 84 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 3), FACING_S)),
	/* 85 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 3), FACING_S)),
	/* 86 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 3), FACING_S)),
	/* 87 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 3), FACING_S)),
	/* 88 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 2), FACING_S)),
	/* 89 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 1), FACING_S)),
	/* 90 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_S)),
	/* 91 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_S)),
	/* 92 */ DirtRoadTile(DirtRoadLink(Cell(0, 3), FACING_S)),
	/* 93 */ DirtRoadTile(DirtRoadLink(Cell(0, 2), FACING_S)),
	/* 94 */ DirtRoadTile(DirtRoadLink(Cell(0, 2), FACING_S)),
	/* 95 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N)),
	/* 96 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N)),
	/* 97 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N)),
	/* 98 */ DirtRoadTile(DirtRoadLink(Cell(1, 0), FACING_N), DirtRoadLink(Cell(0, 3), FACING_S)),
	/* 99 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(1, 3), FACING_S)),
	/* 100 */ DirtRoadTile(),
	/* 101 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(0, 0), FACING_E)),
	/* 102 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(0, 0), FACING_E)),
	/* 103 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_S)),
	/* 104 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_S)),
	/* 105 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(0, 0), FACING_E)),
	/* 106 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_W), DirtRoadLink(Cell(0, 0), FACING_E)),
	/* 107 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_S)),
	/* 108 */ DirtRoadTile(DirtRoadLink(Cell(0, 0), FACING_N), DirtRoadLink(Cell(0, 0), FACING_S))
};


/// <summary>
/// Fetches the road library entry for a dirt road tile.
/// Use this routine to get from a tile already sitting on the map back to the entry that
/// describes where its roads connect. Anything that is not a dirt road lands on the first
/// entry.
/// </summary>
/// <param name="ittype">The isometric tile type to look up.</param>
/// <returns>Returns with the index of the matching dirt road entry.</returns>
int Dirt_Road_Tile_Index(IsometricTileType ittype)
{
	if (ittype >= IsometricTileTypeClass::DirtRoadCurve && ittype < IsometricTileTypeClass::DirtRoadCurve + DIRT_ROAD_TILE_TOTAL) {
		return(ittype - IsometricTileTypeClass::DirtRoadCurve);
	}
	if (ittype < IsometricTileTypeClass::DirtRoadSlopes || ittype >= IsometricTileTypeClass::DirtRoadSlopes + DIRT_ROAD_SLOPES_COUNT) {
		return(0);
	}
	return(ittype - IsometricTileTypeClass::DirtRoadSlopes + DIRT_ROAD_TILE_TOTAL);
}


/// <summary>
/// Stamps every dirt road entry with its tile type.
/// The road library is written out as connection data alone, so nothing in it knows which
/// isometric tile it stands for until this routine runs. Call it before laying any dirt
/// road.
/// </summary>
void Init_Dirt_Roads(void)
{
	int i;
	for (i = 0; i < DIRT_ROAD_TILE_TOTAL; i++) {
		DirtRoadTiles[i].ITType = IsometricTileType(IsometricTileTypeClass::DirtRoadCurve + i);
	}
	for (i = 0; i < DIRT_ROAD_SLOPES_COUNT; i++) {
		DirtRoadTiles[i + DIRT_ROAD_TILE_TOTAL].ITType = IsometricTileType(IsometricTileTypeClass::DirtRoadSlopes + i);
	}
}
