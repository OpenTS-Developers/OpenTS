/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "preview.h"

#include "_map.h"
#include "_surface.h"
#include "_tactica.h"
#include "ccfile.h"
#include "ccini.h"
#include "cell.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "language\language.h"
#include "lzopipe.h"
#include "lzostraw.h"
#include "overtype.h"
#include "ownrdraw.h"
#include "pcx.h"
#include "scenario.h"
#include "surface.h"
#include "tactical.h"
#include "terrain.h"
#include "windlg.h"
#include "xpipe.h"
#include "xstraw.h"

#include "color.hh"

#include <algorithm>


/// <summary>
/// Creates an empty map preview.
/// The object starts out with no picture at all. One must be rendered or read in
/// before the preview can be drawn.
/// </summary>
MapPreviewClass::MapPreviewClass(void) :
	SurfacePtr(NULL)
{
}


/// <summary>
/// Discards any preview image this object is holding.
/// </summary>
MapPreviewClass::~MapPreviewClass(void)
{
	if (SurfacePtr != NULL) {
		delete SurfacePtr;
		SurfacePtr = NULL;
	}
}


/// <summary>
/// Draws the preview image into a dialog.
/// This routine is used by the scenario selection dialogs. The picture is scaled to fit
/// the preview frame of the window while keeping its proportions.
/// </summary>
/// <param name="window">The dialog window that owns the preview frame.</param>
void MapPreviewClass::Blit_Preview(HWND window)
{
	ValidateRect(window, NULL);

	if (SurfacePtr != NULL) {
		RECT winrect;
		Get_Display_Rect(GetDlgItem(window, IDC_PREVIEW_FRAME), &winrect);
		Rect framerect(winrect.left, winrect.top, winrect.right - winrect.left, winrect.bottom - winrect.top);
		Rect destrect;
		Rect srcrect = SurfacePtr->Get_Rect();
		int scale = std::min(1000 * framerect.Width / srcrect.Width, 1000 * framerect.Height / srcrect.Height);

		destrect.X = framerect.X + framerect.Width / 2 - (scale * srcrect.Width) / 2000;
		destrect.Y = framerect.Y + framerect.Height / 2 - (scale * srcrect.Height) / 2000;
		destrect.Width = (scale * srcrect.Width) / 1000;
		destrect.Height = (scale * srcrect.Height) / 1000;
		AlternateSurface->Blit_From(destrect, *SurfacePtr, SurfacePtr->Get_Rect(), false, false);
	}
}


/// <summary>
/// Converts a map cell into its location on the preview.
/// </summary>
/// <returns>Returns with the preview pixel that the cell falls on.</returns>
Point2D MapPreviewClass::Cell_To_Preview_Pixel(Cell cell) const
{
	int x;
	int y;
	TacticalMap->Lepton_To_Map_Pixel(Coord(cell).X, Coord(cell).Y, x, y);
	return(Point2D(x / ISO_TILE_PIXEL_W, y / ISO_TILE_PIXEL_H));
}


