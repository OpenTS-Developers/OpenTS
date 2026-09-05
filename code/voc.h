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
class SaveStreamClass;
class VocClass;

// Pan runs from -100 at the left to 100 at the right.
enum { SOUND_PAN_CENTER = 0 };

void Init_Vocs(CCINIClass const &ini);
void Free_Vocs(void);

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
// restarts an endless loop that ended, and stops one that scrolled away.
AudioHandle Play_If_In_Range(VocType voc, Coord const & coord, AudioHandle * handle);

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

		bool Can_Play(void) const;
		AudioHandle Play(float vol, int pan = SOUND_PAN_CENTER);
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
