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

/* $Header: /CounterStrike/CELL.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : CELL.H                                                       *
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

#include "_map.h"
#include "abstract.h"
#include "globals.h"
#include "map.h"
#include "rect.h"

#include "isotype.hh"
#include "land.hh"
#include "mzone.hh"
#include "overlay.hh"
#include "smudge.hh"
#include "tiberium.hh"

class FoggedObjectClass;
class LightConvertClass;
class TagClass;
class TiberiumClass;
class BuildingClass;
class UnitClass;
class TechnoClass;
class TerrainClass;
class IsometricTileTypeClass;
template<class T> class DynamicVectorClass;

typedef DynamicVectorClass<FoggedObjectClass *> FOGGED_OBJECT_LIST;

/****************************************************************************
**	Each cell on the map is controlled by the following structure.
*/
class CellClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		/*
		**	This is the ID number of this cell. By placing the ID number here, it doesn't have
		**	be calculated. Calculating this number requires a divide and would occur about
		**	5.72031 bijillion times per second.
		*/
		Cell CellID;

		/*
		 * When the cell falls under fog, the objects standing in it are replaced by
		 * snapshots so the player keeps seeing what was there when he last looked. This
		 * is the list of those snapshots, and it stays NULL until the first one is needed.
		 */
		FOGGED_OBJECT_LIST * FoggedObjects;

		/*
		 * On a cell beneath a bridge deck (that does not host the deck piece itself),
		 * points to the bridge deck cell governing this location. NULL on deck cells
		 * and non-bridge cells.
		 */
		CellClass * BridgeDeckCell;

		/// Unused
		CellClass * UnusedCell;

		/*
		 * This is the converter that supplies the remap tables the cell's terrain is drawn
		 * through, chosen to match its tint values. Cells that light alike share one, so
		 * it is reference counted rather than owned.
		 */
		LightConvertClass * Drawer;

		/*
		**	This contains the icon number and set to use for the base
		**	of the terrain. All rendering on an icon occurs AFTER the icon
		**	specified by this element is rendered. It is the lowest of the low.
		*/
		IsometricTileType ITType;

		/*
		 * Pointer to the tag attached to this cell, or NULL if it carries none. A unit
		 * that enters a tagged cell springs the tag's trigger, which is how the scenario
		 * detects a unit reaching or crossing a particular place.
		 */
		TagClass * Tag;

		/*
		**	The second layer of 'terrain' icons is represented by a simple
		**	type number and a value byte. This is sufficient for handling
		**	concrete and walls.
		*/
		OverlayType Overlay;

		/*
		**	This is used to specify any special 'stain' overlay icon. This
		**	typically includes infantry bodies or other temporary marks.
		*/
		SmudgeType Smudge;

		/*
		 * This is how freely the cell can be crossed, derived from the terrain and its
		 * occupiers by Recalc_Passability. The zone builder groups cells of matching
		 * passability, so this value decides the shape of the reachability map as much as
		 * it decides whether any one unit may enter.
		 */
		PassabilityType Passability;

		/*
		**	Smudges and walls need to record ownership values. For walls, this
		**	allows adjacent building placement logic to work. For smudges, it
		**	allows building over smudges that are no longer attached to buildings
		**	in addition to fixing the adjacent placement logic.
		*/
		HousesType Owner;

		/*
		**	This flag tells you what type of infantry currently occupy the
		**	cell or are moving into it.
		*/
		HousesType InfType;
		HousesType BridgeInfType;

		/*
		 * This is the frame this cell was last flagged for redraw on. A cell can be
		 * flagged many times over in the course of one frame, so a repeat within the same
		 * frame is recognized here and dropped rather than drawn a second time.
		 */
		int LastRedrawFrame;

		/// Unused
		int LastUnknownDrawFrame;

		/*
		 * These record the frame and the clipping rectangle the bridge deck over this
		 * cell was last drawn with. A deck piece spans several cells and would otherwise
		 * be drawn once for each of them, so a repeat within the same frame and the same
		 * window is skipped.
		 */
		int LastBridgeDrawFrame;
		Rect LastBridgeDrawRect;

		/*
		 * These are bit lists of which houses have cloaked, sensed or built over this
		 * cell -- one bit per house. Cloaking and sensing are per house because an object
		 * hidden from one player may be plainly visible to another, and the occupation
		 * bits let base placement logic tell whose structures already stand here.
		 */
		unsigned CloakedBy;
		unsigned SensedBy;
		unsigned OccupiedBy;

	private:

		/*
		**	These point to the object(s) that are located in this cell or overlap
		**	this cell.
		*/
		ObjectClass * OccupierPtr;
		ObjectClass * BridgeOccupierPtr;

		/*
		**	The land type of this cell.
		*/
		LandType Land;

	public:

		/*
		 * These are the lighting values in force on this cell, recomputed by Init_Drawer
		 * whenever the light sources over it change. Intensity and Ambient say how strongly
		 * the cell is lit, the brightness values scale the tile and the objects standing on
		 * it, and the tints shift their color. Caching them here spares the renderer a fresh
		 * gather of nearby light sources for every cell it draws.
		 */
		int Intensity;
		short Ambient;
		short Brightness;
		short TileBrightness;
		short AltBrightness;
		short RedTint;
		short GreenTint;
		short BlueTint;

		/*
		 * If a tunnel passes through this cell, then this is the tube it belongs to.
		 * Otherwise it is -1. Pathfinding follows it to cross terrain that is otherwise
		 * unreachable.
		 */
		short Tube;

		/*
		 * This is the map redraw counter as of the last bridge draw, recorded alongside
		 * LastBridgeDrawFrame so that a redraw forced from outside the normal frame
		 * sequence still repaints the deck.
		 */
		unsigned char LastBridgeDrawRedraws;

		/*
		 * If ice is permitted to form over this cell, then this flag will be true. It is
		 * a property of the scenario rather than of the terrain, so it survives in the
		 * saved game.
		 */
		unsigned char IsIceGrowthAllowed;

		/*
		 * This is which piece of its isometric tile the cell holds. A tile covers several
		 * cells, so this index selects the artwork, the sub position bits and the slope
		 * that belong to this corner of it.
		 */
		unsigned char SubTile;

		/*
		 * This is the ground level of the cell, expressed in whole height levels. It is
		 * what the zone builder compares between neighbors to decide whether ground is
		 * smooth enough to walk between.
		 */
		char Height;

		/*
		 * If the cell slopes, then this is its ramp shape, biased by one so that zero
		 * means level ground. It selects both the tile artwork and the depth shape used
		 * to sort objects standing on the slope.
		 */
		unsigned char Ramp;

		/*
		 * This is how far the cell's artwork rises above the nominal tile height, expressed
		 * in quarter tile steps. Tall terrain such as a cliff face overhangs the cells behind
		 * it, so the renderer consults this to decide which of them must be redrawn with it.
		 */
		char Elevation;

		/*
		 * This is the value byte that goes with the overlay type. For walls it records
		 * the damage stage and the shape needed to join the neighboring wall sections;
		 * for tiberium it records how much has grown here.
		 */
		unsigned char OverlayData;

		/*
		 * This is the value byte that goes with the smudge type. A smudge larger than a
		 * single cell uses it to record which piece of the stain this cell holds.
		 */
		unsigned char SmudgeData;

		/*
		 * These are the shroud and fog shape frames for this cell, as returned by Cell_Shadow.
		 * They record how much of the cell is covered and from which direction, so the piece
		 * can be drawn without examining the neighborhood again. A frame of -1 means the cell
		 * is fully revealed, and -2 means it is solid black.
		 */
		char ShadowFrame;
		char FogFrame;

		/*
		 * This is the number of objects standing in the eight cells adjacent to this one.
		 * A search may leave its subzone corridor around a cell with a non-zero count, so
		 * a unit can still work its way past an obstacle that the corridor steers clear of.
		 */
		unsigned char AdjacentObjectCount;

		/*
		**	This array of bit flags is used to indicate which sub positions
		**	within the cell are either occupied or are soon going to be
		**	occupied. For vehicles, the cells that the vehicle is passing over
		**	will be flagged with the vehicle bit. For infantry, the the sub
		**	position the infantry is stopped at or headed toward will be marked.
		**	The sub positions it passes over will NOT be marked.
		*/
		union {
			struct {
				unsigned Center:1;
				unsigned NW:1;
				unsigned NE:1;
				unsigned SW:1;
				unsigned SE:1;
				unsigned Vehicle:1;		// Reserved for vehicle occupation.
				unsigned Monolith:1;	// Some immovable blockage is in cell.
				unsigned Building:1;	// A building of some time (usually blocks movement).
			} Occupy;
			unsigned char Composite;
		} Flag;

		/*
		 * This is the same set of occupation flags, but for the bridge deck rather than
		 * for the ground. A cell spanned by a bridge has two independent surfaces that
		 * objects can stand on, so each needs its own record of which sub positions are
		 * taken. Which one applies to an object is decided by whether it is traveling
		 * over the bridge or beneath it.
		 */
		union {
			struct {
				unsigned Center:1;
				unsigned NW:1;
				unsigned NE:1;
				unsigned SW:1;
				unsigned SE:1;
				unsigned Vehicle:1;		// Reserved for vehicle occupation.
				unsigned Monolith:1;	// Some immovable blockage is in cell.
				unsigned Building:1;	// A building of some time (usually blocks movement).
			} Occupy;
			unsigned char Composite;
		} BridgeFlag;

		/*
		**	Does this cell need to be updated on the radar map?  If something changes in the cell
		**	that might change the radar map imagery, then this flag will be set. It gets cleared
		**	when the cell graphic is updated to the radar map.
		*/
		unsigned IsPlot:1;

		/*
		**	Does this cell contain the special placement cursor graphic?  This graphic is
		**	present when selecting a site for building placement.
		*/
		unsigned IsCursorHere:1;

		/*
		**	A mapped cell has some portion of it visible. Maybe it has a shroud piece
		**	over it and maybe not.
		*/
		unsigned IsMapped:1;

		/*
		**	A visible cell means that it is completely visible with no shroud over
		**	it at all.
		*/
		unsigned IsVisible:1;

		/*
		 * A visible cell means that it is completely visible with no fog over
		 * it at all.
		 */
		unsigned IsFogVisible:1;

		/*
		 * A mapped cell has some portion of it visible. Maybe it has a fog piece
		 * over it and maybe not.
		 */
		unsigned IsFogMapped:1;

		/*
		**	Every cell can be assigned a waypoint.  A waypoint can only be assigned
		**	to one cell, and vice-versa.  This bit simply indicates whether this
		**	cell is assigned a waypoint or not.
		*/
		unsigned IsWaypoint:1;

		/*
		**	Is this cell currently under the radar map cursor?  If so then it
		**	needs to be updated whenever the map is updated.
		*/
		unsigned IsRadarCursor:1;

		/*
		**	If this cell contains a house flag, then this will be true. The actual house
		**	flag it contains is specified by the Owner field.
		*/
		unsigned IsFlagged:1;

		/*
		**	This is a working flag used to help keep track of what cells should be
		**	shrouded. By using this flag it allows a single pass through the map
		**	cells for determining shadow regrowth logic.
		*/
		unsigned IsToShroud:1;

		/*
		 * This is a working flag used to help keep track of what cells should be
		 * fogged. By using this flag it allows a single pass through the map
		 * cells for determining fog regrowth logic.
		 */
		unsigned IsToFog:1;

		/*
		 * This cell hosts a bridge deck piece: it carries the bridge overlay and
		 * draws the raised deck (lifted by BRIDGE_CELL_HEIGHT). This is the center
		 * cell of a bridge row.
		 */
		unsigned IsBridgeDeck:1;

		/*
		 * This cell is covered by a bridge overlay (on this cell, or on a
		 * neighbor cell).
		 */
		unsigned IsUnderBridge:1;

		/*
		 * A walking unit may stand on, or move onto and off of, the bridge deck at
		 * this cell; it gates the bridge-height step during pathfinding. The far
		 * end of a span is deliberately excluded.
		 */
		unsigned IsBridgeTraversable:1;

		/*
		 * If this cell was spanned by a bridge that has since been destroyed, then this
		 * flag will be true. It preserves the footprint of the wrecked span so that the
		 * bridge can be located and repaired, and so bridge height still applies here.
		 */
		unsigned WasUnderBridge:1;

		/*
		 * Orientation of the bridge deck at this cell. Set for bridges placed facing
		 * FACING_N and clear for those placed facing FACING_W; this selects which
		 * adjacent cell continues the bridge body.
		 */
		unsigned IsBridgeEastWest:1;

		/*
		 * The rendered bridge deck surface covers this cell's footprint (the
		 * under-bridge shadow shifted one cell forward, since the deck is drawn
		 * raised). It applies the deck-height offset when resolving which cell a
		 * coordinate falls on.
		 */
		unsigned IsBridgeSurface:1;

		/*
		 * If the bridge spanning this cell has taken damage, then this flag will be true.
		 * It selects the cracked artwork for the deck and for the bridge end pieces, so
		 * that the damage shows before the span actually collapses.
		 */
		unsigned IsBridgeDamaged:1;

		/*
		 * This is a working flag used to help keep track of what cells should grow ice.
		 * Candidates are marked in one pass over the map cells and thickened in the next,
		 * so ice grown on one cell cannot go on to seed another within the same pass.
		 */
		unsigned IsToGrowIce:1;

		/*
		 * This is a working flag used to help keep track of what cells should grow veins.
		 * Candidates are marked while the veinhole's spread is being gathered and cleared
		 * once it has been applied, so no cell can be grown twice in one pass.
		 */
		unsigned IsToGrowVeins:1;

		/*
		 * This cell lies in the shadow of an overhead structure -- a bridge deck
		 * above it, or a shadow-casting isometric tile. Objects drawn on an
		 * overshadowed cell are darkened.
		 */
		unsigned IsOvershadowed:1;

		/*
		 * An ambient animation is currently attached to this cell (for example a
		 * vein-attack or tile overlay animation). Prevents attaching a second one;
		 * the animation clears the flag when it is removed.
		 */
		unsigned IsAnimAttached:1;

		/*
		 * Marks a cell lying on a unit's predicted movement path. Toggled by the A*
		 * pathfinding code so that other units route around cells already claimed by
		 * a planned path.
		 */
		unsigned IsPredictedPath:1;

		/*
		 * If this cell lies within the radius of an active electromagnetic pulse, then this
		 * flag will be true. Nothing consults it, so whether an object standing here is
		 * disrupted is decided by the object rather than by the cell it occupies.
		 */
		unsigned IsAffectedByEMP:1;

		/*
		 * This cell lies on a horizontal trigger line -- the map row aligned with a
		 * tagged cell whose trigger watches for a horizontal crossing. A unit
		 * entering such a cell fires the TEVENT_CROSS_HORIZONTAL triggers on that
		 * row. Rebuilt on each map overpass.
		 */
		unsigned IsHorizontalLine:1;

		/*
		 * This cell lies on a vertical trigger line -- the map column aligned with a
		 * tagged cell whose trigger watches for a vertical crossing. A unit entering
		 * such a cell fires the TEVENT_CROSS_VERTICAL triggers on that column.
		 * Rebuilt on each map overpass.
		 */
		unsigned IsVerticalLine:1;

		/*
		 * If this cell is currently hidden under the fog of war, then this flag will be
		 * true. Any structure standing in a fogged cell is replaced by a snapshot in the
		 * FoggedObjects list, so the player keeps seeing whatever was last visible here.
		 */
		unsigned IsFogged:1;

		//----------------------------------------------------------------
		CellClass(void);
		virtual ~CellClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual int What_Am_I(void) const override;

		/// Hides AbstractClass::Detach rather than overriding it -- the signatures differ.
		void Detach(AbstractClass const * target);

		virtual RTTIType Fetch_RTTI(void) const override;

		/*
		**	Query functions.
		*/
		LEPTON Get_Height(Point2D const & point = Point2D(CELL_LEPTON_W / 2, CELL_LEPTON_H / 2)) const;
		bool Can_Tiberium_Germinate(TiberiumClass const * tiberium) const;
		bool Can_Tiberium_Grow(void) const;
		bool Can_Tiberium_Spread(void) const;
		bool Is_Bridge_Here(void) const;
		BuildingClass * Cell_Building(void) const;
		Cell Fetch_CellID(void) const {return(CellID);}
		void Set_CellID(Cell const & cell);
		Coord Cell_Coord(void) const;
		Coord Closest_Free_Spot(Coord const & coord, bool any=false, bool bridge=false) const;
		Coord Free_Spot(void) const {return(Closest_Free_Spot(Cell_Coord()));}
		CellClass & Adjacent_Cell(FacingType face) {return((CellClass &)((*((CellClass const *)this)).Adjacent_Cell(face)));}
		CellClass const & Adjacent_Cell(FacingType face) const;
		AircraftClass * Cell_Aircraft(bool bridge=false) const;
		InfantryClass * Cell_Infantry(bool bridge=false) const;
		LandType Land_Type(void) const {return(Land);}
		ObjectClass * Cell_Find_Object(RTTIType rtti, bool bridge=false) const;
		ObjectClass * Cell_Object(Point2D const & xy=Point2D(0, 0), bool bridge=false) const;
		ObjectClass * Cell_Occupier(bool bridge=false) const {return(bridge ? BridgeOccupierPtr : OccupierPtr);}
		ObjectClass * Cell_Bridge_Occupier(void) const { return(BridgeOccupierPtr); }
		TechnoClass * Cell_Techno(Point2D const & xy=Point2D(0, 0), bool bridge=false, TechnoClass const * notthis=NULL) const;
		TerrainClass * Cell_Terrain(bool bridge=false) const;
		UnitClass * Cell_Unit(bool bridge=false) const;
		bool Goodie_Check(FootClass * object);
		bool Is_Clear_To_Build(SpeedType loco = SPEED_TRACK, BuildingTypeClass * what = NULL, HouseClass * who = NULL) const;
		bool Is_Clear_To_Move(SpeedType loco, bool ignoreinfantry, bool ignorevehicles, int zone=-1, MZoneType check=MZONE_NORMAL, int cell_height=-1, bool checkbridge=true) const;
		bool Is_Spot_Free(int spot_index, bool bridge=false) const;
		void Cell_Color(RGBClass & lowcolor, RGBClass & highcolor) const;
		int Preview_Cell_Color(unsigned char & unknown, bool terrainonly) const;
		int Clear_Icon(IsometricTileType type, int seq_length) const;
		static int Spot_Index(Coord const & coord);
		bool Has_Tunnel(void) const;
		bool Is_Near_Tunnel_NW(void) const;
		bool Is_Near_Tunnel_ES(void) const;
		bool Can_Enter_Tunnel(FootClass const * foot) const;
		TubeClass * Get_Tunnel(void) const;
		char const * Tiberium_Name(void) const;
		TiberiumType Tiberium_Type_Here(void) const;
		bool Place_Tiberium(TiberiumType tib, int data);
		int Tiberium_Value(void) const;
		LEPTON Occupier_Height(void) const;
		FacingType Bounce_Direction(Coord const & target_coord) const;

		virtual Coord Center_Coord(void) const override;
		virtual Coord As_Coord(void) const override;

		bool Is_Cloaked(HousesType house) const;
		bool Is_Sensed(HousesType house) const;
		void Cloaked_By(HousesType house);
		void Uncloaked_By(HousesType house);
		void Sensed_By(HousesType house);
		void Unsensed_By(HousesType house);
		bool Should_Draw_As_Cloaked(HousesType house) const;

		int Get_Vein_Frame(void) const;
		bool Can_Place_Veins(void);
		void Redraw_Veins(void);
		void Place_Veins(void);
		void Trigger_Veins(void);

		bool Is_Tile_Water(void) const;
		bool Is_Tile_Clear(void) const;
		bool Is_Tile_Ramp(void) const;
		bool Is_Tile_Cliff(void) const;
		bool Is_Tile_Shore(void) const;
		bool Is_Tile_With_Water(void) const;
		bool Is_Tile_Swamp(void) const;
		bool Is_Tile_Misc_Pavement(void) const;
		bool Is_Tile_Pavement(void) const;
		bool Is_Tile_Dirt_Road(void) const;
		bool Is_Tile_Paved_Road(void) const;
		bool Is_Tile_Paved_Road_End(void) const;
		bool Is_Tile_Paved_Road_Slope(void) const;
		bool Is_Tile_Road_Median(void) const;
		bool Is_Tile_Ice(void) const;
		bool Is_Tile_Bridge(void) const;
		bool Is_Tile_Train_Bridge(void) const;
		bool Is_Tile_Clear_To_Sand_LAT(void) const;
		bool Is_Tile_Clear_To_Green_LAT(void) const;
		bool Is_Tile_Destroyable_Cliff(void) const;

		bool Fixup_LAT(void);

		bool Is_Fogged(void) const;
		bool Is_Shrouded(void) const;
		bool Can_Build_Here(void) const;
		int Occupation_Mask(HousesType house) const;

		/*
		**	Object placement and removal flag operations.
		*/
		void Occupy_Down(ObjectClass * object, bool bridge=false);
		void Occupy_Up(ObjectClass * object, bool bridge=false);
		bool Flag_Place(HousesType house);
		bool Flag_Remove(void);

		/*
		**	Display and rendering controls.
		*/
		void Wipe_Depth(Point2D const & point, Rect const & cliprect);
		void Draw_Shroud_And_Fog(Point2D const & point, Rect const & cliprect);
		void Draw_Shadow_Cast(Point2D const & point, Rect const & cliprect);
		void Draw_It(Point2D const & xy, Rect const & cliprect, bool objects=false) const;
		void Shimmer(void);
		void Init_Drawer(LightConvertClass * drawer = NULL, int intensity = 0x10000, int ambient = 0, int brightness = 1000, int tile_brightness = 1000, int alt_brightness = 1000);
		void Pick_Drawer(LightConvertClass * & drawer, int & intensity, int & ambient, int & brightness, int & tile_brightness, int & alt_brightness) const;
		void Inc_Drawer_Ref_Count(LightConvertClass * convert) const;
		void Dec_Drawer_Ref_Count(LightConvertClass * convert) const;
		Rect Overlay_Render_Rect(void) const;
		Rect Overlay_Shadow_Render_Rect(void) const;
		Rect Cell_Render_Rect(void) const;
		bool Draw_Placement_Cursor(Point2D const & xpoint, Rect const & cliprect, bool zeroalpha);
		void Draw_Shroud_Or_Fog_Shape(Point2D const & xpoint, Rect const & cliprect, int index);
		void Draw_Fog_Shape(Point2D const & xpoint, Rect const & cliprect, int index);
		void Draw_Overlay_Shadow(Point2D const & xpoint, Rect const & cliprect);
		void Draw_Overlay(Point2D const & point, Rect const & cliprect);
		Point2D Overlay_Draw_Offset(void) const;
		void Register_As_Dirty(void);
		void Fetch_Icon(IsometricTileTypeClass *& ittype, int& subtile, int * icon, bool doicon) const;

		/*
		**	Maintenance calculation support.
		*/
		bool Grow_Tiberium(void);
		bool Spread_Tiberium(bool forced=false);
		int Tiberium_Adjust(bool pregame=false);
		void Set_Wall_Owner(void);
		void Wall_Update(bool justneighbors=false);
		bool Has_Wall_Or_Gate(OverlayType type=OVERLAY_NONE, FacingType facing=FACING_NONE) const;
		BuildingClass * Get_Gate(void) const;
		void Recalc_Attributes(int cell_height=-1);
		void Recalc_Passability(void);
		void Destroy_Bridge(void);
		void On_Bridge_Collapse(void);
		void Set_Under_Bridge(FacingType facing, bool state = true);
		void Set_Under_Rail_Bridge(FacingType facing, bool state = true);
		int  Reduce_Tiberium(int levels);
		int  Reduce_Weed(void);
		int  Reduce_Wall(int damage);
		void Incoming(Coord const & threat=COORD_NONE, bool forced=false, bool nokidding=false, bool bridge=false);
		void Adjust_Threat(HousesType house, int threat_value);
		void Force_New_Slope_For_Occupiers(void) const;
		void Kill_Illegal_Occupiers(void);

		Cell Get_Bridge_Deck_Cell(void) const
		{
			if (IsBridgeDeck) {
				return(Fetch_CellID());
			} else {
				return(BridgeDeckCell->Fetch_CellID());
			}
		}

		CellClass * Get_Bridge_Deck_CellClass(void) const
		{
			CellClass * bridge_deck = NULL;
			if (IsUnderBridge) {
				if (IsBridgeDeck) {
					bridge_deck = &Map[Fetch_CellID()];
				} else {
					bridge_deck = &Map[BridgeDeckCell->Fetch_CellID()];
				}
			}
			return(bridge_deck);
		}

		bool Is_Overlay_Low_Bridge(void) const { return(Overlay >= OVERLAY_LOWBRIDGE_01 && Overlay <= OVERLAY_LOWBRIDGE_26); }
		bool Is_Overlay_Bridge(void) const { return(Overlay == OVERLAY_BRIDGE1 || Overlay == OVERLAY_BRIDGE2); }
		bool Is_Overlay_Rail_Bridge(void) const { return(Overlay == OVERLAY_RAIL_BRIDGE1 || Overlay == OVERLAY_RAIL_BRIDGE2); }

		void Attach_Tag(TagClass *tag);

		void Fog_Cell(void);
		void Unfog_Cell(void);
		void Remove_Fogged_Objects(void);

		void Register_For_Redraw(void);

		bool Can_Burrow_Here(void) const;

		void Init_Light(int & intensity, int & ambient, int & brightness, int & tile_brightness, int & alt_brightness, int & red_tint, int & green_tint, int & blue_tint) const;
		void Recalc_Light(void);

	private:
		CellClass (CellClass const &);
};


inline CellClass * AbstractClass::As_CellClass(void)
{
	return(dynamic_cast<CellClass *>(this));
}


inline CellClass const * AbstractClass::As_CellClass(void) const
{
	return(dynamic_cast<CellClass const *>(this));
}
