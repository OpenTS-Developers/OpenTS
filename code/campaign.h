/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once
#include "abstype.h"

#include "campaign.hh"
#include "vq.hh"

class CCINIClass;

class CampaignClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		CampaignClass(char const * name = NULL);
		virtual ~CampaignClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual bool Read_INI(CCINIClass const & ini) override;

		static CampaignType From_Name(char const * name);

	public:
		/*
		 * This is the side the campaign is played as, read from its "CD" key. The key is
		 * named for the game disc the campaign shipped on, one disc per side, which is why
		 * 0 means GDI, 1 means Nod and 2 means Firestorm. Only the first two get an intro.
		 */
		int CDNumber;

		/*
		 * This is the file name of the scenario that the campaign opens with. It is forced to
		 * upper case as it is read from the rules.
		 */
		char ScenarioName[_MAX_FNAME+_MAX_EXT];

		/*
		 * This is the movie that plays once the last mission of the campaign has been won.
		 */
		VQType FinalMovie;

		/*
		 * This is the campaign's title as it appears in the mission selection list. Until the
		 * rules supply one, it is borrowed from the campaign's given name.
		 */
		char Description[128];

		/*
		 * This is the expansion that the campaign belongs to. The campaign is only offered to
		 * the player when that expansion is enabled; ADDON_BASE_GAME means it is always
		 * available.
		 */
		int RequiredAddon;
};

void Read_Battle_INI(CCINIClass const & ini);
