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

/* $Header: /CounterStrike/MSGBOX.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OPTIONS.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 8, 1994                                                 *
 *                                                                                             *
 *                  Last Update : June 8, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "language/language.h"
#include "win.h"

class WWMessageBox
{
		int Caption;

	public:
		WWMessageBox(int caption=TXT_NONE) {Caption = caption;};

		/*
		 * Convenience forwarders; the Enter key answers with button 0.
		 */
		int Process(int msg, int b1txt) { return(_Process(msg, 0, b1txt, TXT_NONE, TXT_NONE, false)); }
		int Process(int msg, int b1txt, int b2txt, bool preserve=false) { return(_Process(msg, 0, b1txt, b2txt, TXT_NONE, preserve)); }
		int Process(int msg, int b1txt, int b2txt, int b3txt, bool preserve=false) { return(_Process(msg, 0, b1txt, b2txt, b3txt, preserve)); }
		int Process(const char * msg, int b1txt) { return(_Process(msg, 0, b1txt, TXT_NONE, TXT_NONE, false)); }
		int Process(char const * msg, int b1txt, int b2txt, bool preserve=false) { return(_Process(msg, 0, b1txt, b2txt, TXT_NONE, preserve)); }
		int Process(char const * msg, int b1txt, int b2txt, int b3txt, bool preserve=false) { return(_Process(msg, 0, b1txt, b2txt, b3txt, preserve)); }

		/*
		 * 'defresponse' is the button index the box returns when the player
		 * answers with the Enter key (IDOK) instead of clicking a button.
		 * These carry a distinct name because overloading them against the
		 * wrappers above would be ambiguous.
		 */
		int _Process(const char *msg, int defresponse, const char *b1txt, const char *b2txt=NULL, const char *b3txt=NULL, bool preserve=false);
		int _Process(int msg, int defresponse, int b1txt, int b2txt=TXT_NONE, int b3txt=TXT_NONE, bool preserve=false);
		int _Process(char const *msg, int defresponse, int b1txt=TXT_OK, int b2txt=TXT_NONE, int b3txt=TXT_NONE, bool preserve=false);
};
