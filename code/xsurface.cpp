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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/wwlib/xsurface.cpp                                $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 10/16/00 11:42a                                             $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Blit_Clip -- Perform rectangle clipping in preparation for a blit.                        *
 *   XSurface::Blit_From -- Blit entire surface.                                               *
 *   XSurface::Blit_From -- Blit from one surface to this one w/ clipping.                     *
 *   XSurface::Blit_From -- Blit one region to another.                                        *
 *   XSurface::Blit_Plain -- Blit a plain rectangle from one surface to another.               *
 *   XSurface::Blit_Trans -- Blit a rectangle with transparency checking.                      *
 *   XSurface::Draw_Line -- Draw a line and perform clipping.                                  *
 *   XSurface::Draw_Line -- Draws a line upon the surface.                                     *
 *   XSurface::Draw_Rect -- Draw a rectangle with an arbitrary clipping rectangle.             *
 *   XSurface::Draw_Rect -- Draws a rectangle on the surface.                                  *
 *   XSurface::Fill -- Fill the entire surface with the color specified.                       *
 *   XSurface::Fill_Rect -- Fill a rectangle but perform clipping on the fill.                 *
 *   XSurface::Fill_Rect -- Fills a rectangle with the color specified.                        *
 *   XSurface::Get_Pixel -- Fetches a pixel from the surface.                                  *
 *   XSurface::Prep_For_Blit -- Clips and prepares pointers for a blit operation.              *
 *   XSurface::Put_Pixel -- Stores a pixel at the location specified.                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "xsurface.h"

#include "blit.h"
#include "blitblit.h"

#include <cassert>
#include <utility>

/***********************************************************************************************
 * XSurface::Draw_Line -- Draws a line upon the surface.                                       *
 *                                                                                             *
 *    This routine will draw a line on the surface between the points specified.               *
 *                                                                                             *
 * INPUT:   startpoint  -- Pixel coordinate of one end point to the line.                      *
 *                                                                                             *
 *          endpoint    -- Pixel coordinate of the other end point to the line.                *
 *                                                                                             *
 *          color       -- The color to draw the line with.                                    *
 *                                                                                             *
 * OUTPUT:  bool; Was the line drawn without error?                                            *
 *                                                                                             *
 * WARNINGS:   This routine is currently very brain-dead. It only draws vertical or            *
 *             horizontal lines. It needs to be updated to handle any angle lines and should   *
 *             perform line-clipping rather than point-pushing if the line intersects the      *
 *             clipping rectangle.                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Draw_Line(Point2D const & startpoint, Point2D const & endpoint, int color)
{
	return(XSurface::Draw_Line(Get_Rect(), startpoint, endpoint, color));
}


/***********************************************************************************************
 * XSurface::Draw_Line -- Draw a line and perform clipping.                                    *
 *                                                                                             *
 *    Use this routine to draw a line on the surface but also clip the line draw against       *
 *    an arbitrary sub-rectangle within the surface.                                           *
 *                                                                                             *
 * INPUT:   cliprect -- The clipping rectangle for this line draw.                             *
 *                                                                                             *
 *          startpoint  -- The starting point of the line draw. This point is relative to the  *
 *                         clipping rectangle.                                                 *
 *                                                                                             *
 *          endpoint    -- The ending point fo the line draw. This point is also relative to   *
 *                         the clipping rectangle.                                             *
 *                                                                                             *
 *          color    -- The screen format color value to store for each pixel of the line.     *
 *                                                                                             *
 * OUTPUT:  bool; Was the line drawn? A 'false' return value would indicate that the line      *
 *                was clipped away.                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Draw_Line(Rect const & xcliprect, Point2D const & startpoint, Point2D const & endpoint, int color)
{
	//assert(xcliprect.Is_Valid());

	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect cliprect = Intersect(xcliprect, Get_Rect());

	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	Point2D start = Bias_To(startpoint, cliprect);
	Point2D end = Bias_To(endpoint, cliprect);

	if (!Clip_Line_To_Rect(start, end, cliprect)) return(false);

	if (start.X > end.X) {
		std::swap(start, end);
	}

	int bpp = Bytes_Per_Pixel();
	void * buffer = Lock(start);
	if (buffer != NULL) {

		if (start.Y == end.Y) {

			/*
			 * Simplest of the blits, straight horizontal line.
			 */
			if (bpp == 1) {
				memset(buffer, color, end.X - start.X + 1);
			} else {
				for (int i = 0; i <= end.X - start.X; i++) {
					*((unsigned short *)buffer + i) = color;
				}
			}

		} else if (start.X == end.X) {
			int pitch = start.Y > end.Y ? -Stride() : Stride();

			/*
			 * Straight vertical line.
			 */
			int dy = abs(end.Y - start.Y);
			for (int i = 0; i <= dy; i++) {
				if (bpp == 1) {
					*(unsigned char *)buffer = color;
				} else {
					*(unsigned short *)buffer = color;
				}
				buffer = (unsigned char *)buffer + pitch;
			}
		} else {
			/*
			 * Distances to x and y.
			 */
			int dx = end.X - start.X;
			int dy = end.Y - start.Y;
			/*
			 * The line isn't straight so we need to do some maths.
			 */
			int pitch = Stride();
			if (dy < 0) {
				pitch = -pitch;
			}

			dy = abs(dy);
			int dx2 = 2 * dx;
			int dy2 = 2 * dy;

			if (dx > dy) {

				/*
				 * The slope is not steep.
				 */
				int delta = dy2 - dx;

				/*
				 * Plot low line.
				 */
				for (int i = 0; i < dx; i++) {
					if (bpp == 1) {
						*((unsigned char *)buffer + i) = color;
					} else {
						*((unsigned short *)buffer + i) = color;
					}

					if (delta > 0) {
						buffer = (unsigned char *)buffer + pitch;
						delta -= dx2;
					}

					delta += dy2;
				}
			} else {

				/*
				 * The slope is steep.
				 */
				int delta = dx2 - dy;
				int k = 0;

				/*
				 * Plot high line.
				 */
				for (int i = 0; i < dy; i++) {
					if (bpp == 1) {
						*((unsigned char *)buffer + k) = color;
					} else {
						*((unsigned short *)buffer + k) = color;
					}

					if (delta > 0) {
						k++;
						delta -= dy2;
					}

					delta += dx2;
					buffer = (unsigned char *)buffer + pitch;
				}
			}
		}

		Unlock();
		return(true);
	}
	return(false);
}


