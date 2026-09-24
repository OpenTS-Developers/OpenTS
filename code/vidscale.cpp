/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "vidscale.h"

#include "video.h"

#include <cmath>


/// <summary>
/// Is the frame drawn at some size or position other than the window's own?
/// </summary>
/// <returns>bool; Do window positions need converting before the game sees them?</returns>
bool Video_Scaling_Active(void)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	return(scale.DestX != 0 || scale.DestY != 0 || scale.DestWidth != scale.GameWidth || scale.DestHeight != scale.GameHeight);
}


/// <summary>
/// Converts a position in the window's client area into one in the frame.
/// A position on one of the letterbox bars lands outside the frame rather than being
/// pulled onto its edge.
/// </summary>
/// <param name="point">The position to convert in place.</param>
void Window_Point_To_Game(Point2D & point)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	if (scale.DestWidth > 0 && scale.DestHeight > 0) {
		point.X = (int)floor((point.X - scale.DestX) * (double)scale.GameWidth / (double)scale.DestWidth);
		point.Y = (int)floor((point.Y - scale.DestY) * (double)scale.GameHeight / (double)scale.DestHeight);
	}
}


/// <summary>
/// Converts a position in the frame into one in the window's client area.
/// </summary>
/// <param name="point">The position to convert in place. It comes back at the top left
/// corner of the area the frame pixel covers on screen.</param>
void Game_Point_To_Window(Point2D & point)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	if (scale.GameWidth > 0 && scale.GameHeight > 0) {
		point.X = scale.DestX + (int)floor(point.X * (double)scale.DestWidth / (double)scale.GameWidth);
		point.Y = scale.DestY + (int)floor(point.Y * (double)scale.DestHeight / (double)scale.GameHeight);
	}
}


/// <summary>
/// Pulls a position onto the frame if it lies outside it.
/// </summary>
/// <param name="point">The position to clamp in place.</param>
void Clamp_To_Game(Point2D & point)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	if (point.X < 0) point.X = 0;
	if (point.Y < 0) point.Y = 0;
	if (scale.GameWidth > 0 && point.X >= scale.GameWidth) point.X = scale.GameWidth - 1;
	if (scale.GameHeight > 0 && point.Y >= scale.GameHeight) point.Y = scale.GameHeight - 1;
}
