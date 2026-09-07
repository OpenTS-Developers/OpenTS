/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "tiberium.h"

#include "_map.h"
#include "animtype.h"
#include "astar.h"
#include "ccini.h"
#include "cell.h"
#include "findmake.h"
#include "globals.h"
#include "incdec.h"
#include "overtype.h"
#include "priority.h"
#include "savestream.h"
#include "scenario.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"

#include <algorithm>

#define MAX_SPREAD_DELAY	50
#define MAX_GROWTH_DELAY	50
#define MAX_GROWTH_STAGE	11

DynamicVectorClass<TiberiumClass *> Tiberiums;


/// <summary>
/// Creates a tiberium type.
/// This routine builds a tiberium type and adds it to the master tiberium list. The
/// place it lands at in that list becomes the heap identifier the cells refer to it by.
/// </summary>
/// <param name="ininame">The rules section name this tiberium type is described by.</param>
TiberiumClass::TiberiumClass(char const * ininame) :
	BASECLASS(ininame),
	HeapID(TIBERIUM_NONE),
	SpreadPercentage(0.1),
	GrowthPercentage(0.1),
	SpreadDelay(0),
	GrowthDelay(0),
	CreditValue(0),
	Power(0),
	Color(0),
	Debris(),
	Overlay(NULL),
	FrameCount(0),
	Variety(0),
	RampVariety(0),
	SpreadCount(0),
	SpreadQueue(NULL),
	SpreadState(NULL),
	SpreadNodes(NULL),
	SpreadTimer(),
	GrowthCount(0),
	GrowthQueue(NULL),
	GrowthState(NULL),
	GrowthNodes(NULL),
	GrowthTimer()
{
	HeapID = (TiberiumType)Tiberiums.Count();
	Tiberiums.Add(this);
	Debris.Clear();
	AbstractTypePtrTracker.Add(this);
}


/// <summary>
/// Destroys the tiberium type.
/// This routine releases the spread and growth tracking data and takes the tiberium type
/// back out of the master tiberium list.
/// </summary>
TiberiumClass::~TiberiumClass(void)
{
	Detach_This_From_All(this);
	AbstractTypePtrTracker.Delete(this);
	Tiberiums.Delete(this);

	Clear_Spread();
	Clear_Growth();
}


