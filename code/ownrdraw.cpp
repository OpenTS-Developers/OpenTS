/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// What outlives the dialog layer: the remapped bitmap-font drawer the briefing and the
// restate screen draw with, the GDI text the credits put on a surface, the surface cache,
// the blend masks, the hotkey spelling and the mouse capture the graphic menus take.

#include "always.h"

#include "ownrdraw.h"

#include "_surface.h"
#include "_xmouse.h"
#include "bsurface.h"
#include "arraylist.h"
#include "dbgprint.h"
#include "dict.h"
#include "dsurface.h"
#include "globals.h"
#include "hsv.h"
#include "keyboard.h"
#include "misc.h"
#include "mixfile.h"
#include "rgb.h"
#include "srfcache.h"
#include "surface.h"
#include "utf8.h"
#include "video.h"

#include <cmath>
#include <cstring>


using namespace OwnerDraw;

extern unsigned int Wstring_Hash(Wstring & string);


static int _mouse_counter;

COLORREF ODColorText;
COLORREF ODColorTextDim;
COLORREF ODColorDisabled;
COLORREF ODColorFrame;
COLORREF ODListBoxColor;
COLORREF ODColorUnused1;

SurfaceCacheClass SurfaceCache;

unsigned short ODRComponentMask;
unsigned short ODGComponentMask;
unsigned short ODBComponentMask;

static int ODColorToHiColor(COLORREF color);
float ODCalcTextRemapFactor(int hue);
bool ODGetFontMetrics(char const * font_name, FontMetrics * metrics);
void ODDrawCharRemap(Surface & dst_surf, const char *text, int max_chars, Rect const & rect, char const *font_name, COLORREF color, char flags, int char_spacing);


/// <summary>
/// Converts a Windows color reference into a display pixel.
/// The dialog colors are all written as RGB() values, so they have to be packed into the
/// pixel layout of the display surface before anything can be drawn with them.
/// </summary>
/// <returns>Returns with the packed pixel value. An all-ones color is passed through
/// unchanged.</returns>
static int ODColorToHiColor(COLORREF color)
{
	if (color == 0xFFFFFFFF) {
		return(0xFFFFFFFF);
	}

	union {
		struct {
			unsigned int red : 8;
			unsigned int green : 8;
			unsigned int blue : 8;
			unsigned int a : 8;
		};
		int v;
	} c;

	c.v = color;

	return(DSurface::Build_Hicolor_Pixel(c.red, c.green, c.blue));
}


// The dialog fonts are 256-cell sheets in Windows-1252 order; a code point without a cell
// draws as '?'.
static unsigned char OD_Glyph(char32_t code)
{
	if (code < ' ') {
		return((unsigned char)code);
	}
	int index = UTF8::Windows_1252_Glyph(code);
	return((unsigned char)(index < 0 ? '?' : index));
}


/// <summary>
/// Sets the family of colors the drawers paint with.
/// </summary>
void OwnerDraw::Initialize(void)
{
	ODColorText = RGB(112,255,0);
	ODColorTextDim = RGB(16,144,16);
	ODColorDisabled = RGB(144,144,144);
	ODColorFrame = RGB(78, 182, 220);
	ODListBoxColor = RGB(34,80,97);
	ODColorUnused1 = RGB(22, 55, 68);
}


/// <summary>
/// Sets up the color component masks used for blending.
/// The masks depend on how the display surface packs its pixels, so this routine cannot
/// run until the video mode is known.
/// </summary>
void ODInitMasks(void)
{
	ODRComponentMask = 255;
	ODRComponentMask = ODRComponentMask >> DSurface::Get_Red_Left();
	ODRComponentMask <<= DSurface::Get_Red_Right();

	ODGComponentMask = 255;
	ODGComponentMask = ODGComponentMask >> DSurface::Get_Green_Left();
	ODGComponentMask <<= DSurface::Get_Green_Right();

	ODBComponentMask = 255;
	ODBComponentMask = ODBComponentMask >> DSurface::Get_Blue_Left();
	ODBComponentMask <<= DSurface::Get_Blue_Right();
}


