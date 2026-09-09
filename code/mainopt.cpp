/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "mainopt.h"

#include "_map.h"
#include "_mixfile.h"
#include "_rect.h"
#include "_surface.h"
#include "convert.h"
#include "data.h"
#include "dbgprint.h"
#include "audio/audioengine.h"
#include "dsurface.h"
#include "gamedlg.h"
#include "globals.h"
#include "init.h"
#include "language/language.h"
#include "misc.h"
#include "video.h"
#include "mixfile.h"
#include "msgbox.h"
#include "newmenu.h"
#include "ownrdraw.h"
#include "sidebar.h"
#include "sounddlg.h"
#include "stimer.h"
#include "surface.h"
#include "ui/uidisplay.h"
#include "ui/uishell.h"
#include "wwmouse.h"

#include "color.hh"


INT_PTR CALLBACK Main_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
INT_PTR CALLBACK Display_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
bool Change_Display_Mode(int width, int height);
bool Test_Display_Mode_Dialog(int width, int height);
INT_PTR CALLBACK Test_Display_Mode_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
static void Display_Options_Dialog(void);

// The presenters the display and confirmation dialog procedures are views of, each for the
// life of one dialog.
static UIDisplayPresenterClass * _DisplayPresenter = NULL;
static UIConfirmModePresenterClass * _ConfirmPresenter = NULL;


static void Queue_And_Drain(UIPresenterClass & presenter, char const * name, int value = 0)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	presenter.Queue(intent);
	presenter.Drain();
}


/// <summary>
/// Brings up the main options dialog.
/// This routine drives the options menu, dispatching to the sound, display, keyboard and
/// game settings dialogs until the player backs out. A resolution change is offered as a
/// trial first, and the settings are written out when the player leaves.
/// </summary>
/// <remarks>Game logic is suspended for the duration of this routine.</remarks>
void Main_Options_Dialog(void)
{
	bool old_game_active = GameActive;
	GameActive = false;

	HWND main_handle;
	LONG main_rc;

	while (true) {
		do {
			main_rc = -1;
			main_handle = OwnerDraw::Begin_Dialog(IDD_OPT_MAIN, Main_Options_Dialog_Proc);
		} while (main_handle == 0);
		SetWindowLongPtr(main_handle, DWLP_USER, (LONG_PTR)&main_rc);

		OwnerDraw::Move_Dialog(main_handle, -1, (HiddenSurface->Get_Height() - 400) / 2 + 147);
		OwnerDraw::Display_Dialog(main_handle);

		while (main_rc < 0) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				break;
			}
			Title_Screen_Restore();
		}

		OwnerDraw::End_Dialog(main_handle);

		switch (main_rc) {
			case IDC_OPTMAIN_SOUND:
				SoundControlsClass().Dialog();
				break;

			case IDC_OPTMAIN_DISPLAY:
				Display_Options_Dialog();
				break;

			case IDC_OPTMAIN_KEYBOARD:
				Options.Hotkey_Dialog();
				break;

			case IDC_OPTMAIN_GAME_SETTINGS:
				GameControlsClass().Dialog();
				break;

			default:
				Options.Save_Settings();
				GameActive = old_game_active;
				return;
		}
	}
}


/// <summary>
/// Handles the main options dialog.
/// This routine reports the button the player pressed back to the options dialog driver so
/// that it can bring up the appropriate sub dialog. The sound button is disabled when there
/// is no audio hardware to talk to.
/// </summary>
INT_PTR CALLBACK Main_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int *result;
	HWND handle;

	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc == 0) {
		result = (int *)GetWindowLongPtr(window, DWLP_USER);
		switch (message) {

			case WM_COMMAND:
				*result = LOWORD(wparam);
				break;

			case WM_INITDIALOG:
				handle = GetDlgItem(window, IDC_OPTMAIN_SOUND);
				if (handle) {
					EnableWindow(handle, AudioEngine.Is_Available());
				}
				break;

		}
		return(0);
	}
	return(rc);
}


