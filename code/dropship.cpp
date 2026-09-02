/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "dropship.h"

#include "_alpha.h"
#include "_convert.h"
#include "_font.h"
#include "_keyboar.h"
#include "_mixfile.h"
#include "_palette.h"
#include "_surface.h"
#include "_zbuffer.h"
#include "bsurface.h"
#include "ccfile.h"
#include "ccrand.h"
#include "conquer.h"
#include "convert.h"
#include "data.h"
#include "dialog.h"
#include "draw.h"
#include "dsurface.h"
#include "font.h"
#include "globals.h"
#include "house.h"
#include "infatype.h"
#include "keyboard.h"
#include "language/language.h"
#include "misc.h"
#include "mixfile.h"
#include "mouse.h"
#include "rules.h"
#include "scenario.h"
#include "scheme.h"
#include "shapebtn.h"
#include "shapeset.h"
#include "stimer.h"
#include "techtype.h"
#include "unittype.h"
#include "weapon.h"
#include "zbuffer.h"

#include <algorithm>
#include <climits>
#include <cstring>

void Draw_Cost(int cost, int maxcost, Surface & surface, Point2D const & drawpoint, char const * title, bool center, ConvertClass & drawer, Blitter const * blitter);
void Draw_Unit_Info(Surface *surface, ConvertClass *drawer, TechnoTypeClass *techtype, bool allowed);


/// <summary>
/// Creates an empty dropship loadout.
/// </summary>
DropshipLoadoutClass::DropshipLoadoutClass(void) :
	CreationFrame(Frame),
	Unused1(0),
	Unused2(INT_MAX),
	UnusedBool1(false),
	EntryCount(0),
	Entries{},
	TotalCost(0)
{

}


/// <summary>
/// Destroys this dropship loadout.
/// </summary>
DropshipLoadoutClass::~DropshipLoadoutClass(void)
{
	//nothing
}


/// <summary>
/// Adds a unit to this dropship loadout.
/// The request is refused when the dropship is full or when the scenario has
/// already been given as many of that unit as it allows. On success the cost of
/// the unit is added to the loadout total and the scenario's tally of restricted
/// units is charged for it.
/// </summary>
/// <param name="ttype">The unit to load aboard.</param>
/// <returns>bool; Was the unit taken aboard?</returns>
bool DropshipLoadoutClass::Add(TechnoTypeClass * ttype)
{
	if (EntryCount < MAX_ENTRIES) {
		int usage_index;

		if (!Scen->AllowableUnits.Count()) {
			usage_index = -1;
		} else {
			usage_index = Get_Allowable_Index(ttype);
		}

		/// If not found in allowed units, or if over max count for that type, reject
		if (usage_index != -1 && Scen->AllowableUnitCounts[usage_index] >= Scen->AllowableUnitMaximums[usage_index]) {
			return(false);
		}

		Entries[EntryCount++] = ttype;
		TotalCost += ttype->Raw_Cost();

		if (usage_index != -1) {
			Scen->AllowableUnitCounts[usage_index]++;
		}
		return(true);
	}

	return(false);
}


/// <summary>
/// Removes a unit from this dropship loadout.
/// The cost of the unit is refunded to the loadout total and the scenario's tally
/// of restricted units is credited back. The remaining units close up so that the
/// loadout has no gaps in it.
/// </summary>
/// <param name="index">The loadout slot to empty.</param>
/// <returns>bool; Was a unit removed?</returns>
bool DropshipLoadoutClass::Remove(int index)
{
	if (EntryCount > 0 && index >= 0 && index < EntryCount) {
		int usage_index = Get_Allowable_Index(Entries[index]);

		if (usage_index != -1) {
			Scen->AllowableUnitCounts[usage_index]--;
		}
		TotalCost -= Entries[index]->Raw_Cost();

		/// Shift entries down if not removing the last one
		for (int i = index; i < EntryCount - 1; ++i) {
			Entries[i] = Entries[i + 1];
		}

		/*
		 * Null out the now-unused last slot
		 */
		EntryCount--;
		Entries[EntryCount] = NULL;

		return(true);
	}
	return(false);
}


/// <summary>
/// Empties this dropship loadout.
/// Every unit is removed one at a time, so the scenario's tally of restricted
/// units is credited back for each of them.
/// </summary>
void DropshipLoadoutClass::Clear(void)
{
	UnusedBool1 = false;
	int count = EntryCount;
	while (count > 0) {
		Remove(0);
		count--;
	}
	EntryCount = 0;
	TotalCost = 0;
}


/// <summary>
/// Fetches one of the units in this loadout.
/// </summary>
/// <param name="index">The loadout slot to fetch.</param>
/// <returns>Returns with the unit in that slot, or NULL if the slot is empty.</returns>
/// <remarks>The index is not checked, so be sure it lies within the loadout.</remarks>
TechnoTypeClass * DropshipLoadoutClass::Fetch(int index)
{
	return(Entries[index]);
}


/// <summary>
/// Finds a unit's place in the scenario's allowable unit list.
/// This routine is used to reach the tally and the limit the scenario has set for
/// the unit. Units the scenario does not restrict have no place in the list.
/// </summary>
/// <param name="ttype">The unit to look up.</param>
/// <returns>Returns with the index into the allowable unit list, or -1 if the scenario
/// places no limit on this unit.</returns>
int DropshipLoadoutClass::Get_Allowable_Index(TechnoTypeClass * ttype)
{
	if (Scen->AllowableUnits.Count()) {
		for (int i = 0; i < Scen->AllowableUnits.Count(); ++i) {
			if (Scen->AllowableUnits[i] == ttype && Scen->AllowableUnitMaximums[i] != -1) {
				return(i);
			}
		}
	}
	return(-1);
}