/// <summary>
/// Draws a dashed line on the surface.
/// This routine plots only those pixels that the dash pattern enables. The offset it
/// returns lets a caller chain several segments together with the dashes running unbroken
/// across the joins.
/// </summary>
/// <param name="pattern">The dash pattern. A true entry plots a pixel, a false entry
/// skips it.</param>
/// <param name="offset">The starting position within the dash pattern.</param>
/// <returns>Returns with the pattern offset to carry into the next segment.</returns>
/// <remarks>The dash pattern must hold at least 16 entries.</remarks>
int XSurface::Draw_Dashed_Line(Point2D const & startpoint, Point2D const & endpoint, unsigned color, bool pattern[], int offset)
{
	Point2D start = startpoint;
	Point2D end = endpoint;

	int initial_offset = 1;

	if (start.X > end.X) {
		std::swap(start, end);
		int norm = abs(start.X - end.X);
		if (norm < abs(start.Y - end.Y) + 1) {
			norm = abs(start.Y - end.Y) + 1;
		}
		offset = (offset + norm) % 16;
		initial_offset *= -1;
	}

	int bpp = Bytes_Per_Pixel();
	void *ptr = Lock(start);

	if (ptr != NULL) {
		if (start.Y == end.Y) {
			/*
			 * Simplest of the blits, straight horizontal line.
			 */
			for (int i = 0; i <= end.X - start.X; i++, offset += initial_offset) {
				if (offset < 0) {
					offset += 16;
				}
				if (offset >= 16) {
					offset -= 16;
				}
				if (pattern[offset]) {
					//if (bpp == 1) {
						//*((unsigned char *)ptr + i) = color;
					//} else {
						*((unsigned short *)ptr + i) = color;
					//}
				}
			}

		} else if (start.X == end.X) {
			int pitch = start.Y > end.Y ? -Stride() : Stride();
			/*
			 * Straight vertical line.
			 */
			int dy = abs(end.Y - start.Y);
			for (int i = 0; i <= dy; i++, offset += initial_offset) {
				if (offset < 0) {
					offset += 16;
				}
				if (offset >= 16) {
					offset -= 16;
				}
				if (pattern[offset]) {
					if (bpp == 1) {
						*(unsigned char *)ptr = color;
					} else {
						*(unsigned short *)ptr = color;
					}
				}
				ptr = (unsigned char *)ptr + pitch;
			}

		} else {
			/*
			 * Distances to x and y.
			 */
			int dx = end.X - start.X;
			int dy = end.Y - start.Y;
			/*
			 * The line isn't straight so we need to do some maths.
			 */
			int pitch = Stride();
			if (dy < 0) {
				pitch = -pitch;
			}

			dy = abs(dy);
			int dx2 = 2 * dx;
			int dy2 = 2 * dy;

			if (dx > dy) {

				/*
				 * The slope is not steep.
				 */
				int delta = dy2 - dx;

				/*
				 * Plot low line.
				 */
				for (int i = 0; i < dx; i++, offset += initial_offset) {

					if (offset < 0) {
						offset += 16;
					}
					if (offset >= 16) {
						offset -= 16;
					}
					if (pattern[offset]) {
						if (bpp == 1) {
							*((unsigned char *)ptr + i) = color;
						} else {
							*((unsigned short *)ptr + i) = color;
						}
					}

					if (delta > 0) {
						ptr = (unsigned char *)ptr + pitch;
						delta -= dx2;
					}

					delta += dy2;

				}
			} else {

				/*
				 * The slope is steep.
				 */
				int delta = dx2 - dy;
				int k = 0;

				/*
				 * Plot high line.
				 */
				for (int i = 0; i < dy; i++, offset += initial_offset) {

					if (offset < 0) {
						offset += 16;
					}
					if (offset >= 16) {
						offset -= 16;
					}
					if (pattern[offset]) {
						if (bpp == 1) {
							*((unsigned char *)ptr + k) = color;
						} else {
							*((unsigned short *)ptr + k) = color;
						}
					}

					if (delta > 0) {
						k++;
						delta -= dy2;
					}

					delta += dx2;
					ptr = (unsigned char *)ptr + pitch;
				}
			}
		}

		Unlock();
	}

	return(offset);
}


/// <summary>
/// Draws a dashed line, plotting only where the alpha mask allows.
/// The plain surface carries no alpha mask, so it refuses the request. DSurface supplies
/// the working version of this routine.
/// </summary>
int XSurface::Draw_Masked_Dashed_Line(Point2D const & startpoint, Point2D const & endpoint, unsigned color, bool pattern[], int offset, bool draw_on_zero_alpha)
{
	return(false);
}


/// <summary>
/// Draws a line, plotting only where the alpha mask allows.
/// The plain surface carries no alpha mask, so it refuses the request. DSurface supplies
/// the working version of this routine.
/// </summary>
bool XSurface::Draw_Masked_Line(Point2D const & startpoint, Point2D const & endpoint, unsigned color, bool draw_on_zero_alpha)
{
	return(false);
}


/// <summary>
/// Draws a line shaded between a start and an end depth.
/// The plain surface has no depth buffer to work against, so it refuses the request.
/// DSurface supplies the working version of this routine.
/// </summary>
bool XSurface::Draw_Depth_Shaded_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, unsigned color, int start_depth, int end_depth, bool write_depth)
{
	return(false);
}


/// <summary>
/// Draws a glowing line shaded between a start and an end depth.
/// The plain surface has no depth buffer to work against, so it refuses the request.
/// DSurface supplies the working version of this routine.
/// </summary>
bool XSurface::Draw_Depth_Glow_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, int glow_strength, int start_depth, int end_depth, bool write_depth)
{
	return(false);
}


/// <summary>
/// Draws an antialiased line shaded between a start and an end depth.
/// The plain surface has no depth buffer to work against, so it refuses the request.
/// DSurface supplies the working version of this routine.
/// </summary>
bool XSurface::Draw_Depth_Antialiased_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, RGBClass & color, int start_depth, int end_depth, bool write_depth, bool blend_red, bool blend_green, bool blend_blue, float intensity)
{
	return(false);
}


