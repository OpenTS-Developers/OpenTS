/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiinput.hh"

#include <RmlUi/Core/SystemInterface.h>

#include <chrono>

class UIShellHostClass;


// RmlUi's view of the wall clock, the host's log, resource naming and string table, the
// pointer shape and the clipboard.
class UIRmlSystemClass : public Rml::SystemInterface
{
	public:
		UIRmlSystemClass(UIShellHostClass & host);

		virtual double GetElapsedTime(void) override;
		virtual bool LogMessage(Rml::Log::Type type, Rml::String const & message) override;
		virtual int TranslateString(Rml::String & translated, Rml::String const & input) override;
		virtual void JoinPath(Rml::String & translated, Rml::String const & documentpath, Rml::String const & path) override;
		virtual void SetMouseCursor(Rml::String const & name) override;
		virtual void SetClipboardText(Rml::String const & text) override;
		virtual void GetClipboardText(Rml::String & text) override;

		// How many errors and assertions RmlUi has logged so far.
		int Error_Count(void) const { return(Errors); }

		// The pointer shape the documents last asked for; the shell shows it while the
		// pointer is theirs and forgets it when a screen closes.
		UICursor Cursor_Request(void) const { return(Cursor); }
		void Reset_Cursor_Request(void) { Cursor = UI_CURSOR_ARROW; }

	private:
		UIShellHostClass & Host;
		std::chrono::steady_clock::time_point Start;
		int Errors = 0;
		UICursor Cursor = UI_CURSOR_ARROW;
};
