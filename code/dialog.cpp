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

/* $Header: /CounterStrike/DIALOG.CPP 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : DIALOG.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : September 10, 1993                                           *
 *                                                                                             *
 *                  Last Update : July 31, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Clip_Text_Print -- Prints text with clipping and <TAB> support.                           *
 *   Dialog_Box -- draws a dialog background box                                               *
 *   Display_Place_Building -- Displays the "place building" dialog box.                       *
 *   Display_Select_Target -- Displays the "choose target" prompt.                             *
 *   Display_Status -- Display the player scenario status box.                                 *
 *   Draw_Box -- Displays a highlighted box.                                                   *
 *   Draw_Caption -- Draws a caption on a dialog box.                                          *
 *   Fancy_Text_Print -- Prints text with a drop shadow.                                       *
 *   Plain_Text_Print -- Prints text without using a color scheme                              *
 *   Redraw_Needed -- Determine if sidebar needs to be redrawn.                                *
 *   Render_Bar_Graph -- Renders a specified bargraph.                                         *
 *   Simple_Text_Print -- Prints text with a drop shadow.                                      *
 *   Window_Box -- Draws a fancy box over the specified window.                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "dialog.h"

#include "_font.h"
#include "_surface.h"
#include "data.h"
#include "font.h"
#include "gadget.h"
#include "lightcon.h"
#include "scheme.h"
#include "surface.h"
#include "utf8.h"
#include "vector.h"

#include "color.hh"


/***********************************************************************************************
 * Draw_Box -- Displays a highlighted box.                                                     *
 *                                                                                             *
 *    This will draw a highlighted box to the logicpage. It can                                *
 *    optionally fill the box with a color as well. This is a low level                        *
 *    function and thus, it doesn't do any graphic mode color adjustments.                     *
 *                                                                                             *
 * INPUT:   x,y   -- Upper left corner of the box to be drawn (pixels).                        *
 *                                                                                             *
 *          w,h   -- Width and height of box (in pixels).                                      *
 *                                                                                             *
 *          up    -- Is the box rendered in the "up" stated?                                   *
 *                                                                                             *
 *          filled-- Is the box to be filled.                                                  *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/28/1991 JLB : Created.                                                                 *
 *   05/30/1992 JLB : Embedded color codes.                                                    *
 *   07/31/1992 JLB : Depressed option added.                                                  *
 *=============================================================================================*/
void Draw_Box(Rect const & rect, BoxStyleEnum up, bool filled)
{
	ColorScheme * scheme = ColorSchemes[GadgetClass::Get_Color_Scheme()];

	// Filler, Shadow, Hilite, Corner colors

	BoxStyleType const ButtonColors[BOXSTYLE_COUNT] = {
		{ scheme->Background, scheme->Highlight, scheme->Shadow,  scheme->Corners}, // Down
		{ scheme->Background, scheme->Shadow, scheme->Highlight,  scheme->Corners}, // Raised
		{ DKGREY, WHITE,  BLACK,  DKGREY},                                          // Disabled down
		{ DKGREY, BLACK,  LTGREY, DKGREY},                                          // Disabled up
		{ scheme->Background,  scheme->Box, scheme->Box,  scheme->Background},      // List box
		{ scheme->Background,  scheme->Box, scheme->Box,  scheme->Background},      // Dialog box
	};

	//w--;
	//h--;
	BoxStyleType const &style = ButtonColors[up];

	if (filled) {
		LogicalSurface->Fill_Rect(rect, scheme->Converter->Convert_Pixel(style.Filler));
	}

	switch (up) {
		case (BOXSTYLE_BOX):
			LogicalSurface->Draw_Rect(rect, scheme->Converter->Convert_Pixel(style.Highlight));
			break;

		case (BOXSTYLE_BORDER):
			LogicalSurface->Draw_Rect(Rect(rect.X+1, rect.Y+1, rect.Width-1, rect.Height-1), scheme->Converter->Convert_Pixel(style.Highlight));
			break;

		default:
			LogicalSurface->Draw_Line(Point2D(rect.X, rect.Y+rect.Height), Point2D(rect.X+rect.Width, rect.Y+rect.Height), scheme->Converter->Convert_Pixel(style.Shadow));
			LogicalSurface->Draw_Line(Point2D(rect.X+rect.Width, rect.Y), Point2D(rect.X+rect.Width, rect.Y+rect.Height), scheme->Converter->Convert_Pixel(style.Shadow));

			LogicalSurface->Draw_Line(Point2D(rect.X, rect.Y), Point2D(rect.X+rect.Width, rect.Y), scheme->Converter->Convert_Pixel(style.Highlight));
			LogicalSurface->Draw_Line(Point2D(rect.X, rect.Y), Point2D(rect.X, rect.Y+rect.Height), scheme->Converter->Convert_Pixel(style.Highlight));

			LogicalSurface->Put_Pixel(Point2D(rect.X, rect.Y+rect.Height), scheme->Converter->Convert_Pixel(style.Corner));
			LogicalSurface->Put_Pixel(Point2D(rect.X+rect.Width, rect.Y), scheme->Converter->Convert_Pixel(style.Corner));
			break;
	}
}


