/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "coord.h"

template<class T> class DynamicVectorClass;

class IonBlastClass
{
	public:
		IonBlastClass(Coord coord);
		~IonBlastClass(void);

		void AI(void);
		void Draw_It(void);

		static void One_Time(void);
		static void Update_All(void);
		static void Clear_All(void);
		static void Delete_All(void);
		static void Draw_All(void);

	private:
		/*
		 * This is the location the blast landed at. It is the center that the ripple
		 * expands from as well as the point the damage and the explosion animations are
		 * applied at.
		 */
		Coord Position;

		/*
		 * This is the age of the blast in game frames. It doubles as the index of the
		 * precalculated ripple surface for this frame, and when it reaches the last
		 * of them the wave has run its course and the blast deletes itself.
		 */
		int Lifetime;

	public:
		/*
		 * This is the list of every ion blast currently in progress. A blast adds itself
		 * as it is created and removes itself as it dies, so the game logic and the
		 * renderer have only to walk this list to look after them all.
		 */
		static DynamicVectorClass<IonBlastClass *> IonBlasts;
};