/// <summary>
/// Walks a clipped line, handing each point to a callback.
/// This routine is for callers that must do more at each step than store a color -- the
/// radar event boxes stamp their edges through it. Points that fall outside the clipping
/// rectangle never reach the callback.
/// </summary>
/// <param name="xcliprect">The clipping rectangle to plot within.</param>
/// <param name="drawer_callback">The routine to call for each point along the line.</param>
/// <returns>bool; Did any part of the line survive the clipping?</returns>
bool XSurface::Plot_Line(Rect const & xcliprect, Point2D & startpoint, Point2D & endpoint, void(*drawer_callback)(Point2D const &))
{
	//assert(xcliprect.Is_Valid());

	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect cliprect = Intersect(xcliprect, Get_Rect());

	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	Point2D start = Bias_To(startpoint, cliprect);
	Point2D end = Bias_To(endpoint, cliprect);

	if (!Clip_Line_To_Rect(start, end, cliprect)) return(false);

	if (start.X > end.X) {
		std::swap(start, end);
	}

	if (start.Y == end.Y) {

		int x = end.X - start.X;

		/*
		 * Simplest of the blits, straight horizontal line.
		 */
		for (int i = 0; i <= x; i++) {
			drawer_callback(Point2D(start.X + i, end.Y));
		}

	} else if (start.X == end.X) {

		/*
		 * Straight vertical line.
		 */
		int dy = abs(start.Y - end.Y);
		int y = end.Y;
		if (start.Y <= end.Y) {
			y = start.Y;
		}
		for (int i = 0; i <= dy; i++) {
			drawer_callback(Point2D(start.X, y + i));
		}
	} else {
		/*
		 * Distances to x and y.
		 */
		int x2 = end.X;
		int dx = x2 - start.X;
		int dy = end.Y - start.Y;
		/*
		 * The line isn't straight so we need to do some maths.
		 */
		int pitch = 1;
		if (dy < 0) {
			pitch = -pitch;
		}

		dy = abs(dy);
		int dx2 = 2 * dx;
		int dy2 = 2 * dy;

		if (dx > dy) {

			/*
			 * The slope is not steep.
			 */
			int delta = dy2 - dx;
			int y = start.Y;

			/*
			 * Plot low line.
			 */
			for (int i = start.X; i <= x2; i++) {
				drawer_callback(Point2D(i, y));

				if (delta > 0) {
					delta -= dx2;
					y += pitch;
				}

				delta += dy2;
			}
		} else {

			/*
			 * The slope is steep.
			 */
			int delta = dx2 - dy;
			int x = start.X;
			int y = start.Y;
			int k = 0;

			/*
			 * Plot high line.
			 */
			for (int i = 0; i <= dy; i++) {
				drawer_callback(Point2D(x + k, y));

				if (delta > 0) {
					k++;
					delta -= dy2;
				}

				delta += dx2;
				y += pitch;
			}
		}
	}

	/*
	 * This routine plots through the callback and never locks the surface, so this unlock
	 * has nothing to pair with.
	 */
	Unlock();

	return(true);
}


/***********************************************************************************************
 * XSurface::Draw_Rect -- Draws a rectangle on the surface.                                    *
 *                                                                                             *
 *    This routine will draw a line around the rectangle specified. The line will lie just     *
 *    within the rectangle.                                                                    *
 *                                                                                             *
 * INPUT:   crect -- The rectangle dimensions to use.                                          *
 *                                                                                             *
 *          color -- The color to draw the rectangle.                                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Draw_Rect(Rect const & crect, int color)
{
	return(XSurface::Draw_Rect(Get_Rect(), crect, color));
}


/***********************************************************************************************
 * XSurface::Draw_Rect -- Draw a rectangle with an arbitrary clipping rectangle.               *
 *                                                                                             *
 *    This routine will draw the rectangle but will also clip the draw against the clipping    *
 *    rectangle provided.                                                                      *
 *                                                                                             *
 * INPUT:   cliprect -- The clipping rectangle to clip the draw against.                       *
 *                                                                                             *
 *          rect     -- The rectangle to draw. The coordinates are relative to the clipping    *
 *                      rectangle.                                                             *
 *                                                                                             *
 *          color    -- The color ot use for this rectangle draw.                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Draw_Rect(Rect const & cliprect, Rect const & crect, int color)
{
	Point2D ul(crect.X, crect.Y);
	Point2D ur(crect.X+crect.Width-1, crect.Y);
	Point2D ll(crect.X, crect.Y+crect.Height-1);
	Point2D lr(crect.X+crect.Width-1, crect.Y+crect.Height-1);

	Draw_Line(cliprect, ul, ur, color);
	Draw_Line(cliprect, ul, ll, color);
	Draw_Line(cliprect, ur, lr, color);
	Draw_Line(cliprect, ll, lr, color);
	return(true);
}


/***********************************************************************************************
 * XSurface::Get_Pixel -- Fetches a pixel from the surface.                                    *
 *                                                                                             *
 *    This routine will fetch a pixel element from the surface at the location specified.      *
 *                                                                                             *
 * INPUT:   point -- Coordinate to fetch the pixel from.                                       *
 *                                                                                             *
 * OUTPUT:  Returns with the pixel value at that coordinate. The interpretation of the return  *
 *          value depends on the pixel format of the surface.                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int XSurface::Get_Pixel(Point2D const & point) const
{
	int color = 0;
	void * pointer = ((Surface*)this)->Lock(point);
	if (pointer != NULL) {
		if (Bytes_Per_Pixel() == 2) {
			color = *((unsigned short*)pointer);
		} else {
			color = *((unsigned char*)pointer);
		}
		((Surface*)this)->Unlock();
	}
	return(color);
}


/***********************************************************************************************
 * XSurface::Put_Pixel -- Stores a pixel at the location specified.                            *
 *                                                                                             *
 *    This routine will store a pixel at the coordinate specified on the surface.              *
 *                                                                                             *
 * INPUT:   point    -- The coordinate to place the pixel at.                                  *
 *                                                                                             *
 *          color    -- The pixel data to store.                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the pixel stored?                                                        *
 *                                                                                             *
 * WARNINGS:   The color value specified is the raw pixel value that will be stored. The       *
 *             format of this value is dependant upon the pixel format of the surface.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Put_Pixel(Point2D const & point, int color)
{
	void * pointer = Lock(point);
	if (pointer != NULL) {
		if (Bytes_Per_Pixel() == 2) {
			*((unsigned short*)pointer) = (unsigned short)color;
		} else {
			*((unsigned char*)pointer) = (unsigned char)color;
		}
		Unlock();
		return(true);
	}
	return(false);
}


/// <summary>
/// Fetches the color of a pixel, but only if it lies within the clipping rectangle.
/// </summary>
/// <param name="rect">The clipping rectangle that the point must fall within.</param>
/// <returns>Returns with the raw color value of the pixel, or zero if the point was
/// clipped away.</returns>
int XSurface::Get_Pixel_Clip(Point2D const & point, Rect const & rect) const
{
	unsigned pixel = 0;

	if (rect.Is_Point_Within(point)) {
		void *buffptr = Lock(point);

		if (buffptr != NULL) {
			if (Bytes_Per_Pixel() == 2) {
				pixel = *static_cast<unsigned short *>(buffptr);
			} else {
				pixel = *static_cast<unsigned char *>(buffptr);
			}
			Unlock();
		}
	}
	return(pixel);
}


/// <summary>
/// Sets the color of a pixel, but only if it lies within the clipping rectangle.
/// </summary>
/// <param name="rect">The clipping rectangle that the point must fall within.</param>
/// <returns>bool; Was the pixel plotted?</returns>
bool XSurface::Put_Pixel_Clip(Point2D const & point, int color, Rect const & rect)
{
	if (rect.Is_Point_Within(point)) {
		void *buffptr = Lock(point);

		if (buffptr != NULL) {
			if (Bytes_Per_Pixel() == 2) {
				*static_cast<unsigned short *>(buffptr) = color;
			} else {
				*static_cast<unsigned char *>(buffptr) = color;
			}
			Unlock();
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * XSurface::Fill_Rect -- Fills a rectangle with the color specified.                          *
 *                                                                                             *
 *    This routine will fill the rectangle with a specified color. The rectangle filled will   *
 *    be clipped appropriately.                                                                *
 *                                                                                             *
 * INPUT:   fillrect -- The rectangle to fill.                                                 *
 *                                                                                             *
 *          color    -- The color to use when filling the rectangle.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the rectangle filled without error?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Fill_Rect(Rect const & fillrect, int color)
{
	return(XSurface::Fill_Rect(fillrect, Get_Rect(), color));
}


/// <summary>
/// Fills a run of longwords with a color value.
/// This is the low level filler that the surface rectangle fill routines use for the bulk
/// of each row. The pointer returned lets the caller finish off an odd trailing pixel.
/// </summary>
/// <param name="count">The number of longwords to fill.</param>
/// <param name="color">The longword value to store. A 16 bit color must be doubled into
/// both halves of it.</param>
/// <returns>Returns with a pointer to just past the last longword written.</returns>
static inline void *surface_quick_fill(void *buf, int count, int color)
{
	unsigned int * ptr = (unsigned int *)buf;

	for (int index = 0; index < count; index++) {
		*ptr++ = (unsigned int)color;
	}

	return(ptr);
}


/***********************************************************************************************
 * XSurface::Fill_Rect -- Fill a rectangle but perform clipping on the fill.                   *
 *                                                                                             *
 *    This will fill a rectangle of the specified dimensions. The fill request will be         *
 *    clipped against the clipping rectangle provided.                                         *
 *                                                                                             *
 * INPUT:   cliprect -- The clipping rectangle to use for this fill.                           *
 *                                                                                             *
 *          fillrect -- The rectangle to fill with the specified color. The rectangle is       *
 *                      relative to the clipping rectangle coordinates.                        *
 *                                                                                             *
 *          color    -- The screen format pixel value to draw with.                            *
 *                                                                                             *
 * OUTPUT:  bool; Was the fill request carried out? A 'false' return value would indicate      *
 *                that the fill was clipped away.                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Fill_Rect(Rect const & cliprect, Rect const & fillrect, int color)
{
	if (!fillrect.Is_Valid()) return(false);

	/*
	**	Ensure that the requested clipping rectangle is actually legal.
	*/
	Rect wrect = Intersect(cliprect, Get_Rect());

	/*
	**	Clip the rectangle to be filled against the clipping rectangle supplied.
	*/
	Rect crect = Intersect(wrect, fillrect.Bias_To(cliprect));
	if (!crect.Is_Valid()) return(false);

	int height = crect.Height;
	int width = crect.Width;
	int pitch = Stride();

	void * buffer = Lock(crect.Top_Left());
	if (buffer != NULL) {
		if (Bytes_Per_Pixel() == 1) {
			for (int y = 0; y < crect.Height; y++) {
				memset(buffer, color, crect.Width);
				buffer = ((unsigned char *)buffer) + pitch;
			}
		} else {
			switch ((unsigned int)buffer & 3) {
				case 0: {
					int odd_pixel = width & 1;
					width >>= 1;
					int doubled_color = (color << 16) | color;
					for (int i = 0; i < height; i++) {
						void *tail = surface_quick_fill(buffer, width, doubled_color);
						if (odd_pixel) {
							*(unsigned short *)tail = color;
						}
						buffer = (unsigned char *)buffer + pitch;
					}
					break;
				}

				case 2: {
					if (width <= 1) {
						if (width == 1) {
							for (int i = 0; i < height; i++) {
								*(unsigned short *)buffer = color;
								buffer = (unsigned char *)buffer + pitch;
							}
						}
					} else {
						width--;
						int odd_pixel = width & 1;
						width >>= 1;
						int doubled_color = (color << 16) | color;
						for (int i = 0; i < height; i++) {
							*(unsigned short *)buffer = color;
							void * tail = surface_quick_fill((unsigned short *)buffer + 1, width, doubled_color);
							if (odd_pixel) {
								*(unsigned short *)tail = color;
							}
							buffer = (unsigned char *)buffer + pitch;
						}
					}
					break;
				}

				default: {
					for (int y = 0; y < height; y++) {
						for (int x = 0; x < width; x++) {
							((unsigned short*)buffer)[x] = (unsigned short)color;
						}
						buffer = ((unsigned char *)buffer) + pitch;
					}
					break;
				}
			}
		}
		Unlock();
		return(true);
	}
	return(false);
}


