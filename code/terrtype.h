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

#include "objtype.h"
#include "rgb.h"

#include "bsize.hh"
#include "terrain.hh"
#include "theater.hh"


/****************************************************************************
**	These are the different TYPES of terrain objects. Every terrain object must
**	be one of these types.
*/
class TerrainTypeClass : public ObjectTypeClass
{
		typedef ObjectTypeClass BASECLASS;

	public:
		/*
		**	Which terrain object does this class type represent.
		*/
		TerrainType HeapID;

		/*
		 * This is the size of the cell footprint this terrain object covers, taken from the
		 * artwork rather than the rules. It selects the occupy list held in Occupy.
		 */
		BSizeType Foundation;

		/*
		 * This is the color that this terrain object is drawn with on the radar map. It is
		 * not specified in the rules, but lifted from the first color of the artwork.
		 */
		RGBClass RadarColor;

		/*
		 * These control the idle animation of an animated terrain object. The rate is the
		 * delay in game frames between animation stages, and the probability (0 - 1) is the
		 * chance per frame that an idle object will begin the animation again.
		 */
		int AnimationRate;
		float AnimationProbability;

		/*
		 * This is a vertical offset, expressed in pixels, added to the terrain object's draw
		 * position (and to its depth) so that oversized artwork lines up with the cell it
		 * occupies.
		 */
		int YDrawFudge;

		/*
		 * This is the type of Tiberium that a Tiberium spawning terrain object (a blossom
		 * tree) grows. It is only consulted when IsTiberiumSpawn is true.
		 */
		int TiberiumToSpawn;

		/*
		 * These specify which of the cell's infantry sub-positions this terrain object
		 * physically fills, as a three bit mask (0 - 7), one for each theater the artwork
		 * differs in. A value other than 7 leaves the cell only partially blocked.
		 */
		int TemperateOccupationBits;
		int SnowOccupationBits;

		/*
		**	Does this terrain object get placed on the water instead of the ground?
		*/
		bool IsWaterBased;

		/*
		**	Does this terrain object spawn the growth of Tiberium? Blossom trees are
		**	a good example of this.
		*/
		bool IsTiberiumSpawn;

		/*
		**	If this terrain object is flammable (such as trees are) then this
		**	flag will be true. Flammable objects can catch fire if damaged by
		**	flame type weapons.
		*/
		bool IsFlammable;

		/*
		 * If this terrain object plays an idle animation rather than sitting on a single
		 * still frame, then this flag will be true.
		 */
		bool IsAnimated;

		/*
		 * If this terrain object is the veinhole monster, then this flag will be true. A
		 * veinhole is not sentient, but it is always a legal target and it may be selected
		 * on the tactical map even though no house owns it.
		 */
		bool IsVeinhole;

		//----------------------------------------------------------------
		TerrainTypeClass(char const * ininame = NULL);
		virtual ~TerrainTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual int Fetch_Heap_ID(void) const override;

		static TerrainType From_Name(char const * name);
		static TerrainTypeClass * Find_Or_Make(const char * name);
		static void Init(TheaterType theater = THEATER_FIRST);
		static void One_Time(void){}

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house) const override;
		virtual ObjectClass * Create_One_Of(HouseClass *) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;

	private:
		Cell const * Occupy;
};
