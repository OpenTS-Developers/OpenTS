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
 *                     $Archive:: /G/wwlib/dsurface.cpp                                       $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/23/00 2:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   DSurface::Blit_From -- Blit from one surface to this one.                                 *
 *   DSurface::Blit_From -- Blit graphic memory from one rectangle to another.                 *
 *   DSurface::Build_Hicolor_Pixel -- Construct a hicolor pixel according to the surface pixel *
 *   DSurface::Build_Remap_Table -- Build a highcolor remap table.                             *
 *   DSurface::Bytes_Per_Pixel -- Fetches the bytes per pixel of the surface.                  *
 *   DSurface::Create_Primary -- Creates a primary (visible) surface.                          *
 *   DSurface::DSurface -- Create a surface attached to specified DDraw Surface Object.        *
 *   DSurface::DSurface -- Default constructor for surface object.                             *
 *   DSurface::DSurface -- Off screen direct draw surface constructor.                         *
 *   DSurface::Fill_Rect -- Fills a rectangle with clipping control.                           *
 *   DSurface::Fill_Rect -- This routine will fill the specified rectangle.                    *
 *   DSurface::Lock -- Fetches a working pointer into surface memory.                          *
 *   DSurface::Restore_Check -- Checks for and restores surface memory if necessary.           *
 *   DSurface::Stride -- Fetches the bytes between rows.                                       *
 *   DSurface::Unlock -- Unlock a previously locked surface.                                   *
 *   DSurface::~DSurface -- Destructor for a direct draw surface object.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "dsurface.h"

#include "blit.h"
#include "dbgprint.h"
#include "misc.h"
#include "video.h"

#include <algorithm>
#include <math.h>
#include <utility>

extern bool GameInFocus;

/*
 * The surfaces are 16 bit 565 and nothing else. The engine refuses to start on a display
 * it cannot present that way, so the shifts that pack a color into a pixel are fixed
 * rather than worked out from the display.
 */
int DSurface::RedRight = 11;
int DSurface::RedLeft = 3;
int DSurface::BlueRight = 0;
int DSurface::BlueLeft = 3;
int DSurface::GreenRight = 5;
int DSurface::GreenLeft = 2;

unsigned short DSurface::HalfbrightMask = 0;
unsigned short DSurface::QuarterbrightMask = 0;
unsigned short DSurface::EighthbrightMask = 0;

bool DSurface::AllowStretchBlits = true;
int DSurface::PrimaryColorMode = COLORMODE_565;


/***********************************************************************************************
 * DSurface::DSurface -- Off screen direct draw surface constructor.                           *
 *                                                                                             *
 *    This constructor will create a Direct Draw enabled surface in video memory if possible.  *
 *    Such a surface will be able to use hardware assist if possible. The surface created      *
 *    is NOT visible. It only exists as a work surface and cannot be flipped to the visible    *
 *    surface. It can only be blitted to the visible surface.                                  *
 *                                                                                             *
 * INPUT:   width    -- The width of the surface to create.                                    *
 *                                                                                             *
 *          height   -- The height of the surface to create.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The surface pixel format is the same as that of the visible display mode. It    *
 *             is important to construct surfaces using this routine, only AFTER the display   *
 *             mode has been set.                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface::DSurface(int width, int height) :
	BASECLASS(width, height),
	BytesPerPixel(2),
	IsPrimary(false),
	GDIBitmap(NULL),
	GDIDC(NULL),
	GDIOldBitmap(NULL),
	GDIBuffer(NULL),
	Pitch(0)
{
	/*
	 * BITMAPINFO carries room for a single color entry, but a bitfields bitmap is
	 * described by three masks following the header, so the header is declared with
	 * room for them rather than written past its end.
	 */
	struct {
		BITMAPINFOHEADER Header;
		unsigned long Masks[3];
	} info;

	memset(&info, 0, sizeof(info));

	/*
	 * A negative height asks for the rows in the order the engine expects, with the top
	 * one first. The masks spell out the 565 layout.
	 */
	info.Header.biSize = sizeof(BITMAPINFOHEADER);
	info.Header.biWidth = width;
	info.Header.biHeight = -height;
	info.Header.biPlanes = 1;
	info.Header.biBitCount = 16;
	info.Header.biCompression = BI_BITFIELDS;

	info.Masks[0] = 0xF800;
	info.Masks[1] = 0x07E0;
	info.Masks[2] = 0x001F;

	GDIDC = CreateCompatibleDC(NULL);
	if (GDIDC == NULL) {
		return;
	}

	GDIBitmap = CreateDIBSection(GDIDC, (BITMAPINFO *)&info, DIB_RGB_COLORS, &GDIBuffer, NULL, 0);
	if (GDIBitmap == NULL) {
		DeleteDC(GDIDC);
		GDIDC = NULL;
		GDIBuffer = NULL;
		return;
	}

	GDIOldBitmap = SelectObject(GDIDC, GDIBitmap);

	DIBSECTION section;
	if (GetObject(GDIBitmap, sizeof(section), &section) == sizeof(section)) {
		Pitch = section.dsBm.bmWidthBytes;
	} else {
		Pitch = width * 2;
	}
}


/***********************************************************************************************
 * DSurface::~DSurface -- Destructor for a direct draw surface object.                         *
 *                                                                                             *
 *    This will destruct (make invalid) the direct draw surface.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface::~DSurface(void)
{
	/*
	 * GDI will not free a bitmap that is still selected into a context, so the one the
	 * context started with has to go back first.
	 */
	if (GDIDC != NULL) {
		if (GDIOldBitmap != NULL) {
			SelectObject(GDIDC, GDIOldBitmap);
			GDIOldBitmap = NULL;
		}
		DeleteDC(GDIDC);
		GDIDC = NULL;
	}

	if (GDIBitmap != NULL) {
		DeleteObject(GDIBitmap);
		GDIBitmap = NULL;
	}

	GDIBuffer = NULL;
}


/***********************************************************************************************
 * DSurface::Create_Primary -- Creates a primary (visible) surface.                            *
 *                                                                                             *
 *    This routine is used to create the surface object that represents the currently          *
 *    visible display. The surface is not allocated, it is merely linked to the preexisting    *
 *    surface that the Windows GDI is also currently using.                                    *
 *                                                                                             *
 * INPUT:   backsurface -- Optional pointer to specify where the backpage (flip enabled)       *
 *                         pointer will be placed. If this parameter is NULL, then no          *
 *                         back surface will be created.                                       *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the primary surface.                                     *
 *                                                                                             *
 * WARNINGS:   There can be only one primary surface. If an additional call to this routine    *
 *             is made, another surface pointer will be returned, but it will point to the     *
 *             same surface as before.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
DSurface * DSurface::Create_Primary(void)
{
	DSurface * surface = new DSurface(VideoModeWidth, VideoModeHeight);

	if (surface == NULL || surface->Get_Buffer() == NULL) {
		delete surface;
		return(NULL);
	}

	surface->IsPrimary = true;

	HalfbrightMask = (unsigned short)Build_Hicolor_Pixel(127, 127, 127);
	QuarterbrightMask = (unsigned short)Build_Hicolor_Pixel(63, 63, 63);
	EighthbrightMask = (unsigned short)Build_Hicolor_Pixel(31, 31, 31);

	return(surface);
}


/***********************************************************************************************
 * DSurface::GetDC -- Get the windows device context from our surface                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS: Any current locks will get unlocked while the DC is held                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/21/2000 NAK : Created.                                                                 *
 *=============================================================================================*/
HDC DSurface::GetDC(void)
{
	if (GDIDC == NULL) {
		return(NULL);
	}

	/*
	 * The count is raised so the software blitter keeps off the pixels while GDI is
	 * drawing on them, which is what it did when this context came from DirectDraw.
	 */
	LockCount++;
	return(GDIDC);
}


/// <summary>
/// Releases a device context obtained from GetDC.
/// </summary>
/// <param name="hdc">The context to release.</param>
/// <returns>int; Always one. The context outlives the call and is reused.</returns>
int DSurface::ReleaseDC(HDC hdc)
{
	/*
	 * GDI batches its drawing, so the pixels are not all there until it is flushed.
	 * Everything else reads them directly.
	 */
	GdiFlush();

	if (LockCount > 0) {
		LockCount--;
	}

	if (IsPrimary && LockCount == 0) {
		Video_Mark_Dirty();
	}

	return(1);
}


