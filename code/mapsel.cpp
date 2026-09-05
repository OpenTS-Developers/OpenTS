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

/* $Header: /CounterStrike/MAPSEL.CPP 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : MAPSEL.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Barry W. Green                                               *
 *                                                                                             *
 *                   Start Date : April 17, 1995                                               *
 *                                                                                             *
 *                  Last Update : April 27, 1995   [BWG]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Bit_It_In -- Pixel fade graphic copy.                                                     *
 *   Map_Selection -- Starts the whole process of selecting next map to go to                  *
 *   Print_Statistics -- Prints statistics on country selected                                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "mapsel.h"

#include "_keyboar.h"
#include "_surface.h"
#include "addon.h"
#include "ccfile.h"
#include "convert.h"
#include "dbgprint.h"
#include "dsaudio.h"
#include "globals.h"
#include "houstype.h"
#include "keyboard.h"
#include "msanim.h"
#include "mschoice.h"
#include "msengine.h"
#include "msfont.h"
#include "msgbox.h"
#include "pcx.h"
#include "scenario.h"
#include "stimer.h"
#include "surface.h"
#include "theme.h"
#include "timer.h"

#include <cassert>


/// The binary confirms this was in the cpp as the vtable is inside this module
/// Moving it into a header would make the first thing that includes the header construct the vtable in that module

class MapSelect : public MSEngine {
	public:
		bool Presentation(ScenarioClass * scenario);
		bool Advance_Progression(ScenarioClass * scenario, char const * map_name);

	private:
		bool Init(ScenarioClass * scenario);
		void Deinit(void);

		void Play_Sound(char const * name);
		void Queue_Voice(char const * name, int delay);
		void Start_Voice(char const * name);
		void Stop_Voice(bool fade);

		virtual void Process_Idle(void);
		const char * Process_Input(MapStage * stage);

	private:
		int XOffset;
		int YOffset;

		MapChoice Choices;

		ConvertClass * AnimDrawer;
		ConvertClass * OverlayDrawer;

		MSFont * Font;
		Rect TextRect;

		char const * VoiceName;
		CDTimerClass<SystemTimerClass> VoiceTimer;
		AudioHandle VoiceHandle;
		Surface * ClickMap;
};


/***********************************************************************************************
 * Map_Selection -- Starts the whole process of selecting next map to go to                    *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/18/1996 BWG : Created.                                                                 *
 *=============================================================================================*/
const char * Map_Selection(ScenarioClass * scen)
{
	if (MapSelect().Presentation(scen) == false) {
		WWMessageBox().Process("Unable to initiate Map Selection!\n", TXT_OK);
		return(NULL);
	}

	DebugString("MapSelect: Scenario - %s, Stage - %ld\n", scen->ScenarioName, scen->Stage);

	return((strlen(scen->ScenarioName) == 0) ? NULL : scen->ScenarioName);
}


/// <summary>
/// Advances the campaign to a specific map.
/// This is the way into the map selection system when the next map is already known, such
/// as when a mission hands directly off to another. An error box is displayed if the map
/// is not one the campaign can reach from here.
/// </summary>
/// <param name="scenario">The scenario to advance. It is updated with the map named.</param>
/// <param name="map_name">Name of the scenario to advance to.</param>
/// <returns>Returns with the map name if the advance succeeded, otherwise NULL.</returns>
const char * Map_Select_Advance(ScenarioClass * scenario, const char * map_name)
{
	if (MapSelect().Advance_Progression(scenario, map_name) == false) {
		WWMessageBox().Process("Unable to initiate Map Selection!\n", TXT_OK);
		return(NULL);
	}

	return(map_name);
}


