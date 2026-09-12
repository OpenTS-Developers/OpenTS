/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "loaddlg.h"

#include "isotype.hh"

class CellClass;
class MapPreviewClass;

#define RANDOM_MAP_FILE_NAME "RandMap.Sed"

enum BiomeType {
	BIOME_TUNDRA,
	BIOME_TAIGA,
	BIOME_TEMPERATE,
	BIOME_DESERT,
	BIOME_MUTATED,
	BIOME_COUNT,

	BIOME_FIRST = 0,
};

enum TimeOfDayType {
	TIME_OF_DAY_MORNING,
	TIME_OF_DAY_AFTERNOON,
	TIME_OF_DAY_DUSK,
	TIME_OF_DAY_NIGHT,
	TIME_OF_DAY_COUNT,

	TIME_OF_DAY_FIRST = 0,
};

enum MapSizeType {
	MAPSIZE_SMALL,
	MAPSIZE_MEDIUM,
	MAPSIZE_LARGE,
	MAPSIZE_VERY_LARGE,
	MAPSIZE_COUNT,

	MAPSIZE_FIRST = 0,
};


class MapRegionClass
{
public:
	struct CellData {
		/*
		 * This is the map cell this record belongs to. A slot lying off the playfield is
		 * never filled in and still holds Cell(0,0), which is how the passes skip it.
		 */
		Cell CellID;

		/*
		 * This is how far the cell's ground is to be raised or lowered when the hills are
		 * raised, in whole height levels. Generate_Hills steps the terrain to meet it.
		 */
		double HillHeight;

		/*
		 * This is how far the hill height walk may wander at this cell, drawn from the
		 * already settled neighbors and bounded by the seed's hills setting.
		 */
		double HillVolatility;

		/*
		 * These are the chances, per cell, of each kind of ground cover being planted there.
		 * The vegetation seeding pass fills them in and Generate_Vegetation tries them in
		 * turn -- the first to come up decides what the cell gets, so the order matters.
		 */
		double GreenTileChance;
		double SandTileChance;
		double RoughTileChance;
		double ForestChance;

		/*
		 * This is the region that has laid claim to the cell, or zero while it is unclaimed.
		 * The region passes also park negative scratch values here while measuring a region.
		 */
		int RegionID;

		/*
		 * This is the id stamped on the cell by whichever routine is currently spreading out
		 * from a seed, tested against that routine's own id to keep it from doubling back over
		 * ground it has covered. Where RegionID records who owns a cell for good, this records
		 * only who has reached it during the spread now running.
		 */
		int SpreadID;

		int WaterMask;               /// Water adjacency mask for region building

		/*
		 * If this cell lies well in from the border of the region being worked on, then this
		 * flag will be true. It keeps a lake off the fringe of the ground it grows on.
		 */
		bool Interior;

		/*
		 * If an earlier pass has finished with this cell and later ones must leave it alone,
		 * then this flag will be true. It protects the ground beside shorelines and cliffs
		 * from the hill height walk, and keeps ground cover off it.
		 */
		bool Inviolate;

		bool TilePlaced;             /// Has this cell been flagged to contain a tile?
		bool Forested;               /// Has this cell been considered for placing trees?

		/*
		 * If the blotch of ice being painted has already reached this cell, then this flag
		 * will be true. It is cleared before each variant so a blotch cannot double back.
		 */
		bool Iced;

		/*
		 * If the cell belongs to the piece of ground Mark_Fill_Area grew out from its seed --
		 * water joining water, or clear ground standing at the seed's own height -- then this
		 * flag will be true.
		 */
		bool InFillArea;

		/*
		 * If the cell lies within the fill area or the ring of cells around it, then this flag
		 * will be true. It is what the cliff and shore passes test before touching a cell, so
		 * that they may dress the outside edge of the piece as well as the piece itself.
		 */
		bool InFillReach;

		/*
		 * If a region growing up against this cell may absorb it, then this flag will be
		 * true. The water spread sets it on the ground it reaches and Claim_Expandable_Cells
		 * does the absorbing; wiping a region's cells clears it again.
		 */
		bool CanExpand;

