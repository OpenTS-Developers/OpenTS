/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// The engine side of the wait box: the class the save, load and progress code shows while it
// works. The presenter and view live in uiwaitbox.cpp so that the test harness can drive them
// without the engine.

#include "ui/uiwaitbox.h"

#include "ownrdraw.h"
#include "ui/uirmlview.h"
#include "ui/uishell.h"


UIWaitBoxClass::UIWaitBoxClass(void) :
	Dialog(NULL)
{
}


UIWaitBoxClass::~UIWaitBoxClass(void)
{
	Hide();
}


void UIWaitBoxClass::Show(char const * text)
{
	Hide();

	if (Show_Document(text, false)) {
		return;
	}

	Dialog = OwnerDraw::Custom_Message_Box(text, NULL, NULL);
	if (Dialog != NULL) {
		OwnerDraw::Display_Dialog(Dialog);
	}
}


bool UIWaitBoxClass::Show_Document(char const * text, bool bar)
{
	Hide();

	// A visible Win32 dialog takes the mouse before a document can, so a notice over one stays Win32.
	if (!UI_Use_Rml() || UI_Legacy_Dialog_Visible()) {
		return(false);
	}

	Presenter = std::make_unique<UIWaitBoxPresenterClass>((text != NULL) ? text : "", bar);
	View = UI_Wait_Box_View(*Presenter);

	if (!UI_Show_Modeless(*View)) {
		View.reset();
		Presenter.reset();
		return(false);
	}
	return(true);
}


void UIWaitBoxClass::Set_Text(char const * text)
{
	if (View != nullptr) {
		Presenter->Text = (text != NULL) ? text : "";
		View->Sync();
		UI_Refresh();
	} else if (Dialog != NULL) {
		OwnerDraw::Set_Custom_Message_Box_Text(Dialog, text);
	}
}


void UIWaitBoxClass::Set_Fraction(double fraction)
{
	if (View != nullptr) {
		Presenter->Set_Fraction(fraction);
		View->Sync();
		UI_Refresh();
	}
}


void UIWaitBoxClass::Hide(void)
{
	if (View != nullptr) {
		UI_Hide_Modeless(*View);
		View.reset();
		Presenter.reset();
	}

	if (Dialog != NULL) {
		OwnerDraw::End_Dialog(Dialog);
		Dialog = NULL;
	}
}


bool UIWaitBoxClass::Is_Shown(void) const
{
	return(View != nullptr || Dialog != NULL);
}
