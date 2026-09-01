/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "loco.h"

#include "_map.h"
#include "_tactica.h"
#include "cell.h"
#include "coord.h"
#include "foot.h"
#include "globals.h"
#include "map.h"
#include "savestream.h"
#include "swizzle.h"
#include "tactical.h"

#include "fire.hh"
#include "move.hh"
#include "visual.hh"
#include "zgrad.hh"

#include <cassert>

extern ULONG COMRefCount;


/// <summary>
/// Constructs a locomotor that is not yet attached to anything.
/// The locomotor comes up powered and with no object to drive.
/// </summary>
/// <remarks>Link_To_Object must be called before the locomotor can be asked to do anything
/// useful.</remarks>
LocomotionClass::LocomotionClass(void) :
	LinkedTo(NULL),
	IsPowered(true),
	Dirty(true),
	RefCount(0)
{
}


/// <summary>
/// Destroys the locomotor and severs the link to the object it drove.
/// </summary>
LocomotionClass::~LocomotionClass(void)
{
	LinkedTo = NULL;
}


/// <summary>
/// Attaches this locomotor to the object it is to drive.
/// This is called as the object is being created, and every other service the locomotor
/// offers depends on it having been called first.
/// </summary>
/// <param name="pointer">Pointer to the foot class object this locomotor will carry about.</param>
/// <returns>Returns with S_OK, since the attachment cannot fail.</returns>
HRESULT STDMETHODCALLTYPE LocomotionClass::Link_To_Object(void *pointer)
{
	LinkedTo = (FootClass *)pointer;
	return(S_OK);
}


/// <summary>
/// Builds the transform matrix to draw the object with.
/// The base locomotor merely turns the object to its body facing, snapped to one of the
/// facings the voxel art is drawn at. Locomotors that tilt, bank, or bob their object
/// will override this routine.
/// </summary>
/// <param name="key">Optional cache key for the voxel renderer, which the facing is folded
/// into. May be NULL, and a key of -1 means the drawing is not to be cached.</param>
/// <returns>Returns with the matrix to transform the object by.</returns>
Matrix3D STDMETHODCALLTYPE LocomotionClass::Draw_Matrix(int *key)
{
	Matrix3D draw_matrix(true);

	draw_matrix.Rotate_Z(LinkedTo->PrimaryFacing.Current().As_Radian32());

	if (key && *key != -1) {
		*key *= 32;
		*key |= LinkedTo->PrimaryFacing.Current().As_Dir32();
	}
	return(draw_matrix);
}


/// <summary>
/// Builds the transform matrix to draw the object's shadow with.
/// A shadow lies flat on the ground, so the object's facing is applied on top of the
/// slope of the cell it is currently over.
/// </summary>
/// <param name="key">Optional cache key for the voxel renderer, which the slope and facing are
/// folded into. May be NULL, and a key of -1 means the shadow is not to be cached.</param>
/// <returns>Returns with the matrix to transform the shadow by.</returns>
Matrix3D STDMETHODCALLTYPE LocomotionClass::Shadow_Matrix(int *key)
{
	int ramp = Map[LinkedTo->Get_Coord()].Ramp;

	Matrix3D draw_matrix(Get_Slope_Matrix(ramp));
	draw_matrix.Rotate_Z(LinkedTo->PrimaryFacing.Current().As_Radian32());

	if (key && *key != -1) {
		*key = 32 * (ramp + (*key << 6));
		*key |= LinkedTo->PrimaryFacing.Current().As_Dir32();
	}
	return(draw_matrix);
}


/// <summary>
/// Fetches the pixel offset to draw the object's shadow at.
/// The shadow belongs on the ground rather than with the object, so the offset carries it
/// down by however far the object is flying above the terrain.
/// </summary>
/// <returns>Returns with the pixel offset to shift the shadow by when drawing.</returns>
Point2D STDMETHODCALLTYPE LocomotionClass::Shadow_Point(void)
{
	Point2D pt;

	pt.X = 0;
	pt.Y = TacticalMap->Z_Lepton_To_Pixel(LinkedTo->HeightAGL);

	return(pt);
}