		CellData(void);

		/*
		 * Cell-data grid access.
		 */
		static bool Is_Not_Inviolate(Cell const &cell);
		static bool Is_Interior(Cell const &cell);
		static void Set_Inviolate(Cell const &cell);
		static void Clear_Inviolate(Cell const &cell);
		static void Clear_Inviolates(void);
		static void Set_Region(Cell const &cell, int region_id);
		static int Get_Region(Cell const &cell);
		static void Set_Spread(Cell const &cell, int patch_id);
		static int Get_Spread(Cell const &cell);
		static void Reset_Spreads(bool);
	};

	public:

		MapRegionClass(Cell cell);
		~MapRegionClass(void);

		/*
		 * Cell-data grid access.
		 */
		static MapRegionClass::CellData &Get_Cell_Data(Cell const &cell);
		static MapRegionClass::CellData &Get_Cell_Data(int index);

		/*
		 * Region creation and lifetime.
		 */
		static MapRegionClass *Create_Region_From_Cell(CellClass *cptr);
		static void Create_Water_Regions(void);
		static void Create_Land_Regions(bool unsplittable);
		static bool Make_Regions(void);
		static void Make_Ice_Regions(void);
		static void Destroy_Cell_Regions(void);
		static void Delete_All_Regions(void);
		static MapRegionClass *From_ID(int id);

		/*
		 * Random cell pickers.
		 */
		static Cell Pick_Random_Map_Cell(void);
		static Cell Pick_Random_Clear_Map_Cell(void);
		static Cell Pick_Random_Clear_Cell_In_Zone(int zone, int region_id);
		Cell Pick_Random_Cell(void);
		Cell Pick_Random_Clear_Cell(void);
		Cell Pick_Random_Clear_Interior_Cell(void);

		/*
		 * Region shape and growth operations.
		 */
		void Recount_Cells(void);
		void Clear_Cells(int new_id, int cell_height);
		DynamicVectorClass<Cell> *Build_Border_Cell_List(void);
		DynamicVectorClass<Cell> *Build_Border_Cell_List2(void);
		void Reassign_Border_Cells(int rings, unsigned int region_id);
		bool Grow(int rings);
		bool Claim_Expandable_Cells(void);
		bool Split_Region(void);
		void Increase_Height(char cell_height);
		void Mark_Interior_Cells(void);
		double Get_Angle_Score(const Cell & cell1, const Cell & cell2, double ref_angle);

