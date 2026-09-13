/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The developer overlays as the harness sees them: never active, wanting nothing. The
// shell links against these instead of the Dear ImGui module, which reaches the engine.

#include "ui/dev/uidev.h"


bool UIDev_Active(void)
{
	return(false);
}


void UIDev_Toggle(UIRmlRenderClass const &)
{
}


void UIDev_Tick(void)
{
}


void UIDev_Render(UIRmlRenderClass &)
{
}


void UIDev_Shutdown(UIRmlRenderClass &)
{
}


void UIDev_Mouse_Position(int, int)
{
}


bool UIDev_Mouse_Button(int, bool)
{
	return(false);
}


bool UIDev_Mouse_Wheel(float)
{
	return(false);
}


bool UIDev_Key(WPARAM, bool)
{
	return(false);
}


bool UIDev_Character(wchar_t)
{
	return(false);
}


void UIDev_Focus(bool)
{
}


bool UIDev_Wants_Mouse(void)
{
	return(false);
}
