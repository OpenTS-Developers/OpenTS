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

#include "dialog.hh"


class CCToolTip : public ToolTipManager
{
		typedef ToolTipManager BASECLASS;

	public:
		CCToolTip(HWND hWnd) :			/// Inlined in Windows_Procedure
			BASECLASS(hWnd),
			UseSidebarSurface(false),
			Style(TPF_MAP)
		{
		}

		virtual ~CCToolTip() override {}

		virtual bool Update(ToolTipText *text) override;
		virtual void Reset(const ToolTipText *text) override;
		virtual void Draw_Current(bool sidebar = false) override;
		virtual void Draw(const ToolTipText *text) override;
		virtual const char *ToolTip_Text(int id) override;

	protected:
		/*
		 * If the draw in progress is destined for the sidebar surface, then this flag will
		 * be true. A tooltip that falls upon the sidebar is only drawn when it is set, so
		 * that the tactical pass never paints onto the sidebar's surface.
		 */
		bool UseSidebarSurface;

		/*
		 * This is the text style the tooltip is printed in. It also picks the font that
		 * the tooltip box is measured against.
		 */
		TextPrintType Style;
};
