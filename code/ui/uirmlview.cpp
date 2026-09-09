/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/uirmlview.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/ID.h>
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Log.h>
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
	Loaded();
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
	ModelCreated = false;
	Model = Rml::DataModelHandle();
	Types.reset();
}


bool UIRmlViewClass::Is_Shown(void) const
{
	return(Doc != nullptr && Doc->IsVisible());
}


// Enter accepts and Escape cancels whichever element has focus, as the dialog keys do.
void UIRmlViewClass::ProcessEvent(Rml::Event & event)
{
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
