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

/* $Header: /counterstrike/SIDEBAR.CPP 2     3/17/97 1:05a Steve_tall $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SIDEBAR.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 20, 1994                                             *
 *                                                                                             *
 *                  Last Update : October 9, 1996 [JLB]                                        *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SidebarClass::AI -- Handles player clicking on sidebar area.                              *
 *   SidebarClass::Abandon_Production -- Stops production of the object specified.             *
 *   SidebarClass::Activate -- Controls the sidebar activation.                                *
 *   SidebarClass::Activate_Demolish -- Controls the demolish button on the sidebar.           *
 *   SidebarClass::Activate_Repair -- Controls the repair button on the sidebar.               *
 *   SidebarClass::Activate_Upgrade -- Controls the upgrade button on the sidebar.             *
 *   SidebarClass::Add -- Adds a game object to the sidebar list.                              *
 *   SidebarClass::Draw_It -- Renders the sidebar display.                                     *
 *   SidebarClass::Factory_Link -- Links a factory to a sidebar strip.                         *
 *   SidebarClass::Init_Clear -- Sets sidebar to a known (and deactivated) state               *
 *   SidebarClass::Init_IO -- Adds buttons to the button list                                  *
 *   SidebarClass::Init_Theater -- Performs theater-specific initialization                    *
 *   SidebarClass::One_Time -- Handles the one time game initializations.                      *
 *   SidebarClass::One_Time -- Handles the one time game initializations.                      *
 *   SidebarClass::Recalc -- Examines the sidebar data and updates it as necessary.            *
 *   SidebarClass::Refresh_Cells -- Intercepts the refresh, looking for sidebar controls.      *
 *   SidebarClass::SBGadgetClass::Action -- Special function that controls the mouse over the s*
 *   SidebarClass::Scroll -- Handles scrolling the sidebar object strip.                       *
 *   SidebarClass::Set_Current -- Sets a specified object that controls the sidebar display.   *
 *   SidebarClass::SidebarClass -- Default constructor for the sidebar.                        *
 *   SidebarClass::SidebarClass -- This is the no initialization constructor for the sidebar.  *
 *   SidebarClass::StripClass::AI -- Input and AI processing for the side strip.               *
 *   SidebarClass::StripClass::Abandon_Produ -- Abandons production associated with sidebar.   *
 *   SidebarClass::StripClass::Activate -- Adds the strip buttons to the input system.         *
 *   SidebarClass::StripClass::Add -- Add an object to the side strip.                         *
 *   SidebarClass::StripClass::Deactivate -- Removes the side strip buttons from the input syst*
 *   SidebarClass::StripClass::Draw_It -- Render the sidebar display.                          *
 *   SidebarClass::StripClass::Factory_Link -- Links a factory to a sidebar button.            *
 *   SidebarClass::StripClass::Flag_To_Redra -- Flags the sidebar strip to be redrawn.         *
 *   SidebarClass::StripClass::Get_Special_Cameo -- Fetches the special event cameo shape.     *
 *   SidebarClass::StripClass::Init_Clear -- Sets sidebar to a known (and deactivated) state   *
 *   SidebarClass::StripClass::Init_IO -- Adds buttons to the button list                      *
 *   SidebarClass::StripClass::Init_Theater -- Performs theater-specific initialization        *
 *   SidebarClass::StripClass::One_Time -- Performs one time actions necessary for the side str*
 *   SidebarClass::StripClass::Recalc -- Revalidates the current sidebar list of objects.      *
 *   SidebarClass::StripClass::Scroll -- Causes the side strip to scroll.                      *
 *   SidebarClass::StripClass::SelectClass:: -- Action function when buildable cameo is selecte*
 *   SidebarClass::StripClass::SelectClass:: -- Assigns special values to a buildable select bu*
 *   SidebarClass::StripClass::SelectClass::SelectClass -- Default constructor.                *
 *   SidebarClass::StripClass::StripClass -- Default constructor for the side strip class.     *
 *   SidebarClass::Which_Column -- Determines which column a given type should appear.         *
 *   SidebarClass::Zoom_Mode_Control -- Handles the zoom mode toggle operation.                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "sidebar.h"

#include "_bench.h"
#include "_convert.h"
#include "_map.h"
#include "_mixfile.h"
#include "_rtti.h"
#include "_rules.h"
#include "_surface.h"
#include "_tooltip.h"
#include "bench.h"
#include "building.h"
#include "cctooltip.h"
#include "conquer.h"
#include "convert.h"
#include "data.h"
#include "dialog.h"
#include "draw.h"
#include "event.h"
#include "factory.h"
#include "font.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "incdec.h"
#include "language\language.h"
#include "map.h"
#include "mixfile.h"
#include "movie.h"
#include "queue.h"
#include "rect.h"
#include "rules.h"
#include "savestream.h"
#include "scheme.h"
#include "session.h"
#include "shapeset.h"
#include "super.h"
#include "suprtype.h"
#include "surface.h"
#include "voc.h"
#include "vox.h"

#include "bench.hh"
#include "color.hh"

#include <algorithm>

ShapeSet const * SidebarClass::SidebarShape = NULL;
ShapeSet const * SidebarClass::SidebarMiddleShape = NULL;
ShapeSet const * SidebarClass::SidebarBottomShape = NULL;
ShapeSet const * SidebarClass::SidebarAddonShape = NULL;

/***************************************************************************
**	This points to the main sidebar shapes. These include the upgrade and
**	repair buttons.
*/
//TheaterType SidebarClass::StripClass::LastTheater = THEATER_NONE;

enum ButtonNumberType {
	BUTTON_RADAR = 100,
	BUTTON_REPAIR,
	BUTTON_POWER,
	BUTTON_SELL,
	BUTTON_UPGRADE,
	BUTTON_WAYPOINT
};

/*
**	Sidebar buttons
*/
SidebarClass::SBGadgetClass SidebarClass::Background;
ShapeButtonClass SidebarClass::Repair;
ShapeButtonClass SidebarClass::Power;
ShapeButtonClass SidebarClass::Upgrade;
ShapeButtonClass SidebarClass::Waypoint;
ShapeButtonClass SidebarClass::StripClass::UpButton[COLUMNS];
ShapeButtonClass SidebarClass::StripClass::DownButton[COLUMNS];
SidebarClass::StripClass::SelectClass
SidebarClass::StripClass::SelectButton[COLUMNS][MAX_SLOTS];

bool SidebarClass::IsToBlitSidebar = false;

/*
**	Shape data pointers
*/
ShapeSet * SidebarClass::StripClass::LogoShapes = NULL;
ShapeSet const * SidebarClass::StripClass::ClockShapes;
ShapeSet const * SidebarClass::StripClass::RechargeClockShapes;
ShapeSet const * SidebarClass::StripClass::DarkenShapes;

void Print_Cameo_Text(char const * string, Point2D const & point, Rect const & cliprect, int maxlinelen);


/// <summary>
/// Default constructor for the sidebar background gadget.
/// This gadget exists to cover the sidebar so that the mouse reverts to its normal shape
/// whenever it strays over there. The sidebar gives the gadget its real position and size
/// when it lays itself out.
/// </summary>
SidebarClass::SBGadgetClass::SBGadgetClass(void) :
	GadgetClass(0, 0, 1, 1, LEFTUP)
{
}


/***********************************************************************************************
 * SidebarClass::SidebarClass -- Default constructor for the sidebar.                          *
 *                                                                                             *
 *    Constructor for the sidebar handler. It basically sets up the sidebar to the empty       *
 *    condition.                                                                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SidebarClass::SidebarClass(void) :
	BASECLASS(),
	IsCameoText(false),
	IsSidebarActive(false),
	IsToRedrawCredits(false),
	IsToRedraw(true),
	IsRepairActive(false),
	IsUpgradeActive(false),
	IsDemolishActive(false)
{
	/*
	**	Set up the coordinates for the sidebar strips. These coordinates are for
	**	the upper left corner.
	*/
	new (&Column[0]) StripClass(InitClass());
	new (&Column[1]) StripClass(InitClass());

	Column[0].X = COLUMN_ONE_X;
	Column[0].Y = COLUMN_ONE_Y;
	Column[0].ObjectRect = Rect(Column[0].X, Column[0].Y, StripClass::OBJECT_WIDTH, StripClass::OBJECT_HEIGHT * Max_Visible());
	Column[1].X = COLUMN_TWO_X;
	Column[1].Y = COLUMN_TWO_Y;
	Column[1].ObjectRect = Rect(Column[1].X, Column[1].Y, StripClass::OBJECT_WIDTH, StripClass::OBJECT_HEIGHT * Max_Visible());
}


/// <summary>
/// Turns the cameo cost text on or off.
/// This routine is called when the player changes the option. With the text enabled, every
/// build cameo is captioned with its cost, so the sidebar is flagged to redraw itself.
/// </summary>
/// <param name="on">Should the cameo text be displayed?</param>
void SidebarClass::Toggle_Cameo_Text(bool on)
{
	if (on != IsCameoText) {
		IsCameoText = on;
		IsToRedraw = true;
		Flag_To_Redraw();
	}
}


