/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


// A pointer position in the overlay's own pixels.
struct UIPointerPosition
{
	int X;
	int Y;
	bool Inside;
};


// Converts a client area position into the overlay's coordinates. The overlay covers the
// frame's destination rectangle, whose right and bottom edges are exclusive. A position
// outside keeps its offset, so a captured pointer can still be followed there.
inline UIPointerPosition UI_Client_To_Overlay(int destx, int desty, int destwidth, int destheight, int clientx, int clienty)
{
	UIPointerPosition position;
	position.X = clientx - destx;
	position.Y = clienty - desty;
	position.Inside = position.X >= 0 && position.Y >= 0 && position.X < destwidth && position.Y < destheight;
	return(position);
}