/// <summary>
/// Loads the art the surviving drawers read.
/// The cache is a plain lookup: a name it was never given reads back as no surface at all,
/// so everything drawn from it has to be put there up front.
/// </summary>
static void ODCacheImages(void)
{
	// The remap font the briefing and the score animations draw with.
	SurfaceCache.CachePCX("dlgsysi.pcx", 1);
	SurfaceCache.CachePalettedPCX("dlgsysa.pcx");

	// The button pieces the mission restate screen builds its buttons out of.
	SurfaceCache.CachePCX("bue_li24.pcx");
	SurfaceCache.CachePCX("bue_mi24.pcx");
	SurfaceCache.CachePCX("bue_ri24.pcx");
	SurfaceCache.CachePCX("bde_li24.pcx");
	SurfaceCache.CachePCX("bde_mi24.pcx");
	SurfaceCache.CachePCX("bde_ri24.pcx");
	SurfaceCache.CachePCX("bue_li30.pcx");
	SurfaceCache.CachePCX("bue_mi30.pcx");
	SurfaceCache.CachePCX("bue_ri30.pcx");
	SurfaceCache.CachePCX("bde_li30.pcx");
	SurfaceCache.CachePCX("bde_mi30.pcx");
	SurfaceCache.CachePCX("bde_ri30.pcx");
}


/// <summary>
/// Sets the colors up, builds the blend masks and loads the art. Later calls do nothing, so
/// anything drawing with them may ask for them.
/// </summary>
void OwnerDraw::Prepare_Resources(void)
{
	Initialize();

	static bool _inited = false;
	if (!_inited) {
		ODInitMasks();
		ODCacheImages();
		_inited = true;
	}
}


/// <summary>
/// Converts a key code into its printable name.
/// This routine is used by the hotkey control to show a binding the way the player's
/// keyboard layout names it, with the modifier names spelled out ahead of the key.
/// </summary>
/// <param name="key">The key, complete with its modifier bits, to spell out.</param>
/// <param name="buffer">Buffer to build the name in.</param>
/// <remarks>Be sure that the buffer is big enough for the modifier names as well.</remarks>
int Build_Hotkey_String(KeyNumType key, char * buffer)
{
	char key_name[32];
	unsigned char modifier = HIBYTE(key);

	buffer[0] = '\0';

	UINT lparam;

	/// (p << 16) - places the scan code into bits 16-23.
	/// (1 <<  0) - purpose unknown; Windows does not document this bit.
	/// (1 << 24) - Extended-key bit. Distinguishes some keys on an enhanced keyboard.
	/// (1 << 25) - "Don't care" bit. Should not distinguish between left and right ctrl and shift keys.

	if ((modifier & (WWKEY_ALT_BIT >> 8)) != 0) {
		lparam = MapVirtualKey(VK_MENU, 0) ;
		lparam = (lparam << 16);
		lparam |= (1 << 0);
		lparam |= (1 << 25);
		GetKeyNameText(lparam, key_name, sizeof(key_name));
		strcat(buffer, key_name);
		strcat(buffer, "+");
	}

	if ((modifier & (WWKEY_CTRL_BIT >> 8)) != 0) {
		lparam = MapVirtualKey(VK_CONTROL, 0);
		lparam = (lparam << 16);
		lparam |= (1 << 0);
		lparam |= (1 << 25);
		GetKeyNameText(lparam, key_name, sizeof(key_name));
		strcat(buffer, key_name);
		strcat(buffer, "+");
	}

	if ((modifier & (WWKEY_SHIFT_BIT >> 8)) != 0) {
		lparam = MapVirtualKey(VK_SHIFT, 0);
		lparam = (lparam << 16);
		lparam |= (1 << 0);
		lparam |= (1 << 25);
		GetKeyNameText(lparam, key_name, sizeof(key_name));
		strcat(buffer, key_name);
		strcat(buffer, "+");
	}

	lparam = MapVirtualKey(key & 0xFF, 0);
	lparam = (lparam << 16);
	lparam |= (1 << 0);
	lparam |= (1 << 25);

	if ((modifier & (WWKEY_RLS_BIT >> 8)) != 0) {
		lparam |= (1 << 24);
	}

	GetKeyNameText(lparam, key_name, sizeof(key_name));
	strcat(buffer, key_name);

	return(0);
}


