/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "gamewindow.h"

#include "_keyboar.h"
#include "_map.h"
#include "_surface.h"
#include "_tooltip.h"
#include "_ui.h"
#include "_xmouse.h"
#include "cctooltip.h"
#include "dbgprint.h"
#include "globals.h"
#include "gscreen.h"
#include "init.h"
#include "misc.h"
#include "movies.h"
#include "platform/platform.h"
#include "platform/windowevent.hh"
#include "queue.h"
#include "session.h"
#include "ui/uishell.h"
#include "video.h"
#include "vidscale.h"
#include "wincursor.h"
#include "winstub.h"
#include "wwmouse.h"


static bool _HandlingMouseWheel = false;
static bool _InterfaceTookKey = false;


static bool Is_Mouse_Event(WindowEventType type)
{
	switch (type) {
		case WINDOW_EVENT_MOUSE_MOVE:
		case WINDOW_EVENT_MOUSE_DOWN:
		case WINDOW_EVENT_MOUSE_UP:
		case WINDOW_EVENT_MOUSE_WHEEL:
			return(true);

		default:
			return(false);
	}
}


static void Recalculate_Confining_Rect(void)
{
	if (MouseCursor != NULL) {
		((WWMouseClass *)MouseCursor)->Calc_Confining_Rect();
	}
}


static void Set_Game_Focus(bool focused)
{
	if (GameInFocus == focused) {
		return;
	}

	GameInFocus = focused;
	if (!GameInFocus) {
		Focus_Loss();
		DebugString("Focus lost\n");
	} else {
		Focus_Restore();
		DebugString("Focus gained\n");
	}
}


/// <summary>
/// Passes an event from the main window to the tooltips, the interface, the tactical map,
/// and the key queue, in that order, and applies the game's own response to it.
/// </summary>
/// <returns>True when the interface took the event, so the window must not act on it
/// either.</returns>
bool Game_Window_Handle_Event(WindowEvent const & event)
{
	if (ToolTips != NULL) {
		ToolTips->Handle_Window_Event(event);
	}

	bool const taken = UIShell.Handle_Window_Event(event);

	// A character belongs with the key press that typed it, so the game gets neither when
	// the interface took the press.
	if (event.Type == WINDOW_EVENT_KEY_DOWN) {
		_InterfaceTookKey = taken;
	} else if (event.Type == WINDOW_EVENT_TEXT && _InterfaceTookKey) {
		return(true);
	}

	if (taken) {
		return(true);
	}

	// The map and the key queue work in frame coordinates.
	WindowEvent frame = event;
	if (Is_Mouse_Event(event.Type) && Video_Scaling_Active()) {
		POINT point;
		point.x = event.X;
		point.y = event.Y;
		Window_Point_To_Game(point);
		frame.X = point.x;
		frame.Y = point.y;
	}

	Map.Handle_Window_Event(frame);

	switch (event.Type) {
		case WINDOW_EVENT_EXPOSED:
			Game_Window_On_Paint(GameInFocus == true || WindowedMode == true);
			break;

		case WINDOW_EVENT_RESIZED:
			Video_On_Resize(event.Width, event.Height);
			Video_Set_Refresh_Rate(Platform_Window_Refresh_Rate());
			Recalculate_Confining_Rect();
			break;

		case WINDOW_EVENT_DISPLAY_CHANGED:
			Video_Set_Refresh_Rate(Platform_Window_Refresh_Rate());
			break;

		case WINDOW_EVENT_MOVED:
			if (WindowedMode == true) {
				Recalculate_Confining_Rect();
			}
			break;

		case WINDOW_EVENT_FOCUS_GAINED:
		case WINDOW_EVENT_FOCUS_LOST:
			Set_Game_Focus(event.Type == WINDOW_EVENT_FOCUS_GAINED);
			return(false);

		case WINDOW_EVENT_MOUSE_UP:
			if (event.Button == WINDOW_BUTTON_RIGHT) {
				Game_Window_On_Right_Mouse_Up();
			}
			break;

		case WINDOW_EVENT_MOUSE_WHEEL:
			if (!event.Horizontal) {
				Game_Window_On_Mouse_Wheel(event.Wheel < 0.0f ? -1 : 1);
			}
			break;

		// A running game resigns rather than closing: the exit is played through the
		// queue, and the game ends itself once it arrives.
		case WINDOW_EVENT_CLOSE_REQUESTED:
			if (GameActive && PlayerPtr != NULL && !Session.Play) {
				Queue_Exit();
			}
			break;

		default:
			break;
	}

	/*
	**	Pass this event through to the keyboard handler.
	*/
	if (Keyboard != NULL) {
		Keyboard->Handle_Window_Event(frame);
	}

	return(false);
}


/// <summary>
/// Shows the pointer the interface or the game wants over the window.
/// </summary>
/// <returns>True when either of them chose a pointer.</returns>
bool Game_Window_Select_Cursor(void)
{
	return(UIShell.Handle_Set_Cursor() || Win_Cursor_Handle_Set_Cursor());
}


/// <summary>
/// Shows the pointer the interface or the game wants, or the window's arrow when neither
/// chooses one. The arrow stays hidden while the game has hidden the pointer it released.
/// </summary>
void Game_Window_Update_Cursor(void)
{
	if (Game_Window_Select_Cursor()) {
		return;
	}

	bool const visible = (MouseCursor == NULL || MouseCursor->Get_Mouse_State() >= 0);
	Platform_Set_Cursor(visible ? Platform_System_Cursor(PLATFORM_CURSOR_ARROW) : NULL);
}


/// <summary>
/// Lets go of the main window at shutdown. Nothing is pumped from it afterwards, and a clean
/// shutdown in progress is marked finished.
/// </summary>
void Game_Window_Closed(void)
{
	if (ToolTips != NULL) {
		delete ToolTips;
		ToolTips = NULL;
	}
	MainWindow = NULL;

	/*
	**	If we are shutting down gracefully then flag that the message loop has finished.
	*/
	if (ReadyToQuit == 1) {
		ReadyToQuit = 2;
	}
}


/// <summary>
/// Updates and presents the frame when the application window needs repainting.
/// </summary>
void Game_Window_On_Paint(bool update_surface)
{
	if (update_surface) {
		if (MouseCursor != NULL && VisibleSurface != NULL && HiddenSurface != NULL && CompositeSurface != NULL) {
			if (ScenarioActive == true) {
				Map.Blit_Sidebar(true);
				Update_Visible_Surface(CompositeSurface);
			} else if (Movie_Is_Playing() == true) {
				Movie_Update_Visible_Surface();
			} else {
				Update_Visible_Surface(HiddenSurface);
			}
		}
	}
	Video_Present_If_Dirty();
}


/// <summary>
/// Stops tactical scrolling from coasting after the right mouse button is released.
/// </summary>
void Game_Window_On_Right_Mouse_Up(void)
{
	Map.Set_Scroll_Coasting_Allowed(false);
}


/// <summary>
/// Applies a mouse wheel step to the sidebar.
/// </summary>
void Game_Window_On_Mouse_Wheel(int delta)
{
	if (_HandlingMouseWheel) {
		return;
	}

	_HandlingMouseWheel = true;
	Execute_Command(delta < 0 ? "SidebarDown" : "SidebarUp");
	_HandlingMouseWheel = false;
}
