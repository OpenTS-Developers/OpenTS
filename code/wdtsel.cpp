/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"
#include "language/language.h"

#include "_keyboar.h"
#include "_surface.h"
#include "ccfile.h"
#include "convert.h"
#include "data.h"
#include "globals.h"
#include "ini.h"
#include "keyboard.h"
#include "mschoice.h"
#include "msfont.h"
#include "newmenu.h"
#include "netshare.h"
#include "ownrdraw.h"
#include "pcx.h"
#include "session.h"
#include "theme.h"
#include "vector.h"
#include "wdtnet.h"
#include "worlddom.h"

typedef VectorClass<Point2D> POINT2D_LIST;

typedef VectorClass<Rect> RECT_LIST;


using namespace WorldDominationTour;


/// <summary>
/// Creates the world domination territory selection screen.
/// This routine assembles the whole screen out of the map database -- the voice overs, the
/// sound effects, the layout, the artwork and the region currently being fought over -- and
/// then sets it running.
/// </summary>
/// <param name="campaign">The campaign whose current state the screen presents.</param>
/// <param name="vq_anim">Should the region backdrop play as a movie rather than a still
/// image?</param>
Selection::Selection(Campaign * campaign, bool vq_anim) :
	MSEngine(),
	TourCampaign(campaign),
	SelectedConflict(),
	VoicePlayer(campaign->PlayerFaction),
	Brokeout(false),
	LastPoint(-1, -1),
	StartTick(campaign->Properties.Get_Current_Tick()),
	Position(0, 0),
	LastHoverZone(-1),
	WorldMap(campaign->Properties.Get_Map_ID()),
	MapOrigin(0, 0),
	RegionDrawer(NULL),
	TerritoryDrawer(NULL),
	LogoDrawer(NULL),
	RegionAnim(NULL),
	ClickMapImage(NULL),
	HoveredTerritoryID(-1),
	MousedTerritory(NULL),
	TargetDrawer(NULL),
	ZoomingTarget(NULL),
	ThrobbingTarget(NULL),
	ThrobbingTargetDividingFrame(32),
	CancelButtonRectangle(RECT_NONE),
	CancelButtonAnim(NULL),
	CancelButtonHoverAnim(NULL),
	CancelButtonHovered(false),
	TextRect(500, 100, 600, 350),
	TextAnim(NULL),
	Font(new MSFont(false)),
	CurrentState(),
	ThemeName(NULL),
	SfxEntries()
{
	Add_Animation(WDT_New_Voiced_Animation(VoicePlayer));

	CCFileClass file("WDTMAP.INI");
	INIClass ini;
	ini.Load(file);

	Init_Voices(ini, "VoiceOvers");
	Init_Sounds(ini, "SOUNDS");

	char entry[4];

	char layout[64];
	sprintf(entry, "%d", campaign->Get_Campaign_Properties().Get_Layout());
	ini.Get_String("LAYOUTS", entry, "", layout, sizeof(layout));

	Init_Dimensions(ini, layout);

	char region[64];
	sprintf(entry, "%d", campaign->Get_Campaign_Properties().Get_Map_ID());
	ini.Get_String("MAPS", entry, "", region, sizeof(region));

	Init_Art(ini, layout);
	Init_Regions(ini, region, vq_anim);

	Start();
}


/// <summary>
/// Shuts the selection screen down and releases everything it owns.
/// </summary>
Selection::~Selection(void)
{
	End();

	MousedTerritory = NULL;

	Remove_Target_Anims();
	Remove_Target_Anims2();

	for (MSSfxEntry * entry : SfxEntries) {
		delete entry;
	}

	delete RegionDrawer;
	delete TerritoryDrawer;
	delete LogoDrawer;
	delete ClickMapImage;
	delete TargetDrawer;
	delete ZoomingTarget;
	delete ThrobbingTarget;
	delete Font;

	if (ThemeName != NULL) {
		free(ThemeName);
	}
}


/// <summary>
/// Fetches the voice over settings for the selection screen.
/// Along with the voice list itself, this routine picks up the score thresholds that
/// decide how enthusiastic the announcer gets about the campaign so far.
/// </summary>
/// <param name="ini">The map database to fetch the voice overs from.</param>
/// <param name="section">The section that describes the voice overs.</param>
void Selection::Init_Voices(INIClass const & ini, const char * section)
{
	VoicePlayer.Init(ini, section);
	WinningThreshold = ini.Get_Int(section, "WinningThreshold", WinningThreshold);
	MegaWinningThreshold = ini.Get_Int(section, "MegaWinningThreshold", MegaWinningThreshold);
	LosingThreshold = ini.Get_Int(section, "LosingThreshold", LosingThreshold);
	MegaLosingThreshold = ini.Get_Int(section, "MegaLosingThreshold", MegaLosingThreshold);
}

/// <summary>
/// Creates the sound effect table for the selection screen.
/// Every entry in the section becomes a named sound effect that the screen can ask for by
/// name later on. The music theme for the screen is chosen here as well.
/// </summary>
/// <param name="ini">The map database to fetch the sound list from.</param>
/// <param name="section">The section that lists the sound effects.</param>
void Selection::Init_Sounds(INIClass const & ini, const char * section)
{
	char buffer[64];

	ThemeName = strdup("MAPS");

	int count = ini.Entry_Count(section);
	for (int i = 0; i < count; i++) {
		char const * entry = ini.Get_Entry(section, i);
		if (entry != NULL) {
			if (ini.Get_String(section, entry, NULL, buffer, sizeof(buffer)) > 0) {
				MSSfxEntry * sfx = new MSSfxEntry(entry, buffer);
				if (sfx->Get_Name() == NULL) {
					delete sfx;
				} else {
					Add_Sfx_Entry(sfx);
				}
			}
		}
	}
}


