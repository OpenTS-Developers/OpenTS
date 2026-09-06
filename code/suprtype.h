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

#include "action.hh"
#include "overlay.hh"
#include "super.hh"
#include "vox.hh"

class BuildingTypeClass;
class SuperWeaponTypeClass;
class ShapeSet;
class WeaponTypeClass;

class SuperWeaponTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;

	public:
		SuperWeaponTypeClass(char const * ininame = NULL);
		virtual ~SuperWeaponTypeClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual int Fetch_Heap_ID(void) const override;

		virtual bool Read_INI(CCINIClass const & ini) override;

		virtual ActionType What_Action(Cell const & cell, ObjectClass * object);

		static SuperWeaponTypeClass * Find_Or_Make(const char * name);
		static SuperWeaponTypeClass * From_Action(ActionType action);
		static SuperWeaponType From_Name(char const * name);

	public:
		/*
		 * This is the index of this super weapon type within the super weapon type heap. It
		 * is assigned as the type is created, and is how the weapon is referred to elsewhere.
		 */
		SuperWeaponType HeapID;

		/*
		 * Pointer to the weapon this super weapon delivers. The missile launched by a silo
		 * takes its bullet, warhead, speed and range from it.
		 */
		WeaponTypeClass const * Weapon;

		/*
		 * These are the voice announcements this super weapon makes -- when it has finished
		 * charging, when it starts charging, when the player clicks its cameo before it is
		 * ready, and when it is clicked while its charging is suspended.
		 */
		VoxType VoxRecharge;
		VoxType VoxCharging;
		VoxType VoxImpatient;
		VoxType VoxSuspend;

		/*
		 * This is how long the super weapon takes to charge, expressed in game frames (the
		 * rules express it in minutes). A charge drain weapon measures its active time
		 * against this as well.
		 */
		int RechargeTime;

		/*
		 * This is which hard coded behavior the super weapon delivers -- the ion cannon, the
		 * multi missile, the firestorm defense, and so on. Several rules entries may share
		 * one behavior, so this is not the same thing as the heap identifier.
		 */
		SuperWeaponType Type;

		/*
		 * Pointer to the sidebar cameo artwork, fetched from the mix files by the name in
		 * SidebarImage. A missing shape falls back on the generic icon.
		 */
		ShapeSet const * CameoData;

		/*
		 * Where this super weapon's cameo sorts among the others of its kind on the sidebar,
		 * lowest first.
		 */
		int CameoSortOrder;

		/*
		 * This is the mouse action the player performs to aim this super weapon. It decides
		 * the cursor shown over the map and is how the click finds its way back here.
		 */
		ActionType Action;

		/*
		 * Pointer to a building type that must also be standing before this super weapon is
		 * granted. If NULL, then the building that offers the weapon is enough by itself.
		 */
		BuildingTypeClass const * AuxBuilding;

		/*
		 * This is the base file name of the sidebar cameo artwork, defaulting to the name of
		 * the rules section. The cameo is looked up under it whenever the type is read in.
		 */
		TStringID<24> SidebarImage;

		/*
		 * If this super weapon stays switched on once fired and drains its charge back down
		 * again rather than discharging in an instant, then this flag will be true. The
		 * firestorm defense works this way, which is why it has a charge drain state.
		 */
		bool UseChargeDrain;

		/*
		 * If this super weapon needs power in order to charge, then this flag will be true.
		 * An owner short of power suspends the charging rather than losing the weapon.
		 */
		bool IsPowered;

		/*
		 * If this super weapon is charged by hand rather than by the passage of time, then
		 * this flag will be true. Its timer is left stopped when the weapon is granted and
		 * after every firing, so something else must decide when it becomes ready.
		 */
		bool IsManualControl;
};

SuperWeaponType Special_From_Name(char const * name);
