/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "savemgr.h"

#include "_map.h"
#include "_rules.h"
#include "_wsproto.h"
#include "ccfile.h"
#include "data.h"
#include "dbgprint.h"
#include "gamedirs.h"
#include "globals.h"
#include "ipxmgr.h"
#include "language/language.h"
#include "loaddlg.h"
#include "msgbox.h"
#include "netdlg.h"
#include "netglobal.h"
#include "ownrdraw.h"
#include "ui/uiwaitbox.h"
#include "rawfile.h"
#include "rules.h"
#include "saveload.h"
#include "savever.h"
#include "scenario.h"
#include "session.h"
#include "spawner.h"
#include "stimer.h"
#include "wsproto.h"

#include "dialog.hh"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>


static AutosaveClass::KindType Single_Player_Kind(void)
{
	return(Session.Type == GAME_NORMAL ? AutosaveClass::KindType::Campaign : AutosaveClass::KindType::Skirmish);
}


// Not 0: every other notice takes that, and so does a chat line from the first house.
static constexpr int SAVE_ANNOUNCEMENT_ID = -1;


void SaveManagerClass::Service(void)
{
	Autosave_Service();
	Quick_Save_Service();
	Process_Pending_Save_Game();
	Post_Pending_Notice();
	Process_Pending_Load_Game();
}


static int Save_Message_Timeout(void)
{
	return(int(Rule->MessageDelay * TICKS_PER_MINUTE));
}


static void Add_Save_Message(int id, int text)
{
	Session.Messages.Add_Message(NULL, id, Fetch_String(text), PlayerPtr->Scheme,
		TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW), Save_Message_Timeout());
	Map.Flag_To_Redraw();
}


/// <summary>
/// Reports the outcome of a save, or why one was refused, in the message list.
/// </summary>
void SaveManagerClass::Post_Save_Notice(int text)
{
	Add_Save_Message(0, text);
}


/// <summary>
/// Accepts a save request at the boundary shared by every engine caller.
/// Solo and skirmish games save immediately. A synchronized multiplayer request is held
/// until the frame has finished retiring dead objects. A quiet request is written without
/// the saving box; the first request of a frame settles that and what the save reports.
/// </summary>
/// <returns>Returns true when the save completed or the multiplayer request was accepted.</returns>
bool SaveManagerClass::Request_Save_Game(char const * file_name, char const * descr, bool quiet,
	NoticeType notice)
{
	if (file_name == NULL || descr == NULL) return(false);

	if (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH) {
		bool saved = Save_Game(file_name, descr);
		Record_Save_Outcome(notice, saved);
		return(saved);
	}

	if (!MultiplayerSavingAllowed) {
		DebugString("Ignoring multiplayer save request because a player has left this match\n");
		Record_Save_Outcome(notice, false);
		return(false);
	}

	if (MultiplayerLoad.Is_Pending()) {
		DebugString("Ignoring multiplayer save request because a load is pending\n");
		Record_Save_Outcome(notice, false);
		return(false);
	}

	if (MultiplayerSavePending) {
		DebugString("Coalescing duplicate multiplayer save request\n");
		return(true);
	}

	PendingSaveFileName = file_name;
	PendingSaveDescription = descr;
	MultiplayerSaveQuiet = quiet;
	PendingSaveNotice = notice;
	MultiplayerSavePending = true;
	return(true);
}


/// <summary>
/// Writes the synchronized save request accepted during this frame, if any.
/// The request is cleared before writing so a callback cannot cause it to be written twice.
/// </summary>
void SaveManagerClass::Process_Pending_Save_Game(void)
{
	if (!MultiplayerSavePending) return;

	std::string file_name;
	std::string description;
	file_name.swap(PendingSaveFileName);
	description.swap(PendingSaveDescription);
	bool quiet = MultiplayerSaveQuiet;
	NoticeType notice = PendingSaveNotice;
	MultiplayerSavePending = false;
	MultiplayerSaveQuiet = false;
	PendingSaveNotice = NoticeType::None;

	if (MultiplayerSavingAllowed) {
		UIWaitBoxClass box;
		if (!quiet) {
			box.Show(Fetch_String(TXT_SAVING_GAME));
		}
		bool saved = Save_Game(file_name.c_str(), description.c_str());
		box.Hide();
		Record_Save_Outcome(notice, saved);
		if (saved && SpawnCopyPending) {
			Write_Spawn_Copy();
		}
	}
}


