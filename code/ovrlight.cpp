/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "ovrlight.h"

#include "_map.h"
#include "_rect.h"
#include "_rules.h"
#include "_surface.h"
#include "_tactica.h"
#include "bsurface.h"
#include "dsurface.h"
#include "globals.h"
#include "rules.h"
#include "scenario.h"
#include "surface.h"
#include "tactical.h"

#include <algorithm>

DynamicVectorClass<SpotLightClass *> SpotLights;

int SpotLightColorMode = -1;
int *SpotLightColorTable;
BSurface *SpotLightSurfaces[SpotLightClass::SPOTLIGHT_SURFACE_COUNT + SpotLightClass::SPOTLIGHT_EXTRA_SURFACE_COUNT];

extern "C" {
/*
 * Externs to the colour routines in colorops.cpp
 */
void __cdecl Adjust_Color_565(void *pal1, void *pal2, int red, int green, int blue, int intensity, char *arg7);
void __cdecl Adjust_Color_555(void *pal1, void *pal2, int red, int green, int blue, int intensity, char *arg7);
void __cdecl Adjust_Color_556(void *pal1, void *pal2, int red, int green, int blue, int intensity, char *arg7);
void __cdecl Adjust_Color_655(void *pal1, void *pal2, int red, int green, int blue, int intensity, char *arg7);

void __cdecl Brighten_Color_565(unsigned char *mul_buffer, unsigned short *color_buffer, int mulbuff_width, int color_buff_width, int dst_width, int dst_height, int *color_table);
void __cdecl Brighten_Color_555(unsigned char *mul_buffer, unsigned short *color_buffer, int mulbuff_width, int color_buff_width, int dst_width, int dst_height, int *color_table);
void __cdecl Brighten_Color_556(unsigned char *mul_buffer, unsigned short *color_buffer, int mulbuff_width, int color_buff_width, int dst_width, int dst_height, int *color_table);
void __cdecl Brighten_Color_655(unsigned char *mul_buffer, unsigned short *color_buffer, int mulbuff_width, int color_buff_width, int dst_width, int dst_height, int *color_table);
}


/// <summary>
/// Creates a spot light at the location specified.
/// This routine is used by the explosion, particle, and building light code to throw a
/// short lived glow onto the map. The light adds itself to the master list, so it will be
/// updated and drawn until it burns out without the creator having to keep hold of it.
/// </summary>
/// <param name="size">The scale of the light, which governs how wide the glow grows.</param>
SpotLightClass::SpotLightClass(Coord pos, int size) :
	Radius(0),
	Position(pos),
	Size(size)

{
	SpotLights.Add(this);
}


/// <summary>
/// Destroys the spot light.
/// The light removes itself from the master list, so it stops being updated and drawn.
/// </summary>
SpotLightClass::~SpotLightClass(void)
{
	SpotLights.Delete(this);
}


/// <summary>
/// Advances this spot light by one game frame.
/// The light widens as it ages and burns out once it has grown as far as it may.
/// </summary>
/// <remarks>The light deletes itself when it burns out, so the caller must not touch it
/// again after this routine returns.</remarks>
void SpotLightClass::AI(void)
{
	Radius += SPOTLIGHT_RADIUS_STEP;
	if (Radius >= SPOTLIGHT_MAX_RADIUS) {
		delete this;
	}
}


/// <summary>
/// Processes every active spot light.
/// This routine is called once per game logic loop. Lights that have grown past their
/// limit are disposed of as they are processed.
/// </summary>
void SpotLightClass::Update_All(void)
{
	for (int i = SpotLights.Count() - 1; i >= 0; i--) {
		SpotLights[i]->AI();
	}
}


