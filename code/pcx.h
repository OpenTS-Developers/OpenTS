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
 *                     $Archive:: /Commando/Library/PCX.H                                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "bsurface.h"
#include "palette.h"
#include "wwfile.h"

#include <cstring>

#pragma pack(push,1)
struct RGB {
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};

struct PCX_HEADER
{
	char	id;
	char	version;
	char	encoding;
	char	pixelsize;
	unsigned short	x;
	unsigned short	y;
	unsigned short	width;
	unsigned short	height;
	short	xres;
	short	yres;
	RGB	ega_palette[16];
	char	nothing;
	unsigned char	color_planes;
	unsigned short	byte_per_line;
	short	palette_type;
	short	horz_screen_size;
	short	vert_screen_size;
	char	filler[54];
};
#pragma pack(pop)

bool Read_PCX_Size(FileClass & file, int & width, int & height);
Surface * Read_PCX_File(FileClass & file_handle, PaletteClass * palette=NULL, void * buff=NULL, int size=0);
bool Write_PCX_File(FileClass & file, Surface & pic, PaletteClass * palette);
