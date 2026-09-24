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


enum UIDesyncChoiceType
{
	UI_DESYNC_NONE,
	UI_DESYNC_LOAD,
	UI_DESYNC_CONTINUE,
	UI_DESYNC_QUIT,
};


struct UIDesyncPlayerRow
{
	std::string Name;
	std::string Status;
	std::string Color;
	std::string Mark;
};


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

	float Countdown = 0.0f;
	std::string CountdownColor;
};


class UIDesyncServiceClass
{
	public:
		virtual ~UIDesyncServiceClass(void) = default;
		virtual void Read(UIDesyncState & state) = 0;
		virtual void Say(char const * text) = 0;
		virtual bool Settled(void) = 0;
};


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


std::unique_ptr<UIViewClass> UI_Desync_View(UIDesyncPresenterClass & presenter);

UIDesyncServiceClass & UI_Desync_Service(void);

UIDesyncChoiceType UI_Desync_Dialog(void);