/// <summary>
/// Performs the one time initialization of the spot light system.
/// This routine builds the brightness ramp surfaces that every spot light is drawn from,
/// and the table the brightening routines read a pixel's channels out of.
/// </summary>
/// <remarks>It is safe to call this routine again after the video mode changes -- the
/// artwork is built only once, but the color mode is picked up afresh each time.</remarks>
void SpotLightClass::One_Time(void)
{
	int i;
	int j;

	static bool _one_time = false;
	if (!_one_time) {
		for (i = 0; i < SPOTLIGHT_SURFACE_COUNT; i++) {
			if (SpotLightSurfaces[i] == NULL) {
				SpotLightSurfaces[i] = new BSurface(256, 128, 1);
			}
			SpotLightSurfaces[i]->Fill(0);
			BSurface surf(256, 256, 1);
			surf.Fill(0);
			int radius = i * 2 + 1;
			if (radius > 0) {
				for (int irad = radius, color = -2; irad > 0; irad -= 2, color += 4) {
					surf.Draw_Circle(Point2D(128, 128), irad, Rect(0, 0, 255, 255), std::max(0, color));
				}
			}
			for (j = 0; j < 128; j++) {
				SpotLightSurfaces[i]->Blit_From(Rect(0, j, 255, 1), surf, Rect(0, 2 * j, 255, 1));
			}
		}

		for (i = 0; i < SPOTLIGHT_EXTRA_SURFACE_COUNT; i++) {
			if (SpotLightSurfaces[i + SPOTLIGHT_SURFACE_COUNT] == NULL) {
				SpotLightSurfaces[i + SPOTLIGHT_SURFACE_COUNT] = new BSurface(256, 128, 1);
			}
			SpotLightSurfaces[i + SPOTLIGHT_SURFACE_COUNT]->Fill(0);
			BSurface surf(256, 256, 1);
			surf.Fill(0);
			int radius = (CELL_PIXEL_H * Rule->SpotlightRadius);
			surf.Draw_Circle(Point2D(128, 128), i - int(radius / -358.4), Rect(0, 0, 255, 255), 128 - i * 6);
			for (j = 0; j < 128; j++) {
				SpotLightSurfaces[i + SPOTLIGHT_SURFACE_COUNT]->Blit_From(Rect(0, j, 255, 1), surf, Rect(0, 2 * j, 255, 1));
			}
		}

		_one_time = true;
	}

	SpotLightColorMode = DSurface::Get_Primary_Color_Mode();

	if (SpotLightColorTable == NULL) {
		SpotLightColorTable = new int[65536];
		for (int color = 0; color < 65536; color++) {
			RGBClass rgb = DSurface::Deconstruct_Hicolor_Pixel(color);
			SpotLightColorTable[color] = (rgb.Get_Red() << 16) | (rgb.Get_Green() << 8) | (rgb.Get_Blue());
		}
	}
}


/// <summary>
/// Frees the artwork the spot lights draw with.
/// This routine is called during shutdown to release the brightness ramp surfaces and the
/// hicolor lookup table that One_Time built.
/// </summary>
void SpotLightClass::Clear_All(void)
{
	if (SpotLightColorTable != NULL) {
		delete [] SpotLightColorTable;
		SpotLightColorTable = NULL;
	}
	for (int i = 0; i < SPOTLIGHT_SURFACE_COUNT + SPOTLIGHT_EXTRA_SURFACE_COUNT; i++) {
		delete SpotLightSurfaces[i];
		SpotLightSurfaces[i] = NULL;
	}
}


