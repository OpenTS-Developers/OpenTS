/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmltexture.h"


int UITestSheetLookups = 0;


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
	UITestSheetLookups++;
	image = UIImageIndexed();
	return(false);
}
