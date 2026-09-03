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

#include "always.h"

#include "syncreport.h"

#include "_logic.h"
#include "_map.h"
#include "_mono.h"
#include "_rtti.h"
#include "_script.h"
#include "_vanim.h"
#include "_warhead.h"
#include "_weapon.h"
#include "aircraft.h"
#include "airctype.h"
#include "anim.h"
#include "animtype.h"
#include "blight.h"
#include "building.h"
#include "builtype.h"
#include "bullet.h"
#include "bullettype.h"
#include "cell.h"
#include "crc.h"
#include "dbgprint.h"
#include "drive.h"
#include "empulse.h"
#include "event.h"
#include "factory.h"
#include "getcpu.h"
#include "globals.h"
#include "goptions.h"
#include "house.h"
#include "houstype.h"
#include "iloco.h"
#include "incdec.h"
#include "infantry.h"
#include "infatype.h"
#include "light.h"
#include "mono.h"
#include "opents_build.h"
#include "overlay.h"
#include "overtype.h"
#include "particle.h"
#include "partsys.h"
#include "psystype.h"
#include "ptype.h"
#include "queue.h"
#include "saveload.h"
#include "scenario.h"
#include "scheme.h"
#include "script.h"
#include "session.h"
#include "side.h"
#include "smudge.h"
#include "smudtype.h"
#include "spawner.h"
#include "syncrechook.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "target.h"
#include "taskforc.h"
#include "team.h"
#include "teamtype.h"
#include "terrain.h"
#include "tevent.h"
#include "tiberium.h"
#include "trigger.h"
#include "trigtype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "vanim.h"
#include "vanimtype.h"
#include "version.h"
#include "warhead.h"
#include "waypoint.h"
#include "weapon.h"

#include <cstdio>
#include <float.h>


namespace {
	int LastReportFrame = -1;
	int ReportsThisSession = 0;
	bool OutOfSyncPlayer[MAX_PLAYERS] = {};
	constexpr int SYNC_REPORT_MAX_AGE_DAYS = 30;
	constexpr int SYNC_REPORT_SESSION_CAP = 3;
}


/// <summary>
/// Clears the per-frame and per-session report guards at the start of a game.
/// </summary>
void Sync_Report_Reset(void)
{
	LastReportFrame = -1;
	ReportsThisSession = 0;
	memset(OutOfSyncPlayer, 0, sizeof(OutOfSyncPlayer));
}


/// <summary>
/// Answers whether this house has already been reported as diverged from us. An identifier
/// outside the player range is never marked, so it answers false.
/// </summary>
bool Sync_Is_Out_Of_Sync(int house_id)
{
	if (house_id < 0 || house_id >= MAX_PLAYERS) {
		return(false);
	}
	return(OutOfSyncPlayer[house_id]);
}


/// <summary>
/// Records that this house has diverged from us, so that it is reported once rather than on
/// every frame that follows. Ignores an identifier outside the player range.
/// </summary>
void Sync_Mark_Out_Of_Sync(int house_id)
{
	if (house_id < 0 || house_id >= MAX_PLAYERS) {
		return;
	}
	OutOfSyncPlayer[house_id] = true;
}


/// <summary>
/// Writes the report for a detected divergence, subject to a once-per-frame and per-session cap.
/// Leaves all session, connection and UI state untouched; the caller decides what to do next.
/// </summary>
/// <returns>bool; Was a report file written by this call?</returns>
bool Report_Out_Of_Sync(EventClass const * events, int count, unsigned const * crc_ring, unsigned ring_size)
{
	if (Frame == LastReportFrame || ReportsThisSession >= SYNC_REPORT_SESSION_CAP) {
		return(false);
	}
	LastReportFrame = Frame;
	ReportsThisSession++;
	Print_CRCs(events, count, crc_ring, ring_size);
	return(true);
}


/**************************************************************************
 * Print_CRCs -- Prints a data file for finding Sync Bugs                 *
 *                                                                        *
 * INPUT:                                                                 *
 *      ev -- the event whose checksum disagreed, or NULL when the        *
 *            playback trap asked for the report                          *
 *                                                                        *
 *      crc_ring -- our recent frame checksums, indexed by frame          *
 *                  number masked to the ring size                        *
 *                                                                        *
 *      ring_size -- entries in the ring; a power of two                  *
 *                                                                        *
 * OUTPUT:                                                                *
 *      none                                                              *
 *                                                                        *
 * WARNINGS:                                                              *
 *      none                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   05/09/1995 BRR : Created.                                            *
 *========================================================================*/