/// <summary>
/// Draws this spot light onto the tactical map.
/// This routine brightens the pixels already on the logical surface through a circular
/// ramp, so the light reads as a glow over whatever has been rendered beneath it. Nothing
/// is drawn if the light lies off screen or under fog of war.
/// </summary>
void SpotLightClass::Draw_It(void)
{
	static char _index_table[SPOTLIGHT_MAX_RADIUS + 10] = {
		5, 10, 15, 20, 25, 30, 35, 40,
		45, 50, 55, 60, 61, 62, 63, 63,
		63, 62, 61, 60, 59, 58, 57, 56,
		55, 54, 53, 52, 51, 50, 49, 48,
		47, 46, 45, 44, 43, 42, 41, 40,
		39, 38, 37, 36, 35, 34, 33, 32,
		31, 30, 29, 28, 27, 26, 25, 24,
		23, 22, 21, 20, 19, 18, 17, 16,
		15, 14, 13, 12, 11, 10, 9, 8,
		7, 6, 5, 4, 3, 2, 1, 0,
		64, 65, 66, 67, 68, 69, 70, 71,
		72, 73,
	};

	Point2D drawpoint;
	if (TacticalMap->Coord_To_Pixel(Position, drawpoint)) {
		if (!Scen->Special.IsFogOfWar || !Map.Is_Fogged(Position)) {
			Surface *dsurf = LogicalSurface;
			Point2D pt = drawpoint - Point2D(128, 64);
			Rect dcliprect = TacticalRect;
			Rect drect(pt, 255, 127);
			int index = _index_table[Radius];
			if (index < SpotLightClass::SPOTLIGHT_SURFACE_COUNT) {
				index = index * Size / SpotLightClass::SPOTLIGHT_SURFACE_COUNT;
			}
			Surface * ssurf = SpotLightSurfaces[index];
			int stride = dsurf->Stride();
			bool overlapped = false;
			Rect srect(0, 0, 255, 127);
			Rect scliprect(0, 0, 255, 127);
			void * dbuffer;
			void * sbuffer;
			if (XSurface::Prep_For_Blit(*dsurf, dcliprect, drect, *ssurf, scliprect, srect, overlapped, dbuffer, sbuffer)) {

				unsigned short *dptr = (unsigned short *)dbuffer;
				unsigned char *sptr = (unsigned char *)sbuffer;
				unsigned char *sptr_row = sptr;
				unsigned short *dptr_row = dptr;

				if (SpotLightColorTable != NULL) {
					switch (SpotLightColorMode) {

						case COLORMODE_555:
							Brighten_Color_555(sptr, dptr, 256, stride, srect.Width, srect.Height, SpotLightColorTable);
							break;

						case COLORMODE_556:
							Brighten_Color_556(sptr, dptr, 256, stride, srect.Width, srect.Height, SpotLightColorTable);
							break;

						case COLORMODE_565:
							Brighten_Color_565(sptr, dptr, 256, stride, srect.Width, srect.Height, SpotLightColorTable);
							break;

						case COLORMODE_655:
							Brighten_Color_655(sptr, dptr, 256, stride, srect.Width, srect.Height, SpotLightColorTable);
							break;

						default:
							goto fallback;
					}
				} else {
					fallback:
					for (int y = 0; y < srect.Height; y++) {
						for (int x = 0; x < srect.Width; x++) {
							unsigned char spixel = *sptr++;
							if (spixel != 0) {
								unsigned short dpixel = *dptr;
								RGBClass rgb = DSurface::Deconstruct_Hicolor_Pixel(dpixel);
								int rr = rgb.Get_Red() + ((rgb.Get_Red() * spixel) >> 8);
								int gg = rgb.Get_Green() + ((rgb.Get_Green() * spixel) >> 8);
								int bb = rgb.Get_Blue() + ((rgb.Get_Blue() * spixel) >> 8);
								*dptr = DSurface::Build_Hicolor_Pixel(std::min(255, rr), std::min(255, gg), std::min(255, bb));
							}
							dptr++;
						}
						sptr_row += 256;
						sptr = sptr_row;
						dptr_row = (unsigned short *)((unsigned char *)dptr_row + stride);
						dptr = dptr_row;
					}
				}
				dsurf->Unlock();
				ssurf->Unlock();
			}
		}
	}
}


/// <summary>
/// Draws every active spot light.
/// This routine is called by the tactical map render after the scene has been laid down,
/// since a spot light brightens the pixels already present rather than drawing its own.
/// </summary>
void SpotLightClass::Draw_All(void)
{
	for (int i = SpotLights.Count() - 1; i >= 0; i--) {
		SpotLights[i]->Draw_It();
	}
}
