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
 *                     $Archive:: /Commando/Library/MISC.H                                    $*
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

#include "win.h"

extern bool WindowedMode;

/*========================= C++ Routines ==================================*/

/*
**	Pointer to function to call if we detect a focus loss
*/
extern	void (*Misc_Focus_Loss_Function)(void);
/*
**	Pointer to function to call if we detect a surface restore
*/
extern	void (*Misc_Focus_Restore_Function)(void);

/*
**	Function to call if we detect focus loss
*/
extern	void (*Audio_Focus_Loss_Function)(void);


/*
 * The size of the frame the game renders into, which video.cpp owns.
 */
extern int VideoModeWidth;
extern int VideoModeHeight;

/*=========================================================================*/
/* The following prototypes are for the file: EXIT.CPP							*/
/* Prog_End Must be supplied by the user program in startup.cpp				*/
/*=========================================================================*/
void __cdecl Prog_End(void);
//void __cdecl Exit(INT errorval, const char *message, ...);

/*=========================================================================*/
/* The following prototypes are for the file: DELAY.CPP							*/
/*=========================================================================*/
void Delay(int duration);
void Vsync(void);


/*========================= Assembly Routines ==============================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=========================================================================*/
/* The following prototype is for the file: SHAKESCR.ASM							*/
/*=========================================================================*/

void __cdecl Shake_Screen(int shakes);

//void * Build_Fading_Table(PaletteClass const & palette, void * dest, int color, int frac);
//void * __cdecl Build_Fading_Table(void const *palette, void const *dest, long int color, long int frac);

/*=========================================================================*/
/* The following prototypes are for the file: DETPROC.ASM						*/
/*=========================================================================*/

extern WORD __cdecl  Processor(void);
extern WORD __cdecl Operating_System(void);



/*=========================================================================*/
/* The following prototypes are for the file: OPSYS.ASM							*/
/*=========================================================================*/

extern WORD OperationgSystem;

#ifdef __cplusplus
}
#endif

/*=========================================================================*/
