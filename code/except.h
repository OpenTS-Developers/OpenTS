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
 *                     $Archive:: /Commando/Code/wwlib/Except.h                               $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/07/02 12:28p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "win.h"

#ifdef _WIN32
#include <sal.h>
#endif

// Posted to the main window so that a requested test fault happens inside window procedure
// dispatch, which the operating system unwinds differently from an ordinary call.
#define WM_EXCEPTION_TEST (WM_APP + 0x54)

// Exception codes OpenTS raises itself. Routing an engine error through RaiseException rather
// than reporting it in place is what gives the handler a genuine machine context to dump: a
// terminate or pure call handler is entered with none.
#define EXCEPTION_OPENTS_FATAL				0xE0545301
#define EXCEPTION_OPENTS_TERMINATE			0xE0545302
#define EXCEPTION_OPENTS_PURECALL			0xE0545303
#define EXCEPTION_OPENTS_INVALID_PARAMETER	0xE0545304

void Install_Exception_Handler(void);
void Exception_Register_Log_File(char const * path);

void Exception_Set_Test_Mode(char const * mode);
void Exception_Run_Immediate_Test(void);
void Exception_Run_Post_Window_Test(void);
void Exception_Wndproc_Test_Fault(void);

void __cdecl Fatal(_Printf_format_string_ char const * message, ...);