/***********************************************************************************************
 * Format_Window_String -- Separates a String into Lines.                                      *
 *   This function will take a long string and break it up into lines                          *
 *   which are not longer then the window width. Any character < ' ' is                        *
 *   considered a new line marker and will be replaced by a NULL.                              *
 *                                                                                             *
 * INPUT:      char *String - string to be formated.                                           *
 *             int maxlinelen - Max length of any line in pixels.                              *
 *                                                                                             *
 * OUTPUT:     int - number of lines string is.                                                *
 *                                                                                             *
 * WARNINGS:    The string passed in will be modified - NULLs will be put                      *
 *                into each position that will be a new line.                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/27/1992  SB : Created.                                                                 *
 *   05/18/1995 JLB : Greatly revised for new font system.                                     *
 *     09/04/1996 BWG : Added '@' is treated as a carriage return for width calculations.      *
 *=============================================================================================*/
int Format_Window_String(char * string, FontClass const * font, int maxlinelen, int & width, int & height)
{
	int	linelen;
	int	lines = 0;
	width	= 0;
	height = 0;

	// If no string was passed in, then there are no lines.
	if (!string || font == NULL) return(0);

	// While there are more letters left divide the line up.
	while (*string) {
		linelen = 0;
		height += font->Get_Height();
		lines++;

		/*
		**	Look for special line break character and force a line break when it is
		**	discovered.
		*/
		if (*string == '@') {
			*string = '\r';
		}

		char * start = string;

		// While the current line is less then the max length...
		while (linelen < maxlinelen && *string != '\r' && *string != '\0' && *string != '@') {
			linelen += font->Char_Pixel_Width(UTF8::Decode(string));
		}

		// if the line is to long...
		if (linelen >= maxlinelen) {

			/*
			**	Back up to an appropriate location to break.
			*/
			while (*string != ' ' && *string != '\r' && *string != '\0' && *string != '@' && string > start) {
				linelen -= font->Char_Pixel_Width(UTF8::Peek(string));
				string = UTF8::Previous(start, string);
			}

		}

		/*
		**	Record the largest width of the worst case string.
		*/
		if (linelen > width) {
			width = linelen;
		}

		/*
		**	Force a break at the end of the line.
		*/
		if (*string) {
		 	*string++ = '\r';
		}
	}
	return(lines);
}


