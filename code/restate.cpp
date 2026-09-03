/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2025 Electronic Arts Inc.
 * Copyright 2026 OpenTS contributors
 *
 * Contains material derived from Electronic Arts source code.
 * Modified by OpenTS contributors, 2026.
 * EA's GPLv3 Section 7 additional terms and supplemental warranty
 * disclaimers apply; see LICENSE.md.
 ******************************************************************************/

#include "always.h"

#include "_keyboar.h"
#include "_palette.h"
#include "_rules.h"
#include "_surface.h"
#include "_xmouse.h"
#include "addon.h"
#include "ccfile.h"
#include "convert.h"
#include "dbgprint.h"
#include "globals.h"
#include "keyboard.h"
#include "language/language.h"
#include "movie.h"
#include "msanim.h"
#include "msengine.h"
#include "msfont.h"
#include "ownrdraw.h"
#include "rules.h"
#include "scenario.h"
#include "srfcache.h"
#include "surface.h"
#include "textbtn.h"
#include "theme.h"
#include "voc.h"
#include "winstub.h"

#include "dialog.hh"

#include <algorithm>

class MyButton : public TextButtonClass {
	public:
		MyButton(MSEngine *engine, unsigned id, int text, TextPrintType style, int x, int y, int w=-1, int h=-1, bool black_border=false, bool no_background=false) :
			TextButtonClass(id, text, style, x, y, w, h, black_border, no_background),
			Engine(engine)
		{
		}
		virtual ~MyButton(void) {}

		virtual int Draw_Me(int forced=false)
		{
			if (!IsDisabled) {
				Draw_Background();
				Draw_Text(String);
				if (IsPressed && !WasPressed) {
					Sound_Effect(Rule->GenericClick);
				}
				WasPressed = IsPressed;
				Rect r(X, Y, Width + 1, Height + 1);
				Engine->Add_Update_Rect(r);
				return(true);
			}
			return(false);
		}

		/// <summary>
		/// Restores the screen behind the button.
		/// This routine paints the untouched backdrop back over the button and marks the area
		/// for update. It is how the "more" button vanishes once the player is done with it.
		/// </summary>
		void Draw(void)
		{
			Rect rect(X, Y, Width + 1, Height + 1);
			HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
			Engine->Add_Update_Rect(rect);
		}

	protected:
		/// <summary>
		/// Draws the face of the button.
		/// This routine builds the button face out of the left cap, stretched middle, and right
		/// cap images kept in the surface cache, picking the artwork that suits the height of the
		/// button and whether it is currently pressed.
		/// </summary>
		virtual void Draw_Background(void)
		{
			Rect origin;
			origin.X = X;
			origin.Y = Y;
			origin.Width = Width;
			Rect dest_rect;
			Rect source_rect;

			unsigned int size_index = 0;
			int small_widths[2] = {7, 7};
			int widths[2] = {10, 10};

			origin.Height = Height;
			int heights[2] = {24, 30};

			for (unsigned int i = 0; i < 2; i++) {
				if (heights[i] > origin.Height && i != 0) {
					break;
				}
				size_index = i;
			}

			int height = heights[size_index];
			int small_width = small_widths[size_index];
			int width = widths[size_index];
			origin.Y += (origin.Height - height) / 2;
			Rect rect;
			char buffer[40];

			sprintf(buffer, "b%ce_li%d.pcx", IsPressed != false ? 'd' : 'u', height);
			Surface * image = SurfaceCache.GetSurface(buffer);
			origin.Height = image->Get_Height();
			dest_rect = origin;
			dest_rect.Width = small_width;
			source_rect.Width = small_width;
			source_rect.Y = 0;
			source_rect.X = 0;
			dest_rect.Height = height;
			source_rect.Height = height;
			HiddenSurface->Blit_From(dest_rect, *image, source_rect);

			sprintf(buffer, "b%ce_mi%d.pcx", IsPressed != false ? 'd' : 'u', height);
			image = SurfaceCache.GetSurface(buffer);
			rect = origin;
			rect.X += small_width;
			rect.Width -= width;
			rect.Height = image->Get_Height();
			SurfaceCache.Draw(rect, *HiddenSurface, *image, 0, 0);

			sprintf(buffer, "b%ce_ri%d.pcx", IsPressed != false ? 'd' : 'u', height);
			image = SurfaceCache.GetSurface(buffer);
			dest_rect = origin;
			dest_rect.X += origin.Width - width;
			dest_rect.Width = width;
			dest_rect.Height = image->Get_Height();
			source_rect.Height = dest_rect.Height;
			source_rect.Width = dest_rect.Width;
			source_rect.Y = 0;
			source_rect.X = 0;
			HiddenSurface->Blit_From(dest_rect, *image, source_rect);
		}

