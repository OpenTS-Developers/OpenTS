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


struct UIReconnectPlayer
{
	std::string Name;
	float Filled = 1.0f;
	std::string Color;
};


struct UIReconnectState
{
	std::vector<UIReconnectPlayer> Players;
	std::vector<std::string> Messages;
	std::string TimeRemaining;
};


class UIReconnectPresenterClass : public UIPresenterClass
{
	public:
		UIReconnectPresenterClass(void) = default;
		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		UIReconnectState State;

		bool Cancelled = false;

		int Take_Kick_Vote(void);

	private:
		int Kick = -1;
};


std::unique_ptr<UIViewClass> UI_Reconnect_View(UIReconnectPresenterClass & presenter);


class UIReconnectBoxClass
{
	public:
		UIReconnectBoxClass(void);
		~UIReconnectBoxClass(void);

		bool Show(UIReconnectState const & state);
		void Update(UIReconnectState const & state);
		void Hide(void);
		bool Is_Shown(void) const;
		bool Cancelled(void) const;
		int Take_Kick_Vote(void);

	private:
		std::unique_ptr<UIReconnectPresenterClass> Presenter;
		std::unique_ptr<UIViewClass> View;
};
