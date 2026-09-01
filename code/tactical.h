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

#include "abstract.h"
#include "matrix3d.h"
#include "rect.h"
#include "stimer.h"
#include "timer.h"

#include <vector>

template<class T> class DynamicVectorClass;
class CellClass;
class Cell;
class ShadowControlClass;
class Surface;

/*
 * Identifies which rendering pass Tactical::Render() should perform. A frame is composed from a
 * sequence of passes: the view is scrolled and the cached surfaces swapped (DRAW_PASS_PAN), the
 * terrain/tile/building layers are redrawn into the cached surface (DRAW_PASS_BACKGROUND), and
 * finally the dynamic objects, units and tactical overlays are drawn on top (DRAW_PASS_FOREGROUND).
 * DRAW_PASS_FULL performs every step in a single call.
 */
enum DrawPassType {
	DRAW_PASS_PAN = 0,
	DRAW_PASS_BACKGROUND = 1,
	DRAW_PASS_FOREGROUND = 2,
	DRAW_PASS_FULL = 3
};

class ShadowControlClass
{
	public:
		ShadowControlClass(void) {}
		ShadowControlClass(ShadowControlClass const & that)
		{
			Unknown1 = that.Unknown1;
			Unknown2 = that.Unknown2;
			Unknown3 = that.Unknown3;
			Unknown4 = that.Unknown4;
			Unknown5 = that.Unknown5;
			Unknown6 = that.Unknown6;
			Unknown7 = that.Unknown7;
		}

		/// Unused
		int Unknown1;
		int Unknown2;
		int Unknown3;
		int Unknown4;
		int Unknown5;
		Rect Unknown6;
		int Unknown7;
};

struct DirtyAreaStruct
{
	/*
	 * This is the region of the tactical view that has to be redrawn, expressed in pixels
	 * relative to the upper left corner of the view.
	 */
	Rect Area;

	/*
	 * If the shroud and the alpha lighting over the area have to be refreshed as well as the
	 * terrain beneath them, then this flag will be true. An area that only needs its terrain
	 * redrawn leaves them alone, which keeps that work off the common path.
	 */
	bool IsToRefreshShroud;

	DirtyAreaStruct(void) : Area(0,0,0,0), IsToRefreshShroud(false) {}
	DirtyAreaStruct(Rect const & area, bool refresh_shroud) : Area(area), IsToRefreshShroud(refresh_shroud) {}

	DirtyAreaStruct(DirtyAreaStruct const & that) : Area(that.Area), IsToRefreshShroud(that.IsToRefreshShroud) {}

	bool operator == (DirtyAreaStruct const &that) const { return(IsToRefreshShroud == that.IsToRefreshShroud && Area == that.Area); }
	bool operator != (DirtyAreaStruct const &that) const { return(IsToRefreshShroud != that.IsToRefreshShroud || Area != that.Area); }

};


