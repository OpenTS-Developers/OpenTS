/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Stands in for the window, the platform and the frame the key queue reaches, so the queue
// runs with no window: nothing arrives from a pump, no key is held, and every position lies
// inside the frame.

#include "msgloop.h"
#include "platform/platform.h"
#include "vidscale.h"


void Windows_Message_Handler(void)
{
}


bool Platform_Key_Down(int)
{
	return(false);
}


void Clamp_To_Game(POINT &)
{
}