/// <summary>
/// Draws word wrapped text with a remapped bitmap font.
/// This routine breaks the text into lines that will fit the rectangle -- honoring the
/// newlines already in it and breaking at a space wherever one can be found -- and hands
/// each line in turn to ODDrawCharRemap.
/// </summary>
/// <param name="name">The base name of the font sheets to draw with.</param>
/// <param name="flags">The OD_DRAW_CHAR alignment flags to lay each line out with.</param>
/// <param name="char_spacing">The extra spacing to insert between characters.</param>
int OD_Draw_Text_Remap(Surface & surface, const char * text, Rect const & rect, char const * name, COLORREF color, int flags, int char_spacing)
{
	int line_len = strlen(text);
	char const * line_ptr = text;
	Rect draw_rect = rect;

	FontMetrics data;
	if (!ODGetFontMetrics(name, &data)) {
		return(0);
	}

	while (line_len) {
		if (line_ptr) {
			char const * nl_ptr = strchr(line_ptr, '\n');
			if (nl_ptr) {
				int nl_len = (int)(nl_ptr - line_ptr) + 1;
				if (line_len >= nl_len) {
					line_len = nl_len;
				}
			}
		}

		if ((unsigned char)*line_ptr <= ' ') {
			++line_ptr;
			if (--line_len == 0) {
				return(0);
			}
		}

		int text_width = 0;
		for (char const * cursor = text; cursor - text < line_len; ) {
			text_width += char_spacing + data.charWidths[OD_Glyph(UTF8::Decode(cursor))];
		}

		if (text_width > draw_rect.Width - draw_rect.X) {
			int fallback = (int)UTF8::Boundary_Before(line_ptr, line_len - 1);
			int cut = line_len - 1;

			flags &= ~4;

			while (cut > 0) {
				if ((unsigned char)line_ptr[cut] <= ' ') {
					break;
				}
				--cut;
			}
			if (cut > 0) {
				line_len = cut;
				if (cut != -1) {
					continue;
				}
			}

			line_len = fallback;
		} else {
			ODDrawCharRemap(surface, line_ptr, line_len, draw_rect, name, color, (char)flags, char_spacing);
			line_ptr += line_len;
			draw_rect.Y += data.glyphHeight;
			line_len = strlen(line_ptr);
		}
	}

	return(0);
}


/// <summary>
/// Determines how strongly a hue should be remapped.
/// The font remapper uses this to pull its hue shift back around the primary colors, so
/// that text tinted near one of them does not swing away from the color asked for.
/// </summary>
/// <param name="hue">The hue to compute the factor for.</param>
/// <returns>Returns with the scale factor; the nearer the hue sits to a primary, the smaller
/// it gets.</returns>
float ODCalcTextRemapFactor(int hue)
{
	float val = 1.0f;

	int arr[3];
	arr[0] = 43;
	arr[1] = 128;
	arr[2] = 213;

	for (int i = 0; i < 3; i++) {
		int value = arr[i];

		if (hue > value - 16 && hue <= value) {
			val = float(value - hue);
			val *= (1.0f / 16);
			val *= (60.0f / 100);
			val += (40.0f / 100);
		} else if (hue > value && hue <= value + 16) {
			val = float(hue - value);
			val *= (1.0f / 16);
			val *= (60.0f / 100);
			val += (40.0f / 100);
		}
	}
	return(val);
}


