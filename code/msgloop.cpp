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
 *                     $Archive:: /Commando/Code/wwlib/msgloop.cpp                            $*
 *                                                                                             *
 *                      $Author:: Steve_t                                                     $*
 *                                                                                             *
 *                     $Modtime:: 2/05/02 1:17p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Add_Accelerator -- Adds a keyboard accelerator to the message handler.                    *
 *   Add_Modeless_Dialog -- Adds a modeless dialog box to the message handler.                 *
 *   Remove_Accelerator -- Removes an accelerator from the message processor.                  *
 *   Remove_Modeless_Dialog -- Removes the dialog box from the message tracking handler.       *
 *   Windows_Message_Handler -- Handles windows message.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "msgloop.h"

#include "_tooltip.h"
#include "_ui.h"
#include "cctooltip.h"
#include "vector.h"
#include "video.h"


/***********************************************************************************************
 * Windows_Message_Handler -- Handles windows message.                                         *
 *                                                                                             *
 *    This routine will take all messages that have accumulated in the message queue and       *
 *    dispatch them to their respective recipients. When the message queue has been emptied,   *
 *    then this routine will return. By using this routine, it is possible to have the main    *
 *    program run in the main thread and yet still have it behave like a normal program as     *
 *    far as message handling is concerned. To achieve this, this routine must be called on    *
 *    a semi-frequent basis (a few times a second is plenty).                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/17/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void Windows_Message_Handler(void)
{
	if (MainWindow == 0) return;

	MSG msg;

	/*
	**	Process windows messages until the message queue is exhuasted.
	*/
	while (PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE)) {
		if (!GetMessageW( &msg, NULL, 0, 0 )) {
			return;
		}

		/*
		**	If the message makes it to this point, then it must be a normal message. Process
		**	it in the normal fashion. The message will appear in the window message handler
		**	for the window that it was directed to.
		*/
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	if (ToolTips != NULL) {
		ToolTips->Service();
	}

	/*
	 * The menus, the loading screens and the score screens all draw and then come back
	 * here rather than through the game's own present, so this is where their frames
	 * reach the screen.
	 */
	Video_Present_If_Dirty();
}
