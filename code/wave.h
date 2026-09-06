/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "facing.h"
#include "object.h"
#include "polygon.h"
#include "vector.h"

#include "wave.hh"

class WaveClass : public ObjectClass
{
		typedef ObjectClass BASECLASS;

	public:
		WaveClass(Coord const & source_coord, Coord const & target_coord, TechnoClass * source, WaveType type, TechnoClass * target);
		WaveClass(void);
		virtual ~WaveClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual void Detach(AbstractClass const * target, bool all = true) override;

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual LayerType In_Which_Layer(void) const override;

		virtual bool Limbo(void) override;
		virtual bool Unlimbo(Coord const & coord, Dir256 facing = DIR_N) override;

		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		void Draw_Sonic(Point2D const & point, Rect const & cliprect);
		void Draw_Laser(Point2D const & point, Rect const & cliprect);

		virtual void AI(void) override;

		static void Init_Statics(void);

	private:
		void Set_Sonic_Pixel(int x, int xoff, int y, int yscreen, unsigned short * buffer, Rect const & cliprect) const;
		void Set_Laser_Pixel(unsigned short * buffer, int mult) const;

		void Laser_AI(void);

		void Sonic_AI(void);
		void Wave_Recalc_Affected_Cells(void);
		void Build_Wave_Shape(Coord const & coord1, Coord const & coord2);
		void Wave_Shape_AI(void);
		void Sonic_Damage(Coord const & coord);
		void Sonic_Add_Cell(Cell const & cell);

		void Init_Offset_Tables(void);

	private:
		/*
		 * This is the object that the wave was fired at. A sonic wave stays tethered to it,
		 * and stops growing and fades once the firer stops aiming at it or it strays too far.
		 */
		TechnoClass * Target;

		/*
		 * This specifies what kind of wave this is. It selects the drawing and logic
		 * handlers as well as the size template that the wave's geometry is built from.
		 */
		WaveType Type;

		/*
		 * This is the coordinate of the near end of the wave, at the muzzle of the object
		 * that fired it. The wave is placed upon the map here and takes its drawing depth
		 * from this height.
		 */
		Coord StartCoord;

		/*
		 * This is the coordinate of the far end of the wave. A laser overshoots its target
		 * a little so that the beam appears to pass through it.
		 */
		Coord EndCoord;

		Point2D WaveStartMiddle; 	/// initial wave pixel start
		Point2D WaveEndMiddle; 		/// initial wave pixel end
		Point2D WaveEndLeft; 		/// initial wave pixel end left
		Point2D WaveEndRight; 		/// initial wave pixel end right
		Point2D WaveStartLeft; 		/// initial wave pixel start left
		Point2D WaveStartRight; 	/// initial wave pixel start right

		/*
		 * These are the four corners of the wave, spread out from the line of fire by the
		 * width of its wave type and rotated to lie along it. They are converted into the
		 * screen points that the wave is rasterized from.
		 */
		Coord WaveEndLeftCoord;
		Coord WaveEndRightCoord;
		Coord WaveStartLeftCoord;
		Coord WaveStartRightCoord;

		/*
		 * If this wave is still tethered to the object that fired it, then this flag will
		 * be true. Once the link is broken the wave stops growing, its trailing edge is
		 * drawn in after the leading one, and the wave expires.
		 */
		bool IsWaveActive;

		/*
		 * This is the number of game frames the sonic wave has left to live, counting down
		 * from 100. It doubles as the phase of the ripple, so the distortion appears to
		 * travel along the wave as the wave ages.
		 */
		int WaveEC;

		double WaveProgress; /// sonic starting percent
		double FadeProgress; /// sonic ending percent

		PolygonShapeStruct WaveShape; /// the shape (geometry) of the wave

		Point2D ActiveWaveStartMiddle; 	/// active wave pixel start
		Point2D ActiveWaveEndMiddle; 	/// active wave pixel end
		Point2D ActiveWaveEndLeft; 		/// active wave pixel end left
		Point2D ActiveWaveEndRight; 	/// active wave pixel end right
		Point2D ActiveWaveStartLeft; 	/// active wave pixel start left
		Point2D ActiveWaveStartRight; 	/// active wave pixel start right

		/*
		 * This is the wave's polygon rasterized into one span per scanline, rebuilt from
		 * WaveShape every time the wave is drawn and thrown away again afterwards. The
		 * distortion is applied by sweeping along each of those spans in turn.
		 */
		mutable PolygonRasterStruct DrawData;

		/*
		 * These are the offsets, in pixels, that step one pixel along in each of the eight
		 * facings upon the surface being drawn to. They depend upon the stride of that
		 * surface, so they are rebuilt before the wave is drawn.
		 */
		int DirectionStrides[FACING_COUNT];

		/*
		 * This is the direction the wave appears to travel in upon the screen. It decides
		 * which way the rows are swept as the wave is drawn and which way the distortion
		 * lifts the pixels it samples.
		 */
		FacingType Direction;

		/*
		 * This is the brightness of the laser wave, which fades a little every frame until
		 * the beam is too faint to be worth drawing. It is the amount by which the red of
		 * each pixel lying under the beam is raised.
		 */
		int LaserEC;

		/*
		 * This is the object that fired the wave. Its weapon supplies the damage and the
		 * warhead the wave inflicts, and it is never harmed by its own wave.
		 */
		TechnoClass * Source;

		/// Unused
		FacingClass Facing;

		/*
		 * This is the list of cells that the wave currently lies over. It is rebuilt every
		 * frame as the wave grows and fades, and each cell in it takes the wave's damage.
		 */
		DynamicVectorClass<CellClass *> AffectedCells;

		/*
		 * This is the pixel offset that the sonic distortion lifts its replacement pixel
		 * from, indexed by the amplitude of the ripple at that point (0 - 12). It runs
		 * from zero to three pixels along the wave's direction of travel.
		 */
		int WaveIntensityTable[13];
};

Coord Lerp(Coord const & a, Coord const & b, float t);
Point2D Lerp(Point2D const & a, Point2D const & b, float t);
FacingType Facing_Between_Points(Point2D const & pt1, Point2D const & pt2);