/// <summary>
/// Fetches the screen layout for the selection screen.
/// The screen is centered within the display and the map, the text panel and the cancel
/// button are then positioned relative to it.
/// </summary>
/// <param name="ini">The map database to fetch the layout from.</param>
/// <param name="section">The section that describes this screen layout.</param>
void Selection::Init_Dimensions(INIClass const & ini, const char * section)
{
	Point2D size(640, 400);
	size = ini.Get_Point(section, "Size", size);
	Position = Point2D((HiddenSurface->Get_Width() - size.X) / 2, (HiddenSurface->Get_Height() - size.Y) / 2);
	MapOrigin = ini.Get_Point(section, "MapOrigin", MapOrigin);
	TextRect = ini.Get_Rect(section, "TextRect", TextRect);
	TextRect += Position;
	CancelButtonRectangle = ini.Get_Rect(section, "CancelButtonRectangle", CancelButtonRectangle);
	CancelButtonRectangle += Position;
}


/// <summary>
/// Creates the campaign map and everything that is drawn over it.
/// This routine brings in the region backdrop, the click map that turns a mouse position
/// into a territory, the territories themselves and the cancel button artwork -- all of it
/// chosen to suit the player's faction.
/// </summary>
/// <param name="ini">The map database to fetch the region artwork from.</param>
/// <param name="section">The section that describes this region.</param>
/// <param name="vq_anim">Should the backdrop play as a movie rather than a still image?</param>
void Selection::Init_Regions(INIClass const & ini, const char * section, bool vq_anim)
{
	char const * faction_name = TourCampaign->PlayerFaction == 3 ? "NOD" : "GDI";

	char region_name[64];
	ini.Get_String(section, "RegionName", NULL, region_name, sizeof(region_name));

	char buffer[64];
	sprintf(buffer, "%s%s.vqa", faction_name, region_name);
	if (vq_anim) {
		RegionAnim = new MSVQAnim(buffer, AlternateSurface, &Anims, true);
	} else {
		RegionAnim = new MSPCXAnim(buffer, &Anims, true);
	}

	Add_Animation(RegionAnim);

	sprintf(buffer, "%s%s.pal", faction_name, region_name);
	RegionDrawer = Create_Drawer(buffer);
	TerritoryDrawer = Create_Drawer(buffer);

	ini.Get_String(section, "ClickMap", 0, buffer, sizeof(buffer));
	CCFileClass click_map_file(buffer);
	ClickMapImage = Read_PCX_File(click_map_file);
	click_map_file.Close();

	char const * faction_prefix = TourCampaign->PlayerFaction == 3 ? "nr" : "gr";

	char region_prefix[64];
	ini.Get_String(section, "RegionPrefix", NULL, region_prefix, sizeof(region_prefix));

	char territory_name[64];
	sprintf(territory_name, "%s%s", region_prefix, faction_prefix);

	WorldMap.Add_Territory(territory_name, ini, section, TourCampaign->PlayerFaction, *this, TerritoryDrawer, &Anims, Position);

	char background_name[64];
	sprintf(background_name, "%sBack.PCX", region_prefix);
	CancelButtonAnim = new MSPCXAnim(background_name, &Anims, CancelButtonRectangle.TopLeft, true);
	CancelButtonAnim->Set_Active(false);
	Add_Animation(CancelButtonAnim);

	sprintf(background_name, "%sBackh.PCX", region_prefix);
	CancelButtonHoverAnim = new MSPCXAnim(background_name, &Anims, CancelButtonRectangle.TopLeft, true);
	CancelButtonHoverAnim->Set_Active(false);
	Add_Animation(CancelButtonHoverAnim);
}


/// <summary>
/// Creates the decorative artwork for the selection screen.
/// </summary>
/// <param name="ini">The map database to fetch the artwork from.</param>
/// <param name="section">The section that describes this screen layout.</param>
void Selection::Init_Art(INIClass const & ini, const char * section)
{
	Init_Logo(ini, section);
	Init_Target(ini, section);
}


/// <summary>
/// Creates the faction logo animation for the selection screen.
/// The artwork is chosen to suit the player's faction and placed wherever the screen
/// layout asks for it.
/// </summary>
/// <param name="ini">The map database to fetch the logo position from.</param>
/// <param name="section">The section that describes this screen layout.</param>
void Selection::Init_Logo(INIClass const & ini, const char * section)
{
	char buffer[64];
	char const * faction_name = TourCampaign->PlayerFaction == 3 ? "NOD" : "GDI";

	sprintf(buffer, "%sLogo.PAL", faction_name);
	LogoDrawer = Create_Drawer(buffer);

	sprintf(buffer, "%sLogo.SHP", faction_name);

	Point2D position(0,0);
	position = ini.Get_Point(section, "Logo", position);
	position += Position;

	Add_Animation(new MSShapeAnim(buffer, position.X, position.Y, LogoDrawer, 5));
}


