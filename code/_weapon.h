/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "point.h"

template<class T> class DynamicVectorClass;
class WeaponTypeClass;


extern DynamicVectorClass<WeaponTypeClass *> Weapons;

struct WeaponDataStruct {
	/*
	 * This points to the weapon mounted in this slot, or is NULL if the slot carries none.
	 */
	WeaponTypeClass * Weapon;

	/*
	 * This is the offset from the object's center to the point a shot is launched from,
	 * expressed in leptons. It is measured in the turret's frame, so it swings around as
	 * the turret turns.
	 */
	TPoint3D<int> FireFLH;

	/*
	 * This is how far out along the barrel the muzzle sits, expressed in leptons. It is
	 * measured after the barrel has been pitched, so the muzzle rides up with an elevated
	 * barrel.
	 */
	int BarrelLength;

	/*
	 * This raises the point the barrel pivots about, expressed in leptons, so that the shot
	 * lines up with the middle of the barrel artwork rather than with its base.
	 */
	int BarrelThickness;

	// Carries the weapon slot to or from a save game.
	template<typename S>
	void Serialize(S & stream)
	{
		stream.Serialize(Weapon);
		stream.Serialize(FireFLH);
		stream.Serialize(BarrelLength);
		stream.Serialize(BarrelThickness);
	}
};