/// <summary>
/// Restores power to the locomotor.
/// This undoes an earlier Power_Off and lets the object travel under its own means once
/// more.
/// </summary>
/// <returns>bool; Is the locomotor powered after the change?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Power_On(void)
{
	IsPowered = true;
	return(Is_Powered());
}


/// <summary>
/// Removes power from the locomotor.
/// This routine is used to strand the object where it stands, such as when it is caught
/// by an EMP pulse or its owner loses base power.
/// </summary>
/// <returns>bool; Is the locomotor powered after the change?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Power_Off(void)
{
	IsPowered = false;
	return(Is_Powered());
}


/// <summary>
/// Does the locomotor currently have power?
/// An unpowered locomotor will not carry its object anywhere, which is how an EMP or a
/// loss of base power leaves a unit stranded.
/// </summary>
/// <returns>bool; Is the locomotor powered?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Is_Powered(void)
{
	return(IsPowered);
}


/// <summary>
/// Is this locomotor disrupted by an ion storm?
/// Locomotors that rely on flight or some other fragile means of travel will override
/// this routine so that a storm can bring their objects down.
/// </summary>
/// <returns>bool; Is the locomotor sensitive to ion storms?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Is_Ion_Sensitive(void)
{
	return(false);
}


/// <summary>
/// Adds a reference to this locomotor.
/// Anything that holds on to a locomotor takes a reference first, which keeps the
/// locomotor alive until that holder releases it again.
/// </summary>
/// <returns>Returns with the number of references now outstanding.</returns>
ULONG STDMETHODCALLTYPE LocomotionClass::AddRef(void)
{
	++COMRefCount;
	return(InterlockedIncrement(&RefCount));
}


/// <summary>
/// Releases a reference to this locomotor.
/// When the last reference goes away the locomotor destroys itself, so the caller must
/// not touch its pointer afterward.
/// </summary>
/// <returns>Returns with the number of references still outstanding.</returns>
ULONG STDMETHODCALLTYPE LocomotionClass::Release(void)
{
	--COMRefCount;

	ULONG count = InterlockedDecrement(&RefCount);
	if (count == 0) {
		delete this;
	}
	return(count);
}


/// <summary>
/// Fetches one of the interfaces this locomotor implements.
/// A locomotor answers to IUnknown, IPersist, IPersistStream, and ILocomotion. Any other
/// interface asked for is refused.
/// </summary>
/// <param name="riid">The identifier of the interface being asked for.</param>
/// <param name="ppvObject">Pointer to the location to store the interface pointer in.</param>
/// <returns>Returns with S_OK, or E_NOINTERFACE if the interface is not supported.</returns>
/// <remarks>An interface fetched successfully carries a reference. The caller must release
/// it when finished with it.</remarks>
LONG STDMETHODCALLTYPE LocomotionClass::QueryInterface(REFIID riid, LPVOID *ppvObject)
{
	if (ppvObject == NULL) {
		return(E_POINTER);
	}

	*ppvObject = NULL;

	if (riid == IID_IUnknown) {
		*ppvObject = (IUnknown *)(ILocomotion *)this;
	}
	if (riid == IID_IPersistStream) {
		*ppvObject = (IPersistStream *)this;
	}
	if (riid == IID_ILocomotion) {
		*ppvObject = (ILocomotion *)this;
	}
	if (riid == IID_IPersist) {
		*ppvObject = (IPersist *)this;
	}
	if (*ppvObject == NULL) {
		return(E_NOINTERFACE);
	}

	AddRef();
	return(S_OK);
}


/// <summary>
/// Saves the locomotor out to a save game stream.
/// The locomotor's address is written ahead of its data, which is what lets the swizzle
/// manager remap every pointer to it when the game is loaded again.
/// </summary>
/// <param name="cleardirty">Should the locomotor be marked as no longer needing a save?</param>
/// <returns>Returns with the result of the write, or E_POINTER if no stream was supplied.</returns>
HRESULT STDMETHODCALLTYPE LocomotionClass::Save(IStream * stream, BOOL cleardirty)
{
	if (stream == NULL) {
		return(E_POINTER); /// E_INVALIDARG
	}

	return(Save_Members(stream, cleardirty));
}


