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


class LightConvertClass;
template<class T> class DynamicVectorClass;


class LightSourceClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		LightSourceClass(Coord coord, int visibility, int intensity, int red, int green, int blue);
		LightSourceClass(void);
		virtual ~LightSourceClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		static void Reset(void);
		static void Process_Lighting(int time_budget_ms, bool force = false);

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;

		bool Is_Enabled(void) const {return(IsEnabled);};
		void Enable(bool update = false);
		void Disable(bool update = false);
		void Recalculate_Affected_Cells(bool defer = false);

	public:
		class PendingCellClass
		{
			public:
				PendingCellClass(Cell cell) :
					Converter(NULL),
					Intensity(0x10000),
					Ambient(0),
					Brightness(1000),
					TileBrightness(1000),
					AltBrightness(1000),
					CellID(cell)
				{
				}

			public:
				/*
				 * This is the drawer picked for the cell, or NULL while the pick is still
				 * outstanding. It doubles as the mark that the entry is finished with, so
				 * an entry that already has one costs nothing on a later pass.
				 */
				LightConvertClass * Converter;

				/*
				 * These are the lighting values picked along with the drawer, held here
				 * until the whole queue is committed to the map in one pass. They are the
				 * cell members of the same names.
				 */
				unsigned int Intensity;
				unsigned short Ambient;
				unsigned short Brightness;
				unsigned short TileBrightness;
				unsigned short AltBrightness;

				/*
				 * This is the cell that is waiting to be relit.
				 */
				Cell CellID;
		};

	public:
		/*
		 * This is the brightness of this light at its center, as a fixed point value scaled
		 * by 1000. What a cell actually receives falls off linearly with distance out to the
		 * Visibility radius.
		 */
		int Intensity;

		/*
		 * These are the color tints this light adds to the cells it reaches, each a fixed
		 * point value scaled by 1000 and each falling off with distance as the Intensity
		 * does, so that a light may glow in a color of its own.
		 */
		int RedTint;
		int GreenTint;
		int BlueTint;

		/*
		 * This is the point the light shines from, fixed when the source is created.
		 */
		Coord Position;

		/*
		 * This is the radius, expressed in leptons, that this light reaches. Cells further
		 * out than this are left alone.
		 */
		int Visibility;

	private:
		/*
		 * If this light is switched on and contributing to the cells around it, then this
		 * flag will be true. A light is created switched off so that its owner can settle it
		 * into place before the map is relit around it.
		 */
		bool IsEnabled;

		/*
		 * This is the queue of cells whose relighting has been put off until later. Entries
		 * pile up as lights are switched on and off and are drained by Process_Lighting,
		 * which spreads the work over several frames so that a sweeping lighting change
		 * cannot stall the game.
		 */
		static DynamicVectorClass<PendingCellClass *> PendingCells;

	public:
		/*
		 * If a light source changing state should relight the cells around it, then this
		 * flag will be true. It is cleared while lights are being created or destroyed in
		 * bulk, so that the map is not rebuilt once for every light involved.
		 */
		static bool Recalc;
};

extern DynamicVectorClass<LightSourceClass *> LightSources;
