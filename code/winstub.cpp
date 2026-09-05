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

/* $Header: /CounterStrike/WINSTUB.CPP 3     3/13/97 2:06p Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : WINSTUB.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Steve Tall                                                   *
 *                                                                                             *
 *                   Start Date : 10/04/95                                                     *
 *                                                                                             *
 *                  Last Update : October 4th 1995 [ST]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Overview:                                                                                   *
 *   This file contains stubs for undefined externals when linked under Watcom for Win 95      *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 *                                                                                             *
 * Functions:                                                                                  *
 *   Assert_Failure -- display the line and source file where a failed assert occurred         *
 *   Check_For_Focus_Loss -- check for the end of the focus loss                               *
 *   Create_Main_Window -- opens the MainWindow for C&C                                        *
 *   Focus_Loss -- this function is called when a library function detects focus loss          *
 *   Memory_Error_Handler -- Handle a possibly fatal failure to allocate memory                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "winstub.h"

#include "_keyboar.h"
#include "_map.h"
#include "_rect.h"
#include "_tooltip.h"
#include "ccfile.h"
#include "cctooltip.h"
#include "convert.h"
#include "dbgprint.h"
#include "draw.h"
#include "dsaudio.h"
#include "dsurface.h"
#include "except.h"
#include "gamewindow.h"
#include "globals.h"
#include "goptions.h"
#include "misc.h"
#include "movie.h"
#include "msgroute.h"
#include "nativewindow.hh"
#include "pcx.h"
#include "queue.h"
#include "resource.h"
#include "session.h"
#include "theme.h"
#include "video.h"
#include "win.h"
#include "wincursor.h"
#include "windlg.h"
#include "winfix.h"
#include "wsproto.h"
#include "wwmouse.h"
#include "mainopt.h"
#include "conquer.h"
#include "opents_version.h"

#include <algorithm>
#include <commctrl.h>

int		ShowCommand;
HWND	MainWindow;
HWND	UnusedWindow;

HINSTANCE	ProgramInstance;
bool _MouseCaptured;


//void output(short,short)
//{}

/*
 * Taken from later Windows SDK after what is shipped in VS6
 */

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL (WM_MOUSELAST+1)  /// message that will be supported
#endif

#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif
///////////////////////////////////////////////////////////

//unsigned long CCFocusMessage = WM_USER+50;	//Private message for receiving application focus
extern	void VQA_PauseAudio(void);
extern	void VQA_ResumeAudio(void);

ThemeType OldTheme = THEME_NONE;


/***********************************************************************************************
 * Focus_Loss -- this function is called when a library function detects focus loss            *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    2/1/96 2:10PM ST : Created                                                               *
 *=============================================================================================*/

void Focus_Loss(void)
{
	DebugString("Focus_Loss()\n");
	Pause_Ingame_Movie(true);
	OldTheme = Theme.What_Is_Playing();
	Theme.Suspend();
	if (Audio_Available()) Audio.Stop_Primary_Sound_Buffer();
	if (MouseCursor) {
		_MouseCaptured = MouseCursor->Is_Captured();
		DebugString("Focus_Loss(): _MouseCaptured = %s\n", _MouseCaptured ? "true" : "false");
		MouseCursor->Release_Mouse();
	}
}


/// <summary>
/// Restores the game when it regains the input focus.
/// This routine is the counterpart to Focus_Loss. It starts the sound and the music
/// back up, recaptures the mouse if it was captured when focus was lost, and flags the
/// whole screen for redraw.
/// </summary>
void Focus_Restore(void)
{
	DebugString("Focus_Restore()\n");
	if (Audio_Available()) Audio.Start_Primary_Sound_Buffer(TRUE);
	DebugString("Focus_Restore(): _MouseCaptured = %s\n", _MouseCaptured ? "true" : "false");
	if (MouseCursor && _MouseCaptured == true && !Debug_Map) {
		MouseCursor->Capture_Mouse();
	}
	Heal_Dialog_Controls();
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	InvalidateRect(MainWindow, 0, 0);
	Theme.Play_Song(OldTheme);
	Pause_Ingame_Movie(false);
	if (WS_Top_Window()) {
		SetActiveWindow(WS_Top_Window());
		SetFocus(WS_Top_Window());
	}
}