/// <summary>
/// Loads the locomotor back from a save game stream.
/// The locomotor announces its new address to the swizzle manager before its data is
/// read in, so that every saved pointer to it can be remapped and its link back to the
/// object it drives can be restored. The reference count belongs to the running session
/// rather than to the saved state, so it survives the load untouched.
/// </summary>
/// <param name="stream">The stream to read the locomotor back from.</param>
/// <returns>Returns with the result of the read, or E_POINTER if no stream was supplied.</returns>
HRESULT STDMETHODCALLTYPE LocomotionClass::Load(IStream * stream)
{
	if (stream == NULL) {
		return(E_POINTER); /// E_INVALIDARG
	}

	return(Load_Members(stream));
}


/// <summary>
/// Writes the members this locomotor describes out to the save stream.
/// The locomotor's address goes out first as its swizzle identity, and the members follow
/// in the order Serialize names them.
/// </summary>
/// <param name="stream">The stream to write to.</param>
/// <param name="cleardirty">Should the locomotor be marked clean once it has been written?</param>
/// <returns>Returns with S_OK when the record was written, otherwise a failure code.</returns>
HRESULT LocomotionClass::Save_Members(IStream * stream, BOOL cleardirty)
{
	if (stream == NULL) {
		return(E_POINTER);
	}

	uintptr_t id = (uintptr_t)(this);

	HRESULT result = stream->Write(&id, sizeof(id), NULL);
	if (FAILED(result)) {
		return(result);
	}

	SaveStreamClass savestream(stream, SaveStreamClass::MODE_SAVE);
	Serialize(savestream);

	if (SUCCEEDED(savestream.Result()) && cleardirty) {
		Dirty = false;
	}

	return(savestream.Result());
}


/// <summary>
/// Reads the members this locomotor describes back from the save stream.
/// The saved address is handed to the swizzle system so that pointers elsewhere in the
/// save game can be remapped onto this locomotor, and the members follow.
/// </summary>
/// <param name="stream">The stream to read from.</param>
/// <returns>Returns with S_OK when the record was read, otherwise a failure code.</returns>
HRESULT LocomotionClass::Load_Members(IStream * stream)
{
	if (stream == NULL) {
		return(E_POINTER);
	}

	uintptr_t id;

	HRESULT result = stream->Read(&id, sizeof(id), NULL);
	if (FAILED(result)) {
		return(result);
	}

	assert(id != 0);
	Swizzle_Here_I_Am(id, this);

	SaveStreamClass savestream(stream, SaveStreamClass::MODE_LOAD);
	savestream.Set_Context(typeid(*this).name(), id);
	Serialize(savestream);

	if (SUCCEEDED(savestream.Result())) {
		Post_Load();
	}

	return(savestream.Result());
}


/// <summary>
/// Lists the members every locomotor carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void LocomotionClass::Serialize(SaveStreamClass & stream)
{
	stream.Serialize(LinkedTo);
	stream.Serialize(IsPowered);
	stream.Serialize(Dirty);

	// RefCount -- belongs to the running session rather than the record.
}


/// <summary>
/// Restores the state a locomotor could not carry in its record.
/// The bare locomotor carries nothing of the sort, so there is nothing to do.
/// </summary>
void LocomotionClass::Post_Load(void)
{
}


/// <summary>
/// Fetches the number of bytes needed to save this locomotor.
/// A record is as long as the members a class names, so the count is not known before
/// the members have been written. Nothing in the game asks for it, so rather than
/// walk the locomotor twice this reports that the size cannot be supplied.
/// </summary>
/// <param name="pcbSize">Pointer to the value to fill in with the required byte count.</param>
/// <returns>Returns with E_NOTIMPL.</returns>
LONG STDMETHODCALLTYPE LocomotionClass::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
	return(E_NOTIMPL);
}


/// <summary>
/// Asks the object to step out of the way in the direction specified.
/// This routine is used when another object needs the cell this one happens to be
/// occupying. The base locomotor cannot be moved and declines.
/// </summary>
/// <returns>bool; Did the object step out of the way?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Push(DirType dir)
{
	return(false);
}


/// <summary>
/// Shoves the object aside in the direction specified.
/// This is the more forceful companion to Push, used when a loitering object must be
/// displaced to clear the way. The base locomotor will not budge.
/// </summary>
/// <returns>bool; Was the object shoved out of the way?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Shove(DirType dir)
{
	return(false);
}


