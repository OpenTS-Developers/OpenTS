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
#include <string>
#include <vector>

class UIViewClass;


// One player the game is waiting on: their name, how much of their bar is still filled, and
// the color it has turned as the wait drags on.
struct UIReconnectPlayer
{
	std::string Name;
	float Filled = 1.0f;
	std::string Color;
};


// What the notice shows while the game waits for the others.
struct UIReconnectState
{
	std::vector<UIReconnectPlayer> Players;
	std::vector<std::string> Messages;
	std::string TimeRemaining;
};


// The notice stands beside the game rather than over it, so it answers nothing by closing:
// the buttons record what the player asked for and the wait loop reads it back.
class UIReconnectPresenterClass : public UIPresenterClass
{
	public:
		UIReconnectPresenterClass(void) = default;

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIReconnectState State;

		// True once the player has asked to leave the game.
		bool Cancelled = false;

		// The player a kick was proposed for since this was last read, or -1.
		int Take_Kick_Vote(void);

	private:
		int Kick = -1;
};


// The RmlUi view over a reconnect presenter, bound to reconnect.rml. The presenter must
// outlive it.
std::unique_ptr<UIViewClass> UI_Reconnect_View(UIReconnectPresenterClass & presenter);


// The notice the wait loop shows: a document beside the game where the shell can draw one,
// and nothing otherwise, so the caller opens its Win32 dialog instead. It hides itself when
// it goes out of scope.
class UIReconnectBoxClass
{
	public:
		UIReconnectBoxClass(void);
		~UIReconnectBoxClass(void);

		// False shows nothing, so the caller opens its own Win32 dialog.
		bool Show(UIReconnectState const & state);
		// Executes what the player pressed since the last update, then shows the new state;
		// the two readers below answer for the presses an update has executed.
		void Update(UIReconnectState const & state);
		void Hide(void);

		bool Is_Shown(void) const;
		bool Cancelled(void) const;
		int Take_Kick_Vote(void);

	private:
		std::unique_ptr<UIReconnectPresenterClass> Presenter;
		std::unique_ptr<UIViewClass> View;
};
