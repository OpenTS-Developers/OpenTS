/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "ui/screens/restate/uirestate.h"

#include "gametext.h"
#include "ui/rml/rmlview.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementUtilities.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/Input.h>
#include <cmath>
#include <utility>


namespace
{

// The old screen's text box in game pixels, less one font cell of width.
const int PAGE_WIDTH = 400;
const int PAGE_LINES = 14;
const int LINE_HEIGHT = 20;
const int BOX_HEIGHT = 280;

const float FRAME_WIDTH = 640.0f;
const float FRAME_HEIGHT = 400.0f;


// The old font's space was an en space wide, and RmlUi has no word spacing.
std::string Widen_Spaces(std::string const & text)
{
	std::string wide;
	wide.reserve(text.size());
	for (char c : text) {
		if (c == ' ') {
			wide += "\xE2\x80\x82";
		} else {
			wide += c;
		}
	}
	return(wide);
}


bool Is_Lead_Byte(char c)
{
	return(((unsigned char)c & 0xC0) != 0x80);
}


// The old wrap kept a word on its line when only its last letter crossed the limit.
std::string Without_Last_Character(std::string const & text)
{
	std::size_t end = text.size();
	while (end > 0 && !Is_Lead_Byte(text[end - 1])) {
		end--;
	}
	return(text.substr(0, (end > 0) ? end - 1 : 0));
}


std::size_t Fitting_Prefix(std::string const & word, std::function<int(std::string const &)> const & width, int line_width)
{
	std::size_t cut = 0;
	std::size_t next = 0;
	while (next < word.size()) {
		next++;
		while (next < word.size() && !Is_Lead_Byte(word[next])) {
			next++;
		}
		if (cut > 0 && width(word.substr(0, next)) >= line_width) {
			break;
		}
		cut = next;
	}
	return(cut);
}


void Wrap_Paragraph(std::string const & paragraph, std::function<int(std::string const &)> const & width, int line_width, std::vector<std::string> & lines)
{
	std::string line;
	std::size_t index = 0;
	std::size_t const length = paragraph.size();

	while (index < length) {
		std::size_t const gap = index;
		while (index < length && paragraph[index] == ' ') {
			index++;
		}
		std::string const spaces = paragraph.substr(gap, index - gap);

		std::size_t const start = index;
		while (index < length && paragraph[index] != ' ') {
			index++;
		}
		std::string word = paragraph.substr(start, index - start);
		if (word.empty()) {
			break;
		}

		std::string const candidate = line.empty() ? word : line + spaces + word;
		if (width(Without_Last_Character(candidate)) < line_width) {
			line = candidate;
			continue;
		}

		if (!line.empty()) {
			lines.push_back(line);
		}

		while (width(word) >= line_width) {
			std::size_t const cut = Fitting_Prefix(word, width, line_width);
			if (cut >= word.size()) {
				break;
			}
			lines.push_back(word.substr(0, cut));
			word.erase(0, cut);
		}
		line = word;
	}

	lines.push_back(line);
}

}


UIRestatePresenterClass::UIRestatePresenterClass(UIRestateServiceClass & service, UIClockClass & clock, std::string text, bool video) :
	Video(video),
	Service(service),
	Clock(clock),
	Text(std::move(text))
{
}


// A line break drops the spaces after it; only an overlong word splits.
void UIRestatePresenterClass::Lay_Out(std::function<int(std::string const &)> const & width, int line_width, int page_lines)
{
	std::string const text = Game_Text_Lines(Text);
	std::vector<std::string> lines;
	std::string paragraph;
	std::size_t index = 0;

	while (index < text.size()) {
		char const c = text[index++];
		if (c == '\n') {
			Wrap_Paragraph(paragraph, width, line_width, lines);
			paragraph.clear();
			while (index < text.size() && text[index] == ' ') {
				index++;
			}
			continue;
		}
		paragraph += c;
	}

	if (!paragraph.empty()) {
		Wrap_Paragraph(paragraph, width, line_width, lines);
	}

	if (page_lines < 1) {
		page_lines = 1;
	}

	Pages.clear();
	for (std::size_t first = 0; first < lines.size(); first += page_lines) {
		std::size_t const last = (first + page_lines < lines.size()) ? first + page_lines : lines.size();
		Pages.emplace_back(lines.begin() + first, lines.begin() + last);
	}

	LaidOut = true;
	if (Pages.empty()) {
		Typing = false;
		Done = true;
		Generation++;
		return;
	}

	Start_Page(0);
}