/// <summary>
/// Switches the game over to a new render resolution.
/// Every drawing surface is destroyed and recreated at the new size, so any pointer held
/// across this call is stale.
/// </summary>
/// <param name="width">The width to render at.</param>
/// <param name="height">The height to render at.</param>
/// <returns>bool; Was the mode changed? If not, nothing has been disturbed.</returns>
bool Change_Display_Mode(int width, int height)
{
	DebugString("About to set video mode\n");

	Hide_Mouse();

	if (!Video_Set_Mode(width, height)) {
		DebugString("Video_Set_Mode failed.\n");
		Show_Mouse();
		return(false);
		}

	VisibleRect = Rect(0, 0, width, height);
	DebugString("VisibleRect: %dx%d\n", width, height);

	if (VisibleSurface != NULL) {
		delete VisibleSurface;
		VisibleSurface = NULL;
	}

	if (AlternateSurface != NULL) {
		delete AlternateSurface;
		AlternateSurface = NULL;
	}

	if (HiddenSurface != NULL) {
		delete HiddenSurface;
		HiddenSurface = NULL;
	}

	if (TileSurface != NULL) {
		delete TileSurface;
		TileSurface = NULL;
	}

	if (SidebarSurface != NULL) {
		delete SidebarSurface;
		SidebarSurface = NULL;
	}

	if (CompositeSurface != NULL) {
		delete CompositeSurface;
		CompositeSurface = NULL;
	}

	VisibleSurface = DSurface::Create_Primary();
	if (VisibleSurface == NULL) {
		Show_Mouse();
		return(false);
	}

	/*
	 * A window that is tracking the frame follows it to the new size. One the player
	 * sized themselves, and a window covering the screen, both stay as they are and the
	 * frame is scaled into them instead.
	 */
	if (WindowedMode && Options.WindowWidth <= 0 && Options.WindowHeight <= 0) {
		RECT windowrect;
		SetRect(&windowrect, 0, 0, width, height);
		AdjustWindowRectEx(&windowrect, GetWindowLong(MainWindow, GWL_STYLE), FALSE, GetWindowLong(MainWindow, GWL_EXSTYLE));

		int newwidth = windowrect.right - windowrect.left;
		int newheight = windowrect.bottom - windowrect.top;

		/*
		 * The window grows about its middle rather than its corner, so the picture stays
		 * where the player was looking.
		 */
		RECT current;
		GetWindowRect(MainWindow, &current);
		int x = current.left + (((current.right - current.left) - newwidth) / 2);
		int y = current.top + (((current.bottom - current.top) - newheight) / 2);

		/*
		 * Growing about the middle can push the window past the edges of the screen, and a
		 * title bar above the top of it cannot be grabbed to bring the window back.
		 */
		MONITORINFO monitor;
		monitor.cbSize = sizeof(monitor);
		if (GetMonitorInfo(MonitorFromWindow(MainWindow, MONITOR_DEFAULTTONEAREST), &monitor)) {
			if (x + newwidth > monitor.rcWork.right) x = monitor.rcWork.right - newwidth;
			if (y + newheight > monitor.rcWork.bottom) y = monitor.rcWork.bottom - newheight;
			if (x < monitor.rcWork.left) x = monitor.rcWork.left;
			if (y < monitor.rcWork.top) y = monitor.rcWork.top;
		}

		SetWindowPos(MainWindow, NULL, x, y, newwidth, newheight, SWP_NOZORDER);
	}

	Rect temp = VisibleRect;
	temp.X = ((Options.IsSidebarOnRight || Debug_Map) ? 0 : SidebarClass::SIDE_WIDTH);
	temp.Y = 16;
	temp.Width -= SidebarClass::SIDE_WIDTH;
	temp.Height -= 16;

	Allocate_Surfaces(VisibleRect, Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, SidebarClass::SIDE_WIDTH, VisibleRect.Height));
	LogicalSurface = HiddenSurface;

	if (MouseCursor != NULL) {
		((WWMouseClass*)MouseCursor)->Calc_Confining_Rect();
	}

	Map.Set_View_Dimensions(temp);

	Map.Init_IO();
	Map.Activate(
#ifdef _DEBUG
		Debug_Map == true ? 1 : 0
#else
		1
#endif
	);
	Map.Reposition_Sidebar();
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	Show_Mouse();

	DebugString("Mode change complete.\n");

	return(true);
}


/// <summary>
/// Tries a display mode out and asks the player to confirm it.
/// This routine switches to the requested mode and puts up a confirmation dialog. If the
/// player does not accept the mode -- or says nothing at all, because a bad mode may well
/// leave the screen unreadable -- the previous resolution is restored.
/// </summary>
/// <param name="width">The width of the display mode to try.</param>
/// <param name="height">The height of the display mode to try.</param>
/// <returns>bool; Was the new display mode accepted and left in place?</returns>
bool Test_Display_Mode_Dialog(int width, int height)
{
	DebugString("Testing display mode @ %dx%d\n", width, height);
	Hide_Mouse();
	HiddenSurface->Fill(TBLACK);
	Update_Visible_Surface();

	if (!Change_Display_Mode(width, height)) {
		return(false);
	}

	HiddenSurface->Fill(TBLACK);
	Update_Visible_Surface();
	Show_Mouse();
	Draw_Menu_Background();

	UIConfirmModePresenterClass presenter(UI_Clock());
	_ConfirmPresenter = &presenter;

	HWND dialog = OwnerDraw::Begin_Dialog(IDD_OPT_CONFIRM_MODE, Test_Display_Mode_Dialog_Proc);
	if (dialog) {
		OwnerDraw::Display_Dialog(dialog);

		presenter.Refresh();
		while (!presenter.Result.has_value()) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				break;
			}
			Title_Screen_Restore();
			presenter.Refresh();
		}

		OwnerDraw::End_Dialog(dialog);
	}
	_ConfirmPresenter = NULL;

	if (dialog && (!presenter.Result.has_value() || *presenter.Result != UI_RESULT_ACCEPTED)) {
		DebugString("Resetting display mode @ %dx%d\n", Options.ScreenWidth, Options.ScreenHeight);
		Change_Display_Mode(Options.ScreenWidth, Options.ScreenHeight);
		LogicalSurface = HiddenSurface;
		return(false);
	}

	DebugString("Keeping display mode @ %dx%d\n", width, height);
	LogicalSurface = HiddenSurface;
	return(true);
}