/// <summary>
/// Fetches the target marker artwork for the player's faction.
/// This routine picks up the zooming and throbbing target animations, along with the
/// palette they draw through, that mark a conflict on the campaign map.
/// </summary>
/// <param name="ini">The map database to fetch the artwork names from.</param>
/// <param name="section">The section that describes this screen layout.</param>
void Selection::Init_Target(INIClass const & ini, const char * section)
{
	char buffer[64];
	char buffer2[64];
	char const * faction_name = TourCampaign->PlayerFaction == 3 ? "NOD" : "GDI";

	sprintf(buffer2, "%sZoomingTarget", faction_name);
	if (ini.Get_String(section, buffer2, NULL, buffer, sizeof(buffer)) > 0) {
		ZoomingTarget = new MSAnimEntry(buffer);
	}

	sprintf(buffer2, "%sTargetPalette", faction_name);
	ini.Get_String(section, buffer2, NULL, buffer, sizeof(buffer));
	TargetDrawer = Create_Drawer(buffer);

	sprintf(buffer2, "%sThrobbingTarget", faction_name);
	ini.Get_String(section, buffer2, NULL, buffer, sizeof(buffer));
	ThrobbingTarget = new MSAnimEntry(buffer);

	sprintf(buffer2, "%sThrobbingTargetDividingFrame", faction_name);
	ThrobbingTargetDividingFrame = ini.Get_Int(section, buffer2, 32);
}


/// <summary>
/// Fetches a pixel from the territory click map.
/// This routine is used when translating a mouse position into a territory. A point that
/// falls outside the click map picks up the default value instead of reading out of bounds.
/// </summary>
/// <param name="surface">The click map to sample.</param>
/// <param name="point">The point to sample, relative to the click map.</param>
/// <param name="defval">The value to use for a point outside the click map.</param>
/// <returns>Returns with the click map color at the point specified.</returns>
int Get_Click_Map_Pixel(Surface * surface, Point2D const & point, int defval)
{
	return(point.X >= 0 && point.X < surface->Get_Width() && point.Y >= 0 && point.Y < surface->Get_Height() ? surface->Get_Pixel(point) : defval);
}


/// <summary>
/// Runs the territory selection screen until the player commits or backs out.
/// This routine owns the mouse loop for the campaign map. It raises the cancel button,
/// routes clicks and hover movement to the map, and does not come back until the player
/// has either picked a territory or pressed cancel.
/// </summary>
/// <returns>Returns with the territory picked. Otherwise, NULL is returned.</returns>
Territory * Selection::Pick_Territory(void)
{
	CancelButtonAnim->Set_Active(true);
	Set_Text(TourCampaign->Properties.Get_Long_Desc());
	bool canceled = false;
	bool selected = false;

	Keyboard->Clear();

	while (!canceled) {
		Wait_For_Focus();
		Point2D point(MouseCursor->Get_Mouse_X(), MouseCursor->Get_Mouse_Y());

		if (Keyboard->Check() != KN_NONE) {
			int key = Keyboard->Get();
			if (key & KN_LMOUSE) {
				if ((key & WWKEY_RLS_BIT) != 0) {
					if (CancelButtonRectangle.Is_Point_Within(point)) {
						Play_SFX("Canceled");
						canceled = true;
					}
				} else {
					Handle_Click(point, selected);
				}
			}

		/*
		 * The hover update only runs on frames with no pending keyboard event:
		 * every path through the event handling above jumps past this block.
		 */
		} else if (point != LastPoint) {
			LastPoint = point;
			int over_cancel = CancelButtonRectangle.Is_Point_Within(point);

			int zone = over_cancel ? 2 : 1;

			if (LastHoverZone != zone) {

				switch (LastHoverZone) {

				case 1:
					On_Map_Leave();
					break;

				case 2:
					On_Cancel_Leave();
					break;
				}
			}

			switch (zone) {

			case 1:
				if (LastHoverZone != 1) {
					On_Map_Enter();
				}
				Update_Hovered_Territory(point - MapOrigin - Position);
				break;

			case 2:
				if (LastHoverZone != 2) {
					On_Cancel_Enter();
				}
				break;
			}

			LastHoverZone = zone;
		}

		Wait_Delay(1);
		if (canceled) {
			break;
		}
		if (selected) {
			return(MousedTerritory);
		}
	}

	if (selected) {
		return(MousedTerritory);
	}

	return(0);
}


/// <summary>
/// Handles a mouse click on the territory selection screen.
/// This routine routes the click either to the cancel button or to the campaign map,
/// where it becomes an attempt to pick a territory.
/// </summary>
/// <param name="point">The screen position that was clicked.</param>
/// <param name="selected">Set to true if the click landed on a selectable territory.</param>
void Selection::Handle_Click(Point2D const & point, bool & selected)
{
	switch (CancelButtonRectangle.Is_Point_Within(point) ? 2 : 1) {

		case 1:
			selected = Select_Territory_At(point - MapOrigin - Position);
			if (selected) {
				Play_SFX("ConflictSelected");
			}
			break;

		case 2:
			On_Cancel_Enter();
			break;

	}
}


/// <summary>
/// Selects whichever territory lies beneath the point.
/// </summary>
/// <param name="point">The point to look up, relative to the campaign map.</param>
/// <returns>bool; Is there a conflict waiting on the territory selected?</returns>
bool Selection::Select_Territory_At(Point2D const & point)
{
	Update_Hovered_Territory(point);

	Territory * territory = Find_Territory_By_ID(WorldMap.Territories, HoveredTerritoryID);
	return(Select_Territory(territory));
}


/// <summary>
/// Tracks which territory the mouse is hovering over.
/// This routine reads the territory out of the click map and, when it differs from the
/// one already under the mouse, selects the new one.
/// </summary>
/// <param name="point">The mouse position, relative to the campaign map.</param>
void Selection::Update_Hovered_Territory(Point2D const & point)
{
	int pixel = Get_Click_Map_Pixel(ClickMapImage, point, -1);

	if (pixel != HoveredTerritoryID) {
		Territory * territory = Find_Territory_By_ID(WorldMap.Territories, pixel);
		HoveredTerritoryID = pixel;
		Select_Territory(territory);
	}
}


