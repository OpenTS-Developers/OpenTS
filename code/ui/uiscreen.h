/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include <optional>
#include <string>
#include <vector>


enum UIResult
{
	UI_RESULT_ACCEPTED,
	UI_RESULT_CANCELLED,
	UI_RESULT_SESSION_ENDED,
	UI_RESULT_FAILED_TO_OPEN,
};


struct UIIntent
{
	std::string Name;
	int Value = 0;
	std::string Text;
};


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
		void Drain(void);
		void Discard(void);
		bool Has_Pending(void) const;
		virtual void Execute(UIIntent const & intent) = 0;
		virtual void Refresh(void) = 0;

		std::optional<UIResult> Result;

	private:
		std::vector<UIIntent> Pending;
};