/// <summary>
/// Fetches this tiberium type's settings from the rules.
/// This routine reads the spread and growth behavior, the harvest value and power
/// rating, and the artwork the tiberium is drawn and shattered with.
/// </summary>
/// <returns>bool; Was this tiberium type present in the database?</returns>
bool TiberiumClass::Read_INI(CCINIClass const & ini)
{
	if (BASECLASS::Read_INI(ini)) {

		SpreadDelay = ini.Get_Int(IniName, "Spread", SpreadDelay);
		SpreadPercentage = ini.Get_Float(IniName, "SpreadPercentage", SpreadPercentage);
		GrowthDelay = ini.Get_Int(IniName, "Growth", GrowthDelay);
		GrowthPercentage = ini.Get_Float(IniName, "GrowthPercentage", GrowthPercentage);
		CreditValue = ini.Get_Int(IniName, "Value", CreditValue);
		Power = ini.Get_Int(IniName, "Power", Power);
		Color = ini.Get_Scheme_Index(IniName, "Color", Color);
		Debris = TGet_TypeList<AnimTypeClass>(ini, IniName, "Debris", Debris);

		switch (ini.Get_Int(IniName, "Image", -1)) {
			case -1:
				break;

			case 2:
				Overlay = OverlayTypes[OVERLAY_LARGE_TIBERIUM01];
				FrameCount = 1;
				Variety = 12;
				break;

			case 3:
				Overlay = OverlayTypes[OVERLAY_TIBERIUM2_01];
				RampVariety = 8;
				FrameCount = 12;
				Variety = 12;
				break;

			case 4:
				Overlay = OverlayTypes[OVERLAY_TIBERIUM3_01];
				RampVariety = 8;
				FrameCount = 12;
				Variety = 12;
				break;

			case 1:
			default:
				Overlay = OverlayTypes[OVERLAY_TIBERIUM01];
				RampVariety = 8;
				FrameCount = 12;
				Variety = 12;
				break;
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Creates the tiberium types the rules ask for.
/// This routine walks the list of tiberium names in the rules and either updates the
/// matching tiberium type or creates a fresh one for each name it finds there.
/// </summary>
bool TiberiumClass::Process(CCINIClass const & ini)
{
	char buffer[24];
	int count = ini.Entry_Count("Tiberiums");
	for (int i = 0; i < count; i++) {
		char const * index = ini.Get_Entry("Tiberiums", i);
		if (ini.Get_String("Tiberiums", index, "", buffer, sizeof(buffer)) > 0) {
			int tibindex = atoi(index);
			TiberiumClass * tib;
			if (tibindex < Tiberiums.Count()) {
				tib = Tiberiums[tibindex];
			} else {
				tib = new TiberiumClass(buffer);
			}
			tib->Read_INI(ini);
		}
	}
	return(true);
}


/// <summary>
/// Adds this tiberium type to the game state checksum.
/// This routine is used by the network sync check to prove that every machine agrees on
/// the tiberium rules in force.
/// </summary>
void TiberiumClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(SpreadDelay);
	crc(GrowthDelay);
	crc(CreditValue);
	crc(Power);
	crc(Color);
	crc(FrameCount);
	crc(Variety);
}


ClassID TiberiumClass::Class_ID(void) const
{
	return(ClassID_TiberiumClass);
}


/// <summary>
/// Loads this tiberium type from a stream.
/// The spread and growth pools are dropped before the members arrive, since the counts
/// they track are about to be replaced with the saved ones.
/// </summary>
/// <returns>bool; Was the record read whole?</returns>
/// <remarks>The spread and growth systems are not saved, so they come back empty. They
/// must be rebuilt once the game has finished loading.</remarks>
bool TiberiumClass::Load(SaveStreamClass & stream)
{
	Clear_Spread();
	Clear_Growth();

	return(Load_Members(stream));
}


/// <summary>
/// Lists the members this tiberium type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void TiberiumClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(SpreadDelay);
	stream.Serialize(SpreadPercentage);
	stream.Serialize(GrowthDelay);
	stream.Serialize(GrowthPercentage);
	stream.Serialize(CreditValue);
	stream.Serialize(Power);
	stream.Serialize(Color);
	stream.Serialize(Debris);
	stream.Serialize(Overlay);
	stream.Serialize(FrameCount);
	stream.Serialize(Variety);
	stream.Serialize(RampVariety);
	stream.Serialize(SpreadCount);
	// SpreadQueue -- pools sized to the map rather than saved state; Load drops them and the
	// tiberium systems build them again from the map itself.
	// SpreadState
	// SpreadNodes
	stream.Serialize(SpreadTimer);
	stream.Serialize(GrowthCount);
	// GrowthQueue -- the growth pools, dropped and rebuilt the same way.
	// GrowthState
	// GrowthNodes
	stream.Serialize(GrowthTimer);
}


/// <summary>
/// Removes any reference this tiberium type holds to the specified object.
/// This routine is called when an object is about to leave the game, so that the
/// tiberium type is not left pointing at artwork that no longer exists.
/// </summary>
void TiberiumClass::Detach(AbstractClass const * target, bool all)
{
	for (int i = Debris.Count() -1; i >= 0; i--) {
		if (Debris[i] == target) {
			Debris.Delete_Index(i);
		}
	}

	if (Overlay == target) {
		Overlay = NULL;
	}
}


/// <summary>
/// Handles the spreading logic for every tiberium type.
/// This routine is called from the main game logic and gives each tiberium type a spread
/// pass whenever its spread delay has run out. Scenarios that have tiberium growth
/// switched off are skipped entirely.
/// </summary>
void TiberiumClass::Tiberium_Spread(void)
{
	if (Scen->IsTibGrowth) {
		for (int i = 0; i < Tiberiums.Count(); i++) {
			if (!Tiberiums[i]->SpreadTimer) {
				Tiberiums[i]->Spread_AI();
				Tiberiums[i]->SpreadTimer = Tiberiums[i]->SpreadDelay;
			}
		}
	}
}


/// <summary>
/// Starts the tiberium spread system up.
/// This routine is called once the map is ready, so that every tiberium type can build
/// a spread queue for the cells it finds there.
/// </summary>
void TiberiumClass::Init_Tiberium_Spread_System(void)
{
	for (int i = 0; i < Tiberiums.Count(); i++)	{
		Tiberiums[i]->Init_Spread();
	}
}


/// <summary>
/// Shuts the tiberium spread system down.
/// This routine is called when a scenario ends, releasing the spread tracking data held
/// by every tiberium type.
/// </summary>
void TiberiumClass::Deinit_Tiberium_Spread_System(void)
{
	for (int i = 0; i < Tiberiums.Count(); i++)	{
		Tiberiums[i]->Clear_Spread();
	}
}


/// <summary>
/// Handles the spreading logic for this tiberium type.
/// This routine seeds new tiberium from a share of the enqueued cells, keeping any cell
/// that still has room around it in the queue for a later pass. The share taken is
/// randomized so that the spreading does not look mechanical.
/// </summary>
void TiberiumClass::Spread_AI(void)
{
	if (SpreadQueue && SpreadQueue->Count() && SpreadPercentage > 0.00001) {

		/*
		 * The amount we spread depends on how many spreads are enqueued.
		 * Randomize it so that it feels more natural.
		 */
		int count = std::min(25, std::max(5, (int)(SpreadQueue->Count() * SpreadPercentage)));
		count = (abs(Scen->RandomNumber()) % count) + 1;

		/*
		 * SpreadQueue does not recycle its entries.
		 * When space runs low, we need to clear and recalculate it.
		 */
		if (SpreadQueue->Count() > Map_Cell_Count() - 20) {
			Recalc_Spread();
		}

		int index = 0;
		CellNode * node = SpreadQueue->Extract_Min();

		while (index < count && node != NULL) {
			CellClass * cellptr = &Map[node->Element];
			int possible_spreads = 0;

			/*
			 * Count how many neighbors we can spread Tiberium to.
			 */
			for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
				CellClass & adjacent = cellptr->Adjacent_Cell(facing);
				if (adjacent.Can_Tiberium_Germinate(NULL)) {
					possible_spreads++;
				}
			}

			if (possible_spreads != 0) {
				cellptr->Spread_Tiberium();
				index++;

				/*
				 * If there's more than one possibility, then re-enqueue this cell to spread again later.
				 */
				if (possible_spreads > 1) {
					SpreadNodes[SpreadCount].Element = cellptr->CellID;
					SpreadNodes[SpreadCount].Score = 0;
					SpreadQueue->Insert(SpreadNodes[SpreadCount++]);
					SpreadState[Map_Cell_Index(cellptr->CellID)] = true;
				}
			} else {
				SpreadState[Map_Cell_Index(cellptr->CellID)] = false;
			}

			if (index < count) {
				node = SpreadQueue->Extract_Min();
			}
		}
	}
}


