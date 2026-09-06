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

/* $Header: /CounterStrike/WARHEAD.CPP 1     3/03/97 10:26a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : WARHEAD.CPP                                                  *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 05/20/96                                                     *
 *                                                                                             *
 *                  Last Update : July 19, 1996 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   WarheadTypeClass::As_Pointer -- Convert a warhead type number into a pointer.             *
 *   WarheadTypeClass::Read_INI -- Fetches the warhead data from the INI database.             *
 *   WarheadTypeClass::WarheadTypeClass -- Default constructor for warhead objects.            *
 *   WarheadTypeClass::operator delete -- Returns warhead object back to special memory pool.  *
 *   WarheadTypeClass::operator new -- Allocate a warhead object from the special heap.        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "warhead.h"

#include "_warhead.h"
#include "animtype.h"
#include "ccini.h"
#include "findmake.h"
#include "globals.h"
#include "partsys.h"
#include "psystype.h"
#include "savestream.h"
#include "session.h"
#include "stimer.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"
#include "vector.h"


/// <summary>
/// Converts a percentage string into a fractional multiplier.
/// This routine is used when reading the warhead's armor modifier list, where each value
/// may be written either as a percentage or as a plain fraction.
/// </summary>
/// <param name="string">The text to convert.</param>
/// <returns>Returns with the value as a fraction, such that "100%" becomes one.</returns>
float _Parse_Percentage(const char * string)
{
	if (strchr(string, '%')) {
		return((double)atoi(string) / 100);
	}

	return(atof(string));
}


/***********************************************************************************************
 * WarheadTypeClass::WarheadTypeClass -- Default constructor for warhead objects.              *
 *                                                                                             *
 *    This default constructor for a warhead object will fill in all the default values        *
 *    for a warhead. It is presumed that these values will be normal unless specifically       *
 *    overridden by the INI database.                                                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
WarheadTypeClass::WarheadTypeClass(char const * ininame) :
	BASECLASS(ininame),
	Deform(0),
	ProneDamage(1.),
	DeformThreshhold(0),
	ExplosionSet(),
	SpreadFactor(1),
	InfantryDeath(0),
	WebDuration(20 * TICKS_PER_SECOND),
	WebDurationVariation(25),
	WebRadius(2),
	LimpetFactor(0),
	Particle(NULL),
	IsWallDestroyer(false),
	IsWebby(false),
	IsWoodDestroyer(false),
	IsTiberiumDestroyer(false),
	IsOrganic(false),
	IsSparky(false),
	IsFire(false),
	IsConventional(false),
	IsRocker(false),
	IsBright(false),
	IsEMEffect(false),
	IsVeinhole(false)
{
	Create_ID();

	Warheads.Add(this);
	AbstractTypePtrTracker.Add(this);

	for (int armor = ARMOR_FIRST; armor < ARMOR_COUNT; armor++) {
		Modifier[armor] = 1;
	}
}


/// <summary>
/// Destructor for warhead objects.
/// This routine will break every link to this warhead before dropping it from the list of
/// warheads the game knows about.
/// </summary>
WarheadTypeClass::~WarheadTypeClass(void)
{
	Detach_This_From_All(this, true);
	Warheads.Delete(this);
	AbstractTypePtrTracker.Delete(this);
}


/***********************************************************************************************
 * WarheadTypeClass::Read_INI -- Fetches the warhead data from the INI database.               *
 *                                                                                             *
 *    Use this routine to retrieve the data specific to this warhead type class object from    *
 *    the INI database specified. Typical use of this is when processing the rules.ini         *
 *    file.                                                                                    *
 *                                                                                             *
 * INPUT:   ini   -- Reference to the INI database to fetch the values from.                   *
 *                                                                                             *
 * OUTPUT:  bool; Was the warhead entry found and the data retrieved?                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/19/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
bool WarheadTypeClass::Read_INI(CCINIClass const & ini)
{
	if (ini.Is_Present(IniName)) {
		SpreadFactor = ini.Get_Int(Name(), "Spread", SpreadFactor);
		Particle = TGet_Class(ini, Name(), "Particle", Particle);

		IsConventional = ini.Get_Bool(Name(), "Conventional", IsConventional);
		IsWallDestroyer = ini.Get_Bool(Name(), "Wall", IsWallDestroyer);
		IsWoodDestroyer = ini.Get_Bool(Name(), "Wood", IsWoodDestroyer);
		IsTiberiumDestroyer = ini.Get_Bool(Name(), "Tiberium", IsTiberiumDestroyer);
		IsSparky = ini.Get_Bool(Name(), "Sparky", IsSparky);
		IsRocker = ini.Get_Bool(Name(), "Rocker", IsRocker);
		IsFire = ini.Get_Bool(Name(), "Fire", IsFire);
		IsBright = ini.Get_Bool(Name(), "Bright", IsBright);

		ExplosionSet = TGet_TypeList<AnimTypeClass>(ini, Name(), "AnimList", ExplosionSet);

		InfantryDeath = ini.Get_Int(Name(), "InfDeath", InfantryDeath);

		Deform = ini.Get_Float(Name(), "Deform", Deform);
		DeformThreshhold = ini.Get_Int(Name(), "DeformThreshhold", DeformThreshhold);

		IsEMEffect = ini.Get_Bool(Name(), "EMEffect", IsEMEffect);

		IsWebby = ini.Get_Bool(Name(), "Webby", IsWebby);
		if (IsWebby) {
			WebDuration = ini.Get_Int(Name(), "WebDuration", WebDuration);
			WebDurationVariation = ini.Get_Int(Name(), "WebDurationVariation", WebDurationVariation);
			WebRadius = ini.Get_Int(Name(), "WebRadius", WebRadius);
		}
		LimpetFactor = ini.Get_Float(Name(), "LimpetFactor", LimpetFactor);

		ProneDamage = ini.Get_Float(Name(), "ProneDamage", ProneDamage);
		IsVeinhole = ini.Get_Bool(Name(), "Veinhole", IsVeinhole);

		char buffer[128];
		if (ini.Get_String(Name(), "Verses", "100%%,100%%,100%%,100%%,100%%", buffer, sizeof(buffer))) {
			char * aval = strtok(buffer, ",");
			for (int armor = ARMOR_FIRST; armor < ARMOR_COUNT; armor++) {
				double percent = _Parse_Percentage(aval);
				Modifier[armor] = percent;
				aval = strtok(NULL, ",");
			}
		}

		IsOrganic = (Modifier[ARMOR_STEEL] == 0);

		if (Session.Type != GAME_NORMAL && strcmp(Name(), "ARTYHE") == 0) {
			ProneDamage = .3;
			Modifier[ARMOR_NONE] = .4;
			Modifier[ARMOR_WOOD] = .85;
			Modifier[ARMOR_ALUMINUM] = .68;
			Modifier[ARMOR_STEEL] = .35;
			Modifier[ARMOR_CONCRETE] = .35;
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Adds this warhead's data to the game state checksum.
/// This routine is used by the multiplayer sync check to prove that every machine in the
/// game is playing with the same warhead rules.
/// </summary>
void WarheadTypeClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(SpreadFactor);
	crc(IsWallDestroyer);
	crc(IsWoodDestroyer);
	crc(IsTiberiumDestroyer);
	crc(IsOrganic);
	crc(IsSparky);
	crc(IsFire);
	crc(IsConventional);
	crc(IsRocker);
	crc(IsBright);
	crc(Deform);
	crc(DeformThreshhold);
	crc(ProneDamage);
	crc(IsVeinhole);
	for (int armor = ARMOR_FIRST; armor < ARMOR_COUNT; armor++) {
		crc(Modifier[armor]);
	}

	crc(ExplosionSet.Count());
	crc(InfantryDeath);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is part of the persistence interface. The save code stores the identifier
/// so that the object can be recognized when the game is loaded back in.
/// </summary>
/// <param name="retval">Pointer to the identifier to fill in.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT WarheadTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_WarheadTypeClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this warhead carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void WarheadTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(Deform);
	stream.Serialize(Modifier);
	stream.Serialize(ProneDamage);
	stream.Serialize(DeformThreshhold);
	stream.Serialize(ExplosionSet);
	stream.Serialize(InfantryDeath);
	stream.Serialize(SpreadFactor);
	stream.Serialize(WebDuration);
	stream.Serialize(WebDurationVariation);
	stream.Serialize(WebRadius);
	stream.Serialize(LimpetFactor);
	stream.Serialize(Particle);
	stream.Serialize(IsWallDestroyer);
	stream.Serialize(IsWebby);
	stream.Serialize(IsWoodDestroyer);
	stream.Serialize(IsTiberiumDestroyer);
	stream.Serialize(IsOrganic);
	stream.Serialize(IsSparky);
	stream.Serialize(IsFire);
	stream.Serialize(IsConventional);
	stream.Serialize(IsRocker);
	stream.Serialize(IsBright);
	stream.Serialize(IsEMEffect);
	stream.Serialize(IsVeinhole);
}


/// <summary>
/// Fetches the warhead of the name specified, creating it if it does not exist yet.
/// Use this routine while processing the rules so that a warhead mentioned before its own
/// section has been read will still resolve to a real object.
/// </summary>
/// <param name="name">The INI name of the warhead desired.</param>
/// <returns>Returns with a pointer to the warhead type of that name.</returns>
WarheadTypeClass * WarheadTypeClass::Find_Or_Make(const char * name)
{
	return(TFind_Or_Make<WarheadTypeClass>(name, Warheads));
}


/// <summary>
/// Removes any reference this warhead holds to the specified object.
/// This routine is called when an object is about to be destroyed so that the warhead
/// will not be left pointing at it.
/// </summary>
void WarheadTypeClass::Detach(AbstractClass const * target, bool all)
{
	if (target == (AbstractClass *)Particle) {
		Particle = NULL;
	}
	ExplosionSet.Delete((AnimTypeClass *)target);
}


/// <summary>
/// Finds the warhead type that goes by the name specified.
/// This routine is used when resolving a warhead reference in the rules database.
/// </summary>
/// <param name="name">The INI name of the warhead to search for.</param>
/// <returns>Returns with a pointer to the warhead type. Otherwise, NULL is returned.</returns>
WarheadTypeClass *WarheadTypeClass::From_Name(char const * name)
{
	for (int index = 0; index < Warheads.Count(); index++) {
		if (stricmp(Warheads[index]->Name(), name) == 0) {
			return(Warheads[index]);
		}
	}
	return(NULL);
}


/// <summary>
/// Fetches the RTTI type of this object.
/// </summary>
/// <returns>Returns with the RTTI identifier that marks this object as a warhead type.</returns>
RTTIType WarheadTypeClass::Fetch_RTTI(void) const
{
	return(RTTI_WARHEADTYPE);
}
