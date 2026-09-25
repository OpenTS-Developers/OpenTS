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

#include "always.h"

#include "tooltip.h"

#include "data.h"
#include "dbgprint.h"
#include "sdl/sdlwindow.h"
#include "vidscale.h"
#include "windowevent.hh"


/// <summary>
/// Constructs an empty tooltip manager.
/// The manager starts out deactivated and carrying no tooltips, with the default hover
/// delay and lifetime.
/// </summary>
/// <remarks>No tooltip will appear until Activate is called.</remarks>
ToolTipManager::ToolTipManager(void) :
	TimerRunning(false),
	TimerDue(0),
	IsActive(false),
	CurrentToolTip(NULL),
	ToolTipDelay(TOOLTIP_DELAY),
	ToolTipLifetime(TOOLTIP_LIFETIME)
{
	ToolTips.Clear();
	ToolTipIndex.Clear();
}


/// <summary>
/// Destroys the tooltip manager and every tooltip registered with it.
/// </summary>
ToolTipManager::~ToolTipManager(void)
{
	Reset_Current();

	ToolTipIndex.Clear();

	for (int i = 0; i < ToolTips.Count(); i++) {
		ToolTip * ptr = (ToolTip *)ToolTips[i];
		if (ptr != NULL) {
			delete ptr;
		}
	}

	ToolTips.Clear();
}


/// <summary>
/// Turns the tooltip system on or off.
/// A deactivated manager ignores the message traffic entirely, so no tooltip will appear
/// until it is switched back on. Anything on display is taken down as it goes off.
/// </summary>
/// <param name="state">Should tooltips be displayed?</param>
void ToolTipManager::Activate(bool state)
{
	if (IsActive != state) {
		IsActive = state;
		if (!IsActive) {
			TimerRunning = false;
			Reset_Current();
		}

		DebugString("Tooltips are %s.\n", IsActive == true ? "on" : "off");
	}
}


/// <summary>
/// Follows the mouse over the main window. Mouse movement restarts the hover countdown, and
/// a button press or release takes down whatever is showing.
/// </summary>
void ToolTipManager::Handle_Window_Event(WindowEvent const & event)
{
	if (IsActive != true) {
		return;
	}

	switch (event.Type) {
		case WINDOW_EVENT_MOUSE_DOWN:
		case WINDOW_EVENT_MOUSE_UP:
			if (event.Button <= WINDOW_BUTTON_MIDDLE && event.Clicks == 1) {
				TimerRunning = false;
				Reset_Current();
			}
			break;

		case WINDOW_EVENT_MOUSE_MOVE:
			Start_Timer(ToolTipDelay);
			Reset_Current();
			break;

		default:
			break;
	}
}


/// <summary>
/// Brings up the tooltip under the mouse once it has rested for the hover delay, and takes it
/// down once its lifetime has passed. Tooltips change only when the message pump calls this.
/// </summary>
void ToolTipManager::Service(void)
{
	if (IsActive != true || !TimerRunning || (long)(timeGetTime() - TimerDue) < 0) {
		return;
	}

	TimerRunning = false;
	if (CurrentToolTip != NULL) {
		Reset_Current();
	} else {
		int x = 0;
		int y = 0;
		Main_Window_Cursor_Position(x, y);
		LastMousePos.X = x;
		LastMousePos.Y = y;
		Window_Point_To_Game(LastMousePos);
		CurrentToolTip = Find_From_Pos(LastMousePos);
		if (Process() == true) {
			Start_Timer(ToolTipLifetime);
		}
	}
}


void ToolTipManager::Start_Timer(int delay)
{
	TimerRunning = true;
	TimerDue = timeGetTime() + (unsigned long)delay;
}


/// <summary>
/// Fetches how long the mouse must hover before a tooltip appears.
/// </summary>
/// <returns>Returns with the hover time, in milliseconds.</returns>
int ToolTipManager::Get_Timer_Delay(void)
{
	return(ToolTipDelay);
}