/***********************************************************************************************
 * Simple_Text_Print -- Prints text with a drop shadow.                                        *
 *                                                                                             *
 *    This routine functions like Text_Print, but will render a drop                           *
 *    shadow (in black).                                                                       *
 *                                                                                             *
 *    The C&C gradient font colors are as follows:                                             *
 *         0      transparent (background)                                                     *
 *         1      foreground color for mono-color fonts only                                   *
 *         2      shadow under characters ("drop shadow")                                      *
 *         3      shadow all around characters ("full shadow")                                 *
 *         4-10   unused                                                                       *
 *         11      top row                                                                     *
 *         12      next row                                                                    *
 *         13      next row                                                                    *
 *         14      next row                                                                    *
 *         15      bottom row                                                                  *
 *                                                                                             *
 * INPUT:   text  -- Pointer to text to render.                                                *
 *                                                                                             *
 *          x,y   -- Pixel coordinate for to print text.                                       *
 *                                                                                             *
 *          fore  -- Foreground color.                                                         *
 *                                                                                             *
 *          back  -- Background color.                                                         *
 *                                                                                             *
 *          flag  -- Text print control flags.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1991 JLB : Created.                                                                 *
 *   10/26/94   JLB : Handles font X spacing in a more friendly manner.                        *
 *=============================================================================================*/
Point2D Simple_Text_Print(char const * text, Surface & surface, Rect const & rect, Point2D const & pt, ColorScheme * scheme, int back, TextPrintType flag, int fore)
{
	FontClass const * font=0;			// Font to use.
	int				shadow;				// Requested shadow value.
	unsigned char	fontpalette[16];	// Working font palette array.
	int forecolor;

	Point2D _point = pt;

	if (scheme == NULL) {
		scheme = Fetch_Scheme_By_Name(DEFAULT_GADGET_SCHEME);
	}

	/*
	**	Init the font palette to the given background color
	*/
	memset(&fontpalette[0], back, sizeof(fontpalette));

	forecolor = scheme->Color;

	/*
	**	A gradient font always requires special fixups for the palette
	*/
	int point = (flag & (TextPrintType)0x000F);
	if (/*point == TPF_VCR ||*/ point == TPF_6PT_GRAD || point == TPF_METAL12 || point == TPF_EFNT /*|| point == TPF_TYPE*/) {

		/*
		** If a gradient palette is specified, copy the remap table directly, otherwise
		**	use the foreground color as the entire font remap color.
		*/
		/*if (flag & TPF_USE_GRAD_PAL) {
			memcpy(fontpalette, scheme->FontRemap, 16);
			forecolor = scheme->Color;
			if (point == TPF_TYPE) {
				forecolor = fontpalette[1];
			}
		} else {*/
			/*
			 * Use the foreground color as the entire font remap color.
			 */
			memset(&fontpalette[4], scheme->Color, 12);
			forecolor = scheme->Color;
		//}

		/*
		**	Medium color: set all font colors to a medium value.  This flag
		**	overrides any gradient effects.
		*/
		if (flag & TPF_MEDIUM_COLOR) {
			forecolor = scheme->Color;
			memset(&fontpalette[4], scheme->Color, 12);
		}

		/*
		**	Bright color: set all font colors to a bright value.  This flag
		**	overrides any gradient effects.
		*/
		if (flag & TPF_BRIGHT_COLOR) {
			forecolor = scheme->Bright;
			memset(&fontpalette[4], scheme->BrightColor, 12);
		}
	}

	/*
	**	Change the current font if it differs from the font desired.
	*/
	font = Font_From_TPF(flag);

	/*
	**	Change the current font palette according to the dropshadow flags.
	*/
	shadow = (flag & (TPF_NOSHADOW|TPF_DROPSHADOW|TPF_FULLSHADOW|TPF_LIGHTSHADOW));
	switch (shadow) {

		/*
		**	The text is rendered plain.
		*/
		case TPF_NOSHADOW:
			fontpalette[2] = back;
			fontpalette[3] = back;
			break;

		/*
		**	The text is rendered with a simple
		**	drop shadow.
		*/
		case TPF_DROPSHADOW:
			fontpalette[2] = BLACK;
			fontpalette[3] = back;
			break;

		/*
		**	Special engraved text look for the options
		**	dialog system.
		*/
		case TPF_LIGHTSHADOW:
			fontpalette[2] = ((14 * 16) + 7)+1;
			fontpalette[3] = back;
			break;

		/*
		**	Each letter is surrounded by black. This is used
		**	when the text will be over a non-plain background.
		*/
		case TPF_FULLSHADOW:
			fontpalette[2] = BLACK;
			fontpalette[3] = BLACK;
			break;

		default:
			break;
	}
	//if (point != TPF_TYPE) {
		fontpalette[0] = back;
		fontpalette[fore] = scheme->Color;
	//}

	/*
	**	Display the (centered) message if there is one.
	*/
	if (text && *text) {
		switch (flag & (TPF_CENTER|TPF_RIGHT)) {
			case TPF_CENTER:
				_point.X -= font->String_Pixel_Width(text)>>1;
				break;

			case TPF_RIGHT:
				_point.X -= font->String_Pixel_Width(text);
				break;

			default:
				break;
		}

		fontpalette[fore] = forecolor;
		fontpalette[TBLACK] = back;
		_point = font->Print(text, surface, rect, _point, *scheme->Converter, fontpalette);
	}
	return(_point);
}


