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

/* $Header: /counterstrike/SESSION.H 4     3/10/97 6:23p Steve_tall $ */
/***************************************************************************
 **   C O N F I D E N T I A L --- W E S T W O O D    S T U D I O S        **
 *                                                                         *
 *                 Project Name : Command & Conquer                        *
 *                                                                         *
 *                    File Name : SESSION.H                                *
 *                                                                         *
 *                   Programmer : Bill R. Randolph                         *
 *                                                                         *
 *                   Start Date : 11/30/95                                 *
 *                                                                         *
 *                  Last Update : November 30, 1995 [BRR]                  *
 *                                                                         *
 * The purpose of this class is to contain those variables & routines      *
 * specifically related to a multiplayer game.                             *
 *                                                                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "ccfile.h"
#include "connect.h"
#include "event.h"
#include "house.h" /// needed for HOUSE_NAME_MAX
#include "ipxaddr.h"
#include "msglist.h"
#include "special.h"
#include "sun.h" /// needed for MAX_PLAYERS
#include "typelist.h"
#include "version.h"
#include "win.h"

#include <cstring>

#include "chat.hh"
#include "dialog.hh"
#include "diff.hh"

//---------------------------------------------------------------------------
// Forward declarations
//---------------------------------------------------------------------------
class AircraftClass;
class AnimClass;
class BuildingClass;
class BulletClass;
class InfantryClass;
class UnitClass;
class CellClass;
class INIClass;
class SaveStreamClass;

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
//...........................................................................
// Various limiting values
//...........................................................................
#define	MPLAYER_BUILD_LEVEL_MAX		10		// max build level in multiplay
#define	MAX_MPLAYER_COLORS			8		// max # of colors

//...........................................................................
// Max sizes of packets we want to send
// The IPX packet's size is IPX's max size (546), rounded down to accommodate
// the max number of events possible.
//...........................................................................
#define	MAX_IPX_PACKET_SIZE			(((546 - sizeof(CommHeaderType)) / \
												sizeof(EventClass) ) * sizeof(EventClass))
// Sizes the scenario transfer chunk below, so it is part of the network protocol.
#define	MAX_SERIAL_PACKET_SIZE		256

//...........................................................................
// Max length of player names fields; attempt to use the constant for the
// HouseClass, if it's been defined; otherwise, define it myself.
//...........................................................................
#ifdef HOUSE_NAME_MAX
#define	MPLAYER_NAME_MAX			HOUSE_NAME_MAX
#else
#define	MPLAYER_NAME_MAX			12		// max length of a player's name
#endif

//...........................................................................
// Values to control the multiplayer score screen
//...........................................................................
#define	MAX_MULTI_NAMES	8		// max # names (rows) on the score screen
#define	MAX_MULTI_GAMES	4		// max # games (columns) on the score screen

//...........................................................................
// Min value for MaxAhead; only applies for COMM_PROTOCOL_MULTI_E_COMP.
//...........................................................................
#define NETWORK_MIN_MAX_AHEAD		2

//...........................................................................
// Send period (in frames) for COMM_PROTOCOL_MULTI_E_COMP and above
//...........................................................................
#define DEFAULT_FRAME_SEND_RATE		3

#define SERIAL_MAX					23
#define ENCRYPTION_STRING_LENGTH	128

//---------------------------------------------------------------------------
// Enums
//---------------------------------------------------------------------------
//...........................................................................
// Types of games; used to tell which protocol we're using
//...........................................................................
enum GameType {
	GAME_NORMAL,									// not multiplayer
	GAME_MODEM,										// retired; slot kept, save headers store these values
	GAME_NULL_MODEM,								// retired
	GAME_IPX,										// IPX Network game
	GAME_INTERNET,									// Internet H2H
	GAME_SKIRMISH,									// 1 plr vs. AI's
	GAME_WDT,										/// World Domination Tour game
};

//...........................................................................
// Commands sent over the network Global Channel
//...........................................................................
enum NetCommandType {
	NET_QUERY_GAME,				// Hey, what games are out there?
	NET_ANSWER_GAME,			// Yo, Here's my game's name!
	NET_QUERY_PLAYER,			// Hey, what players are in this game?
	NET_ANSWER_PLAYER,			// Yo, I'm in that game!
	NET_CHAT_ANNOUNCE,			// I'm at the chat screen
	NET_CHAT_REQUEST,			// Respond with a CHAT_ANNOUNCE, please.
	NET_QUERY_JOIN,				// Hey guys, can I play too?
	NET_CONFIRM_JOIN,			// Well, OK, if you really want to.
	NET_REJECT_JOIN,			// No, you can't join; sorry, dude.
	NET_GAME_OPTIONS,			// Hey, dudes, here's some new game options
	NET_SIGN_OFF,				// Bogus, dudes, my boss is coming; I'm outta here!
	NET_GO,						// OK, jump into the game loop!
	NET_MESSAGE,				// Here's a message
	NET_PING,					// I'm pinging you to take a time measurement
	NET_LOADGAME,				// start a game by loading a saved game
	NET_PROGRESS_REPORT,		//
	NET_REQ_SCENARIO,			// Reqest that host sends the scenario file to the other players.
	NET_FILE_INFO,				// Info about the file that is going to be transferred
	NET_FILE_CHUNK,				// A chunk of scenario
	NET_READY_TO_GO,			// Sent in response to a 'GO' command
	NET_NO_SCENARIO,			// Scenario isnt available on remote machine so we cant play
	NET_FILE_INFO_ACK,			//
	NET_PUB_GAMEOPT,			//
	NET_PRIV_GAMEOPT,			//
	NET_PREVIEW_MODE,			//
	NET_PREVIEW_ACK,			//
	NET_REQ_PREVIEW,			//
	NET_PROPOSE_KICK,			//
	NET_MOVIE_SKIP,				// Which fullscreen movie the sender is watching and whether he wants it skipped
	NET_HOST_ANNOUNCE,			// The launch file's host names itself once the connections exist.
	NET_DESYNC_HEARTBEAT,		// Sent every second while the out-of-sync dialog halts the game.
	NET_DESYNC_CONTINUE,		// The master's decision to play on without the players out of sync.
	NET_LOAD_GAME,				// The master names the multiplayer save every machine loads.
};

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------
//...........................................................................
// An entry on the score screen is defined by this structure
//...........................................................................
struct MPlayerScoreType {
	char Name[MPLAYER_NAME_MAX];

	/*
	 * This is the color scheme this player last fought under, so that his row on the score
	 * screen is drawn in the color he wore on the battlefield.
	 */
	int Scheme;

	int Wins;

	/*
	 * These are the numbers of units and buildings this player lost in each round. If -1, then
	 * he took no part in that round.
	 */
	int Lost[MAX_MULTI_GAMES];
	int Kills[MAX_MULTI_GAMES];

	/*
	 * These record how much of what this player built was still standing at the end of each
	 * round, expressed as a percentage. If -1, then he took no part in that round.
	 */
	int Built[MAX_MULTI_GAMES];

	/*
	 * These are the point totals this player earned in each round, with a bonus folded in for
	 * surviving one. If -1, then he took no part in that round.
	 */
	int Score[MAX_MULTI_GAMES];
};

