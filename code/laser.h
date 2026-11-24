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
#include "rgb.h"
#include "stage.h"

template<class T> class DynamicVectorClass;

class LaserDrawClass : public StageClass
{
	public:
		LaserDrawClass(Coord start, Coord end, int zadjust,
			RGBClass inner_color, RGBClass outer_color, RGBClass outer_spread,
			int duration, bool blinks, bool fades, float start_intensity, float end_intensity);
		~LaserDrawClass(void);

		void Draw_It(void);
		void AI(void);

		static void Update_All(void);
		static void All_Clear(void);
		static void Draw_All(void);

	public:
		/*
		 * These are the world coordinates of the two ends of the beam. Neither end follows
		 * the object that fired or the object that was hit, so the beam stays where it was
		 * first drawn for the whole of its short life.
		 */
		Coord Start;
		Coord End;

		/*
		 * This is the depth bias applied to the starting end of the beam, so that a shot
		 * leaving a muzzle above or below the firing object's own render row still sorts
		 * correctly against it.
		 */
		int ZAdjust;

		/*
		 * This is the color of the beam's thin core, drawn as a single line from one end to
		 * the other.
		 */
		RGBClass InnerColor;

		/*
		 * This is the color of the beam's outer glow -- two further lines drawn a pixel to
		 * either side of the core. If it is black, then the beam has no glow at all.
		 */
		RGBClass OuterColor;

		/*
		 * This is how far each channel of the glow color may stray from OuterColor. A fresh
		 * deviation is rolled every frame, which is what makes the glow shimmer.
		 */
		RGBClass OuterSpread;

		/*
		 * This is the lifetime of the beam, expressed in game frames. The beam deletes
		 * itself once its animation stage reaches this value.
		 */
		int Duration;

		/*
		 * If this beam should flicker on and off as it ages, then this flag will be true.
		 */
		bool Blinks;

		/*
		 * This is the current phase of a blinking beam, toggled every frame. While it is
		 * true the beam draws nothing at all, which is what produces the flicker.
		 */
		bool BlinkState;

		/*
		 * If the beam's intensity should slide from StartIntensity to EndIntensity across
		 * its life, then this flag will be true. Otherwise it draws at full intensity.
		 */
		bool Fades;

		/*
		 * These are the intensities the beam is drawn at when it is created and when it
		 * expires (0 - 1). They are interpolated over the beam's duration, but only when
		 * the Fades flag is set.
		 */
		float StartIntensity;
		float EndIntensity;

		/*
		 * This is the master list of every beam that is currently active. A beam adds
		 * itself here when it is created and removes itself when it is deleted, so the
		 * update and draw passes need no other bookkeeping.
		 */
		static DynamicVectorClass<LaserDrawClass *> LaserDraws;
};
