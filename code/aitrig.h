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
#include "ccini.h"

/*
 * AI trigger type enumeration
 */
enum AITriggerEnum {
	AIT_ENEMY_OWNS_X_COND_N = 0,
	AIT_HOUSE_OWNS_X_COND_N,
	AIT_POWER_YELLOW,
	AIT_POWER_RED,
	AIT_ENEMY_MONEY_COND_N,

	AIT_COUNT,
	AIT_NONE = -1,
};

/*
 * Condition enumeration
 */
enum ConditionEnum {
	COND_LT = 0,    /// less than
	COND_LE,        /// less than or equal to
	COND_EQ,        /// equal to
	COND_GE,        /// greater than or equal to
	COND_GT,        /// greater than
	COND_NE,        /// not equal to
	COND_COUNT
};

enum AITriggerHouseType
{
	AITRIG_HOUSE_NONE,		/// <none>
	AITRIG_HOUSE_INDEX,		/// Index from the rules list.
	AITRIG_HOUSE_ALL,		/// <any>
};

class HouseClass;
class TechnoTypeClass;
class TeamTypeClass;
class CCINIClass;
class CRCEngine;

class AITriggerTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		AITriggerTypeClass(const char *name = NULL);
		~AITriggerTypeClass(void);

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		static AITriggerTypeClass * Find_Or_Make(char const * ininame);

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_AITRIGGERTYPE);}

		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Write_INI(CCINIClass & ini) const override;
		static void Read_All(CCINIClass const & ini, INIScopeType scope);
		static void Write_All(CCINIClass & ini, INIScopeType scope);

		bool Process(HouseClass *house, HouseClass *enemy, bool skip_base_defense);

		double Get_Current_Weight(void) const { return(CurWeight); }

		void Set_Condition_Object(TechnoTypeClass *obj);

		TeamTypeClass * Get_First_TeamType(void){return(TeamTypeOne);}
		TeamTypeClass * Get_Second_TeamType(void){return(TeamTypeTwo);}

		void Set_First_TeamType(TeamTypeClass *team);
		void Set_Second_TeamType(TeamTypeClass *team);

		void Record_Success(void);
		void Record_Failure(void);

	private:
		bool Check_Enemy_Owns(HouseClass *house, HouseClass *enemy);
		bool Check_House_Owns(HouseClass *house, HouseClass *enemy);
		bool Check_Enemy_Yellow_Power(HouseClass *house, HouseClass *enemy);
		bool Check_Enemy_Red_Power(HouseClass *house, HouseClass *enemy);
		bool Check_Enemy_Money(HouseClass *house, HouseClass *enemy);

	private:
		/*
		 * This specifies which condition this trigger tests before it may be sprung. If it
		 * is AIT_NONE, then there is no condition and only the trigger's other gates apply.
		 */
		AITriggerEnum Type;

		/*
		 * This records whether the trigger came from the global rules or from the scenario's
		 * own local list. Global triggers are passed over entirely when the scenario asks
		 * for them to be ignored.
		 */
		INIScopeType Scope;

		/*
		 * This specifies how the trigger is restricted by house -- to none, to the single
		 * house named by the House field, or to all of them. Only campaign play consults it.
		 */
		AITriggerHouseType TrigHouse;

		/*
		 * If this trigger is allowed to be considered at all, then this flag will be true.
		 * Global triggers are always enabled; local ones take their state from the scenario's
		 * companion enable section, and are forced on outside of campaign play.
		 */
		bool IsEnabled;

		/*
		 * This is the house type this trigger is restricted to, which is only meaningful
		 * when TrigHouse is AITRIG_HOUSE_INDEX. If HOUSE_NONE, then no house was named.
		 */
		int House;

		/*
		 * This restricts the trigger to one side in multiplay: the side's position in the
		 * rules' side list, counted from one. If zero, then the trigger is available to every
		 * side.
		 */
		int MultiSide;

		/*
		 * This is the tech level the house must have reached before this trigger may be
		 * sprung. It is raised to cover whatever its teams need in order to be built.
		 */
		int TechLevelNeeded;

		/*
		 * This is the weight this trigger carries when the house picks between the triggers
		 * it could spring -- the heavier the weight, the likelier the pick. It is nudged up
		 * on success and down on failure, so that triggers that keep paying off get used.
		 */
		double CurWeight;

		/*
		 * These are the bounds the current weight is held within after every success or
		 * failure, so that a trigger can neither be forgotten nor become the only choice.
		 */
		double MinWeight;
		double MaxWeight;

		/*
		 * If this trigger may be used outside of campaign play, then this flag will be true.
		 */
		bool IsAvailableInSkirmish;

		/*
		 * If this trigger exists to raise base defense teams, then this flag will be true.
		 */
		bool IsForBaseDefense;

		/*
		 * These flags specify which difficulty settings the trigger is allowed on. Campaign
		 * play matches them against the scenario's difficulty, while skirmish and multiplay
		 * match them against the house's own handicap, which runs the opposite way.
		 */
		bool IsEnabledInEasy;
		bool IsEnabledInMedium;
		bool IsEnabledInHard;

		/*
		 * Pointer to the object type that the "owns" conditions tally up. If NULL, then that
		 * tally is taken to be zero.
		 */
		TechnoTypeClass *ConditionObject;

		/*
		 * These are the teams this trigger creates when it springs. The first is required --
		 * a trigger without one can never spring -- while the second is optional.
		 */
		TeamTypeClass *TeamTypeOne;
		TeamTypeClass *TeamTypeTwo;

		union {
			struct {
				int Number;
				ConditionEnum Condition;
			};
			char Data[32];
		} Params;

		/*
		 * These are the trigger's track record, counting the times it has been sprung and
		 * the times that it paid off. The ratio between them biases the weight adjustment,
		 * so that a long history counts for more than any single outcome.
		 */
		int TimesSucceded;
		int TimesExecuted;

		/*
		 * These are the names of the INI sections the trigger list is read from and written
		 * to -- the triggers themselves, and the companion section that records which of the
		 * scenario's local triggers are enabled.
		 */
		static char const * const INI_NAME;
		static char const * const INI_NAME_ENABLE;
};