/// <summary>
/// Advances the campaign to a map without showing the selection screen.
/// This routine is used when the destination is already decided and only the bookkeeping
/// is wanted. The map must be one of the choices offered by the scenario's current stage.
/// </summary>
/// <param name="scen">The scenario to advance. It is updated with the map named.</param>
/// <param name="map_name">Name of the scenario to advance to.</param>
/// <returns>bool; Was the map found among the available choices?</returns>
bool MapSelect::Advance_Progression(ScenarioClass * scen, char const * map_name)
{
	if ((scen == NULL) || (map_name == NULL)) {
		return(false);
	}

	HouseTypeClass * house = HouseTypes[scen->PlayerHouse];
	if (house == NULL) {
		DebugString("MapSelect: Invalid house!\n");
		return(false);
	}

	DebugString("MapSelect: House %s\n", (const char*)house->IniName);

	if (Choices.Initialize((const char*)house->IniName) == false) {
		Choices.Deinit();
		DebugString("MapSelect: Unable to initialize choices!\n");
		return(false);
	}

	MapStage * stage = Choices.Find_Stage_By_ID(scen->Stage);

	if (stage == NULL) {
		Choices.Deinit();
		DebugString("MapSelect: Invalid stage!\n");
		return(false);
	}

	scen->Set_Scenario_Name("");

	for (int index = 0; index < stage->Get_Selection_Count(); index++) {
		MapSelection * selection = stage->Get_Selection(index);

		if (selection != NULL) {
			MapStage * sel_stage = Choices.Find_Stage_By_Name(selection->Get_Stage_Label());

			if ((sel_stage != NULL) && stricmp(sel_stage->Get_Scenario_Name(), map_name) == 0) {
				scen->Set_Scenario_Name(sel_stage->Get_Scenario_Name());
				scen->Stage = Choices.Get_Stage_ID(sel_stage);
				break;
			}
		}
	}

	Choices.Deinit();
	return(strlen(scen->ScenarioName) > 0);
}


/// <summary>
/// Runs the map selection screen.
/// This routine plays the map movie, brings on the overlays and targets, then hands
/// control to the player to choose where the campaign goes next.
/// </summary>
/// <param name="scenario">The scenario to advance. It is updated with the map chosen.</param>
/// <returns>bool; Was the map selection screen displayed?</returns>
bool MapSelect::Presentation(ScenarioClass * scenario)
{
	const char * selected = NULL;

	Rect rect;

	if (Init(scenario) == false) {
		return(false);
	}

	MapStage * map_stage = Choices.Find_Stage_By_ID(scenario->Stage);

	DebugString("MapSelect: Stage: %s\n", map_stage->Get_Stage_Label());

	if (map_stage->Get_Map_VQ_Name() != NULL) {

		if (Get_Required_Addon() == ADDON_FIRESTORM) {
			Theme.Play_Song(Theme.From_Name("FSMAP"));
		} else {
			Theme.Play_Song(Theme.From_Name("MAPS"));
		}

		Hide_Mouse();
		AlternateSurface->Fill(0);
		HiddenSurface->Fill(0);
		VisibleSurface->Blit_From(*HiddenSurface);

		MSAnim * anim = new MSVQAnim(map_stage->Get_Map_VQ_Name(), AlternateSurface, &Anims);
		Add_Animation(anim);

		int i;
		if (Choices.Anim_Entry_Count() > 0) {
			for (i = 0; i < Choices.Anim_Entry_Count(); i++) {
				MSAnimEntry * choice_anim = Choices.Get_Anim_Entry(i);

				if (choice_anim != NULL) {
					Add_Animation(new MSShapeAnim(choice_anim->Get_Filename(),
						choice_anim->Get_X_Pos() + XOffset, choice_anim->Get_Y_Pos() + YOffset,
						AnimDrawer, choice_anim->Get_Rate()));
				}
			}
		}

		for (i = 0; i < map_stage->Text_Entry_Count(); i++) {
			MSTextEntry * text = map_stage->Get_Text_Entry(i);

			if (text != NULL) {
				MSPrintAnim::Word_Wrap(text->Get_String(), Font, 640);
				Add_Animation(new MSPrintAnim(text->Get_String(), XOffset + text->Get_X_Pos(), YOffset + text->Get_Y_Pos(), Font, TextRect, text->Get_Start_Time()));
			}
		}

		Wait_For_Anim(anim);
		Wait_Delay(TIMER_SECOND);

		for (i = 0; i < map_stage->Overlay_Count(); i++) {
			anim = new MSOverlayAnim(map_stage->Get_Overlay_Name(i), XOffset, YOffset, OverlayDrawer, 5, &Anims);

			Play_Sound("Overlay");
			Add_Animation(anim);
			Wait_For_Anim(anim);
			Wait_Delay(3 * TIMER_SECOND / 4);
		}

		for (i = 0; i < map_stage->Target_Count(); i++) {
			Point2D target = map_stage->Get_Target(i);

			target.X += XOffset;
			target.Y += YOffset;

			anim = new MSFadeAnim("TARGET1.SHP", target.X, target.Y, AnimDrawer, 3, SHAPE_CENTER, &Anims);
			MSShapeAnim * shape_anim = new MSShapeAnim("TARGET2.SHP", target.X, target.Y, AnimDrawer, 5, true, (ShapeFlags_Type)(SHAPE_CENTER));

			shape_anim->Set_Stop_Frame(32);

			MapSelection * selection = map_stage->Get_Selection(i);

			if (selection != NULL) {
				selection->Set_Target_Anim(shape_anim);
			}

			Play_Sound("TargetFlyIn");
			Add_Animation(anim);
			Wait_For_Anim(anim);
			Add_Animation(shape_anim);
		}

		Show_Mouse();
		selected = Process_Input(map_stage);
		Hide_Mouse();

		Theme.Stop(true);

		AlternateSurface->Fill(0);
		HiddenSurface->Fill(0);
		VisibleSurface->Blit_From(*HiddenSurface);
		Show_Mouse();

		if (selected != NULL) {
			DebugString("Selected %s\n", selected);

			map_stage = Choices.Find_Stage_By_Name(selected);
			selected = map_stage->Get_Scenario_Name();

			if (selected != NULL) {
				scenario->Set_Scenario_Name(selected);
				scenario->Stage = Choices.Get_Stage_ID(map_stage);
			}
		}
	}

	Deinit();

	return(true);
}


