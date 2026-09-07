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
#include "vector.h"

class BrainClass;

class NeuronClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

		friend class BrainClass;

	public:
		NeuronClass(void);
		virtual ~NeuronClass(void) override;

		virtual ClassID Class_ID(void) const override;

		virtual RTTIType Fetch_RTTI(void) const override { return(RTTI_NEURON); }

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual void Compute_CRC(CRCEngine &) const override;

	private:
		/// Unused, previous and next neuron?
		void * Pointer1;
		void * Pointer2;

		/*
		 * This points back to the brain that has taken this neuron on, recorded as the
		 * neuron is added to it.
		 */
		BrainClass * MyBrain;

		/*
		 * This is the game frame that this neuron came into being on. The age it yields is
		 * the only thing a neuron contributes to the multiplayer sync check.
		 */
		int CreationFrame;

		/// Unused
		int Unk1;
};

class BrainClass
{
	public:
		virtual ~BrainClass(void);

		void Deinit(void);
		void Init(int min, int max);
		bool Add_Neuron(NeuronClass *neuron);

		HRESULT Load(SaveStreamClass & stream);
		HRESULT Save(SaveStreamClass & stream, BOOL cleardirty);

		void Serialize(SaveStreamClass & stream, BOOL cleardirty = FALSE);

	private:
		/*
		 * This is the list of neurons that this brain has taken on. The brain owns them
		 * outright, so they are destroyed along with it.
		 */
		DynamicVectorClass<NeuronClass *> Neurons;

		/*
		 * These are the fewest and the most neurons that this brain is meant to hold, fixed
		 * when the brain is prepared for use. Only the ceiling is enforced -- a neuron
		 * offered to a brain that has filled up is turned away.
		 */
		int MinCount;
		int MaxCount;

};
