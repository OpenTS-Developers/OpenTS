/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uienginehost.h"

#include "_keyboar.h"
#include "_rules.h"
#include "_ui.h"
#include "audio/audioengine.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "keyboard.h"
#include "mainloop.h"
#include "mixfile.h"
#include "movies.h"
#include "msgloop.h"
#include "rules.h"
#include "session.h"
#include "ui/uishell.h"
#include "video.h"
#include "voc.h"
#include "wincursor.h"
#include "windlg.h"


// The pointer the window shows when the game's is off it, as under a Win32 dialog.
static HCURSOR Window_Cursor(void)
{
	HCURSOR cursor = (HCURSOR)GetClassLongPtr(MainWindow, GCLP_HCURSOR);
	return(cursor != NULL ? cursor : LoadCursor(NULL, IDC_ARROW));
}


class UIEngineHostClass : public UIShellHostClass
{
	public:
		virtual HWND Main_Window(void) const override
		{
			return(MainWindow);
		}

		virtual UIFrameRect Frame(void) const override
		{
			VideoScaleInfo const & scale = Video_Get_Scale_Info();
			UIFrameRect frame;
			frame.X = scale.DestX;
			frame.Y = scale.DestY;
			frame.Width = scale.DestWidth;
			frame.Height = scale.DestHeight;
			frame.ScaleX = scale.ScaleX;
			frame.ScaleY = scale.ScaleY;
			return(frame);
		}

		virtual void Mark_Overlay_Dirty(void) override
		{
			Video_Mark_Overlay_Dirty();
		}

		virtual void Present_If_Dirty(void) override
		{
			Video_Present_If_Dirty();
		}

		virtual void Present_Now(void) override
		{
			Video_Present_Now();
		}

		virtual bool Movie_Playing(void) const override
		{
			return(Movie_Is_Playing());
		}

		virtual void Play_Sample(char const * name, float volume) override
		{
			if (Options.SoundVolume <= 0.0) {
				return;
			}
			AudioEngine.Play_Sample(MixFileClass::Retrieve(name), AUDIO_GROUP_SFX, volume, 255);
		}

		virtual void Play_Click(void) override
		{
			Sound_Effect(Rule->GenericClick);
		}

		virtual bool Animate_Screens(void) const override
		{
			return(true);
		}

		// The pixel art filter magnifies the frame to the next whole multiple point for
		// point and shrinks that smoothly; the art follows it so both come out alike.
		virtual int Art_Magnification(void) const override
		{
			if (Options.ScaleMode != VIDEO_SCALE_PIXELART) {
				return(1);
			}

			UIFrameRect frame = Frame();
			float ratio = frame.ScaleX < frame.ScaleY ? frame.ScaleX : frame.ScaleY;
			if (ratio <= 1.0f) {
				return(1);
			}

			int factor = (int)ratio;
			if ((float)factor < ratio - 0.001f) {
				factor++;
			}
			return(factor);
		}

		virtual bool Legacy_Dialog_Visible(void) const override
		{
			for (int index = 0; index < g_DialogCount; index++) {
				if (g_Dialogs[index].handle != NULL && IsWindowVisible(g_Dialogs[index].handle)) {
					return(true);
				}
			}
			return(Any_Modeless_Dialog_Visible());
		}

		virtual bool Legacy_Dialogs_Requested(void) const override
		{
			return(Options.LegacyDialogs);
		}

		virtual bool Bitmap_System_Font(void) const override
		{
			return(Options.BitmapSystemFont);
		}

		virtual bool Developer_Keys_Armed(void) const override
		{
			return(Debug_Flag);
		}

		virtual void Clear_Keyboard_Queue(void) override
		{
			Keyboard->Clear();
		}

		virtual void Focus_Main_Window(void) override
		{
			SetFocus(MainWindow);
		}

		virtual bool Take_Capture(void) override
		{
			if (GetCapture() == MainWindow) {
				return(false);
			}
			SetCapture(MainWindow);
			return(true);
		}