/// <summary>
/// Prepares the map selection screen for display.
/// This routine loads the map choice data for the player's house, creates the drawers,
/// font and click map that the presentation needs, and centers the artwork on the screen.
/// A failure tidies up after itself before returning.
/// </summary>
/// <returns>bool; Was the map selection screen successfully prepared?</returns>
bool MapSelect::Init(ScenarioClass * scenario)
{
	AnimDrawer = NULL;
	OverlayDrawer = NULL;
	Font = NULL;
	ClickMap = NULL;

	if (scenario == NULL) {
		DebugString("MapSelect: Invalid Scenario pointer!\n");
		return(false);
	}

	HouseTypeClass * htype = HouseTypes[scenario->PlayerHouse];
	if (htype == NULL) {
		DebugString("MapSelect: Invalid house!\n");
		return(false);
	}

	DebugString("MapSelect: House %s\n", (const char*)htype->IniName);

	if (Choices.Initialize((const char*)htype->IniName) == false) {
		Choices.Deinit();
		DebugString("MapSelect: Unable to initialize choices!\n");
		return(false);
	}

	MapStage * map_stage = Choices.Find_Stage_By_ID(scenario->Stage);

	if (map_stage == NULL) {
		DebugString("MapSelect: Invalid stage!\n");
		Deinit();
		return(false);
	}

	AnimDrawer = Create_Drawer(Choices.Get_Anim_Palette_Name());

	if (AnimDrawer == NULL) {
		DebugString("MapSelect: Unable to create animation drawer!\n");
		Deinit();
		return(false);
	}

	OverlayDrawer = Create_Drawer("MSOVRLY.PAL");

	if (OverlayDrawer == NULL) {
		DebugString("MapSelect: Unable to create overlay drawer!\n");
		Deinit();
		return(false);
	}

	CCFileClass file(map_stage->Get_Click_Map_Name());
	ClickMap = Read_PCX_File(file);
	file.Close();

	if (ClickMap == NULL) {
		DebugString("MapSelect: Unable to create clickmap!\n");
		Deinit();
		return(false);
	}

	Font = new MSFont();

	if (Font == NULL) {
		DebugString("MapSelect: Unable to create font!\n");
		Deinit();
		return(false);
	}

	XOffset = ((HiddenSurface->Get_Width() - 640) / 2);
	YOffset = ((HiddenSurface->Get_Height() - 400) / 2);

	TextRect = *Choices.Get_Text_Rect();
	TextRect.X += XOffset;
	TextRect.Y += YOffset;

	int i = 0;

	while (map_stage != NULL) {
		MSPrintAnim::Word_Wrap(map_stage->Get_Description(), Font, TextRect.Width);
		i++;
		map_stage = Choices.Find_Stage_By_ID((unsigned short)i);
	}

	VoiceName = NULL;
	VoiceTimer = 0;
	VoiceHandle.Clear();

	return(true);
}


