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
 *                     $Archive:: /VSS_Sync/wwlib/wwfont.cpp                                  $*
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 10/16/00 11:42a                                             $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WWFontClass::Print -- Print text to the surface specified.                                *
 *   WWFontClass::WWFontClass -- Constructor for a font class object.                          *
 *   WWFontClass::Char_Pixel_Width -- Fetch the pixel width of the character specified.        *
 *   WWFontClass::String_Pixel_Width -- Determines the width of the string in pixels.          *
 *   WWFontClass::Raw_Width -- Fetch the raw width of a character.                             *
 *   WWFontClass::Raw_Height -- Fetch the height of the font.                                  *
 *   WWFontClass::Get_Width -- Get normalized width of the nominal font character.             *
 *   WWFontClass::Get_Height -- Fetch the normalized height of the nominal font character.     *
 *   WWFontClass::Set_XSpacing -- Set the X spacing override value.                            *
 *   WWFontClass::Set_YSpacing -- Set the vertical (Y) spacing override value.                 *
 *   Set_Font_Data -- Allow font data to be set after construction.                            *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "wwfont.h"

#include "_convert.h"
#include "convert.h"

#include "dbgprint.h"
#include "utf8.h"

#include <algorithm>


#define FONTINFOMAXHEIGHT		4
#define FONTINFOMAXWIDTH		5

#define	FUDGEDIV		16


/***********************************************************************************************
 * WWFontClass::WWFontClass -- Constructor for a font class object.                            *
 *                                                                                             *
 *    This constructs a font object as it is based upon the font data output by the legacy     *
 *    FONTMAKE.EXE utility.                                                                    *
 *                                                                                             *
 * INPUT:   fontdata -- Pointer to the data as output by the fontmake utility.                 *
 *                                                                                             *
 *          isoutlined  -- Is this font data available with outline pixels embedded?           *
 *                                                                                             *
 *          shadow   -- Does the font data have shadow pixels embedded?                        *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
WWFontClass::WWFontClass(void const * fontdata, bool isoutlined, int shadow) :
	IsOutlinedData(isoutlined),
	Shadow(shadow),
	FontXSpacing(0),
	FontYSpacing(0),
	FontData((FontType *)fontdata),
	IsWindows1252(false)
{
	Set_XSpacing(0);
	Set_YSpacing(0);

	// A font with the full table is laid out as Windows-1252; the 12-point metal font stops
	// at 219 glyphs and follows code page 437.
	IsWindows1252 = Glyph_Count() >= 0xFF;
	DebugString("WWFontClass: %d glyphs, %dx%d cell, %s order\n", Glyph_Count(), Raw_Width(), Raw_Height(), IsWindows1252 ? "Windows-1252" : "code page 437");
}


/***********************************************************************************************
 * WWFontClass::Char_Pixel_Width -- Fetch the pixel width of the character specified.          *
 *                                                                                             *
 *    This will return with the pixel width of the character specified.                        *
 *                                                                                             *
 * INPUT:   c  -- The character to determine the pixel width for.                              *
 *                                                                                             *
 * OUTPUT:  Returns with the pixel width of the character.                                     *
 *                                                                                             *
 * WARNINGS:   The return width is the screen real estate width which may differ from the      *
 *             actual pixels of the character. This difference is controlled by the font       *
 *             X spacing.                                                                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Char_Pixel_Width(char32_t code) const
{
	if (code < ' ') return(0); /// ' ' is the first printable character
	int raw = (*(((unsigned char *)FontData) + FontData->WidthBlockOffset + Glyph_Index(code)));
	raw += FontXSpacing;
	return(raw);
}


/***********************************************************************************************
 * WWFontClass::String_Pixel_Width -- Determines the width of the string in pixels.            *
 *                                                                                             *
 *    This routine is used to determine how many pixels wide the string will be if it were     *
 *    printed.                                                                                 *
 *                                                                                             *
 * INPUT:   string   -- The string to convert into its pixel width.                            *
 *                                                                                             *
 * OUTPUT:  Returns with the number of pixels the string would span if it were printed.        *
 *                                                                                             *
 * WARNINGS:   This routine does not take into account clipping.                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::String_Pixel_Width(char const * string) const
{
	if (string == NULL) return(0);

	int largest = 0;		// Largest recorded width of the string.
	int width = 0;
	while (*string) {
		char32_t code = UTF8::Decode(string);
		if (code == '\r' || code == '\n') {
			largest = std::max(largest, width);
			width = 0;
		} else {
			width += Char_Pixel_Width(code);	// add each char's width
		}
	}
	largest = std::max(largest, width);
	return(largest);
}


/****************************************************************************
*
* NAME
*     String_Pixel_Bounds(String, Bounds)
*
* DESCRIPTION
*     Calculate the bounding box for the specified string.
*
* INPUTS
*     String - String to calculate bounds for
*     Bounds - Rect to fill with bounds
*
* RESULT
*     NONE
*
****************************************************************************/
void WWFontClass::String_Pixel_Bounds(const char * string, Rect& bounds) const
{
	bounds.X = 0;
	bounds.Y = 0;
	bounds.Width = 0;
	bounds.Height = 0;

	if (string == NULL) {
		return;
	}

	int width = 0;
	int height = Get_Height();

	while ( *string != 0 ) {

		char32_t code = UTF8::Decode(string);
		if (( code == '\r' ) || ( code == '\n' )) {

			height += Get_Height();
			bounds.Width = std::max( bounds.Width, width );
			width = 0;
		} else {
			width += Char_Pixel_Width(code);	// add each char's width
		}
	}

	bounds.Width	= std::max( bounds.Width, width );
	bounds.Height	= height;
}