/// <summary>
/// Creates the preview image of the current map.
/// This routine renders the playable area of the map into a miniature picture, coloring
/// each cell according to its terrain, tiberium, and overlay, and marking the player
/// starting positions. The picture becomes this object's current preview.
/// </summary>
void MapPreviewClass::Create_Preview(void)
{
	CellClass *cptr;

	Map.Reset_Iterator();

	int minx = 10000;
	int maxx = 0;
	int miny = 10000;
	int originx = 10000;
	int originy = 10000;
	int maxy = 0;

	cptr = Map.Iterate();
	while (cptr != NULL) {
		if (Map.In_Radar(cptr->CellID)) {
			Point2D pixel = Cell_To_Preview_Pixel(cptr->CellID);
			if (pixel.X < minx) {
				originx = pixel.X;
				minx = pixel.X;
			}
			if (pixel.X > maxx) {
				maxx = pixel.X;
			}
			if (pixel.Y < miny) {
				originy = pixel.Y;
				miny = pixel.Y;
			}
			if (pixel.Y > maxy) {
				maxy = pixel.Y;
			}
		}
		cptr = Map.Iterate();

	}

	int width = 2 * (maxx - minx);
	int height = maxy - miny;

	if (SurfacePtr != NULL) {
		delete SurfacePtr;
	}

	SurfacePtr = new DSurface(width, height);
	SurfacePtr->Fill(0);

	Map.Reset_Iterator();

	cptr = Map.Iterate();

	SurfacePtr->Lock();
	while (cptr != NULL) {
		Cell cell = cptr->Fetch_CellID();
		if (Map.In_Radar(cell)) {
			unsigned char unknown = 0;
			unsigned int lowcolor = Map[cell].Preview_Cell_Color(unknown, false);
			Point2D pixel = Cell_To_Preview_Pixel(cell);

			int x = 2 * (pixel.X - minx);
			int y = pixel.Y - miny;

			bool hasradarcolor = false;
			int level = 100;
			int radarcolor = 0;

			if (cptr->Overlay != OVERLAY_NONE) {
				OverlayTypeClass *optr = OverlayTypes[cptr->Overlay];
				RGBClass rgb = optr->Get_Radar_Color(cptr->OverlayData);
				if (optr->IsTiberium) {
					level = 75;
					hasradarcolor = true;
					radarcolor = DSurface::Build_Hicolor_Pixel(rgb.Get_Red(), rgb.Get_Green(), rgb.Get_Blue());
					if (cptr->OverlayData < 2) {
						level = 25;
					} else if (cptr->OverlayData < 5) {
						level = 50;
					}
					lowcolor = DSurface::Blend_Pixel(radarcolor, lowcolor, level);
				} else {
					/*
					 * Non-Tiberium overlays flag the cell as colored but never set
					 * radarcolor, so the cell draws black on the preview.
					 */
					if (DSurface::Build_Hicolor_Pixel(rgb.Get_Red(), rgb.Get_Green(), rgb.Get_Blue()) != 0) {
						hasradarcolor = true;
					}
				}
			} else {
				TerrainClass *optr = (TerrainClass *)cptr->Cell_Occupier();
				if (optr != NULL && optr->What_Am_I() == RTTI_TERRAIN) {
					hasradarcolor = true;
					level = 75;
					radarcolor = DSurface::Build_Hicolor_Pixel(optr->Class->RadarColor.Get_Red(), optr->Class->RadarColor.Get_Green(), optr->Class->RadarColor.Get_Blue());
				}
			}
			/// The cell's low and high preview colors arrive packed into one value.
			int highcolor = lowcolor & 0x0000FFFF;
			lowcolor >>= 16;
			if (hasradarcolor) {
				if (level == 100) {
					highcolor = radarcolor;
					lowcolor = radarcolor;
				} else {
					highcolor = DSurface::Blend_Pixel(radarcolor, highcolor, level);
					lowcolor = DSurface::Blend_Pixel(radarcolor, lowcolor, level);
				}
			}

			SurfacePtr->Put_Pixel_Clip(Point2D(x, y), lowcolor, SurfacePtr->Get_Rect());
			SurfacePtr->Put_Pixel_Clip(Point2D(x + 1, y), highcolor, SurfacePtr->Get_Rect());

			minx = originx;
			miny = originy;
		}
		cptr = Map.Iterate();
	}

	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (Scen->Is_Valid_Waypoint(i)) {
			Cell cell = Scen->Get_Waypoint_Cell(i);
			Point2D pixel = Cell_To_Preview_Pixel(cell);
			Rect rect;
			rect.X = 2 * (pixel.X - minx) - 1;
			rect.Y = pixel.Y - miny - 1;
			rect.Width = 4;
			rect.Height = 4;
			SurfacePtr->Fill_Rect(SurfacePtr->Get_Rect(), rect, DSurface::Build_Hicolor_Pixel(240, 0, 0));
			miny = originy;
		}
	}

	SurfacePtr->Unlock();
}