/***********************************************************************************************
 * DSurface::Bytes_Per_Pixel -- Fetches the bytes per pixel of the surface.                    *
 *                                                                                             *
 *    This routine will return with the number of bytes that each pixel consumes. The value    *
 *    is dependant upon the graphic mode of the display.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the bytes per pixel of the surface object.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int DSurface::Bytes_Per_Pixel(void) const
{
	return(BytesPerPixel);
}


/***********************************************************************************************
 * DSurface::Stride -- Fetches the bytes between rows.                                         *
 *                                                                                             *
 *    This routine will return the number of bytes to add so that the pointer will be          *
 *    positioned at the same column, but one row down the screen. This value may very well     *
 *    NOT be equal to the width multiplied by the bytes per pixel.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the byte difference between subsequent pixel rows.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int DSurface::Stride(void) const
{
	return(Pitch);
}


/***********************************************************************************************
 * DSurface::Lock -- Fetches a working pointer into surface memory.                            *
 *                                                                                             *
 *    This routine will return with a pointer to the pixel at the location specified. In order *
 *    to directly manipulate surface memory, the surface memory must be mapped into the        *
 *    program's logical address space. In addition, all blitter activity on the surface will   *
 *    be suspended. Every call to Lock must be have a corresponding call to Unlock if the      *
 *    pointer returned is not equal to NULL.                                                   *
 *                                                                                             *
 * INPUT:   point -- Pixel coordinate to return a pointer to.                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the pixel specified. If the return value is NULL, then   *
 *          the surface could not be locked and no call to Unlock should be performed.         *
 *                                                                                             *
 * WARNINGS:   It is important not to keep a surface locked indefinately since the blitter     *
 *             will not be able to function. Due to the time that locking consumes, it is      *
 *             also important to not perform unnecessarily frequent Lock calls.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void * DSurface::Lock(Point2D point) const
{
	if (GDIBuffer == NULL) return(NULL);
	if (point.X < 0 || point.Y < 0) return(NULL);

	BASECLASS::Lock();
	return(((char *)GDIBuffer) + point.Y * Stride() + point.X * Bytes_Per_Pixel());
}


/// <summary>
/// Determines if the surface can be locked.
/// </summary>
/// <returns>bool; Can the surface be locked?</returns>
bool DSurface::Can_Lock(int x, int y) const
{
	return(GDIBuffer != NULL);
}


/// <summary>
/// Determines if the blitter is free to take more work.
/// Blits are done on this thread, so it always is.
/// </summary>
/// <returns>bool; Is the blitter ready for another operation?</returns>
bool DSurface::Can_Blit(void) const
{
	return(true);
}


/***********************************************************************************************
 * DSurface::Unlock -- Unlock a previously locked surface.                                     *
 *                                                                                             *
 *    After a surface has been successfully locked, a call to the Unlock() function is         *
 *    required.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the unlock successful?                                                   *
 *                                                                                             *
 * WARNINGS:   Only pair a call to Unlock if the prior Lock actually returned a non-NULL       *
 *             value.                                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Unlock(void) const
{
	if (LockCount > 0) {
		BASECLASS::Unlock();
		if (IsPrimary && LockCount == 0) {
			Video_Mark_Dirty();
		}
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * DSurface::Blit_From -- Blit graphic memory from one rectangle to another.                   *
 *                                                                                             *
 *    This routine will blit from the source surface to this surface. The entire surfaces      *
 *    serve as the clipping rectangles.                                                        *
 *                                                                                             *
 * INPUT:   destrect -- The destination rectangle.                                             *
 *                                                                                             *
 *          ssource  -- The source surface to blit from.                                       *
 *                                                                                             *
 *          sourecrect  -- The source rectangle.                                               *
 *                                                                                             *
 *          trans    -- Should transparency checking be performed?                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed without error?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Blit_From(Rect const & destrect, Surface const & ssource, Rect const & sourcerect, bool trans, bool unknown)
{
	return(Blit_From(Get_Rect(), destrect, ssource, ssource.Get_Rect(), sourcerect, trans, unknown));
}


/***********************************************************************************************
 * DSurface::Blit_From -- Blit from one surface to this one.                                   *
 *                                                                                             *
 *    Use this routine to blit a rectangle from the specified surface to this surface while    *
 *    performing clipping upon the blit rectangles specified.                                  *
 *                                                                                             *
 * INPUT:   dcliprect   -- The clipping rectangle to use for this surface.                     *
 *                                                                                             *
 *          destrect    -- The destination rectangle of the blit. The is relative to the       *
 *                         dcliprect parameter.                                                *
 *                                                                                             *
 *          ssource     -- The source surface of the blit.                                     *
 *                                                                                             *
 *          scliprect   -- The source clipping rectangle.                                      *
 *                                                                                             *
 *          sourcrect   -- The source rectangle of the blit. This rectangle is relative to     *
 *                         the source clipping rectangle.                                      *
 *                                                                                             *
 *          trans       -- Is this a transparent blit request?                                 *
 *                                                                                             *
 * OUTPUT:  bool; Was there a blit performed? A 'false' return value would indicate that the   *
 *                blit was clipped into nothing.                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Blit_From(Rect const & dcliprect, Rect const & destrect, Surface const & ssource, Rect const & scliprect, Rect const & sourcerect, bool trans, bool unknown)
{
	if (!dcliprect.Is_Valid() || !scliprect.Is_Valid() || !destrect.Is_Valid() || !sourcerect.Is_Valid()) return(false);

	bool samesize = (sourcerect.Width == destrect.Width && sourcerect.Height == destrect.Height);

	/*
	 * The software blitter handles everything except a size change between two of these
	 * surfaces, which GDI stretches instead.
	 */
	if (trans || !ssource.Is_GDI_Backed() || samesize) {
		bool result = BASECLASS::Blit_From(dcliprect, destrect, ssource, scliprect, sourcerect, trans, unknown);
		if (result && IsPrimary) {
			Video_Mark_Dirty();
		}
		return(result);
	}

	DSurface const & source = (DSurface const &)ssource;

	if (GDIDC == NULL || source.GDIDC == NULL) {
		return(false);
	}

	Rect drect = destrect.Bias_To(dcliprect);
	Rect srect = sourcerect.Bias_To(scliprect);

	drect = Intersect(drect, Intersect(dcliprect, Get_Rect()));
	if (!drect.Is_Valid()) return(false);

	/*
	 * Both sets of pixels are read and written directly elsewhere, so any drawing GDI
	 * still holds has to land first.
	 */
	GdiFlush();

	SetStretchBltMode(GDIDC, COLORONCOLOR);
	bool result = StretchBlt(GDIDC, drect.X, drect.Y, drect.Width, drect.Height,
		source.GDIDC, srect.X, srect.Y, srect.Width, srect.Height, SRCCOPY) != 0;

	GdiFlush();

	if (result && IsPrimary) {
		Video_Mark_Dirty();
	}

	return(result);
}


/***********************************************************************************************
 * DSurface::Fill_Rect -- This routine will fill the specified rectangle.                      *
 *                                                                                             *
 *    This routine will fill the specified rectangle with a color.                             *
 *                                                                                             *
 * INPUT:   fillrect -- The rectangle to fill.                                                 *
 *                                                                                             *
 *          color    -- The color to fill with.                                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the fill performed without error?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Fill_Rect(Rect const & fillrect, int color)
{
	return(DSurface::Fill_Rect(Get_Rect(), fillrect, color));
}


/***********************************************************************************************
 * DSurface::Fill_Rect -- Fills a rectangle with clipping control.                             *
 *                                                                                             *
 *    This routine will fill a rectangle on this surface, but will clip the request against    *
 *    a clipping rectangle first.                                                              *
 *                                                                                             *
 * INPUT:   cliprect -- The clipping rectangle to use for this surface.                        *
 *                                                                                             *
 *          fillrect -- The rectangle to fill with the specified color. The rectangle is       *
 *                      relative to the clipping rectangle.                                    *
 *                                                                                             *
 *          color    -- The color (surface dependant format) to use when filling the rectangle *
 *                      pixels.                                                                *
 *                                                                                             *
 * OUTPUT:  bool; Was a fill operation performed? A 'false' return value would mean that the   *
 *                fill request was clipped into nothing.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool DSurface::Fill_Rect(Rect const & cliprect, Rect const & fillrect, int color)
{
	if (GDIBuffer == NULL || !fillrect.Is_Valid()) return(false);

	bool result = BASECLASS::Fill_Rect(cliprect, fillrect, color);

	if (result && IsPrimary) {
		Video_Mark_Dirty();
	}

	return(result);
}


/// <summary>
/// Fills a rectangle with a translucent color.
/// This routine blends the color into the pixels already on the surface rather than replacing
/// them. It is used for the dimmed panels the score and multiplayer score screens lay their
/// text over. Only a hicolor surface can be filled this way.
/// </summary>
/// <param name="xcliprect">The area to fill. It is clipped against the surface.</param>
/// <param name="color">The color to blend into the surface.</param>
/// <param name="opacity">How solid the fill should be, from 0 for invisible to 100.</param>
/// <returns>bool; Was the rectangle filled?</returns>
bool DSurface::Fill_Rect_Trans(Rect const & xcliprect, const RGBClass & color, unsigned int opacity)
{
	//assert(xcliprect.Is_Valid());

	if (Bytes_Per_Pixel() < 2) {
		return(false);
	}

	if (!xcliprect.Is_Valid()) {
		return(false);
	}

	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect cliprect = Intersect(Get_Rect(), xcliprect);

	if (!cliprect.Is_Valid()) {
		return(false);
	}

	unsigned short r_mask;
	unsigned short g_mask;
	unsigned short b_mask;

	r_mask = 255;
	g_mask = 255;
	b_mask = 255;

	r_mask >>= (unsigned short)RedLeft;
	g_mask >>= (unsigned short)GreenLeft;
	b_mask >>= (unsigned short)BlueLeft;

	r_mask <<= (unsigned short)RedRight;
	g_mask <<= (unsigned short)GreenRight;
	b_mask <<= (unsigned short)BlueRight;

	unsigned short *ptr = (unsigned short *)Lock(cliprect.Top_Left());
	if (ptr == NULL) {
		return(false);
	}

	if (opacity > 100) {
		opacity = 100;
	}

	opacity = ((opacity * 255) / 100);
	unsigned short s2 = (255 - opacity);

	unsigned short c1 = Build_Hicolor_Pixel(color.Get_Red(), color.Get_Green(), color.Get_Blue());

	for (int y = 0; y < cliprect.Height; y++) {
		int pos = y * (Stride() / 2);

		for (int x = 0; x < cliprect.Width; x++) {
			unsigned short *p = &ptr[pos];
			unsigned short c2 = *p;

			unsigned int r1 = (c1 & r_mask) * opacity;
			unsigned int g1 = (c1 & g_mask) * opacity;
			unsigned int b1 = (c1 & b_mask) * opacity;

			unsigned int r2 = (c2 & r_mask) * s2;
			unsigned int g2 = (c2 & g_mask) * s2;
			unsigned int b2 = (c2 & b_mask) * s2;

			unsigned int r = ((r1 + r2) / 256);
			unsigned int g = ((g1 + g2) / 256);
			unsigned int b = ((b1 + b2) / 256);

			*p = ((r & r_mask) | (g & g_mask) | b);
			pos++;
		}
	}

	Unlock();
	return(true);
}


/***********************************************************************************************
 * DSurface::Build_Remap_Table -- Build a highcolor remap table.                               *
 *                                                                                             *
 *    This will build a complete hicolor remap table for the palette specified. This table     *
 *    can then be used to quickly fetch a pixel that matches the color index of the palette.   *
 *                                                                                             *
 * INPUT:   table -- The location to store the hicolor table. The buffer must be 256*2 bytes   *
 *                   long.                                                                     *
 *                                                                                             *
 *          palette  -- The palette to use to create the remap table.                          *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void DSurface::Build_Remap_Table(unsigned short * table, int count, PaletteClass const & palette)
{
	//assert(table != NULL);

	/*
	**	Build the hicolor index table according to the palette.
	*/
	if (count >= 1) {
		int i = count - 1;

		if (i >= 0) {
			int dividend = 0;

			i++;
			do {
				int intensity = count > 1 ? dividend / (count - 1) : 65536;

				for (int index = 0; index < 256; index++) {
					const RGBClass *rgb = &palette[index];
					int r = (rgb->Get_Red() * intensity) >> 16;
					int g = (rgb->Get_Green() * intensity) >> 16;
					int b = (rgb->Get_Blue() * intensity) >> 16;

					r = std::min(r, 255);
					g = std::min(g, 255);
					b = std::min(b, 255);

					*table = Build_Hicolor_Pixel(r, g, b);
					table++;
				}

				dividend += 131072;
				i--;
			} while (i);
		}
	}
}


