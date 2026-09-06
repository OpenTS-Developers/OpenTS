/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#include "always.h"

#include "movieskip.h"

#include "_keyboar.h"
#include "conquer.h"
#include "crc.h"
#include "data.h"
#include "dbgprint.h"
#include "dialog.h"
#include "font.h"
#include "globals.h"
#include "ipxmgr.h"
#include "keyboard.h"
#include "language/language.h"
#include "netglobal.h"
#include "point.h"
#include "rgb.h"
#include "scheme.h"
#include "session.h"
#include "stimer.h"
#include "surface.h"
#include "timer.h"

#include "color.hh"
#include "dialog.hh"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>


namespace MovieSkip
{
	namespace {

		constexpr int SERVICE_INTERVAL = TIMER_SECOND / 20;
		constexpr int STATUS_INTERVAL = TIMER_SECOND;
		constexpr int BOX_PADDING = 6;
		constexpr int BOX_OPACITY = 75;

		struct VoteState
		{
			bool IsActive = false;
			unsigned int Movie = 0;
			unsigned int Instance = 0;
			std::array<bool, MAX_PLAYERS> Votes = {};
			CDTimerClass<SystemTimerClass> ServiceTimer;
			CDTimerClass<SystemTimerClass> StatusTimer;
		};

		VoteState State;
		unsigned int MoviesStarted = 0;
		int LocalScopeDepth = 0;
		unsigned int LastIgnoredMovie = 0;
		unsigned int LastIgnoredInstance = 0;


		/// <summary>Whether the machines of this session keep in step while a movie plays.</summary>
		bool Is_Network_Game(void)
		{
			return((Session.Type == GAME_IPX || Session.Type == GAME_INTERNET) && !Session.Play && Session.Players.Count() > 1);
		}


		int Local_Player(void)
		{
			if (Session.Players.Count() == 0 || Session.Players[0] == NULL) {
				return(-1);
			}
			int const id = Session.Players[0]->Player.ID;
			return(id >= 0 && id < MAX_PLAYERS ? id : -1);
		}


		char const * Player_Name(int player)
		{
			for (int index = 0; index < Session.Players.Count(); index++) {
				NodeNameType const * node = Session.Players[index];
				if (node != NULL && node->Player.ID == player) {
					return(node->Name);
				}
			}
			return("");
		}


		unsigned int Movie_Digest(char const * name)
		{
			CRCEngine crc;
			for (; name != NULL && *name != '\0'; name++) {
				crc((char)std::toupper((unsigned char)*name));
			}
			return((unsigned int)crc());
		}


		void Send_Status(void)
		{
			int const local = Local_Player();
			GlobalPacketType packet;
			NetGlobal::Initialize_Packet(packet, NET_MOVIE_SKIP);
			if (local >= 0) {
				std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name) - 1);
			}
			packet.MovieSkip.Movie = State.Movie;
			packet.MovieSkip.Instance = State.Instance;
			packet.MovieSkip.Vote = local >= 0 && State.Votes[local] ? 1 : 0;

