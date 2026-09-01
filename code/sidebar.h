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

/* $Header: /CounterStrike/SIDEBAR.H 1     3/03/97 10:25a Joe_bostic $ */
/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : SIDEBAR.H                                                    *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : October 20, 1994                                             *
 *                                                                                             *
 *                  Last Update : October 20, 1994   [JLB]                                     *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "control.h"
#include "gadget.h"
#include "power.h"
#include "shapebtn.h"
#include "stage.h"

#include "rtti.hh"
#include "super.hh"


class InitClass {};
class FactoryClass;

class SidebarClass : public PowerClass
{
		typedef PowerClass BASECLASS;

	public:
		virtual void Serialize(SaveStreamClass & stream) override;

	public:
		/*
		**	These constants are used to control the sidebar rendering. They are instantiated
		**	as enumerations since C++ cannot use "const" in this context.
		*/
		enum SideBarClassEnums {
			SIDE_Y=148,						// The Y position of sidebar upper left corner.
			SIDE_WIDTH=168,					// Width of the entire sidebar (in pixels).
			SIDE_BODY_Y=138,				/// Y position where the sidebar body (below the radar) begins.
			CREDITS_HEIGHT=16,				/// Height of the credits readout area at the top of the sidebar.

			COLUMN_ONE_X=24,				// Sidestrip upper left coordinates...
			COLUMN_ONE_Y=26,
			COLUMN_TWO_X=92,
			COLUMN_TWO_Y=26,

			BUTTON_ONE_X=31,				// Left button X coordinate.
			BUTTON_ONE_Y=-9,				// Left button Y coordinate.
			BUTTON_SPACING=27,				/// Spacing between the top row buttons (in pixels).

			GADGET_CAMEO=1000,				/// Tooltip ID of the first cameo slot.

			COLUMNS=2						// Number of side strips on sidebar.
		};

		static ShapeSet const * SidebarShape;

		/*
		 * This is the repeating middle section of the sidebar backdrop. It is stacked
		 * once for every visible object slot, so that one set of artwork covers the
		 * sidebar at whatever height the current screen resolution gives it.
		 */
		static ShapeSet const * SidebarMiddleShape;

		/*
		 * This is the bottom cap of the sidebar backdrop, drawn directly below the last
		 * of the stacked middle sections.
		 */
		static ShapeSet const * SidebarBottomShape;

		/*
		 * This is the small panel drawn below the bottom cap. It dresses up the strip of
		 * sidebar left over beneath the object slots.
		 */
		static ShapeSet const * SidebarAddonShape;

		SidebarClass(void);

		/*
		**	Initialization
		*/
		virtual void One_Time(void) override;                       // One-time inits
		virtual void Init_Clear(void) override;                     // Clears all to known state
		virtual void Init_IO(void) override;                        // Inits button list
		virtual void Init_For_House(void) override;
		void Reload_Sidebar(void);							// Loads house-specific sidebar art
		void Toggle_Cameo_Text(bool on);

		virtual void AI(KeyNumType & input, Point2D const & xy) override;
		virtual void Draw_It(bool complete) override;
		virtual void Reposition_Sidebar(void) override;
		virtual char const * Help_Text(int id) override;

		void Zoom_Mode_Control(void);
		bool Abandon_Production(RTTIType type, FactoryClass * factory);
		bool Activate(int control);
		bool Add(RTTIType type, int ID);
		bool Sidebar_Click(KeyNumType & input, int x, int y);
		void Recalc(void);
		bool Factory_Link(FactoryClass * factory, RTTIType type, int id);

		int Max_Visible(void);

		void Set_Cameo_Text(bool state);

		void Blit_Sidebar(bool complete);

		/*
		**	Each side strip is managed by this class. It handles all strip specific
		**	actions.
		*/
		class StripClass : public StageClass
		{
			public:
			class SelectClass : public ControlClass
			{
				public:
					SelectClass(void);

					void Set_Owner(StripClass & strip, int index);

					StripClass * Strip;
					int Index;

				protected:
					virtual int Action(unsigned flags, KeyNumType & key) override;
			};

			public:
				StripClass(void) {}
				StripClass(InitClass const &);

				void Serialize(SaveStreamClass & stream);

