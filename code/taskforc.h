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

#include "abstype.h"
#include "ccini.h"
#include "emember.h"

class TaskForceClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		TaskForceClass(char const *name=NULL);
		virtual ~TaskForceClass(void) override;

		virtual ClassID Class_ID(void) const override;

		static TaskForceClass * Find_Or_Make(char const * name);

		static void Read_All(CCINIClass const & ini, INIScopeType scope);
		static void Write_All(CCINIClass & ini, INIScopeType scope);
		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Write_INI(CCINIClass & ini) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TASKFORCE);}
		virtual void Compute_CRC(CRCEngine & crc) const override;

		static TaskForceClass * From_Name(char const * name);
		static TaskForceClass * From_Given_Name(char const * name);

		int Required_Object_Count(void) const;
		int Needed_Tech_Level(void) const;
		bool Has_Only_Infantry(void) const;

		TaskForceClass(TaskForceClass const &that);
		TaskForceClass operator=(TaskForceClass const &that);

	public:
		/*
		 * This is the group number that teams built from this task force belong to. A team
		 * recruits objects already carrying that number first, which keeps successive
		 * teams from poaching one another's units. If -1, then there is no group.
		 */
		int Group;

		/*
		**	Number and type of members desired for this team.
		*/
		int ClassCount;
		INIScopeType Scope;
		EnlistedMemberClass Members[MAX_TEAM_CLASSCOUNT];
};
