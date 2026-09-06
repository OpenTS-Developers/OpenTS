/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#define INCLUDE_COM
#include "always.h"

#include "brain.h"

#include "crc.h"
#include "ftimer.h"
#include "globals.h"
#include "savestream.h"
#include "sun.h"
#include "tracker.h"
#include "vector.h"


/// <summary>
/// Creates a neuron.
/// The neuron notes the frame it came into being on and starts out unattached -- a brain
/// adopts it later. It registers itself with the object trackers so that it can be
/// detached cleanly when the things it refers to go away.
/// </summary>
NeuronClass::NeuronClass(void):
	Pointer1(NULL),
	Pointer2(NULL),
	MyBrain(NULL),
	CreationFrame(Frame)
{
	TeamPtrTracker.Add(this);
	AbstractTypePtrTracker.Add(this);
}


/// <summary>
/// Removes this neuron from the object trackers.
/// </summary>
NeuronClass::~NeuronClass(void)
{
	AbstractTypePtrTracker.Delete(this);
	TeamPtrTracker.Delete(this);
}


/// <summary>
/// Fetches the class identifier of this object.
/// This routine is used by the save game system so that it knows what kind of object to
/// construct when the stream is read back in.
/// </summary>
/// <param name="retval">Pointer to the place to store the class identifier.</param>
/// <returns>Returns with S_OK, or E_POINTER if no destination was supplied.</returns>
HRESULT NeuronClass::GetClassID(CLSID * retval)
{
	if (retval == NULL) return(E_POINTER);
	*retval = CLSID_NeuronClass;
	return(S_OK);
}


/// <summary>
/// Lists the members this neuron carries.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
void NeuronClass::Serialize(SaveStreamClass & stream)
{
	BASECLASS::Serialize(stream);

	// Pointer1 -- untyped, so they carry no swizzle identity, and nothing reads them.
	// Pointer2
	// MyBrain -- a brain carries no swizzle identity of its own, and sets this again as it adopts
	// the neuron on the way back in.
	stream.Serialize(CreationFrame);
	stream.Serialize(Unk1);
}


/// <summary>
/// Folds this neuron's state into a running CRC.
/// The multiplayer sync check accumulates every object's state each frame; a CRC that
/// differs between machines means the simulations have drifted apart. A neuron
/// contributes its age, since its pointers are meaningless across machines.
/// </summary>
/// <param name="crc">The running CRC to fold this neuron's state into.</param>
void NeuronClass::Compute_CRC(CRCEngine &crc) const
{
	BASECLASS::Compute_CRC(crc);
	crc((int)(Frame - CreationFrame));
}


/// <summary>
/// Destroys the brain along with every neuron it owns.
/// </summary>
BrainClass::~BrainClass(void)
{
	Deinit();
}


/// <summary>
/// Destroys every neuron this brain owns.
/// The brain itself survives and can be filled again afterwards.
/// </summary>
void BrainClass::Deinit(void)
{
	int count = Neurons.Count();
	for (int i = 0; i < count; i++) {
		delete Neurons[i];
		Neurons[i] = NULL;
	}

	Neurons.Clear();
}


/// <summary>
/// Prepares the brain to hold neurons.
/// Any neurons the brain was already holding are thrown away before the new limits take
/// effect, so this routine can be used to start a brain over from scratch.
/// </summary>
/// <param name="min">The fewest neurons this brain is meant to hold.</param>
/// <param name="max">The most neurons this brain will accept.</param>
void BrainClass::Init(int min, int max)
{
	Deinit();
	MinCount = min;
	MaxCount = max;
}


/// <summary>
/// Adds a neuron to this brain.
/// The neuron is taken on only if the brain still has room for it, and it is told which
/// brain now owns it. A neuron that is turned away is left in the caller's hands.
/// </summary>
/// <returns>bool; Was the neuron taken on?</returns>
bool BrainClass::Add_Neuron(NeuronClass *neuron)
{
	if (Neurons.Count() < MaxCount) {
		Neurons.Add(neuron);
		neuron->MyBrain = this;
		return(true);
	}
	return(false);
}


/// <summary>
/// Saves this brain to the save game stream.
/// </summary>
/// <param name="cleardirty">Should the neurons be marked clean once they are written?</param>
/// <returns>
/// Returns with S_OK when the brain was written, E_POINTER when no stream was supplied,
/// or the stream's own failure code.
/// </returns>
HRESULT BrainClass::Save(SaveStreamClass & stream, BOOL cleardirty)
{

	Serialize(stream, cleardirty);
	return(stream.Result());
}


/// <summary>
/// Reads this brain back from the save game stream.
/// Whatever neurons the brain was holding are destroyed first, so the stream's neurons
/// entirely replace them.
/// </summary>
/// <returns>
/// Returns with S_OK when the brain was read, E_POINTER when no stream was supplied, or
/// the stream's own failure code.
/// </returns>
HRESULT BrainClass::Load(SaveStreamClass & stream)
{

	stream.Set_Context("BrainClass");
	Serialize(stream);
	return(stream.Result());
}


/// <summary>
/// Lists the members this brain carries.
/// The neurons are owned outright rather than shared, so each is written into the stream
/// whole instead of travelling as a pointer.
/// </summary>
/// <param name="stream">The stream carrying the members.</param>
/// <param name="cleardirty">Should the neurons be marked clean once they are written?</param>
void BrainClass::Serialize(SaveStreamClass & stream, BOOL cleardirty)
{
	int count = Neurons.Count();
	stream.Serialize(count);

	if (stream.Is_Loading()) {
		Deinit();
	}

	for (int i = 0; i < count && !stream.Was_Error(); i++) {
		if (stream.Is_Loading()) {
			NeuronClass * neuron = new NeuronClass;
			neuron->Load(stream);
			Add_Neuron(neuron);
		} else {
			Neurons[i]->Save(stream, cleardirty);
		}
	}
	// MinCount -- the limits a brain was prepared with rather than anything it accumulated.
	// MaxCount
}
