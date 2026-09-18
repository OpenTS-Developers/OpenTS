/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once


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
		void Invalidate_Frame(void);
		void Upload_Completed(void);
		void Reset(void);
		bool Is_Dirty(void) const;
		VideoDirtySnapshotType Consume(void);
		void Restore(VideoDirtySnapshotType const & snapshot);

	private:
		bool Game = false;
		bool Overlay = false;
		bool Uploaded = false;
};
