/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/waitbox/uiwaitbox.h"

#include "ui/rml/rmlview.h"

#include <utility>


UIWaitBoxPresenterClass::UIWaitBoxPresenterClass(std::string text, bool bar) :
	Text(std::move(text)),
	Bar(bar)
{
}


void UIWaitBoxPresenterClass::Execute(UIIntent const &)
{
}


void UIWaitBoxPresenterClass::Refresh(void)
{
}


void UIWaitBoxPresenterClass::Set_Fraction(double fraction)
{
	if (fraction < 0.0) {
		fraction = 0.0;
	}
	if (fraction > 1.0) {
		fraction = 1.0;
	}
	Percent = (int)(fraction * 100.0 + 0.5);
}


namespace
{

class UIWaitBoxViewClass : public UIRmlViewClass
{
	public:
		explicit UIWaitBoxViewClass(UIWaitBoxPresenterClass & presenter) :
			UIRmlViewClass(presenter, "wait.rml", "wait"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.Bind("text", &Data.Text) && model.Bind("bar", &Data.Bar) && model.Bind("percent", &Data.Percent));
		}

	private:
		UIWaitBoxPresenterClass & Data;
};

}


std::unique_ptr<UIRmlViewClass> UI_Wait_Box_View(UIWaitBoxPresenterClass & presenter)
{
	return(std::make_unique<UIWaitBoxViewClass>(presenter));
}
