/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "mapcapture.h"

#include "_logic.h"
#include "_map.h"
#include "_rect.h"
#include "_surface.h"
#include "_tactica.h"
#include "addon.h"
#include "ccfile.h"
#include "ccini.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "globals.h"
#include "headless.h"
#include "house.h"
#include "init.h"
#include "launcherapply.h"
#include "light.h"
#include "logic.h"
#include "mplayer.h"
#include "png.h"
#include "queue.h"
#include "rawfile.h"
#include "scenario.h"
#include "session.h"
#include "sidebar.h"
#include "spawnerconfig.h"
#include "tactical.h"
#include "tracker.h"

#include <cstdio>
#include <filesystem>
#include <string>
#include <vector>


namespace MapCapture
{
	namespace {

		// Random animation delays come from the seed, so a fixed one draws the same frame every run.
		constexpr int CAPTURE_SEED = 1;

		char const * const SETTINGS_SECTION = "Capture";

		enum class ShroudType { None, Player };
		enum class SeatsType { Observer, Listed };
		enum class AreaType { Local, Full };

		struct CaptureType
		{
			std::string Map;
			std::string Output;
			ShroudType Shroud = ShroudType::None;
			SeatsType Seats = SeatsType::Observer;
			AreaType Area = AreaType::Local;
			int Frames = 0;
		};

		CaptureType Capture;
		std::string DescriptionPath;
		bool IsShorthand = false;
		Spawner::Config ListedSeats;


		/// <summary>
		/// Reads one of a [Capture] setting's named values. A missing key keeps the current value.
		/// </summary>
		/// <returns>bool; Is the key missing or one of the names given?</returns>
		bool Read_Choice(INIClass const & ini, char const * key, char const * const * names, int count, int & choice)
		{
			char value[64];
			if (ini.Get_String(SETTINGS_SECTION, key, "", value, sizeof(value)) == 0) {
				return(true);
			}

			for (int index = 0; index < count; index++) {
				if (stricmp(value, names[index]) == 0) {
					choice = index;
					return(true);
				}
			}

			Headless::Fail("[%s] %s=%s is not one of the values it accepts.", SETTINGS_SECTION, key, value);
			return(false);
		}


		/// <summary>
		/// Reads the [Capture] settings over their defaults.
		/// </summary>
		/// <returns>bool; Were the settings valid?</returns>
		bool Read_Settings(INIClass const & ini)
		{
			static char const * const _shrouds[] = { "None", "Player" };
			static char const * const _seats[] = { "Observer", "Listed" };
			static char const * const _areas[] = { "Local", "Full" };

			int shroud = (int)Capture.Shroud;
			int seats = (int)Capture.Seats;
			int area = (int)Capture.Area;
			if (!Read_Choice(ini, "Shroud", _shrouds, ARRAY_SIZE(_shrouds), shroud)
					|| !Read_Choice(ini, "Seats", _seats, ARRAY_SIZE(_seats), seats)
					|| !Read_Choice(ini, "Area", _areas, ARRAY_SIZE(_areas), area)) {
				return(false);
			}

			Capture.Shroud = (ShroudType)shroud;
			Capture.Seats = (SeatsType)seats;
			Capture.Area = (AreaType)area;
			Capture.Frames = ini.Get_Int(SETTINGS_SECTION, "Frames", Capture.Frames);

			if (Capture.Frames < 0) {
				Headless::Fail("[%s] Frames=%d is negative.", SETTINGS_SECTION, Capture.Frames);
				return(false);
			}
			return(true);
		}


		/// <summary>
		/// Reads the capture's map, output and settings from the description file, and its seats
		/// for Seats=Listed. A relative output path is taken from the file's folder.
		/// </summary>
		/// <returns>bool; Was the file read and the capture in it complete and valid?</returns>
		bool Read_Description(void)
		{
			RawFileClass file(DescriptionPath.c_str());
			if (!file.Is_Available()) {
				Headless::Fail("The capture description %s was not found.", DescriptionPath.c_str());
				return(false);
			}

			INIClass ini;
			ini.Load(file);

			if (!Read_Settings(ini)) {
				return(false);
			}

			char value[_MAX_PATH];
			ini.Get_String(SETTINGS_SECTION, "Map", "", value, sizeof(value));
			Capture.Map = value;
			ini.Get_String(SETTINGS_SECTION, "Output", "", value, sizeof(value));
			Capture.Output = value;

			if (Capture.Map.empty() || Capture.Output.empty()) {
				Headless::Fail("[%s] needs both Map= and Output=.", SETTINGS_SECTION);
				return(false);
			}

			std::filesystem::path output(Capture.Output);
			if (output.is_relative()) {
				output = std::filesystem::absolute(std::filesystem::path(DescriptionPath)).parent_path() / output;
				Capture.Output = output.string();
			}

			ListedSeats.Read_INI(ini);
			return(true);
		}


