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

#include "objtype.h"
#include "rgb.h"
#include "vector.h"

#include "anim.hh"
#include "isotype.hh"
#include "land.hh"

#include <string>

class LightConvertClass;
class Surface;
class ShapeSet;

#pragma pack(4)
struct IsoTileRecord
{
	/*
	 * These are the pixel coordinates of this sub-tile's image within the tile set.
	 */
	int X;
	int Y;

	/*
	 * This is the byte offset from the start of this record to the extra image -- the taller
	 * piece of artwork that a cliff or a cave needs standing above the tile proper.
	 */
	int ExtraOffset;

	/*
	 * This is the byte offset from the start of this record to the sub-tile's depth data,
	 * which the drawing code reads to sort the tile against the objects standing on it.
	 */
	int ZDataOffset;

	/*
	 * This is the byte offset from the start of this record to the extra image's depth data.
	 */
	int ExtraZOffset;

	/*
	 * These are the bounds of the extra image in pixels. Its position is in the same space
	 * as the tile's own X and Y, so the difference between the pairs places it on the tile.
	 */
	int ExtraX;
	int ExtraY;
	int ExtraWidth;
	int ExtraHeight;

	/*
	 * If this sub-tile has an extra image above the tile proper, then this flag will be true.
	 */
	unsigned int IsHasExtraData:1;

	/*
	 * If this sub-tile carries depth data of its own, then this flag will be true.
	 */
	unsigned int IsHasZData:1;

	/*
	 * If this sub-tile takes its variation at random rather than from the clear terrain
	 * pattern, then this flag will be true -- it keeps open ground from repeating.
	 */
	unsigned int IsRandomized:1;

	/*
	 * This is the number of height levels this sub-tile lifts the cell it covers, so that a
	 * tile laid across rising ground raises each of its cells by the right amount.
	 */
	unsigned char Height;

	/*
	 * This is the control color of the sub-tile (0 - 15), which names the kind of ground it
	 * is. Land_Type turns it into the LandType that the movement code works in.
	 */
	signed char TileType;

	/*
	 * This is the way the ground slopes beneath this sub-tile, which the movement and the
	 * drawing code both consult. If zero, then the ground there is flat.
	 */
	signed char RampType;

	/*
	 * These are the two terrain colors of the sub-tile, taken from the control section of the
	 * original artwork. The radar and the map preview shade between them by cell height.
	 */
	RGBStruct LowColor;
	RGBStruct HighColor;
};
#pragma pack()

#pragma pack(4)
class IsoTileSet
{
	friend class IsometricTileClass;
	friend class IsometricTileTypeClass;

	public:
		IsoTileRecord const * Fetch_Record_Pointer(int index) const
		{
			return(Record_At(index % Tile_Count()));
		}
		IsoTileRecord const * Fetch_Record_Pointer_Unsafe(int index) const
		{
			return(Record_At(index));
		}
		IsoTileRecord * Fetch_Record_Pointer_Unsafe(int index)
		{
			return((IsoTileRecord *)Record_At(index));
		}

		/*
		**	Query functions.
		*/

		int Map_Width(void) const { return(MapWidth); }
		int Map_Height(void) const { return(MapHeight); }
		int Tile_Count(void) const { return(MapWidth * MapHeight); }
		int Pixel_Width(void) const { return(Width); }
		int Pixel_Height(void) const { return(Height); }

		int Get_Highest_Height(void) const
		{
			int height = 0;
			for (int index = 0; index < Tile_Count(); ++index) {
				IsoTileRecord const * record = Fetch_Record_Pointer(index);
				if (record != NULL) {
					if (record->Height > height)
						height = record->Height;
				}
			}
			return(height);
		}

	protected:
		/*
		 * These are the dimensions of this tile's sub-tile grid, in cells. One image record is
		 * stored per sub-tile, so the two multiplied together give the record count.
		 */
		int MapWidth;
		int MapHeight;

		/*
		 * The width and height of the tile (in pixels).
		 */
		int Width;
		int Height;

		/*
		 * This is the first of the tile set's image record offsets, one per sub-tile, each a
		 * byte offset from the start of the set. A zero offset means the sub-tile is absent
		 * from the diamond. Reach a record through Fetch_Record_Pointer rather than through
		 * the array.
		 */
		int TileOffsets[1];


	private:
		IsoTileRecord const * Record_At(int index) const
		{
			if (TileOffsets[index] == 0) {
				return(NULL);
			}
			return((IsoTileRecord const *)((unsigned char const *)this + TileOffsets[index]));
		}