#pragma pack(1)
//...........................................................................
// This is a "node", used for the lists of available games & players.  The
// 'Game' structure is used for games; the 'Player' structure for players.
//...........................................................................
struct NodeNameType {
	char Name[MPLAYER_NAME_MAX];		// player or game name
		IPXAddressClass Address;
	union {
		struct {
			unsigned char IsOpen;		// is the game open?
			unsigned char Addon;
			unsigned int LastTime;		// last time we heard from this guy
		} Game;
		struct {
			char Serial[SERIAL_MAX];	//
			int House;					// "ActLike" House of this player
			int Color;					// Color of this player
			int ID;						// Actual House of this player
			int ProcessTime;			// Length of time to process players main loop
			int Status;					//
			int SquadID;				//
			int SpawnChoice;			// starting waypoint asked for; -1 = the engine picks
			int Handicap;				// difficulty asked for; -1 = the session default
			unsigned AlliesMask;		// seats allied with, one bit per seat index
			bool IsObserver;			// watches rather than plays; the house it becomes starts defeated
		} Player;
		struct {
			unsigned int LastTime;		// last time we heard from this guy
			unsigned char LastChance;	// we're about to remove him from the list
			int Color;					// chat player's color
		} Chat;
	};

	// A new node asks for nothing, leaving the start position and difficulty to the game.
	NodeNameType(void)
	{
		memset(this, 0, sizeof(*this));
		Player.SpawnChoice = -1;
		Player.Handicap = -1;

		// The memset above wipes the broadcast address the default constructor supplies.
		Address = IPXAddressClass();
	}
};


