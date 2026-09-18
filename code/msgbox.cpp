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

/* $Header: /CounterStrike/MSGBOX.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : OPTIONS.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : June 8, 1994                                                 *
 *                                                                                             *
 *                  Last Update : August 24, 1995 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WWMessageBox::Process -- Handles all the options graphic interface.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "msgbox.h"

#include "data.h"
#include "ui/screens/msgbox/uimsgbox.h"


/***********************************************************************************************
 * WWMessageBox::Process -- pops up a message with yes/no, etc                                 *
 *                                                                                             *
 * This function displays a dialog box with a one-line message, and                            *
 * up to two buttons. The 2nd button is optional. The buttons default                          *
 * to "OK" and nothing, respectively. The hotkeys for the buttons are                          *
 * RETURN and ESCAPE.                                                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      msg         message to display                                                         *
 *      b1txt         text for 1st button                                                      *
 *      b2txt         text for 2nd button; NULL = no 2nd button                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      # of button selected (0 = 1st)                                                         *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      'msg' text must be <= 38 chars                                                         *
 *      'b1txt' and 'b2txt' must each be <= 18 chars                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/08/1994 BR : Created.                                                                  *
 *   05/18/1995 JLB : Uses new font and dialog style.                                          *
 *   08/24/1995 JLB : Handles three buttons.                                                   *
 *=============================================================================================*/
int WWMessageBox::_Process(const char * msg, int defresponse, const char * b1txt, const char * b2txt, const char * b3txt, bool preserve)
{
	return(UI_Message_Box(msg, defresponse, b1txt, b2txt, b3txt));
}


/***********************************************************************************************
 * WWMessageBox::Process -- this one takes integer text arguments                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *      msg         message to display                                                         *
 *      b1txt         text for 1st button                                                      *
 *      b2txt         text for 2nd button; NULL = no 2nd button                                *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *      # of button selected (0 = 1st)                                                         *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *      'msg' text must be <= 38 chars                                                         *
 *      'b1txt' and 'b2txt' must each be <= 18 chars                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/12/1994 BR : Created.                                                                  *
 *   06/18/1995 JLB : Simplified.                                                              *
 *=============================================================================================*/
int WWMessageBox::_Process(int msg, int defresponse, int b1txt, int b2txt, int b3txt, bool preserve)
{
	const char *message = Fetch_String(msg);
	return(_Process(message, defresponse, Fetch_String(b1txt), Fetch_String(b2txt), Fetch_String(b3txt), preserve));
}


/***********************************************************************************************
 * WWMessageBox::Process -- Displays message box.                                              *
 *                                                                                             *
 *    This routine will display a message box and wait for player input. It varies from the    *
 *    other message box functions only in the type of parameters it takes.                     *
 *                                                                                             *
 * INPUT:   msg   -- Pointer to text string for the message itself.                            *
 *                                                                                             *
 *          b1txt -- Text number for the "ok" button.                                          *
 *                                                                                             *
 *          b2txt -- Text number for the "cancel" button.                                      *
 *                                                                                             *
 * OUTPUT:  Returns with the button selected. "true" if "OK" was pressed, and "false" if       *
 *          "CANCEL" was pressed.                                                              *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int WWMessageBox::_Process(char const * msg, int defresponse, int b1txt, int b2txt, int b3txt, bool preserve)
{
	return(_Process(msg, defresponse, Fetch_String(b1txt), Fetch_String(b2txt), Fetch_String(b3txt), preserve));
}
