/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uiscreen.h"


void UIPresenterClass::Queue(UIIntent const & intent)
{
	Pending.push_back(intent);
}


void UIPresenterClass::Drain(void)
{
	std::vector<UIIntent> intents;
	intents.swap(Pending);

	for (UIIntent const & intent : intents) {
		if (Result.has_value()) {
			break;
		}
		Execute(intent);
	}
}


void UIPresenterClass::Discard(void)
{
	Pending.clear();
}


bool UIPresenterClass::Has_Pending(void) const
{
	return(!Pending.empty());
}