/***********************************************************************************************
 * SidebarClass::One_Time -- Handles the one time game initializations.                        *
 *                                                                                             *
 *    This routine is used to load the graphic data that is needed by the sidebar display. It  *
 *    should only be called ONCE.                                                              *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Only call this routine once when the game first starts.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::One_Time(void)
{
	BASECLASS::One_Time();

	Column[0].One_Time(0);
	Column[1].One_Time(1);

	StripClass::RechargeClockShapes = (ShapeSet const *)MFCD::Retrieve("RCLOCK2.SHP");
	StripClass::ClockShapes = (ShapeSet const *)MFCD::Retrieve("GCLOCK2.SHP");
}


/***********************************************************************************************
 * SidebarClass::Init_Clear -- Sets sidebar to a known (and deactivated) state                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Init_Clear(void)
{

	BASECLASS::Init_Clear();

	IsToRedraw = true;
	IsRepairActive = false;
	IsUpgradeActive = false;
	IsDemolishActive = false;

	Column[0].Init_Clear();
	Column[1].Init_Clear();

	Activate(false);
}


/***********************************************************************************************
 * SidebarClass::Init_IO -- Adds buttons to the button list                                    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Init_IO(void)
{
	BASECLASS::Init_IO();

	SidebarRect.X = TacticalRect.X + TacticalRect.Width;
	SidebarRect.Y = SIDE_Y;
	SidebarRect.Width = 641 - SidebarRect.X;
	SidebarRect.Height = (TacticalRect.Y - SidebarRect.Y) + TacticalRect.Height;

	/*
	**	Add the sidebar's buttons only if we're not in editor mode.
	*/
	if (!Debug_Map) {
		int xoff = -480;
		int yoff = 3;

		Repair.IsSticky = true;
		Repair.ID = BUTTON_REPAIR;
		Repair.X = SidebarRect.X;
		Repair.Y = SidebarRect.Y;
		Repair.DrawOffsetX = xoff;
		Repair.DrawOffsetY = yoff;
		Repair.DrawOnSidebar = true;
		Repair.ShapeDrawer = SidebarDrawer;
		Repair.IsPressed = false;
		Repair.IsToggleType = true;
		Repair.ReflectButtonState = true;

		Upgrade.IsSticky = true;
		Upgrade.ID = BUTTON_UPGRADE;
		Upgrade.X = Repair.X + BUTTON_SPACING;
		Upgrade.Y = Repair.Y;
		Upgrade.DrawOffsetX = xoff;
		Upgrade.DrawOffsetY = yoff;
		Upgrade.DrawOnSidebar = true;
		Upgrade.ShapeDrawer = SidebarDrawer;
		Upgrade.IsPressed = false;
		Upgrade.IsToggleType = true;
		Upgrade.ReflectButtonState = true;

		Power.IsSticky = true;
		Power.ID = BUTTON_POWER;
		Power.X = Upgrade.X + BUTTON_SPACING;
		Power.Y = Upgrade.Y;
		Power.DrawOffsetX = xoff;
		Power.DrawOffsetY = yoff;
		Power.DrawOnSidebar = true;
		Power.ShapeDrawer = SidebarDrawer;
		Power.IsPressed = false;
		Power.IsToggleType = true;
		Power.ReflectButtonState = true;

		Waypoint.IsSticky = true;
		Waypoint.ID = BUTTON_WAYPOINT;
		Waypoint.X = Power.X + BUTTON_SPACING;
		Waypoint.Y = Power.Y;
		Waypoint.DrawOffsetX = xoff;
		Waypoint.DrawOffsetY = yoff;
		Waypoint.DrawOnSidebar = true;
		Waypoint.ShapeDrawer = SidebarDrawer;
		Waypoint.IsPressed = false;
		Waypoint.IsToggleType = true;
		Waypoint.ReflectButtonState = true;

		Waypoint.Enable();

		Column[0].Init_IO(0);
		Column[1].Init_IO(1);

		Reposition_Sidebar();

		/*
		**	If a game was loaded & the sidebar was enabled, pop it up now
		*/
		if (IsSidebarActive) {
			IsSidebarActive = false;
			Activate(1);
//			Background.Zap();
//			Add_A_Button(Background);
		}
	}
}


/// <summary>
/// Loads the sidebar artwork that belongs to the player's house.
/// This routine is called whenever the house the player controls is established. It builds
/// the sidebar's own drawing translation table from the sidebar palette and then fetches
/// the frame, mode button and scroll arrow shapes that are drawn through it.
/// </summary>
void SidebarClass::Init_For_House(void)
{
	int i;

	BASECLASS::Init_For_House();

	PaletteClass pal;

	memmove(&pal, MFCD::Retrieve("SIDEBAR.PAL"), sizeof(pal));

	for (i = 0; i < PaletteClass::COLOR_COUNT; i++) {
		pal[i] = RGBClass(pal[i].Get_Red() * 4, pal[i].Get_Green() * 4, pal[i].Get_Blue() * 4);
	}

	if (SidebarDrawer != NULL) {
		delete SidebarDrawer;
		SidebarDrawer = NULL;
	}

	SidebarDrawer = new ConvertClass(pal, pal, *VisibleSurface, 1, false);

	Upgrade.Set_Shape((ShapeSet *)MFCD::Retrieve("SELL.SHP"));
	Upgrade.ShapeDrawer = SidebarDrawer;
	Power.Set_Shape((ShapeSet *)MFCD::Retrieve("POWER.SHP"));
	Power.ShapeDrawer = SidebarDrawer;
	Waypoint.Set_Shape((ShapeSet *)MFCD::Retrieve("WAYP.SHP"));
	Waypoint.ShapeDrawer = SidebarDrawer;
	Repair.Set_Shape((ShapeSet *)MFCD::Retrieve("REPAIR.SHP"));
	Repair.ShapeDrawer = SidebarDrawer;

	SidebarShape = (ShapeSet *)MFCD::Retrieve("SIDE1.SHP");
	SidebarMiddleShape = (ShapeSet *)MFCD::Retrieve("SIDE2.SHP");
	SidebarBottomShape = (ShapeSet *)MFCD::Retrieve("SIDE3.SHP");
	SidebarAddonShape = (ShapeSet *)MFCD::Retrieve("ADDON.SHP");

	for (i = 0; i < COLUMNS; i++) {
		StripClass::UpButton[i].Set_Shape((ShapeSet *)MFCD::Retrieve("R-UP.SHP"));
		StripClass::UpButton[i].ShapeDrawer = SidebarDrawer;
		StripClass::DownButton[i].Set_Shape((ShapeSet *)MFCD::Retrieve("R-DN.SHP"));
		StripClass::DownButton[i].ShapeDrawer = SidebarDrawer;
	}
}


/***********************************************************************************************
 * SidebarClass::Reload_Sidebar -- Loads appropriate sidebar shapes depending on house         *
 *                                                                                             *
 * INPUT:  none                                                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/18/1996 BWG : Created.                                                                  *
 *=============================================================================================*/
void SidebarClass::Reload_Sidebar(void)
{
	Column[0].X = COLUMN_ONE_X;
	Column[0].Y = COLUMN_ONE_Y;
	Column[0].ObjectRect = Rect(Column[0].X, Column[0].Y, StripClass::OBJECT_WIDTH, (StripClass::OBJECT_HEIGHT) * Max_Visible());

	Column[1].X = COLUMN_TWO_X;
	Column[1].Y = COLUMN_ONE_Y;
	Column[1].ObjectRect = Rect(Column[1].X, Column[1].Y, StripClass::OBJECT_WIDTH, (StripClass::OBJECT_HEIGHT) * Max_Visible());
}


/***********************************************************************************************
 * SidebarClass::Which_Column -- Determines which column a given type should appear.           *
 *                                                                                             *
 *    Use this function to resolve what column the specified object type should be placed      *
 *    into.                                                                                    *
 *                                                                                             *
 * INPUT:   otype -- Pointer to the object type class of the object in question.               *
 *                                                                                             *
 * OUTPUT:  Returns with the column number that the object should be placed in.                *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/01/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int SidebarClass::Which_Column(RTTIType type)
{
	if (type == RTTI_BUILDINGTYPE || type == RTTI_BUILDING) {
		return(0);
	}
	return(1);
}


/***********************************************************************************************
 * SidebarClass::Factory_Link -- Links a factory to a sidebar strip.                           *
 *                                                                                             *
 *    This routine will link the specified factory to the sidebar strip. A factory must be     *
 *    linked to the sidebar so that as the factory production progresses, the sidebar will     *
 *    show the production progress.                                                            *
 *                                                                                             *
 * INPUT:   factory  -- The factory number to attach.                                          *
 *                                                                                             *
 *          type     -- The object type number.                                                *
 *                                                                                             *
 *          id       -- The object sub-type number.                                            *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully attached to the sidebar strip?                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Factory_Link(FactoryClass * factory, RTTIType type, int id)
{
	assert((unsigned)type < RTTI_COUNT);
	assert(id >= 0);

	return(Column[Which_Column(type)].Factory_Link(factory, type, id));
}


/***********************************************************************************************
 * SidebarClass::Activate_Repair -- Controls the repair button on the sidebar.                 *
 *                                                                                             *
 *    Use this routine to turn the repair sidebar button on and off. Typically, the button     *
 *    is enabled when the currently selected structure is friendly and damaged.                *
 *                                                                                             *
 * INPUT:   control  -- The controls how the button is to be activated or deactivated;         *
 *                      0  -- Turn button off.                                                 *
 *                      1  -- Turn button on.                                                  *
 *                      -1 -- Toggle button state.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the button previously activated?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate_Repair(int control)
{
	bool old = IsRepairActive;

	if (control == -1) {
		control = IsRepairActive ? 0 : 1;
	}
	switch (control) {
		case 1:
			IsRepairActive = true;
			break;

		default:
		case 0:
			IsRepairActive = false;
			break;
	}
	if (old != IsRepairActive) {
		Flag_To_Redraw();
		IsToRedraw = true;

		if (!IsRepairActive) {
//			Help_Text(TXT_NONE);
			Set_Default_Mouse(MOUSE_NORMAL, false);
		}
	}
	return(old);
}


/***********************************************************************************************
 * SidebarClass::Activate_Upgrade -- Controls the upgrade button on the sidebar.               *
 *                                                                                             *
 *    Use this routine to turn the upgrade sidebar button on and off. Typically, the button    *
 *    is enabled when the currently selected structure can be upgraded and disabled otherwise. *
 *                                                                                             *
 * INPUT:   control  -- The controls how the button is to be activated or deactivated;         *
 *                      0  -- Turn button off.                                                 *
 *                      1  -- Turn button on.                                                  *
 *                      -1 -- Toggle button state.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the button previously activated?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate_Upgrade(int control)
{
	bool old = IsUpgradeActive;
	if (control == -1) {
		control = IsUpgradeActive ? 0 : 1;
	}
	switch (control) {
		case 1:
			IsUpgradeActive = true;
			break;

		default:
		case 0:
			IsUpgradeActive = false;
			break;
	}
	if (old != IsUpgradeActive) {
		Flag_To_Redraw();
		IsToRedraw = true;
		if (!IsUpgradeActive) {
			Set_Default_Mouse(MOUSE_NORMAL, false);
		}
	}
	return(old);
}


/***********************************************************************************************
 * SidebarClass::Activate_Demolish -- Controls the demolish button on the sidebar.             *
 *                                                                                             *
 *    Use this routine to turn the demolish/dismantle sidebar button on and off. Typically,    *
 *    the button is enabled when a friendly building is selected and disabled otherwise.       *
 *                                                                                             *
 * INPUT:   control  -- The controls how the button is to be activated or deactivated;         *
 *                      0  -- Turn button off.                                                 *
 *                      1  -- Turn button on.                                                  *
 *                      -1 -- Toggle button state.                                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the button previously activated?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate_Demolish(int control)
{
	bool old = IsDemolishActive;

	if (control == -1) {
		control = IsDemolishActive ? 0 : 1;
	}
	switch (control) {
		case 1:
			IsDemolishActive = true;
			break;

		default:
		case 0:
			IsDemolishActive = false;
			break;
	}
	if (old != IsDemolishActive) {
		Flag_To_Redraw();
		IsToRedraw = true;
		if (!IsDemolishActive) {
			Set_Default_Mouse(MOUSE_NORMAL, false);
		}
	}
	return(old);
}


/***********************************************************************************************
 * SidebarClass::Add -- Adds a game object to the sidebar list.                                *
 *                                                                                             *
 *    This routine is used to add a game object to the sidebar. Call this routine when a       *
 *    factory type building is created. It handles the case of adding an item that has already *
 *    been added -- it just ignores it.                                                        *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object that is being added.                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the object added to the sidebar?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/17/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Add(RTTIType type, int id)
{
	assert((unsigned)type < RTTI_COUNT);

	/*
	**	Add the sidebar only if we're not in editor mode.
	*/
	if (!Debug_Map) {
		int column = Which_Column(type);

		if (Column[column].Add(type, id)) {
			Activate(1);
			IsToRedraw = true;
			Flag_To_Redraw();
			return(true);
		}
		return(false);
	}

	return(false);
}