/// <summary>
/// Draws a line of text with a remapped bitmap font.
/// This routine builds a table that shifts the font's own palette toward the color asked
/// for and then alpha blends each character onto the destination surface. It is the low
/// level draw that all of the owner-draw remapped text ends up going through.
/// </summary>
/// <param name="max_chars">The maximum number of characters of the text to draw.</param>
/// <param name="rect">The rectangle to align the text within.</param>
/// <param name="font_name">The base name of the font sheets to draw with.</param>
/// <param name="flags">The OD_DRAW_CHAR alignment flags to lay the text out with.</param>
/// <param name="char_spacing">The extra spacing to insert between characters.</param>
void ODDrawCharRemap(Surface & dst_surf, const char *text, int max_chars, Rect const & rect, char const *font_name, COLORREF color, char flags, int char_spacing)
{
	int i;
	Rect draw_rect = rect;

	char name_i[64];
	strcpy(name_i, font_name);
	strcat(name_i, "i.pcx");

	char palette[768];
	Surface *sheet_i = SurfaceCache.GetSurface(name_i, palette);
	if (sheet_i == NULL) {
		return;
	}

	char name_a[64];
	strcpy(name_a, font_name);
	strcat(name_a, "a.pcx");

	Surface *sheet_a = SurfaceCache.GetSurface(name_a, NULL);
	if (sheet_a == NULL) {
		return;
	}

	RGBClass remap_rgb((unsigned char)color, (unsigned char)(color >> 8), (unsigned char)(color >> 16));
	HSVClass remap_hsv = remap_rgb;
	RGBClass pal_rgb;
	HSVClass out_hsv;

	int hue = remap_hsv.Get_Hue();

	int end = int(hue + 15.0);
	float min_factor = 1.0f;
	for (i = int(hue - 15.0); i <= end; ++i) {
		float factor = ODCalcTextRemapFactor(i);
		if (factor < min_factor) {
			min_factor = factor;
		}
	}

	unsigned char sat = (unsigned char)remap_hsv.Get_Saturation();
	unsigned char val = (unsigned char)remap_hsv.Get_Value();

	unsigned short remap_table[256];
	float hue_float = (float)hue;
	unsigned char *pal = (unsigned char *)&palette;
	for (i = 0; i < 256; ++i) {
		pal_rgb.Set_Red(pal[0]);
		pal_rgb.Set_Green(pal[1]);
		pal_rgb.Set_Blue(pal[2]);
		HSVClass pal_hsv = pal_rgb;

		/*
		 * Start from the palette entry's HSV and adjust each channel. The
		 * wholesale copy is fully overwritten below.
		 */
		out_hsv = pal_hsv;
		out_hsv.Set_Hue((unsigned char)(int)(hue_float - (int)(68.0f - pal_hsv.Get_Hue()) * min_factor));
		out_hsv.Set_Saturation((unsigned char)((sat * out_hsv.Get_Saturation()) >> 8));
		out_hsv.Set_Value((unsigned char)((val * out_hsv.Get_Value()) >> 8));

		RGBClass out_rgb = out_hsv;
		pal_rgb = out_rgb;

		int packed = (((out_rgb.Get_Blue() << 8) | out_rgb.Get_Green()) << 8) | out_rgb.Get_Red();
		remap_table[i] = (unsigned short)ODColorToHiColor(packed);
		pal += 3;
	}

	FontMetrics font_data;
	if (!ODGetFontMetrics(font_name, &font_data)) {
		return;
	}

	if ((int)strlen(text) < max_chars) {
		max_chars = strlen(text);
	}

	int total_width = 0;
	for (char const * cursor = text; cursor - text < max_chars; ) {
		total_width += font_data.charWidths[OD_Glyph(UTF8::Decode(cursor))] + char_spacing;
	}

	if ((flags & OD_DRAW_CHAR_FLAG_HORIZONTAL_CENTER) != 0) {
		draw_rect.X += (draw_rect.Width - draw_rect.X - total_width) / 2;
	} else if ((flags & OD_DRAW_CHAR_ALIGN_FLAG_RIGHT) != 0) {
		draw_rect.X = draw_rect.Width - total_width - 1;
	}

	if ((flags & OD_DRAW_CHAR_FLAG_VERTICAL_CENTER) != 0) {
		draw_rect.Y = draw_rect.Y + (draw_rect.Height - font_data.glyphHeight - draw_rect.Y) / 2;
	}

	draw_rect.Y -= font_data.topMargin;
	--draw_rect.X;

	unsigned char *src_i = (unsigned char *)sheet_i->Lock();
	unsigned char *src_a = (unsigned char *)sheet_a->Lock();
	unsigned char *dst = (unsigned char *)dst_surf.Lock();

	if (src_i != NULL && src_a != NULL && dst != NULL) {
		int cell_w = font_data.glyphWidth + font_data.leftMargin;
		int cell_h = font_data.glyphHeight + font_data.topMargin;
		int chars_per_row = sheet_i->Get_Width() / (font_data.glyphWidth + font_data.leftMargin);
		int dst_stride = dst_surf.Stride() / 2;
		int src_stride = sheet_i->Stride();

		int x = draw_rect.X;
		for (char const * cursor = text; cursor - text < max_chars; ) {

			unsigned char index = OD_Glyph(UTF8::Decode(cursor));
			if (index <= ' ') {
				x += font_data.charWidths[index] + char_spacing;
			} else {
				int glyph = index + 1;
				int src_x = (glyph % chars_per_row) * cell_w;
				int src_y = (glyph / chars_per_row) * cell_h;

				int src_y_end = src_y + cell_h;
				int src_delta = src_i - src_a;
				unsigned char *alpha_col = src_a + (src_y * src_stride + src_x);
				unsigned char *dst_col = dst + 2 * (dst_stride * draw_rect.Y + x);

				for (int sx = src_x; sx < src_x + cell_w; ++sx) {
					if (src_y < src_y_end) {
						unsigned short *dst_px = (unsigned short *)dst_col;
						unsigned char *alpha_px = alpha_col;

						int sy = src_y_end - src_y;
						do {
							unsigned char alpha = *alpha_px;
							if (alpha != 0) {
								unsigned char index = alpha_px[src_delta];
								*dst_px = OD_Blend_Color(*dst_px, remap_table[index], alpha);
							}

							dst_px += dst_stride;
							alpha_px += src_stride;
							--sy;
						} while (sy != 0);
					}

					++alpha_col;
					dst_col += 2;
				}

				x += font_data.charWidths[index] + char_spacing;
			}
		}
	}

	if (&dst_surf != NULL) {
		dst_surf.Unlock();
	}
	sheet_a->Unlock();
	sheet_i->Unlock();
}