				bool Add(RTTIType type, int ID);
				bool Abandon_Production(FactoryClass const * factory);
				bool Scroll(bool up);
				bool Page(bool up);
				bool AI(KeyNumType & input, Point2D const & xy);
				char const * Help_Text(int id);
				void Draw_It(bool complete);
				void One_Time(int id);
				void Init_Clear(void);
				void Init_IO(int id);
				bool Recalc(void);
				void Activate(void);
				void Deactivate(void);
				void Flag_To_Redraw(void);
				bool Factory_Link(FactoryClass * factory, RTTIType type, int id);
				ShapeSet const * Get_Special_Cameo(SuperWeaponType type);

				/*
				**	Working numbers used when rendering and processing the side strip.
				*/
				enum SideBarGeneralEnums {
					BUTTON_UP=200,
					BUTTON_DOWN=210,
					BUTTON_SELECT=220,
					MAX_BUILDABLES=225,				// Maximum number of object types in sidebar.
					OBJECT_HEIGHT=51,				// Pixel height of each buildable object.
					OBJECT_WIDTH=64,				// Pixel width of each buildable object.
					MAX_VISIBLE=4,					// Number of object slots visible at any one time.
					MAX_SLOTS=60,					// Maximum number of object slots at any resolution.
					SCROLL_RATE=OBJECT_HEIGHT,		// The pixel jump while scrolling (larger is faster).
					UP_X_OFFSET=5,					// Scroll up arrow coordinates.
					UP_Y_OFFSET=25,					/// Y offset of the scroll arrows below the last object slot.
					DOWN_X_OFFSET=34,				// Scroll down arrow coordinates.
					DOWN_Y_OFFSET=UP_Y_OFFSET,
					TEXT_X_OFFSET=30,				// X offset to print "ready" text.
					TEXT_Y_OFFSET=2,				// Y offset to print "ready" text.
					CAMEO_TEXT_Y_OFFSET=41,			/// Y offset to print the cameo name text.
					QUEUE_COUNT_X_OFFSET=60			/// X offset to print the queued production count.
				};

				/*
				**	This is the coordinate of the upper left corner that this side strip
				**	uses for rendering.
				*/
				int X,Y;

				/*
				 * This is the area, in sidebar coordinates, covered by the strip's visible
				 * object slots, recomputed whenever the slot count changes.
				 */
				Rect ObjectRect;

				/*
				**	This is a unique identifier for the sidebar strip. Using this identifier,
				**	it is possible to differentiate the button messages that arrive from the
				**	common input button list.  It >MUST< be equal to the strip's index into
				**	the Column[] array, because the strip uses it to access the stripclass
				**	buttons.
				*/
				int ID;

				/*
				**	Shape numbers for the shapes in the STRIP.SHP file.
				*/
				enum SideBarStipShapeEnums {
					SB_BLANK,			// The blank rectangle to use if there are no objects present.
					SB_FRAME
				};

				/*
				**	If this particular side strip needs to be redrawn, then this flag
				**	will be true.
				*/
				bool IsToRedraw;

				/*
				**	If construction is in progress (no other objects in this strip can
				**	be started), then this flag will be true. It will be cleared when
				**	the strip is free to start production again.
				*/
				bool IsBuilding;

				/*
				**	This controls the sidebar slide direction. If this is true, then the sidebar
				**	will scroll downward -- revealing previous objects.
				*/
				bool IsScrollingDown;

				/*
				**	If the sidebar is scrolling, then this flag is true. Otherwise it is false.
				*/
				bool IsScrolling;

				/*
				**	This is the object (sidebar slot) that is flashing. Only one slot can be flashing
				**	at any one instant. This is usually the result of a click on the slot and construction
				**	has commenced.
				*/
				unsigned Flasher;

				/*
				**	As the sidebar scrolls up and down, this variable holds the index for the topmost
				**	visible sidebar slot.
				*/
				int TopIndex;

				/*
				**	This is the queued scroll direction and amount. The sidebar
				**	will scroll the number of slots indicated by this value. This
				**	value is set according to the scroll buttons.
				*/
				int Scroller;

				/*
				**	The sidebar has smooth scrolling. This is the number of pixels the sidebar
				**	has slide down. Thus, if this value were 5, then there would be 5 pixels of
				**	the TopIndex-1 sidebar object visible. When the Slid value reaches 24, then
				**	the value resets to zero and the TopIndex is decremented. For sliding in the
				**	opposite direction, change the IsScrollingDown flag.
				*/
				int Slid;

