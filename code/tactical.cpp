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

#define INCLUDE_COM
#include "always.h"

#include "tactical.h"

#include "_alpha.h"
#include "_convert.h"
#include "_font.h"
#include "_map.h"
#include "_rect.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "_zbuffer.h"
#include "alphashp.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "builtype.h"
#include "cell.h"
#include "convert.h"
#include "coord.h"
#include "data.h"
#include "dialog.h"
#include "display.h"
#include "draw.h"
#include "dsurface.h"
#include "fog.h"
#include "font.h"
#include "globals.h"
#include "house.h"
#include "incdec.h"
#include "inline.h"
#include "ionblast.h"
#include "isotype.h"
#include "laser.h"
#include "layer.h"
#include "mixfile.h"
#include "mouse.h"
#include "overtype.h"
#include "ovrlight.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "session.h"
#include "stimer.h"
#include "sun.h"
#include "terrain.h"
#include "terrtype.h"
#include "vector.h"
#include "vein.h"
#include "waypoint.h"
#include "zbuffer.h"

#include "ramp.hh"
#include "scrspeed.hh"

#include <utility>


extern Point2D Lerp(Point2D const & a, Point2D const & b, float t);

#ifdef _DEBUG
bool DrawOccupierLinks = false;
bool OccupationBitPrint = false;
bool PassabilityPrint = false;
#endif


Rect TacticalDimensions(0,0,0,0);	/// the rectangle the tactical view occupies, set by Set_View_Dimensions
std::vector<Tactical::Selectable> Tactical::SelectableObjects;


DynamicVectorClass<DirtyAreaStruct> Tactical::DirtyAreas;

DynamicVectorClass<ShadowControlClass *> Tactical::ShadowControls;


/// <summary>
/// Constructs the tactical map and prepares its coordinate transforms.
/// This routine builds the forward and inverse isometric projections that the view uses to
/// move between world coordinates and screen pixels, and publishes the new object as the
/// global tactical map that the rest of the game talks to.
/// </summary>
Tactical::Tactical(void) :
	LastAIFrame(-1),
	field_58(0),
	field_59(0),
	TacPixelX(0),
	TacPixelY(0),
	LastTacPixelX(0),
	LastTacPixelY(0),
	ZoomFactor(1.0),
	MoveFrom(COORD_NONE),
	MoveTo(COORD_NONE),
	MoveSpeed(0),
	MoveFactor(0),
	TacticalCoord(0,0),
	LastTacticalCoord(0,0),
	DesiredTacticalCoord(0,0),
	IsFirstRender(true),
	IsToRedraw(false),
	UnusedBool(false),
	VisibleCellRect(RECT_NONE),
	RubberBandStart(0,0),
	RubberBandEnd(0,0),
	WaypointAnimCounter(0),
	WaypointAnimTimer()
{
	ScreenText[0] = '\0';

	CoordToPixelMatrix.Make_Identity();
	CoordToPixelMatrix.Rotate_X(RAD_60);
	CoordToPixelMatrix.Rotate_Z(RAD_45);
	CoordToPixelMatrix.Scale(ISO_TILE_PIXEL_W / CELL_LEPTON_DIAG);

	/*
	 * PixelToCoordMatrix is the inverse isometric projection: it maps a screen-pixel offset
	 * (relative to the tactical view origin) back to world-lepton XY. Pixel X is scaled by the
	 * cell-height ratio and pixel Y by the cell-width ratio; the first row sums the two terms
	 * (world X) and the second row takes their difference (world Y).
	 */
	float x_scale = (float)(CELL_LEPTON_W / CELL_PIXEL_W) + 0.6667f;
	float y_scale = (float)(CELL_LEPTON_H / CELL_PIXEL_H) + 0.3333302f;
	PixelToCoordMatrix.Set(
		 y_scale, x_scale, 0.0f, 0.0f,
		-y_scale, x_scale, 0.0f, 0.0f,
		 0.0f,    0.0f,    1.0f, 0.0f);

	TacticalMap = this;
}


/// <summary>
/// Destroys the tactical map and drops the global pointer to it.
/// </summary>
Tactical::~Tactical(void)
{
	TacticalMap = NULL;
}


/*
 * Coordinate conversion pipeline.
 * The tactical view maps between four spaces:
 * Cell             - integer map grid cell.
 * Coord            - world position in leptons (CELL_LEPTON per cell); Z is height.
 * Pixel            - screen pixel relative to the visible view (TacPixel subtracted).
 * Pixel (absolute) - pixel in the full virtual-map space (no TacPixel offset).
 * Forward (world -> screen):
 * Coord --Rectangular_To_Isometric--> iso-leptons --/CELL_LEPTON-->
 * absolute pixel --(Z lift)--> --(- TacPixel)--> screen pixel
 * Rectangular_To_Isometric  - pure 45-degree grid->diamond rotation (no scale/offset).
 * Coord_To_Pixel_Absolute   - Coord -> absolute pixel (rotation + lepton->pixel + Z lift).
 * Coord_To_Pixel            - Coord -> screen pixel (+ visibility test); subtracts TacPixel.
 * Z_Lepton_To_Pixel         - height lepton -> vertical pixel lift.
 * Inverse (screen -> world):
 * Pixel_To_Lepton           - absolute pixel -> world-lepton XY (applies PixelToCoordMatrix).
 * Pixel_To_Coord(_Absolute) - pixel -> Coord; the non-absolute form adds TacPixel first.
 * Pixel_To_Cell             - screen pixel -> Cell.
 * Pixel_To_Z_Lepton         - vertical pixel -> height lepton.
 * Coord_To_Cell - height-aware coord -> cell (picking).
 * Only the inverse uses a matrix (PixelToCoordMatrix); the forward path uses the integer
 * Rectangular_To_Isometric. CoordToPixelMatrix is the forward matrix but is currently unused.
 */


/// <summary>
/// Rotates a rectangular grid point onto the isometric axes.
/// This is the low level projection routine that the Coord to pixel conversions are built
/// from. The result keeps whatever scale went in, so leptons come back as leptons.
/// </summary>
/// <param name="point">The rectangular grid point, in lepton or cell units.</param>
/// <returns>Returns with the point rotated onto the isometric screen axes.</returns>
Point2D Tactical::Rectangular_To_Isometric(Point2D point)
{
	Point2D iso;
	iso.X = point.X * ISO_TILE_PIXEL_W / 2;
	iso.Y = point.X * ISO_TILE_PIXEL_H / 2;
	iso.X += point.Y * ISO_TILE_PIXEL_W / -2;
	iso.Y += point.Y * ISO_TILE_PIXEL_H / 2;
	return(iso);
}


/// <summary>
/// Projects a world Coord to an absolute (virtual-space) screen pixel: isometric rotation,
/// lepton-to-pixel divide, then a Z-height lift. Unlike Coord_To_Pixel it does not subtract the
/// tactical-view origin, so the result is in absolute pixel space.
/// </summary>
/// <param name="coord">World coordinate in leptons (X, Y, Z).</param>
/// <returns>Absolute screen-pixel position.</returns>
Point2D Tactical::Coord_To_Pixel_Absolute(Coord const & coord)
{
	Point2D pixel = Rectangular_To_Isometric(Point2D(coord.X, coord.Y));
	pixel.X /= CELL_LEPTON;
	pixel.Y /= CELL_LEPTON;
	pixel.Y -= Z_Lepton_To_Pixel(coord.Z);
	return(pixel);
}


/// <summary>
/// Projects a height-less world point to an absolute screen pixel.
/// This is the flat companion to the Coord form, for callers that have no height to lift by.
/// The tactical view origin is not subtracted, so the result is in absolute pixel space.
/// </summary>
/// <param name="point">The world point, in leptons.</param>
/// <returns>Returns with the absolute screen-pixel position.</returns>
Point2D Tactical::Coord_To_Pixel_Absolute(Point2D const & point)
{
	Point2D pixel = Rectangular_To_Isometric(point);
	pixel /= CELL_LEPTON;
	int lift = Z_Lepton_To_Pixel(0);
	return(pixel);
}


/// <summary>
/// Inverse projection primitive: maps an absolute screen pixel back to world-lepton XY by applying
/// PixelToCoordMatrix. Drops Z. Backs Pixel_To_Coord and Pixel_To_Coord_Absolute.
/// </summary>
/// <param name="pixel">Absolute screen-pixel position.</param>
/// <returns>World-lepton XY (no height).</returns>
Point2D Tactical::Pixel_To_Lepton(Point2D const & pixel)
{
	Vector3 vector(pixel.X, pixel.Y, 0.0);
	vector = PixelToCoordMatrix * vector;
	return(Point2D(vector.X, vector.Y));
}


/// <summary>
/// Converts a height in leptons into a vertical pixel lift.
/// This routine is used by the coordinate conversions to raise an object off the ground plane
/// of the isometric view by its height.
/// </summary>
/// <returns>Returns with the number of pixels the object should be lifted by.</returns>
int Tactical::Z_Lepton_To_Pixel(LEPTON lepton)
{
	int fudge = 0;

	static double pixels_per_lepton = ISO_TILE_PIXEL_W / CELL_LEPTON_DIAG;
	static double z_pixels_per_lepton = std::sin(RAD_60) * pixels_per_lepton;

	if (lepton >= (CELL_LEPTON * 3) + (CELL_LEPTON / 2) + 40) {
		fudge = 1;
	}

	return(int(lepton * z_pixels_per_lepton + fudge + 0.5));
}


/// <summary>
/// Converts a vertical pixel lift back into a height in leptons.
/// This is the reverse of Z_Lepton_To_Pixel.
/// </summary>
/// <returns>Returns with the height that the pixel lift stands for.</returns>
LEPTON Tactical::Pixel_To_Z_Lepton(int pixel)
{
	static double z_leptons_per_pixel = CELL_LEPTON_DIAG / (std::sin(RAD_60) * ISO_TILE_PIXEL_W);

	return(LEPTON(z_leptons_per_pixel * (pixel - 0.5)));
}


/***********************************************************************************************
 * DisplayClass::Coord_To_Pixel -- Determines X and Y pixel coordinates.                       *
 *                                                                                             *
 *    This is the routine that figures out the location on the screen for                      *
 *    a specified coordinate. It is one of the fundamental routines                            *
 *    necessary for rendering the game objects. It performs some quick                         *
 *    tests to see if the coordinate is in a visible region and returns                        *
 *    this check as a boolean value.                                                           *
 *                                                                                             *
 * INPUT:   coord -- The coordinate to check.                                                  *
 *                                                                                             *
 *          x,y   -- Reference to the pixel coordinates that this                              *
 *                   coordinate would be when rendered.                                        *
 *                                                                                             *
 * OUTPUT:  bool; Is this coordinate in a visible portion of the map?                          *
 *                                                                                             *
 * WARNINGS:   If the coordinate is not in a visible portion of the                            *
 *             map, then this X and Y parameters are not set.                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/14/1994 JLB : Created.                                                                 *
 *   12/15/1994 JLB : Converted to member function.                                            *
 *   01/07/1995 JLB : Uses inline functions to extract coord components.                       *
 *   08/09/1995 JLB : Uses new coordinate system.                                              *
 *=============================================================================================*/
#define	EDGE_ZONE_W (ISO_TILE_PIXEL_W*6)
#define	EDGE_ZONE_H	(ISO_TILE_PIXEL_H*6)
bool Tactical::Coord_To_Pixel(Coord const & coord, Point2D & pixel)
{
	pixel = Coord_To_Pixel_Absolute(coord);
	pixel -= Point2D(TacPixelX, TacPixelY);

	if (pixel.X < -EDGE_ZONE_W) {
		return(false);
	}

	if (pixel.X > TacticalDimensions.Width + EDGE_ZONE_W) {
		return(false);
	}

	if (pixel.Y < -EDGE_ZONE_H) {
		return(false);
	}

	if (pixel.Y > TacticalDimensions.Height + EDGE_ZONE_H) {
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * DisplayClass::Pixel_To_Coord -- converts screen coord to COORDINATE                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      x,y      pixel coordinates to convert                                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      COORDINATE of pixel                                                                    *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      none.                                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/09/1994 BR : Created.                                                                  *
 *   12/06/1994 JLB : Uses map dimension variables in display class.                           *
 *   12/10/1994 JLB : Uses union to speed building coordinate value.                           *
 *=============================================================================================*/
Coord Tactical::Pixel_To_Coord(Point2D const & pixel)
{
	/*
	**	If pixel coordinate is over the tactical map, then translate it into a coordinate
	**	value. If not, then just return with NULL.
	*/
	if (pixel.X < TacticalDimensions.Width && pixel.Y < TacticalDimensions.Height + TacticalDimensions.Y) {

		/*
		**	Normalize the pixel coordinates to be relative to the upper left corner
		**	of the tactical map. The coordinates are expressed in leptons.
		*/
		Point2D lepton = Pixel_To_Lepton(pixel + Point2D(TacPixelX, TacPixelY));
		return(Coord(lepton.X, lepton.Y, 0));
	}

	return(COORD_NONE);
}


/// <summary>
/// Converts an absolute screen pixel into a world coordinate.
/// Unlike Pixel_To_Coord, the pixel is taken to already be in absolute (virtual map) space,
/// so the tactical view origin is not added to it first.
/// </summary>
/// <param name="pixel">The absolute pixel position to convert.</param>
/// <returns>Returns with the world coordinate. Otherwise, COORD_NONE is returned if the pixel
/// lies outside the tactical view.</returns>
Coord Tactical::Pixel_To_Coord_Absolute(Point2D const & pixel)
{
	/*
	**	If pixel coordinate is over the tactical map, then translate it into a coordinate
	**	value. If not, then just return with NULL.
	*/
	if (pixel.X < TacticalDimensions.Width && pixel.Y < TacticalDimensions.Height) {

		Point2D lepton = Pixel_To_Lepton(pixel);
		return(Coord(lepton.X, lepton.Y, 0));
	}

	return(COORD_NONE);
}


/// <summary>
/// Starts a smooth scroll of the tactical view toward a coordinate.
/// This routine is used by the trigger system to pan the view to a point of interest. Only
/// the destination and rate are recorded here; the AI routine walks the view there a step at
/// a time over the following frames.
/// </summary>
/// <param name="coord">The coordinate to scroll the view to.</param>
/// <param name="speed">The scroll speed setting to travel at.</param>
void Tactical::Setup_Trigger_Scroll(Coord const & coord, int speed)
{
	static float _speeds[SCROLL_SPEED_COUNT] = {
		0.0015f,
		0.0030f,
		0.0075f,
		0.0300f,
		0.0600f
	};

	MoveFactor = 0;
	MoveSpeed = _speeds[speed];

	Point2D to;
	Coord_To_Pixel(coord, to);
	to.X += TacPixelX;
	to.Y += TacPixelY;
	MoveTo = to;
	MoveFrom = TacticalCoord;
}


/// <summary>
/// Handles the per-frame housekeeping of the tactical view.
/// This routine carries a trigger-driven scroll one step closer to its destination, ticks the
/// waypoint path animation along, and applies any view position that was asked for since the
/// last frame.
/// </summary>
/// <remarks>It is harmless to call this routine more than once in a game frame; only the
/// first call moves anything.</remarks>
void Tactical::AI(void)
{
	bool coordchanged = false;

	if (Session.Play) {
		return;
	}

	if (LastAIFrame != Frame) {
		if (ScenarioActive) {
			if (MoveTo != COORD_NONE && MoveSpeed != 0) {
				MoveFactor += MoveSpeed;
				if (MoveFactor > 1.0) {
					MoveFactor = 1.0;
				}

				Point2D move = Lerp(MoveFrom, MoveTo, MoveFactor);

				if (MoveFactor >= 1.0) {
					MoveTo = Point2D(0,0);
					MoveFrom = Point2D(0,0);
					MoveSpeed = 0;
					MoveFactor = 0;
				}

				Set_Tactical_Position(move);
				coordchanged = true;
			}
		}

		if (WaypointAnimTimer == 0) {
			WaypointAnimTimer = Rule->WaypointAnimationSpeed;
			WaypointAnimCounter++;
		}
	}

	LastAIFrame = Frame;

	if (DesiredTacticalCoord != TacticalCoord && !coordchanged) {
		Set_Tactical_Position(DesiredTacticalCoord);
	}
}


/// <summary>
/// Registers a region of the tactical view as needing a redraw.
/// This routine is called by anything that changes the look of the map. Where an area already
/// on the list can absorb the new one more cheaply than tracking both, the two are merged, so
/// that the render passes have as few regions to walk over as possible.
/// </summary>
/// <param name="area">The region of the view that has to be redrawn.</param>
/// <param name="refresh_shroud">Does the shroud and alpha lighting over the area need refreshing too?</param>
void Tactical::Register_Dirty_Area(Rect area, bool refresh_shroud)
{
	int best_index = -1;
	int best = 0x7FFFFFFF;

	for (int i = 0; i < DirtyAreas.Count(); i++) {

		Rect rect = DirtyAreas[i].Area;
		Rect inter = Intersect(rect, area);
		Rect whole = Union(rect, area);

		if (inter.Is_Valid()) {
			if (inter == area) {
				if (!refresh_shroud || DirtyAreas[i].IsToRefreshShroud) {
					return;
				}
			} else if (inter == rect) {
				if (refresh_shroud || !DirtyAreas[i].IsToRefreshShroud) {
					DirtyAreas.Delete_Index(i);
					i--;
				}
			} else {
				int size = whole.Size();
				if (size < (rect.Size() + area.Size()) && size < best) {
					best = size;
					best_index = i;
				}
			}
		}

	}

	if (best_index == -1) {
		DirtyAreas.Add(DirtyAreaStruct(area, refresh_shroud));
	} else {
		DirtyAreas[best_index] = DirtyAreaStruct(Union(area, DirtyAreas[best_index].Area), refresh_shroud || DirtyAreas[best_index].IsToRefreshShroud);
	}
}


void Tactical_noop(int,int,int,int)
{

}


/// <summary>
/// Clears the depth buffer over the parts of the view that are about to be redrawn.
/// This routine is called by Render before the background pass. The registered dirty areas
/// are shifted by the scroll offset and brought back inside the view first, so this routine
/// also discards the areas that have scrolled away entirely.
/// </summary>
/// <param name="fullredraw">Is the whole view being redrawn anyway?</param>
/// <param name="xoff">Horizontal scroll offset to shift the dirty areas by.</param>
/// <param name="yoff">Vertical scroll offset to shift the dirty areas by.</param>
/// <param name="cliprect">The clipping rectangle for the per-cell depth wipes.</param>
void Tactical::Wipe_Depth(bool fullredraw, int xoff, int yoff, Rect const & cliprect)
{
	if (!fullredraw && DirtyAreas.Count()) {
		for (int i = DirtyAreas.Count() - 1; i >= 0; i--) {
			Rect redraw = DirtyAreas[i].Area;
			redraw += Point2D(xoff, yoff);
			redraw = Intersect(TacticalRect - TacticalRect.TopLeft, redraw);
			DirtyAreas[i].Area = redraw;

			if (!redraw.Is_Valid()) {
				DirtyAreas.Delete_Index(i);
			} else {
				DepthBuffer->Update(redraw);
			}
		}
	}

	if (!CellRedraw.empty() && !fullredraw) {
		for (int i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = CellRedraw[i]->Cell_Coord();
			coord.Z = 0;
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);
			pixel += Point2D(ISO_TILE_PIXEL_W, ISO_TILE_PIXEL_H) / -2;
			pixel += TacticalRect.TopLeft;

			if (MainWindow) {
				CellRedraw[i]->Wipe_Depth(pixel, cliprect);
			}
		}
	}
}


/// <summary>
/// Renders the iso tiles that need refreshing this frame.
/// This routine is called by Render as part of the background pass. The cells flagged for
/// redraw and the registered dirty areas are refreshed, along with either the whole view or
/// only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Tiles(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw)
{
	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		Draw_Tiles(TacticalRect, TacticalRect);
	} else {
		if (xpanrect.Is_Valid()) {
			Draw_Tiles(xpanrect, xpanrect);
		}
		if (ypanrect.Is_Valid()) {
			Draw_Tiles(ypanrect, ypanrect);
		}
	}

	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty() && !fullredraw) {
		for (int i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = Coord_Whole(Coord(CellRedraw[i]->Fetch_CellID()));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);

			Rect rect;
			rect.X = pixel.X - ISO_TILE_PIXEL_W / 2;
			rect.Y = pixel.Y;
			rect.Width = ISO_TILE_PIXEL_W;
			rect.Height = ISO_TILE_PIXEL_H;
			rect.Y += TacticalRect.Y;

			Rect inter = Intersect(TacticalRect, rect);

			if (inter.Is_Valid()) {
				Draw_Tiles(CellRedraw[i]->Fetch_CellID(), rect);
			}
		}
	}

	/*
	 * The areas that were registered as dirty.
	 */
	if (DirtyAreas.Count() && !fullredraw) {
		for (int j = DirtyAreas.Count() - 1; j >= 0; j--) {
			Rect redraw = DirtyAreas[j].Area;
			redraw.Y += TacticalRect.Y;
			redraw = Intersect(TacticalRect, redraw);

			if (redraw.Is_Valid()) {
				Draw_Tiles(redraw, redraw);
			}
		}
	}
}


