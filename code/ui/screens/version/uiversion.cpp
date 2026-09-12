/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/version/uiversion.h"

#include "ui/rml/rmlview.h"

#include <utility>


UIVersionPresenterClass::UIVersionPresenterClass(std::vector<std::string> lines) :
	Lines(std::move(lines))
{
}


void UIVersionPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "ok") {
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "cancel") {
		Result = UI_RESULT_CANCELLED;
	}
}


void UIVersionPresenterClass::Refresh(void)
{
}


namespace
{

class UIVersionViewClass : public UIRmlViewClass
{
	public:
		explicit UIVersionViewClass(UIVersionPresenterClass & presenter) :
			UIRmlViewClass(presenter, "version.rml", "version"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.RegisterArray<std::vector<std::string>>() && model.Bind("lines", &Data.Lines));
		}

	private:
		UIVersionPresenterClass & Data;
};

}


std::unique_ptr<UIRmlViewClass> UI_Version_View(UIVersionPresenterClass & presenter)
{
	return(std::make_unique<UIVersionViewClass>(presenter));
}
