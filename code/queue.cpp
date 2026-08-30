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

/* $Header: /counterstrike/QUEUE.CPP 6     3/14/97 5:12p Steve_tall $ */
/***************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : QUEUE.CPP                                *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 11/28/95                                 *
 *                                                                         *
 *                  Last Update : October 14, 1996 [BRR]                   *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions for Queueing Events:                                          *
 *   Queue_Mission -- Queue a mega mission event.                          *
 *   Queue_Options -- Queue the options event.                             *
 *   Queue_Exit -- Add the exit game event to the queue.                   *
 *                                                                         *
 * Functions for processing Queued Events:                                 *
 *   Queue_AI -- Process all queued events.                                *
 *   Queue_AI_Normal -- Process all queued events.                         *
 *   Queue_AI_Multiplayer -- Process all queued events.                    *
 *                                                                         *
 * Main Multiplayer Queue Logic:                                           *
 *   Wait_For_Players -- Waits for other systems to come on-line           *
 *   Generate_Timing_Event -- computes & queues a RESPONSE_TIME event      *
 *   Process_Send_Period -- timing for sending packets every 'n' frames    *
 *   Send_Packets -- sends out events from the OutList                     *
 *   Send_FrameSync -- Sends a FRAMESYNC packet                            *
 *   Process_Receive_Packet -- processes an incoming packet                *
 *   Process_Serial_Packet -- Handles an incoming serial packet            *
 *   Can_Advance -- determines if it's OK to advance to the next frame     *
 *   Process_Reconnect_Dialog -- processes the reconnection dialog         *
 *   Handle_Timeout -- attempts to reconnect; if fails, bails.             *
 *   Stop_Game -- stops the game                                           *
 *                                                                         *
 * Packet Compression / Decompression:                                     *
 *   Build_Send_Packet -- Builds a big packet from a bunch of little ones. *
 *   Add_Uncompressed_Events -- adds uncompressed events to a packet       *
 *   Add_Compressed_Events -- adds compressed events to a packet           *
 *                                                                         *
 * DoList Management:                                                      *
 *   Execute_DoList -- Executes commands from the DoList                   *
 *   Clean_DoList -- Cleans out old events from the DoList                 *
 *   Queue_Record -- Records the DoList to disk                            *
 *   Queue_Playback -- plays back queue entries from a record file         *
 *                                                                         *
 * Debugging:                                                              *
 *   Compute_Game_CRC -- Computes a CRC value of the entire game.          *
 *   Add_CRC -- Adds a value to a CRC                                      *
 *   Print_CRCs -- Prints a data file for finding Sync Bugs                *
 *   Init_Queue_Mono -- inits mono display                                 *
 *   Update_Queue_Mono -- updates mono display                             *
 *   Print_Framesync_Values -- displays frame-sync variables               *
 *   Check_Mirror -- Checks mirror memory                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"

#include "queue.h"

#include "_keyboar.h"
#include "_logic.h"
#include "_map.h"
#include "_mono.h"
#include "_rtti.h"
#include "_rules.h"
#include "_script.h"
#include "_surface.h"
#include "_tactica.h"
#include "_vanim.h"
#include "_warhead.h"
#include "_weapon.h"
#include "_xmouse.h"
#include "aircraft.h"
#include "airctype.h"
#include "anim.h"
#include "animtype.h"
#include "blight.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "cell.h"
#include "connmgr.h"
#include "conquer.h"
#include "crc.h"
#include "data.h"
#include "dbgprint.h"
#include "dsurface.h"
#include "empulse.h"
#include "factory.h"
#include "getcpu.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "ipxmgr.h"
#include "keyboard.h"
#include "language\language.h"
#include "light.h"
#include "mainloop.h"
#include "mono.h"
#include "msgbox.h"
#include "msgloop.h"
#include "netdlg.h"
#include "netglobal.h"
#include "netpacket.h"
#include "nettiming.h"
#include "netshare.h"
#include "opents_build.h"
#include "overlay.h"
#include "overtype.h"
#include "ownrdraw.h"
#include "particle.h"
#include "partsys.h"
#include "psystype.h"
#include "ptype.h"
#include "rules.h"
#include "scenario.h"
#include "scheme.h"
#include "script.h"
#include "session.h"
#include "side.h"
#include "smudge.h"
#include "smudtype.h"
#include "stats.h"
#include "stimer.h"
#include "tactical.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforc.h"
#include "terrain.h"
#include "tevent.h"
#include "tiberium.h"
#include "trigger.h"
#include "trigtype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vanim.h"
#include "vanimtype.h"
#include "version.h"
#include "warhead.h"
#include "waypoint.h"
#include "weapon.h"
#include "windlg.h"
#include "winstub.h"
#include "wsproto.h"

#include "special.hh"

#include <algorithm>
#include <array>
#include <ctime>


/********************************** Defines *********************************/
//#define SHOW_MONO		0			// Replaced with _DEBUG


/********************************** Globals *********************************/
//---------------------------------------------------------------------------
// GameCRC is the current computed CRC value for this frame.
// CRC[] is a record of our last 32 game CRC's.
// ColorNames is for debug output in Print_CRCs
//---------------------------------------------------------------------------
static unsigned int GameCRC;
static unsigned int CRC[256] =
	{0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0};


//...........................................................................
// Mono debugging variables:
// NetMonoMode: 0 = show connection output, 1 = flowcount output
// NewMonoMode: set by anything that toggles NetMonoMode; re-inits screen
// IsMono: used for taking control of Mono screen away from the engine
//...........................................................................
int NetMonoMode = 1;
int NewMonoMode = 1;
static int IsMono = 0;
int SentFrameSyncCount = 0;

//---------------------------------------------------------------------------
// Several routines return various codes; here's an enum for all of them.
//---------------------------------------------------------------------------
enum RetcodeType {
	RC_NORMAL,              // no news is good news
	RC_PLAYER_READY,        // a new player has been heard from
	RC_SCENARIO_MISMATCH,   // scenario mismatch
	RC_NOT_RESPONDING,      // other player not responding (timeout/hung up)
	RC_CANCEL,              // user cancelled
};


struct FrameSyncStruct {
	int frame;				/// other players' frame #
	unsigned int sent;		// # cmds other player claims to have sent
	unsigned int recv;		// # cmds actually received from others
	unsigned int timing;

	FrameSyncStruct(void);
};


/// <summary>
/// Constructs an empty frame sync record.
/// The counters start at zero and the frame number starts out invalid, so that a player
/// who has not reported in yet cannot be mistaken for one sitting on frame zero.
/// </summary>
FrameSyncStruct::FrameSyncStruct(void)
{
	frame = -1;
	sent = 0;
	recv = 0;
	timing = 0;
}


//........................................................................
// Frame-sync'ing variables
//........................................................................
FrameSyncStruct SyncBarFrameSync[MAX_PLAYERS - 1];
BasicTimerClass<SystemTimerClass> SentFrameSyncTimer;
FrameSyncStruct TheirFrameSync[MAX_PLAYERS - 1];
unsigned short SentCommandCount;								// # cmds I've sent out

static std::array<unsigned int, static_cast<std::size_t>(NetPacket::DecodeError::COUNT)> NetworkPacketDrops = {};


/// <summary>Records and rate-limits one stable event-packet rejection reason.</summary>
static void Record_Network_Packet_Drop(NetPacket::DecodeError error)
{
	std::size_t const index = static_cast<std::size_t>(error);
	if (error == NetPacket::DecodeError::NONE || index >= NetworkPacketDrops.size()) {
		return;
	}

	unsigned int const count = ++NetworkPacketDrops[index];
	if (count == 1 || (count & (count - 1)) == 0) {
		DebugString("Network event packet drop [%s]: %u\n", NetPacket::Error_Name(error), count);
	}
}



/********************************* Prototypes *******************************/
//...........................................................................
// Main multiplayer queue logic
//...........................................................................
static void Queue_AI_Normal(void);
static void Queue_AI_Multiplayer(void);
static RetcodeType Wait_For_Players(int first_time, ConnManClass *net,
	int resend_delta, int dialog_time, int timeout, char *multi_packet_buf,
	int multi_packet_max, int my_sent, FrameSyncStruct *their);
static void Generate_Timing_Event(ConnManClass *net, int my_sent);
static void Generate_Real_Timing_Event(void);
static void Generate_Network_Report_Event(ConnManClass *net);
static int Process_Send_Period(ConnManClass *net);	//, int init);
static int Send_Packets(ConnManClass *net, char *multi_packet_buf,
	int multi_packet_max, int max_ahead, int my_sent);
static void Send_FrameSync(ConnManClass *net, int cmd_count);
static RetcodeType Process_Receive_Packet(ConnManClass *net,
	char *multi_packet_buf, int id, int packetlen, FrameSyncStruct *their, BasicTimerClass<SystemTimerClass> *timer);
static int Can_Advance(ConnManClass *net, int max_ahead, FrameSyncStruct *their, int *frame_stall, int *count_stall);
static int Process_Reconnect_Dialog(CDTimerClass<SystemTimerClass> *timeout_timer,
	FrameSyncStruct *their, int num_conn, int reconn, int fresh,
	BasicTimerClass<SystemTimerClass> *timer);
static int Handle_Timeout(ConnManClass *net, FrameSyncStruct *their);
static void Stop_Game(bool=false);
BOOL CALLBACK Reconnect_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
static void Close_Reconnect_Dialog(void);
void Kick_Player_Now(ConnManClass *net, int kickee, FrameSyncStruct * their, bool error);
bool Cast_Kick_Vote(int kicker, int kickee);
void Multiplayer_Debug_Print(bool noframecheck);

//...........................................................................
// Packet compression/decompression:
//...........................................................................
static int Build_Send_Packet(void *buf, int bufsize, int frame_delay,
	int num_cmds, int cap, int & processed);
int Add_Uncompressed_Events(void *buf, int bufsize, int frame_delay, int size,
	int cap);
int Add_Compressed_Events(void *buf, int bufsize, int frame_delay, int size,
	int cap,  int & processed);

//...........................................................................
// DoList management:
//...........................................................................
static int Execute_DoList(int max_houses, HousesType base_house,
	ConnManClass *net, CDTimerClass<FrameTimerClass> *skip_crc,
//	ConnManClass *net, TCountDownTimerClass *skip_crc,
	FrameSyncStruct *their);
static void Clean_DoList(ConnManClass *net);
static void Queue_Record(void);
static void Queue_Playback(void);

//...........................................................................
// Debugging:
//...........................................................................
static void Compute_Game_CRC(void);
void Add_CRC(unsigned int *crc, unsigned int val);
static void Print_CRCs(EventClass *ev);
static void Init_Queue_Mono(ConnManClass *net);
static void Update_Queue_Mono(ConnManClass *net, int flow_index);
static void Print_Framesync_Values(int curframe, unsigned int max_ahead,
	int num_connections, FrameSyncStruct *their, unsigned short my_sent);

void Dump_Packet_Too_Late_Stuff(EventClass *event);


/**************************************************************************  *
 * Queue_Mission -- Queue a mega mission event.                              *
 *                                                                           *
 * This routine is called when the player causes a change to a game unit.    *
 * The event that initiates the change is queued to as a result of a call    *
 * to this routine.                                                          *
 *                                                                           *
 * INPUT:                                                                    *
 *      whom      Whom this mission request applies to (a friendly unit).    *
 *    mission   The mission to assign to this object.                        *
 *    target   The target of this mission (if any).                          *
 *    dest      The movement destination for this mission (if any).          *
 *                                                                           *
 * OUTPUT:                                                                   *
 *      Was the mission request queued successfully?                         *
 *                                                                           *
 * WARNINGS:                                                                 *
 *      none.                                                                *
 *                                                                           *
 * HISTORY:                                                                  *
 *   09/21/1995 JLB : Created.                                               *
 *=========================================================================*/
bool Queue_Mission(TargetClass whom, MissionType mission, TargetClass & target, TargetClass & destination)
{
	OutList.push_back(EventClass(PlayerPtr->HeapID, whom, mission, target, destination));
	return(true);
}


/***********************************************************************************************
 * Queue_Mission -- Queue a mega mission event, formation override for common speed.           *
 *                                                                                             *
 *    This routine is called when the player causes a change to a game unit. The event that    *
 *    initiates the change is queued to as a result of a call to this routine.                 *
 *                                                                                             *
 * INPUT:   whom     -- Whom this mission request applies to (a friendly unit).                *
 *                                                                                             *
 *          mission  -- The mission to assign to this object.                                  *
 *                                                                                             *
 *          target   -- The target of this mission (if any).                                   *
 *                                                                                             *
 *          dest     -- The movement destination for this mission (if any).                    *
 *                                                                                             *
 *            speed      -- The override speed for this unit.                                  *
 *                                                                                             *
 *            maxspeed -- The override maximum speed for this unit.                            *
 *                                                                                             *
 * OUTPUT:  Was the mission request queued successfully?                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/21/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool Queue_Mission(TargetClass whom, MissionType mission, TargetClass & target, TargetClass & destination, SpeedType speed, MPHType maxspeed)
{
	OutList.push_back(EventClass(PlayerPtr->HeapID, whom, mission, target, destination, speed, maxspeed));
	return(true);
}


/***************************************************************************
 * Queue_Options -- Queue the options event.                               *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      Was the options screen event queued successfully?                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
bool Queue_Options(void)
{
	if (PlayerPtr->IsToWin || PlayerPtr->IsToLose || PlayerPtr->IsToDie) {
		return(false);
	}

	OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::OPTIONS));
	return(true);

}		/* end of Queue_Options */


/***************************************************************************
 * Queue_Exit -- Add the exit game event to the queue.                     *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      Was the exit event queued successfully?                            *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
bool Queue_Exit(void)
{
	OutList.push_back(EventClass(PlayerPtr->HeapID, EventClass::EXIT));
	return(true);

}		/* end of Queue_Exit */


/***************************************************************************
 * Queue_AI -- Process all queued events.                                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
void Queue_AI(void)
{
	if (Frame >= 0) {
		Session.Apply_Staged_Network_Timing(static_cast<unsigned int>(Frame));
	}

	if (Session.Play) {
		Queue_Playback();
	}

	else {

		switch (Session.Type) {

			case GAME_SKIRMISH:
			case GAME_NORMAL:
				Queue_AI_Normal();
				break;

			case GAME_IPX:
			case GAME_INTERNET:
				Queue_AI_Multiplayer();
				break;
		}
	}
}	/* end of Queue_AI */


/***************************************************************************
 * Queue_AI_Normal -- Process all queued events.                           *
 *                                                                         *
 * This is the "normal" version of the queue management routine.  It does  *
 * the following:                                                          *
 * - Transfers items in the OutList to the DoList                          *
 * - Executes any commands in the DoList that are supposed to be done on   *
 *   this frame #                                                          *
 * - Cleans out the DoList                                                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   09/21/1995 JLB : Created.                                             *
 *=========================================================================*/
static void Queue_AI_Normal(void)
{
	//------------------------------------------------------------------------
	// Move events from the OutList (events generated by this player) into the
	// DoList (the list of events to execute).
	//------------------------------------------------------------------------
	while (!OutList.empty()) {
		OutList.front().IsExecuted = false;
		DoList.push_back(OutList.front());
		OutList.pop_front();
	}

	//------------------------------------------------------------------------
	// Save the DoList to disk, if we're in "Record" mode
	//------------------------------------------------------------------------
	if (Session.Record) {
		Queue_Record();
	}

	//------------------------------------------------------------------------
	// Execute the DoList; if an error occurs, bail out.
	//------------------------------------------------------------------------
	if (!Execute_DoList(1, PlayerPtr->Class->House, NULL, NULL, NULL)) {
		GameActive = 0;
		return;
	}

	//------------------------------------------------------------------------
	// Clean out the DoList
	//------------------------------------------------------------------------
	Clean_DoList(NULL);

}	/* end of Queue_AI_Normal */


