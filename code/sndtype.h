/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

// Reads sound definitions from SOUND.INI into the engine's sound type. The
// grammar follows Yuri's Revenge, with defaults that keep the shipped
// Tiberian Sun files playing as they did.

#pragma once

#include "audio/audioevent.h"

class INIClass;


// The engine's defaults with the file's [Defaults] section applied over them.
void Sound_Type_Read_Defaults(INIClass const & ini, AudioEventTypeClass & defaults);

// [General] Channels=, or the fallback, clamped to what the engine can hold.
int Sound_Type_Read_Channels(INIClass const & ini, int fallback);

// Fills the type from [section], taking every key the section omits from the
// defaults. Returns false when the section is absent; the type is then the
// defaults named after the section, with the section name as its one sound.
bool Sound_Type_Fill_In(INIClass const & ini, char const * section, AudioEventTypeClass const & defaults, AudioEventTypeClass & type);

// The value parsers, exposed for the tests and the speech table.
int Sound_Type_Parse_Priority(char const * text, int fallback);
float Sound_Type_Parse_Volume(char const * text, float fallback);
unsigned Sound_Type_Parse_Type(char const * text, unsigned fallback);
unsigned Sound_Type_Parse_Control(char const * text, unsigned fallback);

// One or two numbers, in milliseconds, or seconds when written with a point.
bool Sound_Type_Parse_Delay(char const * text, int & low, int & high);

// One or two percentages. A single value spans -v..v for a pitch shift and
// -v..0 for a volume shift.
bool Sound_Type_Parse_Shift(char const * text, bool attenuate, int & low, int & high);