		/// <summary>
		/// Draws the label of the button.
		/// This routine prints the label using the dialog system's remapped text colors,
		/// nudging it down and to the right while the button is held down.
		/// </summary>
		virtual void Draw_Text(char const * text)
		{
			Rect rect(X, Y, X + Width, Y + Height - 2);
			if (IsPressed) {
				rect.X += 2;
				rect.Y += 4;
			}
			OD_Draw_Text_Remap(*HiddenSurface, text, rect, "dlgsys", ODColorText, 5, 0);
		}


	private:
		bool WasPressed;
		MSEngine *Engine;
};

/// The binary confirms this was in the cpp as the vtable is inside this module
/// Moving it into a header would make the first thing that includes the header construct the vtable in that module

class RestateMission : public MSEngine {
	public:
		RestateMission(void);
		virtual ~RestateMission(void);
		bool Presentation(ScenarioClass * scen);

		virtual void Do_Custom_Draw(Surface *surface);

	private:
		bool Init(ScenarioClass * scen);
		void Cleanup(void);

		bool User_Input(void);
		void More_Button(int x, int y);
		MyButton * Get_Button(unsigned int id);

		ScenarioClass *Scenario;
		char BriefingText[1024];
		char * String;
		Rect StringRect;
		int CenterX;
		int CenterY;
		MSFont * Font;
		ConvertClass * Drawer;
		GadgetClass * ButtonList;
		DynamicVectorClass<MyButton *> Buttons;
};

enum {
	BUTTON_RESUME = 1,
	BUTTON_VIDEO = 2,
	BUTTON_MORE = 3,

	BUTTON_COUNT = 3,
};

struct RestateButtonStruct {
	int ID;
	int Text;
	Rect Area;

	RestateButtonStruct(int id, int text, Rect rect) :
		Text(text),
		Area(rect),
		ID(id)
	{
	}
};

RestateButtonStruct _buttons[BUTTON_COUNT] = {
	RestateButtonStruct(BUTTON_RESUME, TXT_RESUME_MISSION, Rect(0, 360, 150, 24)),
	RestateButtonStruct(BUTTON_VIDEO, TXT_VIDEO, Rect(0, 360, 150, 24)),
	RestateButtonStruct(BUTTON_MORE, TXT_MORE, Rect(0, 340, 150, 24)),
};


/***********************************************************************************************
 * Restate_Mission -- Handles restating the mission objective.                                 *
 *                                                                                             *
 *    This routine will display the mission objective (as text). It will also give the         *
 *    option to redisplay the mission briefing video.                                          *
 *                                                                                             *
 * INPUT:   name  -- The scenario name. This is the unique identifier for the scenario         *
 *                   briefing text as it appears in the "MISSION.INI" file.                    *
 *                                                                                             *
 * OUTPUT:  Returns the response from the dialog. This will either be 1 if the video was       *
 *          requested, or 0 if the return to game options button was selected.                 *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/23/1995 JLB : Created.                                                                 *
 *   08/06/1995 JLB : Uses preloaded briefing text.                                            *
 *=============================================================================================*/