struct ButtonFadeEffect
{
	unsigned int StartTime;
	int CandidateIndex;
	float Alpha;
	float Direction;
	bool StopAtLow;

	ButtonFadeEffect(unsigned int start_time, int candidate_index, float alpha, float direction, bool stop_at_low) :
		StartTime(start_time),
		CandidateIndex(candidate_index),
		Alpha(alpha),
		Direction(direction),
		StopAtLow(stop_at_low)
	{
	}
};

/*
 * One cameo slot dissolving from one piece of art to another. The "from" art is
 * blitted over the slot as it is, then the "to" art is blitted on top of it through
 * the alpha buffer, whose level ramps up to 255 over the life of the effect.
 */
struct CrossDissolveEffect
{
	Rect FromRect;
	Rect ToRect;
	Rect DestRect;
	BSurface *FromSurface;
	BSurface *ToSurface;
	int Alpha;
	unsigned int StartTime;
	Blitter const *FromBlitter;
	Blitter const *ToBlitter;
	bool WasDrawn;

	CrossDissolveEffect(Rect const &from_rect, Rect const &to_rect, Rect const &dest_rect, BSurface *from_surface, BSurface *to_surface, Blitter const *from_blitter, Blitter const *to_blitter) :
		FromRect(from_rect),
		ToRect(to_rect),
		DestRect(dest_rect),
		FromSurface(from_surface),
		ToSurface(to_surface),
		Alpha(0),
		StartTime(::timeGetTime()),
		FromBlitter(from_blitter),
		ToBlitter(to_blitter),
		WasDrawn(false)
	{
	}
};