class Tactical : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		/*
		 * Constructors, destructors, and persistence.
		 */
		Tactical(void);
		virtual ~Tactical(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TACTICALMAP);}

		virtual void Serialize(SaveStreamClass & stream) override;

		/*
		 * Per frame processing and teardown.
		 */
		virtual void AI(void) override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;

		/*
		 * Coordinate conversion.
		 */
		static int Z_Lepton_To_Pixel(LEPTON lepton);
		static LEPTON Pixel_To_Z_Lepton(int pixel);

		bool Coord_To_Pixel(Coord const & coord, Point2D & pixel);
		Coord Pixel_To_Coord(Point2D const & pixel);
		Coord Pixel_To_Coord_Absolute(Point2D const & pixel);
		Cell Pixel_To_Cell(Point2D const & pixel);

		static Point2D Rectangular_To_Isometric(Point2D point);
		Point2D Coord_To_Pixel_Absolute(Coord const & coord);
		Point2D Coord_To_Pixel_Absolute(Point2D const & point);
		Point2D Pixel_To_Lepton(Point2D const & pixel);

		static void Rectangular_To_Isometric(int xin, int yin, int & xout, int & yout);
		static void Isometric_To_Rectangular(int xin, int yin, int & xout, int & yout);
		void Cell_To_Rectangular(int xin, int yin, int & xout, int & yout);
		void Cell_To_Rectangular_Centered(int xin, int yin, int & xout, int & yout);
		void Lepton_To_Map_Pixel(int xin, int yin, int & xout, int & yout);

		Cell Coord_To_Cell(Coord const & coord);

		/*
		 * View position, dimensions, and scrolling.
		 */
		void Set_View_Dimensions(Rect const & dimensions);

		void Set_Tactical_Position(Point2D const & point);
		void Set_Tactical_Position(Coord const & coord);
		Point2D Get_Tactical_Position(void);
		Point2D Get_Relative_Tactical_Position(void);

		void Tactical_Position_Limits(Point2D & minimum, Point2D & maximum);
		bool Clamp_To_Tactical_Rect(Point2D & pixel);
		Point2D Clamp_Pixel_To_Tactical(Point2D const & pixel);

		void Scroll_Map(FacingType facing, int distance);
		FacingType Scroll_Dir(FacingType dir);
		void Setup_Trigger_Scroll(Coord const & coord, int speed);

		/*
		 * Redraw registration.
		 */
		void Register_Dirty_Area(Rect area, bool refresh_shroud = false);
		void Reset_Dirty_Rectangles(void);
		void Register_Dirty_Buildings(Rect area);
		void Flag_Cell(CellClass & cell);

		/*
		 * Object selection and the rubber band.
		 */
		void Select_These(Rect const & rect, void (*select_callback)(ObjectClass * object) = NULL);
		ObjectClass * Get_Selectable_Object(Point2D const & point);

		bool Contains_Selectable_Buildings(Rect rect);
		void Add_Buildings_To_Selectable(Rect rect);

		bool Add_To_Selectables(ObjectClass * object, Point2D point);

		void Start_Rubber_Band(Point2D const & point);
		void Modify_Rubber_Band(Point2D const & point);
		void Select_Rubber_Band(void (*select_callback)(ObjectClass * object));
		void End_Rubber_Band(void);
		void Draw_Rubber_Band(void);

		/*
		 * Terrain rendering passes.
		 */
		void Wipe_Depth(bool fullredraw, int xoff, int yoff, Rect const & cliprect);
		void Render_Tiles(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw);
		void Render_Tile_Shadows(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw);
		void Render_Overlays(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw);
		void Render_Fogged_Objects(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw);
		void Render_Shroud(Rect const & xpanrect, Rect const & ypanrect, Rect const & cliprect, bool fullredraw);
		void Render_Buildings(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw);
		void Render_Terrain(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw);
		void Render_Outside_Map(Surface & surface);
		void Render(Surface & surface, bool fullredraw, int drawpass);
		bool Blit_Tiles(void);

		/*
		 * Map and object drawing.
		 */
		virtual bool Draw_3D_Line(Coord const & coord1, Coord const & coord2, int color, bool write_depth);
		void Draw_Overlays(Rect const & area);
		void Draw_Shroud(Rect const & area);
		void Draw_Tiles(Rect const & area, Rect const & cliprect);
		void Draw_Tile_Shadows(Rect const & area, Rect const & cliprect);
		void Draw_Tiles(Cell const & cell, Rect const & cliprect);
		void Draw_Tile_Shadows(int, Rect const & area);

		static void Draw_Render_Hooks(ObjectClass * object, Rect const & cliprect);
		void Draw_Objects(bool forced);
		void Draw_Terrain(bool forced, Rect const & area, Rect const & cliprect);
		void Draw_Fogged_Objects(Rect const & area);
		void Draw_Buildings(bool forced, Rect const & area, Rect const & cliprect);

		/*
		 * Placement previews.
		 */
		void Draw_Placement(bool drawtrans);
		void Draw_Fence_Placement(bool drawtrans, Cell cell);
		void Draw_Firestorm_Wall_Placement(bool drawtrans, Cell cell);
		void Draw_Wall_Placement(bool drawtrans, Cell cell);

		/*
		 * Cell queries.
		 */
		int Get_Cell_Height(Coord const & coord);
		int Get_Cell_Ramp(Coord const & coord);
		CellClass *Find_Bridge_Owner_Cell(Cell const & cell);
		void Add_Shadow_Control(ShadowControlClass control);

		int Cell_Shadow(Cell const & cell, bool fog);
		bool Is_Cell_Visible (Cell const & cell);
		void Update_Visible_Cells(void);

		/*
		 * Caption text and map overlays.
		 */
		void Set_Caption_Text(int text_id);
		void Clear_Caption_Text(void);
		void Draw_Screen_Text(char const * text);

		void Draw_Rally_Points(bool inshroud);
		void Draw_Waypoints(bool inshroud);
		static void Draw_Radial_Indicators(void);

		/*
		 * Debug overlays.
		 */
		void Debug_Draw_Occupier_Links(void);
		void Debug_Draw_Occupation_Flags(void);

#ifdef _DEBUG
		void Debug_Draw_Waypoint_Markers(void);
		void Debug_Draw_Current_Mouse_Cell(void);