/// <summary>
/// Handles the mode confirmation dialog.
/// This routine hands the button the player pressed to the confirmation presenter, which
/// tells the mode test whether the new resolution was accepted or rejected.
/// </summary>
INT_PTR CALLBACK Test_Display_Mode_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc == 0) {
		UIConfirmModePresenterClass * presenter = _ConfirmPresenter;
		if (presenter != NULL && message == WM_COMMAND) {
			switch (LOWORD(wparam)) {
				case IDOK:
					Queue_And_Drain(*presenter, "ok");
					break;

				case IDCANCEL:
					Queue_And_Drain(*presenter, "cancel");
					break;
			}
		}
		return(0);
	}
	return(rc);
}


/// <summary>
/// Runs the display options until the player leaves them or a new display mode is kept.
/// A picked mode is tried out first, and a mode the player does not confirm brings the
/// dialog back.
/// </summary>
static void Display_Options_Dialog(void)
{
	while (true) {
		UIDisplayState state;
		UI_Display_State(state);
		UIDisplayPresenterClass presenter(UI_Display_Service(), state);
		_DisplayPresenter = &presenter;

		HWND handle;
		LONG rc;
		do {
			rc = -1;
			handle = OwnerDraw::Begin_Dialog(IDD_OPT_DISPLAY, Display_Options_Dialog_Proc);
		} while (handle == 0);
		SetWindowLongPtr(handle, DWLP_USER, (LONG_PTR)&rc);
		OwnerDraw::Display_Dialog(handle);

		while (rc < 0) {
			if (OwnerDraw::Dialog_Message_Handler() == true) {
				break;
			}
			Title_Screen_Restore();
		}

		OwnerDraw::End_Dialog(handle);
		_DisplayPresenter = NULL;

		if (!presenter.Picked.has_value()) {
			break;
		}

		if (WWMessageBox().Process(TXT_ABOUT_TO_TRY_MODE, TXT_OK, TXT_CANCEL) != 0) {
			break;
		}
		if (Test_Display_Mode_Dialog(presenter.Picked->Width, presenter.Picked->Height)) {
			Options.ScreenWidth = presenter.Picked->Width;
			Options.ScreenHeight = presenter.Picked->Height;
			break;
		}
	}
}


/// <summary>
/// Handles the display options dialog messages.
/// This routine seeds the resolution list and the movie switch from the presenter's state
/// and hands the player's picks back to it; the presenter records the mode to try.
/// </summary>
static __forceinline BOOL Display_Options_Dialog_Body(HWND window, UINT message, WPARAM wparam)
{
	UIDisplayPresenterClass * presenter = _DisplayPresenter;
	if (presenter == NULL) {
		return(0);
	}

	int * result = (int *)GetWindowLongPtr(window, DWLP_USER);
	switch (message) {
		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				default:
					return(0);

				case IDC_DISPLAY_RESLIST: {
					HWND list = GetDlgItem(window, IDC_DISPLAY_RESLIST);
					Queue_And_Drain(*presenter, "select", ListBox_GetCurSel(list));
				}
				return(0);

				case IDOK: {
					HWND button = GetDlgItem(window, IDC_STRETCH_MOVIES);
					if (button) {
						Queue_And_Drain(*presenter, "stretch", Button_GetCheck(button) == BST_CHECKED);
					}
					Queue_And_Drain(*presenter, "ok");
				}
				break;

				case IDCANCEL:
					Queue_And_Drain(*presenter, "cancel");
					break;
			}
			*result = LOWORD(wparam);
			break;

		case WM_INITDIALOG: {
			UIDisplayState const & state = presenter->State;
			HWND list = GetDlgItem(window, IDC_DISPLAY_RESLIST);
			for (UIDisplayMode const & mode : state.Modes) {
				ListBox_AddString(list, mode.Label.c_str());
			}
			ListBox_SetCurSel(list, state.Selected);

			HWND button = GetDlgItem(window, IDC_STRETCH_MOVIES);
			if (button) {
				Button_SetCheck(button, state.StretchMovies);
			}
		}
		break;

	}
	return(0);
}


/// <summary>
/// Handles the display options dialog.
/// This routine gives the owner draw dialog system first refusal on the message and only
/// deals with what it leaves behind.
/// </summary>
INT_PTR CALLBACK Display_Options_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	INT_PTR rc = OwnerDraw::Default_Dialog_Proc(window, message, wparam, lparam);
	if (rc == 0) {
		return(Display_Options_Dialog_Body(window, message, wparam));
	}
	return(rc);
}
