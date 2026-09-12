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
#include "_ui.h"
#include "conquer.h"
#include "data.h"
#include "dbgprint.h"
#include "globals.h"
#include "goptions.h"
#include "keyboard.h"
#include "mainloop.h"
#include "movies.h"
#include "msgloop.h"
#include "session.h"
#include "ui/uishell.h"
#include "video.h"
#include "windlg.h"


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

		virtual bool Movie_Playing(void) const override
		{
			return(Movie_Is_Playing());
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


UIResult UI_Run_Modal(UIRmlViewClass & view)
{
	return(UIShell.Run_Modal(view, UI_Service_Game));
}
