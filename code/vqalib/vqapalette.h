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

#ifndef VQMPALETTE_H
#define VQMPALETTE_H
/****************************************************************************
*
*        C O N F I D E N T I A L -- W E S T W O O D  S T U D I O S
*
*----------------------------------------------------------------------------
*
* FILE
*     Palette.h (32-Bit protected mode)
*
* DESCRIPTION
*     Palette definitions.
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*
* DATE
*     Febuary 3, 1995
*
****************************************************************************/

/* Prototypes */

#ifdef __cplusplus
extern "C" {
#endif

void __cdecl SetPalette(unsigned char *palette,long numbytes,unsigned long slowpal);
void __cdecl ReadPalette(void *palette);
void __cdecl SetDAC(long color, long red, long green, long blue);
void __cdecl TranslatePalette(void *pal24, void *pal15, long numbytes);

#ifdef __cplusplus
}
#endif

void SortPalette(unsigned char *pal, long numcolors);

#endif /* VQMPALETTE_H */

