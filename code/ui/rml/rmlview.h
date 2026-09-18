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


class UIRmlViewClass : public Rml::EventListener, public UIViewClass
{
	public:
		UIRmlViewClass(UIPresenterClass & presenter, char const * document, char const * model);
		virtual ~UIRmlViewClass(void);

		bool Prepare(Rml::Context & context);
		virtual bool Prepare(UIShellClass & shell) override;
		virtual void Show(bool modal) override;
		virtual void Hide(void) override;
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
		virtual void Sync(void) override = 0;

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) = 0;
		virtual void Loaded(void) {}
		virtual void ProcessEvent(Rml::Event & event) override;
		void Queue(char const * name, int value = 0, char const * text = nullptr);

		Rml::DataModelHandle Model;

	private:
		static bool Sounds_A_Click(Rml::Element const * element);
		static bool Takes_Enter(Rml::Element const * element);
		void Place_Wallpaper(void);
		void Size_Grips(void);

		UIPresenterClass & Owner;
		Rml::Context * Host = nullptr;
		UIShellClass * Shell = nullptr;
		Rml::ElementDocument * Doc = nullptr;
		Rml::UniquePtr<Rml::DataTypeRegister> Types;
		bool ModelCreated = false;
		float Wallpaper = 1.0e9f;
		bool Anchored = false;
		Rml::String DocumentName;
		Rml::String ModelName;
};
