/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

class UIPresenterClass;
class UIShellClass;


class UIViewClass
{
	public:
		virtual ~UIViewClass(void) = default;

		virtual bool Prepare(UIShellClass & shell) = 0;
		virtual void Show(bool modal) = 0;
		virtual void Hide(void) = 0;
		virtual void Release(void) = 0;
		virtual void Sync(void) = 0;
		virtual bool Is_Shown(void) const = 0;
		virtual void Placed(void) {}

		virtual float Reveal_Width(void) const { return(0.0f); }
		virtual void Reveal_To(float width) { (void)width; }
		virtual void Reveal_Done(void) {}

		virtual UIPresenterClass & Presenter(void) const = 0;
		virtual char const * Name(void) const = 0;
};