/***************************************************************************
 * Queue_AI_Multiplayer -- Process all queued events.                      *
 *                                                                         *
 * This is the network version of the queue management routine.  It does   *
 * the following:                                                          *
 * - If this is the 1st frame, waits for other systems to signal ready     *
 * - Generates a timing event, to allow the connection time to be dynamic  *
 * - Handles timing related to sending packets every 'n' frames            *
 * - Sends outgoing events                                                 *
 * - Frame-syncs to the other systems (see below)                          *
 * - Executes & cleans out the DoList                                      *
 *                                                                         *
 * The Frame-Sync'ing logic is the heart & soul of network play.  It works *
 * by ensuring that any system won't out-run the other system by more than *
 * 'Session.MaxAhead' frames; this in turn ensures that a packet's         *
 * execution frame # won't have been passed by the time that packet is     *
 * received by all systems.                                                *
 *                                                                         *
 * To achieve this, the system must keep track of all other system's       *
 * current frame #'s; these are stored in an array called 'their_frame[]'. *
 * However, because current frame #'s are sent in FRAMEINFO packets, which *
 * don't require an ACK, and command packets are sent in packets requiring *
 * an ACK, it's possible for a command packet to get lost, and the next    *
 * frame's FRAMEINFO packet to not get lost; the other system may then     *
 * advance past the frame # the command is to execute on!  So, to prevent  *
 * this, all FRAMEINFO packets include a CommandCount field.  This value   *
 * tells the other system how many events it should have received by this  *
 * time.  This system can therefore keep track of how many commands it's   *
 * actually received, and compare it to the CommandCount field, to see if  *
 * it's missed an event packet.  The # of events we've received from each  *
 * system is stored in 'their_recv[]', and the # events they say they've   *
 * sent is stored in 'their_sent[]'.                                       *
 *                                                                         *
 * Thus, two conditions must be met in order to advance to the next frame: *
 * - Our current frame # must be < their_frame + Session.MaxAhead          *
 * - their_recv[i] must be >= their_sent[i]                                *
 *                                                                         *
 * 'their_frame[] is updated by Process_Receive_Packet()                   *
 * 'their_recv[] is updated by Process_Receive_Packet()                    *
 * 'their_sent[] is updated by Process_Receive_Packet()                    *
 * 'my_sent' is updated by this routine.                                   *
 *                                                                         *
 * The order of the arrays their_frame[] etc is the same order the         *
 * connections are created in.  The Sender's ID is passed to               *
 * Connection_Index() to obtain the array index.                           *
 *                                                                         *
 * The only routines allowed to pop up dialogs are:                        *
 *    Wait_For_Players() (only pops up the reconnect dialog)               *
 *    Execute_DoList() (tells if out of sync, or packet recv'd too late)   *
 *                                                                         *
 * Sign-off's are detected by:                                             *
 * - Timing out while waiting for a packet                                 *
 * - Detecting that the other player is now at the score screen or         *
 *   connection dialog (serial)                                            *
 * - If we see an EventClass::EXIT event on the private channel            *
 *                                                                         *
 * The current communications protocol, COMM_PROTOCOL_MULTI_E_COMP, has    *
 * the following properties:                                               *
 * - It compresses packets, so that the minimum number of bytes are        *
 *   transmitted.  Packets are compressed by extracting all info common to *
 *   the events into the packet header, and then sending only the bytes    *
 *   relevant to each type of event.  For instance, if 100 infantry guys   *
 *   are told to move to the same location, the command itself & the       *
 *   location will be included in the 1st movement command only; after     *
 *   that, there will be a rep count then 99 infantry TARGET numbers,      *
 *   identifying all the infantry told to move.                            *
 * - The protocol also only sends packets out every 'n' frames.  This cuts *
 *   the data rate dramatically.  It means that 'Session.MaxAhead' must be *
 *   divisible by 'n'; also, the minimum value for 'Session.MaxAhead' is   *
 *   'n * 2', to give both sides some "breathing" room in case a FRAMEINFO *
 *   packet gets missed.                                                   *
 *                                                                         *
 * Note:  For synchronization-waiting loops (like waiting to hear from all *
 * other players, waiting to advance to the next frame, etc), use          *
 * Net.Num_Connections() rather than Session.NumPlayers; this reflects the *
 * actual # of connections, and can be "faked" into playing even when      *
 * there aren't any other players actually there.  A typical example of    *
 * this is playing back a recorded game.  For command-execution loops, use *
 * Session.NumPlayers.  This ensures all commands get executed, even if    *
 * there isn't a human generating those commands.                          *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Queue_AI_Multiplayer(void)
{
	if (Session.Type == GAME_SKIRMISH) return;

	//........................................................................
	// Enums:
	//........................................................................
	static struct {
		int MIXFILE_RESEND_DELTA;	// ticks b/w resends
		int FRAMESYNC_DLG_TIME;		// timeout waiting for mixfiles.
		int FRAMESYNC_TIMEOUT;		// time until displaying reconnect dialog
		int MIXFILE_TIMEOUT;		// timeout waiting for frame sync packet
	} _timings[8] = {
		{ 0, 0, 0, 0 },
		{ 120, 900, 420, 3600 },
		{ 120, 900, 420, 3600 },
		{ 60, 900, 420, 2400 },
		{ 15, 1200, 720, 7200 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }
	};

	int timeout_factor = (Session.Type == GAME_INTERNET) ? 6 : 1;

	//........................................................................
	// Variables for sending, receiving & parsing packets:
	//........................................................................
	ConnManClass *net;					// ptr to access all multiplayer functions
	EventClass packet;					// for sending single frame-sync's
	char *multi_packet_buf;				// buffer for sending/receiving
	int multi_packet_max;				// max length of multi_packet_buf
	bool is_error;

	//........................................................................
	// Timing variables
	//........................................................................
	static CDTimerClass<FrameTimerClass> skip_crc;	// to delay the CRC check

	//........................................................................
	// Other misc variables
	//........................................................................
	int i;
	RetcodeType rc;
	int reconnect_dlg = 0;				// 1 = the reconnect dialog is displayed

	//------------------------------------------------------------------------
	// Initialize the packet buffer pointer & its max size
	//------------------------------------------------------------------------
		multi_packet_buf = Session.MetaPacket;
		multi_packet_max = Session.MetaSize;
		net = &Ipx;

	//------------------------------------------------------------------------
	// Debug stuff
	//------------------------------------------------------------------------
	Init_Queue_Mono(net);
	Update_Queue_Mono (net, 0);

	//------------------------------------------------------------------------
	// Compute the Game's CRC
	//------------------------------------------------------------------------
	Compute_Game_CRC();
	CRC[Frame & (ARRAY_SIZE(CRC) - 1)] = GameCRC;

	//------------------------------------------------------------------------
	// If we've just started a game, or loaded a multiplayer game, we must
	// wait for all other systems to signal ready.
	//------------------------------------------------------------------------
	if (Frame==0 || Session.LoadGame) {
		//.....................................................................
		// Initialize static locals
		//.....................................................................
		for (i = 0; i < MAX_PLAYERS - 1; i++) {
			TheirFrameSync[i].frame = -1;
			TheirFrameSync[i].sent = 0;
			TheirFrameSync[i].recv = 0;
		}
		skip_crc = Frame + ARRAY_SIZE(CRC);
		SentCommandCount = 0;
		for (i = 0; i < ARRAY_SIZE(CRC); i++)
			CRC[i] = 0;

		//.....................................................................
		// If we've loaded a saved game:
		// - If this game was saved as the result of a lost connection, clear
		//   the CRC value so it will always match the other system's
		// - Otherwise, use the GameCRC value, so we'll compare save-game files
		//   rather than scenario INI files
		//.....................................................................
		if (Session.LoadGame) {
			if (Session.EmergencySave)
				ScenarioCRC = 0;
			else
				ScenarioCRC = GameCRC;
		}

		//.....................................................................
		// Send our initial FRAMESYNC packet
		//.....................................................................
		SentFrameSyncCount = 0;
		SentFrameSyncTimer = BasicTimerClass<SystemTimerClass>();
		Send_FrameSync(net, SentCommandCount);

		//.....................................................................
		// Wait for the other guys
		//.....................................................................
		rc = Wait_For_Players (1, net, _timings[Session.Type].MIXFILE_RESEND_DELTA, _timings[Session.Type].FRAMESYNC_DLG_TIME,
			_timings[Session.Type].MIXFILE_TIMEOUT, multi_packet_buf, multi_packet_max,
			SentCommandCount, TheirFrameSync);

		if (rc != RC_NORMAL) {
			if (Session.Type == GAME_INTERNET){
				Register_Game_End_Time();
			}
			is_error = true;
			Session.Suspended++;
			if (rc == RC_NOT_RESPONDING) {
				WWMessageBox().Process (TXT_SYSTEM_NOT_RESPONDING, TXT_OK);
			}
			else if (rc == RC_SCENARIO_MISMATCH) {
				WWMessageBox().Process (TXT_SCENARIOS_DO_NOT_MATCH, TXT_OK);
			}
			else if (rc == RC_CANCEL) {
				is_error = false;
			}
			Session.Suspended--;
			Stop_Game(!is_error);
			return;
		}

		//.....................................................................
		// Re-initialize frame numbers (in case somebody signed off while I was
		//	waiting for MIX files to load; we would have fallen through, but
		//	their frame # would still be -1).
		//.....................................................................
		for (i = 0; i < MAX_PLAYERS - 1; i++) {
			Session.PlayerLatency[i] = 0;
			TheirFrameSync[i].frame = 0;
		}

		//.....................................................................
		// Reset the network response time computation, now that we're both
		// sending data again  (loading MIX files will have introduced
		// deceptively large values).
		//.....................................................................
		net->Reset_Response_Time(true);

		//.....................................................................
		// Initialize the frame timers
		//.....................................................................
		if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
			Process_Send_Period(net);//, 1);
		}

		//.....................................................................
		// Turn off our special load-game flags
		//.....................................................................
		if (Session.LoadGame) {
			Session.EmergencySave = false;
			Session.LoadGame = false;
		}

	} 	// end of Frame 0 wait

	//------------------------------------------------------------------------
	// Adjust connection timing parameters every 128 frames.
	//------------------------------------------------------------------------

	else if ( (Frame & 0x007f) == 0) {
		//
		// If we're using the new spiffy protocol, do proper timing handling.
		// If we're the net "master", compute our desired frame rate & new
		// 'MaxAhead' value.
		//
		//if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {

			// Every peer reports its processing time and worst local RTT.
			Generate_Network_Report_Event(net);

		//} else {
		// 	//
		// 	// For the older protocols, do the old broken timing handling.
		// 	//
		// 	Generate_Timing_Event(net, SentCommandCount);
		// }
	}

	// The deterministic master periodically evaluates the shared reports.
	if (Session.Am_I_Master() && (Session.PrecalcMaxAhead != 0 || Session.PrecalcDesiredFrameRate != 0 ||
		(Frame & (NetTiming::EVALUATION_INTERVAL - 1)) == 0)) {
		Generate_Real_Timing_Event();
	}

	//------------------------------------------------------------------------
	// Only process every 'FrameSendRate' frames
	//------------------------------------------------------------------------
	if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
		if (!Process_Send_Period(net)) {	//, 0)) {
			if (IsMono) {
				MonoClass::Disable();
			}
			return;
		}
	}

	//------------------------------------------------------------------------
	// Send our data packet(s); update my command-sent counter
	//------------------------------------------------------------------------
	SentCommandCount += Send_Packets(net, multi_packet_buf, multi_packet_max,
		Session.MaxAhead, SentCommandCount);

	//------------------------------------------------------------------------
	// If this is our first time through, we're done.
	//------------------------------------------------------------------------
	if (Frame==0) {
		if (IsMono) {
			MonoClass::Disable();
		}
		return;
	}

	//------------------------------------------------------------------------
	// Frame-sync'ing: wait until it's OK to advance to the next frame.
	//------------------------------------------------------------------------
	rc = Wait_For_Players (0, net,
	TIMER_SECOND, /// (Session.MaxAhead << 3),
	std::max((int) net->Response_Time() * 3, _timings[Session.Type].FRAMESYNC_TIMEOUT ),
	_timings[Session.Type].MIXFILE_TIMEOUT,
	multi_packet_buf, multi_packet_max, SentCommandCount, TheirFrameSync);

	if (rc != RC_NORMAL) {
		DebugString("Wait_For_Players returned %d\n", rc);
		if (Session.Type == GAME_INTERNET) {
			Register_Game_End_Time();
		}
		is_error = true;
		Session.Suspended++;
		if (rc == RC_NOT_RESPONDING) {
			WWMessageBox().Process (TXT_SYSTEM_NOT_RESPONDING, TXT_OK);
		}
		else if (rc == RC_SCENARIO_MISMATCH) {
			WWMessageBox().Process (TXT_SCENARIOS_DO_NOT_MATCH, TXT_OK);
		}
		else if (rc == RC_CANCEL) {
			is_error = false;
		}
		Session.Suspended--;
		Stop_Game(!is_error);
		return;
	}

	//------------------------------------------------------------------------
	// Save the DoList to disk, if we're in "Record" mode
	//------------------------------------------------------------------------
	if (Session.Record) {
		Queue_Record();
	}

	//------------------------------------------------------------------------
	// Execute the DoList; if an error occurs, bail out.
	//------------------------------------------------------------------------
	if (!Execute_DoList(Session.MaxPlayers, HOUSE_FIRST, net, &skip_crc, TheirFrameSync)) {
		DebugString("Failure executing DoList\n");
		if (Session.Type == GAME_INTERNET){
			Register_Game_End_Time();
		}
		Stop_Game();
		return;
	}

	//------------------------------------------------------------------------
	// Clean out the DoList
	//------------------------------------------------------------------------
	Clean_DoList(net);

	if (IsMono) {
		MonoClass::Disable();
	}

}	// end of Queue_AI_Multiplayer


/// <summary>
/// Waits for the other players to reach the end of the game.
/// This routine is called once the game is decided but the other systems may still be a
/// few frames behind. It keeps the connections serviced, the frame syncs flowing and the
/// map rendering until everybody has caught up, then signs off with a flurry of frame
/// syncs so nobody is left waiting on us. A player who never answers is given up on rather
/// than waited for forever.
/// </summary>
void Wait_For_End_Of_Queue(void)
{
	DebugString("Waiting to exit game\n");

	//........................................................................
	// Enums:
	//........................................................................
	enum {
		MIXFILE_RESEND_DELTA = 2 * TIMER_SECOND,     // ticks b/w resends
		MIXFILE_TIMEOUT = 2 * TIMER_MINUTE,          // timeout waiting for mixfiles.
		FRAMESYNC_DLG_TIME = (3*TIMER_SECOND),       // time until displaying reconnect dialog
		FRAMESYNC_TIMEOUT = (15*TIMER_SECOND),       // timeout waiting for frame sync packet
	};

	int timeout_factor = (Session.Type == GAME_INTERNET) ? 6 : 1;

	//........................................................................
	// Variables for sending, receiving & parsing packets:
	//........................................................................
	ConnManClass *net;					// ptr to access all multiplayer functions
	EventClass packet;					// for sending single frame-sync's
	char *multi_packet_buf;				// buffer for sending/receiving
	int multi_packet_max;				// max length of multi_packet_buf
	int their_oldest_frame;			// other players' oldest frame #
	int their_oldest_index;
	int i;


	//........................................................................
	// Variables for sending, receiving & parsing packets:
	//........................................................................
	EventClass *event;      // event ptr for parsing incoming packets
	int packetlen;          // size of meta-packet sent, & received
	int id;                 // id of other player
	int messages_this_loop; // to limit # messages processed each loop
	int message_limit;      // max # messages we'll read each frame
	RetcodeType rc;
	int num_ready;							// # players signalling ready
	num_ready = 0;

	int x,y;


	//------------------------------------------------------------------------
	// Initialize the packet buffer pointer & its max size
	//------------------------------------------------------------------------
		multi_packet_buf = Session.MetaPacket;
		multi_packet_max = Session.MetaSize;
		net = &Ipx;

	//------------------------------------------------------------------------
	// Send our data packet(s); update my command-sent counter
	//------------------------------------------------------------------------
	SentCommandCount += Send_Packets(net, multi_packet_buf, multi_packet_max,
		Session.MaxAhead, SentCommandCount);

	CDTimerClass<SystemTimerClass> timer1 = TIMER_SECOND / 4;
	CDTimerClass<SystemTimerClass> timer2 = 10 * TIMER_SECOND;
	BasicTimerClass<SystemTimerClass> timer3;

	DebugString("Entering wait loop\n");

	while (true) {

		Keyboard->Check();

		Update_Queue_Mono (net, 2);

		//---------------------------------------------------------------------
		// Resend a frame-sync packet if longer than one propagation delay goes
		// by; this prevents a "deadlock".  If he's waiting for me to advance,
		// but has missed my last few FRAMEINFO packets, I may be waiting for
		// him to advance.  Resending a FRAMESYNC ensures he knows what frame
		// number I'm on.
		//---------------------------------------------------------------------
		if (!timer1) {
			timer1 = TIMER_SECOND / 4;		// time to retry
			Update_Queue_Mono (net, 3);
			DebugString("Resending framesync\n");
			Send_FrameSync(net, SentCommandCount);
		}

		//---------------------------------------------------------------------
		// Service the connections
		//---------------------------------------------------------------------
		net->Service();

		if (!timer2) {
			DebugString("Timed out waiting for end game!\n");
			return;
		}


		//---------------------------------------------------------------------
		// Check for an incoming message.  We must still process commands
		// even if 'first_time' is set, in case the other system got my 1st
		// FRAMESYNC, but I didn't get his; he'll be at the next frame, and
		// may be sending commands.
		// We have to limit the number of incoming messages we handle; it's
		// possible to go into an infinite loop processing modem messages.
		// (This feature is disabled for Ten; we need to keep the TCP buffers
		// clear, so we read all the packets we can every time.)
		//---------------------------------------------------------------------
		messages_this_loop = 0;
		message_limit = MAX_EVENTS;

		while ( (messages_this_loop++ < message_limit) &&
			net->Get_Private_Message (multi_packet_buf, multi_packet_max, &packetlen, &id) ) {

			Keyboard->Check();

			Update_Queue_Mono (net, 5);

			/*..................................................................
			Get an event ptr to the incoming message
			..................................................................*/
			event = (EventClass *)multi_packet_buf;

			//------------------------------------------------------------------
				// Process the incoming packet
				//------------------------------------------------------------------
				rc = Process_Receive_Packet(net, multi_packet_buf, id, packetlen, TheirFrameSync, &timer3);

			//..................................................................
			// Service the connection, to clean out the receive queues
			//..................................................................
			net->Service();
		}

		Call_Back();

		if (SpecialDialog == SDLG_NONE) {
			KeyNumType key;
			Map.Input(key, x, y);
			if (key) {
				Keyboard_Process(key);
			}
			TacticalMap->AI();
			Map.Render();
		}

		//------------------------------------------------------------------------
		//	Find the oldest frame # in 'their_frame'
		//------------------------------------------------------------------------
		their_oldest_index = -1;
		their_oldest_frame = Frame + 1000;
		for (i = 0; i < net->Num_Connections(); i++) {
			if (TheirFrameSync[i].frame < their_oldest_frame) {
				their_oldest_frame = TheirFrameSync[i].frame;
				their_oldest_index = i;
			}
		}

		if (int(their_oldest_frame + Session.FrameSendRate) < Frame) {
			DebugString("Waiting for player %d on frame %d\n", their_oldest_index, TheirFrameSync[their_oldest_index].frame);
			continue;
		}
		break;

	}

	DebugString("their_oldest_frame = %d\n", their_oldest_frame);
	DebugString("All players have finished - ready to proceed to score screen\n");

	Send_FrameSync(net, SentCommandCount);
	Send_FrameSync(net, SentCommandCount);
	Send_FrameSync(net, SentCommandCount);
	Send_FrameSync(net, SentCommandCount);
}