/// <summary>
/// Handles the dropship loadout screen.
/// This routine runs the pre-mission screen where the player fills the dropships
/// with the units to arrive with. It owns its own input loop and does not return
/// until the player accepts the loadout, at which point the chosen units are
/// recorded in the player house's dropship loadouts ready for the mission to
/// deliver them.
/// </summary>
/// <remarks>The alpha and depth buffers are re-created around this screen, so anything
/// held in them beforehand is lost.</remarks>
void Dropship_Screen(void)
{
	int i;
	int j;
	int k;

	int dropship_count = Scen->StartingDropships;
	int money = PlayerPtr->Available_Money();
	unsigned int money_display = 0;

	static const char *_drop_btn_names[] = {
		"DROP0001.SHP",
		"DROP0002.SHP",
		"DROP0003.SHP",
	};

	static const char *_green_btn_names[] = {
		"DGREEN1.SHP",
		"DGREEN2.SHP",
		"DGREEN3.SHP",
		"DGREEN4.SHP",
	};

	/*
	 * How many units one dropship can carry.
	 */
	#define SLOT_PER_DROPSHIP DropshipLoadoutClass::MAX_ENTRIES

	/*
	 * The cameo grid the player picks units from. It is two columns of four,
	 * each cameo separated from the next by the gap values.
	 */
	static const int _cameo_count = 8;
	static const int _cameo_x = 491;
	static const int _cameo_y = 25;
	static const int _cameo_width = 64;
	static const int _cameo_height = 48;
	static const int _cameo_gap_x = 4;
	static const int _cameo_gap_y = 2;

	/*
	 * The two buttons that scroll the cameo grid, and the places they occupy
	 * in the button array.
	 */
	static const int _up_x = 527;
	static const int _up_y = 229;
	static const int _down_x = 557;
	static const int _down_y = 229;
	static const int _up_index = 8;
	static const int _down_index = 9;

	/*
	 * The loadout slots the chosen units are shown in. They follow the scroll
	 * buttons in the button array, and wrap onto a second row after two of them.
	 */
	static const int _slot_index = 10;
	static const int _slot_width = 64;
	static const int _slot_gap_x = 2;
	static const int _slot_x = 53;
	static const int _slot_height = 48;

	/*
	 * Units cheaper than this are never offered.
	 */
	static const int _minimum_cost = 10;

	/*
	 * The green lights that flash beside the dropship as it is loaded.
	 */
	static const int _light_rate = 100;
	static const int _light_x = 371;

	/*
	 * A cameo that has just been used up fades between the two alpha levels at
	 * this rate.
	 */
	static const int _fade_rate = 10;
	static const int _fade_low = 63;
	static const int _fade_scale = 70;

	/*
	 * The looping animation printed down the left of the screen.
	 */
	static const int _loadout_rate = 250;
	static const int _loadout_x = 45;
	static const int _loadout_y = 0;

	/*
	 * How quickly a cameo dissolves into the slot it was dropped into.
	 */
	static const int _dissolve_scale = 30;
	static const int _dissolve_rate = 7;

	/*
	 * The credits readout, and how often it is allowed to count toward the
	 * amount the player has left.
	 */
	static const int _money_x = 500;
	static const int _money_y = 385;
	static const int _money_rate = 50;

	/*
	 * A button stays "clicked" for this long, so that holding the mouse down
	 * does not load the dropship in one go.
	 */
	static const int _click_delay = 300;

	/*
	 * A third button width, which nothing reads: every rect that could have
	 * spent it folds to the same bytes the cameo width gives.
	 */
	static const int _button_width = 64;

	/*
	 * The light that blinks in the cockpit. It is stepped at this rate once it
	 * starts, and it only starts on the rare frame that the chance comes up.
	 */
	static const int _pilot_rate = 150;
	static const float _pilot_chance = 0.02f;
	static const int _pilot_x = 284;
	static const int _pilot_y = 151;
	static const int _pilot_delay = 5;

	static const int _loadout_slot_ys[3][3][SLOT_PER_DROPSHIP] = {
		{
			{69,69,119,119,119},
			{0,0,0,0,0},
			{0,0,0,0,0}
		},
		{
			{69,69,119,119,119},
			{209,209,259,259,259},
			{0,0,0,0,0}
		},
		{
			{39,39,89,89,89},
			{159,159,209,209,209},
			{279,279,329,329,329}
		}
	};

	static const int _green_light_ys[] = {
		10, 60, 110, 160
	};

	ShapeSet *green_shapes[ARRAY_SIZE(_green_btn_names)];
	for (i = 0; i < ARRAY_SIZE(_green_btn_names); ++i) {
		CCFileClass file(_green_btn_names[i]);
		unsigned char *shape_buf = new unsigned char[file.Size()];
		file.Read(shape_buf, file.Size());
		green_shapes[i] = (ShapeSet *)shape_buf;
	}

	ShapeSet *loadout_shape = (ShapeSet *)MFCD::Retrieve("LOADOUT.SHP");
	ShapeSet *pilotlight_shape = (ShapeSet *)MFCD::Retrieve("PILOTLIT.SHP");
	ShapeSet *no_cameo_shape = (ShapeSet *)MFCD::Retrieve("XXICON.SHP");

	CCFileClass drop_shape_file(_drop_btn_names[dropship_count - 1]);
	unsigned char *drop_shape_buf = new unsigned char[drop_shape_file.Size()];
	drop_shape_file.Read(drop_shape_buf, drop_shape_file.Size());
	ShapeSet *drop_shape = (ShapeSet *)drop_shape_buf;

	BSurface *background_surface = new BSurface(drop_shape->Get_Width(), drop_shape->Get_Height(), 1, drop_shape->Get_Data(0));

	int drop_height = drop_shape->Get_Height();
	int drop_width = drop_shape->Get_Width();
	int y = (HiddenSurface->Get_Height() - drop_height) / 2;
	int x = (HiddenSurface->Get_Width() - drop_width) / 2;

	PaletteClass palette_data;
	memmove(&palette_data, (unsigned char *)MFCD::Retrieve("DROPSHIP.PAL"), sizeof(palette_data));
	for (i = 0; i < 256; ++i) {
		palette_data[i] = RGBClass(palette_data[i].Get_Red() * 4, palette_data[i].Get_Green() * 4, palette_data[i].Get_Blue() * 4);
	}

	ConvertClass *drawer_dropship = new ConvertClass(palette_data, GamePalette, *VisibleSurface, 63, false);
	ConvertClass *drawer_dropship_alpha = new ConvertClass(palette_data, GamePalette, *VisibleSurface, 1, false);

	memmove(&palette_data, (unsigned char *)MFCD::Retrieve("CAMEO.PAL"), sizeof(palette_data));
	for (i = 0; i < 256; ++i) {
		palette_data[i] = RGBClass(palette_data[i].Get_Red() * 4, palette_data[i].Get_Green() * 4, palette_data[i].Get_Blue() * 4);
	}
	ConvertClass *drawer_cameo = new ConvertClass(palette_data, GamePalette, *VisibleSurface, 1, false);

	Blitter const *plain_blitter = drawer_dropship->Blitter_From_Flags(SHAPE_NORMAL);
	Blitter const *cameo_alpha_blitter = drawer_cameo->Blitter_From_Flags(SHAPE_ALPHA_BLEND);
	Blitter const *drop_alpha_blitter = drawer_dropship_alpha->Blitter_From_Flags(SHAPE_ALPHA_BLEND);

	HiddenSurface->Fill(0);

	DynamicVectorClass<TechnoTypeClass *> candidates;
	int type_count = InfantryTypes.Count() + UnitTypes.Count();

	unsigned int ownable_mask = 1 << HouseTypes.ID(PlayerPtr->Class);

	if (Scen->AllowableUnits.Count() > 0) {
		for (i = 0; i < Scen->AllowableUnits.Count(); ++i) {
			TechnoTypeClass *techtype = Scen->AllowableUnits[i];
			if ((techtype->Ownable & ownable_mask) && Scen->AllowableUnitMaximums[i] != 0) {
				candidates.Add(techtype);
			}
		}
	} else {
		for (k = 0; k < type_count; ++k) {
			TechnoTypeClass *techtype = NULL;
			if (k < InfantryTypes.Count()) {
				techtype = InfantryTypes[k];
			} else {
				techtype = UnitTypes[k - InfantryTypes.Count()];
			}

			if ((int)techtype->Level > PlayerPtr->Control.TechLevel) {
				continue;
			}
			if (techtype->Fetch_RTTI() == RTTI_INFANTRYTYPE && techtype->Category == CATEGORY_CIVILIAN) {
				continue;
			}
			if (techtype->Level == -1) {
				continue;
			}
			if (techtype->Raw_Cost() <= _minimum_cost) {
				continue;
			}
			if ((techtype->Ownable & ownable_mask) == 0) {
				continue;
			}

			candidates.Add(techtype);
		}
	}

	int cameo_top = 0;
	bool can_scroll_up = false;
	int button_count = _cameo_count + 2 + dropship_count * SLOT_PER_DROPSHIP;
	bool can_scroll_down = candidates.Count() > _cameo_count;
	ShapeButtonClass **buttons = new ShapeButtonClass *[button_count];
	ToggleClass *button_list = NULL;

	int id = 1;
	for (i = 0; i < _cameo_count; ++i) {
		ShapeButtonClass *button = new ShapeButtonClass(id, i < candidates.Count() ? (ShapeSet const *)candidates[i]->Get_Cameo_Data() : no_cameo_shape, x + _cameo_x + (_cameo_width + _cameo_gap_x) * (i % 2), y + _cameo_y + (_cameo_height + _cameo_gap_y) * (i / 2), 0, 0, true);
		button->ShapeDrawer = CameoDrawer;
		buttons[i] = button;
		if (i == 0) {
			button_list = buttons[0];
		} else {
			button->Add(*button_list);
		}
		++id;
	}

	ShapeButtonClass *up_button = new ShapeButtonClass(id, (ShapeSet *)MFCD::Retrieve("DROPUP.SHP"), x + _up_x, y + _up_y, 0, 0, false);
	up_button->ShapeDrawer = drawer_dropship;
	up_button->IsSticky = true;
	buttons[_up_index] = up_button;
	up_button->Add(*button_list);
	++id;

	ShapeButtonClass *down_button = new ShapeButtonClass(id, (ShapeSet *)MFCD::Retrieve("DROPDOWN.SHP"), x + _down_x, y + _down_y, 0, 0, false);
	down_button->ShapeDrawer = drawer_dropship;
	down_button->IsSticky = true;
	buttons[_down_index] = down_button;
	down_button->Add(*button_list);
	++id;

	for (i = 0; i < dropship_count; ++i) {
		for (j = 0; j < SLOT_PER_DROPSHIP; ++j) {
			int xpos = j * (_slot_width + _slot_gap_x);
			ShapeButtonClass *button = new ShapeButtonClass(id, NULL, x + _slot_x + (xpos < 2 * (_slot_width + _slot_gap_x) ? xpos : xpos - 2 * (_slot_width + _slot_gap_x)), y + _loadout_slot_ys[dropship_count - 1][i][j], 0, 0, false);
			button->ShapeDrawer = CameoDrawer;
			buttons[id - 1] = button;
			button->Add(*button_list);
			++id;
		}
	}

	button_list->Turn_On();

	bool full_redraw = true;
	bool accept = false;
	bool recent_click = false;

	unsigned int light_frame[ARRAY_SIZE(_green_light_ys)];
	unsigned int light_start[ARRAY_SIZE(_green_light_ys)];
	memset(light_frame, 0xFF, sizeof(light_frame));
	memset(light_start, 0, sizeof(light_start));

	bool force_light_redraw = false;

	DynamicVectorClass<ButtonFadeEffect *> button_fades;

	unsigned int money_display_time = ::timeGetTime();
	unsigned int screen_start_time = money_display_time;
	int loadout_anim_frame = 0;
	unsigned int last_input_time = 0;
	int pilot_frame = -1;

	CDTimerClass<SystemTimerClass> pilot_timer = _pilot_delay;
	unsigned int pilot_start_time = 0;

	DynamicVectorClass<CrossDissolveEffect *> cross_dissolves;
	bool force_info_redraw = false;

	TechnoTypeClass *last_hover_type = NULL;
	int last_hover_slot = -1;
	int selected_count = 0;

	DynamicVectorClass<TechnoTypeClass *> selections;

	Rect alpha_area = AlphaBuffer->Get_Bounds();
	Hide_Mouse();
	delete AlphaBuffer;
	AlphaBuffer = NULL;
	AlphaBuffer = new ABuffer(Rect(0, 0, HiddenSurface->Get_Width(), HiddenSurface->Get_Height()));
	Show_Mouse();

	LogicalSurface = HiddenSurface;
	Keyboard->Clear();

	while (!accept) {
		bool redraw = false;

		if (full_redraw) {
			full_redraw = false;
			redraw = true;
			Draw_Shape(*HiddenSurface, *drawer_dropship, drop_shape, 0, Point2D(0, 0), Rect(x, y, drop_shape->Get_Width(), drop_shape->Get_Height()), SHAPE_NORMAL);

			for (i = 0; i < button_count; ++i) {
				int usage_index = -1;
				if (i + cameo_top < candidates.Count()) {
					usage_index = DropshipLoadoutClass::Get_Allowable_Index(candidates[cameo_top + i]);
				}

				if (i >= _cameo_count || usage_index == -1 || Scen->AllowableUnitCounts[usage_index] < Scen->AllowableUnitMaximums[usage_index]) {
					AlphaBuffer->Fill(127, Rect(x + buttons[i]->X, y + buttons[i]->Y, _cameo_width, _cameo_height));
				} else {
					AlphaBuffer->Fill(_fade_low, Rect(x + buttons[i]->X, y + buttons[i]->Y, _cameo_width, _cameo_height));
				}
				buttons[i]->Draw_Me(0);
			}
		}

		Call_Back();

		KeyNumType input = button_list->Input();
		if (!recent_click && (input & KN_BUTTON) != 0) {
			input = (KeyNumType)(input & ~KN_BUTTON);
			last_input_time = ::timeGetTime();

			if (input <= _cameo_count && selected_count < dropship_count * SLOT_PER_DROPSHIP) {
				int candidate_index = cameo_top + input - 1;
				if (candidate_index >= 0 && candidate_index < candidates.Count()) {
					TechnoTypeClass *techtype = candidates[cameo_top + input - 1];

					int usage_index = !Scen->AllowableUnits.Count() ? -1 : DropshipLoadoutClass::Get_Allowable_Index(techtype);
					if (money - techtype->Raw_Cost() >= 0 && (usage_index == -1 || Scen->AllowableUnitCounts[usage_index] < Scen->AllowableUnitMaximums[usage_index])) {
						buttons[_slot_index + selected_count]->Set_Shape((ShapeSet const *)techtype->Get_Cameo_Data(), 0, 0);

						ShapeSet const *cameo_shape = (ShapeSet const *)techtype->Get_Cameo_Data();
						ShapeButtonClass *slot_button = buttons[_slot_index + selected_count];
						BSurface *cameo_surface = new BSurface(cameo_shape->Get_Width(), cameo_shape->Get_Height(), 1, cameo_shape->Get_Data(0));

						CrossDissolveEffect *dissolve_effect =
							new CrossDissolveEffect(Rect(slot_button->X - x, slot_button->Y - y, slot_button->Width, slot_button->Height),
													Rect(0, 0, cameo_shape->Get_Width(), cameo_shape->Get_Height()),
													Rect(slot_button->X, slot_button->Y, slot_button->Width, slot_button->Height),
													background_surface,
													cameo_surface,
													drop_alpha_blitter,
													cameo_alpha_blitter);
						cross_dissolves.Add(dissolve_effect);

						if (usage_index != -1) {
							Scen->AllowableUnitCounts[usage_index]++;
						}
						selections.Add(techtype);

						redraw = true;
						selected_count++;
						recent_click = true;

						int light_row = (input - 1) / 2;
						if (light_frame[light_row] == (unsigned int)-1) {
							light_frame[light_row] = 0;
							light_start[light_row] = ::timeGetTime();
						}

						force_light_redraw = true;
						for (j = 0; ; ++j) {
							if (j >= button_fades.Count()) {
								ButtonFadeEffect *fade = new ButtonFadeEffect(::timeGetTime(), candidate_index, 127.0f, -1.0f, false);
								if (usage_index != -1 && Scen->AllowableUnitCounts[usage_index] >= Scen->AllowableUnitMaximums[usage_index]) {
									force_info_redraw = true;
									fade->StopAtLow = true;
								}
								button_fades.Add(fade);
								break;
							}
							if (button_fades[j]->CandidateIndex == candidate_index) {
								break;
							}
						}

						money -= techtype->Raw_Cost();
					}
				}
			} else if ((input == 9 && can_scroll_up) || (input == 10 && can_scroll_down)) {
				if (input == 9) {
					cameo_top -= 2;
				} else {
					cameo_top += 2;
				}

				for (i = 0; i < _cameo_count; ++i) {
					int candidate_index = cameo_top + i;
					buttons[i]->Set_Shape(candidate_index < candidates.Count() ? (ShapeSet const *)candidates[candidate_index]->Get_Cameo_Data() : no_cameo_shape, 0, 0);

					int usage_index = -1;
					if (candidate_index < candidates.Count()) {
						usage_index = DropshipLoadoutClass::Get_Allowable_Index(candidates[candidate_index]);
					}
					if (usage_index == -1 || Scen->AllowableUnitCounts[usage_index] < Scen->AllowableUnitMaximums[usage_index]) {
						AlphaBuffer->Fill(127, Rect(buttons[i]->X, buttons[i]->Y, _cameo_width, _cameo_height));
					} else {
						AlphaBuffer->Fill(_fade_low, Rect(buttons[i]->X, buttons[i]->Y, _cameo_width, _cameo_height));
					}
					buttons[i]->Draw_Me(1);
				}

				buttons[input - 1]->IsPressed = false;
				buttons[input - 1]->Draw_Me(1);

				can_scroll_up = cameo_top != 0;
				redraw = true;
				can_scroll_down = cameo_top < candidates.Count() - _cameo_count;
			} else if (input >= 11 && input < dropship_count * SLOT_PER_DROPSHIP + 11) {
				int remove_index = input - 11;
				if (remove_index < selections.Count()) {
					int usage_index = DropshipLoadoutClass::Get_Allowable_Index(selections[remove_index]);
					if (usage_index != -1) {
						if (Scen->AllowableUnitCounts[usage_index] >= Scen->AllowableUnitMaximums[usage_index]) {
							for (j = 0; j < candidates.Count(); ++j) {
								if (candidates[j] == selections[remove_index]) {
									ButtonFadeEffect *fade = new ButtonFadeEffect(::timeGetTime(), j, 127.0f, -1.0f, false);
									fade->Direction = 1.0f;
									fade->Alpha = 63.0f;
									button_fades.Add(fade);
									break;
								}
							}
						}
						Scen->AllowableUnitCounts[usage_index]--;
					}

					money += selections[remove_index]->Raw_Cost();
					selections.Delete_Index(remove_index);
					redraw = true;

					for (i = 0; i < selected_count - remove_index - 1; ++i) {
						ShapeButtonClass *dest_slot = buttons[input + i - 1];

						ShapeSet const *old_shape = dest_slot->Get_Shape_Data();
						BSurface *old_surface = new BSurface(old_shape->Get_Width(), old_shape->Get_Height(), 1, old_shape->Get_Data(0));

						dest_slot->Set_Shape(buttons[input + i]->Get_Shape_Data(), 0, 0);

						ShapeSet const *new_shape = dest_slot->Get_Shape_Data();
						BSurface *new_surface = new BSurface(new_shape->Get_Width(), new_shape->Get_Height(), 1, new_shape->Get_Data(0));

						CrossDissolveEffect *dissolve_effect =
							new CrossDissolveEffect(Rect(0, 0, dest_slot->Width, dest_slot->Height),
													Rect(0, 0, dest_slot->Width, dest_slot->Height),
													Rect(dest_slot->X, dest_slot->Y, dest_slot->Width, dest_slot->Height),
													old_surface,
													new_surface,
													cameo_alpha_blitter,
													cameo_alpha_blitter);
						cross_dissolves.Add(dissolve_effect);
					}

					ShapeButtonClass *last_slot = buttons[selected_count + 9];
					ShapeSet const *last_shape = last_slot->Get_Shape_Data();
					BSurface *last_surface = new BSurface(last_shape->Get_Width(), last_shape->Get_Height(), 1, last_shape->Get_Data(0));

					CrossDissolveEffect *dissolve_effect =
						new CrossDissolveEffect(Rect(0, 0, last_shape->Get_Width(), last_shape->Get_Height()),
												Rect(last_slot->X - x, last_slot->Y - y, last_slot->Width, last_slot->Height),
												Rect(last_slot->X, last_slot->Y, last_slot->Width, last_slot->Height),
												last_surface,
												background_surface,
												cameo_alpha_blitter,
												drop_alpha_blitter);
					cross_dissolves.Add(dissolve_effect);

					last_slot->Set_Shape(NULL, 0, 0);
					selected_count--;
				}
			}
		} else if (input == KN_SPACE) {
			accept = true;
		}

		if (buttons[_up_index]->IsPressed && can_scroll_up && !recent_click) {
			buttons[_up_index]->Draw_Me(0);
			redraw = true;
		}
		if (buttons[_down_index]->IsPressed && can_scroll_down && !recent_click) {
			buttons[_down_index]->Draw_Me(0);
			redraw = true;
		}

		for (i = 0; i < ARRAY_SIZE(_green_light_ys); ++i) {
			if (light_frame[i] != (unsigned int)-1) {
				int frame = light_frame[i];
				int next_frame = (timeGetTime() - light_start[i]) / _light_rate;
				if (next_frame != frame || force_light_redraw) {
					int light_y = y + _green_light_ys[i];
					int light_x = x + _light_x;
					ShapeSet *shape = green_shapes[i];
					if (next_frame > shape->Get_Count()) {
						Blit_Block(
							*HiddenSurface,
							*drawer_dropship,
							*background_surface,
							Rect(light_x - x, light_y - y, shape->Get_Width(), shape->Get_Height()),
							Point2D(0, 0),
							Rect(light_x, light_y, shape->Get_Width(), shape->Get_Height()),
							NULL,
							plain_blitter);
						light_frame[i] = (unsigned int)-1;
					} else {
						Draw_Shape(
							*HiddenSurface,
							*drawer_dropship,
							shape,
							frame,
							Point2D(0, 0),
							Rect(light_x, light_y, shape->Get_Width(), shape->Get_Height()),
							SHAPE_NORMAL);
						light_frame[i] = next_frame;
					}
					redraw = true;
				}
			}
		}

		for (i = button_fades.Count() - 1; i >= 0; --i) {
			ButtonFadeEffect *effect = button_fades[i];
			int alpha;
			if (effect->Direction < 0.0f) {
				alpha = 127 - (_fade_rate * timeGetTime() - _fade_rate * effect->StartTime) / _fade_scale;
				if (alpha < _fade_low) {
					alpha = _fade_low;
					effect->Direction = 1.0f;
					effect->StartTime = timeGetTime();
				}
			} else {
				alpha = (_fade_rate * timeGetTime() - _fade_rate * effect->StartTime) / _fade_scale + _fade_low;
				if (alpha > 127) {
					alpha = 127;
				}
			}

			float falpha = (float)alpha;
			if (falpha != effect->Alpha) {
				effect->Alpha = falpha;
				if (effect->CandidateIndex >= cameo_top && effect->CandidateIndex <= cameo_top + _cameo_count) {
					int screen_index = effect->CandidateIndex - cameo_top;
					AlphaBuffer->Fill(falpha, Rect(x + _cameo_x + (_cameo_width + _cameo_gap_x) * (screen_index % 2), y + _cameo_y + (_cameo_height + _cameo_gap_y) * (screen_index / 2), _cameo_width, _cameo_height));
					buttons[screen_index]->Draw_Me(1);
					redraw = true;
				}

				if (effect->Alpha == 127.0f || (effect->StopAtLow && effect->Alpha == 63.0f)) {
					delete effect;
					button_fades.Delete_Index(i);
				}
			}
		}

		if (cross_dissolves.Count() != 0) {
			i = 0;
			if (i < cross_dissolves.Count()) {
				int preceding_count = 0;
				do {
					CrossDissolveEffect *effect = cross_dissolves[i];
					bool overlap_drawn = false;
					bool draw_background = true;

					j = i - 1;
					for (int scan = preceding_count; scan >= 1; --scan) {
						CrossDissolveEffect *earlier = cross_dissolves[j];
						if (earlier->DestRect == effect->DestRect) {
							draw_background = false;
							if (overlap_drawn || earlier->WasDrawn) {
								overlap_drawn = true;
							}
						}
						--j;
					}

					int alpha = std::min(255ul, (_dissolve_rate * timeGetTime() - _dissolve_rate * effect->StartTime) / _dissolve_scale);

					if (alpha != effect->Alpha || overlap_drawn) {
						effect->Alpha = alpha;
						if (draw_background) {
							Blit_Block(*HiddenSurface, *drawer_dropship, *effect->FromSurface, effect->FromRect, Point2D(0, 0), effect->DestRect, NULL, effect->FromBlitter);
						}
						AlphaBuffer->Fill(effect->Alpha, effect->DestRect);
						Blit_Block(*HiddenSurface, *drawer_cameo, *effect->ToSurface, effect->ToRect, Point2D(0, 0), effect->DestRect, NULL, effect->ToBlitter);
						effect->WasDrawn = true;
						redraw = true;
					}

					if (alpha == 255) {
						if (effect->FromSurface != background_surface) {
							delete effect->FromSurface;
						}
						if (effect->ToSurface != background_surface) {
							delete effect->ToSurface;
						}
						delete effect;
						cross_dissolves.Delete_Index(i);
						--i;
						--preceding_count;
					}

					++i;
					++preceding_count;
				} while (i < cross_dissolves.Count());
			}
		}

		recent_click = (::timeGetTime() - last_input_time) < _click_delay;

		int loadout_count = loadout_shape->Get_Count();
		int next_loadout_frame = ((::timeGetTime() - screen_start_time) / _loadout_rate) % loadout_count;
		if (next_loadout_frame != loadout_anim_frame) {
			loadout_anim_frame = next_loadout_frame;
			Draw_Shape(*HiddenSurface, *drawer_dropship, loadout_shape, loadout_anim_frame, Point2D(0, 0), Rect(Point2D(x, y) + Point2D(_loadout_x, _loadout_y), loadout_shape->Get_Width(), loadout_shape->Get_Height()), SHAPE_NORMAL);
			redraw = true;
		}

		unsigned int now = ::timeGetTime();
		if ((unsigned int)money != money_display) {
			unsigned int elapsed = now - money_display_time;
			if (elapsed >= _money_rate) {
				int previous_display = money_display;
				double distance = (double)abs(money - (int)money_display);
				double elapsed_time = (double)elapsed;
				double step = std::min(std::sqrt(distance) * elapsed_time * 0.04, distance);

				int delta = (int)step;
				if (money < (int)money_display) {
					delta = -delta;
				}

				money_display += delta;
				Draw_Cost(money_display, previous_display, *background_surface, Point2D(x + _money_x, y + _money_y), Fetch_String(TXT_CREDITS_COLON), false, *drawer_dropship, plain_blitter);
				redraw = true;
				money_display_time = now;
			}
		} else {
			money_display_time = now;
		}

		if (pilot_frame >= 0) {
			int next_pilot_frame = (::timeGetTime() - pilot_start_time) / _pilot_rate;
			if (next_pilot_frame != pilot_frame) {
				if (next_pilot_frame < pilotlight_shape->Get_Count()) {
					pilot_frame = next_pilot_frame;
					Draw_Shape(*HiddenSurface, *drawer_dropship, pilotlight_shape, pilot_frame, Point2D(0, 0), Rect(x + _pilot_x, y + _pilot_y, pilotlight_shape->Get_Width(), pilotlight_shape->Get_Height()), SHAPE_NORMAL);
					redraw = true;
				} else {
					pilot_frame = -1;
					Blit_Block(*HiddenSurface, *drawer_dropship, *background_surface, Rect(_pilot_x, _pilot_y, pilotlight_shape->Get_Width(), pilotlight_shape->Get_Height()), Point2D(0, 0), Rect(x + _pilot_x, y + _pilot_y, pilotlight_shape->Get_Width(), pilotlight_shape->Get_Height()), NULL, plain_blitter);
					pilot_timer = TIMER_SECOND;
					redraw = true;
				}
			}
		} else {
			if (pilot_timer == 0) {
				if (Scen->RandomNumber(0, INT_MAX - 1) / (double)(INT_MAX - 1) < _pilot_chance) {
					pilot_frame = 0;
					pilot_start_time = ::timeGetTime();
					Draw_Shape(*HiddenSurface, *drawer_dropship, pilotlight_shape, 0, Point2D(0, 0), Rect(x + _pilot_x, y + _pilot_y, pilotlight_shape->Get_Width(), pilotlight_shape->Get_Height()), SHAPE_NORMAL);
					redraw = true;
				}

				pilot_timer = _pilot_delay;
			}
		}

		Point2D mouse_pos = MouseCursor->Get_Mouse_Point();
		int hover_slot;
		int previous_hover_slot;
		TechnoTypeClass *hover_type = NULL;

		int grid_x = x + _cameo_x;
		int grid_y = y + _cameo_y;
		if (mouse_pos.X >= grid_x && mouse_pos.X < grid_x + 2 * (_cameo_width + _cameo_gap_x) && mouse_pos.Y >= grid_y && mouse_pos.Y < grid_y + 4 * (_cameo_height + _cameo_gap_y)) {
			int col = (mouse_pos.X - x - _cameo_x) / (_cameo_width + _cameo_gap_x);
			int row = (mouse_pos.Y - y - _cameo_y) / (_cameo_height + _cameo_gap_y);
			int left = x + (_cameo_width + _cameo_gap_x) * col + _cameo_x;
			int top = y + (_cameo_height + _cameo_gap_y) * row + _cameo_y;
			if (mouse_pos.X >= left
				&& mouse_pos.X < left + _cameo_width
				&& mouse_pos.Y >= top
				&& mouse_pos.Y < top + _cameo_height
				&& cameo_top + col + 2 * row < candidates.Count()) {
				hover_type = candidates[cameo_top + col + 2 * row];
				previous_hover_slot = last_hover_slot;
				hover_slot = col + 2 * row;
				last_hover_slot = hover_slot;
			} else {
				hover_slot = -1;
				previous_hover_slot = last_hover_slot;
				last_hover_slot = -1;
			}
		} else {
			hover_slot = -1;
			previous_hover_slot = last_hover_slot;
			last_hover_slot = -1;
		}

		bool hover_allowed;
		int usage_index = DropshipLoadoutClass::Get_Allowable_Index(hover_type);
		if (usage_index != -1) {
			hover_allowed = Scen->AllowableUnitCounts[usage_index] < Scen->AllowableUnitMaximums[usage_index];
		} else {
			hover_allowed = true;
		}

		if (hover_type != last_hover_type || force_info_redraw) {
			last_hover_type = hover_type;
			Draw_Unit_Info(background_surface, drawer_dropship, hover_type, hover_allowed);
			redraw = true;
		}

		force_light_redraw = false;
		if (redraw) {
			if (hover_slot != previous_hover_slot && previous_hover_slot != -1) {
				buttons[previous_hover_slot]->Draw_Me(1);
			}

			if (hover_slot != -1) {
				int color = hover_allowed ? DSurface::Build_Hicolor_Pixel(0, 255, 0) : DSurface::Build_Hicolor_Pixel(255, 0, 0);
				HiddenSurface->Draw_Rect(Rect(x + _cameo_x + (_cameo_width + _cameo_gap_x) * (hover_slot % 2), y + _cameo_y + (_cameo_height + _cameo_gap_y) * (hover_slot / 2), _cameo_width, _cameo_height), color);
			}

			Update_Visible_Surface(HiddenSurface, NULL);
		}
	}

	for (i = 0; i < selections.Count(); ++i) {
		PlayerPtr->DropshipLoadouts[i / SLOT_PER_DROPSHIP].Add(selections[i]);
	}

	delete drawer_dropship;
	delete drawer_cameo;
	delete drawer_dropship_alpha;
	delete background_surface;
	delete [] drop_shape_buf;
	for (i = 0; i < ARRAY_SIZE(green_shapes); ++i) {
		delete green_shapes[i];
	}
	for (i = 0; i < button_count; ++i) {
		delete buttons[i];
	}
	delete [] buttons;

	Hide_Mouse();

	delete AlphaBuffer;
	AlphaBuffer = NULL;
	AlphaBuffer = new ABuffer(alpha_area);

	if (DepthBuffer) {
		delete DepthBuffer;
		DepthBuffer = NULL;
	}
	DepthBuffer = new ZBuffer(alpha_area);

	Show_Mouse();
}


