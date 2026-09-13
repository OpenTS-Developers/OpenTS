/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "videodirty.h"


void VideoDirtyStateClass::Mark_Game(void)
{
	Game = true;
}


void VideoDirtyStateClass::Mark_Overlay(void)
{
	Overlay = true;
}


void VideoDirtyStateClass::Invalidate_Frame(void)
{
	Uploaded = false;
	Game = true;
}


void VideoDirtyStateClass::Upload_Completed(void)
{
	Uploaded = true;
}


void VideoDirtyStateClass::Reset(void)
{
	Game = false;
	Overlay = false;
	Uploaded = false;
}


bool VideoDirtyStateClass::Is_Dirty(void) const
{
	return(Game || Overlay);
}


VideoDirtySnapshotType VideoDirtyStateClass::Consume(void)
{
	VideoDirtySnapshotType result = { Game, Overlay, Game || !Uploaded };
	Game = false;
	Overlay = false;
	return(result);
}


void VideoDirtyStateClass::Restore(VideoDirtySnapshotType const & snapshot)
{
	Game = Game || snapshot.Game;
	Overlay = Overlay || snapshot.Overlay;
	if (!Game && !Overlay) {
		Overlay = true;
	}
}
