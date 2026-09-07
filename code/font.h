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
 *                     $Archive:: /Commando/Code/wwlib/font.h                                 $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/28/00 2:40p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "rect.h"

class Surface;
class ConvertClass;

/*
**	A font object represent the data that comprises the individual characters as well
**	as drawing text in the font to a surface. This is an abstract base class that is to be
**	derived into a concrete version.
*/
class FontClass
{
	public:
		virtual ~FontClass(void) {}

		virtual int Char_Pixel_Width(char32_t code) const = 0;
		int Char_Pixel_Width(char) const = delete;
		virtual int String_Pixel_Width(char const * string) const = 0;
		virtual void String_Pixel_Bounds(const char * string, Rect & bounds) const = 0;
		virtual int Get_Width(void) const = 0;
		virtual int Get_Height(void) const = 0;
		virtual Point2D Print(char const * string, Surface & surface, Rect const & cliprect, Point2D const & point, ConvertClass const & converter, unsigned char const * remap=NULL) const = 0;

		virtual int Set_XSpacing(int x) = 0;
		virtual int Set_YSpacing(int y) = 0;
};
