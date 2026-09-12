/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "empulse.h"

#include "_map.h"
#include "_rules.h"
#include "aircraft.h"
#include "anim.h"
#include "building.h"
#include "ccrand.h"
#include "cell.h"
#include "crc.h"
#include "globals.h"
#include "house.h"
#include "infantry.h"
#include "infatype.h"
#include "mouse.h"
#include "rules.h"
#include "savestream.h"
#include "sun.h"
#include "tag.h"
#include "tracker.h"
#include "unit.h"
#include "unittype.h"

DynamicVectorClass<EMPulseClass *> EMPulseClass::EMPulses;


/// <summary>
/// Creates an electromagnetic pulse over the map.
/// This routine is used when a warhead with the electromagnetic effect detonates, and by
/// the trigger action that fires the pulse weapon off. The new pulse adds itself to the
/// global pulse list and immediately disrupts everything standing within its radius.
/// </summary>
/// <param name="cell">The cell that the pulse is centered upon.</param>
/// <param name="spread">The radius of the pulse, expressed in cells.</param>
/// <param name="duration">The number of game frames the pulse will remain in effect.</param>
/// <param name="source">The object that will be credited with the pulse's handiwork.</param>
EMPulseClass::EMPulseClass(Cell cell, int spread, int duration, TechnoClass * source) :
	BASECLASS(),
	CellID(cell),
	Spread(spread),
	CreationFrame(Frame),
	Duration(duration)
{
	Create_ID();
	EMPulses.Add(this);
	Create(source);
}


/// <summary>
/// Creates a blank pulse.
/// This routine is used by the save/load system, which needs an empty object to read the
/// saved state into.
/// </summary>
EMPulseClass::EMPulseClass(void) :
	BASECLASS(),
	CellID(CELL_NONE),
	Spread(0),
	CreationFrame(0),
	Duration(0)
{

}


/// <summary>
/// Destroys the pulse.
/// This routine lifts the pulse's effect off the map, breaks any references other objects
/// hold to it, and takes it out of the global pulse list.
/// </summary>
EMPulseClass::~EMPulseClass(void)
{
	if (GameActive) {
		Destroy();
	}
	Detach_This_From_All(this, true);
	EMPulses.Delete(this);
}


/// <summary>
/// Deletes every pulse in the game.
/// This routine is used when the scenario is torn down, so that no pulse survives into the
/// next game.
/// </summary>
void EMPulseClass::Reset(void)
{
	while (EMPulses.Count()) {
		delete EMPulses[0];
	}
}


/// <summary>
/// Deletes any pulses that have outlived their duration.
/// This routine is called by the game logic so that an expired pulse releases its hold on
/// the objects and cells it was disrupting.
/// </summary>
void EMPulseClass::Update_All(void)
{
	for (int i = EMPulses.Count() - 1; i >= 0; i--) {
		EMPulseClass * pulse = EMPulses[i];
		if (Frame >= pulse->CreationFrame + pulse->Duration) {
			delete pulse;
		}
	}
}