/***********************************************************************************************
 * SidebarClass::Scroll -- Handles scrolling the sidebar object strip.                         *
 *                                                                                             *
 *    This routine is used to scroll the sidebar strip of objects. The strip appears whenever  *
 *    a building is selected that can produce units. If the number of units to produce is      *
 *    greater than what the sidebar can hold, this routine is used to scroll the other object  *
 *    into view so they can be selected.                                                       *
 *                                                                                             *
 * INPUT:   up -- Should the scroll be upwards? Upward scrolling reveals object that are       *
 *                later in the list of objects.                                                *
 *                                                                                             *
 * OUTPUT:  bool; Did scrolling occur?                                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Scroll(bool up, int column)
{
	if (_dialog_count != 0) {
		return(false);
	}

	if (column == -1) {
		bool scr = false;
		if (Column[0].Scroll(up)) {
			scr = true;
		}
		if (Column[1].Scroll(up)) {
			scr = true;
		}
		if (scr) {
			IsToRedraw = true;
			Flag_To_Redraw();
			return(true);
		}
		if (!scr) {
			Sound_Effect(Rule->ScoldSound);
		}
		return(false);
	}

	if (Column[column].Scroll(up)) {
		// No need to redraw the whole sidebar juts because we scrolled a strip is there? ST - 10/15/96 7:29PM
		//IsToRedraw = true;
		Flag_To_Redraw();
		return(true);
	}

	return(false);
}


/// <summary>
/// Handles paging the sidebar object strips.
/// This routine works just like Scroll, save that the strips move a whole screenful of
/// objects at a time. The player is scolded with a sound effect when there is nothing left
/// in that direction to page to.
/// </summary>
/// <param name="up">Should the paging be upwards?</param>
/// <param name="column">The strip to page, or -1 to page both of them together.</param>
/// <returns>bool; Did any paging occur?</returns>
bool SidebarClass::Page(bool up, int column)
{
	if (column == -1) {
		bool scr = false;
		if (Column[0].Page(up)) {
			scr = true;
		}
		if (Column[1].Page(up)) {
			scr = true;
		}
		if (scr) {
			IsToRedraw = true;
			Flag_To_Redraw();
			return(true);
		}
		if (!scr) {
			Sound_Effect(Rule->ScoldSound);
		}
		return(false);
	}

	if (Column[column].Page(up)) {
		// No need to redraw the whole sidebar juts because we scrolled a strip is there? ST - 10/15/96 7:29PM
		//IsToRedraw = true;
		Flag_To_Redraw();
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * SidebarClass::Draw_It -- Renders the sidebar display.                                       *
 *                                                                                             *
 *    This routine performs the actual drawing of the sidebar display.                         *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the sidebar imagery changed at all?                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *   12/31/1994 JLB : Split rendering off into the sidebar strip class.                        *
 *=============================================================================================*/
void SidebarClass::Draw_It(bool complete)
{
	complete = complete || IsForceCompleteRedraw;
	Map.LastDrawRect = RECT_NONE;

	BASECLASS::Draw_It(complete);

	BStart(BENCH_SIDEBAR);

	Surface * old = LogicalSurface;
	LogicalSurface = SidebarSurface;

	Rect window(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

	if (IsSidebarActive && (IsToRedraw || complete) && !Debug_Map) {
		if (complete || Column[0].IsToRedraw || Column[1].IsToRedraw) {

			int y = SidebarRect.Y;

			/*
			**	The sidebar shape is too big in 640x400 so it needs to be drawn in three chunks.
			*/
			Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarShape, 0, Point2D(0, y), window, SHAPE_WIN_REL);
			y += SidebarClass::SidebarShape->Get_Height();

			for (int i = 0; i < Max_Visible(); i++) {
				Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarMiddleShape, 0, Point2D(0, y), window, SHAPE_WIN_REL);
				y += SidebarMiddleShape->Get_Height();
			}

			Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarBottomShape, 0, Point2D(0, y), window, SHAPE_WIN_REL);
			Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarAddonShape, 0, Point2D(0, y + SidebarBottomShape->Get_Height()), window, SHAPE_WIN_REL);

			Column[0].IsToRedraw = true;
			Column[1].IsToRedraw = true;
		}

		Repair.Draw_Me(true);
		Upgrade.Draw_Me(true);
		Power.Draw_Me(true);
		Waypoint.Draw_Me(true);
		IsToBlitSidebar = true;
	}

	/*
	**	Draw the side strip elements by calling their respective draw functions.
	*/
	if (IsSidebarActive) {
		Column[0].Draw_It(complete);
		Column[1].Draw_It(complete);
	}
	if (Repair.IsDrawn) {
		IsToBlitSidebar = true;
		Repair.IsDrawn = false;
	}
	if (Upgrade.IsDrawn) {
		IsToBlitSidebar = true;
		Upgrade.IsDrawn = false;
	}
	if (Power.IsDrawn) {
		IsToBlitSidebar = true;
		Power.IsDrawn = false;
	}
	if (Waypoint.IsDrawn) {
		IsToBlitSidebar = true;
		Waypoint.IsDrawn = false;
	}
	if (ToolTips != NULL) {
		ToolTips->Draw_Current(true);
	}
	IsToRedraw = false;
	IsForceCompleteRedraw = false;

	Blit_Sidebar(complete);
	LogicalSurface = old;

	BEnd(BENCH_SIDEBAR);
}


/// <summary>
/// Copies the sidebar onto the visible screen.
/// This routine is the last step of the sidebar's redraw. When nothing but the credits have
/// changed, only that much is copied, so an idle sidebar costs almost nothing.
/// </summary>
/// <param name="complete">Should the entire sidebar be copied rather than just the portion
/// that changed?</param>
void SidebarClass::Blit_Sidebar(bool complete)
{
	if (IsSidebarActive && GameActive && ScenarioActive) {

		if (!IsToBlitSidebar && !complete) {
			IsToBlitSidebar = false;
			if (Map.LastDrawRect == RECT_NONE) {
				if (IsToRedrawCredits) {
					VisibleSurface->Blit_From(
						Rect((Options.IsSidebarOnRight ? TacticalRect.Width : 0), 0, SIDE_WIDTH, CREDITS_HEIGHT),
						*SidebarSurface,
						Rect(0, 0, SIDE_WIDTH, CREDITS_HEIGHT),
						false,
						true
					);
					IsToRedrawCredits = false;
				}
				IsToBlitSidebar = false;
				return;
			}
		} else {
			IsToBlitSidebar = true;
		}

		if (Map.LastDrawRect == RECT_NONE && !complete) {
			VisibleSurface->Blit_From(Rect((Options.IsSidebarOnRight ? TacticalRect.Width : 0), 0, SIDE_WIDTH, CREDITS_HEIGHT), *SidebarSurface, Rect(0, 0, SIDE_WIDTH, CREDITS_HEIGHT));
			VisibleSurface->Blit_From(Rect((Options.IsSidebarOnRight ? TacticalRect.Width : 0), SIDE_BODY_Y, SIDE_WIDTH, SidebarSurface->Get_Height() - SIDE_BODY_Y), *SidebarSurface, Rect(0, SIDE_BODY_Y, SIDE_WIDTH, SidebarSurface->Get_Height() - SIDE_BODY_Y));
		} else if (!IsToBlitSidebar) {
			VisibleSurface->Blit_From(Rect(Map.LastDrawRect.X + (Options.IsSidebarOnRight ? TacticalRect.Width : 0), Map.LastDrawRect.Y, Map.LastDrawRect.Width, Map.LastDrawRect.Height), *SidebarSurface, Map.LastDrawRect);
		} else {
			Rect sb_rect = SidebarSurface->Get_Rect();
			VisibleSurface->Blit_From(Rect((Options.IsSidebarOnRight ? TacticalRect.Width : 0), 0, sb_rect.Width, sb_rect.Height), *SidebarSurface, Rect(0, 0, sb_rect.Width, sb_rect.Height));
		}
	}
	IsToBlitSidebar = false;
}


/***********************************************************************************************
 * SidebarClass::AI -- Handles player clicking on sidebar area.                                *
 *                                                                                             *
 *    This routine handles the processing necessary when the player clicks on the sidebar.     *
 *    Typically, this is selection of the item to build.                                       *
 *                                                                                             *
 * INPUT:   input -- Reference to the keyboard input value.                                    *
 *                                                                                             *
 *          x,y   -- Mouse coordinates at time of input.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the click handled?                                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/28/94   JLB : Created.                                                                 *
 *   11/11/1994 JLB : Processes input directly.                                                *
 *   12/26/1994 JLB : Uses factory manager class for construction handling.                    *
 *   12/31/1994 JLB : Simplified to use the sidebar strip class handlers.                      *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *   06/27/1995 JLB : <TAB> key toggles sidebar.                                               *
 *=============================================================================================*/
void SidebarClass::AI(KeyNumType & input, Point2D const & xy)
{
	Point2D xy_rel = xy - Point2D(480, 0);

	if (!Debug_Map) {
		Activate(1);	// Force the sidebar always on in Win95 mode
	}

	if (!Debug_Map) {
		Column[0].AI(input, xy_rel);
		Column[1].AI(input, xy_rel);
	}

	if (IsSidebarActive) {

		/*
		**	If there are any buildings in the payer's inventory, then allow the repair
		**	option.
		*/
		if (PlayerPtr->CurBuildings > 0) {
			Activate_Repair(true);
		} else {
			Activate_Repair(false);
		}

		if (input == (BUTTON_REPAIR|KN_BUTTON)) {
			Repair_Mode_Control(-1);
		}

		if (input == (BUTTON_POWER|KN_BUTTON)) {
			Power_Mode_Control(-1);
		}

		if (input == (BUTTON_WAYPOINT|KN_BUTTON)) {
			Waypoint_Mode_Control(-1, false);
		}

		if (input == (BUTTON_UPGRADE|KN_BUTTON)) {
			Sell_Mode_Control(-1);
		}
	}

	if ((!IsRepairMode) && Repair.IsOn) {
		Repair.Turn_Off();
	}

	if ((!IsSellMode) && Upgrade.IsOn) {
		Upgrade.Turn_Off();
	}

	if ((!IsPowerMode) && Power.IsOn) {
		Power.Turn_Off();
	}

	if ((!IsWaypointMode) && Waypoint.IsOn) {
		Waypoint.Turn_Off();
	}

	BASECLASS::AI(input, xy);
}