#endif

		Cell Clamp_Cursor_To_Tactical(Cell const & cell, Cell const * list);

		void Noop(int);

	public:
		/*
		 * This is the caption text drawn across the middle of the tactical view. It is empty
		 * whenever there is no caption to show, which is how the message is taken down again.
		 * The scenario won and scenario lost announcements are put up through it.
		 */
		char ScreenText[64];

		/*
		 * This is the game frame the per-frame housekeeping was last done on. The AI routine
		 * may be called more than once in a frame, so this ensures that a trigger scroll and
		 * the waypoint animation only advance once per frame however often it is called.
		 */
		int LastAIFrame;

		/// Unused
		bool field_58;
		bool field_59;

		/*
		**	This is the pixel offset for the upper left corner of the tactical map.
		*/
		int TacPixelX;
		int TacPixelY;
		int LastTacPixelX;
		int LastTacPixelY;

		/*
		 * This is the magnification the tactical view is presented at. At 1.0 the map appears
		 * at its natural size; a larger factor takes a smaller region from the center of the
		 * rendered surface and stretches it to fill the tactical rectangle.
		 */
		double ZoomFactor;

		struct Selectable {

			/*
			 * Pointer to the object this entry was created for. The list is only as fresh as
			 * the last render, so an entry may name an object that is no longer active.
			 */
			ObjectClass * Object;

			/*
			 * This is the pixel the object was drawn at, expressed in absolute tactical
			 * pixels rather than relative to the view. The click search converts the click
			 * to match and then takes the entry nearest to it.
			 */
			Point2D Position;
		};

		/*
		 * These are the objects that drew themselves during the last render pass and can
		 * therefore be clicked on, in the order they rendered. It is emptied at the start
		 * of every pass, since the objects build it up again as they draw.
		 */
		static std::vector<Selectable> SelectableObjects;

		/*
		 * These carry a trigger driven scroll of the view. The position is interpolated from
		 * MoveFrom to MoveTo as MoveFactor climbs from 0 to 1 by MoveSpeed each frame, and all
		 * four are cleared once it arrives. This is what makes the scroll glide, not jump.
		 */
		Point2D MoveFrom;
		Point2D MoveTo;
		float MoveSpeed;
		float MoveFactor;

		/*
		 * These are the cells flagged for redraw since the last render. A pass that is
		 * not a full redraw refreshes only these cells, which keeps an ordinary frame
		 * off the cost of redrawing the whole view.
		 */
		std::vector<CellClass *> CellRedraw;

		/*
		 * The tactical map display position is expressed in absolute pixels, taken about
		 * the center of the view. This should not be altered directly. Use the
		 * Set_Tactical_Position function instead.
		 */
		Point2D TacticalCoord;

		/*
		 * This is the position the tactical map was displaying when it was last rendered.
		 * Comparing it against TacticalCoord tells the render pass whether the view moved
		 * since then, so a pan that cancels itself out does not rebuild the cached layers.
		 */
		Point2D LastTacticalCoord;

		/*
		**	This is the coordinate that the tactical map should be in at next available opportunity.
		*/
		Point2D DesiredTacticalCoord;

		/*
		 * If the tactical map has yet to complete a render pass, then this flag will be true.
		 * Nothing has been drawn while it is set, so the LastTacticalCoord comparison means
		 * nothing yet and a bare pixel pan is enough to force the cached layers to rebuild.
		 */
		bool IsFirstRender;

		/*
		**	If something in the tactical map is to be redrawn, this flag is set to true.
		*/
		bool IsToRedraw;

		/// Unused
		bool UnusedBool;

		/*
		 * This is the range of cells the tactical view covers, expressed in cell coordinates
		 * and padded by a tile or two along each edge. The render passes and Is_Cell_Visible
		 * work from this rectangle rather than examining every cell on the map.
		 */
		Rect VisibleCellRect;

		/*
		 * These are the two corners of the "rubber band" the player is dragging out over the
		 * tactical map, expressed in pixels relative to the view. Both are reset to 0,0 when
		 * the band ends, and a start corner of 0,0 is what marks the band as not in progress.
		 */
		Point2D RubberBandStart;
		Point2D RubberBandEnd;

		/*
		 * These animate the waypoint path markers drawn over the map. The counter selects
		 * which frame of the waypoint cursor to draw and steps on every time the timer
		 * expires, which is then restarted from the WaypointAnimationSpeed rule.
		 */
		int WaypointAnimCounter;
		CDTimerClass<SystemTimerClass> WaypointAnimTimer;

		/*
		 * This is the forward isometric projection, built at construction from a 60 degree
		 * tilt, a 45 degree rotation and the tile-to-cell scale. The forward conversions work
		 * from the integer Rectangular_To_Isometric instead, so nothing reads it.
		 */
		Matrix3D CoordToPixelMatrix;

		/*
		 * This is the inverse isometric projection, which turns a screen pixel offset back
		 * into world lepton X and Y. It is built once at construction, and every screen to
		 * world conversion the tactical map makes runs through it.
		 */
		Matrix3D PixelToCoordMatrix;

		/*
		 * These are the regions of the tactical view that have to be refreshed on the next
		 * background pass. Overlapping regions are merged as they are registered, and the
		 * whole list is discarded once the cached layers have been brought up to date.
		 */
		static DynamicVectorClass<DirtyAreaStruct> DirtyAreas;

		/*
		 * These are the shadow controls handed to the tactical map. The list owns every control
		 * in it -- each is copied onto the heap as it is added.
		 */
		static DynamicVectorClass<ShadowControlClass *> ShadowControls;
};


#ifdef _DEBUG
extern bool DrawOccupierLinks;
extern bool OccupationBitPrint;
extern bool PassabilityPrint;
#endif
