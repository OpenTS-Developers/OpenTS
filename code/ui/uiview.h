/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The view the shell runs: one presentation of a presenter, whichever toolkit draws it.
// No toolkit type appears here; each toolkit's view implements it over its own.

#pragma once

class UIPresenterClass;
class UIShellClass;


class UIViewClass
{
	public:
		virtual ~UIViewClass(void) = default;

		// Loads what the view needs against the shell. False leaves nothing behind, so the
		// caller can open another presentation.
		virtual bool Prepare(UIShellClass & shell) = 0;
		virtual void Show(bool modal) = 0;
		virtual void Hide(void) = 0;
		// Drops what Prepare loaded. Safe to call more than once.
		virtual void Release(void) = 0;
		// Pushes what the presenter changed into the presentation.
		virtual void Sync(void) = 0;
		virtual bool Is_Shown(void) const = 0;

		virtual UIPresenterClass & Presenter(void) const = 0;
		// What to call the view in a log line.
		virtual char const * Name(void) const = 0;
};
