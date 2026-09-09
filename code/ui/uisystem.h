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


// RmlUi's view of the engine's clock, debug log, resource naming and string table.
class UISystemInterfaceClass : public Rml::SystemInterface
{
	public:
		UISystemInterfaceClass(void);

		virtual double GetElapsedTime(void) override;
		virtual bool LogMessage(Rml::Log::Type type, Rml::String const & message) override;
		virtual int TranslateString(Rml::String & translated, Rml::String const & input) override;
		virtual void JoinPath(Rml::String & translated, Rml::String const & documentpath, Rml::String const & path) override;

		// How many errors and assertions RmlUi has logged so far.
		int Error_Count(void) const { return(Errors); }

	private:
		unsigned int StartTime;
		int Errors = 0;
};
