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
#include "tmission.h"

class ScriptTypeClass;

class ScriptClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

		friend class ScriptTypeClass;

	public:
		ScriptClass(ScriptTypeClass *type = NULL);
		virtual ~ScriptClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override { return(RTTI_SCRIPT); }

		virtual void Compute_CRC(CRCEngine &) const override;

		TeamMissionClass Get_Current_Mission(void);
		TeamMissionClass Get_Next_Mission(void);

		bool Stop_Script(void);
		bool Set_Line(int linenum);

		bool Next_Mission(void);
		bool Has_Missions_Remaining(void);

	private:
		/*
		 * Pointer to the script type that supplies the mission list this script is working
		 * its way through. Any number of teams may be running scripts of the same type.
		 */
		ScriptTypeClass *Class;

		/// Unused
		int Unused1;

		/*
		 * This is the mission line that the script is presently sitting on. It advances as
		 * the team satisfies each mission in turn, and is -1 while the script has yet to be
		 * started or has been stopped.
		 */
		int CurrentLineNumber;
};

class ScriptTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

		friend class ScriptClass;

	public:
		ScriptTypeClass(char const *name = NULL);
		virtual ~ScriptTypeClass(void) override;

		static ScriptTypeClass * Find_Or_Make(char const * ininame = NULL);

		virtual HRESULT GetClassID(CLSID * retval) override;

		static void Read_All(CCINIClass const & ini, INIScopeType scope);
		static void Write_All(CCINIClass & ini, INIScopeType scope);

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Write_INI(CCINIClass & ini) const override;

		virtual void Compute_CRC(CRCEngine & crc) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_SCRIPTTYPE);}
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);};

		static ScriptTypeClass * From_Name(char const * name);
		static ScriptTypeClass * From_Given_Name(char const * name);

	public:
		/*
		 * This is the script type's position within the master script type list, taken as
		 * the type is created. It identifies the type wherever a pointer will not serve.
		 */
		int HeapID;

		/*
		 * This records where the script type came from -- SCOPE_GLOBAL for the ones the AI
		 * rules declare for every mission, SCOPE_LOCAL for those a scenario declares for
		 * its own use.
		 */
		INIScopeType Scope;

		/*
		**	Number and list of missions that this team will follow.
		*/
		int MissionCount;
		TeamMissionClass MissionList[MAX_TEAM_MISSIONS];

		/*
		 * This is the name of the INI section that lists every script type a scenario
		 * declares.
		 */
		static char const * const INI_NAME;
};