/// <summary>
/// Stores the preview image into the INI database.
/// This routine is used when a scenario is saved. If there is no preview picture on
/// hand, one is rendered from the current map and then discarded again afterwards.
/// </summary>
/// <returns>bool; Was a preview image stored?</returns>
bool MapPreviewClass::Write_INI(CCINIClass & ini)
{
	static char const * const PREVIEW = "Preview";
	static char const * const PREVIEWPACK = "PreviewPack";

	bool created = false;
	if (SurfacePtr == NULL) {
		Create_Preview();
		created = true;
	}

	ini.Clear(PREVIEW);

	ini.Put_Rect(PREVIEW, "Size", SurfacePtr->Get_Rect());

	BufferPipe bpipe(AlternateSurface->Lock(), AlternateSurface->Get_Width() * AlternateSurface->Get_Height() * AlternateSurface->Bytes_Per_Pixel());
	LZOPipe comp(LZOPipe::COMPRESS);
	comp.Put_To(&bpipe);

	int total = 0;
	RGBClass rgb(0,0,0);

	SurfacePtr->Lock();
	for (int y = 0; y < SurfacePtr->Get_Height(); y++) {
		for (int x = 0; x < SurfacePtr->Get_Width(); x++) {
			int pixel = SurfacePtr->Get_Pixel(Point2D(x,y));
			rgb = DSurface::Deconstruct_Hicolor_Pixel(pixel);

			total += comp.Put(&rgb, sizeof(rgb));
		}
	}
	SurfacePtr->Unlock();

	total += comp.End();
	ini.Clear(PREVIEWPACK);
	if (total > 0) {
		ini.Put_UUBlock(PREVIEWPACK, AlternateSurface->Lock(), total);
		AlternateSurface->Unlock();
	}

	AlternateSurface->Unlock();

	if (created) {
		delete SurfacePtr;
		SurfacePtr = NULL;
	}
	return(total > 0);
}


/// <summary>
/// Reads the preview image from the INI database.
/// The picture is rebuilt from the compressed block that Write_INI stored and becomes
/// this object's current preview.
/// </summary>
/// <returns>bool; Was a complete preview image recovered?</returns>
bool MapPreviewClass::Read_INI(CCINIClass const & ini)
{
	static char const * const PREVIEW = "Preview";
	static char const * const PREVIEWPACK = "PreviewPack";

	if (SurfacePtr != NULL) {
		delete SurfacePtr;
	}

	Rect rect = ini.Get_Rect(PREVIEW, "Size", Map.LocalRect);

	SurfacePtr = new DSurface(rect.Width, rect.Height);
	SurfacePtr->Fill(0);

	int size = AlternateSurface->Get_Width() * AlternateSurface->Get_Height() * AlternateSurface->Bytes_Per_Pixel();
	int length = ini.Get_UUBlock(PREVIEWPACK, (unsigned char *)AlternateSurface->Lock(), size);

	if (length > 0) {
		unsigned char *buffer = (unsigned char *)AlternateSurface->Lock();
		BufferStraw bstraw(buffer, length);
		LZOStraw uncomp(LZOStraw::DECOMPRESS);
		uncomp.Get_From(&bstraw);
		unsigned char rgb[3] = {0,0,0};
		SurfacePtr->Lock();

		for (int y = 0; y < SurfacePtr->Get_Height(); y++) {
			for (int x = 0; x < SurfacePtr->Get_Width(); x++) {
				if (uncomp.Get(rgb, sizeof(rgb)) != sizeof(rgb)) {
					SurfacePtr->Unlock();
					AlternateSurface->Unlock();
					AlternateSurface->Unlock();
					return(false);
				}
				SurfacePtr->Put_Pixel(Point2D(x,y), DSurface::Build_Hicolor_Pixel(rgb[0], rgb[1], rgb[2]));
			}
		}

		SurfacePtr->Unlock();
		AlternateSurface->Unlock();
	}
	AlternateSurface->Unlock();
	return(true);
}