/// <summary>
/// Renders the iso tile shadows that need refreshing this frame.
/// This routine is called by Render as part of the background pass. The cells flagged for
/// redraw and the registered dirty areas are refreshed, along with either the whole view or
/// only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Tile_Shadows(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw)
{
	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		Draw_Tile_Shadows(TacticalRect, TacticalRect);
	} else {
		if (xpanrect.Is_Valid()) {
			Draw_Tile_Shadows(xpanrect, xpanrect);
		}
		if (ypanrect.Is_Valid()) {
			Draw_Tile_Shadows(ypanrect, ypanrect);
		}
	}

	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty() && !fullredraw) {
		for (int i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = Coord_Whole(Coord(CellRedraw[i]->Fetch_CellID()));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);

			Rect rect;
			rect.X = pixel.X - ISO_TILE_PIXEL_W / 2;
			rect.Y = pixel.Y;
			rect.Width = ISO_TILE_PIXEL_W;
			rect.Height = ISO_TILE_PIXEL_H;
			rect.Y += TacticalRect.Y;

			Rect inter = Intersect(TacticalRect, rect);

			if (inter.Is_Valid()) {
				Draw_Tile_Shadows(inter, inter);
			}
		}
	}

	/*
	 * The areas that were registered as dirty.
	 */
	if (DirtyAreas.Count()) {
		for (int j = DirtyAreas.Count() - 1; j >= 0; j--) {
			Rect redraw = DirtyAreas[j].Area;
			redraw.Y += TacticalRect.Y;
			redraw = Intersect(TacticalRect, redraw);

			if (redraw.Is_Valid()) {
				Draw_Tile_Shadows(redraw, redraw);
			}
		}
	}
}


/// <summary>
/// Renders the cell overlays that need refreshing this frame.
/// This routine is called by Render as part of the background pass. The cells flagged for
/// redraw and the registered dirty areas are refreshed, along with either the whole view or
/// only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Overlays(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw)
{
	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		Draw_Overlays(TacticalRect);
	} else {
		if (xpanrect.Is_Valid()) {
			Draw_Overlays(xpanrect);
		}
		if (ypanrect.Is_Valid()) {
			Draw_Overlays(ypanrect);
		}
	}

	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty() && !fullredraw) {
		for (int i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = Coord_Whole(Coord(CellRedraw[i]->Fetch_CellID()));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);

			Rect rect;
			rect.X = pixel.X - ISO_TILE_PIXEL_W / 2;
			rect.Y = pixel.Y;
			rect.Width = ISO_TILE_PIXEL_W;
			rect.Height = ISO_TILE_PIXEL_H;
			rect.Y += TacticalRect.Y;

			Draw_Overlays(rect);
		}
	}

	/*
	 * The areas that were registered as dirty.
	 */
	if (DirtyAreas.Count()) {
		for (int i = DirtyAreas.Count() - 1; i >= 0; i--) {
			Rect redraw = DirtyAreas[i].Area;
			redraw.Y += TacticalRect.Y;
			redraw = Intersect(TacticalRect, redraw);

			if (redraw.Is_Valid()) {
				Draw_Overlays(redraw);
			}
		}
	}
}


/// <summary>
/// Renders the fogged object images that need refreshing this frame.
/// This routine is called by Render as part of the background pass. The cells flagged for
/// redraw and the registered dirty areas are refreshed, along with either the whole view or
/// only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Fogged_Objects(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw)
{
	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		Draw_Fogged_Objects(TacticalRect);
	} else {
		if (xpanrect.Is_Valid()) {
			Draw_Fogged_Objects(xpanrect);
		}
		if (ypanrect.Is_Valid()) {
			Draw_Fogged_Objects(ypanrect);
		}
	}

	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty() && !fullredraw) {
		for (int i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = Coord_Whole(Coord(CellRedraw[i]->Fetch_CellID()));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);

			Rect rect;
			rect.X = pixel.X - ISO_TILE_PIXEL_W / 2;
			rect.Y = pixel.Y + TacticalRect.Y;
			rect.Width = ISO_TILE_PIXEL_W;
			rect.Height = ISO_TILE_PIXEL_H;

			Draw_Fogged_Objects(rect);
		}
	}

	/*
	 * The areas that were registered as dirty.
	 */
	if (DirtyAreas.Count()) {
		for (int i = DirtyAreas.Count() - 1; i >= 0; i--) {
			Rect redraw = DirtyAreas[i].Area;
			redraw.Y += TacticalRect.Y;
			redraw = Intersect(TacticalRect, redraw);

			if (redraw.Is_Valid()) {
				Draw_Fogged_Objects(redraw);
			}
		}
	}
}


/// <summary>
/// Renders the shroud and fog over the view.
/// This routine is called by Render alongside the other layer renderers. The cells flagged
/// for redraw and the registered dirty areas are covered again, along with either the whole
/// view or only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="cliprect">The clipping rectangle for the per-cell shroud and fog draws.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Shroud(Rect const & xpanrect, Rect const & ypanrect, Rect const & cliprect, bool fullredraw)
{
	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty() && !fullredraw) {
		for (int i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = CellRedraw[i]->Cell_Coord();
			coord.Z = 0;
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);
			pixel += Point2D(ISO_TILE_PIXEL_W, ISO_TILE_PIXEL_H) / -2;
			pixel += TacticalRect.TopLeft;

			if (MainWindow) {
				CellRedraw[i]->Draw_Shroud_And_Fog(pixel, cliprect);
			}
			AlphaShapeClass::Draw_In_Area(pixel, cliprect);
		}
	} else if (fullredraw) {
		Draw_Shroud(TacticalRect);
	}

	/*
	 * The strips the pan exposed. A full redraw has already covered them.
	 */
	if (xpanrect.Is_Valid() && !fullredraw) {
		Draw_Shroud(xpanrect);
	}
	if (ypanrect.Is_Valid() && !fullredraw) {
		Draw_Shroud(ypanrect);
	}

	/*
	 * The areas that were registered as dirty.
	 */
	for (int i = 0; i < DirtyAreas.Count(); i++) {
		Rect redraw = DirtyAreas[i].Area + TacticalRect.TopLeft;
		Rect inter = Intersect(redraw, cliprect);

		if (DirtyAreas[i].IsToRefreshShroud) {
			AlphaBuffer->Update(inter);
			Draw_Shroud(inter);
		}
	}
}


/// <summary>
/// Renders the buildings that need refreshing this frame.
/// This routine is called by Render as part of the background pass. The cells flagged for
/// redraw and the registered dirty areas are refreshed, along with either the whole view or
/// only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Buildings(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw)
{
	int i;

	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty()) {
		for (i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = Coord_Whole(Coord(CellRedraw[i]->Fetch_CellID()));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);

			Rect rect;
			rect.X = pixel.X - ISO_TILE_PIXEL_W / 2;
			rect.Y = pixel.Y;
			rect.Width = ISO_TILE_PIXEL_W;
			rect.Height = ISO_TILE_PIXEL_H;

			Rect cliprect = rect;

			rect.Y += TacticalRect.Y;
			cliprect.Y += TacticalRect.Y;

			Draw_Buildings(true, rect, cliprect);
		}
	}

	/*
	 * The areas that were registered as dirty.
	 */
	if (DirtyAreas.Count()) {
		for (i = DirtyAreas.Count() - 1; i >= 0; i--) {
			Rect redraw = DirtyAreas[i].Area;
			redraw.Y += TacticalRect.Y;
			Rect inter = Intersect(TacticalRect, redraw);
			if (inter.Is_Valid()) {
				Rect cliprect = inter;
				Draw_Buildings(true, inter, cliprect);
			}
		}
	}

	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		Draw_Buildings(true, TacticalRect, TacticalRect);
	} else {
		if (xpanrect.Is_Valid()) {
			Draw_Buildings(true, xpanrect, xpanrect);
		}
		if (ypanrect.Is_Valid()) {
			Draw_Buildings(true, ypanrect, ypanrect);
		}
	}
}


/// <summary>
/// Renders the terrain objects that need refreshing this frame.
/// This routine is called by Render as part of the background pass. The cells flagged for
/// redraw and the registered dirty areas are refreshed, along with either the whole view or
/// only the strips that have just scrolled into it.
/// </summary>
/// <param name="xpanrect">The strip that scrolled in horizontally, if there is one.</param>
/// <param name="ypanrect">The strip that scrolled in vertically, if there is one.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
void Tactical::Render_Terrain(Rect const & xpanrect, Rect const & ypanrect, bool fullredraw)
{
	int i;

	/*
	 * The cells that were flagged for redraw.
	 */
	if (!CellRedraw.empty()) {
		for (i = 0; i < (int)CellRedraw.size(); i++) {
			Coord coord = Coord_Whole(Coord(CellRedraw[i]->Fetch_CellID()));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);

			Rect rect;
			rect.X = pixel.X - ISO_TILE_PIXEL_W / 2;
			rect.Y = pixel.Y;
			rect.Width = ISO_TILE_PIXEL_W;
			rect.Height = ISO_TILE_PIXEL_H;

			Rect cliprect = rect;

			rect.Y += TacticalRect.Y;
			cliprect.Y += TacticalRect.Y;

			Draw_Terrain(true, rect, cliprect);
		}
	}

	/*
	 * The areas that were registered as dirty.
	 */
	if (DirtyAreas.Count()) {
		for (i = DirtyAreas.Count() - 1; i >= 0; i--) {
			Rect redraw = DirtyAreas[i].Area;
			redraw.Y += TacticalRect.Y;
			Rect inter = Intersect(TacticalRect, redraw);

			if (inter.Is_Valid()) {
				Rect cliprect = inter;
				Draw_Terrain(true, inter, cliprect);
			}
		}
	}

	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		Draw_Terrain(true, TacticalRect, TacticalRect);
	} else {
		if (xpanrect.Is_Valid()) {
			Draw_Terrain(true, xpanrect, xpanrect);
		}
		if (ypanrect.Is_Valid()) {
			Draw_Terrain(true, ypanrect, ypanrect);
		}
	}
}


/// <summary>
/// Blacks out the parts of the tactical view that the map does not reach.
/// </summary>
/// <param name="surface">The composite surface to paint on. It must already be locked.</param>
void Tactical::Render_Outside_Map(Surface & surface)
{
	Point2D minimum;
	Point2D maximum;
	Tactical_Position_Limits(minimum, maximum);

	// The box spans everything the view can show across the whole legal scroll range.
	int left = minimum.X - TacticalRect.Width / 2 - TacPixelX;
	int right = maximum.X + TacticalRect.Width / 2 - TacPixelX;
	int top = TacticalRect.Y + minimum.Y - TacticalRect.Height / 2 - TacPixelY;
	int bottom = TacticalRect.Y + maximum.Y + TacticalRect.Height / 2 - TacPixelY;

	if (left > TacticalRect.X) {
		surface.Fill_Rect(Rect(TacticalRect.X, TacticalRect.Y, left - TacticalRect.X, TacticalRect.Height), TBLACK);
	}

	if (right < TacticalRect.X + TacticalRect.Width) {
		surface.Fill_Rect(Rect(right, TacticalRect.Y, TacticalRect.X + TacticalRect.Width - right, TacticalRect.Height), TBLACK);
	}

	if (top > TacticalRect.Y) {
		surface.Fill_Rect(Rect(TacticalRect.X, TacticalRect.Y, TacticalRect.Width, top - TacticalRect.Y), TBLACK);
	}

	if (bottom < TacticalRect.Y + TacticalRect.Height) {
		surface.Fill_Rect(Rect(TacticalRect.X, bottom, TacticalRect.Width, TacticalRect.Y + TacticalRect.Height - bottom), TBLACK);
	}
}