		/// <summary>
		/// Does the map play as a campaign mission? A multiplayer map may name a player house
		/// too, so only a house the map itself lists, other than Neutral or Special, counts.
		/// </summary>
		bool Is_Campaign_Map(CCINIClass & ini)
		{
			if (ini.Get_Bool("Basic", "MultiplayerOnly", false)) {
				return(false);
			}

			char player[64];
			ini.Get_String("Basic", "Player", "", player, sizeof(player));
			if (player[0] == '\0' || stricmp(player, "Neutral") == 0 || stricmp(player, "Special") == 0) {
				return(false);
			}

			int const count = ini.Entry_Count("Houses");
			for (int index = 0; index < count; index++) {
				char house[64];
				ini.Get_String("Houses", ini.Get_Entry("Houses", index), "", house, sizeof(house));
				if (stricmp(house, player) == 0) {
					return(true);
				}
			}

			return(false);
		}


		void Seat_Observer(void)
		{
			Session.Type = GAME_SKIRMISH;

			Clear_Vector(&Session.Players);
			Clear_Vector(&Session.Computers);

			Session.Options.Bases = false;
			Session.Options.Goodies = false;
			Session.Options.UnitCount = 0;
			Session.Options.AIPlayers = 0;
			Session.Options.FogOfWar = false;

			std::snprintf(Session.Handle, sizeof(Session.Handle), "%s", "Observer");
			Session.House = HOUSE_FIRST;
			Session.ColorIdx = 0;
			Session.PrefColor = 0;

			NodeNameType * node = new NodeNameType;
			std::snprintf(node->Name, sizeof(node->Name), "%s", Session.Handle);
			node->Player.House = Session.House;
			node->Player.Color = Session.ColorIdx;
			node->Player.ProcessTime = -1;
			node->Player.IsObserver = true;
			Session.Players.Add(node);

			Session.NumPlayers = 1;
			Session.PlayerHouse = (HousesType)Session.House;
		}


		unsigned Allies_Mask(Spawner::Config::SlotType const & seat)
		{
			unsigned mask = 0;
			for (int ally : seat.Alliances) {
				if (ally >= 0 && ally < Spawner::Config::SLOT_COUNT) {
					mask |= 1u << ally;
				}
			}
			return(mask);
		}