		/*
		 * Region connections: neighbors, bridges and ramps.
		 */
		void Build_Neighbor_Regions(void);
		void Make_Region_Connections(void);
		unsigned Build_Neighbor_Region_Mask(Cell const & cell, bool & valid, int unknown);
		bool Connect_Regions_With_Bridge(MapRegionClass * region1, MapRegionClass * region2);
		bool Is_Bridge_Allowed(Rect const & rect) const;
		bool Place_Bridge_Hut(Rect const & rect) const;
		bool Connect_Regions_With_Ramp(Cell const & cell, int region_id, bool allow_fallback);
		bool Build_South_Ramp(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_East_Ramp(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_North_Ramp(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_West_Ramp(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_Corner_Ramp_SE(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_Corner_Ramp_NE(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_Corner_Ramp_NW(Cell const & cell1, Cell const & cell2, int region_id);
		bool Build_Corner_Ramp_SW(Cell const & cell1, Cell const & cell2, int region_id);
		bool Is_Ramp_Area_Clear(Rect & rect, int region_id);
		void Flatten_Area(Rect const & rect, int region_id);
		void Flatten_Area(Rect const & rect);

	public:
		/*
		 * Pointer to the list of region numbers this region borders. It is built just before
		 * the bridges and ramps are laid out and thrown away directly afterward, so it is NULL
		 * at any other time.
		 */
		DynamicVectorClass<int> *NeighborRegions;

		/*
		 * This is the number the region is known by, handed out from TotalCount at creation.
		 * It is what gets stamped into the RegionID of every cell the region claims.
		 */
		int ID;

		/*
		 * This is the number of cells the region owns. It decides whether the region is big
		 * enough to be worth splitting, or too small to stand alone and better merged away.
		 */
		int CellCount;

		/*
		 * This is the height every cell of the region stands at, taken from the seed cell. A
		 * difference between two neighboring regions is what calls for a ramp or a cliff.
		 */
		int CellHeight;

		/*
		 * If the region is a stretch of water rather than dry land, then this flag will be
		 * true. A water region is never split, and is spanned with bridges instead of ramps.
		 */
		bool ContainsWater;

		/*
		 * This is the cell the region was first grown out from. Nothing consults it -- the
		 * region is worked on through its Cells list and its RegionBounds instead.
		 */
		Cell Position;

		/*
		 * If the region is to be left whole, then this flag will be true. It is set once a
		 * region is small enough to keep, so the splitting pass stops picking it over.
		 */
		bool Unsplittable;

		/*
		 * If the region connector could not carve a single ramp down to a neighbor, then this
		 * flag will be false. Nothing consults it, so the region ships as it stands.
		 */
		bool HasConnections;

		/// Unused
		int SplitCount;

		/*
		 * If the region has been folded into a neighbor and is only waiting to be deleted,
		 * then this flag will be true. The merge pass steps over it from then on.
		 */
		bool Discardable;

		/*
		 * These are the cells the region owns, rebuilt from the cell data grid whenever the
		 * region tallies are brought back into step. The splitting pass works from this list.
		 */
		DynamicVectorClass<Cell> Cells;

		/*
		 * This is the smallest rectangle enclosing the region's cells, grown as the cells are
		 * counted, so that a later pass need not sweep the whole map to find them.
		 */
		Rect RegionBounds;

		/*
		 * These are all the regions the generator currently holds. A region adds itself here
		 * when it is created and takes itself out again when it is destroyed.
		 */
		static DynamicVectorClass<MapRegionClass *> MapRegions;

		/*
		 * This is the number of regions created since the last wipe, and doubles as the number
		 * the next one will be given. It counts creations rather than live regions, so a
		 * destroyed region leaves a hole in the numbering.
		 */
		static int TotalCount;

		/*
		 * These bound the diamond of cells the generator treats as part of the map. They are
		 * worked out from the playable rectangle whenever the map is sized, and My_In_Radar
		 * tests a cell against them.
		 */
		static int MapStartDiagonal;
		static int MapEndDiagonal;
};


class MapSeedClass : public LoadOptionsClass
{
		typedef LoadOptionsClass BASECLASS;

	public:
		MapSeedClass(void);

		/*
		 * Seed file persistence.
		 */
		bool Save(const char * name);
		bool Load(const char * name);
		bool Delete(void);

		virtual bool Load_File(const char * file_name) override;
		virtual bool Save_File(const char * file_name, const char * descr) override;
		virtual bool Delete_File(const char * file_name) override;
		virtual bool Read_File(FileEntryClass * entry, WIN32_FIND_DATAA * ff) override;

	protected:

		virtual int Save_Confirmation(void) const override;

	public:

		/*
		 * Dialog interaction.
		 */
		void Get_Settings(HWND dialog);
		void Set_Settings(HWND dialog);
		void Set_Scroll_Bar(HWND handle, unsigned int min, unsigned int max, int position, bool enable);
		void Set_Checkbox(HWND handle, bool state, bool enable);

		/*
		 * Settings adjustment.
		 */
		void Randomize(void);
		void Fixup_Settings(void);
		void Fixup_WDT_Settings(void);
		void Clamp_Setting(int & setting, int min, int max, bool use_midpoint) const;

	public:
		/*
		 * This is the kind of country the map is to be laid out in (a BiomeType), which
		 * settles the theater and colors every later decision about terrain and cover.
		 */
		int Biome;

		/*
		 * This is how hilly the ground should be (0 - 100). It scales both the step the hill
		 * height walk takes and how far that walk may wander; a low enough setting leaves the
		 * map flat.
		 */
		int Hills;

		/*
		 * This is the time of day the battle is fought at (a TimeOfDayType), which sets the
		 * scenario's ambient light and how many street lights the towns are given.
		 */
		int Time;

		/*
		 * This is how much of the map should be under water (0 - 100). It sets the budget the
		 * lake and river passes have to spend, and at zero the map is left dry.
		 */
		int WaterAmount;

		/*
		 * This is the number of players the map is built for (2 - MAX_PLAYERS). It fixes how
		 * many starting points must be found and, with the size settings, how large the
		 * playfield is.
		 */
		int NumPlayers;

		/*
		 * This is how richly the tiberium fields are grown (1 - 100). It governs only how much
		 * tiberium each field holds; TiberiumLayout governs how many fields there are.
		 */
		int Tiberium;

		/*
		 * This is how many tiberium fields the map is given (0 - 100), scaled against the
		 * player count so that a crowded map is not left starved.
		 */
		int TiberiumLayout;

		/*
		 * This is how thickly the ground cover is planted (0 - 100). It scales the green,
		 * sand, rough and forest chances that the seeding pass leaves on each cell.
		 */
		int Vegetation;

		/*
		 * This is how built up the map should be (0 - 100), governing how many urban areas and
		 * how many lone rural buildings are scattered over it.
		 */
		int Cities;

		/*
		 * These are the map's proportions, a MapSizeType each. They do not give the playfield
		 * size outright -- they choose it within the range the player count allows.
		 */
		int Width;
		int Height;

		/*
		 * This is how freely a unit should be able to get about the map (0 - 100). It is the
		 * chance that a pair of neighboring regions is joined by more than one ramp.
		 */
		int Accessibility;

		/*
		 * This is how much cliff the map should carry (0 - 100). It sets the size a height
		 * region may reach before it is split, and it is the splitting into regions at
		 * differing heights that leaves the cliff faces behind.
		 */
		int Cliffs;

		/*
		 * This is the number the generator's random sequence is started from (0 - 65535), so
		 * that the same settings and the same seed always give back the same map. If -1, then
		 * a fresh number is rolled when the map is generated.
		 */
		int Seed;

		/*
		 * This is the text the load and save dialogs list this seed under, and the buffer that
		 * the inherited Description pointer is aimed at.
		 */
		char MapDescription[128];

		/*
		 * This is the chance that a tiberium field is given creatures to guard it (0 - 100).
		 * The dialog offers it only as a checkbox, which sets it to 30 or to zero.
		 */
		int TiberiumWildlife;

		/*
		 * This is the number of veinhole monsters the map is to be given (0 - 5). A crowded
		 * map simply ends up with fewer of them.
		 */
		int VeinholeMonsters;

		/*
		 * If the map is to suffer ion storms, then this flag will be true. The generator then
		 * folds ION.INI into the rules and the lighting. Firestorm only.
		 */
		bool UseIonStorms;

		/*
		 * If some of the map's fields are to grow the second tiberium type rather than the
		 * first, then this flag will be true. Firestorm only.
		 */
		bool UseBlueTiberium;

		/*
		 * If the map's lighting is to shift as the battle wears on, then this flag will be
		 * true. It is the one setting left out of the multiplayer map digest, which reaches
		 * from Biome to the end of the class and stops short of it.
		 */
		bool UseTransitions;

};

class MapGeneratorClass
{
	public:
		MapGeneratorClass(void);
		~MapGeneratorClass(void);

		/*
		 * Top-level generation and housekeeping.
		 */
		void Generate_Random_Map(bool full_init, HWND dialog);
		void Init_Map(bool full_init);
		void Cleanup(void);
		void Update_Progress(int percent_progress);

		/*
		 * Cell-data grid access.
		 */
		void Set_Cell_Data_Region(Cell const &cell, int region_id);
		int Get_Cell_Data_Region(Cell const &cell);
		void Set_Cell_Data_Spread(Cell const &cell, int patch_id);
		int Get_Cell_Data_Spread(Cell const &cell);
		void Clear_Cell_Data_Spreads(void);

		/*
		 * Region bookkeeping.
		 */
		void Init_Regions(void);
		bool Make_Regions(void);
		bool Expand_All_High_Ground(void);
		bool Grow_Region(int id, int rings, Rect rect, char set_height, char height);
		bool Clear_Region_Border(int id, int rings, int new_id);
		void Clear_Region_Cells(int id, int new_id, int cell_height);
		DynamicVectorClass<Cell> *Build_Region_Border_Cell_List(int id);
		static bool Region_Cell_Count_At_Least(Rect const & rect, int region_id, int, int count);
		void Set_Height_Of_ID(int id, char cell_height);
		void Increase_Height_Of_ID(int id, char cell_height);
		void Increase_Height_Except_ID(int cell_height, int id);

		/*
		 * Water, ice and swamps.
		 */
		void Seed_Water(void);
		bool Seed_Lake(const Cell & cell);
		bool Seed_Arctic_Lake(const Cell & cell);
		bool Seed_River(const Cell & cell, double start_angle, bool is_branch);
		bool Seed_Arctic_River(const Cell & cell, double start_angle);
		bool Grow_Water_Region(int region_id, float spread_scale, Rect const & bounds, Cell const & origin, bool claim_frontier);
		bool Place_Waterfall(int region_id, Cell const & cell1, Cell const & cell2, int direction, bool & placed, double & head_x, double & head_y);
		int Get_Target_Water_Amount(void);
		void Generate_Swamp(DynamicVectorClass<Cell> & cells, int last);
		void Seed_Ice(DynamicVectorClass<Cell> &cells, IsometricTileType last);
		void Smooth_Ice(void);

		/*
		 * Hills.
		 */
		void Seed_Hill_Anchors(void);
		void Seed_Hill_Heights(void);
		void Generate_Hills(void);

		/*
		 * Vegetation and terrain detail.
		 */
		void Seed_Vegetation(void);
		void Seed_Arctic_Vegetation(void);
		void Generate_Vegetation(void);
		void Generate_Arctic_Vegetation(void);
		void Place_Forest(CellClass const * cellptr, int count, double density);
		void Place_Tile_Patch(CellClass * cellptr, IsometricTileType ittype, int count, int origin_id, bool on_pavement);
		void Generate_Mold(void);
		void Generate_Crystals(const Cell & cell);

		/*
		 * Tiberium and wildlife.
		 */
		void Create_Tiberium(void);
		void Create_Tiberium_Patch(Cell const &cell, int count, int patch_id, bool use_primary_tiberium, bool place_tree, bool spawn_wildlife);

		/*
		 * Urban and rural areas.
		 */
		void Generate_Urban_Areas(void);
		DynamicVectorClass<Cell> * Create_Urban_Area(Cell const & cell, int size);
		void Generate_Urban_Roads(const DynamicVectorClass<Cell> & cells);
		void Generate_Urban_Buildings(const DynamicVectorClass<Cell> & cells);
		void Generate_Urban_Units(const DynamicVectorClass<Cell> & cells);
		void Generate_Urban_Pavement(DynamicVectorClass<Cell> & cells);
		void Generate_Rural_Areas(void);
		void Generate_Rural_Buildings(const Cell & cell);
		void Generate_Rural_Units(const Cell & cell);
		bool Generate_Rural_Roads(const Cell & cell);
		void Generate_Lights(void);
		void Generate_Veinholes(void);

		/*
		 * Road and tile placement helpers.
		 */
		Cell Plan_Paved_Road_Junction(Cell const & cell, Rect const & rect, FacingType dir);
		Cell Plan_Paved_Road(Cell const & cell, Rect const & rect, FacingType dir);
		static bool Can_Place_Paved_Road(Rect const & rect, bool overlap_road, bool overlap_road_end);
		static bool Can_Place_Paved_Road_End(Rect const & rect, bool overlap_road);
		static bool Can_Place_Paved_Road_Junction(Rect & rect);
		static bool Can_Place_Misc_Pavement(Rect const & rect);
		static bool Find_Dirt_Road_Tile(FacingType facing, Cell const & target_cell, int max_connections, IsometricTileType & out_ittype, Cell & out_origin, int & out_link_index);
		static void Place_Tile(IsometricTileType ittype, const Cell & cell, int region = -1, int height = -1);
		static bool Can_Place_Tile(IsometricTileType ittype, Cell const & cell, bool collision, bool in_map);
		static void Remove_Tile(IsometricTileType ittype, Cell const & cell);
		static void Mark_Tile(IsometricTileType ittype, Cell const & cell, bool placed);
		static bool Is_Area_Cliff_Free(const Cell & cell);
		static bool Is_Cell_Area_Clear(Rect const & rect, bool check_data);
		static Rect Get_Cell_Bounding_Rect(const DynamicVectorClass<Cell> & cells);

		/*
		 * Spread scoring functions.
		 */
		double Get_Angle_Score(const Cell & cell1, const Cell & cell2, double ref_angle);
		double Get_Ice_Score(const Cell & cell1, const Cell & cell2, double random_range) const;
		double Get_Spread_Score(const Cell & cell1, const Cell & cell2, int spread_step) const;
		double Get_Tiberium_Score(const Cell & cell1, const Cell & cell2) const;
		double Get_Urban_Score(const Cell & cell1, const Cell & cell2) const;
		double Get_Forest_Score(const Cell & cell1, const Cell & cell2) const;
		double Get_Tile_Patch_Score(const Cell & cell1, const Cell & cell2) const;

		/*
		 * Player start locations.
		 */
		bool Init_Start_Points(void);

	public:
		/*
		 * These are the settings the next map is to be built from, as the random map dialog
		 * left them. Every generation pass reads the map it is meant to make from here.
		 */
		MapSeedClass SeedData;

		/*
		 * Pointer to a copy of the settings the last preview was built from, kept so that the
		 * generator can tell whether the theater and tile set still suit the map being asked
		 * for or must be loaded afresh.
		 */
		MapSeedClass * MapSeeder;

		/*
		 * Pointer to the thumbnail of the generated map, which the random map dialog shows the
		 * player and the multiplayer code passes on to the other players.
		 */
		MapPreviewClass * MapPreview;

		/*
		 * These are the dimensions of the playable part of the map in cells, chosen from the
		 * seed's size settings and player count. The generator sizes its working arrays and
		 * its region limits from them.
		 */
		int LocalWidth;
		int LocalHeight;

		/*
		 * This is how much water the lake and river passes have laid down so far, measured in
		 * cells. It is what they spend against the seed's water budget.
		 */
		int SeededWaterAmount;

		/*
		 * This is the number given to the region being laid down at the moment. Each water or
		 * ice feature takes the next one, so that a feature which comes out badly can be
		 * wiped again by its number alone.
		 */
		int WorkingRegionID;

		/*
		 * This is the level the whole map is laid out flat at before any hills are raised, and
		 * the level a wiped region is flattened back to.
		 */
		int CellHeight;

		/*
		 * If the rivers of this map may end in a waterfall, then this flag will be true. It is
		 * rolled once per map, so a map either has them or it does not.
		 */
		bool PlaceWaterfall;
};

extern MapGeneratorClass RandomMapGen;


inline bool My_In_Radar(Cell const &cell)
{
	int x = cell.X;
	int y = cell.Y;
	return (y + x > MapRegionClass::MapStartDiagonal &&
			x - y < MapRegionClass::MapStartDiagonal &&
			y - x < MapRegionClass::MapStartDiagonal &&
			y + x <= MapRegionClass::MapEndDiagonal) ? true : false;
}

void Do_Random_Map(HWND, bool (*callback)());
int Do_Random_Map_Dialog(bool (*callback)());

extern MapRegionClass::CellData *RMGCellData;
extern unsigned int MapCellStride;