/***************************************************************************
 * Wait_For_Players -- Waits for other systems to come on-line             *
 *                                                                         *
 * This routine performs the most critical logic in multiplayer; that of   *
 * synchronizing my frame number with those of the other systems.          *
 *                                                                         *
 * INPUT:                                                                  *
 *      first_time            1 = 1st time this routine is called          *
 *      net                  ptr to connection manager                     *
 *      resend_delta         time (ticks) between FRAMESYNC resends        *
 *      dialog_time            time (ticks) until pop up a reconnect dialog*
 *      timeout               time (ticks) until we give up the ghost      *
 *      multi_packet_buf      buffer to store packets in                   *
 *      my_sent               # commands I've sent so far                  *
 *      their_frame            array of their frame #'s                    *
 *      their_sent            array of their CommandCount values           *
 *      their_recv            array of # cmds I've received from them      *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      RC_NORMAL            OK to advance to the next frame               *
 *      RC_CANCEL            user hit 'Cancel' at the timeout countdown dlg*
 *      RC_NOT_RESPONDING      other player(s) not responding              *
 *      RC_SCENARIO_MISMATCH   scenario's don't match (first_time only)    *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int SyncWaitElapsed;		/// how long Wait_For_Players has been waiting
static RetcodeType Wait_For_Players(int first_time, ConnManClass *net,
	int resend_delta, int dialog_time, int timeout, char *multi_packet_buf,
	int multi_packet_max, int my_sent, FrameSyncStruct *their)
{
	//........................................................................
	// Variables for sending, receiving & parsing packets:
	//........................................................................
	EventClass *event;      // event ptr for parsing incoming packets
	int packetlen;          // size of meta-packet sent, & received
	int id;                 // id of other player
	int messages_this_loop; // to limit # messages processed each loop
	int message_limit;      // max # messages we'll read each frame

	//........................................................................
	// Variables used only if 'first_time':
	//........................................................................
	int num_ready;							// # players signalling ready

	/*
	 * ........................................................................
	 * Variables for the frame-sync stall display
	 * ........................................................................
	 */
	bool stall_drawn = false;			/// true = the current stall has been recorded
	int loop_count;						/// # of times through the wait loop

	//........................................................................
	// Timing variables
	//........................................................................
	CDTimerClass<SystemTimerClass> retry_timer;		// time between FRAMESYNC packet resends
	CDTimerClass<SystemTimerClass> dialog_timer;	// time to pop up a dialog
	CDTimerClass<SystemTimerClass> timeout_timer;	// general-purpose timeout
	BasicTimerClass<SystemTimerClass> timer;

	//........................................................................
	// Dialog variables
	//........................................................................
	int reconnect_dlg = 0;				// 1 = the reconnect dialog is displayed

	//........................................................................
	// Other misc variables
	//........................................................................
	KeyNumType input;   // for user input
	int x,y;            // for map input
	RetcodeType rc;

	//------------------------------------------------------------------------
	// Wait to hear from all other players
	//------------------------------------------------------------------------
	num_ready = 0;
	retry_timer = resend_delta; // time to retry
	dialog_timer = dialog_time; // time to show dlg
	timeout_timer = timeout;    // time to bail out
	timer = 0;
	loop_count = 0;

	while (1) {
		Keyboard->Check();

		Update_Queue_Mono (net, 2);

		//---------------------------------------------------------------------
		// Resend a frame-sync packet if longer than one propagation delay goes
		// by; this prevents a "deadlock".  If he's waiting for me to advance,
		// but has missed my last few FRAMEINFO packets, I may be waiting for
		// him to advance.  Resending a FRAMESYNC ensures he knows what frame
		// number I'm on.
		//---------------------------------------------------------------------
		if (!retry_timer) {
			retry_timer = resend_delta;		// time to retry
			Update_Queue_Mono (net, 3);
			Send_FrameSync(net, my_sent);
		}

		//---------------------------------------------------------------------
		// Service the connections
		//---------------------------------------------------------------------
		net->Service();

		//---------------------------------------------------------------------
		// Pop up a reconnect dialog if enough time goes by
		//---------------------------------------------------------------------
		if (!dialog_timer && SpecialDialog==SDLG_NONE) {
			int fresh = (reconnect_dlg==0);
			int reconn = (first_time==0);

			if (Process_Reconnect_Dialog(&timeout_timer, their,				//	(Returns immediately.)
				net->Num_Connections(), reconn, fresh, &timer)) {
				if (reconnect_dlg) {
					Close_Reconnect_Dialog();
				}
				return(RC_CANCEL);
			}

			/*
			 * ..................................................................
			 * Tally one pending kick proposal per pass.
			 * ..................................................................
			 */
			if (Session.KickProposals.Count() > 0) {
				Cast_Kick_Vote(Session.KickProposals[0]->Kick.KickerID, Session.KickProposals[0]->Kick.KickeeID);
				delete Session.KickProposals[0];
				Session.KickProposals.Delete_Index(0);
			}

			/*
			 * ..................................................................
			 * Kick anyone who has been voted out by all the other players.
			 * ..................................................................
			 */
			for (int i = 0; i < Session.Players.Count(); i++) {
				int const player_id = Session.Players[i]->Player.ID;
				if (player_id < 0 || player_id >= MAX_PLAYERS) {
					continue;
				}

				int votes = Session.KickVoteCount[player_id];
				if (votes >= Session.Players.Count() - 1) {
					DebugString("Kicking player %s from the game due to %d votes\n", Session.Players[i]->Name, votes);
					if (i == 0) {
						DebugString("Outvoted player is me!\n");
						if (reconnect_dlg) {
							Close_Reconnect_Dialog();
						}
						return(RC_CANCEL);
					}
					Kick_Player_Now(net, net->Connection_Index(player_id), their, true);
				}
			}

			reconnect_dlg = 1;
		}

		//---------------------------------------------------------------------
		// Exit if too much time goes by (the other system has crashed or
		// bailed)
		//---------------------------------------------------------------------
		if (!timeout_timer) {
			//..................................................................
			// For the first-time run, just give up; something's wrong.
			//..................................................................
			if (first_time) {
				if (reconnect_dlg) {
					Close_Reconnect_Dialog();
				}
				return(RC_NOT_RESPONDING);
			}
			//..................................................................
			// Otherwise, we're in the middle of a game; so, the modem &
			// network must deal with a timeout differently.
			//..................................................................
			else {
				Update_Queue_Mono (net, 4);

				if (Handle_Timeout(net, their)) {
					Map.Flag_To_Redraw(GS_REDRAW_ALL);	// erase modem reconnect dialog
					Map.Render();
					retry_timer = resend_delta;
					dialog_timer = dialog_time;
					timeout_timer = timeout;
				} else {
					if (reconnect_dlg) {
						Close_Reconnect_Dialog();
					}
					return(RC_NOT_RESPONDING);
				}
			}
		}

		//---------------------------------------------------------------------
		// Check for an incoming message.  We must still process commands
		// even if 'first_time' is set, in case the other system got my 1st
		// FRAMESYNC, but I didn't get his; he'll be at the next frame, and
		// may be sending commands.
		// We have to limit the number of incoming messages we handle; it's
		// possible to go into an infinite loop processing modem messages.
		//---------------------------------------------------------------------
		messages_this_loop = 0;
		message_limit = MAX_EVENTS;

		while ( (messages_this_loop++ < message_limit) &&
			net->Get_Private_Message (multi_packet_buf, multi_packet_max, &packetlen, &id) ) {

			Keyboard->Check();

			Update_Queue_Mono (net, 5);

			/*..................................................................
			Get an event ptr to the incoming message
			..................................................................*/
			event = (EventClass *)multi_packet_buf;

			//------------------------------------------------------------------
			// Process the incoming packet
			//------------------------------------------------------------------
			rc = Process_Receive_Packet(net, multi_packet_buf, id, packetlen, their, &timer);
			//..................................................................
			// New player heard from
			//..................................................................
			if (rc == RC_PLAYER_READY) {
				num_ready++;
			}
			//..................................................................
			// Scenario's don't match
			//..................................................................
			else if (rc == RC_SCENARIO_MISMATCH) {
				if (reconnect_dlg) {
					Close_Reconnect_Dialog();
				}
				return(RC_SCENARIO_MISMATCH);
			}

			//..................................................................
			// Service the connection, to clean out the receive queues
			//..................................................................
			net->Service();
		}

		//---------------------------------------------------------------------
		// Debug output
		//---------------------------------------------------------------------
		if (reconnect_dlg) {
			Print_Framesync_Values(Frame, Session.MaxAhead, net->Num_Connections(),
				their, my_sent);
		}

		//---------------------------------------------------------------------
		// Attempt to advance to the next frame.
		//---------------------------------------------------------------------
		//.....................................................................
		// For the first-time run, just check to see if we've heard from
		// everyone.
		//.....................................................................
		if (first_time) {
			if (num_ready >= net->Num_Connections()) {
				break;
			}
		}
		//.....................................................................
		// For in-game processing, we have to check their_sent, their_recv,
		// their_frame, etc.
		//.....................................................................
		else {
			int frame_stall;					/// who's holding up the frame count
			int count_stall;					/// who's holding up the command count

			if (Can_Advance(net, Session.MaxAhead, their, &frame_stall, &count_stall)) {
				break;
			}

			/*
			 * ..................................................................
			 * Record (and optionally display) who's stalling the game.
			 * ..................................................................
			 */
			int show_stall = 1;
			if (Session.ShowInternetDebug && loop_count > 0 && (!stall_drawn || frame_stall != -1 || count_stall != -1)) {
				Multiplayer_Debug_Print(true);
			} else if (stall_drawn) {
				show_stall = 0;
			}
			if (show_stall) {
				if (frame_stall != -1) {
					if (Session.ShowInternetDebug) {
						VisibleSurface->Fill_Rect(Rect(frame_stall + 100, 475, 35, 3), DSurface::Build_Hicolor_Pixel(255, 255, 40));
					}
					Session.ConnectionStats[frame_stall].FrameSyncStalls++;
				}
				if (count_stall != -1) {
					if (Session.ShowInternetDebug) {
						VisibleSurface->Fill_Rect(Rect(count_stall + 140, 475, 40, 3), DSurface::Build_Hicolor_Pixel(255, 40, 40));
					}
					Session.ConnectionStats[count_stall].CommandCountStalls++;
				}
				stall_drawn = true;
			}
		}

		//---------------------------------------------------------------------
		// Service game stuff.  Servicing the map's input, and rendering the
		// map, allows the map to scroll even though we're hung up waiting for
		// packets.  Don't do this if 'first_time' is set, since users could be
		// waiting a very long time for all systems to load the scenario, and
		// it gets frustrating being able to scroll around without doing
		// anything.
		//---------------------------------------------------------------------
		Call_Back();
		if (!first_time && SpecialDialog == SDLG_NONE && reconnect_dlg==0) {
			Map.Input(input, x, y);
			if (input)
				Keyboard_Process(input);
			TacticalMap->AI();
			Map.Render();
		}

		loop_count++;

	}	/* end of while */

	if (reconnect_dlg) {
		Close_Reconnect_Dialog();
	}
	if (stall_drawn && Session.ShowInternetDebug) {
		Rect rect(100, 475, 540, 4);
		VisibleSurface->Fill_Rect(rect, 0);
	}
	return(RC_NORMAL);

}	// end of Wait_For_Players


/***************************************************************************
 * Generate_Timing_Event -- computes & queues a RESPONSE_TIME event        *
 *                                                                         *
 * This routine adjusts the connection timing on the local system; it also *
 * optionally generates a RESPONSE_TIME event, to tell all systems to      *
 * dynamically adjust the current MaxAhead value.  This allows both the    *
 * MaxAhead & the connection retry logic to have dynamic timing, to adjust *
 * to varying line conditions.                                             *
 *                                                                         *
 * INPUT:                                                                  *
 *      net         ptr to connection manager                              *
 *      my_sent      # commands I've sent out so far                       *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Generate_Timing_Event(ConnManClass *net, int my_sent)
{
	unsigned int resp_time;			// connection response time, in ticks
	EventClass ev;

	//------------------------------------------------------------------------
	// Measure the current connection response time.  This time will be in
	// 60ths of a second, and represents full round-trip time of a packet.
	// To convert to one-way packet time, divide by 2; to convert to game
	// frames, divide again by 4, assuming a game rate of 15 fps.
	//------------------------------------------------------------------------
	resp_time = net->Response_Time();

	//------------------------------------------------------------------------
	//	Adjust my connection retry timing; only do this if I've sent out more
	// than 5 commands, so I know I have a measure of the response time.
	//------------------------------------------------------------------------
	if (my_sent > 5) {

		net->Set_Timing (resp_time + TIMER_SECOND / 6, -1, (resp_time * 4) + TIMER_SECOND / 4);

		//.....................................................................
		// If I'm the network "master", I'm also responsible for updating the
		// MaxAhead value on all systems, so do that here too.
		//.....................................................................
		if (Session.Am_I_Master()) {
			ev.Type = EventClass::RESPONSE_TIME;
			//..................................................................
			// For multi-frame compressed events, the MaxAhead must be an even
			// multiple of the FrameSendRate.
			//..................................................................
			if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
				ev.Data.FrameInfo.Delay = std::max( ((((resp_time / 8) +
					(Session.FrameSendRate - 1)) / Session.FrameSendRate) *
					Session.FrameSendRate), (Session.FrameSendRate * 2) );
			}
			//..................................................................
			// For sending packets every frame, just use the 1-way connection
			// response time.
			//..................................................................
			else {
				if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
					ev.Data.FrameInfo.Delay = std::max<unsigned int>( (resp_time / 8),
						 NETWORK_MIN_MAX_AHEAD );
				}
			}
			OutList.push_back(ev);
		}
	}

}	// end of Generate_Timing_Event


/// <summary>Maps the validated game-speed setting to its historical frame-rate target.</summary>
static int Game_Speed_Frame_Rate(void)
{
	switch (Options.GameSpeed) {
		case 0: return(60);
		case 1: return(45);
		case 2: return(30);
		case 3: return(20);
		case 4: return(15);
		case 5: return(12);
		case 6: return(10);
		default: return(60);
	}
}


/***************************************************************************
 * Generate_Real_Timing_Event -- Generates a TIMING event                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      Only the deterministic session master may call this routine.       *
 *                                                                         *
 * HISTORY:                                                                *
 *   07/02/1996 BRR : Created.                                             *
 *=========================================================================*/
static void Generate_Real_Timing_Event(void)
{
	EventClass event;
	memset(&event, 0, sizeof(event));

	if (Session.PrecalcMaxAhead != 0 || Session.PrecalcDesiredFrameRate != 0) {
		NetTiming::TimingSettings const settings{Session.PrecalcDesiredFrameRate > 30u ? 10u : 5u, static_cast<unsigned int>(Session.PrecalcMaxAhead)};
		if (Session.PrecalcDesiredFrameRate > 0 && Session.PrecalcDesiredFrameRate <= 60 && NetTiming::Timing_Settings_Are_Valid(settings)) {
			event.Type = EventClass::TIMING;
			event.Data.Timing.DesiredFrameRate = Session.PrecalcDesiredFrameRate;
			event.Data.Timing.MaxAhead = settings.MaxAhead;
			event.Data.Timing.FrameSendRate = settings.FrameSendRate;
			OutList.push_back(event);
		} else {
			DebugString("Ignoring invalid precalculated network timing values\n");
		}

		Session.PrecalcMaxAhead = 0;
		Session.PrecalcDesiredFrameRate = 0;
		return;
	}

	int highest_process_milliseconds = 0;
	for (int index = 0; index < Session.Players.Count(); index++) {
		NodeNameType const * player = Session.Players[index];
		if (player == NULL || player->Player.ProcessTime < 0) {
			return;
		}
		highest_process_milliseconds = std::max(highest_process_milliseconds, player->Player.ProcessTime);
	}

	unsigned int process_frame_rate = highest_process_milliseconds == 0 ? 60u : static_cast<unsigned int>(std::max(1, 1000 / highest_process_milliseconds));
	unsigned int const desired_frame_rate = std::min(process_frame_rate, static_cast<unsigned int>(Game_Speed_Frame_Rate()));
	NetTiming::TimingEvaluation const evaluation = Session.Evaluate_Network_Timing(desired_frame_rate, static_cast<unsigned int>(Frame));
	if (!evaluation.Changed && desired_frame_rate == static_cast<unsigned int>(Session.DesiredFrameRate)) {
		return;
	}

	event.Type = EventClass::TIMING;
	event.Data.Timing.DesiredFrameRate = desired_frame_rate;
	event.Data.Timing.MaxAhead = evaluation.Settings.MaxAhead;
	event.Data.Timing.FrameSendRate = evaluation.Settings.FrameSendRate;
	OutList.push_back(event);
}


/// <summary>Queues the local process-time and worst-RTT report.</summary>
static void Generate_Network_Report_Event(ConnManClass *net)
{
	if (Session.ProcessFrames <= 0) {
		return;
	}

	int const average_process_milliseconds = std::clamp(Session.ProcessTicks / Session.ProcessFrames, 0,
		static_cast<int>(NetTiming::MAXIMUM_PROCESS_MILLISECONDS));
	std::optional<NetTiming::Milliseconds> const worst_round_trip = net->Worst_Local_Round_Trip_MS();

	EventClass event;
	memset(&event, 0, sizeof(event));
	event.Type = EventClass::NETWORK_REPORT;
	event.Data.NetworkReport.AverageProcessMilliseconds = static_cast<std::uint16_t>(average_process_milliseconds);
	event.Data.NetworkReport.WorstRoundTripMilliseconds = !worst_round_trip || *worst_round_trip >= EventClass::NETWORK_RTT_UNAVAILABLE
		? EventClass::NETWORK_RTT_UNAVAILABLE : static_cast<std::uint16_t>(*worst_round_trip);
	OutList.push_back(event);

	Session.ProcessTicks = 0;
	Session.ProcessFrames = 0;
}