/// <summary>
/// Advances the campaign map to a new state.
/// This routine hands each territory whose ownership has changed over to its new owner,
/// playing the occupation sound as it goes, and then runs the zoom and throb markers
/// over the territories that are up for grabs.
/// </summary>
/// <param name="state">The campaign state the map is to be brought to.</param>
/// <param name="show_targets">Should the disputed territories be marked as targets?</param>
/// <param name="silent">Should the occupation sound effects be suppressed?</param>
void Selection::Do_Target_Anim(State const & state, bool show_targets, bool silent)
{
	/*
	 * Tear down any animations left over from the previous cycle transition.
	 */
	for (Territory * teardown : WorldMap.Territories) {
		MSAnim * anim = teardown->Remove_Target_Anim();
		if (anim != NULL) {
			Remove_Anim(anim);
		}
	}

	MS_ANIM_LIST zoom_anims;
	MS_ANIM_LIST throb_anims;

	/*
	 * Walk each territory and advance any whose state has changed, playing the
	 * matching occupation sound effect. When requested, queue the zoom/throb
	 * target animations for newly disputed territories.
	 */
	for (Territory * territory : WorldMap.Territories) {
		unsigned id = territory->ID;
		int old_state = CurrentState.Get_Territory_State(id);
		int new_state = ((State &)state).Get_Territory_State(id);

		if (new_state != old_state) {
			Check_For_Breakout();

			territory->Set_Owner_State(new_state, *this, !Brokeout || silent);

			if (!silent && !Brokeout) {
				switch (new_state) {

				case 3:
					Play_SFX("NODOccupy");
					break;

				case 2:
					Play_SFX("GDIOccupy");
					break;

				case 1:
					Play_SFX("Disputed");
					break;
				}
			}
		}

		if (show_targets) {
			if (new_state == 1) {
				Point2D target = Position + MapOrigin + territory->Target;

				if (ZoomingTarget != NULL) {
					MSFadeAnim * fade = new MSFadeAnim(ZoomingTarget->Get_Filename(), target.X, target.Y, TargetDrawer, 3, SHAPE_CENTER, &Anims);
					zoom_anims.Add(fade);
				}

				MSShapeAnim * shape = new MSShapeAnim(ThrobbingTarget->Get_Filename(), target.X, target.Y, TargetDrawer, 5, true, SHAPE_CENTER);
				shape->Set_Stop_Frame(ThrobbingTargetDividingFrame - 1);
				shape->Set_Start_Frame(0);
				shape->Set_Frame(0);
				territory->Set_Target_Anim(shape);
				throb_anims.Add(shape);
			}
		}
	}

	/*
	 * Play the queued target animations. On abort, discard the zoom animations;
	 * otherwise play the zoom SFX and wait for each to complete.
	 */
	MSAnim ** zoom_iter = zoom_anims.begin();
	for (MSAnim * throb : throb_anims) {
		Check_For_Breakout();

		if (zoom_iter != zoom_anims.end()) {
			if (Brokeout) {
				MSAnim * anim = *zoom_iter;
				if (anim != NULL) {
					delete anim;
				}
			} else {
				Play_SFX("TargetZoom");
				Add_Animation(*zoom_iter);
				Wait_For_Anim(*zoom_iter, 300000);
			}
			zoom_iter++;
		}

		Add_Animation(throb);
	}

	CurrentState = state;
}


/// <summary>
/// Makes a territory the one under the player's attention.
/// This routine takes the highlight and the pending conflict away from whichever
/// territory held them, highlights the new one, and replaces the screen caption with
/// its name, description, and game options. Passing NULL merely clears the selection.
/// </summary>
/// <param name="territory">The territory to select, or NULL to select none at all.</param>
/// <returns>bool; Is there a conflict waiting on the selected territory?</returns>
bool Selection::Select_Territory(Territory * territory)
{
	bool was_set = SelectedConflict.Is_Territory_Set();

	if (territory == MousedTerritory) {
		return(was_set);
	}

	if (MousedTerritory != NULL) {
		if (SelectedConflict.Is_Territory_Set()) {
			SelectedConflict = Conflict(NULL);
		}
		MousedTerritory->Set_Highlight(false, *this);
		MSShapeAnim * anim = (MSShapeAnim *)MousedTerritory->TargetAnim;
		if (anim != NULL) {
			anim->Set_Stop_Frame(ThrobbingTargetDividingFrame - 1);
			anim->Set_Start_Frame(0);
			anim->Set_Frame(0);
		}
	}

	MousedTerritory = territory;

	if (territory == NULL) {
		Set_Text(TourCampaign->Properties.Get_Long_Desc(), 0);
		VoicePlayer.Queue(Voices::VOICE_TERRITORY_SELECT, false, 30);
		return(was_set);
	}

	bool found = false;
	char * description = NULL;

	int id = territory->ID;
	int territory_state = CurrentState.Get_Territory_State(id);
	int outcome = 2;
	if (territory_state >= 2 && territory_state <= 3) {
		outcome = TourCampaign->PlayerFaction != territory_state;
	}
	VoicePlayer.Queue(Voices::VOICECAT_TERRITORY, (Outcome)outcome, false, false, 30);

	MousedTerritory->Set_Highlight(true, *this);

	char buffer[512];

	if (TourCampaign->Properties.Get_Current_Tick() == StartTick) {
		Conflict * conflict = TourCampaign->FindConflict(MousedTerritory->ID);

		if (conflict != NULL) {
			found = true;
			SelectedConflict = *conflict;
			strcpy(buffer, "\n");
			conflict->Process_Game_Options(&buffer[1], 511);
			description = buffer;
			MSShapeAnim * anim = (MSShapeAnim *)territory->TargetAnim;
			if (anim != NULL) {
				anim->Set_Stop_Frame(-1);
				anim->Set_Start_Frame(ThrobbingTargetDividingFrame);
				anim->Set_Frame(ThrobbingTargetDividingFrame);
			}
		}
	}

	char dest[256];
	sprintf(dest, Fetch_String(TXT_WDT_FORMAT_TWO_LINES), MousedTerritory->Name, MousedTerritory->Description);
	if (description != NULL) {
		unsigned int len = strlen(dest);
		if ((int)(256 - len) > 0) {
			strncat(dest, buffer, 256 - len);
		}
	}
	Set_Text(dest, 0);
	return(found);
}