/// <summary>
/// Fetches the metrics of a remappable bitmap font.
/// This routine measures the font's sheet -- the margins, the size of a character cell and
/// the inked width of every character -- so that the remap text routines know how to lay
/// characters out. Measuring is expensive, so the result is kept by font name.
/// </summary>
/// <param name="font_name">The base name of the font, without the sheet suffix.</param>
/// <param name="metrics">Buffer to fill in with the measurements.</param>
/// <returns>bool; Were the metrics available?</returns>
bool ODGetFontMetrics(char const * font_name, FontMetrics * metrics)
{
	static Dictionary<Wstring, FontMetrics> metricsDict(Wstring_Hash);

	char buf[64];
	strcpy(buf, font_name);
	strcat(buf, "a.pcx");

	Wstring name;
	name = (char *)font_name;
	name.toLower();

	FontMetrics * found = NULL;
	if (metricsDict.getPointer(name, &found)) {
		if (metrics != NULL) {
			*metrics = *found;
			return(true);
		}
	}

	DebugString("TS: Computing font metrics....\n");

	FontMetrics temp;
	memset(&temp, 0, sizeof(temp));

	char palette[768];
	Surface * surf = SurfaceCache.GetSurface(buf, palette);
	if (surf == NULL) {
		return(false);
	}

	char * basePtr = (char *)surf->Lock();
	int stride = surf->Stride();

	/*
	 * ----------------------------------------------------------------
	 * Vertical metrics: topMargin = blank rows above the glyph row,
	 * glyphHeight = inked rows (probed at column 4).
	 * ----------------------------------------------------------------
	 */
	temp.topMargin = 0;
	while (temp.topMargin < surf->Get_Height()) {
		if (basePtr[stride * temp.topMargin + 4] != 0) break;
		++temp.topMargin;
	}
	int y = temp.topMargin;
	while (y < surf->Get_Height()) {
		if (basePtr[stride * y + 4] == 0) break;
		++y;
		++temp.glyphHeight;
	}

	/*
	 * ----------------------------------------------------------------
	 * Horizontal metrics: leftMargin = blank columns before the glyphs,
	 * glyphWidth = inked columns (probed along row 'top').
	 * ----------------------------------------------------------------
	 */
	temp.leftMargin = 0;
	while (temp.leftMargin < surf->Get_Width()) {
		if (basePtr[stride * temp.topMargin + temp.leftMargin] != 0) break;
		++temp.leftMargin;
	}
	int left = temp.leftMargin;

	int x;
	x = left;
	while (x < surf->Get_Width()) {
		if (basePtr[stride * temp.topMargin + x] == 0) break;
		++x;
		++temp.glyphWidth;
	}

	/*
	 * ----------------------------------------------------------------
	 * Compute per-character metrics
	 * ----------------------------------------------------------------
	 */
	int width = surf->Get_Width();
	int charsPerRow = width / (left + temp.glyphWidth);
	for (int ch = 0; ch < 256; ++ch) {

		int left = temp.leftMargin;
		int fontHeight = temp.glyphHeight;
		int top = temp.topMargin;
		int fontWidth = temp.glyphWidth;

		int glyphY = top + (fontHeight + top) * ((ch + 1) / charsPerRow);
		int glyphX = left + (left + fontWidth) * ((ch + 1) % charsPerRow);

		int first = -1;
		int last = 0;

		for (int x = glyphX; x < glyphX + fontWidth; ++x) {
			int nonEmpty = 0;
			for (int y = glyphY; y < glyphY + fontHeight; ++y) {
				if (basePtr[stride * y + x] != 0) ++nonEmpty;
			}
			if (nonEmpty) {
				last = x;
				if (first == -1) first = x;
			}
		}

		if (first != -1) {
			temp.charWidths[ch] = (last - first + 1);
		} else {
			temp.charWidths[ch] = (fontWidth / 3 + 1);
		}
	}

	surf->Unlock();

	/*
	 * ----------------------------------------------------------------
	 * Store result in caller's buffer
	 * ----------------------------------------------------------------
	 */
	memcpy(metrics, &temp, sizeof(FontMetrics));

	metricsDict.add(name, temp);

	return(true);
}


