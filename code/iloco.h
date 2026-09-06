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
#include "face.h"
#include "matrix3d.h"
#include "point.h"

#include "fire.hh"
#include "layer.hh"
#include "move.hh"
#include "visual.hh"
#include "zgrad.hh"

#include <comdef.h>
#include <memory>

/// Names and comments from TLBs

EXTERN_C const IID IID_ILocomotion;

/*
 * Game object locomotion handler.
 */
MIDL_INTERFACE("070F3290-9841-11D1-B709-00A024DDAFD1")
ILocomotion : public IUnknown
{
public:
	/*
	 * Links object to locomotor.
	 */
	virtual HRESULT STDMETHODCALLTYPE Link_To_Object(void *pointer) = 0;

	/*
	 * Sees if object is moving.
	 */
	virtual boolean STDMETHODCALLTYPE Is_Moving(void) = 0;

	/*
	 * Fetches destination coordinate.
	 */
	virtual Coord STDMETHODCALLTYPE Destination(void) = 0;

	/*
	 * Fetches immediate (next cell) destination coordinate.
	 */
	virtual Coord STDMETHODCALLTYPE Head_To_Coord(void) = 0;

	/*
	 * Determine if specific cell can be entered.
	 */
	virtual MoveType STDMETHODCALLTYPE Can_Enter_Cell(Cell cell) = 0;

	/*
	 * Should object cast a shadow?
	 */
	virtual boolean STDMETHODCALLTYPE Is_To_Have_Shadow(void) = 0;

	/*
	 * Fetch voxel draw matrix.
	 */
	virtual Matrix3D STDMETHODCALLTYPE Draw_Matrix(int *key) = 0;

	/*
	 * Fetch shadow draw matrix.
	 */
	virtual Matrix3D STDMETHODCALLTYPE Shadow_Matrix(int *key) = 0;

	/*
	 * Draw point center location.
	 */
	virtual Point2D STDMETHODCALLTYPE Draw_Point(void) = 0;

	/*
	 * Shadow draw point center location.
	 */
	virtual Point2D STDMETHODCALLTYPE Shadow_Point(void) = 0;

	/*
	 * Visual character for drawing.
	 */
	virtual VisualType STDMETHODCALLTYPE Visual_Character(boolean flag) = 0;

	/*
	 * Z adjust control value.
	 */
	virtual int STDMETHODCALLTYPE Z_Adjust(void) = 0;

	/*
	 * Z gradient control value.
	 */
	virtual ZGradientType STDMETHODCALLTYPE Z_Gradient(void) = 0;

	/*
	 * Process movement of object.
	 */
	virtual boolean STDMETHODCALLTYPE Process(void) = 0;

	/*
	 * Instruct to move to location specified.
	 */
	virtual void STDMETHODCALLTYPE Move_To(Coord to) = 0;

	/*
	 * Stop moving at first opportunity.
	 */
	virtual void STDMETHODCALLTYPE Stop_Moving(void) = 0;

	/*
	 * Try to face direction specified.
	 */
	virtual void STDMETHODCALLTYPE Do_Turn(DirType coord) = 0;

	/*
	 * Object is appearing in the world.
	 */
	virtual void STDMETHODCALLTYPE Unlimbo(void) = 0;

	/*
	 * Special tilting AI function.
	 */
	virtual void STDMETHODCALLTYPE Tilt_Pitch_AI(void) = 0;

	/*
	 * Locomotor becomes powered.
	 */
	virtual boolean STDMETHODCALLTYPE Power_On(void) = 0;

	/*
	 * Locomotor loses power.
	 */
	virtual boolean STDMETHODCALLTYPE Power_Off(void) = 0;

	/*
	 * Is locomotor powered?
	 */
	virtual boolean STDMETHODCALLTYPE Is_Powered(void) = 0;

	/*
	 * Is locomotor sensitive to ion storms?
	 */
	virtual boolean STDMETHODCALLTYPE Is_Ion_Sensitive(void) = 0;

	/*
	 * Push object in direction specified.
	 */
	virtual boolean STDMETHODCALLTYPE Push(DirType dir) = 0;

	/*
	 * Shove object (with spin) in direction specified.
	 */
	virtual boolean STDMETHODCALLTYPE Shove(DirType dir) = 0;

	/*
	 * Force drive track -- special case only.
	 */
	virtual void STDMETHODCALLTYPE Force_Track(int track, Coord coord) = 0;

	/*
	 * What display layer is it located in.
	 */
	virtual LayerType STDMETHODCALLTYPE In_Which_Layer(void) = 0;

	/*
	 * Don't use this function.
	 */
	virtual void STDMETHODCALLTYPE Force_Immediate_Destination(Coord coord) = 0;

	/*
	 * Force a voxel unit to a given slope. Used in cratering.
	 */
	virtual void STDMETHODCALLTYPE Force_New_Slope(int ramp) = 0;

	/*
	 * Is it actually moving across the ground this very second?
	 */
	virtual boolean STDMETHODCALLTYPE Is_Moving_Now(void) = 0;

	/*
	 * Actual current speed of object expressed as leptons per game frame.
	 */
	virtual int STDMETHODCALLTYPE Apparent_Speed(void) = 0;

	/*
	 * Special drawing feedback code (locomotor specific meaning)
	 */
	virtual int STDMETHODCALLTYPE Drawing_Code(void) = 0;

	/*
	 * Queries if any locomotor specific state prevents the object from firing.
	 */
	virtual FireErrorType STDMETHODCALLTYPE Can_Fire(void) = 0;

	/*
	 * Queries the general state of the locomotor.
	 */
	virtual int STDMETHODCALLTYPE Get_Status(void) = 0;

	/*
	 * Forces a hunter seeker droid to find a target.
	 */
	virtual void STDMETHODCALLTYPE Acquire_Hunter_Seeker_Target(void) = 0;

	/*
	 * Is this object surfacing?
	 */
	virtual boolean STDMETHODCALLTYPE Is_Surfacing(void) = 0;

	/*
	 * Lifts all occupation bits associated with the object off the map
	 */
	virtual void STDMETHODCALLTYPE Mark_All_Occupation_Bits(int mark) = 0;

	/*
	 * Is this object in the process of moving into this coord.
	 */
	virtual boolean STDMETHODCALLTYPE Is_Moving_Here(Coord to) = 0;

	/*
	 * Will this object jump tracks?
	 */
	virtual boolean STDMETHODCALLTYPE Will_Jump_Tracks(void) = 0;

	/*
	 * Infantry moving query function
	 */
	virtual boolean STDMETHODCALLTYPE Is_Really_Moving_Now(void) = 0;

	/*
	 * Falsifies the IsReallyMoving flag in WalkLocomotionClass
	 */
	virtual void STDMETHODCALLTYPE Stop_Movement_Animation(void) = 0;

	/*
	 * Locks the locomotor from being deleted
	 */
	virtual void STDMETHODCALLTYPE Lock(void) = 0;

	/*
	 * Unlocks the locomotor from being deleted
	 */
	virtual void STDMETHODCALLTYPE Unlock(void) = 0;

	/*
	 * Queries internal variables
	 */
	virtual int STDMETHODCALLTYPE Get_Track_Number(void) = 0;

	/*
	 * Queries internal variables
	 */
	virtual int STDMETHODCALLTYPE Get_Track_Index(void) = 0;

	/*
	 * Queries internal variables
	 */
	virtual int STDMETHODCALLTYPE Get_Speed_Accum(void) = 0;
};

/*
 * ILocomtion com smart pointer declaration.
 */
_COM_SMARTPTR_TYPEDEF(ILocomotion, __uuidof(ILocomotion));
