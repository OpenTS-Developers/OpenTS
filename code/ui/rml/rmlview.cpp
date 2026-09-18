/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/rml/rmlview.h"

#include "ui/uishell.h"

#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementScroll.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/ID.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Property.h>
#include <RmlUi/Core/Variant.h>
#include <cmath>


UIRmlViewClass::UIRmlViewClass(UIPresenterClass & presenter, char const * document, char const * model) :
	Owner(presenter),
	DocumentName(document),
	ModelName(model)
{
}


UIRmlViewClass::~UIRmlViewClass(void)
{
	Release();
}


bool UIRmlViewClass::Prepare(Rml::Context & context)
{
	Release();
	Host = &context;
	Types = Rml::MakeUnique<Rml::DataTypeRegister>();

	Rml::DataModelConstructor constructor = context.CreateDataModel(ModelName, Types.get());
	if (!constructor) {
		Rml::Log::Message(Rml::Log::LT_ERROR, "The data model %s for %s could not be created.", ModelName.c_str(), DocumentName.c_str());
		Release();
		return(false);
	}
	ModelCreated = true;

	bool bound = Bind(constructor);
	bound = constructor.BindEventCallback("queue", [this](Rml::DataModelHandle, Rml::Event &, Rml::VariantList const & arguments) {
		if (!arguments.empty()) {
			Rml::String const name = arguments[0].Get<Rml::String>();
			Rml::String text;
			int value = 0;
			if (arguments.size() > 1) {
				value = arguments[1].Get<int>();
				text = arguments[1].Get<Rml::String>();
			}

			if (arguments.size() > 2) {
				text = arguments[2].Get<Rml::String>();
			}

			Queue(name.c_str(), value, text.c_str());
		}
	}) && bound;
	Model = constructor.GetModelHandle();

	if (!bound) {
		Rml::Log::Message(Rml::Log::LT_ERROR, "The data model %s for %s could not be bound.", ModelName.c_str(), DocumentName.c_str());
		Release();
		return(false);
	}

	Doc = context.LoadDocument(DocumentName);
	if (Doc == nullptr) {
		Rml::Log::Message(Rml::Log::LT_ERROR, "%s did not load.", DocumentName.c_str());
		Release();
		return(false);
	}

	Doc->AddEventListener(Rml::EventId::Keydown, this, true);
	Doc->AddEventListener(Rml::EventId::Mousedown, this);
	Loaded();
	return(true);
}


bool UIRmlViewClass::Prepare(UIShellClass & shell)
{
	if (shell.Rml_Context() == nullptr || !Prepare(*shell.Rml_Context())) {
		return(false);
	}

	Shell = &shell;
	return(true);
}


void UIRmlViewClass::Show(bool modal)
{
	if (Doc != nullptr) {
		Doc->Show(modal ? Rml::ModalFlag::Modal : Rml::ModalFlag::None, Rml::FocusFlag::Document);
	}
}


void UIRmlViewClass::Hide(void)
{
	if (Doc != nullptr) {
		Doc->Hide();
	}
}


void UIRmlViewClass::Release(void)
{
	if (Doc != nullptr) {
		Doc->Hide();
		Doc->RemoveEventListener(Rml::EventId::Keydown, this, true);
		Doc->RemoveEventListener(Rml::EventId::Mousedown, this);
	}

	if (Host != nullptr) {
		if (ModelCreated) {
			Host->RemoveDataModel(ModelName);
		}
		if (Doc != nullptr) {
			Host->UnloadDocument(Doc);
		}
	}

	Doc = nullptr;
	Host = nullptr;
	Shell = nullptr;
	ModelCreated = false;
	Wallpaper = 0x7FFFFFFF;
	Model = Rml::DataModelHandle();
	Types.reset();
}


bool UIRmlViewClass::Is_Shown(void) const
{
	return(Doc != nullptr && Doc->IsVisible());
}


static Rml::Element * Reveal_Element(Rml::ElementDocument * document)
{
	return(document != nullptr ? document->GetElementById("reveal") : nullptr);
}


float UIRmlViewClass::Reveal_Width(void) const
{
	if (Doc == nullptr || Doc->GetAttribute<Rml::String>("reveal", "") == "none") {
		return(0.0f);
	}

	Rml::Element * reveal = Reveal_Element(Doc);
	return(reveal != nullptr ? reveal->GetBox().GetSize().x : 0.0f);
}


void UIRmlViewClass::Reveal_To(float width)
{
	Rml::Element * reveal = Reveal_Element(Doc);
	if (reveal == nullptr) {
		return;
	}

	Rml::Element * chrome = Doc->GetElementById("chrome");
	if (chrome != nullptr && !Anchored) {
		float full = reveal->GetBox().GetSize().x;
		chrome->SetProperty(Rml::PropertyId::Position, Rml::Property(Rml::Style::Position::Relative));
		chrome->SetProperty(Rml::PropertyId::Width, Rml::Property(full, Rml::Unit::PX));
		chrome->SetProperty(Rml::PropertyId::Left, Rml::Property(50.0f, Rml::Unit::PERCENT));
		chrome->SetProperty(Rml::PropertyId::MarginLeft, Rml::Property(full * -0.5f, Rml::Unit::PX));
		Anchored = true;
	}

	Doc->SetClass("revealing", true);
	reveal->SetProperty(Rml::PropertyId::Width, Rml::Property(width < 0.0f ? 0.0f : width, Rml::Unit::PX));
}


void UIRmlViewClass::Placed(void)
{
	Place_Wallpaper();
	Size_Grips();
}


