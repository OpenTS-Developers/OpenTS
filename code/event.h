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

/* $Header: /CounterStrike/EVENT.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : EVENT.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 12/09/94                                                     *
 *                                                                                             *
 *                  Last Update : December 9, 1994 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "coord.h"
#include "special.h"
#include "target.h"

#include "anim.hh"
#include "house.hh"
#include "mission.hh"
#include "mph.hh"
#include "speed.hh"

#include <cstdint>
#include <cstring>

/*
**	This event class is used to contain all external game events (things that the player can
**	do at any time) so that these events can be transported between linked computers. This
**	encapsulation is required in order to ensure that each event affects all computers at the
**	same time (same game frame).
**	NOTE: If you add or remove an event type, you must also update the globals
**	EventLength[] and EventNames[].
*/
#pragma pack(push,1)
class EventClass
{
	public:

		/*
		**	All external events are identified by these labels.
		*/
		enum EventType {
			EMPTY,

			POWERON,
			POWEROFF,
			ALLY,           // Make allie of specified house.
			MEGAMISSION,    // Full change of mission with target and destination.
			MEGAMISSION_F,  // Full change of mission with target and destination, and formation overrides.
			IDLE,           // Request to enter idle mode.
			SCATTER,        // Request to scatter from current location.
			DESTRUCT,       // Self destruct request (surrender action).
			DEPLOY,         // MCV is to deploy at current location.
			PLACE,          // Place building at location specified.
			OPTIONS,        // Bring up options screen.
			GAMESPEED,      // Set game speed
			PRODUCE,        // Start or Resume production.
			SUSPEND,        // Suspend production.
			ABANDON,        // Abandon production.
			PRIMARY,        // Primary factory selected.
			SPECIAL_PLACE,  // Special target location selected
			EXIT,           // Exit game.
			ANIMATION,      // Flash ground as movement feedback.
			REPAIR,         // Repair specified object.
			SELL,           // Sell specified object.
			SELLCELL,       // Sell wall at specified cell.
			SPECIAL,        // Special options control.

			// Private events.
			FRAMESYNC,			// Game-connection packet; includes Scenario CRC & sender's frame #
									// Used to initiate game connection phase & to reconnect;
									// When one of these is received, the receiver knows there are
									// no associated commands in this packet.
			MESSAGE,				// Message to another player (The message is the 40 bytes
									// after the event class).
			RESPONSE_TIME,		// use a new propagation delay value
			FRAMEINFO,			// Game-heartbeat packet; includes Game CRC & command count
									// All packets sent for a frame are prefixed with one of these
			SAVEGAME,   // allows multiplayer games to save
			ARCHIVE,    // Updates archive target on specified object.
			ADDPLAYER,  // Add a new player

			TIMING,				// new timing values for all systems to use
			PROCESS_TIME,		// a system's average processing time, in ticks per frame

			PAGEUSER,			// retired; slot kept, the event type travels in network packets
			REMOVEPLAYER,

			LATENCYFUDGE,
			NETWORK_REPORT,
			ABANDON_COUNT,
			GRANT_CONTROL,		// Grant or withdraw shared control of the sender's units.
			CHAT_PROOF,			// A chat line's fingerprint. Reserved: nothing sends it, and executing it changes nothing.
			SELECTED,			// The owner selected the object, which springs its tag's "selected by player" event.
			UNDEPLOY,			// Undeploy a building

			LAST_EVENT,			// one past the last event
		};

		static constexpr std::uint16_t NETWORK_RTT_UNAVAILABLE = UINT16_MAX;

		unsigned char Type;		// Type of queue command object.

		/*
		** 'Frame' is the frame that the command should execute on.
		**	27 bits gives over 25 days of playing time without wrapping,
		**	at 30 frames per second, so it should be plenty!
		*/
		int Frame;

		/*
		**	This bit tells us if we've already executed this event.
		*/
		bool IsExecuted;

		/*
		**	House index of the player originating this event
		*/
		int ID;

