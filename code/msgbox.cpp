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
#include "globals.h"
#include "init.h"
#include "ownrdraw.h"
#include "winfix.h"

INT_PTR CALLBACK Message_Box_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
void Message_Box_On_WM_COMMAND(HWND window, int id, int control, int notify_code);

int _default_response = 0;

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
#define	BUTTON_1		IDC_MSGBOX_OK
#define	BUTTON_2		IDCANCEL
#define	BUTTON_3		IDC_MSGBOX_BTN3
#define	BUTTON_FLAG	0x8000
int WWMessageBox::_Process(const char * msg, int defresponse, const char * b1txt, const char * b2txt, const char * b3txt, bool preserve)
{
	int retval = -1;
	int numbuttons = 0;

	_default_response = defresponse;

	HWND dialog = OwnerDraw::Begin_Dialog(IDD_MSGBOX_3, Message_Box_Proc);

	if (dialog != NULL) {
		SetWindowLongPtr(dialog, GWLP_USERDATA, (LONG_PTR)&retval);

		if (msg != NULL && msg[0] != '\0') {
			SetDlgItemText(dialog, IDC_MSGBOX_TEXT, msg);
		}

		if (b1txt != NULL && b1txt[0] != '\0') {
			SetDlgItemText(dialog, BUTTON_1, b1txt);
			numbuttons = 1;
		} else {
			EnableWindow(GetDlgItem(dialog, BUTTON_1), FALSE);
			ShowWindow(GetDlgItem(dialog, BUTTON_1), SW_HIDE);
		}

		if (b2txt != NULL && b2txt[0] != '\0') {
			SetDlgItemText(dialog, BUTTON_2, b2txt);
			numbuttons = 2;
		} else {
			EnableWindow(GetDlgItem(dialog, BUTTON_2), FALSE);
			ShowWindow(GetDlgItem(dialog, BUTTON_2), SW_HIDE);
		}

		if (b3txt != NULL && b3txt[0] != '\0') {
			SetDlgItemText(dialog, BUTTON_3, b3txt);
			numbuttons = 3;
		} else {
			EnableWindow(GetDlgItem(dialog, BUTTON_3), FALSE);
			ShowWindow(GetDlgItem(dialog, BUTTON_3), SW_HIDE);

			if (numbuttons == 1) {
				RECT rect;
				GetWindowRect(GetDlgItem(dialog, BUTTON_3), &rect);
				ScreenToClient(dialog, (LPPOINT)&rect);
				ScreenToClient(dialog, (LPPOINT)&rect.right);
				MoveWindow(GetDlgItem(dialog, BUTTON_1), rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, FALSE);
			}
		}

		OwnerDraw::Display_Dialog(dialog);

		if (numbuttons > 0) {
			while (retval < 0) {
				if (OwnerDraw::Dialog_Message_Handler() == true) {
					break;
				}

				if (!GameActive) {
					Title_Screen_Restore();
				}
			}
		} else {
			retval = 0;
		}

		OwnerDraw::End_Dialog(dialog);
	}

	return(retval);
}


/// <summary>
/// Handles the dialog messages for the message box.
/// This routine gives the owner draw system first crack at every message and only steps
/// in for the button notifications and the window dragging that it does not already deal
/// with.
/// </summary>
/// <returns>BOOL; Was the message handled here?</returns>
INT_PTR CALLBACK Message_Box_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);

	if (rc == FALSE) {
		switch (message) {

			case WM_COMMAND:
				Message_Box_On_WM_COMMAND(window, LOWORD(wparam), 0, HIWORD(wparam));
				rc = FALSE;
				break;

			case WM_MOVING:
				rc = On_WM_MOVING(window, wparam, lparam);
				break;

			default:
				rc = FALSE;
				break;
		}
	}

	return(rc);
}


/// <summary>
/// Handles a button press within the message box dialog.
/// This routine records which of the buttons the player picked in the result slot that
/// the message box is waiting on. The Enter key arrives here as IDOK and yields the
/// default response the caller asked for.
/// </summary>
/// <param name="id">The identifier of the control that sent the notification.</param>
void Message_Box_On_WM_COMMAND(HWND window, int id, int control, int notify_code)
{
	int *retval = (int*)GetWindowLongPtr(window, GWLP_USERDATA);
	switch (id) {

		case IDOK:
			if (notify_code == BN_CLICKED) {
				*retval = _default_response;
			}
			break;

		case BUTTON_1:
			if (notify_code == BN_CLICKED) {
				*retval = 0;
			}
			break;

		case IDCANCEL:
			if (notify_code == BN_CLICKED) {
				*retval = 1;
			}
			break;

		case BUTTON_3:
			if (notify_code == BN_CLICKED) {
				*retval = 2;
			}
			break;
	}
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
