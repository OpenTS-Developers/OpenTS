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

/* $Header: /CounterStrike/ADATA.CPP 3     3/07/97 4:27p Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ADATA.CPP                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 30, 1994                                                 *
 *                                                                                             *
 *                  Last Update : July 9, 1996 [JLB]                                           *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AnimTypeClass::AnimTypeClass -- Constructor for animation types.                          *
 *   AnimTypeClass::One_Time -- Performs one time action for animation types.                  *
 *   AnimTypeClass::Init -- Load any animation artwork that is theater specific.               *
 *   Anim_Name -- Fetches the ASCII name of the animation type specified.                      *
 *   AnimTypeClass::As_Reference -- Fetch a reference to the animation type specified.         *
 *   AnimTypeClass::Init_Heap -- Initialize the animation type system.                         *
 *   AnimTypeClass::operator new -- Allocate an animation type object from private pool.       *
 *   AnimTypeClass::operator delete -- Returns an anim type class object back to the pool.     *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define INCLUDE_COM
#include "always.h"

#include "animtype.h"

#include "_mixfile.h"
#include "_rules.h"
#include "_theater.h"
#include "ccfile.h"
#include "data.h"
#include "dbgprint.h"
#include "findmake.h"
#include "globals.h"
#include "incdec.h"
#include "mixfile.h"
#include "overtype.h"
#include "rules.h"
#include "savestream.h"
#include "scenario.h"
#include "shapeset.h"
#include "sun.h"
#include "swizzle.h"
#include "tracker.h"
#include "warhead.h"


/// <summary>
/// Releases shape data that this type loaded through the file layer.
/// </summary>
static void Free_Demand_Loaded_Shape(void const *& data)
{
	delete [] (char *)data;
	data = NULL;
}


/***********************************************************************************************
 * AnimTypeClass::AnimTypeClass -- Constructor for animation types.                            *
 *                                                                                             *
 *    This is the constructor for static objects that elaborate the various animation types    *
 *    allowed in the game. Each animation in the game is of one of these types.                *
 *                                                                                             *
 * INPUT:   see below...                                                                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/23/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
AnimTypeClass::AnimTypeClass(char const *ininame) :
	BASECLASS(ininame),
	HeapID(ANIM_NONE),
	Biggest(0),
	Damage(0.0),
	Delay(1),
	Start(0),
	LoopStart(0),
	LoopEnd(0),
	Stages(0),
	Loops(0),
	Sound(VOC_NONE),
	ChainTo(NULL),
	DetailLevel(0),
	TranslucencyDetailLevel(0),
	RandomLoopDelayMin(0),
	RandomLoopDelayMax(0),
	RandomRateMin(0),
	RandomRateMax(0),
	Translucency(0),
	Spawns(NULL),
	SpawnCount(0),
	StartSound(VOC_NONE),
	BounceSound(VOC_NONE),
	ExpireSound(VOC_NONE),
	BounceAnim(NULL),
	ExpireAnim(NULL),
	TrailerAnim(NULL),
	TrailerSeperation(0),
	Elasticity(0.8f),
	MinZVel(3.5),
	MaxZVel(3.5),
	MaxXYVel(15.0),
	Warhead(NULL),
	DamageRadius(0),
	TiberiumSpawnType(NULL),
	TiberiumSpreadRadius(0),
	YSortAdjust(0),
	YDrawOffset(0),
	RunningFrames(0),
	IsFlamingGuy(false),
	IsVeins(false),
	IsMeteor(false),
	IsTiberiumChainReaction(false),
	IsTiberium(false),
	IsBouncer(false),
	IsTiled(false),
	IsShouldUseCellDrawer(true),
	IsUseNormalLight(false),
	IsDemandLoad(false),
	IsFreeAfterPlaying(false),
	IsAnimatedTiberium(false),
	IsAltPalette(false),
	IsNormalized(false),
	IsGroundLayer(false),
	IsFlat(false),
	IsTranslucent(false),
	IsScorcher(false),
	IsFlameThrower(false),
	IsCraterForming(false),
	IsSticky(false),
	IsPingPong(false),
	IsReverse(false),
	IsShouldFogRemove(true)
{
	Create_ID();
	IsSentient = true;
	IsStealthy = true;
	IsSelectable = false;
	IsLegalTarget = false;
	IsInsignificant = true;
	IsImmune = true;
	IsFootprint = false;

	AnimTypes.Add(this);
	AbstractTypePtrTracker.Add(this);

	if (ininame != NULL) {
		Load_Image(THEATER_NONE);
	}

	HeapID = (AnimType)AnimTypes.ID(this);
}


/// <summary>
/// Destroys this animation type.
/// Any artwork that was demand loaded is released and the type removes itself from the
/// heaps that track it.
/// </summary>
AnimTypeClass::~AnimTypeClass(void)
{
	if (IsDemandLoad && ImageData) {
		Free_Demand_Loaded_Shape(ImageData);
	}

	AbstractTypePtrTracker.Delete(this);
	AnimTypes.Delete(this);
}


/***********************************************************************************************
 * AnimTypeClass::Init -- Load any animation artwork that is theater specific.                 *
 *                                                                                             *
 *    This routine will examine all the animation types and for any that are theater           *
 *    specific, it will fetch a pointer to the artwork appropriate for the theater specified.  *
 *                                                                                             *
 * INPUT:   theater  -- The theater to align the animation artwork with.                       *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Call this routine when the theater changes.                                     *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void AnimTypeClass::Init(TheaterType theater)
{
	AnimType index;

	if (Debug_Map || theater != LastTheater) {
		for (index = ANIM_FIRST; index < AnimTypes.Count(); index++) {
			AnimTypeClass * anim = AnimTypes[index];

			if (!anim->IsDemandLoad) {
				if (anim->IsTheater) {
					char fullname[_MAX_FNAME+_MAX_EXT];	// Fully constructed iconset name.
					_makepath(fullname, NULL, NULL, anim->Name(), Theaters[theater].Suffix);
					anim->ImageData = MFCD::Retrieve(fullname);
					if (anim->ImageData == NULL) {
						_makepath(fullname, NULL, NULL, anim->Name(), ".SHP");
						anim->ImageData = MFCD::Retrieve(fullname);
					}
				} else if (anim->IsNewTheater) {
					anim->Load_Image(theater);
				}
			} else {
				if (anim->IsTheater || anim->IsNewTheater) {
					if (anim->ImageData != NULL) {
						Free_Demand_Loaded_Shape(anim->ImageData);
					}
				}
			}
		}
	} else {
		for (index = ANIM_FIRST; index < AnimTypes.Count(); index++) {
			AnimTypeClass * anim = AnimTypes[index];
			if (anim->IsNewTheater && anim->ImageData == NULL) {
				anim->Load_Image(theater);
			}
		}
	}
}


/// <summary>
/// Loads any theater specific artwork for this animation.
/// This is the single animation counterpart of Init, used when an animation type comes
/// into being after the theater has already been established.
/// </summary>
/// <param name="theater">The theater to align the animation artwork with.</param>
void AnimTypeClass::Init_Theater(TheaterType theater)
{
	if (Debug_Map || theater != LastTheater) {
		if (!IsTheater || IsDemandLoad) {
			if (IsNewTheater) {
				Load_Image(theater);
			}
		} else {
			char fullname[_MAX_FNAME+_MAX_EXT];	// Fully constructed iconset name.
			_makepath(fullname, NULL, NULL, Name(), Theaters[theater].Suffix);
			ImageData = MFCD::Retrieve(fullname);
		}
	}
}


/// <summary>
/// Loads the artwork for this animation and takes its measurements.
/// This routine fetches the shape file appropriate for the theater specified and then
/// notes the animation's frame count and its largest frame, so that sensible values stand
/// in for anything the rules did not bother to state.
/// </summary>
/// <param name="theater">The theater to align the animation artwork with.</param>
void AnimTypeClass::Load_Image(TheaterType theater)
{
	if (!IsDemandLoad && ImageData == NULL) {
		if (IsTheater) {
			Fetch_Normal_Image();
		} else {
			char fullname[_MAX_FNAME+_MAX_EXT];
			_makepath(fullname, NULL, NULL, !GraphicName.empty() ? Graphic_Name() : Name(), ".SHP");
			Theater_Naming_Convention(fullname, theater);
			ImageData = MFCD::Retrieve(fullname);
		}
	}

	ShapeSet const * shape = (ShapeSet const *)ImageData;
	if (shape != NULL) {
		if (Stages == 0) {
			Stages = shape->Get_Count();
		}
		if (LoopEnd == 0) {
			LoopEnd = Stages;
		}

		int biggest_size = 0;
		int biggest_frame = 0;

		for (int frame = 0; frame < shape->Get_Count(); frame++) {
			Rect rect = shape->Get_Rect(frame);
			if (rect.Size() > biggest_size) {
				biggest_frame = frame;
				biggest_size = rect.Size();
			}
		}

		Biggest = biggest_frame;
	}
}


/***********************************************************************************************
 * Anim_Name -- Fetches the ASCII name of the animation type specified.                        *
 *                                                                                             *
 *    This will convert the animation type specified into a text name. This name can be used   *
 *    for uniquely identifying the animation.                                                  *
 *                                                                                             *
 * INPUT:   anim  -- The anim type to convert to a text string.                                *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the ASCII string that identifies this animation.         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/06/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
char const * AnimTypeClass::Name_From(AnimType anim)
{
	if ((unsigned)anim > (unsigned)AnimTypes.Count()) return("");

	return(AnimTypes[anim]->Name());
}


/// <summary>
/// Converts an ASCII animation name into an animation type number.
/// This is the counterpart of Name_From and is used when translating the rules and the
/// scenario scripts into something the game can work with.
/// </summary>
/// <param name="name">The animation name to search for.</param>
/// <returns>
/// Returns with the matching animation type, or ANIM_NONE if the name is not recognized.
/// </returns>
AnimType AnimTypeClass::From_Name(char const * name)
{
	if (name != NULL) {
		for (int classid = ANIM_FIRST; classid < AnimTypes.Count(); classid++) {
			if (stricmp(AnimTypes[classid]->Name(), name) == 0) {
				return(AnimType)classid;
			}
		}
	}
	return(ANIM_NONE);
}


/// <summary>
/// Fetches this animation type's settings from the rules.
/// This routine gathers the animation's timing, artwork, sound and behavior settings from
/// the rules and art databases. The artwork is consulted for anything the rules chose to
/// leave unstated, such as the frame count.
/// </summary>
/// <param name="ini">The rules database to read this animation's section from.</param>
/// <returns>bool; Was the animation type's data read?</returns>
bool AnimTypeClass::Read_INI(CCINIClass const & ini)
{
	if (!ini.Section_Present(IniName)) {
		return(false);
	}

	if (IsDemandLoad && ImageData != NULL) {
		Free_Demand_Loaded_Shape(ImageData);
	}

	if (BASECLASS::Read_INI(ini)) {
		if (!GraphicName.empty()) {
			if (ImageData == NULL) {
				IsTheater = ArtINI.Get_Bool(Name(), "Theater", IsTheater);
				IsNewTheater = ArtINI.Get_Bool(Name(), "NewTheater", IsNewTheater);
			}
			Load_Image(THEATER_NONE);
		}

		Sound = ArtINI.Get_VocType(Name(), "Report", Sound);
		IsAltPalette = ArtINI.Get_Bool(Name(), "AltPalette", IsAltPalette);
		IsFlat = ini.Get_Bool(Name(), "Flat", IsFlat);
		IsFlameThrower = ini.Get_Bool(Name(), "Flamer", IsFlameThrower);
		IsNormalized = ini.Get_Bool(Name(), "Normalized", IsNormalized);
		IsGroundLayer = ini.Get_Bool(Name(), "Surface", IsGroundLayer);
		IsTranslucent = ini.Get_Bool(Name(), "Translucent", IsTranslucent);
		IsScorcher = ini.Get_Bool(Name(), "Scorch", IsScorcher);
		IsCraterForming = ini.Get_Bool(Name(), "Crater", IsCraterForming);
		IsSticky = ini.Get_Bool(Name(), "Sticky", IsSticky);
		IsPingPong = ini.Get_Bool(Name(), "PingPong", IsPingPong);
		IsReverse = ini.Get_Bool(Name(), "Reverse", IsReverse);
		IsTiberiumChainReaction = ini.Get_Bool(Name(), "TiberiumChainReaction", IsTiberiumChainReaction);

		int delay = ini.Get_Int(Name(), "Rate", -1);
		if (delay != -1) {
			Delay = (delay > 0) ? TICKS_PER_MINUTE / delay : 0;
		}

		Damage = ini.Get_Float(Name(), "Damage", Damage);
		Start = ini.Get_Int(Name(), "Start", Start);
		Stages = ini.Get_Int(Name(), "End", Stages);
		LoopStart = ini.Get_Int(Name(), "LoopStart", LoopStart);
		LoopEnd = ini.Get_Int(Name(), "LoopEnd", LoopEnd);
		Loops = ini.Get_Int(Name(), "LoopCount", Loops);
		ChainTo = TGet_Class(ini, Name(), "Next", ChainTo);

		DetailLevel = ini.Get_Int(Name(), "DetailLevel", DetailLevel);
		TranslucencyDetailLevel = ini.Get_Int(Name(), "TranslucencyDetailLevel", TranslucencyDetailLevel);

		Point2D loop_random_delay = ini.Get_Point(Name(), "RandomLoopDelay", Point2D(RandomLoopDelayMin, RandomLoopDelayMax));
		RandomLoopDelayMin = loop_random_delay.X;
		RandomLoopDelayMax = loop_random_delay.Y;

		Translucency = ini.Get_Int(Name(), "Translucency", Translucency);
		IsTiberium = ini.Get_Bool(Name(), "IsTiberium", IsTiberium);
		YSortAdjust = ini.Get_Int(Name(), "YSortAdjust", YSortAdjust);

		IsDemandLoad = ini.Get_Bool(Name(), "DemandLoad", IsDemandLoad);
		IsFreeAfterPlaying = ini.Get_Bool(Name(), "FreeAfterPlaying", IsFreeAfterPlaying);
		if (IsDemandLoad) {
			ImageData = NULL;
		}

		Elasticity = ini.Get_Float(Name(), "Elasticity", Elasticity);
		MaxXYVel = ini.Get_Float(Name(), "MaxXYVel", MaxXYVel);
		MinZVel = ini.Get_Float(Name(), "MinZVel", MinZVel);

		Spawns = TGet_Class(ini, Name(), "Spawns", Spawns);
		SpawnCount = ini.Get_Int(Name(), "SpawnCount", SpawnCount);
		IsMeteor = ini.Get_Bool(Name(), "IsMeteor", IsMeteor);
		IsVeins = ini.Get_Bool(Name(), "IsVeins", IsVeins);
		TiberiumSpreadRadius = ini.Get_Int(Name(), "TiberiumSpreadRadius", TiberiumSpreadRadius);
		TiberiumSpawnType = TGet_Class(ini, Name(), "TiberiumSpawnType", TiberiumSpawnType);

		IsAnimatedTiberium = ini.Get_Bool(Name(), "IsAnimatedTiberium", IsAnimatedTiberium);
		IsShouldFogRemove = ini.Get_Bool(Name(), "ShouldFogRemove", IsShouldFogRemove);
		IsFlamingGuy = ini.Get_Bool(Name(), "IsFlamingGuy", IsFlamingGuy);
		RunningFrames = ini.Get_Int(Name(), "RunningFrames", RunningFrames);
		YDrawOffset = ini.Get_Int(Name(), "YDrawOffset", YDrawOffset);

		StartSound = ini.Get_VocType(Name(), "StartSound", StartSound);
		BounceSound = ini.Get_VocType(Name(), "BounceSound", BounceSound);
		ExpireSound = ini.Get_VocType(Name(), "ExpireSound", ExpireSound);

		BounceAnim = TGet_Class(ini, Name(), "BounceAnim", BounceAnim);
		ExpireAnim = TGet_Class(ini, Name(), "ExpireAnim", ExpireAnim);
		TrailerAnim = TGet_Class(ini, Name(), "TrailerAnim", TrailerAnim);

		TrailerSeperation = ini.Get_Int(Name(), "TrailerSeperation", TrailerSeperation);
		DamageRadius = ini.Get_Int(Name(), "DamageRadius", DamageRadius);
		Warhead = TGet_Class(ini, Name(), "Warhead", Warhead);

		IsBouncer = ini.Get_Bool(Name(), "Bouncer", IsBouncer);
		IsTiled = ini.Get_Bool(Name(), "Tiled", IsTiled);
		IsShouldUseCellDrawer = ini.Get_Bool(Name(), "ShouldUseCellDrawer", IsShouldUseCellDrawer);
		IsUseNormalLight = ini.Get_Bool(Name(), "UseNormalLight", IsUseNormalLight);

		Point2D random_rate = ini.Get_Point(Name(), "RandomRate", Point2D(-1, -1));
		if (random_rate.X != -1) {
			RandomRateMin = (random_rate.X > 0) ? TICKS_PER_MINUTE / random_rate.X : 0;
		}
		if (random_rate.Y != -1) {
			RandomRateMax = (random_rate.Y > 0) ? TICKS_PER_MINUTE / random_rate.Y : 0;
		}
		if (RandomRateMax < 0) {
			RandomRateMax = 0;
		}
		if (RandomRateMin > RandomRateMax) {
			RandomRateMin = RandomRateMax;
		}

		return(true);
	}
	return(false);
}


/// <summary>
/// Re-attaches the artwork this animation type names, for the theater the scenario is
/// being restored into.
/// </summary>
void AnimTypeClass::Post_Load(void)
{
	BASECLASS::Post_Load();

	Fetch_Voxel_Image();

	if (!IsDemandLoad) {
		Fetch_Normal_Image();
		if (IsTheater) {
			char fullname[_MAX_FNAME+_MAX_EXT];	// Fully constructed iconset name.
			_makepath(fullname, NULL, NULL, Name(), Theaters[Scen->Theater].Suffix);
			ImageData = MFCD::Retrieve(fullname);
		} else if (IsNewTheater) {
			Load_Image(Scen->Theater);
		}
	}
}


/// <summary>
/// Lists the members this animation type carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void AnimTypeClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	stream.Serialize(HeapID);
	stream.Serialize(Biggest);
	stream.Serialize(Damage);
	stream.Serialize(Delay);
	stream.Serialize(Start);
	stream.Serialize(LoopStart);
	stream.Serialize(LoopEnd);
	stream.Serialize(Stages);
	stream.Serialize(Loops);
	stream.Serialize(Sound);
	stream.Serialize(ChainTo);
	stream.Serialize(DetailLevel);
	stream.Serialize(TranslucencyDetailLevel);
	stream.Serialize(RandomLoopDelayMin);
	stream.Serialize(RandomLoopDelayMax);
	stream.Serialize(RandomRateMin);
	stream.Serialize(RandomRateMax);
	stream.Serialize(Translucency);
	stream.Serialize(Spawns);
	stream.Serialize(SpawnCount);
	stream.Serialize(StartSound);
	stream.Serialize(BounceSound);
	stream.Serialize(ExpireSound);
	stream.Serialize(BounceAnim);
	stream.Serialize(ExpireAnim);
	stream.Serialize(TrailerAnim);
	stream.Serialize(TrailerSeperation);
	stream.Serialize(Elasticity);
	stream.Serialize(MinZVel);
	stream.Serialize(MaxZVel);
	stream.Serialize(MaxXYVel);
	stream.Serialize(Warhead);
	stream.Serialize(DamageRadius);
	stream.Serialize(TiberiumSpawnType);
	stream.Serialize(TiberiumSpreadRadius);
	stream.Serialize(YSortAdjust);
	stream.Serialize(YDrawOffset);
	stream.Serialize(RunningFrames);
	stream.Serialize(IsFlamingGuy);
	stream.Serialize(IsVeins);
	stream.Serialize(IsMeteor);
	stream.Serialize(IsTiberiumChainReaction);
	stream.Serialize(IsTiberium);
	stream.Serialize(IsBouncer);
	stream.Serialize(IsTiled);
	stream.Serialize(IsShouldUseCellDrawer);
	stream.Serialize(IsUseNormalLight);
	stream.Serialize(IsDemandLoad);
	stream.Serialize(IsFreeAfterPlaying);
	stream.Serialize(IsAnimatedTiberium);
	stream.Serialize(IsAltPalette);
	stream.Serialize(IsNormalized);
	stream.Serialize(IsGroundLayer);
	stream.Serialize(IsFlat);
	stream.Serialize(IsTranslucent);
	stream.Serialize(IsScorcher);
	stream.Serialize(IsFlameThrower);
	stream.Serialize(IsCraterForming);
	stream.Serialize(IsSticky);
	stream.Serialize(IsPingPong);
	stream.Serialize(IsReverse);
	stream.Serialize(IsShouldFogRemove);
}


/// <summary>
/// Fetches the persistent class identifier of this object.
/// This routine is used by the save game machinery to recognize an animation type when it
/// comes back off the stream.
/// </summary>
/// <param name="retval">Pointer to the place to store the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if there was nowhere to store the answer.</returns>
HRESULT STDMETHODCALLTYPE AnimTypeClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_AnimTypeClass;
	return(S_OK);
}


/// <summary>
/// Submits this animation type to the game state checksum.
/// This routine is used by the network sync check to prove that every machine in the game
/// is playing by the same rules.
/// </summary>
/// <param name="crc">The checksum engine to submit this object's data to.</param>
void AnimTypeClass::Compute_CRC(CRCEngine & crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc(IsNormalized);
	crc(IsGroundLayer);
	crc(IsFlat);
	crc(IsTranslucent);
	crc(IsScorcher);
	crc(IsFlameThrower);
	crc(IsCraterForming);
	crc(IsSticky);
	crc(IsPingPong);
	crc(IsReverse);
	crc(Biggest);
	crc(Damage);
	crc(IsTiberiumChainReaction);
	crc(Delay);
	crc(Start);
	crc(LoopStart);
	crc(LoopEnd);
	crc(Stages);
	crc(Sound);
	crc(DetailLevel);
	crc(TranslucencyDetailLevel);
	crc(RandomLoopDelayMin);
	crc(RandomLoopDelayMax);
	crc(Translucency);
	crc(IsTiled);
}


/// <summary>
/// Fetches the animation type of the name specified, creating it if need be.
/// This routine is used while the rules are being processed, so that one section may refer
/// to an animation whose own section has not been reached yet.
/// </summary>
/// <param name="name">The INI name of the animation type to find.</param>
/// <returns>Returns with a pointer to the animation type that carries that name.</returns>
AnimTypeClass * AnimTypeClass::Find_Or_Make(char const * name)
{
	return(TFind_Or_Make<AnimTypeClass>(name, AnimTypes));
}


/// <summary>
/// Removes this animation type's references to the object specified.
/// This routine is called when an object is about to leave the game, so that the animation
/// will not chain to something that no longer exists.
/// </summary>
/// <param name="target">The object that is about to be removed from the game.</param>
void AnimTypeClass::Detach(AbstractClass const * target, bool all)
{
	if (ChainTo == target) {
		ChainTo = NULL;
	}
}


/// <summary>
/// Fetches the shape data to draw this animation with.
/// A demand loaded animation has no artwork read at scenario start, so this routine will
/// pull the shape file in the first time something actually needs to draw the animation.
/// </summary>
/// <returns>
/// Returns with a pointer to the shape data. Otherwise, NULL is returned.
/// </returns>
void const * AnimTypeClass::Get_Image_Data(void) const
{
	void const * data = ImageData;
	if (data == NULL && IsDemandLoad) {
		DebugString("Demand loading image for %s\n", Full_Name());

		char fullname[_MAX_FNAME + _MAX_EXT];
		_makepath(fullname, NULL, NULL, !GraphicName.empty() ? Graphic_Name() : Name(), ".SHP");

		if (IsTheater) {
			_makepath(fullname, NULL, NULL, Name(), Theaters[Scen->Theater].Suffix);
		} else if (IsNewTheater) {
			Theater_Naming_Convention(fullname, Scen->Theater);
		}

		CCFileClass file(fullname);
		((void const *&)ImageData) = Load_Alloc_Data(file);
		((AnimTypeClass*)this)->Load_Image(Scen->Theater);

		data = ImageData;
	}
	return(data);
}


/// <summary>
/// Frees the demand loaded artwork for this animation.
/// This routine is called when an animation has finished playing, so that a rarely used
/// animation does not hold onto its shape data for the rest of the mission. Only those
/// animations that asked to be freed after playing will give their artwork up.
/// </summary>
void AnimTypeClass::Free_Image(void)
{
	if (IsDemandLoad && ImageData != NULL && IsFreeAfterPlaying) {
		DebugString("Freeing loaded image for %s\n", Full_Name());
		Free_Demand_Loaded_Shape(ImageData);
	}
}