/// <summary>
/// Sets how long the mouse must hover before a tooltip appears.
/// </summary>
/// <param name="delay">The hover time, in milliseconds.</param>
void ToolTipManager::Set_Timer_Delay(int delay)
{
	ToolTipDelay = delay;
}


/// <summary>
/// Fetches how long a tooltip stays on screen.
/// </summary>
/// <returns>Returns with the display time, in milliseconds.</returns>
int ToolTipManager::Get_Lifetime(void)
{
	return(ToolTipLifetime);
}


/// <summary>
/// Sets how long a tooltip stays on screen.
/// </summary>
/// <param name="lifetime">The display time, in milliseconds.</param>
void ToolTipManager::Set_Lifetime(int lifetime)
{
	ToolTipLifetime = lifetime;
}


/// <summary>
/// Fetches the number of tooltips registered with this manager.
/// </summary>
/// <returns>Returns with the count of tooltips currently registered.</returns>
int ToolTipManager::Get_Count(void)
{
	return(ToolTips.Count());
}


/// <summary>
/// Adds a tooltip to the manager.
/// The manager takes a copy, so the caller's record does not have to outlive the call.
/// Identifiers must be unique -- a duplicate is refused rather than displacing the tooltip
/// already registered under that identifier.
/// </summary>
/// <param name="tooltip">The tooltip to copy into the manager.</param>
/// <returns>bool; Was the tooltip added?</returns>
bool ToolTipManager::Add(ToolTip const * tooltip)
{
	if (!Find(tooltip->ID, NULL)) {
		ToolTip * tt = new ToolTip(*tooltip);
		if (tt != NULL) {
			if (ToolTips.Add(tt)) {
				ToolTipIndex.Add_Index(tt->ID, tt);
				return(true);
			}
			delete tt;
		}
	}
	return(false);
}


/// <summary>
/// Removes the tooltip with the identifier specified.
/// The tooltip is destroyed outright. If it happens to be the one on display, the display
/// is cleared first so that nothing is left pointing at it.
/// </summary>
/// <param name="id">Identifier of the tooltip to remove.</param>
void ToolTipManager::Remove(unsigned id)
{
	if (ToolTipIndex.Is_Present(id)) {
		ToolTip * tooltip = (ToolTip *)ToolTipIndex[id];

		if (CurrentToolTip == tooltip) {
			Reset_Current();
		}

		ToolTipIndex.Remove_Index(tooltip->ID);
		ToolTips.Delete(tooltip);

		if (tooltip != NULL) {
			delete tooltip;
		}
	}
}


/// <summary>
/// Finds the tooltip with the identifier specified.
/// </summary>
/// <param name="id">Identifier of the tooltip to look for.</param>
/// <param name="tooltip">Optional destination to copy the tooltip found into.</param>
/// <returns>bool; Was a tooltip with that identifier registered?</returns>
bool ToolTipManager::Find(unsigned id, ToolTip * tooltip)
{
	if (!ToolTipIndex.Is_Present(id)) {
		return(false);
	}

	if (tooltip != NULL) {
		*tooltip = *ToolTipIndex[id];
	}

	return(true);
}


/// <summary>
/// Finds the tooltip whose region covers the point specified.
/// This routine is used to work out which tooltip, if any, the mouse is resting over.
/// </summary>
/// <param name="xy">The point, in frame coordinates, to test the regions against.</param>
/// <returns>Returns with a pointer to the tooltip found, or NULL if the point is bare.</returns>
ToolTip const * ToolTipManager::Find_From_Pos(Point2D &xy)
{
	for (int i = 0; i < ToolTips.Count(); i++) {
		Rect const & region = ToolTips[i]->Region;
		if (xy.X >= region.X &&
			xy.X <= region.X + region.Width &&
			xy.Y >= region.Y &&
			xy.Y <= region.Y + region.Height) {

			return(ToolTips[i]);
		}
	}
	return(NULL);
}


/// <summary>
/// Works out the display metrics for the tooltip text specified.
/// This is the hook a derived manager overrides in order to measure the text and decide
/// where the tooltip box will sit. The base manager takes the text as it stands.
/// </summary>
/// <param name="text">The tooltip display record to prepare.</param>
/// <returns>bool; Can the tooltip be displayed?</returns>
bool ToolTipManager::Update(ToolTipText *text)
{
	return(true);
}