/// <summary>
/// Applies the pulse's effect to everything within its radius.
/// Aircraft that are aloft are brought down, subterranean and surface units are stunned
/// and left sparking, and buildings are powered off for as long as the pulse lasts. Core
/// defenders and limpet mines have their own reactions, and everything caught is given the
/// chance to spring a paralyzed trigger event.
/// </summary>
/// <param name="source">The object that will be credited with the pulse's handiwork.</param>
void EMPulseClass::Create(TechnoClass * source)
{
	if (Duration + CreationFrame > Frame) {
		int spread_sq = Spread * Spread;

		for (int i = 0; i < Aircraft.Count(); i++) {
			AircraftClass * aircraft = Aircraft[i];
			if (aircraft->IsDown && !aircraft->IsInLimbo && !aircraft->In_Air() && aircraft->Strength > 0) {
				if (aircraft->Center_Coord().Distance_To(CellID.As_Coord()) < Spread * CELL_LEPTON) {
					aircraft->Spring_Tag(TEVENT_PARALYZED, aircraft, CELL_NONE, false, source);
					aircraft->Crash(source);
				}
			}
		}

		for (int j = 0; j < DisplayClass::Layer[LAYER_UNDERGROUND].Count(); j++) {
			FootClass * foot = (FootClass *)DisplayClass::Layer[LAYER_UNDERGROUND][j];
			Cell center = foot->PositionCoord;
			int x = center.X - CellID.X;
			int y = center.Y - CellID.Y;
			if (x * x + y * y < spread_sq) {
				foot->Locomotion->Power_Off();
				if (foot->Locomotion->Is_Moving()) {
					foot->Locomotion->Stop_Moving();
				}
				foot->StunDuration = Duration;
				AnimClass * sparks = new AnimClass(Rule->EMPulseSparkles, foot->Center_Coord(), Random_Pick(0, 25));
				if (sparks != NULL) {
					sparks->Attach_To(foot);
				}
				foot->Spring_Tag(TEVENT_PARALYZED, foot, CELL_NONE, false, source);
			}
		}

		for (int y = -Spread; y <= Spread; y++) {
			for (int x = -Spread; x <= Spread; x++) {
				Cell cell = Cell(x, y) + CellID;
				if (cell.X >= 0 && cell.X < MAP_CELL_W && cell.Y >= 0 && cell.Y < MAP_CELL_H) {
					if (Map.Is_Valid(cell)) {
						int dx = cell.X - CellID.X;
						int dy = cell.Y - CellID.Y;
						if (dx * dx + dy * dy <= spread_sq) {
							CellClass & cellptr = Map[cell];

							BuildingClass * building = cellptr.Cell_Building();
							if (building != NULL) {
								if (building->Center_Coord().As_Cell() == cell) {
									if (!building->Class->IsInvisibleInGame) {
										if (building->Class->IsLimpetMine == true) {
											building->Do_Destruction(NULL, source, true, building->Occupy_List());
										} else if (!building->Class->IsCoreDefender) {
											building->Power_Off();
											building->StunDuration = Duration;
											if (building->Class->IsRadar) {
												building->House->RecalcRadar = true;
											}
											if (building->Class->Is_Mobile_Deployer()) {
												Coord coord = building->Center_Coord();
												coord.X += CELL_LEPTON_W / 4;
												coord.Y += CELL_LEPTON_H / 4;
												coord.Z += LEVEL_LEPTON_H / 2;
												AnimClass * sparks = new AnimClass(Rule->EMPulseSparkles, coord, Random_Pick(0, 25));
												if (sparks != NULL) {
													sparks->Attach_To(building);
												}
											}
										}
										building->Spring_Tag(TEVENT_PARALYZED, building, CELL_NONE, false, source);
									}
								}
							} else {
								TechnoClass * techno = cellptr.Cell_Techno();
								while (techno != NULL) {
									bool paralyze = false;
									if ((techno->RTTI == RTTI_UNIT || techno->RTTI == RTTI_AIRCRAFT) && techno->Is_Foot()) {
										if (((FootClass *)techno)->Locomotion != NULL) {
											UnitClass * unit = dynamic_cast<UnitClass *>(techno);
											if (unit != NULL && unit->Class->IsCoreDefender) {
												paralyze = false;
												techno->Spring_Tag(TEVENT_PARALYZED, unit, CELL_NONE, false, source);
											} else {
												paralyze = techno != source;
											}
										}
									}
									if (techno->RTTI == RTTI_INFANTRY && ((InfantryClass *)techno)->Class->IsCyborg || paralyze) {
										if (techno->RTTI != RTTI_UNIT || (!((UnitClass *)techno)->Class->IsLargeVisceroid && !((UnitClass *)techno)->Class->IsSmallVisceroid)) {
											FootClass * foot = ((FootClass *) techno);
											foot->Locomotion->Power_Off();
											if (foot->Locomotion->Is_Moving()) {
												foot->Locomotion->Stop_Moving();
											}
											foot->StunDuration = Duration;
											AnimClass * sparks = new AnimClass(Rule->EMPulseSparkles, foot->Center_Coord(), Random_Pick(0, 25));
											if (sparks != NULL) {
												sparks->Attach_To(foot);
											}
											foot->Spring_Tag(TEVENT_PARALYZED, foot, CELL_NONE, false, source);
										}
									}
									if (techno->Next == NULL || !techno->Next->Is_Techno()) break;
									techno = (TechnoClass *)techno->Next;
								}
							}
						}
					}
				}
			}
		}
	}
}


/// <summary>
/// Lifts the pulse's disruption from the cells it covers.
/// This routine is used when the pulse expires, so that the cells within its radius stop
/// reporting themselves as being under an electromagnetic pulse.
/// </summary>
void EMPulseClass::Destroy(void)
{
	int spread_sq = Spread * Spread;
	for (int y = -Spread; y <= Spread; y++) {
		for (int x = -Spread; x <= Spread; x++) {
			Cell cell = Cell(x, y) + CellID;
			if (cell.X >= 0 && cell.X < MAP_CELL_W && cell.Y >= 0 && cell.Y < MAP_CELL_H) {
				if (Map.Is_Valid(cell)) {
					int dx = cell.X - CellID.X;
					int dy = cell.Y - CellID.Y;
					if (dx * dx + dy * dy <= spread_sq) {
						CellClass & cellptr = Map[cell];
						cellptr.IsAffectedByEMP = false;
					}
				}
			}
		}
	}
}


/// <summary>
/// Adds the state of this pulse to the running game checksum.
/// This routine is used by the multiplayer sync check to prove that every machine holds
/// an identical copy of this object.
/// </summary>
/// <param name="crc">The checksum engine to submit the object state to.</param>
void EMPulseClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(CellID.X);
	crc(CellID.Y);
	crc(Spread);
	crc(CreationFrame);
	crc(Duration);
}


ClassID EMPulseClass::Class_ID(void) const
{
	return(ClassID_EMPulseClass);
}


/// <summary>
/// Lists the members this pulse carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void EMPulseClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// EMPulses -- the master list, which each pulse joins as it is constructed.
	stream.Serialize(CellID);
	stream.Serialize(Spread);
	stream.Serialize(CreationFrame);
	stream.Serialize(Duration);
}
