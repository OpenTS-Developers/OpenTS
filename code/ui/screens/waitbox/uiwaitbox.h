/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ui/uiscreen.h"
#include "win.h"

#include <memory>
#include <string>

class UIViewClass;


// A notice shown while the game works: a line of text and, when asked for, a bar. It takes
// no input and raises no intents.
class UIWaitBoxPresenterClass : public UIPresenterClass
{
	public:
		UIWaitBoxPresenterClass(std::string text, bool bar);

		virtual void Execute(UIIntent const & intent) override;
		virtual void Refresh(void) override;

		// Stores the fraction as a percentage, clamped to 0 through 100.
		void Set_Fraction(double fraction);

		std::string Text;
		bool Bar;
		int Percent = 0;
};


// The RmlUi view over a wait box presenter, bound to wait.rml. The presenter must outlive it.
std::unique_ptr<UIViewClass> UI_Wait_Box_View(UIWaitBoxPresenterClass & presenter);


// The notice a caller shows while it works: a document when the shell can draw one and no
// Win32 dialog is on screen, else the Win32 box. It hides itself when it goes out of scope.
class UIWaitBoxClass
{
	public:
		UIWaitBoxClass(void);
		~UIWaitBoxClass(void);

		void Show(char const * text);
		// The document alone. False shows nothing, so the caller can open its own Win32 presentation.
		bool Show_Document(char const * text, bool bar);
		void Set_Text(char const * text);
		void Set_Fraction(double fraction);
		void Hide(void);

		bool Is_Shown(void) const;

	private:
		HWND Dialog;
		std::unique_ptr<UIWaitBoxPresenterClass> Presenter;
		std::unique_ptr<UIViewClass> View;
};
