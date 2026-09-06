/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "_rules.h"
#include "coord.h"
#include "ftimer.h"
#include "rgb.h"
#include "timer.h"
#include "win.h"

#include "revent.hh"

class SaveStreamClass;
class SaveStreamClass;
template<class T> class DynamicVectorClass;

class RadarEventClass
{
	public:
		static bool Save(SaveStreamClass & stream);
		static bool Load(SaveStreamClass & stream);

	public:
		RadarEventClass(RadarEventType event, Cell cell);
		~RadarEventClass(void);

		void Serialize(SaveStreamClass & stream);

		void Process(void);
		void Draw(void);
		void Plot(void);
		int Get_Visibility_Duration(void);
		int Get_Duration(void);
		int Get_Suppression_Distance(void);
		RGBClass Get_Max_Color(void) const;
		RGBClass Get_Min_Color(void) const;
		void Get_Event_Rect(Point2D (& event_rect)[4]) const;

		static void Draw_Events(void);
		static void Remove_Finished(void);
		static void Clear(void);

	public:
		/*
		 * This is the kind of thing the event is flagging. It picks the colors the ping
		 * is drawn in and, through the rules, its lifetime and suppression distance.
		 */
		RadarEventType Type;

		/*
		 * This is where the event sits on the radar, expressed in radar pixels from the
		 * radar's top left corner, and the box corners are shifted by it before drawing.
		 */
		Point2D Offset;

		/*
		 * This is the distance from the event's center out to a corner of its box. It
		 * shrinks every frame until it reaches the minimum the rules allow, which is what
		 * makes the ping close in on the spot it is flagging.
		 */
		float Radius;

		/*
		 * This is the angle the event's box is turned through, expressed in radians. It
		 * starts at 45 degrees so the ping opens as a diamond and squares up as it settles.
		 */
		float RotationAngle;

		/*
		 * This is how far the box turns each game frame, expressed in radians. It eases off
		 * as the box closes in so the ping comes to rest rather than snapping square.
		 */
		float RotationSpeed;

		/*
		 * This is where the event currently stands between its two colors (0 - 1). The ping
		 * is drawn in the blend it names, which is what gives the box its pulse.
		 */
		float ColorFactor;

		/*
		 * This is how far along the color cycle the event moves each frame. Its sign flips
		 * at either end of the cycle, so the pulse runs back and forth rather than jumping.
		 */
		float ColorSpeed;

		/*
		 * This is the cell that the event is drawing attention to. A later event of the
		 * same kind raised within the suppression distance of it is thrown away.
		 */
		Cell Location;

		/*
		 * This counts down what is left of the event's life. Once it expires and the box
		 * has stopped turning, the event retires itself from the radar.
		 */
		CDTimerClass<FrameTimerClass> DurationTimer;

		/*
		 * This counts down how much longer the settled box stays drawn. The event stops
		 * updating when it expires, though it lingers until DurationTimer runs out too.
		 */
		CDTimerClass<FrameTimerClass> VisibilityTimer;

		/*
		 * If the event's box is still closing in and turning, then this flag will be true.
		 * It is cleared once the box has shrunk as far as it will go and come square.
		 */
		bool IsRotating;

		/*
		 * If the event still has anything to show, then this flag will be true. Clearing
		 * it stops the event being processed, so a spent ping costs nothing per frame.
		 */
		bool IsVisible;

		/*
		 * This is the master list of every radar event in play. Events add themselves to it
		 * as they are created and take themselves out again as they are destroyed.
		 */
		static DynamicVectorClass<RadarEventClass *> RadarEvents;
};


void Process_Radar_Events(void);
bool Submit_Radar_Event(RadarEventType event, Cell cell);
bool Can_Suppress_Radar_Event(RadarEventType event);
bool Try_Suppress_Radar_Event(RadarEventType event, int, Cell cell);
bool No_Radar_Events_Submitted(void);

extern Cell LastRadarEventCell;
