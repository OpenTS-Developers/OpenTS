/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The screen contract: a presenter that knows no toolkit, the intents a view raises and the
// result a screen closes with. No window, control, RmlUi, ImGui or renderer type appears here.

#pragma once

#include <optional>
#include <string>
#include <vector>


// What a screen reports when it closes. A wrapper maps these onto the values its callers expect.
enum UIResult
{
	UI_RESULT_ACCEPTED,
	UI_RESULT_CANCELLED,
	UI_RESULT_SESSION_ENDED,
	UI_RESULT_FAILED_TO_OPEN,
};


// A user action a view raised, as copied data: never a document node, a borrowed buffer or an
// engine pointer.
struct UIIntent
{
	std::string Name;
	int Value = 0;
	std::string Text;
};


// The clock a timed screen reads. The game supplies the system clock; a test supplies one it
// advances by hand.
class UIClockClass
{
	public:
		virtual ~UIClockClass(void) = default;

		virtual int Milliseconds(void) = 0;
};


class UIPresenterClass
{
	public:
		virtual ~UIPresenterClass(void) = default;

		void Queue(UIIntent const & intent);
		// Executes the queued intents in order at the owner's safe point and drops the rest once
		// one of them produced a result.
		void Drain(void);
		void Discard(void);
		bool Has_Pending(void) const;

		virtual void Execute(UIIntent const & intent) = 0;
		// Copies engine state into the view-model. The owner calls it before every drain, so a
		// timed screen advances here.
		virtual void Refresh(void) = 0;

		std::optional<UIResult> Result;

	private:
		std::vector<UIIntent> Pending;
};
