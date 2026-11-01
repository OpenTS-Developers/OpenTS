/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/gametip/uigametip.h"

#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <algorithm>
#include <cmath>


static char const * const UI_GAME_TIP_ELEMENT = "gametip";


void UIGameTipPresenterClass::Execute(UIIntent const &)
{
}


void UIGameTipPresenterClass::Refresh(void)
{
}


/// <summary>
/// Gives a tooltip's text with every line break the game prints written as a newline: an at
/// sign, a carriage return, or a carriage return and line feed together.
/// </summary>
std::string UI_Game_Tip_Text(std::string const & text)
{
	std::string lines;
	lines.reserve(text.size());

	for (std::size_t index = 0; index < text.size(); index++) {
		char const c = text[index];
		if (c == '\r' && index + 1 < text.size() && text[index + 1] == '\n') {
			continue;
		}
		lines.push_back((c == '@' || c == '\r') ? '\n' : c);
	}

	return(lines);
}


/// <summary>
/// Places a tooltip box the way the game placed it: below the pointer, moved left to keep
/// it inside its region's right edge, and moved above the pointer where it would cross the
/// region's bottom edge. A box taller than the space above the pointer is held at the
/// region's top edge.
/// </summary>
void UI_Place_Game_Tip(UIGameTipPlacement const & placement, float width, float height, float & x, float & y)
{
	x = placement.PointerX;
	if (x + width > placement.Right) {
		x = placement.Right - width;
	}

	y = placement.PointerY + placement.Below;
	if (y + height > placement.Bottom) {
		y = placement.PointerY - height;
	}
	if (y < placement.Top) {
		y = placement.Top;
	}
}


namespace
{

class UIGameTipViewClass : public UIRmlViewClass
{
	public:
		explicit UIGameTipViewClass(UIGameTipPresenterClass & presenter) :
			UIRmlViewClass(presenter, "gametip.rml", "gametip"),
			Data(presenter)
		{
		}

		// The document stays hidden until a tooltip has been laid out and placed.
		virtual void Show(bool) override
		{
		}

		virtual void Sync(void) override
		{
			Rml::ElementDocument * document = Document();
			Rml::Element * box = (document != nullptr) ? document->GetElementById(UI_GAME_TIP_ELEMENT) : nullptr;
			if (box == nullptr) {
				return;
			}

			if (!Data.Wanted) {
				document->Hide();
				return;
			}

			std::string const text = UI_Game_Tip_Text(Data.Text);
			if (text != Shown) {
				while (box->HasChildNodes()) {
					box->RemoveChild(box->GetChild(0));
				}
				box->AppendChild(document->CreateTextNode(text));
				Shown = text;
			}

			float const width = std::max(Data.Placement.Right - Data.Placement.Left, 0.0f);
			if (width != Width) {
				box->SetProperty(Rml::PropertyId::MaxWidth, Rml::Property(width, Rml::Unit::PX));
				Width = width;
			}
		}

		virtual void Placed(void) override
		{
			UIRmlViewClass::Placed();

			Rml::ElementDocument * document = Document();
			Rml::Element * box = (document != nullptr) ? document->GetElementById(UI_GAME_TIP_ELEMENT) : nullptr;
			if (box == nullptr || !Data.Wanted) {
				return;
			}

			Rml::Vector2f const size = box->GetBox().GetSize(Rml::BoxArea::Border);
			float x = 0.0f;
			float y = 0.0f;
			UI_Place_Game_Tip(Data.Placement, size.x, size.y, x, y);

			// Whole pixels keep the font's pixels on the screen's.
			box->SetProperty(Rml::PropertyId::Left, Rml::Property(std::floor(x), Rml::Unit::PX));
			box->SetProperty(Rml::PropertyId::Top, Rml::Property(std::floor(y), Rml::Unit::PX));

			if (!document->IsVisible()) {
				document->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
			}
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor &) override
		{
			return(true);
		}

	private:
		UIGameTipPresenterClass & Data;
		std::string Shown;
		float Width = -1.0f;
};

}


std::unique_ptr<UIViewClass> UI_Game_Tip_View(UIGameTipPresenterClass & presenter)
{
	return(std::make_unique<UIGameTipViewClass>(presenter));
}
