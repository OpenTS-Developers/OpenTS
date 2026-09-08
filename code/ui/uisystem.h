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


// RmlUi's view of the engine's clock, debug log and resource naming.
class UISystemInterfaceClass : public Rml::SystemInterface
{
	public:
		UISystemInterfaceClass(void);

		virtual double GetElapsedTime(void) override;
		virtual bool LogMessage(Rml::Log::Type type, Rml::String const & message) override;
		virtual void JoinPath(Rml::String & translated, Rml::String const & documentpath, Rml::String const & path) override;

	private:
		unsigned int StartTime;
};