/// <summary>
/// Draws a money readout on the dropship screen.
/// This routine restores the readout area from the clean background art before
/// printing, so that a shrinking number does not leave the tail of the previous
/// one behind.
/// </summary>
/// <param name="cost">The amount to print.</param>
/// <param name="maxcost">The largest amount that may ever be printed here. It decides how
/// much of the background has to be restored first.</param>
/// <param name="surface">The clean background art to restore the readout area from.</param>
/// <param name="title">The caption to print in front of the amount.</param>
/// <param name="center">Should the readout be centered about the draw point?</param>
void Draw_Cost(int cost, int maxcost, Surface & surface, Point2D const & drawpoint, char const * title, bool center, ConvertClass & drawer, Blitter const * blitter)
{
	static const int _width = 110;
	static const int _height = 15;

	static char cost_text[16];
	static char max_text[24];

	sprintf(cost_text, "%s%d", title, cost);
	sprintf(max_text, "%s%d", title, maxcost);

	int x1, x2;

	if (center) {
		x1 = drawpoint.X - GradFont6Ptr->String_Pixel_Width(cost_text) / 2;
		x2 = drawpoint.X - GradFont6Ptr->String_Pixel_Width(max_text) / 2;
	} else {
		x1 = drawpoint.X;
		x2 = drawpoint.X;
	}

	Rect source = Union(Rect(x1, drawpoint.Y, _width, _height), Rect(x2, drawpoint.Y, _width, _height));
	Rect clip = source;
	clip.X -= (HiddenSurface->Get_Width() - surface.Get_Width()) / 2;
	clip.Y -= (HiddenSurface->Get_Height() - surface.Get_Height()) / 2;
	Blit_Block(*HiddenSurface, drawer, surface, clip, Point2D(0, 0), source, NULL, blitter);

	TextPrintType type = TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW);
	if (center) {
		type = TextPrintType(type|TPF_CENTER);
	}

	Fancy_Text_Print(cost_text, *HiddenSurface, HiddenSurface->Get_Rect(), drawpoint, Fetch_Scheme_By_Name("Yellow"), TBLACK, type);
}