/// <summary>
/// Allocates the tiberium spread tracking data.
/// This routine sets the spread system up for the current map and primes it with every
/// cell that is able to spread. Any previous spread data is discarded first.
/// </summary>
void TiberiumClass::Init_Spread(void)
{
	Clear_Spread();

	SpreadNodes = new CellNode[Map_Cell_Count()];
	SpreadState = new bool [Map_Cell_Count()];
	SpreadQueue = new PriorityQueueClass<CellNode>(Map_Cell_Count());

	Recalc_Spread();
}


/// <summary>
/// Rebuilds the tiberium spread queue from the map.
/// This routine is used when the queue has run out of spare entries. Every cell of this
/// tiberium that is able to spread is enqueued afresh.
/// </summary>
void TiberiumClass::Recalc_Spread(void)
{
	SpreadCount = 0;
	SpreadQueue->Clear();

	for (int i = Map_Cell_Count() - 1; i >= 0; i--) {
		SpreadState[i] = false;
	}

	Map.Reset_Iterator();
	CellClass * iter = Map.Iterate();
	while (iter) {

		if (iter->Tiberium_Type_Here() == HeapID && iter->Can_Tiberium_Spread()) {
			SpreadNodes[SpreadCount].Element = iter->CellID;
			SpreadNodes[SpreadCount].Score = 0.0;
			SpreadQueue->Insert(SpreadNodes[SpreadCount++]);
			SpreadState[Map_Cell_Index(iter->CellID)] = true;
		}

		iter = Map.Iterate();
	}
}


/// <summary>
/// Frees the tiberium spread tracking data.
/// This routine tears down the spread queue and its bookkeeping, leaving the spread
/// system idle until Init_Spread builds it again.
/// </summary>
void TiberiumClass::Clear_Spread(void)
{
	if (SpreadQueue) {
		SpreadQueue->Clear();
		delete SpreadQueue;
		SpreadQueue = NULL;
	}

	if (SpreadNodes) {
		delete [] SpreadNodes;
		SpreadNodes = NULL;
	}

	if (SpreadState) {
		delete [] SpreadState;
		SpreadState = NULL;
	}

	SpreadCount = 0;
}