		virtual void Release_Capture(void) override
		{
			if (GetCapture() == MainWindow) {
				ReleaseCapture();
			}
		}

		virtual bool Screen_To_Client(int & x, int & y) const override
		{
			POINT point;
			point.x = x;
			point.y = y;
			if (!ScreenToClient(MainWindow, &point)) {
				return(false);
			}
			x = point.x;
			y = point.y;
			return(true);
		}

		virtual bool Key_Down(int virtualkey) const override
		{
			if (GetSystemMetrics(SM_SWAPBUTTON) != 0) {
				if (virtualkey == VK_LBUTTON) {
					virtualkey = VK_RBUTTON;
				} else if (virtualkey == VK_RBUTTON) {
					virtualkey = VK_LBUTTON;
				}
			}
			return((GetAsyncKeyState(virtualkey) & 0x8000) != 0);
		}

		virtual bool Key_Toggled(int virtualkey) const override
		{
			return((GetKeyState(virtualkey) & 1) != 0);
		}

		// Windows keeps its faces in one directory, and the file name is the one the system
		// has used for that face since it shipped.
		virtual std::string System_Font_Path(char const * face) const override
		{
			char directory[MAX_PATH];
			unsigned int length = GetWindowsDirectoryA(directory, MAX_PATH);
			if (length == 0 || length >= MAX_PATH) {
				return(std::string());
			}

			std::string path = std::string(directory) + "\\Fonts\\" + face;
			if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
				return(std::string());
			}

			return(path);
		}

		virtual bool Window_Is_Unicode(void) const override
		{
			return(IsWindowUnicode(MainWindow) != FALSE);
		}

		virtual unsigned int Text_Code_Page(void) const override
		{
			return(GetACP());
		}

		virtual void Apply_Cursor(UICursor cursor) override
		{
			LPCTSTR shape = NULL;
			switch (cursor) {
				case UI_CURSOR_TEXT:
					shape = IDC_IBEAM;
					break;
				case UI_CURSOR_HAND:
					shape = IDC_HAND;
					break;
				case UI_CURSOR_RESIZE_NS:
					shape = IDC_SIZENS;
					break;
				case UI_CURSOR_RESIZE_EW:
					shape = IDC_SIZEWE;
					break;
				case UI_CURSOR_RESIZE_NESW:
					shape = IDC_SIZENESW;
					break;
				case UI_CURSOR_RESIZE_NWSE:
					shape = IDC_SIZENWSE;
					break;
				case UI_CURSOR_MOVE:
					shape = IDC_SIZEALL;
					break;
				case UI_CURSOR_UNAVAILABLE:
					shape = IDC_NO;
					break;
				default:
					break;
			}
			SetCursor(shape != NULL ? LoadCursor(NULL, shape) : Window_Cursor());
		}

		virtual void Restore_Game_Cursor(void) override
		{
			Win_Cursor_Refresh();
			if (!Win_Cursor_Handle_Set_Cursor()) {
				SetCursor(Window_Cursor());
			}
		}

		virtual char const * String(int id) const override
		{
			return(Fetch_String(id));
		}

		virtual void Log(char const * text) override
		{
			DebugString("%s", text);
		}
};


// Built on first use, so the shell's global can take it whatever the order of static
// construction.
UIShellHostClass & UI_Engine_Host(void)
{
	static UIEngineHostClass host;
	return(host);
}


// The service pass of OwnerDraw::Dialog_Message_Handler without its tick: the runner ticks
// itself so that it can drain the screen's intents between the update and the present.
bool UI_Service_Game(void)
{
	static bool inmainloop = false;

	Windows_Message_Handler();

	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH && !Session.NetOpen && !Session.Suspended) {
		if (!inmainloop) {
			inmainloop = true;
			bool ended = Main_Loop();
			inmainloop = false;
			return(ended);
		}
	} else {
		Call_Back();
	}

	return(false);
}


UIResult UI_Run_Modal(UIViewClass & view)
{
	return(UIShell.Run_Modal(view, UI_Service_Game));
}
