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
#include "coord.h"

#include "tevent.hh"

template<class T> class DynamicVectorClass;
class TagClass;
class TagTypeClass;
class TriggerClass;
class ObjectClass;
class TechnoClass;

class TagClass : public AbstractClass
{
		typedef AbstractClass BASECLASS;

	public:
		TagClass(TagTypeClass * type=NULL);
		virtual ~TagClass(void) override;

		virtual HRESULT STDMETHODCALLTYPE GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;

		virtual RTTIType Fetch_RTTI(void) const override {return(RTTI_TAG);}
		virtual void Compute_CRC(CRCEngine & crc) const override;
		virtual void Detach(AbstractClass const * target, bool all=true) override;

		void Mark_To_Delete(void);
		bool Is_Marked_To_Delete(void) const;

		bool Is_Cross_Horizontal(void) const;
		bool Is_Cross_Vertical(void) const;
		bool Is_Enters_Zone(void) const;
		bool Is_Allow_Win(void) const;

		bool Is_To_Inherit(void) const;

		void Timer_Global_Reset(int global);
		void Timer_Local_Reset(int local);

		bool Is_Trigger_Attached(TriggerClass * trig) const;
		bool Spring(TEventType event=TEVENT_ANY, ObjectClass * object=NULL, Cell cell=CELL_NONE, bool forced=false, TechnoClass *source=NULL);

		Cell Get_Position(void) const;
		void Set_Position(Cell cell);

		static void All_Timer_Global_Reset(int global);
		static void All_Timer_Local_Reset(int local);

		static void Delete_All(void);

		void Link(TriggerClass *trigger);
		bool Unlink(TriggerClass *trigger);

		bool Is_One_Of_A_Kind(void);

	public:
		/*
		 * Pointer to the tag type this tag was built from. It supplies the persistence rule
		 * and the trigger types that the tag's triggers were made out of.
		 */
		TagTypeClass *Class;

		/*
		 * This is the head of the chain of triggers hanging off this tag. Every one of them
		 * is offered each event the tag springs on.
		 */
		TriggerClass *Trigger;

		/*
		 * This is the number of objects and cells that this tag is currently riding on. A
		 * "semi-persistent" tag holds its fire until only one attachment is left, so that it
		 * goes off on the last of a group rather than on the first.
		 */
		int AttachCount;

		/*
		 * This is the map cell this tag rides on. If it is CELL_NONE, then the tag is attached
		 * to an object rather than to the map.
		 */
		Cell CellID;

		/*
		 * If this tag has been condemned and is merely waiting on the list to be deleted,
		 * then this flag will be true.
		 */
		bool IsToDie;

		/*
		 * This flag is true for as long as the tag's triggers are firing. It keeps a tag from
		 * springing on an event that its own actions caused.
		 */
		bool IsCurrentlySprung;
};

inline TagClass * AbstractClass::As_TagClass(void) { return(dynamic_cast<TagClass *>(this)); }
inline TagClass const * AbstractClass::As_TagClass(void) const { return(dynamic_cast<TagClass const *>(this)); }

extern DynamicVectorClass<TagClass *> Tags;

TagClass * Find_Or_Make(TagTypeClass *type);
