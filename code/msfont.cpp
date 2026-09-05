/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "msfont.h"

#include "_mixfile.h"
#include "_palette.h"
#include "_surface.h"
#include "ccfile.h"
#include "convert.h"
#include "data.h"
#include "dbgprint.h"
#include "draw.h"
#include "audio/audioengine.h"
#include "dsurface.h"
#include "globals.h"
#include "goptions.h"
#include "mixfile.h"
#include "palette.h"
#include "shapeset.h"

#include <cstring>


static int InstanceCount = 0;

static struct {
	void * Sample;
	bool AllocLoaded;
} Sounds[3];


/// <summary>
/// Creates the standard menu font.
/// This routine loads the common font used throughout the menu screens, colored either to
/// suit the side being played or with the neutral palette.
/// </summary>
/// <param name="use_side_palette">Should the font take its colors from the side palette?</param>
MSFont::MSFont(bool use_side_palette) :
	Red(0),
	Green(0),
	Blue(0)
{
	Init("FULLFNT3.SHP", use_side_palette ? "SIDEFNT3.PAL" : "FULLFNT3.PAL");
	InstanceCount++;
}


/// <summary>
/// Creates a font from a shape file.
/// The palette is presumed to sit alongside the shapes under the same name, so only the
/// shape file has to be named.
/// </summary>
/// <param name="file_name">The name of the font shape file to load.</param>
MSFont::MSFont(char const * file_name) :
	Red(0),
	Green(0),
	Blue(0)
{
	char palette_name[256];
	strncpy(palette_name, file_name, 256);
	strtok(palette_name, ".");
	strncat(palette_name, ".PAL", 256);

	Init(file_name, palette_name);
	InstanceCount++;
}


/// <summary>
/// Creates a font from a shape file and a palette.
/// Use this routine when the colors for a font do not live in a palette named after the
/// shape file itself.
/// </summary>
MSFont::MSFont(char const * file_name, char const * palette_name) :
	Red(0),
	Green(0),
	Blue(0)
{
	Init(file_name, palette_name);
	InstanceCount++;
}


/// <summary>
/// Destroys the font.
/// The glyph data and the typing sound effects are shared by every font in play, so they
/// are only let go once the last font has been destroyed.
/// </summary>
MSFont::~MSFont(void)
{
	InstanceCount--;
	if (InstanceCount == 0) {
		for (int i = 0; i < ARRAY_SIZE(Sounds); i++) {
			if (Sounds[i].Sample != NULL) {
				AudioEngine.Stop_Sample_Playing(Sounds[i].Sample);
				AudioEngine.Release_Sample(Sounds[i].Sample);
				if (Sounds[i].AllocLoaded == true) {
					delete Sounds[i].Sample;
				}
				Sounds[i].Sample = NULL;
			}
		}

		if (Drawer != NULL) {
			delete Drawer;
		}

		if (FontFile != NULL) {
			if (AllocLoaded == true) {
				delete FontFile;
			}
		}

		FontFile = NULL;
		AllocLoaded = false;
	}
}


/// <summary>
/// Loads the font shapes and the palette that colors them.
/// This is the routine every constructor leans on. It fetches the glyph shapes from the
/// mixfiles or from disk, builds the drawer that the glyphs are remapped through, and sees
/// to it that the typing sound effects are on hand.
/// </summary>
/// <param name="file_name">The name of the font shape file to load.</param>
/// <param name="palette_name">The name of the palette to color the glyphs with.</param>
/// <returns>bool; Was the font made ready for use?</returns>
/// <remarks>The global CCPalette is overwritten with this font's palette.</remarks>
bool MSFont::Init(char const * file_name, char const * palette_name)
{
	CCFileClass file;

	AllocLoaded = false;
	FontFile = (ShapeSet *)MFCD::Retrieve(file_name);

	if (FontFile == NULL) {
		file.Set_Name(file_name);
		FontFile = (ShapeSet *)Load_Alloc_Data(file);
		AllocLoaded = true;
		DebugString("MSFont: AllocLoaded FULLFNT3.SHP\n");
	}

	if (FontFile == NULL) {
		return(false);
	}

	file.Set_Name(palette_name);
	PaletteClass * palette = (PaletteClass *)Load_Alloc_Data(file);

	if (palette == NULL) {
		return(false);
	}

	for (int gindex = 0; gindex < 256; gindex++) {
		CCPalette[gindex] = RGBClass(
			((unsigned char *)*palette)[gindex*3]<<2,
			((unsigned char *)*palette)[gindex*3+1]<<2,
			((unsigned char *)*palette)[gindex*3+2]<<2);
	}

	unsigned char red = (*palette)[68].Get_Red()<<2;
	unsigned char blue = (*palette)[68].Get_Blue()<<2;
	unsigned char green = (*palette)[68].Get_Green()<<2;

	Red = red;
	Blue = blue;
	Green = green;

	Color = DSurface::Build_Hicolor_Pixel((*palette)[68].Get_Red()<<2, (*palette)[68].Get_Green()<<2, (*palette)[68].Get_Blue()<<2);

	delete palette;

	Drawer = new ConvertClass(CCPalette, CCPalette, *VisibleSurface);

	if (Drawer == NULL) {
		return(false);
	}

	FontWidth = FontFile->Get_Width();
	FontHeight = FontFile->Get_Height();

	if (InstanceCount == 0 && AudioEngine.Is_Available()) {
		Sounds[0].AllocLoaded = false;
		Sounds[0].Sample = (void *)MixFileClass::Retrieve("TEXT1.AUD");

		if (Sounds[0].Sample == NULL) {
			file.Open("TEXT1.AUD");
			Sounds[0].Sample = Load_Alloc_Data(file);
			Sounds[0].AllocLoaded = true;
			DebugString("MSFont: AllocLoaded TEXT1.AUD\n");
		}

		Sounds[1].AllocLoaded = false;
		Sounds[1].Sample = (void *)MixFileClass::Retrieve("TEXT2.AUD");

		if (Sounds[1].Sample == NULL) {
			file.Open("TEXT2.AUD");
			Sounds[1].Sample = Load_Alloc_Data(file);
			Sounds[1].AllocLoaded = true;
			DebugString("MSFont: AllocLoaded TEXT2.AUD\n");
		}

		Sounds[2].AllocLoaded = false;
		Sounds[2].Sample = (void *)MixFileClass::Retrieve("TEXT3.AUD");

		if (Sounds[2].Sample == NULL) {
			file.Open("TEXT3.AUD");
			Sounds[2].Sample = Load_Alloc_Data(file);
			Sounds[2].AllocLoaded = true;
			DebugString("MSFont: AllocLoaded TEXT3.AUD\n");
		}
	}

	return(true);
}