		// Seats the houses the description lists, as a client launch seats them for a skirmish.
		bool Seat_Listed(void)
		{
			// The seats are judged as the launch request the same file would give.
			std::string fault;
			bool playable = ListedSeats.Is_Launchable(fault);
			if (playable) {
				Launch::Message::Request request;
				ListedSeats.To_Request(request);
				Launch::Message::Play const * play = std::get_if<Launch::Message::Play>(&request.Verb);
				Launch::FaultSite site;
				Launch::Game game;
				if (play == nullptr) {
					playable = false;
					fault = "The description resumes a saved game.";
				} else if (!game.From_Message(play->Game, Launch::Limits(), fault, &site)) {
					playable = false;
					fault = ListedSeats.Describe_Fault(site, fault);
				}
			}
			if (!playable) {
				Headless::Fail("The listed seats cannot be played: %s", fault.c_str());
				return(false);
			}

			Session.Type = GAME_SKIRMISH;

			Clear_Vector(&Session.Players);
			Clear_Vector(&Session.Computers);

			Session.Options.Bases = ListedSeats.Bases;
			Session.Options.Credits = ListedSeats.Credits;
			Session.Options.Goodies = ListedSeats.Crates;
			Session.Options.UnitCount = ListedSeats.UnitCount;
			Session.Options.AIPlayers = ListedSeats.AIPlayers;
			Session.Options.AIDifficulty = (DiffType)ListedSeats.AIDifficulty;
			Session.Options.FogOfWar = ListedSeats.FogOfWar;
			BuildLevel = ListedSeats.TechLevel;

			Spawner::Config::SlotType const & local = ListedSeats.Slots[ListedSeats.LocalSlot];
			std::snprintf(Session.Handle, sizeof(Session.Handle), "%s", local.Name.empty() ? "Player" : local.Name.c_str());
			Session.House = local.Country;
			Session.ColorIdx = local.Color;
			Session.PrefColor = local.Color;

			// The game takes the first human seat to be the one it draws for.
			std::vector<int> humans = { ListedSeats.LocalSlot };
			for (int index = 0; index < ListedSeats.HumanCount; index++) {
				if (index != ListedSeats.LocalSlot) {
					humans.push_back(index);
				}
			}

			for (int index : humans) {
				Spawner::Config::SlotType const & seat = ListedSeats.Slots[index];
				NodeNameType * node = new NodeNameType;
				std::snprintf(node->Name, sizeof(node->Name), "%s", seat.Name.empty() ? "Player" : seat.Name.c_str());
				node->Player.House = seat.Country;
				node->Player.Color = seat.Color;
				node->Player.ProcessTime = -1;
				node->Player.SpawnChoice = seat.StartingPosition;
				node->Player.AlliesMask = Allies_Mask(seat);
				node->Player.IsObserver = seat.IsSpectator;
				Session.Players.Add(node);
			}

			for (int index = ListedSeats.HumanCount; index < Spawner::Config::SLOT_COUNT; index++) {
				Spawner::Config::SlotType const & seat = ListedSeats.Slots[index];
				if (seat.Occupancy != Spawner::Config::OccupancyType::Computer) {
					continue;
				}

				NodeNameType * node = new NodeNameType;
				node->Player.House = seat.Country;
				node->Player.Color = seat.Color;
				node->Player.Handicap = Spawner::Config::Playable_Handicap(seat.Handicap);
				node->Player.SpawnChoice = seat.StartingPosition;
				node->Player.AlliesMask = Allies_Mask(seat);
				Session.Computers.Add(node);
			}

			Session.NumPlayers = ListedSeats.HumanCount;
			Session.PlayerHouse = (HousesType)local.Country;
			return(true);
		}


		void Seat_Campaign(void)
		{
			Session.Type = GAME_NORMAL;
			Session.CampaignDifficulty = DIFF_NORMAL;
			Session.CampaignCDifficulty = DIFF_NORMAL;
			Scen->Campaign = CAMPAIGN_NONE;
		}


		// The logic part of a game frame, without input, drawing, the frame rate limit or a start state.
		// Main_Loop runs the same simulation steps, so a step added to either must be added to the other.
		void Run_Logic_Frame(void)
		{
			DisplayClass::Layer[LAYER_GROUND].Sort();
			Logic.AI();
			Queue_AI();
			Frame++;
			Process_Deferred_Deletion();
		}


		bool Render_And_Write(void)
		{
			Rect const & local = Map.LocalRect;

			// The scroll limits of a view exactly this size leave it a single position, which
			// frames the whole LocalSize area.
			Rect const view(0, 0, ISO_TILE_PIXEL_W * local.Width, ISO_TILE_PIXEL_H * (2 * local.Height + 9) / 2);
			if (!view.Is_Valid()) {
				Headless::Fail("The map's LocalSize %d,%d,%d,%d leaves nothing to draw.", local.X, local.Y, local.Width, local.Height);
				return(false);
			}

			Rect const sidebar(0, 0, SidebarClass::SIDE_WIDTH, VisibleRect.Height);
			if (!Allocate_Surfaces(VisibleRect, view, view, sidebar)
					|| CompositeSurface == NULL || TileSurface == NULL
					|| ((DSurface *)CompositeSurface)->Get_Buffer() == NULL
					|| ((DSurface *)TileSurface)->Get_Buffer() == NULL) {
				Headless::Fail("No %dx%d drawing surface could be allocated.", view.Width, view.Height);
				return(false);
			}

			Map.Set_View_Dimensions(view);

			// A full render draws overlays and shadows again inside each dirty area, and the logic
			// frames registered theirs against whatever view was current while they ran.
			TacticalMap->Reset_Dirty_Rectangles();

			LogicalSurface = CompositeSurface;
			TacticalMap->Render(*CompositeSurface, true, DRAW_PASS_FULL);

			DSurface const * surface = (DSurface const *)CompositeSurface;
			unsigned short const * pixels = (unsigned short const *)surface->Get_Buffer();

			RawFileClass file(Capture.Output.c_str());
			if (!Write_PNG_File(file, view.Width, view.Height, surface->Stride(), pixels)) {
				file.Delete();
				Headless::Fail("%s could not be written.", Capture.Output.c_str());
				return(false);
			}

			DebugString("MapCapture: wrote %s, %dx%d, from LocalSize %d,%d,%d,%d.\n", Capture.Output.c_str(),
				view.Width, view.Height, local.X, local.Y, local.Width, local.Height);
			return(true);
		}


