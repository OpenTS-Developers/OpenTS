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


// What the screen answers with. Nothing means the decision came over the network rather than
// from this player, which the caller reads from its state.
enum UIDesyncChoiceType
{
	UI_DESYNC_NONE,
	UI_DESYNC_LOAD,
	UI_DESYNC_CONTINUE,
	UI_DESYNC_QUIT,
};


// One player: the name, how they stand, the color that stands in, and the marker the
// session's master carries.
struct UIDesyncPlayerRow
{
	std::string Name;
	std::string Status;
	std::string Color;
	std::string Mark;
};


// What the screen shows. The master's view carries the three decisions and everyone else's
// carries only the way out, which opens after a wait.
struct UIDesyncState
{
	bool Host = false;
	std::vector<UIDesyncPlayerRow> Players;
	std::vector<std::string> Chat;
	std::string Say;
	bool LoadEnabled = false;
	bool ContinueEnabled = false;
	bool QuitEnabled = false;
	bool Counting = false;
	std::string CountdownText;

	// How much of the bar under the countdown is still filled, from one down to nothing,
	// and the color it turns as the wait runs out.
	float Countdown = 0.0f;
	std::string CountdownColor;
};


// What the screen asks the game for while it stands. The engine implements it; the harness
// hands the screen a recording one.
class UIDesyncServiceClass
{
	public:
		virtual ~UIDesyncServiceClass(void) = default;

		// Copies what the screen shows out of the session.
		virtual void Read(UIDesyncState & state) = 0;

		// Sends a line of chat to everyone still in the session.
		virtual void Say(char const * text) = 0;

		// True once the session has settled the decision, whoever made it.
		virtual bool Settled(void) = 0;
};


// The screen stands until a button is pressed or the session settles the decision without
// this player, as the dialog's loop did.
class UIDesyncPresenterClass : public UIPresenterClass
{
	public:
		explicit UIDesyncPresenterClass(UIDesyncServiceClass & service);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIDesyncState State;
		UIDesyncChoiceType Choice = UI_DESYNC_NONE;

	private:
		UIDesyncServiceClass & Service;
};


// The RmlUi view over a desync presenter, bound to desync.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Desync_View(UIDesyncPresenterClass & presenter);

// The session's service, which lives as long as the dialog it belongs to.
UIDesyncServiceClass & UI_Desync_Service(void);

// Runs the out-of-sync screen, returning what the player pressed. Nothing comes back when the
// session settled the decision under it, and when it could not open.
UIDesyncChoiceType UI_Desync_Dialog(void);
