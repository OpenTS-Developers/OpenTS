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

/// TS Names and comments from FA2/FS

/****************************************************************************
**	Various trigger events and actions require additional data. This enumeration is
**	used to indicate what kind of additional data is required. This is also used
**	for team mission types that might need additional data.
*/
enum NeedType {
	NEED_NONE,					// No additional data is required.
	NEED_THEME,					// Need a musical theme.
	NEED_MOVIE,					// Need a movie to play.
	NEED_SOUND,					// Sound effect.
	NEED_SPEECH,				// Speech from EVA.
	NEED_INFANTRY,				// Infantry type class.
	NEED_UNIT,					// Unit type class.
	NEED_AIRCRAFT,				// Aircraft type class.
	NEED_STRUCTURE,				// Structure type class.
	NEED_WAYPOINT,				// Waypoint letter.
	NEED_NUMBER,				// General number.
	NEED_TRIGGER,				// Trigger object reference.
	NEED_TEAM,					// Team type class.
	NEED_HOUSE,					// House type number.
	NEED_TIME,					// Time delay value required.
	NEED_QUARRY,				// Quarry type is needed.
	NEED_BOOL,					// Boolean value is needed.
	NEED_SPECIAL,				// Special weapon ability.
	NEED_MISSION,				// General unit mission type.
	NEED_HEX_NUMBER,			// General number.
	NEED_SCRIPT,				/// Script type.
	NEED_RECT,					/// Rectangle.
	NEED_GLOBAL,				/// Global variable.
	NEED_ANIM_AND_LOCATION, 	/// Anim type class and waypoint pair.
	NEED_WEAPON_AND_LOCATION,	/// weapon type class and waypoint pair.
	NEED_LIGHT_BEHAVIOR,		/// Spotlight behavior type.
	NEED_TAG,					/// Tag type.
	NEED_METEOR_AND_LOCATION,	/// Meteor size type and waypoint pair.
	NEED_EVENT_AND_LOCATION,	/// Radar event type and waypoint pair.
	NEED_LOCAL,					/// Local variable.
	NEED_SPEED_AND_LOCATION,	/// Move camera velocity type and waypoint.
	NEED_SHOWER_AND_LOCATION,	/// Meteor shower type and waypoint pair.
	NEED_VELOCITY,				/// Move camera velocity type.
	NEED_FLOAT,					/// Floating point number.
	NEED_TEAM_AND_LOCATION,		/// Team type class and waypoint pair.
	NEED_PARTICLE_AND_LOCATION,	/// Particle type class and waypoint pair.
	NEED_BUILDING_ATTACK,		/// Building type and find target type pair.
	NEED_SPLIT,					/// Split group type.
	NEED_SOUND_AND_LOCATION,	/// Sound effect and waypoint pair.
	NEED_TEAM_AND_TIME,			/// Team type and flash count pair.
	NEED_ANIM,					/// Anim type.
	NEED_TALK_BUBBLE,			/// Unit talk bubble type.
	NEED_HOUSE_AND_CREDITS,		// House and signed credit amount pair.
	NEED_STRUCTURE_PLACEMENT,	// House, building type, force flag and waypoint.
};