//...........................................................................
// Packet sent over the global channel to carry a scenario file
//...........................................................................
#define MAX_SEND_FILE_PACKET_SIZE MAX_SERIAL_PACKET_SIZE - 64
struct RemoteFileTransferType {
	NetCommandType	Command;                        // Enum defined above. Should be a file transfer enum.
	unsigned short 	BlockNumber;                    // Index position of this file chunk in the file
	unsigned short		BlockLength;                // Length of data in the RawData buffer

	/*
	 * This is the piece of the scenario file this packet carries. Only the first BlockLength
	 * bytes of it are meaningful.
	 */
	unsigned char 		RawData	[546 - 64 - 3];
};


//...........................................................................
// Packet sent over the network Global Channel
//...........................................................................
struct GlobalPacketType {
	NetCommandType Command;						// One of the enum's defined above
	char Name[MPLAYER_NAME_MAX];				// Player or Game Name
	char Serial[SERIAL_MAX];					//
	union {
		struct {
			unsigned int IsOpen		: 1;		// 1 = game is open for joining
			unsigned int IsFirestorm : 1;		//
		} GameInfo;
		struct {
			int House;							// player's House
			int Color;							// player's color
			unsigned int NameCRC;				// CRC of player's game's name
			unsigned int MinVersion;			// game's min supported version
			unsigned int MaxVersion;			// game's max supported version
			int CheatCheck;						// Unique ID of "rules.ini" file.
			int AICheatCheck;					/// Unique ID of "ai.ini" file.
			int ArtCheatCheck;					/// Unique ID of "art.ini" file.
			unsigned int BuildNumber;			///
		} PlayerInfo;
		struct {
			#if 0 /// Fields that fall 13 bytes short of where TS keeps FileLength (offset 0x83), so the region is padded instead of mapped.
			char Scenario[DESCRIP_MAX];			// Scenario Name
			unsigned int Credits;				// player's credits
			unsigned int IsBases		: 1;	// 1 = bases are allowed
			unsigned int IsTiberium	: 1;		// 1 = tiberium is allowed
			unsigned int IsGoodies	: 1;		// 1 = goodies are allowed
			unsigned int IsGhosties	: 1;		// 1 = ghosts are allowed
			unsigned int OfficialScenario :1;	// Is this scenario an official Westwood one?
			unsigned char BuildLevel;			// buildable level
			unsigned char UnitCount;			// max # units
			unsigned char AIPlayers;			// # of AI players allowed
			int Seed;							// random number seed
			SpecialClass Special;				// command-line options
			unsigned int GameSpeed;				// Game Speed
			unsigned int Version;				// version # common to all players
			#endif
			char pad[0x83 - 0x2F];
			unsigned int FileLength;			// Length of scenario file to expect from host.
			char ShortFileName[13];				// Name of scenario file to expect from host
			unsigned char FileDigest[32];		// Digest of scenario file to expect from host
												// ajw - This is not necessarily null-terminated.
		} ScenarioInfo;
		struct {
			char Buf[MAX_MESSAGE_LENGTH];		// inter-user message
			int Color;							// color of sender of message
			unsigned int NameCRC;				// CRC of sender's Game Name
			ChatScopeType Scope;				// who the message is for
		} Message;
		struct {
			int OneWay;							// one-way response time
		} ResponseTime;
		struct {
			int Why;							// why were we rejected from the game?
		} Reject;
		struct {
			unsigned int ID;   // unique ID for this chat node
			int Color;          // my color
		} Chat;