void Restate_Mission(ScenarioClass * scen)
{
	bool save_started = ScenarioActive;
	ScenarioActive = false;
	if (RestateMission().Presentation(scen) == true) {
		ThemeType theme = Theme.What_Is_Playing();
		Theme.Stop();
		Play_Movie(scen->BriefMovie, THEME_NONE, 1, 1);
		Theme.Play_Song(theme);
	}
	ScenarioActive = save_started;
	Keyboard->Clear();
}


/// <summary>
/// Constructs the mission restatement object.
/// The object holds no scenario, artwork or buttons until Init supplies them.
/// </summary>
RestateMission::RestateMission(void) :
	Scenario(NULL),
	String(NULL),
	Font(NULL),
	Drawer(NULL),
	ButtonList(NULL)
{
	Buttons.Clear();
	BriefingText[0] = '\0';
}


/// <summary>
/// Destroys the mission restatement object.
/// Any artwork and buttons still held are released as the object goes away.
/// </summary>
RestateMission::~RestateMission(void)
{
	Cleanup();
}


/// <summary>
/// Handles the mission restatement presentation.
/// This routine puts up the score backdrop, types the briefing out a page at a time with
/// a "more" button between the pages, and then waits for the player to either resume the
/// mission or ask to see the briefing video again.
/// </summary>
/// <param name="scen">The scenario whose briefing is to be restated.</param>
/// <returns>bool; Did the player ask to see the briefing video?</returns>
bool RestateMission::Presentation(ScenarioClass * scen)
{
	bool result = false;
	if (scen != NULL && AlternateSurface != NULL && HiddenSurface != NULL) {
		if (Init(scen) == true) {
			Keyboard->Clear();
			MouseCursor->Release_Mouse();
			Hide_Mouse();

			Load_Title_Screen("SCORE.PCX", AlternateSurface, &CCPalette);
			HiddenSurface->Blit_From(*AlternateSurface);
			Add_Update_Rect(HiddenSurface->Get_Rect());
			Blit_All(HiddenSurface);

			if (strlen(BriefingText) != 0) {
				Rect rect(CenterX + 110, CenterY + 60, 420, 280);
				MSPrintAnim::Word_Wrap(BriefingText, Font, 420);
				Font->Get_String_Rect(BriefingText, StringRect);
				StringRect.X = rect.X + (rect.Width - StringRect.Width) / 2;
				StringRect.Y = rect.Y + (rect.Height - Font->Get_Font_Height() * (StringRect.Height / Font->Get_Font_Height())) / 2;
				StringRect = Intersect(rect, StringRect);
				MSPrintAnim::Paginate(BriefingText, Font, rect.Height);

				char * token = strtok(BriefingText, "\f");
				if (token != NULL) {
					while (true) {
						MSPrintAnim * anim = new MSWordAnim(token, StringRect.X, StringRect.Y, Font, StringRect, 5);
						Add_Animation(anim);
						Wait_For_Anim(anim);
						String = token;

						token = strtok(NULL, "\f");
						if (token == NULL) {
							break;
						}

						Show_Mouse();
						More_Button(StringRect.X + StringRect.Width / 2, StringRect.Y + StringRect.Height);
						Hide_Mouse();

						HiddenSurface->Blit_From(rect, *AlternateSurface, rect);
						Add_Update_Rect(rect);
					}
				}
			}

			MyButton *resume_button = Get_Button(BUTTON_RESUME);
			if (resume_button != NULL) {
				resume_button->Enable();
			}
			if (scen->BriefMovie != VQ_NONE) {
				MyButton *video_button = Get_Button(BUTTON_VIDEO);
				if (video_button != NULL) {
					video_button->Enable();
				}
			}

			Show_Mouse();
			result = User_Input();
			Hide_Mouse();

			HiddenSurface->Fill(0);
			Add_Update_Rect(HiddenSurface->Get_Rect());
			Blit_All(HiddenSurface);

			Cleanup();
			Keyboard->Clear();

			Show_Mouse();
			MouseCursor->Capture_Mouse();
		}
		return(result);
	}
	return(result);
}