/// <summary>
/// Fetches the area a string will take up when printed.
/// Use this routine to center or otherwise place a block of text before drawing it.
/// Newlines are honored, so the rectangle spans every line of the string.
/// </summary>
/// <param name="rect">The rectangle to fill in. It is anchored at the origin, since it
/// describes the size of the text rather than where it will land.</param>
void MSFont::Get_String_Rect(char const * string, Rect & rect)
{
	int max_width = 0;

	if (string != NULL && strlen(string) != 0) {

		int height = FontHeight;

		do {
			int width = 0;
			while (*string && *string != '\n') {
				width += Get_Character_Width(*string++);
			}

			if (width > max_width) {
				max_width = width;
			}

			if (*string == '\n') {
				string++;
				height += FontHeight;
			}

		} while (*string);

		rect.Set(0, 0, max_width + FontWidth, height);
		return;
	}

	rect.Set(0, 0, 0, 0);
}


/// <summary>
/// Fetches the printed width of a string.
/// Newlines are honored, so a multiple line string reports the width of its longest line.
/// </summary>
/// <returns>Returns with the width in pixels.</returns>
int MSFont::Get_String_Width(char const * string)
{
	int max_width = 0;

	if (string != NULL) {
		do {
			int width = 0;
			while (*string && *string != '\n') {
				width += Get_Character_Width(*string++);
			}

			if (*string == '\n') {
				string++;
			}

			if (width > max_width) {
				max_width = width;
			}

		} while (*string);
	}

	return(max_width);
}


/// <summary>
/// Fetches the printed width of a single character.
/// This is what the measuring and printing routines step along a string with, so the width
/// reported is an advance -- the gap that follows the glyph is included in it.
/// </summary>
/// <returns>Returns with the width in pixels. Characters below the space are worth
/// nothing.</returns>
int MSFont::Get_Character_Width(unsigned char character)
{
	CharToOemBuff((LPCSTR)&character, (LPSTR)&character, 1);

	if (character == ' ') {
		return(8);
	}

	if (character > ' ') {
		int index = (0 >= character - 33) ? 0 : character - 33;
		index = (216 <= index) ? 216 : index;

		int shape_frame = index * 3;
		return(FontFile->Get_Rect(shape_frame + 2).Width + 1);
	}
	return(0);
}


/// <summary>
/// Draws a single character onto the surface.
/// This routine is used by the menu print animations, which walk a string one glyph at a
/// time so that the text appears to be typed out. Anything below the space character has
/// no glyph and is passed over.
/// </summary>
/// <param name="frame">The glyph frame to draw with. The print animations step through the
/// frames to fade a character in.</param>
/// <param name="do_sound">Should a typing sound accompany the character?</param>
void MSFont::Draw_Character(Surface * surface, unsigned char character, int x, int y, int frame, bool do_sound)
{
	if (character == 176) {
		DebugString("Denzil!\n");
	}

	CharToOemBuff((LPCSTR)&character, (LPSTR)&character, 1);

	if (character > ' ') {

		int index = (0 >= character - 33) ? 0 : character - 33;
		index = (216 <= index) ? 216 : index;

		int shape_frame = index * 3;

		if (do_sound == true && frame == 0) {
			void * sample = Sounds[rand() % 3].Sample;
			if (sample != NULL) {
				AudioEngine.Play_Sample(sample, AUDIO_GROUP_SFX, 64.0f / 255.0f, 10);
			}
		}

		Draw_Shape(*surface, *Drawer, FontFile, shape_frame + frame, Point2D(x - FontFile->Get_Rect(shape_frame + 2).X, y), surface->Get_Rect(), SHAPE_WIN_REL);
	}
}


/// <summary>
/// Draws a string onto the surface.
/// A newline in the string starts a fresh line back at the left edge it was handed. Unlike
/// the single character routine, this one prints silently.
/// </summary>
/// <param name="x">The left edge to begin each line of the string at.</param>
/// <param name="frame">The glyph frame to draw the text with.</param>
void MSFont::Draw_String(Surface * surface, unsigned char const * string, int x, int y, int frame)
{
	int current_x = x;

	while (*string) {
		if (*string == '\n') {
			current_x = x;
			y += FontHeight;
		} else {
			if (*string > ' ') {
				unsigned char character;
				CharToOemBuff((LPCSTR)string, (LPSTR)&character, 1);

				int index = (0 >= character - 33) ? 0 : character - 33;
				index = (216 <= index) ? 216 : index;

				int shape_frame = index * 3;

				Draw_Shape(*surface, *Drawer, FontFile, shape_frame + frame, Point2D(current_x - FontFile->Get_Rect(shape_frame + 2).X, y), surface->Get_Rect(), SHAPE_WIN_REL);
			}

			current_x += Get_Character_Width(*string);
		}

		string++;
	}
}