/// <summary>
/// Fetches the font that a text print style calls for.
/// This routine is used by the gadget drawing and text printing routines to turn the point
/// size portion of a print style into the font that should actually be used. A style that
/// names no recognized font falls back to the small six point font, so there is always a
/// font to draw with.
/// </summary>
/// <param name="flags">The text print style whose point size selects the font.</param>
/// <returns>Returns with a pointer to the font this style should be printed with.</returns>
FontClass *Font_From_TPF(TextPrintType flags)
{
	FontClass *font = NULL;

	int point = (flags & (TextPrintType)0x000F);
	switch (point) {
		case TPF_METAL12:
			font = Metal12FontPtr;
			break;

		case TPF_MAP:
			font = MapFontPtr;
			break;

		case TPF_6PT_GRAD:
			font = GradFont6Ptr;
			break;

		case TPF_6POINT:
			font = Font6Ptr;
			break;

		case TPF_EFNT:
			font = EditorFont;
			break;

		case TPF_8POINT:
			font = Font8Ptr;
			break;

		default:
			font = Font6Ptr;
			break;
	}

	return(font);
}


/***********************************************************************************************
 * Fancy_Text_Print -- Prints text with a drop shadow.                                         *
 *                                                                                             *
 *    This routine functions like Text_Print, but will render a drop                           *
 *    shadow (in black).                                                                       *
 *                                                                                             *
 * INPUT:   text  -- Text number to print.                                                     *
 *                                                                                             *
 *          x,y   -- Pixel coordinate for to print text.                                       *
 *                                                                                             *
 *          fore  -- Foreground color.                                                         *
 *                                                                                             *
 *          back  -- Background color.                                                         *
 *                                                                                             *
 *          flag  -- Text print control flags.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine is much slower than normal text print and                          *
 *             if rendered to the SEENPAGE, the intermediate rendering                         *
 *             steps could be visible.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1994 JLB : Created                                                                  *
 *=============================================================================================*/
Point2D __cdecl Fancy_Text_Print(int text, Surface & surface, Rect const & rect, Point2D const & pt, ColorScheme * fore, int back, TextPrintType flag, ...)
{
	char		buffer[512];		// Working staging buffer.
	va_list	arg;					// Argument list var.

	/*
	**	If the text number is valid, then process it.
	*/
	if (text != TXT_NONE) {
		va_start(arg, flag);

		/*
		**	The text string must be locked since the vsprintf function doesn't know
		**	how to handle EMS pointers.
		*/
		char const * tptr = Fetch_String(text);
		vsnprintf(buffer, sizeof(buffer), tptr, arg);
		va_end(arg);

		return(Simple_Text_Print(buffer, surface, rect, pt, fore, back, flag, PURPLE));
	} else {

		/*
		**	Just the flags are to be changed, since the text number is TXT_NONE.
		*/
		return(Simple_Text_Print((char const *)NULL, surface, rect, pt, fore, back, flag, PURPLE));
	}
}