extern bool InMovie;

/// <summary>
/// Handles the Windows messages sent to the main game window.
/// This is the window procedure registered for the main window. It offers each
/// message to the network transport, the map and the keyboard handlers, deals with
/// the messages the game must react to itself -- focus changes, painting, tray
/// locking and shutdown -- and passes everything else back to Windows.
/// </summary>
/// <returns>Returns with the result Windows expects for the message handled.</returns>
LRESULT CALLBACK /*_export*/ Windows_Procedure(HWND hwnd, UINT message, UINT wParam, LONG lParam)
{

	/*
	 * The frame may be drawn scaled, so a click has to be matched against where the
	 * player sees the controls rather than where Windows finds them.
	 */
	{
		LPARAM translated_lparam;
		if (Route_Mouse_Message(hwnd, message, wParam, lParam, &translated_lparam)) {
			return(0);
		}
		lParam = (LONG)translated_lparam;
	}

	int	low_param = LOWORD(wParam);

	/*
	**	Pass on any messages intended for the winsock message handler.
	*/
	if ( PacketTransport ) {
		if ( message == (UINT) PacketTransport->Protocol_Event_Message() ) {
			if ( PacketTransport->Message_Handler (hwnd, message, wParam, lParam) ){
				return( DefWindowProc (hwnd, message, wParam, lParam) );
			}else{
				return(0);
			}
		}
	}

	Map.Message_Handler(hwnd, message, wParam, lParam);

	if (MainWindow) {
		GetMenu(MainWindow);
	}

	switch ( message ) {
		// Raised on request so that crash reporting can be exercised from inside window
		// procedure dispatch, which the operating system unwinds differently from a call.
		case WM_EXCEPTION_TEST:
			Exception_Wndproc_Test_Fault();
			return(0);

//		case WM_SYSKEYDOWN:
//			Mono_Printf("wparam=%08X lparam=%08X\n", (long)wParam, (long)lParam);
			// fall through

//		case WM_MOUSEMOVE:
//		case WM_KEYDOWN:
//		case WM_SYSKEYUP:
//		case WM_KEYUP:
//		case WM_LBUTTONDOWN:
//		case WM_LBUTTONUP:
//		case WM_LBUTTONDBLCLK:
//		case WM_MBUTTONDOWN:
//		case WM_MBUTTONUP:
//		case WM_MBUTTONDBLCLK:
//		case WM_RBUTTONDOWN:
//		case WM_RBUTTONUP:
//		case WM_RBUTTONDBLCLK:
//	 		Keyboard->Message_Handler(hwnd, message, wParam, lParam);
//			return(0);

		case WM_SHOWWINDOW:
			return(0);

		case WM_PAINT:
			Game_Window_On_Paint(GameInFocus == true || WindowedMode == true);
			ValidateRect(hwnd, NULL);
			break;

		case WM_ERASEBKGND:
			return(1);

		case WM_SETCURSOR:
			if (LOWORD(lParam) == HTCLIENT && Win_Cursor_Handle_Set_Cursor()) {
				return(TRUE);
			}
			break;

		case WM_SIZE:
			if (wParam != SIZE_MINIMIZED) {
				Video_On_Resize(LOWORD(lParam), HIWORD(lParam));
				Video_Set_Refresh_Rate(Win_Window_Refresh_Rate(hwnd));
				if (MouseCursor != NULL) {
					((WWMouseClass *)MouseCursor)->Calc_Confining_Rect();
				}
			}
			break;

		case WM_DISPLAYCHANGE:
			Video_Set_Refresh_Rate(Win_Window_Refresh_Rate(hwnd));
			break;

		case WM_CLOSE:
			break;

		case WM_CREATE:
			ToolTips = new CCToolTip(hwnd);
			if (ToolTips) {
				ToolTips->Set_Timer_Delay(500);
			}
			break;

		case WM_MOVE:
			if (WindowedMode == true && MouseCursor != NULL) {
				((WWMouseClass *)MouseCursor)->Calc_Confining_Rect();
			}
			break;

			/*
			**	Windoze message says we have to shut down. Try and do it cleanly.
			*/
		case WM_DESTROY:
			if (ToolTips != NULL) {
				delete ToolTips;
				ToolTips = NULL;
			}
			MainWindow = 0;

			/*
			**	If we are shutting down gracefully than flag that the message loop has finished.
			**	If this is a forced shutdown (ReadyToQuit == 0) then try and close down everything
			**	before we exit.
			*/
			switch (ReadyToQuit) {
				default:
				case 1:
					ReadyToQuit = 2;
					break;

				case 0:
					break;

			}
			return(0);

		case WM_ACTIVATEAPP:
			if (hwnd == MainWindow && GameInFocus != (wParam != 0)) {
				GameInFocus = (wParam != 0);
				if (!GameInFocus) {
					Focus_Loss();
					DebugString("Focus lost\n");
				} else {
					Focus_Restore();
					DebugString("Focus gained\n");
				}
			}
			return(0);

		case WM_RBUTTONUP:
			Game_Window_On_Right_Mouse_Up();
			break;

		case WM_MOVING:
			return(On_WM_MOVING(hwnd, wParam, lParam));

		case WM_MOUSEWHEEL:
			Game_Window_On_Mouse_Wheel(GET_WHEEL_DELTA_WPARAM(wParam));
			break;

		case WM_SYSCOMMAND:
			switch ( wParam ) {

				case SC_CLOSE:
					// A running game resigns rather than closing, and keeps its window: the exit
					// is played through the queue, and the game ends itself once it arrives.
					if (GameActive && PlayerPtr != NULL && !Session.Play) {
						Queue_Exit();
					}
					return(0);

				case SC_SCREENSAVE:
					/*
					**	Windoze is about to start the screen saver. If we just return without passing
					**	this message to DefWindowProc then the screen saver will not be allowed to start.
					*/
					return(0);
			}
			break;

	}

	/*
	**	Pass this message through to the keyboard handler. If the message
	**	was processed and requires no further action, then return with
	**	this information.
	*/
	if (Keyboard->Message_Handler(hwnd, message, wParam, lParam)) {
		return(0);
	}

	return(DefWindowProc (hwnd, message, wParam, lParam));
}