/// <summary>
/// Fetches the position of red within a pixel.
/// This routine and its companions let code outside the surface build and take apart hicolor
/// pixels for itself.
/// </summary>
/// <returns>Returns with the shift that moves a reduced red value up into its place in a
/// hicolor pixel.</returns>
int DSurface::Get_Red_Right(void)
{
	return(RedRight);
}


/// <summary>
/// Fetches the red precision loss of the display.
/// </summary>
/// <returns>Returns with the shift that reduces an eight bit red value to the precision the
/// display can show.</returns>
int DSurface::Get_Red_Left(void)
{
	return(RedLeft);
}


/// <summary>
/// Fetches the position of green within a pixel.
/// </summary>
/// <returns>Returns with the shift that moves a reduced green value up into its place in a
/// hicolor pixel.</returns>
int DSurface::Get_Green_Right(void)
{
	return(GreenRight);
}


/// <summary>
/// Fetches the green precision loss of the display.
/// </summary>
/// <returns>Returns with the shift that reduces an eight bit green value to the precision the
/// display can show.</returns>
int DSurface::Get_Green_Left(void)
{
	return(GreenLeft);
}


/// <summary>
/// Fetches the position of blue within a pixel.
/// </summary>
/// <returns>Returns with the shift that moves a reduced blue value up into its place in a
/// hicolor pixel.</returns>
int DSurface::Get_Blue_Right(void)
{
	return(BlueRight);
}


/// <summary>
/// Fetches the blue precision loss of the display.
/// </summary>
/// <returns>Returns with the shift that reduces an eight bit blue value to the precision the
/// display can show.</returns>
int DSurface::Get_Blue_Left(void)
{
	return(BlueLeft);
}


/// <summary>
/// Fetches the pixel format of the primary surface.
/// The lighting tables and the movie player ask for this so they can pack their own pixels
/// the way the display expects them.
/// </summary>
/// <returns>Returns with the color mode the display was created in.</returns>
int DSurface::Get_Primary_Color_Mode(void)
{
	return(PrimaryColorMode);
}


/// <summary>
/// Draws a clipped, depth-tested "glow" line into the locked DSurface. Each pixel that passes
/// the depth test is read from the surface, has each RGB channel brightened by
/// ((glow_strength*c)>>8)+c
/// (clamped to 255), and is written back. The depth endpoints are re-interpolated across the
/// portion of the line that survives clipping, then the line is rasterized with a 3-axis
/// Bresenham whose major axis is max(dx, |dy|, |dz|).
/// </summary>
/// <param name="cliprect">Clipping rectangle (intersected with the surface rect).</param>
/// <param name="startpoint">Line start point.</param>
/// <param name="endpoint">Line end point.</param>
/// <param name="glow_strength">Brightness multiplier applied to each channel.</param>
/// <param name="start_depth">Depth value at the start point.</param>
/// <param name="end_depth">Depth value at the end point.</param>
/// <param name="write_depth">If true, the depth buffer is updated at each plotted pixel.</param>
/// <returns>True if any portion of the line was drawn, false otherwise.</returns>
bool DSurface::Draw_Depth_Glow_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, int glow_strength, int start_depth, int end_depth, bool write_depth)
{
	Point2D start;
	Point2D end;

	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect rect = Intersect(cliprect, Get_Rect());
	if (rect.Width != 0 || rect.Height != 0) {

	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	start = Bias_To(startpoint, rect);
	end = Bias_To(endpoint, rect);
	int z1 = start_depth;
	int z2 = end_depth;

	/*
	 * Sort the endpoints left-to-right, carrying the depth values with them.
	 */
	if (start.X > end.X) {
		std::swap(start, end);
		z1 = end_depth;
		z2 = start_depth;
	}

	/*
	 * Remember the pre-clip endpoints so the depth values can be re-interpolated
	 * across the portion of the line that survives clipping.
	 */
	int px = start.X;
	int py = start.Y;
	Point2D origend = end;

	if (Clip_Line_To_Rect(start, end, rect)) {

	int zend = z2;
	int zstart = z1;

	/*
	 * Re-interpolate the far depth value if the far endpoint was clipped.
	 */
	if (end.X != origend.X || end.Y != origend.Y) {
		Point2D clipped = Point2D(px, py) - end;
		Point2D full = Point2D(px, py) - origend;
		int fulllen = full.Length();
		unsigned int amount = abs((int)((double)clipped.Length() / (double)fulllen * (double)(z1 - z2)));
		if (z1 < z2) {
			zend = z1 + amount;
		} else {
			zend = z1 - amount;
		}
	}

	/*
	 * Re-interpolate the near depth value if the near endpoint was clipped.
	 */
	if (start.X != px || start.Y != py) {
		Point2D clipped_point(start.X - origend.X, start.Y - origend.Y);
		int clipped = clipped_point.Length();
		Point2D full_point(px - origend.X, py - origend.Y);
		int full = full_point.Length();
		unsigned int amount = abs((int)((double)clipped / (double)full * (double)(z1 - z2)));
		if (z2 > z1) {
			zstart = z2 - amount;
		} else {
			zstart = z2 + amount;
		}
	}

	Bytes_Per_Pixel();
	void * buffer = Lock(start);

	if (DepthBuffer != NULL) {

	Point2D point = start;
	point.Y -= DepthBuffer->Get_Bounds().Y;
	unsigned short * zbuffer = (unsigned short *)DepthBuffer->Get_Buffer_Offset(point);

	int start_y = start.Y;
	int zwrap = -1;
	ZBuffer * depth_buffer = DepthBuffer;
	unsigned short z = (unsigned short)(zstart + (short)(depth_buffer->Get_Bounds().Y + depth_buffer->Get_Scroll()) - start_y - rect.Y);
	int zwidth = depth_buffer->Get_Buffer_Width();

	if (buffer != NULL) {

	/*
	 * Distances to x, y and depth.
	 */
	int dx = end.X - start.X;
	int xcount = dx;
	int dy = end.Y - start.Y;
	int dz = zend - zstart;

	int pitch = Stride();
	if (dy < 0) {
		pitch = -pitch;
		zwidth = -zwidth;
		zwrap = 1;
	}

	int zstep = 1;
	int abs_dy = abs(dy);
	if (dz < 0) {
		zstep = -1;
	}

	int dy2 = 2 * abs_dy;
	int abs_dz = abs(dz);
	int dx2 = 2 * dx;
	int dz2 = 2 * abs_dz;

	if (abs_dz > dx && abs_dz > abs_dy) {

		/*
		 * The depth axis changes fastest.
		 */
		int xdelta = dy2 - abs_dz;
		int ydelta = dx2 - abs_dz;
		if (abs_dz > 0) {
			int offset = 0;
			for (int i = abs_dz; i != 0; i--) {

				if ((unsigned short)z < *zbuffer) {
					RGBClass rgb = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + offset));
					int blue = ((glow_strength * rgb.Get_Blue()) >> 8) + rgb.Get_Blue();
					int red = ((glow_strength * rgb.Get_Red()) >> 8) + rgb.Get_Red();
					int green = ((glow_strength * rgb.Get_Green()) >> 8) + rgb.Get_Green();
					if (blue > 255) {
						blue = 255;
					}
					if (green > 255) {
						green = 255;
					}
					if (red > 255) {
						red = 255;
					}
					*(unsigned short *)((char *)buffer + offset) = (unsigned short)Build_Hicolor_Pixel(red, green, blue);
					if (write_depth) {
						*zbuffer = (unsigned short)z;
					}
				}

				if (xdelta > 0) {
					buffer = (unsigned char *)buffer + pitch;
					xdelta -= dz2;
					zbuffer += zwidth;
					if (zwidth > 0) {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					} else {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
					}
					z += zwrap;
				}

				if (ydelta > 0) {
					zbuffer++;
					offset += 2;
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					ydelta -= dz2;
				}

				z += zstep;
				ydelta += dx2;
				xdelta += dy2;
			}
		}

	} else if (dx > abs_dy) {

		/*
		 * The slope is not steep -- plot a low line.
		 */
		int offset = 0;
		int ydelta = dy2 - dx;
		int zdelta = dz2 - dx;
		if (dx > 0) {
			do {

				if ((unsigned short)z < *zbuffer) {
					RGBClass rgb = Deconstruct_Hicolor_Pixel(*((unsigned short *)buffer + offset));
					int blue = ((glow_strength * rgb.Get_Blue()) >> 8) + rgb.Get_Blue();
					int red = ((glow_strength * rgb.Get_Red()) >> 8) + rgb.Get_Red();
					int green = ((glow_strength * rgb.Get_Green()) >> 8) + rgb.Get_Green();
					if (blue > 255) {
						blue = 255;
					}
					if (green > 255) {
						green = 255;
					}
					if (red > 255) {
						red = 255;
					}
					*((unsigned short *)buffer + offset) = (unsigned short)Build_Hicolor_Pixel(red, green, blue);
					if (write_depth) {
						*zbuffer = (unsigned short)z;
					}
				}

				if (ydelta > 0) {
					buffer = (unsigned char *)buffer + pitch;
					ydelta -= dx2;
					zbuffer += zwidth;
					if (zwidth > 0) {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					} else {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
					}
					z += zwrap;
				}

				if (zdelta > 0) {
					z += zstep;
					zdelta -= dx2;
				}

				zbuffer++;
				ydelta += dy2;
				zdelta += dz2;
				zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);

			} while (++offset < xcount);
		}
	} else {

		/*
		 * The slope is steep -- plot a high line.
		 */
		int xdelta = dx2 - abs_dy;
		int zdelta = dz2 - abs_dy;
		if (abs_dy > 0) {
			int offset = 0;
			for (int i = abs_dy; i != 0; i--) {

				if ((unsigned short)z < *zbuffer) {
					RGBClass rgb = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + offset));
					int blue = ((glow_strength * rgb.Get_Blue()) >> 8) + rgb.Get_Blue();
					int red = ((glow_strength * rgb.Get_Red()) >> 8) + rgb.Get_Red();
					int green = ((glow_strength * rgb.Get_Green()) >> 8) + rgb.Get_Green();
					if (blue > 255) {
						blue = 255;
					}
					if (green > 255) {
						green = 255;
					}
					if (red > 255) {
						red = 255;
					}
					*(unsigned short *)((char *)buffer + offset) = (unsigned short)Build_Hicolor_Pixel(red, green, blue);
					if (write_depth) {
						*zbuffer = (unsigned short)z;
					}
				}

				if (xdelta > 0) {
					zbuffer++;
					offset += 2;
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					xdelta -= dy2;
				}

				if (zdelta > 0) {
					z += zstep;
					zdelta -= dy2;
				}

				xdelta += dx2;
				zdelta += dz2;
				buffer = (unsigned char *)buffer + pitch;
				zbuffer += zwidth;
				if (zwidth > 0) {
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
				} else {
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
				}
				z += zwrap;
			}
		}

	}

	Unlock();
	return(true);

	}
	}
	}
	}

	return(false);
}