/// <summary>
/// Renders the tactical map for one display pass.
/// A frame is assembled from several passes. The pan pass scrolls the cached surfaces, the
/// background pass rebuilds the terrain layers into them, and the foreground pass draws the
/// objects and overlays on top. A caller with no reason to split the work asks for
/// DRAW_PASS_FULL and gets all of it in one call.
/// </summary>
/// <param name="surface">The composite surface the foreground objects are drawn onto.</param>
/// <param name="fullredraw">Redraw the whole view rather than just the scrolled strips?</param>
/// <param name="drawpass">Which rendering pass to perform. See DrawPassType.</param>
void Tactical::Render(Surface & surface, bool fullredraw, int drawpass)
{
	/*
	 * Set true when the view scrolled further than one screen, so the still-valid region can no
	 * longer be blitted and a full redraw is forced. It persists across the passes of a frame.
	 */
	static bool scrolled_off_screen = false;

	/*
	 * The composite surface to draw the foreground objects onto. This tracks the front buffer
	 * across the surface swap performed during the pan pass.
	 */
	Surface * composite = &surface;

	SelectableObjects.clear();
	bool has_selectable_buildings = Contains_Selectable_Buildings(TacticalRect);

	/*
	 * Work out how far the view has panned since the last frame. A pan smaller than the view can be
	 * satisfied by blitting the still-valid region and only redrawing the strips that scrolled in;
	 * a larger pan forces a full redraw.
	 */
	int xoffset = LastTacPixelX - TacPixelX;
	int yoffset = LastTacPixelY - TacPixelY;

	if (drawpass == DRAW_PASS_PAN) {
		if (abs(xoffset) < TacticalRect.Width && abs(yoffset) < TacticalRect.Height) {
			scrolled_off_screen = false;
		} else {
			fullredraw = true;
			scrolled_off_screen = true;
		}
	} else {
		if (scrolled_off_screen) {
			fullredraw = true;
		}
	}

	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		xoffset = 0;
		yoffset = 0;
	}

	/*
	 * Decide whether the cached layers actually need to be rebuilt this pass. A pending redraw,
	 * selectable buildings, dirty areas or a pixel pan all force a rebuild; but once the first
	 * frame has been rendered, a pan is only honoured if the world coordinate also changed. The
	 * foreground pass never rebuilds the cached layers.
	 */
	bool render;
	/*
	 * Either the whole view, or just the strips the pan exposed.
	 */
	if (fullredraw) {
		render = true;
	} else if (IsFirstRender) {
		if (has_selectable_buildings) {
			render = true;
		} else {
			render = IsToRedraw || DirtyAreas.Count() != 0 || TacPixelX != LastTacPixelX || TacPixelY != LastTacPixelY;
		}
	} else if (has_selectable_buildings) {
		render = true;
	} else if (DirtyAreas.Count() == 0 && IsToRedraw) {
		render = true;
	} else if (DirtyAreas.Count() == 0 && TacticalCoord.X == LastTacticalCoord.X && TacticalCoord.Y == LastTacticalCoord.Y) {
		render = false;
	} else {
		render = IsToRedraw || DirtyAreas.Count() != 0 || TacPixelX != LastTacPixelX || TacPixelY != LastTacPixelY;
	}

	if (drawpass != DRAW_PASS_FOREGROUND && render) {

		/*
		 * The magnitude of the pan, and the source/destination rectangles for shifting the
		 * still-valid portion of the cached surface across by that amount.
		 */
		unsigned panwidth = abs(xoffset);
		unsigned panheight = abs(yoffset);
		int ystart = (yoffset >= 0) ? 0 : -yoffset;
		int xstart = (xoffset >= 0) ? 0 : -xoffset;

		Rect sourcerect(TacticalDimensions.X + xstart, TacticalDimensions.Y + ystart,
					TacticalDimensions.Width - panwidth, TacticalDimensions.Height - panheight);
		Rect destrect(TacticalDimensions.X + ((xoffset <= 0) ? 0 : xoffset),
					TacticalDimensions.Y + ((yoffset <= 0) ? 0 : yoffset),
					TacticalDimensions.Width - panwidth, TacticalDimensions.Height - panheight);

		if (xoffset != 0 || yoffset != 0) {
			if (drawpass == DRAW_PASS_PAN || drawpass == DRAW_PASS_FULL) {
				CompositeSurface->Blit_From(destrect, *TileSurface, sourcerect);
			}
			if (drawpass == DRAW_PASS_PAN || drawpass == DRAW_PASS_FULL) {
				DepthBuffer->Pan(-xoffset, -yoffset, 0xFFFFu);
			}
			if (drawpass == DRAW_PASS_PAN || drawpass == DRAW_PASS_FULL) {
				AlphaBuffer->Pan(-xoffset, -yoffset, 0x7Fu);
			}
			if (drawpass == DRAW_PASS_PAN || drawpass == DRAW_PASS_FULL) {

				/*
				 * Swap the composite and tile surfaces so the freshly-panned image becomes the
				 * render target.
				 */
				Surface * oldcomp = CompositeSurface;
				CompositeSurface = TileSurface;
				TileSurface = oldcomp;
				composite = CompositeSurface;
				LogicalSurface = CompositeSurface;
			}
		} else {
			if (fullredraw && (drawpass == DRAW_PASS_PAN || drawpass == DRAW_PASS_FULL)) {
				DepthBuffer->Fill(0xFFFFu);
				AlphaBuffer->Fill(127u);
			}
		}

		if (drawpass == DRAW_PASS_PAN) {
			return;
		}

		if (!Debug_Map) {
			Register_Dirty_Buildings(Rect(TacticalRect.X - 64, TacticalRect.Y - 64, TacticalRect.Width + 128, TacticalRect.Height + 128));
		}

		Surface * prevlogical = LogicalSurface;
		LogicalSurface = TileSurface;
		LogicalSurface->Lock();

		if (drawpass == DRAW_PASS_BACKGROUND || drawpass == DRAW_PASS_FULL) {

			/*
			 * Build the two redraw strips that the pan exposed. 'xpanrect' is the full-height strip
			 * revealed by horizontal scrolling (left edge if scrolled right, right edge if scrolled
			 * left), 'ypanrect' the strip revealed by vertical scrolling. 'bounds' is the region
			 * that the blit above preserved, used to clip the depth wipe and shroud.
			 */
			bool scrolled_left = LastTacPixelX > TacPixelX;
			bool scrolled_right = LastTacPixelX < TacPixelX;
			bool scrolled_up = LastTacPixelY > TacPixelY;
			bool scrolled_down = LastTacPixelY < TacPixelY;

			Rect leftstrip(TacticalRect.X, TacticalRect.Y, panwidth, TacticalRect.Height);
			Rect rightstrip(TacticalRect.X + TacticalRect.Width - panwidth, TacticalRect.Y, panwidth, TacticalRect.Height);
			Rect leftclip = Intersect(leftstrip, TacticalRect);
			Rect rightclip = Intersect(rightstrip, TacticalRect);

			Rect topstrip;
			Rect bottomstrip;
			if (scrolled_left || scrolled_right) {
				int stripx = scrolled_left ? TacticalRect.X + panwidth : TacticalRect.X;
				topstrip = Rect(stripx, TacticalRect.Y, TacticalRect.Width - panwidth, panheight);
				bottomstrip = Rect(stripx, TacticalRect.Y + TacticalRect.Height - panheight, TacticalRect.Width - panwidth, panheight);
			} else {
				topstrip = Rect(TacticalRect.X, TacticalRect.Y, TacticalRect.Width, panheight);
				bottomstrip = Rect(TacticalRect.X, TacticalRect.Y + TacticalRect.Height - panheight, TacticalRect.Width, panheight);
			}
			Rect topclip = Intersect(topstrip, TacticalRect);
			Rect bottomclip = Intersect(bottomstrip, TacticalRect);

			Rect bounds(TacticalRect.X + (scrolled_left ? panwidth : 0),
					TacticalRect.Y + (scrolled_up ? panheight : 0),
					TacticalRect.Width - panwidth, TacticalRect.Height - panheight);

			Rect xpanrect(0, 0, 0, 0);
			Rect ypanrect(0, 0, 0, 0);
			if (scrolled_left) {
				xpanrect = leftclip;
			} else if (scrolled_right) {
				xpanrect = rightclip;
			}
			if (scrolled_up) {
				ypanrect = topclip;
			} else if (scrolled_down) {
				ypanrect = bottomclip;
			}

			Wipe_Depth(fullredraw, xoffset, yoffset, bounds);
			Render_Shroud(xpanrect, ypanrect, bounds, fullredraw);
			Render_Tiles(xpanrect, ypanrect, fullredraw);
			Render_Fogged_Objects(xpanrect, ypanrect, fullredraw);
			Render_Overlays(xpanrect, ypanrect, fullredraw);
			Render_Terrain(xpanrect, ypanrect, fullredraw);
			Render_Tile_Shadows(xpanrect, ypanrect, fullredraw);
			Render_Buildings(xpanrect, ypanrect, fullredraw);

			for (int index = DirtyAreas.Count() - 1; index >= 0; index--) {
				DirtyAreas.Delete_Index(index);
			}

			LogicalSurface->Unlock();
			LogicalSurface = prevlogical;
			CellRedraw.clear();
		}
	}

	/*
	 * Remember this frame's position for the next pan calculation.
	 */
	LastTacticalCoord.X = TacticalCoord.X;
	LastTacticalCoord.Y = TacticalCoord.Y;
	LastTacPixelX = TacPixelX;
	LastTacPixelY = TacPixelY;

	if (drawpass == DRAW_PASS_BACKGROUND || drawpass == DRAW_PASS_FULL) {
		LogicalSurface->Blit_From(TacticalRect, *TileSurface, TacticalRect);
	}

	if (drawpass == DRAW_PASS_FOREGROUND || drawpass == DRAW_PASS_FULL) {
		int i;

		Add_Buildings_To_Selectable(TacticalRect);
		composite->Lock();
		Surface * savedlogical = LogicalSurface;
		LogicalSurface = composite;
		Draw_Waypoints(false);
		Draw_Rally_Points(false);
		Draw_Placement(false);

#ifdef _DEBUG
		if (DrawOccupierLinks) Debug_Draw_Occupier_Links();
		if (OccupationBitPrint) Debug_Draw_Occupation_Flags();
#endif

		VeinholeMonsterClass::Draw_All();
		IonBlastClass::Draw_All();
		Draw_Objects(true);
		SpotLightClass::Draw_All();
		LaserDrawClass::Draw_All();

		for (i = 0; i < CurrentObject.Count(); i++) {
			ObjectClass * object = CurrentObject[i];
			if (object->Class_Of() != NULL && object->Class_Of()->IsHasRadialIndicator) {
				object->Draw_Radial_Indicator();
			}
		}

		Draw_Rubber_Band();
		Draw_Waypoints(true);
		Draw_Rally_Points(true);
		Draw_Placement(true);

		for (i = 0; i < CurrentObject.Count(); i++) {
			ObjectClass * object = CurrentObject[i];
			if (object->Is_Techno() && ((TechnoClass*)object)->IsSelected) {
				if (TechnoClass::ActionLines == true && ((TechnoClass*)object)->House->Is_Player_Control()) {
					((TechnoClass*)object)->Draw_Action_Line();
				}
			}
		}

		Render_Outside_Map(*composite);

		LogicalSurface = savedlogical;
		composite->Unlock();
		Draw_Screen_Text(ScreenText);

		IsFirstRender = false;
		IsToRedraw = false;
	}
}


/// <summary>
/// Sets the caption text shown over the tactical view.
/// The text is pulled from the string table, so the caller only has to know which message it
/// wants displayed.
/// </summary>
/// <param name="text_id">The string table identifier of the caption, or -1 to clear it.</param>
void Tactical::Set_Caption_Text(int text_id)
{
	if (text_id == -1) {
		Clear_Caption_Text();
	} else {
		strcpy(ScreenText, Fetch_String(text_id));
	}
}


/// <summary>
/// Clears the caption text shown over the tactical view.
/// </summary>
void Tactical::Clear_Caption_Text(void)
{
	ScreenText[0] = '\0';
}


/// <summary>
/// Draws a line of text across the middle of the tactical view.
/// This routine paints straight onto the composite surface with GDI, so it does nothing
/// unless that surface can hand out a device context. It also stays quiet while the map
/// editor is running.
/// </summary>
/// <param name="text">The text to display. A NULL or empty string draws nothing.</param>
void Tactical::Draw_Screen_Text(char const * text)
{
	if (Debug_Map) {
		return;
	}
	if (text == NULL || !strlen(text)) {
		return;
	}
	if (CompositeSurface->Is_GDI_Backed()) {
		DSurface * surface = (DSurface *)CompositeSurface;
		Rect rect = TacticalRect;
		HDC hdc = surface->GetDC();
		if (hdc != NULL) {
			HFONT font = CreateFont(28, 20, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_RASTER_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FF_SWISS | DEFAULT_PITCH, NULL);
			HGDIOBJ h = SelectObject(hdc, font);
			Point2D point = Point2D(TacticalRect.Width / 2, TacticalRect.Height / 2);
			SetBkMode(hdc, TRANSPARENT);
			SetTextAlign(hdc, TA_CENTER);
			SetTextColor(hdc, RGB(255, 255, 255));
			TextOut(hdc, rect.X + point.X, rect.Y + point.Y, text, strlen(text));
			SelectObject(hdc, h);
			DeleteObject(font);
			surface->ReleaseDC(hdc);
		}
	}
}


/// <summary>
/// Draws the placement cursor for the object waiting to be built.
/// This routine is used while a pending object is being positioned under the mouse. Every
/// cell of its footprint is marked to show whether it will accept the object, and a wall,
/// fence or firestorm post also previews the run it would connect into.
/// </summary>
/// <param name="drawtrans">Should the placement cursor be drawn translucent?</param>
void Tactical::Draw_Placement(bool drawtrans)
{
	Cell start = Cell(0, 0);

	if (Map.PendingObject == NULL) {
		return;
	}

	Point2D mouse = MouseCursor->Get_Mouse_Point();
	if (!TacticalRect.Is_Point_Within(mouse)) {
		return;
	}

	/*
	 * Work out the footprint to mark. A gate reaches a cell further out on every side
	 * than the cells it actually occupies, so that the player can see what it will join.
	 */
	Cell dims;
	if (Map.PendingObject->Fetch_RTTI() == RTTI_BUILDINGTYPE && ((BuildingTypeClass *)Map.PendingObject)->IsGate) {
		dims = Cell(3, 3);
		start = Cell(-1, -1);
	} else {
		dims = Map.Get_Occupy_Dimensions(Map.CursorSize);
	}

	/*
	 * Mark every cell of the footprint, remembering whether they all accepted the object.
	 */
	bool all_valid = true;
	for (int cx = start.X; cx < dims.X; cx++) {
		for (int cy = start.Y; cy < dims.Y; cy++) {
			Cell cell = Cell(cx, cy) + Map.ZoneCell + Map.ZoneOffset;
			Coord coord = Coord_Whole(Coord(cell));
			Point2D pixel;
			Coord_To_Pixel(coord, pixel);
			pixel.X = ISO_TILE_PIXEL_W / -2 + pixel.X;
			if (Map.In_Radar(cell)) {
				CellClass * cellptr = &Map[cell];
				all_valid = cellptr->Draw_Placement_Cursor(pixel, TacticalRect, drawtrans) && all_valid;
			}
		}
	}

	RTTIType rtti = Map.PendingObject->Fetch_RTTI();

	/*
	 * The map editor moves the real object under the cursor and draws it there, so that
	 * the designer sees the artwork rather than just the cursor.
	 */
	if (Debug_Map && Map.PendingObjectPtr != NULL) {
		Coord coord = Coord(start + Map.ZoneCell + Map.ZoneOffset, 0);
		if (rtti == RTTI_BUILDINGTYPE) {
			coord = Coord_Whole(coord);
		}
		coord.Z = Map.Get_Height_GL(coord);
		Map.PendingObjectPtr->Set_Coord(coord);
		Point2D pixel;
		Coord_To_Pixel(coord, pixel);
		if (rtti != RTTI_ISOTILETYPE) {
			Map.PendingObjectPtr->Editor_Draw_It(pixel, TacticalRect);
		}
	}

	/*
	 * A wall, a fence or a firestorm post joins up with its neighbours, so the run it
	 * would connect into is previewed as well. Only do so where the object could
	 * actually be placed.
	 */
	if (all_valid && Map.PendingObjectPtr != NULL && Map.PendingObjectPtr->Fetch_RTTI() == RTTI_BUILDING) {
		BuildingClass * bptr = (BuildingClass *)Map.PendingObjectPtr;
		if (bptr->Class->IsLaserFencePost) {
			Draw_Fence_Placement(drawtrans, Cell(start.X, start.Y) + Map.ZoneCell + Map.ZoneOffset);
		} else if (bptr->Class->IsFirestormWall) {
			Draw_Firestorm_Wall_Placement(drawtrans, Cell(start.X, start.Y) + Map.ZoneCell + Map.ZoneOffset);
		} else if (bptr->Class->ToOverlay != NULL && bptr->Class->ToOverlay->IsWall) {
			Draw_Wall_Placement(drawtrans, Cell(start.X, start.Y) + Map.ZoneCell + Map.ZoneOffset);
		}
	}
}