struct EzFont {
	char FaceName[128];
	int DeciPtWidth;
	int DeciPtHeight;
	int Attributes;
	HFONT FontHandle;
};

ArrayList<EzFont> g_EzFonts;


/// derived from MSDN "Moving Your Game to Windows, Part III" ttfont.cpp

#define EZ_ATTR_BOLD		  1
#define EZ_ATTR_ITALIC		  2
#define EZ_ATTR_UNDERLINE	  4
#define EZ_ATTR_STRIKEOUT	  8
HFONT Ez_Create_Font (HDC hdc, const char * face_name, int decipt_width, int decipt_height, int attributes);


/// <summary>
/// Fetches a font of the typeface and point size requested.
/// This routine keeps every font it has built, so repeated requests for the same
/// description hand back the same handle rather than burning another GDI object.
/// The dialog drawing code calls this routine wherever it needs a font.
/// </summary>
/// <param name="hdc">The device context to build the font for. If this is NULL, the
/// font is only looked up and never created.</param>
/// <param name="decipt_width">The character width in tenths of a point.</param>
/// <param name="decipt_height">The character height in tenths of a point.</param>
/// <param name="attributes">Bit flags of the EZ_ATTR_ style attributes to apply.</param>
/// <returns>Returns with a handle to the font, or NULL if it was neither cached nor
/// able to be created.</returns>
/// <remarks>The returned handle stays owned by the font cache. Do not delete it.</remarks>
HFONT WS_Get_Font(HDC hdc, const char * face_name, int decipt_width, int decipt_height, int attributes)
{
	EzFont font;

	for (int index = 0; index < g_EzFonts.length(); index++) {
		g_EzFonts.get(font, index);
		if (!strcmp(font.FaceName, face_name) && font.DeciPtWidth == decipt_width && font.DeciPtHeight == decipt_height && font.Attributes == attributes) {
			return(font.FontHandle);
		}
	}

	if (hdc == NULL) {
		return(NULL);
	}

	HFONT hFont = Ez_Create_Font(hdc, face_name, decipt_width, decipt_height, attributes);

	if (hFont == NULL) {
		return(NULL);
	}

	strcpy(font.FaceName, face_name);
	font.DeciPtWidth = decipt_width;
	font.DeciPtHeight = decipt_height;
	font.Attributes = attributes;
	font.FontHandle = hFont;

	if (g_EzFonts.addTail(font)) {
		return(hFont);
	}

	return(NULL);
}

