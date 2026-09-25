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
#include "goptions.h"
#include "gscreen.h"
#include "init.h"
#include "misc.h"
#include "movies.h"
#include "queue.h"
#include "sdl/sdlwindow.h"
#include "session.h"
#include "ui/uishell.h"
#include "video.h"
#include "vidscale.h"
#include "windowevent.hh"
#include "winstub.h"
#include "wwmouse.h"

#include <commctrl.h>


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


static bool Select_Cursor(void)
{
	return(UIShell.Handle_Set_Cursor() || (MouseCursor != NULL && ((WWMouseClass *)MouseCursor)->Show_Game_Pointer()));
}


// The window's arrow stays hidden while the game has hidden the pointer it released.
static void Update_Cursor(void)
{
	if (Select_Cursor()) {
		return;
	}

	bool const visible = (MouseCursor == NULL || MouseCursor->Get_Mouse_State() >= 0);
	Main_Window_Set_Cursor(visible ? Main_Window_System_Cursor(UI_CURSOR_ARROW) : NULL);
}


static bool Handle_Event(WindowEvent const & event)
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
		Point2D point(event.X, event.Y);
		Window_Point_To_Game(point);
		frame.X = point.X;
		frame.Y = point.Y;
	}

	Map.Handle_Window_Event(frame);

	switch (event.Type) {
		case WINDOW_EVENT_EXPOSED:
			Game_Window_On_Paint(GameInFocus == true || WindowedMode == true);
			break;

		case WINDOW_EVENT_RESIZED:
			Video_On_Resize(event.Width, event.Height);
			Video_Set_Refresh_Rate(Main_Window_Refresh_Rate());
			break;

		case WINDOW_EVENT_DISPLAY_CHANGED:
			Video_Set_Refresh_Rate(Main_Window_Refresh_Rate());
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

		// A running game resigns through the queue and then ends itself; otherwise the request
		// is ignored.
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
/// Passes an event from the main window to the tooltips, the interface, the tactical map,
/// and the key queue, in that order, and applies the game's own response to it.
/// </summary>
/// <returns>True when the interface took the event.</returns>
bool Game_Window_Handle_Event(WindowEvent const & event)
{
	bool const taken = Handle_Event(event);

	// Windows asked for the pointer on every mouse move; SDL leaves choosing it to the game.
	if (event.Type == WINDOW_EVENT_MOUSE_MOVE || event.Type == WINDOW_EVENT_FOCUS_GAINED) {
		Update_Cursor();
	}
	return(taken);
}


/// <summary>
/// Opens the main window, with its tooltips and the window's arrow. A window for windowed play
/// has the client size the WindowWidth and WindowHeight settings give, taking the frame's size
/// for either one that is not set; otherwise the window covers the primary display.
/// </summary>
/// <param name="width">The width of the frame.</param>
/// <param name="height">The height of the frame.</param>
/// <returns>False when SDL could not start or the window could not be created.</returns>
bool Game_Window_Open(int width, int height)
{
	InitCommonControls();

	int clientwidth = width;
	int clientheight = height;
	if (WindowedMode) {
		if (Options.WindowWidth > 0) {
			clientwidth = Options.WindowWidth;
		}
		if (Options.WindowHeight > 0) {
			clientheight = Options.WindowHeight;
		}
	}

	if (!Main_Window_Create(WindowedMode, clientwidth, clientheight)) {
		return(false);
	}

	MainWindow = (HWND)Main_Window_Native().Handle;

	ToolTips = new CCToolTip();
	ToolTips->Set_Timer_Delay(500);

	Main_Window_Set_Cursor(Main_Window_System_Cursor(UI_CURSOR_ARROW));
	return(true);
}


/// <summary>
/// Stops the game using the main window at shutdown: its tooltips are deleted and nothing is
/// pumped from it afterwards. The window itself stays until Game_Window_Close.
/// </summary>
void Game_Window_Begin_Shutdown(void)
{
	if (ToolTips != NULL) {
		delete ToolTips;
		ToolTips = NULL;
	}
	MainWindow = NULL;
}


/// <summary>
/// Destroys the main window and stops SDL, first doing what Game_Window_Begin_Shutdown does
/// if it has not run. The renderer must have let go of the window. Calling it again does
/// nothing.
/// </summary>
void Game_Window_Close(void)
{
	Game_Window_Begin_Shutdown();
	Main_Window_Destroy();
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
