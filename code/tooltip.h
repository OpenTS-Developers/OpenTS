/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#pragma once

#include "index.h"
#include "point.h"
#include "rect.h"
#include "vector.h"

struct WindowEvent;

class ToolTip
{
public:
	ToolTip(void) : ID(0), Region(0, 0, 0, 0), Text(0) {}
	~ToolTip(void){}
	ToolTip(ToolTip const &that) : ID(that.ID), Region(that.Region), Text(that.Text) {}

	/*
	 * This is the identifier the tooltip is registered under. No two tooltips held by one
	 * manager may share an identifier, and it is what the manager quotes when it asks for
	 * text that does not come from the string table.
	 */
	unsigned ID;

	/*
	 * This is the rectangle, expressed in the coordinates of the frame the game draws in,
	 * that the mouse must come to rest within for this tooltip to appear.
	 */
	Rect Region;

	/*
	 * This is the string table index of the text to display. If zero, then the tooltip has
	 * no text of its own and the manager is asked to supply some for this identifier.
	 */
	int Text;
};


struct ToolTipText
{
	Point2D Pos;
	int TextWidth;
	int TextHeight;
	char Text[256];
};

class ToolTipManager
{
	public:
		ToolTipManager(void);
		virtual ~ToolTipManager(void);

		void Activate(bool state);

		void Handle_Window_Event(WindowEvent const & event);
		void Service(void);

		int Get_Timer_Delay(void);
		void Set_Timer_Delay(int delay);

		int Get_Lifetime(void);
		void Set_Lifetime(int lifetime);

		int Get_Count(void);

		bool Add(ToolTip const * tooltip);
		void Remove(unsigned id);

		bool Find(unsigned id, ToolTip * tooltip);

		ToolTip const * Find_From_Pos(Point2D &pt);

		virtual bool Update(ToolTipText *text);
		virtual void Reset(ToolTipText const *text);

		bool Process(void);

		virtual void Draw_Current(bool refresh = false);
		virtual void Draw(const ToolTipText *text);
		virtual const char *ToolTip_Text(int id);

		void Reset_Current(void);

		enum {
			TOOLTIP_DELAY = 1000, /// 1 second
			TOOLTIP_LIFETIME = 10000, /// 10 seconds
		};

	private:
		void Start_Timer(int delay);

		// The time Service next acts, while TimerRunning is set.
		bool TimerRunning;
		unsigned long TimerDue;

		/*
		 * If this manager is allowed to display tooltips, then this flag will be true. A
		 * deactivated manager ignores the message traffic entirely, so nothing will appear
		 * however long the mouse rests.
		 */
		bool IsActive;

		/*
		 * This is where the cursor was, in frame coordinates, when the hover delay expired.
		 * It decides which tooltip is chosen and where the tooltip box is placed.
		 */
		Point2D LastMousePos;

		/*
		 * This points to the tooltip the mouse is currently resting over, or NULL when
		 * nothing is on display. It is cleared whenever the tooltip must go away.
		 */
		ToolTip const * CurrentToolTip;

		/*
		 * This is the assembled text and placement of the tooltip on display. It is built
		 * afresh each time a tooltip comes up and handed to the drawing routine as it is.
		 */
		ToolTipText CurrentToolTipInfo;

		/*
		 * This is how long the mouse must rest before a tooltip appears, expressed in
		 * milliseconds. The countdown starts over on every mouse movement.
		 */
		int ToolTipDelay;

		/*
		 * This is how long a tooltip stays on screen before it is taken down again,
		 * expressed in milliseconds.
		 */
		int ToolTipLifetime;

		/*
		 * These are the tooltips registered with this manager. The manager owns its copies
		 * and destroys them all when it goes away.
		 */
		DynamicVectorClass<ToolTip const *> ToolTips;

		/*
		 * This indexes the registered tooltips by their identifier, so that a tooltip can be
		 * found or removed without walking the whole list.
		 */
		IndexClass<unsigned, ToolTip const *> ToolTipIndex;
};