/// <summary>
/// Prepares the mission restatement presentation.
/// This routine fetches the briefing text -- the scenario's own, and the mission INI
/// database's entry for it when the scenario carries none -- creates the font and
/// drawer the animations need, and lays out the buttons the player will be offered.
/// </summary>
/// <param name="scen">The scenario whose briefing is to be restated.</param>
/// <returns>bool; Was the presentation successfully prepared?</returns>
bool RestateMission::Init(ScenarioClass * scen)
{
	CCFileClass file;
	CCINIClass ini;
	char buffer[32];

	Cleanup();

	if (scen == NULL) {
		return(false);
	}

	Scenario = scen;
	CenterX = (HiddenSurface->Get_Width() - 640) / 2;
	CenterY = (HiddenSurface->Get_Height() - 400) / 2;
	file.Close();

	if (strlen(Scenario->BriefingText)) {
		DebugString("Restate: Fetching breifing text from %s\n", Scenario->ScenarioName);
		strcpy(BriefingText, Scenario->BriefingText);

	} else {
	if (Scenario->RequiredAddOn > ADDON_BASE_GAME) {
		sprintf(buffer, "MISSION%1d.INI",  Scenario->RequiredAddOn);
		file.Set_Name(buffer);
	} else {
		file.Set_Name("MISSION.INI");
	}

	if (file.Is_Available() == true) {
		ini.Load(file, false);
		DebugString("Restate: Fetching breifing text from Mission.ini\n");

		if (ini.Is_Present(Scenario->ScenarioName, "Briefing")) {
			ini.Get_String(Scenario->ScenarioName, "Briefing", "", buffer, sizeof(buffer));
			if (strlen(buffer)) {
				ini.Get_TextBlock(buffer, BriefingText, sizeof(BriefingText));
			}
			}
		}
	}

	// The presentation lays the text out itself, so the breaks the briefing carries are folded away.
			char * string = BriefingText;
			while (*string) {
				if (*string != '\n' && *string != '@') {
					string++;
				} else {
					char * next = ++string;
					if (*next != '\0') {
						while (*next != '\0' && *next == ' ') {
							next++;
						}
					}
					if (next != string) {
						strcpy(string, next);
					}
				}
			}

	Font = new MSFont;
	if (Font == NULL) {
		DebugString("Restate: Unable to create font!\n");
		return(false);
	}

	Drawer = Create_Drawer("MAPSEL.PAL");
	if (Drawer == NULL) {
		DebugString("Restate: Unable to create animation drawer!\n");
		return(false);
	}

	/*
	**	Other inits.
	*/
	LogicalSurface = HiddenSurface;

	/*
	**	Initialize the button structures. All are initialized, even though one (or none) may
	**	actually be added to the button list.
	*/
	int i;
	for (i = 0; i < BUTTON_COUNT; i++) {
		MyButton *btn = new MyButton(this,
									_buttons[i].ID,
									_buttons[i].Text,
									TPF_BUTTON,
									_buttons[i].Area.X, _buttons[i].Area.Y, _buttons[i].Area.Width, _buttons[i].Area.Height
									);

		if (btn == NULL) {
			DebugString("Restate: Unable to create button!\n");
			return(false);
		}

		btn->Disable();
		Buttons.Add(btn);
	}

	/*
	**	Add and initialize the buttons to the button list.
	*/
	ButtonList = Buttons[0];
	for (i = 1; i < Buttons.Count(); i++) {
		Buttons[i]->Add(*ButtonList);
	}

	MyButton *resume = Get_Button(BUTTON_RESUME);
	MyButton *video = Get_Button(BUTTON_VIDEO);

	if (scen->BriefMovie == VQ_NONE) {
		resume->X = CenterX + (640 - resume->Width) / 2;
		resume->Y += CenterY;
	} else {
		int width = std::max(resume->Width, video->Width);
		int xx = (2 * (320 - width) / 4);

		resume->X = xx + CenterX;
		resume->Y += CenterY - resume->Height / 2;
		resume->Width = width;

		video->X = resume->X + width + 2 * xx;
		video->Y += CenterY - video->Height / 2;
		video->Width = width;
	}

	AlternateSurface->Fill(0);
	HiddenSurface->Fill(0);

	return(true);
}