/// <summary>
/// Handles the per frame settling of the object's tilt and pitch.
/// Locomotors that rock their object about -- over bumps, on landing, or when it takes a
/// hit -- use this routine to ease the body back toward level.
/// </summary>
void STDMETHODCALLTYPE LocomotionClass::Tilt_Pitch_AI(void)
{
}


/// <summary>
/// Fetches the depth bias to draw the object with.
/// This nudges the object forward or back in the depth buffer so that it sorts correctly
/// against the terrain it is traveling over. The base locomotor needs no such favor.
/// </summary>
/// <returns>Returns with the depth adjustment to apply when drawing the object.</returns>
int STDMETHODCALLTYPE LocomotionClass::Z_Adjust(void)
{
	return(0);
}


/// <summary>
/// Fetches the Z gradient to draw the object with.
/// The gradient tells the depth shaded renderer how the object's depth varies across its
/// shape. The base locomotor reports the upright case.
/// </summary>
/// <returns>Returns with the Z gradient to render the object with.</returns>
ZGradientType STDMETHODCALLTYPE LocomotionClass::Z_Gradient(void)
{
	return(ZGRAD_90DEG);
}


/// <summary>
/// Fetches the visual character to draw the object with.
/// This is how a locomotor fades or hides its object as it burrows, submerges, or
/// otherwise leaves plain sight. The base locomotor never alters the appearance.
/// </summary>
/// <returns>Returns with the visual character to render the object with.</returns>
VisualType STDMETHODCALLTYPE LocomotionClass::Visual_Character(boolean flag)
{
	return(VISUAL_NORMAL);
}


/// <summary>
/// Fetches the pixel offset to draw the object at.
/// The render code adds this to the object's normal screen position, which is how a
/// locomotor makes its object bob, hop, or sink. The base locomotor draws in place.
/// </summary>
/// <returns>Returns with the pixel offset to shift the object by when drawing.</returns>
Point2D STDMETHODCALLTYPE LocomotionClass::Draw_Point(void)
{
	Point2D pt;
	pt.X = 0;
	pt.Y = 0;
	return(pt);
}


/// <summary>
/// Should the object cast a shadow?
/// Locomotors that take their object out of the light -- underground, submerged, or
/// otherwise hidden -- will override this routine to suppress the shadow.
/// </summary>
/// <returns>bool; Should a shadow be drawn for the object?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Is_To_Have_Shadow(void)
{
	return(true);
}


/// <summary>
/// Determines if the object may move into the cell specified.
/// The movement code asks the locomotor rather than the object, since what counts as
/// passable depends entirely on how the object travels. The base locomotor is
/// unrestricted and welcomes every cell.
/// </summary>
/// <returns>Returns with the move legality of the cell.</returns>
MoveType STDMETHODCALLTYPE LocomotionClass::Can_Enter_Cell(Cell cell)
{
	return(MOVE_OK);
}


/// <summary>
/// Forces the object's immediate destination to the coordinate specified.
/// This overrides whatever short term move the locomotor had in mind, and is used when
/// outside code must dictate exactly where the object ends up next.
/// </summary>
/// <param name="coord">The coordinate the object should head to immediately.</param>
void STDMETHODCALLTYPE LocomotionClass::Force_Immediate_Destination(Coord coord)
{
}


/// <summary>
/// Forces the object onto a specific movement track.
/// This routine is used when outside code must dictate the exact motion the object
/// performs, rather than letting the locomotor plan one for itself.
/// </summary>
/// <param name="track">The track number the object should be placed onto.</param>
/// <param name="coord">The coordinate to treat as the start of the track.</param>
void STDMETHODCALLTYPE LocomotionClass::Force_Track(int track, Coord coord)
{
}


/// <summary>
/// Notifies the locomotor that its object has been placed onto the map.
/// This gives derived locomotors their chance to pick up a starting facing, slope, or
/// altitude from the ground the object has just arrived on.
/// </summary>
void STDMETHODCALLTYPE LocomotionClass::Unlimbo(void)
{
}


/// <summary>
/// Turns the object to face the direction specified.
/// The base locomotor has no body of its own to rotate, so the request goes unheeded.
/// </summary>
/// <param name="coord">The direction that the object should come to face.</param>
void STDMETHODCALLTYPE LocomotionClass::Do_Turn(DirType coord)
{
}