/// <summary>
/// Clears the pending tooltip display record.
/// A derived manager overrides this routine when it has display resources of its own to
/// give back as the tooltip is retired.
/// </summary>
/// <param name="text">The tooltip display record being retired.</param>
void ToolTipManager::Reset(const ToolTipText *text)
{
	CurrentToolTipInfo.Pos.X = 0;
	CurrentToolTipInfo.Pos.Y = 0;
	CurrentToolTipInfo.TextWidth = 0;
	CurrentToolTipInfo.TextHeight = 0;
	CurrentToolTip = NULL;
}


/// <summary>
/// Prepares the current tooltip for display.
/// This routine gathers the text belonging to the tooltip the mouse is resting over and
/// positions it at the cursor. A tooltip with nothing to say is dropped rather than shown.
/// </summary>
/// <returns>bool; Is there a tooltip ready to be drawn?</returns>
bool ToolTipManager::Process(void)
{
	ToolTip const * cur = CurrentToolTip;
	if (cur != NULL) {
		const char *str = cur->Text == 0 ? ToolTip_Text(cur->ID) : Fetch_String(cur->Text);

		if (str != NULL && strlen(str) > 0) {

			strncpy(CurrentToolTipInfo.Text, str, sizeof(CurrentToolTipInfo.Text));

			CurrentToolTipInfo.Pos.X = LastMousePos.X;
			CurrentToolTipInfo.Pos.Y = LastMousePos.Y;
			CurrentToolTipInfo.TextWidth = 0;
			CurrentToolTipInfo.TextHeight = 0;

			ToolTipText & txt = CurrentToolTipInfo;

			if (!Update(&CurrentToolTipInfo)) {
				txt.Pos.X = 0;
				txt.Pos.Y = 0;
				txt.TextWidth = 0;
				txt.TextHeight = 0;
				CurrentToolTip = NULL;
			}

		} else {
			CurrentToolTipInfo.Pos.X = 0;
			CurrentToolTipInfo.Pos.Y = 0;
			CurrentToolTipInfo.TextWidth = 0;
			CurrentToolTipInfo.TextHeight = 0;
			CurrentToolTip = NULL;
		}
	}

	return(CurrentToolTip != NULL);
}


/// <summary>
/// Draws the tooltip currently pending, if there is one.
/// This routine is called from the redraw path. Nothing is drawn unless the mouse has
/// rested long enough over a region that has a tooltip attached to it.
/// </summary>
/// <param name="refresh">Should the tooltip be prepared afresh before it is drawn?</param>
void ToolTipManager::Draw_Current(bool refresh)
{
	if (refresh == true) {
		Process();
	}

	if (CurrentToolTip != NULL) {
		Draw(&CurrentToolTipInfo);
	}
}


/// <summary>
/// Draws the tooltip text specified.
/// This is the hook a derived manager overrides in order to render tooltips in the game's
/// own style. The base manager merely reports the text to the debug output.
/// </summary>
/// <param name="text">The tooltip text and placement to render.</param>
void ToolTipManager::Draw(const ToolTipText *text)
{
	DebugString("ToolTip: %s\n", text->Text);
}


/// <summary>
/// Fetches the text for the tooltip specified.
/// This is the hook a derived manager overrides in order to supply text that does not come
/// from the string table. The base manager has no such text to offer.
/// </summary>
/// <param name="id">Identifier of the tooltip whose text is needed.</param>
/// <returns>Returns with a pointer to the text, or NULL if there is none to be had.</returns>
const char *ToolTipManager::ToolTip_Text(int id)
{
	return(NULL);
}


/// <summary>
/// Clears the tooltip currently on display.
/// This routine is used whenever the tooltip must go away -- the mouse moved, a button was
/// pressed, or the tooltip simply outlived its welcome.
/// </summary>
void ToolTipManager::Reset_Current(void)
{
	if (CurrentToolTip != NULL) {
		Reset(&CurrentToolTipInfo);
	}
}