/***********************************************************************************************
 * WWFontClass::Raw_Width -- Fetch the raw width of a character.                               *
 *                                                                                             *
 *    This routine returns the nominal width of the font. This width is basically the width of *
 *    the widest character in the font.                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the width of the widest character in the font.                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Raw_Width(void) const
{
	return(*(((unsigned char *)FontData) + FontData->InfoBlockOffset + FONTINFOMAXWIDTH));
}


/***********************************************************************************************
 * WWFontClass::Raw_Height -- Fetch the height of the font.                                    *
 *                                                                                             *
 *    This returns the height of the font without regard for any Y spacing offsets.            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the height of the font.                                               *
 *                                                                                             *
 * WARNINGS:   All characters in the font have the same logical height even if they actually   *
 *             consume less pixels.                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Raw_Height(void) const
{
	return(*(((unsigned char *)FontData) + FontData->InfoBlockOffset + FONTINFOMAXHEIGHT));
}


int WWFontClass::Glyph_Count(void) const
{
	return((FontData->WidthBlockOffset - FontData->OffsetBlockOffset) / (int)sizeof(unsigned short));
}


/// <summary>
/// Returns the glyph slot that draws code, or the slot for '?' when the font has none.
/// Westwood added the oe ligature to its code page 437 layout at 0xCE, and a slot holding
/// the shared placeholder box at data offset 0 counts as absent.
/// </summary>
unsigned char WWFontClass::Glyph_Index(char32_t code) const
{
	if (code < ' ') {
		return((unsigned char)code);
	}

	int index;
	if (IsWindows1252) {
		index = UTF8::Windows_1252_Glyph(code);
	} else if (code == 0x0153) {
		index = 0xCE;
	} else {
		index = UTF8::OEM_437_Glyph(code);
	}
	if (index < 0 || index >= Glyph_Count()) {
		return('?');
	}

	unsigned short const * fontoffset = (unsigned short const *)(((unsigned char const *)FontData) + FontData->OffsetBlockOffset);
	if (index > ' ' && fontoffset[index] == 0) {
		return('?');
	}
	return((unsigned char)index);
}


/***********************************************************************************************
 * WWFontClass::Get_Width -- Get normalized width of the nominal font character.               *
 *                                                                                             *
 *    This routine is used to fetch the width of the widest character in the font but the      *
 *    width has been biased according to any X spacing override present.                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the normalized width of the widest character in the font.             *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Get_Width(void) const
{
	return(Raw_Width() + ((FontXSpacing > 0) ? FontXSpacing : 0));
}


/***********************************************************************************************
 * WWFontClass::Get_Height -- Fetch the normalized height of the nominal font character.       *
 *                                                                                             *
 *    This will return the height of the font but the returned height will be adjusted by any  *
 *    Y spacing override present.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the height of the font normalized by any spacing overrides.           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Get_Height(void) const
{
	return(Raw_Height() + ((FontYSpacing > 0) ? FontYSpacing : 0));
}


/***********************************************************************************************
 * WWFontClass::Set_XSpacing -- Set the X spacing override value.                              *
 *                                                                                             *
 *    Use this routine to control the horizontal spacing override for this font. If the value  *
 *    is negative, the font becomes compressed. If the value is positive, then the font        *
 *    becomes expanded.                                                                        *
 *                                                                                             *
 * INPUT:   x  -- The X spacing override to use for this font.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the old X spacing override value.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Set_XSpacing(int x)
{
	int old = FontXSpacing;
	FontXSpacing = x;
	if (IsOutlinedData) {
		switch (Shadow) {
			case 0:
				FontXSpacing += -2;
				break;

			case 1:
				FontXSpacing += -1;
				break;

			case 2:
				FontXSpacing += -1;
				break;
		}
	}
	FontXSpacing += Get_Width() / FUDGEDIV;
	return(old);
}


/***********************************************************************************************
 * WWFontClass::Set_YSpacing -- Set the vertical (Y) spacing override value.                   *
 *                                                                                             *
 *    Use this routine to control the "line spacing" of a font. If the Y spacing is negative   *
 *    then the font becomes closer to the line above it. If value is positive, then more       *
 *    space occurs between lines.                                                              *
 *                                                                                             *
 * INPUT:   y  -- The Y spacing override to use for this font.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the old Y spacing override value.                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWFontClass::Set_YSpacing(int y)
{
	int old = FontYSpacing;
	FontYSpacing = y;
	if (IsOutlinedData) {
		switch (Shadow) {
			case 0:
				FontYSpacing += -2;
				break;

			case 1:
				FontYSpacing += -1;
				break;

			case 2:
				FontYSpacing += -1;
				break;
		}
	}
	FontYSpacing += Get_Height() / FUDGEDIV;
	return(old);
}


/***********************************************************************************************
 * WWFontClass::Print -- Print text to the surface specified.                                  *
 *                                                                                             *
 *    This displays text to the surface specified and with the attributes specified.           *
 *                                                                                             *
 * INPUT:   string   -- The string to display to the surface.                                  *
 *                                                                                             *
 *          surface  -- The surface to display the text upon.                                  *
 *                                                                                             *
 *          cliprect -- The clipping rectangle that both clips the text and biases the print   *
 *                      location to.                                                           *
 *                                                                                             *
 *          point    -- The draw position for the upper left corner of the first character.    *
 *                                                                                             *
 *          convert  -- The pixel convert object that is used to draw the correct colors to    *
 *                      the destination surface.                                               *
 *                                                                                             *
 *          remap    -- Auxiliary remap table for font colors.                                 *
 *                                                                                             *
 * OUTPUT:  Returns with the point where the next print should begin if it is to smoothly      *
 *          continue where this print operation left off.                                      *
 *                                                                                             *
 * WARNINGS:   There are two separate drawing routines; one for old fonts and one for new      *
 *                fonts.                                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/26/1997 JLB : Created.                                                                 *
 *   06/20/1887 BNA : Modified to handle new fonts.                                            *
 *   01/24/2000 SKB : Updated from C&C to include BNA code.                                    *
 *   01/24/2000 SKB : put in call for Get_Remap_Palette                                        *
 *   01/24/2000 SKB : Put in call for get converer.                                            *
 *=============================================================================================*/
