/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiscreen.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

class UIRmlViewClass;


// One row of the resolution list: the mode and its label as the dialog prints it.
struct UIDisplayMode
{
	int Width = 0;
	int Height = 0;
	std::string Label;
};


// The engine call the display options make when the player accepts. The game supplies one
// that reaches the options; the test harness supplies one that records the call.
class UIDisplayServiceClass
{
	public:
		virtual ~UIDisplayServiceClass(void) = default;

		virtual void Set_Stretch_Movies(bool on) = 0;
};


// What the dialog shows: the modes the display reports, the row of the mode the settings
// hold (-1 when no row matches) and the movie switch.
struct UIDisplayState
{
	std::vector<UIDisplayMode> Modes;
	int Selected = -1;
	bool StretchMovies = false;
};


// Holds the picked row and the switch until the player accepts. Accepting applies the switch
// at once and, when the row differs from the starting one, records the mode for the caller
// to try; the resolution itself changes only after the trial the caller runs.
class UIDisplayPresenterClass : public UIPresenterClass
{
	public:
		UIDisplayPresenterClass(UIDisplayServiceClass & service, UIDisplayState state);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIDisplayState State;
		std::optional<UIDisplayMode> Picked;

	private:
		UIDisplayServiceClass & Service;
		int Initial;
};


// Asks the player to keep a display mode just switched to. Silence counts as a refusal,
// because a bad mode may leave the screen unreadable: the screen cancels itself when the
// timeout passes.
class UIConfirmModePresenterClass : public UIPresenterClass
{
	public:
		enum {
			DEFAULT_TIMEOUT = 10000
		};

		UIConfirmModePresenterClass(UIClockClass & clock, int timeout = DEFAULT_TIMEOUT);

		virtual void Execute(UIIntent const & intent) override;
		// Starts the clock on its first call and cancels the screen once the timeout passes.
		virtual void Refresh(void) override;

		int Seconds;
		bool TimedOut = false;

	private:
		UIClockClass & Clock;
		int Timeout;
		std::optional<int> Deadline;
};


// The RmlUi views, bound to display.rml and confirm.rml. The presenter must outlive its view.
std::unique_ptr<UIRmlViewClass> UI_Display_View(UIDisplayPresenterClass & presenter);
std::unique_ptr<UIRmlViewClass> UI_Confirm_Mode_View(UIConfirmModePresenterClass & presenter);

// The game's service and the state of the display, shared by the Win32 dialog and the RmlUi
// view.
UIDisplayServiceClass & UI_Display_Service(void);
void UI_Display_State(UIDisplayState & state);

// Runs the display options as an RmlUi screen. False means it could not run as one and the
// caller should open its Win32 dialog; otherwise picked carries the mode the player asked to
// try, if any.
bool UI_Display_Dialog(std::optional<UIDisplayMode> & picked);

// Runs the mode confirmation as an RmlUi screen. False means it could not run as one and the
// caller should open its Win32 dialog; otherwise kept says whether the player kept the mode
// before the timeout.
bool UI_Confirm_Mode_Dialog(bool & kept);