/***********************************************************************************************
 * Fancy_Text_Print -- Prints text with a drop shadow.                                         *
 *                                                                                             *
 *    This routine functions like Text_Print, but will render a drop                           *
 *    shadow (in black).                                                                       *
 *                                                                                             *
 * INPUT:   text  -- Pointer to text to render.                                                *
 *                                                                                             *
 *          x,y   -- Pixel coordinate for to print text.                                       *
 *                                                                                             *
 *          fore  -- Foreground color.                                                         *
 *                                                                                             *
 *          back  -- Background color.                                                         *
 *                                                                                             *
 *          flag  -- Text print control flags.                                                 *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine is much slower than normal text print and                          *
 *             if rendered to the SEENPAGE, the intermediate rendering                         *
 *             steps could be visible.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1991 JLB : Created.                                                                 *
 *   10/26/94   JLB : Handles font X spacing in a more friendly manner.                        *
 *   11/29/1994 JLB : Separated actual draw action.                                            *
 *=============================================================================================*/
Point2D __cdecl Fancy_Text_Print(char const * text, Surface & surface, Rect const & rect, Point2D const &pt, ColorScheme * fore, int back, TextPrintType flag, ...)
{
	char		buffer[512];		// Working staging buffer.
	va_list	arg;					// Argument list var.

	/*
	**	If there is a valid text string pointer then build the final string into the
	**	working buffer before sending it to the simple string printing routine.
	*/
	if (text) {

		/*
		**	Since vsprintf doesn't know about EMS pointers, be sure to surround this
		**	call with locking code.
		*/
		va_start(arg, flag);
		vsnprintf(buffer, sizeof(buffer), text, arg);
		va_end(arg);

		return(Simple_Text_Print(buffer, surface, rect, pt, fore, back, flag, PURPLE));
	} else {

		/*
		**	Just the flags are desired to be changed, so call the simple print routine with
		**	a NULL text pointer.
		*/
		return(Simple_Text_Print((char const *)NULL, surface, rect, pt, fore, back, flag, PURPLE));
	}
}


/***********************************************************************************************
 * Clip_Text_Print -- Prints text with clipping and <TAB> support.                             *
 *                                                                                             *
 *    Use this routine to print text that that should be clipped at an arbitrary right margin  *
 *    as well as possibly recognizing <TAB> characters. Typical users of this routine would    *
 *    be list boxes.                                                                           *
 *                                                                                             *
 * INPUT:   text  -- Reference to the text to print.                                           *
 *                                                                                             *
 *          x,y   -- Pixel coordinate of the upper left corner of the text position.           *
 *                                                                                             *
 *          fore  -- The foreground color to use.                                              *
 *                                                                                             *
 *          back  -- The background color to use.                                              *
 *                                                                                             *
 *          flag  -- The text print flags to use.                                              *
 *                                                                                             *
 *          width -- The maximum pixel width to draw the text. Extra characters beyond this    *
 *                   point will not be printed.                                                *
 *                                                                                             *
 *          tabs  -- Optional pointer to a series of pixel tabstop positions.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
Point2D Conquer_Clip_Text_Print(char const * text, Surface & surface, Rect const &rect, Point2D const & pt, ColorScheme * fore, int back, TextPrintType flag, int width, int const * tabs)
{
	char buffer[512];

	Point2D point = pt;

	if (text) {
		strcpy(buffer, text);

		/*
		**	Set the font and spacing characteristics according to the flag
		**	value passed in.
		*/
		FontClass *font = Font_From_TPF(flag);

		char * source = &buffer[0];
		unsigned offset = 0;
		int processing = true;
		while (processing && offset < (unsigned)width) {
			char * ptr = strchr(source, '\t');

			/*
			**	Zap the tab character. It will be processed later.
			*/
			if (ptr) {
				*ptr = '\0';
			}

			if (*source) {

				/*
				**	Scan forward until the end of the string is reached or the
				**	maximum width, whichever comes first.
				*/
				int w = 0;
				char * bptr = source;
				do {
					w += font->Char_Pixel_Width(UTF8::Decode(bptr));
				} while (*bptr && offset+w < (unsigned)width);

				/*
				**	If the maximum width has been exceeded, then remove the last
				**	character and signal that further processing is not necessary.
				*/
				if (offset+w >= (unsigned)width) {
					bptr = UTF8::Previous(source, bptr);
					w -= font->Char_Pixel_Width(UTF8::Peek(bptr));
					*bptr = '\0';
					processing = 0;
				}

				/*
				**	Print this text block and advance the offset accordingly.
				*/
				point = Simple_Text_Print(source, surface, rect, Point2D(pt.X+offset, pt.Y), fore, back, flag, PURPLE);
				offset += w;
			}

			/*
			**	If a <TAB> was the terminator for this text block, then advance
			**	to the next tabstop.
			*/
			if (ptr) {
				if (tabs) {
					while (offset > (unsigned)*tabs) {
						tabs++;
					}
					offset = *tabs;
				} else {
					offset = ((offset+1 / 50) + 1) * 50;
				}
				source = ptr+1;
			} else {
				break;
			}
		}
	}
	return(point);
}