void UIRmlViewClass::Size_Grips(void)
{
	Rml::Context * context = Doc != nullptr ? Doc->GetContext() : nullptr;
	if (context == nullptr) {
		return;
	}

	float ratio = context->GetDensityIndependentPixelRatio();
	if (ratio <= 0.0f) {
		ratio = 1.0f;
	}

	Rml::ElementList lists;
	Doc->GetElementsByClassName(lists, "list");
	for (Rml::Element * list : lists) {
		Rml::Element * bar = list->GetElementScroll()->GetScrollbar(Rml::ElementScroll::VERTICAL);
		Rml::Element * grip = nullptr;
		for (int index = 0; bar != nullptr && index < bar->GetNumChildren(true); index++) {
			if (bar->GetChild(index)->GetTagName() == "sliderbar") {
				grip = bar->GetChild(index);
			}
		}
		if (bar == nullptr || !bar->IsVisible() || grip == nullptr) {
			continue;
		}

		float row = 14.0f * ratio;
		int visible = (int)(list->GetClientHeight() / row);
		int count = (int)(list->GetScrollHeight() / row + 0.5f);
		int range = count - visible;
		if (range < 1) {
			continue;
		}

		float travel = list->GetClientHeight() / ratio - 44.0f;
		int height = (int)(travel - std::log((double)(range + 1)) * travel * 0.2);
		if (height < 14) {
			height = 14;
		}

		if (std::fabs(grip->GetBox().GetSize().y - (float)height * ratio) > 0.5f) {
			grip->SetProperty("height", Rml::ToString(height) + "dp");
			list->GetElementScroll()->FormatScrollbars();
		}
	}
}


void UIRmlViewClass::Place_Wallpaper(void)
{
	Rml::Element * wallpaper = Doc != nullptr ? Doc->GetElementById("wallpaper-art") : nullptr;
	Rml::Element * dialog = Doc != nullptr ? Doc->GetElementById("reveal") : nullptr;
	Rml::Context * context = Doc != nullptr ? Doc->GetContext() : nullptr;
	if (wallpaper == nullptr || dialog == nullptr || context == nullptr) {
		return;
	}

	float ratio = context->GetDensityIndependentPixelRatio();
	if (ratio <= 0.0f) {
		ratio = 1.0f;
	}

	float middle = (dialog->GetAbsoluteOffset(Rml::BoxArea::Border).y + dialog->GetBox().GetSize().y * 0.5f) / ratio;
	float top = std::floor(((float)context->GetDimensions().y / ratio - 400.0f) * 0.5f);
	float offset = top - middle;
	if (offset == Wallpaper) {
		return;
	}

	Wallpaper = offset;
	wallpaper->SetProperty("margin-top", Rml::ToString(offset) + "dp");
}


void UIRmlViewClass::Reveal_Done(void)
{
	Rml::Element * reveal = Reveal_Element(Doc);
	if (reveal == nullptr) {
		return;
	}

	Rml::Element * chrome = Doc->GetElementById("chrome");
	if (chrome != nullptr && Anchored) {
		chrome->RemoveProperty(Rml::PropertyId::Position);
		chrome->RemoveProperty(Rml::PropertyId::Width);
		chrome->RemoveProperty(Rml::PropertyId::Left);
		chrome->RemoveProperty(Rml::PropertyId::MarginLeft);
		Anchored = false;
	}

	Doc->SetClass("revealing", false);
	reveal->RemoveProperty(Rml::PropertyId::Width);
}


bool UIRmlViewClass::Sounds_A_Click(Rml::Element const * element)
{
	if (element == nullptr) {
		return(false);
	}

	Rml::String const & tag = element->GetTagName();
	if (tag == "input") {
		Rml::String type = element->GetAttribute<Rml::String>("type", "text");
		return(type == "checkbox" || type == "radio" || type == "range" || type == "button" || type == "submit");
	}

	return(tag == "button" || tag == "select" || tag == "dataselect" || tag == "option" || element->IsClassSet("item"));
}


bool UIRmlViewClass::Takes_Enter(Rml::Element const * element)
{
	if (element == nullptr) {
		return(false);
	}

	Rml::String const & tag = element->GetTagName();
	if (tag == "input") {
		Rml::String type = element->GetAttribute<Rml::String>("type", "text");
		return(type == "text" || type == "password");
	}

	return(tag == "textarea");
}


void UIRmlViewClass::ProcessEvent(Rml::Event & event)
{
	if (event.GetId() == Rml::EventId::Mousedown) {
		if (Shell == nullptr || event.GetParameter<int>("button", 0) != 0) {
			return;
		}
		for (Rml::Element * element = event.GetTargetElement(); element != nullptr && element != Doc; element = element->GetParentNode()) {
			if (element->IsClassSet("disabled") || element->HasAttribute("disabled")) {
				return;
			}
			if (Sounds_A_Click(element)) {
				Shell->Play_Click();
				return;
			}
		}
		return;
	}

	if (event.GetId() != Rml::EventId::Keydown) {
		return;
	}

	int key = event.GetParameter<int>("key_identifier", 0);

	if (key == Rml::Input::KI_RETURN || key == Rml::Input::KI_NUMPADENTER) {
		if (Takes_Enter(event.GetTargetElement())) {
			return;
		}
		Queue("ok");
		event.StopPropagation();
	} else if (key == Rml::Input::KI_ESCAPE) {
		Queue("cancel");
		event.StopPropagation();
	}
}


void UIRmlViewClass::Queue(char const * name, int value, char const * text)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	if (text != nullptr) {
		intent.Text = text;
	}
	Owner.Queue(intent);
}