		/*
		 * This reports how far along the sender is in loading the scenario, expressed as a
		 * percentage. It accompanies the NET_PROGRESS_REPORT command.
		 */
		struct {
			int Percent;
		} Progress;

		/*
		 * This names the player proposing that somebody be thrown out of the game and the
		 * player he wants gone. It accompanies the NET_PROPOSE_KICK command.
		 */
		struct {
			unsigned int KickeeID;
			unsigned int KickerID;
		} Kick;

		/*
		 * This names the fullscreen movie the sender is watching, by a digest of its name and by
		 * its count among the movies the session has started, and says whether the sender wants
		 * it skipped. It accompanies the NET_MOVIE_SKIP command.
		 */
		struct {
			unsigned int Movie;
			unsigned int Instance;
			unsigned int Vote;
		} MovieSkip;

		/*
		 * This carries the game options as an encoded string, tagged with the sender's color
		 * and a CRC of his game's name. It accompanies the NET_PUB_GAMEOPT and
		 * NET_PRIV_GAMEOPT commands.
		 */
		struct {
			char Buf[400];
			int Color;
			unsigned int NameCRC;
		} Options;

		/*
		 * This names the numbered multiplayer save every machine is to load, by its slot.
		 * It accompanies the NET_LOAD_GAME command.
		 */
		struct {
			unsigned short Slot;
		} LoadGame;
	};
};
#pragma pack()

//...........................................................................
// For finding sync bugs; filled in by the engine when certain conditions
// are met; the pointers allow examination of objects in the debugger.
//...........................................................................
struct TrapObjectType {
	union {
		AircraftClass *Aircraft;
		AnimClass *Anim;
		BuildingClass *Building;
		BulletClass *Bullet;
		InfantryClass *Infantry;
		UnitClass *Unit;
		void *All;
	} Ptr;
};

/*
**	This is the identifier for a multiplayer mission. This can be used to
**	identify the filename of the mission as well as display the mission in a
**	mission selection list.
*/
class MultiMission
{
	public:
		MultiMission(INIClass const & ini, char const * filename = NULL);
		MultiMission(const char * filename = NULL, char const * description = NULL, char const *digest = NULL, bool official = true);

		void Set_Description(char const * description);
		void Set_Filename(char const * filename);
		void Set_Digest(char const * digest);
		void Set_Official(bool official);
		char const * Description(void) const {return(ScenarioDescription);}
		char const * Get_Filename(void) const {return(Filename);}
		char const * Get_Digest(void) const {return(Digest);}
		bool Get_Official(void) { return(IsOfficial); }

	private:
		char ScenarioDescription[DESCRIP_MAX];
		char Filename[_MAX_PATH];
		char Digest[32];
		bool IsOfficial;

		/*
		 * These are the player limits the mission declares in its own "Multiplay" section.
		 * Nothing consults them, so a mission's limits are not actually enforced.
		 */
		int MinPlayers;
		int MaxPlayers;
};


struct GameOptionsType {
	int 		ScenarioIndex;		//Used on host machine only as index into scenario list
	bool 		Bases;
	int 		Credits;
	bool		BridgeDestruction;	/// Weapons fire can bring bridges down.
	bool 		Goodies;
	bool		ShortGame;			/// A player is beaten once he has no buildings and no MCV.
	int			GameSpeed;			/// Host's speed setting (0 - 6), forced on every player.
	bool		CrapEngineers;		/// Engineers can only capture buildings at condition red.
	bool 		Ghosts;
	int 		UnitCount;
	int 		AIPlayers;			// # of AI players allowed to be built
	DiffType	AIDifficulty;		/// Difficulty applied to every AI house.
	bool		AlliesAllowed;		/// Alliances can be formed and broken during the game.
	bool		HarvTruce;			/// Harvesters are immune from attack.
	bool		CTF;				/// Play as capture the flag.
	bool		FogOfWar;			/// Ground the player can no longer see fogs back over.
	bool		MCVRedeploy;		/// A construction yard can be sold back into an MCV.
	bool		CoachMode;			// A defeated player keeps allied vision and private chat, and gets no map reveal.
	bool		AITakeover;			/// A departed player's house is handed to the computer rather than destroyed.
	char		ScenarioDescription [DESCRIP_MAX];	//Used on client machines only

