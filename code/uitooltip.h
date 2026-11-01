/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "tooltip.h"

#include <memory>
#include <string>

class UIGameTipPresenterClass;
class UIViewClass;


class UIToolTip : public ToolTipManager
{
		typedef ToolTipManager BASECLASS;

	public:
		UIToolTip(void);
		virtual ~UIToolTip() override;

		virtual bool Update(ToolTipText *text) override;
		virtual void Reset(const ToolTipText *text) override;
		virtual void Draw_Current(bool sidebar = false) override;
		virtual void Draw(const ToolTipText *text) override;
		virtual const char *ToolTip_Text(int id) override;

	private:
		bool Open(void);
		void Close(void);
		void Take_Down(void);

		std::unique_ptr<UIGameTipPresenterClass> Presenter;
		std::unique_ptr<UIViewClass> View;
		std::string Sheet;
		bool OpenFailed;

		// The tactical view or the sidebar, whichever the tooltip came up over, in frame coordinates.
		Rect Region;

		bool Drawn;
};