/// <summary>
/// Frees the resources held by the mission restatement.
/// This routine is used when the presentation is finished with, and again before a fresh
/// one is set up, so it is always safe to call.
/// </summary>
void RestateMission::Cleanup(void)
{
	String = NULL;
	if (Font != NULL) {
		delete Font;
		Font = NULL;
	}
	if (Drawer != NULL) {
		delete Drawer;
		Drawer = NULL;
	}
	ButtonList = NULL;
	for (int i = 0; i < Buttons.Count(); i++) {
		MyButton *btn = Buttons[i];
		if (btn != NULL) {
			delete btn;
		}
	}
	Buttons.Clear();
}


/// <summary>
/// Draws the parts of the presentation the animation engine does not own.
/// The engine calls this routine as it updates, so that the briefing page and the
/// buttons are laid over the top of whatever animation is running.
/// </summary>
void RestateMission::Do_Custom_Draw(Surface *surface)
{
	if (String != NULL) {
		Font->Draw_String(surface, (unsigned char *)String, StringRect.X, StringRect.Y, 2);
	}
	if (ButtonList != NULL) {
		ButtonList->Draw_All();
	}
}


/// <summary>
/// Handles the player's input during the mission restatement.
/// This routine polls the button list until the player picks one of the offered choices,
/// or dismisses the page with the space bar or the escape key.
/// </summary>
/// <returns>bool; Did the player ask to see the briefing video?</returns>
bool RestateMission::User_Input(void)
{
	unsigned input = KN_NONE;
	bool running = true;
	Keyboard->Clear();
	if (ButtonList != NULL) {
		ButtonList->Draw_All();
	}
	do {
		Wait_For_Focus();
		if (ButtonList != NULL) {
			input = ButtonList->Input();
		} else {
			if (Keyboard->Check() != KN_NONE) {
				input = Keyboard->Get();
			}
		}
		switch (input) {
			case (BUTTON_RESUME|KN_BUTTON):
			case (BUTTON_VIDEO|KN_BUTTON):
				running = false;
				break;

			case (BUTTON_MORE|KN_BUTTON):
			case (KN_SPACE):
			case (KN_ESC):
				running = false;
				break;
		}
		Wait_Delay(1);

	} while (running == true);
	Keyboard->Clear();

	return(input == (BUTTON_VIDEO|KN_BUTTON) ? true : false);
}


/// <summary>
/// Presents the "more" button and waits for the player.
/// This routine is used between the pages of a long briefing. It parks the button under
/// the text just printed and does not return until the player asks for the next page.
/// </summary>
/// <param name="x">The horizontal center for the button.</param>
/// <param name="y">The top edge for the button.</param>
void RestateMission::More_Button(int x, int y)
{
	MyButton *btn = Get_Button(BUTTON_MORE);
	if (btn) {
		btn->X = x - (btn->Width / 2);
		btn->Y = y;
		btn->Enable();
		User_Input();
		btn->Draw();
		btn->Disable();
	}
}


/// <summary>
/// Fetches the button that carries the specified identifier.
/// </summary>
/// <returns>Returns with a pointer to the button. Otherwise, NULL is returned.</returns>
MyButton *RestateMission::Get_Button(unsigned int id)
{
	for (int i = 0; i < Buttons.Count(); i++) {
		if (Buttons[i]->ID == id) {
			return(Buttons[i]);
		}
	}
	return(NULL);
}