	bool Save(IStream * stream);
	bool Load(IStream * stream);

	void Serialize(SaveStreamClass & stream);
};

struct MPStatsType {
	char Name[64];				/// Player these stats are for; empty marks an unused entry.
	int MaxRoundTrip;			/// Worst round trip time, in milliseconds.
	int Resends;				/// Packets that had to be sent more than once.
	int Lost;					/// Packets from this player that never arrived.
	int PercentLost;			/// Percentage of this player's packets that never arrived.
	int MaxAvgRoundTrip;		/// Worst the average round trip time ever got, in milliseconds.
	int FrameSyncStalls;		/// Times the game had to wait on this player's frame count.
	int CommandCountStalls;		/// Times the game had to wait on this player's command count.
	IPXAddressClass Address;	/// Address these stats were gathered from.
};

//---------------------------------------------------------------------------
// Class Definition
//---------------------------------------------------------------------------
class SessionClass
{
	//------------------------------------------------------------------------
	// Public interface
	//------------------------------------------------------------------------
	public:
		//.....................................................................
		// Constructor/Destructor
		//.....................................................................
		SessionClass(void);
		~SessionClass(void);

		//.....................................................................
		// Initialization
		//.....................................................................
		void One_Time(void);
		void Init(void);

		//.....................................................................
		// Reads/writes to the INI file
		//.....................................................................
		void Read_MultiPlayer_Settings (void);
		void Write_MultiPlayer_Settings (void);
		void Read_Scenario_Descriptions (void);
		void Free_Scenario_Descriptions(void);

		//.....................................................................
		// Utility functions
		//.....................................................................
		int Create_Connections(void);
		bool Am_I_Master(void);
		int Master_Player_ID(void) const;
		void Announce_Master(void);
		void Adopt_Master(int house, char const * name);
		unsigned int Compute_Unique_ID(void);
		void Update_Progress(int percent);
		void Init_Fixed_Alliances(void);
		int Color_Index_To_Scheme(int id);

		//.....................................................................
		// File I/O
		//.....................................................................

		//.....................................................................
		// Debugging / Sync Bugs
		//.....................................................................
		void Trap_Object(void);
		bool Log_To_File(FILE *out);

		//---------------------------------------------------------------------
		// Public Data
		//---------------------------------------------------------------------
		//.....................................................................
		// The type of session being played
		//.....................................................................
		GameType Type;
		bool IsWDT;
		int WDTTerritory;

		//.....................................................................
		// The current communications protocol
		//.....................................................................
		CommProtocolType CommProtocol;

		//.....................................................................
		// Game options
		//.....................................................................
		GameOptionsType Options;

		//.....................................................................
		// Unique workstation ID, for detecting my own packets
		//.....................................................................
		unsigned int UniqueID;

		//.....................................................................
		// Player's local options
		//.....................................................................
		char Handle[MPLAYER_NAME_MAX];      // player name
		int PrefColor;                      // preferred color index
		int ColorIdx;                       // actual color index
		int House;                          // GDI / NOD
		int ObiWan;                         // 1 = player can see all
		bool AIOnly;                        // no human seat plays; only the last enemy falling ends the match
		int Solo;                           // 1 = player can play alone

		/*
		 * The pair a campaign mission is played at. It lives here because the scenario's own
		 * copy is wiped before each mission, while a restart or the next one must keep it.
		 */
		DiffType CampaignDifficulty;
		DiffType CampaignCDifficulty;

