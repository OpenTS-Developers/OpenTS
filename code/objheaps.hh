/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

// Each object and type heap that contributes to the game-state checksum; X is invoked once
// per heap with its global name. The instance list is the subset holding identified objects.
// WaypointPaths is not listed, since a path exists only on the machine whose player drew it.

#define OBJECT_HEAP_LIST(X) \
	X(HouseTypes) X(Houses) X(UnitTypes) X(Units) X(InfantryTypes) X(Infantry) \
	X(BuildingTypes) X(Buildings) X(AircraftTypes) X(Aircraft) X(AITriggerTypes) \
	X(Anims) X(AnimTypes) X(TaskForces) X(TeamTypes) X(Teams) X(ScriptTypes) X(Scripts) \
	X(TagTypes) X(Tags) X(TriggerTypes) X(Triggers) X(Actions) X(Events) X(Factories) \
	X(VoxelAnimTypes) X(VoxelAnims) X(Warheads) X(Weapons) X(ParticleTypes) X(Particles) \
	X(ParticleSystems) X(ParticleSystemTypes) X(BulletTypes) X(Bullets) \
	X(SmudgeTypes) X(OverlayTypes) X(LightSources) X(BuildingLights) X(Tubes) X(Sides) \
	X(Tiberiums) X(EMPulseClass::EMPulses) X(SuperWeaponTypes) X(SuperWeapons) \
	X(TerrainTypes) X(Terrains)

#define OBJECT_HEAP_LIST_INSTANCES(X) \
	X(Houses) X(Units) X(Infantry) X(Buildings) X(Aircraft) X(Anims) X(Teams) X(Scripts) \
	X(Tags) X(Triggers) X(Factories) X(VoxelAnims) X(Particles) X(ParticleSystems) \
	X(Bullets) X(LightSources) X(BuildingLights) X(Tubes) \
	X(EMPulseClass::EMPulses) X(SuperWeapons) X(Terrains)
