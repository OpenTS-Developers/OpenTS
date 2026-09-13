/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// What the presenter owes the screen: a game frame, a UI overlay, or both. Marks raised
// while a present runs are kept for the next one, and a present the renderer refuses
// gives its marks back.

#pragma once


// What one present has to do: draw the game, draw the overlay, and whether the frame's
// pixels have to be uploaded first.
struct VideoDirtySnapshotType
{
	bool Game;
	bool Overlay;
	bool Upload;
};


class VideoDirtyStateClass
{
	public:
		void Mark_Game(void);
		void Mark_Overlay(void);
		// The renderer no longer holds the frame, so the next present uploads it.
		void Invalidate_Frame(void);
		void Upload_Completed(void);
		void Reset(void);

		bool Is_Dirty(void) const;

		// Takes the marks for a present that is about to run.
		VideoDirtySnapshotType Consume(void);
		// Gives back the marks of a present that did not happen. A present is always owed
		// afterwards, at least of the overlay, so a refused one is retried.
		void Restore(VideoDirtySnapshotType const & snapshot);

	private:
		bool Game = false;
		bool Overlay = false;
		bool Uploaded = false;
};
