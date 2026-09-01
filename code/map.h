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

/* $Header: /CounterStrike/MAP.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MAP.H                                                        *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 29, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 29, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "crate.h"
#include "gscreen.h"
#include "rect.h"
#include "vector.h"

#include "house.hh"
#include "isotype.hh"
#include "mzone.hh"
#include "passblty.hh"
#include "speed.hh"
#include "zone.hh"

class AbstractClass;
class ObjectClass;
class HouseClass;
class Straw;
class Pipe;
class CellClass;
class BuildingTypeClass;
class FootClass;
class SaveStreamClass;
template<typename K, typename V>
class HashTableClass;


class MapClass: public GScreenClass
{
		typedef GScreenClass BASECLASS;

	public:

		MapClass(void);
		virtual ~MapClass(void) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		/*
		 * Isometric playfield <-> cell coordinate conversions
		 */
		Point2D PlayRect_To_Cell_Point(Point2D const & point) const;
		Point2D LocalRect_To_Cell_Point(Point2D const & point) const;
		Point2D Cell_To_PlayRect_Point(Point2D const & point) const;
		Point2D Cell_To_LocalRect_Point(Point2D const & point) const;
		Cell PlayRect_To_Cell(Cell const & cell);
		Cell LocalRect_To_Cell(Cell const & cell);
		Cell Cell_To_PlayRect(Cell const & cell);
		Cell Cell_To_LocalRect(Cell const & cell);

		/*
		 * Cell access, validity and visibility
		 */
		CellClass & operator [] (Coord const & coord) const;
		CellClass & operator [] (Cell const & cell) const;
		int ID(CellClass * ptr) {return(Array.ID(ptr));};
		int ID(CellClass & ptr) {return(Array.ID(&ptr));};
		bool Is_Valid(Cell const & cell);

		/*
		**	Initialization
		*/
		virtual void One_Time(void) override;     // Theater-specific inits
		virtual void Init_Clear(void) override;   // Clears all to known state
		virtual void Alloc_Cells(void);           // Allocates buffers
		virtual void Free_Cells(void);            // Frees buffers
		virtual void Init_Cells(void);            // Frees buffers

		/*--------------------------------------------------------
		**	Main functions that deal with groupings of cells within the map or deals with the cell
		**	as it relates to the map - not what the cell contains.
		*/
		void Fresh_Map(void);
		void Sight_From(Coord const & coord, int sightrange, HouseClass * house, bool incremental=false, bool=false, bool=false, bool=true);
		bool In_Radar(Cell const & cell) const;
		bool In_Radar(Coord const & coord) const;
		void Place_Down(Cell const & cell, ObjectClass * object);
		void Pick_Up(Cell const & cell, ObjectClass * object);
		int Overpass(void);
		ObjectClass * Close_Object(Coord const & coord) const;
		Cell Nearby_Location(Cell const & cell, SpeedType speed, int zone=-1, MZoneType check=MZONE_NORMAL, bool checkbridge=false, Point2D dimensions=Point2D(1,1), bool checkoverlay=false, bool checkheight=false, bool checkburrow=false, bool allowunderbridge=true, Cell const & nearto=Cell(0, 0)) const;
		bool Base_Region(Cell const & cell, HousesType & house, ZoneType & zone) const;
		int Cell_Region(Cell const & cell);
		int Cell_Threat(Cell const & cell, HouseClass const & house);
		Cell Pick_Random_Location(void) const;
		bool Place_Random_Crate(void);
		bool Remove_Crate(Cell const & cell);
		void Shroud_The_Map(void);
		void Reveal_The_Map(void);
		virtual void Detach(AbstractClass const * , bool all = true);

		/*
		 * Movement zones
		 */
		bool Is_Same_Cell_Zone(Cell const & from, Cell const & to, MZoneType mzone=MZONE_NORMAL, bool isbridge1=false, bool isbridge2=false, bool leavemap=false);
		int Get_Cell_Zone(Cell const & cell, MZoneType mzone=MZONE_NORMAL, bool bridge=false);
		int Get_Cell_Zone_ID(Cell const & cell);
		int Get_Cell_Zone_Index(Cell const & cell);
		int Get_Cell_Subzone_Index(Cell const & cell);
		void Update_Cell_Zone(Cell const & cell);
		void Update_Cell_Zone_Constructively(Cell const & cell);
		int Zone_Reset(void);
		int Zone_Span(CellZoneStruct * data, int zone, int & skip);
		void Compute_Zone_Connections(void);
		int Zone_Connection_Index(Cell const & cell, int maxdist, int index = 0);
		bool Register_Subzone_Connections(Cell const & cell);
		bool Unregister_Subzone_Connections(Cell const & cell);

		/*
		 * Zone / subzone connection graph
		 */
		void Reset_All_Subzones(void);
		void Reset_Subzone(int subzone);
		int Subzone_Span(CellSubzoneStruct * seed, int subzone_level, int subzone_id, Rect const & bounds, Cell const & cell);
		void Register_Subzone_Zone_Connections(int subzone);
		void Register_Zone_Connection_Entries(ZoneConnectionClass & connection, int index);
		Cell Get_Bridge_Zone_Connection_Cell(CellClass * cptr, bool isbridge);
		Cell Get_Zone_Connection_Destination(Cell const & cell, Cell const & reference);
		Cell Find_Bridge_Span_End_Cell(Cell const & cell, Cell const & reference);
		Cell Find_Bridge_End_Cell_For_Subzone(Cell const & cell, int subzone_level, int subzone_id);
		bool Build_Reachable_Subzones(CellClass * cptr, int subzone_level, DynamicVectorClass<unsigned short> const & connections, FootClass const * foot);
		void Update_Cell_Subzones(Cell const & cell);
		void Register_Subzone_Connection(ZoneConnectionClass * connection);
		void Unregister_Subzone_Connection(ZoneConnectionClass * connection);

		/*
		 * Bridge damage, destruction and repair
		 */
		bool Damage_Bridge(Cell const & cell);
		bool Damage_Low_Bridge(Cell const & cell);
		void Repair_Train_Bridge(Cell const & cell);
		void Repair_Bridge(Cell const & cell);
		bool Can_Repair_Bridge(Cell const & cell);

	private:

		/*
		 * Bridge tile state helpers
		 */
		void Set_Bridge_End_State(Cell const & cell, bool damaged, bool recursive);
		void Set_Bridge_Middle_State(Cell const & cell, IsometricTileType new_tile, IsometricTileType match_tile, int cell_height, bool recursive);

		/*
		 * Train / high per-direction damage and destruction walkers
		 */
		void Internal_Damage_Train_Bridge_EW_BottomRight(Cell const & cell, FacingType dir);
		void Internal_Damage_Train_Bridge_EW_TopLeft(Cell const & cell, FacingType dir);
		void Internal_Destroy_Train_Bridge_EW_BottomRight(Cell const & cell, FacingType dir);
		void Internal_Destroy_Train_Bridge_EW_TopLeft(Cell const & cell, FacingType dir);
		void Internal_Damage_Train_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir);
		void Internal_Damage_Train_Bridge_NS_TopRight(Cell const & cell, FacingType dir);
		void Internal_Destroy_Train_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir);
		void Internal_Destroy_Train_Bridge_NS_TopRight(Cell const & cell, FacingType dir);
		void Internal_Damage_High_Bridge_EW_BottomRight(Cell const & cell, FacingType dir);
		void Internal_Damage_High_Bridge_EW_TopLeft(Cell const & cell, FacingType dir);
		void Internal_Destroy_High_Bridge_EW_BottomRight(Cell const & cell, FacingType dir);
		void Internal_Destroy_High_Bridge_EW_TopLeft(Cell const & cell, FacingType dir);
		void Internal_Damage_High_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir);
		void Internal_Damage_High_Bridge_NS_TopRight(Cell const & cell, FacingType dir);
		void Internal_Destroy_High_Bridge_NS_BottomLeft(Cell const & cell, FacingType dir);
		void Internal_Destroy_High_Bridge_NS_TopRight(Cell const & cell, FacingType dir);

		/*
		 * Train and high bridge span damage and destruction
		 */
		bool Damage_Train_Bridge(Cell const & cell);
		bool Destroy_Train_Bridge_Span(Cell const & cell, FacingType dir, Rect * dirty);
		void Destroy_Train_Bridge_Connections(Cell const & cell);
		bool Damage_High_Bridge(Cell const & cell);
		bool Destroy_High_Bridge_Span(Cell const & cell, FacingType dir, Rect * dirty);
		void Destroy_High_Bridge_Connections(Cell const & cell);
		void Spring_Bridge_Destruction_Triggers(Cell cell1, Cell cell2);

		/*
		 * Train and high bridge span repair walkers
		 */
		void Repair_High_Bridge_Span(CellClass * cptr, FacingType dir, Rect * dirty = NULL);
		void Repair_Train_Bridge_Span(CellClass * cptr, FacingType dir, Rect * dirty);
		void Repair_All_High_Bridges(void);
		void Repair_High_Bridge_From_Cell(Cell const & cell);
		void Repair_Train_Bridge_From_Cell(Cell const & cell);

		/*
		 * Low bridge piece damage and repair
		 */
		static int Get_Low_Bridge_EW_Neighbor_Mask(Cell const & cell);
		static int Get_Low_Bridge_NS_Neighbor_Mask(Cell const & cell);
		bool Damage_Low_Bridge_EW(Cell const & cell);
		bool Damage_Low_Bridge_NS(Cell const & cell);
		void Damage_Low_Bridge_Piece_EW(Cell const & cell);
		void Damage_Low_Bridge_Piece_NS(Cell const & cell);
		void Spring_Low_Bridge_Destroyed_EW(Cell cell);
		void Spring_Low_Bridge_Destroyed_NS(Cell cell);
		void Repair_Low_Bridge_Span(Cell const & cell);
		void Repair_Low_Bridge_EW(Cell const & cell);
		void Repair_Low_Bridge_NS(Cell const & cell);
		static bool Is_Low_Bridge(Cell const & cell);

	public:

		/*
		 * Ice
		 */
		void Smoothen_Ice(Cell const & cell, bool smooth_shore);
		void Smoothen_Full_Ice(Cell const & cell, bool not_cracked);
		void Smoothen_Ice_Shore(Cell const & cell);
		void Smoothen_Ice_Edge(Cell const & cell);
		bool Crack_Ice(CellClass * cellptr, FootClass * object);
		bool Break_Ice(CellClass * cellptr, FootClass * object);
		static bool Is_Tile_Water_Ice(IsometricTileType tile);
		bool Ice_Growth_AI(void);
		void Ice_Solidification_AI(void);
		void Recalc_Ice_Cells(void);

		/*
		 * Procedural map generation: cliffs and shores
		 */
		void Mark_Fill_Area(Cell const & origin, bool on_water);
		bool Rebuild_Cliffs_At(Cell const & cell);
		bool Build_All_Cliffs(int, int region_id);
		bool Expand_High_Ground(CellClass * cellptr, int region_id);
		bool Place_Cliff(CellClass * cellptr, int region_id);
		int Get_High_Ground_Mask(CellClass * cellptr);
		bool Rebuild_Shores_At(Cell const & origin);
		bool Build_All_Shores(int region_id);
		bool Expand_Water(CellClass * cellptr, int region_id);
		void Prune_Water(CellClass * cellptr);
		bool Place_Shore(CellClass * cellptr, int pass, int region_id);
		int Get_Water_Mask(CellClass * cellptr, int control);
		static int Straight_Shore_Length(CellClass * cellptr, FacingType direction);
		bool Pick_Random_Tile_Variant(IsometricTileType tile, int max_tile, int base_height, int seed, bool & success);
		static bool Is_Tile_Shore(IsometricTileType tile);
		static bool Is_Tile_Cliff(IsometricTileType tile, int subtile);

		/*
		 * Terrain deformation and cell recalculation
		 */
		bool Deform_Terrain(Cell const & cell, bool forced);
		void Terrain_Deformation_AI(void);
		void Collapse_Cliff(CellClass * cptr);
		void Area_Reduce_Tiberium(Cell const & cell);
		void Recalc_Cells_In_Rect(Rect const & rect);
		void Recalc_Cells_In_List(DynamicVectorClass<Cell> const & vec);
		void Set_WasUnderBridge_Flags(void);

		/*
		 * Cell iteration and local radar
		 */
		CellClass * Iterate(void);
		void Reset_Iterator(void);
		CellClass * Local_Iterate(void);
		void Reset_Local_Iterator(void);
		CellClass * Get_Local_Grid_Cell(Cell const & cell, int spacing, bool unbiased);
		bool In_Local_Radar(Rect const & rect, bool useheight = true) const;
		bool In_Local_Radar(Cell const & cell, bool useheight = true) const;
		bool In_Local_Radar(CellClass const * cell, bool useheight = true) const;
		bool In_Local_Radar(Coord const & rect) const;
		bool In_Area_Radar(Rect const & rect, Cell const & cell, bool useheight = true) const;
		int Cell_To_Local_Sector_Index(Cell const & cell);
		Cell Local_Sector_Index_To_Cell(int index);

		/*
		 * Fog and shroud
		 */
		bool Is_Shrouded(Coord const & coord);
		bool Is_Fogged(Coord const & coord);
		void Init_Fog_System(void);
		void Deinit_Fog_System(void);
		void Reveal_Nearby_Technos(CellClass * cptr, HouseClass * house, bool onradar);
		CellClass * Find_Nearby_Shroud(FootClass * foot);

		/*
		 * Miscellaneous map queries and helpers
		 */
		int Get_Height_GL(Coord const & coord);
		void Increment_Redraw_Counter(void);
		bool Try_Open_Gate(FootClass * foot, Cell const & cell);
		bool Is_Something_Nearby(Cell const & cell, int radius);
		int Region_Threat(HouseClass * house, int level, int from_subzone, int to_subzone);
		bool Is_Area_Available(Rect const & rect, int house);
		bool Is_Clear_To_Move(Cell const & cell, int width, int height, SpeedType speed, int zone, MZoneType check, int cell_height, bool checkbridge, bool block_overlays);
		Cell Closest_Edge_Cell(Cell const & cell, bool inset = false);
		Cell Clip_To_Map(Cell const & cell);
		Cell Closest_Passable_Cell(Cell const & tarcell, Cell const & objcell);
		bool Check_Map_Integrity(void);
		void Initialize_Wall_Ownership(void);
		Coord Firestorm_On_Path(Coord const & from, Coord const & to, HouseClass * house);
		void Place_Firestorm_Wall(Cell const & cell, HouseClass * owner, BuildingTypeClass * type);
		void Place_Wall(Cell const & cell, HouseClass * owner, BuildingTypeClass * type);
		void Shutdown(void);

		/*
		 * Binary map save / load
		 */
		int Write_Binary_1(Pipe & pipe);
		int Write_Binary_2(Pipe & pipe);
		int Write_Binary_3(Pipe & pipe);
		int Write_Binary_4(Pipe & pipe);
		int Write_Binary_5(Pipe & pipe);
		bool Read_Binary_1(Straw & straw);
		bool Read_Binary_2(Straw & straw);
		bool Read_Binary_3(Straw & straw);
		bool Read_Binary_4(Straw & straw);
		bool Read_Binary_5(Straw & straw);

		/*
		**	Debug routine
		*/
		int Validate(void);

		virtual bool Is_Scrolling(void) const {return(false);};
		virtual void Logic(void);
		virtual void Set_Map_Dimensions(Rect const & size, bool, int cell_height, bool);
		virtual void Set_Local_Dimensions(Rect const & size);

		/*
		 * This is the set of base terrain zones that touch one another, keyed by a packed
		 * pair of zone IDs. It is filled while the zones are being flood filled, and gains a
		 * pair for every bridge and tunnel that is currently intact, so that the zones can
		 * then be unioned into the movement zones of each MZoneType.
		 */
		ZONE_PAIR_HASH_SET * ZoneAdjacency;

		/*
		 * This records the movement zones for this map. Cells share the same zone
		 * number if they are contiguous (terrain consideration only) for a given
		 * style of movement. There is a separate zone layer for each movement type
		 * (see MZoneType). Each layer maps a base terrain-zone ID to its movement-zone
		 * number, so two cells are mutually reachable by a given movement type only
		 * when their zones map to the same number.
		 */
		unsigned short * Zones[MZONE_COUNT];

		/*
		 * This is the number of base terrain zones the last rebuild produced, and thus the
		 * length of every one of the Zones layers. Zone number 0 is never handed out to a
		 * cell -- it stands for "outside the playfield or not yet assigned".
		 */
		int ZoneCount;

		/*
		 * These are the bridge spans and tunnels that link two cells lying in separate
		 * terrain zones. Each one is what lets a movement zone reach across the water or
		 * cliff between its ends, so a connection whose span has been destroyed is left out
		 * of the zone and subzone graphs until it is repaired.
		 */
		DynamicVectorClass<ZoneConnectionClass> ZoneConnections;

		/*
		 * These are the per-cell zone and subzone layers, one entry each for every cell of the
		 * rotated playfield square (CellZoneCount of them). CellZones records a cell's base
		 * terrain zone along with the passability and height that zone was grown from, and
		 * CellSubzones its subzone at each level of pathfinding coarseness.
		 */
		CellZoneStruct * CellZones;
		int CellZoneCount;
		CellSubzoneStruct * CellSubzones;

		/*
		 * This is the subzone graph itself, one slot per level of coarseness. SubzoneTracking
		 * holds a level's subzone records and SubzoneTrackingEntryCount how many of them are
		 * valid, while SubzoneConnectionHashTable is only scratch: a rebuild gathers adjacency
		 * pairs there so that duplicates fall out, then unpacks them into the records and
		 * clears it again.
		 */
		int SubzoneTrackingEntryCount[SUBZONE_COUNT];
		SUBZONE_CONNECTION_HASH_SET * SubzoneConnectionHashTable[SUBZONE_COUNT];
		DynamicVectorClass<SubzoneTrackingStruct> SubzoneTracking[SUBZONE_COUNT];

		/*
		 * These are the bridge cells that the damage pass in progress has destroyed. A cell
		 * coming down kills its occupiers and disturbs its neighbors, so the collapse itself
		 * is held back until the whole span has been resolved.
		 */
		DynamicVectorClass<Cell> PendingBridgeCells;

		/*
		 * These are the cells whose ice has been broken since the last recalculation. Ice
		 * cracks a cell at a time, but the attribute and zone bookkeeping it disturbs is far
		 * cheaper to redo in one go, so the cells are collected here and brought up to date
		 * once the frame's damage has been tallied.
		 */
		DynamicVectorClass<Cell> DirtyIceCells;

		/*
		 * This is the size of the whole isometric playfield -- the rotated "diamond" of cells
		 * the map is laid out in -- with its origin forced to (0,0). The In_Radar diamond
		 * test, the radar map and the zone layer's cell indexing are all sized from it.
		 */
		Rect PlayRect;

		/*
		 * This is the playable sub-region of the playfield, as the scenario declares it: the
		 * requested area intersected with PlayRect and then pulled a couple of cells further
		 * in. In_Local_Radar tests against it, so a cell outside it is treated as lying beyond
		 * the edge of the map even though it is part of the playfield.
		 */
		Rect LocalRect;

		unsigned IterX;				/// Iterator cell X
		unsigned IterY;				/// Iterator cell Y
		unsigned IterColumn;		/// Iterator position on row (column number)
		CellClass ** IterCell;		/// Iterator cell
		int LocalIterX;				/// Local iterator cell X
		int LocalIterY;				/// Local iterator cell Y

		/*
		**	This is the dimensions and position of the sub section of the global map.
		**	It is this region that appears on the radar map and constrains normal
		**	movement.
		*/
		Rect MapRect;

		/*
		**	This is the total value of all harvestable Tiberium on the map.
		*/
		int TotalValue;

		/*
		 * Tracks a cracked ice cell awaiting re-solidification. SolidifyFrame is the
		 * game frame at which the cell turns back into full ice.
		 */
		struct CrackedIceStruct
		{
			CrackedIceStruct(int solidify_frame = 0) : CellID(0,0), SolidifyFrame(solidify_frame) {}

			bool operator==(const CrackedIceStruct & that) const { return(that.CellID == CellID && that.SolidifyFrame == SolidifyFrame); }
			bool operator!=(const CrackedIceStruct & that) const { return(that.CellID != CellID || that.SolidifyFrame != SolidifyFrame); }

			// Carries the cracked ice record to or from a save game.
			template<typename S>
			void Serialize(S & stream)
			{
				stream.Serialize(CellID);
				stream.Serialize(SolidifyFrame);
			}

			/*
			 * This is the cell whose ice was cracked by something crossing it.
			 */
			Cell CellID;

			/*
			 * This is the game frame at which the cell may freeze back to solid ice, set
			 * IceSolidifyDelay frames ahead when the crack happens. Every cracked cell
			 * heals on its own schedule, so the ice recovers raggedly rather than all at
			 * once.
			 */
			int SolidifyFrame;
		};

	protected:

		/*
		 * These record the unfinished half of a terrain deformation. When an explosion
		 * craters a cell, one corner of it slumps immediately and DeformMask keeps the
		 * corners still to come at DeformCell, which are reshaped once the game frame
		 * reaches DeformFrame. The delay is what makes the ground appear to cave in rather
		 * than snap into its new shape.
		 */
		int DeformMask;
		Cell DeformCell;
		int DeformFrame;

		/*
		 * These are the ice cells that are currently cracked, each one waiting for the frame
		 * at which it may freeze over again. Only the cells on this list have to be revisited
		 * as the ice recovers, rather than every cell of the playfield.
		 */
		DynamicVectorClass<CrackedIceStruct> CrackedIce;

	public:
		/*
		**	This is the array of cell objects.
		*/
		VectorClass<CellClass *> Array;

	protected:
		/*
		 * These are the tables that drive a scan of the cells around a point, laid out as
		 * rings of increasing distance: RadiusOffset holds the cell offsets in scan order,
		 * RadiusCount the running total through each radius (0 - 10) so that an incremental
		 * scan can start at an outer ring, and OcclusionOffset the neighbor of each offset
		 * whose height decides whether that cell lies hidden behind higher ground.
		 */
		static int const RadiusCount[11];
		static Cell const RadiusOffset[];
		static Cell const OcclusionOffset[];

		/*
		**	These are the size dimensions of the underlying array of cell objects.
		**	This is the dimensions of the "map" that the tactical view is
		**	restricted to.
		*/
		int	XSize;
		int	YSize;
		int	Size;

		/*
		**	This specifies the information for the various crates in the game.
		*/
		CrateClass Crates[256];

	protected:
		friend class CellClass;

		/*
		 * This counter advances every time the screen is flagged for a redraw. A cell that
		 * caches an expensive draw -- the bridge deck is the one that does -- remembers the
		 * count it last drew under, so it can tell a fresh redraw from being asked to draw
		 * a second time within the same one.
		 */
		unsigned char Redraws;

		/*
		 * These are the cells that currently have a trigger Tag attached to them. Detaching a
		 * tag has to reach every cell that references it, and walking this short list is far
		 * cheaper than sweeping the whole map for it.
		 */
		DynamicVectorClass<Cell> TaggedCells;
};

extern CellClass BlubCell;

int SubzoneHash(unsigned int const & key);
extern int MZonePassability[MZONE_COUNT][PASSABLE_COUNT];