/// <summary>
/// Draws the placement preview for a laser-fence connecting from the pending fence to any
/// player-owned laser fence posts in the four cardinal directions, up to the fence's range.
/// </summary>
/// <param name="drawtrans">Draw the preview translucent (valid) or opaque (invalid)?</param>
/// <param name="cell">The cell the fence is being placed on.</param>
void Tactical::Draw_Fence_Placement(bool drawtrans, Cell cell)
{
	for (int index = 0; index < FACING_COUNT; index += FACING_COUNT / 4) {
		Cell scan = cell;
		int count = 0;
		int range = ((BuildingClass *)Map.PendingObjectPtr)->Class->ThreatRange >> 8;
		if (range <= 0) {
			continue;
		}

		/*
		 * Walk outward until a building is met, or the run is broken by terrain, a
		 * ramp, impassable ground, or the limit of the fence's reach.
		 */
		FacingType facing = (FacingType)index;
		BuildingClass * building = NULL;
		bool found = false;
		while (true) {
			scan = Adjacent_Cell(scan, facing);
			building = Map[scan].Cell_Building();
			if (building != NULL) {
				found = true;
				break;
			}
			CellClass * cellptr = &Map[scan];
			if (cellptr->Cell_Terrain(false) != NULL) {
				break;
			}
			if (cellptr->Ramp > 0) {
				break;
			}
			if (cellptr->Passability != 0) {
				break;
			}
			if (++count >= range) {
				break;
			}
		}

		/*
		 * Lay the preview shape over every cell of the run that was walked.
		 */
		if (found && building->Class->IsLaserFencePost && building->House->Is_Player_Control() && count > 0) {
			int flags = drawtrans ? (SHAPE_ZERO_ALPHA | SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_TRANSLUCENT75)
								  : (SHAPE_NONZERO_ALPHA | SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_TRANSLUCENT75);
			Cell drawcell = Adjacent_Cell(cell, facing);
			for (int i = count; i > 0; i--) {
				int yoff = -1 - LEVEL_PIXEL_H_1 * Map[drawcell].Height;
				int zoff = -2 - LEVEL_PIXEL_H_1 * Map[drawcell].Height;
				Coord coord = Coord_Whole(Coord(drawcell));
				Point2D pixel;
				Coord_To_Pixel(coord, pixel);
				Point2D point;
				point.X = ISO_TILE_PIXEL_W / -2 + pixel.X + ISO_TILE_PIXEL_W / 2;
				point.Y = pixel.Y + yoff;
				Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)DisplayClass::PlacementShapes, 0, point, TacticalRect, (ShapeFlags_Type)flags, 0, zoff);
				drawcell = Adjacent_Cell(drawcell, facing);
			}
		}
	}
}


/// <summary>
/// Draws the placement preview for a firestorm wall connecting from the pending wall to any
/// player-owned firestorm wall in the four cardinal directions, up to the wall's range.
/// </summary>
/// <param name="drawtrans">Draw the preview translucent (valid) or opaque (invalid)?</param>
/// <param name="cell">The cell the wall is being placed on.</param>
void Tactical::Draw_Firestorm_Wall_Placement(bool drawtrans, Cell cell)
{
	for (int index = 0; index < FACING_COUNT; index += FACING_COUNT / 4) {
		Cell scan = cell;
		int count = 0;
		BuildingTypeClass * cls = ((BuildingClass *)Map.PendingObjectPtr)->Class;
		int range = cls->ThreatRange >> 8;
		if (range <= 0) {
			continue;
		}

		/*
		 * Walk outward until a matching wall is met, or the run is broken by a cell
		 * that could not be built on, or the limit of the wall's reach.
		 */
		FacingType facing = (FacingType)index;
		bool found = false;
		while (true) {
			scan = Adjacent_Cell(scan, facing);
			BuildingClass * building = Map[scan].Cell_Building();
			if (building != NULL && building->Class->IsFirestormWall && building->House->Is_Player_Control()) {
				found = true;
				break;
			}
			if (Map[scan].Is_Clear_To_Build(cls->Speed, cls, PlayerPtr)) {
				if (++count < range) {
					continue;
				}
			}
			break;
		}

		/*
		 * Lay the preview shape over every cell of the run that was walked.
		 */
		if (found && count > 0) {
			int flags = drawtrans ? (SHAPE_ZERO_ALPHA | SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_TRANSLUCENT75)
								  : (SHAPE_NONZERO_ALPHA | SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_TRANSLUCENT75);
			Cell drawcell = Adjacent_Cell(cell, facing);
			for (int i = count; i > 0; i--) {
				int yoff = -1 - LEVEL_PIXEL_H_1 * Map[drawcell].Height;
				int zoff = -2 - LEVEL_PIXEL_H_1 * Map[drawcell].Height;
				Coord coord = Coord_Whole(Coord(drawcell));
				Point2D pixel;
				Coord_To_Pixel(coord, pixel);
				Point2D point;
				point.X = ISO_TILE_PIXEL_W / -2 + pixel.X + ISO_TILE_PIXEL_W / 2;
				point.Y = pixel.Y + yoff;
				Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)DisplayClass::PlacementShapes, 0, point, TacticalRect, (ShapeFlags_Type)flags, 0, zoff);
				drawcell = Adjacent_Cell(drawcell, facing);
			}
		}
	}
}

/// <summary>
/// Draws the placement preview for a wall connecting from the pending wall to any player-owned
/// matching wall overlay in the four cardinal directions, up to the wall's range.
/// </summary>
/// <param name="drawtrans">Draw the preview translucent (valid) or opaque (invalid)?</param>
/// <param name="cell">The cell the wall is being placed on.</param>
void Tactical::Draw_Wall_Placement(bool drawtrans, Cell cell)
{
	OverlayTypeClass const * to_overlay = ((BuildingClass *)Map.PendingObjectPtr)->Class->ToOverlay;
	if (to_overlay == NULL) {
		return;
	}

	for (int index = 0; index < FACING_COUNT; index += FACING_COUNT / 4) {
		Cell scan = cell;
		int count = 0;
		BuildingTypeClass * cls = ((BuildingClass *)Map.PendingObjectPtr)->Class;
		int range = cls->ThreatRange >> 8;
		if (range <= 0) {
			continue;
		}

		/*
		 * Walk outward until a matching wall overlay is met, or the run is broken by a
		 * cell that could not be built on, or the limit of the wall's reach.
		 */
		FacingType facing = (FacingType)index;
		bool found = false;
		while (true) {
			scan = Adjacent_Cell(scan, facing);
			CellClass * cellptr = &Map[scan];
			if (cellptr->Overlay != OVERLAY_NONE && to_overlay->HeapID == cellptr->Overlay && PlayerPtr->HeapID == Map[scan].Owner) {
				found = true;
				break;
			}
			if (Map[scan].Is_Clear_To_Build(cls->Speed, cls, PlayerPtr)) {
				if (++count < range) {
					continue;
				}
			}
			break;
		}

		/*
		 * Lay the preview shape over every cell of the run that was walked.
		 */
		if (found && count > 0) {
			int flags = drawtrans ? (SHAPE_ZERO_ALPHA | SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_TRANSLUCENT75)
								  : (SHAPE_NONZERO_ALPHA | SHAPE_WIN_REL | SHAPE_CENTER | SHAPE_TRANSLUCENT75);
			Cell drawcell = Adjacent_Cell(cell, facing);
			for (int i = count; i > 0; i--) {
				int yoff = -1 - LEVEL_PIXEL_H_1 * Map[drawcell].Height;
				int zoff = -2 - LEVEL_PIXEL_H_1 * Map[drawcell].Height;
				Coord coord = Coord_Whole(Coord(drawcell));
				Point2D pixel;
				Coord_To_Pixel(coord, pixel);
				Point2D point;
				point.X = ISO_TILE_PIXEL_W / -2 + pixel.X + ISO_TILE_PIXEL_W / 2;
				point.Y = pixel.Y + yoff;
				Draw_Shape(*LogicalSurface, *NormalDrawer, (ShapeSet const *)DisplayClass::PlacementShapes, 0, point, TacticalRect, (ShapeFlags_Type)flags, 0, zoff);
				drawcell = Adjacent_Cell(drawcell, facing);
			}
		}
	}
}


/// <summary>
/// Copies the cached terrain surface onto the logical surface.
/// This routine is used by the render passes to lay down the terrain that was drawn into the
/// tile cache, before the dynamic objects are composited over the top of it.
/// </summary>
/// <returns>bool; Was the terrain copied across?</returns>
bool Tactical::Blit_Tiles(void)
{
	return(LogicalSurface->Blit_From(TacticalRect, *TileSurface, TacticalRect));
}


/// <summary>
/// Sets the dimensions of the tactical view.
/// This routine is called whenever the screen layout changes -- the sidebar sliding out, for
/// example. The tactical position is reapplied so that the new dimensions cannot leave the
/// view sitting off the edge of the map.
/// </summary>
/// <param name="dimensions">The new rectangle the tactical view occupies.</param>
void Tactical::Set_View_Dimensions(Rect const & dimensions)
{
	TacticalDimensions = dimensions;

	/*
	**	Adjust the tactical cell if it is now in an invalid position
	**	because of the changed dimensions.
	*/
	Set_Tactical_Position(DesiredTacticalCoord);
}


/// <summary>
/// Sets the position of the tactical view.
/// The position is first kept on the map, then the list of visible cells is brought up to
/// date and the view is flagged for a redraw.
/// </summary>
/// <param name="point">The desired view position, in absolute pixels.</param>
void Tactical::Set_Tactical_Position(Point2D const & point)
{
	Point2D clamped = Clamp_Pixel_To_Tactical(point);
	TacticalCoord = clamped;
	DesiredTacticalCoord = clamped;
	Update_Visible_Cells();
	IsToRedraw = true;
}


/// <summary>
/// Sets the tactical view position from a world coordinate.
/// This routine converts the coordinate into an absolute pixel position and hands it to the
/// pixel form, so the view is clamped and refreshed in the usual way.
/// </summary>
/// <param name="coord">The world coordinate to move the view to.</param>
void Tactical::Set_Tactical_Position(Coord const & coord)
{
	Point2D pixel = Coord_To_Pixel_Absolute(coord);

	pixel -= Point2D(TacPixelX, TacPixelY);
	pixel += Point2D(TacPixelX, TacPixelY);

	Set_Tactical_Position(pixel);
}


/// <summary>
/// Fetches the current position of the tactical view.
/// </summary>
/// <returns>Returns with the absolute pixel position the view is showing.</returns>
Point2D Tactical::Get_Tactical_Position(void)
{
	return(TacticalCoord);
}


/// <summary>
/// Fetches the tactical view position relative to the visible view.
/// </summary>
/// <returns>Returns with the view position with the tactical pixel origin removed.</returns>
Point2D Tactical::Get_Relative_Tactical_Position(void)
{
	Point2D tac_offset = Point2D(TacPixelX, TacPixelY);
	return(TacticalCoord - tac_offset);
}


/// <summary>
/// Rotates a rectangular world position onto the isometric screen axes.
/// This is the forward half of the tactical map's coordinate pipeline. No scaling and no view
/// offset are applied, so the result comes back in whatever units it was given.
/// </summary>
void Tactical::Rectangular_To_Isometric(int xin, int yin, int & xout, int & yout)
{
	xout = xin * ISO_TILE_PIXEL_W / 2;
	yout = xin * ISO_TILE_PIXEL_H / 2;
	xout += yin * ISO_TILE_PIXEL_W / -2;
	yout += yin * ISO_TILE_PIXEL_H / 2;
}


/*
 * Inverse of Rectangular_To_Isometric: given (xin,yin) in screen-pixel units, outputs
 * world-space rectangular coordinates, biased by -/+65536.
 */

/// <summary>
/// Converts an isometric screen position back onto the rectangular world axes.
/// This is the reverse of Rectangular_To_Isometric and is the integer counterpart of the
/// matrix-based Pixel_To_Lepton.
/// </summary>
void Tactical::Isometric_To_Rectangular(int xin, int yin, int & xout, int & yout)
{
	xout = ((ISO_TILE_PIXEL_W / 2) * yin + (ISO_TILE_PIXEL_H / 2) * xin) / 576 - 65536;
	yout = ((ISO_TILE_PIXEL_W / 2) * yin - (ISO_TILE_PIXEL_H / 2) * xin) / 576 + 65536;
}


/// <summary>
/// Converts an isometric position into rectangular world coordinates.
/// This routine works the same way as Isometric_To_Rectangular, except that the incoming
/// position is scaled up from cells to leptons before the conversion is applied.
/// </summary>
void Tactical::Cell_To_Rectangular(int xin, int yin, int & xout, int & yout)
{
	int xsh = xin << 8;
	int ysh = yin << 8;
	int y = ((ISO_TILE_PIXEL_W / 2) * ysh - (ISO_TILE_PIXEL_H / 2) * xsh) / 576 + 65536;
	int x = ((ISO_TILE_PIXEL_W / 2) * ysh + (ISO_TILE_PIXEL_H / 2) * xsh) / 576 - 65536;
	xout = x;
	yout = y;
}


/// <summary>
/// Converts a rectangular world position into isometric coordinates measured in cells.
/// This routine rotates onto the isometric axes the way Rectangular_To_Isometric does, then
/// reduces the result from leptons to cells and shifts it clear of the map origin.
/// </summary>
void Tactical::Lepton_To_Map_Pixel(int xin, int yin, int & xout, int & yout)
{
	Rectangular_To_Isometric(xin, yin, xout, yout);

	xout = xout / CELL_LEPTON_W;
	yout = yout / CELL_LEPTON_H;

	xout += (ISO_TILE_PIXEL_W / 2) * MAP_CELL_H;
}


/// <summary>
/// Converts a screen position into rectangular world coordinates about the map center.
/// This routine performs the plain inverse conversion and then removes the map-center bias
/// that conversion carries, leaving coordinates measured from the middle of the map.
/// </summary>
void Tactical::Cell_To_Rectangular_Centered(int xin, int yin, int & xout, int & yout)
{
	static int xoff = -1;
	static int yoff = -1;

	int x, y;
	Cell_To_Rectangular(xin, yin, x, y);

	if (xoff == -1 && yoff == -1) {
		xoff = -65536;
		yoff = 65536;
	}

	x -= xoff;
	y -= yoff;

	xout = x;
	yout = y;
}


/// <summary>
/// Resolves a world Coord to the map cell whose rendered footprint contains it, accounting for
/// terrain height (and bridges). Walks back along the isometric axis from a point below the cell,
/// comparing height-adjusted projections, until one lands on or before the source cell. This is
/// the height-aware coord->cell used for picking; it is not the trivial Coord::As_Cell.
/// </summary>
/// <param name="coord">World coordinate in leptons.</param>
/// <returns>The cell visually under the coord; the source cell if none is found.</returns>
Cell Tactical::Coord_To_Cell(Coord const & coord)
{
	Cell cell = coord;
	Coord center = cell.As_Coord();

	bool isbridge = Map[cell].IsBridgeSurface;
	int baseheight = Map[cell].Height;

	Coord scan = center;
	scan.X += CELL_LEPTON_W * 6;
	scan.Y += CELL_LEPTON_H * 6;
	scan.Z = 0;

	Cell scancell = cell;
	while (true) {
		scan -= Coord(8, 8, 0);
		scancell = scan;

		CellClass *cellptr = &Map[scancell];
		int heightdiff = cellptr->Height - baseheight;
		if (isbridge) {
			heightdiff += BRIDGE_CELL_HEIGHT * cellptr->IsUnderBridge;
		}

		Cell projected = Coord(scan - Coord(heightdiff * (CELL_LEPTON_H / 2), heightdiff * (CELL_LEPTON_H / 2)));
		if (projected.X <= cell.X && projected.Y <= cell.Y) {
			return(scancell);
		}
		if (scancell == cell) {
			return(cell);
		}
	}
}


