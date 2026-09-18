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
#include "audio/audioengine.h"
#include "convert.h"
#include "data.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "gamedlg.h"
#include "globals.h"
#include "init.h"
#include "language/language.h"
#include "misc.h"
#include "mixfile.h"
#include "msgbox.h"
#include "newmenu.h"
#include "sidebar.h"
#include "sounddlg.h"
#include "stimer.h"
#include "surface.h"
#include "ui/screens/display/uidisplay.h"
#include "ui/screens/mainopt/uimainopt.h"
#include "video.h"
#include "wwmouse.h"

#include "color.hh"

#include <optional>


bool Change_Display_Mode(int width, int height);
bool Test_Display_Mode_Dialog(int width, int height);
static void Display_Options_Dialog(void);


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

	while (true) {
		switch (UI_Main_Options_Dialog()) {
			case UI_MAIN_OPTIONS_SOUND:
				SoundControlsClass().Dialog();
				break;

			case UI_MAIN_OPTIONS_DISPLAY:
				Display_Options_Dialog();
				break;

			case UI_MAIN_OPTIONS_KEYBOARD:
				Options.Hotkey_Dialog();
				break;

			case UI_MAIN_OPTIONS_SETTINGS:
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

	if (!UI_Confirm_Mode_Dialog()) {
		DebugString("Resetting display mode @ %dx%d\n", Options.ScreenWidth, Options.ScreenHeight);
		Change_Display_Mode(Options.ScreenWidth, Options.ScreenHeight);
		LogicalSurface = HiddenSurface;

		Draw_Menu_Background();
		return(false);
	}

	DebugString("Keeping display mode @ %dx%d\n", width, height);
	LogicalSurface = HiddenSurface;
	return(true);
}


/// <summary>
/// Runs the display options until the player leaves them or a new display mode is kept.
/// A picked mode is tried out first, and a mode the player does not confirm brings the
/// dialog back.
/// </summary>
static void Display_Options_Dialog(void)
{
	while (true) {
		std::optional<UIDisplayMode> picked = UI_Display_Dialog();
		if (!picked.has_value()) {
			break;
		}

		if (WWMessageBox().Process(TXT_ABOUT_TO_TRY_MODE, TXT_OK, TXT_CANCEL) != 0) {
			break;
		}
		if (Test_Display_Mode_Dialog(picked->Width, picked->Height)) {
			Options.ScreenWidth = picked->Width;
			Options.ScreenHeight = picked->Height;
			break;
		}
	}
}
