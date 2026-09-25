/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ui/uienginehost.h"

#include "_keyboar.h"
#include "_rules.h"
#include "_ui.h"
#include "_xmouse.h"
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
#include "platform/platform.h"
#include "rules.h"
#include "session.h"
#include "ui/uishell.h"
#include "video.h"
#include "voc.h"
#include "wwmouse.h"

#include <cstdio>


std::string UI_Color_Text(COLORREF color)
{
	char text[16];
	std::snprintf(text, sizeof(text), "#%02x%02x%02x", (unsigned)GetRValue(color), (unsigned)GetGValue(color), (unsigned)GetBValue(color));
	return(std::string(text));
}


class UIEngineHostClass : public UIShellHostClass
{
	public:
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
			if (Platform_Mouse_Captured()) {
				return(false);
			}
			Platform_Capture_Mouse(true);
			return(true);
		}

		virtual void Release_Capture(void) override
		{
			Platform_Capture_Mouse(false);
		}

		virtual bool Key_Down(int virtualkey) const override
		{
			return(Platform_Key_Down(virtualkey));
		}

		virtual bool Key_Toggled(int virtualkey) const override
		{
			return(Platform_Key_Toggled(virtualkey));
		}

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

		virtual void Apply_Cursor(UICursor cursor) override
		{
			Platform_Set_Cursor(Platform_System_Cursor(cursor));
		}


		virtual void Restore_Game_Cursor(void) override
		{
			WWMouseClass * mouse = (WWMouseClass *)MouseCursor;
			if (mouse != NULL) {
				mouse->Refresh_Pointer_Scale();
			}
			if (mouse == NULL || !mouse->Show_Game_Pointer()) {
				Platform_Set_Cursor(Platform_System_Cursor(UI_CURSOR_ARROW));
			}
		}

		virtual std::string Clipboard_Text(void) const override
		{
			return(Platform_Clipboard_Text());
		}

		virtual void Set_Clipboard_Text(std::string const & text) override
		{
			Platform_Set_Clipboard_Text(text);
		}

		virtual char const * String(int id) const override
		{
			return(Fetch_String(id));
		}

		virtual int Milliseconds(void) const override
		{
			return((int)GetTickCount64());
		}

		virtual void Log(char const * text) override
		{
			DebugString("%s", text);
		}
};


UIShellHostClass & UI_Engine_Host(void)
{
	static UIEngineHostClass host;
	return(host);
}


/// <summary>
/// Services the game once while a modal screen is up. A network match keeps running its game
/// loop under the screen, so it stays in step with the other players; otherwise only the
/// maintenance callback runs.
/// </summary>
/// <returns>True once the match has ended, which closes the screen.</returns>
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


/// <summary>
/// Shows a screen modally, servicing the game as the modal screen beneath it does, or with
/// UI_Service_Game when there is none.
/// </summary>
UIResult UI_Run_Modal(UIViewClass & view, bool hideparent)
{
	UIServiceCallback const * running = UIShell.Running_Service();
	if (running != nullptr) {
		return(UIShell.Run_Modal(view, *running, hideparent));
	}
	return(UIShell.Run_Modal(view, UI_Service_Game, hideparent));
}


void UI_Serve_Screen(void)
{
	if (!UIShell.Screen_Shown()) {
		return;
	}

	Windows_Message_Handler();
	UIShell.Serve_Shown_Screen();
}


void UI_On_Archives_Change(void)
{
	UIShell.On_Archives_Change();
}
