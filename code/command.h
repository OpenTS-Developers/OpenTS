/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

class CommandClass
{
	public:
		virtual ~CommandClass(void) {}

		virtual char const * Get_Unique_Name(void) const = 0;
		virtual char const * Get_Display_Name(void) const { return(Get_Unique_Name()); }
		virtual char const * Get_Category(void) const = 0;
		virtual char const * Get_Description(void) const = 0;
		virtual void Execute(void) const = 0;

		// Repeatable commands re-execute every frame their bound key is held down, since
		// Windows key-repeat events never reach the game's keyboard buffer.
		virtual bool Is_Repeatable(void) const { return(false); }
};