		/*
		 * If the local player is playing a GDI house, then this flag will be true. A starting
		 * multiplayer scenario takes its side and its speech set from it.
		 */
		bool PlayerIsGDI;

		//.....................................................................
		// Max allowable # of players & actual # of (human) players
		//.....................................................................
		int MaxPlayers;
		int NumPlayers;

		//.....................................................................
		// Frame-sync'ing timing variables
		// 'MaxAhead' is the number of frames ahead of this one to execute
		// a given packet.  It's set by the RESPONSE_TIME event.
		// 'FrameSendRate' is the # frames between data packets
		//.....................................................................
		unsigned int MaxAhead;
		unsigned int FrameSendRate;

		// How long this machine waits on another, in game ticks. Each machine keeps its own: the
		// waits decide when this machine gives up, never what the match computes.
		int ConnTimeout;			/// no loading progress from a machine before it is dropped
		int ReconnectTimeout;		/// silence from a machine, once playing, before it is dropped

		int			DesiredFrameRate;

		int			ProcessTimer;
		int			ProcessTicks;
		int			ProcessFrames;

		/*
		 * This is the largest MaxAhead the game has run at, since the value only ever grows.
		 * The sync bug report carries it as a measure of how bad the connection ever got.
		 */
		int			MaxMaxAhead;

		/*
		 * These are the frame timings Westwood Online worked out from the players' connection
		 * speeds. While either is non-zero the host sends them out instead of measuring the
		 * connections itself, and clears both once it has.
		 */
		int			PrecalcMaxAhead;
		int			PrecalcDesiredFrameRate;

		/*
		 * These are the network statistics gathered for each player over the course of the
		 * game. They feed the network diagnostics display and the sync bug report.
		 */
		MPStatsType ConnectionStats[MAX_PLAYERS];

		/*
		 * These identify the host of an internet game, the machine that hands the network
		 * timings down to everyone else. The name is learned from the chat channel and the
		 * ID is filled in once that name is matched to a player, so it is -1 until then.
		 */
		char MasterPlayerName[MPLAYER_NAME_MAX + 1];
		int MasterPlayerID;

		/*
		 * If the network diagnostics display is to be drawn over the game, then this flag
		 * will be true. It is a developer aid, turned on by a command line switch.
		 */
		bool ShowInternetDebug;

		//.....................................................................
		// This flag is set when we've loaded a multiplayer game.
		//.....................................................................
		int LoadGame;

		//.....................................................................
		// This flag is set when the modem game saves the game due to a lost
		// connection.
		//.....................................................................
		int EmergencySave;

		/*
		 * If Westwood Online allied the players into squads, then this flag will be true.
		 * Such alliances are fixed for the match, so nobody can change sides part way in.
		 */
		bool SquadAlliances;

		/*
		 * If this machine watched the game through to its end, then this flag will be true.
		 * A game the player walked out of is reported differently from one he saw finished.
		 */
		bool SawGameCompletion;

		/*
		 * If the game has fallen out of sync, then this flag will be true. It is reported
		 * along with the results, so that a desync can be told from a clean finish.
		 */
		bool OutOfSync;

		unsigned int PlayingAgainstVersion;		// Negotiated version number

		//.....................................................................
		// List of scenarios & their file numbers
		//.....................................................................
		DynamicVectorClass<MultiMission *> Scenarios;

		char ScenarioFileName[_MAX_FNAME+_MAX_EXT+2];	//File name of scenario to load

		char ScenarioDigest [32+2];						//Digest of scenario to load
		unsigned int ScenarioFileLength;
		bool ScenarioIsOfficial;

		char ScenarioRequests[MAX_PLAYERS];		//Which players requested scenario files
		int  RequestCount;
		IPXAddressClass	HostAddress;

		/*
		 * These are the kick proposals that have arrived but not yet been counted. A proposal
		 * can turn up at any time, while the votes are only tallied from the wait loop.
		 */
		DynamicVectorClass<GlobalPacketType *> KickProposals;

		/*
		 * These are the numbers of votes cast to kick each player out of the game. A player
		 * is dropped once every other player has voted against him.
		 */
		int KickVoteCount[MAX_PLAYERS];