		/*
		**	This union contains the specific data that the event requires.
		*/
		union {
			struct {
				SpecialClass	Data;		// The special option flags.
			} Options;
			struct {
				xCell			Where;			// The cell to sell wall at.
			} SellCell;
			struct {
				xTargetClass	Whom;			// The object to apply the event to.
			} Target;
			struct {
				AnimType		What;   // The animation to create.
				HousesType	Owner;      // The owner of the animation (when it matters).
				xCoord			Where;  // The location to place the animation.
			} Anim;
			struct {
				int 			Value;		// general-purpose data
			} General;
			struct {
				xTargetClass	Whom;			// Whom to apply mission to.
				MissionType	Mission;			// What mission to apply.
				xTargetClass	Target;			// Target to assign.
				xTargetClass	Destination;	// Destination to assign.
			} MegaMission;
			struct {
				xTargetClass	Whom;           // Whom to apply mission to.
				MissionType	Mission;            // What mission to apply.
				xTargetClass	Target;         // Target to assign.
				xTargetClass	Destination;    // Destination to assign.
				SpeedType	Speed;              // Formation override speed.
				MPHType		MaxSpeed;           // Formation override maximum speed.
			} MegaMission_F;
			struct {
				xTargetClass	Whom;   // Whom to apply mission to.
				MissionType Mission;    // What mission to apply.
			} Mission;
			struct {
				xTargetClass	Whom;   // Whom to apply movement change to.
				xTargetClass	Where;  // Where to set NavCom to.
			} NavCom;
			struct {
				xTargetClass	Whom;   // Whom to apply attack change to.
				xTargetClass	Target; // What to set TarCom to.
			} TarCom;
			struct {
				RTTIType		Type;
				int			ID;
			} Specific;
			struct {
				RTTIType		Type;
				int			ID;
				int			Count;	// How many to abandon, the one in progress last.
			} AbandonCount;
			struct {
				RTTIType		Type;
				xCell			Where;
			} Place;
			struct {
				int			ID;
				xCell			Where;
			} Special;

			/*
			**	This structure is used for FRAMEINFO, FRAMESYNC, and RESPONSE_TIME
			**	events; exactly one of these will be sent each frame, whether there's
			**	data that frame or not.
			**	CRC: the game CRC when this packet was generated; used to detect sync errors
			**	CommandCount: # of commands the sender has sent; used to detect missed packets
			**	Delay: sender's propagation delay value for this frame
			*/
			struct {
				unsigned int CRC;
				unsigned short CommandCount;    // # commands sent so far
				unsigned char Delay;            // propagation delay used this frame
														// (Frame - Delay = sender's current frame #)
			} FrameInfo;

			/*
			**	This structure is used for the special variable-length event.  This event
			**	can be anything the application wants it to be.  Its purpose is to allow
			**	relatively large amounts of data to be associated with the event, without
			**	bloating the size of this union (and thus all other event types).
			*/
			struct {
				void * Pointer;
				unsigned int Size;
			} Variable;

			//
			// This structure sets new timing values for all systems in a multiplayer
			// game.  This structure replaces the RESPONSE_TIME event for
			// the COMM_MULTI_E_COMP protocol.
			//
			struct {
				unsigned short DesiredFrameRate;
				unsigned short MaxAhead;
				unsigned char FrameSendRate;
			} Timing;

			//
			// This structure is transmitted by all systems, and is used to compute
			// the "desired" frame rate for the game.
			//
			struct {
				unsigned short AverageTicks;
			} ProcessTime;

			struct {
				std::uint16_t AverageProcessMilliseconds;
				std::uint16_t WorstRoundTripMilliseconds;
				std::uint16_t StallMilliseconds;
			} NetworkReport;

			struct {
				unsigned char Digest[16];
			} ChatProof;

		} Data;

		//-------------- Constructors ---------------------
		// Every byte starts at zero, so payload bytes and padding an event type leaves unused match on every machine.
		EventClass(void) {std::memset(static_cast<void *>(this), 0, sizeof(*this)); Type = EMPTY;};
		EventClass(int index, SpecialClass data);
		EventClass(int index, unsigned char type, TargetClass target);
		EventClass(int index, unsigned char type);
		EventClass(int index, unsigned char type, int val);
		EventClass(int index, unsigned char type, Cell const & cell);
		EventClass(int index, unsigned char type, TargetClass src, TargetClass dest);
		EventClass(int index, TargetClass src, MissionType mission, TargetClass target/*=TARGET_NONE*/, TargetClass destination/*=TARGET_NONE*/);