/// <summary>
/// Draws a clipped, antialiased, depth shaded line.
/// This routine draws the laser and beam weapon effects, and the radial indicator lines
/// of a building. The line is depth tested against the depth buffer so that it passes
/// behind whatever is in front of it, and the alpha buffer supplies the coverage of every
/// pixel painted. Only the color channels asked for are blended toward the color given,
/// so a beam can be tinted to a single gun.
/// </summary>
/// <param name="color">The color the requested channels are blended toward.</param>
/// <param name="start_depth">The depth value at the start of the line.</param>
/// <param name="end_depth">The depth value at the end of the line.</param>
/// <param name="write_depth">Should the depth buffer be updated as the line is drawn?</param>
/// <param name="blend_red">Should the red channel be blended?</param>
/// <param name="blend_green">Should the green channel be blended?</param>
/// <param name="blend_blue">Should the blue channel be blended?</param>
/// <param name="intensity">The weight used when sampling the falloff gradient.</param>
/// <returns>bool; Was any part of the line drawn?</returns>
bool DSurface::Draw_Depth_Antialiased_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, RGBClass & color, int start_depth, int end_depth, bool write_depth, bool blend_red, bool blend_green, bool blend_blue, float intensity)
{
	static int GradientTable[257];
	static bool onetime = false;

	/*
	 * One-time build of the gradient/falloff lookup table.
	 */
	if (!onetime) {
		onetime = true;
		for (int i = 0; i <= 256; i++) {
			GradientTable[i] = (int)((pow((double)i / 128.0 - 1.0, 3.0) + 1.0) * 128.0);
		}
	}

	/*
	 * Shrink the surface rectangle by one pixel on every edge and clip the supplied
	 * clipping rectangle against it.
	 */
	Rect surfrect = Get_Rect();
	surfrect.X += 1;
	surfrect.Y += 1;
	surfrect.Width -= 2;
	surfrect.Height -= 2;

	Rect clip = Intersect(cliprect, surfrect);
	if (clip.Width == 0 && clip.Height == 0) {
		return(false);
	}

	/*
	 * Cache the requested gun values.
	 */
	int red = color.Get_Red();
	int green = color.Get_Green();
	int blue = color.Get_Blue();

	/*
	 * Bias both endpoints into the clipping rectangle, then order them left to right,
	 * carrying the alpha values with them.
	 */
	Point2D start = Bias_To(startpoint, clip);
	Point2D end = Bias_To(endpoint, clip);
	int astart = start_depth;
	int aend = end_depth;
	if (start.X > end.X) {
		std::swap(start, end);
		astart = end_depth;
		aend = start_depth;
	}

	Point2D origend = end;
	int px = start.X;
	int py = start.Y;

	if (Clip_Line_To_Rect(start, end, clip)) {

	int alpha_end = aend;
	int alpha_start = astart;

	/*
	 * Re-interpolate the alpha values across the portion of the line that survived clipping.
	 */
	if (end.X != origend.X || end.Y != origend.Y) {
		Point2D clipped(px - end.X, py - end.Y);
		Point2D full(px - origend.X, py - origend.Y);
		int fulllen = full.Length();
		unsigned int amount = abs((int)((double)clipped.Length() / (double)fulllen * (double)(astart - aend)));
		if (astart < aend) {
			alpha_end = astart + amount;
		} else {
			alpha_end = astart - amount;
		}
	}

	if (start.X != px || start.Y != py) {
		Point2D clipped(start.X - origend.X, start.Y - origend.Y);
		Point2D full(px - origend.X, py - origend.Y);
		int fulllen = full.Length();
		unsigned int amount = abs((int)((double)clipped.Length() / (double)fulllen * (double)(astart - aend)));
		if (aend > astart) {
			alpha_start = aend - amount;
		} else {
			alpha_start = aend + amount;
		}
	}

	void * buffer = Lock(start);

	if (DepthBuffer == NULL) {
		Unlock();
		return(false);
	}

	unsigned short * zbuffer = (unsigned short *)DepthBuffer->Get_Buffer_Offset(Point2D(start.X, start.Y - DepthBuffer->Get_Bounds().Y));
	unsigned short * abuffer = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(Point2D(start.X - AlphaBuffer->Get_Bounds().X, start.Y - AlphaBuffer->Get_Bounds().Y));

	int zwidth = DepthBuffer->Get_Buffer_Width();
	int zwrap = -1;
	unsigned short z = (unsigned short)(alpha_start + (short)(DepthBuffer->Get_Bounds().Y + DepthBuffer->Get_Scroll()) - start.Y - clip.Y);

	if (buffer == NULL) {
		Unlock();
		return(false);
	}

	/*
	 * Distances to x, y and alpha.
	 */
	int dy = end.Y - start.Y;
	int dx = end.X - start.X;
	int xcount = end.X - start.X;
	int adelta = alpha_end - alpha_start;

	int pitch = Stride();
	if (dy < 0) {
		pitch = -pitch;
		zwidth = -zwidth;
		zwrap = 1;
	}

	int astep = 1;
	int abs_dy = abs(dy);
	int half_pitch = pitch / 2;
	if (adelta < 0) {
		astep = -1;
	}

	int dy2 = 2 * abs_dy;
	int abs_adelta = abs(adelta);
	int adelta2 = 2 * abs_adelta;

	if (dx != 0 || abs_dy != 0) {

		if (abs_adelta <= dx || abs_adelta <= abs_dy) {

			/*
			 * Geometric stepping -- one of the two axes drives the loop while the alpha
			 * gradient is advanced through a secondary DDA accumulator.
			 */
			int weight = 256;
			if (dx > abs_dy) {

				/*
				 * The slope is not steep -- step the X axis.
				 */
				int adda = adelta2 - dx;
				int grad = (int)((double)abs_dy / (double)xcount * 256.0);
				int xoff = 0;
				if (dx > 0) {
					int neg_grad = -grad;
					int neighoff = 2 * half_pitch;
					int otherweight = 0;
					do {
						int cov = *abuffer;
						if (z < *zbuffer) {
							if (cov) {
								int lidx = (int)((double)weight * intensity);
								int oidx = (int)((double)otherweight * intensity);
								int lbk = GradientTable[256 - lidx];
								int lf = GradientTable[lidx];
								int nbk = GradientTable[256 - oidx];
								int nf = GradientTable[oidx];

								RGBClass line = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + 2 * xoff));
								RGBClass neighbor = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + neighoff));

								int outnr = neighbor.Get_Red();
								int outlr = line.Get_Red();
								int outng = neighbor.Get_Green();
								int outlg = line.Get_Green();
								int outnb = neighbor.Get_Blue();
								int outlb = line.Get_Blue();
								if (blend_red) {
									outlr = (cov * (red * nf + line.Get_Red() * nbk)) >> 15;
									outnr = (cov * (red * lf + neighbor.Get_Red() * lbk)) >> 15;
								}
								if (blend_green) {
									outlg = (cov * (green * nf + line.Get_Green() * nbk)) >> 15;
									outng = (cov * (green * lf + neighbor.Get_Green() * lbk)) >> 15;
								}
								if (blend_blue) {
									outlb = (cov * (blue * nf + line.Get_Blue() * nbk)) >> 15;
									outnb = (cov * (blue * lf + neighbor.Get_Blue() * lbk)) >> 15;
								}

								int wb = 255;
								if (outnb <= 255) {
									wb = outnb;
								}
								if (outng > 255) {
									outng = 255;
								}
								if (outnr > 255) {
									outnr = 255;
								}
								*(unsigned short *)((char *)buffer + 2 * xoff) = (unsigned short)Build_Hicolor_Pixel(outnr, outng, wb);
								if (outlb > 255) {
									outlb = 255;
								}
								if (outlg > 255) {
									outlg = 255;
								}
								if (outlr > 255) {
									outlr = 255;
								}
								*(unsigned short *)((char *)buffer + neighoff) = (unsigned short)Build_Hicolor_Pixel(outlr, outlg, outlb);
								if (write_depth) {
									*zbuffer = z;
								}
							}
						}

						weight -= grad;
						otherweight -= neg_grad;
						if (otherweight > 256) {
							otherweight -= 256;
							weight += 256;
							buffer = (unsigned char *)buffer + pitch;
							zbuffer += zwidth;
							if (zwidth > 0) {
								zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
							} else {
								zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
							}
							abuffer += zwidth;
							if (zwidth > 0) {
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							} else {
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
							}
							z += zwrap;
						}

						if (adda > 0) {
							z += astep;
							adda -= 2 * xcount;
						}

						adda += adelta2;
						zbuffer++;
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
						abuffer++;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						xoff++;
						neighoff += 2;
					} while (xoff < xcount);
				}
			} else {

				/*
				 * The slope is steep -- step the Y axis.
				 */
				int adda = adelta2 - abs_dy;
				int grad = (int)((double)xcount / (double)abs_dy * 256.0);
				if (abs_dy > 0) {
					int byteoff = 0;
					int neg_grad = -grad;
					int otherweight = 0;
					int i = abs_dy;
					do {
						int cov = *abuffer;
						if (z < *zbuffer) {
							if (cov) {
								int lidx = (int)((double)weight * intensity);
								int oidx = (int)((double)otherweight * intensity);
								int nbk = GradientTable[256 - oidx];
								int lbk = GradientTable[256 - lidx];
								int nf = GradientTable[oidx];
								int lf = GradientTable[lidx];

								RGBClass line = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + byteoff));
								RGBClass neighbor = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + byteoff + 2));

								int outlb = line.Get_Blue();
								int outnb = neighbor.Get_Blue();
								int outlr = line.Get_Red();
								int outnr = neighbor.Get_Red();
								int outng = neighbor.Get_Green();
								int outlg = line.Get_Green();
								if (blend_red) {
									outlr = (cov * (red * lf + line.Get_Red() * lbk)) >> 15;
									outnr = (cov * (red * nf + neighbor.Get_Red() * nbk)) >> 15;
								}
								if (blend_green) {
									outlg = (cov * (green * lf + line.Get_Green() * lbk)) >> 15;
									outng = (cov * (green * nf + neighbor.Get_Green() * nbk)) >> 15;
								}
								if (blend_blue) {
									outlb = (cov * (blue * lf + line.Get_Blue() * lbk)) >> 15;
									outnb = (cov * (blue * nf + neighbor.Get_Blue() * nbk)) >> 15;
								}

								int wb = std::min(255, outlb);
								if (outlg > 255) {
									outlg = 255;
								}
								if (outlr > 255) {
									outlr = 255;
								}
								*(unsigned short *)((char *)buffer + byteoff) = (unsigned short)Build_Hicolor_Pixel(outlr, outlg, wb);
								if (outnb > 255) {
									outnb = 255;
								}
								if (outng > 255) {
									outng = 255;
								}
								if (outnr > 255) {
									outnr = 255;
								}
								*(unsigned short *)((char *)buffer + byteoff + 2) = (unsigned short)Build_Hicolor_Pixel(outnr, outng, outnb);
								if (write_depth) {
									*zbuffer = z;
								}
							}
						}

						weight -= grad;
						otherweight -= neg_grad;
						if (otherweight > 256) {
							otherweight -= 256;
							weight += 256;
							byteoff += 2;
							zbuffer++;
							zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						}

						if (adda > 0) {
							z += astep;
							adda -= dy2;
						}

						adda += adelta2;
						buffer = (unsigned char *)buffer + pitch;
						z += zwrap;
						zbuffer += zwidth;
						if (zwidth > 0) {
							zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
						} else {
							zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
						}
						abuffer += zwidth;
						if (zwidth > 0) {
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
						i--;
					} while (i != 0);
				}

			}

		} else {

			/*
			 * The alpha gradient changes faster than either axis -- alpha drives the loop.
			 */
			int weight = 256;
			if (dx > abs_dy) {

				/*
				 * Alpha-dominant and shallow.
				 */
				int grad = (int)((double)abs_dy / (double)xcount * 256.0);
				int xoff = 0;
				if (dx > 0) {
					int otherweight = 0;
					int neg_grad = -grad;
					int neighoff = 2 * half_pitch;
					do {
						int cov = *abuffer;
						if (z < *zbuffer) {
							if (cov) {
								int lidx = (int)((double)weight * intensity);
								int oidx = (int)((double)otherweight * intensity);
								int nf = GradientTable[oidx];
								int nbk = GradientTable[256 - oidx];
								int lf = GradientTable[lidx];
								int lbk = GradientTable[256 - lidx];

								RGBClass line = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + 2 * xoff));
								RGBClass neighbor = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + neighoff));

								int outlg = line.Get_Green();
								int outng = neighbor.Get_Green();
								int outlr = line.Get_Red();
								int outnr = neighbor.Get_Red();
								int outlb = line.Get_Blue();
								int outnb = neighbor.Get_Blue();
								if (blend_red) {
									outlr = (cov * (red * nf + line.Get_Red() * nbk)) >> 15;
									outnr = (cov * (red * lf + neighbor.Get_Red() * lbk)) >> 15;
								}
								if (blend_green) {
									outlg = (cov * (green * nf + line.Get_Green() * nbk)) >> 15;
									outng = (cov * (green * lf + neighbor.Get_Green() * lbk)) >> 15;
								}
								if (blend_blue) {
									outlb = (cov * (blue * nf + line.Get_Blue() * nbk)) >> 15;
									outnb = (cov * (blue * lf + neighbor.Get_Blue() * lbk)) >> 15;
								}

								int wb = std::min(255, outnb);
								if (outng > 255) {
									outng = 255;
								}
								if (outnr > 255) {
									outnr = 255;
								}
								*(unsigned short *)((char *)buffer + 2 * xoff) = (unsigned short)Build_Hicolor_Pixel(outnr, outng, wb);
								if (outlb > 255) {
									outlb = 255;
								}
								if (outlg > 255) {
									outlg = 255;
								}
								if (outlr > 255) {
									outlr = 255;
								}
								*(unsigned short *)((char *)buffer + neighoff) = (unsigned short)Build_Hicolor_Pixel(outlr, outlg, outlb);
								if (write_depth) {
									*zbuffer = z;
								}
							}
						}

						weight -= grad;
						otherweight -= neg_grad;
						if (otherweight > 256) {
							otherweight -= 256;
							buffer = (unsigned char *)buffer + pitch;
							weight += 256;
							zbuffer += zwidth;
							if (zwidth > 0) {
								zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
							} else {
								zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
							}
							abuffer += zwidth;
							if (zwidth > 0) {
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							} else {
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
							}
							z += zwrap;
						}

						z += astep;
						zbuffer++;
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
						abuffer++;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						xoff += 2;
						neighoff += 4;
					} while (xoff < xcount);
				}
			} else {

				/*
				 * Alpha-dominant and steep.
				 */
				int grad = (int)((double)xcount / (double)abs_dy * 256.0);
				if (abs_dy > 0) {
					int otherweight = 0;
					int neg_grad = -grad;
					int i = abs_dy;
					int byteoff = 0;
					int yadjust = zwrap + astep;
					do {
						int cov = *abuffer;
						if (z < *zbuffer) {
							if (cov) {
								int lidx = (int)((double)weight * intensity);
								int oidx = (int)((double)otherweight * intensity);
								int lf = GradientTable[lidx];
								int lbk = GradientTable[256 - lidx];
								int nbk = GradientTable[256 - oidx];
								int nf = GradientTable[oidx];

								RGBClass line = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + byteoff));
								RGBClass neighbor = Deconstruct_Hicolor_Pixel(*(unsigned short *)((char *)buffer + byteoff + 2));

								int outlb = line.Get_Blue();
								int outnb = neighbor.Get_Blue();
								int outnr = neighbor.Get_Red();
								int outlr = line.Get_Red();
								int outng = neighbor.Get_Green();
								int outlg = line.Get_Green();
								if (blend_red) {
									outlr = (cov * (red * lf + line.Get_Red() * lbk)) >> 15;
									outnr = (cov * (red * nf + neighbor.Get_Red() * nbk)) >> 15;
								}
								if (blend_green) {
									outlg = (cov * (green * lf + line.Get_Green() * lbk)) >> 15;
									outng = (cov * (green * nf + neighbor.Get_Green() * nbk)) >> 15;
								}
								if (blend_blue) {
									outlb = (cov * (blue * lf + line.Get_Blue() * lbk)) >> 15;
									outnb = (cov * (blue * nf + neighbor.Get_Blue() * nbk)) >> 15;
								}

								int wb = 255;
								if (outlb <= 255) {
									wb = outlb;
								}
								if (outlg > 255) {
									outlg = 255;
								}
								if (outlr > 255) {
									outlr = 255;
								}
								*(unsigned short *)((char *)buffer + byteoff) = (unsigned short)Build_Hicolor_Pixel(outlr, outlg, wb);
								if (outnb > 255) {
									outnb = 255;
								}
								if (outng > 255) {
									outng = 255;
								}
								if (outnr > 255) {
									outnr = 255;
								}
								*(unsigned short *)((char *)buffer + byteoff + 2) = (unsigned short)Build_Hicolor_Pixel(outnr, outng, outnb);
								if (write_depth) {
									*zbuffer = z;
								}
							}
						}

						weight -= grad;
						otherweight -= neg_grad;
						if (otherweight > 256) {
							otherweight -= 256;
							weight += 256;
							byteoff += 2;
							zbuffer++;
							zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						}

						buffer = (unsigned char *)buffer + pitch;
						zbuffer += zwidth;
						if (zwidth > 0) {
							zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
						} else {
							zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
						}
						abuffer += zwidth;
						if (zwidth > 0) {
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
						z += yadjust;
						i--;
					} while (i != 0);
				}

			}
		}

		Unlock();
		return(true);
	}

	Unlock();
	return(false);

	}

	return(false);
}