/// <summary>
/// Frees the resources the map selection screen allocated.
/// This routine may be called from a half finished initialization -- whatever was never
/// created is simply skipped over.
/// </summary>
void MapSelect::Deinit(void)
{
	if (ClickMap != NULL) {
		delete ClickMap;
		ClickMap = NULL;
	}

	if (Font != NULL) {
		delete Font;
		Font = NULL;
	}

	if (OverlayDrawer != NULL) {
		delete OverlayDrawer;
		OverlayDrawer = NULL;
	}

	if (AnimDrawer != NULL) {
		delete AnimDrawer;
		AnimDrawer = NULL;
	}

	Choices.Deinit();
}


/// <summary>
/// Performs the idle processing for the map selection screen.
/// The animation engine calls this routine whenever it is waiting, which is where a
/// queued voice over gets started once its delay has expired.
/// </summary>
void MapSelect::Process_Idle(void)
{
	if (VoiceName != NULL && VoiceTimer == 0) {
		Start_Voice(VoiceName);
	}
}


/// <summary>
/// Handles the player's interaction with the map.
/// This routine is the input loop of the map selection screen. As the cursor passes over
/// the click map, the region beneath it is highlighted, its description is printed and its
/// voice over is queued. The loop ends when the player clicks on a region.
/// </summary>
/// <param name="stage">The stage whose regions are available to be selected.</param>
/// <returns>Returns with the label of the region chosen, or NULL if none was.</returns>
const char * MapSelect::Process_Input(MapStage * stage)
{
	KeyNumType key;
	const char * selection = NULL;
	int last_mouse_x = -1;
	int last_mouse_y = -1;
	int mouse_x;
	int mouse_y;
	int last_click_index = -1;
	int click_index;
	MSAnim * anim = NULL;

	if (stage != NULL && ClickMap != NULL) {

		Keyboard->Clear();
		key = KN_NONE;

		do {
			Wait_For_Focus();

			if (Keyboard->Check()) {
				key = Keyboard->Get();
				if (key == KN_LMOUSE) {

					mouse_x = (Keyboard->MouseQX - XOffset);
					mouse_y = (Keyboard->MouseQY - YOffset);

					if ((mouse_x >= 0) && (mouse_x < ClickMap->Get_Width())
							&& (mouse_y >= 0) && (mouse_y < ClickMap->Get_Height())) {

						click_index = ClickMap->Get_Pixel(Point2D(mouse_x, mouse_y));

						selection = stage->Find_Selection_By_Index(click_index);
					}
				}
			} else {

				mouse_x = (Get_Mouse_X() - XOffset);
				mouse_y = (Get_Mouse_Y() - YOffset);

				if ((mouse_x != last_mouse_x) || (mouse_y != last_mouse_y)) {

					last_mouse_x = mouse_x;
					last_mouse_y = mouse_y;

					if ((mouse_x >= 0) && (mouse_x < ClickMap->Get_Width()) && (mouse_y >= 0) && (mouse_y < ClickMap->Get_Height())) {

						click_index = ClickMap->Get_Pixel(Point2D(mouse_x, mouse_y));

						if (click_index != last_click_index) {

							Stop_Voice(true);

							Remove_Anim(anim);

							HiddenSurface->Blit_From(TextRect, *AlternateSurface, TextRect);
							Add_Update_Rect(TextRect);

							if (last_click_index == 0) {
								Play_Sound("MouseOnMap");
							} else if (click_index == 0) {
								Play_Sound("MouseOffMap");
							}

							MapSelection * map_sel;
							MSShapeAnim * target_anim;

							const char * selected = stage->Find_Selection_By_Index(last_click_index);

							if (selected != NULL) {
								Play_Sound("ExitRegion");
								map_sel = stage->Find_Selection_By_Name(selected);

								target_anim = map_sel->Get_Target_Anim();

								if (target_anim != NULL) {
									target_anim->Set_Frame(0);
									target_anim->Set_Start_Frame(0);
									target_anim->Set_Stop_Frame(31);
								}
							}

							selected = stage->Find_Selection_By_Index(click_index);

							if (selected != NULL) {
								Play_Sound("EnterRegion");
								map_sel = stage->Find_Selection_By_Name(selected);

								target_anim = map_sel->Get_Target_Anim();

								if (target_anim != NULL) {
									target_anim->Set_Frame(32);
									target_anim->Set_Start_Frame(32);
									target_anim->Set_Stop_Frame(0xFFFFFFFF);
								}

								const MapStage * over_stage = Choices.Find_Stage_By_Name(selected);

								if (over_stage != NULL) {

									if (over_stage->Get_Description() != NULL) {
										anim = new MSPrintAnim(over_stage->Get_Description(), TextRect.X, TextRect.Y, Font, TextRect);

										Add_Animation(anim);
									}

									if (over_stage->Get_Voiceover() != NULL) {
										Queue_Voice(over_stage->Get_Voiceover(), (TIMER_SECOND / 2));
									}
								}
							}

							last_click_index = click_index;
						}
					}
				}
			}

			Wait_Delay(1);
		} while (selection == NULL);

		Stop_Voice(false);
		Play_Sound("Click");
	}

	Keyboard->Clear();

	return(selection);
}


