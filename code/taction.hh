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

/// TS names and comments derived from FS/FA2.

enum TActionType {
	TACTION_NONE,

	TACTION_WIN,							// player wins!
	TACTION_LOSE,							// player loses.
	TACTION_BEGIN_PRODUCTION,				// computer begins factory production.
	TACTION_CREATE_TEAM,					// computer creates a certain type of team
	TACTION_DESTROY_TEAM,					/// Destroy all teams of the type specified.
	TACTION_ALL_HUNT,						// all enemy units go into hunt mode (teams destroyed).
	TACTION_REINFORCEMENTS,					// player gets reinforcements (house that gets them is determined by the Reinforcement instance)
	TACTION_DZ,								// Deploy drop zone smoke.
	TACTION_FIRE_SALE,						// Sell all buildings and go on rampage.
	TACTION_PLAY_MOVIE,						// Play movie (temporarily suspend game).
	TACTION_TEXT_TRIGGER,					// Triggers a text message display.
	TACTION_DESTROY_TRIGGER,				// Destroy specified trigger.
	TACTION_AUTOCREATE,						// Computer to autocreate teams.
	TACTION_CHANGE_HOUSE,					/// Changes owning house to the one specified.
	TACTION_ALLOWWIN,						// Allows winning if triggered.

	TACTION_REVEAL_ALL,						// Reveal the entire map.
	TACTION_REVEAL_SOME,					// Reveal map around cell #.
	TACTION_REVEAL_ZONE,					// Reveal all of specified zone.
	TACTION_PLAY_SOUND,						// Play sound effect.
	TACTION_PLAY_MUSIC,						// Play musical score.
	TACTION_PLAY_SPEECH,					// Play EVA speech.
	TACTION_FORCE_TRIGGER,					// Force trigger to activate.
	TACTION_START_TIMER,					// Start mission timer.
	TACTION_STOP_TIMER,						// Stop mission timer.
	TACTION_ADD_TIMER,						// Increase mission timer time.
	TACTION_SUB_TIMER,						// Decrease mission timer time.
	TACTION_SET_TIMER,						// Set and start the mission timer.
	TACTION_SET_GLOBAL,						// Set global variable.
	TACTION_CLEAR_GLOBAL,					// Clear global variable.
	TACTION_BASE_BUILDING,					// Automated base building.
	TACTION_CREEP_SHADOW,					// Shadow grows back one 'step'.

	TACTION_DESTROY_OBJECT,					// Destroys the building this trigger is attached to.
	TACTION_1_SPECIAL,						// Add a one-time special weapon ability to house.
	TACTION_FULL_SPECIAL,					// Add a repeating special weapon ability to house.

	TACTION_PREFERRED_TARGET,				// Designates preferred target for house.