Point2D WWFontClass::Print(char const * string, Surface & surface, Rect const & cliprect, Point2D const & drawpoint, ConvertClass const & convertref, unsigned char const * remap) const
{
	if (string == NULL) return(drawpoint);

	/*
	**	Compute the surface relative coordinate for the print position.
	*/
	Point2D point = drawpoint;
	int xpos = Bias_To(point, cliprect).X;
//	int xpos = point.X + cliprect.X;
	int ypos = Bias_To(point, cliprect).Y;
//	int ypos = point.Y + cliprect.Y;

	int xspacing = FontXSpacing + Raw_Width()/FUDGEDIV;
	int yspacing = FontYSpacing + Raw_Width()/FUDGEDIV;

	// This font palette assumes that the font will be used as defined by converter.
	static unsigned char const fontpalette[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
	if (!remap) {
		remap = &fontpalette[0];
	}

	/*
	**	Verify that the clipping rectangle (if present) falls within the bounds of the surface.
	*/
//	Rect cliprect = surface->Get_Rect();
//	cliprect.X = 0;
//	cliprect.Y = 0;

	/*
	**	Trivially check to see if the print start position is outside the bounds of the clipping
	**	rectangle.
	*/
	if (xpos >= cliprect.X+cliprect.Width || ypos >= cliprect.Y+cliprect.Height) {
		return(drawpoint);
	}

	/*
	**	Check to see if access to the surface buffer is possible.
	*/
	void * buffer = surface.Lock();
	if (buffer != NULL) {
		int startx = xpos;
		unsigned char * fontwidth = ((unsigned char*)FontData) + FontData->WidthBlockOffset;
		unsigned short * fontheight = (unsigned short*)(((unsigned char*)FontData) + FontData->HeightOffset);
		unsigned short * fontoffset = (unsigned short*)(((unsigned char*)FontData) + FontData->OffsetBlockOffset);
		int bbp = surface.Bytes_Per_Pixel();

		/*
		**	Process the whole string. Stop when the string reaches the right margin.
		*/
		while (*string != '\0') {
			char32_t code = UTF8::Decode(string);

			/*
			**	Certain control characters serve a formatting purpose. They merely
			**	adjust the next draw position.
			*/
			if (code == '\r') {
				xpos = startx;
				ypos += Raw_Height() + ((yspacing > 0) ? yspacing : 0);
				continue;
			}
			if (code == '\n') {
				xpos = cliprect.X;
				ypos += Raw_Height() + ((yspacing > 0) ? yspacing : 0);
				continue;
			}
			unsigned char c = Glyph_Index(code);

			/*
			**	Fetch working values for the character to display. These are used to
			**	control the size of the character rectangle.
			*/
			int width = fontwidth[c];
			int dheight = fontheight[c] >> 8;
			int firstrow = fontheight[c] & 0xFF;
//			int height = dheight+firstrow;

			/*
			**	Build the character rectangle (surface relative coordinates).
			*/
			Rect crect(xpos, ypos, width+((xspacing > 0) ? xspacing : 0), *(((unsigned char *)FontData) + FontData->InfoBlockOffset + FONTINFOMAXHEIGHT) + ((yspacing > 0) ? yspacing : 0));

			/*
			**	Check to see if any part of this character would appear within the clipping
			**	rectangle. If not, then don't process this character.
			*/
			crect = Intersect(crect, cliprect);
			if (crect.Is_Valid()) {

				/*
				**	Fill the background of the character if the background
				**	color is not transparent.
				*/
				if (remap[0] != 0) {
					surface.Fill_Rect(crect, convertref.Convert_Pixel(remap[0]));
				}

				/*
				**	Loop through all data rows of this character and output the row
				**	data with clipping.
				*/
				if (FontData->FontCompress != (char) 2) { // if the font is the old style
					unsigned char * dataptr = ((unsigned char *)FontData) + fontoffset[c];
					void * drawbuff = (void*)(((char*)buffer) + ((ypos + firstrow) * surface.Stride()) + xpos * bbp);

					for (int h = 0; h < dheight; h++) {

						/*
						**	If the current row to be drawn falls below the clipping rectangle, then
						**	no further drawing for this character is needed -- bail out.
						*/
						if (ypos + firstrow + h >= crect.Y + crect.Height) break;

						/*
						**	If the current row to be drawn falls above the clipping rectangle,
						**	then skip the row and advance the source data pointer as well.
						*/
						if (ypos + firstrow + h < crect.Y) {
							drawbuff = (void*)(((char*)drawbuff) + surface.Stride());
							dataptr = dataptr + ((width+1)/2);
						} else {

							/*
							**	Process one row of character data. For simplicity reasons, just
							**	loop through all pixels of this row and perform clipping at the
							**	pixel level.
							*/
							int dx = xpos;
							int workwidth = width;
							while (workwidth > 0) {

								/*
								**	Fetch the next two pixel values. Two pixel values must
								**	be fetched at the same time since they are nibble packed.
								*/
								int c1 = remap[*dataptr & 0x0F];
								int c2 = remap[(*dataptr++ & 0xF0) >> 4];

								/*
								**	Draw the first pixel if it falls within the clipping
								**	rectangle and is not transparent.
								*/
								if (dx >= cliprect.X && dx < cliprect.X+cliprect.Width) {
									if (c1 != 0) {
										if (bbp == 2) {
											*(short *)drawbuff = (short)convertref.Convert_Pixel(c1);
										} else {
											*(char *)drawbuff = (char)convertref.Convert_Pixel(c1);
										}
									}
								}
								dx += 1;
								drawbuff = ((char*)drawbuff) + bbp;
								workwidth -= 1;
								if (workwidth == 0) break;

								/*
								**	Draw the second pixel if it falls within the clipping
								**	rectangle and is not transparent.
								*/
								if (dx >= cliprect.X && dx < cliprect.X+cliprect.Width) {
									if (c2 != 0) {
										if (bbp == 2) {
											*(short *)drawbuff = (short)convertref.Convert_Pixel(c2);
										} else {
											*(char *)drawbuff = (char)convertref.Convert_Pixel(c2);
										}
									}
								}
								dx += 1;
								drawbuff = ((char*)drawbuff) + bbp;
								workwidth -= 1;
							}

							/*
							**	Move the output pixel pointer to the next line down but at the
							**	left margin of the character block.
							*/
							drawbuff = (void*)((((char*)drawbuff) - (width*bbp)) + surface.Stride());
						}
					}
				} else {
					// the font is of the new type
					unsigned char * dataptr = ((unsigned char *)FontData) + fontoffset[c] + FontData->DataBlockOffset;
					void * drawbuff = (void*)(((char*)buffer) + ((ypos + firstrow) * surface.Stride()) + xpos * bbp);

					for (int h = 0; h < dheight; h++) {

						/*
						**	If the current row to be drawn falls below the clipping rectangle, then
						**	no further drawing for this character is needed -- bail out.
						*/
						if (ypos + firstrow + h >= crect.Y + crect.Height) break;

						/*
						**	If the current row to be drawn falls above the clipping rectangle,
						**	then skip the row and advance the source data pointer as well.
						*/
						if (ypos + firstrow + h < crect.Y) {
							drawbuff = (void*)(((char*)drawbuff) + surface.Stride());
							dataptr = dataptr + width;
						} else {

							/*
							**	Process one row of character data. For simplicity reasons, just
							**	loop through all pixels of this row and perform clipping at the
							**	pixel level.
							*/
							int dx = xpos;
							int workwidth = width;
							while (workwidth > 0) {

								/*
								**	Fetch the next pixel value.
								*/
								int c1 = remap[*dataptr++];	// no remapping

								/*
								**	Draw the pixel if it falls within the clipping
								**	rectangle and is not transparent.
								*/
								if (dx >= cliprect.X && dx < cliprect.X+cliprect.Width) {
									if (c1 != 0) {
										if (bbp == 2) {
											*(short *)drawbuff = (short)convertref.Convert_Pixel(c1);
										} else {
											*(char *)drawbuff = (char)convertref.Convert_Pixel(c1);
										}
									}
								}
								dx += 1;
								drawbuff = ((char*)drawbuff) + bbp;
								workwidth -= 1;
							}

							/*
							**	Move the output pixel pointer to the next line down but at the
							**	left margin of the character block.
							*/
							drawbuff = (void*)((((char*)drawbuff) - (width*bbp)) + surface.Stride());
						}
					}
				}
			}
			xpos += Char_Pixel_Width(code);
//			xpos += width + xspacing;
		}

		point = Point2D(xpos - cliprect.X, ypos - cliprect.Y);
		surface.Unlock();
	}
	return(point);
}