/***********************************************************************************************
 * SidebarClass::Recalc -- Examines the sidebar data and updates it as necessary.              *
 *                                                                                             *
 *    Occasionally a factory gets destroyed. This routine must be called in such a case        *
 *    because it might be possible that sidebar object need to be removed. This routine will   *
 *    examine all existing objects in the sidebar class and if no possible factory can         *
 *    produce it, then it will be removed.                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine is exhaustive and thus time consuming. Only call it when really    *
 *             necessary. Such as when a factory is destroyed rather than when a non-factory   *
 *             is destroyed.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/30/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Recalc(void)
{
	bool redraw = false;

	if (Column[0].Recalc()) {
		redraw = true;
	}
	if (Column[1].Recalc()) {
		redraw = true;
	}
	if (redraw) {
		IsToRedraw = true;
		Flag_To_Redraw();
	}
}


/***********************************************************************************************
 * SidebarClass::Activate -- Controls the sidebar activation.                                  *
 *                                                                                             *
 *    Use this routine to turn the sidebar on or off. This routine handles updating the        *
 *    necessary flags.                                                                         *
 *                                                                                             *
 * INPUT:   control  -- Tells what to do with the sidebar according to the following:          *
 *                         0 = Turn sidebar off.                                               *
 *                         1 = Turn sidebar on.                                                *
 *                         -1= Toggle sidebar on or off.                                       *
 *                                                                                             *
 * OUTPUT:  bool; Was the sidebar already on?                                                  *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/09/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Activate(int control)
{
	bool old = IsSidebarActive;

	if (Session.Play && Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH)
		return(old);

	/*
	**	Determine the new state of the sidebar.
	*/
	switch (control) {
		case -1:
			IsSidebarActive = IsSidebarActive == false;
			break;

		case 1:
			IsSidebarActive = true;
			break;

		default:
		case 0:
			IsSidebarActive = false;
			break;
	}

	/*
	**	Only if there is a change in the state of the sidebar will anything
	**	be done to change it.
	*/
	if (IsSidebarActive != old) {

		/*
		**	If the sidebar is activated but was on the right side of the screen, then
		**	activate it on the left side of the screen.
		*/
		if (IsSidebarActive) {
			Reposition_Sidebar();
			IsToRedraw = true;
			Repair.Zap();
			Add_A_Button(Repair);
			Upgrade.Zap();
			Add_A_Button(Upgrade);
			Power.Zap();
			Add_A_Button(Power);
			Waypoint.Zap();
			Add_A_Button(Waypoint);
			Column[0].Activate();
			Column[1].Activate();
			Background.Zap();
			Add_A_Button(Background);
			RadarButton.Zap();
			Add_A_Button(RadarButton);
		} else {
			Stop_Ingame_Movie();
			Remove_A_Button(Repair);
			Remove_A_Button(Upgrade);
			Remove_A_Button(Power);
			Remove_A_Button(Waypoint);
			Remove_A_Button(Background);
			Column[0].Deactivate();
			Column[1].Deactivate();
			Remove_A_Button(RadarButton);
		}

		/*
		**	Since the sidebar status has changed, update the map so that the graphics
		**	will be rendered correctly.
		*/
		Flag_To_Redraw(GS_REDRAW_ALL);
	}

	return(old);
}


/***********************************************************************************************
 * SidebarClass::StripClass::StripClass -- Default constructor for the side strip class.       *
 *                                                                                             *
 *    This constructor is used to reset the side strip to default empty state.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
SidebarClass::StripClass::StripClass(InitClass const &) :
	X(0),
	Y(0),
	ObjectRect(RECT_NONE),
	ID(0),
	IsToRedraw(true),
	IsBuilding(false),
	IsScrollingDown(false),
	IsScrolling(false),
	Flasher(-1),
	TopIndex(0),
	Scroller(0),
	Slid(0),
	LastSlid(0),
	BuildableCount(0)
{
	for (int index = 0; index < MAX_BUILDABLES; index++) {
		Buildables[index].BuildableID = 0;
		Buildables[index].BuildableType = RTTI_NONE;
		Buildables[index].Factory = NULL;
	}
}


/***********************************************************************************************
 * SidebarClass::StripClass::One_Time -- Performs one time actions necessary for the side stri *
 *                                                                                             *
 *    Call this routine ONCE at the beginning of the game. It handles retrieving pointers to   *
 *    the shape files it needs for rendering.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::One_Time(int )
{
	DarkenShapes = (ShapeSet*)MFCD::Retrieve("DARKEN.SHP");
}


/***********************************************************************************************
 * SidebarClass::StripClass::Get_Special_Cameo -- Fetches the special event cameo shape.       *
 *                                                                                             *
 *    This routine will return with a pointer to the cameo data for the special objects that   *
 *    can appear on the sidebar (e.g., nuclear bomb).                                          *
 *                                                                                             *
 * INPUT:   type  -- The special type to fetch the cameo imagery for.                          *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the cameo imagery for the specified special object.      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/19/1995 JLB : commented                                                                *
 *=============================================================================================*/
ShapeSet const * SidebarClass::StripClass::Get_Special_Cameo(SuperWeaponType type)
{
	if ((unsigned)type < (unsigned)SuperWeaponTypes.Count()) {
		return(SuperWeaponTypes[type]->CameoData);
	}
	return(NULL);
}


/***********************************************************************************************
 * SidebarClass::StripClass::Init_Clear -- Sets sidebar to a known (and deactivated) state     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Init_Clear(void)
{
	IsScrollingDown = false;
	IsScrolling = false;
	IsBuilding = false;
	Flasher = -1;
	TopIndex = 0;
	Slid = 0;
	BuildableCount = 0;

	/*
	**	Since we're resetting the strips, clear out all the buildables & factory pointers.
	*/
	for (int index = 0; index < MAX_BUILDABLES; index++) {
		Buildables[index].BuildableID = 0;
		Buildables[index].BuildableType = RTTI_NONE;
		Buildables[index].Factory = NULL;
	}
}


/***********************************************************************************************
 * SidebarClass::StripClass::Init_IO -- Initializes the strip's buttons                        *
 *                                                                                             *
 * This routine doesn't actually add any buttons to the list.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Init_IO(int id)
{
	ID = id;

	UpButton[ID].IsSticky = true;
	UpButton[ID].ID = BUTTON_UP+id;
	UpButton[ID].DrawOnSidebar = true;
	UpButton[ID].ShapeDrawer = SidebarDrawer;
	UpButton[ID].Set_Flags(GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS);

	DownButton[ID].IsSticky = true;
	DownButton[ID].ID = BUTTON_DOWN+id;
	DownButton[ID].DrawOnSidebar = true;
	DownButton[ID].ShapeDrawer = SidebarDrawer;
	DownButton[ID].Set_Flags(GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS);

	for (int index = 0; index < Map.Max_Visible(); index++) {
		SelectClass & g = SelectButton[ID][index];
		g.ID = BUTTON_SELECT;
		g.X = SidebarRect.X + Map.Column[ID].X;
		g.Y = SidebarRect.Y + Map.Column[ID].Y + (OBJECT_HEIGHT*index);
		g.Width = OBJECT_WIDTH;
		g.Height = OBJECT_HEIGHT;
		g.Set_Owner(*this, index);
	}

}


/***********************************************************************************************
 * SidebarClass::StripClass::Activate -- Adds the strip buttons to the input system.           *
 *                                                                                             *
 *    This routine will add the side strip buttons to the map's input system. This routine     *
 *    should be called once when the sidebar activates.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Never call this routine a second time without first calling Deactivate().       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Activate(void)
{
	UpButton[ID].Zap();
	Map.Add_A_Button(UpButton[ID]);

	DownButton[ID].Zap();
	Map.Add_A_Button(DownButton[ID]);

	for (int index = 0; index < Map.Max_Visible(); index++) {
		SelectButton[ID][index].Zap();
		Map.Add_A_Button(SelectButton[ID][index]);
	}
}


/***********************************************************************************************
 * SidebarClass::StripClass::Deactivate -- Removes the side strip buttons from the input syste *
 *                                                                                             *
 *    Call this routine to remove all the buttons on the side strip from the map's input       *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Never call this routine unless the Activate() function was previously called.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Deactivate(void)
{
	Map.Remove_A_Button(UpButton[ID]);
	Map.Remove_A_Button(DownButton[ID]);
	for (int index = 0; index < Map.Max_Visible(); index++) {
		Map.Remove_A_Button(SelectButton[ID][index]);
	}
}


/***********************************************************************************************
 * SidebarClass::StripClass::Add -- Add an object to the side strip.                           *
 *                                                                                             *
 *    Use this routine to add a buildable object to the side strip.                            *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object type that can be built and is to be added to     *
 *                      the side strip.                                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the object successfully added to the side strip? Failure could be the    *
 *                result of running out of room in the side strip array or the object might    *
 *                already be in the list.                                                      *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::StripClass::Add(RTTIType type, int id)
{
	if (BuildableCount < MAX_BUILDABLES) {
		for (int index = 0; index < BuildableCount; index++) {
			if (Buildables[index].BuildableType == type && Buildables[index].BuildableID == id) {
				return(false);
			}
		}
		if (!ScenarioInit && type != RTTI_SPECIAL) {
			Speak(VOX_NEW_CONSTRUCT);
		}
		Buildables[BuildableCount].BuildableType = type;
		Buildables[BuildableCount].BuildableID = id;
		BuildableCount++;
		IsToRedraw = true;
		return(true);
	}
	return(false);
}


/***********************************************************************************************
 * SidebarClass::StripClass::Scroll -- Causes the side strip to scroll.                        *
 *                                                                                             *
 *    Use this routine to flag the side strip to scroll. The direction scrolled is controlled  *
 *    by the parameter. Scrolling is merely initiated by this routine. Subsequent calls to     *
 *    the AI function and the Draw_It function are required to properly give the appearance    *
 *    of scrolling.                                                                            *
 *                                                                                             *
 * INPUT:   bool; Should the side strip scroll UP? If it is to scroll down then pass false.    *
 *                                                                                             *
 * OUTPUT:  bool; Was the side strip started to scroll in the desired direction?               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   07/29/1995 JLB : Simplified scrolling logic.                                              *
 *=============================================================================================*/
bool SidebarClass::StripClass::Scroll(bool up)
{
	if (up) {
		if (!TopIndex) return(false);
		Scroller--;
	} else {
		if (TopIndex+Map.Max_Visible() >= BuildableCount) return(false);
		Scroller++;
	}
	return(true);
}


/// <summary>
/// Causes the side strip to scroll by a whole page.
/// Use this routine to flag the side strip to move a full screenful of cameos rather than
/// the single one that Scroll would move. As with scrolling, this routine only starts the
/// motion; the AI and Draw_It routines carry it out.
/// </summary>
/// <param name="up">Should the side strip page UP? If it is to page down then pass false.</param>
/// <returns>bool; Was the side strip started to page in the desired direction?</returns>
bool SidebarClass::StripClass::Page(bool up)
{
	if (up) {
		if (!TopIndex) return(false);
		Scroller-=Map.Max_Visible();
	} else {
		if (TopIndex+Map.Max_Visible() >= BuildableCount) return(false);
		Scroller+=Map.Max_Visible();
	}
	return(true);
}