/// <summary>
/// Fills a rectangle with a translucent color.
/// The plain surface cannot blend what it draws with what is already there, so it refuses
/// the request. DSurface supplies the working version of this routine.
/// </summary>
bool XSurface::Fill_Rect_Trans(Rect const & rect, RGBClass const & color, unsigned int opacity)
{
	return(false);
}


/// <summary>
/// Draws a clipped ellipse on the surface.
/// This routine will draw an ellipse, centered about the point specified, using the
/// midpoint ellipse algorithm. Every pixel plotted is clipped against the clipping
/// rectangle specified.
/// </summary>
/// <param name="point">The center point of the ellipse.</param>
/// <param name="radx">The horizontal radius of the ellipse.</param>
/// <param name="rady">The vertical radius of the ellipse.</param>
/// <param name="cliprect">The clipping rectangle to draw within.</param>
/// <param name="color">The raw color value to draw the ellipse with.</param>
/// <returns>bool; Was the ellipse drawn?</returns>
bool XSurface::Draw_Ellipse(Point2D point, int radx, int rady, Rect cliprect, int color)
{
	int pitch = Stride() >> 1;

	/*
	 * Plot the bottom and top points of the ellipse if they are visible.
	 */
	int bottom = point.Y + rady;
	if (point.X >= cliprect.X && point.X < cliprect.X + cliprect.Width && bottom >= cliprect.Y && bottom < cliprect.Y + cliprect.Height) {
		Put_Pixel(Point2D(point.X, bottom), color);
	}

	int top = point.Y - rady;
	if (point.X >= cliprect.X && point.X < cliprect.X + cliprect.Width && top >= cliprect.Y && top < cliprect.Y + cliprect.Height) {
		Put_Pixel(Point2D(point.X, top), color);
	}

	/*
	 * Set up the decision variables for the flat (low slope) arcs of the
	 * ellipse that extend away from the vertical extreme points.
	 */
	int rady_squared = rady * rady;
	int radx_squared = radx * radx;
	int product = rady * radx_squared;
	int y = rady;
	int xterm = 0;
	int yterm = 2 * product;
	int decision = radx_squared / 4 - product;

	short * buffer = (short *)Lock();

	int bottom_offset = pitch * bottom;
	int top_offset = pitch * top;
	int xleft = point.X;
	short * bottom_right = &buffer[bottom_offset + point.X];
	short * top_left = &buffer[top_offset + point.X];
	short * top_right = top_left;
	int xright = point.X;
	short * bottom_left = &buffer[bottom_offset + point.X];

	while (true) {
		decision += rady_squared + xterm;
		if (decision >= 0) {
			yterm -= 2 * radx_squared;
			decision -= yterm;
			y--;

			bottom_right -= pitch;
			top_left += pitch;
			top_right += pitch;
			bottom_left -= pitch;
		}
		xterm += 2 * rady_squared;

		bottom_right++;
		top_left--;
		top_right++;
		bottom_left--;

		xright++;
		xleft--;

		if (xterm >= yterm) {
			break;
		}
		int ypos = y + point.Y;
		if (xright >= cliprect.X && xright < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*bottom_right = color;
		}
		ypos = point.Y - y;
		if (xleft >= cliprect.X && xleft < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*top_left = color;
		}
		ypos = point.Y - y;
		if (xright >= cliprect.X && xright < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*top_right = color;
		}
		ypos = point.Y + y;
		if (xleft >= cliprect.X && xleft < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*bottom_left = color;
		}
	}

	/*
	 * Plot the right and left points of the ellipse if they are visible.
	 */
	Point2D pixel;

	xright = point.X + radx;
	if (xright >= cliprect.X && xright < cliprect.X + cliprect.Width && point.Y >= cliprect.Y && point.Y < cliprect.Y + cliprect.Height) {
		pixel.X = xright;
		pixel.Y = point.Y;
		Put_Pixel(pixel, color);
	}

	xleft = point.X - radx;
	if (xleft >= cliprect.X && xleft < cliprect.X + cliprect.Width && point.Y >= cliprect.Y && point.Y < cliprect.Y + cliprect.Height) {
		pixel.X = xleft;
		pixel.Y = point.Y;
		Put_Pixel(pixel, color);
	}

	/*
	 * Set up the decision variables for the steep (high slope) arcs of the
	 * ellipse that extend away from the horizontal extreme points.
	 */
	y = 0;
	yterm = 0;
	product = radx * rady_squared;
	xterm = 2 * product;
	int x = radx;		/// This value is never used.
	decision = rady_squared / 4 - product;

	int left_offset = pitch * point.Y - radx;
	int center_offset = pitch * point.Y;
	xleft = point.X - radx;
	bottom_right = &buffer[point.X + center_offset + radx];
	top_left = &buffer[point.X + center_offset - radx];
	top_right = &buffer[point.X + center_offset + radx];
	xright = point.X + radx;
	bottom_left = &buffer[point.X + left_offset];

	while (true) {
		decision += radx_squared + yterm;
		if (decision >= 0) {
			xterm -= 2 * rady_squared;
			decision -= xterm;
			bottom_right--;
			xright--;
			top_left++;
			xleft++;
			top_right--;
			bottom_left++;
		}
		yterm += 2 * radx_squared;
		y++;

		bottom_right += pitch;
		top_left -= pitch;
		top_right -= pitch;
		bottom_left += pitch;

		if (yterm > xterm) {
			break;
		}
		int ypos = y + point.Y;
		if (xright >= cliprect.X && xright < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*bottom_right = color;
		}
		ypos = point.Y - y;
		if (xleft >= cliprect.X && xleft < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*top_left = color;
		}
		ypos = point.Y - y;
		if (xright >= cliprect.X && xright < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*top_right = color;
		}
		ypos = point.Y + y;
		if (xleft >= cliprect.X && xleft < cliprect.X + cliprect.Width && ypos >= cliprect.Y && ypos < cliprect.Y + cliprect.Height) {
			*bottom_left = color;
		}
	}

	return(Unlock());
}


