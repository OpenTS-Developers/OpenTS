/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Stands in for the window, the SDL layer and the frame: nothing is pumped, no key is held,
// and every position lies inside the frame.

#include "msgloop.h"
#include "sdl/sdlwindow.h"
#include "vidscale.h"


void Windows_Message_Handler(void)
{
}


bool Main_Window_Key_Down(int)
{
	return(false);
}


void Clamp_To_Game(Point2D &)
{
}