	TACTION_ALL_CHANGE_HOUSE,				/// All objects of one house change ownership.
	TACTION_MAKE_ALLY,						/// Cause this house to ally with another.
	TACTION_MAKE_ENEMY,						/// Cause this house to declare war on another.
	TACTION_CHANGE_ZOOM,					/// Change the radar map zoom level.
	TACTION_RESIZE_PLAYER_VIEW,				/// Resize the player's viewing rectangle.
	TACTION_PLAY_ANIM,						/// Play animation at specified cell.
	TACTION_DO_EXPLOSION,					/// Create explosion at specified cell.
	TACTION_METEOR_IMPACT,					/// Send a meteor at the specified cell.
	TACTION_ION_STORM_START,				/// Start an ion storm.
	TACTION_ION_STORM_STOP,					/// Stop the ion storm in progress.
	TACTION_LOCK_INPUT,						/// Disable user input.
	TACTION_UNLOCK_INPUT,					/// Enable user input.
	TACTION_CENTER_VIEWPOINT,				/// Move tactical view to waypoint.
	TACTION_ZOOM_IN,						/// Zoom the tactical map in.
	TACTION_ZOOM_OUT,						/// Zoom the tactical map out.
	TACTION_RESHROUD,						/// Reshroud the entire map.
	TACTION_CHANGE_SPOTLIGHT_BEHAVIOR,		/// Change attached building's spotlight behavior.
	TACTION_ENABLE_TRIGGER,					/// Enable the specified trigger.
	TACTION_DISABLE_TRIGGER,				/// Disable the specified trigger.
	TACTION_RADAR_EVENT,					/// Create radar event at waypoint.
	TACTION_SET_LOCAL,						/// Set local variable.
	TACTION_CLEAR_LOCAL,					/// Clear local variable.
	TACTION_METEOR_SHOWER,					/// Create meteor shower around waypoint.
	TACTION_REDUCE_TIBERIUM,				/// Reduce Tiberium around waypoint.
	TACTION_SELL_ATTACHED,					/// Sell building attached to this trigger.
	TACTION_TURN_OFF_ATTACHED,				/// Turn off attached building.
	TACTION_TURN_ON_ATTACHED,				/// Turn on attached building.
	TACTION_DAMAGE,							/// Apply 100 points of HE damage at location.
	TACTION_LIGHT_SMALL,					/// Small light flash at location.
	TACTION_LIGHT_MEDIUM,					/// Medium light flash at location.
	TACTION_LIGHT_LARGE,					/// Large light flash at location.
	TACTION_ANNOUNCE_WIN,					/// Announce that player has won.
	TACTION_ANNOUNCE_LOSE,					/// Announce that player has lost.
	TACTION_FORCE_END,						/// Force end of scenario.
	TACTION_DESTROY_TAG,					/// Destroy tag and all attached triggers.
	TACTION_SET_AMBIENT_STEP,				/// Set ambient light fade step value.
	TACTION_SET_AMBIENT_RATE,				/// Set ambient light fade rate.
	TACTION_SET_AMBIENT_LIGHT,				/// Fade ambient light to new level.
	TACTION_BEGIN_AI_TRIGGERS,				/// Start AI triggers for specified house.
	TACTION_STOP_AI_TRIGGERS,				/// Stop AI triggers for specified house.
	TACTION_SET_AI_TRIGGER_TEAM_RATIO,		/// Percentage of teams created for AI triggers.
	TACTION_SET_TEAM_AIRCRAFT_RATIO,		/// Percentage of aircraft created for teams.
	TACTION_SET_TEAM_INFANTRY_RATIO,		/// Percentage of infantry created for teams.
	TACTION_SET_TEAM_UNIT_RATIO,			/// Percentage of units created for teams.
	TACTION_REINFORCEMENTS_SPECIAL,			/// Create reinforcement team at special waypoint.
	TACTION_WAKEUP_SELF,					/// Break out of sleep or harmless mode.
	TACTION_WAKEUP_ALL_SLEEP,				/// Break all units out of sleep mode.
	TACTION_WAKEUP_ALL_HARMLESS,			/// Break all units out of harmless mode.
	TACTION_WAKEUP_GROUP,					/// Wake up all units of specified group.
	TACTION_VEIN_GROWTH,					/// Control if veins grow or not.
	TACTION_TIB_GROWTH,						/// Control if Tiberium grows or not.
	TACTION_ICE_GROWTH,						/// Control if ice grows or not.
	TACTION_PARTICLE_ANIM,					/// Show particle animation at location.
	TACTION_REMOVE_PARTICLE_ANIM,			/// Delete particle anims at location.
	TACTION_ION_LIGHTNING_STRIKE,			/// Single ion storm lightning strike.
	TACTION_GO_BERZERK,						/// Attached cyborg goes berzerk.
	TACTION_ACTIVATE_FIRESTORM,				/// Turn on house's firestorm defense.
	TACTION_DEACTIVATE_FIRESTORM,			/// Turn off house's firestorm defense.
	TACTION_ION_CANNON,						/// Fire Ion Cannon at waypoint.
	TACTION_MULTI_MISSILE,					/// Fire nuke at waypoint from nearest edge.
	TACTION_CHEM_MISSILE,					/// Fire chemical missile at waypoint.
	TACTION_TOGGLE_TRAIN_CARGO,				/// Toggle cargo train dropping crate.
	TACTION_PLAY_SOUND_RANDOM,				/// Play sound effect at random waypoint.
	TACTION_PLAY_SOUND_AT,					/// Play sound effect at waypoint.
	TACTION_PLAY_INGAME_MOVIE,				/// Play movie ingame without pausing.
	TACTION_FLASH_TEAM,						/// Flash team for specified number of frames.
	TACTION_DISABLE_SPEECH,					/// Disable EVA speech.
	TACTION_ENABLE_SPEECH,					/// Enable EVA speech.
	TACTION_SET_GROUP_ID,					/// Set group ID of attached object.
	TACTION_TALK_BUBBLE,					/// Display talk bubble over unit.
	TACTION_GIVE_CREDITS,					// Give or take credits from a house.
	TACTION_ENABLE_SHORT_GAME,				// Turn the short game rule on.
	TACTION_DISABLE_SHORT_GAME,				// Turn the short game rule off.
	TACTION_CREATE_BUILDING_AT,				// Place a building at a waypoint.
	TACTION_HOUSE_DESTROY_ALL,				// Destroy everything a house owns and defeat it.
	TACTION_MAKE_ELITE,						// Promote attached objects to elite.
	TACTION_ENABLE_ALLY_REVEAL,				// Let allies see what each other reveals.
	TACTION_DISABLE_ALLY_REVEAL,			// Stop allies seeing what each other reveals.
	TACTION_CREATE_AUTOSAVE,				// Save the game once the frame has finished.
	TACTION_DELETE_OBJECT,					// Remove attached objects without destroying them.
	TACTION_ALL_ASSIGN_MISSION,				// Give every foot unit of the house a mission.
	TACTION_MAKE_ALLY_ONE_WAY,				// Ally with a house without the reverse alliance.
	TACTION_MAKE_ENEMY_ONE_WAY,				// Declare war on a house.

	TACTION_COUNT,
	TACTION_FIRST=0
};

inline TActionType operator++(TActionType & val)
{
	val = TActionType(int(val) + 1);
	assert(val >= TACTION_FIRST && val <= TACTION_COUNT);
	return(val);
}