/// <summary>
/// Draws a filled circle on the surface.
/// This routine fills the disc with horizontal spans rather than plotting an outline, so
/// the result is solid. Every span is clipped against the rectangle specified.
/// </summary>
/// <param name="rect">The clipping rectangle to draw within.</param>
/// <param name="color">The raw color value to fill the circle with.</param>
void XSurface::Draw_Circle(Point2D center, int radius, Rect rect, int color)
{
	Point2D pt(radius, 0);

	/*
	 * The roundness factor of the circle.
	 * 0 is circle. 50 is rect.
	 */
	const int roundness_val = 2;

	/*
	 * Calculate start decision delta.
	 */
	int d = 3 - (roundness_val * radius);

	do {
		Draw_Line(rect, center + Point2D(-pt.X,  pt.Y), center + Point2D(pt.X,  pt.Y), color);
		Draw_Line(rect, center + Point2D(-pt.Y,  pt.X), center + Point2D(pt.Y,  pt.X), color);
		Draw_Line(rect, center + Point2D(-pt.X, -pt.Y), center + Point2D(pt.X, -pt.Y), color);
		Draw_Line(rect, center + Point2D(-pt.Y, -pt.X), center + Point2D(pt.Y, -pt.X), color);

		/*
		 * Check decision and update it, x and y.
		 */
		if (d < 0) {

			/*
			 * Calculate delta for vertical pixel.
			 */
			d += (4 * pt.Y) + 6;

		} else {

			/*
			 * Calculate delta for diagonal pixel.
			 */
			d += 4 * (pt.Y - pt.X) + 10;
			pt.X--;
		}

		pt.Y++;

	} while (pt.X >= pt.Y);
}


