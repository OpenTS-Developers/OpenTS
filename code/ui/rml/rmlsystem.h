/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <RmlUi/Core/SystemInterface.h>

#include <chrono>

class UIShellHostClass;


// RmlUi's view of the wall clock, the host's log, resource naming and string table.
class UIRmlSystemClass : public Rml::SystemInterface
{
	public:
		UIRmlSystemClass(UIShellHostClass & host);

		virtual double GetElapsedTime(void) override;
		virtual bool LogMessage(Rml::Log::Type type, Rml::String const & message) override;
		virtual int TranslateString(Rml::String & translated, Rml::String const & input) override;
		virtual void JoinPath(Rml::String & translated, Rml::String const & documentpath, Rml::String const & path) override;

		// How many errors and assertions RmlUi has logged so far.
		int Error_Count(void) const { return(Errors); }

	private:
		UIShellHostClass & Host;
		std::chrono::steady_clock::time_point Start;
		int Errors = 0;
};