/// <summary>
/// Holds the outcome of a completed request until the frame boundary reports it. A request
/// with no notice of its own leaves an outcome already waiting untouched.
/// </summary>
void SaveManagerClass::Record_Save_Outcome(NoticeType notice, bool saved)
{
	if (notice == NoticeType::None) return;

	OutcomeNotice = notice;
	OutcomeSaved = saved;
}


/// <summary>
/// Reports the outcome of the save that completed this frame. A save the game asked for
/// reports in place of the announcement it posted, and says nothing about a success it never
/// announced. An outcome that cannot be shown is dropped rather than held over.
/// </summary>
void SaveManagerClass::Post_Pending_Notice(void)
{
	if (OutcomeNotice == NoticeType::None) return;

	NoticeType notice = OutcomeNotice;
	bool saved = OutcomeSaved;
	OutcomeNotice = NoticeType::None;
	OutcomeSaved = false;

	if (!ScenarioActive || Session.Play || PlayerPtr == NULL) return;

	if (notice == NoticeType::Requested) {
		Post_Save_Notice(saved ? TXT_GAME_SAVED : TXT_SAVE_FAILED);
		return;
	}

	int text = saved ? TXT_GAME_AUTO_SAVED : TXT_AUTOSAVE_FAILED;
	if (Session.Messages.Replace_Message(SAVE_ANNOUNCEMENT_ID, 0, Fetch_String(text), Save_Message_Timeout())) {
		Map.Flag_To_Redraw();
		return;
	}
	if (!saved) {
		Post_Save_Notice(text);
	}
}


/// <summary>
/// Opens the save boundary for a newly selected game and discards stale work from the last one.
/// Mission restart deliberately does not call this routine.
/// </summary>
void SaveManagerClass::Reset_Multiplayer_Save_State(void)
{
	MultiplayerSavingAllowed = true;
	MultiplayerSavePending = false;
	PendingSaveFileName.clear();
	PendingSaveDescription.clear();
	PendingSaveNotice = NoticeType::None;
	QuickSaveRequested = false;
	OutcomeNotice = NoticeType::None;
	OutcomeSaved = false;
}


/// <summary>
/// Closes multiplayer saving for the rest of the current match and cancels pending work.
/// </summary>
void SaveManagerClass::Disable_Multiplayer_Saving(void)
{
	MultiplayerSavingAllowed = false;
	MultiplayerSavePending = false;
	PendingSaveFileName.clear();
	PendingSaveDescription.clear();
	PendingSaveNotice = NoticeType::None;
}


/// <summary>
/// Reports whether the current multiplayer match may still accept a save request.
/// </summary>
bool SaveManagerClass::Is_Multiplayer_Saving_Allowed(void) const
{
	return(MultiplayerSavingAllowed);
}


/// <summary>
/// Arms the next automatic save when it falls due and writes an armed one through the save
/// boundary a frame later, once its notice has been drawn.
/// </summary>
void SaveManagerClass::Autosave_Service(void)
{
	if (Session.Play) return;

	bool single = Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH;

	if (!single && (!MultiplayerSavingAllowed || MultiplayerLoad.Is_Pending())) return;

	if (Autosave.Take_Armed()) {
		Autosave.Schedule(Frame);

		if (single) {
			AutosaveClass::KindType kind = Single_Player_Kind();
			int slot = Autosave.Advance(kind);

			char buffer[512];
			std::snprintf(buffer, sizeof(buffer), Fetch_String(TXT_AUTOSAVE_DESCRIPTION), slot + 1, Scen->Description);

			Request_Save_Game(AutosaveClass::File_Name(kind, slot).c_str(), buffer, true, NoticeType::Automatic);
		} else {
			Request_Multiplayer_Save(Fetch_String(TXT_AUTOSAVE_MULTIPLAYER), true, NoticeType::Automatic);
		}
		return;
	}

	// Timed multiplayer saves require a shared launch-file interval.
	if ((single || Spawner_Is_Active()) && Autosave.Is_Due(Frame)) {
		Autosave.Arm();
		Add_Save_Message(SAVE_ANNOUNCEMENT_ID, TXT_AUTOSAVING);
	}
}


