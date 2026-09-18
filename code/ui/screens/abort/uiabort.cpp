/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/abort/uiabort.h"

#include "ui/rml/rmlview.h"

#include <utility>


UIAbortPresenterClass::UIAbortPresenterClass(UIAbortState state) :
	State(std::move(state))
{
}


void UIAbortPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "abort") {
		Choice = UI_ABORT_QUIT;
		Result = UI_RESULT_ACCEPTED;
	} else if (intent.Name == "restart") {
		if (State.RestartEnabled) {
			Choice = UI_ABORT_RESTART;
			Result = UI_RESULT_ACCEPTED;
		}
	} else if (intent.Name == "ok" || intent.Name == "cancel") {
		Choice = UI_ABORT_CONTINUE;
		Result = UI_RESULT_CANCELLED;
	}
}


void UIAbortPresenterClass::Refresh(void)
{
}


namespace
{

class UIAbortViewClass : public UIRmlViewClass
{
	public:
		explicit UIAbortViewClass(UIAbortPresenterClass & presenter) :
			UIRmlViewClass(presenter, "abort.rml", "abort"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.Bind("restartcaption", &Data.State.RestartCaption)
				&& model.Bind("restartenabled", &Data.State.RestartEnabled));
		}

	private:
		UIAbortPresenterClass & Data;
};

}


std::unique_ptr<UIViewClass> UI_Abort_View(UIAbortPresenterClass & presenter)
{
	return(std::make_unique<UIAbortViewClass>(presenter));
}
