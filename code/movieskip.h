/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/


#pragma once

#include "rect.h"

class Surface;
struct GlobalPacketType;


/*
 * Decides when a fullscreen movie ends. In a game against other machines every player has to
 * vote with ESC before the movie stops, so the machines leave it together, and the network is
 * serviced while it plays. A Playback covers one movie from before its file is looked for until
 * it is over. While a LocalScope exists ESC ends a movie at once, for one shown after the
 * machines have stopped keeping in step.
 */
namespace MovieSkip
{
	class Playback
	{
		public:
			Playback(char const * name);
			~Playback(void);
			Playback(Playback const &) = delete;
			Playback & operator=(Playback const &) = delete;
	};

	class LocalScope
	{
		public:
			LocalScope(void);
			~LocalScope(void);
			LocalScope(LocalScope const &) = delete;
			LocalScope & operator=(LocalScope const &) = delete;
	};

	bool Idle(void);
	void Receive(int player, GlobalPacketType const & packet);
	void Draw_Overlay(Surface & surface, Rect const & area);
}