void Print_CRCs(EventClass const * events, int count, unsigned const * crc_ring, unsigned ring_size)
{
	static char _NO_NAME [] = "None";

	int i,j;
	InfantryClass *infp;
	UnitClass *unitp;
	BuildingClass *bldgp;
	AircraftClass *airp;
	ObjectClass *objp;
	FILE *fp;
	HouseClass *housep;
	HousesType house;
	unsigned int GameCRC = 0;

	char filename[512];
	char const * debug_dir = Debug_Directory();
	if (debug_dir != NULL && debug_dir[0] != '\0') {
		SYSTEMTIME now;
		GetLocalTime(&now);
		Delete_Files_Older_Than(debug_dir, "SYNC_*.LOG", SYNC_REPORT_MAX_AGE_DAYS);
		snprintf(filename, sizeof(filename), "%s\\SYNC_H%d_%02u-%02u-%04u_%02u-%02u-%02u_F%d.LOG",
			debug_dir, PlayerPtr->HeapID,
			now.wDay, now.wMonth, now.wYear, now.wHour, now.wMinute, now.wSecond, Frame);
	} else {
		snprintf(filename, sizeof(filename), "SYNC%01d.TXT", PlayerPtr->HeapID);
	}

	Mono_Clear_Screen();
	Mono_Set_Cursor (0,0);

	fp = fopen(filename,"wt");
	if (fp==NULL) {
		DWORD const error = GetLastError();
		DebugString("Failed to open the out-of-sync report %s. Error %d - %s\n", filename, error, Last_Error_Text(error));
		return;
	}
	DebugString("Writing out-of-sync report to %s\n", filename);

	fprintf(fp, "TS Sync\n");

	fprintf(fp, "\nVersion %s\n", Version_Name());
	fprintf(fp, "Internal Version %s\n", VerNum.Version_Name());

	fprintf(fp, "Release Build: %s - %s\r\n", OPENTS_BUILD_DESCRIPTION, OPENTS_COMMIT_DATE);

	fprintf(fp, "Local house: H%d %s\n", PlayerPtr->HeapID, PlayerPtr->IniName.c_str());
	int identity = Spawner_Session_Identity();
	if (identity != 0) {
		fprintf(fp, "Session identity: %08x\n", identity);
	} else {
		fprintf(fp, "Session identity: (lobby) scenario=%s seed=%08x players=%d\n",
			Scen->ScenarioName, Seed, Session.MaxPlayers);
	}
	fprintf(fp, "Seed: %08x\n", Seed);
	fprintf(fp, "Session type: %d\n", Session.Type);
	fprintf(fp, "FPU control word: %x\n", _controlfp(0, 0));

	int cpu_type = PROC_PENTIUM_PRO;
	char vendor[32];
	vendor[0] = '\0';
	Get_CPU_Type(cpu_type, vendor, sizeof(vendor) - 1);
	fprintf(fp, "CPU vendor: %s\r\n", vendor);

	fprintf(fp, "Frames: %d\n", Frame);

	fprintf(fp, "Average FPS: %d\n", SecondsPassed != 0 ? TotalFrames / SecondsPassed : 0);
	fprintf(fp, "Max MaxAhead: %d\n", Session.MaxMaxAhead);
	fprintf(fp, "Latency setting: %d\n", Session.LatencyFudge);
	fprintf(fp, "Game speed setting: %d\n", Options.GameSpeed);
	fprintf(fp, "FrameSendRate: %d\n", Session.FrameSendRate);

	for (i = 0; i < MAX_PLAYERS; i++) {
		MPStatsType *stat = &Session.ConnectionStats[i];
		if (*stat->Name != '\0') {
			fprintf(fp, "\nName: %s\n", stat->Name);
			fprintf(fp, "Max avg round trip: %d\n", stat->MaxAvgRoundTrip);
			fprintf(fp, "Max round trip: %d\n", stat->MaxRoundTrip);
			fprintf(fp, "Resends: %d\n", stat->Resends);
			fprintf(fp, "Frame sync stalls: %d\n", stat->FrameSyncStalls);
			fprintf(fp, "Command cound stalls: %d\n", stat->CommandCountStalls);
			fprintf(fp, "Lost: %d\n", stat->Lost);
			fprintf(fp, "Percent lost: %d\n", stat->PercentLost);
		}
	}

	fprintf(fp, "\n----- Frame checksum ring (newest first) -----\n");
	{
		int const mask = (int)ring_size - 1;
		int const filled = (Frame + 1 < (int)ring_size) ? (Frame + 1) : (int)ring_size;
		for (i = 0; i < filled; i++) {
			int const frame = Frame - i;
			fprintf(fp, "Frame %d slot %3d %08x\n", frame, frame & mask, crc_ring[frame & mask]);
		}
	}

	//
	// Houses
	//
	fprintf(fp, "\n----- Houses -----\n");
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		GameCRC = 0;
		housep = Houses[house];
		fprintf(fp,"%-12s IsHuman:%d  Color:%-12s ID:%-3d HouseType:%s\n",
			(const char *)housep->IniName,
			housep->IsHuman,
			ColorSchemes[housep->Class->Scheme]->Name,
			housep->HeapID,
			HouseTypes[housep->Class->HeapID]->Name());
		//Add_CRC (&GameCRC, (int)housep->Credits + (int)housep->Power +
		//	(int)housep->Drain);
		Mono_Printf("House %s:%x\n",housep->Class->Name(),GameCRC);
	}

	//
	// Infantry
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"\n-------------------- %s Infantry -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Infantry.Count(); i++) {
			infp = (InfantryClass *)Infantry[i];
			if (infp->House==housep) {
				Coord coord = infp->PositionCoord;
				Add_CRC (&GameCRC, (int)coord.As_Int() + (int)infp->PrimaryFacing.Current().As_Dir256());

				int tarcom_index = 0;
				const char *tarcom_name = _NO_NAME;
				AbstractClass *tarcom = infp->TarCom;
				if (tarcom != NULL) {
					if (tarcom->Is_Techno()) {
						tarcom_index = tarcom->Fetch_Heap_ID();
					} else if (tarcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)tarcom;
						tarcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					tarcom_name = Name_From_RTTI(tarcom->RTTI);
				}

				int navcom_index = 0;
				const char *navcom_name = _NO_NAME;
				AbstractClass *navcom = infp->NavCom;
				if (navcom != NULL) {
					if (navcom->Is_Techno()) {
						navcom_index = navcom->Fetch_Heap_ID();
					} else if (navcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)navcom;
						navcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					navcom_name = Name_From_RTTI(navcom->RTTI);
				}

				char type_text[32];
				char tarcom_text[32];
				char navcom_text[32];
				snprintf(type_text, sizeof(type_text), "%d(%s)", infp->Class->HeapID, infp->Class->Name());
				snprintf(tarcom_text, sizeof(tarcom_text), "%s(%d)", tarcom_name, tarcom_index);
				snprintf(navcom_text, sizeof(navcom_text), "%s(%d)", navcom_name, navcom_index);

				fprintf(fp,"COORD:%6d,%6d,%5d  Facing:%3d  Mission:%2d  Type:%-14s Tgt:%-16s Speed:%4d  NavCom:%s\n",
					coord.X,coord.Y,coord.Z,(int)infp->PrimaryFacing.Current().As_Dir256(),infp->Get_Mission(),
					type_text, tarcom_text, int(infp->Speed * (MPH_LIGHT_SPEED + 1.0)), navcom_text);
			}
		}
		Mono_Printf("%s Infantry:%x\n",housep->Class->Name(),GameCRC);
	}

	//
	// Units
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"\n-------------------- %s Units -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Units.Count(); i++) {
			unitp = (UnitClass *)Units[i];
			if (unitp->House==housep) {
				Coord coord = unitp->PositionCoord;
				Add_CRC (&GameCRC, (int)coord.As_Int() + (int)unitp->PrimaryFacing.Current().As_Dir256() +
					(int)unitp->SecondaryFacing.Current().As_Dir256());

				int tarcom_index = 0;
				const char *tarcom_name = _NO_NAME;
				AbstractClass *tarcom = unitp->TarCom;
				if (tarcom != NULL) {
					if (tarcom->Is_Techno()) {
						tarcom_index = tarcom->Fetch_Heap_ID();
					} else if (tarcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)tarcom;
						tarcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					tarcom_name = Name_From_RTTI(tarcom->RTTI);
				}

				int navcom_index = 0;
				const char *navcom_name = _NO_NAME;
				AbstractClass *navcom = unitp->NavCom;
				if (navcom != NULL) {
					if (navcom->Is_Techno()) {
						navcom_index = navcom->Fetch_Heap_ID();
					} else if (navcom->RTTI == RTTI_CELL) {
						CellClass *cptr = (CellClass *)navcom;
						navcom_index = cptr->CellID.X | cptr->CellID.Y << 16;
					}
					navcom_name = Name_From_RTTI(navcom->RTTI);
				}
				char type_text[32];
				char tarcom_text[32];
				char navcom_text[32];
				snprintf(type_text, sizeof(type_text), "%d(%s)", unitp->Class->HeapID, unitp->Class->Name());
				snprintf(tarcom_text, sizeof(tarcom_text), "%s(%d)", tarcom_name, tarcom_index);
				snprintf(navcom_text, sizeof(navcom_text), "%s(%d)", navcom_name, navcom_index);

				fprintf(fp,"COORD:%6d,%6d,%5d  Facing:%3d  Facing2:%3d  Mission:%2d  Type:%-14s Tgt:%-16s NavCom:%-16s TrkNum:%3d TrkInd:%3d SpdAc:%d\n",
					coord.X,coord.Y,coord.Z,(int)unitp->PrimaryFacing.Current().As_Dir256(),(int)unitp->SecondaryFacing.Current().As_Dir256(),unitp->Get_Mission(),
					type_text, tarcom_text, navcom_text,
					unitp->Locomotion->Get_Track_Number(), unitp->Locomotion->Get_Track_Index(), unitp->Locomotion->Get_Speed_Accum());
			}
		}
		Mono_Printf("%s Units:%x\n",housep->Class->Name(),GameCRC);
	}

	//
	// Buildings
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"\n-------------------- %s Buildings -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Buildings.Count(); i++) {
			bldgp = (BuildingClass *)Buildings[i];
			if (bldgp->House==housep) {
				Coord coord = bldgp->PositionCoord;
				Add_CRC (&GameCRC, (int)coord.As_Int() + (int)bldgp->PrimaryFacing.Current().As_Dir256());
				char type_text[32];
				snprintf(type_text, sizeof(type_text), "%d(%s)", bldgp->Class->HeapID, bldgp->Class->Name());

				fprintf(fp,"COORD:%6d,%6d,%5d  Facing:%3d  Mission:%2d  Type:%-14s Tgt:%x\n",
					coord.X,coord.Y,coord.Z,(int)bldgp->PrimaryFacing.Current().As_Dir256(),bldgp->Get_Mission(),
					type_text, TargetClass(bldgp).Encode());
			}
		}
		Mono_Printf("%s Buildings:%x\n",housep->Class->Name(),GameCRC);
	}


	//
	// Aircraft
	//
	for (house = HOUSE_FIRST; house < Houses.Count(); house++) {
		housep = Houses[house];
		GameCRC = 0;
		fprintf(fp,"\n-------------------- %s Aircraft -------------------\n",
			housep->Class->Name());
		for (i = 0; i < Aircraft.Count(); i++) {
			airp = (AircraftClass *)Aircraft[i];
			if (airp->House==housep) {
				Coord coord = airp->PositionCoord;
				Add_CRC (&GameCRC, (int)coord.As_Int() + (int)airp->PrimaryFacing.Current().As_Dir256());
				char type_text[32];
				snprintf(type_text, sizeof(type_text), "%d(%s)", airp->Class->HeapID, airp->Class->Name());

				fprintf(fp,"COORD:%6d,%6d,%5d  Facing:%3d  Mission:%2d  Type:%-14s Tgt:%x\n",
					coord.X,coord.Y,coord.Z,(int)airp->PrimaryFacing.Current().As_Dir256(),airp->Get_Mission(),
					type_text, TargetClass(airp).Encode());
			}
		}
		Mono_Printf("%s Aircraft:%x\n",housep->Class->Name(),GameCRC);
	}

	#if 0
	//
	// Animations
	//
	AnimClass *animp;
		fprintf(fp,"-------------------- Animations -------------------\n");
	for (i = 0; i < Anims.Count(); i++) {
		animp = (AnimClass *)Anims[i];
		fprintf(fp,"Target:%x OwnerHouse:%d Loops:%d\n",
			animp->xObject,
			animp->OwnerHouse,
			animp->Loops);
	}
	#endif

	//------------------------------------------------------------------------
	// Map Layers
	//------------------------------------------------------------------------
	GameCRC = 0;
	for (i = 0; i < LAYER_COUNT; i++) {
		fprintf(fp,"\n>>>> MAP LAYER %d <<<<\n",i);
		for (j = 0; j < Map.Layer[i].Count(); j++) {
			objp = Map.Layer[i][j];
			if (objp->RTTI != RTTI_ANIM || objp->Fetch_ID() != -2) {
				Coord coord = objp->PositionCoord;
				Add_CRC (&GameCRC, (int)coord.As_Int() + (int)objp->RTTI);
				fprintf(fp,"Object %-5d %6d,%6d,%5d  ",j,coord.X,coord.Y,coord.Z);
				if (objp->RTTI != RTTI_WAVE && objp->RTTI != RTTI_LIGHT) {
					char type_text[32];
					snprintf(type_text, sizeof(type_text), "%d(%s)",
						((ObjectClass *)objp)->Class_Of()->Fetch_Heap_ID(),
						(const char *)((ObjectClass *)objp)->Class_Of()->IniName);
					fprintf(fp,"%-10s Type:%-16s ", Name_From_RTTI(objp->RTTI), type_text);
				} else {
					fprintf(fp,"%-10s Type:%-16s ", Name_From_RTTI(objp->RTTI), "0");
				}
				housep = objp->Owner_HouseClass();
				if (housep!=NULL) {
					fprintf(fp,"Owner: %s\n",housep->Class->Name());
				}
				else {
					fprintf(fp,"Owner: NONE\n");
				}
			}
		}
	}
	Mono_Printf("Map Layers:%x  \n",GameCRC);

	//------------------------------------------------------------------------
	// Logic Layers
	//------------------------------------------------------------------------
	GameCRC = 0;
	fprintf(fp,"\n>>>> LOGIC LAYER <<<<\n");
	for (i = 0; i < Logic.Count(); i++) {
		objp = Logic[i];
		if (objp->RTTI != RTTI_ANIM || objp->Fetch_ID() != -2) {
			Coord coord = objp->PositionCoord;
			Add_CRC (&GameCRC, (int)coord.As_Int() + (int)objp->RTTI);
			fprintf(fp,"Object %-5d %6d,%6d,%5d  ",i,coord.X,coord.Y,coord.Z);

			char type_text[32];
			type_text[0] = '\0';

			if (objp->RTTI == RTTI_AIRCRAFT)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((AircraftClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((AircraftClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_ANIM)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((AnimClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((AnimClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_BUILDING)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((BuildingClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((BuildingClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_BULLET)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((BulletClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((BulletClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_INFANTRY)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((InfantryClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((InfantryClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_OVERLAY)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((OverlayClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((OverlayClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_SMUDGE)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((SmudgeClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((SmudgeClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_TERRAIN)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((TerrainClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((TerrainClass *)objp)->Class_Of()->IniName);
			else if (objp->RTTI == RTTI_UNIT)
				snprintf(type_text, sizeof(type_text), "%d(%s)",
					((UnitClass *)objp)->Class_Of()->Fetch_Heap_ID(),
					(const char *)((UnitClass *)objp)->Class_Of()->IniName);

			fprintf(fp,"%-10s Type:%-16s ", Name_From_RTTI(objp->RTTI), type_text);

			housep = objp->Owner_HouseClass();
			if (housep!=NULL) {
				fprintf(fp,"Owner: %s\n",housep->Class->Name());
			}
			else {
				fprintf(fp,"Owner: NONE\n");
			}
		}
	}
	Mono_Printf("Logic:%x  \n",GameCRC);

	// Report the generator's table cursors rather than drawing a number, which would advance the
	// shared generator and diverge the machines further while the report is being written.
	fprintf(fp,"\nScenario generator: Index1=%d Index2=%d\n",
		Scen->RandomNumber.Index_1(), Scen->RandomNumber.Index_2());
	fprintf(fp,"My Frame:%d\n",Frame);

	if (count > 0 && events != NULL) {
		fprintf(fp,"\nOut of sync with %d player(s).\n", count);
		for (i = 0; i < count; i++) {
			EventClass const & ev = events[i];
			int const delay = ev.Data.FrameInfo.Delay;
			int const checked = ev.Frame - delay;
			char const * name = _NO_NAME;
			if (ev.ID >= 0 && ev.ID < Houses.Count() && Houses[(HousesType)ev.ID] != NULL) {
				name = Houses[(HousesType)ev.ID]->IniName.c_str();
			}
			fprintf(fp,"\nOffending event from %s:\n", name);
			fprintf(fp,"Type:         %d\n",ev.Type);
			fprintf(fp,"Frame:        %d\n",ev.Frame);
			fprintf(fp,"House ID:     %d\n",ev.ID);
			fprintf(fp,"Their CRC:    %x\n",ev.Data.FrameInfo.CRC);
			fprintf(fp,"CommandCount: %d\n",ev.Data.FrameInfo.CommandCount);
			fprintf(fp,"Delay:        %d\n",delay);
			fprintf(fp,"Checked frame:%d\n",checked);
			if (delay < (int)ring_size) {
				fprintf(fp,"Our CRC:      %x\n", crc_ring[checked & ((int)ring_size - 1)]);
			}
		}
	} else {
		fprintf(fp,"Report written by the PrintCRC playback trap.\n");
	}

	SyncRecorder.Print_All(fp, Sync_Engine_Names());

	Print_Heap_CRCs(fp);

	fclose(fp);

}