/***********************************************************************************************
 * SidebarClass::StripClass::Flag_To_Redraw -- Flags the sidebar strip to be redrawn.          *
 *                                                                                             *
 *    This utility routine is called when something changes on the sidebar and it must be      *
 *    reflected the next time drawing is performed.                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::Flag_To_Redraw(void)
{
	IsToRedraw = true;
	Map.Flag_To_Redraw();
}


/***********************************************************************************************
 * SidebarClass::StripClass::AI -- Input and AI processing for the side strip.                 *
 *                                                                                             *
 *    The side strip AI processing is performed by this function. This function not only       *
 *    checks for player input, but also handles any graphic logic updating necessary as a      *
 *    result of flashing or construction animation.                                            *
 *                                                                                             *
 * INPUT:   input -- The player input code.                                                    *
 *                                                                                             *
 *          x,y   -- Mouse coordinate to use.                                                  *
 *                                                                                             *
 * OUTPUT:  bool; Did the AI detect that it will need a rendering change? If this routine      *
 *                returns true, then the Draw_It function should be called at the              *
 *                earliest opportunity.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *=============================================================================================*/
bool SidebarClass::StripClass::AI(KeyNumType & input, Point2D const & xy)
{
	KeyNumType key = KeyNumType(input & ~KN_UNK);
	bool redraw = false;

	/*
	**	If this is scroll button for this side strip, then scroll the strip as
	**	indicated.
	*/
	if (key == KeyNumType(UpButton[ID].ID|KN_BUTTON)) {
		UpButton[ID].IsPressed = false;
		if ((input & KN_UNK) != 0) {
			if (!Page(true)) {
				Sound_Effect(Rule->ScoldSound);
			}
		} else {
			if (!Scroll(true)) {
				Sound_Effect(Rule->ScoldSound);
			}
		}
	}
	else if (key == KeyNumType(DownButton[ID].ID|KN_BUTTON)) {
		DownButton[ID].IsPressed = false;
		if ((input & KN_UNK) != 0) {
			if (!Page(false)) {
				Sound_Effect(Rule->ScoldSound);
			}
		} else {
			if (!Scroll(false)) {
				Sound_Effect(Rule->ScoldSound);
			}
		}
	}

	/*
	**	Reflect the scroll desired direction/value into the scroll
	**	logic handler. This might result in up or down scrolling.
	*/
	if (!IsScrolling && Scroller) {
		if (BuildableCount <= Map.Max_Visible()) {
			Scroller = 0;
		} else {

			/*
			**	Top of list is moving toward lower ordered entries in the object list. It looks like
			**	the "window" to the object list is moving up even though the actual object images are
			**	scrolling downward.
			*/
			if (Scroller < 0) {
				if (!TopIndex) {
					Scroller = 0;
				} else {
					Scroller++;
					IsScrollingDown = false;
					IsScrolling = true;
					TopIndex--;
					Slid = 0;
				}

			} else {
				if (TopIndex+Map.Max_Visible() >= BuildableCount) {
					Scroller = 0;
				} else {
					Scroller--;
					Slid = OBJECT_HEIGHT;
					IsScrollingDown = true;
					IsScrolling = true;
				}
			}
		}
	}

	/*
	**	Scroll logic is handled here.
	*/
	if (IsScrolling) {
		if (IsScrollingDown) {
			Slid -= SCROLL_RATE;
			if (Slid <= 0) {
				IsScrolling = false;
				Slid = 0;
				TopIndex++;
			}
		} else {
			Slid += SCROLL_RATE;
			if (Slid >= OBJECT_HEIGHT) {
				IsScrolling = false;
				Slid = 0;
			}
		}
		redraw = true;
	}

	/*
	**	Handle any flashing logic. Flashing occurs when the player selects an object
	**	and provides the visual feedback of a recognized and legal selection.
	*/
	if (Flasher != -1) {
		if (Graphic_Logic()) {
			redraw = true;
			if (Fetch_Stage() >= 7) {
				Set_Rate(0);
				Set_Stage(0);
				Flasher = -1;
			}
		}
	}

	/*
	**	Handle any building clock animation logic.
	*/
	if (IsBuilding) {
		for (int index = 0; index < BuildableCount; index++) {
			FactoryClass * factory = Buildables[index].Factory;

			if (factory && factory->Has_Changed()) {
				redraw = true;
				if (factory->Has_Completed()) {

					/*
					**	Construction has been completed. Announce this fact to the player and
					**	try to get the object to automatically leave the factory. Buildings are
					**	the main exception to the ability to leave the factory under their own
					**	power.
					*/
					TechnoClass * pending = factory->Get_Object();
					if (pending != NULL) {
						switch ((RTTIType)pending->RTTI) {
							case RTTI_UNIT:
							case RTTI_AIRCRAFT:
								OutList.push_back(EventClass(pending->Owner(), EventClass::PLACE, pending->RTTI, CELL_NONE));
								Speak(VOX_UNIT_READY);
								break;

							case RTTI_BUILDING:
								Speak(VOX_CONSTRUCTION);
								break;

							case RTTI_INFANTRY:
								OutList.push_back(EventClass(pending->Owner(), EventClass::PLACE, pending->RTTI, CELL_NONE));
								Speak(VOX_UNIT_READY);
								break;
						}
					}
				}
			}
		}
	}

	/*
	**	If any of the logic determined that this side strip needs to be redrawn, then
	**	set the redraw flag for this side strip.
	*/
	if (redraw) {
		Flag_To_Redraw();
		IsToBlitSidebar = true;
	}

	return(redraw);
}


/// <summary>
/// Fetches the tooltip text for one of the strip's cameos.
/// This routine composes the name and cost line that appears when the mouse lingers over a
/// build cameo. Only the cost is given when the player has cameo text turned on, since the
/// name is already printed beneath the cameo itself.
/// </summary>
/// <param name="id">The cameo slot, counted from the top of the strip as it is displayed.</param>
/// <returns>Returns with a pointer to the help text, or NULL if that slot is empty.</returns>
char const * SidebarClass::StripClass::Help_Text(int id)
{
	static char _buffer[84];

	int i = id + TopIndex;

	if (GameActive) {
		if (i < BuildableCount) {
			if (Buildables[i].BuildableType == RTTI_SPECIAL) {
				return(SuperWeaponTypes[Buildables[i].BuildableID]->Full_Name());
			}

			TechnoTypeClass const * choice = Fetch_Techno_Type(Buildables[i].BuildableType, Buildables[i].BuildableID);
			if (choice == NULL) {
				return(NULL);
			}

			if (Map.IsCameoText) {
				sprintf(_buffer, Fetch_String(TXT_MONEY_FORMAT_1), choice->Cost_Of(PlayerPtr));
			} else {
				sprintf(_buffer, Fetch_String(TXT_MONEY_FORMAT_2), choice->Full_Name(), choice->Cost_Of(PlayerPtr));
			}

			return(_buffer);
		}
	}

	return(NULL);
}


/***********************************************************************************************
 * SidebarClass::StripClass::Draw_It -- Render the sidebar display.                            *
 *                                                                                             *
 *    Use this routine to render the sidebar display. It checks to see if it needs to be       *
 *    redrawn and only redraw if necessary. If the "complete" parameter is true, then it       *
 *    will force redraw the entire strip.                                                      *
 *                                                                                             *
 * INPUT:   complete -- Should the redraw be forced? A force redraw will ignore the redraw     *
 *                      flag.                                                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   08/06/1995 JLB : Handles multi factory tracking in same strip.                            *
 *=============================================================================================*/
