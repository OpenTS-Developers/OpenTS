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

#include "_infantr.h"
#include "techtype.h"

#include "infantry.hh"
#include "pip.hh"

/***************************************************************************
**	The various unit types need specific data that is unique to units as
**	opposed to buildings. This derived class elaborates these additional
**	data types.
*/
class InfantryTypeClass : public TechnoTypeClass
{
		typedef TechnoTypeClass BASECLASS;

	public:
		/*
		**	This value represents the unit class. It can serve as a unique
		**	identification number for this unit class.
		*/
		InfantryType HeapID;

		/*
		**	When this infantry unit is loaded onto a transport, then this
		**	is the pip shape to use. Primarily, this is a color control.
		*/
		PipEnum Pip;

		/*
		**	This is an array of the various animation frame data for the actions that
		**	the infantry may perform.
		*/
		DoInfoStruct const * DoControls;

		/*
		**	There are certain units with special animation sequences built into the
		**	shape file. These values tell how many frames are used for the firing animation.
		*/
		int FireLaunch;
		int ProneLaunch;

		/*
		 * These are the incidental voices this soldier mutters -- the first is idle chatter
		 * for when it is left unselected, the second a remark spoken as it leaves a transport.
		 */
		TypeList<int> VoiceComment;

		/*
		 * If this infantry type is a cyborg, then this flag will be true. A cyborg blows
		 * apart rather than falling over when killed, may go berzerk once half destroyed,
		 * and unlike flesh and blood it is susceptible to an EM pulse.
		 */
		bool IsCyborg;

		/*
		 * If this infantry type is immune to fear, then this flag will be true. Such a soldier
		 * never accumulates Fear, so it will neither cower nor run from what is shooting it.
		 */
		bool IsFearless;

		/*
		**	Does this infantry unit have crawling animation? If not, then this
		**	means that the "crawling" frames are actually running animation frames.
		*/
		bool IsCrawling;

		/*
		**	For those infantry types that can capture buildings, this flag
		**	will be set to true. Typically, this is the engineer.
		*/
		bool IsCapture;

		/*
		**	For infantry types that will run away from any damage causing
		**	events, this flag will be true. Typically, this is so for all
		**	civilians as well as the flame thrower guys.
		*/
		bool IsFraidyCat;

		/*
		 * If this infantry type can stand in tiberium unharmed, then this flag will be true.
		 * Everyone else is poisoned for as long as they remain in the field.
		 */
		bool IsTiberiumProof;

		/*
		**	This flags whether this infantry is actually a civilian. A
		**	civilian uses different voice responses, has less ammunition,
		**	and runs from danger more often.
		*/
		bool IsCivilian;

		/*
		**	If the infantry unit is equipped with C4 explosives, then this
		**	flag will be true. Such infantry can enter and destroy enemy
		**	buildings.
		*/
		bool IsBomber;

		/*
		 * If this infantry type is an engineer, then this flag will be true. An engineer
		 * captures or repairs a building by walking into it, and is consumed in the process.
		 */
		bool IsEngineer;

		/*
		 * If this infantry type masquerades as somebody else, then this flag will be true. To
		 * every house but its owner it is drawn and named as the Disguise rule's type, and no
		 * enemy will willingly shoot at it.
		 */
		bool IsDisguised;

		/*
		 * If this infantry type spies on a building instead of capturing it, then this flag
		 * will be true. Walking into an enemy structure hands its intelligence over to the
		 * agent's house and consumes the agent.
		 */
		bool IsAgent;

		/*
		 * If this infantry type steals rather than fights, then this flag will be true. A
		 * thief hunts tiberium processing structures for want of a weapon, and seizes any
		 * enemy vehicle it can reach, being consumed in the exchange.
		 */
		bool IsThief;

		/*
		 * If this infantry type may commandeer enemy vehicles, then this flag will be true.
		 * It puts the capture cursor over a vehicle and, for a soldier with no weapon of its
		 * own, narrows the list of things worth attacking down to vehicles alone.
		 */
		bool IsVehicleThief;

		/*
		 * If this infantry type is an attack dog rather than a soldier, then this flag will
		 * be true. A dog beds down in tiberium while guarding, panics outright once badly
		 * wounded, and has a burning death animation of its own.
		 */
		bool IsDoggie;

		/*
		 * If this infantry type gets about on a jump jet, then this flag will be true. While
		 * airborne it ignores the terrain below it entirely, but an ion storm grounds it and
		 * puts it back to walking.
		 */
		bool IsJumpJet;

		/*
		 * If this infantry type shrugs off webbing weapons, then this flag will be true.
		 * Everyone else is snared in place and struggles for as long as the warhead specifies.
		 */
		bool IsWebImmune;

		/*
		**	This is the explicit infantry type class constructor.
		*/
		InfantryTypeClass(char const * ininame = NULL);
		virtual ~InfantryTypeClass() override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override;

		static InfantryType From_Name(char const * name);
		static void Init(TheaterType ) {};
		static InfantryTypeClass * Find_Or_Make(char const * name);

		void Read_Sequence_INI(void);
		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual Coord const Coord_Fixup(Coord const & coord) const override;
		virtual Point3D Lepton_Dimensions(void) const override;
		virtual bool Create_And_Place(Cell const & cell, HouseClass * house = NULL) const override;
		virtual ObjectClass * Create_One_Of(HouseClass * house) const override;
		virtual Cell const * Occupy_List(bool placement=false) const override;
		virtual int Repair_Cost(void) const override;
		virtual int Repair_Step(void) const override;
};