/***********************************************************************************************
 * XSurface::Fill -- Fill the entire surface with the color specified.                         *
 *                                                                                             *
 *    The color specified will be filled into the entire surface area (but limited by the      *
 *    surface's current window).                                                               *
 *                                                                                             *
 * INPUT:   color -- The pixel value to use when filling the surface.                          *
 *                                                                                             *
 * OUTPUT:  bool; Was the surface fill performed without error?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Fill(int color)
{
	return(Fill_Rect(Get_Rect(), Get_Rect(), color));
}


/***********************************************************************************************
 * XSurface::Blit_From -- Blit entire surface.                                                 *
 *                                                                                             *
 *    This routine will blit the entire surface.                                               *
 *                                                                                             *
 * INPUT:   source   -- Pointer to the source surface to blit from.                            *
 *                                                                                             *
 *          trans    -- Perform a transparency aware blit?                                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the surface blit performed without error?                                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Blit_From(Surface const & source, bool trans, bool unknown)
{
	Rect drect = Get_Rect();
	drect.X = 0;
	drect.Y = 0;

	Rect srect = source.Get_Rect();
	srect.X = 0;
	srect.Y = 0;
	bool result = Blit_From(drect, source, srect, trans, unknown);

	return(result);
}


/***********************************************************************************************
 * XSurface::Blit_From -- Blit one region to another.                                          *
 *                                                                                             *
 *    Use this routine to copy one rectangle of pixel data to another. The pixels being        *
 *    copied may be processed according to the parameters specified.                           *
 *                                                                                             *
 * INPUT:   destrect    -- The destination rectangle to bit to.                                *
 *                                                                                             *
 *          source      -- Pointer to the source surface to blit from.                         *
 *                                                                                             *
 *          sourcerect  -- The source rectangle to blit from.                                  *
 *                                                                                             *
 *          trans       -- Should transparent pixels be scanned for an skipped?                *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit operation performed without error?                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Blit_From(Rect const & destrect, Surface const & source, Rect const & sourcerect, bool trans, bool unknown)
{
	return(XSurface::Blit_From(Get_Rect(), destrect, source, source.Get_Rect(), sourcerect, trans, unknown));
}


/***********************************************************************************************
 * XSurface::Blit_From -- Blit from one surface to this one w/ clipping.                       *
 *                                                                                             *
 *    This routine will blit a clipped rectangle from the specified surface to this one.       *
 *                                                                                             *
 * INPUT:   dcliprect   -- The clipping rectangle to use for this (destination) surface.       *
 *                                                                                             *
 *          destrect    -- The destanation rect of the blit. Coordinates are relative to the   *
 *                         destination clipping rectangle.                                     *
 *                                                                                             *
 *          source      -- The source surface to blit from.                                    *
 *                                                                                             *
 *          scliprect   -- The source clipping rectangle for the blit.                         *
 *                                                                                             *
 *          sourcrect   -- The source rectangle of the blit. The coordinates are relative to   *
 *                         the source clipping rectangle.                                      *
 *                                                                                             *
 *          trans       -- Is this a transparent pixel aware blit request?                     *
 *                                                                                             *
 * OUTPUT:  bool; Was a blit performed? A 'false' value would mean that the blit was clipped   *
 *                away.                                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Blit_From(Rect const & dcliprect, Rect const & destrect, Surface const & source, Rect const & scliprect, Rect const & sourcerect, bool trans, bool)
{
	Rect drect = destrect;
	Rect srect = sourcerect;

	/*
	**	Perform any clipping against the clipping rectangles. Proceed with the blit only
	**	if there are pixels left unclipped.
	*/
	if (Blit_Clip(drect, dcliprect, srect, scliprect)) {

		if (trans) {
			return(Blit_Trans(*this, drect, source, srect));
		}
		return(Blit_Plain(*this, drect, source, srect));
	}
	return(false);
}


/***********************************************************************************************
 * Blit_Clip -- Perform rectangle clipping in preparation for a blit.                          *
 *                                                                                             *
 *    This routine will take a source rectangle and a source clipping window and intersect     *
 *    these with a dest rectangle and a dest clipping window. The effect will be to alter      *
 *    the source and dest rectangles so that they will stay within the clipping rectangles     *
 *    imposed upon the source and destination surfaces. The process clips the rectangles       *
 *    rather than displacing them when performing its adjustment. It is possible that one      *
 *    or both rectangles are clipped into oblivion by this routine. This condition will be     *
 *    flagged with the return value.                                                           *
 *                                                                                             *
 * INPUT:   drect    -- Reference to the destination rectangle (relative coordinates to the    *
 *                      destination window). This is both an input and output parameter.       *
 *                                                                                             *
 *          dwindow  -- The destination window to clip the dest rect against.                  *
 *                                                                                             *
 *          srect    -- Reference to the source rectangle (relative to the source window).     *
 *                      This is both an input and output parameter.                            *
 *                                                                                             *
 *          swindow  -- The source window to clip the srect against.                           *
 *                                                                                             *
 * OUTPUT:  bool; Was the clip performed and the srect and drect parameters still valid.       *
 *                i.e., they represent at least one pixel that has not been clipped away.      *
 *                                                                                             *
 * WARNINGS:   The rectangles may be clipped into nothingness by this routine. Be sure to      *
 *             check the return value for this condition and perform no blit operation in      *
 *             that case.                                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Blit_Clip(Rect & drect, Rect const & dwindow, Rect & srect, Rect const & swindow)
{
	/*
	**	If the rectangles are of the same dimensions, then a coordinated clipping process is
	**	possible. This results in an intelligent blit even if the source or destination
	**	rectangles is partially outside the clipping rectangle.
	*/
	if (drect.Width == srect.Width && drect.Height == srect.Height) {
		/*
		**	Clip the left and top edges against the clipping window.
		*/
		if (drect.X < 0) {
			srect.X += -drect.X;
			srect.Width -= -drect.X;
			drect.Width -= -drect.X;
			drect.X = 0;
		}
		if (drect.Y < 0) {
			srect.Y += -drect.Y;
			srect.Height -= -drect.Y;
			drect.Height -= -drect.Y;
			drect.Y = 0;
		}

		/*
		**	Clip the right and bottom edges if they spill past the
		**	clipping window boundaries.
		*/
		int rightspill = (drect.X+drect.Width) - dwindow.Width;
		if (rightspill > 0) {
			srect.Width -= rightspill;
			drect.Width -= rightspill;
		}
		int bottomspill = (drect.Y+drect.Height) - dwindow.Height;
		if (bottomspill > 0) {
			srect.Height -= bottomspill;
			drect.Height -= bottomspill;
		}

		/*
		**	Clip the left and top edges against the clipping
		**	window.
		*/
		if (srect.X < 0) {
			drect.X += -srect.X;
			srect.Width -= -srect.X;
			drect.Width -= -srect.X;
			srect.X = 0;
		}
		if (srect.Y < 0) {
			drect.Y += -srect.Y;
			srect.Height -= -srect.Y;
			drect.Height -= -srect.Y;
			srect.Y = 0;
		}

		/*
		**	Clip the right and bottom edges agaist the clipping window.
		*/
		rightspill = (srect.X+srect.Width) - swindow.Width;
		if (rightspill > 0) {
			srect.Width -= rightspill;
			drect.Width -= rightspill;
		}
		bottomspill = (srect.Y+srect.Height) - swindow.Height;
		if (bottomspill > 0) {
			srect.Height -= bottomspill;
			drect.Height -= bottomspill;
		}

	} else {

		/*
		**	Since the rectangles are not of the same dimensions, scaling is presumed. Clipping
		**	in such a case is merely a legality clip against the bounding rectangle. No coordinated
		**	adjustments can occur. For best results, boundary clipping should be performed prior to
		**	calling this routine.
		*/
		drect = Intersect(drect, dwindow);
		srect = Intersect(srect, swindow);
	}

	return(drect.Is_Valid() && srect.Is_Valid());
}