/// <summary>
/// Removes the target marker animation from every territory on the map.
/// </summary>
void Selection::Remove_Target_Anims(void)
{
	for (Territory * territory : WorldMap.Territories) {
		MSAnim * anim = territory->Remove_Target_Anim();
		if (anim != NULL) {
			Remove_Anim(anim);
		}
	}
}


/// An exact duplicate of Remove_Target_Anims. Both are called by the destructor.

/// <summary>
/// Removes the target marker animation from every territory on the map.
/// </summary>
void Selection::Remove_Target_Anims2(void)
{
	for (Territory * territory : WorldMap.Territories) {
		MSAnim * anim = territory->Remove_Target_Anim();
		if (anim != NULL) {
			Remove_Anim(anim);
		}
	}
}


/// <summary>
/// Handles the mouse entering the campaign map.
/// </summary>
void Selection::On_Map_Enter(void)
{
	Play_SFX("MapEnter");
}


/// <summary>
/// Handles the mouse leaving the campaign map.
/// This routine drops the territory highlight along with the caption that went with it.
/// </summary>
void Selection::On_Map_Leave(void)
{
	Play_SFX("MapLeave");
	HoveredTerritoryID = -1;
	Select_Territory(NULL);
}


/// <summary>
/// Handles the mouse entering the cancel button.
/// This routine swaps the button over to its highlighted animation, repaints it, and
/// plays the accompanying sound effect.
/// </summary>
void Selection::On_Cancel_Enter(void)
{
	if (!CancelButtonHovered) {
		CancelButtonHovered = true;
		Play_SFX("MouseCancel");
		if (CancelButtonAnim != NULL) {
			CancelButtonAnim->Set_Active(false);
		}
		if (CancelButtonHoverAnim != NULL) {
			CancelButtonHoverAnim->Set_Active(true);
		}
		Restore_Anims(CancelButtonRectangle);
		Restore_And_Advance();
	}
}


/// <summary>
/// Handles the mouse leaving the cancel button.
/// This routine puts the button back to its idle animation and repaints it.
/// </summary>
void Selection::On_Cancel_Leave(void)
{
	if (CancelButtonHovered) {
		CancelButtonHovered = false;
		if (CancelButtonAnim != NULL) {
			CancelButtonAnim->Set_Active(true);
		}
		if (CancelButtonHoverAnim != NULL) {
			CancelButtonHoverAnim->Set_Active(false);
		}
		Restore_Anims(CancelButtonRectangle);
		Restore_And_Advance();
	}
}

/// <summary>
/// Brings the campaign map up to date and marks the available targets.
/// This routine runs before the player is allowed to choose. It replays whatever has
/// happened since the last visit, plants the target markers on the territories that are
/// up for grabs, and writes the campaign back out again.
/// </summary>
void Selection::Do_Target_Selection(void)
{
	Set_Text();

	int current_tick = TourCampaign->Properties.Get_Current_Tick();
	int previous_tick = TourCampaign->PreviousTick;

	bool silent = true;

	if (current_tick - previous_tick > 8) {
		previous_tick = current_tick - 8;
	}

	if (previous_tick != current_tick) {
		silent = !Present_History(previous_tick, current_tick);
	} else {
		bool mega;
		Outcome outcome = Get_Outcome(current_tick, mega);
		VoicePlayer.Queue(Voices::VOICECAT_STATUS, outcome, mega, true, 0);
	}

	Set_Text();
	Check_For_Breakout();

	Do_Target_Anim(TourCampaign->CycleHistory.Get_State(current_tick), true, silent);
	TourCampaign->Write_INI();
}


/// <summary>
/// Determines how well the player stands at a given campaign cycle.
/// This routine is used to pick the commentary and the caption that match the player's
/// share of the map.
/// </summary>
/// <param name="tick">The campaign cycle to judge.</param>
/// <param name="mega">Set to true when the standing is lopsided enough to deserve the
/// emphatic commentary rather than the ordinary one.</param>
/// <returns>Returns with the outcome -- win, lose, or draw.</returns>
Outcome Selection::Get_Outcome(int tick, bool & mega)
{
	State state = TourCampaign->CycleHistory.Get_State(tick);
	int faction = TourCampaign->PlayerFaction;
	int owned = state.Count_Owned_Territories(faction);

	if (owned <= LosingThreshold) {
		mega = owned <= MegaLosingThreshold;
		return(OUTCOME_LOSE);
	}

	if (owned >= WinningThreshold) {
		mega = owned >= MegaWinningThreshold;
		return(OUTCOME_WIN);
	}

	mega = false;
	return(OUTCOME_DRAW);
}


