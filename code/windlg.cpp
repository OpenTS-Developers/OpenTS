/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "windlg.h"

#include "arraylist.h"
#include "data.h"
#include "globals.h"
#include "init.h"
#include "msgloop.h"
#include "ownrdraw.h"
#include "video.h"
#include "win.h"

#include <commctrl.h>
#include <windows.h>


BOOL CALLBACK Resize_Dialog(HWND window, LPARAM lParam);
BOOL CALLBACK Save_Control_Value_Enum_Proc(HWND window, LPARAM lParam);
void WS_Save_Dialog_Values(HWND window);
void WS_Save_Control_Value(int control_id, unsigned char * data, int size);


HWND g_TopWindow;
int g_TopWindowID;
int g_LastResponse;

WSDialogStruct g_Dialogs[64];
int g_DialogCount;


/// <summary>
/// Fetches a window's rectangle relative to the main game window.
/// The dialog layout code works in the main window's client space rather than in screen
/// coordinates, so it uses this routine in place of GetWindowRect.
/// </summary>
/// <param name="rect">Receives the window rectangle, offset into the main window's
/// client area.</param>
/// <returns>bool; Was the window rectangle available?</returns>
BOOL Get_Display_Rect(HWND window, LPRECT rect)
{
	RECT client;
	BOOL res = GetWindowRect(window, rect);
	if (!res) {
		return(res);
	}
	GetClientRect(MainWindow, &client);
	ClientToScreen(MainWindow, (LPPOINT)&client);
	rect->left -= client.left;
	rect->right -= client.left;
	rect->top -= client.top;
	rect->bottom -= client.top;
	return(res);
}


/// <summary>
/// Finds the stack slot a dialog window occupies.
/// The dialog bookkeeping routines use this routine to turn a window handle back into a
/// position in the dialog stack.
/// </summary>
/// <returns>Returns with the index of the dialog, or -1 if the window is not a tracked
/// dialog.</returns>
inline int WS_Dialog_Index(HWND window)
{
	for (int i = 0; i < g_DialogCount; i++) {
		if (g_Dialogs[i].handle == window) {
			return(i);
		}
	}
	return(-1);
}


/// <summary>
/// Creates a dialog and pushes it onto the dialog stack.
/// This routine builds the dialog from its resource template, registers it with the
/// message loop so its keystrokes are routed properly, rescales it to the presentation
/// layout, and leaves it as the topmost dialog with the focus.
/// </summary>
/// <param name="instance">The module instance to load the dialog template from.</param>
/// <param name="id">The resource identifier of the dialog template.</param>
/// <param name="parent">The window the dialog is to be parented to.</param>
/// <param name="proc">The dialog procedure that messages are routed to.</param>
/// <param name="force_show">Should the dialog be made visible straight away?</param>
/// <returns>Returns with the window handle of the new dialog. If the template is
/// missing or the dialog could not be created, NULL is returned.</returns>
HWND WS_Create_Dialog(HINSTANCE instance, int id, HWND parent, DLGPROC proc, BOOL force_show)
{
	WSDialogStruct *slot = &g_Dialogs[g_DialogCount];
	g_Dialogs[g_DialogCount].handle = 0;
	g_Dialogs[g_DialogCount].id = 0;

	LPCDLGTEMPLATE templ = (LPCDLGTEMPLATE)Fetch_Resource((LPCSTR)id, (LPCSTR)RT_DIALOG);

	if (templ == NULL) {
		return(NULL);
	}

	g_DialogCount++;

	HWND window = CreateDialogIndirectParam(instance, (LPCDLGTEMPLATE)templ, parent, proc, 0);

	if (window == NULL) {
		g_DialogCount--;
		return(NULL);
	}

	_dialog_count++;

	Add_Modeless_Dialog(window);

	EnumChildWindows(window, Resize_Dialog, TRUE);

	Resize_Dialog(window, 0);

	OwnerDraw::Capture_Mouse();

	slot->handle = window;
	slot->id = id;

	if (force_show) {
		ShowWindow(window, SW_SHOWNORMAL);
	}

	SetForegroundWindow(window);
	SetFocus(window);
	g_TopWindow = window;
	g_TopWindowID = id;
	return(window);
}