NativeWindow Win_Native_Window(HWND window)
{
	return(NativeWindow{ NATIVE_WINDOW_DEFAULT, nullptr, window });
}


// Client dimensions are physical pixels because the process is per-monitor DPI aware.
bool Win_Window_Drawable_Size(HWND window, int & width, int & height)
{
	RECT client;
	if (window == NULL || !GetClientRect(window, &client)) {
		return(false);
	}

	width = client.right - client.left;
	height = client.bottom - client.top;
	return(width > 0 && height > 0);
}


int Win_Window_Refresh_Rate(HWND window)
{
	int refreshrate = 0;
	HDC dc = GetDC(window);

	if (dc != NULL) {
		refreshrate = GetDeviceCaps(dc, VREFRESH);
		ReleaseDC(window, dc);
	}

	return(refreshrate);
}


/// <summary>
/// Fetches the build number of this executable.
/// This routine is used by the network code to check that every machine joining a
/// game is running the same build.
/// </summary>
/// <returns>Returns with the packed project version this executable was built from.</returns>
unsigned int Build_Number(void)
{
	return(OPENTS_VERSION_PACKED);
}


/***********************************************************************************************
 * Create_Main_Window -- opens the MainWindow for C&C                                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    instance -- handle to program instance                                            *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    10/10/95 4:08PM ST : Created                                                             *
 *=============================================================================================*/

#define CC_ICON		IDI_SUN
#define CC_CURSOR	IDC_CURSOR1

#define WINDOW_NAME		"Tiberian Sun"