/***************************************************************************
 * Process_Send_Period -- timing for sending packets every 'n' frames      *
 *                                                                         *
 * This function is for a CommProtocol of COMM_PROTOCOL_MULTI_E_COMP only. *
 * It determines if it's time to send a packet or not.                     *
 *                                                                         *
 * INPUT:                                                                  *
 *      net      ptr to connection manager                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = it's time to send a packet; 0 = don't send a packet this frame.*
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Process_Send_Period(ConnManClass *net)	//, int init)
{
	//------------------------------------------------------------------------
	// If the current frame # is not an even multiple of 'FrameSendRate', then
	// it's not time to send a packet; just return.
	//------------------------------------------------------------------------
	if (Frame != (int)(((Frame + (Session.FrameSendRate - 1)) /
		Session.FrameSendRate) * Session.FrameSendRate) ) {

		net->Service();

		if (IsMono) {
			MonoClass::Disable();
		}

		return(0);
	}

	return(1);


}	// end of Process_Send_Period


/***************************************************************************
 * Send_Packets -- sends out events from the OutList                       *
 *                                                                         *
 * This routine computes how many events can be sent this frame, and then  *
 * builds the "meta-packet" & sends it.                                    *
 *                                                                         *
 * The 'cap' value is the max # of events we can send.  Ideally, it should *
 * be based upon the bandwidth of our connection.  Currently, it's just    *
 * hardcoded to prevent the modem from having to resend "too much" data,   *
 * which is about 200 bytes per frame.                                     *
 *                                                                         *
 * INPUT:                                                                  *
 *      net                  ptr to connection manager                     *
 *      multi_packet_buf      buffer to store packets in                   *
 *      multi_packet_max      max size of multi_packet_buf                 *
 *      max_ahead            current game MaxAhead value                   *
 *      my_sent               # commands I've sent this game               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      # events sent, NOT including the FRAMEINFO event                   *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Send_Packets(ConnManClass *net, char *multi_packet_buf,
	int multi_packet_max, int max_ahead, int my_sent)
{
	int cnt;
	int cap;        // max # events to send, NOT including FRAMEINFO event
	int ack_req;    // 0 = no ack required on outgoing packet
	int packetlen;  // size of meta-packet sent
	int processed;
	int num = 0;

	cap = (int)OutList.size();
	cnt = cap;

	//------------------------------------------------------------------------
	// Build our meta-packet & transmit it.
	//------------------------------------------------------------------------
	while (1) {
		//Keyboard->Check();

		Update_Queue_Mono (net, 1);

		//.....................................................................
		// If there are no commands this frame, we'll just be sending a FRAMEINFO
		//	packet; no ack is required.  For the modem's sake, check
		// Session.NumPlayers; no ACK is needed if we're just sending to someone
		// who's left the game.
		//.....................................................................
		if (cap == 0 || OutList.empty() || Session.NumPlayers == 1) {
			ack_req = 0;
		}
		else {
			ack_req = 1;
		}

		//.....................................................................
		// Build & send out our message
		//.....................................................................
		packetlen = Build_Send_Packet (multi_packet_buf, multi_packet_max,
			max_ahead, my_sent, cap, processed);

		cap -= processed;
		num += processed;
		if (processed) {
			ack_req = 1;
		}

		net->Send_Private_Message (multi_packet_buf, packetlen, ack_req);
		SentFrameSyncCount++;

		//.....................................................................
		// Call Service() to actually send the packet
		//.....................................................................
		net->Service();

		//.....................................................................
		// Stop if there's no more data to send, or if our send queue is
		// filling up.
		//.....................................................................
		if (OutList.empty() || (int)OutList.size() > cnt) {
			break;
		}
		if (cap == 0) {
			break;
		}
	}

	return(num);

}	// end of Send_Packets


/***************************************************************************
 * Send_FrameSync -- Sends a FRAMESYNC packet                              *
 *                                                                         *
 * This routine is used to periodically remind the other systems that      *
 * we're still here, and to tell them what frame # we're on, in case       *
 * they've missed my FRAMEINFO packets.                                    *
 *                                                                         *
 * INPUT:                                                                  *
 *      net            ptr to connection manager                           *
 *      cmd_count      # commands I've sent so far                         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Send_FrameSync(ConnManClass *net, int cmd_count)
{
	EventClass packet;

	//------------------------------------------------------------------------
	// Build a frame-sync event to send.  FRAMESYNC packets contain a
	// scenario-based CRC rather than a game-state-based CRC, to let the
	// games compare scenario CRC's on startup.
	//------------------------------------------------------------------------
	memset (&packet, 0, sizeof(EventClass));
	packet.Type = EventClass::FRAMESYNC;
	if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
		packet.Frame = ((Frame + Session.MaxAhead + (Session.FrameSendRate - 1)) /
			 Session.FrameSendRate) * Session.FrameSendRate;
	}
	else {
		packet.Frame = Frame + Session.MaxAhead;
	}
	packet.ID = PlayerPtr->HeapID;
	packet.Data.FrameInfo.CRC = ScenarioCRC;
	packet.Data.FrameInfo.CommandCount = cmd_count;
	packet.Data.FrameInfo.Delay = Session.MaxAhead;

	//------------------------------------------------------------------------
	// Send the event.  For modem, this just sends to the other player;
	// for network, it sends to everyone we're connected to.
	//------------------------------------------------------------------------

	net->Send_Private_Message (&packet, (offsetof(EventClass, Data) +
		size_of(EventClass, Data.FrameInfo)), 0 );
	SentFrameSyncCount++;
	return;

}	// end of Send_FrameSync


/***************************************************************************
 * Process_Receive_Packet -- processes an incoming packet                  *
 *                                                                         *
 * This routine receives a packet from another system, adds it to our      *
 * execution queue (the DoList), and updates my arrays of their frame #,   *
 * their commands-sent, and their commands-received.                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      net               ptr to connection manager                        *
 *      multi_packet_buf   buffer containing packet(s) to parse            *
 *      id                  id of sender                                   *
 *      their_frame         array containing frame #'s of other players    *
 *      their_sent         array containing command count of other players *
 *      their_recv         array containing # recv'd cmds from other players*
 *                                                                         *
 * OUTPUT:                                                                 *
 *      RC_NORMAL:               nothing unusual happened, although        *
 *                            their_sent or their_recv may have been       *
 *                              altered                                    *
 *      RC_PLAYER_READY:         player has been heard from for the 1st time;*
 *                            this presumes that his original              *
 *                            'their_frame[]' value was -1 when this       *
 *                            routine was called                           *
 *      RC_SCENARIO_MISMATCH:   FRAMEINFO scenario CRC doesn't match;      *
 *                            normally only applies after loading a new    *
 *                            scenario or save-game                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static RetcodeType Process_Receive_Packet(ConnManClass *net,
	char *multi_packet_buf, int id, int packetlen, FrameSyncStruct *their, BasicTimerClass<SystemTimerClass> *timer)
{
	RetcodeType retcode = RC_NORMAL;
	NetPacket::Encoding const encoding = Session.CommProtocol == COMM_PROTOCOL_SINGLE_NO_COMP
		? NetPacket::Encoding::UNCOMPRESSED : NetPacket::Encoding::COMPRESSED;
	std::span<std::byte const> const packet(reinterpret_cast<std::byte const *>(multi_packet_buf), packetlen > 0 ? static_cast<std::size_t>(packetlen) : 0);
	// Validate the complete packet before mutating peer state or DoList.
	NetPacket::DecodeResult decoded = NetPacket::Decode_Event_Packet(packet, encoding, id);
	if (!decoded.Succeeded() || !decoded.HasEnvelope) {
		Record_Network_Packet_Drop(decoded.Succeeded() ? NetPacket::DecodeError::INVALID_PREFIX : decoded.Failure.Code);
		return(RC_NORMAL);
	}

	EventClass const & event = decoded.Envelope;

	//------------------------------------------------------------------------
	// Get the index of the sender
	//------------------------------------------------------------------------
	int const index = net->Connection_Index(id);
	if (index < 0 || index >= net->Num_Connections()) {
		Record_Network_Packet_Drop(NetPacket::DecodeError::INVALID_CONNECTION);
		return(RC_NORMAL);
	}

	//------------------------------------------------------------------------
	//	Compute the other player's frame # (at the time this packet was sent)
	//------------------------------------------------------------------------
	int const frame = event.Frame - event.Data.FrameInfo.Delay;
	if (their[index].frame < frame) {

		//.....................................................................
		// If the original frame # for this player is -1, it means we've heard
		// from this player for the 1st time; return the appropriate value.
		//.....................................................................
		if (their[index].frame==-1) {
			retcode = RC_PLAYER_READY;
		}

		their[index].frame = frame;

		if (Session.Type != GAME_INTERNET) {
			Session.PlayerLatency[index] = 0;
		} else {
			unsigned int f = Ipx.Avg_Response_Time(index) / 2;
			f *= LastFramesPerSecond;
			Session.PlayerLatency[index] = (Frame - (f / TIMER_SECOND)) - frame;
		}
	}

	//------------------------------------------------------------------------
	// Extract the other player's CommandCount.  This count will include
	// the commands in this packet, if there are any.
	//------------------------------------------------------------------------
	if (event.Data.FrameInfo.CommandCount > their[index].sent) {

		if ( abs((int)(their[index].sent - event.Data.FrameInfo.CommandCount)) > 500) {
			FILE *fp;
			fp = fopen("badcount.txt","wt");
			if (fp) {
				fprintf(fp,"Event Type:%s\n",EventClass::EventNames[event.Type]);
				fprintf(fp,"Frame:%d  ID:%d  IsExec:%d\n",
					event.Frame,
					event.ID,
					event.IsExecuted);
				if (event.Type != EventClass::FRAMEINFO) {
					fprintf(fp,"!!!!!!!!! bad bug, bad bug !!!!!!!!!\n");//fprintf(fp,"Wrong Event Type!\n");
				} else {
					fprintf(fp,"CRC:%x  CommandCount:%d  Delay:%d\n",
						event.Data.FrameInfo.CRC,
						event.Data.FrameInfo.CommandCount,
						event.Data.FrameInfo.Delay);
				}
				fclose(fp);
			}
		}

		their[index].sent = event.Data.FrameInfo.CommandCount;
	}

	//------------------------------------------------------------------------
	// If this packet was not a FRAMESYNC packet:
	// - Add the events in it to our DoList
	// - Increment our commands-received counter by the number of non-
	//   FRAMEINFO packets received
	//------------------------------------------------------------------------
	if (event.Type != EventClass::FRAMESYNC) {
		for (NetPacket::DecodedEvent const & source : decoded.Events) {
			EventClass queued = source.Event;
			if (queued.Type == EventClass::ADDPLAYER) {
				queued.Data.Variable.Pointer = NULL;
				if (!source.AddPlayerData.empty()) {
					queued.Data.Variable.Pointer = new char[source.AddPlayerData.size()];
					memcpy(queued.Data.Variable.Pointer, source.AddPlayerData.data(), source.AddPlayerData.size());
				}
			}
			DoList.push_back(queued);
		}

		std::size_t const commands = decoded.Events.empty() ? 0 : decoded.Events.size() - 1;
		their[index].recv += static_cast<unsigned short>(commands);
	}

	//------------------------------------------------------------------------
	// If the event was a FRAMESYNC packet, there will be no commands to add,
	// but we must check the ScenarioCRC value.
	//------------------------------------------------------------------------
	else if (event.Type == EventClass::FRAMESYNC) {
		if (event.Data.FrameInfo.CRC != ScenarioCRC) {
			return(RC_SCENARIO_MISMATCH);
		}
		their[index].timing = *timer;
	}

	return(retcode);

}	// end of Process_Receive_Packet


/***************************************************************************
 * Can_Advance -- determines if it's OK to advance to the next frame       *
 *                                                                         *
 * This routine uses the current values stored in their_frame[],           *
 * their_send[], and their_recv[] to see if it's OK to advance to the next *
 * game frame.  We must not advance if:                                    *
 * - If our frame # would be too far ahead of the slowest player (the      *
 *   lowest their_frame[] value).  "Too far" means                         *
 *   (Frame >= their_frame + MaxAhead).                                    *
 * - our current command count doesn't match the sent command count of one *
 *   other player (meaning that we've missed a command packet from that    *
 *   player, and thus the frame # we're receiving from him may be due to a *
 *   FRAMEINFO packet sent later than the command, so we shouldn't use     *
 *   this frame # to see if we should advance; we should wait until we     *
 *   have all the commands before we advance.                              *
 *                                                                         *
 * Of course, this routine assumes the values in their_frame[] etc are     *
 * kept current by the caller.                                             *
 *                                                                         *
 * INPUT:                                                                  *
 *      net            ptr to connection manager                           *
 *      max_ahead      max frames ahead                                    *
 *      their_frame      array of their frame #'s                          *
 *      their_sent      array of their sent command count                  *
 *      their_recv      array of their # received commands                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK to advance; 0 = not OK                                      *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Can_Advance(ConnManClass *net, int max_ahead, FrameSyncStruct *their, int *frame_stall, int *count_stall)
{
	int their_oldest_frame;    // other players' oldest frame #
	int count_ok;               // true = my cmd count matches theirs
	int i;

	*frame_stall = -1;

	//------------------------------------------------------------------------
	// Special case for modem: if the other player has left, go ahead and
	// advance to the next frame; don't wait on him.
	//------------------------------------------------------------------------
	if (Session.NumPlayers == 1) {
		return(1);
	}

	//------------------------------------------------------------------------
	//	Find the oldest frame # in 'their_frame'
	//------------------------------------------------------------------------
	their_oldest_frame = Frame + 1000;
	for (i = 0; i < net->Num_Connections(); i++) {
		if (their[i].frame < their_oldest_frame) {
			their_oldest_frame = their[i].frame;
			*frame_stall = i;
		}
	}

	//------------------------------------------------------------------------
	// I can advance to the next frame IF:
	// 1) I'm less than a one-way propagation delay ahead of the other
	//    players' frame numbers, AND
	//	2) their_recv[i] >= their_sent[i] (ie I've received all the commands
	//    the other players have sent so far).
	//------------------------------------------------------------------------
	count_ok = 1;
	for (i = 0; i < net->Num_Connections(); i++) {
		if (their[i].recv < their[i].sent) {
			count_ok = 0;
			break;
		}
	}
	if (count_ok) {
		if (Frame < (their_oldest_frame + max_ahead)) {
			return(1);
		}
		i = -1;
	}

	//------------------------------------------------------------------------
	/// If the frame # isn't the bottleneck, don't blame anyone for it.
	//------------------------------------------------------------------------
	if (Frame < (their_oldest_frame + max_ahead)) {
		*frame_stall = -1;
	}
	*count_stall = i;

	return(0);

}	// end of Can_Advance


/***************************************************************************
 * Process_Reconnect_Dialog -- processes the reconnection dialog           *
 *                                                                         *
 * This routine [re]draws the reconnection dialog; if 'reconn' is set,     *
 * it tells the user who we're trying to reconnect to; otherwise, is just  *
 * says something generic like "Waiting for connections".                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      timeout_timer   ptr to count down timer, showing time remaining    *
 *      their_frame      array of other players' frame #'s                 *
 *      num_conn         # connections in 'their_frame'                    *
 *      reconn         1 = reconnect, 0 = waiting for first-time connection*
 *      fresh            1 = draw from scratch, 0 = only update time counter*
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = user wants to cancel, 0 = not                                  *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Process_Reconnect_Dialog(CDTimerClass<SystemTimerClass> *timeout_timer,
	FrameSyncStruct *their, int num_conn, int reconn, int fresh,
	BasicTimerClass<SystemTimerClass> *timer)
{
	static int displayed_time = 0;	// time value currently displayed

	static HWND disconnect_dialog;  /// the disconnect/kick dialog
	static int disconnect_return;   /// set to IDCANCEL by Reconnect_Dialog_Proc

	int new_time;
	int oldest_index;						// index of person requiring a reconnect
	int i,j;
	char buf[256];							// for dialog text

	//------------------------------------------------------------------------
	/// Update the frame-sync progress info for Draw_Sync_Bars.
	//------------------------------------------------------------------------
	SyncWaitElapsed = *timer;
	for (i = 0; i < num_conn; i++) {
		SyncBarFrameSync[i] = their[i];
	}

	//------------------------------------------------------------------------
	/// The first time through, create the disconnect/kick dialog.
	//------------------------------------------------------------------------
	if (fresh) {
		TacticalActive = false;
		disconnect_return = -1;
		disconnect_dialog = WS_Create_Dialog(ProgramInstance, IDD_MPLAYER_DISCONNECT, MainWindow, Reconnect_Dialog_Proc, true);
		Center_Window_Within_Window(disconnect_dialog);
		if (disconnect_dialog) {
			SetWindowLong(disconnect_dialog, DWL_USER, (LONG)&disconnect_return);
			MouseCursor->Hide_Mouse();
			ShowWindow(disconnect_dialog, SW_SHOWNORMAL);
			UpdateWindow(disconnect_dialog);
			MouseCursor->Show_Mouse();
		}
	}

	//------------------------------------------------------------------------
	/// If the user hit Cancel, bail out of the game.
	//------------------------------------------------------------------------
	if (disconnect_return == IDCANCEL) {
		WS_Destroy_Dialog(disconnect_dialog, false);
		TacticalActive = true;
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
		return(1);
	}

	//------------------------------------------------------------------------
	// Convert the timer to seconds
	//------------------------------------------------------------------------
	new_time = *timeout_timer / TIMER_SECOND;

	//------------------------------------------------------------------------
	// If the timer has changed, or 'fresh' is set, redraw the dialog
	//------------------------------------------------------------------------
	if (fresh || new_time != displayed_time) {
		displayed_time = new_time;

		HWND item = GetDlgItem(disconnect_dialog, IDC_DISCONNECT_TIME_REMAINING);
		if (item) {
			sprintf(buf, Fetch_String(TXT_TIME_ALLOWED), displayed_time);
			SendMessage(item, WM_SETTEXT, 0, (LPARAM)buf);
		}
		if (!(displayed_time & 1)) {
			PostMessage(disconnect_dialog, WM_PAINT, 0, 0);
		}

		/*
		 * On creation, discard any stale kick proposals, clear the vote
		 * tallies, and fill the message list box.
		 */
		if (fresh) {
			while (Session.KickProposals.Count()) {
				delete Session.KickProposals[0];
				Session.KickProposals.Delete_Index(0);
			}
			memset(Session.KickVoteCount, 0, sizeof(Session.KickVoteCount));
			memset(Session.KickVoteWho, 0xFF, sizeof(Session.KickVoteWho));

			HWND listbox = GetDlgItem(disconnect_dialog, IDC_DISCONNECT_MESSAGES);
			if (listbox) {
				if (reconn) {
					//...............................................................
					// Find the index of the person we're trying to reconnect to
					//...............................................................
					j = 0x7fffffff;
					oldest_index = 0;
					for (i = 0; i < num_conn; i++) {
						if (their[i].frame < j) {
							j = their[i].frame;
							oldest_index = i;
						}
					}
					if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
						sprintf(buf, Fetch_String(TXT_RECONNECTING_TO), Ipx.Connection_Name(Ipx.Connection_ID(oldest_index)));
					} else {
						sprintf(buf, Fetch_String(TXT_RECONNECTING_TO), Session.Players[1]->Name);
					}
					ListBox_AddString(listbox, buf);
					ListBox_AddString(listbox, "");
					if (Session.Type == GAME_INTERNET) {
						ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP3));
						ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP3B));
						ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP3C));
					}
					ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP2));
					if (Session.Type == GAME_INTERNET) {
						ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP2B));
					} else if (Session.Type == GAME_IPX) {
						ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP4));
					}
					ListBox_AddString(listbox, "");
					ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP5));
					ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_HELP1));
					ListBox_AddString(listbox, "");
				} else {
					sprintf(buf, Fetch_String(TXT_WAITING_FOR_CONNECTIONS));
					ListBox_AddString(listbox, buf);
				}
			}
		}
	}

	return(0);

}	// end of Process_Reconnect_Dialog

static int SyncNameButtonControlsIDs[MAX_PLAYERS] = {
	IDC_DISCONNECT_PLAYER1,
	IDC_DISCONNECT_PLAYER2,
	IDC_DISCONNECT_PLAYER3,
	IDC_DISCONNECT_PLAYER4,
	IDC_DISCONNECT_PLAYER5,
	IDC_DISCONNECT_PLAYER6,
	IDC_DISCONNECT_PLAYER7,
	IDC_DISCONNECT_PLAYER8
};
static int SyncBarControlIDs[MAX_PLAYERS] = {
	IDC_DISCONNECT_PLAYER1_BOX,
	IDC_DISCONNECT_PLAYER1_BOX,
	IDC_DISCONNECT_PLAYER3_BOX,
	IDC_DISCONNECT_PLAYER4_BOX,
	IDC_DISCONNECT_PLAYER5_BOX,
	IDC_DISCONNECT_PLAYER6_BOX,
	IDC_DISCONNECT_PLAYER7_BOX,
	IDC_DISCONNECT_PLAYER8_BOX
};


/// <summary>
/// Fetches the connection index for a player.
/// </summary>
/// <param name="player">The player ID to look up.</param>
/// <returns>Returns with the connection index that belongs to the player.</returns>
static int Connection_Index(int player)
{
	return(Ipx.Connection_Index(player));
}


/// <summary>
/// Draws the frame sync bars on the reconnect dialog.
/// Every player in the game gets a bar that shrinks and changes color as the wait on that
/// player drags on, so the humans can see who the game is actually stalled on.
/// </summary>
/// <param name="window">The reconnect dialog that owns the bar controls.</param>
void Draw_Sync_Bars(HWND window)
{
	for (int i = 0; i < Session.Players.Count(); i++) {
		RECT bar_winrect;
		Get_Display_Rect(GetDlgItem(window, SyncBarControlIDs[i]), &bar_winrect);

		Rect bar_rect;
		bar_rect.X = bar_winrect.left;
		bar_rect.Y = bar_winrect.top;
		bar_rect.Width = bar_winrect.right - bar_winrect.left;
		bar_rect.Height = bar_winrect.bottom - bar_winrect.top;

		int playerid = Connection_Index(Session.Players[i]->Player.ID);

		unsigned progress;
		if (i == 0) {
			progress = 0;
		} else {
			progress = SyncWaitElapsed - SyncBarFrameSync[playerid].timing;
		}

		unsigned short color = DSurface::Build_Hicolor_Pixel(0, 200, 0);
		if (progress > 240) {
			color = DSurface::Build_Hicolor_Pixel(200, 200, 0);
			if (progress > 480) {
				color = DSurface::Build_Hicolor_Pixel(200, 0, 0);
			}
		}

		int w = std::max(100 - (int)(100 * progress / 1200), 0) * bar_rect.Width;
		bar_rect.Width = std::max(6, w / 100);

		AlternateSurface->Fill_Rect(AlternateSurface->Get_Rect(), bar_rect, color);
	}
}

bool Cast_Kick_Vote(int kicker, int kickee);


/// <summary>Finds an active session player by stable ID.</summary>
static NodeNameType * Current_Player_From_ID(int player)
{
	if (player < 0 || player >= MAX_PLAYERS) {
		return(NULL);
	}

	for (int index = 0; index < Session.Players.Count(); index++) {
		NodeNameType * current = Session.Players[index];
		if (current != NULL && current->Player.ID == player) {
			return(current);
		}
	}
	return(NULL);
}


