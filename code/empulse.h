/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "abstract.h"
#include "coord.h"
#include "rect.h"

class Cell;
class TechnoClass;
template<class T> class DynamicVectorClass;

class EMPulseClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		EMPulseClass(Cell cell, int spread, int duration, TechnoClass *source);
		EMPulseClass(void);
		virtual ~EMPulseClass(void) override;

		virtual void Compute_CRC(CRCEngine &crc) const override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_EMPULSE);}

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		static void Update_All(void);
		static void Reset(void);

		/*
		 * This is the master list of every electromagnetic pulse currently in effect. A pulse
		 * joins the list as it is created and drops out of it when it expires.
		 */
		static DynamicVectorClass<EMPulseClass *> EMPulses;

	private:
		void Create(TechnoClass * source);
		void Destroy(void);

		/*
		 * This is the cell that the pulse is centered upon. Everything within the pulse's
		 * radius of it is disrupted for as long as the pulse lasts.
		 */
		Cell CellID;

		/*
		 * This is the radius of the pulse, expressed in cells. The area affected is a circle
		 * rather than a square, so the corners of the surrounding block are spared.
		 */
		int Spread;

		/*
		 * This is the game frame the pulse came into being on. Taken with the Duration, it fixes
		 * the frame the pulse expires on and releases everything it had disrupted.
		 */
		int CreationFrame;

		/*
		 * This is the number of game frames the pulse remains in effect. It doubles as the
		 * length of the stun applied to every unit caught in the blast and of the outage
		 * inflicted on every building.
		 */
		int Duration;
};

void Update_EMPulses(void);