				/*
				**	The value of Slid the last time we rendered the sidebar.
				*/
				int LastSlid;

				/*
				**	This is the count of the number of sidebar slots that are active.
				*/
				int BuildableCount;

				/*
				**	This is the array of buildable object types. This array is sorted in the order
				**	that it is to be displayed. This array keeps track of which objects are building
				**	and ready to be placed. The very nature of this method precludes simultaneous
				**	construction of the same object type.
				*/
				struct BuildType {
					BuildType(void) :
						BuildableID(0),
						BuildableType(RTTI_NONE),
						Factory(NULL)
					{}
					BuildType(int id, RTTIType type, FactoryClass *factory=NULL) :
						BuildableID(id),
						BuildableType(type),
						Factory(factory)
					{}

					bool operator==(const BuildType & other) const {
						return(BuildableID == other.BuildableID && BuildableType == other.BuildableType);
					}

					bool operator!=(const BuildType & other) const {
						return(BuildableID != other.BuildableID || BuildableType != other.BuildableType);
					}

					// Carries the sidebar slot to or from a save game.
					template<typename S>
					void Serialize(S & stream)
					{
						stream.Serialize(BuildableID);
						stream.Serialize(BuildableType);
						stream.Serialize(Factory);
					}

					int BuildableID;
					RTTIType BuildableType;
					FactoryClass * Factory;								// Production manager.
				};
				BuildType Buildables[MAX_BUILDABLES];

				/*
				**	Pointer to the shape data for small versions of the logos. These are used as
				**	placeholder pieces on the side bar.
				*/
				static ShapeSet * LogoShapes;

				/*
				**	This points to the animation sequence of frames used to mark the passage of time
				**	as an object is undergoing construction.
				*/
				static ShapeSet const * ClockShapes;
				static ShapeSet const * RechargeClockShapes;
				static ShapeSet const * DarkenShapes;

				static ShapeButtonClass UpButton[COLUMNS];
				static ShapeButtonClass DownButton[COLUMNS];
				static SelectClass SelectButton[COLUMNS][MAX_SLOTS];

		} Column[COLUMNS];

		/*
		 * If the player has asked for cameo text, then this flag will be true. With it
		 * set, every build cameo is captioned with its name and cost, so the pop-up help
		 * for a cameo mentions only the cost.
		 */
		bool IsCameoText;

		/*
		**	If the sidebar is active then this flag is true.
		*/
		bool IsSidebarActive;

		/*
		**	This flag tells the rendering system that the sidebar needs to be redrawn.
		*/
		bool IsToRedraw;

		/*
		 * If something outside of the sidebar's own rendering has drawn over the
		 * sidebar -- a tooltip, for instance -- then this flag will be true. It forces
		 * the next render to rebuild the sidebar in full rather than just the parts
		 * that changed, and is cleared once that render has taken place.
		 */
		bool IsForceCompleteRedraw;

		/*
		 * If the only thing that changed is the credits readout, then this flag will be
		 * true. It lets the blitter copy just the credits strip at the top of the
		 * sidebar to the visible page rather than the whole sidebar.
		 */
		bool IsToRedrawCredits;

		class SBGadgetClass: public GadgetClass {
			public:
				SBGadgetClass(void);

			protected:
				virtual int Action(unsigned flags, KeyNumType & key) override;
		};

		/*
		**	This is the button that is used to collapse and expand the sidebar.
		**	These buttons must be available to derived classes, for Save/Load.
		*/
		static ShapeButtonClass Repair;
		static ShapeButtonClass Upgrade;
		static ShapeButtonClass Power;
		static ShapeButtonClass Waypoint;
		static SBGadgetClass Background;

		/*
		 * Set whenever anything rasterizes onto the sidebar surface; tells
		 * Blit_Sidebar that the surface must be copied to the visible page.
		 */
		static bool IsToBlitSidebar;

		bool Scroll(bool up, int column);
		bool Page(bool up, int column);

	private:
		bool Activate_Repair(int control);
		bool Activate_Upgrade(int control);
		bool Activate_Demolish(int control);
		int Which_Column(RTTIType type);

		bool IsRepairActive;
		bool IsUpgradeActive;
		bool IsDemolishActive;
};