/// <summary>Tests whether a player has already cast a counted kick vote.</summary>
static bool Kick_Vote_Already_Cast(int kicker, int kickee)
{
	if (kicker < 0 || kicker >= MAX_PLAYERS || kickee < 0 || kickee >= MAX_PLAYERS) {
		return(false);
	}

	int const votes = Session.KickVoteCount[kickee];
	if (votes < 0 || votes > MAX_PLAYERS) {
		return(false);
	}
	for (int index = 0; index < votes; index++) {
		if (Session.KickVoteWho[kickee][index] == kicker) {
			return(true);
		}
	}
	return(false);
}


/// <summary>Tests whether the pending queue already contains a kick vote.</summary>
static bool Kick_Proposal_Already_Pending(int kicker, int kickee)
{
	for (int index = 0; index < Session.KickProposals.Count(); index++) {
		GlobalPacketType const * proposal = Session.KickProposals[index];
		if (proposal != NULL && proposal->Kick.KickerID == static_cast<unsigned int>(kicker) &&
			proposal->Kick.KickeeID == static_cast<unsigned int>(kickee)) {
			return(true);
		}
	}
	return(false);
}


/// <summary>Removes a departing player from pending and counted kick votes.</summary>
void Forget_Kick_Player(int player)
{
	if (player < 0 || player >= MAX_PLAYERS) {
		return;
	}

	for (int index = Session.KickProposals.Count() - 1; index >= 0; index--) {
		GlobalPacketType * proposal = Session.KickProposals[index];
		if (proposal == NULL || proposal->Kick.KickerID == static_cast<unsigned int>(player) ||
			proposal->Kick.KickeeID == static_cast<unsigned int>(player)) {
			delete proposal;
			Session.KickProposals.Delete_Index(index);
		}
	}

	for (int target = 0; target < MAX_PLAYERS; target++) {
		int retained[MAX_PLAYERS];
		int retained_count = 0;
		int const votes = Session.KickVoteCount[target];
		if (target != player && votes >= 0 && votes <= MAX_PLAYERS) {
			for (int index = 0; index < votes; index++) {
				int const voter = Session.KickVoteWho[target][index];
				if (voter != player && Current_Player_From_ID(voter) != NULL) {
					retained[retained_count++] = voter;
				}
			}
		}

		memset(Session.KickVoteWho[target], 0xFF, sizeof(Session.KickVoteWho[target]));
		if (retained_count > 0) {
			memcpy(Session.KickVoteWho[target], retained, retained_count * sizeof(retained[0]));
		}
		Session.KickVoteCount[target] = retained_count;
	}
}

/// <summary>
/// Trims the message list box and scrolls it to the end.
/// Use this routine after adding a line to the reconnect dialog's message list, so that
/// the list stays a manageable length and the newest message stays in view.
/// </summary>
/// <param name="listbox">The list box to trim.</param>
void ListBox_Trim(HWND listbox)
{
	int string_count = ListBox_GetCount(listbox);
	if (string_count > 50) {
		ListBox_DeleteString(listbox, 0);
		string_count--;
	}
	ListBox_SetTopIndex(listbox, string_count - 1);
}


/// <summary>
/// Proposes that a player be kicked out of the game.
/// This routine is called when one of the kick buttons on the reconnect dialog is pressed.
/// The proposal is sent to every other player and the local vote is cast right away.
/// Proposing to kick yourself, or to kick anybody at all during a tournament game, earns
/// nothing but a message in the dialog.
/// </summary>
/// <param name="window">The reconnect dialog to report the outcome in.</param>
/// <param name="id">Index into the session player list of the one to be kicked.</param>
void Propose_Kick_Player(HWND window, int id)
{
	if (id < 0 || id >= Session.Players.Count()) {
		return;
	}

	DebugString("Propose_Kick_Player %d - %s. Local id is %d\n", id, Session.Players[id]->Name, Session.Players[0]->Player.ID);
	HWND listbox = GetDlgItem(window, IDC_DISCONNECT_MESSAGES);

	if (id == 0) {
		ListBox_AddString(listbox, Fetch_String(TXT_RECONNECT_KICK_SELF));
		ListBox_Trim(listbox);
		return;
	}

	if (Session.Type == GAME_INTERNET && WestwoodOnline_Tournament) {
		ListBox_AddString(listbox, Fetch_String(TXT_CANT_KICK));
		ListBox_Trim(listbox);
		return;
	}

	int const kicker = Session.Players[0]->Player.ID;
	int const kickee = Session.Players[id]->Player.ID;
	if (Current_Player_From_ID(kicker) == NULL || Current_Player_From_ID(kickee) == NULL || Kick_Vote_Already_Cast(kicker, kickee)) {
		return;
	}

	GlobalPacketType gpacket;
	NetGlobal::Initialize_Packet(gpacket, NET_PROPOSE_KICK);
	strncpy(gpacket.Name, Session.Players[0]->Name, ARRAY_SIZE(gpacket.Name) - 1);
	gpacket.Name[ARRAY_SIZE(gpacket.Name) - 1] = '\0';
	gpacket.Kick.KickerID = static_cast<unsigned int>(kicker);
	gpacket.Kick.KickeeID = static_cast<unsigned int>(kickee);

	for (int i = 1; i < Session.Players.Count(); i++) {
		DebugString("Sending kick proposal to %s\n", Session.Players[i]->Name);
		Ipx.Send_Global_Message(&gpacket, sizeof(gpacket), 1, &Session.Players[i]->Address);
	}

	Cast_Kick_Vote(kicker, kickee);
}


/// <summary>Queues a bounded, canonical kick proposal from a session member.</summary>
NetGlobal::DecodeError Kick_Packet_Received(int kicker, int kickee)
{
	NodeNameType * kicker_player = Current_Player_From_ID(kicker);
	NodeNameType * kickee_player = Current_Player_From_ID(kickee);
	if (kicker_player == NULL || kickee_player == NULL) {
		return(NetGlobal::DecodeError::INVALID_KICK_PLAYER);
	}
	if (kicker == kickee) {
		return(NetGlobal::DecodeError::SELF_KICK);
	}
	if (Kick_Vote_Already_Cast(kicker, kickee) || Kick_Proposal_Already_Pending(kicker, kickee)) {
		return(NetGlobal::DecodeError::DUPLICATE_KICK_PROPOSAL);
	}
	if (Session.KickProposals.Count() >= MAX_PLAYERS * MAX_PLAYERS) {
		return(NetGlobal::DecodeError::KICK_PROPOSAL_QUEUE_FULL);
	}

	GlobalPacketType * newpacket = new GlobalPacketType;
	NetGlobal::Initialize_Packet(*newpacket, NET_PROPOSE_KICK);
	strncpy(newpacket->Name, kicker_player->Name, ARRAY_SIZE(newpacket->Name) - 1);
	newpacket->Name[ARRAY_SIZE(newpacket->Name) - 1] = '\0';
	newpacket->Kick.KickerID = static_cast<unsigned int>(kicker);
	newpacket->Kick.KickeeID = static_cast<unsigned int>(kickee);
	if (!Session.KickProposals.Add(newpacket)) {
		delete newpacket;
		return(NetGlobal::DecodeError::KICK_PROPOSAL_QUEUE_FULL);
	}

	return(NetGlobal::DecodeError::NONE);
}


/// <summary>Records one bounded, deduplicated kick vote.</summary>
bool Cast_Kick_Vote(int kicker, int kickee)
{
	char buffer[256];
	NodeNameType * kicker_player = Current_Player_From_ID(kicker);
	NodeNameType * kickee_player = Current_Player_From_ID(kickee);
	if (kicker_player == NULL || kickee_player == NULL || kicker == kickee) {
		return(false);
	}
	if (Kick_Vote_Already_Cast(kicker, kickee)) {
		return(false);
	}

	int votes = Session.KickVoteCount[kickee];
	if (votes < 0 || votes >= MAX_PLAYERS) {
		return(false);
	}

	Session.KickVoteWho[kickee][votes] = kicker;
	Session.KickVoteCount[kickee]++;

	if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
		DebugString("Player %s votes to kick player %s from the game\n", kicker_player->Name, kickee_player->Name);
		snprintf(buffer, sizeof(buffer), Fetch_String(TXT_RECONNECT_KICK_RECEIVED), kicker_player->Name, kickee_player->Name);

		HWND topwindow = WS_Top_Window();
		HWND listbox = GetDlgItem(topwindow, IDC_DISCONNECT_MESSAGES);
		ListBox_AddString(listbox, buffer);
		ListBox_Trim(listbox);
	}

	return(true);
}


/// <summary>
/// Handles the messages for the reconnect dialog.
/// This is the dialog that appears when the game stalls waiting on somebody. It paints the
/// per-player sync bars and offers a kick button for each player in the game.
/// </summary>
BOOL CALLBACK Reconnect_Dialog_Proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	int * rc = (int *)GetWindowLong(window, DWL_USER);

	switch (message) {
		case IDCANCEL:
			Remove_Modeless_Dialog(window);
			break;

		case WM_DRAWITEM:
			OwnerDraw::Draw_Item((DRAWITEMSTRUCT *)lparam);
			return(TRUE);

		case WM_PAINT:
			OwnerDraw::Draw_Dialog_Back(window);
			Draw_Sync_Bars(window);
			ValidateRect(window, NULL);
			break;

		case WM_INITDIALOG: {
			OwnerDraw::Subclass_Dialog(window, 0);
			Center_Window_Within_Window(window);
			Add_Modeless_Dialog(window);

			int i;
			for (i = 0; i < MAX_PLAYERS; i++) {
				HWND button = GetDlgItem(window, SyncNameButtonControlsIDs[i]);
				EnableWindow(button, FALSE);
				HWND bar = GetDlgItem(window, SyncBarControlIDs[i]);
				EnableWindow(bar, FALSE);
			}

			for (i = 0; i < MAX_PLAYERS; i++) {
				HWND button = GetDlgItem(window, SyncNameButtonControlsIDs[i]);
				HWND bar = GetDlgItem(window, SyncBarControlIDs[i]);
				if (i < Session.Players.Count()) {
					SendMessage(button, WM_SETTEXT, 0, (LPARAM)Session.Players[i]->Name);
					EnableWindow(button, TRUE);
					EnableWindow(bar, TRUE);
				} else {
					DestroyWindow(button);
					DestroyWindow(bar);
				}
			}
			break;
		}

		case WM_MOVING:
			return(On_WM_MOVING(window, wparam, lparam));

		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSCROLLBAR:
		case WM_CTLCOLORSTATIC:
			return((BOOL)GetStockObject(BLACK_BRUSH));

		case WM_ERASEBKGND:
			return(TRUE);

		case WM_COMMAND:
			switch (LOWORD(wparam)) {
				case IDC_DISCONNECT_PLAYER1:
					Propose_Kick_Player(window, 0);
					break;

				case IDC_DISCONNECT_PLAYER2:
					Propose_Kick_Player(window, 1);
					break;

				case IDC_DISCONNECT_PLAYER3:
					Propose_Kick_Player(window, 2);
					break;

				case IDC_DISCONNECT_PLAYER4:
					Propose_Kick_Player(window, 3);
					break;

				case IDC_DISCONNECT_PLAYER5:
					Propose_Kick_Player(window, 4);
					break;

				case IDC_DISCONNECT_PLAYER6:
					Propose_Kick_Player(window, 5);
					break;

				case IDC_DISCONNECT_PLAYER7:
					Propose_Kick_Player(window, 6);
					break;

				case IDC_DISCONNECT_PLAYER8:
					Propose_Kick_Player(window, 7);
					break;

				case IDCANCEL:
					*rc = IDCANCEL;
					break;
			}
			break;
	}

	return(FALSE);
}


/// The name comes from the TS demo build, which ships this routine with symbols.

/// <summary>
/// Closes the reconnect dialog if it is showing.
/// This routine is called once the stalled players have caught up. The tactical map is
/// switched back on and forced to redraw, since the dialog was sitting on top of it.
/// </summary>
static void Close_Reconnect_Dialog(void)
{
	//------------------------------------------------------------------------
	// If the reconnect dialog was shown, force the map to redraw.
	//------------------------------------------------------------------------
	HWND dialog = WS_Find_Dialog(IDD_MPLAYER_DISCONNECT);
	if (dialog) {
		WS_Destroy_Dialog(dialog, false);
		TacticalActive = true;
		Map.Flag_To_Redraw(GS_REDRAW_ALL);
		Map.Render();
	}
}


/// <summary>
/// Removes a player from the running multiplayer game.
/// This routine is called once the other players have voted someone out, or a player has
/// stopped answering altogether. The offending house is flagged as having lost its
/// connection, the game statistics are shipped off if the game is effectively decided, and
/// the connection itself is destroyed.
/// </summary>
/// <param name="kickee">Connection index of the player to be removed.</param>
/// <param name="their">The frame sync records, compacted as the connection goes away.</param>
/// <param name="error">Should the disconnection be reported as an error?</param>
void Kick_Player_Now(ConnManClass *net, int kickee, FrameSyncStruct * their, bool error)
{
	/*
	 * Kicking several players inside a minute is treated as spam rather than as a series of
	 * resignations, so those kicks leave the victims' resigner flags alone.
	 */
	static int last_kick_time;
	bool spamkick = false;
	if (time(NULL) - last_kick_time >= 60) {
		last_kick_time = time(NULL);
	} else {
		spamkick = true;
		last_kick_time = time(NULL);
	}

	int id = net->Connection_ID(kickee);
	DebugString("Kicking player %d from the game - Frame is %d\n", id, Frame);

	if (id == -1) {
		DebugString("Kick_Player_Now bailing - bad house ID\n");
		return;
	}

	Houses[id]->LostConnection = true;

	if (CountAliveTeams(Houses[id]) == 1 && Session.Type == GAME_INTERNET && !GameStatisticsPacketSent) {
		Register_Game_End_Time();
		ConnectionLost = true;
		if (!spamkick) {
			Session.SawGameCompletion = true;
		}
		for (int i = 0; i < Houses.Count(); i++) {
			HouseClass * house = Houses[i];
			if (house != NULL && !house->IsDefeated && house->IsHuman) {
				house->LostConnection = true;
			}
		}
		Send_Statistics_Packet();
	}

	if (CountAliveTeams(Houses[id]) > 1 && !spamkick) {
		Houses[id]->IsResigner = true;
	}

	for (int i = kickee; i < net->Num_Connections() - 1; i++) {
		their[i] = their[i+1];
	}

	Forget_Kick_Player(id);
	Destroy_Connection(id, error);
}


/***************************************************************************
 * Handle_Timeout -- handles a timeout in the wait-for-players loop        *
 *                                                                         *
 * This routine "gracefully" handles a timeout in the frame-sync loop.     *
 *                                                                         *
 * The network game must find the connection that's causing the timeout,   *
 * and destroy it.  The game continues, even if there are no more human    *
 * players left.                                                           *
 *                                                                         *
 * INPUT:                                                                  *
 *      net               ptr to connection manager                        *
 *      their_frame         array containing frame #'s of other players    *
 *      their_sent         array containing command count of other players *
 *      their_recv         array containing # recv'd cmds from other players*
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = it's OK; reset timeout timers & keep processing                *
 *      0 = game over, man                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Handle_Timeout(ConnManClass *net, FrameSyncStruct *their)
{
	int oldest_index;						// index of person requiring a reconnect
	int i,j;
	//int id;

	//------------------------------------------------------------------------
	// For network, destroy the oldest connection
	//------------------------------------------------------------------------
	if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
		j = 0x7fffffff;
		oldest_index = 0;
		for (i = 0; i < net->Num_Connections(); i++) {
			if (their[i].frame < j) {
				j = their[i].frame;
				oldest_index = i;
			}
		}
		Kick_Player_Now(net, oldest_index, their, true);

#if 0
		id = net->Connection_ID(oldest_index);
		/*
		**	Send the game statistics packet now if the game is effectivly over
		*/
		if (Session.Players.Count() == 2 &&
				Session.Type == GAME_INTERNET &&
				!GameStatisticsPacketSent) {
			Register_Game_End_Time();
			ConnectionLost = true;
			Send_Statistics_Packet();		//	Disconnect, and I'll be the only one left.
		}

		if (id != ConnManClass::CONNECTION_NONE) {
			for (i = oldest_index; i < net->Num_Connections() - 1; i++) {
				their[i].frame = their[i+1].frame;
				their[i].sent = their[i+1].sent;
				their[i].recv = their[i+1].recv;
			}
			if (Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) {
				Destroy_Connection(id,1);
			}
		}
#endif
	}

	return(1);

}	// end of Handle_Timeout


/***************************************************************************
 * Stop_Game -- stops the game                                             *
 *                                                                         *
 * This routine clears any global flags that need it, in preparation for   *
 * halting the game prematurely.                                           *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/22/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Stop_Game(bool is_cancelled)
{
	Session.LoadGame = false;
	Session.EmergencySave = false;
	GameActive = 0;
	if (IsMono) {
		MonoClass::Disable();
	}

	if (Session.Type == GAME_INTERNET){
		ConnectionLost = true;
		if (!is_cancelled) Session.SawGameCompletion = true;
		Session.OutOfSync = true;
		Register_Game_End_Time();
		Send_Statistics_Packet();
	}
}	// end of Stop_Game


/***************************************************************************
 * Build_Send_Packet -- Builds a big packet from a bunch of little ones.   *
 *                                                                         *
 * This routine takes events from the OutList, and puts them into a        *
 * "meta-packet", which is transmitted to all systems we're connected to.  *
 * Also, these events are added to our own DoList.                         *
 *                                                                         *
 * Every Meta-Packet we send uses a FRAMEINFO packet as a header; this     *
 * tells the other systems what frame we're on, as well as serving as a    *
 * standard packet header.                                                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf            buffer to store packet in                           *
 *      bufsize         max size of buffer                                 *
 *      frame_delay      desired frame delay to attach to all outgoing packets*
 *      num_cmds         value to use for the CommandCount field           *
 *      cap            max # events to send                                *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      new size of packet                                                 *
 *                                                                         *
 * WARNINGS:                                                               *
 * 'num_cmds' should be the total of of commands, including all those sent *
 * this frame!                                                             *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Build_Send_Packet(void *buf, int bufsize, int frame_delay,
	int num_cmds, int cap, int & processed)
{
	int size = 0;
	EventClass *finfo;

	//------------------------------------------------------------------------
	// All events start with a FRAMEINFO event; fill this part in.
	//------------------------------------------------------------------------
	//........................................................................
	// Set the event type
	//........................................................................
	finfo = (EventClass *)buf;
	finfo->Type = EventClass::FRAMEINFO;
	processed = 0;
	//........................................................................
	// Set the frame to execute this event on; this is protocol-specific
	//........................................................................
	if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
		finfo->Frame = ((Frame + frame_delay + (Session.FrameSendRate - 1)) /
			 Session.FrameSendRate) * Session.FrameSendRate;
	}
	else {
		finfo->Frame = Frame + frame_delay;
	}
	//........................................................................
	// Fill in the rest of the event
	//........................................................................
	finfo->ID = PlayerPtr->HeapID;
	finfo->Data.FrameInfo.CRC = GameCRC;
	finfo->Data.FrameInfo.CommandCount = num_cmds;
	finfo->Data.FrameInfo.Delay = frame_delay;

	//------------------------------------------------------------------------
	// Initialize the # of bytes processed; this is protocol-specific
	//------------------------------------------------------------------------
	if (Session.CommProtocol==COMM_PROTOCOL_SINGLE_NO_COMP) {
		size += sizeof(EventClass);
	}
	else {
		size += (offsetof(EventClass, Data) +
			size_of(EventClass, Data.FrameInfo));
	}

	//------------------------------------------------------------------------
	// Transfer all events from the OutList into the DoList, building our
	// packet while we go.
	//------------------------------------------------------------------------
	switch (Session.CommProtocol) {
		//.....................................................................
		// COMM_PROTOCOL_SINGLE_NO_COMP:
		// We'll send at least a FRAMEINFO every single frame, no compression
		//.....................................................................
		case (COMM_PROTOCOL_SINGLE_NO_COMP):
			size = Add_Uncompressed_Events(buf, bufsize, frame_delay, size, cap);
			break;

		//.....................................................................
		// COMM_PROTOCOL_SINGLE_E_COMP:
		//   Compress a group of packets into our send buffer; send out
		//   compressed packets every frame.
		// COMM_PROTOCOL_MULTI_E_COMP:
		//   Compress a group of packets into our send buffer; send out
		//   compressed packets every 'n' frames.
		//.....................................................................
		case (COMM_PROTOCOL_SINGLE_E_COMP):
		case (COMM_PROTOCOL_MULTI_E_COMP):
			size = Add_Compressed_Events(buf, bufsize, frame_delay, size, cap, processed);
			break;

		//.....................................................................
		// Default: We have no idea what to do, so do nothing.
		//.....................................................................
		default:
			size = 0;
			break;
	}

	return( size );

}	/* end of Build_Send_Packet */


