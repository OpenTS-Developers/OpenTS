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
 *                     $Archive:: /Renegade Setup/Autorun/WinFix.H                            $*
 *                                                                                             *
 *                      $Author:: Maria_l                                                     $*
 *                                                                                             *
 *                     $Modtime:: 11/07/01 5:57p                                              $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WindowsVersionInfo::Major -- Get the major version of the OS                              *
 *   WindowsVersionInfo::Minor -- Get the minor version of the OS                              *
 *   WindowsVersionInfo::Build -- Get the build level of the OS                                *
 *   WindowsVersionInfo::Info -- Get additional system information                             *
 *   WindowsVersionInfo::Is_Win9x -- Determine if we are running on a non-NT system.           *
 *   WindowsVersionInfo::Is_Win95 -- Determine if we are running on a Win95 system.            *
 *   WindowsVersionInfo::Is_Win98 -- Determine if we are running on a Win98 system.            *
 *   WindowsVersionInfo::Is_WinNT -- Determine if we are running on an NT system.              *
 *   WindowsVersionInfo::Is_WinNT5 -- Determine if we are running on an NT 5 system.           *
 *   WindowsVersionInfo::Version   --                                                          *
 *   WindowsVersionInfo::IsOSR2Release --                                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "win.h"

#include <commctrl.h>

/*
 * Custom messages for the TreeView drag-and-drop helpers below. The helpers
 * send these to the dialog that owns the TreeView so it can approve and act on
 * a drag. wParam/lParam are documented per message; all expect the receiver to
 * return a BOOL where noted.
 */
#define TVDRAG_OVER		(WM_USER + 200)		/// drag-over hit-test. wParam = item_data*, lParam = cursor xy. Returns BOOL (valid drop target).
#define TVDRAG_BEGIN	(WM_USER + 201)		/// begin-drag query.  wParam = HWND tree,   lParam = HTREEITEM.  Returns BOOL (allow drag).
#define TVDRAG_DROP		(WM_USER + 202)		/// drop on a valid target. wParam = item_data*, lParam = cursor xy.

void Center_Window_Within_Window(HWND window);
void Center_Window_Within_Window(HWND window, HWND parent);

BOOL On_WM_HELP(LPARAM help_info);
BOOL On_WM_CONTEXTMENU(WPARAM window);
bool On_WM_MOVING(HWND window, WPARAM wparam, LPARAM lparam);

int ComboBox_Find_Item_Data(HWND hwndCtl, WPARAM indexStart, LPARAM data);

void Prefetch_Audio_Buffer(char *src, int size);