/// <summary>
/// Plays one of the map selection sound effects.
/// The effects are looked up by name in the map choice data, so a house that does not
/// bother to define one simply stays quiet.
/// </summary>
/// <param name="name">Name of the sound effect entry to play.</param>
void MapSelect::Play_Sound(char const * name)
{
	if (name != NULL) {
		MSSfxEntry * sfx = Choices.Find_Sound(name);

		if (sfx != NULL) {
			sfx->Play();
		}
	}
}


/// <summary>
/// Queues a voice over to start after a delay.
/// This routine is used when the mouse comes to rest over a region, so that merely
/// dragging the cursor across the map does not set off a burst of speech.
/// </summary>
/// <param name="name">Name of the voice over sample to stream.</param>
/// <param name="delay">The delay, in timer ticks, before the voice over starts.</param>
void MapSelect::Queue_Voice(char const * name, int delay)
{
	Stop_Voice(true);
	VoiceName = name;
	VoiceTimer = delay;
}


/// <summary>
/// Starts the map description voice over playing.
/// The idle handler calls this routine once the queued voice over's delay has run out.
/// Whatever voice over is still playing gets stopped first.
/// </summary>
/// <param name="name">Name of the voice over sample to stream.</param>
void MapSelect::Start_Voice(char const * name)
{
	if (VoiceHandle.Is_Valid()) {
		AudioEngine.Stop_Stream(VoiceHandle);
	}

	VoiceHandle = AudioEngine.Open_Stream(name, AUDIO_GROUP_SPEECH, 1.0f, false);

	if (VoiceHandle.Is_Valid())
		VoiceName = NULL;

	VoiceTimer = 0;
}


/// <summary>
/// Stops the map description voice over.
/// This routine is used when the mouse leaves a region, and when the player commits to a
/// selection. A voice over that was merely queued is discarded along with it.
/// </summary>
/// <param name="fade">Should the sample be faded out rather than cut off?</param>
void MapSelect::Stop_Voice(bool fade)
{
	if (VoiceHandle.Is_Valid()) {
		if (fade == true) {
			VoiceHandle.Fade(1000 / 3);
		} else {
			AudioEngine.Stop_Stream(VoiceHandle);
		}

		VoiceHandle.Clear();
	}

	VoiceName = NULL;
	VoiceTimer = 0;
}