/***************************************************************************
 * Add_Uncompressed_Events -- adds uncompressed events to a packet         *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf            buffer to store packet in                           *
 *      bufsize         max size of buffer                                 *
 *      frame_delay      desired frame delay to attach to all outgoing packets*
 *      size            current packet size                                *
 *      cap            max # events to process                             *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      new size value                                                     *
 *                                                                         *
 * WARNINGS:                                                               *
 *      This routine MUST check to be sure it doesn't overflow the buffer. *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 DRD : Created.                                             *
 *=========================================================================*/
static int Add_Uncompressed_Events(void *buf, int bufsize, int frame_delay,
	int size, int cap)
{
	int num = 0;			// # of events processed
	int ev_size;			// size of event we're adding

	//------------------------------------------------------------------------
	// Loop until there are no more events, or we've processed our max # of
	// events, or the buffer is full.
	//------------------------------------------------------------------------
	while (!OutList.empty() && (num < cap)) {

		//Keyboard->Check();

		if (OutList.front().Type==EventClass::ADDPLAYER) {
			ev_size = sizeof(EventClass) + OutList.front().Data.Variable.Size;
		}
		else {
			ev_size = sizeof(EventClass);
		}
		//.....................................................................
		// Will the next event exceed the size of the buffer?  If so, break.
		//.....................................................................
		if ( (size + ev_size) > bufsize ) {
			return(size);
		}

		//.....................................................................
		// Set the event's frame delay
		//.....................................................................
		OutList.front().Frame = Frame + frame_delay;

		//.....................................................................
		// Set the event's ID
		//.....................................................................
		OutList.front().ID = PlayerPtr->HeapID;

		//.....................................................................
		// Transfer the event in OutList to DoList, un-queue the OutList event.
		//.....................................................................
		OutList.front().IsExecuted = 0;
		DoList.push_back(OutList.front());

		//.....................................................................
		// Add event to the send packet
		//.....................................................................
		if (OutList.front().Type==EventClass::ADDPLAYER) {
			memcpy ( ((char *)buf) + size, &OutList.front(), sizeof(EventClass) );
			size += sizeof(EventClass);
			memcpy ( ((char *)buf) + size,
				OutList.front().Data.Variable.Pointer,
				OutList.front().Data.Variable.Size);
			size += OutList.front().Data.Variable.Size;
		}
		else {
			memcpy ( ((char *)buf) + size, &OutList.front(), sizeof(EventClass) );
			size += sizeof(EventClass);
		}

		//.....................................................................
		// Increment our event counter; delete the last event from the queue
		//.....................................................................
		num++;
		OutList.pop_front();
	}

	return(size);

}	// end of Add_Uncompressed_Events


/***************************************************************************
 * Add_Compressed_Events -- adds an compressed events to a packet          *
 *                                                                         *
 * INPUT:                                                                  *
 *      buf            buffer to store packet in                           *
 *      bufsize         max size of buffer                                 *
 *      frame_delay      desired frame delay to attach to all outgoing packets*
 *      size            reference to current packet size                   *
 *      cap            max # events to process                             *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      new size value                                                     *
 *                                                                         *
 * WARNINGS:                                                               *
 *      This routine MUST check to be sure it doesn't overflow the buffer. *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 DRD : Created.                                             *
 *=========================================================================*/
static int Add_Compressed_Events(void *buf, int bufsize, int frame_delay,
	int size, int cap, int & processed)
{
	int num = 0;                        // # of events processed
	unsigned char eventtype;            // type of event being compressed
	EventClass prevevent;               // last event processed
	int datasize;                       // size of element plucked from event union
	int storedsize;                     // actual # bytes stored from event
	unsigned char *unitsptr = NULL;     // ptr to buffer pos to store mega. rep count
	unsigned char numunits = 0;         // megamission rep count value
	bool missiondup = false;            // flag: is this event a megamission repeat?

	//------------------------------------------------------------------------
	// clear previous event
	//------------------------------------------------------------------------
	memset (&prevevent, 0, sizeof(EventClass));

	if (Debug_Print_Events) {
		DebugString("\nFrame %d: Building Send Packet\n", Frame);
	}

	//------------------------------------------------------------------------
	// Loop until there are no more events, we've processed our max # of
	// events, or the buffer is full.
	//------------------------------------------------------------------------
	while (!OutList.empty() && (num < cap)) {

		//Keyboard->Check();

		eventtype = OutList.front().Type;
		datasize = EventClass::EventLength[ eventtype ];
		//.....................................................................
		// For a variable-sized event, pull the size from the event; otherwise,
		// the size will be the data element size plus the event type value.
		// (The other data elements in the event, Frame, ID, etc, are stored
		// in the packet header.)
		//.....................................................................
		if (eventtype==EventClass::ADDPLAYER) {
			storedsize = datasize + size_of(EventClass, Type) +
				OutList.front().Data.Variable.Size;
		}
		else {
			storedsize = datasize + size_of(EventClass, Type);
		}

		//.....................................................................
		// MegaMission compression:  MegaMissions are stored as:
		//   EventType
		//   Rep Count
		//   MegaMission structure (event # 1 only)
		//   Whom #2
		//   Whom #3
		//   Whom #4
		//   ...
		//   Whom #n
		//.....................................................................
		if (prevevent.Type == EventClass::MEGAMISSION) {
			//..................................................................
			// If previous & current events are both MegaMissions:
			//..................................................................
			if (eventtype == EventClass::MEGAMISSION) {
				//...............................................................
				// If the Mission, Target, & Destination are the same, compress
				// the events into one:
				// - Change datasize to the size of the 'Whom' field only
				// - set total # bytes to store to the size of the 'Whom' only
				// - increment the MegaMission rep count
				// - set the MegaMission rep flag
				//...............................................................
				if (OutList.front().Data.MegaMission.Mission ==
					prevevent.Data.MegaMission.Mission &&
					OutList.front().Data.MegaMission.Target ==
						prevevent.Data.MegaMission.Target &&
					OutList.front().Data.MegaMission.Destination ==
						prevevent.Data.MegaMission.Destination) {

					if (Debug_Print_Events) {
						DebugString("      adding Whom:%x Mission:%s Target:%x Dest:%x\n",
						OutList.front().Data.MegaMission.Whom.Encode(),
						MissionClass::Mission_Name(OutList.front().Data.MegaMission.Mission),
						OutList.front().Data.MegaMission.Target.Encode(),
						OutList.front().Data.MegaMission.Destination.Encode());
					}

					datasize = sizeof(prevevent.Data.MegaMission.Whom);
					storedsize = datasize;
					numunits++;
					missiondup = true;
				}
				//...............................................................
				// Data doesn't match; start a new run of MegaMissions:
				// - Store previous MegaMission rep count
				// - Init 'unitsptr' to buffer pos after next EventType
				// - set total # bytes to store to 'datasize' + sizeof(EventType) +
				//   sizeof (numunits)
				// - init the MegaMission rep count to 1
				// - clear the MegaMission rep flag
				//...............................................................
				else {

					if (Debug_Print_Events) {
						DebugString("  New MEGAMISSION run:\n");
					}

					*unitsptr = numunits;
					unitsptr = ((unsigned char *)buf) + size +
						size_of(EventClass, Type);
					storedsize += sizeof(numunits);
					numunits = 1;
					missiondup = false;
				}
			}
			//..................................................................
			// Previous event was a MegaMission, but this one isn't: end the
			// run of MegaMissions:
			// - Store previous MegaMission rep count
			// - Clear variables
			//..................................................................
			else {
				*unitsptr = numunits;   // save # events in our run
				unitsptr = NULL;        // init other values
				numunits = 0;
				missiondup = false;
			}
		}

		//.....................................................................
		// The previous event is not a MEGAMISSION but the current event is:
		// Set up a new run of MegaMissions:
		// - Init 'unitsptr' to buffer pos after next EventType
		// - set total # bytes to store to 'datasize' + sizeof(EventType) +
		//   sizeof (numunits)
		// - init the MegaMission rep count to 1
		// - clear the MegaMission rep flag
		//.....................................................................
		else if (eventtype == EventClass::MEGAMISSION) {

			if (Debug_Print_Events) {
				DebugString("  New MEGAMISSION run:\n");
			}

			unitsptr = ((unsigned char *)buf) + size +
				size_of(EventClass, Type);
			storedsize += sizeof(numunits);
			numunits = 1;
			missiondup = false;
		}

		//.....................................................................
		// Will the next event exceed the size of the buffer?  If so,
		// stop compressing.
		//.....................................................................
		if ( (size + storedsize) > bufsize )
			break;

		//.....................................................................
		// Set the event's frame delay (this is protocol-dependent)
		//.....................................................................
		if (Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
			OutList.front().Frame = ((Frame + frame_delay +
				(Session.FrameSendRate - 1)) / Session.FrameSendRate) *
				Session.FrameSendRate;
		}
		else {
			OutList.front().Frame = Frame + frame_delay;
		}

		//.....................................................................
		// Set the event's ID
		//.....................................................................
		OutList.front().ID = PlayerPtr->HeapID;

		//.....................................................................
		// Transfer the event in OutList to DoList, un-queue the OutList event.
		//.....................................................................
		OutList.front().IsExecuted = 0;
		DoList.push_back( OutList.front() );

		//---------------------------------------------------------------------
		// Compress the event into the send packet buffer
		//---------------------------------------------------------------------
		switch ( eventtype ) {
			//..................................................................
			// RESPONSE_TIME: just use the Delay field of the FrameInfo union
			//..................................................................
			case (EventClass::RESPONSE_TIME):

				*(unsigned char *)( ((char *)buf) + size) = eventtype;

				memcpy ( ((char *)buf) + size + size_of(EventClass, Type),
					&OutList.front().Data.FrameInfo.Delay, datasize );

				size += (datasize + size_of(EventClass, Type));
				break;

			//..................................................................
			// MEGAMISSION:
			//..................................................................
			case (EventClass::MEGAMISSION):
				//...............................................................
				// Repeated mission in a run:
				//   - Update the rep count (in case we break out)
				//   - Copy the Whom field only
				//...............................................................
				if (missiondup) {
					*unitsptr = numunits;

					memcpy ( ((char *)buf) + size,
						&OutList.front().Data.MegaMission.Whom, datasize );

					size += datasize;
				}
				//...............................................................
				// 1st mission in a run:
				//   - Init the rep count (in case we break out)
				//   - Set the EventType
				//   - Copy the MegaMission structure, leaving room for 'numunits'
				//...............................................................
				else {
					*unitsptr = numunits;

					*(unsigned char *)( ((char *)buf) + size) = eventtype;

					memcpy ( ((char *)buf) + size +
						size_of(EventClass, Type) + sizeof(numunits),
						&OutList.front().Data.MegaMission, datasize );

					size += (datasize + size_of(EventClass, Type) + sizeof(numunits));
				}
				break;

			//..................................................................
			// Variable-sized packets: Copy the packet Size & the buffer
			//..................................................................
			case (EventClass::ADDPLAYER):
				*(unsigned char *)( ((char *)buf) + size) = eventtype;

				memcpy ( ((char *)buf) + size + size_of(EventClass, Type),
					&OutList.front().Data.Variable.Size, datasize );
				size += (datasize + size_of(EventClass, Type));

				memcpy ( ((char *)buf) + size,
					OutList.front().Data.Variable.Pointer,
					OutList.front().Data.Variable.Size);
				size += OutList.front().Data.Variable.Size;

				break;

			//..................................................................
			// Default case: Just copy over the data field from the union
			//..................................................................
			default:
				*(unsigned char *)( ((char *)buf) + size) = eventtype;

				memcpy ( ((char *)buf) + size + size_of(EventClass, Type),
					&OutList.front().Data, datasize );

				size += (datasize + size_of(EventClass, Type));

				break;
		}

		//---------------------------------------------------------------------
		// update # events processed
		//---------------------------------------------------------------------
		num++;

		//---------------------------------------------------------------------
		// Update 'prevevent'
		//---------------------------------------------------------------------
		memcpy ( &prevevent, &OutList.front(), sizeof(EventClass) );

		//---------------------------------------------------------------------
		// Go to the next event to process
		//---------------------------------------------------------------------
		OutList.pop_front();
	}

//	if (Debug_Print_Events) {
//		DebugString("\n");
//	}

	processed = num;
	return(size);

}	// end of Add_Compressed_Events


/***************************************************************************
 * Execute_DoList -- Executes commands from the DoList                     *
 *                                                                         *
 * This routine executes any events in the DoList that need to be executed *
 * on the current game frame.  The events must be executed in a special    *
 * order, so that all systems execute all events in exactly the same       *
 * order.                                                                  *
 *                                                                         *
 * This routine also handles checking the Game CRC sent by other systems   *
 * against my own, to be sure we're still in sync.                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      max_houses   # houses to execute commands for                      *
 *      base_house   HousesType to start with                              *
 *      net         ptr to connection manager; NULL if none                *
 *      skip_crc      a frame-based countdown timer; if it's non-zero, the *
 *                  CRC check will be skipped.  Ignored if NULL.           *
 *      their_frame   array of their frame #'s                             *
 *      their_sent   array of # commands they've sent                      *
 *      their_recv   array of # commands I've received from them           *
 *                                                                         *
 * (their_xxx are ignored if 'net' is NULL.)                               *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      1 = OK, 0 = some error occurred (CRC error, packet rcv'd too late.)*
 *                                                                         *
 * WARNINGS:                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static int Execute_DoList(int max_houses, HousesType base_house,
	ConnManClass *net, CDTimerClass<FrameTimerClass> *skip_crc,
	FrameSyncStruct *their)
{
	HousesType house;
	HouseClass *hptr;
	int i,j,k;
	int index;
	int check_crc;

#if (TIMING_FIX)
	//
	// If MPlayerMaxAhead is recomputed such that it increases, the systems
	// may try to free-run to the new MaxAhead value.  If so, they may miss
	// an event that was generated after the TIMING event was created, but
	// before it executed; this event will be scheduled with the older,
	// shorter MaxAhead value.  If a system doesn't receive this event, it
	// may execute past the frame it's scheduled to execute on, creating
	// a Packet-Recieved-Too-Late error.  To prevent this, find any events
	// that are scheduled to execute during this "period of vulnerability",
	// and re-schedule for the end of that period.
	//
	for (j = 0; j < (int)DoList.size(); j++) {
		if (DoList[j].Type != EventClass::FRAMEINFO &&
			DoList[j].Frame > NewMaxAheadFrame1 &&
			DoList[j].Frame < NewMaxAheadFrame2) {
			DebugString("DoList: Moving event from frame %d to frame %d\n", DoList[j].Frame, NewMaxAheadFrame2);
			DoList[j].Frame = NewMaxAheadFrame2;
		}
	}
#endif

	//------------------------------------------------------------------------
	// Execute the DoList.  Events must be executed in the same order on all
	//	systems; so, execute them in the order of the HouseClass array.  This
	// array is stored in the same order on all systems.
	//------------------------------------------------------------------------
	for (i = 0; i < Houses.Count(); i++) {
		//.....................................................................
		// Convert our index into a HousesType value
		//.....................................................................
		house = (HousesType)(i);
		hptr = Houses[house];

		//.....................................................................
		// If for some reason this house doesn't exist, skip it.
		// Also, if this house has exited the game, skip it.  (The user can
		// generate events after he exits, because the exit event is scheduled
		// at least FrameSendRate*3 frames ahead.  If one system gets these
		// packets & another system doesn't, they'll go out of sync because
		// they aren't checking the CommandCount for that house, since that
		// house isn't connected any more.)
		//.....................................................................
		if (!hptr) {
			continue;
		}
		if (!hptr->IsHuman && !hptr->IsPlayerControl) {
			continue;
		}

		//.....................................................................
		// Loop through all events
		//.....................................................................
		for (j = 0; j < (int)DoList.size(); j++) {

			if (net)
				Update_Queue_Mono (net, 6);

			//..................................................................
			// If this event was from the currently-executing player ID, and it's
			// time to execute it, execute it.
			//..................................................................
			if (DoList[j].ID == hptr->HeapID && Frame >= DoList[j].Frame &&
				!DoList[j].IsExecuted) {

				//...............................................................
				// Error if it's too late to execute this packet!
				// (Hack: disable this check for solo or skirmish mode.)
				//...............................................................
				if (Frame > DoList[j].Frame && DoList[j].Type !=
					EventClass::FRAMEINFO && Session.Type != GAME_NORMAL &&
					Session.Type != GAME_SKIRMISH) {
					Dump_Packet_Too_Late_Stuff(&DoList[j]);
					Session.Suspended++;
					WWMessageBox().Process (TXT_PACKET_TOO_LATE, TXT_OK);
					Session.Suspended--;
					return(0);
				}

				//...............................................................
				// Only execute EXIT & OPTIONS commands if they're from myself.
				//...............................................................
				if (DoList[j].Type==EventClass::EXIT ||
						DoList[j].Type==EventClass::OPTIONS ||
						DoList[j].Type==EventClass::PAGEUSER) {

					if (DoList[j].Type==EventClass::EXIT) {
						int house_count = Houses.Count();
						/*
						**	Flag that this house lost because it quit.
						*/
						HousesType quithouse = HOUSE_NONE;
						HouseClass *quithptr = NULL;

						for (int player = 0; player < house_count ; player++) {
							quithouse = (HousesType)(player);
							quithptr = Houses[quithouse];
							if (!quithptr) {
								continue;
							}
							if (quithptr->HeapID == DoList[j].ID) {
								quithptr->IsGiverUpper = true;
								break;
							}
						}

						/*
						**	Send the game statistics packet now since the game is effectivly over
						*/
						if (CountAliveTeams(quithptr) == 1 &&
								Session.Type == GAME_INTERNET &&
								!GameStatisticsPacketSent) {
							Session.SawGameCompletion = true;
							Register_Game_End_Time();
							Send_Statistics_Packet();		// Event - player aborted, and there were only 2 left.
						}
						if (Session.Type == GAME_INTERNET && !GameStatisticsPacketSent && PlayerPtr != NULL && PlayerPtr == quithptr) {
							DebugString("Sending game results because I quit, but didn't see completion");
							Send_Statistics_Packet();
						}
					}

					if (Debug_Print_Events) {
						if (DoList[j].Type==EventClass::EXIT) {
							DebugString("Exit Event: ID:%d (%s),  Event Frame:%d,  My Frame:%d\n",
								DoList[j].ID,
								Houses[(HousesType)(DoList[j].ID)]->IniName.c_str(),
								DoList[j].Frame,
								Frame);
						}
					}

					if (DoList[j].ID == PlayerPtr->HeapID) {
						DoList[j].Execute();
					} else if (DoList[j].Type==EventClass::EXIT) {
					//............................................................
					// If this EXIT event isn't from myself, destroy the connection
					//	for that player.  The HousesType for this event is the
					// connection ID.
					//............................................................
						if ((Session.Type == GAME_IPX ||
							Session.Type == GAME_INTERNET) && net) {
							index = net->Connection_Index (house);
							if (index != -1) {
								for (k = index; k < net->Num_Connections() - 1; k++) {
									their[k] = their[k+1];
								}
								Destroy_Connection(house,0);
							}
						}
						//
						// Special case for recording playback: turn the house over
						// to the computer.
						//
						if (Session.Play && DoList[j].Type==EventClass::EXIT) {
							hptr->IsHuman = false;
							hptr->IQ = Rule->MaxIQ;
							hptr->Computer_Paranoid();
							hptr->IniName = Fetch_String(TXT_COMPUTER);
							DebugString("Removing a player %s:%d\n", __FILE__, __LINE__);
							Session.NumPlayers--;
						}
					}
				}

				//...............................................................
				// For a FRAMEINFO event, check the CRC value.
				//...............................................................
				else if (DoList[j].Type == EventClass::FRAMEINFO) {
					//............................................................
					// Skip the CRC check if we're less than 32 frames into the game;
					// this will prevent a newly-loaded game from instantly
					// going out of sync, if the games were saved at different
					// frame numbers.
					//............................................................
					if (!skip_crc || *skip_crc == 0) {
						check_crc = 1;
					}
					else {
						check_crc = 0;
					}
					if (check_crc
						&& DoList[j].Frame == Frame
						&& DoList[j].Data.FrameInfo.Delay < ARRAY_SIZE(CRC)) {
						index = ((DoList[j].Frame - DoList[j].Data.FrameInfo.Delay) &
							ARRAY_SIZE(CRC)-1);
						if (CRC[index] != DoList[j].Data.FrameInfo.CRC) {
							Print_CRCs(&DoList[j]);
							Session.Suspended++;
							if (WWMessageBox().Process (TXT_OUT_OF_SYNC,
								TXT_CONTINUE, TXT_STOP) == 0) {
								Session.Suspended--;
								if ((Session.Type == GAME_IPX ||
									Session.Type == GAME_INTERNET) && net) {
									while (net->Num_Connections()) {
										Destroy_Connection (net->Connection_ID(0), -1);
									}
								}
								Map.Flag_To_Redraw(GS_REDRAW_ALL);
							}
							else {
								Session.Suspended--;
								return(0);
							}
							return(1);
						}
					}
				}
				//...............................................................
				// Execute other commands
				//...............................................................
				else {
					DoList[j].Execute();
				}

				//...............................................................
				// Mark this event as executed.
				//...............................................................
				DoList[j].IsExecuted = 1;
			}
		}
	}

	return(1);

}	// end of Execute_DoList


