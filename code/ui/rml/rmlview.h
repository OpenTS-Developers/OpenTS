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
		UIPresenterClass & Owner;
		Rml::Context * Host = nullptr;
		Rml::ElementDocument * Doc = nullptr;
		// The context's shared register refuses a type declared twice, so each view brings its own
		// and a screen can open again.
		Rml::UniquePtr<Rml::DataTypeRegister> Types;
		bool ModelCreated = false;
		Rml::String DocumentName;
		Rml::String ModelName;
};
