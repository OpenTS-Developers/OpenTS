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
 *                     $Archive:: /G/wwlib/blit.cpp                                           $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/15/99 10:13a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Bit_Blit -- Blit a block of pixels to the destination surface.                            *
 *   Bit_Blit -- Blits data to a surface w/ clipping.                                          *
 *   Buffer_Size -- Determines size of buffer for given dimensions.                            *
 *   From_Buffer -- Copy graphic data from a buffer to a surface.                              *
 *   RLE_Blit -- Blits RLE compressed data without extra clipping.                             *
 *   RLE_Blit -- Blits a rectangle of RLE compressed data to a surface.                        *
 *   To_Buffer -- Copies a graphic region into a linear RAM buffer.                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "blit.h"

#include "_alpha.h"
#include "_zbuffer.h"
#include "bsurface.h"
#include "rect.h"

#include <algorithm>


/***********************************************************************************************
 * Buffer_Size -- Determines size of buffer for given dimensions.                              *
 *                                                                                             *
 *    This routine will determine the byte size of a buffer if it were to hold the pixels      *
 *    of the dimensions specified. It takes into account the bytes per pixel.                  *
 *                                                                                             *
 * INPUT:   surface  -- The surface to base the buffer size calculation upon.                  *
 *                                                                                             *
 *          width    -- Pixel width of a graphic region.                                       *
 *                                                                                             *
 *          height   -- Pixel height of a graphic region.                                      *
 *                                                                                             *
 * OUTPUT:  Returns with the number of bytes such a region would consume if it were linearly   *
 *          packed into a memory buffer.                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int Buffer_Size(Surface const & surface, int width, int height)
{
	return(width * height * surface.Bytes_Per_Pixel());
}


/***********************************************************************************************
 * To_Buffer -- Copies a graphic region into a linear RAM buffer.                              *
 *                                                                                             *
 *    This routine will copy the graphic rectangle specified, into a RAM buffer. The size of   *
 *    the RAM buffer must be big enough to hold the pixel data. Use the Buffer_Size() function *
 *    to determine how big it must be.                                                         *
 *                                                                                             *
 * INPUT:   surface  -- The surface to copy the pixel data from.                               *
 *                                                                                             *
 *          rect     -- The graphic rectangle to copy from.                                    *
 *                                                                                             *
 *          buffer   -- Reference to the buffer that will be filled with the pixel data.       *
 *                                                                                             *
 * OUTPUT:  bool; Was the data copy performed without error?                                   *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool To_Buffer(Surface const & surface, Rect const & rect, Buffer & buffer)
{
	if (!rect.Is_Valid()) return(false);

	BSurface from(rect.Width, rect.Height, surface.Bytes_Per_Pixel(), buffer);
	return(from.Blit_From(Rect(0, 0, rect.Width, rect.Height), surface, rect));
}


/***********************************************************************************************
 * From_Buffer -- Copy graphic data from a buffer to a surface.                                *
 *                                                                                             *
 *    This routine will take pixel data and move it from the specified buffer and into the     *
 *    surface rectangle specified. It is the counterpart routine of To_Buffer().               *
 *                                                                                             *
 * INPUT:   surface  -- The surface to store the pixel data to.                                *
 *                                                                                             *
 *          rect     -- The destination rectangle to store the pixel data to.                  *
 *                                                                                             *
 *          buffer   -- Reference to the buffer that contains the pixel data.                  *
 *                                                                                             *
 * OUTPUT:  bool; Was the pixel data copy performed without error?                             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool From_Buffer(Surface & surface, Rect const & rect, Buffer const & buffer)
{
	if (!rect.Is_Valid()) return(false);

	BSurface from(rect.Width, rect.Height, surface.Bytes_Per_Pixel(), buffer);
	return(surface.Blit_From(rect, from, Rect(0, 0, rect.Width, rect.Height)));
}


/***********************************************************************************************
 * Bit_Blit -- Blits data to a surface w/ clipping.                                            *
 *                                                                                             *
 *    This routine will take source pixel data and blit it to the surface specified while      *
 *    also performing clipping on both the source and the destination data. Typical users of   *
 *    this routine would be to draw shape (sprite) data.                                       *
 *                                                                                             *
 * INPUT:   dest     -- Destintaion surface rect. This specifies the destination surface and   *
 *                      any coordinate clipping rectangle.                                     *
 *                                                                                             *
 *          destrect -- The destination rectangle of the blit. The coordinates are relative    *
 *                      to the destination clipping rectangle.                                 *
 *                                                                                             *
 *          source   -- Source surface rect. This specifies the source surface as well as any  *
 *                      clipping rectangle it may contain.                                     *
 *                                                                                             *
 *          srcrect  -- The rectange, relative to the source clipping rectangle, that          *
 *                      specifies the source blit data. It is presumed that the dimensions of  *
 *                      the source rectangle are the same as the destination rectangle.        *
 *                                                                                             *
 *          blitter  -- The blitter to use for moving the source pixels to the destination     *
 *                      surface.                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed even if it was for only a single pixel. Failure would *
 *                indicate that the blit was completely clipped away.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Bit_Blit(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect, Blitter const & blitter, int zdepth, ZGradientType zgrad, int alpha, int unknown)
{
	return(Bit_Blit(dest, dest.Get_Rect(), destrect, source, source.Get_Rect(), sourcerect, blitter, zdepth, zgrad, alpha, unknown));
}


/***********************************************************************************************
 * Bit_Blit -- Blit a block of pixels to the destination surface.                              *
 *                                                                                             *
 *    This routine will blit a block of pixels and perform clipping on the blit as controlled  *
 *    by the clipping rectangles.                                                              *
 *                                                                                             *
 * INPUT:   dest     -- Surface to blit to.                                                    *
 *                                                                                             *
 *          dcliprect-- The destination surface clipping rectangle.                            *
 *                                                                                             *
 *          ddrect   -- The destination rect of the blit. It is relative to the clipping       *
 *                      rectangle and will be clipped against same.                            *
 *                                                                                             *
 *          source   -- The source surface to blit from.                                       *
 *                                                                                             *
 *          scliprect-- The source surface clipping rectangle.                                 *
 *                                                                                             *
 *          ssrect   -- The source rectangle of the blit. It is relative to the source         *
 *                      clipping rectangle and will be clipped against same.                   *
 *                                                                                             *
 *          blitter  -- The blitter to use for blitting of this rectangle.                     *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed? A 'false' return value would indicate that the       *
 *                blit was clipped into nothing.                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Bit_Blit(Surface & dest, Rect const & dcliprect, Rect const & ddrect, Surface const & source, Rect const & scliprect, Rect const & ssrect, Blitter const & blitter, int zdepth, ZGradientType zgrad, int alpha, int)
{
	Rect srect = ssrect;
	Rect drect = ddrect;
	bool overlapped = false;
	void * dbuffer = NULL;
	void * sbuffer = NULL;
	uintptr_t zbuffer_offset = 0;
	uintptr_t abuffer_offset = 0;
	int current_z = 0;
	int zbuffer_pitch = 0;
	int abuffer_pitch = 0;
	int z_fraction = 0;
	int z_step_denominator = 0;
	int z_step_numerator = 0;
	int z_step_ratio = 0;
	ZGradStruct * gradient = &ZGradients[zgrad];
	Rect origdrect = ddrect;

	/*
	**	Prepare for the blit by performing any clipping as well as fetching pointers into the
	**	pixel buffers. If there were any errors, then this blit cannot be performed.
	*/
	if (!XSurface::Prep_For_Blit(dest, dcliprect, drect, source, scliprect, srect, overlapped, dbuffer, sbuffer)) {
		return(false);
	}

	/*
	 * Compute the alpha buffer offset if an alpha buffer is active.
	 */
	if (AlphaBuffer != NULL) {
		Point2D pt = dcliprect.Top_Left() + drect.Top_Left();
		pt.Y -= AlphaBuffer->Bounds.Y;
		abuffer_offset = AlphaBuffer->Get_Buffer_Offset(pt);
	}

	/*
	 * Initialize per-pixel depth (Z) interpolation for this blit.
	 */
	if (DepthBuffer != NULL) {

		Point2D pt = dcliprect.Top_Left() + drect.Top_Left();
		pt.Y -= DepthBuffer->Bounds.Y;
		zbuffer_offset = DepthBuffer->Get_Buffer_Offset(pt);

		z_fraction = 0;
		z_step_denominator = gradient->StepDenominator;
		z_step_numerator = gradient->StepNumerator;
		z_step_ratio = z_step_numerator / z_step_denominator;

		/*
		 * Simple top-down interpolation.
		 */
		if (gradient->IsTopDown) {
			current_z = (unsigned short)DepthBuffer->Get_Scroll_Delta(dcliprect.Y + drect.Y - DepthBuffer->Bounds.Y);
			current_z += zdepth;
			current_z = (current_z / gradient->LineIncrement) * gradient->LineIncrement;
		}

		/*
		 * Fractional interpolation (fixed-point style).
		 */
		else {

			/*
			 * Start from the bottom of the rectangle instead of the top,
			 * so the gradient runs upward across the region.
			 */
			current_z = (unsigned short)DepthBuffer->Get_Scroll_Delta(dcliprect.Y + drect.Y + drect.Height - 1 - DepthBuffer->Bounds.Y);
			current_z += zdepth;

			/*
			 * Align the starting Z to the current fractional ratio.
			 * Pre-adjust so that the first scanline starts with the correct fraction.
			 * Initialize the fractional counter based on how many lines are in the region.
			 */
			current_z = (current_z / z_step_ratio) * z_step_ratio;
			current_z -= drect.Height / z_step_ratio;
			z_fraction = z_step_numerator - drect.Height % z_step_ratio;

			/*
			 * If the fraction wrapped exactly, reset and carry one full increment.
			 */
			if (z_fraction == z_step_numerator) {
				z_fraction = 0;
				current_z += gradient->WrapIncrement;
			}
		}
	}


	/*
	**	If there is no difference between the width and the stride of the source and
	**	destination surfaces, then the copy can be performed as one huge copy operation.
	**	This is the simplist case and the one that is performed with a full screen
	**	blit.
	*/
	if (drect.Width * dest.Bytes_Per_Pixel() == dest.Stride() && dest.Stride() == source.Stride()) {

		int length = std::min(srect.Height*srect.Width, drect.Height*drect.Width);
		if (overlapped) {
			blitter.BlitBackward(dbuffer, sbuffer, length);
		} else {
			blitter.BlitForward(dbuffer, sbuffer, length);
		}

	} else {

		/*
		**	If the rectangles overlap, then the copy must proceed from the
		**	last row to the first rather than the normal direction.
		*/
		int sstride = source.Stride();
		int dstride = dest.Stride();
		if (DepthBuffer != NULL) {
			zbuffer_pitch = DepthBuffer->Get_Buffer_Width();
		}
		if (AlphaBuffer != NULL) {
			abuffer_pitch = AlphaBuffer->Get_Buffer_Width();
		}
		if (overlapped) {
			sstride = -sstride;
			dstride = -dstride;
			zbuffer_pitch = -zbuffer_pitch;
			abuffer_pitch = -abuffer_pitch;
			sbuffer = ((char*)sbuffer) + (srect.Height-1) * source.Stride();
			dbuffer = ((char*)dbuffer) + (drect.Height-1) * dest.Stride();
			if (DepthBuffer != NULL) {
				zbuffer_offset = (zbuffer_offset + 2 * srect.Height) - 2 * DepthBuffer->Get_Buffer_Width();
				zbuffer_offset = DepthBuffer->Wrap_Overflow(zbuffer_offset);
				current_z -= srect.Height - 1;
			}
			if (AlphaBuffer != NULL) {
				abuffer_offset = (abuffer_offset + 2 * srect.Height) - 2 * AlphaBuffer->Get_Buffer_Width();
				abuffer_offset = AlphaBuffer->Wrap_Overflow(abuffer_offset);
			}
		}

		/*
		**	This perform a line-by-line pixel copy.
		*/
		int height = std::min(srect.Height, drect.Height);
		if (overlapped) {
			for (int y = 0; y < height; y++) {
				blitter.BlitBackward(dbuffer, sbuffer, srect.Width, current_z, (void *)zbuffer_offset, (void *)abuffer_offset, alpha);
				dbuffer = (void*)(((char*)dbuffer) + dstride);
				sbuffer = (void*)(((char*)sbuffer) + sstride);
				if (DepthBuffer != NULL) {
					zbuffer_offset += 2 * zbuffer_pitch;
					zbuffer_offset = DepthBuffer->Wrap_Underflow(zbuffer_offset);
					z_fraction += z_step_denominator;
					if (z_fraction >= z_step_numerator) {
						current_z += gradient->WrapIncrement;
						z_fraction -= z_step_numerator;
					}
				}
				if (AlphaBuffer != NULL) {
					abuffer_offset += 2 * abuffer_pitch;
					abuffer_offset = AlphaBuffer->Wrap_Underflow(abuffer_offset);
				}
			}
		} else {
			for (int y = 0; y < height; y++) {
				blitter.BlitForward(dbuffer, sbuffer, srect.Width, current_z, (void *)zbuffer_offset, (void *)abuffer_offset, alpha);
				dbuffer = (void*)(((char*)dbuffer) + dstride);
				sbuffer = (void*)(((char*)sbuffer) + sstride);
				if (DepthBuffer != NULL) {
					zbuffer_offset += 2 * zbuffer_pitch;
					zbuffer_offset = DepthBuffer->Wrap_Overflow(zbuffer_offset);
					z_fraction += z_step_denominator;
					if (z_fraction >= z_step_numerator) {
						current_z += gradient->WrapIncrement;
						z_fraction -= z_step_numerator;
					}
				}
				if (AlphaBuffer != NULL) {
					abuffer_offset += 2 * abuffer_pitch;
					abuffer_offset = AlphaBuffer->Wrap_Overflow(abuffer_offset);
				}
			}
		}
	}

	dest.Unlock();
	source.Unlock();

	return(true);
}