/// <summary>
/// Creates a font of the typeface and point size requested.
/// This routine maps the requested decipoint dimensions through the device context's
/// current transform, so the font it builds matches the coordinate space the caller
/// draws in. Use WS_Get_Font in preference to this routine -- that one caches its fonts.
/// </summary>
/// <param name="hdc">The device context the font is to be built for.</param>
/// <param name="decipt_width">The character width in tenths of a point. Zero lets the
/// typeface choose its aspect.</param>
/// <param name="decipt_height">The character height in tenths of a point.</param>
/// <param name="attributes">Bit flags of the EZ_ATTR_ style attributes to apply.</param>
/// <returns>Returns with a handle to the font created, or NULL if it could not be
/// created.</returns>
/// <remarks>The caller takes ownership of the font handle.</remarks>
HFONT Ez_Create_Font (HDC hdc, const char * face_name, int decipt_width,
					int decipt_height, int attributes)
{
	HFONT		hFont ;
	LOGFONT	lf ;
	POINT		pt ;
	TEXTMETRIC tm ;

	SaveDC (hdc) ;

	SetGraphicsMode (hdc, GM_ADVANCED) ;
	ModifyWorldTransform (hdc, NULL, MWT_IDENTITY) ;
	SetViewportOrgEx (hdc, 0, 0, NULL) ;
	SetWindowOrgEx   (hdc, 0, 0, NULL) ;

	pt.x = decipt_width ;
	pt.y = decipt_height ;

	DPtoLP (hdc, &pt, 1) ;

	lf.lfHeight			= -pt.y ;
	lf.lfWidth			= 0 ;
	lf.lfEscapement		= 0 ;
	lf.lfOrientation	= 0 ;
	lf.lfWeight		 = attributes & EZ_ATTR_BOLD	   ? 700 : 0 ;
	lf.lfItalic		 = attributes & EZ_ATTR_ITALIC    ?   1 : 0 ;
	lf.lfUnderline 	 = attributes & EZ_ATTR_UNDERLINE ?   1 : 0 ;
	lf.lfStrikeOut 	 = attributes & EZ_ATTR_STRIKEOUT ?   1 : 0 ;
	lf.lfCharSet		= ANSI_CHARSET ;
	lf.lfOutPrecision	= 0 ;
	lf.lfClipPrecision	= 0 ;
	lf.lfQuality		= 0 ;
	lf.lfPitchAndFamily	= 0 ;

	strcpy (lf.lfFaceName, face_name) ;

	hFont = CreateFontIndirect (&lf) ;

	if (decipt_width != 0) {
		hFont = (HFONT) SelectObject (hdc, hFont) ;
		GetTextMetrics (hdc, &tm) ;
		DeleteObject (SelectObject (hdc, hFont)) ;
		lf.lfWidth = (int) (tm.tmAveCharWidth *
									fabs (pt.x) / fabs (pt.y) + 0.5);
		hFont = CreateFontIndirect (&lf) ;
	}

	RestoreDC (hdc, -1);
	return(hFont);
}



