/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the notice the frame-sync wait shows: it stands beside the game rather
// than over it, so the wait loop keeps running and reads back what the player pressed.

#include "ui/screens/reconnect/uireconnect.h"

#include "_ui.h"
#include "ui/uishell.h"
#include "ui/uiview.h"


UIReconnectBoxClass::UIReconnectBoxClass(void) = default;


UIReconnectBoxClass::~UIReconnectBoxClass(void)
{
	Hide();
}


bool UIReconnectBoxClass::Show(UIReconnectState const & state)
{
	Hide();

	Presenter = std::make_unique<UIReconnectPresenterClass>();
	Presenter->State = state;
	View = UI_Reconnect_View(*Presenter);

	if (!UIShell.Show_Modeless(*View)) {
		View.reset();
		Presenter.reset();
		return(false);
	}

	return(true);
}


void UIReconnectBoxClass::Update(UIReconnectState const & state)
{
	if (View == nullptr) {
		return;
	}

	Presenter->Drain();
	Presenter->State = state;
	View->Sync();
	UIShell.Refresh();
}


void UIReconnectBoxClass::Hide(void)
{
	if (View != nullptr) {
		UIShell.Hide_Modeless(*View);
		View.reset();
	}
	Presenter.reset();
}


bool UIReconnectBoxClass::Is_Shown(void) const
{
	return(View != nullptr);
}


bool UIReconnectBoxClass::Cancelled(void) const
{
	return(Presenter != nullptr && Presenter->Cancelled);
}


int UIReconnectBoxClass::Take_Kick_Vote(void)
{
	return((Presenter != nullptr) ? Presenter->Take_Kick_Vote() : -1);
}