/// <summary>
/// Draws the unit description panel on the dropship screen.
/// This routine restores the panel from the clean background art and then, if the
/// player is hovering over a unit, prints its name, armament, armor and cost. The
/// text is printed in green when the unit may be taken along and red when it may
/// not.
/// </summary>
/// <param name="surface">The clean background art to restore the panel from.</param>
/// <param name="techtype">The unit to describe, or NULL to just clear the panel.</param>
/// <param name="allowed">May the player take this unit along?</param>
void Draw_Unit_Info(Surface *surface, ConvertClass *drawer, TechnoTypeClass *techtype, bool allowed)
{
	static const int _xpos = 450;
	static const int _ypos = 300;
	static const int _width = 190;
	static const int _height = 80;
	static const int _offset = 15;

	Rect rect1 = Rect(_xpos, _ypos, _width, _height);
	Point2D point = Point2D((HiddenSurface->Get_Width() - surface->Get_Width()) / 2, (HiddenSurface->Get_Height() - surface->Get_Height()) / 2);

	Blit_Block(*HiddenSurface, *drawer, *surface, rect1, Point2D(0,0), Rect(_xpos, _ypos, _width, _height) + point, NULL, drawer->Blitter_From_Flags(SHAPE_NORMAL));

	if (techtype != NULL) {
		char name[40];
		char cost[20];
		char armament[40];
		char armor[32];

		sprintf(name, "Name: %s", techtype->GivenName.c_str());

		if (allowed) {
			sprintf(cost, "Cost: %d", techtype->Raw_Cost());
		} else {
			sprintf(cost, "Cost: N/A");
		}

		if (techtype->Get_Weapon(0)->Weapon != NULL) {
			sprintf(armament, "Armament: %s", techtype->Get_Weapon(0)->Weapon->GivenName.c_str());
		} else {
			sprintf(armament, "Armament: NONE");
		}

		sprintf(armor, "Armor: %s", ArmorName[techtype->Armor]);

		ColorScheme *scheme;
		if (allowed) {
			scheme = Fetch_Scheme_By_Name("Green");
		} else {
			scheme = Fetch_Scheme_By_Name("Red");
		}

		Fancy_Text_Print(name, *HiddenSurface, HiddenSurface->Get_Rect(), Point2D(_xpos, _ypos) + point, scheme, TBLACK, TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW));
		Fancy_Text_Print(armament, *HiddenSurface, HiddenSurface->Get_Rect(), Point2D(_xpos, _ypos + _offset) + point, scheme, TBLACK, TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW));
		Fancy_Text_Print(armor, *HiddenSurface, HiddenSurface->Get_Rect(), Point2D(_xpos, _ypos + _offset * 2) + point, scheme, TBLACK, TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW));
		Fancy_Text_Print(cost, *HiddenSurface, HiddenSurface->Get_Rect(), Point2D(_xpos, _ypos + _offset * 3) + point, scheme, TBLACK, TextPrintType(TPF_6PT_GRAD|TPF_FULLSHADOW));
	}

}
