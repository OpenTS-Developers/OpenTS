/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "point.h"

#include <windows.h>

class Surface;
class XSurface;
class CCINIClass;
class Cell;

class MapPreviewClass
{
public:
	MapPreviewClass(void);
	~MapPreviewClass(void);

	bool Write_INI(CCINIClass & ini);
	bool Read_INI(CCINIClass const & ini);

	bool Read_PCX_Preview(char const * filename);
	bool Read_INI_Preview(char const * filename);

	void Create_Preview(void);
	void Blit_Preview(HWND window);

	unsigned * Create_Paletted_Preview(int, int & size);
	void Create_Preview_Surface(char * buffer);
	XSurface * Get_Preview_Surface(void) { return(SurfacePtr); }

private:
	Point2D Cell_To_Preview_Pixel(Cell cell) const;

public:
	struct Header {
		/*
		 * These are the pixel dimensions of the paletted preview image that follows this
		 * header, which is packed as the header, the color count, the palette, and then one
		 * byte of palette index per pixel.
		 */
		int Width;
		int Height;
	};
	static_assert(sizeof(Header) == 8, "the preview header is 8 bytes on disk");


private:
	/*
	 * This is the surface the map preview image lives on, created when a preview is rendered
	 * from the loaded map or expanded from a packed preview block, and destroyed along with
	 * this object. If NULL, then there is no preview available to display.
	 */
	XSurface * SurfacePtr;
};
