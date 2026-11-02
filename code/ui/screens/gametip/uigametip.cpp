/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/gametip/uigametip.h"

#include "gametext.h"
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
/// Gives a tooltip's text as Game_Text_Lines does, with a carriage return, or a carriage
/// return and line feed together, also written as one newline.
/// </summary>
std::string UI_Game_Tip_Text(std::string const & text)
{
	std::string const escaped = Game_Text_Lines(text);
	std::string lines;
	lines.reserve(escaped.size());

	for (std::size_t index = 0; index < escaped.size(); index++) {
		char const c = escaped[index];
		if (c == '\r' && index + 1 < escaped.size() && escaped[index + 1] == '\n') {
			continue;
		}
		lines.push_back((c == '\r') ? '\n' : c);
	}

	return(lines);
}


/// <summary>
/// Places a tooltip box inside the placement's edges. Beside a control, the box sits against
/// the control's side that faces the middle of the edges. It is centered on the control's
/// height, or level with the pointer on an upright control, and never starts above the
/// control's top. Otherwise it sits below the pointer as the game placed it: moved left to
/// fit, and above the pointer where it would cross the bottom edge. Either way the box then
/// moves to fit the edges, and one taller than the space is held at the top edge.
/// </summary>
void UI_Place_Game_Tip(UIGameTipPlacement const & placement, float width, float height, float & x, float & y)
{
	if (placement.Beside) {
		bool const left = placement.ControlLeft + placement.ControlRight > placement.Left + placement.Right;
		x = left ? placement.ControlLeft - width : placement.ControlRight;

		float const low = placement.ControlTop + height * 0.5f;
		float const high = placement.ControlBottom - height * 0.5f;
		float const middle = (placement.Upright && low <= high)
			? std::clamp(placement.PointerY, low, high)
			: (placement.ControlTop + placement.ControlBottom) * 0.5f;
		y = std::max(middle - height * 0.5f, placement.ControlTop);
		if (y + height > placement.Bottom) {
			y = placement.Bottom - height;
		}
	} else {
		x = placement.PointerX;
		y = placement.PointerY + placement.Below;
		if (y + height > placement.Bottom) {
			y = placement.PointerY - height;
		}
	}

	if (x + width > placement.Right) {
		x = placement.Right - width;
	}
	if (x < placement.Left) {
		x = placement.Left;
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
};

}


std::unique_ptr<UIViewClass> UI_Game_Tip_View(UIGameTipPresenterClass & presenter)
{
	return(std::make_unique<UIGameTipViewClass>(presenter));
}