#define Slider_GetPos(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETPOS, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetRangeMin(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETRANGEMIN, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetRangeMax(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETRANGEMAX, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetTic(hwndCtl, index) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETTIC, (WPARAM)(index), (LPARAM)(0L)))
#define Slider_SetTic(hwndCtl, position) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETTIC, (WPARAM)(0L), (LPARAM)(position)))
#define Slider_SetPos(hwndCtl, position) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETPOS, (WPARAM)(TRUE), (LPARAM)(position)))
#define Slider_SetRange(hwndCtl, min, max) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETRANGE, (WPARAM)(TRUE), (LPARAM)(MAKELONG(min, max))))
#define Slider_SetRangeMin(hwndCtl, min) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETRANGEMIN, (WPARAM)(TRUE), (LPARAM)(min)))
#define Slider_SetRangeMax(hwndCtl, max) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETRANGEMAX, (WPARAM)(TRUE), (LPARAM)(max)))
#define Slider_ClearTics(hwndCtl, redraw) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_CLEARTICS, (WPARAM)(redraw), (LPARAM)(0L)))
#define Slider_SetSel(hwndCtl, minSel, maxSel) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETSEL, (WPARAM)(TRUE), (LPARAM)(MAKELONG(minSel, maxSel))))
#define Slider_SetSelStart(hwndCtl, start) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETSELSTART, (WPARAM)(0L), (LPARAM)(start)))
#define Slider_SetSelEnd(hwndCtl, end) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETSELEND, (WPARAM)(0L), (LPARAM)(end)))
#define Slider_GetPTics(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETPTICS, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetTicPos(hwndCtl, index) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETTICPOS, (WPARAM)(index), (LPARAM)(0L)))
#define Slider_GetNumTics(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETNUMTICS, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetSelStart(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETSELSTART, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetSelEnd(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETSELEND, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_ClearSel(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_CLEARSEL, (WPARAM)(TRUE), (LPARAM)(0L)))
#define Slider_SetTicFreq(hwndCtl, freq) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETTICFREQ, (WPARAM)(freq), (LPARAM)(0L)))
#define Slider_SetPageSize(hwndCtl, pagesize) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETPAGESIZE, (WPARAM)(0L), (LPARAM)(pagesize)))
#define Slider_GetPageSize(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETPAGESIZE, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_SetLineSize(hwndCtl, linesize) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETLINESIZE, (WPARAM)(0L), (LPARAM)(linesize)))
#define Slider_GetLineSize(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETLINESIZE, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_GetThumbRect(hwndCtl, lprc) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETTHUMBRECT, (WPARAM)(0L), (LPARAM)(lprc)))
#define Slider_GetChannelRect(hwndCtl, lprc) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETCHANNELRECT, (WPARAM)(0L), (LPARAM)(lprc)))
#define Slider_SetThumbLength(hwndCtl, length) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETTHUMBLENGTH, (WPARAM)(0L), (LPARAM)(length)))
#define Slider_GetThumbLength(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETTHUMBLENGTH, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_SetToolTips(hwndCtl, hwndTT) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETTOOLTIPS, (WPARAM)(0L), (LPARAM)(hwndTT)))
#define Slider_GetToolTips(hwndCtl) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETTOOLTIPS, (WPARAM)(0L), (LPARAM)(0L)))
#define Slider_SetTipSide(hwndCtl, side) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETTIPSIDE, (WPARAM)(side), (LPARAM)(0L)))
#define Slider_SetBuddy(hwndCtl, fLeft, hwndBuddy) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_SETBUDDY, (WPARAM)(fLeft), (LPARAM)(hwndBuddy)))
#define Slider_GetBuddy(hwndCtl, fLeft) ((LRESULT)(DWORD)SNDMSG((hwndCtl), TBM_GETBUDDY, (WPARAM)(fLeft), (LPARAM)(0L)))


inline void SetSliderRangeAndPos(HWND handle, unsigned short min, unsigned short max, int pos)
{
	Slider_SetRange(handle, min, max);
	Slider_SetPos(handle, pos);
}

#define _SetSliderRangeAndPos(handle, min, max, pos) \
{ \
	Slider_SetRange(handle, min, max); \
	Slider_SetPos(handle, pos); \
} \

/// These were added to the platform headers after the version this project builds against,
/// so each is defined here when it is missing.

#ifndef WC_LISTBOX

/// Listbox Class Name
#define WC_LISTBOXA		"ListBox"
#define WC_LISTBOXW		L"ListBox"

#ifdef UNICODE
#define WC_LISTBOX		WC_LISTBOXW
#else
#define WC_LISTBOX		WC_LISTBOXA
#endif

#endif /// WC_LISTBOX

#ifndef WC_SCROLLBAR

#define WC_STATICA		"Static"
#define WC_STATICW		L"Static"

#ifdef UNICODE
#define WC_STATIC		WC_STATICW
#else
#define WC_STATIC		WC_STATICA
#endif

#endif /// WC_SCROLLBAR

#ifndef WC_SCROLLBAR

#define WC_SCROLLBARA	"ScrollBar"
#define WC_SCROLLBARW	L"ScrollBar"

#ifdef UNICODE
#define WC_SCROLLBAR	WC_SCROLLBARW
#else
#define WC_SCROLLBAR	WC_SCROLLBARA
#endif

#endif /// WC_SCROLLBAR

#ifndef WC_COMBOBOX

#define WC_COMBOBOXA	"ComboBox"
#define WC_COMBOBOXW	L"ComboBox"

#ifdef UNICODE
#define WC_COMBOBOX		WC_COMBOBOXW
#else
#define WC_COMBOBOX		WC_COMBOBOXA
#endif

#endif /// WC_COMBOBOX


#ifndef WC_BUTTON

#define WC_BUTTONA		"Button"
#define WC_BUTTONW		L"Button"

#ifdef UNICODE
#define WC_BUTTON		WC_BUTTONW
#else
#define WC_BUTTON		WC_BUTTONA
#endif

#endif /// WC_BUTTON


#ifndef WC_EDIT

#define WC_EDITA		"Edit"
#define WC_EDITW		L"Edit"

#ifdef UNICODE
#define WC_EDIT			WC_EDITW
#else
#define WC_EDIT			WC_EDITA
#endif

#endif /// WC_EDIT
