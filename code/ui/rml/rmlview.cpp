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
			// One argument reaches a screen both ways, since a variant converts either way and a
			// screen knows which of the two its control carries.
			Rml::String const name = arguments[0].Get<Rml::String>();
			Rml::String text;
			int value = 0;
			if (arguments.size() > 1) {
				value = arguments[1].Get<int>();
				text = arguments[1].Get<Rml::String>();
			}

			// A third argument is the text alone, for a control whose reading and whose words
			// are two different things.
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

	// Keys are taken ahead of the target, or a focused text input keeps Enter and Escape.
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


// The element a document puts the whole of its chrome inside when it wants the opening
// reveal. A document without one is shown whole the moment it opens.
static Rml::Element * Reveal_Element(Rml::ElementDocument * document)
{
	return(document != nullptr ? document->GetElementById("reveal") : nullptr);
}


float UIRmlViewClass::Reveal_Width(void) const
{
	// A document can ask to open whole, which is what `ui-files` offers an author.
	if (Doc == nullptr || Doc->GetAttribute<Rml::String>("reveal", "") == "none") {
		return(0.0f);
	}

	Rml::Element * reveal = Reveal_Element(Doc);
	return(reveal != nullptr ? reveal->GetBox().GetSize().x : 0.0f);
}


// The class is what shows the bars that ride the opening edges; the width is what hides
// everything either side of them.
//
// What the band uncovers has to stay where it is on the screen while the band widens around
// it, so the chrome is pinned to the middle of the band at the width the screen was laid out
// at: half the band across, then back by half the screen, which comes to the same place
// whatever the band's width. The width is read on the first pass, before anything is hidden.
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


// A scroll bar's grip is as long as the dialog layer made it: the travel less a fifth of it
// for each natural-log step of the rows left over, never under fourteen, rather than the
// share of the rows in view the toolkit would give it. The rows are fourteen tall and the
// travel is the bar's height less its two arrow boxes. The toolkit sizes a grip only when
// it formats the bar, and reads the height it was given only after its next update, so
// the bar is formatted again each pass until the grip is the size asked for.
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

	// A scroll bar and its parts are no part of the document tree, so the bar is reached
	// through its list and its grip among the children the tree does not count.
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


// The wallpaper belongs to the screen rather than to the dialog: it is one picture centered
// on the frame in whole pixels, and a dialog shows the part of it lying behind. The kit
// hangs it off the middle of the dialog, which is the middle of the frame only while the
// dialog is centered, so a dialog sitting anywhere else pushes it back by however far it is
// off center, and a dialog of odd height by the half pixel its middle is off the grid. The
// picture is sized in dp, so it is placed in dp. A document without the kit's chrome has
// no picture to place.
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


// A text field answers Enter with a change event that carries a line break, which is how a
// document sends what was typed.
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