		bool Check(void)
		{
			if (!DescriptionPath.empty()) {
				if (IsShorthand) {
					Headless::Fail("Use either -CAPTURE=<file> or -CAPTUREMAP=<map> with -CAPTUREOUT=<png>, not both.");
					return(false);
				}
				return(Read_Description());
			}

			if (Capture.Map.empty() || Capture.Output.empty()) {
				Headless::Fail("A map capture needs both -CAPTUREMAP=<map> and -CAPTUREOUT=<png>.");
				return(false);
			}
			return(true);
		}


		/// <summary>
		/// Loads the map with the seats the capture asks for, runs the logic frames it asks for,
		/// and writes the PNG of the area it asks for. A campaign mission keeps every house it
		/// lists, whatever the seats setting.
		/// </summary>
		/// <returns>bool; Was the PNG written?</returns>
		bool Run(void)
		{
			CCFileClass file(Capture.Map.c_str());
			if (!file.Is_Available()) {
				Headless::Fail("The map %s was not found.", Capture.Map.c_str());
				return(false);
			}

			CCINIClass ini;
			ini.Load(file, false);
			bool const campaign = Is_Campaign_Map(ini);
			bool const listed = !campaign && Capture.Seats == SeatsType::Listed;

			if (Capture.Shroud == ShroudType::Player && !campaign
					&& (!listed || ListedSeats.Slots[ListedSeats.LocalSlot].IsSpectator)) {
				Headless::Fail("Shroud=Player needs a playing seat: set Seats=Listed with a local seat that is not a spectator.");
				return(false);
			}

			// Firestorm adds to the base game's art and rules, so with it every map draws.
			Disable_Addon(ADDON_ANY);
			if (Addon_Installed(ADDON_FIRESTORM) && (!listed || ListedSeats.Firestorm)) {
				Enable_Addon(ADDON_FIRESTORM);
				Set_Required_Addon(ADDON_FIRESTORM);
			}

			if (campaign) {
				Seat_Campaign();
			} else if (listed) {
				if (!Seat_Listed()) {
					return(false);
				}
			} else {
				Seat_Observer();
			}

			CustomSeed = (listed && ListedSeats.Seed != 0) ? ListedSeats.Seed : CAPTURE_SEED;
			Init_Random();

			std::snprintf(Scen->ScenarioName, sizeof(Scen->ScenarioName), "%s", Capture.Map.c_str());
			DebugString("MapCapture: loading %s as a %s.\n", Scen->ScenarioName,
				campaign ? "campaign map" : (listed ? "multiplayer map with listed seats" : "multiplayer map"));

			if (!Read_Scenario(Scen->ScenarioName)) {
				Headless::Fail("The map %s could not be loaded.", Capture.Map.c_str());
				return(false);
			}

			if (Capture.Area == AreaType::Full) {
				Map.Set_Local_Dimensions(Map.PlayRect);
			}

			for (int frame = 0; frame < Capture.Frames && GameActive; frame++) {
				Run_Logic_Frame();
			}

			// The frames commit lighting changes within a wall-clock budget, so any still queued are
			// finished here; otherwise a faster run would draw a different frame.
			LightSourceClass::Process_Lighting(0, true);

			if (Capture.Shroud == ShroudType::None && !PlayerPtr->IsObserver) {
				Map.Reveal_The_Map(PlayerPtr, true);
				Session.ObiWan = 1;
			}

			return(Render_And_Write());
		}


		Headless::Task const Task = { "MapCapture", Check, Run };
	}


	/// <summary>
	/// Names the map a capture run draws, which makes the run headless.
	/// </summary>
	void Request_Map(char const * name)
	{
		Capture.Map = name;
		IsShorthand = true;
		Headless::Request(Task);
	}


	/// <summary>
	/// Names the PNG a capture run writes, replacing any file already there, which makes the
	/// run headless. A relative path is taken from the game's folder.
	/// </summary>
	void Request_Output(char const * path)
	{
		Capture.Output = path;
		IsShorthand = true;
		Headless::Request(Task);
	}


	/// <summary>
	/// Names the file that describes a capture, which makes the run headless. A relative path is
	/// taken from the game's folder.
	/// </summary>
	void Request_Description(char const * path)
	{
		DescriptionPath = path;
		Headless::Request(Task);
	}
}
