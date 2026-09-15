/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// What outlives the dialog layer: the remapped bitmap-font drawer the briefing and the
// restate screen draw with, the GDI text the credits put on a surface, the blend masks,
// the hotkey spelling and the mouse capture the graphic menus take.

#pragma once

#include "keyboard.h"
#include "surface.h"
#include "win.h"


namespace OwnerDraw {

	/*
	 * Measurements of one of the remap fonts, cached by ODGetFontMetrics.
	 */
	struct FontMetrics {
		int charWidths[256];	/// inked width of each character, indexed by character code
		int glyphWidth;			/// width of the inked part of a glyph cell
		int glyphHeight;		/// height of the inked part of a glyph cell
		int topMargin;			/// blank rows above each row of glyphs
		int leftMargin;			/// blank columns before each glyph
	};

	void Initialize(void);
	void Prepare_Resources(void);

	// Release the operating system's pointer while a menu drives the mouse itself, and take it
	// back when the last caller is done. Each capture must be matched by a release.
	int Capture_Mouse(void);
	int Release_Mouse(void);
};


#define OD_TEXT_ALIGN_MIN 1
#define OD_TEXT_ALIGN_CENTER 2
#define OD_TEXT_ALIGN_MAX 3

int OD_Draw_Text_Remap(Surface & surface, const char * string, Rect const & rect, char const * name, COLORREF color, int flags, int char_spacing);
int OD_Draw_Text(COLORREF color, HFONT font, Rect const & rect, const char * text, int len, int x_alignment, int y_alignment, Surface * surface);
HFONT WS_Get_Font(HDC hdc, const char * face_name, int decipt_width, int decipt_height, int attributes);

int Build_Hotkey_String(KeyNumType key, char * buffer);

extern COLORREF ODColorText;
extern COLORREF ODColorTextDim;
extern COLORREF ODColorDisabled;
extern COLORREF ODColorFrame;
extern COLORREF ODListBoxColor;
extern COLORREF ODColorUnused1;


/// Flags for ODDrawCharRemap.
#define OD_DRAW_CHAR_FLAG_HORIZONTAL_CENTER 1
#define OD_DRAW_CHAR_ALIGN_FLAG_RIGHT 2
#define OD_DRAW_CHAR_FLAG_VERTICAL_CENTER 4


extern unsigned short ODRComponentMask;
extern unsigned short ODGComponentMask;
extern unsigned short ODBComponentMask;


/*
 * A straight alpha blend of two frame pixels. DSurface::Blend_Pixel steps its alpha in
 * quarters, so it is not a substitute.
 */
inline unsigned short OD_Blend_Color(unsigned short pixel, unsigned short color, unsigned char alpha)
{
	static unsigned blend_color_alpha;
	static unsigned blend_pixel_alpha;

	blend_color_alpha = alpha;
	blend_pixel_alpha = 255 - alpha;

	unsigned short r = ((((pixel & ODRComponentMask) * blend_pixel_alpha) + ((color & ODRComponentMask) * blend_color_alpha)) >> 8) & ODRComponentMask;
	unsigned short g = ((((pixel & ODGComponentMask) * blend_pixel_alpha) + ((color & ODGComponentMask) * blend_color_alpha)) >> 8) & ODGComponentMask;
	unsigned short b = (((pixel & ODBComponentMask) * blend_pixel_alpha) + ((color & ODBComponentMask) * blend_color_alpha)) >> 8;
	return((unsigned short)(r | g | b));
}