/// <summary>
/// Animates the campaign map across a range of cycles.
/// This routine steps the map from one cycle to the next, captioning each with the day
/// it stands for and pausing between them. The player may cut the sequence short at any
/// point.
/// </summary>
/// <param name="tick_from">The campaign cycle to start the animation from.</param>
/// <param name="tick_to">The campaign cycle to run the animation up to.</param>
/// <returns>bool; Did the sequence end before a single cycle could be presented?</returns>
bool Selection::Present_Ticks(int tick_from, int tick_to)
{
	int i = tick_from;
	bool first = true;
	int len = 0;

	while (i <= tick_to) {
		Check_For_Breakout();
		if (!Brokeout) {
			char str1[64];
			char str2[64];
			strncpy(str1, Fetch_String(TXT_WDT_REVIEWING_HISTORY), sizeof(str1));
			if (first) {
				len = strlen(str1);
			}
			sprintf(str2, Fetch_String(TXT_WDT_DAY), i + 1);
			strcat(str1, str2);
			if (first) {
				char *trim = str1 + len;
				while (*trim == ' ') {
					trim++;
				}
				len = trim - str1;
			}
			Set_Text(str1, first == 0 ? len : 0);
			Do_Target_Anim(TourCampaign->CycleHistory.Get_State(i), false, first);
			first = false;
			Wait_For_Anim(TextAnim);
		} else {
			break;
		}
		int loops = 5;
		while (!Brokeout) {
			if (loops-- == 0) {
				break;
			}
			Wait_Delay(5);
			Check_For_Breakout();
		}
		if (Brokeout) {
			first = false;
			break;
		}
		i++;
	}

	return(first);
}

/// <summary>
/// Handles the review presentation of the campaign map.
/// This routine queues the commentary that suits the player's standing, replays the
/// cycles that have passed since the last visit, and then waits for the voices to
/// finish before it returns.
/// </summary>
void Selection::Presentation(void)
{
	Set_Text();

	unsigned int current_tick = TourCampaign->Properties.Get_Current_Tick();
	unsigned int previous_tick = TourCampaign->PreviousTick;

	if ((int)(current_tick - previous_tick) > 8) {
		previous_tick = current_tick - 8;
	}

	State previous_state = TourCampaign->CycleHistory.Get_State(previous_tick);
	int faction = TourCampaign->PlayerFaction;
	int owned = previous_state.Count_Owned_Territories(faction);

	if (owned >= MegaWinningThreshold) {
		VoicePlayer.Queue(Voices::VOICECAT_OLD_CYCLE, OUTCOME_WIN, owned >= 30, true, 0);
	} else if (owned <= MegaLosingThreshold) {
		VoicePlayer.Queue(Voices::VOICECAT_OLD_CYCLE, OUTCOME_LOSE, owned <= 0, true, 0);
	} else {
		VoicePlayer.Queue(Voices::VOICECAT_OLD_CYCLE, OUTCOME_DRAW, true, true, 0);
	}

	Present_Ticks(previous_tick, current_tick);

	while (VoicePlayer.Playing() && !Brokeout) {
		Wait_Delay(5);
		Check_For_Breakout();
	}
}

/// <summary>
/// Presents the change in the player's fortunes over a span of the campaign.
/// This routine weighs how much territory the player held at either end of the span,
/// queues the commentary that suits the result, and then plays the map animation
/// through the intervening cycles.
/// </summary>
/// <param name="tick_from">The campaign cycle the presentation starts from.</param>
/// <param name="tick_to">The campaign cycle the presentation runs up to.</param>
/// <returns>bool; Did the player cut the presentation short?</returns>
bool Selection::Present_History(int tick_from, int tick_to)
{
	State current_state = TourCampaign->CycleHistory.Get_State(tick_from);
	int faction = TourCampaign->PlayerFaction;
	int current_terr = current_state.Count_Owned_Territories(faction);
	State previous_state = TourCampaign->CycleHistory.Get_State(tick_to);
	faction = TourCampaign->PlayerFaction;
	int previous_terr = previous_state.Count_Owned_Territories(faction);
	int d_terr = previous_terr - current_terr;

	Outcome outcome;
	bool emphasis;

	if (d_terr < -1) {
		outcome = OUTCOME_LOSE;
		emphasis = d_terr < -3 || previous_terr <= MegaLosingThreshold;
	} else if (d_terr > 1) {
		outcome = OUTCOME_WIN;
		emphasis = d_terr > 3 || previous_terr >= MegaWinningThreshold;
	} else {
		outcome = OUTCOME_DRAW;
		bool mega;
		emphasis = Get_Outcome(tick_to, mega) == OUTCOME_LOSE && mega;
	}

	VoicePlayer.Queue(Voices::VOICECAT_HISTORY, outcome, emphasis, true, 0);

	bool interrupted = Present_Ticks(tick_from, tick_to) == false;

	return(interrupted);
}


/// <summary>
/// Checks whether the player has asked to skip the presentation.
/// This routine drains the keyboard and latches the breakout flag if ESC was pressed.
/// The presentation routines poll it between animation steps.
/// </summary>
void Selection::Check_For_Breakout(void)
{
	Wait_For_Focus();
	while (Keyboard->Check()) {
		if (Keyboard->Get() == VK_ESCAPE) {
			Brokeout = true;
		}
	}
}


