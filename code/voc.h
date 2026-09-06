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

#pragma once

#include "audio/audioevent.h"
#include "audio/audiohandle.h"
#include "coord.h"

#include "voc.hh"

class CCINIClass;
class INIClass;
class SaveStreamClass;
class VocClass;

// Pan runs from -100 at the left to 100 at the right.
enum { SOUND_PAN_CENTER = 0 };

void Init_Vocs(CCINIClass const &ini);
void Free_Vocs(void);

// The value vocabulary of SOUND.INI, shared with the speech table.
int Sound_Parse_Priority(char const * text, int fallback);
float Sound_Parse_Volume(char const * text, float fallback);
unsigned Sound_Parse_Type(char const * text, unsigned fallback);
unsigned Sound_Parse_Control(char const * text, unsigned fallback);

// One or two numbers, in milliseconds, or seconds when written with a point.
bool Sound_Parse_Delay(char const * text, int & low, int & high);

// One or two percentages. A single value spans -v..v for a pitch shift and
// -v..0 for a volume shift.
bool Sound_Parse_Shift(char const * text, bool attenuate, int & low, int & high);

// A sound with no place in the world. With a handle, a live event of the same
// sound is re-aimed instead of a second one starting; one of another sound is
// stopped first.
AudioHandle Sound_Effect(VocType voc, float volume = 1.0f, int pan = SOUND_PAN_CENTER, AudioHandle * handle = nullptr);

// A sound at a place in the world, attenuated and panned by where that place
// is on screen, and kept so while it plays.
AudioHandle Sound_Effect(VocType voc, Coord const & coord, AudioHandle * handle = nullptr);

// Spoken responses; the sound effect option does not scale them.
AudioHandle Voice_Sound_Effect(VocType voc, float volume = 1.0f);

// Keeps a placed sound going while its place is in range: re-aims a live one,
// stops one that scrolled away, and starts one that is not playing. With
// start, any sound starts with its attack; without it only an endless loop
// comes back, and without its attack.
AudioHandle Play_If_In_Range(VocType voc, Coord const & coord, AudioHandle * handle, bool start = false);

// The level, 0..1, a sound of this type has at the place, and its pan.
float Calculate_Volume_And_Pan(Coord const & coord, AudioEventTypeClass const & type, int & pan);

// Once per game tick.
void Sound_Effect_AI(void);
void Stop_All_Sound_Effects(void);

// Sounds fixed at a place on the map, kept while the view scrolls over them.
// A looping one plays whenever its place is in range and travels with a save;
// a one-shot plays once if its place is in range when it is placed.
enum { STATIC_SOUND_TRIGGER = 1 };
void Static_Sound(VocType voc, Coord const & coord, int type);
void Static_Sounds_Stop(Coord const & coord, int mask);
void Static_Sounds_Serialize(SaveStreamClass & stream);

VocClass * VocClass_From_Name(char const * name);
char const * Voc_Name(VocType voc);

/***************************************************************************
**	Controls what special effects may occur on the sound effect.
*/
enum ContextType {
	IN_NOVAR,			// No variation or alterations allowed.
	IN_VAR				// Infantry variance response modification.
};

class VocClass
{
	public:
		VocClass(const char *filename);
		~VocClass(void);

		bool Fill_In(CCINIClass const &ini);

		// The engine's defaults with the file's [Defaults] section applied over them.
		static void Read_Defaults(INIClass const & ini, AudioEventTypeClass & defaults);

		// [General] Channels=, or the fallback, clamped to what the engine can hold.
		static int Read_Channels(INIClass const & ini, int fallback);

		// Fills the type from [section], taking every key the section omits from the
		// defaults. Returns false when the section is absent; the type is then the
		// defaults named after the section, with the section name as its one sound.
		static bool Read_Type(INIClass const & ini, char const * section, AudioEventTypeClass const & defaults, AudioEventTypeClass & type);

		bool Can_Play(void) const;
		AudioHandle Play(float vol, int pan = SOUND_PAN_CENTER, bool no_attack = false);
		AudioHandle Play_Voice(float vol);

		VocType Voc_Type(void);
		AudioEventTypeClass const & Type_Data(void) const { return(Type); }

		static VocType From_Name(char const * name);
		friend VocClass *VocClass_From_Name(char const * name);
		friend char const * Voc_Name(VocType voc);

		// The [Defaults] section, as read by Init_Vocs.
		static AudioEventTypeClass Defaults;

	private:
		char 				Name[256];			// Digitized voice file name.
		AudioEventTypeClass Type;
};