/***********************************************************************************************
 * XSurface::Prep_For_Blit -- Clips and prepares pointers for a blit operation.                *
 *                                                                                             *
 *    This performs the clipping operation required before a blit occurs. It examines the      *
 *    source and destination coordinate constraints and performs clipping such that only       *
 *    legal pixels will be blitted. As a consequence it can determine if the blit has been     *
 *    completely clipped and thus can be skipped.                                              *
 *                                                                                             *
 * INPUT:   dest     -- The destination surface rect for the blit.                             *
 *                                                                                             *
 *          drect    -- The rectangle within the destination surface rect for the blit. This   *
 *                      rectangle reference will be adjusted as necessary.                     *
 *                                                                                             *
 *          source   -- The source surface rect of the blit.                                   *
 *                                                                                             *
 *          srect    -- The source rectangle within the source surface rect for the blit.      *
 *                      This rectangle reference will be adjusted as necessary.                *
 *                                                                                             *
 *          overlapped  -- Output reference that stores the boolean answer to the question --  *
 *                         are the blit rectangles overlapping on the same surface?            *
 *                                                                                             *
 *          dbuffer  -- Output reference that stores a pointer to the locked destination       *
 *                      surface. It points to the upper left destination corner pixel.         *
 *                                                                                             *
 *          sbuffer  -- Output reference that stores a pointer to the locked source surface.   *
 *                      It points to the upper left corner source pixel.                       *
 *                                                                                             *
 * OUTPUT:  bool; Can the blit proceed since there is at least one pixel that has not been     *
 *                clipped away? It can also fail if a lock could not be performed on the       *
 *                source or destination surfaces.                                              *
 *                                                                                             *
 * WARNINGS:   The surfaces locked by this routine must be unlocked. If the return value is    *
 *             'true', then the surfaces must be unlocked.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Prep_For_Blit(Surface & dest, Rect & drect, Surface const & source, Rect & srect, bool & overlapped, void * & dbuffer, void * & sbuffer)
{
	return(XSurface::Prep_For_Blit(dest, dest.Get_Rect(), drect, source, source.Get_Rect(), srect, overlapped, dbuffer, sbuffer));
#ifdef NEVER
	overlapped = false;
	dbuffer = NULL;
	sbuffer = NULL;

	if (!drect.Is_Valid() || !srect.Is_Valid()) return(false);

	/*
	**	Perform the clipping of the desired blit rectangles against the surface clipping
	**	rectangles. If it happens that the blit is clipped into oblivion, then bail
	**	immediately -- there is nothing left to do.
	*/
	Rect swindow = source.Get_Rect();
	Rect dwindow = dest.Get_Rect();
	if (!Blit_Clip(drect, dwindow, srect, swindow)) {
		return(false);
	}

	/*
	**	Determine if the rectangles overlap such that a forward blit would
	**	be prohibited. This only occurs if the source and destination refer to the
	**	same surface and the rectangles overlap.
	*/
	overlapped = false;
	if (&source == &dest && srect.Is_Overlapping(drect)) {
		if (srect.Y < drect.Y || (srect.Y == drect.Y && srect.X < drect.X)) {
			overlapped = true;
		}
	}

	/*
	**	Fetch pointers to the source and dest upper left pixel. That is, the upper
	**	left pixel of the source and dest sub-rectangles within each surface
	**	respectively.
	*/
	dbuffer = dest.Lock(drect.Point());
	if (dbuffer == NULL) return(false);
	sbuffer = ((Surface &)source).Lock(srect.Point());
	if (sbuffer == NULL) {
		dest.Unlock();
		return(false);
	}

	return(true);
#endif
}


/***********************************************************************************************
 * XSurface::Prep_For_Blit -- Clips and prepares pointers for a blit operation.                *
 *                                                                                             *
 *    This performs the clipping operation required before a blit occurs. It examines the      *
 *    source and destination coordinate constraints and performs clipping such that only       *
 *    legal pixels will be blitted. As a consequence it can determine if the blit has been     *
 *    completely clipped and thus can be skipped.                                              *
 *                                                                                             *
 * INPUT:   dest     -- The destination surface rect for the blit.                             *
 *                                                                                             *
 *          dcliprect-- The destination clipping rectangle.                                    *
 *                                                                                             *
 *          drect    -- The rectangle within the destination surface rect for the blit. This   *
 *                      rectangle reference will be adjusted as necessary.                     *
 *                                                                                             *
 *          source   -- The source surface rect of the blit.                                   *
 *                                                                                             *
 *          scliprect-- The source clipping rectangle.                                         *
 *                                                                                             *
 *          srect    -- The source rectangle within the source surface rect for the blit.      *
 *                      This rectangle reference will be adjusted as necessary.                *
 *                                                                                             *
 *          overlapped  -- Output reference that stores the boolean answer to the question --  *
 *                         are the blit rectangles overlapping on the same surface?            *
 *                                                                                             *
 *          dbuffer  -- Output reference that stores a pointer to the locked destination       *
 *                      surface. It points to the upper left destination corner pixel.         *
 *                                                                                             *
 *          sbuffer  -- Output reference that stores a pointer to the locked source surface.   *
 *                      It points to the upper left corner source pixel.                       *
 *                                                                                             *
 * OUTPUT:  bool; Can the blit proceed since there is at least one pixel that has not been     *
 *                clipped away? It can also fail if a lock could not be performed on the       *
 *                source or destination surfaces.                                              *
 *                                                                                             *
 * WARNINGS:   The surfaces locked by this routine must be unlocked. If the return value is    *
 *             'true', then the surfaces must be unlocked.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Prep_For_Blit(Surface & dest, Rect const & dcliprect, Rect & drect, Surface const & source, Rect const & scliprect, Rect & srect, bool & overlapped, void * & dbuffer, void * & sbuffer)
{
	overlapped = false;
	dbuffer = NULL;
	sbuffer = NULL;

	if (!drect.Is_Valid() || !dcliprect.Is_Valid() || !srect.Is_Valid() || !scliprect.Is_Valid()) return(false);

	/*
	**	Perform the clipping of the desired blit rectangles against the surface clipping
	**	rectangles. If it happens that the blit is clipped into oblivion, then bail
	**	immediately -- there is nothing left to do.
	*/
	if (!Blit_Clip(drect, dcliprect, srect, scliprect)) {
		return(false);
	}

	/*
	**	Determine if the rectangles overlap such that a forward blit would
	**	be prohibited. This only occurs if the source and destination refer to the
	**	same surface and the rectangles overlap.
	*/
	overlapped = false;
	if (&source == &dest && srect.Is_Overlapping(drect)) {
		if (srect.Y < drect.Y || (srect.Y == drect.Y && srect.X < drect.X)) {
			overlapped = true;
		}
	}

	/*
	**	Fetch pointers to the source and dest upper left pixel. That is, the upper
	**	left pixel of the source and dest sub-rectangles within each surface
	**	respectively.
	*/
	dbuffer = dest.Lock(dcliprect.Top_Left() + drect.Top_Left());
	if (dbuffer == NULL) return(false);
	sbuffer = source.Lock(scliprect.Top_Left() + srect.Top_Left());
	if (sbuffer == NULL) {
		dest.Unlock();
		return(false);
	}

	return(true);
}