/// <summary>
/// Clears the pending spread record for a cell.
/// This routine is called when the contents of a cell change, so that every tiberium
/// type will consider the cell afresh the next time it is offered for spreading.
/// </summary>
void TiberiumClass::Clear_Spread_State(Cell const & cell)
{
	int cellindex = Map_Cell_Index(cell);
	for (int i = 0; i < Tiberiums.Count(); i++) {
		Tiberiums[i]->SpreadState[cellindex] = false;
	}
}


/// <summary>
/// Queues a cell for tiberium spreading.
/// This routine is used when a cell becomes ripe enough to seed its neighbors, so that
/// the spread system will visit it on a later pass.
/// </summary>
void TiberiumClass::Queue_Spread(Cell const & cell)
{
	int cellindex = Map_Cell_Index(cell);
	if (Map[cell].Can_Tiberium_Spread() && !SpreadState[cellindex]) {

		/*
		 * SpreadQueue does not recycle its entries.
		 * When space runs low, we need to clear and recalculate it.
		 */
		if (SpreadCount >= Map_Cell_Count() - 20) {
			Recalc_Spread();
		}

		SpreadNodes[SpreadCount].Element = cell;
		SpreadNodes[SpreadCount].Score = float(Frame + abs(Scen->RandomNumber()) % MAX_SPREAD_DELAY);
		SpreadQueue->Insert(SpreadNodes[SpreadCount++]);
		SpreadState[Map_Cell_Index(cell)] = true;
	}
}


/// <summary>
/// Handles the growth logic for every tiberium type.
/// This routine is called from the main game logic and gives each tiberium type a growth
/// pass whenever its growth delay has run out. Scenarios that have tiberium growth
/// switched off are skipped entirely.
/// </summary>
void TiberiumClass::Tiberium_Growth(void)
{
	if (Scen->IsTibGrowth) {
		for (int i = 0; i < Tiberiums.Count(); i++) {
			if (!Tiberiums[i]->GrowthTimer) {
				Tiberiums[i]->Growth_AI();
				Tiberiums[i]->GrowthTimer = Tiberiums[i]->GrowthDelay * (Scen->Special.IsTGrowth ? 0.3 : 1.0);
			}
		}
	}
}


/// <summary>
/// Starts the tiberium growth system up.
/// This routine is called once the map is ready, so that every tiberium type can build
/// a growth queue for the cells it finds there.
/// </summary>
void TiberiumClass::Init_Tiberium_Growth_System(void)
{
	for (int i = 0; i < Tiberiums.Count(); i++)	{
		Tiberiums[i]->Init_Growth();
	}
}


/// <summary>
/// Shuts the tiberium growth system down.
/// This routine is called when a scenario ends, releasing the growth tracking data held
/// by every tiberium type.
/// </summary>
void TiberiumClass::Deinit_Tiberium_Growth_System(void)
{
	for (int i = 0; i < Tiberiums.Count(); i++)	{
		Tiberiums[i]->Clear_Growth();
	}
}


/// <summary>
/// Handles the growth logic for this tiberium type.
/// This routine ripens a share of the enqueued cells, keeping any cell that has not
/// reached full bloom in the queue and offering it to the spread system as it matures.
/// The share taken is randomized so that the growth does not look mechanical.
/// </summary>
void TiberiumClass::Growth_AI(void)
{
	if (GrowthQueue && GrowthQueue->Count() && GrowthPercentage > 0.00001) {

		/*
		 * The amount we grow depends on how many growths are enqueued.
		 * Randomize it so that it feels more natural.
		 */
		int count = std::min(50, std::max(5, (int)(GrowthQueue->Count() * GrowthPercentage)));
		count = (abs(Scen->RandomNumber()) % count) + 1;

		/*
		 * GrowthQueue does not recycle its entries.
		 * When space runs low, we need to clear and recalculate it.
		 */
		if (GrowthQueue->Count() > Map_Cell_Count() - 2 * count) {
			Recalc_Growth();
		}

		int index = 0;
		CellNode * node = GrowthQueue->Extract_Min();

		while (index < count && node != NULL) {
			CellClass * cellptr = &Map[node->Element];

			if (cellptr->Tiberium_Type_Here() == HeapID) {
				cellptr->Grow_Tiberium();

				/*
				 * If we haven't reached the max growth stage yet, then re-enqueue this cell to grow again later.
				 * Also, take this opportunity to queue this cell to spread, if possible.
				 */
				if (cellptr->OverlayData < MAX_GROWTH_STAGE) {
					GrowthNodes[GrowthCount].Element = node->Element;
					GrowthNodes[GrowthCount].Score = float(Frame + abs(Scen->RandomNumber() % MAX_GROWTH_DELAY));
					GrowthState[Map_Cell_Index(node->Element)] = true;
					GrowthQueue->Insert(GrowthNodes[GrowthCount++]);
					Queue_Spread(node->Element);
				} else {
					GrowthState[Map_Cell_Index(node->Element)] = false;
				}
			}

			index++;
			if (index < count) {
				node = GrowthQueue->Extract_Min();
			}
		}
	}
}