void SidebarClass::StripClass::Draw_It(bool complete)
{
	if (IsToRedraw || complete) {
		IsToRedraw = false;
		IsToBlitSidebar = true;
		Rect cliprect = SidebarRect;
		cliprect.X = 0;

		/*
		**	Redraw the scroll buttons.
		*/
		UpButton[ID].Draw_Me(true);
		DownButton[ID].Draw_Me(true);

		/*
		**	Loop through all the buildable objects that are visible in the strip and render
		**	them. Their Y offset may be adjusted if the strip is in the process of scrolling.
		*/
		for (int i = 0; i < Map.Max_Visible() + (IsScrolling ? 1 : 0); i++) {
			ShapeSet const * shapefile = NULL;

			int index = i+TopIndex;
			int x = X;
			int y = COLUMN_ONE_Y + i * OBJECT_HEIGHT;

			bool production = false;
			bool completed = false;
			int stage = 0;
			bool darken = false;
			FactoryClass * factory = NULL;
			char const * state = NULL;
			bool isready = false;

			/*
			**	If the strip is scrolling, then the offset is adjusted accordingly.
			*/
			if (IsScrolling) {
				y -= OBJECT_HEIGHT - Slid;
			}

			char const * name = NULL;
			TechnoTypeClass const * obj = NULL;

			/*
			**	Fetch the shape number for the object type located at this current working
			**	slot. This shape pointer is used to draw the underlying graphic there.
			*/
			if (index < BuildableCount) {
				SuperWeaponType spc = SUPER_NONE;

				if (Buildables[index].BuildableType != RTTI_SPECIAL) {

					obj = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
					if (obj != NULL) {

						name = obj->Full_Name();

						/*
						**	If there is already a factory producing this kind of object, then all
						**	objects of this type are displays in a disabled state.
						*/
						bool isbusy = false;
						if (obj->RTTI == RTTI_BUILDINGTYPE) {
							isbusy = (PlayerPtr->Fetch_Factory(Buildables[index].BuildableType) != NULL);
						}

						if (obj->Who_Can_Build_Me(true, true, true, PlayerPtr) == NULL) {
							isbusy = true;
						}

						if (!isbusy && PlayerPtr->Can_Build(Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID), false, false) == -1) {
							isbusy = true;
						}

						shapefile = (ShapeSet const *)obj->Get_Cameo_Data();
						factory = Buildables[index].Factory;
						if (factory != NULL) {
							production	= true;
							completed	= factory->Has_Completed();
							if (completed) {
								state = Fetch_String(TXT_READY);
							}
							stage		= factory->Completion();
							darken		= false;
						} else {
							production  = false;
						//							darken      = IsBuilding;

							/*
							**	Darken the imagery if a factory of a matching type is
							**	already busy.
							*/
							darken = isbusy;
						}
					} else {
						darken = false;
					}

				} else {

					spc = SuperWeaponType(Buildables[index].BuildableID);
					name = SuperWeaponTypes[spc]->Full_Name();
					shapefile = Get_Special_Cameo(spc);

					production = true;
					completed = PlayerPtr->SuperWeapon[spc]->Is_Charging() == false;
					isready = PlayerPtr->SuperWeapon[spc]->Can_Place();
					state = PlayerPtr->SuperWeapon[spc]->State_String();
					stage = PlayerPtr->SuperWeapon[spc]->Anim_Stage();
					darken = false;
				}

				if (obj != NULL || spc != SUPER_NONE) {
					/*
					**	If this item is flashing then take care of it.
					**
					*/
					// if (Flasher == index && (Fetch_Stage() & 0x01)) {
					// 	remapper = Map.FadingLight;
					// }

				} else {
					shapefile	= LogoShapes;
					// if (!darken) {
					// 	shapenum		= SB_BLANK;
					// }
				}
			} else {
				shapefile	= LogoShapes;
				production	= false;
			}

			/*
			**	Now that the shape of the object at the current working slot has been found,
			**	draw it and any graphic overlays as necessary.
			*/
			if (shapefile != LogoShapes) {

				if (shapefile != NULL) {
					Draw_Shape(*SidebarSurface, *CameoDrawer, shapefile, 0, Point2D(x, y), cliprect, ShapeFlags_Type(SHAPE_WIN_REL));
				}

				/*
				**	Darken this object because it cannot be produced or is otherwise
				**	unavailable.
				*/
				if (darken) {
					Draw_Shape(*SidebarSurface, *SidebarDrawer, DarkenShapes, 0, Point2D(x, y), cliprect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_DARKEN));
				}
			}

			if (name != NULL) {
				Print_Cameo_Text(name, Point2D(x, y + CAMEO_TEXT_Y_OFFSET), cliprect, OBJECT_WIDTH-2);
			}

			bool hasqueuecount = false;
			if (obj != NULL) {
				FactoryClass * factory = PlayerPtr->Fetch_Factory(obj->RTTI);

				if (factory != NULL) {
					int total = factory->Total(obj);
					if (total > 1 || total > 0 && !factory->Is_Currently_Producing(obj)) {
						Fancy_Text_Print("%d", *SidebarSurface, cliprect, Point2D(x + QUEUE_COUNT_X_OFFSET, y + TEXT_Y_OFFSET), Fetch_Scheme_By_Name("LightGrey"), TBLACK, TextPrintType(TPF_RIGHT|TPF_FULLSHADOW|TPF_8POINT), total);
						hasqueuecount = true;
					}
				}
			}

			/*
			**	Draw the overlapping clock shape if this is object is being constructed.
			**	If the object is completed, then display "Ready" with no clock shape.
			*/
			if (production) {

				/*
				**	Display text showing that the object is ready to place.
				*/
				if (state != NULL) {
					Fancy_Text_Print(state, *SidebarSurface, cliprect, Point2D(x + TEXT_X_OFFSET, y + TEXT_Y_OFFSET), Fetch_Scheme_By_Name("LightBlue"), TBLACK, TextPrintType(TPF_CENTER|TPF_FULLSHADOW|TPF_8POINT));
				}

				if (!completed) {

					if (!isready) {
						Draw_Shape(*SidebarSurface, *SidebarDrawer, ClockShapes, stage + 1, Point2D(x, y), cliprect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_TRANSLUCENT50));
					} else {
						Draw_Shape(*SidebarSurface, *SidebarDrawer, RechargeClockShapes, stage + 1, Point2D(x, y), cliprect, ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_TRANSLUCENT50));
					}

					/*
					**	Display text showing that the construction is temporarily on hold.
					*/
					if (factory && !factory->Is_Building()) {
						if (!hasqueuecount) {
							Fancy_Text_Print(TXT_HOLD, *SidebarSurface, cliprect, Point2D(x + TEXT_X_OFFSET, y + TEXT_Y_OFFSET), Fetch_Scheme_By_Name("LightGrey"), TBLACK, TextPrintType(TPF_CENTER|TPF_FULLSHADOW|TPF_8POINT));
						} else {
							Fancy_Text_Print(TXT_HOLD, *SidebarSurface, cliprect, Point2D(x, y + TEXT_Y_OFFSET), Fetch_Scheme_By_Name("LightGrey"), TBLACK, TextPrintType(TPF_FULLSHADOW|TPF_8POINT));
						}
					}
				}
			}
		}

		LastSlid = Slid;

	} else {

		if (UpButton[ID].IsDrawn) {
			IsToBlitSidebar = true;
			UpButton[ID].IsDrawn = false;
		}

		if (DownButton[ID].IsDrawn) {
			IsToBlitSidebar = true;
			DownButton[ID].IsDrawn = false;
		}
	}
}


/***********************************************************************************************
 * SidebarClass::StripClass::Recalc -- Revalidates the current sidebar list of objects.        *
 *                                                                                             *
 *    This routine will revalidate all the buildable objects in the sidebar. This routine      *
 *    comes in handy when a factory has been destroyed, and the sidebar needs to reflect any   *
 *    change that this requires. It checks every object to see if there is a factory available *
 *    that could produce it. If none can be found, then the object is removed from the         *
 *    sidebar.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; The sidebar has changed as a result of this call?                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   06/26/1995 JLB : Doesn't collapse sidebar when buildables removed.                        *
 *=============================================================================================*/
bool SidebarClass::StripClass::Recalc(void)
{
	int ok;

	if (Debug_Map || !BuildableCount) {
		return(false);
	}

	bool scroll = false;
	int max_visible = Map.Max_Visible();
	BuildType * unshifted = new BuildType[max_visible];

	for (int i = 0; i < max_visible; i++) {
		if (i + TopIndex < BuildableCount) {
			unshifted[i] = Buildables[i + TopIndex];
		}
	}

	/*
	**	Sweep through all objects listed in the sidebar. If any of those object can
	**	not be created -- even in theory -- then they must be removed form the sidebar and
	**	any current production must be abandoned.
	*/
	bool redraw = false;
	for (int index = 0; index < BuildableCount; index++) {
		TechnoTypeClass const * tech = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
		if (tech != NULL) {
			BuildingClass const * who = tech->Who_Can_Build_Me(true, false, false, PlayerPtr);
			ok = who != NULL && who->House->Can_Build(tech, true, true);
		} else {
			if ((unsigned)Buildables[index].BuildableID < (unsigned)PlayerPtr->SuperWeapon.Count()) {
				ok = PlayerPtr->SuperWeapon[Buildables[index].BuildableID]->Is_Present();
			} else {
				ok = false;
			}
		}

		if (!ok) {
			for (int i = 0; i < max_visible; i++) {
				if (unshifted[i] == Buildables[index]) {
					unshifted[i] = BuildType(0, RTTI_NONE);
				}
			}

			/*
			**	Removes this entry from the list.
			*/
			if (BuildableCount > 1 && index < BuildableCount-1) {
				memmove(&Buildables[index], &Buildables[index+1], sizeof(Buildables[0])*((BuildableCount-index)-1));
			}
			redraw = true;
			scroll = true;
			Buildables[BuildableCount-1].Factory = NULL;
			IsToRedraw = true;
			BuildableCount--;
			index--;
		}
	}

	if (scroll) {
		bool got_old = false;
		bool got_new = false;

		int oldpos;
		for (oldpos = 0; oldpos < max_visible; oldpos++) {
			if (unshifted[oldpos] != BuildType(0, RTTI_NONE)) {
				got_old = true;
				break;
			}
		}

		int newpos;
		if (got_old && BuildableCount != 0) {
			for (newpos = 0; newpos < BuildableCount; newpos++) {
				if (Buildables[newpos] == unshifted[oldpos]) {
					got_new = true;
					break;
				}
			}
		}

		if (got_old && got_new) {
			TopIndex = newpos - oldpos;
			TopIndex = std::max(0, std::min(TopIndex, BuildableCount - max_visible));
		} else {
			TopIndex = 0;
		}
	}

	delete[] unshifted;
	return(redraw);
}


/***********************************************************************************************
 * SidebarClass::StripClass::SelectClass::SelectClass -- Default constructor.                  *
 *                                                                                             *
 *    This is the default constructor for the button that controls the buildable cameos on     *
 *    the sidebar strip.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The coordinates are set to zero by this routine. They must be set to the        *
 *             correct values before this button will function.                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
SidebarClass::StripClass::SelectClass::SelectClass(void) :
	ControlClass(0, 0, 0, OBJECT_WIDTH-1, OBJECT_HEIGHT, LEFTPRESS|RIGHTPRESS|LEFTUP),
	Strip(0),
	Index(0)
{
}


/***********************************************************************************************
 * SidebarClass::StripClass::SelectClass:: -- Assigns special values to a buildable select but *
 *                                                                                             *
 *    Use this routine to set custom buildable vars for this particular select button. It      *
 *    uses this information to properly know what buildable object to start or stop production *
 *    on.                                                                                      *
 *                                                                                             *
 * INPUT:   strip    -- Reference to the strip that owns this buildable button.                *
 *                                                                                             *
 *          index    -- The index (0 .. MAX_VISIBLE-1) of this button. This is used to let     *
 *                      the owning strip know what index this button refers to.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::StripClass::SelectClass::Set_Owner(StripClass & strip, int index)
{
	Strip = &strip;
	Index = index;
}


/***********************************************************************************************
 * SidebarClass::StripClass::SelectClass:: -- Action function when buildable cameo is selected *
 *                                                                                             *
 *    This function is called when the buildable icon (cameo) is clicked on. It handles        *
 *    starting and stopping production as indicated.                                           *
 *                                                                                             *
 * INPUT:   flags -- The input event that triggered the call.                                  *
 *                                                                                             *
 *          key   -- The keyboard value at the time of the input.                              *
 *                                                                                             *
 * OUTPUT:  Returns with whether the input list should be scanned further.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   10/09/1996 JLB : Sonar pulse converted to regular event type.                             *
 *=============================================================================================*/