/// <summary>
/// Halts the object's travel.
/// This routine is called when the object must give up on wherever it was going. Derived
/// locomotors use it to abandon their journey and bring the object to a legal rest.
/// </summary>
void STDMETHODCALLTYPE LocomotionClass::Stop_Moving(void)
{
}


/// <summary>
/// Commands the locomotor to carry the object to the coordinate specified.
/// This is how the object hands its locomotor a new place to go. The base locomotor
/// cannot move anything, so the request is quietly ignored.
/// </summary>
void STDMETHODCALLTYPE LocomotionClass::Move_To(Coord to)
{
}


/// <summary>
/// Performs one game frame of locomotion for the object.
/// The linked object calls this from its own AI so that the locomotor can carry it
/// further along its travel. The base locomotor has nothing to carry and reports that it
/// is already at rest.
/// </summary>
/// <returns>bool; Is the locomotor at rest, with nothing further to do?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Process(void)
{
	return(true);
}


/// <summary>
/// Fetches the final destination of the object's travel.
/// The base locomotor never has a journey pending, so it reports that there is no
/// destination at all.
/// </summary>
/// <returns>Returns with the destination coordinate, or COORD_NONE if there is none.</returns>
Coord STDMETHODCALLTYPE LocomotionClass::Destination(void)
{
	Coord coord;
	coord.X = COORD_NONE.X;
	coord.Y = COORD_NONE.Y;
	coord.Z = COORD_NONE.Z;
	return(coord);
}


/// <summary>
/// Fetches the coordinate the object is immediately heading toward.
/// This is the next step of the journey rather than its end. Since the base locomotor
/// has nowhere to go, it reports the object's own position.
/// </summary>
/// <returns>Returns with the coordinate currently being moved toward.</returns>
Coord STDMETHODCALLTYPE LocomotionClass::Head_To_Coord(void)
{
	return(LinkedTo->PositionCoord);
}


/// <summary>
/// Is the object currently in motion?
/// The mission and display code uses this to tell a traveling object from a parked one.
/// The base locomotor never carries its object anywhere, so it always answers no.
/// </summary>
/// <returns>bool; Is the object moving?</returns>
boolean STDMETHODCALLTYPE LocomotionClass::Is_Moving(void)
{
	return(false);
}


/// <summary>
/// Forces the object onto a new ground slope.
/// This routine is called when the terrain beneath the object changes out from under it.
/// Only locomotors that tilt their object with the ground need to act on it.
/// </summary>
/// <param name="ramp">The ramp type of the slope the object should now conform to.</param>
void STDMETHODCALLTYPE LocomotionClass::Force_New_Slope(int ramp)
{
}


/// <summary>
/// Fetches the drawing code the locomotor wants the object rendered with.
/// The render code uses this to pick a presentation that suits how the object is
/// currently traveling. The base locomotor has no preference.
/// </summary>
/// <returns>Returns with the drawing code, or zero for the ordinary presentation.</returns>
int STDMETHODCALLTYPE LocomotionClass::Drawing_Code(void)
{
	return(0);
}


/// <summary>
/// Determines if the locomotor will allow the object to fire.
/// Locomotors that must settle, land, or surface before a weapon can be brought to bear
/// will override this routine. The base locomotor never stands in the way.
/// </summary>
/// <returns>Returns with the reason firing is disallowed, or FIRE_OK if it is permitted.</returns>
FireErrorType STDMETHODCALLTYPE LocomotionClass::Can_Fire(void)
{
	return(FIRE_OK);
}


/// <summary>
/// Fetches the speed that the object appears to be traveling at.
/// This is the plain answer -- the linked object's own current speed. Locomotors whose
/// visible motion differs from the object's logical speed will override this routine.
/// </summary>
/// <returns>Returns with the apparent speed of the linked object.</returns>
int STDMETHODCALLTYPE LocomotionClass::Apparent_Speed(void)
{
	return(LinkedTo->Current_Speed());
}


/// Unlike the other interface identifiers, this one is defined in the locomotion module.
#define INITGUID
#undef DEFINE_GUID
#include <basetyps.h>
#include "iloco_i.c"
