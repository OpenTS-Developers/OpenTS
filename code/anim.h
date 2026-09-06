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

/* $Header: /CounterStrike/ANIM.H 1     3/03/97 10:24a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : ANIM.H                                                       *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : May 30, 1994                                                 *
 *                                                                                             *
 *                  Last Update : May 30, 1994   [JLB]                                         *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "animtype.h"
#include "bounce.h"
#include "coord.h"
#include "object.h"
#include "stage.h"

#include "draw.hh"
#include "layer.hh"

class ConvertClass;


/**********************************************************************************************
**	This is the class that controls the shape animation objects. Shape animation objects are
**	displayed over the top of the game map. Typically, they are used for explosion and fire
**	effects.
*/
class AnimClass : public ObjectClass, public StageClass
{
		typedef ObjectClass BASECLASS;

	public:

		/*
		**	This points to the type of animation object this is.
		*/
		AnimTypeClass * Class;

		AnimClass(AnimTypeClass const * type, Coord const & coord, int timedelay=0, int loop=1, ShapeFlags_Type flags=ShapeFlags_Type(SHAPE_WIN_REL|SHAPE_CENTER), int zadjust = 0);
		AnimClass(void);
		virtual ~AnimClass(void) override;

		virtual HRESULT GetClassID(CLSID * retval) override;

		virtual void Serialize(SaveStreamClass & stream) override;
		virtual void Post_Load(void) override;

		/*---------------------------------------------------------------------
		**	Member function prototypes.
		*/
		static void Post_Load_Game(void);

		void Attach_To(ObjectClass *obj);
		void Make_Invisible(void) {IsInvisible = true;};
		void Make_Visible(void) {IsInvisible = false;};
		static void Do_Atom_Damage(HousesType ownerhouse, Cell const & cell);

		virtual RTTIType Fetch_RTTI(void) const override;
		virtual void Compute_CRC(CRCEngine &) const override;
		virtual bool Limbo(void) override;
		virtual void Delete_Me(void) override;
		virtual int Get_Height(void) const override;

		virtual bool Mark(MarkType mark=MARK_CHANGE) override;
		virtual bool Render(Rect & rect, bool forced, bool extras_only = false) const override;
		virtual Coord Center_Coord(void) const override;
		virtual int Sort_Y(void) const override;
		virtual LayerType In_Which_Layer(void) const override;
		virtual ObjectTypeClass const * Class_Of(void) const override;
		virtual Cell const * Occupy_List(bool = false) const override;
		virtual void Draw_It(Point2D const & point, Rect const & cliprect) const override;
		virtual void AI(void) override;
		virtual void Detach(AbstractClass const * target, bool all) override;

		void Attach_To_Cell(int zadjust)
		{
			IsAttachedToCell = true;
			ZAdjust = zadjust;
			IsToDeleteOnOverpass = true;
		}

		void Disable(void);
		void Enable(void);

		void Vein_Attack_AI(void);
		void Flaming_Guy_AI(void);

		Coord Next_Flaming_Guy_Coord(void);
		bool Is_Valid_Flaming_Guy_Cell(Cell const & cell);

		virtual BounceResultType Bounce_AI(void);
		virtual int Stage_Count(void) const;

		/*
		**	If this animation is attached to an object, then this points to that object. An
		**	animation that is attached will follow that object as it moves. This is important
		**	for animations such as flames and smoke.
		*/
		ObjectClass * xObject;

		/*
		**	If this animation has an owner, then it will be recorded here. An owner
		**	is used when damage is caused by this animation during the middle of its
		**	animation.
		*/
		HousesType OwnerHouse;

		/*
		 * If this animation is to be drawn through a palette other than the standard animation
		 * drawer, then this points to that converter. Tiberium debris and burning victims are
		 * tinted this way. It cannot survive a save, so it is looked up again on loading.
		 */
		ConvertClass * AlternativeDrawer;

		/*
		 * This is the lighting level used when the animation has an AlternativeDrawer, where
		 * 1000 is normal brightness. A type flagged IsUseNormalLight ignores it entirely.
		 */
		unsigned AlternativeBrightness;

		/*
		 * This value biases the depth the animation's shape is written at, expressed in pixels
		 * of height. It lets the animation sort in front of or behind whatever it overlaps.
		 */
		int ZAdjust;

		/*
		 * This value is added to the animation's Y sorting position, biasing where it falls in
		 * its layer's draw order. It comes from the type, unless a building supplies its own.
		 */
		int YSortAdjust;

