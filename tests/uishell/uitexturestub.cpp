/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Art as the harness sees it: none. The real loader reaches the game's file search and the
// mix archives through it, which the harness does not build, and a machine running the
// tests has no game art anyway. Every screen therefore draws in the plain form a player
// missing the art would see, which is what the harness is for.

#include "ui/rml/rmltexture.h"


UIImageResult UI_Load_Image(char const *, std::vector<unsigned char> & rgba, int & width, int & height)
{
	rgba.clear();
	width = 0;
	height = 0;
	return(UI_IMAGE_MISSING);
}


bool UI_Read_File(char const *, std::vector<unsigned char> & bytes)
{
	bytes.clear();
	return(false);
}


bool UI_Load_Indexed_Image(char const *, UIImageIndexed & image)
{
	image = UIImageIndexed();
	return(false);
}