/***********************************************************************************************
 * RLE_Blit -- Blits RLE compressed data without extra clipping.                               *
 *                                                                                             *
 *    This routine will blit a rectangle of RLE compressed data to the specified surface. It   *
 *    is functionally similar to the other RLE blit routine, but does not use any sub          *
 *    clipping rectangles. The blit is naturally clipped to the edge of the destination        *
 *    surface.                                                                                 *
 *                                                                                             *
 * INPUT:   dest     -- Reference to the destination surface.                                  *
 *                                                                                             *
 *          destrect -- The destination rectangle to draw the pixels to.                       *
 *                                                                                             *
 *          source   -- Reference to the source RLE surface data.                              *
 *                                                                                             *
 *          sourcerect  -- The source rectangle of from the RLE surface to blit from.          *
 *                                                                                             *
 *          blitter  -- Reference to the blitter to perform the blit operation with.           *
 *                                                                                             *
 * OUTPUT:  bool; Was a blit performed? A 'false' value would mean that the blit has been      *
 *                clipped into nothing.                                                        *
 *                                                                                             *
 * WARNINGS:   The dimensions of the source and destination rectangles should be the same      *
 *             until such time that the blitter can support scaling (as of this writing, it    *
 *             cannot).                                                                        *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RLE_Blit(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect, RLEBlitter const & blitter, int zdepth, ZGradientType zgrad, int alpha, int unknown)
{
	return(RLE_Blit(dest, dest.Get_Rect(), destrect, source, source.Get_Rect(), sourcerect, blitter, zdepth, zgrad, alpha, unknown, NULL, Point2D(0, 0)));
}


/***********************************************************************************************
 * RLE_Blit -- Blits a rectangle of RLE compressed data to a surface.                          *
 *                                                                                             *
 *    This routine will blit a rectangle of REL compressed pixel data from a sprite to the     *
 *    surface specified. Appropriate clipping and coordinate adjustments will occur as         *
 *    controlled by the parameters. This is the workhorse RLE blit dispatcher routine.         *
 *                                                                                             *
 * INPUT:   dest     -- The destination surface to blit to.                                    *
 *                                                                                             *
 *          dcliprect-- The clipping rectangle to use on the destination. Pixels won't be      *
 *                      drawn outside of this rectangle and the destination rect coordinates   *
 *                      are biased to this clipping rectange.                                  *
 *                                                                                             *
 *          ddrect   -- The destination rectangle of the blit. The upper left coordinates are  *
 *                      biased to the destination clipping rectangle when blitted. Thus,       *
 *                      a dest X,Y position of 0,0 really means the upper left corner of the   *
 *                      destination clipping rectangle.                                        *
 *                                                                                             *
 *          source   -- The source surface of the RLE compressed data.                         *
 *                                                                                             *
 *          scliprect-- It is quite likely that this will be the full size of the source       *
 *                      surface.                                                               *
 *                                                                                             *
 *          ssrect   -- The source rectangle to blit from within the source surface. It        *
 *                      behaves similarly to the ddrect parameter, but for the source. The     *
 *                      width and height of this rectangle should match the width and height   *
 *                      of the ddrect parameter (scaling is not yet supported).                *
 *                                                                                             *
 *          blitter  -- The blitter to use for this pixel copy. It must be an RLE blitter.     *
 *                                                                                             *
 * OUTPUT:  bool; Did the blit draw at least one pixel?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/24/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool RLE_Blit(Surface & dest, Rect const & dcliprect, Rect const & ddrect, Surface const & source, Rect const & scliprect, Rect const & ssrect, RLEBlitter const & blitter, int zdepth, ZGradientType zgrad, int alpha, int, Surface * zshape, Point2D zpoint)
{
	static char _temp_buf[256];

	uintptr_t zbuffer_offset = 0;
	uintptr_t abuffer_offset = 0;
	char * zshapelock = NULL;
	int zbuffer_pitch = 0;
	int abuffer_pitch = 0;
	int current_z = 0;

	Rect srect = ssrect;			// Desired source rect.
	Rect drect = ddrect;			// Desired destination rect.
	Rect zrect;

	int z_fraction = 0;
	int z_step_denominator = 0;
	int z_step_numerator = 0;
	int z_step_ratio = 0;

	int zshapew = 0;
	int zshapelocky = 0;

	ZGradStruct * grad = &ZGradients[zgrad];

	Rect origdrect = ddrect;

	/*
	**	Adjust the desired draw rectangles to account for clipping. This is where the desired rectangles
	**	get clipped to the bounding rectangles of the surfaces.
	*/
	if (!Blit_Clip(drect, dcliprect, srect, scliprect)) {
		return(false);
	}

	/*
	**	Determine the top and left skip margins. These require special handling
	**	since the shape is compressed.
	*/
	int leftmargin = srect.X - scliprect.X;
	int topmargin = srect.Y - scliprect.Y;

	void * dbuffer = dest.Lock(dcliprect.Top_Left() + drect.Top_Left());
	if (dbuffer == NULL) return(false);

	if (DepthBuffer != NULL) {
		Point2D pt = dcliprect.Top_Left() + drect.Top_Left();
		pt.Y -= DepthBuffer->Bounds.Y;
		zbuffer_offset = DepthBuffer->Get_Buffer_Offset(pt);
		zbuffer_pitch = DepthBuffer->Get_Buffer_Width();
		z_fraction = 0;
		z_step_denominator = grad->StepDenominator;
		z_step_numerator = grad->StepNumerator;
		z_step_ratio = z_step_numerator / z_step_denominator;

		if (grad->IsTopDown) {
			current_z = (unsigned short)DepthBuffer->Get_Scroll_Delta(pt.Y);
			current_z += zdepth;
			current_z = (current_z / grad->LineIncrement) * grad->LineIncrement;
		} else {
			current_z = (unsigned short)DepthBuffer->Get_Scroll_Delta(dcliprect.Y + ddrect.Y + ddrect.Height - 1 - DepthBuffer->Bounds.Y);
			current_z += zdepth;
			if (zshape == NULL) {
				current_z = (current_z / z_step_ratio) * z_step_ratio;
				int zy = ddrect.Y + ddrect.Height - drect.Y;
				current_z -= zy / z_step_ratio;
				z_fraction = z_step_numerator - zy % z_step_ratio;
				if (z_fraction == z_step_numerator) {
					z_fraction = 0;
					current_z += grad->WrapIncrement;
				}
			}
		}
		if (zshape != NULL) {
			zrect = zshape->Get_Rect();
			zshapew = zrect.Width;
			zshapelock = (char *)zshape->Lock(Point2D(zrect.X, zrect.Y) + zpoint + Point2D(leftmargin, topmargin));
			zshapelocky = zrect.Y + zpoint.Y + topmargin;
		} else {
			zshapelock = _temp_buf;
		}
	}

	if (AlphaBuffer != NULL) {
		Point2D pt = dcliprect.Top_Left() + drect.Top_Left();
		pt.Y -= AlphaBuffer->Bounds.Y;
		abuffer_offset = AlphaBuffer->Get_Buffer_Offset(pt);
		abuffer_pitch = AlphaBuffer->Get_Buffer_Width();
	}

	/*
	**	Lock the source pointer. This must always lock at location 0,0 since
	**	normal pixel offset logic does not work for RLE compressed buffers. If there
	**	is a pixel offset required, it is handled below.
	*/
	void * sbuffer = source.Lock();
	if (sbuffer == NULL) {
		dest.Unlock();
		return(false);
	}

	/*
	**	Skip any top margin lines. This must be manually performed on a line
	**	by line basis because the length of each line is Variable.
	*/
	while (topmargin > 0) {
		sbuffer = ((unsigned char *)sbuffer) + (*(unsigned short *)sbuffer);
		topmargin--;
	}

	/*
	**	This perform a line-by-line pixel copy.
	*/
	int dstride = dest.Stride();
	int height = std::min(srect.Height, drect.Height);

	/*
	**
	*/
	if (zshape != NULL) {
		assert(zshapelocky + height <= zshape->Get_Height());
	}

	for (int y = 0; y < height; y++) {

		/*
		**	Blit the correct sub-portion to the destination surface.
		*/
		blitter.Blit(dbuffer, ((unsigned short *)sbuffer) + 1, srect.Width, leftmargin, current_z, (void *)zbuffer_offset, (void *)abuffer_offset, alpha, 0, (void *)zshapelock);

		/*
		**	Advance the source and dest pointers for the next line processing.
		*/
		sbuffer = ((unsigned char *)sbuffer) + (*(unsigned short *)sbuffer);
		dbuffer = (void *)(((char *)dbuffer) + dstride);
		if (DepthBuffer != NULL) {
			zbuffer_offset += 2 * zbuffer_pitch;
			zbuffer_offset = DepthBuffer->Wrap_Overflow(zbuffer_offset);
			if (zshape != NULL) {
				zshapelock += zshapew;
			} else {
				z_fraction += z_step_denominator;
				if (z_fraction >= z_step_numerator) {
					current_z += grad->WrapIncrement;
					z_fraction -= z_step_numerator;
				}
			}
			abuffer_offset += 2 * abuffer_pitch;
			abuffer_offset = AlphaBuffer->Wrap_Overflow(abuffer_offset);
		}
	}

	dest.Unlock();
	source.Unlock();

	return(true);
}