int SidebarClass::StripClass::SelectClass::Action(unsigned flags, KeyNumType & key)
{
	if (Strip == NULL) {
		return(1);
	}

	int index = Strip->TopIndex + Index;
	RTTIType otype = Strip->Buildables[index].BuildableType;
	int oid = Strip->Buildables[index].BuildableID;
	FactoryClass * factory = Strip->Buildables[index].Factory;

	TechnoTypeClass const * choice = NULL;
	SuperWeaponType spc = SUPER_NONE;

	Map.Override_Mouse_Shape(MOUSE_NORMAL);

	if (index < Strip->BuildableCount) {
		if (otype != RTTI_SPECIAL) {
			choice = Fetch_Techno_Type(otype, oid);
		} else {
			spc = SuperWeaponType(oid);
		}
	}

	if (spc != SUPER_NONE) {

		/*
		**	Display the help text if the mouse is over the button.
		*/
		if (flags & LEFTUP) {
			flags &= ~LEFTUP;
		}

		/*
		**	A right mouse button signals "cancel".  If we are in targeting
		**	mode then we don't want to be any more.
		*/
		if (flags & RIGHTPRESS) {
			Map.IsTargettingMode = SUPER_NONE;
			PlayerPtr->TargetingSW = NULL;
		}
		/*
		**	A left mouse press signal "activate".  If our weapon type is
		**	available then we should activate it.
		*/
		if (flags & LEFTPRESS) {

			if ((unsigned)spc < (unsigned)PlayerPtr->SuperWeapon.Count()) {
				SuperClass* curr_sw = PlayerPtr->SuperWeapon[spc];
				if (curr_sw->Can_Place()) {
					if (curr_sw->Class->Action == ACTION_NONE) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SPECIAL_PLACE, PlayerPtr->SuperWeapon[spc]->Class->HeapID, Cell(0, 0)));
					} else {
						PlayerPtr->TargetingSW = curr_sw;
						Map.IsTargettingMode = spc;
						Unselect_All();
						Speak(VOX_SELECT_TARGET);
					}
				} else {
					PlayerPtr->SuperWeapon[spc]->Impatient_Click();
					PlayerPtr->TargetingSW = NULL;
				}
			}
		}

	} else {

		if (choice != NULL) {

			/*
			**	Display the help text if the mouse is over the button.
			*/
			if (flags & LEFTUP) {
				flags &= ~LEFTUP;
			}

			/*
			**	A right mouse button signals "cancel".
			*/
			if (flags & RIGHTPRESS) {

				/*
				**	If production is in progress, put it on hold. If production is already
				**	on hold, then abandon it. Money will be refunded, the factory
				**	manager deleted, and the object under construction is returned to
				**	the free pool.
				*/
				if (factory != NULL) {

					/*
					**	Cancels placement mode if the sidebar factory is abandoned or
					**	suspended.
					*/
					if (Map.PendingObjectPtr && Dynamic_Cast<TechnoClass *>(Map.PendingObjectPtr)) {
						Map.PendingObjectPtr = NULL;
						Map.PendingObject = NULL;
						Map.PendingHouse = HOUSE_NONE;
						Map.Set_Cursor_Shape(NULL);
					}

					if (!factory->Is_Building()) {
						Speak(VOX_CANCELED);
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::ABANDON, otype, oid));
					} else {
						Speak(VOX_SUSPENDED);
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::SUSPEND, otype, oid));
						Map.Column[0].IsToRedraw = true;
						Map.Column[1].IsToRedraw = true;
					}
				} else {

					/*
					 * Cancel a queued production.
					 */
					FactoryClass * fptr = PlayerPtr->Fetch_Factory(otype);
					if (fptr != NULL && fptr->Is_Queued(choice)) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::ABANDON, otype, oid));
					}
				}
			}

			if (flags & LEFTPRESS) {

				if (factory != NULL && !factory->Is_Building()) {

					/*
					**	If production has completed, then attempt to have the object exit
					**	the factory or go into placement mode.
					*/
					if (factory->Has_Completed()) {

						TechnoClass * pending = factory->Get_Object();
						if (!pending) {
							if (factory->Get_Special_Item() != -1) {
								Map.IsTargettingMode = SUPER_ANY;
							}
						} else {
							BuildingClass * builder = pending->Who_Can_Build_Me(false, false);
							if (!builder) {
								OutList.push_back(EventClass(pending->Owner(), EventClass::ABANDON, otype, oid));
								Speak(VOX_NO_FACTORY);
							} else {

								/*
								**	If the completed object is a building, then change the
								**	game state into building placement mode. This fact is
								**	not transmitted to any linked computers until the moment
								**	the building is actually placed down.
								*/
								if (pending->RTTI == RTTI_BUILDING) {
							 		PlayerPtr->Manual_Place(builder, (BuildingClass *)pending);
								} else {

									/*
									**	For objects that can leave the factory under their own
									**	power, queue this event and process through normal house
									**	production channels.
									*/
									OutList.push_back(EventClass(pending->Owner(), EventClass::PLACE, otype, CELL_NONE));
								}
							}
						}
					} else {

						/*
						**	The factory must have been in a suspended state. Resume construction
						**	normally.
						*/
						if (otype == RTTI_INFANTRYTYPE) {
							Speak(VOX_TRAINING);
						} else {
							Speak(VOX_BUILDING);
						}
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::PRODUCE, Strip->Buildables[index].BuildableType, Strip->Buildables[index].BuildableID));
					}

				} else {

					/*
					**	If there is already a factory attached to this strip but the player didn't click
					**	on the icon that has the attached factory, then say that the factory is busy and
					**	ignore the click.
					*/
					factory = PlayerPtr->Fetch_Factory(otype);
					bool produce = false;
					if (factory != NULL && (factory->Is_Building() || factory->Has_Production_Target())) {
						if (otype == RTTI_BUILDINGTYPE) {
							Speak(VOX_NO_FACTORY);
						} else {
							produce = true;
						}

					} else {

						/*
						**	If this side strip is already busy with production, then ignore the
						**	input and announce this fact.
						*/
						if (otype == RTTI_INFANTRYTYPE) {
							Speak(VOX_TRAINING);
						} else {
							Speak(VOX_BUILDING);
						}
						produce = true;
					}
					if (produce) {
						OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::PRODUCE, Strip->Buildables[index].BuildableType, Strip->Buildables[index].BuildableID));
					}
				}
			}
		} else {
			flags = 0;
		}
	}

	ControlClass::Action(flags, key);

	return(true);
}


/***********************************************************************************************
 * SidebarClass::SBGadgetClass::Action -- Special function that controls the mouse over the si *
 *                                                                                             *
 *    This routine is called whenever the mouse is over the sidebar. It makes sure that the    *
 *    mouse is always the normal shape while over the sidebar.                                 *
 *                                                                                             *
 * INPUT:   flags -- The event flags that resulted in this routine being called.               *
 *                                                                                             *
 *          key   -- Reference the keyboard code that may be present.                          *
 *                                                                                             *
 * OUTPUT:  Returns that no further keyboard processing is necessary.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int SidebarClass::SBGadgetClass::Action(unsigned , KeyNumType & )
{
	Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
	return(true);
}


/***********************************************************************************************
 * SidebarClass::StripClass::Factory_Link -- Links a factory to a sidebar button.              *
 *                                                                                             *
 *    This routine will link the specified factory to this sidebar strip. The exact button to  *
 *    link to is determined from the object type and id specified. A linked button is one that *
 *    will show appropriate construction animation (clock shape) that matches the state of     *
 *    the factory.                                                                             *
 *                                                                                             *
 * INPUT:   factory  -- The factory number to link to the sidebar.                             *
 *                                                                                             *
 *          type     -- The object type that this factory refers to.                           *
 *                                                                                             *
 *          id       -- The object sub-type that this factory refers to.                       *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully attached? Failure would indicate that there is no     *
 *          object of the specified type and sub-type in the sidebar list.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::StripClass::Factory_Link(FactoryClass * factory, RTTIType type, int id)
{
	for (int index = 0; index < BuildableCount; index++) {
		if (Buildables[index].BuildableType == type && Buildables[index].BuildableID == id) {
			Buildables[index].Factory = factory;
			IsBuilding = true;
			/*
			**	Flag that all the icons on this strip need to be redrawn
			*/
			Flag_To_Redraw();
			return(true);
		}
	}
	return(false);
}


/***********************************************************************************************
 * SidebarClass::Abandon_Production -- Stops production of the object specified.               *
 *                                                                                             *
 *    This routine is used to abandon production of the object specified. The factory will     *
 *    be completely disabled by this call.                                                     *
 *                                                                                             *
 * INPUT:   type     -- The object type that is to be abandoned. The sub-type is not needed    *
 *                      since it is presumed there can be only one type in production at any   *
 *                      one time.                                                              *
 *                                                                                             *
 *          factory  -- The factory number that is doing the production.                       *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully abandoned?                                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SidebarClass::Abandon_Production(RTTIType type, FactoryClass * factory)
{
	return(Column[Which_Column(type)].Abandon_Production(factory));
}


/***********************************************************************************************
 * SidebarClass::StripClass::Abandon_Produ -- Abandons production associated with sidebar.     *
 *                                                                                             *
 *    Production of the object associated with this sidebar is abandoned when this routine is  *
 *    called.                                                                                  *
 *                                                                                             *
 * INPUT:   factory  -- The factory index that is to be suspended.                             *
 *                                                                                             *
 * OUTPUT:  Was the production abandonment successful?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *   08/06/1995 JLB : More intelligent abandon logic for multiple factories.                   *
 *=============================================================================================*/
bool SidebarClass::StripClass::Abandon_Production(FactoryClass const * factory)
{
	bool noprod = true;
	bool abandon = false;
	for (int index = 0; index < BuildableCount; index++) {
		if (Buildables[index].Factory == factory) {
			((FactoryClass *)factory)->Abandon();
			Buildables[index].Factory = NULL;
			abandon = true;
		} else {
			if (Buildables[index].Factory != NULL) {
				noprod = false;
			}
		}
	}

	/*
	**	If there was a change to the strip, then flag the strip to be redrawn.
	*/
	if (abandon) {
		Flag_To_Redraw();
	}

	/*
	**	If there is no production whatsoever on this strip, then flag it so.
	*/
	if (noprod) {
		IsBuilding = false;
	}
	return(abandon);
}


/***********************************************************************************************
 * SidebarClass::Zoom_Mode_Control -- Handles the zoom mode toggle operation.                  *
 *                                                                                             *
 *    This is the function that is called when the map button is pressed. It will toggle       *
 *    between the different modes that the radar map can assume.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/31/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void SidebarClass::Zoom_Mode_Control(void)
{
	/*
	**	If radar is active, cycle as follows:
	**	Zoomed => not zoomed
	**	not zoomed => player status (multiplayer only)
	**	player status => radar spying readout
	**	radar spying readout => zoomed
	*/
	if (Session.Type!=GAME_NORMAL) {
		if (Is_Radar_Existing()) {
			if (Is_Player_Names()) {
				Radar_Activate(1);
				return;
			}
		} else if (Is_Player_Names()) {
			Radar_Activate(0);
			return;
		}

		Radar_Activate(2);
	}
}