void Create_Main_Window ( HINSTANCE instance , int command_show , int width , int height )
{
	InitCommonControls();

	WNDCLASS    	wndclass ;
	//
	// Register the window class
	//

	/*
	 * The dialog controls are hit tested through the main window, so its class has to
	 * report the double clicks they expect.
	 */
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS ;
	wndclass.lpfnWndProc   = Windows_Procedure ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = instance ;
	wndclass.hIcon         = LoadIcon (instance, MAKEINTRESOURCE(CC_ICON)) ;
	wndclass.hCursor       = LoadCursor(ProgramInstance, MAKEINTRESOURCE(CC_CURSOR));
	wndclass.hbrBackground = NULL;
	wndclass.lpszMenuName  = NULL;	///WINDOW_NAME
	wndclass.lpszClassName = WINDOW_NAME;

	RegisterClass (&wndclass) ;


	//
	// Create our main window
	//
	/*
	 * The dialogs paint themselves onto the game's surfaces rather than into their own
	 * windows, so clipping their regions out of the main window would leave holes where
	 * they sit.
	 */
	if (WindowedMode) {
		int clientwidth = (Options.WindowWidth > 0) ? Options.WindowWidth : width;
		int clientheight = (Options.WindowHeight > 0) ? Options.WindowHeight : height;

		MainWindow = CreateWindowEx (
								0,
								WINDOW_NAME,
								WINDOW_NAME,
								WS_OVERLAPPEDWINDOW,
								0,
								0,
								0,
								0,
								NULL,
								NULL,
								instance,
								NULL );

		RECT rect;
		SetRect(&rect, 0, 0, clientwidth, clientheight);
		AdjustWindowRectEx(&rect, GetWindowLong(MainWindow, GWL_STYLE), FALSE, GetWindowLong(MainWindow, GWL_EXSTYLE));

		int windowwidth = rect.right - rect.left;
		int windowheight = rect.bottom - rect.top;
		int x = (GetSystemMetrics(SM_CXSCREEN) - windowwidth) / 2;
		int y = (GetSystemMetrics(SM_CYSCREEN) - windowheight) / 2;

		MoveWindow(MainWindow, std::max(x, 0), std::max(y, 0), windowwidth, windowheight, 1);

	} else {
		/*
		 * The desktop keeps its own resolution and the window simply covers it. The
		 * frame is scaled to fit at presentation time.
		 */
		MainWindow = CreateWindowEx (
								0,
								WINDOW_NAME,
								WINDOW_NAME,
								WS_POPUP,
								0,
								0,
								GetSystemMetrics(SM_CXSCREEN),
								GetSystemMetrics(SM_CYSCREEN),
								NULL,
								NULL,
								instance,
								NULL );
	}

	ShowWindow (MainWindow, SW_NORMAL);
	ShowCommand = command_show;
	UpdateWindow (MainWindow);
	SetFocus (MainWindow);

	RegisterHotKey(MainWindow, 1, MOD_ALT|MOD_CONTROL|MOD_SHIFT, VK_M);

	SetCursor(LoadCursor(ProgramInstance, MAKEINTRESOURCE(CC_CURSOR)));
	Audio.Audio_Focus_Loss_Function = Focus_Loss;

	//Misc_Focus_Loss_Function = &Focus_Loss;
	//Misc_Focus_Restore_Function = &Focus_Restore;
	//Gbuffer_Focus_Loss_Function = &Focus_Loss;
}


/// <summary>
/// Loads a title screen picture and centers it on the surface.
/// This routine is used by the startup and scenario loading sequences to put some
/// artwork on the screen while the game gets itself ready. A paletted picture is
/// drawn through a converter built from the palette supplied.
/// </summary>
/// <param name="name">The name of the picture file to load.</param>
/// <param name="surface">The surface to draw the title screen upon.</param>
/// <param name="palette">The palette to load the picture's colors into.</param>
void Load_Title_Screen(char const * name, Surface * surface, PaletteClass * palette)
{
	Surface *load_buffer;
	CCFileClass file(name);
	load_buffer = Read_PCX_File (file, palette);

	if (load_buffer) {
		Point2D point;
		int x = (surface->Get_Width() - load_buffer->Get_Width()) / 2;
		int y = (surface->Get_Height() - load_buffer->Get_Height()) / 2;
		if (palette && load_buffer->Bytes_Per_Pixel() == 1) {
			ConvertClass *drawer = new ConvertClass(*palette, *palette, *surface);
			Blit_Block(*surface, *drawer, *load_buffer, load_buffer->Get_Rect(), Point2D(x, y), surface->Get_Rect());
			delete drawer;
		} else {

			surface->Blit_From(surface->Get_Rect(), Rect(x, y, load_buffer->Get_Width(), load_buffer->Get_Height()), *load_buffer, load_buffer->Get_Rect(), load_buffer->Get_Rect());
		}
		delete load_buffer;
	}
}
