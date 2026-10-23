/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "battlezoom.h"
#include "battlezoomlayout.h"
#include "data.h"
#include "dbgprint.h"
#include "sdl/sdlwindow.h"
#include "tooltip.h"
#include "uiscalelayout.h"
#include "vidscale.h"

#include <cstdio>

namespace {

Point2D Cursor(0, 0);
Rect Battlefield(0, 16, 1752, 1064);
int Zoom = 1;
int Scale = 1;


class RecordingToolTips : public ToolTipManager
{
public:
	int ID = 0;
	Point2D Position = Point2D(-1, -1);

	virtual char const * ToolTip_Text(int id) override
	{
		ID = id;
		return("tooltip");
	}

	virtual bool Update(ToolTipText * text) override
	{
		Position = text->Pos;
		return(true);
	}
};


bool Hover(int id, Rect const & region, Point2D const & screen, Point2D const & expected)
{
	RecordingToolTips manager;
	ToolTip tooltip;
	tooltip.ID = id;
	tooltip.Region = region;
	manager.Add(&tooltip);
	manager.Activate(true);
	manager.Set_Timer_Delay(0);
	Cursor = screen;
	WindowEvent move;
	move.Type = WINDOW_EVENT_MOUSE_MOVE;
	manager.Handle_Window_Event(move);
	manager.Service();
	return(manager.ID == id && manager.Position == expected);
}

}


bool Main_Window_Cursor_Position(int & x, int & y)
{
	x = Cursor.X;
	y = Cursor.Y;
	return(true);
}


void Window_Point_To_Game(Point2D &)
{
}


void Battle_Zoom_Frame_To_View(Point2D & point)
{
	point = Battle_Zoom_Screen_To_View(point, Battlefield, Zoom);
	point = UI_Scale_Screen_To_Interface(point, Battlefield, Scale);
}


char const * Fetch_String(int)
{
	return("tooltip");
}


void __cdecl DebugString(char const *, ...)
{
}


int Test_Battle_Zoom_Tooltips(void)
{
	int failures = 0;
	auto check = [&failures](bool result, char const * label) {
		std::printf("%-76s %s\n", label, result ? "ok" : "FAILED");
		failures += !result;
	};

	check(Hover(500, Battlefield, Point2D(1200, 600), Point2D(1200, 600)), "tooltip polling keeps unscaled battlefield coordinates");
	Zoom = 2;
	check(Hover(500, Battle_Zoom_View_Rect(Battlefield, Zoom), Point2D(1200, 600), Point2D(600, 308)), "tooltip polling finds and places a tooltip in the zoomed view");

	Battlefield = UI_Scale_Battlefield_Rect(3840, 2160, 3);
	Scale = 3;
	Zoom = 3;
	Point2D const sidebar(Battlefield.Width + 80, 500);
	check(Hover(42, Rect(Battlefield.Width + 60, 490, 60, 50),
		UI_Scale_Interface_To_Screen(sidebar, Battlefield, Scale), sidebar), "tooltip polling uses the scaled sidebar's row and position");
	return(failures);
}