			for (int index = 1; index < Session.Players.Count(); index++) {
				Ipx.Send_Global_Message(&packet, sizeof(packet), 0, &Session.Players[index]->Address);
			}
			State.StatusTimer = STATUS_INTERVAL;
		}


		void Cast_Local_Vote(void)
		{
			int const local = Local_Player();
			if (local < 0 || State.Votes[local]) {
				return;
			}
			State.Votes[local] = true;
			DebugString("Movie skip: this machine votes to skip movie %08x/%u.\n", State.Movie, State.Instance);
			Send_Status();
		}


		/// <summary>Counts the players the vote waits on and how many have voted.</summary>
		void Tally(int & players, int & votes, int & remote_voter)
		{
			int const local = Local_Player();
			players = 0;
			votes = 0;
			remote_voter = -1;
			for (int index = 0; index < Session.Players.Count(); index++) {
				NodeNameType const * node = Session.Players[index];
				int const id = node != NULL ? node->Player.ID : -1;
				if (id < 0 || id >= MAX_PLAYERS) {
					continue;
				}
				players++;
				if (State.Votes[id]) {
					votes++;
					if (id != local) {
						remote_voter = id;
					}
				}
			}
		}

	}	// namespace


	Playback::Playback(char const * name)
	{
		State = VoteState();
		if (!Is_Network_Game()) {
			return;
		}

		MoviesStarted++;
		if (LocalScopeDepth > 0) {
			DebugString("Movie skip: movie \"%s\" ends on ESC alone.\n", name != NULL ? name : "");
			return;
		}

		State.IsActive = true;
		State.Movie = Movie_Digest(name);
		State.Instance = MoviesStarted;
		DebugString("Movie skip: movie %08x/%u \"%s\" waits for every player.\n", State.Movie, State.Instance, name != NULL ? name : "");
	}


	Playback::~Playback(void)
	{
		if (State.IsActive) {
			DebugString("Movie skip: movie %08x/%u is over.\n", State.Movie, State.Instance);
		}
		State = VoteState();
	}


	LocalScope::LocalScope(void)
	{
		LocalScopeDepth++;
	}


	LocalScope::~LocalScope(void)
	{
		LocalScopeDepth--;
	}


	/// <summary>
	/// Runs once per pass of the fullscreen movie loop and answers whether the movie should stop.
	/// In a game against other machines it services the network, tells the other machines where
	/// this one is, and turns ESC into a vote; otherwise ESC stops the movie at once.
	/// </summary>
	bool Idle(void)
	{
		bool const escape = Keyboard->Check() && Keyboard->Get() == (KN_ESC|WWKEY_RLS_BIT);
		if (!State.IsActive) {
			return(escape);
		}

		if (escape) {
			Cast_Local_Vote();
		}

		if (State.ServiceTimer == 0) {
			IPX_Call_Back();
			State.ServiceTimer = SERVICE_INTERVAL;
		}

		if (State.StatusTimer == 0) {
			Send_Status();
		}

		int players;
		int votes;
		int remote_voter;
		Tally(players, votes, remote_voter);
		return(players > 0 && votes == players);
	}


	/// <summary>
	/// Records what another machine says about the movie it is watching. A report about some
	/// other movie, or one that arrives while no movie waits, is ignored.
	/// </summary>
	void Receive(int player, GlobalPacketType const & packet)
	{
		unsigned int const movie = packet.MovieSkip.Movie;
		unsigned int const instance = packet.MovieSkip.Instance;
		if (!State.IsActive || movie != State.Movie || instance != State.Instance) {
			if (packet.MovieSkip.Vote != 0 && (movie != LastIgnoredMovie || instance != LastIgnoredInstance)) {
				DebugString("Movie skip: ignored a vote for movie %08x/%u.\n", movie, instance);
				LastIgnoredMovie = movie;
				LastIgnoredInstance = instance;
			}
			return;
		}

		if (player < 0 || player >= MAX_PLAYERS || packet.MovieSkip.Vote == 0 || State.Votes[player]) {
			return;
		}
		State.Votes[player] = true;
		DebugString("Movie skip: player %d votes to skip movie %08x/%u.\n", player, movie, instance);
	}


	/// <summary>
	/// Prints the state of the vote over the movie frame once anybody has voted.
	/// </summary>
	void Draw_Overlay(Surface & surface, Rect const & area)
	{
		if (!State.IsActive) {
			return;
		}

		int players;
		int votes;
		int remote_voter;
		Tally(players, votes, remote_voter);
		if (votes == 0) {
			return;
		}

		ColorScheme * scheme = Fetch_Scheme_By_Name("Green");
		TextPrintType const flags = TextPrintType(TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW);
		FontClass const * font = Font_From_TPF(flags);
		if (scheme == NULL || font == NULL) {
			return;
		}

		int const local = Local_Player();
		bool const local_voted = local >= 0 && State.Votes[local];
		int const remote_votes = votes - (local_voted ? 1 : 0);

		char status[128];
		if (remote_votes == 1) {
			std::snprintf(status, sizeof(status), Fetch_String(TXT_MOVIE_SKIP_ONE), Player_Name(remote_voter));
		} else if (remote_votes == 0) {
			std::snprintf(status, sizeof(status), "%s", Fetch_String(TXT_MOVIE_SKIP_SELF));
		} else {
			std::snprintf(status, sizeof(status), Fetch_String(TXT_MOVIE_SKIP_MANY), votes);
		}
		char prompt[128];
		std::snprintf(prompt, sizeof(prompt), Fetch_String(local_voted ? TXT_MOVIE_SKIP_WAIT : TXT_MOVIE_SKIP_AGREE), votes, players);

		int const line = font->Get_Height() + 2;
		int const width = std::max(font->String_Pixel_Width(status), font->String_Pixel_Width(prompt));
		Rect const box(area.X + 4, area.Y + 4, width + 2 * BOX_PADDING, 2 * line + 2 * BOX_PADDING);
		surface.Fill_Rect_Trans(box, RGBClass(0, 0, 0), BOX_OPACITY);

		// The print takes its point relative to the clipping rectangle; the fill does not.
		Point2D at(box.X - area.X + BOX_PADDING, box.Y - area.Y + BOX_PADDING);
		Fancy_Text_Print("%s", surface, area, at, scheme, TBLACK, flags, status);
		at.Y += line;
		Fancy_Text_Print("%s", surface, area, at, scheme, TBLACK, flags, prompt);
	}
}