/// <summary>
/// Draws a line of text onto a surface.
/// This routine borrows a device context from the surface, unlocking it as often as it
/// must beforehand, and lets Windows put the text out aligned within the rectangle given.
/// Nothing is drawn while the game does not hold the focus.
/// </summary>
/// <param name="len">The number of characters of the text to draw.</param>
/// <param name="surface">The surface to draw upon, or NULL to draw on the alternate
/// surface.</param>
/// <returns>Returns with the pixel width of the text.</returns>
int OD_Draw_Text(COLORREF color, HFONT font, Rect const & rect, const char * text, int len, int x_alignment, int y_alignment, Surface * surface)
{
	if (!GameInFocus && !WindowedMode) {
		return(0);
	}

	DSurface *destsurf = (DSurface *)surface;
	if (!surface) {
		destsurf = (DSurface *)AlternateSurface;
	}

	int lock_count = 0;
	while (destsurf->Is_Locked()) {
		lock_count++;
		destsurf->Unlock();
	}

	SIZE text_size;

	HDC hDC = destsurf->GetDC();
	if (hDC) {

		if (font) {
			SelectObject(hDC, font);
		}

		SetTextColor(hDC, color);
		SetBkMode(hDC, TRANSPARENT);

		GetTextExtentPoint32(hDC, text, len, &text_size);

		int x_offset = rect.X;
		int y_offset = rect.Y;

		if (x_alignment == OD_TEXT_ALIGN_MIN) {
			x_offset += (rect.Width - text_size.cx + 1) / 2;
		} else if (x_alignment == OD_TEXT_ALIGN_CENTER) {
			x_offset += (text_size.cx + 1) / -2;
		} else if (x_alignment == OD_TEXT_ALIGN_MAX) {
			x_offset += -1 - text_size.cx;
		}

		if (y_alignment == OD_TEXT_ALIGN_MIN) {
			y_offset += (rect.Height - text_size.cy + 1) / 2;
		} else if (y_alignment == OD_TEXT_ALIGN_CENTER) {
			y_offset += (text_size.cy + 1) / -2;
		} else if (y_alignment == OD_TEXT_ALIGN_MAX) {
			y_offset += -1 - text_size.cy;
		}

		TextOut(hDC, x_offset, y_offset, text, len);
		destsurf->ReleaseDC(hDC);
	} else {
		text_size.cx = 0;
	}

	while (lock_count) {
		destsurf->Lock();
		lock_count--;
	}

	return(text_size.cx);
}


/// <summary>
/// Takes the mouse away from the game so that a dialog may use it.
/// The game cursor gives up its capture, leaving Windows free to drive the dialog and its
/// controls.
/// </summary>
/// <returns>Returns with the number of captures now outstanding.</returns>
/// <remarks>Each call must be matched by a call to Release_Mouse.</remarks>
int OwnerDraw::Capture_Mouse(void)
{
	if (MouseCursor != NULL) {
		if (MouseCursor->Is_Captured() == true) {
			MouseCursor->Release_Mouse();
		}
	}
	_mouse_counter++;
	return(_mouse_counter);
}


/// <summary>
/// Gives the mouse back to the game.
/// This routine undoes one Capture_Mouse. Only when the last dialog has finished with the
/// mouse does the game cursor take it back.
/// </summary>
/// <returns>Returns with the number of captures still outstanding.</returns>
int OwnerDraw::Release_Mouse(void)
{
	if (_mouse_counter > 0) {
		_mouse_counter--;
	}
	if (_mouse_counter == 0) {
		if (MouseCursor != NULL) {
			if (!MouseCursor->Is_Captured()) {
				MouseCursor->Capture_Mouse();
			}
		}
	}
	return(_mouse_counter);
}