/// <summary>
/// Closes a dialog along with everything stacked on top of it.
/// This routine records the dialog's control values before it goes, so they can still
/// be read back with WS_Get_Saved_Value, then unregisters and destroys it. Whichever dialog is
/// left underneath becomes the topmost one again and is given back the focus.
/// </summary>
/// <param name="window">The dialog to close. If this is NULL, the topmost dialog is
/// closed.</param>
/// <param name="id">The response to report to whoever is waiting on this dialog.</param>
/// <returns>bool; Was a dialog found and closed?</returns>
bool WS_Destroy_Dialog(HWND window, int id)
{
	if (window == NULL) {
		if (g_DialogCount != 0) {
			window = g_Dialogs[g_DialogCount - 1].handle;
			if (window == NULL) {
				return(false);
			}
		} else {
			return(false);
		}
	}

	int index = WS_Dialog_Index(window);
	if (index == -1) {
		return(false);
	}

	WS_Save_Dialog_Values(window);

	int last = g_DialogCount - 1;
	if (g_DialogCount - 1 >= index) {
		WSDialogStruct *dlg = &g_Dialogs[last];
		int count = last - index + 1;
		do {
			Remove_Modeless_Dialog(dlg->handle);
			DestroyWindow(dlg->handle);
			_dialog_count--;
			OwnerDraw::Release_Mouse();
			dlg--;
			count--;
		} while (count);
	}

	g_DialogCount = index;

	if (g_DialogCount != 0) {
		HWND hwnd = g_Dialogs[g_DialogCount - 1].handle;
		int id = g_Dialogs[g_DialogCount - 1].id;
		SetForegroundWindow(hwnd);
		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);
		g_TopWindow = hwnd;
		g_TopWindowID = id;
		SetFocus(hwnd);
		SendMessage(g_TopWindow, OD_REFOCUS, 0, 0);
	} else {
		g_TopWindow = 0;
		g_TopWindowID = 0;
		SetFocus(MainWindow);
	}
	g_LastResponse = id;

	return(true);
}


/// <summary>
/// Is this window one of the dialogs still open?
/// The wait loop uses this routine to tell when the dialog it is watching over has
/// finally been destroyed.
/// </summary>
/// <returns>bool; Is the window still on the dialog stack?</returns>
BOOL WS_Has_Dialog(HWND window)
{
	for (int i = 0; i < g_DialogCount; i++) {
		if (g_Dialogs[i].handle == window) {
			return(true);
		}
	}
	return(false);
}