/// <summary>
/// Converts a screen pixel to the map cell the player perceives under it, accounting for terrain
/// height and bridges. Projects the pixel back to a base cell, then walks down the iso axis until
/// a cell's height-adjusted footprint reaches the pixel; bridge cells nudge the result to the
/// adjacent cell that the bridge surface visually belongs to.
/// </summary>
/// <param name="pixel">Screen pixel (relative to the tactical view).</param>
/// <returns>The apparent cell under the pixel.</returns>
Cell Tactical::Pixel_To_Cell(Point2D const & pixel)
{
	Point2D lepton = Pixel_To_Lepton(Point2D(TacPixelX + pixel.X - TacticalRect.X, pixel.Y + TacPixelY - TacticalRect.Y));

	Coord frac = Coord_Fraction(Coord(lepton.X, lepton.Y, 0));
	Cell fallback(lepton.X / CELL_LEPTON, lepton.Y / CELL_LEPTON);

	int count = 0;

	/*
	 * This result is never used.
	 */
	Point2D abspixel = Coord_To_Pixel_Absolute(frac);

	int y = pixel.Y - TacticalRect.Y;
	int x = pixel.X - TacticalRect.X;

	int span = 12 * (ISO_TILE_PIXEL_H / 2);
	int scany = span + y;

	while (count < span) {
		abspixel.X = x + TacPixelX;
		abspixel.Y = scany + TacPixelY;

		Point2D scanlepton = Pixel_To_Lepton(abspixel);
		Cell cell(scanlepton.X / CELL_LEPTON, scanlepton.Y / CELL_LEPTON);

		CellClass * cellptr = &Map[cell];
		int celly = scany - cellptr->Height * (ISO_TILE_PIXEL_H / 2);

		if (cellptr->IsUnderBridge) {
			CellClass & east = cellptr->Adjacent_Cell(FACING_E);
			CellClass & south = cellptr->Adjacent_Cell(FACING_S);

			bool bridge1 = cellptr->IsBridgeEastWest && !cellptr->Adjacent_Cell(FACING_N).IsUnderBridge;
			bool bridge2 = !cellptr->IsBridgeEastWest && !cellptr->Adjacent_Cell(FACING_W).IsUnderBridge;
			bool bridge3 = cellptr->IsBridgeEastWest && !south.IsUnderBridge;
			bool bridge4 = !cellptr->IsBridgeEastWest && !east.IsUnderBridge;
			bool bridge5 = cellptr->IsBridgeEastWest && abs(cellptr->Height - east.Height) <= 1 && !east.IsUnderBridge;
			bool bridge6 = !cellptr->IsBridgeEastWest && abs(cellptr->Height - south.Height) <= 1 && !south.IsUnderBridge;

			Coord topcoord = Coord_Whole(Coord(cellptr->CellID, 0));
			Point2D top;
			Coord_To_Pixel(topcoord, top);
			int topy = ISO_TILE_PIXEL_H * cellptr->Height / -2 + top.Y;

			if (topy <= y) {
				if (bridge3 || bridge6) {
					return(Cell(cell.X, cell.Y + 1));
				}
				if (bridge4 || bridge5) {
					return(Cell(cell.X + 1, cell.Y));
				}
			}

			if (bridge1 || bridge2) {
				int dx = x - top.X;
				int dy = y - topy;
				if ((bridge1 && dy - dx / 2 > ISO_TILE_PIXEL_H / 2) ||
					(bridge2 && dx / 2 + dy > ISO_TILE_PIXEL_H / 2)) {
					celly -= (cellptr->IsUnderBridge ? BRIDGE_CELL_HEIGHT * (ISO_TILE_PIXEL_H / 2) : 0);
				}
			} else {
				celly -= (cellptr->IsUnderBridge ? BRIDGE_CELL_HEIGHT * (ISO_TILE_PIXEL_H / 2) : 0);
			}
		}

		if (celly <= y) {
			return(cell);
		}

		scany--;
		count++;
	}

	return(fallback);
}


/// <summary>
/// Fetches the ground height at the given world coordinate.
/// The height is asked of the cell the coordinate falls in, so any ramp or bridge at that
/// spot is taken into account.
/// </summary>
/// <returns>Returns with the height of the ground beneath the coordinate.</returns>
int Tactical::Get_Cell_Height(Coord const & coord)
{
	CellClass *cellptr = &Map[coord];
	return(cellptr->Get_Height(coord));
}


/// <summary>
/// Fetches the ramp type of the cell the coordinate falls in.
/// </summary>
/// <returns>Returns with the ramp index of the cell.</returns>
int Tactical::Get_Cell_Ramp(Coord const & coord)
{
	return(Map[coord].Ramp);
}


/// <summary>
/// Adds a shadow control to the tactical map.
/// The control supplied is copied onto the heap, so the caller's object does not have to
/// outlive the call.
/// </summary>
/// <param name="control">The shadow control to copy and add.</param>
void Tactical::Add_Shadow_Control(ShadowControlClass control)
{
	ShadowControlClass *ctrl = new ShadowControlClass(control);
	ShadowControls.Add(ctrl);
}


/// <summary>
/// Finds the bridge deck cell that covers the given cell.
/// This routine is used when something sits underneath a bridge and the caller needs the deck
/// cell that owns the span above it. Both east-west and north-south bridges are considered.
/// </summary>
/// <param name="cell">The cell to find the covering bridge deck for.</param>
/// <returns>Returns with a pointer to the owning bridge deck cell. Otherwise, NULL is
/// returned.</returns>
CellClass * Tactical::Find_Bridge_Owner_Cell(Cell const & cell)
{
	CellClass * cellptr;
	Cell scan;
	int i;

	/*
	 * Scan south along the two columns an east-west span could reach this cell from.
	 */
	scan = cell + Cell(2, 0);
	for (i = 0; i < 5; ++i) {
		cellptr = &Map[scan];
		if (cellptr->IsBridgeDeck && cellptr->Overlay && cellptr->IsBridgeEastWest) {
			return(cellptr);
		}
		scan.Y++;
	}

	scan = cell + Cell(1, 0);
	for (i = 0; i < 5; ++i) {
		cellptr = &Map[scan];
		if (cellptr->IsBridgeDeck && cellptr->Overlay && cellptr->IsBridgeEastWest) {
			return(cellptr);
		}
		scan.Y++;
	}

	/*
	 * Then scan east along the two rows a north-south span could reach it from.
	 */
	scan = cell + Cell(0, 2);
	for (i = 0; i < 5; ++i) {
		cellptr = &Map[scan];
		if (cellptr->IsBridgeDeck && cellptr->Overlay && !cellptr->IsBridgeEastWest) {
			return(cellptr);
		}
		scan.X++;
	}

	scan = cell + Cell(0, 1);
	for (i = 0; i < 5; ++i) {
		cellptr = &Map[scan];
		if (cellptr->IsBridgeDeck && cellptr->Overlay && !cellptr->IsBridgeEastWest) {
			return(cellptr);
		}
		scan.X++;
	}

	return(NULL);
}


/// <summary>
/// Draws the cell overlays that fall within the given region of the view.
/// This routine is called by Render_Overlays for each part of the tactical view that needs
/// refreshing. The overlay graphics are laid down first and their cast shadows afterwards.
/// </summary>
/// <param name="area">The region of the tactical view to draw the overlays within.</param>
void Tactical::Draw_Overlays(Rect const & area)
{
	Coord lepton = Coord(Pixel_To_Lepton(Point2D(TacPixelX, TacPixelY) + area.Top_Left() - TacticalRect.Top_Left()), 0);
	if (lepton.X < 0) lepton.X = 0;
	if (lepton.Y < 0) lepton.Y = 0;

	Cell origin = Map[lepton].CellID;

	int ycount = area.Height / (ISO_TILE_PIXEL_H / 2) + 20;
	int xcount = area.Width / ISO_TILE_PIXEL_W + 4;

	Rect cliprect = Intersect(TacticalRect, area);
	Cell base(origin.X - 2, origin.Y);

	/*
	 * The overlay graphics first...
	 */
	int ix, iy;
	for (iy = ycount - 1; iy >= 0; iy--) {
		Cell step(iy / 2, (iy + 1) / 2);
		Cell cell = base + step;

		for (ix = xcount; ix > 0; ix--) {
			if (Map.In_Radar(cell)) {
				CellClass * cellptr = &Map[cell];
				if (cellptr->Overlay != OVERLAY_NONE) {
					Rect render = cellptr->Overlay_Render_Rect();
					if (Intersect(cliprect, render).Is_Valid()) {
						Point2D pixel;
						Coord_To_Pixel(Coord_Whole(cell), pixel);
						pixel.X += ISO_TILE_PIXEL_W / -2;
						cellptr->Draw_Overlay(pixel, cliprect);
					}
				}
			}
			cell += Cell(1, -1);
		}
	}

	/*
	 * ...and the shadows they cast over the same band afterwards.
	 */
	for (iy = ycount - 1; iy >= 0; iy--) {
		Cell step(iy / 2, (iy + 1) / 2);
		Cell cell = base + step;

		for (ix = xcount; ix > 0; ix--) {
			if (Map.In_Radar(cell)) {
				CellClass * cellptr = &Map[cell];
				if (cellptr->Overlay != OVERLAY_NONE) {
					Rect shadow = cellptr->Overlay_Shadow_Render_Rect();
					if (Intersect(cliprect, shadow).Is_Valid()) {
						Point2D pixel;
						Coord_To_Pixel(Coord_Whole(cell), pixel);
						pixel.X += ISO_TILE_PIXEL_W / -2;
						cellptr->Draw_Overlay_Shadow(pixel, cliprect);
					}
				}
			}
			cell += Cell(1, -1);
		}
	}
}


/// <summary>
/// Draws the shroud and fog for the iso tiles whose visual footprint falls within the given
/// clip rectangle. The rectangle origin is projected back into cell space to find the start
/// cell, then the resulting diagonal band of cells is scanned and each cell that is in the
/// radar window and clips against the tactical rectangle has its shroud and fog drawn. Finishes
/// by drawing all alpha shapes within the clip rectangle.
/// </summary>
/// <param name="area">The region of the tactical view to draw the shroud and fog within.</param>
void Tactical::Draw_Shroud(Rect const & area)
{
	Coord lepton = Coord(Pixel_To_Lepton(Point2D(TacPixelX, TacPixelY) + area.Top_Left() - TacticalRect.Top_Left()), 0);
	if (lepton.Y < 0) lepton.Y = 0;
	if (lepton.X < 0) lepton.X = 0;

	Cell origin = Map[lepton].CellID;

	int ycount = area.Height / (ISO_TILE_PIXEL_H / 2) + 17;
	int xcount = area.Width / ISO_TILE_PIXEL_W + 4;

	Cell base(origin.X - 2, origin.Y);

	if (MainWindow != NULL) {
		int ix, iy;
		for (iy = 0; iy < ycount; iy++) {
			Cell step(iy / 2, (iy + 1) / 2);
			Cell cell = base + step;

			for (ix = xcount; ix > 0; ix--) {
				if (Map.In_Radar(cell)) {
					CellClass * cellptr = &Map[cell];

					Coord coord = Coord_Whole(Coord(cell));
					Point2D pixel;
					Coord_To_Pixel(coord, pixel);
					pixel.X += ISO_TILE_PIXEL_W / -2;

					Rect cellrect(pixel.X, TacticalRect.Y + pixel.Y, ISO_TILE_PIXEL_W, ISO_TILE_PIXEL_H);
					Rect inter = Intersect(cellrect, area);
					Rect drawrect = Intersect(TacticalRect, inter);

					if (drawrect.Is_Valid()) {
						cellptr->Draw_Shroud_And_Fog(pixel + TacticalRect.Top_Left(), drawrect);
					}
				}
				cell += Cell(1, -1);
			}
		}
	}

	AlphaShapeClass::Draw_All(area);
}


/// <summary>
/// Draws the iso tiles whose visual footprint falls within the given pixel rectangle. The
/// rectangle origin is projected back into cell space to find the start cell, then the resulting
/// diagonal band of cells is scanned and each cell whose render rect clips against the area has
/// its tile drawn.
/// </summary>
/// <param name="area">The pixel rectangle to draw tiles within.</param>
/// <param name="cliprect">The clip rectangle passed to each cell's Draw_It.</param>
void Tactical::Draw_Tiles(Rect const & area, Rect const & cliprect)
{
	Coord lepton = Coord(Pixel_To_Lepton(Point2D(TacPixelX, TacPixelY) + area.Top_Left() - TacticalRect.Top_Left()), 0);
	Cell origin = lepton.As_Cell();
	Cell base(origin.X - 2, origin.Y);

	int ycount = area.Height / (ISO_TILE_PIXEL_H / 2) + 17;
	int xcount = area.Width / ISO_TILE_PIXEL_W + 4;

	int ix, iy;
	for (iy = 0; iy < ycount; iy++) {
		Cell step(iy / 2, (iy + 1) / 2);
		Cell cell = base + step;

		for (ix = xcount; ix > 0; ix--) {
			if (Map.In_Radar(cell)) {
				CellClass * cellptr = &Map[cell];
				Rect render = cellptr->Cell_Render_Rect();
				Rect inter = Intersect(area, render);
				Coord coord = Coord_Whole(Coord(cell));
				Point2D pixel;
				Coord_To_Pixel(coord, pixel);
				pixel.X += ISO_TILE_PIXEL_W / -2;
				if (render.Is_Valid() && inter.Is_Valid()) {
					cellptr->Draw_It(pixel, cliprect, 0);
				}
			}
			cell += Cell(1, -1);
		}
	}
}


/// <summary>
/// Draws the cast shadows for the iso tiles whose visual footprint falls within the given
/// pixel rectangle. The rectangle is projected back into cell space (the start cell is
/// computed twice; the first result is unused), and the resulting diagonal band of cells is
/// scanned ascending so each cell's tile shadow caster is drawn into the clip rectangle.
/// </summary>
/// <param name="area">The pixel rectangle to draw shadows within.</param>
/// <param name="cliprect">The clip rectangle passed to each cell's shadow caster.</param>
void Tactical::Draw_Tile_Shadows(Rect const & area, Rect const & cliprect)
{
	Coord lepton = Coord(Pixel_To_Lepton(Point2D(TacPixelX, TacPixelY) + area.Top_Left() - TacticalRect.Top_Left()), 0);
	Cell origin = lepton.As_Cell();
	Cell base(origin.X - 2, origin.Y);

	lepton = Coord(Pixel_To_Lepton(Point2D(TacPixelX, TacPixelY) + area.Top_Left() - TacticalRect.Top_Left()), 0);
	origin = Map[lepton].CellID;
	base = Cell(origin.X - 2, origin.Y + 3);

	int ycount = area.Height / (ISO_TILE_PIXEL_H / 2) + 20;
	int xcount = area.Width / ISO_TILE_PIXEL_W + 7;

	int ix, iy;
	for (iy = 0; iy < ycount; iy++) {
		Cell step(iy / 2, (iy + 1) / 2);
		Cell cell = base + step;

		for (ix = xcount; ix > 0; ix--) {
			if (Map.In_Radar(cell)) {
				Coord coord = Coord_Whole(Coord(cell));
				Point2D pixel;
				Coord_To_Pixel(coord, pixel);
				pixel.X += ISO_TILE_PIXEL_W / -2;
				Map[cell].Draw_Shadow_Cast(pixel, cliprect);
			}
			cell += Cell(1, -1);
		}
	}
}


/// <summary>
/// Draws the iso tile of a cell along with any taller tiles that overhang it.
/// This routine is used to refresh a single dirty cell of the terrain. Artwork standing on
/// higher ground hangs down over the cells in front of it, so those tiles have to be redrawn
/// as well or the repaired cell punches a hole through the cliff face above it.
/// </summary>
/// <param name="cell">The cell whose tile needs refreshing.</param>
/// <param name="cliprect">Not referenced; the tiles clip to the tactical rectangle.</param>
void Tactical::Draw_Tiles(Cell const & cell, Rect const & cliprect)
{
	CellClass * basecell = &Map[cell];

	Coord coord = Coord_Whole(Coord(cell));
	Point2D screen = Coord_To_Pixel_Absolute(coord);
	if (basecell->Height < 2) {
		Point2D pixel = screen - Point2D(TacPixelX, TacPixelY);
		pixel.X -= ISO_TILE_PIXEL_W / 2;
		basecell->Draw_It(pixel, TacticalRect, 0);
	}

	int x = cell.X;
	int y = cell.Y;
	for (int level = 0; level < 24; level += 2) {
		for (int col = 0; col < 3; col++) {
			int cx = x + ((col + 1) >> 1);
			int cy = y - (col >> 1) + 1;
			if (Map.In_Radar(Cell(cx, cy))) {
				CellClass * cellptr = &Map[Cell(cx, cy)];
				bool draw = false;
				int height = cellptr->Height;
				bool overlap = false;

				if (col != 1) {
					if (height == level + 1) {
						draw = true;
						overlap = true;
					}
				} else if (height > level && height < level + 4) {
					draw = true;
					overlap = true;
				}

				if (cellptr->Elevation > 0 && !draw) {
					int diff = height - level - 1;
					if (col != 1) {
						if (diff < 0 && cellptr->Elevation + diff >= 0) {
							draw = true;
						} else {
							draw = overlap;
						}
					} else if (height - level > 0) {
						draw = overlap;
					} else {
						draw = true;
						if (cellptr->Elevation + height - level <= 0) {
							draw = overlap;
						}
					}
				}

				if (height == level) {
					if (!draw) {

						/*
						 * Same height and not already flagged for redraw: the tile only needs
						 * redrawing if its iso tile carries extra (overhanging) image data.
						 */
						int subtile = cellptr->SubTile;
						int ittype = cellptr->ITType;
						if (ittype == ISOTILE_NONE || ittype == ISOTILE_NONE_LEGACY) {
							continue;
						}
						IsometricTileTypeClass * type = IsometricTileTypes[ittype];
						if (type == NULL) {
							continue;
						}
						IsoTileSet const * set = (IsoTileSet const *)type->Get_Image_Data();
						if (set == NULL) {
							continue;
						}
						if (subtile >= set->Tile_Count() - 1) {
							subtile = set->Tile_Count() - 1;
						}
						IsoTileRecord const * record = set->Fetch_Record_Pointer_Unsafe(subtile);
						if (record == NULL || !record->IsHasExtraData) {
							continue;
						}
					}
				} else if (!draw) {
					continue;
				}

				Cell above(cx, cy);
				coord = Coord_Whole(Coord(above));
				Point2D pixel;
				Coord_To_Pixel(coord, pixel);
				pixel.X -= ISO_TILE_PIXEL_W / 2;
				Map[above].Draw_It(pixel, TacticalRect, 0);
			}
		}
		x++;
		y++;
	}
}