/***************************************************************************
 * Plain_Text_Print -- Prints text without using a color scheme            *
 *                                                                         *
 * INPUT:                                                                  *
 *      text      text to print                                            *
 *      x,y      coords to print at                                        *
 *      fore      desired foreground color                                 *
 *      back      desired background color                                 *
 *      flag      text print control flags                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Do not use the gradient control flag with this routine!  For       *
 *      a gradient appearance, use Fancy_Text_Print.                       *
 *      Despite this routine's name, it is actually faster to call         *
 *      Fancy_Text_Print than this routine.                                *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/05/1996 BRR : Created.                                             *
 *=========================================================================*/
Point2D Plain_Text_Print(int text, Surface & surface, Rect const &rect, Point2D const & xy, int /*fore*/, int back, TextPrintType flag, int scheme, int fore)
{
	return(Simple_Text_Print(Fetch_String(text), surface, rect, xy, ColorSchemes[scheme], back, flag, fore));
}


/***************************************************************************
 * Plain_Text_Print -- Prints text without using a color scheme            *
 *                                                                         *
 * INPUT:                                                                  *
 *      text      text to print                                            *
 *      x,y      coords to print at                                        *
 *      fore      desired foreground color                                 *
 *      back      desired background color                                 *
 *      flag      text print control flags                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Do not use the gradient control flag with this routine!  For       *
 *      a gradient appearance, use Fancy_Text_Print.                       *
 *      Despite this routine's name, it is actually faster to call         *
 *      Fancy_Text_Print than this routine.                                *
 *                                                                         *
 * HISTORY:                                                                *
 *   01/05/1996 BRR : Created.                                             *
 *=========================================================================*/
Point2D Plain_Text_Print(char const * text, Surface & surface, Rect const &rect, Point2D const & pt, int /*fore*/, int back, TextPrintType flag, int scheme, int fore)
{
	return(Simple_Text_Print(text, surface, rect, pt, ColorSchemes[scheme], back, flag, fore));
}


/// <summary>
/// Fetches a font remap palette for a solid color.
/// This routine builds the small remap table that the font printing routines expect, filled
/// so that the text prints in the requested color over a transparent background.
/// </summary>
/// <param name="color">The color to print the text in.</param>
/// <remarks>The palette is a shared buffer, so it only lasts until the next call to this
/// routine.</remarks>
unsigned char * Font_Palette(int color)
{
	static unsigned char _fpalette[16];

	memset(_fpalette, '\0', sizeof(_fpalette));
	memset(&_fpalette[11], color, 5);
	return(_fpalette);
}