/***********************************************************************************************
 * XSurface::Blit_Plain -- Blit a plain rectangle from one surface to another.                 *
 *                                                                                             *
 *    This routine will perform a simple blit of a rectangle from one surface to another.      *
 *                                                                                             *
 * INPUT:   dest     -- The destination surface for the blit.                                  *
 *                                                                                             *
 *          destrect -- The destination rectangle for the blit.                                *
 *                                                                                             *
 *          source   -- The source surface.                                                    *
 *                                                                                             *
 *          sourcerect  -- The rectangle in the source surface to blit from.                   *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Blit_Plain(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect)
{
	if (dest.Bytes_Per_Pixel() == 1) {
		return(Bit_Blit(dest, destrect, source, sourcerect, BlitPlain<unsigned char>()));
	}
	return(Bit_Blit(dest, destrect, source, sourcerect, BlitPlain<unsigned short>()));
}


/***********************************************************************************************
 * XSurface::Blit_Trans -- Blit a rectangle with transparency checking.                        *
 *                                                                                             *
 *    This routine will perform a simple blit of one rectangle to another on the surfaces      *
 *    specified.                                                                               *
 *                                                                                             *
 * INPUT:   dest     -- The destination surface.                                               *
 *                                                                                             *
 *          destrect -- The destination rectangle within the surface.                          *
 *                                                                                             *
 *          source   -- The source surface.                                                    *
 *                                                                                             *
 *          sourcerect  -- The source rectangle.                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed?                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool XSurface::Blit_Trans(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect)
{
	if (dest.Bytes_Per_Pixel() == 1) {
		return(Bit_Blit(dest, destrect, source, sourcerect, BlitTrans<unsigned char>()));
	}
	return(Bit_Blit(dest, destrect, source, sourcerect, BlitTrans<unsigned short>()));
}


typedef int OutCode;

/*
**	Build bits that indicate which end points lie outside the clipping rectangle.
**	Quick checks against these flag bits will speed the clipping process.
*/
const int CODE_INSIDE = 0;	/// 0000
const int CODE_LEFT = 1;	/// 0001
const int CODE_RIGHT = 2;	/// 0010
const int CODE_BOTTOM = 4;	/// 0100
const int CODE_TOP = 8;		// 1000

OutCode Compute_Code(double x, double y, Rect const & rect);


/*
 * Modified version of:
 * https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
 *
 * From the book "Computer Graphics. Principles and Practice in C".
 */

/// <summary>
/// Clips a line down to the part of it that lies within a rectangle.
/// This routine is used by the surface line drawers to trim a line before any pixel is
/// plotted, using the Cohen-Sutherland algorithm. Both end points are dragged in to the
/// rectangle edges as required.
/// </summary>
/// <param name="point1">The start point of the line; adjusted to the clipped start.</param>
/// <param name="point2">The end point of the line; adjusted to the clipped end.</param>
/// <param name="rect">The clipping rectangle to trim the line against.</param>
/// <returns>bool; Does any part of the line fall within the rectangle?</returns>
bool Clip_Line_To_Rect(Point2D & point1, Point2D & point2, Rect const & rect)
{
	int outcode0;
	int outcode1;
	int outcodeOut;

	double x = rect.X;
	double y = rect.Y;

	double x0 = point1.X;
	double y0 = point1.Y;
	double x1 = point2.X;
	double y1 = point2.Y;

	double slope_y = (x1 - x0) / (y1 - y0); /// slope to use for possibly-vertical lines
	double slope_x = (y1 - y0) / (x1 - x0); /// slope to use for possibly-horizontal lines

	/*
	 * Compute outcodes for P0, P1, and whatever point lies outside the clip rectangle.
	 */
	outcode0 = Compute_Code(x0, y0, rect);
	outcode1 = Compute_Code(x1, y1, rect);

AGAIN:
	if (outcode0 == CODE_INSIDE && outcode1 == CODE_INSIDE) {
		/*
		 * Both points inside window; trivially accept and return true.
		 */
		point1.X = (int)x0;
		point1.Y = (int)y0;
		point2.X = (int)x1;
		point2.Y = (int)y1;
		return(true);
	}

	/*
	**	Check to see if the line segment falls outside of the viewing rectangle.
	*/
	if (outcode0 & outcode1) {

		/*
		 * Bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
		 * or BOTTOM), so both must be outside window; exit loop (result is false).
		 */
		return(false);
	}

	/*
	 * Failed both tests, so calculate the line segment to clip
	 * from an outside point to an intersection with clip edge.
	 */

	/*
	 * At least one endpoint is outside the clip rectangle; pick it.
	 */
	outcodeOut = (outcode0 != CODE_INSIDE) ? outcode0 : outcode1;

	/*
	 * Now find the intersection point;
	 * use formulas:
	 * slope = (y1 - y0) / (x1 - x0)
	 * x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
	 * y = y0 + slope * (xm - x0), where xm is xmin or xmax
	 * No need to worry about divide-by-zero because, in each case, the
	 * outcode bit being tested guarantees the denominator is non-zero
	 */
	if (outcodeOut & CODE_TOP) {			/// point is above the clip window
		x = (rect.Y - y0) * slope_y + x0;
		y = rect.Y;
	} else if (outcodeOut & CODE_BOTTOM) {  /// point is below the clip window
		x = ((rect.Y + rect.Height - 1) - y0) * slope_y + x0;
		y = (rect.Height + rect.Y - 1);
	} else if (outcodeOut & CODE_RIGHT) {   /// point is to the right of clip window
		y = ((rect.X + rect.Width - 1) - x0) * slope_x + y0;
		x = (rect.Width + rect.X - 1);
	} else if (outcodeOut & CODE_LEFT) {	/// point is to the left of clip window
		y = (rect.X - x0) * slope_x + y0;
		x = rect.X;
	}

	/*
	 * Now we move outside point to intersection point to clip
	 * and get ready for next pass.
	 */
	if (outcodeOut == outcode0) {
		x0 = x;
		y0 = y;
		outcode0 = Compute_Code(x0, y0, rect);
	} else {
		x1 = x;
		y1 = y;
		outcode1 = Compute_Code(x1, y1, rect);
	}
	goto AGAIN;

	return(false);
}


/// <summary>
/// Determines which edges of the clipping rectangle a point lies beyond.
/// This routine is used by the line clipper to decide whether a point can be trivially
/// accepted or which edge it must be dragged back to.
/// </summary>
/// <param name="rect">The clipping rectangle to test the point against.</param>
/// <returns>Returns with the CODE_ bits for every edge crossed, or CODE_INSIDE if the
/// point lies within the rectangle.</returns>
OutCode Compute_Code(double x, double y, Rect const & rect)
{
	OutCode code = CODE_INSIDE;

	int xx = rect.X + rect.Width;
	if (x >= xx) {
		code |= CODE_RIGHT;
	} else if (x < rect.X) {
		code |= CODE_LEFT;
	}

	int yy = rect.Y + rect.Height;
	if (y >= yy) {
		code |= CODE_BOTTOM;
	} else if (y < rect.Y) {
		code |= CODE_TOP;
	}

	return(code);
}