/// <summary>
/// Sets the caption shown in the screen's text panel.
/// Whatever caption was showing is taken down and the panel restored, then the new text
/// is word wrapped and handed to a fresh print animation.
/// </summary>
/// <param name="text">The text to display, or NULL to leave the panel blank.</param>
/// <param name="start_delay">The delay before the text begins to print itself out.</param>
void Selection::Set_Text(char const * text, int start_delay)
{
	if (TextAnim != NULL) {
		Remove_Anim(TextAnim);
		HiddenSurface->Blit_From(TextRect, *AlternateSurface, TextRect);
		Add_Update_Rect(TextRect);
		TextAnim = NULL;
	}

	if (text != NULL) {
		strncpy(Text, text, sizeof(Text));
		MSPrintAnim::Word_Wrap(Text, sizeof(Text), Font, TextRect.Width);
		TextAnim = new MSPrintAnim(Text, TextRect.X, TextRect.Y, Font, TextRect, start_delay, 1, true, true);;
		TextAnim->Set_Transient(true);
		Add_Animation(TextAnim);
	}
}


/// <summary>
/// Plays one of the screen's named sound effects.
/// The effect is looked up by the name it was given in the INI. An unknown name is
/// quietly ignored.
/// </summary>
/// <param name="name">The name of the sound effect to play.</param>
void Selection::Play_SFX(char const * name)
{
	if (name != NULL) {
		for (MSSfxEntry * entry : SfxEntries) {
			if (stricmp(entry->Get_Name(), name) == 0) {
				entry->Play();
				break;
			}
		}
	}
}


/// <summary>
/// Puts the campaign's own description up as the screen caption.
/// This is the caption the screen wears whenever no territory is under the mouse.
/// </summary>
void Selection::Set_Long_Description(void)
{
	Set_Text(TourCampaign->Properties.Get_Long_Desc());
}


/// <summary>
/// Replays the recent history of a world domination campaign.
/// This routine brings the selection screen up purely as a spectacle -- the player
/// watches the territories change hands and nothing is picked.
/// </summary>
/// <param name="campaign">The campaign whose history is to be replayed.</param>
void WDT_Review_Campaign(Campaign * campaign)
{
	Keyboard->Clear();
	Selection sel(campaign, false);
	sel.Presentation();
}


/// <summary>
/// Handles the world domination territory selection screen.
/// This routine runs the selection screen for the campaign supplied and then copies the
/// chosen territory's game options into the session so that the mission may start.
/// </summary>
/// <param name="campaign">The campaign whose map and conflicts are to be presented.</param>
/// <param name="vq_anim">Should the animated backdrop be used?</param>
/// <returns>bool; Was a playable territory chosen?</returns>
bool WDT_Select_Campaign(Campaign * campaign, bool vq_anim)
{
	Selection sel(campaign, vq_anim);
	sel.Set_Long_Description();
	sel.Do_Target_Selection();
	Territory *selterr = sel.Pick_Territory();
	bool valid = selterr != NULL;
	if (valid) {
		int index = selterr->ID;
		Session.WDTTerritory = index;
		WDTTerritory *terr = WDT_Get_Territory(index);
		if (terr != NULL) {
			Session.Options.UnitCount = terr->UnitCount;
			BuildLevel = terr->TechLevel;
			Session.Options.Credits = terr->StartingCredits;
			Session.Options.AlliesAllowed = terr->AlliesAllowed;
			Session.Options.HarvTruce = terr->HarvTruce;
			Session.Options.Bases = terr->Bases;
			Session.Options.MCVRedeploy = terr->MCVRedeploy;
			Session.Options.FogOfWar = terr->FogOfWar;
			Session.Options.BridgeDestruction = terr->BridgeDestruction;
			Session.Options.Goodies = terr->Goodies;
			Session.Options.ShortGame = terr->ShortGame;
			Session.Options.AIPlayers = 0;
			Session.Options.AIDifficulty = DIFF_NORMAL;
		} else {
			valid = false;
		}
	}

	return(valid);
}


/// <summary>
/// Brings the territory selection screen up.
/// This routine takes the mouse, starts the screen's theme, clears the surfaces, and
/// then waits out the map's introductory animation before the player is given control.
/// </summary>
void Selection::Start(void)
{
	OwnerDraw::Capture_Mouse();
	if (ThemeName != NULL) {
		Theme.Play_Song(Theme.From_Name(ThemeName));
		Theme.Set_Repeat(true);
	}
	Hide_Mouse();
	AlternateSurface->Fill(0);
	HiddenSurface->Fill(0);
	VisibleSurface->Blit_From(*HiddenSurface);
	Keyboard->Clear();
	if (RegionAnim != NULL) {
		VoicePlayer.Queue(Voices::VOICE_STARTUP, true, 0);
		Wait_For_Anim(RegionAnim, 300000);
	}
	Show_Mouse();
}


/// <summary>
/// Shuts the territory selection screen down.
/// This routine fades the music out, clears the screen back to the menu backdrop, and
/// hands the mouse back to the owner draw system.
/// </summary>
void Selection::End(void)
{
	Hide_Mouse();
	Theme.Fade_Out();
	AlternateSurface->Fill(0);
	Draw_Menu_Background();
	Show_Mouse();
	OwnerDraw::Release_Mouse();
}


/// <summary>
/// Converts a click map color index into a territory index.
/// This routine is used by the map authoring scan loops to decide which territory, if
/// any, a click map pixel belongs to. Two of the palette entries stand in for
/// territories that fall outside the directly mapped range.
/// </summary>
/// <param name="color">The palette color index read from the click map.</param>
/// <returns>Returns with the territory index, or 0xFF if the color names no territory.</returns>
unsigned int Territory_Index_From_Click_Map_Color(unsigned int color)
{
	if (color != 32) {
		if (color != 198) {
			if (color > 0x1D) {
				return(0xFF);
			}
			return(color);
		}
		return(23);
	}
	return(18);
}