/// <summary>
/// Finds an open dialog by its resource identifier.
/// Should the same dialog happen to be open more than once, the one nearest the top of
/// the stack is the one found.
/// </summary>
/// <param name="id">The dialog resource identifier to search for.</param>
/// <returns>Returns with the window handle of the dialog, or NULL if no such dialog is
/// open.</returns>
HWND WS_Find_Dialog(int id)
{
	for (int i = g_DialogCount - 1; i >= 0; i--) {
		if (id == g_Dialogs[i].id) {
			return(g_Dialogs[i].handle);
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the dialog sitting above the one given.
/// </summary>
/// <returns>Returns with the window handle of the next dialog up the stack, or NULL if
/// the dialog given is the topmost one or is not tracked at all.</returns>
HWND WS_Next_Upper_Dialog(HWND window)
{
	for (int i = 0; i < g_DialogCount; i++) {
		if (window == g_Dialogs[i].handle && i < g_DialogCount - 1) {
			return(g_Dialogs[i + 1].handle);
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the dialog sitting below the one given.
/// </summary>
/// <returns>Returns with the window handle of the next dialog down the stack, or NULL
/// if the dialog given is the bottom one or is not tracked at all.</returns>
HWND WS_Next_Lower_Dialog(HWND window)
{
	for (int i = g_DialogCount - 1; i >= 0; i--) {
		if (window == g_Dialogs[i].handle && i > 0) {
			return(g_Dialogs[i - 1].handle);
		}
	}
	return(NULL);
}


/// <summary>
/// Waits until a dialog has been dismissed.
/// Use this routine to make one of these modeless dialogs behave as a modal one. It
/// pumps the message queue on the dialog's behalf, keeps the title screen refreshed,
/// and polls the abort callback, returning only once the dialog is gone.
/// </summary>
/// <param name="callback">Optional routine polled on every pass. Should it return true,
/// the dialog is cancelled. May be NULL.</param>
/// <param name="place_on_top">Should the dialog be forced to the front for the duration
/// of the wait?</param>
/// <returns>Returns with the response the dialog was closed with.</returns>
int WS_Wait_Dialog(HWND window, bool (*callback)(void), bool, bool place_on_top)
{
	MSG msg;

	if (place_on_top) {
		SetForegroundWindow(window);
		SendMessage(window, OD_SETTOP, 0, TRUE);
	}

	while (true) {
		if (!WS_Has_Dialog(window)) {
			break;
		}
		if (callback != NULL) {
			if (callback() == TRUE) {
				WS_Destroy_Dialog(window, IDCANCEL);
			}
		}
		Title_Screen_Restore(false);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (!WS_Has_Dialog(window)) {
				break;
			}
		}

		/*
		 * This loop pumps messages itself rather than going through the game's handler,
		 * so anything the dialog drew reaches the screen from here.
		 */
		Video_Present_If_Dirty();

		Sleep(0);
	}

	if (place_on_top) {
		SendMessage(window, OD_SETTOP, 0, FALSE);
	}

	return(g_LastResponse);
}


/// <summary>
/// Fetches the dialog currently on top of the stack.
/// </summary>
/// <returns>Returns with the window handle of the topmost dialog, or NULL if no dialog
/// is up.</returns>
HWND WS_Top_Window(void)
{
	return(g_TopWindow);
}


/// <summary>
/// Fetches the resource identifier of the topmost dialog.
/// </summary>
/// <returns>Returns with the dialog identifier, or zero if no dialog is up.</returns>
int WS_Top_Window_ID(void)
{
	return(g_TopWindowID);
}


/// <summary>
/// Fetches the response the last dialog was closed with.
/// </summary>
/// <returns>Returns with the identifier handed to WS_Destroy_Dialog when the most
/// recently closed dialog went away.</returns>
WPARAM WS_Last_Response(void)
{
	return(g_LastResponse);
}


/// <summary>
/// Records the values of every control in a dialog.
/// This routine is called as a dialog is being closed, so that the caller can still
/// interrogate its controls afterwards with WS_Get_Saved_Value. Whatever was recorded for a
/// previous dialog is discarded first.
/// </summary>
void WS_Save_Dialog_Values(HWND window)
{
	WS_Clear_Saved_Values();
	EnumChildWindows(window, Save_Control_Value_Enum_Proc, NULL);
}


/// <summary>
/// Records the value of one dialog control.
/// This routine is handed to EnumChildWindows by WS_Save_Dialog_Values. An edit box contributes
/// its text, while a button, slider or combo box contributes its current setting.
/// Anything else is passed over without comment.
/// </summary>
/// <returns>Always TRUE, so that child window enumeration carries on.</returns>
BOOL CALLBACK Save_Control_Value_Enum_Proc(HWND window, LPARAM lParam)
{
	char class_name[128];

	GetClassName(window, class_name, sizeof(class_name));
	unsigned int id = GetWindowLong(window, GWL_ID);

	if (!strcmp(class_name, WC_EDIT)) {
		int size = SendMessage(window, WM_GETTEXTLENGTH, 0, 0) + 1;
		if (size > 257) {
			size = 257;
		}

		unsigned char *buf = new unsigned char[size];
		SendMessage(window, WM_GETTEXT, size, (LONG)buf);
		buf[size - 1] = '\0';
		WS_Save_Control_Value(id, buf, size);
		return(TRUE);
	}

	if (!strcmp(class_name, WC_BUTTON)) {
		int *i = new int;
		*i = Button_GetCheck(window);
		WS_Save_Control_Value(id, (unsigned char *)i, sizeof(*i));
		return(TRUE);
	}

	if (!strcmp(class_name, TRACKBAR_CLASS)) {
		int *i = new int;
		*i = Slider_GetPos(window);
		WS_Save_Control_Value(id, (unsigned char *)i, sizeof(*i));
		return(TRUE);
	}

	if (!strcmp(class_name, WC_COMBOBOX)) {
		int *i = new int;
		*i = ComboBox_GetCurSel(window);
		WS_Save_Control_Value(id, (unsigned char *)i, sizeof(*i));
		return(TRUE);
	}

	return(TRUE);
}


ArrayList<int> g_SavedValueIDs;
ArrayList<int> g_SavedValueSizes;
ArrayList<unsigned char *> g_SavedValues;


/// <summary>
/// Records the value of a single dialog control.
/// This routine is the low level record keeper behind WS_Save_Dialog_Values. The value
/// survives the dialog it came from and is handed back later by WS_Get_Saved_Value.
/// </summary>
/// <param name="control_id">The control identifier the value belongs to.</param>
/// <param name="data">Pointer to the value data.</param>
/// <param name="size">The length of the value data, in bytes.</param>
/// <remarks>The value block is adopted by the record and is freed by
/// WS_Clear_Saved_Values. It must be allocated, never a temporary buffer.</remarks>
void WS_Save_Control_Value(int control_id, unsigned char * data, int size)
{
	g_SavedValueIDs.addTail(control_id);
	g_SavedValues.addTail(data);
	g_SavedValueSizes.addTail(size);
}


/// <summary>
/// Fetches the recorded value of a dialog control.
/// Use this routine after a dialog has been closed, when the caller still needs to know
/// what the user left behind in one of its controls.
/// </summary>
/// <param name="control_id">The control identifier whose value is wanted.</param>
/// <param name="dest">Buffer that the recorded value is copied into.</param>
/// <param name="dest_size">The size of the destination buffer, in bytes.</param>
/// <returns>Returns with the number of bytes copied. If no value was recorded for that
/// control, -1 is returned.</returns>
int WS_Get_Saved_Value(int control_id, unsigned char * dest, int dest_size)
{
	int entry_id = 0;
	unsigned char *saved = NULL;
	int saved_size = 0;

	for (int index = 0; index < g_SavedValueIDs.length(); index++) {
		g_SavedValueIDs.get(entry_id, index);
		if (entry_id == control_id) {
			if (index >= 0) {
				g_SavedValues.get(saved, index);
				g_SavedValueSizes.get(saved_size, index);
			}
			if (saved_size < dest_size) {
				dest_size = saved_size;
			}
			memcpy(dest, saved, dest_size);
			return(dest_size);
		}
	}
	return(-1);
}


/// <summary>
/// Discards every recorded dialog control value.
/// This routine frees the value blocks captured by WS_Save_Dialog_Values and empties the
/// record, leaving it ready for the next dialog that gets torn down.
/// </summary>
void WS_Clear_Saved_Values(void)
{
	unsigned char *data = NULL;

	for (int index = 0; index < g_SavedValueIDs.length(); index++) {
		g_SavedValues.get(data, index);
		delete data;
	}
	g_SavedValueIDs.clear();
	g_SavedValueSizes.clear();
	g_SavedValues.clear();
}


/// <summary>
/// Handles messages for the layout reference dialog.
/// This routine handles nothing whatsoever. The reference dialog is created only to be
/// measured and is destroyed again immediately, so every message is left to the default
/// handling.
/// </summary>
BOOL CALLBACK Resize_Dialog_Proc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	// nothing
	return(FALSE);
}


/// <summary>
/// Fetches the client dimensions a dialog template was laid out at.
/// This routine creates the dialog just long enough to measure it and then throws it
/// away. The layout scaling code uses it to discover the resolution a template was
/// designed against.
/// </summary>
/// <param name="template_id">The resource identifier of the dialog template to
/// measure.</param>
/// <param name="pt">Receives the width and height of the dialog's client area.</param>
BOOL Get_Dialog_Resolution(unsigned short template_id, BOOL (__stdcall *dialog_proc)(HWND, UINT, WPARAM, LPARAM), int, POINT &pt)
{
	HWND window;
	tagRECT rcl;

	window = CreateDialogParam(ProgramInstance, (LPCSTR)template_id, 0, dialog_proc, 0);
	GetClientRect(window, &rcl);
	DestroyWindow(window);
	pt.x = rcl.right;
	pt.y = rcl.bottom;
	return(1);
}


/// <summary>
/// Rescales a dialog and every control it holds.
/// Use this routine after a dialog has been created, or whenever its layout has to be
/// rebuilt for the presentation size currently in force.
/// </summary>
void Resize_Dialogs(HWND window)
{
	EnumChildWindows(window, Resize_Dialog, 1);
	Resize_Dialog(window, 0);
}


/// <summary>
/// Rescales a dialog or one of its controls to the presentation layout.
/// This routine is handed to EnumChildWindows by the dialog creation code and by
/// Resize_Dialogs, so that every control is carried from the coordinate space its
/// template was designed in over to the one dialogs are actually presented in.
/// </summary>
/// <param name="lParam">Should the window rectangle be taken relative to its parent?
/// This is set when enumerating child controls, and clear for the dialog itself.</param>
/// <returns>Always TRUE, so that child window enumeration carries on.</returns>
BOOL CALLBACK Resize_Dialog(HWND window, LPARAM lParam)
{
	static int resize_dialog_width;
	static int resize_dialog_height;
	static int resize_dialog_scale_x = 300;
	static int resize_dialog_scale_y = 163;

	LONG w;
	LONG wheight;
	RECT rcl;
	RECT wrcl;

	char class_name[128];
	GetWindowRect(window, &rcl);
	GetClassName(window, class_name, sizeof(class_name));

	if (strcmp(class_name, WC_COMBOBOX) == 0) {
		ComboBox_GetDroppedControlRect(window, &rcl);
	}

	if (lParam) {
		HWND win = (HWND)GetWindowLong(window, GWL_HWNDPARENT);
		GetWindowRect(win, &wrcl);
		rcl.left -= wrcl.left;
		rcl.right -= wrcl.left;
		rcl.top -= wrcl.top;
		rcl.bottom -= wrcl.top;
	}

	if (resize_dialog_width == 0) {
		HWND win = CreateDialogParam(ProgramInstance, (LPCSTR)198, NULL, Resize_Dialog_Proc, NULL);
		GetClientRect(win, &wrcl);
		DestroyWindow(win);
		w = wrcl.right;
		wheight = wrcl.bottom;
		resize_dialog_width = w;
		resize_dialog_height = wheight;
	} else {
		w = resize_dialog_width;
		wheight = resize_dialog_height;
	}

	int width = resize_dialog_scale_x * (rcl.right - rcl.left + 1) / w;
	int height = resize_dialog_scale_y * (rcl.bottom - rcl.top + 1) / wheight;

	int x = resize_dialog_scale_x * rcl.left;
	int y = resize_dialog_scale_y * rcl.top;

	rcl.left = x / w;
	rcl.top = y / wheight;
	rcl.right = width + rcl.left - 1;
	rcl.bottom = height + rcl.top - 1;

	MoveWindow(window, rcl.left, rcl.top, width, height, TRUE);

	return(TRUE);
}


struct EzFont {
	char FaceName[128];
	int DeciPtWidth;
	int DeciPtHeight;
	int Attributes;
	HFONT FontHandle;
};

ArrayList<EzFont> g_EzFonts;


/// derived from MSDN "Moving Your Game to Windows, Part III" ttfont.cpp

#define EZ_ATTR_BOLD		  1
#define EZ_ATTR_ITALIC		  2
#define EZ_ATTR_UNDERLINE	  4
#define EZ_ATTR_STRIKEOUT	  8
HFONT Ez_Create_Font (HDC hdc, const char * face_name, int decipt_width, int decipt_height, int attributes);


/// <summary>
/// Fetches a font of the typeface and point size requested.
/// This routine keeps every font it has built, so repeated requests for the same
/// description hand back the same handle rather than burning another GDI object.
/// The dialog drawing code calls this routine wherever it needs a font.
/// </summary>
/// <param name="hdc">The device context to build the font for. If this is NULL, the
/// font is only looked up and never created.</param>
/// <param name="decipt_width">The character width in tenths of a point.</param>
/// <param name="decipt_height">The character height in tenths of a point.</param>
/// <param name="attributes">Bit flags of the EZ_ATTR_ style attributes to apply.</param>
/// <returns>Returns with a handle to the font, or NULL if it was neither cached nor
/// able to be created.</returns>
/// <remarks>The returned handle stays owned by the font cache. Do not delete it.</remarks>
HFONT WS_Get_Font(HDC hdc, const char * face_name, int decipt_width, int decipt_height, int attributes)
{
	EzFont font;

	for (int index = 0; index < g_EzFonts.length(); index++) {
		g_EzFonts.get(font, index);
		if (!strcmp(font.FaceName, face_name) && font.DeciPtWidth == decipt_width && font.DeciPtHeight == decipt_height && font.Attributes == attributes) {
			return(font.FontHandle);
		}
	}

	if (hdc == NULL) {
		return(NULL);
	}

	HFONT hFont = Ez_Create_Font(hdc, face_name, decipt_width, decipt_height, attributes);

	if (hFont == NULL) {
		return(NULL);
	}

	strcpy(font.FaceName, face_name);
	font.DeciPtWidth = decipt_width;
	font.DeciPtHeight = decipt_height;
	font.Attributes = attributes;
	font.FontHandle = hFont;

	if (g_EzFonts.addTail(font)) {
		return(hFont);
	}

	return(NULL);
}


/// <summary>
/// Creates a font of the typeface and point size requested.
/// This routine maps the requested decipoint dimensions through the device context's
/// current transform, so the font it builds matches the coordinate space the caller
/// draws in. Use WS_Get_Font in preference to this routine -- that one caches its fonts.
/// </summary>
/// <param name="hdc">The device context the font is to be built for.</param>
/// <param name="decipt_width">The character width in tenths of a point. Zero lets the
/// typeface choose its own aspect.</param>
/// <param name="decipt_height">The character height in tenths of a point.</param>
/// <param name="attributes">Bit flags of the EZ_ATTR_ style attributes to apply.</param>
/// <returns>Returns with a handle to the font created, or NULL if it could not be
/// created.</returns>
/// <remarks>The caller takes ownership of the font handle.</remarks>
HFONT Ez_Create_Font (HDC hdc, const char * face_name, int decipt_width,
					int decipt_height, int attributes)
{
	HFONT		hFont ;
	LOGFONT	lf ;
	POINT		pt ;
	TEXTMETRIC tm ;

	SaveDC (hdc) ;

	SetGraphicsMode (hdc, GM_ADVANCED) ;
	ModifyWorldTransform (hdc, NULL, MWT_IDENTITY) ;
	SetViewportOrgEx (hdc, 0, 0, NULL) ;
	SetWindowOrgEx   (hdc, 0, 0, NULL) ;

	pt.x = decipt_width ;
	pt.y = decipt_height ;

	DPtoLP (hdc, &pt, 1) ;

	lf.lfHeight			= -pt.y ;
	lf.lfWidth			= 0 ;
	lf.lfEscapement		= 0 ;
	lf.lfOrientation	= 0 ;
	lf.lfWeight		 = attributes & EZ_ATTR_BOLD	   ? 700 : 0 ;
	lf.lfItalic		 = attributes & EZ_ATTR_ITALIC    ?   1 : 0 ;
	lf.lfUnderline 	 = attributes & EZ_ATTR_UNDERLINE ?   1 : 0 ;
	lf.lfStrikeOut 	 = attributes & EZ_ATTR_STRIKEOUT ?   1 : 0 ;
	lf.lfCharSet		= ANSI_CHARSET ;
	lf.lfOutPrecision	= 0 ;
	lf.lfClipPrecision	= 0 ;
	lf.lfQuality		= 0 ;
	lf.lfPitchAndFamily	= 0 ;

	strcpy (lf.lfFaceName, face_name) ;

	hFont = CreateFontIndirect (&lf) ;

	if (decipt_width != 0) {
		hFont = (HFONT) SelectObject (hdc, hFont) ;
		GetTextMetrics (hdc, &tm) ;
		DeleteObject (SelectObject (hdc, hFont)) ;
		lf.lfWidth = (int) (tm.tmAveCharWidth *
									fabs (pt.x) / fabs (pt.y) + 0.5);
		hFont = CreateFontIndirect (&lf) ;
	}

	RestoreDC (hdc, -1);
	return(hFont);
}
