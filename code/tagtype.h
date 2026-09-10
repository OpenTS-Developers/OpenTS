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

#pragma once

#include "abstype.h"

#include "attach.hh"
#include "need.hh"
#include "persist.hh"

template<class T> class DynamicVectorClass;
class TagTypeClass;
class TriggerTypeClass;


class TagTypeClass : public AbstractTypeClass
{
		typedef AbstractTypeClass BASECLASS;
		friend class TagClass;

	public:
		TagTypeClass(char const * name = NULL);
		virtual ~TagTypeClass(void) override;

		virtual ClassID Class_ID(void) const override;

		static TagTypeClass * From_Name(char const * name);

		static TagTypeClass * Find_Or_Make(char const * ininame);

		static void Read_All(CCINIClass const & ini);
		static void Write_All(CCINIClass & ini);
		virtual bool Read_INI(CCINIClass const & ini) override;
		virtual bool Write_INI(CCINIClass & ini) const override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TAGTYPE);}
		virtual int Fetch_Heap_ID(void) const override {return(HeapID);};

		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual void Detach(AbstractClass const * target, bool all=true) override;

		AttachType Attaches_To(void) const;

		bool Is_Linked(TriggerTypeClass *trigger) const;
		bool Link(TriggerTypeClass *trigger);
		bool Unlink(TriggerTypeClass *trigger);

		bool Is_Cross_Horizontal(void) const;
		bool Is_Cross_Vertical(void) const;
		bool Is_Enters_Zone(void) const;
		bool Is_Allow_Win(void) const;

	public:
		/*
		 * This is this tag type's index within the master TagTypes list, taken as the type is
		 * created. It identifies the tag wherever a pointer will not serve.
		 */
		int HeapID;

	private:
		/*
		**	This flag controls whether the trigger destroys itself after it goes
		**	off.
		**	0 = trigger destroys itself immediately after going off, and removes
		**	    itself from all objects it's attached to
		**	1 = trigger is "Semi-Persistent"; it maintains a count of all objects
		**	    it's attached to, and only actually "springs" after its been
		**	    triggered from all the objects; then, it removes itself.
		**	2 = trigger is Fully Persistent; it just won't go away.
		*/
		PersistentType Persistence;

	public:
		/*
		 * This is the head of the list of trigger types this tag fires, chained together
		 * through each trigger's LinkedTo field. If NULL, then the tag has nothing to do
		 * when it springs.
		 */
		TriggerTypeClass * FirstTrigger;

	private:
		/*
		 * This is the name of the scenario INI section that lists every tag the map declares,
		 * each entry naming the tag's persistence, its display name and its first trigger.
		 */
		static char const * const INI_NAME;
};


extern DynamicVectorClass<TagTypeClass *> TagTypes;