		/*
		 * If this is a burning victim animation, then this is the coordinate he is currently
		 * running toward -- the nearest water in sight, or failing that any neighboring cell
		 * he can still get into. If COORD_NONE, then he has nowhere left to run.
		 */
		Coord FlamingGuyCoords;

		/*
		 * This is the number of cells a burning victim has run through so far. He is allowed
		 * only so many before he gives up and collapses, so he cannot run about forever.
		 */
		int FlamingGuyRetries;

		/*
		 * If this animation is one of the animations a building owns, then this flag will be
		 * true. The building hides it when fogged, so the tactical map leaves it alone.
		 */
		bool IsBuildingAnim;

		/*
		 * This is the physics state of an animation that travels under its own momentum -- a
		 * meteor or a bouncing piece of debris. It moves the animation until it has settled.
		 */
		BounceClass Bounce;

		/*
		 * This is how transparently the animation is drawn (0 - 15), where zero is opaque and
		 * 15 means it is not drawn at all. A building fading in or out of a cloaking field
		 * passes its own level down to each of its animations so that they fade along with it.
		 */
		char TranslucencyLevel;

		/*
		**	Is this animation in a temporary suspended state?  If so, then it won't
		**	be rendered until this value is zero. The flag will be set to false
		**	after the first countdown timer reaches 0.
		*/
		int Delay;

		/*
		**	If this is an animation that damages whatever it is attached to, then this
		**	value holds the accumulation of fractional damage points. When the accumulated
		**	fractions reach 256, then one damage point is applied to the attached object.
		*/
		double Accum;

		/*
		 * These are the shape drawing flags this animation was created with: how its artwork
		 * is positioned and blended. The type's translucency is added to a copy each frame.
		 */
		ShapeFlags_Type ShapeFlags;

		/*
		 * If this animation travels under its own momentum -- a meteor or a piece of bouncing
		 * debris -- then this flag will be true. It is never placed on the map: its Bounce
		 * member carries it along, and it deletes itself once the bouncing has settled.
		 */
		bool IsBouncing;

		/*
		**	This counter tells how many more times the animation should loop before it
		**	terminates.
		*/
		unsigned char Loops;

	protected:
		void Middle(void);
		void Start(void);

	public:
		/*
		 * If this animation belongs to the terrain tile it stands on rather than to an object,
		 * then this flag will be true. It draws through the cell's own drawer and lighting.
		 */
		bool IsAttachedToCell;

		/*
		 * If this animation is to be thrown away the next time the map cleans itself up after
		 * being built, then this flag will be true. The animations a terrain tile spawns are
		 * marked so, to keep a rebuilt map from accumulating copies of them.
		 */
		bool IsToDeleteOnOverpass;

		/*
		 * If this animation is to cause no harm as it plays, then this flag will be true. An
		 * inert animation makes no sound, applies no damage to whatever it is attached to, and
		 * will not set off a tiberium chain reaction -- it is purely decoration.
		 */
		bool IsInert;

		/*
		 * Is this animation hidden under the fog of war? It is copied from the building the
		 * animation belongs to as that building passes into and out of fog, and an animation
		 * whose type is flagged IsShouldFogRemove stops being drawn while it is set.
		 */
		bool IsFogged;

		/*
		 * Has the burning victim stopped running? It is set once he reaches the water he was
		 * making for, exhausts his allowance of cells, or runs out of anywhere to go. From
		 * that point he plays out his death sequence and is deleted when it finishes.
		 */
		bool IsFlamingGuyEnd;

	private:
		/*
		**	Delete this animation at the next opportunity. This is flagged when the
		**	animation is to be prematurely ended as a result of some outside event.
		*/
		bool IsToDelete;

		/*
		**	If the animation has just been created, then don't do any animation
		**	processing until it has been through the render loop at least once.
		*/
		bool IsBrandNew;

		/*
		**	If this animation is invisible, then this flag will be true. An invisible
		**	animation is one that is created for the sole purpose of keeping all
		**	machines synchronized. It will not be displayed.
		*/
		bool IsInvisible;

		/*
		 * If this animation has been temporarily frozen, then this flag will be true. A
		 * disabled animation is still drawn, but it no longer advances through its stages --
		 * this is how a building holds its powered animations still while it has no power.
		 */
		bool IsDisabled;
};

void Shorten_Attached_Anims(ObjectClass * obj);
AnimType Anim_From_Name(char const * name);