		/*
		 * These record who has voted to kick each player, so that nobody gets to vote against
		 * the same victim twice.
		 */
		int KickVoteWho[MAX_PLAYERS][MAX_PLAYERS];

		//.....................................................................
		// This is the multiplayer messaging system
		//.....................................................................
		MessageListClass Messages;
		IPXAddressClass MessageAddress;
		ChatScopeType MessageScope;         // who the open edit goes to
		int MessageTarget;                  // the house a private message goes to, else -1
		char LastMessage[MAX_MESSAGE_LENGTH];

		//.....................................................................
		// This is the multiplayer scorekeeping system
		//.....................................................................
		MPlayerScoreType Score[MAX_MULTI_NAMES];
		int GamesPlayed;		// # games played this run
		int NumScores;			// # active entries in MPlayerScore
		int Winner;				// index of winner of last game
		int CurGame;			// index of current game being played

		//.....................................................................
		// Static arrays
		//.....................................................................
		static char Descriptions[100][40];
		static int CountMin[2];
		static int CountMax[2];
		static char const * GlobalPacketNames[];

		//.....................................................................
		// For Recording & Playing back a file
		//.....................................................................
		CCFileClass RecordFile;
		unsigned Record				: 1;
		unsigned Play				 	: 1;
		unsigned Attract			 	: 1;

		//.....................................................................
		// Network-specific variables
		//.....................................................................
		bool NetStealth;                                // makes us invisible
		bool NetOpen;                                   // 1 = game is open for joining
		bool PlayMovies;                                // a launch file asked for movies outside a campaign
		bool SkipScoreScreen;                           // a launch file asked that the score screen be passed over
		char LoadScreen[_MAX_PATH];                     // the picture to show while the scenario loads, or empty
		int LoadScreenX;                                // where in that picture the loading bars go, or zero for
		int LoadScreenY;                                // the position the game picks for its own
		char DifficultyName[32];                        // what a launch file calls the campaign difficulty
		char GameName[MPLAYER_NAME_MAX];                // game's name
		GlobalPacketType GPacket;                       // global packet
		int GPacketlen;                                 // global packet length
		IPXAddressClass GAddress;                       // address of sender
		unsigned short GProductID;                      // product ID of sender
		char MetaPacket[MAX_IPX_PACKET_SIZE];           // packet building buffer
		int MetaSize;                                   // size of MetaPacket
		DynamicVectorClass <NodeNameType *> Games;      // list of games
		DynamicVectorClass <NodeNameType *> Players;    // list of players
		DynamicVectorClass <NodeNameType *> Chat;       // list of chat nodes

		// The computer players a launch file seated, after the humans; the menu leaves this empty.
		DynamicVectorClass <NodeNameType *> Computers;
		int Suspended;

		/*
		 * These are the numbers of frames each remote player is running behind this machine.
		 * The main loop slows its network send rate when the worst of them grows too large.
		 */
		int PlayerLatency[MAX_PLAYERS];

		/*
		 * This scales up the measured connection response time when the frame timing is
		 * computed (0 - 3), buying tolerance of a laggy link at the cost of responsiveness.
		 */
		int LatencyFudge;

		//.....................................................................
		// For finding Sync Bugs
		//.....................................................................
		int TrapFrame;             // frame # to start trapping 'TrapObject'
		RTTIType TrapObjType;       // type of object to trap
		TrapObjectType TrapObject;  // ptr to object to trap (watch)
		Coord TrapCoord;            // coord of object, 0 = ignore
		TargetClass TrapTarget;     // Target # of object, 0 = ignore
		CellClass * TrapCell;       // Ptr to cell to trap (watch)
		int TrapCheckHeap;          // true = check the heap as of TrapFrame
		int TrapPrintCRC;          // Frame # to print CRC state file
		int ForceDesyncFrame;       // Frame # to corrupt our own checksum at, -1 = never
};

extern SessionClass Session;

/*************************** end of session.h ******************************/