/// <summary>
/// Repositions the sidebar and everything that sits on it.
/// This routine is called whenever the sidebar changes which edge of the screen it lives
/// on, or the display it must fit into changes size. The sidebar rectangle is placed first
/// and then the mode buttons, scroll arrows, cameo buttons and their tooltip regions are
/// moved to suit.
/// </summary>
void SidebarClass::Reposition_Sidebar(void)
{
	int index;

	/*
	 * Position the sidebar.
	 */
	SidebarRect.X = Options.IsSidebarOnRight ? TacticalRect.X + TacticalRect.Width : 0;
	SidebarRect.Y = SIDE_Y;
	SidebarRect.Width = SIDE_WIDTH;
	SidebarRect.Height = TacticalRect.Height + TacticalRect.Y - SIDE_Y;

	BASECLASS::Reposition_Sidebar();

	if (!SidebarShape) {
		SidebarShape = (ShapeSet const *)MFCD::Retrieve("SIDEGDI1.SHP");
		SidebarMiddleShape = (ShapeSet const *)MFCD::Retrieve("SIDEGDI2.SHP");
		SidebarBottomShape = (ShapeSet const *)MFCD::Retrieve("SIDEGDI3.SHP");
	}

	/*
	 * Position the sidebar's buttons.
	 */
	Background.Set_Position(SidebarRect.X + 16, TacticalRect.Y);
	Background.Flag_To_Redraw();

	Repair.Set_Position(SidebarRect.X + BUTTON_ONE_X, SidebarRect.Y + BUTTON_ONE_Y);
	Repair.Flag_To_Redraw();
	Repair.DrawOffsetX = -SidebarRect.X;

	Upgrade.Set_Position(Repair.X + BUTTON_SPACING, Power.Y);
	Upgrade.Flag_To_Redraw();
	Upgrade.DrawOffsetX = -SidebarRect.X;

	Power.Set_Position(Upgrade.X + BUTTON_SPACING, Repair.Y);
	Power.Flag_To_Redraw();
	Power.DrawOffsetX = -SidebarRect.X;

	Waypoint.Set_Position(Power.X + BUTTON_SPACING, Upgrade.Y);
	Waypoint.Flag_To_Redraw();
	Waypoint.DrawOffsetX = -SidebarRect.X;

	/*
	 * Create the tooltips for the sidebar.
	 */
	if (ToolTips) {
		ToolTip tooltip;

		for (index = 0; index < COLUMNS; index++) {
			for (int j = 0; j < 100; j++) {
				ToolTips->Remove((j | (index << 8)) + GADGET_CAMEO);
			}
		}
		int arrowy = SidebarRect.Y + Map.Max_Visible() * StripClass::OBJECT_HEIGHT + StripClass::UP_Y_OFFSET;

		for (int col = 0; col < COLUMNS; col++) {

			StripClass::UpButton[col].Set_Position(SidebarRect.X + Column[col].X + StripClass::UP_X_OFFSET, arrowy);
			StripClass::UpButton[col].Flag_To_Redraw();
			StripClass::UpButton[col].DrawOffsetX = -SidebarRect.X;

			StripClass::DownButton[col].Set_Position(SidebarRect.X + Column[col].X + StripClass::DOWN_X_OFFSET, arrowy);
			StripClass::DownButton[col].Flag_To_Redraw();
			StripClass::DownButton[col].DrawOffsetX = -SidebarRect.X;

			for (int i = 0; i < Map.Max_Visible(); i++) {
				StripClass::SelectButton[col][i].Set_Position(SidebarRect.X + Column[col].X, SidebarRect.Y + Column[col].Y + (StripClass::OBJECT_HEIGHT * i));
				StripClass::SelectButton[col][i].Flag_To_Redraw();
				ToolTip tmp;
				tmp.Text = TXT_NONE;
				tmp.ID = (i | (col << 8)) + GADGET_CAMEO;
				tmp.Region.Set(StripClass::SelectButton[col][i].X,	  StripClass::SelectButton[col][i].Y,
								   StripClass::SelectButton[col][i].Width, StripClass::SelectButton[col][i].Height);
				ToolTips->Add(&tmp);
			}

		}

		tooltip.ID = BUTTON_REPAIR;
		tooltip.Text = TXT_REPAIR_MODE;
		tooltip.Region.Set(Repair.X, Repair.Y, Repair.Width, Repair.Height);
		ToolTips->Remove(tooltip.ID);
		ToolTips->Add(&tooltip);

		tooltip.ID = BUTTON_POWER;
		tooltip.Text = TXT_POWER_MODE;
		tooltip.Region.Set(Power.X, Power.Y, Power.Width, Power.Height);
		ToolTips->Remove(tooltip.ID);
		ToolTips->Add(&tooltip);

		tooltip.ID = BUTTON_SELL;
		tooltip.Text = TXT_SELL_MODE;
		tooltip.Region.Set(Upgrade.X, Upgrade.Y, Upgrade.Width, Upgrade.Height);
		ToolTips->Remove(tooltip.ID);
		ToolTips->Add(&tooltip);

		tooltip.ID = BUTTON_WAYPOINT;
		tooltip.Text = TXT_WAYPOINTMODE;
		tooltip.Region.Set(Waypoint.X, Waypoint.Y, Waypoint.Width, Waypoint.Height);
		ToolTips->Remove(tooltip.ID);
		ToolTips->Add(&tooltip);
	}

	int x = 0;
	int y = RadarButton.Y + RadarButton.Height;
	if (Options.IsSidebarOnRight) {
		Background.Set_Position(TacticalRect.X + TacticalRect.Width, y);
	} else {
		Background.Set_Position(x, y);
	}
	Background.Set_Size(SidebarSurface->Get_Width(), SidebarSurface->Get_Height() - y);
}


/// <summary>
/// Fetches the tooltip text for a sidebar gadget.
/// This routine is called by the tooltip manager. Any gadget the map layers underneath do
/// not claim is taken to be a build cameo and handed to the strip that owns it.
/// </summary>
/// <param name="id">The gadget identifier that text is wanted for.</param>
/// <returns>Returns with a pointer to the help text, or NULL if the gadget has none.</returns>
const char * SidebarClass::Help_Text(int id)
{
	const char *text = BASECLASS::Help_Text(id);
	if (text == NULL) {
		id = id - GADGET_CAMEO;
		int index = id >> 8;
		if (index < COLUMNS && index >= 0) {
			text = Column[index].Help_Text(id % 256U);
		}
	}
	return(text);
}


/// <summary>
/// Determines how many cameos a sidebar strip can show.
/// The answer follows from the height of the sidebar, so it varies with the screen
/// resolution. It is used to lay the strips out and to decide when scrolling is called for.
/// </summary>
/// <returns>Returns with the number of build cameos that will fit in one strip.</returns>
/// <remarks>A strip has MAX_SLOTS select buttons, so a taller picture than those can fill
/// shows the slots it has rather than the slots that would fit.</remarks>
int SidebarClass::Max_Visible(void)
{
	if (SidebarSurface != NULL && SidebarShape != NULL) {
		Rect r = SidebarRect;
		int fits = (r.Height - SidebarBottomShape->Get_Height() - SidebarShape->Get_Height()) / SidebarMiddleShape->Get_Height();
		return(std::min(fits, int(StripClass::MAX_SLOTS)));
	}
	return(StripClass::MAX_VISIBLE);
}


/// <summary>
/// Prints the descriptive text that goes with a sidebar cameo.
/// This routine is used by the strip renderer to caption a cameo with its name and cost.
/// Text too wide for the space is broken at a space or a hyphen and carried onto as many
/// lines as it takes.
/// </summary>
/// <param name="point">Where to print the last line. Earlier lines stack upward from
/// there.</param>
/// <param name="cliprect">The clipping rectangle to print within.</param>
/// <param name="maxlinelen">The width, in pixels, a line may reach before it is broken.</param>
/// <remarks>Nothing at all is printed unless the player has cameo text turned on.</remarks>
void Print_Cameo_Text(char const * string, Point2D const & point, Rect const & cliprect, int maxlinelen)
{
	char buffer[64];

	if (Map.IsCameoText == true && string != NULL) {
		FontClass * font = Font_From_TPF(TextPrintType(TPF_6POINT|TPF_METAL12|TPF_FULLSHADOW));
		Point2D drawpoint = point;

		if (font->String_Pixel_Width(string) > maxlinelen) {
			int len = strlen(string);
			strcpy(buffer, string);
			string = buffer;

			int w = 0;
			do {
				/*
				**	While the current line is less then the max length...
				*/
				int linelen = font->Char_Pixel_Width(buffer[len]);
				while (linelen <= maxlinelen && len-- > 0) {
					linelen += font->Char_Pixel_Width(buffer[len]);
				}

				/*
				**	Back up to an appropriate location to break.
				*/
				while (linelen > maxlinelen) {
					while (buffer[len] != ' ' && buffer[len] != '-' && buffer[len] != '\0') {
						linelen -= font->Char_Pixel_Width(buffer[len]);
						len++;
					}

					if (buffer[len] == '\0') {
						len = 0;
						break;
					}

					if (linelen <= maxlinelen) {
						break;
					}

					linelen -= font->Char_Pixel_Width(buffer[len]);
					len++;
				}

				if (len <= 0) {
					break;
				}

				if (buffer[len] == ' ') {
					buffer[len] = '\0';
					len++;
				} else if (buffer[len] == '-') {
					len++;
				}

				Fancy_Text_Print(&buffer[len], *SidebarSurface, cliprect, drawpoint, Fetch_Scheme_By_Name("LightGrey"), TBLACK, TextPrintType(TPF_FULLSHADOW|TPF_EFNT));
				buffer[len] = '\0';
				drawpoint.Y -= font->Get_Height();
				w = font->String_Pixel_Width(buffer);

			} while (len > 0 && w > maxlinelen);

			/*
			 * Print the rest of the string.
			 */
			finish:;
		}

		Fancy_Text_Print(string, *SidebarSurface, cliprect, drawpoint, Fetch_Scheme_By_Name("LightGrey"), TBLACK, TextPrintType(TPF_FULLSHADOW|TPF_EFNT));
	}
}


/// <summary>
/// Lists the members the sidebar holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void SidebarClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// SidebarShape -- backdrop artwork, fetched for the player's house by Init_For_House.
	// SidebarMiddleShape
	// SidebarBottomShape
	// SidebarAddonShape
	stream.Serialize(Column);
	// IsToRedraw -- a redraw flag; the load asks for a complete draw anyway.
	stream.Serialize(IsCameoText);
	stream.Serialize(IsSidebarActive);

	// IsForceCompleteRedraw -- likewise redraw flags.
	// IsToRedrawCredits
	// Repair -- the sidebar buttons and their blit flag, all set up again by Init_IO.
	// Upgrade
	// Power
	// Waypoint
	// Background
	// IsToBlitSidebar
	stream.Serialize(IsRepairActive);
	stream.Serialize(IsUpgradeActive);
	stream.Serialize(IsDemolishActive);
}


/// <summary>
/// Lists the members one side strip holds.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void SidebarClass::StripClass::Serialize(SaveStreamClass & stream)
{
	StageClass::Serialize(stream);

	/*
	 * not saved: X, Y, ObjectRect, ID -- where the strip sits on the sidebar and which strip it
	 * is, both established by the sidebar constructor and Init_IO for the current screen.
	 *
	 * not saved: IsToRedraw -- a redraw flag; the load asks for a complete draw anyway.
	 */
	stream.Serialize(IsBuilding);
	stream.Serialize(IsScrollingDown);
	stream.Serialize(IsScrolling);
	stream.Serialize(Flasher);
	stream.Serialize(TopIndex);
	stream.Serialize(Scroller);
	stream.Serialize(Slid);
	stream.Serialize(LastSlid);
	stream.Serialize(BuildableCount);
	stream.Serialize(Buildables);

	/*
	 * not saved: LogoShapes, ClockShapes, RechargeClockShapes, DarkenShapes, UpButton,
	 * DownButton, SelectButton -- cameo artwork and the strip's buttons, all set up again by
	 * One_Time, Init_For_House and Init_IO.
	 */
}