/// <summary>
/// Loads the preview image from a picture file.
/// This routine is used for scenarios that ship a hand drawn preview picture instead
/// of one rendered from the map itself.
/// </summary>
/// <param name="filename">The name of the PCX file to load.</param>
/// <returns>bool; Was the preview picture loaded?</returns>
bool MapPreviewClass::Read_PCX_Preview(char const * filename)
{
	CCFileClass file(filename);

	if (file.Is_Available()) {
		if (SurfacePtr != NULL) {
			delete SurfacePtr;
			SurfacePtr = NULL;
		}
		Surface *surf = Read_PCX_File(file);

		if (surf != NULL && surf->Get_Width() && surf->Get_Height()) {
			SurfacePtr = new DSurface(surf->Get_Width(), surf->Get_Height());
			SurfacePtr->Fill(TBLACK);
			SurfacePtr->Blit_From(*surf);
			delete surf;
			return(true);
		}
	}

	return(false);
}


/// <summary>
/// Reads the preview image out of a scenario file.
/// This routine is used to peek at a scenario without loading it. Only as much of the
/// file as is needed to reach the map section is parsed, so that browsing a directory
/// full of scenarios stays cheap.
/// </summary>
/// <param name="filename">The name of the scenario file to peek into.</param>
/// <returns>bool; Was a preview image recovered from the file?</returns>
bool MapPreviewClass::Read_INI_Preview(char const * filename)
{
	CCFileClass file(filename);
	if (file.Is_Available() && file.Open(FileClass::READ)) {
		int percent_load = 0;
		char * buffer = NULL;

		while (percent_load < 100) {
			percent_load += 25;

			int length = percent_load * file.Size() / 100;
			if (buffer != NULL) {
				delete [] buffer;
			}
			buffer = new char[length + 1];
			if (buffer == NULL) {
				return(false);
			}

			file.Seek(0, 0);
			file.Read(buffer, length);
			buffer[length] = '\0';

			char * section = strstr(buffer, "[Map]");
			if (section != NULL) {
				section[0] = '\0';
				CCINIClass ini;
				BufferStraw straw(buffer, section - buffer - 1);
				ini.Load(straw, false);
				bool result = Read_INI(ini);
				delete [] buffer;
				return(result);
			}
		}
		delete [] buffer;
	}
	return(false);
}