/// <summary>
/// Constructs an empty centroid accumulator.
/// </summary>
Centroid::Centroid(void) :
	Position(0,0),
	Count(0)
{
	//nothing
}


/// <summary>
/// Adds another point to the centroid accumulator.
/// </summary>
/// <returns>Returns with a reference to this accumulator so that adds may be chained.</returns>
Centroid const & Centroid::operator+=(Point2D const & pt)
{
	Position += pt;
	Count++;
	return(*this);
}


/// <summary>
/// Fetches the average of every point accumulated so far.
/// </summary>
/// <returns>Returns with the center point. An empty accumulator yields the origin.</returns>
Point2D Centroid::Center_Point(void) const
{
	return(Count > 0 ? Point2D(Position.X / Count, Position.Y / Count) : Point2D(0,0));
}


/// <summary>
/// Determines if two centroid accumulators are equivalent.
/// </summary>
/// <returns>bool; Do both accumulators hold the same running total?</returns>
bool Centroid::operator==(Centroid const & that) const
{
	return(Position == that.Position && Count == that.Count);
}


/// <summary>
/// Builds the territory definition file for a world domination map.
/// This routine is used when authoring a tour map. It reads the two click map images
/// that accompany the map -- one supplying each territory's origin point, the other its
/// extent and center -- and writes an entry per territory back into WDTMap.ini.
/// </summary>
/// <param name="map_name">The name of the map, used as the INI section prefix.</param>
/// <param name="pcx1_file_name">The click map supplying each territory's origin point.</param>
/// <param name="pcx2_file_name">The click map supplying each territory's extent.</param>
void WorldDominationTour::Write_Map_INI(char const * map_name, char const * pcx1_file_name, char const * pcx2_file_name)
{
	CDFileClass ifile;
	/// The unused result captures are load-bearing for codegen: their dead IL weight
	/// affects how the compiler inlines and allocates the rest of the routine.
	int ok = ifile.Open("WDTMap.ini", 1);
	INIClass ini;
	int ok2 = ini.Load(ifile);
	ifile.Close();
	CDFileClass pfile1(pcx1_file_name);
	Surface *surf1 = Read_PCX_File(pfile1);
	CDFileClass pfile2(pcx2_file_name);
	Surface *surf2 = Read_PCX_File(pfile2);

	int height = surf2->Get_Height();
	int width = surf2->Get_Width();

	RECT_LIST rects(30);
	POINT2D_LIST points(30);
	CENTROID_LIST centroids(30);

	for (Rect & rect : rects) {
		rect.X = 0;
		rect.Y = 0;
		rect.Width = 0;
		rect.Height = 0;
	}

	for (Point2D & pt : points) {
		pt = Point2D(0,0);
	}

	/*
	 * Scan the second PCX mask. Each non-background color index accumulates a
	 * centroid sum and a bounding rectangle for that territory.
	 */
	Point2D point;
	for (point.Y = 0; point.Y < height; point.Y++) {
		for (point.X = 0; point.X < width; point.X++) {
			unsigned n = Territory_Index_From_Click_Map_Color(surf2->Get_Pixel(point));
			if (n >= 0xFF) {
				continue;
			}

			if ((unsigned)centroids.Length() < n + 1) {
				centroids.Resize(n + 1);
			}
			centroids[n] += point;

			if ((unsigned)rects.Length() < n + 1) {
				int length = rects.Length();
				rects.Resize(n + 1);
				for (Rect * blank = rects.begin() + length; blank != rects.end(); blank++) {
					blank->X = 0;
					blank->Y = 0;
					blank->Width = 0;
					blank->Height = 0;
				}
			}
			Rect & rect = rects[n];
			if (rect.Width == 0) {
				rect.Height = 0;
			}
			if (point.X < rect.X || point.X >= rect.X + rect.Width || point.Y < rect.Y || point.Y >= rect.Y + rect.Height) {
				rect = Union(rect, Rect(point.X, point.Y, 1, 1));
			}
		}
	}

	/*
	 * Scan the first PCX mask. Each non-background color index records the last
	 * matching pixel position as the territory origin point.
	 */
	for (point.Y = 0; point.Y < height; point.Y++) {
		for (point.X = 0; point.X < width; point.X++) {
			unsigned n = Territory_Index_From_Click_Map_Color(surf1->Get_Pixel(point));
			if (n >= 0xFF) {
				continue;
			}

			if ((unsigned)points.Length() < n + 1) {
				int length = points.Length();
				points.Resize(n + 1);
				for (Point2D * blank = points.begin() + length; blank != points.end(); blank++) {
					*blank = Point2D(0,0);
				}
			}
			points[n] = point;
		}
	}

	Rect * riter = rects.begin();
	Point2D * piter = points.begin();

	char section[256];
	char string[256];
	char idx1[8];
	char idx2[8];
	int ti = 0;
	int ci = 0;
	while (riter != rects.end()) {
		if (piter == points.end()) {
			break;
		}
		sprintf(string, "Territory%02d", ti);
		sprintf(idx1, "%02d", ti);
		ti++;
		sprintf(idx2, "%02d", ti);
		strncpy(section, map_name, 255u);
		strncat(section, idx1, 255u);
		ini.Put_String(map_name, string, section);
		ini.Put_Point(section, "Origin", *piter);
		ini.Put_String(section, "Name", string);
		ini.Put_String(section, "Description", string);
		ini.Put_Point(section, "Target", centroids[ci].Center_Point());
		riter++;
		piter++;
		ci++;
	}

	ifile.Open("WDTMap.ini", 2);
	ini.Save(ifile);
}
