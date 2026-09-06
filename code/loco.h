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
#include "ilocos.h"
#include "persist.h"

class FootClass;
class SaveStreamClass;

// The class identifier of a locomotor reached through its locomotion interface, or
// CLSID_NULL when it is not one of ours.
CLSID Locomotion_Class_ID(ILocomotion * locomotion);

// A new, unlinked locomotor of the registered class, or nothing when the identifier
// names no locomotor.
std::unique_ptr<ILocomotion> Create_Locomotor(CLSID const & classid);

// The locomotor whose record is next in the stream, or nothing when the record names
// something that is not one, which fails the stream.
std::unique_ptr<ILocomotion> Load_Locomotor(SaveStreamClass & stream);


class LocomotionClass : public IPersistent, public ILocomotion
{
	public:
		LocomotionClass(void);
		virtual ~LocomotionClass(void);

		virtual HRESULT Load(SaveStreamClass & stream) override;
		virtual HRESULT Save(SaveStreamClass & stream, BOOL cleardirty) override;

		virtual HRESULT Link_To_Object(void *object) override;
		virtual bool Is_Moving(void) override;
		virtual Coord Destination(void) override;
		virtual Coord Head_To_Coord(void) override;
		virtual MoveType Can_Enter_Cell(Cell cell) override;
		virtual bool Is_To_Have_Shadow(void) override;
		virtual Matrix3D Draw_Matrix(int *key) override;
		virtual Matrix3D Shadow_Matrix(int *key) override;
		virtual Point2D Draw_Point(void) override;
		virtual Point2D Shadow_Point(void) override;
		virtual VisualType Visual_Character(bool flag) override;
		virtual int Z_Adjust(void) override;
		virtual ZGradientType Z_Gradient(void) override;
		virtual bool Process(void) override;
		virtual void Move_To(Coord to) override;
		virtual void Stop_Moving(void) override;
		virtual void Do_Turn(DirType coord) override;
		virtual void Unlimbo(void) override;
		virtual void Tilt_Pitch_AI(void) override;
		virtual bool Power_On(void) override;
		virtual bool Power_Off(void) override;
		virtual bool Is_Powered(void) override;
		virtual bool Is_Ion_Sensitive(void) override;
		virtual bool Push(DirType dir) override;
		virtual bool Shove(DirType dir) override;
		virtual void Force_Track(int track, Coord coord) override;
		virtual void Force_Immediate_Destination(Coord coord) override;
		virtual void Force_New_Slope(int ramp) override;
		virtual bool Is_Moving_Now(void) override {return(Is_Moving());}
		virtual int Apparent_Speed(void) override;
		virtual int Drawing_Code(void) override;
		virtual FireErrorType Can_Fire(void) override;
		virtual int Get_Status() override {return(0);}
		virtual void Acquire_Hunter_Seeker_Target(void) override {}
		virtual bool Is_Surfacing() override {return(false);}
		virtual void Mark_All_Occupation_Bits(int mark) override {}
		virtual bool Is_Moving_Here(Coord to) override {return(false);}
		virtual bool Will_Jump_Tracks(void) override {return(false);}
		virtual bool Is_Really_Moving_Now(void) override {return(Is_Moving_Now());}
		virtual void Stop_Movement_Animation(void) override {}
		virtual void Lock(void) override {}
		virtual void Unlock(void) override {}
		virtual int Get_Track_Number(void) override {return(-1);}
		virtual int Get_Track_Index(void) override {return(-1);}
		virtual int Get_Speed_Accum(void) override {return(-1);}


		/*
		 * Lists this locomotor's members for the save game. An implementation serializes
		 * its base class first and then names every member it owns in the order the header
		 * declares them, so that the same description serves saving and loading.
		 */
		virtual void Serialize(SaveStreamClass & stream);

		/*
		 * Restores whatever the record could not carry. Load_Object calls this once the
		 * record has been checked, so a locomotor never takes its place while its record
		 * is still in doubt.
		 */
		virtual void Post_Load(void);

	protected:

		/*
		 * These carry the record a class describes through Serialize. A class calls these
		 * from its Load and Save; the record is the swizzle identity followed by whatever
		 * members the class names.
		 */
		HRESULT Save_Members(SaveStreamClass & stream, BOOL cleardirty);
		HRESULT Load_Members(SaveStreamClass & stream);

	protected:
		/*
		 * Pointer to the object this locomotor carries about. It is attached as the object
		 * is created, and every service the locomotor offers is performed through it.
		 */
		FootClass *LinkedTo;

		/*
		 * If this locomotor is able to move its object under its own means, then this flag
		 * will be true. It is cleared when an EM pulse or a loss of base power is to strand
		 * the object where it stands.
		 */
		bool IsPowered;

		/*
		 * If this locomotor has changed since it was last written out, then this flag will
		 * be true. It starts out set and is only cleared by a save that asks for it, so the
		 * persistence machinery never assumes a locomotor is already safely on disk.
		 */
		bool Dirty;
};