	/*
	**	Disallow these operations with an IsoTileSet object.
	*/
	private:
		IsoTileSet(void) {};
		IsoTileSet(IsoTileSet const & rvalue);
		IsoTileSet const & operator = (IsoTileSet const & rvalue);
};
#pragma pack()


/****************************************************************************
**	The tile type objects are controlled by this class. It specifies the form
**	of the tile set for the specified object as well as other related datum.
**	It is derived from the ObjectTypeClass solely for the purpose of scenario
**	editing and creation.
*/
class IsometricTileTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		IsometricTileTypeClass(IsometricTileType type = ISOTILE_CLEAR, int unknown1 = 0, unsigned char unknown2 = 0, char const *ininame = NULL, bool skip_registration = false);
		virtual ~IsometricTileTypeClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_ISOTILETYPE);}
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);};

		virtual void Detach(AbstractClass const * target, bool all = true) override;
		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house) const override;
		virtual ObjectClass * Create_One_Of(HouseClass *house) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;

		static IsometricTileType From_Name(char const * name);

		virtual void const * Get_Image_Data(void) const override;

		LandType Land_Type(int tile) const;
		bool Is_Tile_Index_Valid(int tile, bool load);
		bool Load_Tile_Image(void);
		IsometricTileTypeClass *Next_Tile_From_Set(int index);
		static IsometricTileType Fixup_Tile_Type(IsometricTileType current_type);
		static LightConvertClass * Find_Or_Make_Drawer(int r, int g, int b);
		static bool Free_Unused_Drawers(int time_budget_ms, bool force);
		static void Init_Drawers(void);
		static void Read_Control_File(TheaterType theater, bool from_ccfile);
		static void Load_Tiles(bool,bool);
		int Load_Tile_Data(void);
		static void Clear_Use_Counts(void);
		bool Get_Tile_Pixel_Dimensions(int tile, int & width, int & height);
		int Ramp_Type(int tile) const;
		bool Is_Randomized(int tile) const;
		void Draw_Shadow_Caster(int index, Surface * surf, Point2D pt, Rect rect, int offset);
		Cell const * Shadow_Caster_List(void) const;
		void Draw_Tile(LightConvertClass * drawer, int subtile, Surface & surface, int x, int y, Rect cliprect, int height, int brightness, bool use_z, int cell_variation, bool fill, bool depth_only, bool fog, signed int fog_color) const;
		bool Get_Tile_Image(int tilenum, unsigned char **buffer, int width, int height);
		int Get_Y_Offset(int index);
		static void Clear_Z_Shapes(void);
		unsigned short Preview_Tile_Color(int tile, int level, bool use_high_color);
		unsigned short * Preview_Tile(int tile, int level);
		void Build_Preview_Tiles(void);

		int SubTile_Index(int x, int y) const { return(x + (Width * y)); }

	public:

		/*
		 * This is this tile type's own index within the IsometricTileTypes heap, and thus the
		 * value that a cell's ITType records.
		 */
		IsometricTileType HeapID;

		/*
		 * Tile set to use for these tiles when in marble madness mode.
		 */
		IsometricTileType MarbleMadness;

		/*
		 * For marble madness tiles, this is the tile set to use when not in marble madness mode.
		 */
		IsometricTileType NonMarbleMadness;

		/*
		 * This is the heap ID of the first tile type of the set this tile belongs to, so that
		 * subtracting it from the HeapID gives this tile's own position within the set.
		 */
		int TileSetBaseID;

		/*
		 * These are the radar and map preview colors baked from each sub-tile's control colors
		 * -- one ramp of lighting levels per sub-tile -- so that neither the radar nor the
		 * preview need touch the tile artwork.
		 */
		DynamicVectorClass<unsigned short *> PreviewTiles;

		/*
		 * This points to the next visual variation of this tile, the ones whose file name
		 * carries a trailing letter. It is NULL at the end of the chain.
		 */
		IsometricTileTypeClass *NextTileTypeInSet;

		/*
		 * This is the tile set that takes this one's place when the map is converted to the
		 * snow theater. If ISOTILE_INVALID, then the snow theater has no counterpart for it.
		 */
		int ToSnowTheater;

		/*
		 * This is the tile set that takes this one's place when the map is converted to the
		 * temperate theater. Only the map editor makes such a conversion.
		 */
		int ToTemperateTheater;

		/*
		 * These describe the animation that a cell takes on along with a tile from this set --
		 * which animation to create, where within the cell to place it, which sub-tile it
		 * belongs on, and how far to bias its depth. Anim is ANIM_NONE when there is none.
		 */
		AnimType Anim;
		Point2D Offset;
		int AttachesTo;
		int ZAdjust;

		/// Unused
		int Unused1;

		/*
		 * Can this tile set be modified using the raise/lower ground function?
		 */
		bool IsMorphable;

		/*
		 * Do the tiles in this set cast a shadow onto the cells below them? Only the cliff
		 * pieces do, and the theater has room for five such sets.
		 */
		bool IsShadowCaster;

		/*
		 * Should the map editor's tile placement dialog offer this tile set? The game itself
		 * has no use for the flag.
		 */
		bool IsAllowToPlace;

		/*
		 * If the random map generator may still call for this tile set, then this flag will be
		 * true. Its artwork is kept on a random map even when no cell yet uses it.
		 */
		bool IsRequiredForRMG;

		/*
		**	Raw dimensions of this tile set (in icons).
		*/
		int Width;
		int Height;

		/// Unused
		int Unused2;

		/*
		 * This is the number of visual variations on this tile's chain, counting this one. A
		 * cell picks among them by pattern, or at random where the sub-tile is randomized.
		 */
		int NumTileTypesInSet;

		/*
		 * If this tile type has artwork of its own on disk, then this flag will be true. The
		 * artwork is loaded on demand, so this is what says there is something to read in.
		 */
		bool IsFileLoaded;

		/*
		 * This is the name of the file the artwork is read from, theater suffix and all. It is
		 * kept so that a tile type whose image was trimmed away can load itself again.
		 */
		std::string Filename;

		/*
		 * May a subterranean unit surface through ground covered by this tile set? A set that
		 * forbids it keeps a tunnelling unit from digging its way out there.
		 */
		bool IsAllowBurrowing;

		/*
		 * May Tiberium spread onto ground covered by this tile set? Ground that forbids it
		 * stays barren however much Tiberium surrounds it.
		 */
		bool IsAllowTiberium;

		/*
		 * This is the number of cells on the map that use this tile type, counted once the map
		 * has been read. The artwork of a tile type that nothing uses is thrown away rather
		 * than loaded, so that only half a theater need sit in memory.
		 */
		int UseCount;

	public:
		/*
		 * These are the first pieces of the three ramp tile sets -- the plain ramps that carry
		 * the ground up a level, the smoothed pieces that blend a ramp into its neighbors, and
		 * the marble madness stand-ins for the plain ones.
		 */
		static IsometricTileType RampStart;
		static IsometricTileType RampSmooth;
		static IsometricTileType MMRampBase;

		/*
		 * This is the tile set of plain clear ground, which is what a cell shows when nothing
		 * else has been laid on it, and what an empty map is filled with.
		 */
		static IsometricTileType ClearTile;

		/*
		 * These are the solid rough, sand, green and pavement tiles, each the piece a cell
		 * shows when that same ground surrounds it on all four sides.
		 */
		static IsometricTileType RoughTile;
		static IsometricTileType SandTile;
		static IsometricTileType GreenTile;
		static IsometricTileType PaveTile;

		/*
		 * This is the first of the fourteen miscellaneous pavement pieces, the patches of
		 * paving that are not part of a road. The map generator lays them beneath a base.
		 */
		static IsometricTileType MiscPaveTile;

		/*
		 * These are the first of the sixteen pieces that blend each of the four ground types
		 * into clear ground. Fixup_LAT adds a mask of which of the four neighbors are not that
		 * ground, so the pieces must stay in mask order.
		 */
		static IsometricTileType ClearToRoughLat;
		static IsometricTileType ClearToSandLat;
		static IsometricTileType ClearToGreenLat;
		static IsometricTileType ClearToPaveLat;

		/// Unused
		static IsometricTileType HeightBase;
		static IsometricTileType BlackTile;

		/*
		 * These are the first pieces of the road and railway bridge tile sets, sixteen pieces
		 * each. Every part of a span is named by its position within one of these, so the
		 * bridge damage and repair code works in offsets from them.
		 */
		static IsometricTileType BridgeSet;
		static IsometricTileType TrainBridgeSet;

		/*
		 * This is the first of the forty cliff faces -- the sheer rock that steps the ground
		 * from one height to the next. A piece's position within the set names its shape.
		 */
		static IsometricTileType CliffSet;

		/*
		 * This is the first of the forty-two shoreline pieces that carry the edge between land
		 * and water. A piece's position within the set names the facing of its coast, which is
		 * how the smoothing pass matches one piece against its neighbors.
		 */
		static IsometricTileType ShorePieces;

		/*
		 * This is the first of the fourteen open water tiles, and what a map is filled with
		 * when it asks for water rather than clear ground.
		 */
		static IsometricTileType WaterSet;

		/*
		 * These are the first pieces of the three ice tile sets the snow theater provides.
		 * Each holds sixteen full ice tiles, then the cracked tile, then the edge pieces, so
		 * the ice code names a piece by its offset from one of these.
		 */
		static IsometricTileType Ice1Set;
		static IsometricTileType Ice2Set;
		static IsometricTileType Ice3Set;

		/*
		 * This is the first of the forty-eight pieces that dress the land where it runs up
		 * against an ice sheet, chosen by a mask of which of the eight neighbors are ice.
		 */
		static IsometricTileType IceShoreSet;

		/*
		 * These are the first pieces of the two ten-piece slope sets that carry the ground
		 * between height levels; the second is the marble madness counterpart of the first.
		 */
		static IsometricTileType SlopeSetPieces;
		static IsometricTileType SlopeSetPieces2;

		/// Unused
		static IsometricTileType MonorailSlopes;

		/*
		 * These are the first pieces of the four tunnel mouth sets -- road, railway, dirt road
		 * and dirt railway -- of four pieces each, one per facing. Laying one down creates the
		 * TubeClass that carries a unit through the hill.
		 */
		static IsometricTileType Tunnels;
		static IsometricTileType TrackTunnels;
		static IsometricTileType DirtTunnels;
		static IsometricTileType DirtTrackTunnels;

		/*
		 * These are the first pieces of the four waterfall sets, one for each direction the
		 * water may fall. They count as cliff, apart from a pair of walkable sub-tiles on the
		 * first and last piece of each set.
		 */
		static IsometricTileType WaterfallEast;
		static IsometricTileType WaterfallWest;
		static IsometricTileType WaterfallNorth;
		static IsometricTileType WaterfallSouth;

		/*
		 * This is the first of the twenty cliff ramp pieces. The movement code counts them as
		 * cliff, as it does the rest of the rock face.
		 */
		static IsometricTileType CliffRamps;

		/*
		 * This is the first of the fifteen paved road pieces, which the smoothing pass fits
		 * together into a continuous run of road as each piece is laid.
		 */
		static IsometricTileType PavedRoads;

		/*
		 * This is the first of the four pieces that cap the end of a paved road, one per
		 * facing. The map generator lays one wherever a road it has built stops.
		 */
		static IsometricTileType PavedRoadEnds;

		/*
		 * This is the first of the fourteen median pieces, the divider strips laid between the
		 * lanes of a wide paved road.
		 */
		static IsometricTileType Medians;

		/*
		 * This is the first of the ten patches of broken ground, which the random map
		 * generator scatters over open terrain to break it up.
		 */
		static IsometricTileType RoughGround;

		/*
		 * This is the first of the eleven dirt road junctions, the pieces where one dirt road
		 * meets another. Like the curves and straight runs, they count as dirt road.
		 */
		static IsometricTileType DirtRoadJunction;

		/*
		 * This is the first of the curved dirt road pieces -- the ones that turn a road a
		 * quarter circle. Like the junctions and straight runs, they count as dirt road.
		 */
		static IsometricTileType DirtRoadCurve;

		/*
		 * This is the first of the straight runs of dirt road, which make up the length of a
		 * road between its junctions and bends.
		 */
		static IsometricTileType DirtRoadStraight;

		/*
		 * This is the first of the two cliff faces that weapons fire can bring down. The
		 * collapsed face is replaced by slope pieces, so blasting one opens a way up a cliff
		 * that had none. A theater without such cliffs leaves this ISOTILE_INVALID_CLIFF.
		 */
		static IsometricTileType DestroyableCliffs;

		/*
		 * This is the first of the cave mouths cut into a cliff at the water line. They are
		 * cliff as far as movement is concerned, so nothing may drive or walk into one.
		 */
		static IsometricTileType WaterCaves;

		/*
		 * This is the first of the cliff faces that drop straight into water. They count as
		 * cliff, so the shoreline they make is impassable rather than beach.
		 */
		static IsometricTileType WaterCliffs;

		/*
		 * This is the first of the four pieces that carry a paved road up or down a ramp.
		 */
		static IsometricTileType PavedRoadSlopes;

		/*
		 * This is the first of the eight pieces that carry a dirt road up or down a ramp.
		 * The map generator appends them to its road tile table so a road it lays can climb.
		 */
		static IsometricTileType DirtRoadSlopes;

		/*
		 * This is the tile set of loose rock, which the random map generator scatters over
		 * open ground in patches to break up the terrain.
		 */
		static IsometricTileType Rocks;

		/*
		 * This is the solid crystal ground tile, the piece a cell shows when crystal
		 * surrounds it on all four sides.
		 */
		static IsometricTileType CrystalTile;

		/*
		 * This is the first of the sixteen pieces that blend crystal ground into clear
		 * ground. Fixup_LAT adds a mask of which of the four neighbors are not crystal to
		 * this, so the pieces must stay in mask order.
		 */
		static IsometricTileType ClearToCrystalLat;

		/*
		 * This is the first of the crystal cliff pieces. Only part of such a tile is
		 * crystal, so the transition code counts it as crystal on those subtiles alone.
		 */
		static IsometricTileType CrystalCliff;

		/*
		 * This is the solid swamp tile, the piece a cell shows when swamp surrounds it on
		 * all four sides. A theater without swamp leaves this ISOTILE_INVALID.
		 */
		static IsometricTileType SwampTile;

		/*
		 * This is the first of the sixteen pieces that blend swamp into water, picked by
		 * Fixup_LAT from a mask of which of the four neighbors are not swamp.
		 */
		static IsometricTileType WaterToSwampLat;

		/*
		 * This is the solid blue mold tile, the piece a cell shows when mold surrounds it
		 * on all four sides.
		 */
		static IsometricTileType BlueMoldTile;

		/*
		 * This is the first of the sixteen pieces that blend blue mold into clear ground,
		 * picked by Fixup_LAT from a mask of which of the four neighbors are not mold.
		 */
		static IsometricTileType ClearToBlueMoldLat;

		/*
		 * These are the positions within a bridge tile set of the pieces that end a span --
		 * the top left and bottom right ends of an east-west bridge and the top right and
		 * bottom left ends of a north-south one, two pieces for each end. The damage and
		 * repair code walks a span until it reaches one of them.
		 */
		static IsometricTileType BridgeTopLeft1;
		static IsometricTileType BridgeTopLeft2;
		static IsometricTileType BridgeBottomRight1;
		static IsometricTileType BridgeBottomRight2;
		static IsometricTileType BridgeTopRight1;
		static IsometricTileType BridgeTopRight2;
		static IsometricTileType BridgeBottomLeft1;
		static IsometricTileType BridgeBottomLeft2;

		/*
		 * This is the position within a bridge tile set of the first middle piece of an
		 * east-west span. The damaged and destroyed pieces follow it, so a BRIDGE_MIDDLE_*
		 * stage added to this names the piece a section of the span should show.
		 */
		static IsometricTileType BridgeMiddle1;

		/*
		 * This is the position within a bridge tile set of the first middle piece of a
		 * north-south span, with the damaged and destroyed pieces following it.
		 */
		static IsometricTileType BridgeMiddle2;

		/*
		 * These are the tile types that begin each tile set the theater flagged as a shadow
		 * caster, of which there is room for five. A cliff tile subtracts its own type from
		 * one of these to find the row of the shadow table describing its shadow.
		 */
		static IsometricTileType ShadowCasterTiles[5];

		/*
		 * This points to the shape file holding the shadows that cliff and slope tiles cast
		 * onto the cells below them. It is read in when a theater is loaded and thrown away
		 * when the next theater replaces it.
		 */
		static void * CellShadowShapes;

		struct TileInsertType {
			/*
			 * This is the first tile index the insertion affects. Anything numbered below it
			 * predates the extra tiles and needs no adjustment.
			 */
			int Count;

			/*
			 * This is how many tiles the insertion added, and thus how far forward every
			 * index at or past the insertion point must be shifted.
			 */
			int Offset;
		};

		/*
		 * These record where a theater's tile sets have grown since the tiles were first
		 * numbered -- one entry for each set that gained tiles, in ascending order.
		 * Fixup_Tile_Type walks them to bring an older tile index into line with the
		 * numbering the loaded theater actually uses.
		 */
		static DynamicVectorClass<TileInsertType *> TileInsertTypes;
};

#define TILE_CLEAR IsometricTileTypeClass::ClearTile

extern ShapeSet * SlopeZShapes[4];

inline IsometricTileType operator += (IsometricTileType & t1, int t2)
{
	t1 = (IsometricTileType)(((int)t1 + (int)t2));
	return(t1);
}

IsometricTileType Pick_Ice_Tile_Set(void);

inline Point2D IsoTile_Dimensions(const IsometricTileTypeClass * isotype)
{
	int w = ((IsoTileSet const *)isotype->Get_Image_Data())->Map_Width();
	int h = ((IsoTileSet const *)isotype->Get_Image_Data())->Map_Height();
	return(Point2D(w,h));
}

inline Point2D IsoTile_Dimensions2(const IsoTileSet * set)
{
	int w = set->Map_Width();
	int h = set->Map_Height();
	return(Point2D(w,h));
}
