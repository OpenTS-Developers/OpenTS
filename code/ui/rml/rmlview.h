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
#include "ui/uiview.h"

// windowsx.h, which win.h brings in, names two window walkers the way RmlUi names its
// element walkers.
#ifdef GetFirstChild
#undef GetFirstChild
#undef GetNextSibling
#endif

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/DataTypeRegister.h>
#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>

namespace Rml
{
	class Context;
	class ElementDocument;
	class Event;
}


// One document and its data model over a presenter. Prepare loads it against a context, the
// runner shows, drives and releases it, and the presenter outlives it.
class UIRmlViewClass : public Rml::EventListener, public UIViewClass
{
	public:
		UIRmlViewClass(UIPresenterClass & presenter, char const * document, char const * model);
		virtual ~UIRmlViewClass(void);

		// Creates and binds the model, then loads the document. False leaves nothing behind and
		// names the failing resource in the RmlUi log. The shell form loads against its context.
		bool Prepare(Rml::Context & context);
		virtual bool Prepare(UIShellClass & shell) override;
		virtual void Show(bool modal) override;
		virtual void Hide(void) override;
		// Detaches the listener, removes the model and unloads the document while the
		// presenter's storage still lives; the context frees the document on its next update.
		virtual void Release(void) override;

		virtual UIPresenterClass & Presenter(void) const override { return(Owner); }
		Rml::ElementDocument * Document(void) const { return(Doc); }
		char const * Document_Name(void) const { return(DocumentName.c_str()); }
		virtual char const * Name(void) const override { return(DocumentName.c_str()); }
		virtual bool Is_Shown(void) const override;

		virtual float Reveal_Width(void) const override;
		virtual void Reveal_To(float width) override;
		virtual void Reveal_Done(void) override;
		virtual void Placed(void) override;

		// Marks the view-model fields that Execute changed.
		virtual void Sync(void) override = 0;

	protected:
		// Binds the view-model fields; the base binds the queue event.
		virtual bool Bind(Rml::DataModelConstructor & model) = 0;
		// Runs once the document has loaded, for the listeners a view puts on its elements.
		virtual void Loaded(void) {}
		virtual void ProcessEvent(Rml::Event & event) override;
		void Queue(char const * name, int value = 0);

		Rml::DataModelHandle Model;

	private:
		// Whether pressing this element is what the dialogs sounded a click for.
		static bool Sounds_A_Click(Rml::Element const * element);
		void Place_Wallpaper(void);
		void Size_Grips(void);

		UIPresenterClass & Owner;
		Rml::Context * Host = nullptr;
		// The shell the screen is running under, which owns the sound; a view prepared
		// against a bare context has none and stays silent.
		UIShellClass * Shell = nullptr;
		Rml::ElementDocument * Doc = nullptr;
		// The context's shared register refuses a type declared twice, so each view brings its own
		// and a screen can open again.
		Rml::UniquePtr<Rml::DataTypeRegister> Types;
		bool ModelCreated = false;
		// Where the wallpaper was last put, so it is only moved when the screen has moved.
		float Wallpaper = 1.0e9f;
		// Whether the chrome is pinned to the middle of the band a screen opens through.
		bool Anchored = false;
		Rml::String DocumentName;
		Rml::String ModelName;
};