/// <summary>
/// Draws the shadows cast by the iso tiles over a region of the view.
/// This routine differs from its sibling only in clipping the casters against the same
/// region it scans instead of against a clip rectangle of its own.
/// </summary>
/// <param name="area">The area of the view to draw the tile shadows within.</param>
void Tactical::Draw_Tile_Shadows(int, Rect const & area)
{
	Coord lepton = Coord(Pixel_To_Lepton(Point2D(TacPixelX, TacPixelY) + area.Top_Left() - TacticalRect.Top_Left()), 0);
	if (lepton.Y < 0) {
		lepton.X -= lepton.Y;
		lepton.Y = 0;
	}
	if (lepton.X < 0) {
		lepton.Y -= lepton.X;
		lepton.X = 0;
	}

	Cell origin = Map[lepton].CellID;
	Cell base(origin.X - 2, origin.Y + 3);

	int xcount = area.Width / ISO_TILE_PIXEL_W + 7;
	int ycount = area.Height / (ISO_TILE_PIXEL_H / 2) + 20;

	int ix, iy;
	for (iy = 0; iy < ycount; iy++) {
		Cell step(iy / 2, (iy + 1) / 2);
		Cell cell = base + step;

		for (ix = xcount; ix > 0; ix--) {
			if (Map.In_Radar(cell)) {
				Point2D pixel;
				Coord_To_Pixel(Coord_Whole(cell), pixel);
				pixel.X += ISO_TILE_PIXEL_W / -2;
				Map[cell].Draw_Shadow_Cast(pixel, area);
			}
			cell += Cell(1, -1);
		}
	}
}


/// <summary>
/// Keeps a tactical view position from wandering off the map.
/// This routine is used by Set_Tactical_Position before the new view position is committed.
/// While the map editor is running the position is passed through untouched, so the view is
/// free to roam past the edge of the map.
/// </summary>
/// <param name="pixel">The desired view position, in absolute pixels.</param>
/// <returns>Returns with the clamped position, or the position as supplied if it was not
/// clamped.</returns>
Point2D Tactical::Clamp_Pixel_To_Tactical(Point2D const & pixel)
{
	Point2D clamped = pixel;
	if (Clamp_To_Tactical_Rect(clamped) && !Debug_Map) {
		return(clamped);
	} else {
		return(pixel);
	}
}


/// <summary>Scrolls the tactical map in the direction specified.
/// This routine moves the desired view position rather than the view itself; the tactical AI
/// commits the view to it on its next pass.</summary>
/// <param name="facing">The direction to scroll the view in.</param>
/// <param name="distance">The number of pixels to scroll by.</param>
void Tactical::Scroll_Map(FacingType facing, int distance)
{
	static Point2D _scroll[FACING_COUNT] = 	{
		Point2D(0, -1),
		Point2D(1, -1),
		Point2D(1, 0),
		Point2D(1, 1),
		Point2D(0, 1),
		Point2D(-1, 1),
		Point2D(-1, 0),
		Point2D(-1, -1)
	};

	int & old_x = TacticalCoord.X;
	int & old_y = TacticalCoord.Y;

	int new_x = old_x + distance * _scroll[facing].X;
	int new_y = old_y + distance * _scroll[facing].Y;

	if (old_x != new_x || old_y != new_y) {
		new_x = new_x - old_x;
		new_y = new_y - old_y;
		DesiredTacticalCoord.X += new_x;
		DesiredTacticalCoord.Y += new_y;
	}
}


/// <summary>
/// Works out the limits a tactical view position may take.
/// An axis whose playable area is smaller than the view comes back with its maximum below its
/// minimum.
/// </summary>
void Tactical::Tactical_Position_Limits(Point2D & minimum, Point2D & maximum)
{
	minimum.X = TacticalRect.Width / 2 - (ISO_TILE_PIXEL_W >> 1) * (Map.PlayRect.Width - 2 * Map.LocalRect.X);
	maximum.X = minimum.X + ISO_TILE_PIXEL_W * Map.LocalRect.Width - TacticalRect.Width;

	minimum.Y = TacticalRect.Height / 2 + (ISO_TILE_PIXEL_H >> 1) * (Map.PlayRect.Width + 2 * Map.LocalRect.Y - 5);
	maximum.Y = minimum.Y + ISO_TILE_PIXEL_H * (2 * Map.LocalRect.Height + 9) / 2 - TacticalRect.Height;
}


/// <summary>
/// Clamps a tactical view position to the limits of the map.
/// This routine is used by the scroll logic to keep the view from wandering off the edge of
/// the playable area. The position is adjusted in place.
/// </summary>
/// <param name="pixel">The tactical view position to clamp; adjusted in place.</param>
/// <returns>bool; Did the position have to be pulled back?</returns>
bool Tactical::Clamp_To_Tactical_Rect(Point2D & pixel)
{
	Point2D minimum;
	Point2D maximum;
	Tactical_Position_Limits(minimum, maximum);

	if (maximum.X < minimum.X) {
		minimum.X = maximum.X = (minimum.X + maximum.X) / 2;
	}

	if (maximum.Y < minimum.Y) {
		minimum.Y = maximum.Y = (minimum.Y + maximum.Y) / 2;
	}

	bool clamped = false;

	if (pixel.Y < minimum.Y) {
		clamped = true;
		pixel.Y = minimum.Y;
	} else if (pixel.Y > maximum.Y) {
		pixel.Y = maximum.Y;
		clamped = true;
	}

	if (pixel.X < minimum.X) {
		pixel.X = minimum.X;
		clamped = true;
	} else if (pixel.X > maximum.X) {
		pixel.X = maximum.X;
		clamped = true;
	}

	return(clamped);
}


/***********************************************************************************************
 * DisplayClass::Cell_Shadow   -- Determine what shadow icon to use for the cell.              *
 *                                                                                             *
 *    This routine will examine the specified cell and adjacent cells to                       *
 *    determine what shadow icon to use.                                                       *
 *                                                                                             *
 * INPUT:   cell     -- The cell to examine.                                                   *
 *                                                                                             *
 * OUTPUT:  Returns with the shadow icon to use. -2= all black.                                *
 *                                                -1= map cell.                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/01/1994 JLB : Created.                                                                 *
 *   04/04/1994 JLB : Revamped for new shadow icon method.                                     *
 *   04/30/1994 JLB : Converted to member function.                                            *
 *=============================================================================================*/
int Tactical::Cell_Shadow(Cell const & cell, bool fog)
{
	static char const _shadow[1 << FACING_COUNT]={
		-1,33, 2, 2,34,37, 2, 2,
		 4,26, 6, 6, 4,26, 6, 6,
		35,45,17,17,38,41,17,17,
		 4,26, 6, 6, 4,26, 6, 6,
		 8,21,10,10,27,31,10,10,
		12,23,14,14,12,23,14,14,
		 8,21,10,10,27,31,10,10,
		12,23,14,14,12,23,14,14,

		32,36,25,25,44,40,25,25,
		19,30,20,20,19,30,20,20,
		39,43,29,29,42,46,29,29,
		19,30,20,20,19,30,20,20,
		 8,21,10,10,27,31,10,10,
		12,23,14,14,12,23,14,14,
		 8,21,10,10,27,31,10,10,
		12,23,14,14,12,23,14,14,

		 1, 1, 3, 3,16,16, 3, 3,
		 5, 5, 7, 7, 5, 5, 7, 7,
		24,24,18,18,28,28,18,18,
		 5, 5, 7, 7, 5, 5, 7, 7,
		 9, 9,11,11,22,22,11,11,
		13,13,-2,-2,13,13,-2,-2,
		 9, 9,11,11,22,22,11,11,
		13,13,-2,-2,13,13,-2,-2,

		 1, 1, 3, 3,16,16, 3, 3,
		 5, 5, 7, 7, 5, 5, 7, 7,
		24,24,18,18,28,28,18,18,
		 5, 5, 7, 7, 5, 5, 7, 7,
		 9, 9,11,11,22,22,11,11,
		13,13,-2,-2,13,13,-2,-2,
		 9, 9,11,11,22,22,11,11,
		13,13,-2,-2,13,13,-2,-2
	};

	int index = 0, value = -1;

	CellClass const * cellptr = &Map[cell];

	if (fog) {
		/*
		**	Presume solid black if that is what is here already.
		*/
		if (!cellptr->IsFogVisible && !cellptr->IsFogMapped) value = -2;

		if (cellptr->IsFogMapped /*&& !cellptr->IsFogVisible*/) {
			/*
			**	Build an index into the lookup table using all 8 surrounding cells.
			**	We're mapping a revealed cell and we only care about the existence
			**	of black cells.  Bit numbering starts at the upper-right corner and
			**	goes around the cell clockwise, so 0x80 = directly north.
			*/
			Cell c;
			c = cell + Cell(-1, -1);
			if (!Map[c].IsFogMapped) index |= 0x40;
			c = cell + Cell(+0, -1);
			if (!Map[c].IsFogMapped) index |= 0x80;
			c = cell + Cell(+1, -1);
			if (!Map[c].IsFogMapped) index |= 0x01;
			c = cell + Cell(-1, +0);
			if (!Map[c].IsFogMapped) index |= 0x20;
			c = cell + Cell(+1, +0);
			if (!Map[c].IsFogMapped) index |= 0x02;
			c = cell + Cell(-1, +1);
			if (!Map[c].IsFogMapped) index |= 0x10;
			c = cell + Cell(+0, +1);
			if (!Map[c].IsFogMapped) index |= 0x08;
			c = cell + Cell(+1, +1);
			if (!Map[c].IsFogMapped) index |= 0x04;

			value = _shadow[index];
		}
	} else {

		/*
		**	Presume solid black if that is what is here already.
		*/
		if (!cellptr->IsVisible && !cellptr->IsMapped) value = -2;

		if (cellptr->IsMapped /*&& !cellptr->IsVisible*/) {
			/*
			**	Build an index into the lookup table using all 8 surrounding cells.
			**	We're mapping a revealed cell and we only care about the existence
			**	of black cells.  Bit numbering starts at the upper-right corner and
			**	goes around the cell clockwise, so 0x80 = directly north.
			*/
			Cell c;
			c = cell + Cell(-1, -1);
			if (!Map[c].IsMapped) index |= 0x40;
			c = cell + Cell(+0, -1);
			if (!Map[c].IsMapped) index |= 0x80;
			c = cell + Cell(+1, -1);
			if (!Map[c].IsMapped) index |= 0x01;
			c = cell + Cell(-1, +0);
			if (!Map[c].IsMapped) index |= 0x20;
			c = cell + Cell(+1, +0);
			if (!Map[c].IsMapped) index |= 0x02;
			c = cell + Cell(-1, +1);
			if (!Map[c].IsMapped) index |= 0x10;
			c = cell + Cell(+0, +1);
			if (!Map[c].IsMapped) index |= 0x08;
			c = cell + Cell(+1, +1);
			if (!Map[c].IsMapped) index |= 0x04;

			value = _shadow[index];
		}
	}
	return(value);
}


/// <summary>
/// Is the cell within the visible portion of the tactical map?
/// </summary>
/// <returns>bool; Does the cell lie within the visible cell rectangle?</returns>
bool Tactical::Is_Cell_Visible(Cell const & cell)
{
	Point2D point = Point2D(cell.X, cell.Y);
	if (VisibleCellRect.Is_Point_Within(point)) {
		return(true);
	}
	return(false);
}


/// <summary>
/// Recalculates the range of cells the tactical view covers.
/// The render passes and Is_Cell_Visible both work from the cell rectangle this routine
/// produces, rather than testing every cell on the map.
/// </summary>
/// <remarks>Call this routine whenever the view scrolls or its dimensions change.</remarks>
void Tactical::Update_Visible_Cells(void)
{
	TacPixelX = TacticalCoord.X;
	TacPixelY = TacticalCoord.Y;
	TacPixelX -= TacticalDimensions.Width / 2;
	TacPixelY -= TacticalDimensions.Height / 2;

	Point2D lepton = Pixel_To_Lepton(Point2D(TacPixelX - ISO_TILE_PIXEL_W, TacPixelY - ISO_TILE_PIXEL_H));

	VisibleCellRect.X = lepton.X / CELL_LEPTON;
	VisibleCellRect.Y = lepton.Y / CELL_LEPTON;
	VisibleCellRect.Width = TacticalDimensions.Width / ISO_TILE_PIXEL_W + 2;
	VisibleCellRect.Height = TacticalDimensions.Height / (ISO_TILE_PIXEL_H / 2) + 4;
}


/// <summary>
/// Computes an object's on-screen pixel position, clip-tests it against the tactical window
/// (with a 6-tile edge zone), and if visible draws its pre- and post-render decorations.
/// </summary>
/// <param name="object">The object whose pre- and post-render decorations are drawn.</param>
/// <param name="cliprect">The clipping rectangle passed to the draw routines.</param>
void Tactical::Draw_Render_Hooks(ObjectClass * object, Rect const & cliprect)
{
	Point2D pixel;
	if (TacticalMap->Coord_To_Pixel(object->Center_Coord(), pixel)) {
		object->Draw_Pre_Render(pixel, cliprect);
		object->Draw_Post_Render(pixel, cliprect);
	}
}


/// <summary>
/// Renders every object in each display layer (and, after the ground layer, the overlay sprites
/// of placed buildings). Each object kind takes a slightly different path: buildings and static
/// terrain draw only their selection brackets and decorations here (buildings additionally render
/// their body), animating terrain and animations render normally unless fog hides them, and mobile
/// objects run their pre-render hook, render their body, then run their post-render hook.
/// </summary>
/// <param name="forced">Forwarded to each object's Render to force a full redraw.</param>
void Tactical::Draw_Objects(bool forced)
{
	for (int layer = LAYER_FIRST; layer < LAYER_COUNT; layer++) {
		for (int index = 0; index < DisplayClass::Layer[layer].Count(); index++) {
			ObjectClass * obj = DisplayClass::Layer[layer][index];
			RTTIType rtti = obj->RTTI;

			if (rtti == RTTI_BUILDING || (rtti == RTTI_TERRAIN && !((TerrainClass *)obj)->Is_Animating())) {

				/*
				 * Buildings and terrain that is standing still get their decorations drawn
				 * here, but only the building renders its body. Terrain that never moves was
				 * already laid into the cached tile surface by the background pass, so
				 * drawing it a second time would merely cost time.
				 */
				Draw_Render_Hooks(obj,TacticalRect);
				if (rtti == RTTI_BUILDING) {
					obj->Render(TacticalRect, forced, true);
				}
			}
			else if (rtti == RTTI_TERRAIN) {

				/*
				 * Terrain that is animating changes from frame to frame, so it cannot be
				 * cached and must render here. Under the fog it stays hidden -- the fog
				 * layer shows it as the player last saw it.
				 */
				if (MainWindow && !Debug_Map && Map.Is_Fogged(obj->PositionCoord)) {
					continue;
				}
				obj->Render(TacticalRect, forced, false);
			}
			else if (rtti == RTTI_ANIM) {

				/*
				 * Animations render like anything else, with one exception. An animation
				 * that lifts the fog when it plays is held back while its cell is still
				 * fogged, so that it cannot betray what is standing underneath it. The
				 * animations that belong to a building are exempt, since the building has
				 * already given itself away.
				 */
				AnimClass * anim = (AnimClass *)obj;
				if (MainWindow && !Debug_Map && Map.Is_Fogged(anim->Center_Coord())
					&& !anim->IsBuildingAnim && anim->Class->IsShouldFogRemove) {
					continue;
				}
				anim->Render(TacticalRect, forced, false);
			}
			else {

				/*
				 * Everything else is a mobile object, and mobile objects are drawn between
				 * their two decoration hooks so that the shadow and the selection box end
				 * up on the correct side of the body.
				 */
				if (rtti == RTTI_UNIT || rtti == RTTI_AIRCRAFT || rtti == RTTI_INFANTRY) {

					/// The results of both calls are discarded.
					obj->Destination_Coord();
					obj->PositionCoord;

					/*
					 * A unit that has slipped back under the fog of war is not drawn at
					 * all. What the player remembers of it is drawn by the fog layer.
					 */
					if (MainWindow && !Debug_Map && Scen->Special.IsFogOfWar && Map.Is_Fogged(obj->PositionCoord)) {
						continue;
					}
				}

				Point2D pixel;
				if (TacticalMap->Coord_To_Pixel(obj->Center_Coord(), pixel)) {
					obj->Draw_Pre_Render(pixel, TacticalRect);
					obj->Render(TacticalRect, forced, false);
					obj->Draw_Post_Render(pixel, TacticalRect);
				}
			}
		}

		if (layer == LAYER_GROUND) {

			/*
			 * The ground layer is finished, so the pieces that hang off a building -- its
			 * turret, its sand bags, whatever art is attached to it -- are laid over the
			 * top of it now, before the layers above the ground are drawn.
			 */
			for (int bindex = 0; bindex < Buildings.Count(); bindex++) {
				BuildingClass * bptr = Buildings[bindex];
				if (bptr->IsDown) {
					Point2D pixel;
					if (Coord_To_Pixel(bptr->Render_Coord(), pixel)) {
						Rect rect = TacticalRect;
						bptr->Draw_Overlays(pixel, rect);
					}
				}
			}
		}
	}
}