		EventClass(int index, TargetClass src, MissionType mission, TargetClass target, TargetClass destination, SpeedType speed, MPHType maxspeed);

		EventClass(int index, unsigned char type, RTTIType object, int id);
		EventClass(int index, unsigned char type, RTTIType object, int id, int count);
		EventClass(int index, unsigned char type, RTTIType object, Cell const & cell);
		EventClass(int index, unsigned char type, int id, Cell const & cell);
		EventClass(int index, AnimType anim, HousesType owner, Coord const & coord);
		EventClass(int index, unsigned char type, void *ptr, unsigned int size);

		// Why a received event was refused; each value has a log name in event.cpp.
		enum class RejectReason : unsigned int {
			InvalidType,
			InvalidOrigin,
			MissingOrigin,
			InvalidAllyHouse,
			InvalidAnimationType,
			InvalidAnimationOwner,
			InvalidProductionSelector,
			InvalidSuperWeapon,
			InvalidMission,
			InvalidGameSpeed,
			InvalidRemovedHouse,
			InvalidLatencyFudge,
			UnauthorizedSubject,
			UnauthorizedTiming,
			InvalidTimingArithmetic,
			InvalidTimingValues,
			UnschedulableTiming,
			InvalidNetworkReport,
			InvalidAbandonCount,
			UnsentEvent,
			LiveRemoval,
			PlacementNotBuilding,
			PlacementOffMap,
			PlacementTooFar,
			PlacementShrouded,
			ProductionNotAllowed,
			SaleNotAllowed,
			RepairNotAllowed,
			PowerToggleNotAllowed,
			WallNotOwned,
			MissionNotAllowed,
			TargetHidden,
			SuperWeaponOutOfRange,
			AllianceNotAllowed,
			InvalidGrantHouse,
			ControlGrantNotAllowed,
			UndeployNotAllowed,
			Count,
		};

		// Process the event.
		void Execute(void);

		static void Reset_Announcements(void);

		int operator == (EventClass & q) {
			return(memcmp(this, &q, sizeof(q)) == 0);
		};

		static unsigned char EventLength[LAST_EVENT];
		static char const * EventNames[LAST_EVENT];

	private:
		// What the sender must hold over the object an event commands.
		enum class SubjectRule {
			NONE,
			OWNER,
			COMMANDER,	// ownership or a shared control grant
		};

		bool Is_Admitted(void) const;
		void Reject(RejectReason reason, int detail) const;
		TechnoClass * Subject(SubjectRule & rule) const;
		void Announce_Setting_Change(int text) const;

		void Execute_POWER(void);
		void Execute_ARCHIVE(void);
		void Execute_ALLY(void);
		void Execute_DESTRUCT(void);
		void Execute_SPECIAL(void);
		void Execute_REPAIR(void);
		void Execute_SELL(void);
		void Execute_SELLCELL(void);
		void Execute_ANIMATION(void);
		void Execute_PLACE(void);
		void Execute_PRODUCE(void);
		void Execute_SUSPEND(void);
		void Execute_ABANDON(void);
		void Execute_ABANDON_COUNT(void);
		void Execute_PRIMARY(void);
		void Execute_MEGAMISSION(void);
		void Execute_IDLE(void);
		void Execute_DEPLOY(void);
		void Execute_SCATTER(void);
		void Execute_SPECIAL_PLACE(void);
		void Execute_EXIT(void);
		void Execute_OPTIONS(void);
		void Execute_GAMESPEED(void);
		void Execute_RESPONSE_TIME(void);
		void Execute_SAVEGAME(void);
		void Execute_ADDPLAYER(void);
		void Execute_REMOVEPLAYER(void);
		void Execute_LATENCYFUDGE(void);
		void Execute_TIMING(void);
		void Execute_PROCESS_TIME(void);
		void Execute_NETWORK_REPORT(void);
		void Execute_GRANT_CONTROL(void);
		void Execute_SELECTED(void);
		void Execute_UNDEPLOY(void);
};
#pragma pack(pop)