void UIRestatePresenterClass::Execute(UIIntent const & intent)
{
	bool const advance = (intent.Name == "next" || intent.Name == "skip" || intent.Name == "ok" || intent.Name == "cancel");

	if (Typing) {
		if (advance) {
			Finish_Page();
		}

	} else if (More) {
		if (advance || intent.Name == "more") {
			Start_Page(Page + 1);
		}

	} else if (Done) {
		if (advance || intent.Name == "resume") {
			Result = UI_RESULT_ACCEPTED;
		} else if (intent.Name == "video" && Video) {
			ChoseVideo = true;
			Result = UI_RESULT_ACCEPTED;
		}
	}
}


// A late pass takes one step, so a stalled screen slows the fade instead of skipping it.
void UIRestatePresenterClass::Refresh(void)
{
	if (!Typing) {
		return;
	}

	int const now = Clock.Milliseconds();
	if (!Due.has_value()) {
		Due = now + START_DELAY;
		return;
	}

	if (now - *Due < 0) {
		return;
	}

	Due = now + STEP_DELAY;
	Step();
}


std::vector<std::string> const & UIRestatePresenterClass::Lines(void) const
{
	static std::vector<std::string> const _none;
	return(Pages.empty() ? _none : Pages[Page]);
}


int UIRestatePresenterClass::Frame_Of(int line) const
{
	if (line < 0 || line >= (int)Lines().size()) {
		return(-1);
	}

	if (!Typing) {
		return(2);
	}

	if (Steps == 0) {
		return(-1);
	}

	int const last = Steps - 1;
	int const reached = last / 3;
	if (line < reached) {
		return(2);
	}
	return((line == reached) ? last % 3 : -1);
}


void UIRestatePresenterClass::Start_Page(int page)
{
	Page = page;
	Steps = 0;
	Due.reset();
	Typing = true;
	More = false;
	Done = false;
	Generation++;
}


// Each line takes three steps and bleeps as it settles; the last bleeps a step later.
void UIRestatePresenterClass::Step(void)
{
	int const step = Steps++;
	int const count = (int)Lines().size();

	if (step >= count * 3) {
		Finish_Page();
		return;
	}

	if (step % 3 == 2 && step / 3 < count - 1) {
		Service.Bleep();
	}
}


void UIRestatePresenterClass::Finish_Page(void)
{
	Steps = (int)Lines().size() * 3 + 1;
	Typing = false;
	if (Page + 1 < (int)Pages.size()) {
		More = true;
	} else {
		Done = true;
	}
	Service.Bleep();
}


namespace
{

class UIRestateViewClass : public UIRmlViewClass
{
	public:
		explicit UIRestateViewClass(UIRestatePresenterClass & presenter) :
			UIRmlViewClass(presenter, "restate.rml", "restate"),
			Data(presenter)
		{
		}

		virtual void Sync(void) override
		{
			Model.DirtyAllVariables();
			Build_Page();
			Show_Frames();
		}

		virtual void Placed(void) override
		{
			UIRmlViewClass::Placed();
			Place_Frame();
			if (!Data.LaidOut) {
				Lay_Out();
			}
		}

	protected:
		virtual bool Bind(Rml::DataModelConstructor & model) override
		{
			return(model.Bind("typing", &Data.Typing)
				&& model.Bind("more", &Data.More)
				&& model.Bind("done", &Data.Done)
				&& model.Bind("video", &Data.Video));
		}