/// <summary>
/// Draws the terrain objects that fall within the region specified.
/// This routine is one of the tactical map's render passes. Terrain that is currently
/// animating is left to the animation layer to draw.
/// </summary>
/// <param name="forced">Is this redraw forced by outside circumstances?</param>
/// <param name="area">The screen region that has been disturbed.</param>
/// <param name="cliprect">The clipping rectangle to render within.</param>
void Tactical::Draw_Terrain(bool forced, Rect const & area, Rect const & cliprect)
{
	Rect inter = Intersect(area, cliprect);
	for (int i = DisplayClass::Layer[LAYER_GROUND].Count() - 1; i >= 0; i--) {
		ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][i];
		if (obj->RTTI == RTTI_TERRAIN && !obj->Is_Inactive() && !((TerrainClass*)obj)->Is_Animating()) {
			Rect render = obj->Get_Render_Rect() + Point2D(TacticalRect.X, TacticalRect.Y);
			if (render.Is_Overlapping(inter)) {
				Rect clip = cliprect;
				obj->Render(clip, forced, false);
				obj->IsToDisplay = false;
			}
		}
	}
}


/// <summary>
/// Draws the fogged (last-seen) objects within the given screen rectangle by
/// forwarding to the global fog-rendering routine.
/// </summary>
/// <param name="area">Screen rectangle to draw the fogged objects within.</param>
void Tactical::Draw_Fogged_Objects(Rect const & area)
{
	::Draw_Fogged_Objects(area);
}


/// <summary>
/// Draws the buildings that fall within the region specified.
/// This routine is one of the tactical map's render passes. Fogged buildings are left
/// alone, since the fog layer draws them in their remembered state instead.
/// </summary>
/// <param name="forced">Is this redraw forced by outside circumstances?</param>
/// <param name="area">The screen region that has been disturbed.</param>
/// <param name="cliprect">The clipping rectangle to render within.</param>
void Tactical::Draw_Buildings(bool forced, Rect const & area, Rect const & cliprect)
{
	Rect inter = Intersect(area, cliprect);
	for (int i = 0; i < DisplayClass::Layer[LAYER_GROUND].Count(); i++) {
		ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][i];
		if (obj->RTTI == RTTI_BUILDING && (!((BuildingClass*)obj)->IsFogged || Debug_Map)) {
			Rect render = obj->Get_Render_Rect() + Point2D(TacticalRect.X, TacticalRect.Y);
			if (render.Is_Overlapping(inter)) {
				Rect clip = cliprect;
				obj->Render(clip, forced, false);
				obj->IsToDisplay = false;
			}
		}
	}
}


/// <summary>
/// Registers the buildings overlapping the region for redraw.
/// This routine folds any building that still wants to be drawn into the dirty rectangle
/// list, so that the next render pass repaints it.
/// </summary>
/// <param name="area">The screen region that has been disturbed.</param>
void Tactical::Register_Dirty_Buildings(Rect area)
{
	for (int i = 0; i < DisplayClass::Layer[LAYER_GROUND].Count(); i++) {
		ObjectClass * obj = DisplayClass::Layer[LAYER_GROUND][i];
		if (obj->RTTI == RTTI_BUILDING && obj->IsToDisplay) {
			Rect render = obj->Get_Render_Rect();
			if (area.Is_Overlapping(render + Point2D(TacticalRect.X, TacticalRect.Y))) {
				Register_Dirty_Area(render);
				obj->IsToDisplay = false;
			}
		}
	}
}


/// <summary>
/// Is there a building the player could click on in this region?
/// </summary>
/// <param name="rect">The tactical view region to examine.</param>
/// <returns>bool; Was a selectable building found within the region?</returns>
bool Tactical::Contains_Selectable_Buildings(Rect rect)
{
	Point2D top_left;
	Point2D bottom_right;
	top_left.X = rect.X + TacPixelX;
	top_left.Y = rect.Y + TacPixelY;
	bottom_right.X = rect.X + TacPixelX + rect.Width;
	bottom_right.Y = rect.Y + TacPixelY + rect.Height;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass * bptr = Buildings[i];
		if (bptr->IsActive && bptr->IsToDisplay && bptr->IsDown) {
			Point2D point = Coord_To_Pixel_Absolute(bptr->Render_Coord());
			if (point.X >= top_left.X && point.X <= bottom_right.X && point.Y >= top_left.Y && point.Y <= bottom_right.Y) {
				return(true);
			}
		}
	}
	return(false);
}


/// <summary>
/// Adds the buildings within the region to the clickable list.
/// Buildings do not register themselves as they render, so this routine sweeps the building
/// list on their behalf. Buildings that are invisible in game are passed over.
/// </summary>
/// <param name="rect">The tactical view region to gather buildings from.</param>
void Tactical::Add_Buildings_To_Selectable(Rect rect)
{
	Point2D top_left;
	Point2D bottom_right;
	top_left.X = rect.X + TacPixelX;
	top_left.Y = rect.Y + TacPixelY;
	bottom_right.X = rect.X + TacPixelX + rect.Width;
	bottom_right.Y = rect.Y + TacPixelY + rect.Height;

	for (int i = 0; i < Buildings.Count(); i++) {
		BuildingClass *bptr = Buildings[i];
		if (bptr->IsActive  && bptr->IsDown && !bptr->Class->IsInvisibleInGame) {
			Point2D point = Coord_To_Pixel_Absolute(bptr->PositionCoord);
			if (point.X >= top_left.X && point.X <= bottom_right.X && point.Y >= top_left.Y && point.Y <= bottom_right.Y) {
				Add_To_Selectables(bptr, point - Point2D(TacPixelX, TacPixelY));
			}
		}
	}
}


/// <summary>
/// Adds an object to the list of clickable objects.
/// Objects register themselves here as they render, which gives the click handler a cheap
/// list of what is on screen and where. An object drawn well outside the tactical view is
/// turned away.
/// </summary>
/// <param name="object">The object that has just been drawn.</param>
/// <param name="point">The pixel, relative to the tactical view, it was drawn at.</param>
/// <returns>bool; Was the object added to the list?</returns>
bool Tactical::Add_To_Selectables(ObjectClass * object, Point2D point)
{
	if (point.X >= -32 && point.X <= TacticalRect.Width + 32 && point.Y >= -32 && point.Y <= TacticalRect.Height + 32) {
		SelectableObjects.push_back({object, point + Point2D(TacPixelX, TacPixelY)});
		return(true);
	}
	return(false);
}


/// <summary>
/// Begins a rubber band selection at the point specified.
/// This routine is called when the player presses the mouse button down over the tactical
/// map. A band that is already in progress is left undisturbed.
/// </summary>
/// <param name="point">The pixel the drag started at.</param>
void Tactical::Start_Rubber_Band(Point2D const & point)
{
	if (RubberBandStart == Point2D(0, 0)) {
		RubberBandStart = point;
		RubberBandEnd = point;
	}
}


/// <summary>
/// Drags the loose corner of the rubber band to a new point.
/// This routine is called as the mouse moves. It is ignored unless a band is in progress.
/// </summary>
/// <param name="point">The pixel the mouse has been dragged to.</param>
void Tactical::Modify_Rubber_Band(Point2D const & point)
{
	if (RubberBandStart != Point2D(0, 0)) {
		RubberBandEnd = point;
	}
}


/// <summary>
/// Selects every object caught within the rubber band.
/// This routine is called when the player finishes the drag. The band is normalized into a
/// proper screen rectangle, handed to the region selector, and then cleared.
/// </summary>
/// <param name="select_callback">Routine to call for each object the band caught.</param>
void Tactical::Select_Rubber_Band(void (*select_callback)(ObjectClass * object))
{
	if (RubberBandStart != Point2D(0, 0)) {

		Point2D start = RubberBandStart;
		Point2D end = RubberBandEnd;

		/*
		**	Ensure that coordinate number one represents the upper left corner
		**	and coordinate number two represents the lower right corner.
		*/
		if (start.X > end.X) {
			std::swap(start.X, end.X);
		}
		if (start.Y > end.Y) {
			std::swap(start.Y, end.Y);
		}

		Select_These(Rect(start, end.X - start.X + 1, end.Y - start.Y + 1), select_callback);
		RubberBandStart = Point2D(0, 0);
	}
}


/// <summary>
/// Abandons the rubber band selection.
/// This routine throws the band away without selecting anything. Use Select_Rubber_Band
/// instead when the player has completed the drag in earnest.
/// </summary>
void Tactical::End_Rubber_Band(void)
{
	RubberBandStart = Point2D(0, 0);
	RubberBandEnd = Point2D(0, 0);
}


/// <summary>
/// Draws the rubber band selection rectangle.
/// This routine is called during the render pass while the player is dragging out a
/// multi-select box. Nothing is drawn if no band is being dragged.
/// </summary>
void Tactical::Draw_Rubber_Band(void)
{
	if (RubberBandStart != Point2D(0, 0)) {

		Point2D start = RubberBandStart;
		Point2D end = RubberBandEnd;

		/*
		**	Ensure that coordinate number one represents the upper left corner
		**	and coordinate number two represents the lower right corner.
		*/
		if (end.X < start.X) {
			std::swap(start.X, end.X);
		}
		if (end.Y < start.Y) {
			std::swap(start.Y, end.Y);
		}

		int w = (end.X - start.X) + 1;
		int h = (end.Y - start.Y) + 1;
		int color = NormalDrawer->Convert_Pixel(15);
		Rect rect(start, w, h);
		LogicalSurface->Draw_Rect(TacticalRect, rect, color);
	}
}


/*
 * FACING_N		-> FACING_N
 * FACING_NE	-> FACING_N
 * FACING_E		-> FACING_E
 * FACING_SE	-> FACING_SE
 * FACING_S		-> FACING_S
 * FACING_SW	-> FACING_SW
 * FACING_W		-> FACING_N
 * FACING_NW	-> FACING_N
 * no move		-> FACING_NONE
 */

/// <summary>
/// Determines the direction the map is actually able to scroll in.
/// This routine is used by the scroll handler so that the view slides along the edge of the
/// map rather than stopping dead when only part of the requested movement is possible.
/// </summary>
/// <param name="dir">The direction the view has been asked to scroll in.</param>
/// <returns>Returns with the direction the view can scroll in, or FACING_NONE if it cannot
/// move at all.</returns>
FacingType Tactical::Scroll_Dir(FacingType dir)
{
	static Point2D _scroll[FACING_COUNT] = 	{
		Point2D(0, -1),
		Point2D(1, -1),
		Point2D(1, 0),
		Point2D(1, 1),
		Point2D(0, 1),
		Point2D(-1, 1),
		Point2D(-1, 0),
		Point2D(-1, -1)
	};

	static FacingType _newdir[9] = {
		FACING_NW, FACING_N, FACING_NE,
		FACING_W, FACING_NONE, FACING_E,
		FACING_SW, FACING_S, FACING_SE
	};

	Point2D oldpos = TacticalCoord;
	Point2D newpos = oldpos + _scroll[dir];

	if (newpos != oldpos) {
		Point2D diff = TacticalCoord - Clamp_Pixel_To_Tactical(newpos);
		return(_newdir[(diff.Y + 1) * 3 + diff.X + 1]);
	}
	return(dir);
}


/// This is a stub that hands back the cell it was given.

/// <summary>
/// Fetches the cell that the mouse cursor should settle on.
/// This routine is called by the cursor placement code with the cell under the mouse and a
/// list of candidates to consider alongside it.
/// </summary>
/// <returns>Returns with the cell chosen.</returns>
Cell Tactical::Clamp_Cursor_To_Tactical(Cell const & cell, Cell const * list)
{
	return(cell);
}


void Tactical::Noop(int)
{

}


/// <summary>
/// Fetches the object that the player has clicked on.
/// This routine picks the closest eligible object to the pixel specified. Objects in limbo
/// are ignored, as is any cloaked object the player has no business seeing. When nothing
/// suitable is near enough, the occupier of the cell under the pixel is returned instead.
/// </summary>
/// <param name="point">The pixel, relative to the tactical view, that was clicked on.</param>
/// <returns>Returns with a pointer to the object found, or NULL if there is nothing
/// there.</returns>
ObjectClass * Tactical::Get_Selectable_Object(Point2D const & point)
{
	ObjectClass * best = NULL;
	unsigned int bestdist = -1;

	Point2D pixel;
	pixel.X = point.X + TacPixelX;
	pixel.Y = point.Y + TacPixelY;

	for (int index = 0; index < (int)SelectableObjects.size(); index++) {
		Selectable & sel = SelectableObjects[index];
		ObjectClass * obj = sel.Object;

		if (obj != NULL) {

			TechnoClass const * tech = Dynamic_Cast<TechnoClass const *>((AbstractClass const *)obj);
			bool pass = false;

			if (tech != NULL) {
				if (obj->IsActive && !tech->IsInLimbo) {
					if (tech->IsOwnedByPlayer || tech->Cloak != CLOAKED || Map[tech->Center_Coord()].Is_Sensed(PlayerPtr->HeapID)) {
						pass = true;
					}
				}
			}

			if (tech == NULL) {
				if (obj->Class_Of() != NULL && obj->Class_Of()->RTTI == RTTI_TERRAINTYPE) {
					TerrainTypeClass * type = (TerrainTypeClass *)obj->Class_Of();
					if (type->IsVeinhole) {
						pass = true;
					}
				}
			}

			if (pass) {
				Point2D pos = sel.Position - pixel;
				unsigned int dist = int(double(pos.X * pos.X) + double(pos.Y * pos.Y) * 0.5);
				if (dist < bestdist && dist < 200) {
					bestdist = dist;
					best = obj;
				}
			}
		}
	}
	if (best != NULL) {
		return(best);
	}

	/// nothing is in selectables so get whatever is on the cell
	Cell cell = Pixel_To_Cell(TacticalRect.Top_Left() + point);
	return(Map[cell].Cell_Occupier());
}


/// <summary>
/// Removes the target from the list of clickable objects.
/// This routine is called when an object leaves the game, so that the click handler cannot
/// hand back a pointer to something that no longer exists.
/// </summary>
void Tactical::Detach(AbstractClass const * target, bool all)
{
	for (int index = 0; index < (int)SelectableObjects.size(); index++) {
		Selectable & sel = SelectableObjects[index];
		if (sel.Object == (ObjectClass *)target) {
			sel.Object = NULL;
		}
	}
}


/***********************************************************************************************
 * DisplayClass::Select_These -- All selectable objects in region are selected.                *
 *                                                                                             *
 *    Use this routine to simultaneously select all objects within the coordinate region       *
 *    specified. This routine is used by the multi-select rubber band handler.                 *
 *                                                                                             *
 * INPUT:   coord1   -- Coordinate of one corner of the selection region.                      *
 *                                                                                             *
 *          coord2   -- The opposite corner of the selection region.                           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   04/25/1995 JLB : Limited to non-building type.                                            *
 *   03/06/1996 JLB : Allows selection of aircraft with bounding box.                          *
 *=============================================================================================*/
void Tactical::Select_These(Rect const & rect, void (*select_callback)(ObjectClass * object))
{
	AllowVoice = true;

	if (rect.Is_Valid()) {

		/*
		 * Sweep through all selectable objects and select the ones within the
		 * bounding box.
		 */
		for (int index = 0; index < (int)SelectableObjects.size(); index++) {
			Selectable & sel = SelectableObjects[index];
			ObjectClass * obj = sel.Object;

			if (obj == NULL || !obj->IsActive) {
				continue;
			}

			Point2D pos = sel.Position - Point2D(TacPixelX, TacPixelY);

			/*
			**	Only try to select objects that are owned by the player, are allowed to be
			**	selected, and are within the bounding box.
			*/
			if (!rect.Is_Point_Within(pos)) {
				continue;
			}

			if (select_callback == NULL) {

				bool force = false;

				if (obj->RTTI == RTTI_BUILDING) {
					BuildingTypeClass * type = ((BuildingClass *)obj)->Class;
					if (type->UndeploysInto && !type->IsConstructionYard) {
						force = true;
					}
				}

				HouseClass * hptr = obj->Owner_HouseClass();
				if (hptr != NULL && hptr->Is_Player_Control() &&
					obj->Class_Of()->IsSelectable &&
					(obj->RTTI != RTTI_BUILDING || force)) {
					if (obj->Select()) {
						AllowVoice = false;
					}
				}
			} else {
				select_callback(sel.Object);
			}
		}
	}
	AllowVoice = true;
}


