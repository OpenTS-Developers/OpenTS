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
 *                     $Archive:: /G/wwlib/data.h                                             $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "buff.h"
#include "iff.h"
#include "win.h"
#include "wwfile.h"

/// An ID of -1 or 0 is not valid for Fetch_String.
#define TXT_NONE	0

int Load_Picture(FileClass &file, Buffer & scratchbuf, Buffer & destbuf, unsigned char * palette, PicturePlaneType format);
void * Load_Alloc_Data(FileClass & file);
int Load_Uncompress(FileClass & file, Buffer & uncomp_buff, Buffer & dest_buff, void * reserved_data);
char const * Fetch_String(int id);
void * Hires_Load(char * name);

bool Init_Language_Resources(bool show_error);
void Get_Language_Version(char *version_string);
