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

// windowsx.h, which win.h brings in, names two window walkers the way RmlUi names its
// element walkers.
#undef GetFirstChild
#undef GetNextSibling

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/ID.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/Property.h>
#include <RmlUi/Core/Variant.h>


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


// The model has to exist before the document that names it loads.
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
			Queue(arguments[0].Get<Rml::String>().c_str(), arguments.size() > 1 ? arguments[1].Get<int>() : 0);
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

	Doc->AddEventListener(Rml::EventId::Keydown, this);
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
		Doc->RemoveEventListener(Rml::EventId::Keydown, this);
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
	Model = Rml::DataModelHandle();
	Types.reset();
}


bool UIRmlViewClass::Is_Shown(void) const
{
	return(Doc != nullptr && Doc->IsVisible());
}


// The element a document puts the whole of its chrome inside when it wants the opening
// reveal. A document without one is shown whole the moment it opens.
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


// The class is what shows the bars that ride the opening edges; the width is what hides
// everything either side of them.
void UIRmlViewClass::Reveal_To(float width)
{
	Rml::Element * reveal = Reveal_Element(Doc);
	if (reveal == nullptr) {
		return;
	}

	Doc->SetClass("revealing", true);
	reveal->SetProperty(Rml::PropertyId::Width, Rml::Property(width < 0.0f ? 0.0f : width, Rml::Unit::PX));
}


void UIRmlViewClass::Reveal_Done(void)
{
	Rml::Element * reveal = Reveal_Element(Doc);
	if (reveal == nullptr) {
		return;
	}

	Doc->SetClass("revealing", false);
	reveal->RemoveProperty(Rml::PropertyId::Width);
}


// A control the dialog layer drew sounds a click as it goes down. The list is the controls
// that layer sounded: buttons, check boxes, list rows, the combo and its items, and the
// track bar. A text field is not among them, because the dialog layer left one to the real
// Win32 control underneath it.
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


// Enter accepts and Escape cancels whichever element has focus, as the dialog keys do, and
// pressing a control sounds the click the dialog layer sounded.
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
		Queue("ok");
		event.StopPropagation();
	} else if (key == Rml::Input::KI_ESCAPE) {
		Queue("cancel");
		event.StopPropagation();
	}
}


void UIRmlViewClass::Queue(char const * name, int value)
{
	UIIntent intent;
	intent.Name = name;
	intent.Value = value;
	Owner.Queue(intent);
}