/***********************************************************************************************
 * DisplayClass::Flag_Cell -- Flag the specified cell to be redrawn.                           *
 *                                                                                             *
 *    This will flag the cell to be redrawn.                                                   *
 *                                                                                             *
 * INPUT:   cell  -- The cell to be flagged.                                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void Tactical::Flag_Cell(CellClass & cell)
{
	if (Map.DrawFlags == GS_REDRAW_DIRTY && cell.LastRedrawFrame != Frame && cell.IsMapped) {
		cell.LastRedrawFrame = Frame;

		Coord coord = cell.Cell_Coord();

		Coord ground = Coord(coord.X, coord.Y, 0);

		Point2D pixel;
		if (cell.IsMapped && cell.IsVisible) {
			Coord_To_Pixel(ground, pixel);
		} else {
			pixel = Coord_To_Pixel_Absolute(ground);
			pixel -= Point2D(TacPixelX, TacPixelY);
		}

		if (pixel.X >= TacticalRect.X - ISO_TILE_PIXEL_W / 2
			&& pixel.Y >= -ISO_TILE_PIXEL_H
			&& pixel.X <= TacticalRect.X + TacticalRect.Width + ISO_TILE_PIXEL_W / 2
			&& pixel.Y <= TacticalRect.Y + TacticalRect.Height + ISO_TILE_PIXEL_H / 2) {

			CellRedraw.push_back(&cell);
			IsToRedraw = true;
		}
	}
}


/// <summary>
/// Discards every registered dirty rectangle.
/// This routine is called when a new map is being brought up, so that nothing left over
/// from the previous one is repainted over it.
/// </summary>
void Tactical::Reset_Dirty_Rectangles(void)
{
	while (DirtyAreas.Count() > 0) {
		DirtyAreas.Delete_Index(0);
	}
}


/// <summary>
/// Draws rally point lines for the player's selected production buildings. For every selected,
/// player-owned factory that has a rally point assigned, a green dashed line is drawn from the
/// building to its rally point cell. Rally points that fall under a high bridge are raised by
/// the bridge height so the line terminates at the correct screen position.
/// </summary>
/// <param name="inshroud">Should the line be drawn through shrouded/unmapped cells?</param>
void Tactical::Draw_Rally_Points(bool inshroud)
{
	static bool _pattern[8] = {
		true,
		true,
		true,
		true,
		true,
		false,
		false,
		false
	};

	int phase = (0x7FFFFFFF - Frame) % TICKS_PER_SECOND;

	for (int index = CurrentObject.Count() - 1; index >= 0; index--) {

		ObjectClass * object = CurrentObject[index];

		/*
		 * Only player-owned, selected, factory buildings with an assigned rally
		 * point (and not currently deconstructing) should display a rally line.
		 */
		if (object->What_Am_I() != RTTI_BUILDING) {
			continue;
		}

		BuildingClass * building = (BuildingClass *)object;

		if (!building->IsActive) {
			continue;
		}

		if (!building->IsSelected) {
			continue;
		}

		if (building->House != PlayerPtr) {
			continue;
		}

		if (building->Class->ToBuild != RTTI_INFANTRYTYPE &&
			building->Class->ToBuild != RTTI_UNITTYPE &&
			building->Class->ToBuild != RTTI_AIRCRAFTTYPE) {
			continue;
		}

		if (building->ArchiveTarget == NULL) {
			continue;
		}

		if (building->CurrentMission == MISSION_DECONSTRUCTION) {
			continue;
		}

		/*
		 * Compute the screen-space start point at the building's center.
		 */
		Point2D start;
		Coord_To_Pixel(building->Center_Coord(), start);
		start += Point2D(TacticalRect.X, TacticalRect.Y);

		/*
		 * Compute the screen-space end point at the rally point. Snap the rally
		 * coordinate to the ground height and lift it onto the bridge if needed.
		 */
		Coord rally = building->ArchiveTarget->Center_Coord();
		rally.Z = Map.Get_Height_GL(rally);
		if (Map[rally].IsUnderBridge) {
			rally.Z += BRIDGE_LEPTON_HEIGHT;
		}

		Point2D end;
		Coord_To_Pixel(rally, end);
		end += Point2D(TacticalRect.X, TacticalRect.Y);

		/*
		 * Clip the line to the tactical view and draw it as a marching green dash.
		 */
		if (Clip_Line_To_Rect(start, end, TacticalRect)) {
			LogicalSurface->Draw_Masked_Dashed_Line(start, end, DSurface::Build_Hicolor_Pixel(0, 255, 0), _pattern, phase, inshroud);
		}
	}
}


/*
 * Dash pattern for the animated "marching" line that joins the selected waypoint path.
 */
static bool Draw_WaypointPoint_Line_Style[8] = {
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false
};


/// <summary>
/// Draws the local player's waypoint paths over the tactical view.
/// Each waypoint on screen is marked with the waypoint cursor and its number within the
/// path, and consecutive waypoints are joined by a line. The path the player currently has
/// selected is drawn with an animated dashed line so it stands out from the rest.
/// </summary>
/// <param name="inshroud">Draw the waypoints under shroud rather than the visible ones?</param>
void Tactical::Draw_Waypoints(bool inshroud)
{
	int phase = (0x7FFFFFFF - Frame) % TICKS_PER_SECOND;
	int frame = Map.Get_Mouse_Start_Frame(MOUSE_WAYPOINT);
	frame += WaypointAnimCounter % Map.Get_Mouse_Frame_Count(MOUSE_WAYPOINT);

	if (Debug_Map) {
		return;
	}

	/*
	 * The animated dashed-line drawer cycles a few entries near the start of the mouse
	 * drawer's translation table to make the line "march". Preserve those entries up front
	 * and restore them once every path has been drawn.
	 */
	int * dashtable = (int *)((unsigned short *)MouseDrawer->Get_Translate_Table() + 1);
	int dashsave0 = dashtable[0];
	int dashsave1 = dashtable[1];
	int dashsave2 = dashtable[2];
	int dashsave3 = dashtable[3];

	ShapeSet const * mouseshapes = (ShapeSet const *)MixFileClass::Retrieve("MOUSE.SHP");
	int coloridx = (Scen->Theater != THEATER_SNOW) ? LTGREY : BLACK;

	for (PathType path = PATH_FIRST; path < PATH_COUNT; path++) {
		PlayerPtr->Ensure_Path(path);

		int count = PlayerPtr->Paths[path]->Waypoint_Count();
		if (count != 0) {
			Map.Update_Waypoint_Color(path);
		}

		for (int index = 0; index < count; index++) {
			WaypointClass * waypoint = PlayerPtr->Paths[path]->Get_Waypoint(index);

			Point2D pixel;

			/*
			 * The waypoint cursor itself is drawn only when the waypoint lies within the
			 * (generously padded) view and on the requested side of the shroud.
			 */
			bool onscreen = false;
			if (Coord_To_Pixel(waypoint->Location, pixel)) {
				if (inshroud) {
					onscreen = Map.Is_Shrouded(waypoint->Location);
				} else {
					onscreen = !Map.Is_Shrouded(waypoint->Location);
				}
			}

			pixel.Y += TacticalRect.Y;

			if (onscreen) {
				Point2D cursor(pixel.X, pixel.Y - (TacticalRect.Y + 1));
				Draw_Shape(*LogicalSurface, *MouseDrawer, mouseshapes, frame, cursor, TacticalRect, (ShapeFlags_Type)(SHAPE_CENTER | SHAPE_WIN_REL));

				static char buffer[4];
				sprintf(buffer, "%d", index);
				Point2D label(pixel.X - 1, pixel.Y - (TacticalRect.Y + 1) - 25);
				Simple_Text_Print(buffer, *LogicalSurface, TacticalRect, label, ColorSchemes[coloridx], 0, (TextPrintType)(TPF_CENTER | TPF_EFNT), 1);
			}

			/*
			 * Join this waypoint to the next one in the path with a clipped line.
			 */
			WaypointClass * next = PlayerPtr->Paths[path]->Get_Next_Waypoint(waypoint);
			if (next != NULL) {
				Point2D nextpixel;
				Coord_To_Pixel(next->Location, nextpixel);
				nextpixel.Y += TacticalRect.Y;

				if (Clip_Line_To_Rect(pixel, nextpixel, TacticalRect)) {
					unsigned color = MouseDrawer->Convert_Pixel(3);
					if (path == PlayerPtr->SelectedPath) {
						phase = LogicalSurface->Draw_Masked_Dashed_Line(pixel, nextpixel, color, Draw_WaypointPoint_Line_Style, phase, inshroud);
					} else {
						LogicalSurface->Draw_Masked_Line(pixel, nextpixel, color, inshroud);
					}
				}
			}
		}
	}

	/*
	 * Restore the translation table entries the dashed-line drawer animated.
	 */
	dashtable[0] = dashsave0;
	dashtable[1] = dashsave1;
	dashtable[2] = dashsave2;
	dashtable[3] = dashsave3;
}


/// <summary>
/// Draws the cell occupier and object list links.
/// This routine is a debugging aid that joins each cell to the object occupying it, on the
/// ground and on any bridge above it, and chains together the objects that share a cell.
/// </summary>
void Tactical::Debug_Draw_Occupier_Links(void)
{
	Map.Reset_Iterator();
	CellClass *cellptr = Map.Iterate();
	int ocolor1 = DSurface::Build_Hicolor_Pixel(255, 255, 0);
	int lcolor = DSurface::Build_Hicolor_Pixel(255, 0, 255);
	int ocolor2 = DSurface::Build_Hicolor_Pixel(0, 255, 255);
	while (cellptr != NULL) {
		ObjectClass *object;

		/*
		 * Join the cell to whatever stands on the ground within it...
		 */
		object = cellptr->Cell_Occupier(false);
		if (object != NULL) {
			Coord coord1 = cellptr->Cell_Coord();
			Point2D start;
			Coord_To_Pixel(coord1, start);
			Coord coord2 = object->Center_Coord();
			Point2D end;
			Coord_To_Pixel(coord2, end);
			Rect rect(start - Point2D(1, 1), 3, 3);
			LogicalSurface->Fill_Rect(TacticalRect, rect, ocolor1);
			LogicalSurface->Draw_Line(TacticalRect, start, end, ocolor1);
		}

		/*
		 * ...and to whatever stands on the bridge deck above it.
		 */
		object = cellptr->Cell_Occupier(true);
		if (object != NULL) {
			Coord coord1 = cellptr->Cell_Coord();
			Point2D start;
			Coord_To_Pixel(coord1, start);
			start.Y -= CELL_PIXEL_H;
			Coord coord2 = object->Center_Coord();
			Point2D end;
			Coord_To_Pixel(coord2, end);
			Rect rect(start - Point2D(1, 1), 3, 3);
			LogicalSurface->Fill_Rect(TacticalRect, rect, ocolor2);
			LogicalSurface->Draw_Line(TacticalRect, start, end, ocolor2);
		}
		cellptr = Map.Iterate();
	}

	/*
	 * Chain together the objects that share a cell.
	 */
	for (int i = Objects.Count() - 1; i >= 0; i--) {
		ObjectClass *object = Objects[i];
		if (object->Next != NULL) {
			Coord coord1 = object->Center_Coord();
			Point2D start;
			Coord_To_Pixel(coord1, start);
			Coord coord2 = object->Next->Center_Coord();
			Point2D end;
			Coord_To_Pixel(coord2, end);
			LogicalSurface->Draw_Line(TacticalRect, start, end, lcolor);
		}
	}
}


/// <summary>
/// Draws the occupation flags of the visible cells.
/// This routine is a debugging aid that plots a small block for each occupation bit a cell
/// carries, both for the ground and for any bridge deck above it.
/// </summary>
void Tactical::Debug_Draw_Occupation_Flags(void)
{
	Map.Reset_Iterator();
	CellClass *cellptr = Map.Iterate();
	int color = DSurface::Build_Hicolor_Pixel(255, 255, 0);
	while (cellptr != NULL) {
		Rect render = cellptr->Cell_Render_Rect();
		Rect inter = Intersect(render, TacticalRect);
		if (inter.Is_Valid()) {
			Coord coord = cellptr->Cell_Coord();
			Point2D abspixel = Coord_To_Pixel_Absolute(coord);

			Rect rect(Point2D(0, TacticalRect.Y) + (abspixel - Point2D(TacPixelX + 2, TacPixelY + 2)), 4, 4);

			/*
			 * The occupation bits of the ground...
			 */
			if (cellptr->Flag.Occupy.NE) {
				LogicalSurface->Fill_Rect(rect + Point2D(10, 0), color);
			}
			if (cellptr->Flag.Occupy.SE) {
				LogicalSurface->Fill_Rect(rect + Point2D(0, 6), color);
			}
			if (cellptr->Flag.Occupy.SW) {
				LogicalSurface->Fill_Rect(rect + Point2D(-10, 0), color);
			}
			if (cellptr->Flag.Occupy.Building || cellptr->Flag.Occupy.Monolith || cellptr->Flag.Occupy.Vehicle) {
				LogicalSurface->Fill_Rect(rect, color);
			}

			/*
			 * ...and those of the bridge deck above it, plotted a level higher.
			 */
			if (cellptr->BridgeFlag.Occupy.NE) {
				LogicalSurface->Fill_Rect(rect + Point2D(10, -(CELL_PIXEL_H / 4) * BRIDGE_CELL_HEIGHT), color);
			}
			if (cellptr->BridgeFlag.Occupy.SE) {
				LogicalSurface->Fill_Rect(rect + Point2D(0, 6 - (CELL_PIXEL_H / 4) * BRIDGE_CELL_HEIGHT), color);
			}
			if (cellptr->BridgeFlag.Occupy.SW) {
				LogicalSurface->Fill_Rect(rect + Point2D(-10, -(CELL_PIXEL_H / 4) * BRIDGE_CELL_HEIGHT), color);
			}
			if (cellptr->BridgeFlag.Occupy.Building || cellptr->BridgeFlag.Occupy.Monolith || cellptr->BridgeFlag.Occupy.Vehicle) {
				LogicalSurface->Fill_Rect(rect + Point2D(0, -(CELL_PIXEL_H / 4) * BRIDGE_CELL_HEIGHT), color);
			}
		}
		cellptr = Map.Iterate();
	}
}


/// <summary>
/// Draws a depth shaded line between two world coordinates.
/// This routine converts both endpoints into screen pixels and hands them to the surface's
/// depth shaded line drawer, so the line is hidden where terrain stands in front of it.
/// </summary>
/// <param name="color">The raw color value to draw the line with.</param>
bool Tactical::Draw_3D_Line(Coord const & coord1, Coord const & coord2, int color, bool write_depth)
{
	Point2D start;
	Coord_To_Pixel(coord1, start);
	Point2D end;
	Coord_To_Pixel(coord2, end);

	LogicalSurface->Draw_Depth_Shaded_Line(TacticalRect, start, end, color,
		14 - Z_Lepton_To_Pixel(coord1.Z), 14 - Z_Lepton_To_Pixel(coord2.Z), write_depth);
	return(true);
}


/// <summary>
/// Fetches the class identifier of the tactical map.
/// This routine is used by the persistence system to recognize the object when it is read
/// back out of a save game.
/// </summary>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT STDMETHODCALLTYPE Tactical::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_TacticalMapClass;
	return(S_OK);
}


/// <summary>
/// Lists the members the tactical map carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void Tactical::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(ScreenText);
	stream.Serialize(LastAIFrame);
	stream.Serialize(field_58);
	stream.Serialize(field_59);
	stream.Serialize(TacPixelX);
	stream.Serialize(TacPixelY);
	stream.Serialize(LastTacPixelX);
	stream.Serialize(LastTacPixelY);
	stream.Serialize(ZoomFactor);
	// SelectableObjects -- the click list is built afresh by the next render pass.
	stream.Serialize(MoveFrom);
	stream.Serialize(MoveTo);
	stream.Serialize(MoveSpeed);
	stream.Serialize(MoveFactor);
	stream.Serialize(CellRedraw);
	stream.Serialize(TacticalCoord);
	stream.Serialize(LastTacticalCoord);
	stream.Serialize(DesiredTacticalCoord);
	stream.Serialize(IsFirstRender);
	stream.Serialize(IsToRedraw);
	stream.Serialize(UnusedBool);
	stream.Serialize(VisibleCellRect);
	stream.Serialize(RubberBandStart);
	stream.Serialize(RubberBandEnd);
	stream.Serialize(WaypointAnimCounter);
	stream.Serialize(WaypointAnimTimer);
	// CoordToPixelMatrix -- the isometric projections, built from constants as the map is
	// constructed.
	// PixelToCoordMatrix
	// DirtyAreas -- redraw work outstanding for the running session rather than state the map
	// owns.
	// ShadowControls
}


/// <summary>
/// Draws the radial range indicators for the selected objects.
/// This routine is used by the tactical map render pass to show the effect radius of any
/// currently selected object whose type asks for one.
/// </summary>
void Tactical::Draw_Radial_Indicators(void)
{
	for (int i = 0; i < CurrentObject.Count(); i++) {
		ObjectClass * obj = CurrentObject[i];
		if (obj->Class_Of() && obj->Class_Of()->IsHasRadialIndicator) {
			obj->Draw_Radial_Indicator();
		}
	}
}
