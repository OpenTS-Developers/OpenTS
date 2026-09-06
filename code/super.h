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

/* $Header: /CounterStrike/SUPER.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SUPER.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 07/28/95                                                     *
 *                                                                                             *
 *                  Last Update : July 28, 1995 [JLB]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "abstract.h"
#include "ftimer.h"
#include "timer.h"

class SuperWeaponTypeClass;
class HouseClass;

class SuperClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		SuperClass(void);
		SuperClass(SuperWeaponTypeClass * type, HouseClass * owner);
		virtual ~SuperClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual int What_Am_I(void) const override {return(SuperClass::Fetch_RTTI());};

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_SUPERWEAPON);}

		virtual void Compute_CRC(CRCEngine & crc) const override;

		void Detach(AbstractClass const * target, bool all);

		bool Suspend(bool on);
		bool Enable(bool onetime = false, bool player=false, bool quiet=false);
		void Forced_Charge(bool player=false);
		void Place(Cell const & cell, bool player);
		void Drop_Pods(Cell const & cell) const;
		void Deactivate_Firestorm(int, bool player) const;
		bool AI(bool player=false);
		bool Remove(void);
		void Impatient_Click(void) const;
		int Anim_Stage(void) const;
		bool Discharged(bool player, Cell const & cell);
		bool Can_Place(void) const;
		bool Is_Charging(void) const;
		bool Is_Powered(void) const;
		bool Is_Ready(void) const {return(IsReady);}
		bool Is_Present(void) const {return(IsPresent);}
		bool Is_One_Time(void) const {return(IsOneTime && IsPresent);}

		char const * State_String(void) const;

	public:
		/*
		 * This points to the type class that describes this super weapon -- its recharge
		 * time, its cameo, and the action it performs when it is unleashed.
		 */
		SuperWeaponTypeClass *Class;

		bool Recharge(bool player=false);

		/*
		 * This is the house that owns this super weapon and that is credited as the
		 * attacker for whatever the weapon unleashes.
		 */
		HouseClass * House;

		CDTimerClass<FrameTimerClass> Control;

	public:
		/*
		 * If this super weapon lasts only as long as the house owns a building that grants
		 * it, then this flag will be true. A trigger action can clear it to hand the weapon
		 * over permanently.
		 */
		bool NeedsBuilding;
	private:
		bool IsPresent;
		bool IsOneTime;
		bool IsReady;
		bool IsSuspended;

		int OldStage;

		/*
		 * This is where a charge drain weapon (such as the firestorm defense) sits in its
		 * cycle. Such a weapon charges to full, stays ready until it is switched on, and
		 * then drains back down while it is running instead of discharging in one shot.
		 */
		enum {
			SUSPENDED = -1,
			CHARGING,
			READY,
			FIRESTORM_ON
		} ChargeDrainState;

		enum {
			ANIMATION_STAGES=54
		};
};