/// <summary>
/// Allocates the tiberium growth tracking data.
/// This routine sets the growth system up for the current map and primes it with every
/// cell that is able to grow. Any previous growth data is discarded first.
/// </summary>
void TiberiumClass::Init_Growth(void)
{
	Clear_Growth();

	GrowthNodes = new CellNode[Map_Cell_Count()];
	GrowthState = new bool [Map_Cell_Count()];
	GrowthQueue = new PriorityQueueClass<CellNode>(Map_Cell_Count());

	Recalc_Growth();
}


/// <summary>
/// Rebuilds the tiberium growth queue from the map.
/// This routine is used when the queue has run out of spare entries. Every cell of this
/// tiberium that is able to grow is enqueued afresh.
/// </summary>
void TiberiumClass::Recalc_Growth(void)
{
	GrowthCount = 0;
	GrowthQueue->Clear();

	for (int i = Map_Cell_Count() - 1; i >= 0; i--) {
		GrowthState[i] = false;
	}

	Map.Reset_Iterator();
	CellClass * iter = Map.Iterate();
	while (iter) {
		if (iter->Tiberium_Type_Here() == HeapID && iter->Can_Tiberium_Grow()) {
			GrowthNodes[GrowthCount].Element = iter->CellID;
			GrowthNodes[GrowthCount].Score = 0.0;
			GrowthQueue->Insert(GrowthNodes[GrowthCount++]);
			GrowthState[Map_Cell_Index(iter->CellID)] = true;
		}
		iter = Map.Iterate();
	}
}


/// <summary>
/// Frees the tiberium growth tracking data.
/// This routine tears down the growth queue and its bookkeeping, leaving the growth
/// system idle until Init_Growth builds it again.
/// </summary>
void TiberiumClass::Clear_Growth(void)
{
	if (GrowthQueue) {
		GrowthQueue->Clear();
		delete GrowthQueue;
		GrowthQueue = NULL;
	}

	if (GrowthNodes) {
		delete [] GrowthNodes;
		GrowthNodes = NULL;
	}

	if (GrowthState) {
		delete [] GrowthState;
		GrowthState = NULL;
	}

	GrowthCount = 0;
}


/// <summary>
/// Queues a cell for tiberium growth.
/// This routine is used whenever tiberium turns up in a cell that has not reached full
/// bloom, so that the growth system will visit the cell on a later pass.
/// </summary>
void TiberiumClass::Queue_Growth(Cell const & cell)
{
	int cellindex = Map_Cell_Index(cell);
	if (Map[cell].OverlayData < MAX_GROWTH_STAGE) {

		/*
		 * GrowthQueue does not recycle its entries.
		 * When space runs low, we need to clear and recalculate it.
		 */
		if (GrowthCount > Map_Cell_Count() - 10) {
			Recalc_Growth();
		}

		GrowthNodes[GrowthCount].Element = cell;
		GrowthNodes[GrowthCount].Score = float(Frame + abs(Scen->RandomNumber()) % MAX_GROWTH_DELAY);
		GrowthQueue->Insert(GrowthNodes[GrowthCount++]);
		GrowthState[cellindex] = true;
	}
}


/// <summary>
/// Handles the post load processing for the tiberium types.
/// This routine is called by the save game loader once every object has been read back
/// in, giving the tiberium types a chance to repair anything that does not survive the
/// trip.
/// </summary>
void TiberiumClass::Post_Load_Game(void)
{
	// nothing
}
