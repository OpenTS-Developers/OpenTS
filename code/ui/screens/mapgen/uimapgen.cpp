/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/mapgen/uimapgen.h"

#include "ui/rml/rmlsurface.h"
#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <string>


UIMapGenPresenterClass::UIMapGenPresenterClass(UIMapGenServiceClass & service) :
	Service(service)
{
	Service.Read(State);
}


void UIMapGenPresenterClass::Execute(UIIntent const & intent)
{
	if (intent.Name == "ok") {
		Choice = UI_MAPGEN_ACCEPT;
		Result = UI_RESULT_ACCEPTED;

	} else if (intent.Name == "cancel") {
		Choice = UI_MAPGEN_CANCEL;
		Result = UI_RESULT_CANCELLED;

	} else if (intent.Name == "preview") {
		if (State.PreviewEnabled) {
			Service.Preview();
		}

	} else if (intent.Name == "surprise") {
		if (State.SurpriseEnabled) {
			Service.Surprise();
		}

	} else if (intent.Name == "save") {
		Service.Save();

	} else if (intent.Name == "load") {
		if (State.LoadEnabled) {
			Service.Load();
		}

	} else if (intent.Name == "delete") {
		if (State.DeleteEnabled) {
			Service.Delete();
		}

	} else {
		// Every other intent names a setting, which the generator holds to its own bounds, so
		// the model is read back rather than assumed.
		Service.Set(intent.Name.c_str(), intent.Value);
		Service.Read(State);
	}
}


void UIMapGenPresenterClass::Refresh(void)
{
	Service.Read(State);
}


namespace
{

class UIMapGenViewClass : public UIRmlViewClass
{
	public:
		explicit UIMapGenViewClass(UIMapGenPresenterClass & presenter) :
			UIRmlViewClass(presenter, "mapgen.rml", "mapgen"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			for (char const * name : {"environment", "time", "width", "height",
					"players", "cliffs", "accessibility", "hills", "tiberiumamount",
					"tiberiumfields", "water", "vegetation", "cities", "veinholes",
					"ionstorms", "transitions", "lifeforms",
					"playerson", "cliffson", "accessibilityon", "hillson", "tiberiumamounton",
					"tiberiumfieldson", "wateron", "vegetationon", "citieson", "veinholeson",
					"environmenton", "timeon", "widthon", "heighton",
					"ionstormson", "transitionson", "lifeformson", "surpriseon",
					"loadenabled", "deleteenabled", "previewenabled"}) {
				Model.DirtyVariable(name);
			}

			// The picture is bytes, so it is handed over rather than pushed through the
			// model, and only when the generator has drawn another.
			Show_Preview();
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			Rml::StructHandle<UIMapGenOption> option = model.RegisterStruct<UIMapGenOption>();
			if (!option) {
				return(false);
			}
			option.RegisterMember("label", &UIMapGenOption::Label);

			UIMapGenState & state = Data.State;
			return(model.RegisterArray<std::vector<UIMapGenOption>>()
				&& model.Bind("firestorm", &state.Firestorm)
				&& model.Bind("territory", &state.Territory)
				&& model.Bind("environments", &state.Environments)
				&& model.Bind("times", &state.Times)
				&& model.Bind("sizes", &state.Sizes)
				&& model.Bind("environment", &state.Environment)
				&& model.Bind("time", &state.Time)
				&& model.Bind("width", &state.Width)
				&& model.Bind("height", &state.Height)
				&& Bind_Slider(model, "players", state.Players)
				&& Bind_Slider(model, "cliffs", state.Cliffs)
				&& Bind_Slider(model, "accessibility", state.Accessibility)
				&& Bind_Slider(model, "hills", state.Hills)
				&& Bind_Slider(model, "tiberiumamount", state.TiberiumAmount)
				&& Bind_Slider(model, "tiberiumfields", state.TiberiumFields)
				&& Bind_Slider(model, "water", state.Water)
				&& Bind_Slider(model, "vegetation", state.Vegetation)
				&& Bind_Slider(model, "cities", state.Cities)
				&& Bind_Slider(model, "veinholes", state.Veinholes)
				&& model.Bind("ionstorms", &state.IonStorms)
				&& model.Bind("transitions", &state.Transitions)
				&& model.Bind("lifeforms", &state.Lifeforms)
				&& model.Bind("environmenton", &state.EnvironmentEnabled)
				&& model.Bind("timeon", &state.TimeEnabled)
				&& model.Bind("widthon", &state.WidthEnabled)
				&& model.Bind("heighton", &state.HeightEnabled)
				&& model.Bind("ionstormson", &state.IonStormsEnabled)
				&& model.Bind("transitionson", &state.TransitionsEnabled)
				&& model.Bind("lifeformson", &state.LifeformsEnabled)
				&& model.Bind("surpriseon", &state.SurpriseEnabled)
				&& model.Bind("loadenabled", &state.LoadEnabled)
				&& model.Bind("deleteenabled", &state.DeleteEnabled)
				&& model.Bind("previewenabled", &state.PreviewEnabled));
		}

		virtual void Loaded(void) override
		{
			Document()->SetClass(Data.State.Firestorm ? "firestorm" : "original", true);
			Document()->SetClass("territory", Data.State.Territory);
			Shown = -1;
		}

	private:
		bool Bind_Slider(Rml::DataModelConstructor & model, char const * name, UIMapGenSlider & slider)
		{
			std::string base(name);
			return(model.Bind(base, &slider.Value)
				&& model.Bind(base + "min", &slider.Minimum)
				&& model.Bind(base + "max", &slider.Maximum)
				&& model.Bind(base + "on", &slider.Enabled));
		}

		void Show_Preview(void)
		{
			UIMapPreviewImage & preview = Data.State.Preview;
			if (preview.Generation == Shown || Document() == nullptr) {
				return;
			}

			UIRmlSurfaceElementClass * surface = rmlui_dynamic_cast<UIRmlSurfaceElementClass *>(Document()->GetElementById("preview"));
			if (surface == nullptr) {
				return;
			}

			Shown = preview.Generation;
			surface->Set_Image(preview.Width, preview.Height, preview.Pixels);
		}

		UIMapGenPresenterClass & Data;
		int Shown = -1;
};

}


std::unique_ptr<UIViewClass> UI_Map_Generator_View(UIMapGenPresenterClass & presenter)
{
	return(std::make_unique<UIMapGenViewClass>(presenter));
}