/// <summary>
/// Blends one hicolor pixel into another.
/// This routine is used by the map preview blitter to fade a preview in over whatever is
/// already on the surface.
/// </summary>
/// <param name="src_color">The color being blended in.</param>
/// <param name="dst_color">The color already on the surface.</param>
/// <param name="level">How much of the source color to keep, as a percentage. Only the steps
/// 0, 25, 50, 75 and 100 are recognized.</param>
/// <returns>Returns with the blended pixel.</returns>
unsigned short DSurface::Blend_Pixel(unsigned short src_color, unsigned short dst_color, int level)
{
	switch (level) {
		case 100:
			return(src_color);
		case 75:
			return(((src_color / 4) & DSurface::QuarterbrightMask) * 4);
		case 50:
			return(((src_color / 2) & DSurface::HalfbrightMask) + ((dst_color / 2) & DSurface::HalfbrightMask));
		case 25:
			return(((src_color / 4) & DSurface::QuarterbrightMask) * 4);
		case 0:
			return(dst_color);
		default:
			break;
	}
	return(src_color);
}


/// <summary>
/// Draws a line whose color sweeps back and forth between two colors.
/// This routine is used by the radar to outline an event with a pulsing box. The caller holds
/// the sweep state between calls, so the four sides of a box are drawn as one continuous
/// gradient rather than four separate ones.
/// </summary>
/// <param name="gradient_step">The amount the sweep advances per pixel. Its sign is reversed
/// each time the sweep reaches one of the two colors.</param>
/// <param name="gradient_position">Where the sweep currently sits between the two colors.
/// Carried in from the previous segment and updated as the line is drawn.</param>
/// <returns>bool; Was the line drawn?</returns>
bool DSurface::Draw_Ping_Pong_Gradient_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, RGBClass const & start_color, RGBClass const & end_color, float & gradient_step, float & gradient_position) const
{
	//assert(xcliprect.Is_Valid());

	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect clipped_rect = Intersect(cliprect, Get_Rect());

	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	Point2D start = Bias_To(startpoint, clipped_rect);
	Point2D end = Bias_To(endpoint, clipped_rect);

	if (!Clip_Line_To_Rect(start, end, clipped_rect)) return(false);

	if (start.X > end.X) {
		std::swap(start, end);
	}

	if (Bytes_Per_Pixel() == 2) {

		void * buffer = Lock(start);
		if (buffer != NULL) {

			if (start.Y == end.Y) {

				/*
				 * Simplest of the blits, straight horizontal line.
				 */
				for (int i = 0; i <= end.X - start.X; i++) {
					RGBClass gradient_color;
					gradient_color.Lerp(start_color, end_color, gradient_position);

					*((unsigned short *)buffer + i) = (unsigned short)DSurface::Build_Hicolor_Pixel(gradient_color);

					gradient_position += gradient_step;
					if (gradient_position < 0.0f && gradient_step < 0.0f) {
						gradient_position = 0.0f;
						gradient_step = -gradient_step;
					} else if (gradient_position > 1.0f && gradient_step > 0.0f) {
						gradient_position = 1.0f;
						gradient_step = -gradient_step;
					}
				}

			} else if (start.X == end.X) {
				int pitch = start.Y > end.Y ? -Stride() : Stride();

				/*
				 * Straight vertical line.
				 */
				int dy = abs(end.Y - start.Y);
				for (int i = 0; i <= dy; i++) {
					RGBClass gradient_color;
					gradient_color.Lerp(start_color, end_color, gradient_position);

					*(unsigned short *)buffer = (unsigned short)DSurface::Build_Hicolor_Pixel(gradient_color);

					gradient_position += gradient_step;
					if (gradient_position < 0.0f && gradient_step < 0.0f) {
						gradient_position = 0.0f;
						gradient_step = -gradient_step;
					} else if (gradient_position > 1.0f && gradient_step > 0.0f) {
						gradient_position = 1.0f;
						gradient_step = -gradient_step;
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
					for (int i = 0; i <= dx; i++) {
						RGBClass gradient_color;
						gradient_color.Lerp(start_color, end_color, gradient_position);

						*((unsigned short *)buffer + i) = (unsigned short)DSurface::Build_Hicolor_Pixel(gradient_color);

						gradient_position += gradient_step;
						if (gradient_position < 0.0f && gradient_step < 0.0f) {
							gradient_position = 0.0f;
							gradient_step = -gradient_step;
						} else if (gradient_position > 1.0f && gradient_step > 0.0f) {
							gradient_position = 1.0f;
							gradient_step = -gradient_step;
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
					for (int i = 0; i <= dy; i++) {
						RGBClass gradient_color;
						gradient_color.Lerp(start_color, end_color, gradient_position);

						*((unsigned short *)buffer + k) = (unsigned short)DSurface::Build_Hicolor_Pixel(gradient_color);

						gradient_position += gradient_step;
						if (gradient_position < 0.0f && gradient_step < 0.0f) {
							gradient_position = 0.0f;
							gradient_step = -gradient_step;
						} else if (gradient_position > 1.0f && gradient_step > 0.0f) {
							gradient_position = 1.0f;
							gradient_step = -gradient_step;
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
	}
	return(false);
}


/// <summary>
/// Draws a clipped, depth-tested, alpha-masked colored line into the locked DSurface.
/// Uses the global DepthBuffer for the Z-test and the global AlphaBuffer for a per-pixel
/// brightness value (0..127) that darkens the source color. The depth endpoints are
/// re-interpolated across the portion of the line that survives clipping, then the line is
/// rasterized with a 3-axis Bresenham whose major axis is max(dx, |dy|, |dz|).
/// </summary>
/// <param name="cliprect">Clipping rectangle (intersected with the surface rect).</param>
/// <param name="startpoint">Line start point.</param>
/// <param name="endpoint">Line end point.</param>
/// <param name="color">Source color (hicolor pixel value).</param>
/// <param name="start_depth">Depth value at the start point.</param>
/// <param name="end_depth">Depth value at the end point.</param>
/// <param name="write_depth">If true, the depth buffer is updated at each plotted pixel.</param>
/// <returns>True if any portion of the line was drawn, false otherwise.</returns>
bool DSurface::Draw_Depth_Shaded_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, unsigned color, int start_depth, int end_depth, bool write_depth)
{
	/*
	**	Ensure that the clipping rectangle is legal.
	*/
	Rect clipped_rect = Intersect(cliprect, Get_Rect());
	if (clipped_rect.Width == 0 && clipped_rect.Height == 0) {
		return(false);
	}

	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	Point2D start = Bias_To(startpoint, clipped_rect);
	Point2D end = Bias_To(endpoint, clipped_rect);

	/*
	 * Sort the endpoints left-to-right, carrying the depth values with them.
	 */
	int zfirst = start_depth;
	int zlast = end_depth;
	if (start.X > end.X) {
		std::swap(start, end);
		zfirst = end_depth;
		zlast = start_depth;
	}

	/*
	 * Remember the pre-clip endpoints so the depth values can be re-interpolated
	 * across the portion of the line that survives clipping.
	 */
	Point2D unclipped = end;
	int px = start.X;
	int py = start.Y;

	if (!Clip_Line_To_Rect(start, end, clipped_rect)) {
		return(false);
	}

	int zend = zlast;
	int zstart = zfirst;

	/*
	 * Re-interpolate the far depth value if the far endpoint was clipped.
	 */
	if (end.X != unclipped.X || end.Y != unclipped.Y) {
		Point2D clipped_point;
		clipped_point.X = px - end.X;
		double clipped_dx = (double)clipped_point.X;
		clipped_point.Y = py - end.Y;
		int clipped = (int)(int)std::sqrt((double)clipped_point.Y * (double)clipped_point.Y + clipped_dx * clipped_dx);
		Point2D full_point;
		full_point.X = px - unclipped.X;
		double full_dx = (double)full_point.X;
		full_point.Y = py - unclipped.Y;
		int amount = abs((int)(int)((double)clipped
			/ (double)(int)(int)std::sqrt((double)full_point.Y * (double)full_point.Y + full_dx * full_dx)
			* (double)(zfirst - zlast)));
		if (zfirst < zlast) {
			zend = zfirst + amount;
		} else {
			zend = zfirst - amount;
		}
	}

	/*
	 * Re-interpolate the near depth value if the near endpoint was clipped.
	 */
	if (start.X != px || start.Y != py) {
		Point2D clipped_point;
		clipped_point.X = start.X - unclipped.X;
		double clipped_dx = (double)clipped_point.X;
		clipped_point.Y = start.Y - unclipped.Y;
		int clipped = (int)(int)std::sqrt((double)clipped_point.Y * (double)clipped_point.Y + clipped_dx * clipped_dx);
		Point2D full_point;
		full_point.X = px - unclipped.X;
		double full_dx = (double)full_point.X;
		full_point.Y = py - unclipped.Y;
		int amount = abs((int)(int)((double)clipped
			/ (double)(int)(int)std::sqrt((double)full_point.Y * (double)full_point.Y + full_dx * full_dx)
			* (double)(zfirst - zlast)));
		if (zlast > zfirst) {
			zstart = zlast - amount;
		} else {
			zstart = zlast + amount;
		}
	}

	Bytes_Per_Pixel();
	void * buffer = Lock(start);

	if (DepthBuffer == NULL) {
		return(false);
	}

	Point2D point = start;
	point.Y -= DepthBuffer->Get_Bounds().Y;
	unsigned short * zbuffer = (unsigned short *)DepthBuffer->Get_Buffer_Offset(point);
	point = start;
	point.Y -= AlphaBuffer->Get_Bounds().Y;
	unsigned short * abuffer = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(point);

	int start_y = start.Y;
	int zwrap = -1;
	unsigned short z = (unsigned short)(zstart + (short)DepthBuffer->Get_Scroll_Delta(start_y + clipped_rect.Y - DepthBuffer->Get_Bounds().Y));
	int zwidth = DepthBuffer->Get_Buffer_Width();

	int red;
	int green;
	int blue;

	if (buffer != NULL) {
		/// Decompose the source color into its gun components so each plotted pixel
		/// can be darkened by the alpha-buffer brightness value.
		RGBClass rgb(
			(unsigned char)(((unsigned short)color >> RedRight) << RedLeft),
			(unsigned char)(((unsigned short)color >> GreenRight) << GreenLeft),
			(unsigned char)(((unsigned short)color >> BlueRight) << BlueLeft));
		red = rgb.Get_Red();
		green = rgb.Get_Green();
		blue = rgb.Get_Blue();
	} else {
		return(false);
	}

	/*
	 * Distances to x, y and depth.
	 */
	int dy = end.Y - start.Y;
	int dx = end.X - start.X;
	int dz = zend - zstart;

	int pitch = Stride();
	if (dy < 0) {
		pitch = -pitch;
		zwidth = -zwidth;
		zwrap = 1;
	}

	int zstep = 1;
	int abs_dy = abs(dy);
	if (dz < 0) {
		zstep = -1;
	}

	int abs_dz = abs(dz);
	int dx2 = 2 * dx;
	int dy2 = 2 * abs_dy;
	int dz2 = 2 * abs_dz;

	if (abs_dz > dx && abs_dz > abs_dy) {

		/*
		 * The depth axis changes fastest.
		 */
		int xdelta = dy2 - abs_dz;
		int ydelta = dx2 - abs_dz;
		if (abs_dz > 0) {
			int offset = 0;
			for (int i = abs_dz; i != 0; i--) {

				int v = *abuffer;
				if (z < *zbuffer && v != 0) {
					if (v != 127) {
						*(unsigned short *)((char *)buffer + offset) = (unsigned short)((((red * v) >> 7 >> RedLeft) << RedRight) | (((green * v) >> 7 >> GreenLeft) << GreenRight) | (((blue * v) >> 7 >> BlueLeft) << BlueRight));
					} else {
						*(unsigned short *)((char *)buffer + offset) = (unsigned short)color;
					}
					if (write_depth) {
						*zbuffer = z;
					}
				}

				if (xdelta > 0) {
					buffer = (unsigned char *)buffer + pitch;
					xdelta -= dz2;
					zbuffer += zwidth;
					if (zwidth > 0) {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					} else {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
					}
					abuffer += zwidth;
					if (zwidth > 0) {
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					} else {
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
					}
					z += zwrap;
				}

				if (ydelta > 0) {
					zbuffer++;
					offset += 2;
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					abuffer++;
					abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					ydelta -= dz2;
				}

				z += zstep;
				ydelta += dx2;
				xdelta += dy2;
			}
		}
	} else if (dx > abs_dy) {

		/*
		 * The slope is not steep -- plot a low line.
		 */
		int ydelta = dy2 - dx;
		int i = 0;
		int zdelta = dz2 - dx;
		if (dx > 0) {
			for (; i < dx; i++) {

				int v = *abuffer;
				if (z < *zbuffer && v != 0) {
					if (v != 127) {
						*(unsigned short *)((char *)buffer + 2 * i) = (unsigned short)((((red * v) >> 7 >> RedLeft) << RedRight) | (((green * v) >> 7 >> GreenLeft) << GreenRight) | (((blue * v) >> 7 >> BlueLeft) << BlueRight));
					} else {
						*(unsigned short *)((char *)buffer + 2 * i) = (unsigned short)color;
					}
					if (write_depth) {
						*zbuffer = z;
					}
				}

				if (ydelta > 0) {
					buffer = (unsigned char *)buffer + pitch;
					ydelta -= 2 * dx;
					zbuffer += zwidth;
					if (zwidth > 0) {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					} else {
						zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
					}
					z += zwrap;
					abuffer += zwidth;
					if (zwidth > 0) {
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					} else {
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
					}
				}

				if (zdelta > 0) {
					z += zstep;
					zdelta -= 2 * dx;
				}

				zbuffer++;
				ydelta += dy2;
				zdelta += dz2;
				zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
				abuffer++;
				abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
			}
		}
	} else {

		/*
		 * The slope is steep -- plot a high line.
		 */
		int xdelta = dx2 - abs_dy;
		int zdelta = 2 * abs_dz - abs_dy;
		if (abs_dy > 0) {
			int i = abs_dy;
			int offset = 0;
			do {

				int v = *abuffer;
				if (z < *zbuffer && v != 0) {
					if (v != 127) {
						*(unsigned short *)((char *)buffer + offset) = (unsigned short)((((red * v) >> 7 >> RedLeft) << RedRight) | (((green * v) >> 7 >> GreenLeft) << GreenRight) | (((blue * v) >> 7 >> BlueLeft) << BlueRight));
					} else {
						*(unsigned short *)((char *)buffer + offset) = (unsigned short)color;
					}
					if (write_depth) {
						*zbuffer = z;
					}
				}

				if (xdelta > 0) {
					zbuffer++;
					offset += 2;
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
					abuffer++;
					abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					xdelta -= dy2;
				}

				if (zdelta > 0) {
					z += zstep;
					zdelta -= dy2;
				}

				xdelta += dx2;
				zdelta += dz2;
				buffer = (unsigned char *)buffer + pitch;

				zbuffer += zwidth;
				if (zwidth > 0) {
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Overflow((unsigned int)zbuffer);
				} else {
					zbuffer = (unsigned short *)DepthBuffer->Wrap_Underflow((unsigned int)zbuffer);
				}
				z += zwrap;
				abuffer += zwidth;
				if (zwidth > 0) {
					abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
				} else {
					abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
				}

				i--;
			} while (i != 0);
		}
	}

	Unlock();
	return(true);
}


/// <summary>
/// Draws a dashed line masked against the alpha buffer.
/// This routine is used by the tactical map for rally point and waypoint path lines, which
/// march along as the game runs. The dash pattern carries from one segment to the next, so a
/// path made of several calls looks like one unbroken run of dashes.
/// </summary>
/// <param name="pattern">The on and off steps that the dashes are drawn with.</param>
/// <param name="offset">The step within the pattern that this line starts on.</param>
/// <param name="draw_on_zero_alpha">Should the line appear where the alpha buffer is clear
/// rather than where it is set?</param>
/// <returns>Returns with the pattern step the line finished on, ready to be handed back for
/// the following segment.</returns>
/// <remarks>The line is not clipped -- the caller must clip it to the surface first.</remarks>
int DSurface::Draw_Masked_Dashed_Line(Point2D const & startpoint, Point2D const & endpoint, unsigned color, bool pattern[], int offset, bool draw_on_zero_alpha)
{
	/*
	**	High-speed working variables for the clipping rectangle and clipping operation.
	*/
	Point2D start = startpoint;
	Point2D end = endpoint;
	int pattern_step = 1;

	if (Bytes_Per_Pixel() != 2) return(0);

	if (start.X > end.X) {
		std::swap(start, end);
		int dx = abs(start.X - end.X);
		int dy = abs(start.Y - end.Y) + 1;
		if (dx < dy) {
			dx = dy;
		}
		offset = (offset + dx) % 16;
		pattern_step = -1;
	}

	Point2D point = start;
	point.Y -= AlphaBuffer->Get_Bounds().Y;
	unsigned short * abuffer = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(point);
	int astride = AlphaBuffer->Get_Buffer_Width();
	if (start.Y > end.Y) astride = -astride;

	void * buffer = (unsigned short *)Lock(start);
	if (buffer != NULL) {
		if (draw_on_zero_alpha) {

			if (start.Y == end.Y) {

				/*
				 * Simplest of the blits, straight horizontal line.
				 */
				for (int i = 0; i <= end.X - start.X; i++, offset += pattern_step) {
					if (offset < 0) {
						offset += 16;
					}
					if (offset >= 16) {
						offset -= 16;
					}

					if (pattern[offset] && *abuffer == 0) {
						*((unsigned short *)buffer + i) = color;
					}

					if (astride > 0) {
						abuffer++;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					} else {
						abuffer++;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
					}
				}
			} else if (start.X == end.X) {
				int pitch = start.Y > end.Y ? -Stride() : Stride();

				/*
				 * Straight vertical line.
				 */
				int dy = abs(end.Y - start.Y);
				for (int i = 0; i <= dy; i++, offset += pattern_step) {
					if (offset < 0) {
						offset += 16;
					}
					if (offset >= 16) {
						offset -= 16;
					}

					if (pattern[offset] && *abuffer == 0) {
						*(unsigned short *)buffer = color;
					}

					buffer = (unsigned char *)buffer + pitch;

					if (astride > 0) {
						abuffer += astride;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					} else {
						abuffer += astride;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
					}
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
					for (int i = 0; i < dx; i++, offset += pattern_step) {
						if (offset < 0) {
							offset += 16;
						}
						if (offset >= 16) {
							offset -= 16;
						}

						if (pattern[offset] && *abuffer == 0) {
							*((unsigned short *)buffer + i) = color;
						}

						if (delta > 0) {
							buffer = (unsigned char *)buffer + pitch;
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							delta -= dx2;
						}

						delta += dy2;

						if (astride > 0) {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
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
					for (int i = 0; i < dy; i++, offset += pattern_step) {
						if (offset < 0) {
							offset += 16;
						}
						if (offset >= 16) {
							offset -= 16;
						}

						if (pattern[offset] && *abuffer == 0) {
							*((unsigned short *)buffer + k) = color;
						}

						if (delta > 0) {
							k++;
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							delta -= dy2;
						}

						delta += dx2;
						buffer = (unsigned char *)buffer + pitch;

						if (astride > 0) {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
					}
				}
			}
		} else {

			if (start.Y == end.Y) {

				/*
				 * Simplest of the blits, straight horizontal line.
				 */
				for (int i = 0; i <= end.X - start.X; i++, offset += pattern_step) {
					if (offset < 0) {
						offset += 16;
					}
					if (offset >= 16) {
						offset -= 16;
					}

					if (pattern[offset] && *abuffer != 0) {
						*((unsigned short *)buffer + i) = color;
					}

					if (astride > 0) {
						abuffer++;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					} else {
						abuffer++;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
					}
				}
			} else if (start.X == end.X) {
				int pitch = start.Y > end.Y ? -Stride() : Stride();

				/*
				 * Straight vertical line.
				 */
				int dy = abs(end.Y - start.Y);
				for (int i = 0; i <= dy; i++, offset += pattern_step) {
					if (offset < 0) {
						offset += 16;
					}
					if (offset >= 16) {
						offset -= 16;
					}

					if (pattern[offset] && *abuffer != 0) {
						*(unsigned short *)buffer = color;
					}

					buffer = (unsigned char *)buffer + pitch;

					if (astride > 0) {
						abuffer += astride;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
					} else {
						abuffer += astride;
						abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
					}
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
					for (int i = 0; i < dx; i++, offset += pattern_step) {
						if (offset < 0) {
							offset += 16;
						}
						if (offset >= 16) {
							offset -= 16;
						}

						if (pattern[offset] && *abuffer != 0) {
							*((unsigned short *)buffer + i) = color;
						}

						if (delta > 0) {
							buffer = (unsigned char *)buffer + pitch;
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							delta -= dx2;
						}

						delta += dy2;

						if (astride > 0) {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
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
					for (int i = 0; i < dy; i++, offset += pattern_step) {
						if (offset < 0) {
							offset += 16;
						}
						if (offset >= 16) {
							offset -= 16;
						}

						if (pattern[offset] && *abuffer != 0) {
							*((unsigned short *)buffer + k) = color;
						}

						if (delta > 0) {
							k++;
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							delta -= dy2;
						}

						delta += dx2;
						buffer = (unsigned char *)buffer + pitch;

						if (astride > 0) {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
					}
				}
			}
		}

		Unlock();
		return(offset);
	}

	return(offset);
}


/// <summary>
/// Draws a line masked against the alpha buffer.
/// This routine is used by the tactical map to trail a line out to a waypoint. The alpha
/// buffer decides which pixels of the line are allowed to appear, so the line can be made
/// to show only where the shroud is, or only where it is not.
/// </summary>
/// <param name="draw_on_zero_alpha">Should the line appear where the alpha buffer is clear
/// rather than where it is set?</param>
/// <returns>bool; Was the line drawn?</returns>
bool DSurface::Draw_Masked_Line(Point2D const & startpoint, Point2D const & endpoint, unsigned color, bool draw_on_zero_alpha)
{
	if (Bytes_Per_Pixel() == 2) {
		/*
		**	Ensure that the clipping rectangle is legal.
		*/
		Rect cliprect = Get_Rect();

		/*
		**	High-speed working variables for the clipping rectangle and clipping operation.
		*/
		Point2D start = Bias_To(startpoint, cliprect);
		Point2D end = Bias_To(endpoint, cliprect);

		if (!Clip_Line_To_Rect(start, end, cliprect)) return(false);

		if (start.X > end.X) {
			std::swap(start, end);
		}

		Point2D point = start;
		point.Y -= AlphaBuffer->Get_Bounds().Y;
		unsigned short * abuffer = (unsigned short *)AlphaBuffer->Get_Buffer_Offset(point);
		int astride = AlphaBuffer->Get_Buffer_Width();
		if (start.Y > end.Y) astride = -astride;

		void * buffer = (unsigned short *)Lock(start);
		if (buffer != NULL) {
			if (draw_on_zero_alpha) {

				if (start.Y == end.Y) {

					/*
					 * Simplest of the blits, straight horizontal line.
					 */
					for (int i = 0; i <= end.X - start.X; i++) {
						if (*abuffer == 0) {
							*((unsigned short *)buffer + i) = color;
						}

						if (astride > 0) {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
					}
				} else if (start.X == end.X) {
					int pitch = start.Y > end.Y ? -Stride() : Stride();

					/*
					 * Straight vertical line.
					 */
					int dy = abs(end.Y - start.Y);
					for (int i = 0; i <= dy; i++) {
						if (*abuffer == 0) {
							*(unsigned short *)buffer = color;
						}

						if (astride > 0) {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
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
							if (*abuffer == 0) {
								*((unsigned short *)buffer + i) = color;
							}

							if (delta > 0) {
								buffer = (unsigned char *)buffer + pitch;
								abuffer += astride;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
								delta -= dx2;
							}

							if (astride > 0) {
								abuffer++;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							} else {
								abuffer++;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
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
							if (*abuffer == 0) {
								*((unsigned short *)buffer + k) = color;
							}

							if (delta > 0) {
								k++;
								delta -= dy2;
								abuffer++;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							}

							delta += dx2;
							buffer = (unsigned char *)buffer + pitch;

							if (astride > 0) {
								abuffer += astride;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							} else {
								abuffer += astride;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
							}
						}
					}
				}
			} else {

				if (start.Y == end.Y) {

					/*
					 * Simplest of the blits, straight horizontal line.
					 */
					for (int i = 0; i <= end.X - start.X; i++) {
						if (*abuffer != 0) {
							*((unsigned short *)buffer + i) = color;
						}

						if (astride > 0) {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer++;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
						}
					}
				} else if (start.X == end.X) {
					int pitch = start.Y > end.Y ? -Stride() : Stride();

					/*
					 * Straight vertical line.
					 */
					int dy = abs(end.Y - start.Y);
					for (int i = 0; i <= dy; i++) {
						if (*abuffer != 0) {
							*(unsigned short *)buffer = color;
						}

						if (astride > 0) {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
						} else {
							abuffer += astride;
							abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
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
							if (*abuffer != 0) {
								*((unsigned short *)buffer + i) = color;
							}

							if (delta > 0) {
								buffer = (unsigned char *)buffer + pitch;
								abuffer += astride;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
								delta -= dx2;
							}

							if (astride > 0) {
								abuffer++;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							} else {
								abuffer++;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
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
							if (*abuffer != 0) {
								*((unsigned short *)buffer + k) = color;
							}

							if (delta > 0) {
								k++;
								delta -= dy2;
								abuffer++;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							}

							delta += dx2;
							buffer = (unsigned char *)buffer + pitch;

							if (astride > 0) {
								abuffer += astride;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Overflow((unsigned int)abuffer);
							} else {
								abuffer += astride;
								abuffer = (unsigned short *)AlphaBuffer->Wrap_Underflow((unsigned int)abuffer);
							}
						}
					}
				}
			}

			Unlock();
			return(true);
		}
	}

	return(false);
}