		// Space, Enter and Escape advance, but Space and Enter press a visible focused button.
		virtual void ProcessEvent(Rml::Event & event) override
		{
			if (event.GetId() == Rml::EventId::Keydown) {
				int const key = event.GetParameter<int>("key_identifier", 0);
				bool const moves = (key == Rml::Input::KI_SPACE || key == Rml::Input::KI_RETURN || key == Rml::Input::KI_NUMPADENTER || key == Rml::Input::KI_ESCAPE);
				Rml::Element * target = event.GetTargetElement();
				bool const presses = (key != Rml::Input::KI_ESCAPE && target != nullptr && target->GetTagName() == "button" && target->IsVisible());
				if (moves && !presses) {
					Queue("next");
					event.StopPropagation();
					return;
				}
			}
			UIRmlViewClass::ProcessEvent(event);
		}

	private:
		float Ratio(void) const
		{
			Rml::Context * context = (Document() != nullptr) ? Document()->GetContext() : nullptr;
			float ratio = (context != nullptr) ? context->GetDensityIndependentPixelRatio() : 1.0f;
			return((ratio > 0.0f) ? ratio : 1.0f);
		}

		void Lay_Out(void)
		{
			Rml::Element * page = (Document() != nullptr) ? Document()->GetElementById("page") : nullptr;
			if (page == nullptr || page->GetFontFaceHandle() == 0) {
				return;
			}

			Data.Lay_Out([page](std::string const & text) {
				return(Rml::ElementUtilities::GetStringWidth(page, Widen_Spaces(text)));
			}, (int)((float)PAGE_WIDTH * Ratio()), PAGE_LINES);
		}

		// The old screen centered its picture on whole game pixels.
		void Place_Frame(void)
		{
			Rml::Element * frame = (Document() != nullptr) ? Document()->GetElementById("frame") : nullptr;
			if (frame == nullptr) {
				return;
			}

			float const ratio = Ratio();
			Rml::Vector2i const size = Document()->GetContext()->GetDimensions();
			float const left = std::floor(((float)size.x / ratio - FRAME_WIDTH) * 0.5f);
			float const top = std::floor(((float)size.y / ratio - FRAME_HEIGHT) * 0.5f);
			if (left == FrameLeft && top == FrameTop) {
				return;
			}

			FrameLeft = left;
			FrameTop = top;
			frame->SetProperty("left", Rml::ToString(left) + "dp");
			frame->SetProperty("top", Rml::ToString(top) + "dp");
		}

		void Build_Page(void)
		{
			Rml::Element * page = (Document() != nullptr) ? Document()->GetElementById("page") : nullptr;
			if (page == nullptr || Data.Generation == Built) {
				return;
			}
			Built = Data.Generation;

			while (page->GetNumChildren() > 0) {
				page->RemoveChild(page->GetChild(0));
			}

			std::vector<std::string> const & lines = Data.Lines();
			for (std::string const & text : lines) {
				Rml::ElementPtr line = Document()->CreateElement("p");
				line->SetClass("line", true);
				if (!text.empty()) {
					line->AppendChild(Document()->CreateTextNode(Widen_Spaces(text)));
				}
				page->AppendChild(std::move(line));
			}
			Shown.assign(lines.size(), -2);

			int const top = (Data.Pages.size() == 1) ? (BOX_HEIGHT - LINE_HEIGHT * (int)lines.size()) / 2 : 0;
			page->SetProperty("margin-top", Rml::ToString(top) + "dp");
		}

		void Show_Frames(void)
		{
			Rml::Element * page = (Document() != nullptr) ? Document()->GetElementById("page") : nullptr;
			if (page == nullptr) {
				return;
			}

			int const count = (page->GetNumChildren() < (int)Shown.size()) ? page->GetNumChildren() : (int)Shown.size();
			for (int index = 0; index < count; index++) {
				int const frame = Data.Frame_Of(index);
				if (frame == Shown[index]) {
					continue;
				}
				Shown[index] = frame;

				Rml::Element * line = page->GetChild(index);
				line->SetClass("blob", frame == 0);
				line->SetClass("fade", frame == 1);
				line->SetClass("ink", frame == 2);
			}
		}

		UIRestatePresenterClass & Data;
		int Built = -1;
		std::vector<int> Shown;
		float FrameLeft = -1.0e9f;
		float FrameTop = -1.0e9f;
};

}


std::unique_ptr<UIViewClass> UI_Restate_View(UIRestatePresenterClass & presenter)
{
	return(std::make_unique<UIRestateViewClass>(presenter));
}