/***************************************************************************
 * Clean_DoList -- Cleans out old events from the DoList                   *
 *                                                                         *
 * Currently, an event can only be removed from the DoList if it's at the  *
 * head of the list; and event can't be removed from the middle.  So,      *
 * this routine loops as long as the next event in the DoList has been     *
 * executed, it's removed.                                                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      net      ptr to connection manager; ignored if NULL                *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Clean_DoList(ConnManClass *net)
{
	while (!DoList.empty()) {
		if (net)
			Update_Queue_Mono (net, 7);

		//.....................................................................
		// Discard events that have been executed, OR it's too late to execute.
		//	(This happens if another player exits the game; he'll leave FRAMEINFO
		// events lying around in my queue.  They won't have been "executed",
		//	because his IPX connection was destroyed.)
		//.....................................................................
		if ( (DoList.front().IsExecuted) || (Frame > DoList.front().Frame) ) {
			DoList.pop_front();
		}
		else {
			break;
		}
	}

}	// end of Clean_DoList


/***************************************************************************
 * Queue_Record -- Records the DoList to disk                              *
 *                                                                         *
 * This routine just saves any events in the DoList to disk; we can later  *
 * "play back" the recording just be pulling events from disk rather than  *
 * from the network!                                                       *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   08/14/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Queue_Record(void)
{
	if (Frame == 0 && Session.Type != GAME_NORMAL) return;

	int i,j;

	//------------------------------------------------------------------------
	//	Compute # of events to save this frame
	//------------------------------------------------------------------------
	j = 0;
	for (i = 0; i < (int)DoList.size(); i++) {
		if (Frame == DoList[i].Frame && !DoList[i].IsExecuted) {
			j++;
		}
	}

	//------------------------------------------------------------------------
	//	Save the # of events, then all events.
	//------------------------------------------------------------------------
	Session.RecordFile.Write (&j,sizeof(j));
	for (i = 0; i < (int)DoList.size(); i++) {
		if (Frame == DoList[i].Frame && !DoList[i].IsExecuted) {
			Session.RecordFile.Write (&DoList[i],sizeof (EventClass));
			j--;
		}
	}

}	/* end of Queue_Record */


/***************************************************************************
 * Queue_Playback -- plays back queue entries from a record file           *
 *                                                                         *
 * This routine reads events from disk, putting them into the DoList;      *
 * it then executes the DoList just like the network version does.  The    *
 * result is that the game "plays back" like a recording.                  *
 *                                                                         *
 * This routine detects mouse motion and stops playback, so it can work    *
 * like an "attract" mode, showing a demo of the game itself.              *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/15/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Queue_Playback(void)
{
	int numevents;
	EventClass event;
	int i;
	int ok;
	static int mx,my;
	int max_houses;
	HousesType base_house;
	int key;
	int testframe;

	//------------------------------------------------------------------------
	// If the user hits ESC, stop the playback
	//------------------------------------------------------------------------
	if (Keyboard->Check()) {
		key = Keyboard->Get();
		if (key == KA_ESC || Session.Attract) {
			GameActive = 0;
			return;
		}
	}

	//------------------------------------------------------------------------
	// If we're in "Attract" mode, and the user moves the mouse, stop the
	// playback.
	//------------------------------------------------------------------------
	if (Session.Attract && Frame > 0 &&
		(mx != Get_Mouse_X() || my != Get_Mouse_Y())) {
		GameActive = 0;
		return;
	}
	mx = Get_Mouse_X();
	my = Get_Mouse_Y();

	//------------------------------------------------------------------------
	// Compute the Game's CRC
	//------------------------------------------------------------------------
	Compute_Game_CRC();
	CRC[Frame & (ARRAY_SIZE(CRC) - 1)] = GameCRC;

	//------------------------------------------------------------------------
	// If we've reached the CRC print frame, do so & exit
	//------------------------------------------------------------------------
	if (Frame >= Session.TrapPrintCRC) {
		Print_CRCs(NULL);
		Emergency_Exit();
		exit(0);
	}

	//------------------------------------------------------------------------
	// Don't read anything the first time through (since the Queue_AI_Network
	//	routine didn't write anything the first time through); do this after the
	// CRC is computed, since we'll still need a CRC for Frame 0.
	//------------------------------------------------------------------------
	if (Frame==0 && Session.Type!=GAME_NORMAL) {
		return;
	}

	//------------------------------------------------------------------------
	// Only process every 'FrameSendRate' frames
	//------------------------------------------------------------------------
	testframe = ((Frame + (Session.FrameSendRate - 1)) /
		Session.FrameSendRate) * Session.FrameSendRate;
	if ( (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) &&
		Session.CommProtocol == COMM_PROTOCOL_MULTI_E_COMP) {
		if (Frame != testframe) {
			return;
		}
	}

	//------------------------------------------------------------------------
	// Read the DoList from disk
	//------------------------------------------------------------------------
	ok = 1;
	if (Session.RecordFile.Read (&numevents, sizeof(numevents)) ==
		sizeof(numevents)) {
		for (i = 0; i < numevents; i++) {
			if (Session.RecordFile.Read (&event, sizeof(EventClass)) ==
				sizeof(EventClass)) {
				event.IsExecuted = 0;
				DoList.push_back(event);
			}
			else {
				ok = 0;
				break;
			}
		}
	}
	else {
		ok = 0;
	}

	if (!ok) {
		GameActive = 0;
		return;
	}


	//------------------------------------------------------------------------
	// Execute the DoList; if an error occurs, bail out.
	//------------------------------------------------------------------------
	if (Session.Type == GAME_NORMAL) {
		max_houses = 1;
		base_house = PlayerPtr->Class->House;
	}
	else {
		max_houses = Session.MaxPlayers;
		base_house = HOUSE_FIRST;
	}
	if (!Execute_DoList(max_houses, base_house, NULL, NULL, NULL)) {
		GameActive = 0;
		return;
	}

	//------------------------------------------------------------------------
	// Clean out the DoList
	//------------------------------------------------------------------------
	Clean_DoList(NULL);

}	/* end of Queue_Playback */


/***************************************************************************
 * Compute_Game_CRC -- Computes a CRC value of the entire game.            *
 *                                                                         *
 * INPUT:                                                                  *
 *      none.                                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Compute_Game_CRC(void)
{
	int i,j;
	InfantryClass *infp;
	UnitClass *unitp;
	BuildingClass *bldgp;
	ObjectClass *objp;

	GameCRC = 0;
	//------------------------------------------------------------------------
	// Infantry
	//------------------------------------------------------------------------
	for (i = 0; i < Infantry.Count(); i++) {
		infp = (InfantryClass *)Infantry[i];
		Add_CRC (&GameCRC, (int)infp->PositionCoord.As_Int() + (int)infp->PrimaryFacing.Current().As_Dir256());
	}

	//------------------------------------------------------------------------
	// Units
	//------------------------------------------------------------------------
	for (i = 0; i < Units.Count(); i++) {
		unitp = (UnitClass *)Units[i];
		Add_CRC (&GameCRC, (int)unitp->PositionCoord.As_Int() + (int)unitp->PrimaryFacing.Current().As_Dir256() +
			(int)unitp->SecondaryFacing.Current().As_Dir256());
	}

	//------------------------------------------------------------------------
	// Buildings
	//------------------------------------------------------------------------
	for (i = 0; i < Buildings.Count(); i++) {
		bldgp = (BuildingClass *)Buildings[i];
		Add_CRC (&GameCRC, (int)bldgp->PositionCoord.As_Int() + (int)bldgp->PrimaryFacing.Current().As_Dir256());
	}

	//------------------------------------------------------------------------
	// Map Layers
	//------------------------------------------------------------------------
	for (i = 0; i < LAYER_COUNT; i++) {
		for (j = 0; j < Map.Layer[i].Count(); j++) {
			objp = Map.Layer[i][j];
			if (objp->RTTI == RTTI_ANIM && objp->Fetch_ID() == -2) {
				continue;
			}
			Add_CRC (&GameCRC, (int)objp->PositionCoord.As_Int() + (int)objp->RTTI);
		}
	}

	//------------------------------------------------------------------------
	// Logic Layers
	//------------------------------------------------------------------------
	for (i = 0; i < Logic.Count(); i++) {
		objp = Logic[i];
		if (objp->RTTI == RTTI_ANIM && objp->Fetch_ID() == -2) {
			continue;
		}
		Add_CRC (&GameCRC, (int)objp->PositionCoord.As_Int() + (int)objp->RTTI);
	}

	//------------------------------------------------------------------------
	//	A random #
	//------------------------------------------------------------------------
	Add_CRC(&GameCRC, Scen->RandomNumber);

}	/* end of Compute_Game_CRC */


/***************************************************************************
 * Add_CRC -- Adds a value to a CRC                                        *
 *                                                                         *
 * INPUT:                                                                  *
 *      crc      ptr to crc                                                *
 *      val      value to add                                              *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none                                                               *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1995 BRR : Created.                                             *
 *=========================================================================*/
void Add_CRC(unsigned int *crc, unsigned int val)
{
	int hibit;

	if ( (*crc) & 0x80000000) {
		hibit = 1;
	}
	else {
		hibit = 0;
	}

	(*crc) <<= 1;
	(*crc) += val;
	(*crc) += hibit;

}	/* end of Add_CRC */