/// <summary>
/// Asks for a quick save at the next frame boundary.
/// </summary>
void SaveManagerClass::Request_Quick_Save(void)
{
	QuickSaveRequested = true;
}


/// <summary>
/// Writes a requested quick save once the frame has retired its dead objects, behind the box
/// a menu save shows, and reports the outcome in the message list.
/// </summary>
void SaveManagerClass::Quick_Save_Service(void)
{
	if (!QuickSaveRequested) return;
	QuickSaveRequested = false;

	if (!ScenarioActive || Session.Play) return;
	if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) return;

	char description[512];
	std::snprintf(description, sizeof(description), Fetch_String(TXT_QUICKSAVE_DESCRIPTION), Scen->Description);

	UIWaitBoxClass box;
	box.Show(Fetch_String(TXT_SAVING_GAME));
	Request_Save_Game(Quick_Save_File_Name(Single_Player_Kind()).c_str(), description, false,
		NoticeType::Requested);
	box.Hide();
}


int SaveManagerClass::Next_Multiplayer_Save_Slot(void)
{
	for (int slot = 0; slot < MULTIPLAYER_SAVE_SLOTS; slot++) {
		std::string path = Saved_Game_Name(Multiplayer_Save_File_Name(slot).c_str());
		if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
			return(slot);
		}
	}
	return(MULTIPLAYER_SAVE_SLOTS - 1);
}


/// <summary>
/// Requests a save under the next free numbered multiplayer name, so every machine writes
/// the same number for the same frame.
/// </summary>
bool SaveManagerClass::Request_Multiplayer_Save(char const * descr, bool quiet, NoticeType notice)
{
	return(Request_Save_Game(Multiplayer_Save_File_Name(Next_Multiplayer_Save_Slot()).c_str(), descr, quiet,
		notice));
}


/// <summary>
/// Begins the numbered saves of a network match. A new match drops the files a previous match
/// left, its launch-file copy included, so numbering starts at zero on every machine, and a
/// client-launched match owes a fresh copy of its launch file at its first save. A resumed
/// match keeps its files and carries the numbering on.
/// </summary>
void SaveManagerClass::Multiplayer_Saves_Begin_Match(bool resumed)
{
	if (resumed) {
		return;
	}

	int removed = 0;
	for (int slot = 0; slot < MULTIPLAYER_SAVE_SLOTS; slot++) {
		std::string path = Saved_Game_Name(Multiplayer_Save_File_Name(slot).c_str());
		if (DeleteFileA(path.c_str())) {
			removed++;
		}
	}
	if (removed > 0) {
		DebugString("Removed %d multiplayer saves of a previous match\n", removed);
	}
	if (DeleteFileA(Saved_Game_Name("spawnSG.ini").c_str())) {
		DebugString("Removed the launch-file copy of a previous match\n");
	}

	SpawnCopyPending = Spawner_Is_Active();
}


/// <summary>
/// Copies the launch file beside the numbered saves as spawnSG.ini, which the client resumes
/// the match from. The copy stays owed until it is written, so a failed attempt is retried at
/// the next save.
/// </summary>
void SaveManagerClass::Write_Spawn_Copy(void)
{
	CCFileClass source("SPAWN.INI");
	if (!source.Is_Available() || !source.Open(FileClass::READ)) {
		DebugString("SPAWN.INI could not be read for spawnSG.ini\n");
		return;
	}
	int size = source.Size();
	std::vector<char> bytes(size > 0 ? size : 0);
	bool read = size > 0 && source.Read(bytes.data(), size) == size;
	source.Close();
	if (!read) {
		DebugString("SPAWN.INI could not be read for spawnSG.ini\n");
		return;
	}

	// The file object keeps the name pointer, so the path must outlive it.
	std::string copy_path = Saved_Game_Name("spawnSG.ini");
	RawFileClass copy(copy_path.c_str());
	bool written = copy.Open(FileClass::WRITE) && copy.Write(bytes.data(), size) == size;
	copy.Close();
	DebugString(written ? "Wrote spawnSG.ini beside the multiplayer saves\n" : "spawnSG.ini could not be written\n");
	if (written) {
		SpawnCopyPending = false;
	}
}


