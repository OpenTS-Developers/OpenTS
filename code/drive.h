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

/* $Header: /CounterStrike/DRIVE.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : DRIVE.H                                                      *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : April 14, 1994                                               *
 *                                                                                             *
 *                  Last Update : April 14, 1994   [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once


#include "face.h"
#include "ftimer.h"
#include "ipiggy.h"
#include "loco.h"
#include "matrix3d.h"
#include "timer.h"

#include <memory>

#include "mark.hh"

/****************************************************************************
**	Movable objects are handled by this class definition. Moveable objects
**	cover everything except buildings.
*/
class DriveLocomotionClass : public LocomotionClass, public IPiggyback
{
		typedef LocomotionClass BASECLASS;

	public:

		/*---------------------------------------------------------------------
		**	Constructors, Destructors, and overloaded operators.
		*/
		DriveLocomotionClass(void);
		virtual ~DriveLocomotionClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
		virtual ULONG STDMETHODCALLTYPE Release(void) override;

		virtual boolean STDMETHODCALLTYPE Is_Moving(void) override;
		virtual Coord STDMETHODCALLTYPE Destination(void) override;
		virtual Coord STDMETHODCALLTYPE Head_To_Coord(void) override;
		virtual Matrix3D STDMETHODCALLTYPE Draw_Matrix(int *key) override;
		virtual int STDMETHODCALLTYPE Z_Adjust(void) override;
		virtual ZGradientType STDMETHODCALLTYPE Z_Gradient(void) override;
		virtual boolean STDMETHODCALLTYPE Process(void) override;
		virtual void STDMETHODCALLTYPE Move_To(Coord to) override;
		virtual void STDMETHODCALLTYPE Stop_Moving(void) override;
		virtual void STDMETHODCALLTYPE Do_Turn(DirType coord) override;
		virtual void STDMETHODCALLTYPE Unlimbo(void) override;
		virtual void STDMETHODCALLTYPE Force_Track(int track, Coord coord) override;
		virtual LayerType STDMETHODCALLTYPE In_Which_Layer(void) override;
		virtual void STDMETHODCALLTYPE Force_New_Slope(int ramp) override;
		virtual boolean STDMETHODCALLTYPE Is_Moving_Now(void) override;
		virtual void STDMETHODCALLTYPE Mark_All_Occupation_Bits(int mark) override;
		virtual boolean STDMETHODCALLTYPE Is_Moving_Here(Coord to) override;
		virtual boolean STDMETHODCALLTYPE Will_Jump_Tracks(void) override;
		virtual void STDMETHODCALLTYPE Lock(void) override;
		virtual void STDMETHODCALLTYPE Unlock(void) override;
		virtual int STDMETHODCALLTYPE Get_Track_Number(void) override;
		virtual int STDMETHODCALLTYPE Get_Track_Index(void) override;
		virtual int STDMETHODCALLTYPE Get_Speed_Accum(void) override;

		virtual bool Begin_Piggyback(std::unique_ptr<ILocomotion> carried) override;
		virtual std::unique_ptr<ILocomotion> End_Piggyback(void) override;
		virtual bool Is_Ok_To_End(void) override;
		virtual HRESULT Piggyback_CLSID(GUID * classid) override;
		virtual bool Is_Piggybacking(void) override {return(Piggybacker != nullptr);}

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		bool Stop_Driver(void);
		bool Start_Driver(Coord const & headto);

		void Mark_Track(Coord const & headto, MarkType type);

		Matrix3D Get_Slope_Matrix(void) const;
		double Get_Slope_Ratio(void) const;
		void Set_Slope(int ramp);
		BOOL Is_Angled(void) const;
		bool Incoming(Cell cell);
		bool Abandon_Navigation(void);

		/**********************************************************************
		**	These enumerations are used as working constants that exist only
		**	in the DriveClass namespace.
		*/
		enum DriveLocomotionClassEnum {
			BACKUP_INTO_REFINERY=64,    // Track to backup into refinery.
			OUT_OF_REFINERY,            // Track to leave refinery.
			OUT_OF_WEAPON_FACTORY       // Track to leave weapons factory.
		};

		/****************************************************************************
		**	Smooth turning tracks are controlled by this structure and these
		**	processing bits.
		*/
		enum TrackControlType {
			F_=0x00,    // No translation necessary?
			F_T=0x01,   // Transpose X and Y components?
			F_X=0x02,   // Reverse X component sign?
			F_Y=0x04,   // Reverse Y component sign?
			F_D=0x08    // Two cell consumption?
		};

	private:

		struct TurnTrackType {
			char					Track;      // Which track to use.
			char					StartTrack; // Track when starting from stand-still.
			Dir256				Facing;         // Facing when track has been completed.
			TrackControlType	Flag;           // List processing flag bits.
		};

		struct TrackType {
			Point2D Offset;   // Offset to origin coordinate.
			Dir256	Facing;         // Facing (primary track).
		};

		struct RawTrackType {
			TrackType const * Track;    // Pointer to track list.
			int	Jump;                   // Index where track jumping is allowed.
			int	Entry;                  // Entry point if jumping to this track.
			int	Cell;                   // Per cell process should occur at this index.
		};

		/*
		 * These control the tilt of the vehicle's body. When it drives onto a new ramp, the
		 * ramp it was standing on is remembered and the timer is restarted, so that the body
		 * swings over the crest across a few frames instead of snapping to the new angle.
		 */
		int CurrentRamp;
		int PreviousRamp;
		ProgressTimerClass<FrameTimerClass> RampTimer;

		/*
		 * This is the coordinate that the unit has been ordered to drive to. It may lie any
		 * distance away -- the HeadToCoord below is the next step along the way to it. When
		 * this is COORD_NONE, the unit has nowhere it must be.
		 */
		Coord DestinationCoord;

		/*
		**	This is the coordinate that the unit is heading to
		**	as an immediate destination. This coordinate is never further
		**	than once cell (or track) from the unit's location. When this coordinate
		**	is reached, then the next location in the path list becomes the
		**	next HeadTo coordinate.
		*/
		Coord HeadToCoord;

		/*
		**	These speed values are used to accumulate movement and then
		**	convert them into pixel "steps" that are then translated through
		**	the currently running track so that the unit will move.
		*/
		int SpeedAccum;
		double TargetSpeed;

		/*
		**	This the track control logic (used for ground vehicles only). The 'Track'
		**	variable holds the track being followed (0 == not following track). The
		**	'TrackIndex' variable holds the current index into the specified track
		**	(starts at 0).
		*/
		int TrackNumber;
		int TrackIndex;

		/*
		**	This vehicle could be processing a "short track". A short track is one that
		**	doesn't actually go anywhere. Kind of like turning in place.
		*/
		bool IsOnShortTrack;

		/*
		**	Some units must have their turret locked down to face their body direction.
		**	When this flag is set, this condition is in effect. This flag is a more
		**	accurate check than examining the TrackNumber since the turret may be
		**	rotating into position so that a pending track may start. During this process
		**	the track number does not indicate anything.
		*/
		bool IsTurretLockedDown;

		/*
		**	This unit could be either rotating its body or rotating its turret. During the
		**	process of rotation, this flag is set. By examining this flag, unnecessary logic
		**	can be avoided.
		*/
		bool IsRotating;

		/*
		**	If this object is current driving to a short range destination, this flag is
		**	true. A short range destination is either the next cell or the end of the
		**	current "curvy" track. An object that is driving is not allowed to do anything
		**	else until it reaches its destination. The exception is when infantry wish to
		**	head to a different destination, they are allowed to start immediately.
		*/
		bool IsDriving;

		/*
		 * When this unit drives straight through (crushes) a crushable overlay
		 * rather than curving around it, this flag is set so that the unit will
		 * rock/tilt to indicate the impact.
		 */
		bool IsRocking;

		/*
		 * Is the locomotor unlocked thus can be deleted?
		 */
		bool IsLocomotorUnlocked;

		/*
		 * Pointer to the locomotor that has temporarily taken over the movement of this
		 * unit, such as the one that carries it through a tunnel. While one is riding, the
		 * driver answers for it rather than for itself. If NULL, this driver is in sole
		 * charge of the unit.
		 */
		std::unique_ptr<ILocomotion> Piggybacker;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		bool While_Moving(bool just_started=false);
		bool Start_Of_Move(bool & stop_processing, bool retry = true, bool force_straight = false);
		void Lay_Track(void);
		Point2D Smooth_Turn(Point2D const & adj, Dir256 & dir);

		static TurnTrackType const TrackControl[67];
		static RawTrackType const RawTracks[13];
		static TrackType const Track13[];
		static TrackType const Track12[];
		static TrackType const Track11[];
		static TrackType const Track10[];
		static TrackType const Track9[];
		static TrackType const Track8[];
		static TrackType const Track7[];
		static TrackType const Track6[];
		static TrackType const Track5[];
		static TrackType const Track4[];
		static TrackType const Track3[];
		static TrackType const Track2[];
		static TrackType const Track1[24];
};

