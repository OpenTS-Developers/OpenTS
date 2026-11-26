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


/**********************************************************************
**	These enumerations are used to implement RTTI. The target system
**	uses these and thus there can be no more RTTI types than can fit
**	in the exponent of a target value.
*/
enum RTTIType {
	RTTI_NONE=0,

	RTTI_UNIT,
	RTTI_AIRCRAFT,
	RTTI_AIRCRAFTTYPE,
	RTTI_ANIM,
	RTTI_ANIMTYPE,
	RTTI_BUILDING,
	RTTI_BUILDINGTYPE,
	RTTI_BULLET,
	RTTI_BULLETTYPE,
	RTTI_CAMPAIGN,
	RTTI_CELL,
	RTTI_FACTORY,
	RTTI_HOUSE,
	RTTI_HOUSETYPE,
	RTTI_INFANTRY,
	RTTI_INFANTRYTYPE,
	RTTI_ISOTILE,
	RTTI_ISOTILETYPE,
	RTTI_LIGHT,
	RTTI_OVERLAY,
	RTTI_OVERLAYTYPE,
	RTTI_PARTICLE,
	RTTI_PARTICLETYPE,
	RTTI_PARTICLESYSTEM,
	RTTI_PARTICLESYSTEMTYPE,
	RTTI_SCRIPT,
	RTTI_SCRIPTTYPE,
	RTTI_SIDE,
	RTTI_SMUDGE,
	RTTI_SMUDGETYPE,
	RTTI_SPECIAL,
	RTTI_SUPERWEAPONTYPE,
	RTTI_TASKFORCE,
	RTTI_TEAM,
	RTTI_TEAMTYPE,
	RTTI_TERRAIN,
	RTTI_TERRAINTYPE,
	RTTI_TRIGGER,
	RTTI_TRIGGERTYPE,
	RTTI_UNITTYPE,
	RTTI_VOXELANIM,
	RTTI_VOXELANIMTYPE,
	RTTI_WAVE,
	RTTI_TAG,
	RTTI_TAGTYPE,
	RTTI_TIBERIUM,
	RTTI_ACTION,
	RTTI_EVENT,
	RTTI_WEAPONTYPE,
	RTTI_WARHEADTYPE,
	RTTI_WAYPOINT,
	RTTI_ABSTRACT,
	RTTI_TUBE,
	RTTI_LIGHTSOURCE,
	RTTI_EMPULSE,
	RTTI_TACTICALMAP,
	RTTI_SUPERWEAPON,
	RTTI_AITRIGGER,
	RTTI_AITRIGGERTYPE,
	RTTI_NEURON,				// No class uses this value; it keeps the later values in place.
	RTTI_FOGGEDOBJECT,
	RTTI_ALPHASHAPE,
	RTTI_VEINHOLEMONSTER,

	RTTI_COUNT
};