/// <summary>
/// May this machine ask every machine to load a multiplayer save now? Only the master of a
/// network game may, while no load is pending and no recording plays.
/// </summary>
bool SaveManagerClass::Multiplayer_Load_Is_Allowed(void) const
{
	return((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !Session.Play
		&& Session.Am_I_Master() && !MultiplayerLoad.Is_Pending());
}


/// <summary>
/// Offers the master the match's saved games and asks every machine to load the pick. The list
/// pumps the game underneath it unless the session is suspended, so between frames the match
/// keeps running while the master browses.
/// </summary>
/// <returns>bool; Was a load requested?</returns>
bool SaveManagerClass::Multiplayer_Load_Prompt(void)
{
	if (!Multiplayer_Load_Is_Allowed()) {
		return(false);
	}

	MultiplayerLoadOptionsClass list;
	return(list.Load() && Multiplayer_Load_Request(Multiplayer_Save_Slot(list.Picked_File())));
}


/// <summary>
/// Schedules the numbered multiplayer save on this machine and asks every other seat to do the
/// same. The file must be here with the running version's stamp and this kind of game.
/// </summary>
/// <returns>bool; Was the load scheduled?</returns>
bool SaveManagerClass::Multiplayer_Load_Request(int slot)
{
	if (!MultiplayerLoadClass::Slot_Is_Valid(slot) || !Multiplayer_Load_Is_Allowed()) {
		return(false);
	}

	std::string file_name = Multiplayer_Save_File_Name(slot);
	SaveVersionInfo info;
	if (!Get_Savefile_Info(file_name.c_str(), &info) || info.Get_Internal_Version() != ExpectedGameVersion
		|| (GameType)info.Get_Game_Type() != Session.Type) {
		DebugString("Refusing to request the multiplayer save %s\n", file_name.c_str());
		return(false);
	}

	if (!MultiplayerLoad.Schedule(slot, Monotonic_Milliseconds())) {
		return(false);
	}
	DebugString("Asking every machine to load %s\n", file_name.c_str());

	GlobalPacketType packet;
	NetGlobal::Initialize_Packet(packet, NET_LOAD_GAME);
	std::snprintf(packet.Name, sizeof(packet.Name), "%s", Session.Players[0]->Name);
	packet.LoadGame.Slot = (unsigned short)slot;

	for (int index = 1; index < Session.Players.Count(); index++) {
		Ipx.Send_Global_Message(&packet, sizeof(packet), 1, &Session.Players[index]->Address);
		Ipx.Service();
	}
	return(true);
}


/// <summary>
/// Schedules the save the master named, once the packet has passed validation.
/// </summary>
void SaveManagerClass::Multiplayer_Load_Receive(int slot)
{
	if (Session.Type != GAME_IPX && Session.Type != GAME_INTERNET) {
		DebugString("Ignoring a load request outside a network game\n");
		return;
	}

	if (MultiplayerLoad.Is_Pending()) {
		DebugString("Ignoring a load request while slot %d is pending\n", MultiplayerLoad.Slot());
		return;
	}

	if (MultiplayerLoad.Schedule(slot, Monotonic_Milliseconds())) {
		DebugString("The master asked every machine to load %s\n", Multiplayer_Save_File_Name(slot).c_str());
	}
}


bool SaveManagerClass::Multiplayer_Load_Is_Pending(void) const
{
	return(MultiplayerLoad.Is_Pending());
}


bool SaveManagerClass::Multiplayer_Load_Is_In_Progress(void) const
{
	return(MultiplayerLoadInProgress);
}


/// <summary>
/// Drops a seat whose player signed off while this machine was loading, when no connection
/// exists to destroy. The seats are matched to the loaded houses afterwards, so the house
/// passes to the computer as it does for any player who left.
/// </summary>
void SaveManagerClass::Multiplayer_Load_Unseat(int index)
{
	if (index <= 0 || index >= Session.Players.Count()) {
		return;
	}

	NodeNameType * seat = Session.Players[index];
	DebugString("%s signed off during the load; dropping the seat\n", seat->Name);
	Session.Players.Delete(seat);
	delete seat;
	Session.NumPlayers--;
}


/// <summary>
/// Loads a multiplayer save in place of the running match, keeping the seats and rebuilding the
/// connections around the loaded houses. The next frame runs the usual post-load barrier.
/// </summary>
/// <returns>bool; Is the loaded game ready to synchronize? On false the match is lost.</returns>
bool SaveManagerClass::Perform_Multiplayer_Load(char const * file_name)
{
	DebugString("Loading %s in place of the running match\n", file_name);

	// Nothing sent or received for the running match may reach the loaded one.
	if (PacketTransport != NULL) {
		PacketTransport->Discard_In_Buffers();
		PacketTransport->Discard_Out_Buffers();
	}
	while (Ipx.Num_Connections() > 0) {
		Ipx.Delete_Connection(Ipx.Connection_ID(0));
	}
	DoList.clear();
	OutList.clear();

	Session.LoadGame = true;
	MultiplayerLoadInProgress = true;
	bool const loaded = LoadOptionsClass().Load_File(file_name);
	MultiplayerLoadInProgress = false;
	if (!loaded) {
		return(false);
	}
	if (!Reconcile_Players()) {
		return(false);
	}

	if (PacketTransport != NULL) {
		PacketTransport->Discard_In_Buffers();
		PacketTransport->Discard_Out_Buffers();
	}
	if (!Session.Create_Connections()) {
		return(false);
	}
	Ipx.Set_Timing(std::max<unsigned>(TIMER_SECOND, Ipx.Global_Response_Time() + 2), (unsigned int)-1, 10 * TIMER_SECOND);
	Spawner_Announce_Master();

	Reset_Multiplayer_Save_State();
	Map.Flag_To_Redraw(GS_REDRAW_ALL);
	return(true);
}


/// <summary>
/// Waits out the countdown of a scheduled multiplayer load at the end of the frame, then loads
/// the save in place of the running match. A machine whose load fails leaves the match.
/// </summary>
void SaveManagerClass::Process_Pending_Load_Game(void)
{
	if (!MultiplayerLoad.Is_Pending()) {
		return;
	}

	// Nothing of the running match is sent or executed once the load is agreed on.
	DoList.clear();
	OutList.clear();

	Session.Suspended++;
	TacticalActive = false;

	UIWaitBoxClass box;
	box.Show(Fetch_String(TXT_LOADING_SAVED_GAME));

	int shown = -1;
	while (!MultiplayerLoad.Is_Due(Monotonic_Milliseconds())) {
		int seconds = MultiplayerLoad.Seconds_Left(Monotonic_Milliseconds());
		if (box.Is_Shown() && seconds != shown) {
			shown = seconds;
			char buffer[128];
			std::snprintf(buffer, sizeof(buffer),
				Fetch_String(seconds == 1 ? TXT_LOADING_IN_SECOND : TXT_LOADING_IN_SECONDS), seconds);
			box.Set_Text(buffer);
		}
		OwnerDraw::Dialog_Message_Handler();
		Sleep(10);
	}

	box.Hide();
	Session.Suspended--;
	TacticalActive = true;

	std::string file_name = Multiplayer_Save_File_Name(MultiplayerLoad.Slot());
	MultiplayerLoad.Clear();

	if (!Perform_Multiplayer_Load(file_name.c_str())) {
		DebugString("The multiplayer load of %s failed; leaving the match\n", file_name.c_str());
		Session.LoadGame = false;
		Sign_Off_Match();
		Session.Suspended++;
		WWMessageBox().Process(TXT_ERROR_LOADING_GAME, TXT_OK);
		Session.Suspended--;
		GameActive = false;
	}
}