/***************************************************************************
 * Print_CRCs -- Prints a data file for finding Sync Bugs                  *
 *                                                                         *
 * INPUT:                                                                  *
 *      ev -- event to display                                             *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none                                                               *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none                                                               *
 *                                                                         *
 * HISTORY:                                                                *
 *   05/09/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Print_CRCs(EventClass *ev)
{
	static char _NO_NAME [] = "None";

	int i,j;
	InfantryClass *infp;
	UnitClass *unitp;
	BuildingClass *bldgp;
	AircraftClass *airp;
	ObjectClass *objp;
	FILE *fp;
	HouseClass *housep;
	HousesType house;

	char filename[512];
	sprintf(filename, "SYNC%01d.TXT", PlayerPtr->HeapID);
	DebugString("Writing sync data to file %s\n", filename);

	Mono_Clear_Screen();
	Mono_Set_Cursor (0,0);

	fp = fopen(filename,"wt");
	if (fp==NULL) {
		DWORD const error = GetLastError();
		DebugString("Error - failed to open sync file for writing. Error code %d - %s\n", error, Last_Error_Text(error));
		return;
	}

	fprintf(fp, "TS Sync\n");

	fprintf(fp, "\nVersion %s\n", Version_Name());
	fprintf(fp, "Internal Version %s\n", VerNum.Version_Name());

	fprintf(fp, "Release Build: %s - %s\r\n", OPENTS_BUILD_DESCRIPTION, OPENTS_COMMIT_DATE);

	int cpu_type = PROC_PENTIUM_PRO;
	bool mmx = false;
	char vendor[32];
	vendor[0] = '\0';
	Get_CPU_Type(cpu_type, mmx, vendor, sizeof(vendor) - 1);
	fprintf(fp, "CPU vendor: %s\r\n", vendor);

	fprintf(fp, "Frames: %d\n", Frame);

	fprintf(fp, "Average FPS: %d\n", SecondsPassed != 0 ? TotalFrames / SecondsPassed : 0);
	fprintf(fp, "Max MaxAhead: %d\n", Session.MaxMaxAhead);
	fprintf(fp, "Latency setting: %d\n", Session.LatencyFudge);
	fprintf(fp, "Game speed setting: %d\n", Options.GameSpeed);
	fprintf(fp, "FrameSendRate: %d\n", Session.FrameSendRate);

	if (PacketTransport != NULL) {
		for (i = 0; i < PacketTransport->Get_Num_Local_Addresses(); i++) {
			unsigned char *addr = PacketTransport->Get_Local_Address(i);
			if (addr != NULL) {
				unsigned int a = *(unsigned int *)addr;
				fprintf(fp, "Local address: %d.%d.%d.%d\n", (a) & 0xFF, (a >> 8) & 0xFF, (a >> 16) & 0xFF, (a >> 24) & 0xFF);
			}
		}
	}

	for (i = 0; i < MAX_PLAYERS; i++) {
		MPStatsType *stat = &Session.ConnectionStats[i];
		if (*stat->Name != '\0') {
			fprintf(fp, "\nName: %s\n", stat->Name);
			fprintf(fp, "Address: %s\n", stat->Address.As_String());
			fprintf(fp, "Max avg round trip: %d\n", stat->MaxAvgRoundTrip);
			fprintf(fp, "Max round trip: %d\n", stat->MaxRoundTrip);
			fprintf(fp, "Resends: %d\n", stat->Resends);
			fprintf(fp, "Frame sync stalls: %d\n", stat->FrameSyncStalls);
			fprintf(fp, "Command cound stalls: %d\n", stat->CommandCountStalls);
			fprintf(fp, "Lost: %d\n", stat->Lost);
			fprintf(fp, "Percent lost: %d\n", stat->PercentLost);
		}
	}

	for (i = 0; i < ARRAY_SIZE(CRC); i++) {
		fprintf(fp,"CRC[%d]=%x\n",i,CRC[i]);
	}

	//
	// Houses
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		GameCRC = 0;
		housep = Houses[house];
		fprintf(fp,"%s: IsHuman:%d  Color:%s  ID:%d  HouseType:%s\n",
			(const char *)housep->IniName,
			housep->IsHuman,
			ColorSchemes[housep->Class->Scheme]->Name,
			housep->HeapID,
			HouseTypes[housep->Class->HeapID]->Name());
		//Add_CRC (&GameCRC, (int)housep->Credits + (int)housep->Power +
		//	(int)housep->Drain);
		Mono_Printf("House %s:%x\n",housep->Class->Name(),GameCRC);
	}

	//
	// Infantry
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"-------------------- %s Infantry -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Infantry.Count(); i++) {
			infp = (InfantryClass *)Infantry[i];
			if (infp->House==housep) {
				Add_CRC (&GameCRC, (int)infp->PositionCoord.As_Int() + (int)infp->PrimaryFacing.Current().As_Dir256());

				int tarcom_index = 0;
				const char *tarcom_name = _NO_NAME;
				AbstractClass *tarcom = infp->TarCom;
				if (tarcom != NULL) {
					if (tarcom->Is_Techno()) {
						tarcom_index = tarcom->Fetch_Heap_ID();
					} else if (tarcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)tarcom;
						tarcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					tarcom_name = Name_From_RTTI(tarcom->RTTI);
				}

				int navcom_index = 0;
				const char *navcom_name = _NO_NAME;
				AbstractClass *navcom = infp->NavCom;
				if (navcom != NULL) {
					if (navcom->Is_Techno()) {
						navcom_index = navcom->Fetch_Heap_ID();
					} else if (navcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)navcom;
						navcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					navcom_name = Name_From_RTTI(navcom->RTTI);
				}

				fprintf(fp,"COORD:%x   Facing:%d   Mission:%d   Type:%d(%s)   Tgt:%s(%d) Speed:%d NavCom:%s(%d)\n",
					infp->PositionCoord.As_Int(),(int)infp->PrimaryFacing.Current().As_Dir256(),infp->Get_Mission(),
					infp->Class->HeapID, infp->Class->Name(), tarcom_name, tarcom_index, int(infp->Speed * (MPH_LIGHT_SPEED + 1.0)), navcom_name, navcom_index);
			}
		}
		Mono_Printf("%s Infantry:%x\n",housep->Class->Name(),GameCRC);
	}

	//
	// Units
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"-------------------- %s Units -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Units.Count(); i++) {
			unitp = (UnitClass *)Units[i];
			if (unitp->House==housep) {
				Add_CRC (&GameCRC, (int)unitp->PositionCoord.As_Int() + (int)unitp->PrimaryFacing.Current().As_Dir256() +
					(int)unitp->SecondaryFacing.Current().As_Dir256());

				int tarcom_index = 0;
				const char *tarcom_name = _NO_NAME;
				AbstractClass *tarcom = unitp->TarCom;
				if (tarcom != NULL) {
					if (tarcom->Is_Techno()) {
						tarcom_index = tarcom->Fetch_Heap_ID();
					} else if (tarcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)tarcom;
						tarcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					tarcom_name = Name_From_RTTI(tarcom->RTTI);
				}

				int navcom_index = 0;
				const char *navcom_name = _NO_NAME;
				AbstractClass *navcom = unitp->NavCom;
				if (navcom != NULL) {
					if (navcom->Is_Techno()) {
						navcom_index = navcom->Fetch_Heap_ID();
					} else if (navcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)navcom;
						navcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					navcom_name = Name_From_RTTI(navcom->RTTI);
				}
				fprintf(fp,"COORD:%x   Facing:%d   Facing2:%d   Mission:%d   Type:%d(%s)   Tgt:%s(%d)  NavCom:%s(%d)   TrkNum:%d TrkInd:%d SpdAc:%d\n",
					unitp->PositionCoord.As_Int(),(int)unitp->PrimaryFacing.Current().As_Dir256(),(int)unitp->SecondaryFacing.Current().As_Dir256(),unitp->Get_Mission(),
					unitp->Class->HeapID, unitp->Class->Name(), tarcom_name, tarcom_index, navcom_name, navcom_index,
					unitp->Locomotion->Get_Track_Number(), unitp->Locomotion->Get_Track_Index(), unitp->Locomotion->Get_Speed_Accum());
			}
		}
		Mono_Printf("%s Units:%x\n",housep->Class->Name(),GameCRC);
	}

	//
	// Buildings
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"-------------------- %s Buildings -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Buildings.Count(); i++) {
			bldgp = (BuildingClass *)Buildings[i];
			if (bldgp->House==housep) {
				Add_CRC (&GameCRC, (int)bldgp->PositionCoord.As_Int() + (int)bldgp->PrimaryFacing.Current().As_Dir256());
				fprintf(fp,"COORD:%x   Facing:%d   Mission:%d   Type:%d(%s)   Tgt:%x\n",
					bldgp->PositionCoord.As_Int(),(int)bldgp->PrimaryFacing.Current().As_Dir256(),bldgp->Get_Mission(),
					bldgp->Class->HeapID, bldgp->Class->Name(), TargetClass(bldgp).Encode());
			}
		}
		Mono_Printf("%s Buildings:%x\n",housep->Class->Name(),GameCRC);
	}


	/// Aircraft
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"-------------------- %s Aircraft -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Aircraft.Count(); i++) {
			airp = (AircraftClass *)Aircraft[i];
			if (airp->House==housep) {
				Add_CRC (&GameCRC, (int)airp->PositionCoord.As_Int() + (int)airp->PrimaryFacing.Current().As_Dir256());
				fprintf(fp,"COORD:%x   Facing:%d   Mission:%d   Type:%d(%s)   Tgt:%x\n",
					airp->PositionCoord.As_Int(),(int)airp->PrimaryFacing.Current().As_Dir256(),airp->Get_Mission(),
					airp->Class->HeapID, airp->Class->Name(), TargetClass(airp).Encode());
			}
		}
		Mono_Printf("%s Buildings:%x\n",housep->Class->Name(),GameCRC);
	}

	#if 0
	//
	// Animations
	//
	AnimClass *animp;
		fprintf(fp,"-------------------- Animations -------------------\n");
	for (i = 0; i < Anims.Count(); i++) {
		animp = (AnimClass *)Anims[i];
		fprintf(fp,"Target:%x OwnerHouse:%d Loops:%d\n",
			animp->xObject,
			animp->OwnerHouse,
			animp->Loops);
	}
	#endif

	//------------------------------------------------------------------------
	// Map Layers
	//------------------------------------------------------------------------
	GameCRC = 0;
	for (i = 0; i < LAYER_COUNT; i++) {
		fprintf(fp,">>>> MAP LAYER %d <<<<\n",i);
		for (j = 0; j < Map.Layer[i].Count(); j++) {
			objp = Map.Layer[i][j];
			if (objp->RTTI != RTTI_ANIM || objp->Fetch_ID() != -2) {
				Add_CRC (&GameCRC, (int)objp->PositionCoord.As_Int() + (int)objp->RTTI);
				fprintf(fp,"Object %d: %x ",j,objp->PositionCoord.As_Int());
				if (objp->RTTI != RTTI_WAVE && objp->RTTI != RTTI_LIGHT) {
					fprintf(fp,"%s\t(Type:%d(%s)) ",
						Name_From_RTTI(objp->RTTI),
						((ObjectClass *)objp)->Class_Of()->Fetch_Heap_ID(),
						(const char *)((ObjectClass *)objp)->Class_Of()->IniName);
				} else {
					fprintf(fp,"%s\t(Type:%d) ",
						Name_From_RTTI(objp->RTTI),
						0);
				}
				housep = objp->Owner_HouseClass();
				if (housep!=NULL) {
					fprintf(fp,"Owner: %s\n",housep->Class->Name());
				}
				else {
					fprintf(fp,"Owner: NONE\n");
				}
			}
		}
	}
	Mono_Printf("Map Layers:%x  \n",GameCRC);

	//------------------------------------------------------------------------
	// Logic Layers
	//------------------------------------------------------------------------
	GameCRC = 0;
	fprintf(fp,">>>> LOGIC LAYER <<<<\n");
	for (i = 0; i < Logic.Count(); i++) {
		objp = Logic[i];
		if (objp->RTTI != RTTI_ANIM || objp->Fetch_ID() != -2) {
			Add_CRC (&GameCRC, (int)objp->PositionCoord.As_Int() + (int)objp->RTTI);
			fprintf(fp,"Object %d: %x ",i,objp->PositionCoord.As_Int());

			if (objp->RTTI == RTTI_AIRCRAFT)
				fprintf(fp,"Aircraft  (Type:%d(%s)) ",
					((AircraftClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((AircraftClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_ANIM)
				fprintf(fp,"Anim      (Type:%d(%s)) ",
					((AnimClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((AnimClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_BUILDING)
				fprintf(fp,"Building  (Type:%d(%s)) ",
					((BuildingClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((BuildingClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_BULLET)
				fprintf(fp,"Bullet    (Type:%d(%s)) ",
					((BulletClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((BulletClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_INFANTRY)
				fprintf(fp,"Infantry  (Type:%d(%s)) ",
					((InfantryClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((InfantryClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_OVERLAY)
				fprintf(fp,"Overlay   (Type:%d(%s)) ",
					((OverlayClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((OverlayClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_SMUDGE)
				fprintf(fp,"Smudge    (Type:%d(%s)) ",
					((SmudgeClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((SmudgeClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_TERRAIN)
				fprintf(fp,"Terrain   (Type:%d(%s)) ",
					((TerrainClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((TerrainClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_UNIT)
				fprintf(fp,"Unit      (Type:%d(%s)) ",
					((UnitClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((UnitClass *)objp)->Class_Of()->IniName);

			housep = objp->Owner_HouseClass();
			if (housep!=NULL) {
				fprintf(fp,"Owner: %s\n",housep->Class->Name());
			}
			else {
				fprintf(fp,"Owner: NONE\n");
			}
		}
	}
	Mono_Printf("Logic:%x  \n",GameCRC);

	//------------------------------------------------------------------------
	//	Random # generator, frame #
	//------------------------------------------------------------------------
	int rnd = Scen->RandomNumber();
	Mono_Printf("Random Number:%x  \n", rnd);
	fprintf(fp,"\nRandom Number:%d\n",rnd);

	Mono_Printf("My Frame:%d  \n",Frame);
	fprintf(fp,"My Frame:%d\n",Frame);

	CRCEngine *crc = NULL;

	if (ev) {
		fprintf(fp,"Offending event:\n");
		fprintf(fp,"Type:         %d\n",ev->Type);
		fprintf(fp,"Frame:        %d\n",ev->Frame);
		fprintf(fp,"ID:           %x\n",ev->ID);
		fprintf(fp,"CRC:          %x\n",ev->Data.FrameInfo.CRC);
		fprintf(fp,"CommandCount: %d\n",ev->Data.FrameInfo.CommandCount);
		fprintf(fp,"Delay:        %d\n",ev->Data.FrameInfo.Delay);
	}

	CRCEngine *c;
	c = new CRCEngine;
	if (c) {
		crc = c;
	}

	#define CRC_OBJECTS(_vector_) \
	for (j = 0; j < _vector_.Count(); j++) { \
		_vector_[j]->Compute_CRC(*crc); \
		fprintf(fp, "%05d      %08x\n", j, (*crc)()); \
	}

	fprintf(fp, "\n\n*************** House Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	CRC_OBJECTS(HouseTypes);

	fprintf(fp, "\n\n*************** House CRCs*******************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Houses);

	fprintf(fp, "\n\n*************** Unit Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(UnitTypes);

	fprintf(fp, "\n\n*************** Unit CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Units);

	fprintf(fp, "\n\n*************** Infantry Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(InfantryTypes);

	fprintf(fp, "\n\n*************** Infantry CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Infantry);

	fprintf(fp, "\n\n*************** BuildingType CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(BuildingTypes);

	fprintf(fp, "\n\n*************** Building CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Buildings);

	fprintf(fp, "\n\n*************** Aircraft Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(AircraftTypes);

	fprintf(fp, "\n\n*************** Aircraft CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Aircraft);

	fprintf(fp, "\n\n*************** Anim CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	for (j = 0; j < Anims.Count(); j++) {
		AnimClass *anim = Anims[j];
		if (anim->Fetch_ID() != -2) {
			anim->Compute_CRC(*crc);
			fprintf(fp, "%05d      %08x\n", j, (*crc)());
		}
	}

	fprintf(fp, "\n\n*************** Anim Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(AnimTypes);

	fprintf(fp, "\n\n*************** Task Force CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(TaskForces);

	fprintf(fp, "\n\n*************** Team Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(TeamTypes);

	fprintf(fp, "\n\n*************** Team CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Teams);

	fprintf(fp, "\n\n*************** Script Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(ScriptTypes);

	fprintf(fp, "\n\n*************** Script CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Scripts);

	fprintf(fp, "\n\n*************** Tag Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(TagTypes);

	fprintf(fp, "\n\n*************** Tag CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Tags);

	fprintf(fp, "\n\n*************** Trigger Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(TriggerTypes);

	fprintf(fp, "\n\n*************** Trigger CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Triggers);

	fprintf(fp, "\n\n*************** Action CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Actions);

	fprintf(fp, "\n\n*************** Event CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Events);

	fprintf(fp, "\n\n*************** Factory CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Factories);

	fprintf(fp, "\n\n*************** Voxel Anim Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(VoxelAnimTypes);

	fprintf(fp, "\n\n*************** Voxel Anim CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(VoxelAnims);

	fprintf(fp, "\n\n*************** Warhead CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Warheads);

	fprintf(fp, "\n\n*************** Weapon CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Weapons);

	fprintf(fp, "\n\n*************** Particle Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(ParticleTypes);

	fprintf(fp, "\n\n*************** Particle CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Particles);

	fprintf(fp, "\n\n*************** Particle System CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(ParticleSystems);

	fprintf(fp, "\n\n*************** Particle System Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(ParticleSystemTypes);

	fprintf(fp, "\n\n*************** Bullet Type's**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(BulletTypes);

	fprintf(fp, "\n\n*************** Bullet CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Bullets);

	fprintf(fp, "\n\n*************** Waypoint Path CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(WaypointPaths);

	fprintf(fp, "\n\n*************** Smudge Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(SmudgeTypes);

	fprintf(fp, "\n\n*************** Overlay Type CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(OverlayTypes);

	fprintf(fp, "\n\n*************** Light Source CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(LightSources);

	fprintf(fp, "\n\n*************** Building Light CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(BuildingLights);

	fprintf(fp, "\n\n*************** Tube CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Tubes);

	fprintf(fp, "\n\n*************** Sides CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Sides);

	fprintf(fp, "\n\n*************** Tiberium CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(Tiberiums);

	fprintf(fp, "\n\n*************** Em Pulse CRCs**************\n\n");
	fprintf(fp, "Index     CRC\n");
	fprintf(fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	delete crc;
	crc = new CRCEngine;
	CRC_OBJECTS(EMPulseClass::EMPulses);

	delete crc;

	fclose(fp);

}	/* end of Print_CRCs */


/***************************************************************************
 * Init_Queue_Mono -- inits mono display                                   *
 *                                                                         *
 * This routine steals control of the mono screen away from the rest of    *
 * the engine, by setting the global IsMono; if IsMono is set, the other   *
 * routines in this module turn off the Mono display when they're done     *
 * with it, so the rest of the engine won't over-write what we're writing. *
 *                                                                         *
 * INPUT:                                                                  *
 *      net      ptr to connection manager                                 *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Init_Queue_Mono(ConnManClass *net)
{
#ifdef _DEBUG
	//------------------------------------------------------------------------
	// Set 'IsMono' so we can steal the mono screen from the engine
	//------------------------------------------------------------------------
	if ((Frame==0 || Session.LoadGame) && MonoClass::Is_Enabled()) {
		IsMono = true;
	}

	//------------------------------------------------------------------------
	// Enable mono output for our stuff; we must Disable it before we return
	// control to the engine.
	//------------------------------------------------------------------------
	if (IsMono)
		MonoClass::Enable();

	if (net->Num_Connections() > 0) {
		//.....................................................................
		// Network mono debugging screen
		//.....................................................................
		if (NetMonoMode==0) {
			if (Frame==0 || Session.LoadGame || NewMonoMode) {
				net->Configure_Debug (0, sizeof (CommHeaderType),
					size_of(EventClass, Type), (char **)EventClass::EventNames, 0, 27);
				net->Mono_Debug_Print (0,1);
				NewMonoMode = 0;
			}
			else {
				net->Mono_Debug_Print (0,0);
			}
		}
		//.....................................................................
		// Flow control debugging output
		//.....................................................................
		else {
			if (NewMonoMode) {
				Mono_Clear_Screen();
				Mono_Printf("                         Queue AI:\n");	// flowcount[0]
				Mono_Printf("                Build Packet Loop:\n");	// flowcount[1]
				Mono_Printf("                       Frame Sync:\n");	// flowcount[2]
				Mono_Printf("                Frame Sync Resend:\n");	// flowcount[3]
				Mono_Printf("               Frame Sync Timeout:\n");	// flowcount[4]
				Mono_Printf("           Frame Sync New Message:\n");	// flowcount[5]
				Mono_Printf("                 DoList Execution:\n");	// flowcount[6]
				Mono_Printf("                  DoList Cleaning:\n");	// flowcount[7]
				Mono_Printf("\n");
				Mono_Printf("                            Frame:\n");
				Mono_Printf("                 Session.MaxAhead:\n");
				Mono_Printf("                       their_recv:\n");
				Mono_Printf("                       their_sent:\n");
				Mono_Printf("                          my_sent:\n");
				NewMonoMode = 0;
			}
		}
	}
#else
	net = net;
#endif
}	// end of Init_Queue_Mono


/***************************************************************************
 * Update_Queue_Mono -- updates mono display                               *
 *                                                                         *
 * INPUT:                                                                  *
 *      net            ptr to connection manager                           *
 *      flow_index      index # for flow-count updates                     *
 *                     -1: display                                         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Update_Queue_Mono(ConnManClass *net, int flow_index)
{
#ifdef _DEBUG
	static int flowcount[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

	//------------------------------------------------------------------------
	// If 'NetMonoMode' is 1, display flowcount info
	//------------------------------------------------------------------------
	if (NetMonoMode==1) {
		if (flow_index >= 0 && flow_index < 20) {
			Mono_Set_Cursor(35,flow_index);
			flowcount[flow_index]++;
			Mono_Printf("%d",flowcount[flow_index]);
		}
	}
	//------------------------------------------------------------------------
	// Otherwise, display the connection debug screen
	//------------------------------------------------------------------------
	else {
		net->Mono_Debug_Print (0,0);
	}

#else
	flow_index = flow_index;
	net = net;
#endif

}	// end of Update_Queue_Mono


/***************************************************************************
 * Print_Framesync_Values -- displays frame-sync variables                 *
 *                                                                         *
 * INPUT:                                                                  *
 *      curframe               current game Frame #                        *
 *      max_ahead            max-ahead value                               *
 *      num_connections      # connections                                 *
 *      their_recv            # commands I've received from my connections *
 *      their_sent            # commands each connection claims to have sent*
 *      my_sent               # commands I've sent                         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   11/21/1995 BRR : Created.                                             *
 *=========================================================================*/
static void Print_Framesync_Values(int curframe, unsigned int max_ahead,
	int num_connections, FrameSyncStruct *their, unsigned short my_sent)
{
#ifdef _DEBUG
	int i;

	if (NetMonoMode==1) {
		Mono_Set_Cursor(35,9);
		Mono_Printf("%d",curframe);

		Mono_Set_Cursor(35,10);
		Mono_Printf("%d",max_ahead);

		for (i = 0; i < num_connections; i++) {
			Mono_Set_Cursor(35 + i*5,11);
//			Mono_Printf("%4d",(int)their_recv[i]);
		}

		for (i = 0; i < num_connections; i++) {
			Mono_Set_Cursor(35 + i*5,12);
//			Mono_Printf("%4d",(int)their_sent[i]);
		}

		Mono_Set_Cursor(35,13);
		Mono_Printf("%4d",(int)my_sent);
	}
#else
	curframe = curframe;
	max_ahead = max_ahead;
	num_connections = num_connections;
	their = their;
	my_sent = my_sent;
#endif
}	// end of Print_Framesync_Values


/***************************************************************************
 * Dump_Packet_Too_Late_Stuff -- Dumps a debug file to disk                *
 *                                                                         *
 * INPUT:                                                                  *
 *      event      ptr to event to print                                   *
 *                                                                         *
 * OUTPUT:                                                                 *
 *      none.                                                              *
 *                                                                         *
 * WARNINGS:                                                               *
 *      none.                                                              *
 *                                                                         *
 * HISTORY:                                                                *
 *   06/28/1996 BRR : Created.                                             *
 *=========================================================================*/
void Dump_Packet_Too_Late_Stuff(EventClass *event)
{
	DebugString("Packet received too late!\n");
	DebugString("--------- Event data: -------------------\n");
	DebugString("Type:       %s\n",EventClass::EventNames[event->Type]);
	DebugString("Frame:      %d\n",event->Frame);
	DebugString("ID:         %d\n",event->ID);
	DebugString("MaxAhead=%d\n",Session.MaxAhead);
	DebugString("Frame=%d\n",Frame);
	DebugString("FrameSendRate=%d\n", Session.FrameSendRate);
}



/*************************** end of queue.cpp ******************************/
