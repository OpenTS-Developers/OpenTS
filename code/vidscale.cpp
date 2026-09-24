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
#include "win.h"

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
void Window_Point_To_Game(POINT & point)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	if (scale.DestWidth > 0 && scale.DestHeight > 0) {
		point.x = (LONG)floor((point.x - scale.DestX) * (double)scale.GameWidth / (double)scale.DestWidth);
		point.y = (LONG)floor((point.y - scale.DestY) * (double)scale.GameHeight / (double)scale.DestHeight);
	}
}


/// <summary>
/// Converts a position in the frame into one in the window's client area.
/// </summary>
/// <param name="point">The position to convert in place. It comes back at the top left
/// corner of the area the frame pixel covers on screen.</param>
void Game_Point_To_Window(POINT & point)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	if (scale.GameWidth > 0 && scale.GameHeight > 0) {
		point.x = scale.DestX + (LONG)floor(point.x * (double)scale.DestWidth / (double)scale.GameWidth);
		point.y = scale.DestY + (LONG)floor(point.y * (double)scale.DestHeight / (double)scale.GameHeight);
	}
}


/// <summary>
/// Pulls a position onto the frame if it lies outside it.
/// </summary>
/// <param name="point">The position to clamp in place.</param>
void Clamp_To_Game(POINT & point)
{
	VideoScaleInfo const & scale = Video_Get_Scale_Info();

	if (point.x < 0) point.x = 0;
	if (point.y < 0) point.y = 0;
	if (scale.GameWidth > 0 && point.x >= scale.GameWidth) point.x = scale.GameWidth - 1;
	if (scale.GameHeight > 0 && point.y >= scale.GameHeight) point.y = scale.GameHeight - 1;
}