/// <summary>
/// Creates a compact paletted copy of the preview image.
/// The picture is scaled down and its colors are matched against a palette built from
/// the shades that appear most often in it. Use this routine when the preview must be
/// handed on in as few bytes as possible.
/// </summary>
/// <param name="colorcount">The number of palette entries to allow.</param>
/// <param name="size">Filled in with the number of bytes in the block returned.</param>
/// <returns>Returns with a pointer to the newly allocated paletted block, which the caller
/// takes ownership of. If there is no preview to convert, then NULL is returned.</returns>
unsigned * MapPreviewClass::Create_Paletted_Preview(int colorcount, int & size)
{
	int i;
	int k;
	int x;
	int y;

	if (SurfacePtr == NULL) {
		return(NULL);
	}

	DSurface surf(SurfacePtr->Get_Width() / 2, SurfacePtr->Get_Height() / 2);
	surf.Blit_From(surf.Get_Rect(), *SurfacePtr, SurfacePtr->Get_Rect(), false, false);

	int *counts = new int[4096];
	memset(counts, 0, 4096 * sizeof(int));
	int colorsused = 0;

	surf.Lock();
	for (y = 0; y < surf.Get_Height(); y++) {
		for (x = 0; x < surf.Get_Width(); x++) {
			int pixel = surf.Get_Pixel(Point2D(x,y));
			RGBClass color = DSurface::Deconstruct_Hicolor_Pixel(pixel);
			int red = color.Get_Red() & 0xF0;
			int green = color.Get_Green() & 0xF0;
			int blue = color.Get_Blue() & 0xF0;
			surf.Put_Pixel(Point2D(x,y), DSurface::Build_Hicolor_Pixel(red, green, blue));
			int colorindex = (red << 4) | green | (blue >> 4);
			int count = counts[colorindex];
			if (count == 0) {
				colorsused++;
			}
			counts[colorindex] = count + 1;
		}
	}
	surf.Unlock();

	unsigned short *palette = new unsigned short[colorcount];
	for (i = 0; i < colorcount; i++) {
		int bestcolor = -1;
		int bestcount = 0;
		for (k = 0; k < 4096; k++) {
			if (counts[k] > bestcount) {
				bestcount = counts[k];
				bestcolor = k;
			}
		}
		if (bestcolor == -1) {
			break;
		}
		counts[bestcolor] = 0;
		palette[i] = bestcolor;
	}

	unsigned char *indexbuf = new unsigned char[surf.Get_Width() * surf.Get_Height()];
	unsigned char *indexptr = indexbuf;

	surf.Lock();
	for (y = 0; y < surf.Get_Height(); y++) {
		for (x = 0; x < surf.Get_Width(); x++) {
			int pixel = surf.Get_Pixel(Point2D(x,y));
			RGBClass color = DSurface::Deconstruct_Hicolor_Pixel(pixel);

			int bestindex = -1;
			int bestdist = 0xFFFF;
			for (k = 0; k < colorcount; k++) {
				unsigned short entry = palette[k];
				int dist = abs(((entry >> 4) & 0xF0) - color.Get_Red()) + abs((16 * (entry & 0x000F)) - color.Get_Blue()) + abs((entry & 0x00F0) - color.Get_Green());
				if (dist == 0) {
					bestindex = k;
					break;
				}
				if (dist < bestdist) {
					bestdist = dist;
					bestindex = k;
				}
			}
			*indexptr++ = (unsigned char)bestindex;
		}
	}
	surf.Unlock();

	int offset = (colorcount * sizeof(unsigned short)) + sizeof(Header) + sizeof(int);
	int bytes = offset + surf.Get_Width() * surf.Get_Height();
	Header *block = (Header *)new unsigned char[bytes];
	block->Width = surf.Get_Width();
	block->Height = surf.Get_Height();
	*(int *)(block + 1) = colorcount;
	memcpy((unsigned char *)block + sizeof(Header) + sizeof(int), palette, colorcount << 1);
	memcpy(((unsigned char *)block) + offset, indexbuf, surf.Get_Width() * surf.Get_Height());
	delete [] palette;
	delete [] indexbuf;
	delete [] counts;
	size = bytes;
	DebugString("Created paletted preview of size %d bytes\n", bytes);

	return((unsigned *)block);
}


/// <summary>
/// Creates the preview surface from a paletted preview block.
/// This routine is the counterpart of Create_Paletted_Preview. Use it when the preview
/// arrives already packed, rather than being rendered from the map that is loaded.
/// </summary>
/// <param name="buffer">Pointer to the paletted preview block to expand.</param>
void MapPreviewClass::Create_Preview_Surface(char * buffer)
{
	int * header = (int *)buffer;

	int width = *header++;
	int height = *header++;
	int colorcount = *header;
	unsigned short *palette = (unsigned short *)header;

	if (SurfacePtr != NULL) {
		delete SurfacePtr;
	}
	SurfacePtr = new DSurface(width, height);
	SurfacePtr->Fill(TBLACK);

	int offset = (colorcount * sizeof(unsigned short)) + sizeof(Header) + sizeof(int);
	unsigned char * indexptr = (unsigned char *)buffer + offset;
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			unsigned short entry = palette[*indexptr++ + 2];
			int color = DSurface::Build_Hicolor_Pixel((entry >> 4) & 0x00F0, entry & 0x00F0, 16 * (entry & 0x000F));

			SurfacePtr->Put_Pixel_Clip(Point2D(x, y), color, SurfacePtr->Get_Rect());
		}
	}
}
